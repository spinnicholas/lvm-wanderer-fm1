/* SPDX-License-Identifier: GPL-3.0-only; Atkinson Hyperlegible throughout. */
static void text_center(int32_t y,const font_t *f,const char *s,uint16_t fg)
{aa_text((240-width(f,s))/2,y,f,s,fg);}
/* Bounded fitting of pre-rendered fonts. Cache survives the four LCD bands and
 * periodic redraws; no allocation, rasterization or work in the audio callback. */
typedef struct {
 char text[48];const font_t *font;
 uint16_t w,h,cap;uint8_t split,wrap,valid;
} zoom_fit_t;
static zoom_fit_t zoom_fit_cache[4];
static uint8_t zoom_fit_next;
static uint32_t zoom_text_width(const font_t *f,const char *s,uint32_t n)
{
 uint32_t w=0;
 for(uint32_t i=0;i<n;i++){
  uint32_t k;for(k=0;k<f->n;k++)if(f->chars[k]==s[i])break;
  if(k==f->n)return 65535u;
  w+=f->adv[k];
 }
 return w+2u*FONT_PAD;
}
static const zoom_fit_t *zoom_fit(const char *s,uint16_t w,uint16_t h,uint16_t cap,uint8_t wrap)
{
 uint32_t n=0;while(s[n]&&n<47u)n++;
 for(uint32_t i=0;i<4;i++){
  zoom_fit_t *p=&zoom_fit_cache[i];uint32_t j=0;
  if(!p->valid||p->w!=w||p->h!=h||p->cap!=cap||p->wrap!=wrap)continue;
  while(j<n&&p->text[j]==s[j])j++;
  if(j==n&&!p->text[j])return p;
 }
 zoom_fit_t *p=&zoom_fit_cache[zoom_fit_next++%4u];
 p->w=w;p->h=h;p->cap=cap;p->wrap=wrap;p->valid=1;p->font=&FONT_T;p->split=0;
 for(uint32_t i=0;i<n;i++)p->text[i]=s[i];p->text[n]=0;
 const font_t *fonts[]={&FONT_Z,&FONT_X,&FONT_B,&FONT_R,&FONT_Q,&FONT_V,&FONT_P,&FONT_T};
 for(uint32_t i=0;i<sizeof fonts/sizeof *fonts;i++){
  const font_t *f=fonts[i];
  if(f->h>cap||f->h>h)continue;
  if(zoom_text_width(f,s,n)<=w){p->font=f;return p;}
  if(!wrap||2u*f->h+8u>h)continue;
  uint32_t best=65535u,split=0;
  for(uint32_t j=1;j+1u<n;j++)if(s[j]==' '&&s[j-1]!=' '&&s[j+1]!=' '){
   uint32_t a=zoom_text_width(f,s,j),b=zoom_text_width(f,s+j+1u,n-j-1u);
   uint32_t balance=a>b?a-b:b-a;
   if(a<=w&&b<=w&&balance<best){best=balance;split=j;}
  }
  if(split){p->font=f;p->split=(uint8_t)split;return p;}
 }
 return p;
}
static void zoom_text(int32_t x,int32_t y,uint16_t w,uint16_t h,uint16_t cap,const char *s,uint8_t wrap,uint16_t fg)
{
 const zoom_fit_t *p=zoom_fit(s,w,h,cap,wrap);const font_t *f=p->font;
 int32_t top=y+((int32_t)h-(int32_t)f->h*(p->split?2:1)-(p->split?8:0))/2;
 if(p->split){
  char first[48];for(uint32_t i=0;i<p->split;i++)first[i]=p->text[i];first[p->split]=0;
  aa_text(x+((int32_t)w-width(f,first))/2,top,f,first,fg);
  const char *second=p->text+p->split+1u;
  aa_text(x+((int32_t)w-width(f,second))/2,top+f->h+8,f,second,fg);
 }else aa_text(x+((int32_t)w-width(f,p->text))/2,top,f,p->text,fg);
}
static const font_t *zoom_number_font(const char *value,uint16_t h,uint8_t digits,uint8_t fractional)
{
 if(!zoom_steady||!digits||digits>6u)return 0;
 uint8_t percent_value=0;
 for(const char *c=value;*c;c++){
  if(*c=='.')fractional=1;
  else if(*c=='%')percent_value=1;
  else if(*c<'0'||*c>'9')return 0;
 }
 /* Zero is the widest digit in all generated Atkinson sizes (range-tested).
  * Reserve the whole parameter domain, including both sides of a unit change. */
 char probe[12];uint32_t n=0;
 for(uint32_t i=0;i<digits;i++){if(fractional&&i==digits-1u)probe[n++]='.';probe[n++]='0';}
 if(percent_value)probe[n++]='%';probe[n]=0;
 const font_t *f=zoom_fit(probe,216,h,104,0)->font;
 return width(f,value)+2*FONT_PAD<=216?f:0;
}
static void zoom_parameter_range(const char *label,const char *value,const char *unit,uint8_t digits,uint8_t fractional,uint16_t fg)
{
 zoom_text(12,42,216,40,34,label,0,fg);
 uint16_t h=unit[0]?84:112;
 const font_t *f=zoom_number_font(value,h,digits,fractional);
 if(f)text_center(88+(h-f->h)/2,f,value,fg);
 else zoom_text(12,88,216,h,104,value,1,fg);
 if(unit[0])zoom_text(12,175,216,27,23,unit,0,fg);
}
/* Label and value have separate lines, so long names and units never compete. */
static void control_row(uint32_t k,int32_t y,const char *label,const char *value,const char *unit,
    uint8_t numeric,uint8_t selected,uint16_t fg,uint16_t pa,uint16_t guide,uint16_t accent)
{
 const font_t *f=numeric?&FONT_P:&FONT_T;
 int32_t units=width(&FONT_T,unit);
 int32_t end=220-(units?units+7:0);
 char n[12];
 /* The knob being turned is a solid, high-contrast block, not just a border. */
 uint16_t ink=selected?theme_palette()->bg:fg;
 card(8,y,224,48,selected?accent:pa,selected?fg:guide,selected);
 aa_text(16,y+4,&FONT_T,number(k+1u,n),ink);
 aa_text(40,y+4,&FONT_T,label,ink);
 aa_text(end-width(f,value),y+(numeric?22:25),f,value,ink);
 if(units)aa_text(220-units,y+25,&FONT_T,unit,selected?ink:guide);
}
static void knob_tabs(uint32_t count,uint32_t selected,int32_t y,uint16_t fg,uint16_t pa,uint16_t guide)
{
 for(uint32_t k=0;k<count;k++){
  int32_t x=18+(int32_t)k*55;char n[12];
  card(x,y,40,30,pa,k==selected?fg:guide,k==selected);
  aa_text(x+12,y+6,&FONT_T,number(k+1u,n),fg);
 }
}
static const char *editor_value(uint32_t k,char *buf,const char **unit)
{
 *unit="";const ws_patch_t *p=ws_edit_patch();
 if(editor_page==EDIT_PRINT){
  if(k==0)return wp_writing?"PRINTING":wp_armed?"ARMED":"READY";
  if(k==1)return wp_redo?"REDO":wp_undo_valid?"UNDO":"CANCEL";
  if(k==2)return anchor_view?"ANCHOR":"CURRENT";
  return wp_active?"RAM":"EVENT";
 }
 const uint8_t *value=editor_param(k,p);uint32_t v=value?*value:0;
 static const char *const filter_names[WS_FILTER_COUNT]={"LOW","BAND","HIGH","LP 12","LP 24"};
 uint8_t env=(editor_page==0||editor_page==2||(editor_page>=6&&editor_page<=9));
 if(editor_page==EDIT_PATTERN&&k==0){static const char *names[WM_PATTERN_COUNT]={"FREE","OFFBEATS","EUCLID","CASCADE","WALK","ECHOES","LATE","BAR END","BUILD","CHAOS"};return names[v];}
 if(editor_page==EDIT_SCATTER){
  if(k==0){const char *styles[WM_SCATTER_MODES]={"MIXED","SLICES","HOPS","REVERSE","TAPE","CLOUD","PITCHED","RUSH"};return styles[v];}
  if(k==1){if(!v)return "AUTO";*unit="MS";v=WM_GRAIN_MS[v];}
 }
 if(env&&k!=2){*unit="MS";v=WS_MS[v];}
 else if(p->engine==3&&((editor_page==3&&k==1)||(editor_page==4&&k==0))){*unit="MS";v=WS_MS[v]<40?40:WS_MS[v];}
 else if(editor_page==1&&k==0){*unit="HZ";v=p->filter>=WS_LP12?WN_LP_HZ[v]:WN_HZ[v];}
 else if(editor_page==1&&k==2)return filter_names[v];
 else if(editor_page==3&&k==0)return WS_ENGINE_NAMES[v];
 else if(editor_page==3&&k==1&&p->engine==0){const char *waves[10]={"SAW","SAW PULSE","PULSE","SINE","TRI","STACK","REED","NOISE","SYNC","RING"};return waves[v];}
 else if(editor_page==EDIT_COLOR&&p->engine==5&&k<2){*unit="/4";}
 else if(editor_page==EDIT_COLOR&&p->engine!=5&&((p->engine==2&&k<2)||(k==0&&p->engine))){
  const char *pd[8]={"SAW","SQUARE","PULSE","DOUBLE SIN","SAW PULSE","RES SAW","RES TRI","RES TRAP"};
  const char *fm[6]={"SINE","HALF","RECTIFY","SINE+2","SINE+3","SINE"};
  const char *st[6]={"PLUCK","NOISE","HAMMER","SOFT","PLUCK","NOISE"};const char *mo[6]={"BAR","SKIN","BOWL","PIPE","ORGAN","GLASS"};
  return p->engine==2?pd[v]:p->engine==1?fm[v]:p->engine==3?st[v]:mo[v];}
 else if(editor_page==EDIT_COLOR&&p->engine==2&&k==3)return v==1?"RING":"BLEND";
 else if(editor_page==EDIT_COLOR&&p->engine==3&&k==2)return filter_names[v];
 else if(editor_page==5&&k==2){const char *dest[3]={"FILTER","SHAPE","PITCH"};return dest[v];}
 else if(editor_page==10){*unit=p->engine==5?"/4":"/16";}
 else if((editor_page==12&&k==0)||(editor_page==3&&k==1&&(p->engine==1||p->engine==5))){v++;}
 else if(editor_page==12&&k>=2&&p->engine!=5){*unit="/16";}
 else if(editor_page==3&&k==2&&(p->engine==1||p->engine==0||p->engine==5)){*unit=p->engine==5?"/4":"/16";}
 if((*unit)[0]=='M'&&v>=1000u){*unit="S";number(v/1000u,buf);uint32_t n=0;while(buf[n])n++;buf[n++]='.';buf[n++]=(char)('0'+(v%1000u)/100u);buf[n]=0;return buf;}
 if((*unit)[0]=='H'&&v>=1000u){*unit="KHZ";number(v/1000u,buf);uint32_t n=0;while(buf[n])n++;buf[n++]='.';buf[n++]=(char)('0'+(v%1000u)/100u);buf[n]=0;return buf;}
 (void)p;return number(v,buf);
}
static void screen_editor(uint16_t bg,uint16_t fg,uint16_t pa,uint16_t guide)
{
 uint8_t engine=ws_edit_patch()->engine;
 const char *title=engine==5&&editor_page==0?"OP 6 ENV":engine==5&&editor_page==2?"OP 5 ENV":engine==4&&editor_page==10?"MODAL RATIOS":engine==4&&editor_page==11?"PARTIAL LVLS":EDIT_TITLES[editor_page];
 const wm_theme_t *theme=theme_palette();
 uint16_t accent=editor_page==EDIT_PRINT?theme->anchor:
    editor_page==EDIT_SCATTER||editor_page==EDIT_PATTERN?theme->layer:theme->evolve;
 aa_text(10,7,&FONT_T,title,fg);
 if(editor_page!=EDIT_PRINT&&editor_page!=EDIT_SCATTER&&editor_page!=EDIT_PATTERN)
  aa_text(188,7,&FONT_T,layer_focus?"2":"1",fg);
 transport_symbol(209,6,fg);section_rule(accent,guide);
 if(editor_page==EDIT_PRINT){
  const char *state=wp_writing?"PRINTING":wp_armed?"NEXT LOOP":wp_dry_view?"DRY ANCHOR":anchor_view?"ANCHOR":wp_redo?"UNDO LOOP":wp_active?"AUDIO LOOP":"EVENT LOOP";
  if(wm.extra_large){
   mid(96,45,wp_writing||wp_armed?ICO_RECORD:ICO_MEMORY,fg);zoom_text(12,96,216,44,44,state,0,fg);
   const char *hints[4]={"REC / PRINT",wp_writing||wp_armed?"ENV / CANCEL":wp_redo?"ENV / REDO":"ENV / UNDO","LFO / ANCHOR",!wp_has_anchor?"RAM EMPTY":wp_active?"KNOB 4 / EVENT":"KNOB 4 / RAM"};
   const char *hint=hints[editor_selected];zoom_text(12,146,216,27,23,hint,0,fg);
   knob_tabs(4,editor_selected,179,fg,pa,guide);
   text_center(215,&FONT_T,"RAM / SAVE NOTES",fg);return;
  }
  mini(104,42,wp_writing||wp_armed?ICO_RECORD:ICO_MEMORY,fg);text_center(83,&FONT_P,state,fg);
  text_center(117,&FONT_T,"REC / PRINT",fg);
  text_center(140,&FONT_T,wp_writing||wp_armed?"ENV / CANCEL":wp_redo?"ENV / REDO":"ENV / UNDO",fg);
  text_center(163,&FONT_T,"LFO / ANCHOR",fg);
  text_center(186,&FONT_T,!wp_has_anchor?"RAM EMPTY":wp_active?"KNOB 4 / EVENT":"KNOB 4 / RAM",fg);
  text_center(215,&FONT_T,"RAM / SAVE NOTES",fg);return;
 }
 if(wm.extra_large){
  uint32_t k=editor_selected;char buf[12];const char *unit,*v=editor_value(k,buf,&unit);
  uint8_t env=editor_page==0u||editor_page==2u||(editor_page>=6u&&editor_page<=9u);
  uint8_t fractional=(env&&k!=2u)||(editor_page==1u&&k==0u)||(engine==3&&((editor_page==3&&k==1)||(editor_page==4&&k==0)));
  panel(8,39,224,164,pa);
  zoom_parameter_range(editor_label(k),v,unit,(uint8_t)(editor_page==EDIT_SCATTER&&k==1u?4u:3u),(uint8_t)(fractional||(editor_page==EDIT_SCATTER&&k==1u)),fg);
  knob_tabs(editor_count(),k,208,fg,pa,guide);
 }else{
  for(uint32_t k=0;k<editor_count();k++){
   char buf[12];const char *unit,*value=editor_value(k,buf,&unit);
   control_row(k,37+(int32_t)k*50,editor_label(k),value,unit,value==buf,
       k==editor_selected,fg,pa,guide,accent);
  }
  for(uint32_t i=0;i<EDIT_PAGES;i++)
   cv_rect(12+(int32_t)(i*216u/EDIT_PAGES),237,216u/EDIT_PAGES-4u,
       editor_page==i?3:1,editor_page==i?fg:guide);
 }
 (void)bg;
}
static void screen_preset_name(uint16_t fg,uint16_t guide)
{
 status_header(ICO_SOUND,layer_focus?"LAYER 2":"LAYER 1",fg,guide);mid(96,44,ICO_SOUND,fg);
 const uint8_t sound=ws_edit_sound();
 zoom_text(12,96,216,73,wm.extra_large?34:23,ws_edit_name(),1,fg);
 char value[12];text_center(180,&FONT_N,number(sound+1,value),fg);
 for(uint32_t i=0;i<WM_PRESET_BANKS;i++)cv_rect(9+(int32_t)i*15,224,12,6,i==sound/10?fg:guide);
}
