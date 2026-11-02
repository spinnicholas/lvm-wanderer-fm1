/* Native 240x240 production screenshots: engine contexts, names and transactional states. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static void shot(const char *name){char path[256];snprintf(path,sizeof path,"build/screens-1.3/%s.ppm",name);screen();write_ppm_as(path);}
int main(void){fm1_ms=100;splash_active=focus=notice=confirm=settings_open=0;editor_open=1;
for(uint32_t engine=0;engine<5;engine++)for(uint32_t theme=0;theme<2;theme++)for(uint32_t zoom=0;zoom<2;zoom++)for(uint32_t page=0;page<EDIT_PAGES;page++){
if(page>=6&&page<=12&&!(engine==1||(engine==4&&page!=12)))continue;
ws_select(engine*20);wm.light=theme;wm.extra_large=zoom;editor_page=page;
for(uint32_t k=0;k<(zoom?editor_count():1);k++){editor_selected=k;char name[90];snprintf(name,sizeof name,"editor-e%u-t%u-z%u-p%02u-k%u",engine,theme,zoom,page,k);shot(name);}}
for(uint32_t theme=0;theme<2;theme++)for(uint32_t zoom=0;zoom<2;zoom++)for(uint32_t state=0;state<8;state++){
wm.light=theme;wm.extra_large=zoom;editor_page=EDIT_PRINT;wp_armed=state==1;wp_writing=state==2;wp_has_anchor=state>=3;wp_active=state>=3&&state!=7;wp_undo_valid=state>=4;wp_redo=state==5;wp_dry_view=state==6;anchor_view=state==6;
char name[80];snprintf(name,sizeof name,"print-t%u-z%u-state%u",theme,zoom,state);shot(name);}
wp_reset();anchor_view=0;editor_open=0;focus=7;focus_until=10000;
for(uint32_t theme=0;theme<2;theme++)for(uint32_t zoom=0;zoom<2;zoom++)for(uint32_t p=0;p<100;p++){wm.light=theme;wm.extra_large=zoom;ws_select(p);char name[80];snprintf(name,sizeof name,"preset-t%u-z%u-p%02u",theme,zoom,p);shot(name);}
focus=editor_open=0;notice=N_AUDIO_LOCK;notice_until=0;for(uint32_t t=0;t<2;t++){wm.light=t;char name[50];snprintf(name,sizeof name,"audio-lock-t%u",t);shot(name);}
editor_open=1;notice=focus=confirm=settings_open=0;editor_page=EDIT_SCATTER;editor_selected=1;
for(uint32_t t=0;t<2;t++)for(uint32_t z=0;z<2;z++)for(uint32_t size=0;size<WM_GRAIN_COUNT;size++){
 wm.light=t;wm.extra_large=z;wm.grain_size=size;wm.decay=127;wm.repeat_style=2;
 char name[80];snprintf(name,sizeof name,"scatter-t%u-z%u-size%u",t,z,size);shot(name);}
return 0;}
