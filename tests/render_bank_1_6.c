/* Production DSP auditions and metering, without effects or normalization. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static FILE *audio;
static uint64_t energy;
static uint32_t peak,count,clips;
static void frames(uint32_t total)
{
 while(total){uint32_t n=total>256?256:total;int32_t out[512]={0};int16_t pcm[512];app_audio(out,n);
  for(uint32_t i=0;i<n*2;i++){
   uint32_t a=(uint32_t)abs(out[i]);if(a>peak)peak=a;
   assert(a<32767);energy+=(int64_t)out[i]*out[i];count++;pcm[i]=(int16_t)out[i];
  }
  if(audio)assert(fwrite(pcm,2,n*2,audio)==n*2);total-=n;
 }
}
static void fresh(uint32_t p,uint8_t melt)
{
 running=recording=anchor_view=0;reset_performance();wp_reset();
 wm=(wm_state_t)WM_DEFAULTS;ws_select(p);wm.fx_on=0;wm.melt=melt;
 cues_enabled=0;vol=100;master_smooth=25600;ages=0;memset(ws_voice,0,sizeof ws_voice);
 /* Settle transport/arp config and bounded effect clearing before the first note. */
 FILE *saved_audio=audio;audio=0;frames(FS/2);audio=saved_audio;energy=peak=count=0;
}
static uint8_t center(uint32_t p)
{
 /* Generated register data is an audition aid, outside the firmware. */
 static const uint8_t notes[150]={
#include "../build/bank-1.6-registers.inc"
 };
 return notes[p];
}
int main(int argc,char **argv)
{
 uint8_t trim=(uint8_t)(argc>1&&!strcmp(argv[1],"--meter"));
 FILE *meta=fopen(trim?"build/bank-1.6-meter.csv":"build/bank-1.6.csv","w");assert(meta);
 fprintf(meta,"index,name,engine,level,note,melt,peak,energy,samples\n");
 for(uint32_t p=0;p<WM_PRESET_COUNT;p++){
  if(!trim){char path[90];snprintf(path,sizeof path,"build/bank-1.6/%03u.raw",p+1);audio=fopen(path,"wb");assert(audio);}
  fresh(p,0);note_on(center(p),110,1);frames(FS*2);note_off(center(p),1);frames(FS);
  fprintf(meta,"%u,%s,%u,%u,%u,0,%u,%llu,%u\n",p+1,WS_NAMES[p],WS_PRESETS[p].engine,WS_PRESETS[p].level,center(p),peak,(unsigned long long)energy,count);
  if(audio)assert(!fclose(audio));audio=0;
 }
 if(!trim&&argc==1)for(uint32_t p=0;p<WM_PRESET_COUNT;p++)for(int32_t r=-12;r<=12;r+=12)for(uint8_t m=0;;m=(uint8_t)(m+64)){
   uint8_t melt=m==128?127:m;fresh(p,melt);uint32_t note=center(p)+r;
   note_on(note,110,1);frames(FS*3/4);note_off(note,1);frames(FS/4);
   assert(energy>10000);fprintf(meta,"%u,%s,%u,%u,%u,%u,%u,%llu,%u\n",p+1,WS_NAMES[p],WS_PRESETS[p].engine,WS_PRESETS[p].level,note,melt,peak,(unsigned long long)energy,count);
   if(melt==127)break;
 }
 fclose(meta);printf("150 production auditions%s, no digital clipping\n",trim||argc>1?"":" and 1350 register/Melt probes");
 return 0;
}
