/* 2.0 defaults, non-destructive performance shortcuts, lane patch routing,
 * MIDI ownership, and compatibility with v13 projects. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void)
{
 assert(wm.sound==0&&!(wm.fx_on&1u)&&wm.grain_size==8);
 assert(!strcmp(ws_factory_name(wm.preset_bank,wm.sound),"AFTERGLOW"));
 assert(!strcmp(ws_factory_name(WS_BANK_1_9,0),"ROUND SUB"));
 assert(ws_current_patch()->filter==WS_LP24);
 const uint32_t oct=1u<<BTN_OCT_UP,play=1u<<BTN_PLAY;
 fm1_ms=100;fm1_in.buttons=oct;assert(!performance_edges(oct,0));
 fm1_ms=1599;assert(!performance_edges(0,0)&&!wm.extra_large);
 fm1_ms=1600;assert(!performance_edges(0,0)&&wm.extra_large&&!octave);
 fm1_ms=2400;assert(!performance_edges(0,0)&&wm.extra_large);
 fm1_in.buttons=0;assert(!performance_edges(0,oct));
 fm1_in.buttons=oct;assert(!performance_edges(oct,0));fm1_in.buttons=0;
 assert(performance_edges(0,oct)==oct);
 ws_select(33);wm.memory.e[0]=(wm_event_t){60,100,192};
 fm1_in.buttons=play;assert(!performance_edges(play,0));
 fm1_ms+=2000;assert(!performance_edges(0,0)&&!running&&wm.sound==0&&wm.memory.e[0].note==60);
 fm1_in.buttons=0;assert(!performance_edges(0,play));
 fm1_in.buttons=play;assert(!performance_edges(play,0));fm1_in.buttons=0;
 assert(performance_edges(0,play)==play);
 running=1;fm1_in.buttons=play;assert(performance_edges(play,0)==play);transport();
 fm1_in.buttons=0;assert(!performance_edges(0,play)&&!running);
 /* Memory and Layer hit the same note at the same time with different patches. */
 wm.layer_enabled=1;wm.layer_sound=35;wm.layer.e[0]=wm.memory.e[0];
 step_index=0;step_play();
 assert(voice[0].gate&&voice[1].gate&&voice[0].note==voice[1].note);
 assert(voice[0].patch_source==PATCH_MEMORY&&voice[1].patch_source==PATCH_LAYER);
 assert(!memcmp(&ws_voice[0].p,&WS_PRESETS_2[0],sizeof(ws_patch_t)));
 assert(!memcmp(&ws_voice[1].p,&WS_PRESETS_2[34],sizeof(ws_patch_t)));
 editor_page=1;editor_adjust(0,1);
 assert(!memcmp(&ws_voice[1].p,&WS_PRESETS_2[34],sizeof(ws_patch_t)));
 assert(wm_catch());anchor_view=1;step_index=0;step_play();anchor_view=0;
 settings_page=SETTINGS_LAYER;settings_open=1;settings_adjust(0,1);
 assert(voice[note_on(69,100,1)].patch_source==PATCH_LAYER);
 /* Settings filter held MIDI notes and leave local ownership alone. */
 layer_target_locked(0);settings_page=SETTINGS_MIDI;arp_update_config();midi_message(2,0x90,62,100);
 settings_adjust(0,1);assert(wm.midi_input==1);
 for(uint32_t v=0;v<NV;v++)assert(!voice[v].gate||voice[v].live<18u);
 settings_adjust(1,2);midi_message(1,0x90,82,100);
 for(uint32_t v=0;v<NV;v++)assert(!voice[v].gate||voice[v].note!=82);
 midi_message(1,0x91,83,100);
 uint8_t accepted=0;for(uint32_t v=0;v<NV;v++)accepted|=voice[v].gate&&voice[v].note==83;
 assert(accepted);
 settings_adjust(3,-1);midi_message(1,0xc1,7,0);assert(wm.sound==0);
 settings_adjust(3,1);midi_message(1,0xc1,7,0);assert(wm.sound==7);
 for(uint32_t page=SETTINGS_MIDI;page<=SETTINGS_LAYER;page++){
  settings_page=page;
  for(settings_index=0;settings_index<settings_count();settings_index++)
   for(wm.extra_large=0;wm.extra_large<2;wm.extra_large++)screen();
 }
 wm.extra_large=0;
 wm_disk_t disk={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),wm};
 assert(wm_valid(&disk)&&!slot_save(2,&disk,sizeof disk));
 static wm_state_t loaded;
 assert(project_read_slot(2,&loaded)&&loaded.layer_sound==35&&loaded.midi_channel==2);
 wm_disk_v13_t old={WM_MAGIC,13,sizeof(wm_state_v13_t),{0}};
 wm.preset_bank=WS_BANK_1_9;wm.patch_valid=0;wm.sound=0;wm.reverb_type=RV_PLATE;
 memcpy(&old.state,&wm,__builtin_offsetof(wm_state_t,layer_sound));
 assert(!slot_save(3,&old,sizeof old)&&project_read_slot(3,&loaded));
 assert(loaded.preset_bank==WS_BANK_1_9&&!loaded.layer_sound&&!loaded.midi_channel&&loaded.reverb_type==RV_PLATE);
 assert(!strcmp(ws_factory_name(loaded.preset_bank,loaded.sound),"ROUND SUB"));
 assert(!slot_load(3,&old,sizeof old)&&old.version==13u);
 wm.drift=85;wm.melt=0;
 const wm_event_t before={64,100,192};
 assert(wm_evolve(before,15,0).note==64&&wm_evolve(before,15,4).note==64);
 assert(wm_evolve(before,12,2).note==64&&wm_evolve(before,15,2).note!=64);
 wm.melt=127;assert(wm_evolve(before,15,2).gate<192&&wm_evolve(before,12,2).velocity>100);
 editor_page=EDIT_SCATTER;editor_move(1);assert(editor_page==EDIT_PATTERN);
 puts("2.0 defaults, holds, split sounds, MIDI ownership, musical motion, accessible screens and migration pass.");
}
