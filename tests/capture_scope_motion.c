/* SPDX-License-Identifier: GPL-3.0-only */
#define DINGHY_HOST 1
#define main host_main
#include "../firmware/src/dinghy.c"
#undef main
int main(void)
{
    cues_enabled=0;scope_set_open(1);wm.sound=52;wm.fx_on=7;
    note_on(60,120,1);char path[128];
    for(uint32_t frame=0;frame<48;frame++){
        wm.melt=(uint8_t)(frame*127u/47u);
        if(frame==28)note_off(60,1);
        for(uint32_t k=0;k<8;k++){int32_t b[512]={0};app_audio(b,256);}
        fm1_ms+=46;screen();snprintf(path,sizeof path,"build/scope-motion/%03u.ppm",frame);write_ppm_as(path);
    }
}
