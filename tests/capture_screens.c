/* SPDX-License-Identifier: GPL-3.0-only
 * Deterministic captures from the production renderer, never drawn mockups.
 */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static void shot(const char *name)
{
    char path[256];snprintf(path,sizeof path,"build/screens/%s.ppm",name);
    project_poll();screen();app_leds();write_ppm_as(path);
    printf("%s bright=%llu dim=%llu\n",name,(unsigned long long)host_leds,(unsigned long long)host_glow);
}
int main(void)
{
    project_initialise();fm1_ms=100;shot("01-home-dark");
    wm_record(0,60,100);wm_record(4,64,100);wm_record(8,67,100);wm_record(12,72,100);
    wm_catch();wm.layer_enabled=layer_focus=1;wm_record(2,55,90);wm_record(10,62,90);
    shot("02-home-phrase-layer");
    running=1;step_index=4;shot("03-home-playing");
    recording=1;shot("04-home-recording");recording=0;running=0;
    anchor_view=1;shot("05-anchor-listening");
    wm.extra_large=1;shot("06-anchor-zoom");anchor_view=0;shot("07-memory-zoom");
    wm.extra_large=0;wm.light=1;shot("08-home-light");wm.extra_large=1;shot("09-zoom-light");
    wm.extra_large=wm.light=0;
    settings_open=1;
    for(settings_page=0;settings_page<3;settings_page++){
        settings_index=0;
        shot(settings_page==0?"10-settings-system":settings_page==1?"11-settings-effects":"12-settings-performance");
    }
    settings_page=2;settings_index=2;shot("13-delay-division");
    settings_page=SETTINGS_REVERB;settings_index=2;wm.reverb_size=96;shot("14-reverb-size");
    settings_page=SETTINGS_ACCESS_FEEL;settings_index=1;cues_enabled=0;shot("15-cues-disabled");cues_enabled=1;
    wm.light=1;settings_page=0;shot("16-settings-light");wm.light=0;settings_open=0;
    project_slot=7;
    for(confirm=1;confirm<=5;confirm++){
        const char *names[]={"","17-confirm-clear-layer","18-confirm-save-slot8","19-confirm-load-slot8","20-confirm-new-session","21-confirm-restore-anchor"};
        shot(names[confirm]);
    }
    confirm=0;focus_until=2000;
    for(focus=1;focus<=7;focus++){
        if(focus==6)continue;
        focus_value=focus==4?112:focus==7?100:127;
        if(focus==7)wm.sound=99;
        if(focus==5)vol=127;
        const char *names[]={"","22-melt","23-drift","24-decay","25-tempo","26-master","","27-preset100"};
        shot(names[focus]);
    }
    focus=0;notice=N_SAVED;notice_until=1300;shot("28-saved");
    notice=N_STOP_FIRST;shot("29-stop-first");
    notice=N_NO_VALID_SAVE;shot("30-invalid-save");
    notice=focus=confirm=0;notice_until=0;
    settings_open=1;wm.extra_large=1;settings_page=0;settings_index=1;shot("31-settings-zoom");
    settings_page=2;settings_index=3;wm.delay_feedback=0;shot("32-feedback-zero-zoom");
    settings_page=SETTINGS_REVERB;settings_index=3;shot("33-damping-zoom");
    settings_page=2;settings_index=2;wm.delay_div=2;shot("34-delay-timing-zoom");
    settings_open=0;wm.extra_large=0;confirm=2;shot("35-save-normal");
    wm.extra_large=1;shot("36-save-zoom");confirm=0;
    scope_set_open(1);cues_enabled=0;vol=100;running=0;reset_performance();
    for(uint32_t k=0;k<70;k++){int32_t b[512]={0};app_audio(b,256);}
    note_on(60,120,1);
    for(uint32_t k=0;k<24;k++){int32_t b[512]={0};app_audio(b,256);}
    shot("37-scope-live");scope_hold=1;shot("38-scope-held");scope_hold=0;
    wm.light=1;shot("39-scope-light");wm.light=0;panic();
    for(uint32_t k=0;k<70;k++){int32_t b[512]={0};app_audio(b,256);}
    shot("40-scope-silence");scope_set_open(0);wm.extra_large=0;
    notice=N_SAVE_FAILED;notice_until=0;shot("41-write-failure");
    notice=N_NO_VALID_SAVE;shot("42-empty-slot");
    notice=N_STOP_FIRST;shot("43-stop-first-persistent");
    notice=confirm=focus=scope_open=settings_open=0;wm.loop_steps=32;wm.memory.e[0].gate=1600;
    wm.memory.e[19]=(wm_event_t){67,100,800};wm.light=wm.extra_large=0;shot("46-two-bar-held-notes");
    wm.extra_large=1;shot("47-two-bar-zoom");wm.extra_large=0;settings_open=1;settings_page=3;
    settings_index=0;wm.root=6;shot("48-music-root-fsharp");
    settings_index=1;wm.scale=2;shot("49-music-minor");wm.scale=0;shot("50-music-free");
    settings_index=2;shot("51-music-length32");wm.loop_steps=16;shot("52-music-length16");
    wm.light=wm.extra_large=1;settings_index=0;shot("53-music-root-zoom-light");
    settings_index=1;wm.scale=3;shot("54-music-dorian-light");
    settings_index=2;wm.loop_steps=32;shot("55-music-length-light");
    settings_open=confirm=focus=notice=scope_open=0;wm.light=wm.extra_large=0;
    project_dirty=1;project_origin_slot=255;wm.loop_steps=32;
    for(uint32_t v=0;v<3;v++)*wm_at(&wm.memory,0,v)=(wm_event_t){60+v*3,100,1400};shot("56-polyphonic-memory");
    running=0;recording=0;project_slot=0;confirm=2;apply_confirm();notice=0;shot("57-saved-project-home");
    wm.memory.e[0].note=62;project_poll();shot("58-unsaved-edits-home");
    vol=0;project_poll();shot("59-digital-mute-home");
    wm.extra_large=1;shot("60-status-zoom");wm.extra_large=0;vol=100;
    confirm=2;shot("61-save-existing-slot");project_slot=7;shot("62-save-no-valid-project");
    project_slot=0;confirm=0;notice=N_PREF_FAILED;notice_until=0;shot("63-project-saved-preferences-failed");
    notice=N_SAVED;notice_until=0;shot("64-persistent-save-success");notice=N_LOADED;shot("65-persistent-load-success");
    notice=0;wm.light=wm.extra_large=1;project_poll();shot("66-edited-status-light-zoom");
    puts("64 major views plus two splash themes cover the 1.2 interface");
}
