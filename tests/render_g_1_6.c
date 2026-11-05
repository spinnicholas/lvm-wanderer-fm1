/* Five clean neighbour notes in each production engine for physical-key review. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void)
{
 const uint8_t presets[5]={20,40,54,100,130};
 FILE *f=fopen("build/G-neighbours-1.6.raw","wb");assert(f);
 for(uint32_t e=0;e<5;e++)for(uint32_t note=53;note<=57;note++){
  running=recording=anchor_view=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;
  ws_select(presets[e]);wm.fx_on=0;wm.melt=0;vol=100;master_smooth=25600;cues_enabled=0;ages=0;
  memset(ws_voice,0,sizeof ws_voice);memset(voice,0,sizeof voice);
  for(uint32_t b=0;b<50;b++){int32_t out[512]={0};app_audio(out,256);}
  performance_on(note,100,1);
  for(uint32_t n=0;n<FS*7/10;){uint32_t k=FS*7/10-n;if(k>256)k=256;
   if(n>=FS/2)performance_off(note,1,0);
   int32_t out[512]={0};int16_t pcm[512];app_audio(out,k);
   for(uint32_t i=0;i<k*2;i++){assert(abs(out[i])<32767);pcm[i]=(int16_t)out[i];}
   assert(fwrite(pcm,2,k*2,f)==k*2);n+=k;
  }
 }
 assert(!fclose(f));puts("F3, F#3, G3, G#3, A3: Analog / FM / Phase / String / Modal, fixed velocity, dry");
}
