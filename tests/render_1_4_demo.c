/* Listening examples from the actual app callback. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static FILE *f;
static void render(uint32_t n){while(n){uint32_t k=n>256?256:n;int32_t out[512]={0};int16_t pcm[512];app_audio(out,k);for(uint32_t j=0;j<k*2;j++)pcm[j]=(int16_t)out[j];fwrite(pcm,2,k*2,f);n-=k;}}
static void fresh(void){running=recording=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;ws_select(33);wm.fx_on=5;memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);memset(arp_configured,0,sizeof arp_configured);ages=audio_frames=step_index=step_clock=cycle_index=step_primed=motion_phase=0;step_samples=FS*15/wm.bpm;cues_enabled=0;vol=75;master_smooth=75*256;}
int main(void){f=fopen("build/1.4-demo.raw","wb");for(uint32_t mode=0;mode<9;mode++){fresh();wm.decay=mode?110:0;wm.repeat_style=mode?mode-1u:0;wm.grain_size=5;wm.scatter_edge=96;for(uint32_t i=0;i<16;i+=4){wm.memory.e[i]=(wm_event_t){48+(i/4u)%2u*7u,110,768};wm.memory.extra[i][0]=(wm_event_t){60+i/4u*3u,100,640};}scatter_seed(17);running=1;render(FS*2);}
for(uint32_t mode=0;mode<4;mode++){fresh();ws_select(mode==0?1:33);wm.arp_on=1;wm.arp_mode=mode==3?12:mode;wm.arp_latch=1;wm.arp_octaves=2;wm.arp_swing=20;performance_on(60,110,1);performance_on(63,110,1);performance_on(67,110,1);performance_on(70,110,1);render(FS*2);}fclose(f);}
