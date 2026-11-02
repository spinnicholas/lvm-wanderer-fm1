/* Render actual production DSP without effects, plus an expressive shortlist. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static void frames(FILE *f,uint32_t count)
{
    while(count){uint32_t n=count>256u?256u:count;int32_t out[512]={0};int16_t pcm[512];app_audio(out,n);
        for(uint32_t j=0;j<n*2u;j++)pcm[j]=(int16_t)out[j];
        if(f)fwrite(pcm,2,n*2u,f);count-=n;
    }
}
static void silence(void)
{running=recording=0;reset_performance();frames(NULL,FS/2);}
int main(int argc,char **argv)
{
    cues_enabled=0;vol=100;wm.fx_on=0;
    FILE *all=fopen("build/bank-1.1.raw","wb");if(!all)return 1;
    for(uint32_t p=0;p<100;p++){
        silence();wm.sound=(uint8_t)p;note_on(p>=30&&p<40?36:60,110,1);
        frames(all,FS*2);note_off(p>=30&&p<40?36:60,1);frames(all,FS);
    }
    fclose(all);
    const uint8_t picks[]={0,10,22,31,35,42,49,50,66,72,76,79};
    FILE *demo=fopen("build/showcase-1.1.raw","wb");if(!demo)return 1;
    for(uint32_t k=0;k<sizeof picks;k++){
        silence();wm.sound=picks[k];wm.fx_on=5;wm.chorus_mix=22;wm.reverb_mix=35;
        if(picks[k]>=30&&picks[k]<40){
            for(uint32_t n=0;n<8;n++){
                uint32_t note=36u+(n%4u==3u?7u:0u);note_on(note,105,1);frames(demo,FS/5);note_off(note,1);frames(demo,FS/20);
            }
        }else{
            note_on(60,105,1);note_on(63,95,1);note_on(67,100,1);
            frames(demo,FS*2);note_off(60,1);note_off(63,1);note_off(67,1);
        }
        frames(demo,FS*2);
    }
    fclose(demo);puts("100 dry auditions (3 seconds each) and 12 four-second musical excerpts rendered");
}
