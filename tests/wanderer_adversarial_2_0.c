/* Deterministic stress against production scheduling, DSP and UI. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static uint32_t seed=0x826d1a7u;
static uint32_t random32(void)
{seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void render(uint32_t n)
{
 int32_t out[514]={0};
 out[0]=0x123456;out[2u*n+1u]=0x654321;
 app_audio(out+1,n);
 assert(out[0]==0x123456&&out[2u*n+1u]==0x654321);
 for(uint32_t i=1;i<=2u*n;i++)assert(out[i]>=-30000&&out[i]<=30000);
}
int main(void)
{
 const uint32_t oct=1u<<BTN_OCT_UP,play=1u<<BTN_PLAY;
 /* Press at wrap, consume on a late release, and cancel a pending play on panic. */
 fm1_ms=UINT32_MAX;fm1_in.buttons=oct;
 assert(!performance_edges(oct,0));fm1_ms+=1501u;fm1_in.buttons=0;
 assert(!performance_edges(0,oct)&&wm.extra_large);
 fm1_in.buttons=play;assert(!performance_edges(play,0));panic();
 fm1_ms+=2200u;fm1_in.buttons=0;assert(!performance_edges(0,play)&&!running);
 /* A held button in a destructive confirmation only stops playback. */
 ws_select(12);confirm=2;fm1_in.buttons=play;assert(!performance_edges(play,0));
 fm1_ms+=2100u;assert(!performance_edges(0,0)&&wm.sound==12);
 fm1_in.buttons=0;assert(!performance_edges(0,play));confirm=0;
 /* Scope/clock redraw and clock switching with active transport. */
 wm.arp_clock=1;arp_update_config();page_dirty=0;
 arp_realtime(1,0xfe);assert(!page_dirty);
 arp_realtime(1,0xf8);audio_frames+=918u;arp_realtime(1,0xf8);
 assert(arp_clock_quarter==918u*24u&&!page_dirty);
 arp_realtime(1,0xfa);assert(running&&page_dirty&&!step_primed);
 wm.arp_clock=2;arp_update_config();assert(!arp_clock_seen&&!arp_clock_quarter);
 arp_realtime(1,0xfc);assert(running);
 arp_realtime(2,0xfc);assert(!running);
 /* Change from the end of a slow step to triple speed without losing phase. */
 tempo_set(60);transport();step_clock=step_samples-1u;step_primed=1;
 tempo_set(180);assert(step_clock==step_samples-1u);
 render(9);assert(step_index==1&&step_clock==8);transport();
 /* Print cannot change length or tempo during its first capture. A
  * MIDI Start in the middle aborts the incomplete generation. */
 wm.memory.e[0]=(wm_event_t){60,109,192};assert(wp_arm());
 tempo_set(90);length_adjust_locked(1);
 assert(wm.bpm==180&&wm.loop_steps==16&&notice==N_AUDIO_LOCK);
 transport();render(1);assert(wp_writing&&!wp_has_anchor);
 arp_realtime(2,0xfa);assert(!wp_writing&&!wp_has_anchor&&!wp_active);
 transport();notice=0;
 /* CRC-valid slot payloads can still contain invalid state. Reject each
  * hostile field before it reaches a table index or scheduler. */
 static wm_disk_t disk={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),WM_DEFAULTS};
 static wm_state_t loaded=WM_DEFAULTS;
 assert(wm_valid(&disk));
 uint8_t *fields[]={&disk.state.layer_sound,&disk.state.midi_input,
                   &disk.state.midi_channel,&disk.state.loop_steps,
                   &disk.state.reverb_type,&disk.state.grain_size};
 for(uint32_t i=0;i<sizeof fields/sizeof fields[0];i++){
  uint8_t prior=*fields[i];*fields[i]=255;
  assert(!wm_valid(&disk)&&!slot_save(5,&disk,sizeof disk));
  assert(!project_read_slot(5,&loaded)&&loaded.loop_steps==16);
  *fields[i]=prior;
 }
 /* Anchor playback and Layer audition must retain separate patch snapshots. */
 wm.memory.e[0]=(wm_event_t){60,109,192};ws_select(0);assert(wm_catch());
 anchor_view=1;uint32_t anchor=note_on(60,100,1);
 assert(voice[anchor].patch_source==PATCH_ANCHOR);
 settings_open=1;settings_page=SETTINGS_LAYER;layer_target_locked(1);wm.layer_sound=35;
 uint32_t layer=note_on(62,100,1);assert(voice[layer].patch_source==PATCH_LAYER);
 wm.layer_sound=0;uint32_t follow=note_on(64,100,1);assert(voice[follow].patch_source==PATCH_MEMORY);
 anchor_view=settings_open=0;layer_target_locked(0);editor_page=1;editor_adjust(0,2);
 assert(ws_voice[anchor].p.cutoff==wm.anchor_patch.cutoff);
 assert(!memcmp(&ws_voice[layer].p,&WS_PRESETS_3[34],sizeof(ws_patch_t)));
 reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;
 wm.fx_on=7;wm.fx_new_on=15;wm.layer_enabled=1;wm.layer_sound=44;
 wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;
 wm.crush_mix=wm.tape_mix=wm.phaser_mix=wm.fold_mix=127;
 wm.tape_feedback=wm.delay_feedback=wm.fold_drive=127;
 wm.tape_heads=2;wm.reverb_size=127;vol=127;cues_enabled=0;
 for(uint32_t i=0;i<16;i++){
  wm.memory.e[i]=(wm_event_t){(uint8_t)(42+i),120,256};
  wm.layer.e[i]=(wm_event_t){(uint8_t)(54+i),127,512};
 }
 transport();
 for(uint32_t i=0;i<16000u;i++){
  uint32_t r=random32();
  if((i&31u)==0){
   ws_select((uint8_t)(r%WM_PRESET_COUNT));wm.reverb_type=(uint8_t)((r>>8)%RV_COUNT);
   wm.grain_size=(uint8_t)((r>>16)%WM_GRAIN_COUNT);
   wm.repeat_style=(uint8_t)((r>>19)%WM_SCATTER_MODES);wm.scatter_pattern=(uint8_t)((r>>23)%WM_PATTERN_COUNT);
   wm.melt=(uint8_t)(r&127u);wm.drift=(uint8_t)((r>>7)&127u);wm.decay=(uint8_t)((r>>14)&127u);
   wm.layer_sound=(uint8_t)((r>>16)%(WM_PRESET_COUNT+1));
  }
  if((i&127u)==0&&!wp_timing_locked())tempo_set((uint8_t)(60u+(r%121u)));
  if(i==512u)(void)wp_arm();
  if((i&255u)==0&&wp_undo_valid)(void)wp_undo();
  if((i&7u)==0)performance_on((r>>16)&127u,127,1);
  if((i&7u)==4u)for(uint32_t n=0;n<NV;n++)if(voice[n].live==1u)performance_off(voice[n].note,1,0);
  render(1u+(r&255u));
  assert(step_clock<=step_samples&&step_index<wm.loop_steps&&wp_ring_write<WP_RING_SAMPLES);
 }
 assert(wp_has_anchor);
 panic();render(256);
 for(uint32_t n=0;n<NV;n++)assert(!voice[n].gate&&!voice[n].amp);
 puts("Late releases, timer wrap, panic, MIDI redraw/clock, tempo phase, layer audition, malformed slots and 16,000 randomized callbacks passed.");
}
