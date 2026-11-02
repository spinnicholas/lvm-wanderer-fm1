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
static const char *editor_value(uint32_t k,char *buf,const char **unit)
{
 *unit="";const ws_patch_t *p=ws_current_patch();
 if(editor_page==EDIT_PRINT){
  if(k==0)return wp_writing?"PRINTING":wp_armed?"ARMED":"READY";
  if(k==1)return wp_redo?"REDO":wp_undo_valid?"UNDO":"CANCEL";
  if(k==2)return anchor_view?"ANCHOR":"CURRENT";
  return wp_active?"RAM":"EVENT";
 }
 uint8_t *value=editor_param(k);uint32_t v=value?*value:0;
 uint8_t env=(editor_page==0||editor_page==2||(editor_page>=6&&editor_page<=9));
 if(editor_page==EDIT_SCATTER){
  if(k==0){const char *styles[WM_SCATTER_MODES]={"MIXED","SLICES","HOPS","REVERSE","TAPE","CLOUD","PITCHED","FILLS"};return styles[v];}
  if(k==1){if(!v)return "AUTO";*unit="MS";v=WM_GRAIN_MS[v];}
 }
 if(env&&k!=2){*unit="MS";v=WS_MS[v];}
 else if(editor_page==1&&k==0){*unit="HZ";v=WS_HZ[v];}
 else if(editor_page==1&&k==2){const char *types[3]={"LOW","BAND","HIGH"};return types[v];}
 else if(editor_page==3&&k==0)return WS_ENGINE_NAMES[v];
 else if(editor_page==3&&k==1&&p->engine==0){const char *waves[6]={"SAW","HOLLOW","PULSE","SINE","TRI","GLASS"};return waves[v];}
 else if(editor_page==5&&k==2){const char *dest[3]={"FILTER","SHAPE","PITCH"};return dest[v];}
 else if(editor_page==10){*unit="/16";}
 else if((editor_page==12&&k==0)||(editor_page==3&&k==1&&p->engine==1)){v++;}
 else if(editor_page==12&&k>=2){*unit="/16";}
 else if(editor_page==3&&k==2&&(p->engine==1||p->engine==0)){*unit="/16";}
 if((*unit)[0]=='M'&&v>=1000u){*unit="S";number(v/1000u,buf);uint32_t n=0;while(buf[n])n++;buf[n++]='.';buf[n++]=(char)('0'+(v%1000u)/100u);buf[n]=0;return buf;}
 if((*unit)[0]=='H'&&v>=1000u){*unit="KHZ";number(v/1000u,buf);uint32_t n=0;while(buf[n])n++;buf[n++]='.';buf[n++]=(char)('0'+(v%1000u)/100u);buf[n]=0;return buf;}
 (void)p;return number(v,buf);
}
static void screen_editor(uint16_t bg,uint16_t fg,uint16_t pa,uint16_t guide)
{
 editor_ensure();
 const char *title=wm.patch.engine==4&&editor_page==10?"MODAL RATIOS":wm.patch.engine==4&&editor_page==11?"PARTIAL LVLS":EDIT_TITLES[editor_page];
 aa_text(10,7,&FONT_T,title,fg);transport_symbol(209,6,fg);cv_rect(12,33,216,2,guide);
 if(editor_page==EDIT_PRINT){
  const char *state=wp_writing?"PRINTING":wp_armed?"NEXT LOOP":wp_dry_view?"DRY ANCHOR":anchor_view?"ANCHOR":wp_redo?"UNDO LOOP":wp_active?"AUDIO LOOP":"EVENT LOOP";
  if(wm.extra_large){
   mid(96,45,wp_writing||wp_armed?ICO_RECORD:ICO_MEMORY,fg);zoom_text(12,96,216,44,44,state,0,fg);
   const char *hints[4]={"REC / PRINT",wp_writing||wp_armed?"ENV / CANCEL":wp_redo?"ENV / REDO":"ENV / UNDO","LFO / ANCHOR",!wp_has_anchor?"RAM EMPTY":wp_active?"KNOB 4 / EVENT":"KNOB 4 / RAM"};
   const char *hint=hints[editor_selected];zoom_text(12,146,216,27,23,hint,0,fg);
   for(uint32_t j=0;j<4;j++){int32_t x=18+(int32_t)j*55;card(x,179,40,30,pa,j==editor_selected?fg:guide,j==editor_selected);char b[12];aa_text(x+12,185,&FONT_T,number(j+1,b),fg);}
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
  uint8_t fractional=(env&&k!=2u)||(editor_page==1u&&k==0u);
  zoom_parameter_range(editor_label(k),v,unit,3,fractional,fg);
  for(uint32_t j=0;j<editor_count();j++){int32_t x=18+(int32_t)j*55;card(x,208,40,30,pa,j==k?fg:guide,j==k);char b[12];aa_text(x+12,214,&FONT_T,number(j+1,b),fg);}
 }else{
  for(uint32_t k=0;k<editor_count();k++){
   int32_t x=8,y=42+(int32_t)k*43;
   card(x,y,224,40,pa,k==editor_selected?fg:guide,k==editor_selected);
   const char *label=editor_page==1&&k==0?"FREQ":editor_label(k);aa_text(16,y+11,&FONT_T,label,fg);
   char buf[12];const char *unit,*value=editor_value(k,buf,&unit);
   uint8_t numeric=value==buf;const font_t *f=numeric?&FONT_P:&FONT_T;
   int32_t units=width(&FONT_T,unit),end=222-(units?units+4:0);
   aa_text(end-width(f,value),y+(numeric?8:11),f,value,fg);
   if(units)aa_text(222-units,y+13,&FONT_T,unit,fg);
  }
  for(uint32_t i=0;i<EDIT_PAGES;i++)cv_rect(11+(int32_t)i*15,223,10,editor_page==i?6:2,editor_page==i?fg:guide);
 }
 (void)bg;
}
static void screen_preset_name(uint16_t fg,uint16_t guide)
{
 status_header(ICO_SOUND,fg,guide,0);mid(96,44,ICO_SOUND,fg);
 const char *name=wm.legacy?"LEGACY":WS_NAMES[wm.sound];const font_t *font=wm.extra_large?&FONT_V:&FONT_P;
 char a[32],b[32];uint32_t split=0,n=0;while(name[n]){if(name[n]==' ')split=n;n++;}
 if(!split)split=n;for(uint32_t i=0;i<split;i++)a[i]=name[i];a[split]=0;
 uint32_t j=0;for(uint32_t i=split+(name[split]==' ');i<n;i++)b[j++]=name[i];b[j]=0;
 if(width(font,a)>216||width(font,b)>216)font=&FONT_P;
 if(width(font,name)<=216)text_center(108,font,name,fg);
 else{text_center(98,font,a,fg);text_center(wm.extra_large?137:132,font,b,fg);}
 char value[12];text_center(180,&FONT_N,number(wm.sound+1,value),fg);
 for(uint32_t i=0;i<10;i++)cv_rect(17+(int32_t)i*21,224,16,6,i==wm.sound/10?fg:guide);
}
