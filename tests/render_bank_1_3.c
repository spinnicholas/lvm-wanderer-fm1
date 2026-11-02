/* Auditions from the production DSP, never a desktop imitation. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static FILE *audio;static int64_t sum;static int32_t peak;
static void frames(uint32_t count){while(count){uint32_t n=count>256?256:count;int32_t out[512]={0};app_audio(out,n);int16_t pcm[512];for(uint32_t i=0;i<n*2;i++){pcm[i]=(int16_t)out[i];int32_t a=abs(out[i]);if(a>peak)peak=a;sum+=(int64_t)out[i]*out[i];}if(audio)fwrite(pcm,2,n*2,audio);count-=n;}}
int main(void){cues_enabled=0;vol=100;wm.fx_on=0;FILE *meta=fopen("build/bank-1.3.csv","w");audio=fopen("build/bank-1.3.raw","wb");fprintf(meta,"index,name,engine,peak,energy\n");for(uint32_t p=0;p<100;p++){
 FILE *save=audio;audio=0;running=0;reset_performance();wp_reset();frames(FS/2);audio=save;ws_select(p);wm.fx_on=0;sum=0;peak=0;
 note_on(60,110,1);frames(FS*3/5);note_off(60,1);frames(FS*2/5);fprintf(meta,"%u,%s,%u,%d,%lld\n",p+1,WS_NAMES[p],p/20,peak,(long long)sum);
 }fclose(audio);fclose(meta);return 0;}
