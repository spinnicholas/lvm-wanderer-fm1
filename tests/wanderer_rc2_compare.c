#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <time.h>
int main(int argc,char **argv){int wet=argc>1?atoi(argv[1]):0;uint64_t sum=0;clock_t begin=clock();
 for(uint32_t patch=0;patch<150;patch++){
  wm=(wm_state_t)WM_DEFAULTS;reset_performance();wp_reset();ws_select(patch);ages=0;motion_phase=0;
  memset(ws_voice,0,sizeof ws_voice);memset(voice,0,sizeof voice);running=recording=anchor_view=0;cues_enabled=0;cue_stop();vol=100;master_smooth=25600;
  wm.fx_on=wet?7:0;wm.fx_new_on=wet?15:0;wm.crush_mix=wm.fold_mix=wm.phaser_mix=wm.tape_mix=55;
  while(fx_clearing)fx_clear_chunk();
  for(uint32_t v=0;v<4;v++)note_on(53+v*4,100,1);
  uint32_t h=0;for(uint32_t b=0;b<96;b++){int32_t out[512]={0};if(b==64)for(uint32_t v=0;v<4;v++)note_off(53+v*4,1);app_audio(out,256);for(uint32_t i=0;i<512;i++){h=h*33+(uint32_t)out[i];sum+=(int64_t)out[i]*out[i];}}
  printf("%u %u\n",patch,h);
 }
 fprintf(stderr,"seconds %.4f energy %llu\n",(double)(clock()-begin)/CLOCKS_PER_SEC,(unsigned long long)sum);return 0;
}
