/* Compile against both trees. Old factory selections must render identically. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#ifndef WANDERER_SOURCE
#define WANDERER_SOURCE "../firmware/src/dinghy.c"
#endif
#include WANDERER_SOURCE
#undef main
#include <assert.h>
static uint32_t hash;
static void frames(uint32_t total,uint8_t meter)
{
 while(total){uint32_t n=total>256?256:total;int32_t out[512]={0};app_audio(out,n);
  if(meter)for(uint32_t i=0;i<n*2;i++){hash^=(uint32_t)out[i];hash*=16777619u;}
  total-=n;
 }
}
int main(void)
{
 for(uint32_t p=0;p<100;p++){
  running=recording=anchor_view=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;
#ifdef OLD_BANK
  ws_select(p);
#else
  wm.sound=p;wm.preset_bank=WS_BANK_1_4;wm.patch_valid=0;
#endif
  cues_enabled=0;vol=100;master_smooth=25600;wm.fx_on=0;wm.melt=63;
  memset(ws_voice,0,sizeof ws_voice);ages=0;frames(FS/2,0);hash=2166136261u;
  note_on(48,110,1);note_on(60,90,1);frames(FS/3,1);note_off(48,1);note_off(60,1);frames(FS/4,1);
  printf("%u %08x\n",p+1,hash);
 }
 return 0;
}
