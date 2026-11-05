/* SPDX-License-Identifier: GPL-3.0-only
 * Wanderer 1.8: patterns, new grain sizes, v11 migration,
 * input history bounds, Print/Anchor/Undo isolation and stereo headroom. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
static uint32_t hash(const void *v,size_t n){
 const uint8_t *p=v;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;
}
static void render(uint32_t n){
 while(n){uint32_t m=n>256?256:n;int32_t out[512]={0};app_audio(out,m);
  for(uint32_t i=0;i<2*m;i++)assert(out[i]>=-32767&&out[i]<=32767);
  n-=m;
 }
}
int main(void){
 assert(WM_GRAIN_COUNT==10&&WM_GRAIN_MS[8]==512&&WM_GRAIN_MS[9]==768);
 assert(WP_RING_SAMPLES==12288u);
 /* v11 prefix must match exactly; do not copy its trailing pad into PATTERN. */
 assert(offsetof(wm_state_t,fx_new_on)==sizeof(wm_state_v10_t));
 assert(offsetof(wm_state_t,scatter_pattern)+1u==sizeof(wm_state_v11_t));
 wm=(wm_state_t)WM_DEFAULTS;ws_select(0);
 wm.scatter_pattern=8;wm.grain_size=9;wm.memory.e[0]=(wm_event_t){60,115,4096};
 wm_disk_t d={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),wm};
 assert(wm_valid(&d));d.state.scatter_pattern=WM_PATTERN_COUNT;assert(!wm_valid(&d));
 d.state.scatter_pattern=8;assert(!slot_save(2,&d,sizeof d));
 static wm_state_t out;assert(project_read_slot(2,&out)&&out.scatter_pattern==8&&out.grain_size==9);
 /* As recorded by version 1.7, the last byte is padding, not a pattern. */
 wm_disk_v11_t old={WM_MAGIC,11,sizeof(wm_state_v11_t),{0}};
 memcpy(&old.state,&wm,offsetof(wm_state_t,scatter_pattern));
 assert(!wm_migrate_v11_v12_ptr(&old)); /* Index 9 was introduced in v12. */
 old.state.prefix.grain_size=8;
 ((uint8_t*)&old.state)[sizeof old.state-1u]=0xff;
 assert(!slot_save(3,&old,sizeof old));
 assert(project_read_slot(3,&out)&&out.scatter_pattern==0&&out.grain_size==8&&out.memory.e[0].note==60);
 assert(!slot_load(3,&old,sizeof old)&&old.version==11u);
 assert(!wm_migrate_v11_v12_ptr(&(wm_disk_v11_t){.magic=WM_MAGIC,.version=11,.size=1}));
 /* 512 ms existing offsets remain untouched; 768 is fixed. */
 wm=(wm_state_t)WM_DEFAULTS;wp_reset();wp_ring_count=WP_RING_SAMPLES;
 for(uint32_t s=8;s<=9;s++)for(uint32_t offset=0;offset<8;offset++){
  wm.grain_size=s;scatter.kind=SC_PITCH;scatter.offset=offset;scatter.divisions=2;
  wp_scatter_begin();uint32_t want=s==9?12288u:(uint32_t)WM_GRAIN_MS[s]*16u*(6u+offset%5u)/8u;
  assert(wp_repeat_size==want&&wp_repeat_on&&wp_ring_start<WP_RING_SAMPLES);
 }
 wm.grain_size=9;wp_ring_count=12287;scatter.kind=SC_CLOUD;wp_scatter_begin();assert(!wp_repeat_on);
 wp_ring_count=12288;wp_scatter_begin();assert(wp_repeat_on);
 /* Every pattern produces events, stays within bounds, preserves melody notes.
  * Free also passes the old regression tests separately. */
 wm=(wm_state_t)WM_DEFAULTS;step_samples=FS*15u/wm.bpm;
 for(uint32_t pat=1;pat<WM_PATTERN_COUNT;pat++){
  wm.scatter_pattern=pat;wm.repeat_style=6;scatter_seed(0xdeadbeef);
  uint32_t events=0;
  for(uint32_t n=0;n<wm.loop_steps*8u;n++){
   scatter_plan(127,0,n%wm.loop_steps,n/wm.loop_steps);
   assert(scatter.kind>=SC_CLEAN&&scatter.kind<=SC_RUSH&&scatter.divisions<=24u&&scatter.offset<=7u);
   events+=(scatter.kind!=SC_CLEAN);
  }
  assert(events>0);
 }
 /* Silence, bypass and changing loop length never shift the pattern grid. */
 wm.scatter_pattern=7;wm.repeat_style=7;wm.decay=127;
 for(uint32_t length=16;length<=32;length+=16){
  wm.loop_steps=(uint8_t)length;scatter_seed(42);
  scatter_plan(0,0,0,0);scatter_plan(127,1,1,0);
  scatter_plan(127,0,length-3,4);assert(scatter.kind==SC_CLEAN);
  scatter_plan(127,0,length-2,4);assert(scatter.kind!=SC_CLEAN&&scatter.divisions==4);
  step_index=length-1;cycle_index=4;step_play();
  assert(scatter.kind==SC_RUSH&&scatter.divisions==8);
 }
 /* Test all modes on live audio with effects engaged, at 768 ms. */
 wm=(wm_state_t)WM_DEFAULTS;wp_reset();wm.fx_on=7;wm.fx_new_on=15;
 wm.decay=127;wm.scatter_edge=127;wm.scatter_mix=105;wm.grain_size=9;
 wm.memory.e[0]=(wm_event_t){60,110,4096};
 wm.memory.e[4]=(wm_event_t){67,105,4096};
 wm.memory.e[8]=(wm_event_t){72,105,4096};
 wm.memory.e[12]=(wm_event_t){64,105,4096};
 cues_enabled=0;running=1;ws_select(25);render(FS*2);
 uint32_t note_hash=hash(&wm.memory,sizeof wm.memory);
 for(uint32_t pat=0;pat<WM_PATTERN_COUNT;pat++){
  wm.scatter_pattern=pat;wm.repeat_style=(uint8_t)(pat%WM_SCATTER_MODES);
  scatter_seed(10+pat);render(FS/2u);
 }
 assert(hash(&wm.memory,sizeof wm.memory)==note_hash);
 /* Round-trip Print with snapshot invariants. */
 wm.scatter_pattern=2;wm.grain_size=9;
 assert(wp_arm());render(step_samples*wm.loop_steps*3u);
 assert(wp_has_anchor);
 uint32_t original=hash(wp_arena[0],sizeof wp_arena[0]);
 for(uint32_t pat=0;pat<WM_PATTERN_COUNT;pat++){
  wm.scatter_pattern=pat;render(step_samples*wm.loop_steps);
 }
 assert(hash(wp_arena[0],sizeof wp_arena[0])==original);
 wm.decay=0;render(4096);assert(!wp_scatter_mix);
 panic();render(256);assert(!wp_scatter_mix&&!scatter.kind);
 printf("10 independent patterns, 512/768 ms bounds, v11 read-only migration, Print/Anchor isolation, FX headroom pass\n");
}
