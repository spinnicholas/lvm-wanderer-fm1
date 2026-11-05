#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void)
{
 assert(WM_PRESET_COUNT==150&&WM_PRESET_BANKS==15);
 for(uint32_t p=0;p<WM_PRESET_COUNT;p++){
  assert(ws_patch_valid(&WS_PRESETS[p])&&ws_patch_valid(&WS_PRESETS_2[p]));ws_select(p);assert(wm.sound==p);
  assert(wm.preset_bank==WS_BANK_CURRENT&&wm.patch_valid&&!wm.legacy);
  assert(!memcmp(&wm.patch,&WS_PRESETS_3[p],sizeof wm.patch));
  assert(wm_preset_move(p,150,0)==p&&wm_preset_move(p,-150,0)==p);
  assert(wm_preset_move(p,15,1)==p&&wm_preset_move(p,-15,1)==p);
 }
 assert(wm_preset_move(0,-1,0)==149&&wm_preset_move(149,1,0)==0);
 assert(wm_preset_move(0,-1,1)==140&&wm_preset_move(149,1,1)==9);
 /* MIDI banks latch per port AND channel and act only on Program Change. */
 ws_select(1);midi_message(1,0xb0,32,1);assert(wm.sound==1);
 midi_message(1,0xc0,21,0);assert(wm.sound==149);
 midi_message(1,0xc1,0,0);assert(wm.sound==0);
 midi_message(2,0xc0,127,0);assert(wm.sound==127);
 midi_message(1,0xc0,22,0);assert(wm.sound==127);
 midi_message(1,0xb0,0,1);midi_message(1,0xc0,0,0);assert(wm.sound==127);
 midi_message(1,0xb0,0,0);midi_message(1,0xc0,0,0);assert(wm.sound==128);
 midi_message(1,0xb0,32,127);midi_message(1,0xc0,127,0);assert(wm.sound==128);
 midi_message(1,0xb0,32,0);midi_message(1,0xc0,0,0);assert(wm.sound==0);
 /* v8 factory fallback and names remain 1.4; custom and Anchor snapshots survive. */
 wm_disk_v8_t old={WM_MAGIC,8,sizeof(wm_state_v8_t),{0}};
 wm=(wm_state_t)WM_DEFAULTS;ws_select(149);wm_record(0,60,110);assert(wm_catch());
 wm_disk_t disk={WM_MAGIC,WM_DISK_VERSION,sizeof wm,wm};assert(wm_valid(&disk));
 assert(wm.anchor_sound==149&&wm.anchor_preset_bank==WS_BANK_CURRENT);
 assert(!slot_save(0,&disk,sizeof disk));static wm_state_t loaded;
 assert(project_read_slot(0,&loaded));assert(!memcmp(&loaded,&wm,sizeof wm));
 memcpy(&old.state,&wm,(uint32_t)((uint8_t*)&old.state.arp_clock-(uint8_t*)&old.state)+1u);
 old.state.sound=33;old.state.anchor_sound=60;old.state.patch=WS_PRESETS_1_4[33];
 old.state.anchor_patch=WS_PRESETS_1_4[60];old.state.patch.cutoff=47;
 const wm_state_t *p=wm_migrate_v8_ptr(&old);assert(p);
 assert(p->preset_bank==WS_BANK_1_4&&p->anchor_preset_bank==WS_BANK_1_4);
 assert(!memcmp(&p->patch,&old.state.patch,sizeof p->patch));
 assert(!memcmp(&p->anchor_patch,&old.state.anchor_patch,sizeof p->anchor_patch));
 assert(!slot_save(1,&old,sizeof old));assert(project_read_slot(1,&loaded));
 assert(loaded.patch.cutoff==47&&loaded.has_anchor&&loaded.anchor.e[0].note==60);
 old.state.patch_valid=old.state.anchor_patch_valid=0;p=wm_migrate_v8_ptr(&old);assert(p);wm=*p;
 assert(ws_current_patch()==&WS_PRESETS_1_4[33]);assert(!strcmp(ws_factory_name(wm.preset_bank,wm.sound),"SOFT POLY"));
 anchor_view=1;uint32_t v=note_on(60,110,1);assert(!memcmp(&ws_voice[v].p,&WS_PRESETS_1_4[60],sizeof(ws_patch_t)));
 anchor_view=0;confirm=5;apply_confirm();assert(wm.preset_bank==WS_BANK_1_4&&wm.sound==60);
 ws_select(149);assert(wm.preset_bank==WS_BANK_CURRENT);
 old.state.sound=100;assert(!wm_migrate_v8_ptr(&old));old.state.sound=33;
 old.state.anchor_sound=100;assert(!wm_migrate_v8_ptr(&old));
 disk.state.sound=150;assert(!wm_valid(&disk));disk.state.sound=149;
 disk.state.anchor_sound=150;assert(!wm_valid(&disk));disk.state.anchor_sound=149;
 disk.state.legacy=1;assert(!wm_valid(&disk));
 puts("150 presets, 15 banks, MIDI addressing/isolation, current save/load, frozen v8 fallback, snapshots and Anchor restoration pass");
 return 0;
}
