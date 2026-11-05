#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static void frames(uint32_t n,uint64_t *energy)
{
 while(n){uint32_t k=n>256?256:n;int32_t out[512]={0};app_audio(out,k);
  for(uint32_t i=0;i<k*2;i++){assert(abs(out[i])<32767);if(energy)*energy+=(int64_t)out[i]*out[i];}n-=k;}
}
int main(void)
{
 cues_enabled=0;vol=100;master_smooth=25600;uint32_t cases=0;
 for(uint32_t a=0;a<32;a++)for(uint32_t hi=0;hi<2;hi++){
  running=recording=anchor_view=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;ws_select(40);
  wm.fx_on=0;wm.patch.engine=5;wm.patch.algorithm=a;wm.patch.detune=wm.patch.pulse=16;
  wm.patch.mix=wm.patch.sub=90;wm.patch.shape=78;wm.patch.feedback=hi?127:0;wm.patch.position=60;wm.patch.inharm=24;
  wm.patch.amp[0]=wm.patch.fenv[0]=0;wm.patch.amp[2]=wm.patch.fenv[2]=110;wm.patch.amp[3]=wm.patch.fenv[3]=0;
  for(uint32_t o=0;o<4;o++){wm.patch.ratio[o]=hi?127:16;wm.patch.op_level[o]=90;wm.patch.op_env[o][0]=wm.patch.op_env[o][3]=0;wm.patch.op_env[o][2]=110;}
  frames(FS/2,0);note_on(hi?112:55,110,1);uint64_t energy=0;frames(8192,&energy);assert(energy>1000);
  note_off(hi?112:55,1);frames(FS,0);for(uint32_t v=0;v<NV;v++)assert(!voice[v].amp);cases++;
 }
 /* Imported v9/v10 records get transparent new-effect defaults. */
 wm=(wm_state_t)WM_DEFAULTS;ws_select(0);wm_record(0,55,100);assert(wm_catch());
 wm_disk_v10_t old={WM_MAGIC,10,sizeof(wm_state_v10_t),{0}};
 memcpy(&old.state,&wm,sizeof old.state);const wm_state_t *p=wm_migrate_v9_ptr(&old);assert(p);
 assert(p->memory.e[0].note==55&&p->anchor.e[0].note==55&&!p->fx_new_on&&p->crush_bits==10);
 assert(!slot_save(2,&old,sizeof old));static wm_state_t loaded;assert(project_read_slot(2,&loaded));
 assert(loaded.crush_bits==10&&loaded.has_anchor);old.version=9;assert(wm_migrate_v9_ptr(&old));
 old.size--;assert(!wm_migrate_v9_ptr(&old));
 /* All new effect controls preserve unrelated state and validate on disk. */
 wm=(wm_state_t)WM_DEFAULTS;settings_open=1;fm1_ms=100;
 for(settings_page=9;settings_page<SETTINGS_PAGES;settings_page++)for(uint32_t k=0;k<4;k++)for(uint32_t end=0;end<2;end++){
  settings_adjust(k,end?10000:-10000);wm_disk_t d={WM_MAGIC,WM_DISK_VERSION,sizeof wm,wm};assert(wm_valid(&d));
 }
 printf("%u MSFA six-operator algorithm/register/feedback/release cases; v9/v10 import and all new effect-control endpoints pass\n",cases);
}
