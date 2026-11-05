#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
int main(void)
{
 fm1_ms=100;editor_open=1;splash_active=focus=notice=confirm=settings_open=0;
 for(uint32_t e=0;e<6;e++)for(uint32_t light=0;light<2;light++)for(uint32_t zoom=0;zoom<2;zoom++)for(uint32_t k=0;k<4;k++){
  uint32_t p=0;while(WS_PRESETS[p].engine!=e)p++;ws_select(p);wm.light=light;wm.extra_large=zoom;editor_page=EDIT_COLOR;editor_selected=k;
  if(k==0)editor_adjust(k,10000);
  char name[128];snprintf(name,sizeof name,"build/screens-1.7/color-e%u-t%u-z%u-k%u.ppm",e,light,zoom,k);screen();write_ppm_as(name);
 }
 editor_open=0;settings_open=1;
 for(uint32_t page=9;page<SETTINGS_PAGES;page++)for(uint32_t theme=0;theme<2;theme++)for(uint32_t zoom=0;zoom<2;zoom++)for(uint32_t k=0;k<4;k++){
  settings_page=page;settings_index=k;wm.light=theme;wm.extra_large=zoom;
  char name[128];snprintf(name,sizeof name,"build/screens-1.7/fx-p%u-t%u-z%u-k%u.ppm",page,theme,zoom,k);screen();write_ppm_as(name);
 }
 settings_open=0;focus=7;focus_until=100000;
 for(uint32_t i=0;i<16;i++){
  uint32_t p=i==15?149:i*10;ws_select(p);wm.light=i%4==3;wm.extra_large=i%2;
  char name[128];snprintf(name,sizeof name,"build/screens-1.7/preset-%03u.ppm",p+1);screen();write_ppm_as(name);
 }
 return 0;
}
