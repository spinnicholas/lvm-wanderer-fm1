/* Production-DSP comparison: six four-second segments with the same seed. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static FILE *audio;
static void frames(uint32_t n){while(n){uint32_t c=n>256?256:n;int32_t o[512]={0};int16_t pcm[512];app_audio(o,c);for(uint32_t i=0;i<c*2;i++)pcm[i]=(int16_t)o[i];if(audio)fwrite(pcm,2,c*2,audio);n-=c;}}
int main(void){FILE *file=fopen("build/scatter-grains-preview.raw","wb");
 const uint8_t sizes[6]={0,1,3,5,6,6};
 for(uint32_t segment=0;segment<6;segment++){
  audio=0;running=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;ws_select(29);cues_enabled=0;vol=100;master_smooth=25600;
  memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);ages=audio_frames=motion_phase=0;
  step_index=step_clock=cycle_index=step_primed=0;step_samples=FS*15u/wm.bpm;
  wm.fx_on=segment==5?6:0;wm.delay_mix=48;wm.reverb_mix=72;wm.grain_size=sizes[segment];wm.decay=segment?110:0;frames(FS/2);
  const uint8_t notes[8]={60,67,63,70,65,72,67,58};for(uint32_t i=0;i<8;i++)wm.memory.e[i*2]=(wm_event_t){notes[i],110,(uint16_t)(i%3?192:400)};
  scatter_seed(13579);running=1;audio=file;frames(FS*4);
 }
 fclose(file);return 0;}
