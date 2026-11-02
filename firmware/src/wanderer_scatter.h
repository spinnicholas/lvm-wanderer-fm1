/* SPDX-License-Identifier: GPL-3.0-only
 * One bounded gesture per step. Randomness never changes the transport clock.
 * State is tiny; audio history and generation arenas remain owned by Print.
 */
#pragma once
enum { SC_CLEAN,SC_STUTTER,SC_HOP,SC_JUMP,SC_HOLD,SC_DROP,SC_RUSH,SC_REVERSE,SC_TAPE,SC_CLOUD,SC_PITCH };
static struct {uint32_t rng,serial;uint16_t offset;uint8_t kind,divisions,parts,span,rest;} scatter={.rng=0x735ca771u};
static uint32_t scatter_random(void)
{uint32_t x=scatter.rng;if(!x)x=0x735ca771u;x^=x<<13;x^=x>>17;x^=x<<5;scatter.rng=x;return x;}
static void scatter_clear(void)
{scatter.kind=SC_CLEAN;scatter.parts=1;scatter.divisions=1;scatter.offset=0;scatter.span=1;}
static void scatter_seed(uint32_t seed)
{scatter.rng=seed?seed:0x735ca771u;scatter.rest=0;scatter.serial=0;scatter_clear();}
static void scatter_plan(uint32_t amount,uint8_t bypass)
{
 scatter_clear();if(!amount||bypass)return;
 uint32_t r=scatter_random();scatter.serial++;
 if(scatter.rest){scatter.rest--;return;}
 /* Silence between clusters matters as much as activity. Low amounts stay sparse. */
 uint32_t chance=2u+amount*amount/190u;
 /* Resolve phrase starts; variation lives around a stable rhythmic skeleton. */
 if(wm.repeat_style>=3u&&((scatter.serial-1u)%wm.loop_steps)==0u)return;
 if(wm.repeat_style==7u&&((scatter.serial-1u)%8u)<6u)return;
 if(r%100u>=chance)return;
 scatter.kind=(uint8_t)(1u+(r>>8)%6u);
 static const uint8_t divs[8]={2,3,4,6,8,12,16,24};
 uint32_t range=amount<40?3u:amount<88?5u:8u;
 scatter.divisions=divs[(r>>16)%range];scatter.parts=scatter.divisions>8?8:scatter.divisions;
 scatter.offset=(uint16_t)(1u+(wm.scatter_variation<32u?(scatter.serial/4u)%3u:(r>>21)%7u));scatter.span=(uint8_t)(1u+(r>>28)%3u);
 if(scatter.kind==SC_DROP)scatter.parts=1;
 if(scatter.kind==SC_HOP||scatter.kind==SC_JUMP)scatter.parts=1;
 if(scatter.kind==SC_RUSH)scatter.parts=3u+(r>>18)%4u;
 if((r>>12)%5u==0)scatter.rest=(uint8_t)(amount<70?3u:1u);
}
static uint32_t scatter_note_step(uint32_t step,uint32_t length)
{
 if(scatter.kind==SC_HOP)return (step+length+(scatter.offset&1u?1u:length-1u))%length;
 if(scatter.kind==SC_JUMP)return (step+length-scatter.offset%length)%length;
 return step;
}
