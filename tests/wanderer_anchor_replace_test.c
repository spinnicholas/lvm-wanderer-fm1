/* SPDX-License-Identifier: GPL-3.0-only
 * 3.1: tap SAVE sets the Anchor; with an Anchor already set it asks
 * NEW ANCHOR / REPLACE ANCHOR, and SELECT makes Memory as heard (wander
 * included) the new Anchor. One Undo restores the previous Anchor. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static void fresh(void)
{
    wm=(wm_state_t)WM_DEFAULTS;ww_reset();wp_reset();
    running=recording=anchor_view=layer_focus=0;confirm=focus=notice=0;
    splash_active=0;wm_can_undo=0;fm1_ms=100;
}
static int same(const wm_phrase_t *a,const wm_phrase_t *b){return !memcmp(a,b,sizeof *a);}
int main(void)
{
    /* No notes: nothing to set. */
    fresh();save_tap();assert(!wm.has_anchor&&confirm==0&&notice==N_CATCH_FIRST);

    /* First tap sets the Anchor, no question. */
    fresh();wm_record(0,60,100);wm_record(4,64,100);save_tap();
    assert(wm.has_anchor&&notice==N_ANCHORED&&confirm==0&&same(&wm.anchor,&wm.memory));
    wm_phrase_t first=wm.anchor;

    /* Change Memory; the second tap asks, HOME-style cancel leaves everything. */
    wm_record(8,67,100);save_tap();assert(confirm==7&&same(&wm.anchor,&first));
    confirm=0;assert(same(&wm.anchor,&first));

    /* SELECT replaces: the Anchor becomes Memory; Undo restores the old one. */
    save_tap();assert(confirm==7);apply_confirm();
    assert(confirm==0&&notice==N_ANCHOR_REPLACED&&wm.has_anchor&&same(&wm.anchor,&wm.memory)&&!same(&wm.anchor,&first));
    assert(!wm.has_branch&&!wm.branch_melt&&!ww.loops&&!ww.distance);
    wm_phrase_t second=wm.anchor;
    assert(undo_events_locked());assert(same(&wm.anchor,&first));
    (void)second;

    /* With wander: what you hear (wander baked in) becomes the Anchor. */
    fresh();for(uint32_t s=0;s<16;s+=2)wm_record(s,60+s,100);save_tap();assert(wm.has_anchor);
    wm.melt=100;wm.drift=127;
    for(int i=0;i<12;i++)ww_loop();
    assert(ww.loops);
    wm_state_t heard_state=wm;ww_state_t heard_ww=ww;
    assert(ww_keep());wm_phrase_t heard=wm.memory;              /* reference: Keep's result */
    assert(!same(&heard,&heard_state.memory));                 /* wander really moved notes */
    wm=heard_state;ww=heard_ww;
    save_tap();assert(confirm==7);apply_confirm();
    assert(notice==N_ANCHOR_REPLACED&&same(&wm.anchor,&heard)&&same(&wm.memory,&heard));
    assert(!ww.loops&&!ww.distance);
    assert(undo_events_locked());assert(same(&wm.anchor,&heard_state.anchor)&&same(&wm.memory,&heard_state.memory));

    /* Listening to the Anchor, or with printed audio, it is never offered. */
    fresh();wm_record(0,60,100);save_tap();wm_record(4,62,100);anchor_view=1;save_tap();
    assert(confirm==0&&notice==N_ANCHOR_SAFE);anchor_view=0;

    /* Works while the loop plays (no STOP FIRST). */
    running=1;save_tap();assert(confirm==7);apply_confirm();assert(notice==N_ANCHOR_REPLACED);running=0;

    /* The confirm screen and the message render in Normal and Zoom, every theme. */
    for(uint32_t t=0;t<THEME_COUNT;t++)for(uint32_t z=0;z<2;z++){
        color_theme=t;wm.extra_large=(uint8_t)z;confirm=7;notice=0;screen();
        confirm=0;notice=N_ANCHOR_REPLACED;screen();
    }
    color_theme=0;wm.extra_large=0;confirm=7;notice=0;screen();write_ppm_as("build/screens/confirm-new-anchor.ppm");
    wm.extra_large=1;screen();write_ppm_as("build/screens/confirm-new-anchor-zoom.ppm");
    puts("SAVE sets the Anchor, asks before replacing it, replaces with Memory as heard, Undo restores, never while listening");
    return 0;
}
