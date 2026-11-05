#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static void frames(uint32_t n){while(n){uint32_t b=n>256?256:n;int32_t o[512]={0};app_audio(o,b);n-=b;}}
static uint32_t screen_hash(void){screen();uint32_t h=0;for(uint32_t i=0;i<240u*240u;i++)h=h*33u+fb[i];return h;}
int main(void){cues_enabled=0;vol=100;wm=(wm_state_t)WM_DEFAULTS;ws_select(38);wm_record(0,60,100);wm_catch();ws_patch_t anchor=wm.anchor_patch;
 editor_page=0;fm1_ms=100;editor_adjust(0,20);assert(wm.patch.amp[0]!=anchor.amp[0]);assert(!memcmp(&wm.anchor_patch,&anchor,sizeof anchor));
 ws_patch_t edited=wm.patch;confirm=5;apply_confirm();assert(!memcmp(&wm.patch,&anchor,sizeof anchor));assert(wm_undo_once()&&!memcmp(&wm.patch,&edited,sizeof edited));notice=0;
 ws_patch_t edited_copy=edited;(void)edited_copy;wm.grain_size=6;confirm=2;apply_confirm();assert(notice==N_SAVED);wm.patch.cutoff=1;wm.grain_size=0;confirm=3;apply_confirm();assert(wm.grain_size==6);assert(notice==N_LOADED&&!memcmp(&wm.patch,&edited,sizeof edited));assert(!memcmp(&wm.anchor_patch,&anchor,sizeof anchor));
 wm_disk_t disk={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),wm};
 for(uint32_t i=0;i<sizeof(ws_patch_t)-6;i++){wm_disk_t bad=disk;((uint8_t*)&bad.state.patch)[i]=255;assert(!wm_valid(&bad));}assert(wm_valid(&disk));
 /* Read an exact v6 project through the production scanner, without rewriting. */
 wm_disk_v7_t old={WM_MAGIC,6,sizeof(wm_state_v7_t),{0}};memcpy(&old.state,&disk.state,sizeof old.state);old.state.grain_size=0;
 assert(!slot_save(7,&old,sizeof old));wm_state_t migrated;
 assert(project_read_slot(7,&migrated)&&!memcmp(&migrated,&old.state,(uint32_t)((uint8_t*)&migrated.scatter_edge-(uint8_t*)&migrated))&&migrated.arp_octaves==1&&migrated.scatter_edge==72);
 wm_disk_v7_t unchanged;assert(!slot_load(7,&unchanged,sizeof unchanged)&&!memcmp(&unchanged,&old,sizeof old));
 old.state.grain_size=1;assert(!slot_save(7,&old,sizeof old)&&!project_read_slot(7,&migrated));

 /* Maximum 32 steps at 60 BPM: both full Print generations fit the fixed arenas. */
 reset_performance();wp_reset();wm.loop_steps=32;wm.bpm=60;step_samples=FS/4;wm.fx_on=0;step_index=step_clock=cycle_index=step_primed=0;
 assert(wp_arm());running=1;frames(step_samples*32+1);assert(wp_active&&wp_has_anchor&&wp_length==128000);assert(wp_undo()&&wp_dry_view);assert(wp_undo()&&!wp_dry_view);
 assert(wp_arm());frames(step_samples*64);assert(wp_generation==2&&wp_undo_valid);transport();editor_open=1;editor_page=EDIT_PRINT;host_leds|=((uint64_t)1<<KEY_LED(7));leds_was_running=1;app_leds();assert(!(host_leds&((((uint64_t)1<<27)-1)<<14)));
 wp_restore();assert(wp_dry_view&&!anchor_view&&wp_active);listening_set(0);assert(wp_dry_view);assert(wp_undo()&&!wp_dry_view);
 settings_index=2;settings_open=1;settings_page=3;uint8_t steps=wm.loop_steps;length_adjust_locked(-1);assert(notice==N_AUDIO_LOCK&&wm.loop_steps==steps);settings_open=0;
 /* Audio history is RAM-only; loading the notes/patch never pretends to restore it. */
 confirm=3;apply_confirm();assert(notice==N_LOADED&&!wp_active&&!wp_has_anchor&&!memcmp(&wm.patch,&edited,sizeof edited));
 fm1_ms=2000;fm1_in.buttons=1u<<BTN_REC;detail_edges(1u<<BTN_REC,0);assert(rec_pending);panic();fm1_in.buttons=0;assert(!(detail_edges(0,1u<<BTN_REC)&(1u<<BTN_REC)));
 editor_open=1;confirm=2;notice=0;uint32_t modal=screen_hash();editor_open=0;assert(screen_hash()==modal);
 editor_open=1;confirm=0;notice=N_SAVED;notice_until=0;modal=screen_hash();editor_open=0;assert(screen_hash()==modal);notice=0;
 puts("patch/Anchor snapshots, save/load, malformed parameters, 8-second Print x2, first Undo/Redo, editor Stop LEDs, RAM-only reload and panic during held REC pass");return 0;}
