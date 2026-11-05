/* SPDX-License-Identifier: GPL-3.0-only
 * Wanderer adapter for the Apache-2.0 MSFA/Felucca core. No factory DX voices.
 * The existing patch record holds six ratios, levels and ADSR controls:
 * OP1..4 in their arrays; OP5 ratio/level/env in detune/sub/fenv;
 * OP6 ratio/level/env in pulse/mix/amp. No reinterpretation of older engines.
 */
#include "wanderer_fm6_core.c"
typedef struct {
 fm6_note_t note;fm6_lfo_t lfo;ws_patch_t source;
 uint8_t patch[FP_SIZE+1],pos,done;
 int32_t block[32];
} wn_fm6_t;
static wn_fm6_t wn_fm6[NV];
static void wn_fm6_patch(uint8_t *dst,const ws_patch_t *p)
{
 memset(dst,0,FP_SIZE+1);
 for(uint32_t i=0;i<6;i++){
  uint8_t *op=dst+(5u-i)*FP_OP;
  const uint8_t *env=i<4?p->op_env[i]:i==4?p->fenv:p->amp;
  uint32_t ratio=i<4?p->ratio[i]:i==4?p->detune:p->pulse;if(ratio<2)ratio=2;
  uint32_t coarse=ratio/4u;if(!coarse)coarse=0;
  uint32_t divisor=coarse?coarse*4u:2u;
  int32_t fine=((int32_t)ratio*100/(int32_t)divisor)-100;
  if(fine<0){coarse=0;fine=(int32_t)ratio*100/2-100;}
  op[FP_FC]=(uint8_t)coarse;op[FP_FF]=(uint8_t)wn_clampi(fine,0,99);op[FP_DET]=7;
  op[FP_OL]=(uint8_t)wn_clampi(i<4?p->op_level[i]:i==4?p->sub:p->mix,0,99);
  op[FP_R1]=WN_FM6_ATTACK[env[0]];op[FP_R1+1]=op[FP_R1+2]=WN_FM6_DECAY[env[1]];op[FP_R1+3]=WN_FM6_DECAY[env[3]];
  op[FP_L1]=99;op[FP_L1+1]=op[FP_L1+2]=WN_FM6_SUSTAIN[env[2]];op[FP_L1+3]=0;
  op[FP_BP]=36;op[FP_RD]=p->inharm/2;op[FP_RC]=0;op[FP_RS]=p->inharm/32;op[FP_KVS]=p->position/18;
 }
 for(uint32_t i=0;i<4;i++){dst[FP_PR1+i]=99;dst[FP_PL1+i]=50;}
 dst[FP_ALG]=p->algorithm&31u;dst[FP_FB]=p->feedback*7u/127u;
 dst[FP_LFS]=p->lfo_rate*99u/127u;dst[FP_LFW]=p->wave%6u;dst[FP_LPMS]=3;
 dst[FP_LPMD]=p->lfo_dest==2?p->lfo_depth*30u/127u:0;dst[FP_TRNSP]=24;
}
static void wn_fm6_start(uint32_t v,const ws_patch_t *p,uint32_t note,uint32_t vel)
{
 wn_fm6_t *s=&wn_fm6[v];memset(s,0,sizeof *s);s->source=*p;wn_fm6_patch(s->patch,p);
 fm6_note_init(&s->note,s->patch,(int32_t)note,(int32_t)vel,1);fm6_lfo_reset(&s->lfo,s->patch);fm6_lfo_key(&s->lfo);
 for(uint32_t i=0;i<6;i++)s->note.base[5u-i]=WN_FM6_RATIO[i<4?p->ratio[i]:i==4?p->detune:p->pulse];
}
static int32_t wn_fm6_sample(uint32_t v,const ws_patch_t *p,uint32_t note,uint32_t vel,uint8_t gate,int32_t index)
{
 wn_fm6_t *s=&wn_fm6[v];
 if(!s->pos){
  if(memcmp(&s->source,p,sizeof *p)){
   s->source=*p;wn_fm6_patch(s->patch,p);fm6_lfo_reset(&s->lfo,s->patch);
   for(uint32_t k=0;k<6;k++){
    const uint8_t *op=s->patch+k*FP_OP;int32_t ol=fm6_scaleout(op[FP_OL])+fm6_scale_level((int32_t)note,op);
    ol=(ol>127?127:ol)*32+fm6_scale_vel((int32_t)vel,op[FP_KVS]);s->note.env[k].outlevel=(int16_t)(ol<0?0:ol);
    uint32_t i=5u-k;s->note.base[k]=WN_FM6_RATIO[i<4?p->ratio[i]:i==4?p->detune:p->pulse];fm6_env_advance(&s->note.env[k],op,s->note.env[k].ix);
   }
  }
  if(s->note.down!=gate)fm6_note_key(&s->note,s->patch,gate);
  int32_t dt[6]={0};int32_t lfo=fm6_lfo_sample(&s->lfo),delay=fm6_lfo_delay(&s->lfo);
  memset(s->block,0,sizeof s->block);
  fm6_note_compute(&s->note,s->patch,s->block,lfo,delay,fm6_note_logfreq((int32_t)note*16,0),p->algorithm,p->feedback*7u/127u,dt,(index-64)*932067);
  s->done=(uint8_t)fm6_note_done(&s->note,s->patch,p->algorithm);
 }
 int32_t y=s->block[s->pos]/2048;s->pos=(uint8_t)((s->pos+1u)&31u);return y;
}
