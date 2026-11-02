#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static int16_t reference[FS/5*2];
static void fresh(void)
{
 running=recording=anchor_view=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;
 memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);cues_enabled=cue_detail=1;cue_level=64;
 vol=127;master_smooth=127*256;cue_boundary_valid=0;fm1_ms=1000;
}
static uint32_t render(uint32_t block,int16_t *samples,uint32_t count)
{
 uint32_t pos=0,peak=0;
 while(pos<count){uint32_t n=count-pos;if(n>block)n=block;int32_t out[778]={0};app_audio(out,n);
  for(uint32_t i=0;i<n*2;i++){assert(abs(out[i])<=32767);if((uint32_t)abs(out[i])>peak)peak=abs(out[i]);if(samples)samples[pos*2+i]=(int16_t)out[i];}pos+=n;
 }
 return peak;
}
int main(void)
{
 fresh();cue_toggle(1);assert(cue_step==tone_step(64)&&cue_follow_step==tone_step(76)&&cue_stage==1);
 uint32_t total=0;while(cue_left){int32_t v=cue_sample();assert(abs(v)<=1024);total++;assert(total<FS/5);}
 assert(total==2*(FS/32)+FS/100&&!cue_stage);
 cue_toggle(0);assert(cue_step==tone_step(76)&&cue_follow_step==tone_step(64));cue_stop();
 cue_boundary(1,0,0,127);assert(cue_step==tone_step(60)&&cue_follow_step==tone_step(48));cue_stop();
 cue_boundary(63,64,0,127);assert(cue_step==tone_step(72)&&!cue_stage);cue_stop();
 cue_boundary(126,127,0,127);assert(cue_step==tone_step(72)&&cue_follow_step==tone_step(84));cue_stop();
 cue_boundary(127,127,0,127);assert(!cue_left);
 fm1_ms+=200;cue_boundary(63,64,0,127);cue_stop();cue_boundary(63,64,0,127);assert(!cue_left);
 cue(80);uint32_t kept=cue_step;cue_boundary(1,0,0,127);assert(cue_step==kept&&cue_priority==2);
 cue_stop();cue_detail=0;cue_boundary(126,127,0,127);assert(!cue_left);cue_toggle(1);assert(cue_step==tone_step(76)&&!cue_stage);
 cues_enabled=0;assert(!cue_sample()&&!cue_left);cues_enabled=1;cue_level=0;cue_toggle(1);assert(!cue_left);
 puts("up/down toggle motifs, minimum/midpoint/maximum, sample-counted gaps, rate limiting, no clamped repeats, action priority and mute pass");
 fresh();cue_toggle(1);render(256,reference,FS/5);
 const uint32_t blocks[]={1,17,64,255,389};
 for(uint32_t b=0;b<5;b++){
  fresh();cue_toggle(1);uint32_t pos=0;
  while(pos<FS/5){uint32_t n=FS/5-pos;if(n>blocks[b])n=blocks[b];int32_t out[778]={0};app_audio(out,n);
   for(uint32_t i=0;i<n*2;i++)assert(out[i]==reference[pos*2+i]);pos+=n;}
 }
 fresh();cue_level=127;cue_toggle(1);uint32_t loud=render(256,0,FS/5);
 fresh();cue_level=32;cue_toggle(1);uint32_t soft=render(256,0,FS/5);assert(loud>soft*3&&soft>0);
 fresh();scope_set_open(1);cue_toggle(1);assert(render(256,0,FS/5)>0);scope_snapshot();assert(!scope_peak);scope_set_open(0);
 puts("identical audio across six callback sizes, independent cue level, bounded output and clean scope trace pass");
 /* Print silence while loud cues are audible: only the post-capture output may contain them. */
 fresh();wm.bpm=180;step_samples=FS*15/wm.bpm;wp_armed=1;running=1;step_index=step_clock=step_primed=0;
 uint32_t frames=step_samples*16+256,at=0,heard=0;
 while(at<frames){if(at%(256*10)==0)cue_toggle((at/(256*10))&1);uint32_t n=frames-at;if(n>256)n=256;heard|=render(256,0,n);at+=n;}
 assert(heard&&wp_active&&wp_has_anchor);
 for(uint32_t i=0;i<wp_length;i++)assert(wp_sample(0,i)==0);
 assert(!capture_count());for(uint32_t v=0;v<NV;v++)assert(!voice[v].gate);
 puts("audible cues stay out of the printed Anchor, capture events and four synth voices");
 fresh();wm.fx_on=7;wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;wm.delay_feedback=wm.reverb_size=127;
 wm.layer_enabled=1;wm.decay=127;wm.arp_on=1;wm.arp_gate=100;
 for(uint32_t i=0;i<16;i++){wm.memory.e[i]=(wm_event_t){48+i%12,127,512};wm.layer.e[i]=(wm_event_t){60+i%12,127,768};}
 assert(wm_catch());uint32_t anchor=0;const uint8_t *bytes=(const uint8_t*)&wm.anchor;
 for(uint32_t i=0;i<sizeof wm.anchor;i++)anchor=anchor*33u+bytes[i];
 performance_on(48,127,1);performance_on(55,127,1);performance_on(60,127,1);running=1;
 uint32_t stress=FS*30u,serial=0;
 while(stress){
  if(serial++%6u==0){cue_level=((serial/6u)&1u)?127:32;cue_toggle((serial/6u)&1u);}
  uint32_t n=stress>256?256:stress;render(256,0,n);stress-=n;
 }
 uint32_t after=0;for(uint32_t i=0;i<sizeof wm.anchor;i++)after=after*33u+bytes[i];assert(after==anchor);
 panic();assert(!cue_left&&!cue_stage&&!running&&!recording);
 puts("30 seconds: dense cue retriggers with both layers, arp, Scatter and maximum effects remain bounded; Anchor and panic pass");
 /* A short production-DSP preview, in order: minimum, midpoint, maximum, off, on. */
 fresh();FILE *f=fopen("build/1.4-RC6-Cues.raw","wb");assert(f);
 for(uint32_t event=0;event<5;event++){
  cue_stop();cue_boundary_valid=0;fm1_ms+=200;
  if(event==0)cue_boundary(1,0,0,127);if(event==1)cue_boundary(63,64,0,127);if(event==2)cue_boundary(126,127,0,127);
  if(event>=3)cue_toggle(event==4);
  render(256,reference,FS/5);assert(fwrite(reference,2,FS/5*2,f)==FS/5*2);
 }
 fclose(f);
}
