/* Wanderer 3.0 screen gallery: build/shots/*.ppm (host only). */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static void shot(const char *name){char p[96];screen();snprintf(p,sizeof p,"build/shots/%s.ppm",name);write_ppm_as(p);}
int main(void)
{
 splash_active=0;cues_enabled=0;ww.rng=7;
 static const uint8_t notes[8]={60,63,67,65,60,70,68,67};
 for(uint32_t z=0;z<2;z++){
  char n[64];wm=(wm_state_t)WM_NEW_DEFAULTS;ws_select(0);ww_reset();wm.extra_large=(uint8_t)z;
  for(uint32_t i=0;i<8;i++)wm.memory.e[i*2u]=(wm_event_t){notes[i],100,192};
  snprintf(n,sizeof n,"home-fresh-z%u",z);shot(n);
  wm_protect_source();wm.drift=96;wm.melt=72;for(uint32_t i=0;i<7;i++)ww_loop();running=1;step_primed=1;step_index=5;
  snprintf(n,sizeof n,"home-wander-z%u",z);shot(n);
  focus=2;focus_value=96;focus_until=0;snprintf(n,sizeof n,"focus-drift-z%u",z);shot(n);focus=0;
  notice=N_RETURNED;notice_until=0;snprintf(n,sizeof n,"notice-home-z%u",z);shot(n);
  notice=N_BRANCHED;snprintf(n,sizeof n,"notice-kept-z%u",z);shot(n);notice=0;
  editor_open=1;editor_page=0;editor_selected=1;snprintf(n,sizeof n,"edit-amp-z%u",z);shot(n);
  editor_page=EDIT_SCATTER;editor_selected=2;snprintf(n,sizeof n,"edit-scatter-z%u",z);shot(n);editor_open=0;
  settings_open=1;settings_page=0;settings_index=0;snprintf(n,sizeof n,"settings-z%u",z);shot(n);settings_open=0;
  running=0;
 }
 for(color_theme=0;color_theme<THEME_COUNT;color_theme++){char n[32];wm.extra_large=0;snprintf(n,sizeof n,"theme-%u",color_theme);shot(n);}
 return 0;
}
