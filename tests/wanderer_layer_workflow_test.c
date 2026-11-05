/* Live lane targeting, isolated patches, mute, and project migration. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>

int main(void)
{
 assert(wm.layer_enabled&&ws_layer_own()&&!layer_focus);
 cues_enabled=0;
 ws_select(8);
 wm.memory.e[0]=(wm_event_t){60,100,400};
 wm.layer.e[0]=(wm_event_t){67,100,400};
 running=1;step_play();
 uint32_t prior=step_index;
 assert(voice[0].gate&&voice[1].gate);
 assert(voice[0].patch_source==PATCH_MEMORY&&voice[1].patch_source==PATCH_LAYER);
 settings_page=SETTINGS_LAYER;settings_open=1;
 settings_adjust(0,1);
 assert(layer_focus&&wm.layer_enabled&&running&&step_index==prior);
 assert(voice[0].gate&&voice[1].gate);
 ws_select(40);
 assert(wm.sound==8&&wm.layer_sound==41&&voice[0].gate&&voice[1].gate);
 assert(!memcmp(&ws_voice[0].p,&WS_PRESETS_3[8],sizeof(ws_patch_t)));
 assert(!memcmp(&ws_voice[1].p,&WS_PRESETS_3[0],sizeof(ws_patch_t)));
 uint32_t audition=note_on(70,100,1);
 assert(voice[audition].patch_source==PATCH_LAYER&&voice[audition].patch==40);
 editor_page=1;editor_adjust(0,-1);
 assert(wm.layer_patch_valid&&wm.layer_patch.cutoff<WS_PRESETS_3[40].cutoff);
 assert(!memcmp(&ws_voice[audition].p,&wm.layer_patch,sizeof(ws_patch_t)));
 assert(!memcmp(&ws_voice[0].p,&wm.patch,sizeof(ws_patch_t)));

 /* Mute affects Layer sequencer playback only. */
 settings_adjust(1,-1);
 assert(!wm.layer_enabled&&voice[0].gate&&!voice[1].gate&&voice[audition].gate);
 recording=1;capture_on_at(71,100,1,2);
 assert(wm.layer.e[2].note==71&&!wm.memory.e[2].velocity);
 settings_adjust(0,-1);assert(!layer_focus&&recording&&capture_count()==1);
 capture_on_at(72,100,1,3);
 assert(wm.memory.e[3].note==72&&!wm.layer.e[3].velocity&&capture_count()==2);
 audio_frames+=step_samples;
 capture_off(71,1,0);
 assert(wm.layer.e[2].gate==256u&&capture_count()==1);
 capture_off(72,1,0);
 assert(wm.memory.e[3].gate==256u&&!capture_count());
 /* MIDI retrigger across the switch finishes the previous lane's note.
  * Pedal-up closes the new lane's capture even after switching back. */
 settings_open=0;fm1_in.notes=0;
 midi_message(1,0x90,74,100);
 audio_frames+=step_samples;
 layer_target_locked(1);assert(capture_count()==1);
 midi_message(1,0x90,74,100);assert(capture_count()==1);
 midi_message(1,0xb0,64,127);midi_message(1,0x80,74,0);
 layer_target_locked(0);assert(capture_count()==1);
 audio_frames+=step_samples;
 midi_message(1,0xb0,64,0);assert(!capture_count());
 settings_open=1;
 recording_end();
 ws_select(12);assert(wm.sound==12&&wm.layer_sound==41);
 assert(wm.layer_patch_valid&&running&&step_index==prior);

 /* Linking retains the custom patch. A new preset on Layer splits it again. */
 settings_adjust(2,-1);assert(!ws_layer_own());
 settings_adjust(0,1);assert(ws_edit_sound()==12);
 settings_adjust(2,1);assert(ws_edit_sound()==40&&wm.layer_patch_valid);
 settings_adjust(2,-1);
 midi_message(1,0xc0,7,0);
 assert(ws_layer_own()&&wm.layer_sound==8&&wm.sound==12);
 /* Editor keeps a compatible page. An FM-only page cannot remain open
  * when switching to the analog layer. */
 ws_select(0);layer_target_locked(0);ws_select(0);
 editor_open=1;editor_page=4;editor_selected=2;
 layer_target_locked(1);assert(editor_page==4&&editor_selected==2);
 layer_target_locked(0);editor_page=7;editor_selected=3;
 layer_target_locked(1);assert(editor_page==0&&editor_selected==0);
 editor_open=0;
 /* A Layer 2-only project keeps its established transpose base. */
 wm_clear_phrase(&wm.memory);wm.root=7;wm.transpose_base=2;
 record_undo_pending=1;capture_on_at(75,100,1,5);
 assert(wm.transpose_base==2);capture_off(75,1,0);


 /* Old v14 slots use their original bank and mute, even with dirty padding. */
 static wm_state_t loaded;
 wm_disk_v14_t old={WM_MAGIC,14,WM_V14_SIZE,{0}};
 wm.layer_sound=35;wm.layer_enabled=0;wm.midi_channel=9;
 memcpy(old.state,&wm,WM_V14_USED);
 memset(old.state+WM_V14_USED,255,WM_V14_SIZE-WM_V14_USED);
 assert(!slot_save(1,&old,sizeof old)&&project_read_slot(1,&loaded));
 assert(loaded.layer_sound==35&&!loaded.layer_enabled&&loaded.layer_preset_bank==WS_BANK_2_0); /* v14 indexes keep their 2.0 sounds */
 assert(loaded.midi_channel==9&&!loaded.layer_link&&!loaded.layer_patch_valid);
 wm=loaded;editor_page=1;editor_adjust(0,-1);assert(wm.layer_patch_valid);
 static wm_disk_t saved;
 saved=(wm_disk_t){WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),wm};
 assert(wm_valid(&saved)&&!slot_save(2,&saved,sizeof saved)&&project_read_slot(2,&loaded));
 assert(loaded.layer_patch_valid&&!memcmp(&loaded.layer_patch,&wm.layer_patch,sizeof(ws_patch_t)));
 saved.state.layer_patch.engine=255;assert(!wm_valid(&saved));

 /* Confirm every Scatter Grid value fits alongside its label at the
  * production font size, including clearance for the glyph padding. */
 editor_page=EDIT_PATTERN;
 for(wm.scatter_pattern=0;wm.scatter_pattern<WM_PATTERN_COUNT;wm.scatter_pattern++){
  char buf[12];const char *unit;
  const char *value=editor_value(0,buf,&unit);
  assert(16+width(&FONT_T,editor_label(0))+2*FONT_PAD+6<=222-width(&FONT_T,value));
 }
 puts("Layer switch, independent preset and editor, mute, link, MIDI, migration and Scatter Grid fit pass.");
}
