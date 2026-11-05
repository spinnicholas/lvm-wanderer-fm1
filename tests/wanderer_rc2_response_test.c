#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static void defaults(void){wm=(wm_state_t)WM_DEFAULTS;reset_performance();wp_reset();ws_select(0);wm.fx_on=wm.fx_new_on=0;cues_enabled=0;cue_stop();vol=100;master_smooth=25600;running=recording=0;metronome_on=0;prev_notes=host_note_latched=fm1_in.notes=0;}
static uint32_t gates(void){uint32_t n=0;for(uint32_t v=0;v<NV;v++)n+=voice[v].gate!=0;return n;}
static void render(int32_t *p,uint32_t n){memset(p,0,n*2*sizeof *p);app_audio(p,n);}
int main(void){
 defaults();fm1_in.notes=host_note_latched=0x95;app_idle();assert(gates()==4); /* During display service, before the outer loop. */
 fm1_in.notes=0;app_idle();assert(!gates());
 host_note_latched=2;app_idle();assert(gates()==1);int32_t out[512];for(int i=0;i<16;i++)render(out,256);assert(!gates());
 defaults();metronome_on=1;assert(!preferences_save());metronome_on=0;preferences_load();assert(metronome_on);
 settings_page=SETTINGS_OUTPUT;assert(settings_count()==3);settings_adjust(2,-1);assert(!metronome_on);settings_adjust(2,1);assert(metronome_on);
 uint32_t seen=0;settings_page=0;for(uint32_t i=0;i<SETTINGS_PAGES;i++){assert(!(seen&(1u<<settings_page)));seen|=1u<<settings_page;settings_page=settings_move(1);}assert(seen==((1u<<SETTINGS_PAGES)-1)&&settings_page==0);
 settings_page=13;uint8_t order[]={9,12,1,2,10,11,3};for(uint32_t i=0;i<sizeof order;i++){settings_page=settings_move(1);assert(settings_page==order[i]);}
 for(uint32_t page=0;page<SETTINGS_PAGES;page++)for(uint32_t zoom=0;zoom<2;zoom++){settings_page=page;settings_open=1;wm.extra_large=zoom;settings_index=settings_count()-1u;screen();}settings_open=0;
 defaults();metronome_on=1;running=recording=1;step_index=step_clock=step_primed=0;step_samples=FS/8;
 render(out,256);uint64_t e=0;for(int i=0;i<512;i++)e+=(int64_t)out[i]*out[i];assert(e>10000);for(uint32_t j=0;j<WM_STEPS;j++)assert(!wm_step_notes(&wm.memory,j)); /* Monitoring only; no events. */
 recording=0;render(out,256);for(int i=0;i<512;i++)assert(!out[i]);
 /* Sample-accurate beats do not depend on callback length. */
 for(uint32_t enabled=0;enabled<2;enabled++){defaults();metronome_on=enabled;running=recording=1;step_index=step_clock=step_primed=0;step_samples=FS/8;uint32_t beats=0;
  for(uint32_t f=0;f<step_samples*8u;f++){int32_t p[2]={0};uint32_t old=click_left;app_audio(p,1);if(click_left>old)beats++;if(!enabled)assert(!p[0]&&!p[1]);}assert(beats==(enabled?2:0));}
 defaults();anchor_view=1;wm.has_anchor=1;notice=0;
 recording_toggle();assert(!recording&&!running&&notice==N_ANCHOR_LISTEN);
 listening_set(0);assert(!notice);
 recording_toggle();assert(recording&&running);
 recording_end();transport();
 wm_mark();tempo_set(165);step_clock=step_samples/3u;
 uint32_t old_clock=step_clock,old_duration=step_samples;
 assert(undo_events_locked());
 assert(step_samples==FS*15u/wm.bpm&&step_clock==old_clock*step_samples/old_duration);
 assert(screen_refresh_interval()==0);
 running=1;assert(screen_refresh_interval()==250u);
 editor_open=1;assert(screen_refresh_interval()==0);editor_open=0;
 scope_set_open(1);assert(screen_refresh_interval()==80u);
 wm.reduced_motion=1;assert(screen_refresh_interval()==250u);
 scope_set_hold(1);assert(screen_refresh_interval()==0);
 scope_set_open(0);assert(screen_refresh_interval()==0);
 puts("Anchor recording guard, Undo timing and redraw policy pass");
 puts("Display-time four-key onset/release/tap, metronome opt-in/persistence/record-only timing, page grouping and both display sizes pass");
}
