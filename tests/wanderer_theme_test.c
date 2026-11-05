#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void){
 assert(color_theme==0);wm=(wm_state_t)WM_DEFAULTS;fm1_ms=100;splash_active=notice=focus=confirm=0;settings_open=1;settings_page=0;
 assert(!settings_bool(0));settings_adjust(0,10000);assert(color_theme==THEME_COUNT-1);settings_adjust(0,-10000);assert(!color_theme);
 for(uint32_t t=0;t<THEME_COUNT;t++){
  color_theme=t;wm.light=t==THEME_LIGHT;uint16_t sig=access_signature();assert(!preferences_save());color_theme=0;preferences_load();assert(color_theme==t&&access_signature()==sig&&wm.light==(t==THEME_LIGHT));
  for(uint32_t z=0;z<2;z++)for(uint32_t page=0;page<SETTINGS_PAGES;page++){
   settings_open=1;wm.extra_large=z;settings_page=page;
   for(settings_index=0;settings_index<settings_count();settings_index++)screen();
  }
  wm.extra_large=0;settings_page=8;settings_index=0;
  char path[120];snprintf(path,sizeof path,"build/rc3/theme-%u.ppm",t);screen();write_ppm_as(path);
  wm.extra_large=1;settings_page=0;settings_index=0;snprintf(path,sizeof path,"build/rc3/theme-%u-zoom.ppm",t);screen();write_ppm_as(path);
  settings_open=0;wm.extra_large=0;screen();wm.extra_large=1;screen();
  editor_open=1;editor_ensure();for(editor_page=0;editor_page<EDIT_PAGES;editor_page++){editor_selected=0;screen();}editor_open=0;
  confirm=2;screen();confirm=0;notice=N_STOP_FIRST;screen();notice=0;scope_open=1;screen();scope_open=0;
  const wm_theme_t *p=theme_palette();printf("PALETTE %u %s %u %u %u %u %u %u %u %u\n",t,p->name,p->bg,p->panel,p->memory,p->text,p->label,p->anchor,p->evolve,p->layer);
 }
 wm_preferences_t old={WM_MAGIC,1,sizeof old,0,0,0,100,1,0,{0,0}};
 assert(!settings_save(&old,sizeof old));color_theme=5;preferences_load();assert(color_theme==0&&!wm.light);
 old.light=1;assert(!settings_save(&old,sizeof old));preferences_load();assert(color_theme==THEME_LIGHT&&wm.light);
 old.version=2;old.light=THEME_COUNT;assert(!preferences_valid(&old));old.light=0;old.version=3;assert(!preferences_valid(&old));
 puts("Theme defaults, all settings/Zoom/editor/home/messages/scope renders, preference round trips, v1 migration and invalid records pass");
}
