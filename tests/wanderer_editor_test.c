#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void){fm1_ms=100;uint32_t checks=0;editor_open=1;
for(uint32_t engine=0;engine<WS_ENGINES;engine++)for(uint32_t zoom=0;zoom<2;zoom++)for(uint32_t theme=0;theme<2;theme++){
 ws_select(engine*20);wm.extra_large=zoom;wm.light=theme;
 for(uint32_t page=0;page<EDIT_PAGES;page++){
  editor_page=page;
  for(uint32_t k=0;k<editor_count();k++){editor_selected=k;
   for(uint32_t end=0;end<2;end++){
    wm.patch.engine=(uint8_t)engine;
    if(page!=EDIT_PRINT){fm1_ms+=700;editor_adjust(k,end?10000:-10000);}
    screen();checks++;
    if(!zoom&&page!=EDIT_PRINT){char b[12];const char *unit,*value=editor_value(k,b,&unit);const char *label=page==1&&k==0?"FREQ":editor_label(k);
     const font_t *f=value==b?&FONT_P:&FONT_T;int32_t units=width(&FONT_T,unit);int32_t x=222-(units?units+4:0)-width(f,value);
     if(x<16+width(&FONT_T,label)+4){fprintf(stderr,"collision engine%u page%u k%u label%s value%s x%d\\n",engine,page,k,label,value,x);return 2;}
    }
   }
  }
 }
}
printf("%u editor engine/page/knob/range/theme/Zoom renders: no clipping or label collisions\n",checks);return 0;}
