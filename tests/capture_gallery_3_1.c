/* 3.1 screen gallery: every screen, Normal and Zoom, WANDERER / CONTRAST / LIGHT.
 * Writes build/gallery/<theme>-<zoom>-NN-<name>.ppm (host only; text overflow aborts). */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static uint32_t shot_n;static const char *tag_theme,*tag_zoom;
static void shot(const char *name){char p[128];screen();snprintf(p,sizeof p,"build/gallery/%s-%s-%02u-%s.ppm",tag_theme,tag_zoom,shot_n++,name);write_ppm_as(p);}
static void song(void)
{
 wm=(wm_state_t)WM_NEW_DEFAULTS;ws_select(0);ww_reset();ww.rng=7;
 static const uint8_t notes[8]={60,63,67,65,60,70,68,67};
 for(uint32_t i=0;i<8;i++)wm.memory.e[i*2u]=(wm_event_t){notes[i],100,192};
 for(uint32_t i=0;i<4;i++)wm.layer.e[i*4u]=(wm_event_t){(uint8_t)(48+i*3),100,600};
 wm_protect_source();wm.drift=96;wm.melt=72;for(uint32_t i=0;i<7;i++)ww_loop();
}
int main(void)
{
 static const uint8_t themes[3]={0,1,THEME_LIGHT};
 for(uint32_t t=0;t<3;t++)for(uint32_t z=0;z<2;z++){
  color_theme=themes[t];tag_theme=WM_THEMES[color_theme].name;tag_zoom=z?"zoom":"normal";shot_n=0;
  splash_active=0;cues_enabled=0;song();wm.extra_large=(uint8_t)z;wm.light=color_theme==THEME_LIGHT;
  running=1;step_primed=1;step_index=5;
  shot("home");layer_focus=1;shot("home-layer");layer_focus=0;anchor_view=1;shot("home-anchor");anchor_view=0;
  recording=1;shot("home-recording");recording=0;wm.layer_enabled=0;shot("home-layer-off");wm.layer_enabled=1;
  uint8_t foci[6]={1,2,3,4,5,7};const char *fn[6]={"melt","drift","scatter","tempo","level","preset"};
  for(uint32_t i=0;i<6;i++){focus=foci[i];focus_value=80;focus_until=0;shot(fn[i]);}focus=0;
  editor_open=1;for(editor_page=0;editor_page<EDIT_PAGES;editor_page++){if(!editor_page_supported(editor_page,ws_edit_patch()->engine))continue;editor_selected=1;char n[40];snprintf(n,sizeof n,"edit-%s",EDIT_TITLES[editor_page]);for(char *c=n;*c;c++)if(*c==' ')*c='-';shot(n);}editor_open=0;
  settings_open=1;for(uint32_t i=0;i<SETTINGS_PAGES;i++){settings_page=SETTINGS_ORDER[i];settings_index=0;char n[40];snprintf(n,sizeof n,"settings-%s",SETTINGS_TITLES[settings_page]);for(char *c=n;*c;c++)if(*c==' ')*c='-';shot(n);}settings_open=0;
  running=0;
  for(confirm=1;confirm<=6;confirm++){char n[24];snprintf(n,sizeof n,"confirm-%u",confirm);shot(n);}confirm=0;
  static const uint8_t ns[]={N_BRANCHED,N_RETURNED,N_UNDO,N_SAVED,N_SAVE_FAILED,N_STOP_FIRST,N_CATCH_FIRST,N_CLICK_ON,N_ERASED,N_THEME};
  for(uint32_t i=0;i<sizeof ns;i++){notice=ns[i];notice_until=0;char n[24];snprintf(n,sizeof n,"notice-%u",ns[i]);shot(n);}notice=0;
  scope_set_open(1);for(uint32_t i=0;i<SCOPE_N;i++)scope_trace[i]=(int16_t)(wn_sine_i(i*0x04000000u)/3);scope_hold=1;shot("scope");scope_hold=0;scope_set_open(0);
  count_in_total=step_samples*16u;count_in_pos=step_samples*4u+5u;shot("count-in");count_in_total=0;
 }
 printf("gallery ok\n");return 0;
}
