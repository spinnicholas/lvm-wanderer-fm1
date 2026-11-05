/* Every factory sound, 4 voices, extreme Melt/effects, sanitizer instrumented. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void)
{
 uint32_t peak=0,cases=0;
 for(uint32_t p=0;p<WM_PRESET_COUNT;p++)for(uint32_t mode=0;mode<3;mode++){
  running=recording=anchor_view=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;
  ws_select(p);wm.melt=mode==0?0:mode==1?64:127;wm.fx_on=7;
  wm.chorus_mix=wm.delay_mix=wm.reverb_mix=wm.delay_feedback=wm.reverb_size=127;
  vol=127;cues_enabled=0;master_smooth=32512;
  for(uint32_t b=0;b<90;b++){int32_t out[512]={0};app_audio(out,256);}
  for(uint32_t v=0;v<NV;v++)note_on(36+v*12+(mode==2?24:0),127,1);
  uint64_t energy=0;
  for(uint32_t b=0;b<180;b++){
   int32_t out[512]={0};app_audio(out,256);
   for(uint32_t k=0;k<512;k++){uint32_t a=abs(out[k]);assert(a<32767);if(a>peak)peak=a;energy+=(int64_t)out[k]*out[k];}
  }
  assert(energy>10000);panic();for(uint32_t v=0;v<NV;v++)assert(!voice[v].gate&&!voice[v].amp);cases++;
 }
 printf("%u four-voice factory/Melt/max-effect cases: audible, bounded, panic releases; peak %u\n",cases,peak);
 return 0;
}
