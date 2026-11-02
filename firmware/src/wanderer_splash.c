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
/* Palette RLE renders only the current 60-row DMA band. No allocation. */
static void splash_scene(void)
{
 for(uint32_t y=0;y<240u;y++){
  int32_t py=(int32_t)y+cv_oy;if((uint32_t)py>=cv_h)continue;
  uint32_t x=0;
  for(uint32_t k=SPLASH_ROW[y];k<SPLASH_ROW[y+1];k+=2){
   uint32_t n=SPLASH_DATA[k];uint16_t color=swap16(SPLASH_PALETTE[SPLASH_DATA[k+1]]);
   for(uint32_t j=0;j<n;j++,x++)if(x<cv_w)cv_px[(uint32_t)py*cv_w+x]=color;
  }
 }
}
static void splash_screen(void)
{
    for(int32_t band=0;band<4;band++){
        cv_begin(240,60,RGB(0,0,0));
        cv_oy=-band*60;
        splash_scene();
        cv_blit(0,(uint32_t)band*60u);
    }
    cv_oy=0;
}
