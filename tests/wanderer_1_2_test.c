/* Actual app: anticipated player frustrations become concrete regression cases. */
#include <assert.h>
#define DINGHY_HOST 1
#define main host_main
#include "../firmware/src/dinghy.c"
#undef main
static void blocks(uint32_t count)
{while(count--){int32_t out[512]={0};app_audio(out,256);for(uint32_t i=0;i<512;i++)assert(abs(out[i])<=32767);}}
static void fresh(void)
{
    capture_finish_all();wm=(wm_state_t)WM_DEFAULTS;recording=running=anchor_view=confirm=settings_open=scope_open=scope_hold=0;
    record_undo_pending=0;step_index=step_clock=cycle_index=step_primed=0;
    audio_frames=fm1_ms=0;step_samples=FS*15u/wm.bpm;vol=100;cues_enabled=0;
    notice=focus=wm_can_undo=save_pending=0;save_down=0;
    host_fault_settings_save=host_fault_slot_save=host_fault_slot_load=0;
    reset_performance();blocks(64);project_initialise(1);
}
static uint32_t lane_voices(uint32_t lane)
{uint32_t n=0;for(uint32_t v=0;v<NV;v++)n+=voice[v].gate&&!voice[v].live&&voice[v].seq_lane==lane;return n;}
static uint8_t contains(const wm_phrase_t *p,uint32_t step,uint32_t note)
{for(uint32_t v=0;v<WM_POLY;v++){wm_event_t e=wm_get(p,step,v);if(e.velocity&&e.note==note)return 1;}return 0;}
static uint32_t gate_for(const wm_phrase_t *p,uint32_t step,uint32_t note)
{for(uint32_t v=0;v<WM_POLY;v++){wm_event_t e=wm_get(p,step,v);if(e.velocity&&e.note==note)return e.gate;}return 0;}
int main(void)
{
    fresh();
    recording_toggle();capture_on(60,100,1);capture_on(63,95,1);capture_on(67,105,1);
    assert(capture_count()==3&&wm_step_notes(&wm.memory,0)==3);blocks(40);
    capture_off(63,1,0);assert(capture_count()==2);blocks(40);
    capture_off(60,1,0);capture_off(67,1,0);recording_toggle();assert(!recording&&!capture_count());
    assert(gate_for(&wm.memory,0,63)>400&&gate_for(&wm.memory,0,63)<500);
    assert(gate_for(&wm.memory,0,60)>800&&gate_for(&wm.memory,0,67)>800);
    wm_phrase_t chord=wm.memory;running=0;reset_performance();step_index=0;step_play();
    assert(lane_voices(0)==3);blocks(42);assert(lane_voices(0)==2);blocks(45);assert(!lane_voices(0));
    assert(wm_catch());wm.scale=2;wm.drift=127;for(uint32_t l=0;l<8;l++)ww_loop();assert(ww_keep()); /* 3.1: Keep replaces branch */
    assert(wm_step_notes(&wm.memory,0)==3&&!memcmp(&wm.anchor,&chord,sizeof chord));
    for(uint32_t v=0;v<3;v++){wm_event_t e=wm_get(&wm.memory,0,v);assert(wm_in_scale(e.note));}
    puts("three-note chords retain independent held durations, replay polyphonically and survive Anchor/scale evolution/branch");
    fresh();recording_toggle();capture_on(60,100,1);capture_on(62,100,1);capture_on(64,100,1);capture_on(65,100,1);capture_on(67,100,1);capture_on(69,100,1);
    assert(capture_count()==4&&wm_step_notes(&wm.memory,0)==4);
    assert(!contains(&wm.memory,0,60)&&!contains(&wm.memory,0,62));
    assert(contains(&wm.memory,0,64)&&contains(&wm.memory,0,65)&&contains(&wm.memory,0,67)&&contains(&wm.memory,0,69));
    capture_off(60,1,0);assert(capture_count()==4);recording_end();
    fresh();wm_record(4,60,100);*wm_at(&wm.memory,4,1)=(wm_event_t){67,100,500};
    recording_toggle();capture_on(60,100,1);blocks(130);capture_off(60,1,0);
    assert(!contains(&wm.memory,4,60)&&contains(&wm.memory,4,67));
    puts("five/six-note overflow keeps four recent starts; held overdubs clear covered repeats without erasing other chord pitches");
    fresh();recording_toggle();midi_message(1,0x90,60,100);midi_message(1,0x90,64,100);midi_message(1,0x90,67,100);
    midi_message(1,0xb0,64,127);midi_message(1,0x80,60,0);midi_message(1,0x80,64,0);blocks(80);
    midi_message(2,0xb0,64,0);assert(capture_count()==3);
    midi_message(1,0xb0,64,0);assert(capture_count()==1);midi_message(1,0x80,67,0);assert(!capture_count());
    fresh();wm.loop_steps=32;step_primed=1;step_index=31;recording_toggle();step_primed=1;step_index=31;
    capture_on(55,100,1);capture_on(62,100,1);blocks(130);capture_finish_all();
    assert(wm_step_notes(&wm.memory,30)==2&&gate_for(&wm.memory,30,55)>1300);
    puts("polyphonic MIDI sustain/source isolation and two-bar wrap keep every captured gate owned correctly");
    fresh();for(uint32_t v=0;v<4;v++)*wm_at(&wm.memory,0,v)=(wm_event_t){60+v*3,100,800};
    wm.layer_enabled=1;wm.layer.e[0]=(wm_event_t){36,100,800};
    step_index=0;step_play();
    assert(lane_voices(0)==3&&lane_voices(1)==1);
    fresh();wm.memory.e[0]=(wm_event_t){48,100,800};wm.layer_enabled=1;
    for(uint32_t v=0;v<4;v++)*wm_at(&wm.layer,0,v)=(wm_event_t){60+v*3,100,800};
    step_index=0;step_play();
    assert(lane_voices(0)==1&&lane_voices(1)==3);
    puts("simultaneous Memory/Layer chord starts share the four sound voices predictably");
    fresh();wm_mark();wm.memory.e[0]=(wm_event_t){60,100,800};recording_toggle();recording_toggle();
    assert(wm_undo_once()&&!wm_has_notes(&wm.memory));
    fresh();recording_toggle();capture_on(60,100,1);capture_on(63,100,1);capture_on(67,100,1);blocks(100);recording_toggle();transport();
    project_poll();assert(project_dirty&&project_origin_slot==255);
    project_slot=2;confirm=2;apply_confirm();assert(notice==N_SAVED&&notice_sticky()&&!notice_until);
    assert(!project_dirty&&project_origin_slot==2&&slot_presence[2]==2);
    assert(wm_undo_once());project_poll();assert(project_dirty&&project_origin_slot==2&&!wm_has_notes(&wm.memory));
    confirm=3;apply_confirm();assert(notice==N_LOADED&&wm_step_notes(&wm.memory,0)==3&&!project_dirty);
    assert(wm_undo_once());project_poll();assert(project_dirty&&!wm_has_notes(&wm.memory)&&project_origin_slot==2);
    confirm=3;apply_confirm();vol=37;project_poll();assert(project_dirty);
    host_fault_settings_save=1;confirm=2;apply_confirm();assert(notice==N_PREF_FAILED&&preferences_pending&&project_dirty&&slot_presence[2]==2);
    wm_state_t saved;assert(project_read_slot(2,&saved)&&wm_step_notes(&saved.memory,0)==3);
    confirm=2;apply_confirm();assert(notice==N_SAVED&&!preferences_pending&&!project_dirty&&clean_level==37);
    wm.memory.e[0].note=61;project_poll();host_fault_slot_save=1;confirm=2;apply_confirm();assert(notice==N_SAVE_FAILED&&project_dirty&&slot_presence[2]==0);
    host_fault_slot_load=1;confirm=2;apply_confirm();assert(notice==N_SAVE_FAILED&&project_dirty&&slot_presence[2]==0);
    assert(project_read_slot(2,&saved)); /* Written bytes may be valid; unverified is still not success. */
    puts("empty record passes retain Undo; Save establishes a reference; Undo after Save is dirty; preferences failure never falsely reports project loss or a clean session");
    /* 3.1: projects round-trip in the current format; anything else is never loaded or rewritten. */
    fresh();wm.loop_steps=32;wm.root=7;wm.scale=3;ws_select(73);
    wm.memory.e[31]=(wm_event_t){72,110,1800};wm_protect_source();
    project_slot=0;confirm=2;apply_confirm();assert(notice==N_SAVED);
    wm=(wm_state_t)WM_DEFAULTS;project_slot=0;confirm=3;apply_confirm();assert(notice==N_LOADED&&wm.loop_steps==32&&wm.root==7&&wm.scale==3&&wm.sound==73);
    assert(wm.memory.e[31].gate==1800&&!wm.memory.extra[31][0].velocity&&wm.has_anchor);
    {static uint8_t foreign[64];memset(foreign,0x5a,sizeof foreign);assert(!slot_save(5,foreign,sizeof foreign));
     uint8_t original[sizeof foreign+4];memcpy(original,host_slot[5],sizeof original);
     project_slot=5;confirm=3;apply_confirm();assert(notice==N_NO_VALID_SAVE&&!memcmp(original,host_slot[5],sizeof original));}
    project_slot=1;confirm=2;apply_confirm();wm.memory.e[31].note=0;confirm=3;apply_confirm();assert(wm.memory.e[31].note==72);
    uint32_t writes=host_store_writes;wm=(wm_state_t)WM_DEFAULTS;project_initialise(1);assert(host_store_writes==writes&&slot_presence[0]==2&&slot_presence[1]==2&&slot_presence[5]==1&&!wm_has_notes(&wm.memory)&&project_origin_slot==255);
    puts("current-format projects round-trip 32-step held notes/key/sound; foreign data is refused untouched; boot caches slot validity without writing");
    fresh();wm_record(0,60,100);fm1_in.buttons=1u<<BTN_SAVE;
    assert(!(save_edges(1u<<BTN_SAVE,0)&(1u<<BTN_SAVE)));fm1_ms=901;save_edges(0,0);
    assert(confirm==2&&!wm.has_anchor&&!save_pending);fm1_in.buttons=0;assert(!save_edges(0,1u<<BTN_SAVE));
    fresh();wm_record(0,60,100);fm1_in.buttons=1u<<BTN_SAVE;save_edges(1u<<BTN_SAVE,0);fm1_ms=50;fm1_in.buttons=0;
    assert(save_edges(0,1u<<BTN_SAVE)&(1u<<BTN_SAVE));assert(wm_catch());
    puts("short Save remains Catch; held Save opens project Save without an intermediate Catch or a release action");
    fresh();wm.light=0;wm_record(0,60,100);wm_mark();wm.memory.e[0].note=72;wm.light=wm.extra_large=wm.reduced_motion=1;
    assert(wm_undo_once()&&wm.memory.e[0].note==60&&wm.light&&wm.extra_large&&wm.reduced_motion);
    for(uint32_t theme=0;theme<2;theme++)for(uint32_t zoom=0;zoom<2;zoom++){
        wm.light=theme;wm.extra_large=zoom;confirm=settings_open=notice=focus=scope_open=0;vol=0;
        for(uint32_t state=0;state<3;state++){project_dirty=state==1;project_origin_slot=state==0?255:7;screen();}
        for(notice=N_SAVED;notice<=N_PREF_FAILED;notice++)screen();
        notice=0;
        for(uint32_t cached=0;cached<3;cached++){slot_presence[project_slot]=cached;confirm=2;screen();confirm=3;screen();}confirm=0;
    }
    puts("Undo preserves accessibility choices; project/mute/length footer and save outcomes render in both themes and sizes");
    fresh();wm.loop_steps=32;wm.sound=72;wm.fx_on=7;
    wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;wm.delay_feedback=wm.reverb_size=127;
    vol=127;recording_toggle();uint32_t random=0x12c0ffeeu;
    for(uint32_t t=0;t<8192;t++){
        random=random*1664525u+1013904223u;
        uint8_t source=(random>>31)+1u,channel=(random>>26)&3u,note=48u+((random>>16)%25u);
        if((random&7u)<4u)midi_message(source,0x90u|channel,note,70u+(random>>8)%58u);
        else if((random&7u)<7u)midi_message(source,0x80u|channel,note,0);
        else midi_message(source,0xb0u|channel,64u,(random&256u)?127u:0u);
        blocks(1);assert(capture_count()<=WM_POLY);
        if(!(t%128u)){
            wm_disk_t check={WM_MAGIC,WM_DISK_VERSION,sizeof wm,wm};assert(wm_valid(&check));
        }
    }
    for(uint32_t source=1;source<=2;source++)for(uint32_t channel=0;channel<4;channel++)midi_message(source,0xb0u|channel,120,0);
    recording_end();assert(!capture_count());transport();reset_performance();blocks(128);
    puts("8192 rapid MIDI/sustain/overdub blocks across sources/channels, two-bar wraps and maximum effects retain valid events and bounded filters; all captures release");
    fresh();wm.has_anchor=wm.layer_enabled=layer_focus=1;octave=1;
    wm.memory.e[0]=(wm_event_t){60,100,800};wm_mark();wm_can_undo=1;
    const uint8_t statuses[]={N_SAVED,N_LOADED,N_SAVE_FAILED,N_PREF_FAILED,N_NO_VALID_SAVE,N_CATCH_FIRST,N_STOP_FIRST};
    for(uint32_t r=0;r<2;r++)for(uint32_t i=0;i<sizeof statuses;i++){
        notice=statuses[i];recording=running=(uint8_t)r;step_primed=1;step_index=1;step_clock=0;app_leds();
        uint64_t expected=1u<<BTN_SEL;
        if(r)expected|=(1u<<BTN_REC)|(1u<<BTN_PLAY);
        if(notice==N_SAVED||notice==N_SAVE_FAILED||notice==N_PREF_FAILED)expected|=1u<<BTN_SEQ;
        if(notice==N_LOADED||notice==N_NO_VALID_SAVE)expected|=1u<<BTN_ARP;
        assert(host_leds==expected&&host_glow==((1u<<BTN_HOME)|(1u<<BTN_FX)));
    }
    notice=0;recording=running=0;app_leds();
    assert((host_leds&(1u<<BTN_EDIT))&&(host_leds&(1u<<BTN_SAVE))&&!(host_leds&(1u<<BTN_SEL)));
    puts("persistent result/error LED scenes clear unrelated Home state, retain real transport/recording, and restore Home after acknowledgment");
    puts("WANDERER 1.2 PRODUCT REFINEMENT TESTS PASSED");
}
