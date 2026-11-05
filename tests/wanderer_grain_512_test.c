#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void){
 wm=(wm_state_t)WM_DEFAULTS;step_samples=FS*15/wm.bpm;wp_reset();
 assert(WM_GRAIN_MS[6]==128&&WM_GRAIN_MS[7]==256&&WM_GRAIN_MS[8]==512);
 editor_open=1;editor_page=EDIT_SCATTER;editor_selected=1;fm1_ms=100;assert(wm.grain_size==8);editor_adjust(1,-1);editor_adjust(1,1);assert(wm.grain_size==8);
 char b[16];const char *unit;assert(!strcmp(editor_value(1,b,&unit),"512")&&!strcmp(unit,"MS"));
 splash_active=settings_open=confirm=notice=focus=0;
 for(color_theme=0;color_theme<THEME_COUNT;color_theme++)for(uint32_t zoom=0;zoom<2;zoom++){wm.extra_large=zoom;screen();if(color_theme==0){char p[80];snprintf(p,sizeof p,"build/rc4/grain512-%u.ppm",zoom);write_ppm_as(p);}}
 wm_disk_t d={WM_MAGIC,WM_DISK_VERSION,sizeof wm,wm};assert(wm_valid(&d));assert(!slot_save(3,&d,sizeof d));static wm_state_t loaded;assert(project_read_slot(3,&loaded)&&loaded.grain_size==8);
 uint32_t windows=0;
 wp_ring_count=WP_RING_SAMPLES;
 for(uint32_t i=0;i<WP_RING_SAMPLES;i++)wp_ring[i]=(int16_t)((int32_t)(i%2001)-1000);
 for(uint32_t grain=7;grain<=8;grain++)for(uint32_t kind=SC_STUTTER;kind<=SC_RUSH;kind++)for(uint32_t offset=0;offset<10;offset++)for(uint32_t wr=0;wr<3;wr++){
  wm.grain_size=grain;scatter.kind=kind;scatter.offset=offset;scatter.divisions=1;wp_ring_write=wr==0?0:wr==1?4095:WP_RING_SAMPLES-1;wp_active=0;wp_scatter_begin();
  uint32_t expected=WM_GRAIN_MS[grain]*16u*(6u+offset%5u)/8u;assert(wp_repeat_size==expected&&wp_repeat_on&&wp_ring_start<WP_RING_SAMPLES);
  for(uint32_t reverse=0;reverse<2;reverse++)for(uint32_t phase=0;phase<wp_scatter_window;phase++){
   int32_t y=wp_scatter_read(phase,wp_scatter_window,0,reverse);assert(y>=-1000&&y<=1000);
  }windows++;
 }
 /* At 512 ms, history must be genuinely available before the effect starts. */
 wm.grain_size=8;scatter.kind=SC_STUTTER;scatter.offset=2;wp_ring_count=8191;wp_scatter_begin();assert(wp_repeat_size==8192&&!wp_repeat_on);wp_ring_count=8192;wp_scatter_begin();assert(wp_repeat_on);
 scatter.offset=4;wp_ring_count=WP_RING_SAMPLES;wp_scatter_begin();assert(wp_repeat_size==10240&&wp_repeat_on);
 wp_repeat_on=0;wp_ring_count=0;wp_ring_write=WP_RING_SAMPLES-3;running=1;wm.decay=0;memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);
 for(uint32_t n=0;n<FS*2u;n+=128){int32_t out[256]={0};app_audio_core(out,128);assert(wp_ring_write<WP_RING_SAMPLES&&wp_ring_count<=WP_RING_SAMPLES);}assert(wp_ring_count==WP_RING_SAMPLES);
 printf("%u new-size gesture/offset/wrap cases, full fractional/reverse reads, 512 ms warmup, history wrap, save/load, all themes/Zoom pass\n",windows);
}
