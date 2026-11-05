/* SPDX-License-Identifier: GPL-3.0-only
 * Wanderer 1.7: fixed-point instruments, native MSFA FM6 and BLEP sync.
 * Sync adapted from Skiff/Felucca, Leo Kuroshita / Hugelton Instruments,
 * GPL-3.0-only. Complete pinned sources and notices: assets/dsp/.
 */
#include "wanderer_dsp.h"
#include "wanderer_fm6.c"
static inline int32_t wn_abs(int32_t x){return x<0?-x:x;}
typedef struct {int32_t value;uint8_t stage;} ws_env_t;
typedef struct {int32_t a1,a2,a3,ic1,ic2,frac1,frac2;} ws_lp_stage_t;
typedef struct {
 ws_patch_t p;ws_env_t amp,filter,op[4];
 uint32_t ph[4],inc[4],pitch_step,lfo,rng,control,pos,length,excite,pick;
 int32_t last,velocity,feedback,feedback_old,shape,damp,pulse,drive,detune,sync_step;
 int32_t amp_now,amp_step,op_now[4],op_step[4],alias[4],cut_sm,dc_prev,dc_out;
 int32_t a1,a2,a3,k,ic1,ic2,ks_prev,ks_ap_in,ks_ap_out,ks_coef,ks_loss;
 int32_t mode_x[4],mode_y[4],mode_c[4],mode_s[4];
 int32_t coef_cut,coef_res,coef_mode,fm_index;
 ws_lp_stage_t lp[2];
 wn_lad_coef_t ladder;wn_lad_state_t ladder_state;wn_pd_t pd[2];uint8_t modern,modal_decay[4];
} ws_voice_t;
static ws_voice_t ws_voice[NV];
static int16_t ws_string[NV][4096] __attribute__((section(".pool")));
static int32_t ws_clamp(int32_t x,int32_t lo,int32_t hi){return wn_clampi(x,lo,hi);}
static int32_t ws_env(ws_env_t *e,const uint8_t *p,uint8_t gate)
{
 const int32_t top=1<<24;if(!gate)e->stage=3;
 if(e->stage==0){int32_t d=top+top/4-e->value;e->value+=(int32_t)(((uint32_t)(d>>8)*WN_ATTACK_RATE[p[0]])>>8)+1;
  if(e->value>=top){e->value=top;e->stage=1;}}
 else if(e->stage==2)e->value=(int32_t)p[2]*132104;
 else{int32_t target=e->stage==3?0:(int32_t)p[2]*132104,d=e->value-target;uint32_t mag=d<0?-d:d;
  int32_t change=(int32_t)(((mag>>8)*WS_RATE[p[e->stage==3?3:1]])>>8)+1;e->value+=d<0?change:-change;
  if((e->stage==1&&e->value<=target+512)||(!target&&e->value<512)){e->value=target;if(e->stage==1)e->stage=2;}}
 return e->value>>9;
}
static uint32_t ws_ratio_inc(uint32_t base,uint32_t ratio)
{uint64_t n=((uint64_t)base*ratio)>>4;return n>0x78000000u?0x78000000u:(uint32_t)n;}
static int32_t ws_alias_gain(uint32_t base,uint32_t ratio)
{uint64_t n=((uint64_t)base*ratio)>>4;if(n>=0x78000000u)return 0;return n<=0x50000000u?32767:(int32_t)((0x78000000u-(uint32_t)n)>>15)*32767/20480;}
static const ws_patch_t *ws_current_patch(void)
{return wm.patch_valid?&wm.patch:ws_factory_patch(wm.preset_bank,wm.sound);}
static uint8_t ws_layer_own(void){return wm.layer_sound&&!wm.layer_link;}
static const ws_patch_t *ws_layer_patch(void)
{return wm.layer_patch_valid?&wm.layer_patch:ws_factory_patch(wm.layer_preset_bank,wm.layer_sound-1u);}
static const ws_patch_t *ws_edit_patch(void)
{return layer_focus&&ws_layer_own()?ws_layer_patch():ws_current_patch();}
static uint8_t ws_edit_sound(void)
{return layer_focus&&ws_layer_own()?wm.layer_sound-1u:wm.sound;}
static const char *ws_edit_name(void)
{
 if(layer_focus&&ws_layer_own())return wm.layer_legacy?"CLASSIC":ws_factory_name(wm.layer_preset_bank,wm.layer_sound-1u);
 return wm.legacy?"CLASSIC":ws_factory_name(wm.preset_bank,wm.sound);
}
/* Restore a retained independent patch or take a snapshot of Memory when
 * splitting a linked part for the first time. */
static void ws_layer_copy_memory(void)
{
 wm.layer_sound=wm.sound+1u;wm.layer_preset_bank=wm.preset_bank;wm.layer_legacy=wm.legacy;
 wm.layer_patch_valid=wm.patch_valid;if(wm.patch_valid)wm.layer_patch=wm.patch;
 wm.layer_link=0;
}
static void ws_layer_unlink(void)
{
 if(!wm.layer_sound)ws_layer_copy_memory();
 wm.layer_link=0;
}
static void ws_select(uint8_t index)
{
 if(index>=WM_PRESET_COUNT)return;
 if(layer_focus){
  wm.layer_sound=index+1u;wm.layer_link=wm.layer_legacy=wm.layer_patch_valid=0;wm.layer_preset_bank=WS_BANK_CURRENT;
 }else{
  wm.sound=index;wm.legacy=0;wm.preset_bank=WS_BANK_CURRENT;wm.patch=WS_PRESETS_2[index];wm.patch_valid=1;
 }
}
/* Nominal cutoff in Q8 semitones for the explicit low-pass modes. */
static int32_t ws_lp_cut(uint32_t knob){return (25<<8)+(int32_t)(knob*(105u<<8)/127u);}
static void ws_start(uint32_t v,uint32_t note,uint32_t vel)
{
 ws_voice_t *s=&ws_voice[v];uint32_t phases[4];memcpy(phases,s->ph,sizeof phases);int32_t last=s->last;memset(s,0,sizeof *s);
 uint8_t source=voice[v].patch_source;
 if(source==PATCH_LAYER)s->p=*ws_layer_patch();
 else if(source==PATCH_ANCHOR)s->p=wm.anchor_patch_valid?wm.anchor_patch:*ws_factory_patch(wm.anchor_preset_bank,wm.anchor_sound);
 else s->p=*ws_current_patch();
 s->modern=!(source==PATCH_LAYER?wm.layer_legacy:source==PATCH_ANCHOR?wm.anchor_legacy:wm.legacy);
 s->last=last;s->velocity=vel;
 s->rng=(ages*747796405u)^((note+1u)*2891336453u);if(!s->rng)s->rng=1;
 s->coef_cut=-1;s->coef_res=-1;s->pitch_step=tone_step(note);s->cut_sm=s->p.filter>=WS_LP12?ws_lp_cut(s->p.cutoff):s->p.cutoff*137*256/127;
 if(s->p.engine==5)wn_fm6_start(v,&s->p,note,vel);
 if(!s->p.engine){memcpy(s->ph,phases,sizeof phases);if(!s->ph[0]&&!s->ph[1]){s->ph[0]=s->rng;s->ph[1]=s->rng*2654435761u;s->ph[2]=s->rng>>1;}}
 if(s->p.engine!=5)for(uint32_t o=0;o<4;o++)s->inc[o]=ws_ratio_inc(s->pitch_step,s->p.ratio[o]);
 if(!s->p.engine&&s->p.wave==5)s->inc[3]=s->pitch_step;
 if(s->p.engine==3){
 /* Fractional allpass, compensating the loss filter's delay; no random per-note notch. */
 uint32_t inc=s->pitch_step,den=inc>>16,whole=0xffffffffu/inc,frac=(0xffffffffu%inc)/(den?den:1u);
 if(frac>65535u)frac=65535u;uint32_t delay=whole*65536u+frac-(uint32_t)s->p.damp*256u;
 s->length=delay>>16;if(s->length<3)s->length=3;if(s->length>4094)s->length=4094;
 frac=delay&65535u;s->ks_coef=(int32_t)(((65536u-frac)*32768u)/(65536u+frac));
 s->ks_loss=WN_STRING_LOSS[note][s->p.shape];s->pick=s->length*s->p.position/128u;
 if(!s->pick)s->pick=1;if(s->pick>=s->length)s->pick=s->length-1;s->excite=s->length;
 }
 if(s->p.engine==4)for(uint32_t k=0;k<4;k++){s->modal_decay[k]=255;int32_t strike=wn_abs(wn_sine_i((k+1u)*(s->p.position+12u)*0x01000000u));
  int32_t hard=ws_clamp(100+((int32_t)s->p.shape-64)*(int32_t)k/2,15,165);
  s->mode_x[k]=((s->p.op_level[k]*(12000+strike/2))>>7)*hard/128;}
}
static uint32_t wn_random(ws_voice_t *s)
{uint32_t x=s->rng;x^=x<<13;x^=x>>17;x^=x<<5;return s->rng=x;}
static int32_t wn_sine_hi(uint32_t ph)
{uint32_t i=ph>>22;int32_t a=WN_SINE_HI[i],b=WN_SINE_HI[(i+1)&1023];return a+(int32_t)(((int64_t)(b-a)*(ph&0x3fffffu))>>22);}
static void ws_lp_coef(ws_lp_stage_t *p,int32_t g,int32_t damping)
{
 int32_t den=4096+((g*(g+damping))>>12);
 p->a1=(int32_t)((4096u<<13)/(uint32_t)den);
 p->a2=(p->a1*g)>>12;
 p->a3=(p->a2*g)>>12;
}
/* Trapezoidal state-variable low pass, with independent cutoff and damping.
 * Half-scale input and bounded states keep all sample-rate products in 32 bits. */
static int32_t ws_lp_step(ws_lp_stage_t *p,int32_t in)
{
 int32_t v3=in-p->ic2;
 /* Carry the sub-sample fraction forward. Rounding every sample down
  * adds a DC offset when the cutoff is low and the filter is nearly still. */
 int32_t band=p->a1*p->ic1+p->a2*v3+p->frac1;
 int32_t low=p->a2*p->ic1+p->a3*v3+p->frac2;
 int32_t v1=band>>13,v2=p->ic2+(low>>13);
 p->frac1=band&8191;p->frac2=low&8191;
 p->ic1=ws_clamp(2*v1-p->ic1,-65535,65535);
 p->ic2=ws_clamp(2*v2-p->ic2,-65535,65535);
 return v2;
}
static void ws_control(uint32_t v,uint8_t gate,int32_t melt)
{
 ws_voice_t *s=&ws_voice[v];const ws_patch_t *p=&s->p;int32_t amp=ws_env(&s->amp,p->amp,gate),fe=ws_env(&s->filter,p->fenv,gate);
 s->amp_step=(amp-s->amp_now)/16;
 if(p->engine==1||p->engine==4)for(uint32_t o=0;o<4;o++){int32_t e=ws_env(&s->op[o],p->op_env[o],gate);s->op_step[o]=(e-s->op_now[o])/16;}
 s->lfo+=WS_LFO[p->lfo_rate];int32_t lf=wn_sine_i(s->lfo),cents=p->lfo_dest==2?(lf*p->lfo_depth)>>15:0;
 s->pitch_step=(uint32_t)(((uint64_t)voice[v].inc*WN_CENTS[ws_clamp(cents,-127,127)+127])>>16);
 s->shape=ws_clamp(p->shape+p->morph[1]*melt/127+(p->lfo_dest==1?(lf*p->lfo_depth)>>15:0),0,127);
 if(p->engine==1){
  int32_t index=s->shape*(80+s->velocity*47/127)/127;
  s->fm_index=index*(int32_t)WN_FM_BAND[voice[v].note]/32767;
 }
 s->damp=ws_clamp(p->damp+p->morph[2]*melt/127,0,127);
 s->pulse=ws_clamp(p->pulse+p->morph[3]*melt/127+(p->engine==0&&p->lfo_dest==1?(lf*p->lfo_depth)>>15:0),8,119);
 s->drive=ws_clamp(p->drive+p->morph[4]*melt/127,0,127);s->detune=ws_clamp(p->detune+p->morph[5]*melt/127,0,127);
 uint8_t modern_lp=p->filter>=WS_LP12;
 int32_t cut=modern_lp?ws_lp_cut(p->cutoff):p->cutoff*137*256/127;
 if(p->engine<2)cut+=((int32_t)voice[v].note-60)*p->position*256/127;
 if(p->engine!=2||modern_lp)cut+=(fe*p->envdepth*(modern_lp?9:21))/(127*32);
 if(p->lfo_dest==0)cut+=(lf*p->lfo_depth*(modern_lp?1:3))>>10;
 cut+=p->morph[0]*melt*256/127;s->cut_sm+=(ws_clamp(cut,modern_lp?25*256:0,(modern_lp?130:136)*256)-s->cut_sm)/4;
 if(s->coef_cut!=s->cut_sm||s->coef_res!=p->resonance||s->coef_mode!=(p->engine*WS_FILTER_COUNT+p->filter)){
 s->coef_cut=s->cut_sm;s->coef_res=p->resonance;s->coef_mode=p->engine*WS_FILTER_COUNT+p->filter;
 int32_t i=s->cut_sm>>8,g=WN_SVF_G[i];if(i<136)g+=((WN_SVF_G[i+1]-g)*(s->cut_sm&255))>>8;
 if(modern_lp){
  /* At zero resonance the 24 dB mode uses a Butterworth pole pair.
   * Only the resonant section's damping changes, preserving DC gain. */
  int32_t neutral=p->filter==WS_LP12?5793:3135;
  int32_t damping=neutral-((neutral-780)*p->resonance*p->resonance)/(127*127);
  ws_lp_coef(&s->lp[0],g,damping);
  if(p->filter==WS_LP24)ws_lp_coef(&s->lp[1],g,7568);
 }else{
 s->k=8192-p->resonance*7400/127;int32_t den=4096+((g*(g+s->k))>>12);
 s->a1=(int32_t)((4096u<<13)/(uint32_t)den);s->a2=(s->a1*g)>>12;s->a3=(s->a2*g)>>12;
 if(!p->engine&&!p->filter)wn_lad_coef(&s->ladder,s->cut_sm,p->resonance*3850/127);
 }
 }
 if(p->engine==5)return;
 if(p->engine!=2&&p->engine!=3)for(uint32_t k=0;k<4;k++){
  if(!p->engine&&k!=1u)continue;
  uint32_t ratio=p->engine==4&&p->wave==3?16u*(2u*k+1u):p->ratio[k]+(p->engine==4&&k?p->inharm*k/32u:0u);
  if(p->engine==1&&k>=2)ratio<<=(p->sub>3?3:p->sub);
  uint32_t previous_inc=s->inc[k];
  s->inc[k]=ws_ratio_inc(s->pitch_step,ratio);if(p->engine==1||p->engine==4)s->alias[k]=ws_alias_gain(s->pitch_step,ratio);
  if(p->engine==4){int32_t d=ws_clamp(p->op_env[k][1]-s->damp*(int32_t)k/(p->wave==1?5:p->wave==2?20:10),0,127);
   /* Reuse the pole rotation while pitch and damping are stationary. */
   if(previous_inc==s->inc[k]&&s->modal_decay[k]==d)continue;
   s->modal_decay[k]=(uint8_t)d;uint32_t r=WN_RADIUS24[d];
   int32_t a=wn_sine_hi(s->inc[k]+0x40000000u),b=wn_sine_hi(s->inc[k]);
   /* Normalize the interpolated rotation: pitch must not change a body's decay. */
   int32_t error=(1<<30)-(int32_t)(((int64_t)a*a+(int64_t)b*b)>>30);
   a+=(int32_t)(((int64_t)a*error)>>31);b+=(int32_t)(((int64_t)b*error)>>31);
   s->mode_c[k]=(int32_t)(((int64_t)a*r)>>30);s->mode_s[k]=(int32_t)(((int64_t)b*r)>>30);}
 }
 if(!p->engine){
  if(p->wave==5)s->inc[3]=(uint32_t)(((uint64_t)s->pitch_step*WN_CENTS[127-s->detune])>>16);
  s->inc[0]=s->pitch_step;s->inc[1]=(uint32_t)(((uint64_t)s->inc[1]*WN_CENTS[127+s->detune])>>16);
  if(p->wave==8){uint32_t q=(uint32_t)ws_clamp(s->detune+(fe*s->shape)/(32768*2),0,127);s->inc[1]=(uint32_t)(((uint64_t)s->pitch_step*WN_SYNC_RATIO[q])>>16);}
  if(s->inc[1]>0x78000000u)s->inc[1]=0x78000000u;}
 if(p->engine==2){uint32_t d=ws_clamp(s->shape*441+(fe*p->envdepth*7)/(127*4),0,56000);if(d>WN_PD_DEPTH[voice[v].note])d=WN_PD_DEPTH[voice[v].note];
  wn_pd_setup(&s->pd[0],p->wave,d);wn_pd_setup(&s->pd[1],p->algorithm,d);s->inc[0]=s->pitch_step;s->inc[1]=(uint32_t)(((uint64_t)s->pitch_step*WN_CENTS[127+s->detune])>>16);}
 if(p->engine==3)s->ks_loss=WN_STRING_LOSS[voice[v].note][s->shape];
}
static int32_t wn_triangle(uint32_t ph,uint32_t inc)
{int32_t a=wn_sine_i(ph),b=wn_sine_i(ph*3u),c=wn_sine_i(ph*5u);return (a-((b*ws_alias_gain(inc,48))>>15)/9+((c*ws_alias_gain(inc,80))>>15)/25)*4/5;}
static int32_t wn_analog_wave(uint32_t ph,uint32_t inc,uint32_t wave,uint32_t pw)
{if(wave==3)return wn_sine_i(ph);if(wave==4)return wn_triangle(ph,inc);if(wave==2)return wn_osc_pulse(ph,inc,pw)*2;if(wave==1)return (wn_osc_saw(ph,inc)+wn_osc_pulse(ph,inc,pw))/2;return wn_osc_saw(ph,inc);}
static int32_t wn_fm_wave(uint32_t ph,uint32_t wave)
{int32_t a=wn_sine_i(ph);if(wave==1)return a<0?0:a;if(wave==2)return wn_abs(a)-20860;if(wave==3)return (a+wn_sine_i(ph*2u)/2)*2/3;if(wave==4)return (a+wn_sine_i(ph*3u)/3)*3/4;return a;}
static int32_t ws_osc(uint32_t v)
{
 ws_voice_t *s=&ws_voice[v];const ws_patch_t *p=&s->p;uint32_t inc=s->pitch_step;int32_t x=0;
 if(!p->engine){uint32_t ph=s->ph[0],ph2=s->ph[1],pw=(uint32_t)s->pulse<<25;
  int32_t a=0,b=0;
  if(p->wave<5u||p->wave>=8u)a=wn_analog_wave(ph,inc,p->wave,pw);
  if(p->wave<5u)b=wn_analog_wave(ph2,s->inc[1],p->wave,pw);
  if(p->wave==0)b=(b*(128-s->shape)+wn_triangle(ph2,s->inc[1])*s->shape)>>7;
  if(p->wave==8){int32_t bl=wn_blep(ph2,s->inc[1]);
   if(s->sync_step){bl=(bl*((s->sync_step-1)>>1))>>15;s->sync_step=0;}
   b=(int32_t)(ph2>>16)-32768-bl;
   if(ph>0xffffffffu-inc)b-=(wn_blep(ph,inc)*(int32_t)(ph2>>17))>>15;
   uint32_t next=ph+inc;if(next<inc){uint32_t f=next/((inc>>16)|1u),at=ph2+s->inc[1]-f*(s->inc[1]>>16);s->sync_step=(int32_t)(at>>16)+1;s->ph[1]=f*(s->inc[1]>>16)-s->inc[1];}
  }
  else if(p->wave==9){a=(int32_t)(((int64_t)a*wn_sine_i(ph2))>>15);b=a;}
  else if(p->wave==5){uint32_t low=s->inc[3];a=(wn_osc_saw(ph,inc)+wn_osc_saw(ph2,s->inc[1])+wn_osc_saw(s->ph[3],low))/3;s->ph[3]+=low;b=a;}
  else if(p->wave==6){a=(wn_sine_i(ph)+((wn_sine_i(ph*3u)*ws_alias_gain(inc,48))>>15)/3+((wn_sine_i(ph*5u)*ws_alias_gain(inc,80))>>15)/5)*2/3;b=a;}
  else if(p->wave==7){a=b=(int32_t)(wn_random(s)>>17)-16384;}
  x=(a*(128-p->mix)+b*p->mix)>>7;if(p->sub)x=(x*128+wn_sine_i(s->ph[2])*p->sub)/(128+p->sub);
  if(p->inharm)x+=((int32_t)(wn_random(s)>>17)-16384)*p->inharm/256;
  s->ph[0]+=inc;s->ph[1]+=s->inc[1];s->ph[2]+=inc>>1;
 }else if(p->engine==1){int32_t o[4];uint32_t alg=p->algorithm;
  int32_t index=s->fm_index;
  for(int32_t k=3;k>=0;k--){int32_t mod=0;if(k==3)mod=(s->feedback+s->feedback_old)/2;
   else if(alg==0)mod=o[k+1];else if(alg==1)mod=k==0?(o[1]+o[2]+o[3])/2:0;
   else if(alg==2)mod=k==0?o[1]:k==2?o[3]:0;else if(alg==3)mod=k==0?(o[1]+o[2])/2:k==1?o[3]:0;
   else if(alg==4)mod=k>0?o[k+1]:0;else if(alg==5)mod=k<2?o[2]:k==2?o[3]:0;else if(alg==6)mod=k<3?o[3]:0;
   uint32_t ph=s->ph[k]+(uint32_t)(mod*index)*512u;s->ph[k]+=s->inc[k];int32_t y=wn_fm_wave(ph,p->wave);y=(y*s->op_now[k])>>15;o[k]=(((y*p->op_level[k])>>7)*s->alias[k])>>15;}
  s->feedback_old=s->feedback;s->feedback=o[3]*p->feedback/128;
  x=alg==2?(o[0]*(128-p->mix)+o[2]*p->mix)/128:alg==4||alg==5?(o[0]+o[1])/2:alg==6?(o[0]+o[1]+o[2])/3:alg==7?(o[0]+o[1]+o[2]+o[3])/4:o[0];
 }else if(p->engine==2){int32_t a=wn_pd_wave(&s->pd[0],s->ph[0]>>16),b=wn_pd_wave(&s->pd[1],s->ph[1]>>16);
  s->ph[0]+=s->inc[0];s->ph[1]+=s->inc[1];s->ph[2]+=inc>>1;if(p->inharm==1)b=(a*b)>>15;
  x=(a*(128-p->mix)+b*p->mix)>>7;if(p->sub)x=(x*128+wn_sine_i(s->ph[2])*p->sub)/(128+p->sub);
 }else if(p->engine==3){uint32_t pos=s->pos;int32_t a;
  if(s->excite){int32_t tri=pos<s->pick?(int32_t)(pos*32768u/s->pick):(int32_t)((s->length-pos)*32768u/(s->length-s->pick));
   if(p->wave==1)a=(int32_t)(wn_random(s)>>17)-16384;
   else if(p->wave==2){uint32_t width=s->length/20u+2u;int32_t d=wn_abs((int32_t)pos-(int32_t)s->pick);
    /* Bipolar hammer: compact positive strike and a negative shoulder.
     * No arbitrary DC offset, even on the lowest playable strings. */
    a=d<(int32_t)width?24000*(int32_t)(width-d)/(int32_t)width:d<(int32_t)(width*2u)?-24000*(int32_t)(width*2u-d)/(int32_t)width:0;}
   else if(p->wave==3){uint32_t ph=(uint32_t)((uint64_t)pos*0xffffffffu/s->length);
    a=wn_sine_i(ph)/2+wn_sine_i(ph*2u)*(127-p->position)/512+wn_sine_i(ph*3u)*p->pulse/1024;}
   else{int32_t noise=(int32_t)(wn_random(s)>>17)-16384;
    a=((tri-16384)*(128-p->pulse)+noise*p->pulse)>>7;}
   ws_string[v][pos]=(int16_t)a;s->excite--;
  }else{a=ws_string[v][pos];int32_t low=(a*(256-s->damp)+s->ks_prev*s->damp)>>8;s->ks_prev=a;
   int32_t ap=s->ks_ap_in+((s->ks_coef*(low-s->ks_ap_out))>>15);s->ks_ap_in=low;s->ks_ap_out=ws_clamp(ap,-32767,32767);ws_string[v][pos]=(int16_t)((s->ks_ap_out*s->ks_loss)>>15);}
  s->pos=pos+1u==s->length?0u:pos+1u;
  s->feedback+=(a-s->feedback)/4;x=(a*(128-p->mix)+s->feedback*p->mix)>>7;
 }else if(p->engine==5)x=wn_fm6_sample(v,p,voice[v].note,(uint32_t)s->velocity,voice[v].gate!=0,s->shape);
 else{if(p->wave==3||p->wave==4||p->amp[2]>60){for(uint32_t k=0;k<4;k++){int32_t y=(wn_sine_i(s->ph[k])*s->op_now[k])>>15;s->ph[k]+=s->inc[k];x+=(((y*p->op_level[k])>>9)*s->alias[k])>>15;}}
  else for(uint32_t k=0;k<4;k++){int32_t a=s->mode_x[k],b=s->mode_y[k];int32_t nx=(int32_t)(((int64_t)a*s->mode_c[k]-(int64_t)b*s->mode_s[k])>>24),ny=(int32_t)(((int64_t)a*s->mode_s[k]+(int64_t)b*s->mode_c[k])>>24);
   s->mode_x[k]=ws_clamp(nx,-24000,24000);s->mode_y[k]=ws_clamp(ny,-24000,24000);x+=((nx*s->alias[k])>>15)/3;}}
 s->dc_out=x-s->dc_prev+((s->dc_out*32670)>>15);s->dc_prev=x;return s->dc_out;
}
static int32_t wn_filter(ws_voice_t *s,int32_t x)
{
 const ws_patch_t *p=&s->p;if(s->drive)x=wn_softclip((int32_t)(((int64_t)x*(32768+s->drive*384))>>15));
 if(p->filter>=WS_LP12){
  int32_t y=ws_lp_step(&s->lp[0],ws_clamp(x,-65535,65535)/2);
  if(p->filter==WS_LP24)y=ws_lp_step(&s->lp[1],ws_clamp(y,-65535,65535));
  int32_t a=wn_abs(y);
  if(a>18000)y=(y<0?-1:1)*(18000+wn_softclip((a-18000)*2)/2);
  return y*2;
 }
 if(!p->engine&&!p->filter)return wn_lad_step(&s->ladder,&s->ladder_state,x/2)*2;
 if(p->engine&&p->cutoff>=124&&!p->filter&&(!p->envdepth||p->engine==2)
    &&!(p->lfo_dest==0&&p->lfo_depth)&&!(p->engine==1&&p->position)&&!p->morph[0])return x;
 int32_t in=x/2,v3=in-s->ic2;
 int32_t v1=(int32_t)(((int64_t)s->a1*s->ic1+(int64_t)s->a2*v3)>>13),v2=s->ic2+(int32_t)(((int64_t)s->a2*s->ic1+(int64_t)s->a3*v3)>>13);
 s->ic1=ws_clamp(2*v1-s->ic1,-120000,120000);s->ic2=ws_clamp(2*v2-s->ic2,-120000,120000);
 int32_t y=p->filter==1?v1:p->filter==2?in-((s->k*v1)>>12)-v2:v2,a=wn_abs(y);
 if(a>18000)y=(y<0?-1:1)*(18000+wn_softclip((a-18000)*2)/2);return y*2;
}
static void ws_render(uint32_t v,int32_t *out,uint32_t n,int32_t melt)
{
 if(!voice[v].gate&&!voice[v].amp)return;ws_voice_t *s=&ws_voice[v];uint32_t remain=voice[v].remain;uint8_t gate=voice[v].gate!=0;
 for(uint32_t i=0;i<n;i++){
  if(remain&&!--remain)gate=0,voice[v].gate=0;if(!(s->control++&15u))ws_control(v,gate,melt);
  if(!gate&&(s->p.engine==5?wn_fm6[v].done:!s->amp.value)){voice[v].amp=0;break;}
  s->amp_now=ws_clamp(s->amp_now+s->amp_step,0,32768);if(s->p.engine==1||s->p.engine==4)for(uint32_t o=0;o<4;o++)s->op_now[o]=ws_clamp(s->op_now[o]+s->op_step[o],0,32768);
  int32_t x=ws_clamp(wn_filter(s,ws_osc(v)),-65535,65535);
  if(s->p.engine!=5){x=(int32_t)(((int64_t)x*s->amp_now)>>15);x=x*s->velocity/127;}
  x=x*s->p.level/256;
  if(voice[v].splice_left){uint32_t t=voice[v].splice_left--;x=(x*(int32_t)(32u-t)+voice[v].splice*(int32_t)t)/32;}
  s->last=x;voice[v].last=x;voice[v].amp=s->p.engine==5?(!wn_fm6[v].done?32768:0):(s->amp.value>>10)*s->velocity;
  if(s->amp.value&&!voice[v].amp&&s->p.engine!=5)voice[v].amp=1;out[i*2u]+=x;
 }voice[v].remain=remain;
}
