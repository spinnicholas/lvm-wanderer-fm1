/* Callback segmentation cannot change musical timing or printed generation bytes. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static uint32_t hash_render(uint32_t block,uint8_t print,uint8_t grain)
{
 running=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;ws_select(24);wm.decay=127;wm.grain_size=grain;wm.repeat_style=print?1:0;wm.fx_on=0;cues_enabled=0;vol=100;master_smooth=25600;
 step_samples=FS*15u/wm.bpm;step_index=step_clock=cycle_index=step_primed=audio_frames=ages=motion_phase=0;
 memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);memset(wp_arena,0,sizeof wp_arena);wp_ring_write=0;
 for(uint32_t i=0;i<64;i++){int32_t o[512]={0};app_audio(o,256);}wm_record(0,60,100);wm_record(7,64,100);wm_record(15,67,100);
 scatter_seed(12345);if(print)wp_arm();running=1;uint32_t count=step_samples*wm.loop_steps*2+1,hash=0;
 while(count){uint32_t n=count>block?block:count;int32_t o[1024]={0};app_audio(o,n);for(uint32_t i=0;i<n*2;i++)hash=hash*33u+(uint32_t)o[i];count-=n;}
 if(print){assert(wp_has_anchor);hash=0;for(uint32_t i=0;i<WP_BLOCKS*WP_BLOCK;i++)hash=hash*33u+wp_arena[wp_current][i];}
 return hash;
}
int main(void){uint32_t sizes[]={1,17,64,255,256,389};
for(uint32_t grain=0;grain<WM_GRAIN_COUNT;grain+=2)for(uint32_t mode=0;mode<2;mode++){uint32_t expected=hash_render(256,mode,grain);for(uint32_t i=0;i<6;i++){uint32_t hash=hash_render(sizes[i],mode,grain);if(hash!=expected)fprintf(stderr,"mode%u size%u got%u expected%u\n",mode,sizes[i],hash,expected);assert(hash==expected);}}
wm.legacy=0;wm.loop_steps=16;wm.decay=1;assert(wm_repeat_divisions(15,0)==1&&wm_repeat_divisions(15,3)==2);wm.decay=127;assert(wm_repeat_divisions(15,0)==4);wm.legacy=1;assert(wm_repeat_divisions(15,3)==1);
puts("48 callback-size runs (Auto, 8, 32 and 128 ms) produce identical repeat audio / printed bytes; phrase fill density and legacy isolation pass");}
