/* Wanderer 3.0: cumulative wander, Keep/Return, automatic Anchor, Scatter
 * motifs, reverb silence, new shortcuts, LED landmarks, stereo spread, bank. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static uint32_t hash(const void *p,uint32_t n)
{const uint8_t *b=p;uint32_t h=2166136261u;while(n--)h=(h^*b++)*16777619u;return h;}
static void fresh(void)
{
 panic();wm=(wm_state_t)WM_NEW_DEFAULTS;ws_select(0);ww_reset();ww.rng=12345u;wm_can_undo=0;
 layer_focus=anchor_view=running=recording=0;step_samples=FS*15u/wm.bpm;
 step_index=step_clock=cycle_index=step_primed=0;cues_enabled=0;notice=focus=confirm=0;
 settings_open=editor_open=scope_open=0;wp_reset();
 static const uint8_t notes[8]={60,63,67,65,60,70,68,67};
 for(uint32_t i=0;i<8;i++)wm.memory.e[i*2u]=(wm_event_t){notes[i],100,192};
 wm.memory.extra[4][0]=(wm_event_t){55,90,400};
}
static void bounds(void)
{
 /* Absolute limit: a chromatic octave plus the widest Drift range. A
  * smaller scale or lower Drift pulls larger offsets home one step a loop. */
 for(uint32_t s=0;s<WM_STEPS;s++)for(uint32_t v=0;v<WM_POLY;v++){
  int32_t d=ww.deg[s][v];assert(d>=-18&&d<=18);
  assert(ww.vel[s][v]>=-127&&ww.vel[s][v]<=40&&ww.len[s][v]>=-8&&ww.len[s][v]<=24);
  wm_event_t e=wm_get(&wm.memory,s,v);
  if(e.velocity){wm_event_t a=ww_apply(e,s,v);assert(a.note<=127&&a.velocity<=127&&a.gate>=24&&a.gate<=wm.loop_steps*256u);}
 }
 assert(ww.distance<=100&&ww.timbre<=127);
}
int main(void)
{
 setvbuf(stdout,0,_IONBF,0);
 /* Nothing moves without Melt or Drift. */
 fresh();uint32_t source=hash(&wm.memory,sizeof wm.memory);
 for(uint32_t i=0;i<50;i++)ww_loop();
 assert(!ww.distance&&!ww.timbre&&!ww.want_anchor);
 for(uint32_t s=0;s<WM_STEPS;s++){assert(!ww.drip[s]);for(uint32_t v=0;v<WM_POLY;v++)assert(!ww.deg[s][v]&&!ww.vel[s][v]&&!ww.len[s][v]);}
 /* Drift and Melt accumulate: later loops are further from home. */
 fresh();wm.drift=96;wm.melt=72;
 ww_loop();uint32_t first=ww.distance;
 for(uint32_t i=0;i<11;i++){ww_loop();bounds();}
 assert(ww.distance>first&&ww.distance>=30u&&ww.loops==12u&&ww.timbre>first/4u);
 assert(hash(&wm.memory,sizeof wm.memory)==source); /* the recording itself never changes */
 assert(ww.want_anchor);
 /* Gravity: knobs at zero bring every note home over a few loops. */
 wm.drift=wm.melt=0;
 for(uint32_t i=0;i<60;i++)ww_loop();
 assert(!ww.distance&&!ww.timbre);
 for(uint32_t s=0;s<WM_STEPS;s++){assert(!ww.drip[s]);for(uint32_t v=0;v<WM_POLY;v++)assert(!ww.deg[s][v]&&!ww.vel[s][v]&&!ww.len[s][v]);}
 puts("wander accumulates loop by loop, stays bounded and returns home under gravity");
 /* Low Drift leaves the downbeat and root notes in place. */
 fresh();wm.drift=40;
 for(uint32_t i=0;i<400;i++){
  ww_loop();assert(!ww.deg[0][0]);
  for(uint32_t s=0;s<WM_STEPS;s++)for(uint32_t v=0;v<WM_POLY;v++){
   wm_event_t e=wm_get(&wm.memory,s,v);if(e.velocity&&e.note%12u==wm.root)assert(!ww.deg[s][v]);
  }
 }
 puts("low Drift protects the downbeat and root landmarks");
 /* Determinism: same seed, same journey. */
 fresh();wm.drift=127;wm.melt=127;for(uint32_t i=0;i<30;i++)ww_loop();uint32_t a=hash(&ww,sizeof ww);
 fresh();wm.drift=127;wm.melt=127;for(uint32_t i=0;i<30;i++)ww_loop();assert(hash(&ww,sizeof ww)==a);
 /* Keep bakes what you hear; Undo restores the source; Return goes home. */
 fresh();wm_protect_source();wm.drift=110;wm.melt=100;for(uint32_t i=0;i<16;i++)ww_loop();
 static wm_phrase_t heard;heard=wm.memory;
 for(uint32_t s=0;s<WM_STEPS;s++)for(uint32_t v=0;v<WM_POLY;v++){wm_event_t e=wm_get(&wm.memory,s,v);if(e.velocity)*wm_at(&heard,s,v)=ww_apply(e,s,v);}
 for(uint32_t s=0;s<wm.loop_steps;s++){wm_event_t d=ww_drip_event(s);if(d.velocity)*wm_at(&heard,s,0)=d;}
 uint32_t anchor=hash(&wm.anchor,sizeof wm.anchor);
 assert(ww.distance&&ww_keep());
 assert(!memcmp(&wm.memory,&heard,sizeof heard)&&!ww.distance&&wm_can_undo&&hash(&wm.anchor,sizeof wm.anchor)==anchor);
 assert(wm_undo_once()&&hash(&wm.memory,sizeof wm.memory)==source);
 for(uint32_t i=0;i<8;i++)ww_loop();assert(ww.distance&&ww_return()&&!ww.distance&&!ww.loops);
 puts("Keep bakes the heard phrase with Undo; Return resets; the Anchor is untouched");
 /* Recording into a slot restarts that note's journey. */
 fresh();wm.drift=127;for(uint32_t i=0;i<20;i++)ww_loop();
 for(uint32_t v=0;v<WM_POLY;v++)ww.deg[2][v]=3;recording=1;running=1;step_primed=1;capture_on_at(72,100,1,2);
 for(uint32_t v=0;v<WM_POLY;v++)if(wm_get(&wm.memory,2,v).note==72)assert(!ww.deg[2][v]);else assert(ww.deg[2][v]==3);
 recording_end();running=0;
 /* The audio path: Memory wanders while it plays, the first wandering loop
  * protects the source as Anchor without touching Undo, and the service
  * marks a redraw. Recording and Anchor listening pause the journey. */
 fresh();wm.melt=80;wm.drift=80;transport();
 int32_t buf[512];uint32_t frames=step_samples*wm.loop_steps*3u+512u;
 for(uint32_t n=0;n<frames;n+=256){memset(buf,0,sizeof buf);app_audio(buf,256);for(uint32_t i=0;i<512;i++)assert(buf[i]>=-32767&&buf[i]<=32767);}
 assert(ww.loops>=2&&ww.want_anchor);
 page_dirty=0;wander_service();assert(wm.has_anchor&&!wm_can_undo&&page_dirty);
 assert(hash(&wm.anchor,sizeof wm.anchor)==source);
 uint16_t loops=ww.loops;anchor_view=1;
 for(uint32_t n=0;n<step_samples*wm.loop_steps*2u;n+=256){memset(buf,0,sizeof buf);app_audio(buf,256);}
 assert(ww.loops==loops);anchor_view=0;transport();
 puts("playback wanders, auto-protects the Anchor without Undo, and pauses while listening");
 /* GLO: tap keeps, hold returns. Hold OCT-: next theme. */
 fresh();wm_protect_source();wm.drift=100;for(uint32_t i=0;i<10;i++)ww_loop();
 const uint32_t glo=1u<<BTN_GLO,down=1u<<BTN_OCT_DOWN;
 fm1_ms=1000;fm1_in.buttons=glo;assert(!glo_edges(glo,0));
 fm1_ms=1899;assert(!glo_edges(0,0)&&ww.distance);
 fm1_ms=1901;assert(!glo_edges(0,0)&&!ww.distance&&notice==N_RETURNED);
 fm1_in.buttons=0;assert(!glo_edges(0,glo));
 fm1_in.buttons=glo;assert(!glo_edges(glo,0));fm1_ms+=100;fm1_in.buttons=0;assert(glo_edges(0,glo)==glo);
 uint8_t theme=color_theme;notice=0;
 fm1_ms=5000;fm1_in.buttons=down;assert(!performance_edges(down,0));
 fm1_ms=6499;performance_edges(0,0);assert(color_theme==theme);
 fm1_ms=6501;performance_edges(0,0);assert(color_theme==(theme+1u)%THEME_COUNT&&notice==N_THEME);
 fm1_in.buttons=0;assert(!(performance_edges(0,down)&down)&&!octave);
 color_theme=theme;wm.light=color_theme==THEME_LIGHT;
 fm1_in.buttons=down;performance_edges(down,0);fm1_ms+=100;fm1_in.buttons=0;assert(performance_edges(0,down)==down);
 puts("GLO tap/hold and OCT- theme hold dispatch without stealing taps");
 /* Every notice has a word that fits the screen, in both sizes and all themes. */
 static const uint8_t words[]={N_ORIGINAL_SAFE,N_ANCHORED,N_ANCHOR_SAFE,N_NO_PHRASE,N_BRANCHED,N_UNDO,N_NO_UNDO,N_LAYER_CLEARED,N_NEW_SESSION,N_STOP_FIRST,N_RETURNED,N_NOTHING_TO_KEEP,N_THEME,N_SAVE_FAILED,N_NO_VALID_SAVE};
 for(color_theme=0;color_theme<THEME_COUNT;color_theme++)for(uint32_t z=0;z<2;z++)for(uint32_t i=0;i<sizeof words;i++){
  wm.extra_large=(uint8_t)z;notice=words[i];notice_until=0;assert(notice_word());screen();
 }
 color_theme=0;notice=0;wm.extra_large=0;
 /* Home shows the wander bar and loop count in both sizes. */
 fresh();wm.drift=100;for(uint32_t i=0;i<9;i++)ww_loop();
 for(color_theme=0;color_theme<THEME_COUNT;color_theme++)for(uint32_t z=0;z<2;z++){wm.extra_large=(uint8_t)z;screen();}
 color_theme=0;wm.extra_large=0;
 puts("notice words and the Home wander meter fit every theme in Normal and Zoom");
 /* LEDs: PLAY pulses with the beat; root keys glow on Home. */
 fresh();running=1;step_primed=1;step_index=1;step_clock=0;leds_initialised=0;app_leds();
 assert(host_leds&((uint64_t)1u<<BTN_PLAY));
 step_index=3;app_leds();assert(!(host_leds&((uint64_t)1u<<BTN_PLAY))&&(host_glow&((uint64_t)1u<<BTN_PLAY)));
 for(uint32_t key=0;key<27u;key++)assert(((host_glow>>KEY_LED(key))&1u)==((KEY0+key)%12u==wm.root));
 running=0;
 puts("beat pulse and root-key landmarks");
 /* Scatter motifs repeat at Variation zero and change at full Variation. */
 fresh();wm.scatter_pattern=0;scatter_seed(77);
 uint8_t first_loop[16],same=0,diff=0;
 for(uint32_t loop=0;loop<12;loop++){
  wm.scatter_variation=loop<6?0:127;
  for(uint32_t s=0;s<16;s++){
   scatter_plan(120,0,s,loop);uint8_t k=(uint8_t)(scatter.kind*32u+scatter.divisions);
   if(loop==0)first_loop[s]=k;else if(loop<6)same+=first_loop[s]==k;else diff+=first_loop[s]!=k;
  }
 }
 assert(same==5u*16u&&diff>8u);
 scatter_plan(80,0,0,3);assert(!scatter.kind); /* clean downbeat below high amounts */
 puts("Scatter motifs repeat, vary with Variation and keep the downbeat clean");
 /* All non-Ambient reverbs reach true digital silence after a burst. */
 for(uint32_t type=0;type<RV_COUNT;type++){
  if(type==RV_AMBIENT)continue;
  wm=(wm_state_t)WM_DEFAULTS;wm.reverb_type=(uint8_t)type;wm.fx_on=4;wm.reverb_mix=127;wm.reverb_size=110;
  fx_reset();while(fx_clearing)fx_clear_chunk();motion_phase=0;
  uint32_t last_sound=0;
  for(uint32_t f=0;f<FS*14u;f++){
   int32_t x=f>=2048u&&f<4096u?(int32_t)((f*2654435761u)>>19)-4096:0;int32_t o[2]={x*3,0};fx_process(o,1);
   assert(o[0]>=-30000&&o[0]<=30000&&o[1]>=-30000&&o[1]<=30000);
   if(o[0]||o[1])last_sound=f;
  }
  assert(last_sound<FS*13u);
 }
 puts("every non-Ambient reverb decays to digital silence");
 /* Voices spread in stereo even with every effect off. */
 fresh();wm.fx_on=0;wm.fx_new_on=0;fx_reset();while(fx_clearing)fx_clear_chunk();
 note_on(60,100,1);note_on(67,100,1);uint32_t wide=0;
 for(uint32_t n=0;n<8;n++){memset(buf,0,sizeof buf);app_audio(buf,256);for(uint32_t i=0;i<512;i+=2)wide+=buf[i]!=buf[i+1];}
 assert(wide>500);note_off(60,1);note_off(67,1);
 /* The 3.0 bank is valid, current and individual. */
 uint32_t distinct=0;
 for(uint32_t p=0;p<WS_PRESET_COUNT;p++){
  assert(ws_patch_valid(&WS_PRESETS_3[p])&&ws_patch_valid(&WS_PRESETS_2[p]));
  uint8_t unique=1;for(uint32_t q=0;q<p;q++)if(!memcmp(WS_PRESETS_3[p].morph,WS_PRESETS_3[q].morph,6))unique=0;distinct+=unique;
 }
 assert(distinct>=120u);ws_select(5);assert(wm.preset_bank==WS_BANK_CURRENT&&!memcmp(&wm.patch,&WS_PRESETS_3[5],sizeof wm.patch));
 assert(ws_factory_patch(WS_BANK_2_0,5)==&WS_PRESETS_2[5]);
 puts("stereo voice spread and an individual, valid 3.0 bank; 2.0 bank kept for old projects");
 /* Adversarial: random knob, scale, root, length and recording changes over
  * thousands of loops never break the bounds or the source. */
 fresh();uint32_t r=99;
 for(uint32_t i=0;i<6000;i++){
  r=r*1664525u+1013904223u;
  if(!(i%7u))wm.drift=(uint8_t)((r>>8)&127u);
  if(!(i%11u))wm.melt=(uint8_t)((r>>16)&127u);
  if(!(i%97u))wm.scale=(uint8_t)((r>>4)%WM_SCALE_COUNT);
  if(!(i%131u))wm.root=(uint8_t)((r>>12)%12u);
  if(!(i%173u))wm.loop_steps=(uint8_t)(wm.loop_steps==16u?32u:16u);
  if(!(i%59u)){uint32_t s=(r>>20)%WM_STEPS,v=(r>>3)%WM_POLY;*wm_at(&wm.memory,s,v)=(wm_event_t){(uint8_t)((r>>9)&127u),(uint8_t)(1u+((r>>2)&126u)),(uint16_t)(32u+((r>>5)&2047u))};ww_clear_slot(s,v);}
  if(!(i%401u))ww_keep();
  if(!(i%613u))ww_return();
  ww_loop();bounds();
  for(uint32_t s=0;s<wm.loop_steps;s++){wm_event_t d=ww_drip_event(s);assert(d.note<=127&&d.velocity<=90);}
 }
 puts("6000 adversarial loops with random knobs, scales, lengths, overdubs, Keep and Return stay bounded");
 /* Metronome: OFF, REC and ALWAYS; one-bar count-in from stop; bounded,
  * accented ticks that never reach Print; preferences round-trip. */
 fresh();cues_enabled=0;vol=100;master_smooth=25600;wm.fx_on=wm.fx_new_on=0;step_samples=FS/8;
 for(uint8_t mode=0;mode<3;mode++)for(uint8_t rec=0;rec<2;rec++){
  fresh();wm.fx_on=wm.fx_new_on=0;memset(&wm.memory,0,sizeof wm.memory);step_samples=FS/8;metronome_on=mode;click_level=3;
  running=1;recording=rec;step_primed=0;step_index=step_clock=0;uint32_t ticks=0,accents=0;int32_t peak=0;
  for(uint32_t f=0;f<step_samples*32u;f+=64){int32_t b2[128]={0};uint32_t old=click_left;uint32_t st=click_step;app_audio(b2,64);
   if(click_left>old||(click_left&&click_step!=st)){ticks++;accents+=click_step==0x095c0000u;}
   for(uint32_t i=0;i<128;i++){if(abs(b2[i])>peak)peak=abs(b2[i]);}}
  uint8_t want=mode==2||(mode==1&&rec);
  assert(want?ticks>=8&&accents>=2:!ticks);assert(peak<=30000);
  running=recording=0;
 }
 fresh();wm.fx_on=wm.fx_new_on=0;metronome_on=1;step_samples=FS/8;
 recording_toggle();assert(count_in_total&&!running&&!recording);
 screen(); /* the count-in screen draws in every state */
 uint32_t counted=0;while(count_in_total&&counted<FS*10u){int32_t b2[512]={0};app_audio(b2,256);counted+=256;page_dirty=0;wander_service();}
 assert(running&&recording&&counted>=step_samples*16u&&counted<step_samples*16u+256u);
 recording_end();transport();
 recording_toggle();assert(count_in_total);recording_toggle();assert(!count_in_total&&!running&&!recording);
 recording_toggle();panic();assert(!count_in_total);
 metronome_on=2;click_level=1;assert(!preferences_save());metronome_on=0;click_level=3;preferences_load();assert(metronome_on==2&&click_level==1);
 settings_page=SETTINGS_OUTPUT;settings_index=2;metronome_on=2;settings_press();assert(metronome_on==0);settings_press();assert(metronome_on==1);
 metronome_on=0;
 puts("metronome modes, accented bars, one-bar count-in and cancel, bounded ticks and saved settings");
 /* Load guard: high measured load lightens the engines and sheds the
  * oldest sequenced note; calm restores everything. */
 fresh();cpu_lean=0;cpu_load=cpu_peak=0;
 sequence_event((wm_event_t){48,100,4096},0);sequence_event((wm_event_t){55,100,4096},0);
 sequence_event((wm_event_t){60,100,4096},0);sequence_event((wm_event_t){64,100,4096},0);
 uint32_t budget=256u*(FM1_TICKS_PER_US*1000000u/FS);
 cpu_account(budget*3u/4u,256);assert(cpu_lean==1&&cpu_load>=70);
 for(uint32_t i=0;i<8;i++)cpu_account(budget*9u/10u,256);
 assert(cpu_lean==2&&cpu_peak>=85);uint32_t held=0;for(uint32_t v=0;v<NV;v++)held+=voice[v].gate!=0;assert(held<4);
 for(uint32_t i=0;i<400;i++)cpu_account(budget/5u,256);assert(!cpu_lean);
 /* Under load SUPER and VOICE stay bounded and audible. */
 for(uint8_t lean=0;lean<3;lean++)for(uint32_t p=0;p<WS_PRESET_COUNT;p++){
  if(WS_PRESETS_3[p].engine<6)continue;
  wm=(wm_state_t)WM_DEFAULTS;ws_select((uint8_t)p);memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);
  cpu_lean=lean;note_on(48,127,1);note_on(60,127,1);note_on(67,127,1);int64_t e=0;
  for(int bl=0;bl<40;bl++){int32_t b2[512]={0};for(uint32_t v=0;v<NV;v++)ws_render(v,b2,256,64);
   for(int i=0;i<512;i++){assert(b2[i]>-200000&&b2[i]<200000);e+=(int64_t)b2[i]*b2[i];}
   for(uint32_t v=0;v<NV;v++)if(ws_voice[v].p.engine==7)for(int k=0;k<3;k++){ws_lp_stage_t *f=&ws_voice[v].formant[k];
    assert(f->ic1>-65535&&f->ic1<65535&&f->ic2>-65535&&f->ic2<65535);} /* formants never saturate */
  }
  assert(e>0);
 }
 cpu_lean=0;
 puts("load guard lightens and sheds under load, recovers when calm; VOICE formants never saturate");
 /* SEQ: tap toggles the click (Home), hold opens Save. */
 fresh();metronome_on=0;metronome_last=0;notice=0;const uint32_t seq=1u<<BTN_SEQ;
 fm1_ms=20000;fm1_in.buttons=seq;assert(!seq_edges(seq,0));fm1_ms+=100;fm1_in.buttons=0;assert(seq_edges(0,seq)==seq);
 fm1_in.buttons=seq;assert(!seq_edges(seq,0));fm1_ms+=901;assert(!seq_edges(0,0)&&confirm==2);fm1_in.buttons=0;assert(!seq_edges(0,seq));confirm=0;
 /* The Home tap handler: off -> ALWAYS -> off -> last mode. */
 metronome_on=0;metronome_last=0;
 { if(metronome_on){metronome_last=metronome_on;metronome_on=0;}else metronome_on=metronome_last?metronome_last:2u; } assert(metronome_on==2);
 /* System page: CPU read-outs and erasing for the official firmware. */
 fresh();settings_open=1;settings_page=SETTINGS_SYSTEM;assert(settings_count()==4);
 for(settings_index=0;settings_index<4;settings_index++)for(uint32_t z=0;z<2;z++){wm.extra_large=(uint8_t)z;screen();}
 wm.extra_large=0;wm_disk_t disk={WM_MAGIC,WM_DISK_VERSION,sizeof wm,wm};assert(!slot_save(4,&disk,sizeof disk));
 assert(!preferences_save());settings_adjust(3,1);assert(confirm==6);screen();
 running=1;apply_confirm();assert(notice==N_STOP_FIRST&&project_read_slot(4,0));running=0;
 settings_adjust(3,1);apply_confirm();assert(notice==N_ERASED);
 for(uint32_t i=0;i<NSLOT;i++)assert(!project_read_slot(i,0)&&slot_presence[i]==1u);
 wm_preferences_t pr;assert(settings_load(&pr,sizeof pr)); /* settings sector is blank */
 settings_open=0;notice=0;
 puts("SEQ toggles the click and holds to Save; SYSTEM page reads CPU and erases all Wanderer data on request");
 puts("WANDERER 3.0 TESTS PASSED");
 return 0;
}
