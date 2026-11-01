/* SPDX-License-Identifier: GPL-3.0-only
 * Exercise the actual app, DSP, MIDI, save and LED functions under sanitizers.
 */
#include <assert.h>
#include <limits.h>
#define DINGHY_HOST 1
#define main wanderer_scenario_main
#include "../firmware/src/dinghy.c"
#undef main

static void render(uint32_t blocks)
{
    int32_t out[512];
    while(blocks--){
        memset(out,0,sizeof out);app_audio(out,256);
        for(uint32_t i=0;i<512u;i++)assert(out[i]>=-32767&&out[i]<=32767);
    }
}
static uint32_t owned(uint32_t owner)
{uint32_t n=0;for(uint32_t v=0;v<NV;v++)n+=voice[v].gate&&voice[v].live==owner;return n;}
static void fresh(void)
{
    wm=(wm_state_t)WM_DEFAULTS;running=recording=anchor_view=confirm=settings_open=0;
    octave=project_slot=0;step_samples=FS*15u/wm.bpm;
    vol=100;cues_enabled=0;wm_can_undo=0;reset_performance();render(64);
}
int main(void)
{
    fresh();
    for(uint32_t n=0;n<1000u;n++){cue(n);assert(tone_step(n)>0u);}
    cues_enabled=1;cue(555u);assert(cue_step==tone_step(127));cue_left=0;
    /* USB ch1, USB ch2, TRS ch1 and local keys all share pitch 60 safely. */
    midi_message(1,0x90,60,100);midi_message(1,0x91,60,110);
    midi_message(2,0x90,60,120);note_on(60,100,1);
    assert(owned(2)==1&&owned(3)==1&&owned(18)==1&&owned(1)==1);
    midi_message(1,0x80,60,0);assert(!owned(2)&&owned(3)&&owned(18)&&owned(1));
    midi_message(2,0x90,60,0);assert(!owned(18)&&owned(3)&&owned(1));
    midi_message(1,0xb1,64,127);midi_message(1,0x81,60,0);assert(owned(3));
    midi_message(1,0xb0,64,0);assert(owned(3));
    midi_message(1,0xb1,64,0);assert(!owned(3));
    note_off(60,1);assert(!owned(1));
    midi_message(1,0x90,61,100);midi_message(1,0x90,61,120);assert(owned(2)==1);
    midi_message(1,0xc0,99,0);assert(wm.sound==99);midi_message(1,0xc0,127,0);assert(wm.sound==99);
    midi_message(1,0xb0,1,87);assert(wm.melt==87);
    midi_message(1,0xb0,123,0);assert(!owned(2));
    puts("MIDI notes, sources, channels, sustain, velocity-zero, retrigger, preset and CC pass");
    fresh();
    step_primed=1;step_index=1;step_clock=100;assert(capture_step()==0);
    step_clock=step_samples-100;assert(capture_step()==1);
    step_index=0;step_clock=100;assert(capture_step()==15);
    wm_record(0,60,100);assert(wm_catch());wm.memory.e[0].note=67;wm_mark();
    confirm=5;apply_confirm();assert(wm.memory.e[0].note==60&&wm.anchor.e[0].note==60);
    assert(wm_undo_once()&&wm.memory.e[0].note==67);
    puts("nearest-step recording, Anchor restore and undo pass");
    fresh();wm.layer.e[0]=(wm_event_t){72,100};step_index=0;step_play();assert(!owned(0));
    wm.layer_enabled=1;step_index=0;step_play();assert(owned(0)==1);
    puts("layer switch gates sequenced playback as well as selecting the recording lane");
    /* Eight real app save/load paths, CRC model plus readback, current v4 schema. */
    for(uint32_t slot=0;slot<NSLOT;slot++){
        project_slot=(uint8_t)slot;wm.sound=(uint8_t)(slot*11);wm.memory.e[0].note=(uint8_t)(40+slot);
        confirm=2;apply_confirm();assert(notice==N_SAVED);
    }
    for(uint32_t slot=0;slot<NSLOT;slot++){
        project_slot=(uint8_t)slot;confirm=3;apply_confirm();
        assert(notice==N_LOADED&&wm.sound==slot*11&&wm.memory.e[0].note==40+slot);
    }
    wm.sound=91;running=1;confirm=2;apply_confirm();assert(notice==N_STOP_FIRST);running=0;
    confirm=3;apply_confirm();assert(wm.sound==77);
    host_slot[7][4+8]=0;confirm=3;apply_confirm();assert(notice==N_NO_VALID_SAVE&&wm.sound==77);
    vol=37;cues_enabled=0;wm.light=wm.extra_large=wm.reduced_motion=1;
    assert(!preferences_save());vol=100;wm.light=wm.extra_large=wm.reduced_motion=0;cues_enabled=1;
    preferences_load();assert(vol==37&&!cues_enabled&&wm.light&&wm.extra_large&&wm.reduced_motion);
    /* No boot auto-write; a malformed settings record cannot modify defaults. */
    host_set[4]=0;vol=62;preferences_load();assert(vol==62);
    puts("eight independent slots, stopped-only saves, invalid-load rejection and preferences pass");
    fresh();
    wm.has_anchor=1;wm.layer_enabled=1;anchor_view=1;wm_can_undo=1;octave=1;running=recording=1;
    notice=0;
    host_leds=host_glow=((uint64_t)1<<41)-1;leds_initialised=0;
    app_leds();assert(host_leds==((1u<<1)|(1u<<5)|(1u<<6)|(1u<<9)|(1u<<12)|(1u<<13)));
    settings_open=1;app_leds();assert(host_leds==((1u<<3)|(1u<<12))&&host_glow==(1u<<8));
    settings_open=0;confirm=2;app_leds();assert(host_leds==((1u<<3)|(1u<<11)|(1u<<12)));
    confirm=3;app_leds();assert(host_leds==((1u<<3)|(1u<<10)|(1u<<12)));
    confirm=1;app_leds();assert(host_leds==((1u<<2)|(1u<<3)|(1u<<12)));
    confirm=5;app_leds();assert(host_leds==((1u<<3)|(1u<<5)|(1u<<12)));
    for(uint32_t i=0;i<10000;i++){
        settings_open=i%3==0;confirm=settings_open?0:(i%6);app_leds();
        assert(!(host_leds>>41));
        if(settings_open)assert(host_leds==((1u<<3)|(1u<<12)));
    }
    panic();settings_open=0;wm.has_anchor=wm.layer_enabled=wm_can_undo=anchor_view=octave=0;
    app_leds();assert(!host_leds&&!host_glow);
    note_on(60,100,1);app_leds();assert(host_leds==((uint64_t)1<<21));
    note_off(60,1);app_leds();assert(!host_leds);
    puts("complete LED scenes, stale-mask cleanup, 10000 view transitions and note releases pass");
    /* Render every sound at the lowest and highest MIDI notes and full four-voice FX load. */
    fresh();wm.fx_on=7;wm.chorus_mix=wm.delay_mix=wm.reverb_mix=127;
    wm.delay_feedback=wm.reverb_size=127;
    for(uint32_t p=0;p<100u;p++){
        wm.sound=(uint8_t)p;
        for(uint32_t n=0;n<128u;n++){
            note_on(n,127,1);render(1);note_off(n,1);
        }
    }
    render(128);vol=0;render(64);
    int32_t out[512]={0};app_audio(out,256);
    for(uint32_t i=0;i<512u;i++)assert(out[i]==0);
    panic();render(64);assert(!fx_clearing);
    vol=100;render(32);memset(out,0,sizeof out);app_audio(out,256);
    for(uint32_t i=0;i<512u;i++)assert(out[i]==0);
    puts("12800 preset/note pairs, full-wet bounds, complete master mute and panic silence pass");
    puts("WANDERER CORE RELEASE TESTS PASSED");
}
