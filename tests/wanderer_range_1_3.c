/* Deliberate hostile parameter corners on all five engines, filters and FM routings. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void){cues_enabled=0;vol=127;uint32_t cases=0;int32_t peak=0;
for(uint32_t engine=0;engine<5;engine++)for(uint32_t type=0;type<3;type++)for(uint32_t alg=0;alg<8;alg++)for(uint32_t corner=0;corner<4;corner++){
 running=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;ws_select(engine*20);wm.melt=127;wm.fx_on=7;wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;wm.delay_feedback=wm.reverb_size=127;
 ws_patch_t *p=&wm.patch;p->amp[0]=0;p->amp[1]=0;p->amp[2]=127;p->amp[3]=127;p->filter=type;p->algorithm=alg;p->cutoff=corner&1?127:0;p->resonance=127;p->envdepth=127;p->drive=127;p->level=127;p->shape=127;p->feedback=127;p->lfo_depth=127;
 for(uint32_t op=0;op<4;op++){p->op_env[op][0]=0;p->op_env[op][2]=127;p->op_level[op]=127;p->ratio[op]=corner&2?127:1;}
 for(uint32_t clear=0;clear<64;clear++){int32_t out[512]={0};app_audio(out,256);}assert(!fx_clearing);
 for(uint32_t v=0;v<4;v++)note_on((corner&2?100:0)+v*9,127,1);
 for(uint32_t block=0;block<20;block++){int32_t out[512]={0};app_audio(out,256);for(uint32_t i=0;i<512;i++){int32_t a=abs(out[i]);assert(a<=32767);if(a>peak)peak=a;}}
 cases++;
}
printf("%u engine/filter/routing/extreme-register corners with four voices and max effects: bounded, no sanitizer overflow; peak %d\n",cases,peak);return 0;}
