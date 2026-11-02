/* Ten minutes of production DSP with both event layers, Scatter, arp and wet bus. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void){cues_enabled=0;vol=127;wm=(wm_state_t)WM_DEFAULTS;ws_select(33);wm.fx_on=7;wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;wm.layer_enabled=1;wm.decay=127;wm.arp_on=1;wm.arp_gate=100;wm.arp_octaves=4;wm.arp_swing=50;step_samples=FS*15/wm.bpm;
 for(uint32_t i=0;i<WM_STEPS;i++){wm.memory.e[i]=(wm_event_t){48+i%12,127,512};wm.layer.e[i]=(wm_event_t){60+i%12,127,768};}wm_catch();uint32_t anchor=0;const uint8_t *b=(const uint8_t*)&wm.anchor;for(uint32_t i=0;i<sizeof wm.anchor;i++)anchor=anchor*33u+b[i];
 performance_on(48,127,1);performance_on(55,127,1);performance_on(60,127,1);running=1;
 uint32_t total=FS*600u,serial=0;uint64_t energy=0;
 while(total){uint32_t n=total>389u?389u:total;int32_t out[778]={0};app_audio(out,n);for(uint32_t i=0;i<n*2;i++){assert(abs(out[i])<=32767);energy+=(uint32_t)abs(out[i]);}total-=n;
  if(++serial%1000u==0){wm.repeat_style=(wm.repeat_style+1u)%WM_SCATTER_MODES;wm.grain_size=(wm.grain_size+1u)%WM_GRAIN_COUNT;wm.arp_mode=(wm.arp_mode+1u)%WM_ARP_MODES;wm.arp_rate=(wm.arp_rate+1u)%WM_ARP_RATES;}
 }
 uint32_t after=0;for(uint32_t i=0;i<sizeof wm.anchor;i++)after=after*33u+b[i];assert(after==anchor&&energy>1000000);panic();assert(!running&&!arp_count&&!capture_count());for(uint32_t v=0;v<NV;v++)assert(!voice[v].gate&&!voice[v].amp);puts("600 seconds: both layers, four shared voices, max effects, full Scatter/arp variation; bounded output, immutable Anchor and clean panic");}
