/* SPDX-License-Identifier: GPL-3.0-only
 * One bounded gesture per step. Randomness never changes the transport clock.
 * State is tiny; audio history and generation arenas remain owned by Print.
 */
#pragma once
enum { SC_CLEAN,SC_STUTTER,SC_HOP,SC_JUMP,SC_HOLD,SC_DROP,SC_RUSH,SC_REVERSE,SC_TAPE,SC_CLOUD,SC_PITCH };
static struct {uint32_t rng,serial,motif;uint16_t offset;uint8_t kind,divisions,rest,walk;} scatter={.rng=0x735ca771u};
/* 3.0: Scatter gestures belong to a motif. Each step's decision is a hash of
 * the motif and the step, so a gesture returns in the same place next loop
 * and the result is a rhythm, not noise. Variation sets how often a new
 * motif replaces the old one at the loop start (0 = never, full = every loop). */
static uint32_t scatter_hash(uint32_t x)
{x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;return x;}
static uint32_t scatter_random(void)
{uint32_t x=scatter.rng;if(!x)x=0x735ca771u;x^=x<<13;x^=x>>17;x^=x<<5;scatter.rng=x;return x;}
static void scatter_clear(void)
{scatter.kind=SC_CLEAN;scatter.divisions=1;scatter.offset=0;}
static void scatter_seed(uint32_t seed)
{scatter.rng=seed?seed:0x735ca771u;scatter.rest=0;scatter.serial=0;scatter.walk=3;scatter.motif=0;scatter_clear();}
/* These algorithms run once per musical step, never in the audio callback.
 * Free retains legacy scheduling. The other patterns follow the loop playhead.
 * Probability affects event density, not transport or source memory. */
static uint8_t scatter_pattern_gate(uint32_t r,uint32_t amount,uint32_t slot,uint32_t cycle)
{
 uint32_t length=wm.loop_steps;
 uint32_t chance=(amount*amount+127u)/127u;
 if((r&127u)>=chance)return 0;
 switch(wm.scatter_pattern){
 case 1: return (uint8_t)(slot%4u==1u||slot%4u==3u);                  /* pulse */
 case 2: {uint32_t hits=3u+amount/18u;if(hits>length)hits=length;
          return (uint8_t)(((slot*hits)%length)<hits);}                /* Euclidean */
 case 3: return (uint8_t)(slot%8u>=4u);                              /* cascade */
 case 4: return (uint8_t)(slot%2u==1u);                              /* drunk walk */
 case 5: return (uint8_t)(slot%8u==2u||slot%8u==3u||slot%8u==6u);    /* echoes */
 case 6: return (uint8_t)(slot%8u==6u||slot%8u==7u);                 /* rewind */
 case 7: return (uint8_t)(slot+2u>=length);                          /* fills */
 case 8: {uint32_t hits=2u+(cycle%8u)*2u;
          return (uint8_t)(((slot*hits)%length)<hits);}                /* eight-bar morph */
 case 9: return (uint8_t)(slot%4u!=0u||(r>>8&3u)==0u);               /* chaos */
 default:return 0;
 }
}
static void scatter_plan(uint32_t amount,uint8_t bypass,uint32_t slot,uint32_t cycle)
{
 scatter_clear();if(!amount||bypass)return;
 if(!slot||!scatter.motif){
  uint32_t change=scatter_random();
  if(!scatter.motif||(change&127u)<wm.scatter_variation)scatter.motif=scatter_random()|1u;
 }
 uint32_t r=scatter_hash(scatter.motif^(slot*0x9e3779b1u));
 scatter.serial++;
 if(!wm.scatter_pattern){
  /* Free: the downbeat stays clean below high amounts, and phrase
   * endings attract gestures. */
  if(!slot&&amount<100u)return;
  uint32_t chance=2u+amount*amount/190u;
  if(slot+2u>=wm.loop_steps)chance=chance*2u>100u?100u:chance*2u;
  if(r%100u>=chance)return;
 }else{
  if(!scatter_pattern_gate(r,amount,slot,cycle))return;
 }
 scatter.kind=(uint8_t)(1u+(r>>8)%6u);
 static const uint8_t divs[8]={2,3,4,6,8,12,16,24};
 uint32_t range=amount<40?3u:amount<88?5u:8u;
 scatter.divisions=divs[(r>>16)%range];
 scatter.offset=(uint16_t)(1u+(((r>>3)&127u)<wm.scatter_variation?(r>>21)%7u:(scatter.serial/4u)%3u));
 if(!wm.scatter_pattern)return;
 /* Constrain patterns to musical gestures. Keep sound style independent:
  * step_play still selects Pitched/Cloud/Tape after we choose the timing. */
 switch(wm.scatter_pattern){
 case 1: scatter.divisions=(slot&2u)?4u:2u;break;
 case 2: scatter.divisions=(slot&1u)?3u:4u;break;
 case 3: scatter.divisions=(uint8_t)(2u<<((slot%8u-4u)&3u));break;
 case 4: {uint8_t direction=(uint8_t)((r>>17)&3u);
          if(direction==0&&scatter.walk>1u)scatter.walk--;
          if(direction==3&&scatter.walk<7u)scatter.walk++;
          scatter.offset=scatter.walk;break;}
 case 5: scatter.offset=(uint16_t)(1u+(slot%8u==3u?2u:0u));scatter.divisions=4u;break;
 case 6: scatter.divisions=2u;break;
 case 7: scatter.divisions=slot+1u==wm.loop_steps?8u:4u;break;
 case 8: scatter.divisions=(uint8_t)(2u+(cycle%8u)*2u);break;
 case 9: scatter.divisions=divs[(r>>19)&7u];break;
 }
}
