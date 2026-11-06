/* Integration assertions against production input, scheduler, DSP and persistence. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static void frames(uint32_t count){while(count){uint32_t n=count>256?256:count;int32_t o[512]={0};app_audio(o,n);for(uint32_t i=0;i<n*2;i++)assert(abs(o[i])<=32767);count-=n;}}
static void fresh(void){running=recording=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);memset(arp_configured,0,sizeof arp_configured);arp_clock_seen=0;arp_clock_quarter=0;ages=audio_frames=0;step_index=step_clock=cycle_index=step_primed=0;step_samples=FS*15/wm.bpm;ws_select(0);wm.fx_on=0;cues_enabled=0;confirm=settings_open=editor_open=anchor_view=0;}
static void expect_pattern(uint8_t mode,const uint8_t *expected,uint32_t count){fresh();wm.arp_on=1;wm.arp_mode=mode;performance_on(60,100,1);performance_on(64,100,1);performance_on(67,100,1);for(uint32_t i=0;i<count;i++){frames(1);assert(arp_note==expected[i]);uint32_t n=arp_until;frames(n);}}
int main(void){
 uint8_t up[]={60,64,67,60},down[]={67,64,60,67},bounce[]={60,64,67,64,60},doubleup[]={60,60,64,64,67,67},repeat[]={60,64,67,67,64,60,60};
 expect_pattern(0,up,4);expect_pattern(1,down,4);expect_pattern(2,bounce,5);expect_pattern(4,doubleup,6);expect_pattern(10,repeat,7);
 fresh();wm.arp_on=1;wm.arp_mode=6;performance_on(67,100,1);performance_on(60,100,1);performance_on(64,100,1);frames(1);assert(arp_note==67);frames(arp_until+1);assert(arp_note==60);
 for(uint32_t mode=0;mode<WM_ARP_MODES;mode++)for(uint32_t rate=0;rate<WM_ARP_RATES;rate++){
  fresh();wm.arp_on=1;wm.arp_mode=mode;wm.arp_rate=rate;wm.arp_octaves=4;wm.arp_gate=100;wm.arp_swing=50;
  for(uint32_t n=0;n<80;n++)performance_on(48+n%80,127,2+n%32);assert(arp_count==64);
  frames(step_samples*8u);assert(arp_note<=127);panic();assert(!arp_count&&arp_note==128);for(uint32_t v=0;v<NV;v++)assert(!voice[v].gate);
 }
 fresh();wm.arp_on=1;wm.arp_latch=1;performance_on(60,100,1);performance_off(60,1,0);frames(1);assert(arp_count==1&&arp_note==60);performance_on(65,100,1);assert(arp_count==1);frames(arp_until+1);assert(arp_note==65);wm.arp_latch=0;arp_update_config();performance_off(65,1,0);assert(!arp_count&&arp_note==128);
 fresh();wm.arp_on=1;midi_message(1,0x90,60,100);midi_message(2,0x90,60,100);midi_message(1,0x80,60,0);assert(arp_count==1&&arp_keys[0].owner==18);midi_message(2,0xb0,64,127);midi_message(2,0x80,60,0);assert(arp_count==1);midi_message(2,0xb0,64,0);assert(!arp_count);
 fresh();wm.arp_on=1;wm.arp_latch=1;midi_message(1,0x90,60,100);midi_message(1,0xb0,123,0);assert(!arp_count);
 fresh();wm.arp_on=1;recording_toggle();performance_on(60,100,1);performance_on(64,100,1);frames(step_samples*8);performance_off(60,1,0);performance_off(64,1,0);capture_finish_all();uint32_t events=0;for(uint32_t i=0;i<WM_STEPS;i++)for(uint32_t v=0;v<WM_POLY;v++){wm_event_t e=wm_get(&wm.memory,i,v);if(e.velocity){events++;assert(e.note==60||e.note==64);assert(e.gate>=32&&e.gate<512);}}assert(events>=6);transport();assert(!arp_count&&arp_note==128);
 fresh();wm.arp_on=1;wm.arp_clock=1;performance_on(60,100,1);frames(256);assert(arp_note==128);midi_message(2,0xf8,0,0);assert(!arp_clock_seen);midi_message(1,0xfa,0,0);midi_message(1,0xf8,0,0);for(uint32_t k=0;k<30;k++){frames(FS/48);fm1_ms+=21;midi_message(1,0xf8,0,0);}assert(arp_clock_seen&&arp_clock_quarter>FS/3&&arp_clock_quarter<FS);midi_message(1,0xfc,0,0);assert(!running&&!arp_count);
 for(uint32_t page=0;page<SETTINGS_PAGES;page++)for(uint32_t k=0;k<4;k++){fresh();settings_page=page;settings_adjust(k,100000);settings_adjust(k,-100000);wm_disk_t disk={WM_MAGIC,WM_DISK_VERSION,sizeof wm,wm};assert(wm_valid(&disk));}
 fresh();for(uint32_t mode=0;mode<WM_SCATTER_MODES;mode++)for(uint32_t size=0;size<WM_GRAIN_COUNT;size++){
  wm.repeat_style=mode;wm.grain_size=size;wm.decay=127;wm.scatter_edge=mode&1?0:127;wm.memory.e[0]=(wm_event_t){60,127,8192};running=1;frames(step_samples*32);running=0;reset_performance();}
 fresh();wm.memory.e[0]=(wm_event_t){60,120,1024};wm.memory.e[8]=(wm_event_t){67,110,1024};assert(wp_arm());running=1;frames(step_samples*16+1);assert(wp_has_anchor&&wp_active);
 uint32_t protected_audio=0;for(uint32_t i=0;i<sizeof wp_arena[0];i++)protected_audio=protected_audio*33u+wp_arena[0][i];
 for(uint32_t mode=0;mode<WM_SCATTER_MODES;mode++)for(uint32_t size=0;size<WM_GRAIN_COUNT;size++){
  wm.repeat_style=mode;wm.grain_size=size;wm.decay=127;wm.scatter_edge=mode&1u?0:127;frames(step_samples*16);
  uint32_t h=0;for(uint32_t i=0;i<sizeof wp_arena[0];i++)h=h*33u+wp_arena[0][i];assert(h==protected_audio);
 }
 assert(wp_arm());frames(step_samples*32);assert(wp_undo_valid&&wp_undo()&&wp_undo());
 uint32_t after_print=0;for(uint32_t i=0;i<sizeof wp_arena[0];i++)after_print=after_print*33u+wp_arena[0][i];assert(after_print==protected_audio);panic();
 puts("1.4 patterns, all mode/rate combinations, extreme chords/gates/swing, MIDI ownership/pedal/latch, generated recording, external clock, control bounds and Scatter matrix pass");return 0;
}
