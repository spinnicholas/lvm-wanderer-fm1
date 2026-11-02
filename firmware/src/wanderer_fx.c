/* SPDX-License-Identifier: GPL-3.0-only
 * Compact 16 kHz wet bus, 44.1 kHz dry path. Delay taps share chorus storage.
 * 40 KiB total; bounded clearing; no allocations, floating point or ISR divides.
 */
#define DELAY_MASK 16383u
static int16_t delay_line[16384] __attribute__((section(".pool")));
static int16_t verb_a[887] __attribute__((section(".pool")));
static int16_t verb_b[1091] __attribute__((section(".pool")));
static int16_t verb_c[1153] __attribute__((section(".pool")));
static int16_t verb_d[965] __attribute__((section(".pool")));
static uint32_t delay_write,delay_frames_smooth,fx_phase;
static uint16_t verb_pos[4];static int32_t verb_tone[4],mix_smooth[3];
static int32_t fx_input,wet_l,wet_r,wet_old_l,wet_old_r;
static volatile uint8_t fx_clearing;static uint32_t fx_clear_part,fx_clear_at;
static int16_t fx_bound(int32_t x){return (int16_t)(x>32767?32767:x< -32767?-32767:x);}
static int32_t fx_limit(int32_t x)
{if(x>29000)x=29000+((x-29000)>>3);if(x< -29000)x=-29000+((x+29000)>>3);return fx_bound(x);}
static void fx_reset(void){fx_clear_part=fx_clear_at=0;fx_clearing=1;}
static void fx_clear_chunk(void)
{
 int16_t *lines[5]={delay_line,verb_a,verb_b,verb_c,verb_d};const uint32_t sizes[5]={16384,887,1091,1153,965};
 uint32_t budget=512;
 while(budget&&fx_clear_part<5){uint32_t n=sizes[fx_clear_part]-fx_clear_at;if(n>budget)n=budget;
  memset(lines[fx_clear_part]+fx_clear_at,0,n*2);fx_clear_at+=n;budget-=n;
  if(fx_clear_at==sizes[fx_clear_part]){fx_clear_at=0;fx_clear_part++;}}
 if(fx_clear_part==5){delay_write=delay_frames_smooth=fx_phase=0;fx_input=wet_l=wet_r=wet_old_l=wet_old_r=0;
  memset(verb_pos,0,sizeof verb_pos);memset(verb_tone,0,sizeof verb_tone);memset(mix_smooth,0,sizeof mix_smooth);fx_clearing=0;}
}
static void fx_process(int32_t *out,uint32_t n)
{
 if(fx_clearing){fx_clear_chunk();for(uint32_t i=0;i<n;i++)out[i*2+1]=out[i*2];return;}
 uint32_t target=((step_samples*160u)/441u)*(wm.delay_div+1u);if(target>16000)target=16000;
 if(!delay_frames_smooth)delay_frames_smooth=target;
 int32_t targets[3]={(wm.fx_on&1)?wm.chorus_mix*256:0,(wm.fx_on&2)?wm.delay_mix*256:0,(wm.fx_on&4)?wm.reverb_mix*256:0};
 int32_t feedback=wm.delay_feedback*86/127,room_fb=70+wm.reverb_size*42/127;
 for(uint32_t i=0;i<n;i++){
  int32_t dry=out[i*2];fx_input+=(dry-fx_input)>>1;
  for(uint32_t k=0;k<3;k++)mix_smooth[k]+=(targets[k]-mix_smooth[k])>>7;
  fx_phase+=16000;
  if(fx_phase>=FS){fx_phase-=FS;wet_old_l=wet_l;wet_old_r=wet_r;
   if(delay_frames_smooth<target)delay_frames_smooth++;else if(delay_frames_smooth>target)delay_frames_smooth--;
   uint32_t dt=delay_write;
   int32_t echo=delay_line[(dt-delay_frames_smooth)&DELAY_MASK];
   int32_t echo_r=delay_line[(dt-delay_frames_smooth-191u)&DELAY_MASK];
   uint32_t tri=(motion_phase>>21)&2047u;int32_t sweep=((int32_t)(tri<1024?tri:2047-tri)-512)>>4;
   int32_t cl=delay_line[(dt-255u-(uint32_t)sweep)&DELAY_MASK],cr=delay_line[(dt-329u+(uint32_t)sweep)&DELAY_MASK];
   delay_line[dt]=fx_bound(fx_input+((echo*((wm.fx_on&2)?feedback:0))>>7));delay_write=(dt+1)&DELAY_MASK;
   int16_t *lines[4]={verb_a,verb_b,verb_c,verb_d};const uint16_t lens[4]={887,1091,1153,965};
   int32_t tap[4],sum=0;
   for(uint32_t k=0;k<4;k++){tap[k]=lines[k][verb_pos[k]];verb_tone[k]+=((tap[k]-verb_tone[k])*(160-(wm.reverb_damp>>1)))>>8;sum+=verb_tone[k];}
   for(uint32_t k=0;k<4;k++){int32_t reflected=verb_tone[k]-(sum>>1);lines[k][verb_pos[k]]=fx_bound((wm.fx_on&4)?(fx_input>>1)+((reflected*room_fb)>>7):0);
    if(++verb_pos[k]==lens[k])verb_pos[k]=0;}
   wet_l=((cl*(mix_smooth[0]>>8))>>8)+((echo*(mix_smooth[1]>>8))>>8)+(((tap[0]+tap[2])*(mix_smooth[2]>>8))>>8);
   wet_r=((cr*(mix_smooth[0]>>8))>>8)+((echo_r*(mix_smooth[1]>>8))>>8)+(((tap[1]+tap[3])*(mix_smooth[2]>>8))>>8);
   motion_phase+=67110u;
  }
  int32_t frac=(int32_t)((fx_phase*256u)/FS);
  out[i*2]=fx_limit(dry+wet_old_l+(((wet_l-wet_old_l)*frac)>>8));
  out[i*2+1]=fx_limit(dry+wet_old_r+(((wet_r-wet_old_r)*frac)>>8));
 }
}
