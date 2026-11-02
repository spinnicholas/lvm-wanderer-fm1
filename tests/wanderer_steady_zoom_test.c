#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void)
{
 zoom_steady=1;const font_t *base=zoom_number_font("0",112,3,0);assert(base);
 for(uint32_t v=0;v<=127;v++){char b[12];number(v,b);assert(zoom_number_font(b,112,3,0)==base);assert(width(base,b)+2*FONT_PAD<=216);}
 const font_t *percentage=zoom_number_font("0%",112,3,0);
 for(uint32_t v=0;v<=100;v++){char b[12];number(v,b);size_t n=strlen(b);b[n]='%';b[n+1]=0;assert(zoom_number_font(b,112,3,0)==percentage);assert(width(percentage,b)+2*FONT_PAD<=216);}
 cues_enabled=0;wm.extra_large=1;editor_open=1;settings_open=0;fm1_ms=100;
 uint32_t cases=0;
 for(uint32_t engine=0;engine<5;engine++){
  ws_select(engine*20);editor_ensure();
  for(editor_page=0;editor_page<EDIT_PAGES;editor_page++)if(editor_page!=EDIT_PRINT){
   for(uint32_t k=0;k<4;k++){
    wm.patch.engine=(uint8_t)engine;
    editor_selected=k;const font_t *previous=0;
    uint8_t env=editor_page==0u||editor_page==2u||(editor_page>=6u&&editor_page<=9u);
    uint8_t fractional=(env&&k!=2u)||(editor_page==1u&&k==0u);
    editor_adjust(k,-10000);
    for(uint32_t v=0;v<128;v++){
     char b[12];const char *unit,*value=editor_value(k,b,&unit);
     const font_t *font=zoom_number_font(value,unit[0]?84:112,3,fractional);
     if(value==b){assert(font);if(previous)assert(font==previous);previous=font;assert(width(font,value)+2*FONT_PAD<=216);}
     screen();editor_adjust(k,1);cases++;
    }
   }
  }
 }
 assert(!zoom_number_font("CHORD",112,3,0));assert(!zoom_number_font("1/32 DOT",112,3,0));
 zoom_steady=0;assert(!zoom_number_font("100",112,3,0));
 puts("0-127 and 0-100 percent stay one size; all engine/editor ranges and time/frequency unit thresholds fit; words retain adaptive layout and STEADY OFF restores it");
 printf("%u production Zoom range renders passed\n",cases);
}
