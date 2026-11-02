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
int main(void)
{
 fm1_ms=100;
 for(uint32_t page=0;page<SETTINGS_PAGES;page++){
  settings_page=page;
  for(uint32_t k=0;k<settings_count();k++)check(SETTINGS_LABELS[page][k],40,34,0);
 }
 for(uint32_t m=0;m<WM_ARP_MODES;m++)check(ARP_MODES[m],112,104,1);
 for(uint32_t r=0;r<WM_ARP_RATES;r++)check(ARP_RATES[r],112,104,1);
 const char *words[]={"ON","OFF","INTERNAL","USB","TRS","MIXOLYD","HARM MIN","C#","PHASE DIST","SUBTRACT","REVERSE","PITCHED","PRINTING","DRY ANCHOR","12.0"};
 for(uint32_t i=0;i<sizeof words/sizeof *words;i++)check(words[i],112,104,1);
 for(uint32_t i=0;i<=127;i++){char b[12];number(i,b);check(b,112,104,1);check(b,84,104,1);}
 assert(zoom_fit("CHORD",216,112,104,1)->font->h>FONT_P.h);
 assert(zoom_fit("DOWN REPEAT",216,112,104,1)->split!=0);
 assert(zoom_fit("ON",216,112,104,1)->font->h>=70);
 /* All engine/page/control extremes, with actual formatted labels and units. */
 editor_open=1;settings_open=0;wm.extra_large=1;
 for(uint32_t engine=0;engine<5;engine++){
  ws_select(engine*20);
  for(editor_page=0;editor_page<EDIT_PAGES;editor_page++)if(editor_page!=EDIT_PRINT){
   for(uint32_t k=0;k<editor_count();k++)for(uint32_t end=0;end<2;end++){
    editor_selected=k;fm1_ms+=700;editor_adjust(k,end?10000:-10000);
    char b[12];const char *unit,*value=editor_value(k,b,&unit);
    check(editor_label(k),40,34,0);check(value,unit[0]?84:112,104,1);
    screen();
   }
  }
 }
 printf("%u fit cases: padded bounds, word wrapping, units, cached reuse and all editor extremes passed; cache %zu bytes\n",cases,sizeof zoom_fit_cache);
}
