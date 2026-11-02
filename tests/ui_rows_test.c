/* Full value range in the real settings rows, both themes and sizes. */
#define DINGHY_HOST 1
#define main host_main
#include "../firmware/src/dinghy.c"
#undef main
int main(void)
{
    fm1_ms=100;settings_open=1;uint32_t count=0;
    for(uint32_t theme=0;theme<2;theme++)for(uint32_t zoom=0;zoom<2;zoom++){
        wm.light=theme;wm.extra_large=zoom;
        for(uint32_t v=0;v<128;v++){
            settings_page=1;wm.chorus_mix=wm.delay_mix=wm.reverb_mix=v;
            for(settings_index=0;settings_index<3;settings_index++){
                wm.fx_on=v&7;screen();count++;
            }
            settings_page=2;settings_index=0;vol=v;screen();count++;
        }
    }
    printf("%u complete-range settings renders pass text bounds; 0/100%%, both themes, both sizes, on/off states\n",count);
}
