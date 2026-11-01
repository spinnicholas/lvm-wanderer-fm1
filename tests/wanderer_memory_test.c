/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../firmware/src/wanderer_memory.h"
int main(void)
{
    wm_disk_t d;
    wm_disk_legacy_t old={0};
    wm_event_t before, after;
    assert(sizeof(wm_state_legacy_t)==144u);
    assert(sizeof(wm_state_t)==2076u);
    assert(sizeof(wm_disk_t)==2084u);
    assert(wm_preset_move(0,-1,0)==99 && wm_preset_move(99,1,0)==0);
    assert(wm_preset_move(0,-1,1)==90 && wm_preset_move(99,1,1)==9);
    assert(wm_preset_move(37,2,1)==57 && wm_preset_move(37,2,0)==39);
    assert(!wm_catch());
    assert(!wm_branch_now(0));
    assert(!wm_record(16,60,100));
    assert(!wm_record(0,128,100));
    assert(!wm_record(0,60,0));
    assert(wm_record(0,60,100));
    assert(wm_record(4,64,90));
    assert(wm_catch());
    assert(!wm_catch());
    before=wm.anchor.e[0];
    wm.drift=127;wm.decay=127;wm.melt=67;
    after=wm_evolve(wm.memory.e[0],0,1);
    assert(after.note!=before.note);
    assert(wm_evolve(wm.memory.e[0],0,15).velocity<before.velocity);
    assert(wm.anchor.e[0].note==before.note && wm.anchor.e[0].velocity==before.velocity);
    wm.layer_enabled=1;
    assert(wm_record(5,67,111));
    assert(wm.layer.e[5].note==67 && !wm.memory.e[5].velocity);
    assert(wm_branch_now(1));
    assert(wm.has_branch && wm.memory.e[0].note==after.note);
    assert(wm.branch_melt==67 && wm.melt==0 && wm.drift==0 && wm.decay==0);
    assert(wm.anchor.e[0].note==before.note && wm.anchor.e[0].velocity==before.velocity);
    assert(wm_undo_once());
    assert(!wm.has_branch && wm.melt==67 && wm.drift==127);
    assert(!wm_undo_once());
    d.magic=WM_MAGIC;d.version=WM_DISK_VERSION;d.size=sizeof wm;d.state=wm;
    assert(wm_valid(&d));
    d.state.bpm=0;assert(!wm_valid(&d));d.state.bpm=112;
    d.state.anchor.e[0].note=255;assert(!wm_valid(&d));
    d.state.anchor.e[0].note=60;d.version=WM_DISK_VERSION;d.state.sound=2;assert(wm_valid(&d));
    d.state.sound=99;assert(wm_valid(&d));
    d.state.sound=100;assert(!wm_valid(&d));d.state.sound=0;
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
    old.magic=WM_MAGIC;old.version=2;old.size=sizeof old.state;
    old.state.magic=WM_MAGIC;old.state.bpm=112;old.state.sound=2;
    assert(wm_valid_legacy(&old));
    assert(wm_migrate_legacy(&old).sound==2);
    assert(wm_migrate_legacy(&old).reverb_mix==42);
    old.version=1;old.state.sound=255;
    assert(wm_valid_legacy(&old));
    assert(wm_migrate_legacy(&old).sound==0);
    old.state.magic=0;assert(!wm_valid_legacy(&old));
    puts("event-memory preservation, layer, branch, undo and storage validation pass");
    return 0;
}
