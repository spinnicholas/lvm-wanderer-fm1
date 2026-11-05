/* SPDX-License-Identifier: GPL-3.0-only */
#define DINGHY_HOST 1
#define main host_main
#include "../firmware/src/dinghy.c"
#undef main
int main(void)
{
    fm1_ms=100;uint32_t views=0;
    for(uint32_t theme=0;theme<2;theme++)for(uint32_t zoom=0;zoom<2;zoom++){
        wm.light=theme;wm.extra_large=zoom;settings_open=1;
        for(settings_page=0;settings_page<SETTINGS_PAGES;settings_page++)
            for(settings_index=0;settings_index<settings_count();settings_index++){screen();views++;}
        settings_open=0;
        for(focus=1;focus<=7;focus++)if(focus!=6){
            focus_value=127;focus_until=2000;screen();views++;
        }
        focus=0;
        for(confirm=1;confirm<=5;confirm++){project_slot=7;screen();views++;}
        confirm=0;
        for(notice=1;notice<=N_ANCHOR_LISTEN;notice++){notice_until=0;screen();views++;}notice=0;
        scope_set_open(1);
        for(scope_gain=0;scope_gain<5;scope_gain++){
            for(uint32_t i=0;i<SCOPE_N;i++)scope_frames[scope_ready][i]=(i&1u)?32767:-32767;
            screen();views++;
        }
        scope_gain=2;scope_set_open(0);
    }
    printf("%u renderer states pass native 240x240 text bounds, themes, zoom, detail min/max, errors and scope extremes\n",views);
}
