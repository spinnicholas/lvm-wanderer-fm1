/* SPDX-License-Identifier: GPL-3.0-only
 * LVM Wanderer event memory. No audio is recorded in this prototype.
 * Anchor is immutable until a deliberately new session is started.
 */
#pragma once
#include <stdint.h>
#include "wanderer_sound_data.h"
#define WM_STEPS 32u
#define WM_DISK_VERSION 8u
#define WM_SCATTER_MODES 8u
#define WM_ARP_MODES 13u
#define WM_ARP_RATES 12u
#define WM_GRAIN_COUNT 7u
static const uint8_t WM_GRAIN_MS[WM_GRAIN_COUNT]={0,4,8,16,32,64,128};
#define WM_POLY 4u
#define WM_SCALE_COUNT 8u
#define WM_GATE_UNIT 256u
#define WM_PRESET_COUNT 100u
#define WM_MAGIC 0x314d564cu /* LVM1 */
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

typedef struct {
    uint32_t magic;
    wm_phrase_t anchor, memory, layer, branch;
    uint8_t has_anchor, has_branch, layer_enabled;
    uint8_t melt, drift, decay, bpm, branch_melt;
    uint8_t light, extra_large, reduced_motion, sound;
    uint8_t fx_on, chorus_mix, delay_mix, reverb_mix;
    uint8_t delay_div, delay_feedback, reverb_size, reverb_damp;
    uint8_t loop_steps,root,scale,reserved;
} wm_state_v5_t;
typedef struct {uint32_t magic;uint16_t version,size;wm_state_v5_t state;} wm_disk_v5_t;
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
} wm_state_v7_t;
typedef struct {uint32_t magic;uint16_t version,size;wm_state_v7_t state;} wm_disk_v7_t;
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
} wm_state_t;
#define WM_DEFAULTS { .magic=WM_MAGIC,.loop_steps=16,.root=0,.scale=2,.bpm=112,.fx_on=5, \
    .chorus_mix=26,.delay_mix=32,.reverb_mix=42, \
    .delay_div=2,.delay_feedback=48,.reverb_size=72,.reverb_damp=56, \
    .scatter_edge=72,.scatter_mix=100,.scatter_tone=90,.scatter_variation=64, \
    .arp_rate=6,.arp_octaves=1,.arp_gate=75 }
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
static uint8_t wm_catch(void)
{
    if(!wm_has_notes(&wm.memory))return 0;
    if(wm.has_anchor)return 0; /* Catch may never overwrite the source. */
    wm_mark();wm.anchor=wm.memory;wm.has_anchor=1;wm.anchor_patch=wm.patch_valid?wm.patch:WS_PRESETS[wm.sound];wm.anchor_patch_valid=(uint8_t)!wm.legacy;wm.anchor_legacy=wm.legacy;wm.anchor_sound=wm.sound;return 1;
}
static uint8_t wm_record(uint32_t step,uint32_t note,uint32_t vel)
{
    wm_phrase_t *p=wm.layer_enabled?&wm.layer:&wm.memory;
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
/* Eight-cycle phrases resolve to the performed motif. Moves are read-only,
 * scale degrees rather than accumulated random semitones. */
static wm_event_t wm_evolve(wm_event_t e,uint32_t step,uint32_t cycle)
{
    uint32_t n,phase=cycle%8u;
    if(!e.velocity)return e;
    if(wm.drift&&phase&&(wm.legacy||((step%4u)!=0u&&e.note%12u!=wm.root))&&((step*5u+phase*3u)%16u)<((wm.drift+7u)/8u)){
        int32_t degree=((step/4u+phase)&1u)?-1:1;
        if(wm.drift>95u&&phase>3u)degree*=2;
        e.note=wm_scale_move(e.note,degree);
    }
    if(!wm.legacy)return e;
    n=127u-(wm.decay*(cycle>15u?15u:cycle))/24u;
    if(n<40u)n=40u;
    e.velocity=(uint8_t)((e.velocity*n+63u)/127u);
    if(e.velocity<1u)e.velocity=1u;
    return e;
}
/* Fill density follows phrase endings. Low amounts leave three full cycles untouched. */
static uint32_t wm_repeat_divisions(uint32_t step,uint32_t cycle)
{
 uint32_t a=wm.decay;if(wm.legacy||!a)return 1;
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
static uint8_t wm_branch_now(uint32_t cycle)
{
    uint32_t i;
    if(!wm.has_anchor)return 0;
    wm_mark();
    for(i=0;i<WM_STEPS;i++)for(uint32_t v=0;v<WM_POLY;v++)*wm_at(&wm.branch,i,v)=wm_evolve(wm_get(&wm.memory,i,v),i,cycle);
    wm.memory=wm.branch;wm.has_branch=1;
    wm.branch_melt=(uint8_t)(wm.branch_melt+wm.melt>127u?127u:wm.branch_melt+wm.melt);
    wm.melt=wm.drift=wm.decay=0;return 1;
}
typedef struct {wm_event_t e[WM_STEPS];} wm_phrase_v4_t;
typedef struct {
    uint32_t magic;
    wm_phrase_v4_t anchor,memory,layer,branch;
    uint8_t has_anchor,has_branch,layer_enabled;
    uint8_t melt,drift,decay,bpm,branch_melt;
    uint8_t light,extra_large,reduced_motion,sound;
    uint8_t fx_on,chorus_mix,delay_mix,reverb_mix;
    uint8_t delay_div,delay_feedback,reverb_size,reverb_damp;
    uint8_t loop_steps,root,scale,reserved;
} wm_state_v4_t;
typedef struct {uint32_t magic;uint16_t version,size;wm_state_v4_t state;} wm_disk_v4_t;
typedef struct {uint32_t magic;uint16_t version,size;wm_state_t state;} wm_disk_t;
typedef struct {uint8_t note,velocity;} wm_event_v3_t;
typedef struct {wm_event_v3_t e[16];} wm_phrase_v3_t;
typedef struct {
    uint32_t magic;
    wm_phrase_v3_t anchor,memory,layer,branch;
    uint8_t has_anchor,has_branch,layer_enabled;
    uint8_t melt,drift,decay,bpm,branch_melt;
    uint8_t light,extra_large,reduced_motion,sound;
    uint8_t fx_on,chorus_mix,delay_mix,reverb_mix;
    uint8_t delay_div,delay_feedback,reverb_size,reverb_damp;
} wm_state_v3_t;
typedef struct {uint32_t magic;uint16_t version,size;wm_state_v3_t state;} wm_disk_v3_t;
/* The previous 0.7 slot is read at its original byte length and migrated. */
typedef struct {
    uint32_t magic;
    wm_phrase_v3_t anchor,memory,layer,branch;
    uint8_t has_anchor,has_branch,layer_enabled;
    uint8_t melt,drift,decay,bpm,branch_melt;
    uint8_t light,extra_large,reduced_motion,sound;
} wm_state_legacy_t;
typedef struct {uint32_t magic;uint16_t version,size;wm_state_legacy_t state;} wm_disk_legacy_t;
static uint8_t ws_patch_valid(const ws_patch_t *p)
{
 const uint8_t *bytes=(const uint8_t*)p;
 for(uint32_t i=0;i<sizeof(ws_patch_t)-6u;i++)if(bytes[i]>127u)return 0;
 for(uint32_t i=0;i<6;i++)if(p->morph[i]==-128)return 0;
 return p->engine<5u&&p->wave<6u&&p->filter<3u&&p->lfo_dest<3u&&p->algorithm<8u;
}
static __attribute__((noinline)) uint8_t wm_valid(const wm_disk_t *d)
{
    uint32_t i;
    if(d->magic!=WM_MAGIC||d->version!=WM_DISK_VERSION||d->size!=sizeof(wm_state_t)||
       d->state.magic!=WM_MAGIC||d->state.bpm<60u||d->state.bpm>180u||
       d->state.melt>127u||d->state.drift>127u||d->state.decay>127u||d->state.branch_melt>127u||
       d->state.has_anchor>1u||d->state.has_branch>1u||d->state.layer_enabled>1u||
       d->state.light>1u||d->state.extra_large>1u||d->state.reduced_motion>1u||
       (d->state.loop_steps!=16u&&d->state.loop_steps!=32u)||d->state.root>11u||
       d->state.legacy>1u||d->state.patch_valid>1u||d->state.anchor_patch_valid>1u||d->state.keyboard_scale>1u||d->state.repeat_style>=WM_SCATTER_MODES||d->state.transpose_base>11u||d->state.anchor_legacy>1u||d->state.anchor_sound>=100u||
       d->state.scale>=WM_SCALE_COUNT||d->state.grain_size>=WM_GRAIN_COUNT||d->state.sound>=WM_PRESET_COUNT||d->state.fx_on>7u||
       d->state.chorus_mix>127u||d->state.delay_mix>127u||d->state.reverb_mix>127u||
       d->state.delay_div>3u||d->state.delay_feedback>127u||
       d->state.reverb_size>127u||d->state.reverb_damp>127u||
       d->state.scatter_edge>127u||d->state.scatter_mix>127u||d->state.scatter_tone>127u||d->state.scatter_variation>127u||
       d->state.arp_on>1u||d->state.arp_mode>=WM_ARP_MODES||d->state.arp_rate>=WM_ARP_RATES||
       d->state.arp_octaves<1u||d->state.arp_octaves>4u||d->state.arp_gate<5u||d->state.arp_gate>100u||
       d->state.arp_swing>50u||d->state.arp_latch>1u||d->state.arp_clock>2u)return 0;
    if((d->state.patch_valid&&!ws_patch_valid(&d->state.patch))||(d->state.anchor_patch_valid&&!ws_patch_valid(&d->state.anchor_patch)))return 0;
    const wm_phrase_t *phrases[4]={&d->state.anchor,&d->state.memory,&d->state.layer,&d->state.branch};
    for(uint32_t p=0;p<4u;p++)for(i=0;i<WM_STEPS;i++)for(uint32_t v=0;v<WM_POLY;v++){
        wm_event_t e=wm_get(phrases[p],i,v);
        if(e.note>127u||e.velocity>127u||e.gate>8192u)return 0;

    }
    return 1;
}

static __attribute__((noinline)) wm_phrase_t wm_migrate_phrase(const wm_phrase_v3_t *p)
{
    wm_phrase_t out={{{0,0,0}}};
    for(uint32_t i=0;i<16u;i++)out.e[i]=(wm_event_t){p->e[i].note,p->e[i].velocity,192u};
    return out;
}
#define WM_COPY_OLD(s,o) do { \
    (s).anchor=wm_migrate_phrase(&(o).anchor);(s).memory=wm_migrate_phrase(&(o).memory); \
    (s).layer=wm_migrate_phrase(&(o).layer);(s).branch=wm_migrate_phrase(&(o).branch); \
    (s).has_anchor=(o).has_anchor;(s).has_branch=(o).has_branch;(s).layer_enabled=(o).layer_enabled; \
    (s).melt=(o).melt;(s).drift=(o).drift;(s).decay=(o).decay;(s).bpm=(o).bpm; \
    (s).branch_melt=(o).branch_melt;(s).light=(o).light;(s).extra_large=(o).extra_large; \
    (s).reduced_motion=(o).reduced_motion;(s).sound=(o).sound; \
} while(0)
static __attribute__((noinline)) const wm_state_t *wm_migrate_legacy_ptr(const wm_disk_legacy_t *d)
{
    static const wm_state_t reset=WM_DEFAULTS;static wm_state_t s;s=reset;WM_COPY_OLD(s,d->state);
    s.legacy=1;s.sound=d->version==2u?d->state.sound:0u;s.anchor_legacy=1;s.anchor_sound=s.sound;s.scale=0;return &s;
}
static __attribute__((noinline)) wm_state_t wm_migrate_legacy(const wm_disk_legacy_t *d)
{return *wm_migrate_legacy_ptr(d);}
static __attribute__((noinline)) const wm_state_t *wm_migrate_v3_ptr(const wm_disk_v3_t *d)
{
    static const wm_state_t reset=WM_DEFAULTS;static wm_state_t s;s=reset;WM_COPY_OLD(s,d->state);
    s.fx_on=d->state.fx_on;s.chorus_mix=d->state.chorus_mix;
    s.delay_mix=d->state.delay_mix;s.reverb_mix=d->state.reverb_mix;
    s.delay_div=d->state.delay_div;s.delay_feedback=d->state.delay_feedback;
    s.reverb_size=d->state.reverb_size;s.reverb_damp=d->state.reverb_damp;
    s.legacy=1;s.anchor_legacy=1;s.anchor_sound=s.sound;s.scale=0;return &s;
}
static __attribute__((noinline)) wm_state_t wm_migrate_v3(const wm_disk_v3_t *d)
{return *wm_migrate_v3_ptr(d);}
static __attribute__((noinline)) uint8_t wm_valid_legacy(const wm_disk_legacy_t *d)
{
    static wm_disk_t candidate={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),{0}};
    if(d->magic!=WM_MAGIC||(d->version!=1u&&d->version!=2u)||
       d->size!=sizeof(wm_state_legacy_t)||d->state.magic!=WM_MAGIC)return 0;
    candidate.state=*wm_migrate_legacy_ptr(d);return wm_valid(&candidate);
}
static __attribute__((noinline)) uint8_t wm_valid_v3(const wm_disk_v3_t *d)
{
    static wm_disk_t candidate={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),{0}};
    if(d->magic!=WM_MAGIC||d->version!=3u||d->size!=sizeof(wm_state_v3_t)||d->state.magic!=WM_MAGIC)return 0;
    candidate.state=*wm_migrate_v3_ptr(d);return wm_valid(&candidate);
}

static __attribute__((noinline)) wm_phrase_t wm_migrate_phrase_v4(const wm_phrase_v4_t *p)
{
    wm_phrase_t out={.e={{0,0,0}}};for(uint32_t i=0;i<WM_STEPS;i++)out.e[i]=p->e[i];return out;
}
static __attribute__((noinline)) const wm_state_t *wm_migrate_v4_ptr(const wm_disk_v4_t *d)
{
    static const wm_state_t reset=WM_DEFAULTS;static wm_state_t s;s=reset;
    s.anchor=wm_migrate_phrase_v4(&d->state.anchor);s.memory=wm_migrate_phrase_v4(&d->state.memory);
    s.layer=wm_migrate_phrase_v4(&d->state.layer);s.branch=wm_migrate_phrase_v4(&d->state.branch);
    s.has_anchor=d->state.has_anchor;s.has_branch=d->state.has_branch;s.layer_enabled=d->state.layer_enabled;
    s.melt=d->state.melt;s.drift=d->state.drift;s.decay=d->state.decay;s.bpm=d->state.bpm;
    s.branch_melt=d->state.branch_melt;s.light=d->state.light;s.extra_large=d->state.extra_large;
    s.reduced_motion=d->state.reduced_motion;s.sound=d->state.sound;
    s.fx_on=d->state.fx_on;s.chorus_mix=d->state.chorus_mix;s.delay_mix=d->state.delay_mix;s.reverb_mix=d->state.reverb_mix;
    s.delay_div=d->state.delay_div;s.delay_feedback=d->state.delay_feedback;s.reverb_size=d->state.reverb_size;s.reverb_damp=d->state.reverb_damp;
    s.loop_steps=d->state.loop_steps;s.root=d->state.root;s.scale=d->state.scale;s.grain_size=d->state.reserved;
    s.legacy=1;s.anchor_legacy=1;s.anchor_sound=s.sound;return &s;
}
static __attribute__((noinline)) wm_state_t wm_migrate_v4(const wm_disk_v4_t *d)
{return *wm_migrate_v4_ptr(d);}
static __attribute__((noinline)) uint8_t wm_valid_v4(const wm_disk_v4_t *d)
{
    static wm_disk_t candidate={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),{0}};
    if(d->magic!=WM_MAGIC||d->version!=4u||d->size!=sizeof(wm_state_v4_t)||d->state.magic!=WM_MAGIC||d->state.reserved)return 0;
    candidate.state=*wm_migrate_v4_ptr(d);return wm_valid(&candidate);
}

/* Migration/validation workspaces are main-thread-only static storage, never ISR work.
 * Avoid large struct-return temporaries on the hardware stacks. */
/* Byte-for-byte legacy v5 layout. A migrated Decay remains Decay. */
static __attribute__((noinline)) const wm_state_t *wm_migrate_v5_ptr(const wm_disk_v5_t *d)
{
 static const wm_state_t reset=WM_DEFAULTS;static wm_state_t s;s=reset;
 const uint8_t *src=(const uint8_t*)&d->state;uint8_t *dst=(uint8_t*)&s;
 for(uint32_t i=0;i<sizeof(wm_state_v5_t);i++)dst[i]=src[i];
 s.legacy=1;s.anchor_legacy=1;s.anchor_sound=s.sound;return &s;
}
static __attribute__((noinline)) wm_state_t wm_migrate_v5(const wm_disk_v5_t *d)
{return *wm_migrate_v5_ptr(d);}
static __attribute__((noinline)) uint8_t wm_valid_v5(const wm_disk_v5_t *d)
{
 if(d->magic!=WM_MAGIC||d->version!=5u||d->size!=sizeof(wm_state_v5_t)||d->state.reserved)return 0;
 static wm_disk_t c={WM_MAGIC,WM_DISK_VERSION,sizeof(wm_state_t),{0}};c.state=*wm_migrate_v5_ptr(d);return wm_valid(&c);
}

/* v6/v7 import is read-only and uses a static workspace, preserving boot stack bounds. */
static __attribute__((noinline)) const wm_state_t *wm_migrate_v7_ptr(const wm_disk_v7_t *d)
{
 if(d->magic!=WM_MAGIC||(d->version!=6u&&d->version!=7u)||d->size!=sizeof(wm_state_v7_t)||
    d->state.repeat_style>2u||(d->version==6u&&d->state.grain_size))return 0;
 static wm_disk_t c;static const wm_state_t reset=WM_DEFAULTS;
 c.magic=WM_MAGIC;c.version=WM_DISK_VERSION;c.size=sizeof(wm_state_t);c.state=reset;
 memcpy(&c.state,&d->state,(uint32_t)((uint8_t*)&c.state.scatter_edge-(uint8_t*)&c.state));
 if(d->version==6u)c.state.grain_size=0;
 return wm_valid(&c)?&c.state:0;
}
static __attribute__((noinline)) uint8_t wm_valid_v7(const wm_disk_v7_t *d)
{return wm_migrate_v7_ptr(d)!=0;}
/* Internal compatibility helpers for callers constructing the current struct. */
static __attribute__((noinline)) const wm_state_t *wm_migrate_v6_ptr(const wm_disk_t *d)
{
 if(d->version!=6u||d->state.grain_size)return 0;
 static wm_disk_v7_t old;old.magic=d->magic;old.version=6;old.size=sizeof(wm_state_v7_t);
 memcpy(&old.state,&d->state,sizeof old.state);return wm_migrate_v7_ptr(&old);
}
static __attribute__((noinline)) uint8_t wm_valid_v6(const wm_disk_t *d)
{return wm_migrate_v6_ptr(d)!=0;}
