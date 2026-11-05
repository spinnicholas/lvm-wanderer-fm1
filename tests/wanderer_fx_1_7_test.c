/* Signal properties: bypass, quantization, diffusion, tails, reset and timing. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static int32_t pcm[512];
static void clear(void)
{motion_phase=0;fx_reset();uint32_t chunks=0;while(fx_clearing){fx_clear_chunk();assert(++chunks<80);}}
static void settle(void)
{for(uint32_t b=0;b<64;b++){memset(pcm,0,sizeof pcm);fx_process(pcm,256);}}
static uint32_t hash(uint32_t chunk)
{
 wm=(wm_state_t)WM_DEFAULTS;wm.fx_on=7;wm.fx_new_on=15;wm.crush_mix=wm.fold_mix=80;wm.tape_sync=0;wm.tape_time=25;
 clear();uint32_t h=0,pos=0;
 while(pos<FS*2){uint32_t n=FS*2-pos;if(n>chunk)n=chunk;
  for(uint32_t i=0;i<n;i++){pcm[i*2]=wn_sine_i((pos+i)*0x01900000u)/4;pcm[i*2+1]=0;} /* platform contract: out arrives zeroed */
  fx_process(pcm,n);for(uint32_t i=0;i<n*2;i++){assert(abs(pcm[i])<32767);h=h*33u+(uint32_t)pcm[i];}pos+=n;
 }return h;
}
static uint64_t tape_early(uint8_t heads)
{
 wm=(wm_state_t)WM_DEFAULTS;wm.fx_on=0;wm.fx_new_on=2;wm.tape_sync=0;wm.tape_time=20;wm.tape_wow=0;wm.tape_feedback=0;wm.tape_mix=127;wm.tape_heads=heads;
 clear();settle();uint64_t early=0;
 for(uint32_t f=0;f<FS/8;f++){int32_t out[2]={f==0?12000:0,0};fx_process(out,1);if(f>FS/50)early+=(int64_t)out[0]*out[0];}
 return early;
}
int main(void)
{
 for(uint32_t phase=0;phase<441u;phase++)assert(fx_fraction(phase)==(int32_t)(phase*256u/441u));
 wm=(wm_state_t)WM_DEFAULTS;wm.fx_on=0;wm.fx_new_on=0;clear();
 for(uint32_t i=0;i<512;i+=2)pcm[i]=(int32_t)(i*53)-10000;
 int32_t dry[512];memcpy(dry,pcm,sizeof dry);fx_process(pcm,256);for(uint32_t i=0;i<512;i+=2)assert(pcm[i]==dry[i]&&pcm[i+1]==dry[i]);
 wm.fx_new_on=1;wm.crush_bits=16;wm.crush_rate=0;wm.crush_mix=127;settle();
 memcpy(pcm,dry,sizeof pcm);fx_process(pcm,256);for(uint32_t i=0;i<512;i+=2)assert(pcm[i]==dry[i]&&pcm[i+1]==dry[i]);
 wm.crush_bits=4;settle();memcpy(pcm,dry,sizeof pcm);fx_process(pcm,256);
 for(uint32_t i=0;i<512;i+=2)assert(!pcm[i]||((abs(pcm[i])-2048)%4096)==0);
 assert(tape_early(0)==0&&tape_early(2)>10000);
 wm=(wm_state_t)WM_DEFAULTS;wm.fx_on=3;wm.chorus_mix=127;wm.delay_mix=127;wm.delay_feedback=127;clear();settle();
 for(uint32_t i=0;i<FS;i++){int32_t out[2]={i==0?16000:0,0};fx_process(out,1);}
 for(uint32_t i=0;i<1024;i++)assert(chorus_line[i]==0); /* Echo feedback cannot enter chorus storage. */
 uint32_t expected=hash(256);uint32_t sizes[]={1,17,64,255};for(uint32_t i=0;i<4;i++)assert(hash(sizes[i])==expected);
 /* Maximal seven-effect combination, retriggers, complete silence/reset. */
 wm=(wm_state_t)WM_DEFAULTS;wm.fx_on=7;wm.fx_new_on=15;wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;
 wm.crush_mix=wm.tape_mix=wm.phaser_mix=wm.fold_mix=127;wm.tape_feedback=wm.fold_drive=wm.phaser_depth=127;wm.crush_bits=1;wm.crush_rate=15;
 clear();for(uint32_t b=0;b<1000;b++){for(uint32_t i=0;i<256;i++)pcm[i*2]=b<200?wn_sine_i((b*256u+i)*0x04321000u):0;
  fx_process(pcm,256);for(uint32_t i=0;i<512;i++)assert(abs(pcm[i])<32767);}
 clear();memset(pcm,0,sizeof pcm);fx_process(pcm,256);for(uint32_t i=0;i<512;i++)assert(!pcm[i]);
 puts("FX: exact bypass and 16-bit identity, 4-bit quantization, earlier multi-head echoes, independent chorus storage, five callback sizes, seven-effect extremes, bounded reset and silence pass");
}
