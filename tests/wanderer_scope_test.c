/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#define DINGHY_HOST 1
#define main host_application_main
#include "../firmware/src/dinghy.c"
#undef main
static void block(void){int32_t out[512]={0};app_audio(out,256);}
static void clear(void)
{
    scope_set_open(0);scope_hold=ui_chord_latched=0;ui_pending=home_down=anchor_down=fx_down=0;
    settings_open=confirm=anchor_view=recording=running=0;wm=(wm_state_t)WM_DEFAULTS;
    cues_enabled=0;vol=100;master_smooth=25600;reset_performance();
    for(uint32_t k=0;k<70;k++)block();
}
int main(void)
{
    clear();confirm=2;project_slot=7;wm.has_anchor=1;anchor_view=1;recording=1;
    fm1_ms=10;fm1_in.buttons=1u<<BTN_SEL;assert(!ui_edges(1u<<BTN_SEL,0));
    fm1_ms=11;fm1_in.buttons|=1u<<BTN_HOME;assert(!ui_edges(1u<<BTN_HOME,0));
    assert(scope_open&&confirm==2&&project_slot==7&&anchor_view&&recording);
    assert(!home_down&&!ui_pending);
    assert(!ui_edges(0,0)&&scope_open);
    fm1_in.buttons=1u<<BTN_HOME;assert(!ui_edges(0,1u<<BTN_SEL));
    fm1_in.buttons=0;assert(!ui_edges(0,1u<<BTN_HOME));assert(!ui_chord_latched);
    fm1_ms=20;fm1_in.buttons=1u<<BTN_HOME;assert(!ui_edges(1u<<BTN_HOME,0));
    fm1_in.buttons|=1u<<BTN_SEL;assert(!ui_edges(1u<<BTN_SEL,0));assert(!scope_open);
    fm1_in.buttons=0;assert(!ui_edges(0,(1u<<BTN_SEL)|(1u<<BTN_HOME)));
    /* Solo release remains the original action; press cannot confirm a save. */
    assert(!ui_edges(1u<<BTN_SEL,0));assert(ui_edges(0,1u<<BTN_SEL)==(1u<<BTN_SEL));
    assert(!ui_edges(1u<<BTN_HOME,0));assert(ui_edges(0,1u<<BTN_HOME)==(1u<<BTN_HOME));
    puts("both chord orders, release isolation, underlying context and solo actions pass");
    clear();scope_set_open(1);scope_time=0;
    for(uint32_t i=0;i<3000;i++)scope_feed((i%100u)<50u?2000:-2000);
    scope_snapshot();assert(scope_generation>0&&scope_peak==2000);
    int16_t saved[SCOPE_N];memcpy(saved,scope_trace,sizeof saved);uint32_t gen=scope_generation;
    scope_hold=1;for(uint32_t i=0;i<3000;i++)scope_feed(123);scope_snapshot();
    assert(scope_generation==gen&&!memcmp(saved,scope_trace,sizeof saved));
    scope_hold=0;scope_reader=scope_ready;uint32_t reader=scope_reader;
    int16_t owned[SCOPE_N];memcpy(owned,scope_frames[reader],sizeof owned);
    for(uint32_t i=0;i<30000;i++)scope_feed(i&1u?3000:-3000);
    assert(!memcmp(owned,scope_frames[reader],sizeof owned));scope_reader=3;
    for(uint32_t i=0;i<6000;i++){scope_feed(0);}
    scope_snapshot();assert(scope_peak==0);
    puts("real sample capture, freeze, protected reader ownership and silent flatline pass");
    clear();scope_set_open(1);cues_enabled=1;cue(84);
    int32_t energy=0;
    for(uint32_t k=0;k<16;k++){int32_t b[512]={0};app_audio(b,256);for(uint32_t i=0;i<512;i++)energy|=b[i];}
    scope_snapshot();assert(energy&&scope_peak==0);
    cues_enabled=0;cue_left=0;cue(60);assert(!cue_left);
    assert(!preferences_save());cues_enabled=1;preferences_load();assert(!cues_enabled);
    cues_enabled=1;assert(!preferences_save());cues_enabled=0;preferences_load();assert(cues_enabled);
    puts("earcons stay out of trace, off suppresses new cues, saved on/off preferences pass");
    clear();scope_set_open(1);note_on(60,127,1);for(uint32_t k=0;k<30;k++)block();
    scope_snapshot();assert(scope_peak>100);panic();for(uint32_t k=0;k<100;k++)block();scope_snapshot();assert(!scope_peak);
    puts("live instrument trace and panic flatline pass");
    /* Stress extreme display time/gain without modifying musical state. */
    for(uint32_t i=0;i<10000;i++){
        uint8_t r=running,rec=recording;scope_set_open((uint8_t)(i&1u));
        assert(running==r&&recording==rec);
    }
    puts("10000 scope open/close cycles preserve musical state");
    puts("RC2 SCOPE, CHORD AND EARCON TESTS PASSED");
}
