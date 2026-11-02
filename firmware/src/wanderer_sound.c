/* SPDX-License-Identifier: GPL-3.0-only
 * Original four-voice fixed-point engine. No allocation or floating point in audio.
 * Families: band-limited subtractive, four-op FM, phase distortion, string, modal.
 */
typedef struct {int32_t value;uint8_t stage;} ws_env_t;
typedef struct {
 ws_patch_t p;ws_env_t amp,filter,op[4];
 uint32_t ph[4],inc[4],lfo,rng,excite,pos,length,control;
 int32_t f1,f2,f3,f4,tone,cut,h,h2,shape,pulse,detune,damp,drive,velocity,feedback,last,frac,pick,drive_gain;
 uint32_t pitch_step;
 uint32_t bend_a,bend_b;uint8_t modern,band2;
} ws_voice_t;
static ws_voice_t ws_voice[NV];
static int16_t ws_string[NV][4096] __attribute__((section(".pool")));
static int32_t ws_clamp(int32_t x,int32_t lo,int32_t hi){return x<lo?lo:x>hi?hi:x;}
static int32_t ws_env(ws_env_t *e,const uint8_t *p,uint8_t gate)
{
 const int32_t top=16777216;
 if(!gate)e->stage=3;
 if(e->stage==0){e->value+=(int32_t)WS_ATTACK[p[0]];if(e->value>=top){e->value=top;e->stage=1;}}
 else{
  int32_t target=e->stage==3?0:(int32_t)p[2]*132104;
  if(e->stage==2)e->value=target;
  else {int32_t delta=e->value-target;uint32_t mag=(uint32_t)(delta<0?-delta:delta);
   int32_t change=(int32_t)(((mag>>8)*WS_RATE[p[e->stage==3?3:1]])>>8)+1;
   if(delta<0)e->value+=change;else e->value-=change;
   if((e->stage==1&&e->value<=target+512)||(!target&&e->value<512)){e->value=target;if(e->stage==1)e->stage=2;}
  }
 }
 return e->value>>9;
}
static uint32_t ws_ratio_inc(uint32_t base,uint32_t ratio)
{
 uint32_t product=(base>>8)*ratio;
 while(product>0x06000000u)product>>=1;
 return product<<4;
}
static const ws_patch_t *ws_current_patch(void)
{return wm.patch_valid?&wm.patch:&WS_PRESETS[wm.sound];}
static void ws_select(uint8_t index)
{wm.sound=index;wm.legacy=0;wm.patch=WS_PRESETS[index];wm.patch_valid=1;}
static void ws_start(uint32_t v,uint32_t note,uint32_t vel)
{
 ws_voice_t *s=&ws_voice[v];int32_t last=s->last;
 memset(s,0,sizeof *s);s->p=anchor_view&&wm.anchor_patch_valid?wm.anchor_patch:*ws_current_patch();
 s->modern=(uint8_t)!(anchor_view&&wm.has_anchor?wm.anchor_legacy:wm.legacy);s->last=last;s->velocity=(int32_t)vel;
 s->rng=(ages*747796405u)^((note+1u)*2891336453u);s->lfo=ages*119304647u;
 /* Independent phase at each onset; detuning must not start as one thin waveform. */
 if(s->p.engine==0){s->ph[0]=s->rng;s->ph[1]=s->rng*2654435761u;s->ph[2]=s->rng>>1;}
 uint32_t inc=tone_step(note);for(uint32_t o=0;o<4;o++){
  uint32_t ratio=s->p.ratio[o];s->inc[o]=ws_ratio_inc(inc,ratio);
  /* Fold ratios down before the Nyquist boundary; no unbounded phase wrapping. */
  while(s->inc[o]>0x60000000u)s->inc[o]>>=1;
 }
 if(s->p.engine!=1)s->inc[0]=inc;s->pitch_step=inc;s->tone=s->p.cutoff*220*256/127;
 s->length=inc?0xffffffffu/inc+1u:4095u;
 s->frac=inc?(int32_t)(32768u-(0xffffffffu%inc)/(inc>>16)/2u):0;
 if(s->length<3)s->length=3;if(s->length>4095)s->length=4095;
 s->pick=(int32_t)(s->length*s->p.position/128u);
 s->excite=s->length;s->control=0;
}
static void ws_control(uint32_t v,uint8_t gate,int32_t melt)
{
 ws_voice_t *s=&ws_voice[v];const ws_patch_t *p=&s->p;
 ws_env(&s->amp,p->amp,gate);int32_t fe=ws_env(&s->filter,p->fenv,gate);
 if(p->engine==1||p->engine==4)for(uint32_t o=0;o<4;o++){
  if(p->engine==4){uint8_t env[4];memcpy(env,p->op_env[o],4);
   env[1]=(uint8_t)ws_clamp((int32_t)env[1]-((p->damp+p->morph[2]*melt/127)*((int32_t)o+1))/12,0,127);ws_env(&s->op[o],env,gate);
  }else ws_env(&s->op[o],p->op_env[o],gate);
 }
 s->lfo+=WS_LFO[p->lfo_rate];
 int32_t lfo=sine(s->lfo)>>8;
 int32_t cut=p->cutoff*220/127+(p->morph[0]*melt)/127+((fe*p->envdepth)>>15);
 if(p->lfo_dest==0)cut+=(lfo*p->lfo_depth)>>8;
 cut+=((int32_t)voice[v].note-60)/4;
 s->tone+=(ws_clamp(cut,0,220)*256-s->tone)>>2;
 s->cut=FILTER_G[s->tone>>8];s->h=FILTER_H[p->resonance>>4][s->tone>>8];s->h2=FILTER_H[0][s->tone>>8];
 int32_t pitch=p->lfo_dest==2?(lfo*p->lfo_depth)>>8:0;
 uint32_t base=voice[v].inc;s->pitch_step=(uint32_t)((int32_t)base+((int32_t)(base>>16)*pitch*38));
 if(p->engine==1||p->engine==4)for(uint32_t k=0;k<4;k++){
   uint32_t ratio=p->ratio[k]+(p->engine==4&&k?(p->inharm*k)/24u:0u);
   s->inc[k]=ws_ratio_inc(s->pitch_step,ratio);while(s->inc[k]>0x60000000u)s->inc[k]>>=1;
 }
 s->shape=ws_clamp(p->shape+(p->morph[1]*melt)/127+(p->lfo_dest==1?(lfo*p->lfo_depth)>>8:0),1,126);
 s->damp=ws_clamp(p->damp+(p->morph[2]*melt)/127,0,127);
 if(p->engine==1&&voice[v].note>84u)s->shape>>=(voice[v].note>108u?3u:voice[v].note>96u?2u:1u);
 if(p->engine==2&&voice[v].note>72u)s->shape=64+(s->shape-64)*ws_clamp(108-(int32_t)voice[v].note,0,36)/36;
 s->pulse=ws_clamp(p->pulse+(p->morph[3]*melt)/127,8,119);
 s->drive=ws_clamp(p->drive+(p->morph[4]*melt)/127,0,127);
 s->drive_gain=4194304/(128+s->drive/3);
 s->detune=ws_clamp(p->detune+(p->morph[5]*melt)/127,0,127);
 if(p->engine==0){s->inc[1]=ws_ratio_inc(s->pitch_step,p->ratio[1])+(s->pitch_step>>15)*(uint32_t)s->detune;s->band2=wave_band(s->inc[1]);}
 /* PD slopes are calculated at control rate. Piecewise-linear phase is continuous. */
 if(p->engine==2){uint32_t split=(uint32_t)s->shape*512u+256u;
 s->bend_a=0x80000000u/split;s->bend_b=0x80000000u/(65536u-split);}
}
static uint32_t ws_modphase(int32_t sample,int32_t depth)
{return (uint32_t)(sample*depth)*1024u;}
static int32_t ws_osc(uint32_t v)
{
 ws_voice_t *s=&ws_voice[v];const ws_patch_t *p=&s->p;int32_t x=0;
 uint32_t inc=s->pitch_step,ph=s->ph[0];
 if(p->engine==0){
  s->ph[0]+=inc;s->ph[1]+=s->inc[1];s->ph[2]+=inc>>1;
  uint32_t band=voice[v].band1;
  int32_t a=p->wave==2?(wave_sample(ph,0,band)-wave_sample(ph+((uint32_t)s->pulse<<25),0,band))/2:p->wave==5?wave_sample(ph,2,band):osc_sample(ph,p->wave,band);
  int32_t b=osc_sample(s->ph[1],p->wave==5?0:p->wave,s->band2);
  /* Same-family unison retains harmonic weight; pulse/rounding is a deliberate blend. */
  int32_t pw=(wave_sample(s->ph[1],0,s->band2)-wave_sample(s->ph[1]+((uint32_t)s->pulse<<25),0,s->band2))/2;
  if(p->wave==2)b=pw;
  else if(p->wave==1)b=(b*(128-s->shape)+pw*s->shape)>>7;
  else if(p->wave==5)b=(b+osc_sample(s->ph[1]+0x551d2a3fu,0,s->band2))/2;
  else b=(b*(256-s->shape)+sine(s->ph[1])*s->shape)>>8;
  x=(a*(128-p->mix)+b*p->mix)>>7;
  /* Add a solid sub fundamental without replacing the body of the main oscillators. */
  x=(x*128+sine(s->ph[2])*p->sub)/(128+p->sub/2);
 }else if(p->engine==1){
  int32_t o[4];uint32_t alg=p->algorithm;
  for(int32_t k=3;k>=0;k--){
   int32_t mod=0;if(k==3)mod=s->feedback;
   else if(alg==0)mod=o[k+1];
   else if(alg==4)mod=k>0?o[k+1]:0;
   else if(alg==1)mod=k==0?o[1]+o[2]+o[3]:0;
   else if(alg==2)mod=(k==0?o[1]:k==2?o[3]:0);
   else if(alg==3)mod=(k==0?o[1]+o[2]:k==1?o[3]:0);
   else if(alg==5)mod=(k<2?o[2]:k==2?o[3]:0);
   else if(alg==6)mod=(k<3?o[3]:0);
   s->ph[k]+=s->inc[k];
   int32_t y=sine(s->ph[k]+ws_modphase(mod,s->shape));
   o[k]=((y*(s->op[k].value>>9))>>15)*p->op_level[k]>>7;
  }
  s->feedback=(o[3]*p->feedback)>>8;
  x=alg==2?(o[0]+o[2])/2:alg==4||alg==5?(o[0]+o[1])/2:alg==6?(o[0]+o[1]+o[2])/3:alg==7?(o[0]+o[1]+o[2]+o[3])/4:o[0];
 }else if(p->engine==2){
  s->ph[0]+=inc;s->ph[1]+=inc+(inc>>14)*(uint32_t)s->detune;
  uint32_t a=ph>>16,split=(uint32_t)s->shape*512u+256u;
  uint32_t bent=a<split?a*s->bend_a:0x80000000u+(a-split)*s->bend_b;
  int32_t a1=sine(bent),b=sine(s->ph[1]);
  x=(a1*(128-p->mix)+b*p->mix)>>7;
 }else if(p->engine==3){
  uint32_t pos=s->pos,next=pos+1u==s->length?0u:pos+1u;
  int32_t a=ws_string[v][pos],b=ws_string[v][next];
  if(s->excite){
   uint32_t other=pos+(uint32_t)s->pick;if(other>=s->length)other-=s->length;
   uint32_t r=(pos+s->rng)*747796405u+2891336453u,t=(other+s->rng)*747796405u+2891336453u;
   r=((r>>((r>>28u)+4u))^r)*277803737u;t=((t>>((t>>28u)+4u))^t)*277803737u;
   a=((int32_t)(r>>16)-(int32_t)(t>>16))/2;s->excite--;
  }else{
   int32_t old=a;int32_t symmetric=(b+s->feedback)/2;
   a=(a*(128-s->damp)+symmetric*s->damp)>>7;
   a=a+(((b-a)*s->frac)>>15);s->feedback=old;
   a=(a*(32540+s->shape))>>15;
  }
  ws_string[v][pos]=(int16_t)a;s->pos=next;x=a;
 }else{
  for(uint32_t k=0;k<4;k++){
   s->ph[k]+=s->inc[k];
   x+=((((sine(s->ph[k])*(s->op[k].value>>9))>>15)*p->op_level[k])>>9)*(128-(k?p->position/2:0))>>7;
  }
 }
 return x;
}
static void ws_render(uint32_t v,int32_t *out,uint32_t n,int32_t melt)
{
 if(!voice[v].gate&&!voice[v].amp)return;
 ws_voice_t *s=&ws_voice[v];uint32_t remain=voice[v].remain;int32_t g=voice[v].gate;
 for(uint32_t i=0;i<n;i++){
  if(remain&&!--remain)g=voice[v].gate=0;
  if(!(s->control++&15u))ws_control(v,(uint8_t)(g!=0),melt);
  if(!g&&s->amp.value==0){voice[v].amp=0;break;}
  int32_t x=ws_osc(v);x=(x*(128+s->drive))>>7;
  x=ws_clamp(x,-28000,28000);x-=((((x*x)>>15)*x)>>15)/3;
  int32_t v1=((s->f1+((s->cut*(x-s->f2))>>14))*s->h)>>14;
  int32_t v2=s->f2+((s->cut*v1)>>14);
  s->f1=ws_clamp(2*v1-s->f1,-65535,65535);s->f2=ws_clamp(2*v2-s->f2,-65535,65535);
  int32_t filtered=s->p.filter==0?v2:s->p.filter==1?v1:x-v2-((v1*(243-((s->p.resonance>>4)*28)))>>7);
  if(s->p.engine==0&&s->p.filter==0){
   /* Cascaded TPT low pass: four poles for subtractive patches, no unstable feedback ladder. */
   int32_t q=s->h2;
   int32_t u=((s->f3+((s->cut*(filtered-s->f4))>>14))*q)>>14;
   int32_t w=s->f4+((s->cut*u)>>14);
   s->f3=ws_clamp(2*u-s->f3,-65535,65535);s->f4=ws_clamp(2*w-s->f4,-65535,65535);
   filtered=w;
  }
  filtered=ws_clamp(filtered,-32767,32767);
  filtered=(filtered*s->drive_gain)>>15;
  x=(filtered*(s->amp.value>>9))>>15;x=(x*s->velocity)>>7;x=(x*s->p.level)>>8;
  if(voice[v].splice_left){uint32_t t=voice[v].splice_left--;x=(x*(int32_t)(32u-t)+voice[v].splice*(int32_t)t)>>5;}
  s->last=x;voice[v].last=x;voice[v].amp=(s->amp.value>>10)*s->velocity;
  if(s->amp.value&&voice[v].amp==0)voice[v].amp=1; /* Finish sub-LSB releases independent of callback size. */
  out[i*2u]+=x;
 }
 voice[v].remain=remain;
}
