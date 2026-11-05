/* SPDX-License-Identifier: GPL-3.0-only
 * Host-synthesized comparison (not a hardware recording). Output PCM WAV. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
static void le16(FILE *f,uint32_t x){fputc(x&255u,f);fputc((x>>8)&255u,f);}
static void le32(FILE *f,uint32_t x){le16(f,x&0xffffu);le16(f,x>>16);}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 const uint32_t seconds=6,sections=3,frames=seconds*sections*FS;
 FILE *f=fopen(argv[1],"wb");if(!f)return 3;
 fwrite("RIFF",1,4,f);le32(f,36u+frames*4u);fwrite("WAVEfmt ",1,8,f);
 le32(f,16);le16(f,1);le16(f,2);le32(f,FS);le32(f,FS*4);le16(f,4);le16(f,16);
 fwrite("data",1,4,f);le32(f,frames*4u);
 for(uint32_t section=0;section<sections;section++){
  running=0;reset_performance();wp_reset();fx_reset();while(fx_clearing)fx_clear_chunk();
  wm=(wm_state_t)WM_DEFAULTS;ws_select(34);cues_enabled=0;
  wm.fx_on=7;wm.reverb_mix=68;wm.delay_mix=25;wm.chorus_mix=22;
  wm.scatter_edge=110;wm.scatter_mix=108;wm.scatter_tone=102;
  wm.decay=112;wm.grain_size=section?9:8;
  wm.repeat_style=section==0?6u:section==1?5u:4u;
  wm.scatter_pattern=section==0?3u:section==1?8u:4u;
  wm.memory.e[0]=(wm_event_t){48,104,950};
  wm.memory.e[4]=(wm_event_t){55,98,800};
  wm.memory.e[8]=(wm_event_t){60,103,950};
  wm.memory.e[12]=(wm_event_t){64,95,780};
  scatter_seed(0x1234876u+section*41u);
  step_index=step_clock=cycle_index=step_primed=0;step_samples=FS*15u/wm.bpm;
  running=1;
  for(uint32_t pos=0;pos<seconds*FS;){
   uint32_t n=seconds*FS-pos;if(n>256)n=256;
   int32_t audio[512]={0};app_audio(audio,n);
   for(uint32_t j=0;j<n*2u;j++){int32_t v=audio[j];if(v>32767)v=32767;if(v< -32768)v=-32768;le16(f,(uint16_t)v);}
   pos+=n;
  }
 }
 fclose(f);printf("18 s: Pitched+Cascade (512), Cloud+Morph (768), Tape+Drunk (768); host-rendered only\n");
}
