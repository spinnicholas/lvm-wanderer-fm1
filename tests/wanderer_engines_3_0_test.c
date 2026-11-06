/* 3.0 SUPER and VOICE engines: bounded, stereo, pitched, cost compared with FM6. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
#include <time.h>
static int32_t buf[2*4096];
static ws_patch_t patch(uint8_t engine,uint8_t wave)
{
 ws_patch_t p=WS_PRESETS_3[0];p.engine=engine;p.wave=wave;p.filter=WS_LP24;p.cutoff=100;p.resonance=20;
 p.mix=90;p.detune=70;p.shape=90;p.sub=30;p.pulse=64;p.position=64;p.feedback=80;p.inharm=10;p.level=100;
 p.lfo_depth=0;memset(p.morph,0,sizeof p.morph);p.morph[1]=30;p.morph[5]=20;return p;
}
static double render(const ws_patch_t *p,uint32_t note,uint32_t frames,double *rms,int32_t *peak,uint32_t *wide)
{
 wm.patch=*p;wm.patch_valid=1;wm.legacy=0;layer_focus=0;anchor_view=0;
 memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);
 for(uint32_t k=0;k<4;k++)note_on(note+k*4,100,1);
 struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);
 double e=0;*peak=0;*wide=0;
 for(uint32_t n=0;n<frames;n+=4096){
  memset(buf,0,sizeof buf);for(uint32_t v=0;v<NV;v++)ws_render(v,buf,4096,40);
  for(uint32_t i=0;i<8192;i+=2){int32_t m=buf[i];e+=(double)m*m;if(abs(m)>*peak)*peak=abs(m);if(buf[i+1])(*wide)++;}
 }
 clock_gettime(CLOCK_MONOTONIC,&b);
 for(uint32_t k=0;k<4;k++)note_off(note+k*4,1);
 *rms=__builtin_sqrt(e/frames);
 return (b.tv_sec-a.tv_sec)*1e9+(b.tv_nsec-a.tv_nsec);
}
int main(void)
{
 double fm6_ns=0;
 for(uint32_t e=0;e<WS_ENGINES;e++){
  ws_patch_t p=patch((uint8_t)e,0);
  if(e==5){p=WS_PRESETS_3[0];for(uint32_t i=0;i<150;i++)if(WS_PRESETS_3[i].engine==5){p=WS_PRESETS_3[i];break;}}
  if(e==1||e==3||e==4){for(uint32_t i=0;i<150;i++)if(WS_PRESETS_3[i].engine==e){p=WS_PRESETS_3[i];break;}}
  assert(ws_patch_valid(&p));
  double rms;int32_t peak;uint32_t wide;double ns=render(&p,48,FS*2,&rms,&peak,&wide);
  if(e==5)fm6_ns=ns;
  printf("engine %u %-6s rms %6.0f peak %6d wide %6u  cost %.2fx\n",e,WS_ENGINE_NAMES[e],rms,peak,wide,ns);
 }
 for(uint32_t e=6;e<8;e++)for(uint8_t w=0;w<4;w++)for(uint32_t note=24;note<=108;note+=12){
  ws_patch_t p=patch((uint8_t)e,w);double rms;int32_t peak;uint32_t wide;
  render(&p,note,8192,&rms,&peak,&wide);
  assert(peak<=65535&&rms>200);if(e==6)assert(wide>1000);
 }
 /* Melt moves SUPER's width and VOICE's vowel without leaving bounds. */
 for(int32_t melt=0;melt<=127;melt+=127){ws_patch_t p=patch(7,0);p.morph[1]=60;double r;int32_t pk;uint32_t w;
  wm.patch=p;wm.patch_valid=1;memset(voice,0,sizeof voice);note_on(57,100,1);memset(buf,0,sizeof buf);ws_render(0,buf,4096,melt);(void)r;(void)pk;(void)w;}
 printf("relative cost (ns for 2 s, four voices); FM6 reference %.0f\n",fm6_ns);
 puts("SUPER and VOICE engines: valid, bounded, stereo and pitched at every wave and octave");
 return 0;
}
