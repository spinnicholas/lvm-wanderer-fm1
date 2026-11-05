/* Production DSP and state integration tests. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static uint32_t checks;
#define CHECK(x) do{assert(x);checks++;}while(0)
static int64_t energy;
static void frames(uint32_t n)
{while(n){uint32_t count=n>256?256:n;int32_t out[512]={0};app_audio(out,count);for(uint32_t i=0;i<count*2;i++){CHECK(out[i]>=-32767&&out[i]<=32767);energy+=(int64_t)out[i]*out[i];}n-=count;}}
static void fresh(void)
{running=recording=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;step_samples=FS*15u/wm.bpm;step_clock=step_index=cycle_index=step_primed=0;anchor_view=0;cues_enabled=0;vol=100;frames(FS/2);wm.fx_on=0;energy=0;}
int main(void)
{
 fresh();
 for(uint32_t p=0;p<WM_PRESET_COUNT;p++){
  fresh();ws_select(p);note_on(60,110,1);frames(FS/2);if(energy<=1000000)fprintf(stderr,"quiet patch %u %s energy %lld\n",p,WS_NAMES[p],(long long)energy);CHECK(energy>1000000);note_off(60,1);frames(FS*3);CHECK(!voice[0].gate);
 }
 puts("150 engines/patches audible; release paths bounded");
 for(uint32_t engine=0;engine<5;engine++){
  fresh();uint32_t preset=0;while(WS_PRESETS[preset].engine!=engine)preset++;ws_select(preset);wm.melt=127;wm.fx_on=7;wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;
  for(uint32_t v=0;v<4;v++)note_on(36+v*7,127,1);frames(FS);panic();frames(FS/2);for(uint32_t v=0;v<4;v++)CHECK(!voice[v].gate&&!voice[v].amp);
 }
 fresh();wm.drift=127;wm.decay=127;
 for(uint32_t cycle=0;cycle<32;cycle++){
  wm_event_t root={60,100,512},landmark={64,90,700};CHECK(wm_evolve(root,3,cycle).note==60);CHECK(wm_evolve(landmark,4,cycle).note==64);
  CHECK(wm_evolve(landmark,5,cycle).velocity==90);if(cycle%8==0)CHECK(wm_evolve(landmark,5,cycle).note==64);
 }
 fresh();wm_record(0,60,100);wm_record(4,64,100);wm_record(8,67,100);wm_record(12,72,100);wm_catch();
 CHECK(wp_arm());transport();CHECK(wp_armed);frames(step_samples*wm.loop_steps);CHECK(wp_writing&&!wp_active);frames(1);CHECK(wp_active&&wp_has_anchor&&!wp_writing);CHECK(wp_length>0&&wp_length<=WP_MAX);
 uint32_t anchor_hash=0;for(uint32_t i=0;i<WP_BLOCKS*WP_BLOCK;i++)anchor_hash=anchor_hash*33+wp_arena[0][i];
 CHECK(wp_undo()&&wp_dry_view);CHECK(wp_undo()&&!wp_dry_view);
 uint8_t original=wp_current;CHECK(wp_arm());frames(step_samples*wm.loop_steps);CHECK(wp_writing);CHECK(wp_undo());CHECK(!wp_writing&&wp_current==original);
 CHECK(wp_arm());frames(step_samples*wm.loop_steps*2);CHECK(!wp_writing&&wp_undo_valid);uint8_t printed=wp_current;CHECK(printed!=original);CHECK(wp_undo()&&wp_current==original);CHECK(wp_undo()&&wp_current==printed);
 uint32_t newhash=0;for(uint32_t i=0;i<WP_BLOCKS*WP_BLOCK;i++)newhash=newhash*33+wp_arena[0][i];CHECK(newhash==anchor_hash);
 transport();CHECK(!running&&!wp_writing&&!wp_armed);app_leds();for(uint32_t k=0;k<27;k++)CHECK(!(host_leds&((uint64_t)1<<KEY_LED(k))));
 puts("Print generation commit/cancel/Undo/Redo, protected audio Anchor, stop LEDs");
 wm_disk_v5_t old={WM_MAGIC,5,sizeof(wm_state_v5_t),{0}};old.state.magic=WM_MAGIC;old.state.bpm=112;old.state.loop_steps=16;old.state.decay=94;old.state.sound=72;
 CHECK(wm_valid_v5(&old));wm_state_t migrated=wm_migrate_v5(&old);CHECK(migrated.legacy&&migrated.decay==94&&migrated.sound==72);
 wm_disk_t disk={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),{0}};disk.state=wm;CHECK(wm_valid(&disk));
 printf("%u assertions passed\n",checks);return 0;
}
