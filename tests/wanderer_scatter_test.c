#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static uint32_t hash(const void *data,uint32_t n){const uint8_t *p=data;uint32_t h=0;while(n--)h=h*33u+*p++;return h;}
static void render(uint32_t count){while(count){uint32_t n=count>256?256:count;int32_t o[512]={0};app_audio(o,n);for(uint32_t i=0;i<n*2;i++)assert(abs(o[i])<=32767);count-=n;}}
static uint32_t held_state(uint8_t amount){
 running=0;reset_performance();wp_reset();wm=(wm_state_t)WM_DEFAULTS;ws_select(24);cues_enabled=0;wm.fx_on=0;
 memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);ages=audio_frames=0;step_index=step_clock=cycle_index=step_primed=0;step_samples=FS*15u/wm.bpm;
 wm.memory.e[0]=(wm_event_t){60,110,4096};wm.memory.extra[0][0]=(wm_event_t){67,100,4096};wm.decay=amount;scatter_seed(111);running=1;render(FS);
 uint32_t h=hash(ws_voice,sizeof ws_voice);for(uint32_t v=0;v<NV;v++){h=h*33u+voice[v].remain;h=h*33u+(uint32_t)voice[v].gate;}return h;
}
static void grain_checks(void){
 wm=(wm_state_t)WM_DEFAULTS;wm.grain_size=0;step_samples=FS*15u/wm.bpm;wp_reset();
 wp_ring_count=WP_RING_SAMPLES;scatter.kind=SC_STUTTER;scatter.divisions=8;scatter.offset=2;
 wp_scatter_begin();assert(wp_repeat_size==(step_samples*160u/441u)/8u);
 for(uint32_t size=1;size<WM_GRAIN_COUNT;size++)for(uint32_t kind=SC_STUTTER;kind<=SC_RUSH;kind++)for(uint32_t offset=1;offset<=7;offset++){
  wm.grain_size=size;scatter.kind=kind;scatter.offset=offset;wp_scatter_begin();
  uint32_t base=WM_GRAIN_MS[size]*16u;
  assert(size==WM_GRAIN_COUNT-1u?wp_repeat_size==WP_RING_SAMPLES:(wp_repeat_size>=base*3u/4u&&wp_repeat_size<=base*5u/4u));
  assert(wp_repeat_size>=32&&wp_repeat_size<=WP_RING_SAMPLES&&wp_ring_start<WP_RING_SAMPLES);
 }
 wm_disk_t d={WM_MAGIC,6,sizeof(wm_state_t),WM_DEFAULTS};
 d.state.grain_size=0;d.state.patch_valid=1;d.state.patch=WS_PRESETS[24];d.state.memory.e[0]=(wm_event_t){64,120,8192};d.state.repeat_style=2;
 d.state.preset_bank=d.state.anchor_preset_bank=WS_BANK_1_3;
 const wm_state_t *m=wm_migrate_v6_ptr(&d);assert(wm_valid_v6(&d)&&m&&!memcmp(m,&d.state,sizeof *m));
 d.state.grain_size=1;assert(!wm_valid_v6(&d));d.version=WM_DISK_VERSION;
 for(uint32_t size=0;size<WM_GRAIN_COUNT;size++){d.state.grain_size=size;assert(wm_valid(&d));}
 d.state.grain_size=WM_GRAIN_COUNT;assert(!wm_valid(&d));
 /* The actual app callback must feed Scatter into the wet bus. A source made
  * solely by Scatter leaves a tail once the source is removed. */
 for(uint32_t effect=1;effect<=4;effect*=2){
  wm=(wm_state_t)WM_DEFAULTS;wm.fx_on=effect;wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;
  wm.delay_div=0;wm.delay_feedback=80;wm.decay=127;wm.grain_size=4;
  memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);wp_reset();fx_reset();while(fx_clearing)fx_clear_chunk();
  cues_enabled=0;running=1;scatter.kind=SC_STUTTER;scatter.offset=1;wp_ring_count=WP_RING_SAMPLES;
  for(uint32_t i=0;i<WP_RING_SAMPLES;i++)wp_ring[i]=6000;
  wp_scatter_begin();wp_scatter_delay=0;
  for(uint32_t i=0;i<32;i++){int32_t o[512]={0};app_audio_core(o,256);}
  wm.decay=0;wp_repeat_on=0;wp_scatter_mix=0;uint64_t energy=0;
  for(uint32_t i=0;i<32;i++){int32_t o[512]={0};app_audio_core(o,256);for(uint32_t j=0;j<512;j++)energy+=(uint32_t)abs(o[j]);}
  assert(energy>100000);
 }
 running=0;reset_performance();
}
int main(void){grain_checks();assert(held_state(0)==held_state(127));running=0;reset_performance();wp_reset();step_index=step_clock=cycle_index=step_primed=0;uint32_t kinds[7]={0},low=0,high=0;scatter_seed(1);
for(uint32_t i=0;i<10000;i++){scatter_plan(127,0,0,0);assert(scatter.divisions>=1&&scatter.divisions<=24);kinds[scatter.kind]++;if(scatter.kind)high++;}
for(uint32_t i=0;i<7;i++)assert(kinds[i]>100);
scatter_seed(1);for(uint32_t i=0;i<10000;i++){scatter_plan(8,0,0,0);if(scatter.kind)low++;}assert(low<high/8);
scatter_plan(0,0,0,0);assert(!scatter.kind);scatter_plan(127,1,0,0);assert(!scatter.kind);
wm=(wm_state_t)WM_DEFAULTS;wm.decay=127;wm.fx_on=0;cues_enabled=0;vol=100;ws_select(24);step_samples=FS*15u/wm.bpm;
wm_record(0,60,110);wm_record(4,63,100);wm_record(8,67,100);wm_catch();uint32_t notes=hash(&wm.memory,sizeof wm.memory),anchor=hash(&wm.anchor,sizeof wm.anchor);
scatter_seed(9);running=1;render(step_samples*16*8);assert(hash(&wm.memory,sizeof wm.memory)==notes&&hash(&wm.anchor,sizeof wm.anchor)==anchor);
wm.decay=0;render(128);assert(!wp_scatter_mix);wm.decay=127;
assert(wp_arm());render(step_samples*32+1);assert(wp_has_anchor);uint32_t audio=hash(wp_arena[0],sizeof wp_arena[0]);
for(uint32_t style=0;style<3;style++)for(uint32_t size=0;size<WM_GRAIN_COUNT;size++){wm.repeat_style=style;wm.grain_size=size;render(step_samples*16*2);assert(hash(wp_arena[0],sizeof wp_arena[0])==audio);}
assert(wp_arm());render(step_samples*32);assert(wp_undo_valid);assert(wp_undo());assert(wp_undo());assert(hash(wp_arena[0],sizeof wp_arena[0])==audio);
panic();render(64);assert(!wp_scatter_mix&&!scatter.kind);for(uint32_t v=0;v<NV;v++)assert(!voice[v].gate);
printf("all six gestures + clean gaps; low %u/high %u per 10000; zero/panic, bounded output, note/Anchor integrity, Print/Undo/Redo pass\n",low,high);}
