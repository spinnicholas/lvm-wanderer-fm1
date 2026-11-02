/* SPDX-License-Identifier: GPL-3.0-only
 * Real recording durations, musical evolution, persistence migration and DSP. */
#include <assert.h>
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static void blocks(uint32_t count)
{
    while(count--){int32_t out[512]={0};app_audio(out,256);
        for(uint32_t j=0;j<512;j++)assert(out[j]>=-32767&&out[j]<=32767);
    }
}
static void fresh(void)
{
    capture_finish_all();wm=(wm_state_t)WM_DEFAULTS;wm.legacy=1; /* Legacy bank compatibility. */
    scope_open=scope_hold=settings_open=confirm=anchor_view=running=recording=0;
    step_index=step_clock=cycle_index=step_primed=0;audio_frames=0;
    step_samples=FS*15u/wm.bpm;vol=100;cues_enabled=0;
    reset_performance();blocks(64);
}
int main(void)
{
    fresh();
    assert(sizeof(wm_state_legacy_t)==144&&sizeof(wm_state_v3_t)==152&&sizeof(wm_disk_v3_t)==160);
    for(uint32_t scale=0;scale<WM_SCALE_COUNT;scale++)for(uint32_t root=0;root<12;root++){
        wm.scale=(uint8_t)scale;wm.root=(uint8_t)root;wm.drift=127;
        for(uint32_t note=0;note<128;note++)for(uint32_t phase=0;phase<8;phase++){
            wm_event_t e={(uint8_t)note,100,1500};
            wm_event_t t=wm_evolve(e,3,phase);
            assert(t.note<=127&&t.gate==1500);
            if(t.note!=note)assert(wm_in_scale(t.note));
            if(!phase)assert(t.note==note);
            assert(abs((int)t.note-(int)note)<=7);
        }
    }
    fresh();wm_record(0,60,100);wm.memory.e[0].gate=1400;
    assert(wm_catch());wm.drift=127;wm.melt=70;
    assert(wm_branch_now(4));assert(wm.anchor.e[0].note==60&&wm.anchor.e[0].gate==1400);
    assert(wm_undo_once()&&wm.drift==127);assert(!wm_undo_once());
    puts("98304 key/scale/note/phase cases: scale-bound moves, bounded intervals, phrase resolution, duration and Anchor preservation");
    fresh();running=recording=1;
    capture_on(60,100,1);blocks(120);capture_off(60,1,0);
    uint32_t ticks=wm.memory.e[0].gate;
    assert(ticks>1200&&ticks<1500);assert(!capture[0].active);
    running=recording=0;reset_performance();wm.fx_on=0;blocks(64);
    wm.sound=19;sequence_event(wm.memory.e[0],0);
    uint32_t v=0;while(v<NV&&!voice[v].gate)v++;assert(v<NV);
    assert(voice[v].remain>step_samples*4);
    blocks(50);assert(voice[v].gate);blocks(80);assert(!voice[v].gate);
    puts("held local note records ~700 ms and replays through multiple steps before release");
    fresh();wm_set_length(32);running=recording=1;step_primed=1;step_index=31;step_clock=0;
    capture_on(65,110,1);blocks(130);capture_off(65,1,0);
    assert(wm.memory.e[30].gate>1300);assert(!wm.memory.e[0].velocity);
    for(uint32_t i=0;i<64;i++){step_play();assert(step_index<32);}
    assert(wm_record(31,72,100));assert(wm_set_length(16));assert(wm.memory.e[31].velocity);
    assert(wm_undo_once()&&wm.loop_steps==32&&wm.memory.e[31].velocity);
    puts("32-step second-bar and boundary-spanning recording; shortening preserves hidden events; Undo restores length");
    fresh();running=recording=1;
    midi_message(1,0x90,60,100);blocks(25);
    midi_message(2,0x80,60,0);assert(capture[0].active);
    midi_message(1,0xb0,64,127);midi_message(1,0x80,60,0);blocks(80);assert(capture[0].active);
    midi_message(1,0xb0,64,0);assert(!capture[0].active&&wm.memory.e[0].gate>1100);
    capture_on(62,100,1);blocks(20);transport();assert(!capture[0].active);
    fresh();running=recording=1;capture_on(60,100,1);blocks(10);
    capture_on(67,100,1);assert(capture_count()==2);blocks(10);
    capture_off(60,1,0);assert(capture_count()==1);capture_off(67,1,0);assert(!capture_count());
    fresh();running=recording=1;capture_on(60,100,1);blocks(600);
    assert(wm.memory.e[0].gate==4096);panic();assert(!capture[0].active);
    puts("MIDI source isolation, sustain pedal, replaced note, transport finalization and whole-loop hold cap pass");
    fresh();wm_set_length(32);wm.root=7;wm.scale=3;wm_record(29,69,112);wm.memory.e[29].gate=1800;
    for(uint32_t i=0;i<NSLOT;i++){
        project_slot=(uint8_t)i;confirm=2;apply_confirm();assert(notice==N_SAVED);
        wm.memory.e[29].gate=1;confirm=3;apply_confirm();
        assert(notice==N_LOADED&&wm.loop_steps==32&&wm.root==7&&wm.scale==3&&wm.memory.e[29].gate==1800);
    }
    wm_disk_v3_t old={0};old.magic=WM_MAGIC;old.version=3;old.size=sizeof old.state;
    old.state.magic=WM_MAGIC;old.state.bpm=120;old.state.sound=76;old.state.fx_on=7;
    old.state.memory.e[15]=(wm_event_v3_t){72,105};old.state.has_anchor=1;old.state.anchor=old.state.memory;
    assert(wm_valid_v3(&old));slot_save(0,&old,sizeof old);project_slot=0;confirm=3;apply_confirm();
    assert(notice==N_LOADED&&wm.loop_steps==16&&wm.scale==0&&wm.sound==76);
    assert(wm.memory.e[15].gate==192&&wm.memory.e[15].note==72&&wm.anchor.e[15].velocity==105&&!wm.memory.e[16].velocity);
    old.state.memory.e[15].note=255;assert(!wm_valid_v3(&old));
    wm_disk_legacy_t legacy={0};legacy.magic=WM_MAGIC;legacy.version=2;legacy.size=sizeof legacy.state;
    legacy.state.magic=WM_MAGIC;legacy.state.bpm=100;legacy.state.memory.e[0]=(wm_event_v3_t){60,100};
    assert(wm_valid_legacy(&legacy));slot_save(1,&legacy,sizeof legacy);project_slot=1;confirm=3;apply_confirm();
    assert(notice==N_LOADED&&wm.memory.e[0].note==60&&wm.memory.e[0].gate==192);
    legacy.version=1;legacy.state.sound=255;assert(wm_valid_legacy(&legacy));
    wm_disk_t d={WM_MAGIC,WM_DISK_VERSION,sizeof wm,wm};assert(wm_valid(&d));
    d.state.loop_steps=17;assert(!wm_valid(&d));d.state.loop_steps=16;
    d.state.root=12;assert(!wm_valid(&d));d.state.root=0;
    d.state.scale=8;assert(!wm_valid(&d));d.state.scale=2;
    d.state.memory.e[0].gate=8193;assert(!wm_valid(&d));
    puts("all eight v4 slots and v1/v2/v3 imports pass; malformed musical metadata rejected");
    fresh();wm.sound=72;wm.fx_on=0;note_on(60,100,1);blocks(280);
    uint32_t amp=0;for(uint32_t k=0;k<NV;k++)amp+=voice[k].amp;
    assert(amp>1000000);note_off(60,1);blocks(100);uint32_t tail=0;for(uint32_t k=0;k<NV;k++)tail+=voice[k].amp;
    assert(tail>0&&tail<amp);blocks(900);for(uint32_t k=0;k<NV;k++)assert(!voice[k].amp);
    puts("slow pad sustains after 1.6 s, releases gradually and reaches silence");
    fresh();
    for(uint32_t theme=0;theme<2;theme++)for(uint32_t zoom=0;zoom<2;zoom++)for(uint32_t scale=0;scale<8;scale++){
        wm.light=theme;wm.extra_large=zoom;wm.scale=scale;settings_open=1;settings_page=3;
        for(settings_index=0;settings_index<3;settings_index++){
            for(uint32_t root=0;root<12;root++){wm.root=root;screen();}
        }
    }
    settings_open=0;wm.loop_steps=32;screen();wm.extra_large=0;screen();
    puts("1152 Music page renders plus 32-step Home views fit 240x240");
    puts("WANDERER 1.1 MUSICAL RECORDING AND ENGINE TESTS PASSED");
}
