/* Render every factory sound (bank argument: 2 = 2.0, 3 = 3.0) and print
 * index, RMS, peak and zero-crossing rate. Used by tools/level_bank_3_0.py. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static int32_t buf[2*4096];
int main(int argc,char **argv)
{
 int bank=argc>1?atoi(argv[1]):3;
 for(uint32_t i=0;i<WS_PRESET_COUNT;i++){
  const ws_patch_t *p=bank==2?&WS_PRESETS_2[i]:&WS_PRESETS_3[i];
  wm.patch=*p;wm.patch_valid=1;wm.legacy=0;layer_focus=anchor_view=0;
  memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);
  uint32_t fam=i/10u,low=fam==1u||fam==2u;
  static const uint8_t chord[3]={48,52,55};
  if(low)note_on(36,100,1);else for(uint32_t k=0;k<3;k++)note_on(chord[k],100,1);
  double e=0;int32_t peak=0;uint32_t zc=0,frames=0;int32_t prev=0;
  for(uint32_t n=0;n<FS*2u;n+=4096){
   if(n>=FS*3u/2u){if(low)note_off(36,1);else for(uint32_t k=0;k<3;k++)note_off(chord[k],1);}
   memset(buf,0,sizeof buf);for(uint32_t v=0;v<NV;v++)ws_render(v,buf,4096,0);
   for(uint32_t j=0;j<8192;j+=2){int32_t m=buf[j];e+=(double)m*m;if(abs(m)>peak)peak=abs(m);zc+=(m^prev)<0;prev=m;frames++;}
  }
  printf("%u %.1f %d %.4f\n",i,__builtin_sqrt(e/frames),peak,(double)zc/frames);
 }
 return 0;
}
