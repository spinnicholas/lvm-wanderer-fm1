/* SPDX-License-Identifier: GPL-3.0-only
 * LVM Wanderer event memory. Anchor is immutable until a deliberately new session.
 * 3.1: one current project format (v16). Every new firmware version starts
 * fresh, so there is no migration code for older projects.
 */
#pragma once
#include <stdint.h>
#include "wanderer_bank.h"
/* The generated tables describe six engines; 3.0 added SUPER (6) and VOICE (7). */
#undef WS_ENGINES
#define WS_ENGINES 8u
#define WM_STEPS 32u
#define WM_DISK_VERSION 16u
enum {WS_LOW,WS_BAND,WS_HIGH,WS_LP12,WS_LP24,WS_FILTER_COUNT};
enum {RV_ROOM,RV_HALL,RV_PLATE,RV_SPRING,RV_CAVERN,RV_DEEP_SPACE,RV_AMBIENT,RV_COUNT};
#define WM_SCATTER_MODES 8u
#define WM_ARP_MODES 13u
#define WM_ARP_RATES 12u
#define WM_GRAIN_V11_COUNT 9u /* hop/jump lookback threshold (sound behaviour) */
#define WM_GRAIN_COUNT 10u
/* 512 ms and all older selections are unchanged. 768 is fixed to bound RAM.
 * Native RC4 linker margins preclude the 1-second RAM experiment. */
static const uint16_t WM_GRAIN_MS[WM_GRAIN_COUNT]={0,4,8,16,32,64,128,256,512,768};
#define WM_PATTERN_COUNT 10u
#define WM_POLY 4u
#define WM_SCALE_COUNT 8u
#define WM_GATE_UNIT 256u
#define WM_PRESET_COUNT WS_PRESET_COUNT
#define WM_PRESET_BANKS (WM_PRESET_COUNT/10u)
/* Only the current factory bank exists; the bank fields are kept at this value. */
#define WS_BANK_CURRENT 4u
#define WM_MAGIC 0x334d564cu /* LVM3: new format, older projects are rejected */
typedef struct { uint8_t note, velocity; uint16_t gate; } wm_event_t; /* velocity zero means empty */
/* Keep the primary event first; the other three slots store chord/overdub voices. */
typedef struct { wm_event_t e[WM_STEPS]; wm_event_t extra[WM_STEPS][WM_POLY-1u]; } wm_phrase_t;
static wm_event_t *wm_at(wm_phrase_t *p,uint32_t step,uint32_t slot)
{return slot?&p->extra[step][slot-1u]:&p->e[step];}
static wm_event_t wm_get(const wm_phrase_t *p,uint32_t step,uint32_t slot)
{return slot?p->extra[step][slot-1u]:p->e[step];}
static uint32_t wm_step_notes(const wm_phrase_t *p,uint32_t step)
{uint32_t n=0;for(uint32_t v=0;v<WM_POLY;v++)n+=wm_get(p,step,v).velocity!=0;return n;}
static void wm_clear_phrase(wm_phrase_t *p)
{for(uint32_t s=0;s<WM_STEPS;s++)for(uint32_t v=0;v<WM_POLY;v++)wm_at(p,s,v)->velocity=0;}
/* The project state. Bank and "legacy" fields are always WS_BANK_CURRENT / 0 in 3.1. */
typedef struct {
    uint32_t magic;
    wm_phrase_t anchor, memory, layer, branch;
    uint8_t has_anchor, has_branch, layer_enabled;
    uint8_t melt, drift, decay, bpm, branch_melt;
    uint8_t light, extra_large, reduced_motion, sound;
    uint8_t fx_on, chorus_mix, delay_mix, reverb_mix;
    uint8_t delay_div, delay_feedback, reverb_size, reverb_damp;
    uint8_t loop_steps,root,scale,grain_size;
    uint8_t legacy,patch_valid,anchor_patch_valid,keyboard_scale;
    uint8_t repeat_style,transpose_base,anchor_legacy,anchor_sound;
    ws_patch_t patch,anchor_patch;
    uint8_t scatter_edge,scatter_mix,scatter_tone,scatter_variation;
    uint8_t arp_on,arp_mode,arp_rate,arp_octaves,arp_gate,arp_swing,arp_latch,arp_clock;
    uint8_t preset_bank,anchor_preset_bank;
    uint8_t fx_new_on,crush_bits,crush_rate,crush_mix;
    uint8_t tape_mix,tape_time,tape_feedback,tape_tone,tape_wow,tape_heads,tape_sync;
    uint8_t phaser_mix,phaser_rate,phaser_depth;
    uint8_t fold_mix,fold_drive,fold_tone,chorus_rate,chorus_depth;
    uint8_t scatter_pattern; /* v12 */
    uint8_t reverb_type; /* v13 */
    uint8_t layer_sound; /* 0 means no independent snapshot; otherwise preset index + 1 */
    uint8_t midi_input,midi_channel,midi_ignore_program; /* v14 */
    uint8_t layer_link,layer_patch_valid,layer_preset_bank,layer_legacy;
    ws_patch_t layer_patch;
} wm_state_t;
typedef struct {uint32_t magic;uint16_t version,size;wm_state_t state;} wm_disk_t;
static const ws_patch_t *ws_factory_patch(uint8_t bank,uint8_t sound)
{(void)bank;return &WS_PRESETS_3[sound<WS_PRESET_COUNT?sound:0u];}
static const char *ws_factory_name(uint8_t bank,uint8_t sound)
{(void)bank;return WS_NAMES_3[sound<WS_PRESET_COUNT?sound:0u];}
#define WM_DEFAULT_FIELDS .magic=WM_MAGIC,.loop_steps=16,.root=0,.scale=2,.bpm=112,.fx_on=4,.grain_size=8, \
    .chorus_mix=26,.delay_mix=32,.reverb_mix=42, \
    .delay_div=2,.delay_feedback=48,.reverb_size=72,.reverb_damp=56, \
    .scatter_edge=72,.scatter_mix=100,.scatter_tone=90,.scatter_variation=64, \
    .arp_rate=6,.arp_octaves=1,.arp_gate=75, \
    .preset_bank=WS_BANK_CURRENT,.anchor_preset_bank=WS_BANK_CURRENT, \
    .crush_bits=10,.crush_rate=3,.crush_mix=64, \
    .tape_mix=52,.tape_time=90,.tape_feedback=64,.tape_tone=62,.tape_wow=20,.tape_heads=1,.tape_sync=3, \
    .phaser_mix=64,.phaser_rate=35,.phaser_depth=85, \
    .fold_mix=64,.fold_drive=40,.fold_tone=95,.chorus_rate=34,.chorus_depth=64,.layer_preset_bank=WS_BANK_CURRENT
#define WM_DEFAULTS { WM_DEFAULT_FIELDS }
/* New sessions start with an independent, audible Layer. Import templates
 * preserve the stored mute and follow behavior of earlier sessions. */
#define WM_NEW_DEFAULTS { WM_DEFAULT_FIELDS,.layer_enabled=1,.layer_sound=1 }
static wm_state_t wm = WM_NEW_DEFAULTS;
static uint8_t layer_focus; /* Edit and record target, independent of playback. */
static wm_state_t wm_undo;
static uint8_t wm_can_undo;
static uint8_t wm_preset_move(uint8_t sound,int32_t delta,uint8_t banks)
{
    if(banks){
        int32_t bank=((int32_t)sound/10+delta%(int32_t)WM_PRESET_BANKS+(int32_t)WM_PRESET_BANKS)%(int32_t)WM_PRESET_BANKS;
        return (uint8_t)(bank*10+sound%10u);
    }
    return (uint8_t)(((int32_t)sound+delta%(int32_t)WM_PRESET_COUNT+(int32_t)WM_PRESET_COUNT)%(int32_t)WM_PRESET_COUNT);
}
#ifndef WM_MARK_HOOK
#define WM_MARK_HOOK() ((void)0)
#define WM_UNDO_HOOK() ((void)0)
#endif
static void wm_mark(void) { wm_undo=wm;wm_can_undo=1;WM_MARK_HOOK(); }
static uint8_t wm_undo_once(void)
{
    if (!wm_can_undo) return 0;
    uint8_t light=wm.light,zoom=wm.extra_large,still=wm.reduced_motion;
    wm=wm_undo;wm.light=light;wm.extra_large=zoom;wm.reduced_motion=still;
    wm_can_undo=0;WM_UNDO_HOOK();return 1;
}
static uint8_t wm_has_notes(const wm_phrase_t *p)
{
    uint32_t i; for(i=0;i<WM_STEPS;i++) if(wm_step_notes(p,i))return 1;
    return 0;
}
static uint8_t wm_memory_playable(void)
{
    for(uint32_t i=0;i<wm.loop_steps;i++)if(wm_step_notes(&wm.memory,i))return 1;
    return 0;
}
static uint8_t wm_protect_source(void);
static uint8_t wm_catch(void)
{
    if(!wm_memory_playable())return 0;
    if(wm.has_anchor)return 0; /* Catch may never overwrite the source. */
    wm_mark();return wm_protect_source();
}
/* The Anchor copy itself. Wander calls this without touching Undo. */
static uint8_t wm_protect_source(void)
{
    if(wm.has_anchor||!wm_memory_playable())return 0;
    wm.anchor=wm.memory;wm.has_anchor=1;wm.anchor_patch=wm.patch_valid?wm.patch:*ws_factory_patch(wm.preset_bank,wm.sound);wm.anchor_patch_valid=(uint8_t)!wm.legacy;wm.anchor_legacy=wm.legacy;wm.anchor_sound=wm.sound;wm.anchor_preset_bank=wm.preset_bank;return 1;
}
static uint8_t wm_record(uint32_t step,uint32_t note,uint32_t vel)
{
    wm_phrase_t *p=layer_focus?&wm.layer:&wm.memory;
    if(step>=wm.loop_steps||note>127||vel==0||vel>127)return 0;
    /* A quantized monophonic lane: latest note at the step wins. */
    p->e[step]=(wm_event_t){(uint8_t)note,(uint8_t)vel,192u};
    return 1;
}
/* Free, major, natural minor, Dorian, minor/major pentatonic, Mixolydian,
 * harmonic minor. Scale affects generated pitches; the performed source is exact. */
static const uint16_t WM_SCALES[WM_SCALE_COUNT]={0xfff,0xab5,0x5ad,0x6ad,0x4a9,0x295,0x6b5,0x9ad};
static uint8_t wm_in_scale(uint32_t note)
{return (uint8_t)((WM_SCALES[wm.scale]>>((note+12u-wm.root)%12u))&1u);}
static uint8_t wm_scale_move(uint8_t note,int32_t degrees)
{
    int32_t n=note,dir=degrees<0?-1:1,count=degrees<0?-degrees:degrees;
    while(count--){
        int32_t candidate=n;
        do {candidate+=dir;} while(candidate>=0&&candidate<=127&&!wm_in_scale((uint32_t)candidate));
        if(candidate<0||candidate>127)break;
        n=candidate;
    }
    return (uint8_t)n;
}
/* Resolve to the original motif every fourth loop. Low Drift acts only at
 * phrase endings; stronger settings add answering notes on weak beats. */
static int32_t wm_drift_answer(uint32_t step,uint32_t cycle)
{
 uint32_t phase=cycle&3u,tail=wm.loop_steps-step;
 if(!wm.drift||!phase||!(step&3u))return 0;
 if(wm.drift<32u)return phase==3u&&tail<=2u?-1:0;
 if(tail<=4u)return phase==3u?-1:1;
 if(wm.drift<80u||((step>>1)&1u)!=(phase&1u))return 0;
 return phase==2u?1:-1;
}
/* Fill density follows phrase endings. Low amounts leave three full cycles untouched. */
static uint32_t wm_repeat_divisions(uint32_t step,uint32_t cycle)
{
 uint32_t a=wm.decay;if(!a)return 1;
 uint8_t end=step+1u==wm.loop_steps;
 uint8_t eligible=a<32u?(end&&cycle%4u==3u):a<64u?end:a<96u?step%8u==7u:a<112u?step%4u==3u:step%2u==1u;
 return !eligible?1u:a<64u?2u:a<112u?3u:4u;
}
/* Shortening hides the second bar, preserving it for an immediate return/Undo. */
static uint8_t wm_set_length(uint8_t length)
{
    if((length!=16u&&length!=32u)||length==wm.loop_steps)return 0;
    wm_mark();wm.loop_steps=length;return 1;
}
static uint8_t ws_patch_valid(const ws_patch_t *p)
{
 const uint8_t *bytes=(const uint8_t*)p;
 for(uint32_t i=0;i<sizeof(ws_patch_t)-6u;i++)if(bytes[i]>127u)return 0;
 for(uint32_t i=0;i<6;i++)if(p->morph[i]==-128)return 0;
 return p->engine<WS_ENGINES&&p->wave<(p->engine==0?10u:p->engine==2?8u:p->engine>=6u?4u:6u)&&p->filter<WS_FILTER_COUNT&&p->lfo_dest<3u&&p->algorithm<(p->engine==5?32u:8u);
}
static __attribute__((noinline)) uint8_t wm_valid(const wm_disk_t *d)
{
    uint32_t i;
    /* 3.1 has one bank and no classic sounds. */
    if(d->state.legacy||d->state.anchor_legacy||d->state.layer_legacy||d->state.preset_bank!=WS_BANK_CURRENT||
       d->state.anchor_preset_bank!=WS_BANK_CURRENT||d->state.layer_preset_bank!=WS_BANK_CURRENT)return 0;
    if(d->magic!=WM_MAGIC||d->version!=WM_DISK_VERSION||d->size!=sizeof(wm_state_t)||
       d->state.magic!=WM_MAGIC||d->state.bpm<60u||d->state.bpm>180u||
       d->state.melt>127u||d->state.drift>127u||d->state.decay>127u||d->state.branch_melt>127u||
       d->state.has_anchor>1u||d->state.has_branch>1u||d->state.layer_enabled>1u||
       d->state.light>1u||d->state.extra_large>1u||d->state.reduced_motion>1u||
       (d->state.loop_steps!=16u&&d->state.loop_steps!=32u)||d->state.root>11u||
       d->state.legacy>1u||d->state.patch_valid>1u||d->state.anchor_patch_valid>1u||d->state.keyboard_scale>1u||d->state.repeat_style>=WM_SCATTER_MODES||d->state.transpose_base>11u||d->state.anchor_legacy>1u||
       d->state.preset_bank>WS_BANK_CURRENT||d->state.anchor_preset_bank>WS_BANK_CURRENT||
       d->state.anchor_sound>=WM_PRESET_COUNT||
       d->state.scale>=WM_SCALE_COUNT||d->state.grain_size>=WM_GRAIN_COUNT||d->state.sound>=WM_PRESET_COUNT||d->state.fx_on>7u||
       d->state.chorus_mix>127u||d->state.delay_mix>127u||d->state.reverb_mix>127u||
       d->state.delay_div>3u||d->state.delay_feedback>127u||
       d->state.reverb_size>127u||d->state.reverb_damp>127u||d->state.reverb_type>=RV_COUNT||d->state.layer_sound>WM_PRESET_COUNT||
       d->state.layer_link>1u||d->state.layer_patch_valid>1u||d->state.layer_preset_bank>WS_BANK_CURRENT||d->state.layer_legacy>1u||
       d->state.midi_input>2u||d->state.midi_channel>16u||d->state.midi_ignore_program>1u||
       d->state.scatter_edge>127u||d->state.scatter_mix>127u||d->state.scatter_tone>127u||d->state.scatter_variation>127u||d->state.scatter_pattern>=WM_PATTERN_COUNT||
       d->state.arp_on>1u||d->state.arp_mode>=WM_ARP_MODES||d->state.arp_rate>=WM_ARP_RATES||
       d->state.arp_octaves<1u||d->state.arp_octaves>4u||d->state.arp_gate<5u||d->state.arp_gate>100u||
       d->state.arp_swing>50u||d->state.arp_latch>1u||d->state.arp_clock>2u)return 0;
    if(d->state.fx_new_on>15u||d->state.crush_bits<1u||d->state.crush_bits>16u||d->state.crush_rate>15u||
       d->state.crush_mix>127u||d->state.tape_mix>127u||d->state.tape_time>127u||d->state.tape_feedback>127u||
       d->state.tape_tone>127u||d->state.tape_wow>127u||d->state.tape_heads>2u||d->state.tape_sync>3u||
       d->state.phaser_mix>127u||d->state.phaser_rate>127u||d->state.phaser_depth>127u||
       d->state.fold_mix>127u||d->state.fold_drive>127u||d->state.fold_tone>127u||
       d->state.chorus_rate>127u||d->state.chorus_depth>127u)return 0;
    if((d->state.patch_valid&&!ws_patch_valid(&d->state.patch))||(d->state.anchor_patch_valid&&!ws_patch_valid(&d->state.anchor_patch))||
       (d->state.layer_patch_valid&&(!d->state.layer_sound||!ws_patch_valid(&d->state.layer_patch))))return 0;
    const wm_phrase_t *phrases[4]={&d->state.anchor,&d->state.memory,&d->state.layer,&d->state.branch};
    for(uint32_t p=0;p<4u;p++)for(i=0;i<WM_STEPS;i++)for(uint32_t v=0;v<WM_POLY;v++){
        wm_event_t e=wm_get(phrases[p],i,v);
        if(e.note>127u||e.velocity>127u||e.gate>8192u)return 0;

    }
    return 1;
}

