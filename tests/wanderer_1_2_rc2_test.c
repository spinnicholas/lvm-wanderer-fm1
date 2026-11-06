/* SPDX-License-Identifier: GPL-3.0-only
 * Reproducers for chord replacement, counter wrap and listening transitions. */
#include <assert.h>
#ifndef WANDERER_APP
#define WANDERER_APP "../firmware/src/dinghy.c"
#endif
#define DINGHY_HOST 1
#define main host_main
#include WANDERER_APP
#undef main
static void fresh(void)
{
    wm=(wm_state_t)WM_DEFAULTS;recording=running=anchor_view=confirm=settings_open=notice=scope_open=0;
    capture_serial=audio_frames=step_index=step_clock=step_primed=0;
    step_samples=FS*15u/wm.bpm;cues_enabled=0;reset_performance();project_initialise(1);recording_toggle();
}
static uint8_t has(uint32_t note)
{for(uint32_t v=0;v<WM_POLY;v++)if(wm_get(&wm.memory,0,v).velocity&&wm_get(&wm.memory,0,v).note==note)return 1;return 0;}
static uint8_t active(uint32_t note)
{for(uint32_t c=0;c<CAPTURE_COUNT;c++)if(capture[c].active&&capture[c].note==note)return 1;return 0;}
static void fill(void)
{capture_on(60,100,1);capture_on(62,100,1);capture_on(64,100,1);capture_on(65,100,1);}
int main(int argc,char **argv)
{
    uint32_t only=argc>1?(uint32_t)atoi(argv[1]):0;
    if(!only||only==1){
        fresh();fill();capture_on(65,100,1);capture_on(67,100,1);capture_on(62,100,1);capture_on(69,100,1);
        assert(has(62)&&has(65)&&has(67)&&has(69)&&!has(60)&&!has(64));
        puts("full chord replaces oldest held onset after retriggers, preserving recently played notes");
    }
    if(!only||only==2){
        fresh();fill();capture_off(62,1,0);capture_on(67,100,1);
        assert(has(60)&&has(64)&&has(65)&&has(67)&&!has(62)&&active(60));
        puts("full chord reuses released positions before shortening another held note");
    }
    if(!only||only==3){
        fresh();capture_serial=0xfffffffdu;fill();step_primed=1;step_index=2;step_clock=0;capture_on(72,100,1);
        assert(!active(60)&&active(62)&&active(64)&&active(65)&&active(72));
        puts("capture-pool replacement preserves chronology across unsigned serial wrap");
    }
#ifndef OLD_REPRO_ONLY
    if(!only||only==4){
        fresh();step_primed=1;step_index=1;step_clock=step_samples/2u-1u;
        uint32_t at=capture_step();assert(at==0);capture_on_at(60,100,1,at);
        step_clock=step_samples/2u+1u;assert(capture_step()==1);
        capture_on_at(64,100,1,at);capture_on_at(67,100,1,at);
        assert(wm_step_notes(&wm.memory,0)==3&&!wm_step_notes(&wm.memory,1));
        recording_end();step_index=0;reset_performance();step_play();uint32_t n=0;
        for(uint32_t v=0;v<NV;v++)n+=voice[v].gate!=0;
        assert(n==3);
        puts("local chord batch retains one quantized onset while scheduler crosses a half-step");
    }
    if(!only||only==5){
        fresh();sequence_event((wm_event_t){60,100,3000},0);sequence_event((wm_event_t){55,100,3000},1);
        note_on(72,100,1);capture_on(72,100,1);anchor_view=1;listening_set(0);
        assert(!anchor_view&&!recording&&!capture_count());uint32_t live=0;
        for(uint32_t v=0;v<NV;v++)if(voice[v].gate){assert(voice[v].live==1&&voice[v].note==72);live++;}
        assert(live==1);note_off(72,1);
        puts("HOME listening return releases previous sequence and finishes capture while preserving live keys");
    }
#endif
    return 0;
}
