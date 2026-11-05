/* Content fitting must keep glyphs, units and controls in disjoint rectangles. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static uint32_t cases;
static void check(const char *s,uint16_t height,uint16_t cap,uint8_t wrap)
{
 const zoom_fit_t *p=zoom_fit(s,216,height,cap,wrap);
 uint32_t n=0;while(s[n])n++;
 assert(p->font->h<=cap);
 assert((p->split?2u*p->font->h+8u:p->font->h)<=height);
 if(p->split){
  assert(s[p->split]==' ');
  assert(zoom_text_width(p->font,s,p->split)<=216u);
  assert(zoom_text_width(p->font,s+p->split+1,n-p->split-1)<=216u);
 }else assert(zoom_text_width(p->font,s,n)<=216u);
 uint8_t next=zoom_fit_next;
 assert(zoom_fit(s,216,height,cap,wrap)==p&&zoom_fit_next==next);
 cases++;
}
static void check_row(const char *label,const char *value,const char *unit,uint8_t numeric)
{
 int32_t units=width(&FONT_T,unit);
 const font_t *font=numeric?&FONT_P:&FONT_T;
 assert(16+width(&FONT_T,"4")+2*FONT_PAD<40);
 assert(40+width(&FONT_T,label)+FONT_PAD<=226);
 assert(220-(units?units+7:0)-width(font,value)-FONT_PAD>=12);
}
int main(void)
{
 fm1_ms=100;
 for(uint32_t page=0;page<SETTINGS_PAGES;page++){
  settings_page=page;
  for(uint32_t k=0;k<settings_count();k++){
   const char *label=SETTINGS_LABELS[page][k];
   check(label,40,34,0);
   settings_adjust(k,-10000);
   for(uint32_t i=0;i<=127u;i++){
    char b[12];const char *value=settings_value(k,b);
    check_row(label,value,"",value==b);
    check(value,page==0u?88u:112u,104,1);
    uint32_t previous=settings_get(k);
    settings_adjust(k,1);
    if(settings_get(k)==previous)break;
   }
  }
 }
 for(uint32_t m=0;m<WM_ARP_MODES;m++)check(ARP_MODES[m],112,104,1);
 for(uint32_t r=0;r<WM_ARP_RATES;r++)check(ARP_RATES[r],112,104,1);
 const char *words[]={"ON","OFF","INTERNAL","USB","TRS","MIXOLYD","HARM MIN","C#","PHASE DIST","SUBTRACT","REVERSE","PITCHED","PRINTING","DRY ANCHOR","12.0"};
 for(uint32_t i=0;i<sizeof words/sizeof *words;i++)check(words[i],112,104,1);
 for(uint32_t i=0;i<=127;i++){char b[12];number(i,b);check(b,112,104,1);check(b,84,104,1);}
 assert(zoom_fit("CHORD",216,112,104,1)->font->h>FONT_P.h);
 assert(zoom_fit("DOWN REPEAT",216,112,104,1)->split!=0);
 assert(zoom_fit("ON",216,112,104,1)->font->h>=70);
 wm=(wm_state_t)WM_DEFAULTS;layer_focus=0;zoom_steady=1;
 /* All engine/page/control extremes, with actual formatted labels and units. */
 editor_open=1;settings_open=0;wm.extra_large=1;
 for(uint32_t engine=0;engine<6;engine++){
  wm=(wm_state_t)WM_DEFAULTS;wm.extra_large=1;editor_ensure()->engine=(uint8_t)engine;
  for(editor_page=0;editor_page<EDIT_PAGES;editor_page++)if(editor_page!=EDIT_PRINT&&editor_page_supported(editor_page,engine)){
   for(uint32_t k=0;k<editor_count();k++)for(uint32_t end=0;end<2;end++){
    editor_selected=k;fm1_ms+=700;
    if(editor_page!=3u||k!=0u)editor_adjust(k,end?10000:-10000);
    char b[12];const char *unit,*value=editor_value(k,b,&unit);
    check_row(editor_label(k),value,unit,value==b);
    check(editor_label(k),40,34,0);check(value,unit[0]?84:112,104,1);
    screen();
   }
  }
 }
 printf("%u fit cases: padded bounds, word wrapping, units, cached reuse and all editor extremes passed; cache %zu bytes\n",cases,sizeof zoom_fit_cache);
}
