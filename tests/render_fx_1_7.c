/* Listening examples from the production renderer; no post normalization. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static FILE *audio;
static void block(uint32_t n)
{int32_t out[512]={0};int16_t pcm[512];app_audio(out,n);for(uint32_t i=0;i<n*2;i++){assert(abs(out[i])<32767);pcm[i]=(int16_t)out[i];}if(audio)assert(fwrite(pcm,2,n*2,audio)==n*2);}
static void fresh(uint32_t scene,uint8_t wet)
{
 running=recording=anchor_view=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;
 const uint8_t preset[4]={23,44,70,33};ws_select(preset[scene]);wm.fx_on=0;wm.fx_new_on=0;wm.melt=0;
 if(wet){wm.fx_new_on=scene==0?1:scene==1?2:scene==2?4:8;wm.crush_bits=8;wm.crush_rate=9;wm.crush_mix=100;
  wm.tape_mix=105;wm.tape_feedback=82;wm.tape_heads=2;wm.tape_wow=30;wm.tape_tone=80;
  wm.phaser_mix=112;wm.phaser_rate=65;wm.phaser_depth=110;wm.fold_drive=92;wm.fold_mix=86;wm.fold_tone=99;}
 cues_enabled=0;vol=100;master_smooth=25600;ages=0;memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);
 for(uint32_t n=0;n<FS/2;n+=256)block(256);
}
int main(void)
{
 FILE *out=fopen("build/effects-1.7.raw","wb");assert(out);
 for(uint32_t scene=0;scene<4;scene++)for(uint32_t wet=0;wet<2;wet++){
  audio=0;fresh(scene,(uint8_t)wet);audio=out;
  for(uint32_t f=0;f<FS*(wet?4:3);){
   if(!f){note_on(60,110,1);if(scene==2){note_on(64,110,1);note_on(67,110,1);note_on(71,110,1);}}
   if(scene==1&&(f==FS/2||f==FS||f==FS*3/2)){note_off(60,1);note_off(64,1);note_off(67,1);note_on(f==FS/2?64:f==FS?67:72,110,1);}
   if(f==FS*2)for(uint32_t n=0;n<128;n++)note_off(n,1);
   uint32_t end=FS*(wet?4:3),next=f<FS/2?FS/2:f<FS?FS:f<FS*3/2?FS*3/2:f<FS*2?FS*2:end;
   uint32_t n=next-f;if(n>256)n=256;block(n);f+=n;
  }
  int16_t silence[882]={0};assert(fwrite(silence,2,882,out)==882);
 }
 assert(!fclose(out));puts("Bitcrush, tape echo, stereo phaser and wavefolder: dry then wet production examples");
}
