/* Actual production rendering. Text bounds are checked by the host renderer. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static void shot(const char *n){if(settings_open&&!wm.extra_large)for(uint32_t k=0;k<settings_count();k++){char b[12];const char *v=settings_value(k,b);if(width(&FONT_T,v)>184||width(&FONT_T,SETTINGS_LABELS[settings_page][k])>184){fprintf(stderr,"Row collision: %s / %s\n",SETTINGS_LABELS[settings_page][k],v);exit(9);}}char p[200];snprintf(p,sizeof p,"build/screens-1.4/%s.ppm",n);screen();write_ppm_as(p);}
int main(void){fm1_ms=100;splash_active=focus=notice=confirm=0;ws_select(33);wm.arp_on=1;wm.arp_mode=11;wm.arp_rate=10;wm.arp_octaves=4;wm.repeat_style=5;wm.grain_size=6;wm.decay=100;
for(uint32_t t=0;t<2;t++)for(uint32_t z=0;z<2;z++){
 display_hold=0;cue_detail=zoom_steady=cues_enabled=1;cue_level=64;
 wm.light=t;wm.extra_large=z;settings_open=editor_open=0;char n[80];snprintf(n,sizeof n,"home-t%u-z%u",t,z);shot(n);
 editor_open=1;editor_page=EDIT_SCATTER;
 for(uint32_t mode=0;mode<WM_SCATTER_MODES;mode++){wm.repeat_style=mode;for(uint32_t k=0;k<4;k++){editor_selected=k;snprintf(n,sizeof n,"scatter-t%u-z%u-m%u-k%u",t,z,mode,k);shot(n);}}
 editor_open=0;settings_open=1;
 for(uint32_t page=0;page<SETTINGS_PAGES;page++){settings_page=page;for(uint32_t k=0;k<settings_count();k++){settings_index=k;snprintf(n,sizeof n,"settings-t%u-z%u-p%u-k%u",t,z,page,k);shot(n);}}
 for(uint32_t mode=0;mode<WM_ARP_MODES;mode++){settings_page=6;settings_index=1;wm.arp_mode=mode;snprintf(n,sizeof n,"arp-t%u-z%u-m%u",t,z,mode);shot(n);}
 settings_page=SETTINGS_ACCESS_FEEL;
 for(uint32_t v=0;v<4;v++){settings_index=0;display_hold=v;snprintf(n,sizeof n,"access-hold-t%u-z%u-v%u",t,z,v);shot(n);}
 for(uint32_t v=0;v<3;v++){settings_index=1;cues_enabled=v!=0;cue_detail=v==2;snprintf(n,sizeof n,"access-cues-t%u-z%u-v%u",t,z,v);shot(n);}
 for(uint32_t v=0;v<3;v++){settings_index=2;cue_level=v==0?0:v==1?64:127;snprintf(n,sizeof n,"access-level-t%u-z%u-v%u",t,z,v);shot(n);}
 for(uint32_t v=0;v<2;v++){settings_index=3;zoom_steady=v;snprintf(n,sizeof n,"access-steady-t%u-z%u-v%u",t,z,v);shot(n);}
 zoom_steady=1;settings_open=0;editor_open=1;ws_select(0);editor_ensure();editor_page=0;editor_selected=2;
 const uint8_t values[3]={9,99,100};
 for(uint32_t v=0;v<3;v++){wm.patch.amp[2]=values[v];snprintf(n,sizeof n,"steady-sustain-t%u-z%u-v%u",t,z,values[v]);shot(n);}
 for(uint32_t v=0;v<2;v++){
  editor_page=1;editor_selected=0;wm.patch.cutoff=72u+v;snprintf(n,sizeof n,"steady-frequency-t%u-z%u-v%u",t,z,v);shot(n);
  editor_page=0;editor_selected=0;wm.patch.amp[0]=97u+v;snprintf(n,sizeof n,"steady-time-t%u-z%u-v%u",t,z,v);shot(n);
 }
}puts("Production Home, Scatter and settings renders complete");}
