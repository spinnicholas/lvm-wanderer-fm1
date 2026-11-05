/* Frequency-response and live-editor regression for the new low-pass modes. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static uint64_t measure(uint8_t mode,uint8_t cut,uint8_t res,uint32_t hz)
{
 wm=(wm_state_t)WM_DEFAULTS;ws_select(30);
 wm.patch.filter=mode;wm.patch.cutoff=cut;wm.patch.resonance=res;
 wm.patch.position=wm.patch.envdepth=wm.patch.lfo_depth=wm.patch.drive=0;
 voice[0].note=60;voice[0].inc=tone_step(60);
 ws_start(0,60,100);ws_control(0,1,0);
 uint64_t sum=0;
 uint32_t phase=0,inc=(uint32_t)((((uint64_t)hz)<<32)/FS);
 for(uint32_t i=0;i<16384;i++){
  int32_t y=wn_filter(&ws_voice[0],wn_sine_i(phase)/12);
  phase+=inc;
  if(i>=8192)sum+=(uint32_t)(y*y);
 }
 return sum;
}
int main(void)
{
 const uint8_t cut=60;
 uint32_t hz=WN_LP_HZ[cut];
 uint64_t d12=measure(WS_LP12,cut,0,hz*4),d24=measure(WS_LP24,cut,0,hz*4);
 assert(d12>64u*d24);
 for(uint8_t mode=WS_LP12;mode<=WS_LP24;mode++){
  uint64_t bass=measure(mode,cut,0,55),bass_res=measure(mode,cut,127,55);
  assert(bass_res*100u>bass*95u&&bass_res*100u<bass*107u);
  uint64_t neutral=measure(mode,cut,0,hz),peak=measure(mode,cut,127,hz);
  assert(peak>5u*neutral);
  assert(measure(mode,105,0,2500)>100u*measure(mode,30,0,2500));
  for(uint8_t i=0;i<127;i+=21){
   measure(mode,i,127,WN_LP_HZ[i]);
   measure(mode,i,127,12000);
  }
  printf("LP%u: cutoff %u Hz, bass ratio %llu%%, peak energy %llux\n",
   mode==WS_LP12?12:24,hz,(unsigned long long)(bass_res*100/bass),(unsigned long long)(peak/neutral));
 }
 editor_open=1;editor_page=1;editor_selected=2;fm1_ms=100;
 wm=(wm_state_t)WM_DEFAULTS;ws_select(30);wm.patch.filter=WS_LOW;
 arp_update_config();uint32_t held=note_on(60,108,1);assert(held<NV);int32_t audio[512]={0};app_audio(audio,256);
 editor_adjust(2,100);assert(wm.patch.filter==WS_LP24);
 assert(ws_voice[held].p.filter==WS_LP24&&voice[held].splice_left==32);
 char value[16];const char *unit;
 assert(!strcmp(editor_value(2,value,&unit),"LP 24"));
 editor_adjust(2,-1);assert(!strcmp(editor_value(2,value,&unit),"LP 12"));
 for(wm.extra_large=0;wm.extra_large<2u;wm.extra_large++){screen();}
 wm.extra_large=0;
 wm.memory.e[0]=(wm_event_t){60,100,256};assert(wm_catch());
 assert(wm.anchor_patch.filter==WS_LP12);
 wm_disk_t disk={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),wm};assert(wm_valid(&disk));
 disk.state.patch.filter=WS_FILTER_COUNT;assert(!wm_valid(&disk));
 puts("Low-pass frequency response, live switching, Zoom and saved patch validation pass.");
}
