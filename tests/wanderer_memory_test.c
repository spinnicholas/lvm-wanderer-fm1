/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../firmware/src/wanderer_memory.h"
int main(void)
{
    wm_disk_t d;
    wm_event_t before;
    assert(sizeof(wm_state_t)>2076u&&sizeof(wm_state_t)<4096u);
    assert(wm_preset_move(0,-1,0)==149 && wm_preset_move(149,1,0)==0);
    assert(wm_preset_move(0,-1,1)==140 && wm_preset_move(149,1,1)==9);
    assert(wm_preset_move(37,2,1)==57 && wm_preset_move(37,2,0)==39);
    assert(!wm_catch());
    assert(!wm_record(16,60,100));
    assert(!wm_record(0,128,100));
    assert(!wm_record(0,60,0));
    assert(wm_record(0,60,100));
    assert(wm_record(4,64,90));
    assert(wm_catch());
    assert(!wm_catch());
    before=wm.anchor.e[0];
    wm.drift=127;wm.decay=127;wm.melt=67;
    wm.layer_enabled=layer_focus=1;
    assert(wm_record(5,67,111));
    assert(wm.layer.e[5].note==67 && !wm.memory.e[5].velocity);
    /* One-step Undo restores the whole project and leaves the Anchor alone. */
    wm_mark();wm.melt=0;wm.drift=0;wm.memory.e[0].note=72;
    assert(wm.anchor.e[0].note==before.note && wm.anchor.e[0].velocity==before.velocity);
    assert(wm_undo_once());
    assert(wm.melt==67 && wm.drift==127 && wm.memory.e[0].note==60);
    assert(!wm_undo_once());
    d.magic=WM_MAGIC;d.version=WM_DISK_VERSION;d.size=sizeof wm;d.state=wm;
    assert(wm_valid(&d));
    d.state.bpm=0;assert(!wm_valid(&d));d.state.bpm=112;
    d.state.anchor.e[0].note=255;assert(!wm_valid(&d));
    d.state.anchor.e[0].note=60;d.version=WM_DISK_VERSION;d.state.sound=2;assert(wm_valid(&d));
    d.state.sound=99;assert(wm_valid(&d));
    d.state.sound=149;assert(wm_valid(&d));
    d.state.legacy=1;assert(!wm_valid(&d));d.state.legacy=0;           /* 3.1: no classic sounds */
    d.state.preset_bank=3;assert(!wm_valid(&d));d.state.preset_bank=WS_BANK_CURRENT; /* one bank only */
    d.magic=0x314d564cu;assert(!wm_valid(&d));d.magic=WM_MAGIC;        /* older LVM1 records are refused */
    d.state.sound=150;assert(!wm_valid(&d));d.state.sound=0;
    d.version=2;assert(!wm_valid(&d));
    d.version=WM_DISK_VERSION;d.size=0;assert(!wm_valid(&d));
    d=(wm_disk_t){WM_MAGIC,WM_DISK_VERSION,sizeof wm,wm};
    for(uint32_t lane=0;lane<4;lane++){
        wm_phrase_t *phrases[]={&d.state.anchor,&d.state.memory,&d.state.layer,&d.state.branch};
        for(uint32_t step=0;step<WM_STEPS;step++)for(uint32_t v=0;v<WM_POLY;v++){
            wm_event_t *e=wm_at(phrases[lane],step,v),keep=*e;
            e->note=128;assert(!wm_valid(&d));*e=keep;
            e->velocity=128;assert(!wm_valid(&d));*e=keep;
            e->gate=8193;assert(!wm_valid(&d));*e=keep;
        }
    }
    assert(wm_valid(&d));
    puts("event-memory preservation, layer, undo and current-format validation pass");
    return 0;
}
