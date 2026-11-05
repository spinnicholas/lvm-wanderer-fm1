/* SPDX-License-Identifier: GPL-3.0-only
 * Wanderer 1.7: separate chorus, digital delay and multi-head tape echo.
 * Insert order: wavefolder, bitcrush, stereo phaser, then time/space buses.
 * All buffer clearing is bounded; no allocation or floating point in audio.
 */
#define DELAY_MASK 16383u
#define TAPE_LEN 12288u
#define CHORUS_MASK 1023u
static int16_t delay_line[16384] __attribute__((section(".pool")));
static int16_t tape_line[TAPE_LEN] __attribute__((section(".pool")));
static int16_t chorus_line[1024] __attribute__((section(".pool")));
static int16_t verb_a[887] __attribute__((section(".pool")));
static int16_t verb_b[1091] __attribute__((section(".pool")));
static int16_t verb_c[1153] __attribute__((section(".pool")));
static int16_t verb_d[965] __attribute__((section(".pool")));
/* Ambient stores four fractional bits per sample to preserve quiet tails
 * through hundreds of passes. Old fractions are hidden by verb_age. */
enum {AMB_A=521,AMB_B=733,AMB_C=977,AMB_D=857};
static uint8_t verb_frac[(AMB_A+AMB_B+AMB_C+AMB_D+1)/2] __attribute__((section(".pool")));
static const uint16_t AMB_OFFSET[4]={0,AMB_A,AMB_A+AMB_B,AMB_A+AMB_B+AMB_C};
static int16_t verb_ap[2][256] __attribute__((section(".pool")));
#define VERB_PRE_MASK 1023u
static int16_t verb_pre[VERB_PRE_MASK+1u] __attribute__((section(".pool")));
typedef struct {
 uint16_t lines[4],diffusers[2],pre,early[2];
 uint8_t diffusion,damp,feedback,range,width;
} reverb_profile_t;
/* 3.0 network refinements, per type: in-loop modulation depth (Q8 samples),
 * low-cut shift (higher = lower corner; 0 = none) and output tap density. */
static const uint8_t VERB_MOD[RV_COUNT]={0,96,40,0,128,224,0};
static const uint8_t VERB_LOWCUT[RV_COUNT]={6,7,7,0,7,8,0};
/* Delay lengths are samples at the 16 kHz wet clock. Every geometry fits
 * the legacy buffers. The feedback matrix is shared across all seven types. */
static const reverb_profile_t VERB_PROFILES[RV_COUNT]={
 {{887,1091,1153,965},{149,211},0,{0,0},64,160,70,42,0},      /* Room */
 {{733,941,1097,863},{113,197},271,{145,217},78,189,85,34,82}, /* Hall */
 {{337,487,631,557},{53,89},61,{19,41},88,219,92,28,105},      /* Plate */
 {{181,359,503,263},{127,71},47,{101,179},94,173,78,36,115},   /* Spring */
 {{881,1069,1129,947},{107,167},821,{269,581},52,137,88,33,86}, /* Cavern */
 {{881,1087,1147,953},{181,241},997,{0,0},83,200,116,10,118},  /* Deep Space */
 {{AMB_A,AMB_B,AMB_C,AMB_D},{151,233},413,{0,0},82,160,0,0,66},      /* Ambient */
};
static uint32_t delay_write,delay_frames_smooth,fx_phase,tape_phase,tape_write,tape_delay_q,chorus_write,chorus_phase,fx_frames;
static uint32_t wow_phase,flutter_phase,phaser_phase,crush_count;
static uint16_t verb_pos[4],ap_pos[2],verb_age,verb_pre_pos,verb_gain;
static uint8_t verb_type;
static int32_t verb_input_tone;
static int32_t verb_low[4],verb_fb[4];
static int32_t verb_tone[4],mix_smooth[7],phaser_s[2][6],phaser_y[2],phaser_a[2];
static int32_t fx_input,tape_tone,tape_dc,tape_l,tape_r,tape_old_l,tape_old_r,fold_lp,crush_hold;
static int32_t wet_l,wet_r,wet_old_l,wet_old_r;
static volatile uint8_t fx_clearing;
static uint8_t phaser_awake;
static void verb_restart(uint8_t type)
{
 verb_type=type;verb_age=verb_pre_pos=0;verb_input_tone=0;
 memset(verb_pos,0,sizeof verb_pos);memset(verb_tone,0,sizeof verb_tone);memset(ap_pos,0,sizeof ap_pos);
 memset(verb_low,0,sizeof verb_low);
}
static uint32_t fx_clear_part,fx_clear_at;
static const uint8_t FX_HOLD[16]={1,2,3,4,5,6,8,10,12,16,20,24,32,40,48,64};
static int16_t fx_bound(int32_t x){return (int16_t)ws_clamp(x,-32767,32767);}
static int32_t fx_limit(int32_t x)
{
 if(x>24000)return 30000-(int32_t)(36000000u/((uint32_t)x-18000u));
 if(x< -24000)return -30000+(int32_t)(36000000u/((uint32_t)(-x)-18000u));
 return x;
}
static int32_t fx_slew(int32_t x,int32_t t,uint32_t shift)
{int32_t d=t-x;if(!d)return x;int32_t step=d/(int32_t)(1u<<shift);return x+(step?step:d>0?1:-1);}
static int32_t fx_blend(int32_t a,int32_t b,int32_t q)
{return a+(int32_t)(((int64_t)(b-a)*q)>>15);}
static int32_t fx_mix(uint8_t p){return p==127?32768:(int32_t)p*258;}
static int32_t fx_read(const int16_t *line,uint32_t mask,uint32_t write,uint32_t q)
{uint32_t d=q>>8,f=q&255u;int32_t a=line[(write-d)&mask],b=line[(write-d-1u)&mask];return a+(((b-a)*(int32_t)f)>>8);}
static int32_t fx_tape_read(uint32_t q)
{uint32_t d=q>>8,f=q&255u,at=tape_write>=d?tape_write-d:tape_write+TAPE_LEN-d;
 int32_t a=tape_line[at],b=tape_line[at?at-1u:TAPE_LEN-1u];return a+(((b-a)*(int32_t)f)>>8);}
static uint32_t fx_tape_ms(void)
{
 if(!wm.tape_sync)return 40u+(uint32_t)wm.tape_time*960u/127u;
 uint32_t ms=60000u/wm.bpm;
 if(wm.tape_sync==1)ms/=2;else if(wm.tape_sync==2)ms=ms*3/4;
 return ms>1000u?1000u:ms;
}
static void fx_reset(void){fx_clear_part=fx_clear_at=0;fx_clearing=1;}
static void fx_clear_chunk(void)
{
 int16_t *lines[8]={delay_line,tape_line,chorus_line,verb_a,verb_b,verb_c,verb_d,&verb_ap[0][0]};
 const uint32_t sizes[8]={16384,TAPE_LEN,1024,887,1091,1153,965,512};
 uint32_t budget=768;
 while(budget&&fx_clear_part<8){uint32_t n=sizes[fx_clear_part]-fx_clear_at;if(n>budget)n=budget;
  memset(lines[fx_clear_part]+fx_clear_at,0,n*2);fx_clear_at+=n;budget-=n;
  if(fx_clear_at==sizes[fx_clear_part]){fx_clear_at=0;fx_clear_part++;}}
 if(fx_clear_part==8){
  delay_write=delay_frames_smooth=fx_phase=tape_phase=tape_write=tape_delay_q=chorus_write=chorus_phase=fx_frames=0;
  wow_phase=flutter_phase=phaser_phase=crush_count=0;
  fx_input=tape_tone=tape_dc=tape_l=tape_r=tape_old_l=tape_old_r=fold_lp=crush_hold=0;
  wet_l=wet_r=wet_old_l=wet_old_r=0;
  verb_restart(wm.reverb_type);verb_gain=256u;
  memset(mix_smooth,0,sizeof mix_smooth);memset(phaser_s,0,sizeof phaser_s);memset(phaser_y,0,sizeof phaser_y);
  memset(phaser_a,0,sizeof phaser_a);phaser_awake=0;fx_clearing=0;
 }
}
static int32_t fx_fold(int32_t x)
{
 int32_t z=(int32_t)(((int64_t)x*(256+wm.fold_drive*20))>>8);
 uint32_t t=((uint32_t)z+32768u)&131071u;
 int32_t y=t<65536u?(int32_t)t-32768:98303-(int32_t)t;
 int32_t k=800+wm.fold_tone*250;fold_lp+=(int32_t)(((int64_t)(y-fold_lp)*k)>>15);
 return fx_blend(x,fold_lp,mix_smooth[6]);
}
static int32_t fx_crush(int32_t x)
{
 uint32_t hold=FX_HOLD[wm.crush_rate];
 if(!crush_count){int32_t v=ws_clamp(x,-32767,32767);uint32_t step=1u<<(16u-wm.crush_bits),a=(uint32_t)(v<0?-v:v);
  int32_t y=(int32_t)((a/step)*step+step/2u);crush_hold=v? (v<0?-y:y):0;}
 if(++crush_count>=hold)crush_count=0;
 return fx_blend(x,crush_hold,mix_smooth[3]);
}
/* Canonical first-order allpasses, two independent six-stage sweeps. */
static int32_t fx_phaser(int32_t x,uint32_t ch)
{
 int32_t y=x/2+phaser_y[ch]/4,a=phaser_a[ch];
 for(uint32_t k=0;k<6;k++){
  int32_t next=(int32_t)(((int64_t)a*y)>>15)+phaser_s[ch][k];
  phaser_s[ch][k]=ws_clamp(y-(int32_t)(((int64_t)a*next)>>15),-131072,131072);y=ws_clamp(next,-65535,65535);
 }
 phaser_y[ch]=y;
 return fx_blend(x,x/2+y/2,mix_smooth[5]);
}
static int32_t verb_history(uint32_t samples)
{
 return verb_age>=samples?verb_pre[(verb_pre_pos-samples)&VERB_PRE_MASK]:0;
}
static int32_t verb_modulated_tap(const int16_t *line,uint32_t pos,uint32_t length,uint32_t phase,uint32_t depth)
{
 if(verb_age<length)return 0;
 uint32_t triangle=phase>>20;
 if(triangle>=2048u)triangle=4095u-triangle;
 uint32_t q=triangle*depth>>8; /* up to depth*8 Q8 samples */
 uint32_t at=pos+(q>>8);
 if(at>=length)at-=length;
 uint32_t next=at+1u==length?0u:at+1u;
 return line[at]+(((line[next]-line[at])*(int32_t)(q&255u))>>8);
}
/* Every line decays at the same rate per second: a longer line loses more per
 * pass. First-order g^(L/Lavg) keeps the tail even instead of ringing on its
 * shortest line. Q12, recomputed once per block from the Q7 network gain. */
static void verb_line_gains(uint8_t type,int32_t feedback)
{
 if(type==RV_AMBIENT)return; /* Ambient keeps its own Q15 loss path. */
 const reverb_profile_t *p=&VERB_PROFILES[type];
 uint32_t avg=((uint32_t)p->lines[0]+p->lines[1]+p->lines[2]+p->lines[3])/4u;
 int32_t loss=(128-feedback)*32;
 for(uint32_t k=0;k<4;k++){
  int32_t lk=(int32_t)((uint32_t)loss*p->lines[k]/(avg?avg:1u));
  verb_fb[k]=ws_clamp(4096-lk,0,4064);
 }
}
static int32_t verb_feedback(uint8_t type)
{
 const reverb_profile_t *p=&VERB_PROFILES[type];
 if(type==RV_AMBIENT){
  /* Spread the shorter settings out and approach unity gradually. */
  int32_t remaining=127-wm.reverb_size;
  return 32768-(80+remaining*remaining*870/(127*127));
 }
 return p->feedback+wm.reverb_size*p->range/127;
}
static void verb_process(int32_t input,int32_t feedback,int32_t damp,int32_t *left,int32_t *right)
{
 /* Fade the old geometry down before changing heads. Age checks silence
  * stale RAM until each line refills, avoiding a large clear in the ISR. */
 if(verb_type!=wm.reverb_type){
  if(verb_gain>=2u)verb_gain-=2u;
  if(!verb_gain)verb_restart(wm.reverb_type);
 }else if(verb_gain<256u)verb_gain+=2u;
 const reverb_profile_t *p=&VERB_PROFILES[verb_type];
 uint8_t ambient=verb_type==RV_AMBIENT;
 if(ambient&&!(wm.fx_on&4u)&&!mix_smooth[2]){
  /* Let the wet fade finish, then invalidate the diffusion and delay
   * history so a quiet remnant cannot return on the next enable. */
  if(verb_age)verb_restart(verb_type);
  *left=*right=0;return;
 }
 int32_t vin=input,early_l=0,early_r=0;
 if(verb_type!=RV_ROOM){
  verb_pre[verb_pre_pos]=fx_bound(input);
  vin=verb_history(p->pre);
  if(p->early[0]){early_l=verb_history(p->early[0]);early_r=verb_history(p->early[1]);}
  verb_pre_pos=(verb_pre_pos+1u)&VERB_PRE_MASK;
  if(verb_type==RV_SPRING||verb_type==RV_AMBIENT){
   verb_input_tone=fx_slew(verb_input_tone,vin,3u);
   vin=verb_type==RV_SPRING?(vin-verb_input_tone)*2:verb_input_tone;
  }
 }
 for(uint32_t k=0;k<2;k++){
  uint32_t len=p->diffusers[k];
  int32_t delayed=verb_age>=len?verb_ap[k][ap_pos[k]]:0;
  int32_t y=delayed-(vin*p->diffusion)/128;
  verb_ap[k][ap_pos[k]]=fx_bound(vin+(y*p->diffusion)/128);vin=y;
  if(++ap_pos[k]==len)ap_pos[k]=0;
 }
 int16_t *lines[4]={verb_a,verb_b,verb_c,verb_d};
 int32_t tap[4],filtered[4],sum=0;
 int32_t ambient_damp=1+((wm.reverb_damp*3)>>4);
 for(uint32_t k=0;k<4;k++){
  uint32_t pos=verb_pos[k];
  tap[k]=0;
  if(verb_age>=p->lines[k]){
   tap[k]=lines[k][pos];
   /* Lines 2 and 4 wander a few samples against each other, so the
    * modes keep moving instead of ringing as a fixed comb. */
   if(VERB_MOD[verb_type]&&(k&1u))
    tap[k]=verb_modulated_tap(lines[k],pos,p->lines[k],motion_phase+(k>>1)*0x40000000u,VERB_MOD[verb_type]);
   if(ambient){
    uint32_t frac_at=AMB_OFFSET[k]+pos,shift=(frac_at&1u)*4u;
    tap[k]=tap[k]*16+((verb_frac[frac_at>>1]>>shift)&15);
   }
  }
  int32_t error=tap[k]-verb_tone[k],scaled=error*damp;
  if(verb_type==RV_ROOM)verb_tone[k]+=scaled>>8;
  else if(error){int32_t step=scaled/256;verb_tone[k]+=step?step:error>0?1:-1;}
  /* A light low-pass blend keeps the wash dark without stripping the
   * mids at every trip around the long-decay network. */
  filtered[k]=ambient?tap[k]-(tap[k]-verb_tone[k])*ambient_damp/256:verb_tone[k];
  if(VERB_LOWCUT[verb_type]){
   /* Gentle low cut inside the loop: long tails stay clear, never muddy. */
   /* Q8 state rounded toward zero: the correction itself falls to zero in
    * silence, so the low cut can never sustain a limit cycle. */
   verb_low[k]+=(filtered[k]*256-verb_low[k])/(1<<VERB_LOWCUT[verb_type]);
   filtered[k]-=verb_low[k]/256;
  }
  sum+=filtered[k];
 }
 for(uint32_t k=0;k<4;k++){
  int32_t reflected=filtered[k]-(ambient?sum/2:sum>>1);
  if(ambient){
   uint32_t frac_at=AMB_OFFSET[k]+verb_pos[k],shift=(frac_at&1u)*4u;
   int32_t mag=wn_abs(reflected);
   /* Use the small loss term and round toward silence. Products fit in
    * signed 32 bits at the full Q4 delay range. */
   int32_t loss=(mag*(32768-feedback)+32767)>>15;
   int32_t fed=reflected<0?reflected+loss:reflected-loss;
   int32_t value=(wm.fx_on&4u)?ws_clamp(vin*6+fed,-524272,524287):0;
   lines[k][verb_pos[k]]=(int16_t)(value>>4);
   uint8_t *frac=&verb_frac[frac_at>>1];
   *frac=(uint8_t)((*frac&~(15u<<shift))|(((uint32_t)value&15u)<<shift));
  }else{
   int32_t injection=(verb_type==RV_SPRING||verb_type==RV_DEEP_SPACE)&&(k&1u)?-vin:vin;
   int32_t fed=(reflected*verb_fb[k])/4096; /* rounds toward silence */
   lines[k][verb_pos[k]]=fx_bound((wm.fx_on&4u)?injection+fed:0);
  }
  if(++verb_pos[k]==p->lines[k])verb_pos[k]=0;
 }
 if(verb_age<2048u)verb_age++;
 int32_t l=tap[0]+tap[2],r=tap[1]+tap[3];
 if(ambient){l/=16;r/=16;}
 else if(verb_type!=RV_ROOM&&verb_age>=2048u){
  /* Decorrelated output taps from inside the other side's lines thicken the
   * stereo image without adding feedback paths. */
  uint32_t a=verb_pos[1]+p->lines[1]/3u,b=verb_pos[3]+p->lines[3]*2u/3u;
  uint32_t c=verb_pos[0]+p->lines[0]/2u,d=verb_pos[2]+p->lines[2]/5u;
  if(a>=p->lines[1])a-=p->lines[1];if(b>=p->lines[3])b-=p->lines[3];
  if(c>=p->lines[0])c-=p->lines[0];if(d>=p->lines[2])d-=p->lines[2];
  int32_t xl=(verb_b[a]-verb_d[b])/2,xr=(verb_a[c]-verb_c[d])/2;
  l=(l*3+xl*2)/4;r=(r*3+xr*2)/4;
 }
 if(verb_type!=RV_ROOM){
  int32_t mid=(l+r)/2,side=((l-r)*p->width)/256;
  /* Parallel early reflections define the enclosure without destabilizing feedback. */
  int32_t early=verb_type==RV_CAVERN?72:verb_type==RV_SPRING?92:36;
  l=(mid+side)*3/4+(early_l*early)/128;
  r=(mid-side)*3/4+(early_r*early)/128;
 }
 *left=(l*verb_gain)/256;*right=(r*verb_gain)/256;
}
/* Both wet clocks advance in units of 100 Hz. For all 441 remainders
 * at 44.1 kHz this is exactly floor(phase * 256 / 441). */
_Static_assert(FS==44100u,"FX interpolation assumes a 44.1 kHz audio clock");
static int32_t fx_fraction(uint32_t phase){return (int32_t)((phase*9511u)>>14);}
static void fx_process(int32_t *out,uint32_t n)
{
 if(fx_clearing){fx_clear_chunk();for(uint32_t i=0;i<n;i++)out[i*2+1]=out[i*2];return;}
 uint32_t target=((step_samples*160u)/441u)*(wm.delay_div+1u);if(target>16000)target=16000;
 if(!delay_frames_smooth)delay_frames_smooth=target;
 uint32_t tape_target=fx_tape_ms()*12u*256u;if(!tape_delay_q)tape_delay_q=tape_target;
 int32_t targets[7]={(wm.fx_on&1)?fx_mix(wm.chorus_mix):0,(wm.fx_on&2)?fx_mix(wm.delay_mix):0,(wm.fx_on&4)?fx_mix(wm.reverb_mix):0,
  (wm.fx_new_on&1)?fx_mix(wm.crush_mix):0,(wm.fx_new_on&2)?fx_mix(wm.tape_mix):0,(wm.fx_new_on&4)?fx_mix(wm.phaser_mix):0,(wm.fx_new_on&8)?fx_mix(wm.fold_mix):0};
 uint8_t mix_moving=0;for(uint32_t k=0;k<7;k++)mix_moving|=mix_smooth[k]!=targets[k];
 int32_t feedback=wm.delay_feedback*86/127;
 const reverb_profile_t *room=&VERB_PROFILES[verb_type],*next=&VERB_PROFILES[wm.reverb_type];
 int32_t room_fb=verb_feedback(verb_type);
 int32_t room_damp=room->damp-(wm.reverb_damp>>1);
 int32_t next_fb=verb_feedback(wm.reverb_type);
 int32_t next_damp=next->damp-(wm.reverb_damp>>1);
 verb_line_gains(verb_type,verb_type==wm.reverb_type?next_fb:room_fb);
 for(uint32_t i=0;i<n;i++){
  if(mix_moving){mix_moving=0;for(uint32_t k=0;k<7;k++){mix_smooth[k]=fx_slew(mix_smooth[k],targets[k],7);mix_moving|=mix_smooth[k]!=targets[k];}}
  int32_t dry=out[i*2];
  if(mix_smooth[6])dry=fx_fold(dry);
  if(mix_smooth[3])dry=fx_crush(dry);else crush_count=0;
  if(!(fx_frames++&31u)&&mix_smooth[5])for(uint32_t c=0;c<2;c++){
   int32_t lfo=(wn_sine_i(phaser_phase+c*0x40000000u)+32768)/2;
   phaser_a[c]=-31130+(int32_t)(((int64_t)lfo*(3000+wm.phaser_depth*205))>>15);
  }
  phaser_phase+=WN_FX_INC[wm.phaser_rate];
  int32_t left=dry,right=dry;
  if(mix_smooth[5]){phaser_awake=1;left=fx_phaser(dry,0);right=fx_phaser(dry,1);}
  else if(phaser_awake){memset(phaser_s,0,sizeof phaser_s);phaser_y[0]=phaser_y[1]=0;phaser_awake=0;}
  /* Voice spread arrives as a side signal in the right slot. */
  int32_t side=out[i*2+1];left+=side;right-=side;
  int32_t send=(left+right)/2;
  fx_input+=(send-fx_input)/2;
  /* Below halfway this is the original additive send. Above halfway,
   * cross out the direct voice so full Mix exposes the space alone. */
  int32_t verb_boost=mix_smooth[2]-16384;
  if(verb_boost>0){
   int32_t dry_gain=(16384-verb_boost)>>6;
   left=left*dry_gain/256;right=right*dry_gain/256;
  }
  chorus_line[chorus_write]=fx_bound(send);int32_t cl=0,cr=0;
  if(mix_smooth[0]){
   for(uint32_t k=0;k<3;k++){
    int32_t motion=(wn_sine_i(chorus_phase+k*0x55555555u)*wm.chorus_depth*3)>>7;
    uint32_t q=(uint32_t)((400+(int32_t)k*79)*256+motion);
    cl+=fx_read(chorus_line,CHORUS_MASK,chorus_write,q);
    cr+=fx_read(chorus_line,CHORUS_MASK,chorus_write,q+73u*256u);
   }
   left+=(cl*(mix_smooth[0]>>8))/(3*256);
   right+=(cr*(mix_smooth[0]>>8))/(3*256);
  }
  chorus_write=(chorus_write+1u)&CHORUS_MASK;chorus_phase+=WN_FX_INC[wm.chorus_rate];
  wow_phase+=28244u;flutter_phase+=594096u;
  tape_phase+=120u;
  if(tape_phase>=441u){
   tape_phase-=441u;tape_old_l=tape_l;tape_old_r=tape_r;
   tape_delay_q=(uint32_t)fx_slew((int32_t)tape_delay_q,(int32_t)tape_target,12);
   if(mix_smooth[4]){
    int32_t wow=(wn_sine_i(wow_phase)*wm.tape_wow)>>12,flutter=(wn_sine_i(flutter_phase)*wm.tape_wow)>>16;
    uint32_t q=(uint32_t)ws_clamp((int32_t)tape_delay_q+wow+flutter,24*256,12100*256);
    int32_t last=fx_tape_read(q),sum=last,side=fx_tape_read(q+19*256);
    if(wm.tape_heads){sum+=fx_tape_read(q*2/3);side+=fx_tape_read(q*2/3+13*256);}
    if(wm.tape_heads==2){sum+=fx_tape_read(q/3);side+=fx_tape_read(q/3+7*256);}
    int32_t heads=wm.tape_heads+1;tape_l=sum/heads;tape_r=side/heads;
    tape_tone+=(int32_t)(((int64_t)(last-tape_tone)*(1200+wm.tape_tone*225))>>15);
    tape_dc+=(tape_tone-tape_dc)/512;
    int32_t fb=(wm.fx_new_on&2)?wm.tape_feedback*235:0;
    tape_line[tape_write]=(int16_t)wn_softclip(fx_input+(int32_t)(((int64_t)(tape_tone-tape_dc)*fb)>>15));
   }else{
    /* Bypassed: maintain a fresh input-history ring, skip six fractional
     * reads, waveform lookups, feedback filter and division. */
    tape_l=tape_r=tape_tone=tape_dc=0;
    tape_line[tape_write]=fx_bound(fx_input);
   }
   if(++tape_write==TAPE_LEN)tape_write=0;
  }
  int32_t tl=0,tr=0;
  if(mix_smooth[4]>=256){
   int32_t tf=fx_fraction(tape_phase);
   tl=(tape_old_l+(((tape_l-tape_old_l)*tf)>>8))*(mix_smooth[4]>>8)/256;
   tr=(tape_old_r+(((tape_r-tape_old_r)*tf)>>8))*(mix_smooth[4]>>8)/256;
   left+=tl;right+=tr;
  }
  fx_phase+=160u;
  if(fx_phase>=441u){fx_phase-=441u;wet_old_l=wet_l;wet_old_r=wet_r;
   if(delay_frames_smooth<target)delay_frames_smooth++;else if(delay_frames_smooth>target)delay_frames_smooth--;
   int32_t echo=delay_line[(delay_write-delay_frames_smooth)&DELAY_MASK],echo_r=delay_line[(delay_write-delay_frames_smooth-191u)&DELAY_MASK];
   delay_line[delay_write]=fx_bound(fx_input+((echo*((wm.fx_on&2)?feedback:0))>>7));delay_write=(delay_write+1)&DELAY_MASK;
   int32_t vin=(fx_input+(tl+tr)/2+echo*(mix_smooth[1]>>8)/512)/2;
   int32_t rev_l,rev_r;
   verb_process(vin,verb_type==wm.reverb_type?next_fb:room_fb,
     verb_type==wm.reverb_type?next_damp:room_damp,&rev_l,&rev_r);
   /* Up to 3x return gain, outside the feedback network and before the
    * stereo output limiter. The gain stays outside the decay loop. */
   int32_t verb_wet=(mix_smooth[2]+(verb_boost>0?verb_boost*4:0))>>8;
   wet_l=echo*(mix_smooth[1]>>8)/256+rev_l*verb_wet/256;
   wet_r=echo_r*(mix_smooth[1]>>8)/256+rev_r*verb_wet/256;
   motion_phase+=67110u;
  }
  int32_t frac=fx_fraction(fx_phase);
  out[i*2]=fx_limit(left+wet_old_l+(((wet_l-wet_old_l)*frac)>>8));
  out[i*2+1]=fx_limit(right+wet_old_r+(((wet_r-wet_old_r)*frac)>>8));
 }
}
