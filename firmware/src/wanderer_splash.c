/* SPDX-License-Identifier: GPL-3.0-only
 * LVM Wanderer boot identity, animated in 3.1: the LVM mark appears, then the
 * eight letters of "Wanderer" glide in from the right one after another and
 * settle in place. Only the wordmark strip is redrawn per frame, in the
 * existing band buffer: no frame buffer, no allocation, nothing in the audio
 * callback. A fresh start (new firmware version) erases one flash sector per
 * frame behind the animation. Any key skips the animation (never the erase);
 * Reduced Motion shows the finished logo without movement.
 * Include after the drawing helpers and before screen().
 */
#include "wanderer_splash_art.h"
#define SPLASH_STAGGER_MS 170u   /* between one letter's start and the next */
#define SPLASH_MOVE_MS 650u      /* each letter's glide */
#define SPLASH_TOTAL_MS 2700u    /* everything has landed by ~1850 ms; then a short hold */
#define SPLASH_STILL_MS 1500u    /* Reduced Motion: the finished logo */
static uint8_t splash_active,splash_drawn,splash_skip;
static uint32_t splash_started;
static uint32_t splash_elapsed(void){return fm1_ms-splash_started;}
static uint8_t splash_moving(void)
{
    return (uint8_t)(splash_active&&!splash_skip&&!wm.reduced_motion&&
        splash_elapsed()<SPLASH_STAGGER_MS*(SPLASH_LETTERS-1u)+SPLASH_MOVE_MS+60u);
}
static void splash_dismiss(void)
{
    if(!splash_active)return;
    /* A key skips the animation; a fresh-start erase still finishes first. */
    if(fresh_left){if(!splash_skip){splash_skip=1;page_dirty=1;}return;}
    splash_active=0;page_dirty=1;
}
static void splash_tick(void)
{
    if(!splash_active||fresh_left)return;
    uint32_t total=wm.reduced_motion?SPLASH_STILL_MS:SPLASH_TOTAL_MS;
    if(splash_skip||splash_elapsed()>=total){splash_active=0;page_dirty=1;}
}
/* The LVM mark: palette runs, current band only. */
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
/* Where letter i is now: off to the right, gliding in with an ease-out cubic. */
static int32_t splash_letter_x(uint32_t i)
{
    int32_t home=SPLASH_LX[i];
    if(splash_skip||wm.reduced_motion)return home;
    int32_t t=(int32_t)splash_elapsed()-(int32_t)(i*SPLASH_STAGGER_MS);
    if(t<=0)return 240;
    uint32_t p=(uint32_t)t>=SPLASH_MOVE_MS?4096u:(uint32_t)t*4096u/SPLASH_MOVE_MS,inv=4096u-p;
    uint32_t left=((inv*inv)>>12)*inv>>12;           /* (1 - p)^3, Q12 */
    return home+(int32_t)(((uint32_t)(244-home)*left)>>12);
}
static void splash_letters(void)
{
    for(uint32_t i=0;i<SPLASH_LETTERS;i++){
        int32_t x=splash_letter_x(i);if(x>=240)continue;
        for(uint32_t r=0;r<SPLASH_LH[i];r++){
            int32_t py=(int32_t)SPLASH_LY[i]+(int32_t)r+cv_oy;if((uint32_t)py>=cv_h)continue;
            uint32_t row=SPLASH_LFIRST[i]+r,gx=0;
            for(uint32_t k=SPLASH_LROW[row];k<SPLASH_LROW[row+1u];k++){
                uint32_t token=SPLASH_LDATA[k],alpha=token>>4,count=(token&15u)+1u;
                if(alpha)alpha_span(x+(int32_t)gx,py,count,alpha,(uint16_t)SPLASH_WORD_COLOR);
                gx+=count;
            }
        }
    }
}
static void splash_screen(void)
{
    if(!splash_drawn){
        for(int32_t band=0;band<4;band++){
            cv_begin(240,60,RGB(0,0,0));cv_oy=-band*60;
            splash_scene();splash_letters();
            cv_blit(0,(uint32_t)band*60u);
        }
        splash_drawn=1;
    }else{
        /* Only the wordmark strip changes; the mark stays on the panel. */
        const uint32_t top=SPLASH_WORD_TOP-1u,h=SPLASH_WORD_BOTTOM-top+1u;
        cv_begin(240,h,RGB(0,0,0));cv_oy=-(int32_t)top;
        splash_letters();
        cv_blit(0,top);
    }
    cv_oy=0;
    fresh_step();   /* one sector per frame, behind the animation */
}
