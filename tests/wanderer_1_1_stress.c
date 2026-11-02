#include <assert.h>
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
int main(void)
{
    cues_enabled=0;vol=127;wm.fx_on=7;wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;
    wm.delay_feedback=wm.reverb_size=127;wm.melt=127;
    int32_t peak=0;
    for(uint32_t p=0;p<100;p++){
        reset_performance();wm.sound=p;
        for(uint32_t v=0;v<4;v++)note_on(36+v*12,127,1);
        for(uint32_t b=0;b<430;b++){
            int32_t out[512]={0};app_audio(out,256);
            for(uint32_t i=0;i<512;i++){
                int32_t a=out[i]<0?-out[i]:out[i];if(a>peak)peak=a;
                assert(a<32767);
            }
            for(uint32_t v=0;v<4;v++){
                assert(abs(voice[v].f1)<=65535&&abs(voice[v].f2)<=65535);
                assert(voice[v].amp>=0&&voice[v].amp<=8128*256);
            }
        }
    }
    printf("100 patches x four held voices x 2.5 s, maximum effects/Melt/master: no overflow, clipping or unstable filter; peak %d\n",peak);
}
