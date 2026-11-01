/* SPDX-License-Identifier: GPL-3.0-only
 * LVM Wanderer event memory. No audio is recorded in this prototype.
 * Anchor is immutable until a deliberately new session is started.
 */
#pragma once
#include <stdint.h>
#define WM_STEPS 16u
#define WM_PRESET_COUNT 100u
#define WM_MAGIC 0x314d564cu /* LVM1 */
typedef struct { uint8_t note, velocity; } wm_event_t; /* velocity zero means empty */
typedef struct { wm_event_t e[WM_STEPS]; } wm_phrase_t;
typedef struct {
    uint32_t magic;
    wm_phrase_t anchor, memory, layer, branch;
    uint8_t has_anchor, has_branch, layer_enabled;
    uint8_t melt, drift, decay, bpm, branch_melt;
    uint8_t light, extra_large, reduced_motion, sound;
    uint8_t fx_on, chorus_mix, delay_mix, reverb_mix;
    uint8_t delay_div, delay_feedback, reverb_size, reverb_damp;
} wm_state_t;
#define WM_DEFAULTS { .magic=WM_MAGIC,.bpm=112,.fx_on=5, \
    .chorus_mix=26,.delay_mix=32,.reverb_mix=42, \
    .delay_div=2,.delay_feedback=48,.reverb_size=72,.reverb_damp=56 }
static wm_state_t wm = WM_DEFAULTS;
static wm_state_t wm_undo;
static uint8_t wm_can_undo;
static uint8_t wm_preset_move(uint8_t sound,int32_t delta,uint8_t banks)
{
    if(banks){
        int32_t bank=((int32_t)sound/10+delta%10+10)%10;
        return (uint8_t)(bank*10+sound%10u);
    }
    return (uint8_t)(((int32_t)sound+delta%100+100)%100);
}
static void wm_mark(void) { wm_undo=wm; wm_can_undo=1; }
static uint8_t wm_undo_once(void)
{
    if (!wm_can_undo) return 0;
    wm=wm_undo; wm_can_undo=0; return 1;
}
static uint8_t wm_has_notes(const wm_phrase_t *p)
{
    uint32_t i; for(i=0;i<WM_STEPS;i++) if(p->e[i].velocity)return 1;
    return 0;
}
static uint8_t wm_catch(void)
{
    if(!wm_has_notes(&wm.memory))return 0;
    if(wm.has_anchor)return 0; /* Catch may never overwrite the source. */
    wm_mark();wm.anchor=wm.memory;wm.has_anchor=1;return 1;
}
static uint8_t wm_record(uint32_t step,uint32_t note,uint32_t vel)
{
    wm_phrase_t *p=wm.layer_enabled?&wm.layer:&wm.memory;
    if(step>=WM_STEPS||note>127||vel==0||vel>127)return 0;
    /* A quantized monophonic lane: latest note at the step wins. */
    p->e[step].note=(uint8_t)note;p->e[step].velocity=(uint8_t)vel;
    return 1;
}
/* Read-only evolution: every original event and the Anchor remain recoverable. */
static wm_event_t wm_evolve(wm_event_t e,uint32_t step,uint32_t cycle)
{
    uint32_t n;
    if(!e.velocity)return e;
    if(wm.drift && ((step+cycle*3u)%8u)<((wm.drift+31u)/32u)) {
        int32_t delta=((step+cycle)&1u)?-1:1;
        if(wm.drift>95u)delta*=2;
        n=(uint32_t)((int32_t)e.note+delta);
        if(n<=127u)e.note=(uint8_t)n;
    }
    /* Audible aging has a floor. Returning to Anchor restores full velocity. */
    n=127u-(wm.decay* (cycle>15u?15u:cycle))/24u;
    if(n<40u)n=40u;
    e.velocity=(uint8_t)((e.velocity*n+63u)/127u);
    if(e.velocity<1u)e.velocity=1u;
    return e;
}
static uint8_t wm_branch_now(uint32_t cycle)
{
    uint32_t i;
    if(!wm.has_anchor)return 0;
    wm_mark();
    for(i=0;i<WM_STEPS;i++)wm.branch.e[i]=wm_evolve(wm.memory.e[i],i,cycle);
    wm.memory=wm.branch;wm.has_branch=1;
    wm.branch_melt=(uint8_t)(wm.branch_melt+wm.melt>127u?127u:wm.branch_melt+wm.melt);
    wm.melt=wm.drift=wm.decay=0;return 1;
}
typedef struct {uint32_t magic;uint16_t version,size;wm_state_t state;} wm_disk_t;
/* The previous 0.7 slot is read at its original byte length and migrated. */
typedef struct {
    uint32_t magic;
    wm_phrase_t anchor,memory,layer,branch;
    uint8_t has_anchor,has_branch,layer_enabled;
    uint8_t melt,drift,decay,bpm,branch_melt;
    uint8_t light,extra_large,reduced_motion,sound;
} wm_state_legacy_t;
typedef struct {uint32_t magic;uint16_t version,size;wm_state_legacy_t state;} wm_disk_legacy_t;
static uint8_t wm_valid(const wm_disk_t *d)
{
    uint32_t i;
    if(d->magic!=WM_MAGIC||d->version!=3u||d->size!=sizeof(wm_state_t)||
       d->state.magic!=WM_MAGIC||d->state.bpm<60u||d->state.bpm>180u||
       d->state.melt>127u||d->state.drift>127u||d->state.decay>127u||d->state.branch_melt>127u||
       d->state.has_anchor>1u||d->state.has_branch>1u||d->state.layer_enabled>1u||
       d->state.light>1u||d->state.extra_large>1u||d->state.reduced_motion>1u||
       d->state.sound>=WM_PRESET_COUNT||d->state.fx_on>7u||
       d->state.chorus_mix>127u||d->state.delay_mix>127u||d->state.reverb_mix>127u||
       d->state.delay_div>3u||d->state.delay_feedback>127u||
       d->state.reverb_size>127u||d->state.reverb_damp>127u)return 0;
    for(i=0;i<WM_STEPS;i++) {
        if(d->state.anchor.e[i].note>127u||d->state.memory.e[i].note>127u||
           d->state.layer.e[i].note>127u||d->state.branch.e[i].note>127u||
           d->state.anchor.e[i].velocity>127u||d->state.memory.e[i].velocity>127u||
           d->state.layer.e[i].velocity>127u||d->state.branch.e[i].velocity>127u)return 0;
    }
    return 1;
}
static uint8_t wm_valid_legacy(const wm_disk_legacy_t *d)
{
    wm_disk_t candidate={0};
    if(d->magic!=WM_MAGIC||(d->version!=1u&&d->version!=2u)||
       d->size!=sizeof(wm_state_legacy_t))return 0;
    candidate.magic=WM_MAGIC;candidate.version=3;candidate.size=sizeof(wm_state_t);
    candidate.state=(wm_state_t)WM_DEFAULTS;
    candidate.state.anchor=d->state.anchor;candidate.state.memory=d->state.memory;
    candidate.state.layer=d->state.layer;candidate.state.branch=d->state.branch;
    candidate.state.has_anchor=d->state.has_anchor;candidate.state.has_branch=d->state.has_branch;
    candidate.state.layer_enabled=d->state.layer_enabled;
    candidate.state.melt=d->state.melt;candidate.state.drift=d->state.drift;
    candidate.state.decay=d->state.decay;candidate.state.bpm=d->state.bpm;
    candidate.state.branch_melt=d->state.branch_melt;
    candidate.state.light=d->state.light;candidate.state.extra_large=d->state.extra_large;
    candidate.state.reduced_motion=d->state.reduced_motion;
    candidate.state.sound=d->version==2u?d->state.sound:0u;
    return wm_valid(&candidate);
}
static wm_state_t wm_migrate_legacy(const wm_disk_legacy_t *d)
{
    wm_state_t state=WM_DEFAULTS;
    state.anchor=d->state.anchor;state.memory=d->state.memory;
    state.layer=d->state.layer;state.branch=d->state.branch;
    state.has_anchor=d->state.has_anchor;state.has_branch=d->state.has_branch;
    state.layer_enabled=d->state.layer_enabled;
    state.melt=d->state.melt;state.drift=d->state.drift;
    state.decay=d->state.decay;state.bpm=d->state.bpm;
    state.branch_melt=d->state.branch_melt;
    state.light=d->state.light;state.extra_large=d->state.extra_large;
    state.reduced_motion=d->state.reduced_motion;
    state.sound=d->version==2u?d->state.sound:0u;
    return state;
}
