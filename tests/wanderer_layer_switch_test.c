/* SPDX-License-Identifier: GPL-3.0-only
 * 3.1: the Home Layer switch. Tap FX flips it (on release); holding FX 1.6 s
 * asks New session and leaves the Layer alone. Settings > LAYER shows the same
 * ON/OFF words; its knob 4 CLEAR asks before clearing and says EMPTY when there
 * is nothing to clear. FX glows while the Layer is ON; LFO glows only when
 * there is an Anchor to hear. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
#include <string.h>
static void fresh(void)
{
    wm=(wm_state_t)WM_NEW_DEFAULTS;ww_reset();wp_reset();
    running=recording=anchor_view=layer_focus=0;confirm=focus=notice=0;
    settings_open=editor_open=scope_open=0;splash_active=0;fm1_ms=100;fx_pending=0;fx_down=0;
}
static uint32_t press(uint32_t held_ms)
{
    const uint32_t bit=1u<<BTN_FX;uint32_t out=0;
    fm1_in.buttons=bit;out|=fx_edges(bit,0);
    for(uint32_t t=0;t<held_ms;t+=10){fm1_ms+=10;out|=fx_edges(0,0);}
    fm1_in.buttons=0;out|=fx_edges(0,bit);
    return out;
}
int main(void)
{
    char buf[12];
    /* New project: Layer ON; a tap turns it OFF, another turns it ON. */
    fresh();assert(wm.layer_enabled);
    assert(press(120)&(1u<<BTN_FX));layer_switch_tap();
    assert(!wm.layer_enabled&&notice==N_LAYER_OFF&&confirm==0);
    layer_switch_tap();assert(wm.layer_enabled&&notice==N_LAYER_ON);
    /* Undo restores the switch like any other change. */
    assert(undo_events_locked()&&!wm.layer_enabled);

    /* Hold 1.6 s: New session asks; the Layer is not flipped on release. */
    fresh();uint32_t out=press(1700);
    assert(confirm==4&&!(out&(1u<<BTN_FX))&&wm.layer_enabled);

    /* Settings > LAYER: knob 2 says ON/OFF, like the switch. */
    fresh();settings_open=1;settings_page=SETTINGS_LAYER;
    assert(settings_count()==4u);
    assert(!strcmp(settings_value(1,buf),"ON"));
    settings_adjust(1,-1);assert(!wm.layer_enabled&&!strcmp(settings_value(1,buf),"OFF"));
    settings_adjust(1,1);assert(wm.layer_enabled);

    /* Knob 4 CLEAR: EMPTY does nothing; with notes it asks, then clears. */
    assert(!strcmp(settings_value(3,buf),"EMPTY"));settings_adjust(3,1);assert(confirm==0);
    layer_focus=1;wm_record(0,62,100);layer_focus=0;assert(wm_has_notes(&wm.layer));
    assert(!strcmp(settings_value(3,buf),"LAYER"));
    settings_adjust(3,-1);assert(confirm==0);                      /* turning down never asks */
    settings_index=3;settings_press();assert(confirm==1);          /* SELECT asks too */
    confirm=0;settings_adjust(3,1);assert(confirm==1);
    apply_confirm();assert(!wm_has_notes(&wm.layer)&&notice==N_LAYER_CLEARED&&settings_open);
    assert(undo_events_locked()&&wm_has_notes(&wm.layer));         /* Undo brings it back */

    /* Lamps: FX glows while the Layer is ON; LFO only with an Anchor. */
    fresh();wm_record(0,60,100);
    leds_initialised=0;app_leds();
    assert(host_glow&((uint64_t)1u<<BTN_FX));
    assert(!(host_glow&((uint64_t)1u<<BTN_LFO)));                  /* no Anchor yet: nothing to hear */
    layer_switch_tap();notice=0;app_leds();
    assert(!(host_glow&((uint64_t)1u<<BTN_FX)));
    assert(wm_catch());app_leds();assert(host_glow&((uint64_t)1u<<BTN_LFO));

    /* The switch, the notices and the Layer page draw in every theme and size. */
    for(uint32_t t=0;t<THEME_COUNT;t++)for(uint32_t z=0;z<2;z++){
        color_theme=t;wm.extra_large=(uint8_t)z;
        settings_open=0;notice=N_LAYER_ON;screen();notice=N_LAYER_OFF;screen();notice=0;screen();
        settings_open=1;settings_page=SETTINGS_LAYER;for(settings_index=0;settings_index<4;settings_index++)screen();
    }
    puts("FX tap flips the Layer switch, hold asks New session, Layer page ON/OFF and CLEAR (asks), FX/LFO lamps honest");
    return 0;
}
