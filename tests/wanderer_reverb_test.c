/* Reverb algorithms, type switching, accessible controls and disk compatibility. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static void fresh_reverb(uint8_t type)
{
 wm=(wm_state_t)WM_DEFAULTS;
 wm.reverb_type=type;wm.fx_on=4;wm.reverb_mix=127;wm.reverb_size=110;wm.reverb_damp=56;
 fx_reset();while(fx_clearing)fx_clear_chunk();
 motion_phase=0;
}
static uint64_t tail_at_mix(uint8_t type,uint8_t mix)
{
 fresh_reverb(type);wm.reverb_mix=mix;
 uint64_t energy=0;
 for(uint32_t frame=0;frame<FS;frame++){
  int32_t sample=frame>=2048u&&frame<3072u?wn_sine_i(frame*0x02900000u)/4:0;
  int32_t out[2]={sample,0};fx_process(out,1);
  if(mix==127&&frame>=2048u&&frame<2088u)assert(!out[0]&&!out[1]);
  assert(abs(out[0])<=30000&&abs(out[1])<=30000);
  if(frame>=3072u)energy+=(uint32_t)(out[0]*out[0]+out[1]*out[1]);
 }
 return energy;
}
/* A short three-tone chord, followed by silence. Use whole-second
 * energy windows so a single quiet beat cannot masquerade as a cutoff. */
static void ambient_decay(uint8_t size,uint32_t seconds,uint64_t *early,uint64_t *late)
{
 fresh_reverb(RV_AMBIENT);wm.reverb_size=size;
 *early=*late=0;
 for(uint32_t frame=0;frame<FS*(seconds+1u);frame++){
  int32_t sample=0;
  if(frame>2048u&&frame<FS/2u){
   sample=(wn_sine_i(frame*12755598u)+wn_sine_i(frame*19110994u)+wn_sine_i(frame*25511197u))/6;
   if(frame>FS/2u-1000u)sample=sample*(int32_t)(FS/2u-frame)/1000;
  }
  int32_t out[2]={sample,0};fx_process(out,1);
  uint32_t energy=(uint32_t)(out[0]*out[0]+out[1]*out[1]);
  if(frame>=FS&&frame<2u*FS)*early+=energy;
  if(frame>=seconds*FS)*late+=energy;
 }
}
static void ambient_release(void)
{
 fresh_reverb(RV_AMBIENT);wm.reverb_size=127;wm.reverb_damp=0;
 for(uint32_t frame=0;frame<4u*FS;frame++){
  int32_t out[2]={wn_sine_i(frame*4153924u),0};fx_process(out,1);
  assert(abs(out[0])<=30000&&abs(out[1])<=30000);
 }
 /* Off drops the tail and clears the delay path even after sustained drive. */
 wm.fx_on=0;
 for(uint32_t frame=0;frame<FS/4u;frame++){
  int32_t out[2]={0,0};fx_process(out,1);
 }
 wm.fx_on=4;
 for(uint32_t frame=0;frame<FS/4u;frame++){
  int32_t out[2]={0,0};fx_process(out,1);
  assert(!out[0]&&!out[1]);
 }
}
int main(void)
{
 setvbuf(stdout,0,_IONBF,0);
 uint32_t fingerprint[RV_COUNT];
 uint64_t tails[RV_COUNT];
 for(uint32_t type=0;type<RV_COUNT;type++){
  uint64_t moderate=tail_at_mix(type,63),full=tail_at_mix(type,127);
  assert(moderate&&full>=moderate*25u);
  fresh_reverb((uint8_t)type);
  uint32_t hash=0,stereo=0,first=FS*4u;
  uint64_t tail=0;
  for(uint32_t frame=0;frame<FS*4u;frame++){
   int32_t out[2]={frame>=2048u&&frame<3072u?wn_sine_i(frame*0x02900000u)/2:0,0};
   fx_process(out,1);
   assert(abs(out[0])<=30000&&abs(out[1])<=30000);
   hash=hash*33u+(uint32_t)out[0]*7u+(uint32_t)out[1];
   if(out[0]!=out[1])stereo++;
   if(frame>2048u&&first==FS*4u&&abs(out[0]-out[1])>=8)first=frame;
   if(frame>=FS)tail+=(uint32_t)(out[0]*out[0]+out[1]*out[1]);
  }
  assert(stereo>500&&first<2048u+FS/4u);
  for(uint32_t prior=0;prior<type;prior++)assert(fingerprint[prior]!=hash);
  fingerprint[type]=hash;tails[type]=tail;
  printf("type %u stereo %u onset %u samples late energy %llu\n",type,stereo,first-2048u,(unsigned long long)tail);
 }
 assert(tails[RV_DEEP_SPACE]>tails[RV_SPRING]&&tails[RV_DEEP_SPACE]>tails[RV_HALL]);
 uint64_t early,late;
 ambient_decay(0,20,&early,&late);assert(early&&!late);
 ambient_decay(72,20,&early,&late);assert(early&&late>early/2000u&&late<early/20u);
 ambient_decay(127,60,&early,&late);assert(early&&late>early/8192u&&late<early/20u);
 ambient_release();
 puts("Ambient Size sweep, minute-long tail and clean bypass pass.");
 fresh_reverb(RV_HALL);
 for(uint32_t k=0;k<FS/4u;k++){
  int32_t out[2]={k<FS/8u?wn_sine_i(k*0x06130200u)/2:0,0};fx_process(out,1);
 }
 wm.reverb_type=RV_DEEP_SPACE;
 for(uint32_t k=0;k<2048u;k++){
  int32_t out[2]={0,0};fx_process(out,1);
  if(verb_type==RV_DEEP_SPACE&&verb_age>1u&&verb_age<VERB_PROFILES[RV_DEEP_SPACE].pre)
   assert(abs(out[0])<=1&&abs(out[1])<=1);
 }
 assert(verb_type==RV_DEEP_SPACE&&verb_gain==256u);
 settings_page=SETTINGS_REVERB;settings_open=1;settings_index=0;
 wm.reverb_type=RV_ROOM;
 char value[16];
 for(uint32_t type=RV_ROOM;type<RV_COUNT;type++){
  assert(wm.reverb_type==type&&settings_get(0)==type+1u);
  for(wm.extra_large=0;wm.extra_large<2u;wm.extra_large++){
   const char *name=settings_value(0,value);
   assert(name!=value);
   screen();
  }
  wm.extra_large=0;
  if(type+1u<RV_COUNT)settings_adjust(0,1);
 }
 settings_press();assert(!strcmp(settings_value(0,value),"OFF")&&wm.reverb_type==RV_AMBIENT);
 settings_press();assert(!strcmp(settings_value(0,value),"AMBIENT"));
 wm_disk_t saved={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),wm};
 static wm_state_t loaded;
 assert(wm_valid(&saved)&&!slot_save(3,&saved,sizeof saved));
 assert(project_read_slot(3,&loaded)&&loaded.reverb_type==RV_AMBIENT);
 saved.state.reverb_type=RV_COUNT;assert(!wm_valid(&saved));
 wm_disk_v11_t old={WM_MAGIC,12,sizeof(wm_state_v11_t),{0}};
 wm.scatter_pattern=8;wm.grain_size=9;
 memcpy(&old.state,&wm,sizeof old.state);
 assert(!slot_save(2,&old,sizeof old)&&project_read_slot(2,&loaded));
 assert(loaded.reverb_type==RV_ROOM&&loaded.scatter_pattern==8&&loaded.grain_size==9);
 assert(!slot_load(2,&old,sizeof old)&&old.version==12u);
 puts("All reverb types, clean switching, Normal/Zoom UI and v12/current slots pass.");
}
