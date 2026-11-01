/* SPDX-License-Identifier: GPL-3.0-only
 * LVM Wanderer boot identity. Uses existing band buffer; small flash-resident alpha masks; no heap.
 * Include after the drawing helpers and before screen().
 */
#define WANDERER_SPLASH_MS 1100u
static uint8_t splash_active;
static uint32_t splash_started;
static void splash_dismiss(void)
{
    if(splash_active){splash_active=0;page_dirty=1;}
}
static void splash_tick(void)
{
    if(splash_active&&(uint32_t)(fm1_ms-splash_started)>=WANDERER_SPLASH_MS)
        splash_dismiss();
}
#include "wanderer_splash_art.h"
/* Row indexed RLE: render only the current DMA band, blend 4-bit coverage. */
static void splash_mask(const splash_mask_t *m,uint16_t fg)
{
    for(uint32_t y=0;y<m->h;y++){
        int32_t py=(int32_t)m->y+(int32_t)y+cv_oy;
        if((uint32_t)py>=cv_h)continue;
        uint32_t x=m->x;
        for(uint32_t k=m->row[y];k<m->row[y+1];k++){
            uint32_t v=m->data[k],alpha=v>>4,n=(v&15u)+1u;
            for(uint32_t j=0;j<n;j++,x++)if(alpha&&x<cv_w){
                uint32_t pos=(uint32_t)py*cv_w+x;
                uint16_t old=swap16(cv_px[pos]);
                uint32_t r=(((old>>11)*(15u-alpha)+(fg>>11)*alpha)+7u)/15u;
                uint32_t g=((((old>>5)&63u)*(15u-alpha)+((fg>>5)&63u)*alpha)+7u)/15u;
                uint32_t b=(((old&31u)*(15u-alpha)+(fg&31u)*alpha)+7u)/15u;
                cv_px[pos]=swap16((r<<11)|(g<<5)|b);
            }
        }
    }
}
static void splash_scene(void)
{
    splash_mask(&SPLASH_orbit,wm.light?RGB(99,137,133):RGB(115,152,151));
    splash_mask(&SPLASH_signal,wm.light?RGB(17,102,94):RGB(143,228,211));
    splash_mask(&SPLASH_star,wm.light?RGB(131,88,24):RGB(245,207,145));
    splash_mask(&SPLASH_title,wm.light?INK:RGB(241,243,232));
    splash_mask(&SPLASH_brand,wm.light?RGB(66,99,93):RGB(184,204,196));
}
static void splash_screen(void)
{
    for(int32_t band=0;band<4;band++){
        cv_begin(240,60,wm.light?RGB(234,246,243):DARK);
        cv_oy=-band*60;
        splash_scene();
        cv_blit(0,(uint32_t)band*60u);
    }
    cv_oy=0;
}
