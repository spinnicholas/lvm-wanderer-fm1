/* Adversarial segmentation: generated note recordings and audio must be invariant. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static uint32_t run(uint32_t block,uint8_t mode,uint8_t rate,uint8_t record){
 running=recording=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);memset(arp_configured,0,sizeof arp_configured);arp_rng=1234;ages=audio_frames=0;step_index=step_clock=cycle_index=step_primed=motion_phase=0;step_samples=FS*15u/wm.bpm;cues_enabled=0;vol=100;master_smooth=25600;settings_open=confirm=anchor_view=0;wm.fx_on=0;ws_select(24);wm.arp_on=1;wm.arp_mode=mode;wm.arp_rate=rate;wm.arp_gate=87;wm.arp_swing=33;wm.arp_octaves=3;wm.decay=100;wm.repeat_style=6;wm.grain_size=5;
 arp_update_config();if(record){running=recording=1;record_undo_pending=1;memset(record_replace,0,sizeof record_replace);}
 performance_on(48,100,1);performance_on(55,127,1);performance_on(60,110,1);scatter_seed(123);uint32_t count=FS*2,hash=0;
 while(count){uint32_t n=count>block?block:count;int32_t out[2048]={0};app_audio(out,n);for(uint32_t i=0;i<n*2;i++)hash=hash*33u+(uint32_t)out[i];count-=n;}
 capture_finish_all();if(record){const uint8_t *p=(const uint8_t*)&wm.memory;for(uint32_t i=0;i<sizeof wm.memory;i++)hash=hash*33u+p[i];}return hash;
}
int main(void){uint32_t sizes[]={1,17,64,255,256,389,1024};for(uint32_t mode=0;mode<WM_ARP_MODES;mode++)for(uint32_t rate=0;rate<WM_ARP_RATES;rate+=2)for(uint32_t record=0;record<2;record++){uint32_t expected=run(256,mode,rate,record);for(uint32_t i=0;i<7;i++){uint32_t got=run(sizes[i],mode,rate,record);if(got!=expected)fprintf(stderr,"mode%u rate%u rec%u block%u: %u != %u\n",mode,rate,record,sizes[i],got,expected);assert(got==expected);}}puts("1092 callback-size comparisons: arp audio, swing/gates and recorded notes identical");}
