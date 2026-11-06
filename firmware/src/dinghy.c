/* SPDX-License-Identifier: GPL-3.0-only
 * LVM Wanderer: an event-memory instrument on Dinghy's Felucca platform.
 * Copyright (C) 2026 Nicholas James, Low Vision Music.
 * Dinghy / platform credits and GPL notices remain in LICENSING.md.
 */
#define APP_NAME "LVM WANDERER"
#define STORE_MAGIC 0x314d564cu
#define DATA_FAMILY "LVM"
#define LCD_IDLE app_idle
static void app_idle(void);
#include "platform.c"
static void project_mark(void);
static void project_undo(void);
#define WM_MARK_HOOK() project_mark()
#define WM_UNDO_HOOK() project_undo()
#include "wanderer_memory.h"
#include "wanderer_themes.h"

/* Atkinson Hyperlegible, SIL OFL. Offline generated 4-bit run-length glyphs. */
typedef struct {uint8_t h,n;const char *chars;const uint8_t *adv,*bw;const uint32_t *off;const uint8_t *data;} font_t;
#include "wanderer_font.h"
#include "wanderer_icons.h"
#include "wanderer_waves.h"
#include "wanderer_presets.h"
#include "wanderer_filter.h"
#define NV 4u
#define KEY0 53u
#define DARK RGB(0,0,0)
#define PALE RGB(231,245,242)
#define INK RGB(13,29,35)
#define CYAN RGB(105,232,218)
#define BLUE RGB(166,175,250)
#define GOLD RGB(255,202,113)
enum {
    N_NONE, N_ORIGINAL_SAFE, N_ANCHORED, N_ANCHOR_SAFE, N_NO_PHRASE,
    N_BRANCHED, N_CATCH_FIRST, N_UNDO, N_NO_UNDO, N_STILL, N_LIVE,
    N_LAYER_CLEARED, N_NEW_SESSION, N_SAVE_FAILED, N_SAVED,
    N_NO_VALID_SAVE, N_LOADED, N_STOP_FIRST, N_PREF_FAILED, N_AUDIO_LOCK, N_ANCHOR_LISTEN,
    N_RETURNED, N_NOTHING_TO_KEEP, N_THEME
};
enum {PATCH_MEMORY,PATCH_LAYER,PATCH_ANCHOR};
static struct {
    volatile uint32_t inc;volatile int32_t gate;
    uint32_t ph,ph2,ph3,lfo,inc2,age,remain;
    int32_t amp,f1,f2,tone,brightness,fm_brightness,last,splice;
    uint8_t note,live,patch,band1,band2,stage,sustained,splice_left,seq_lane,patch_source;
} voice[NV];
static uint32_t motion_phase;
static uint32_t ages,step_clock,step_index,cycle_index,step_samples=5906u;
static uint8_t step_primed;
static uint32_t audio_frames;
static uint32_t capture_step(void);
static void ww_clear_slot(uint32_t step,uint32_t slot);
#define CAPTURE_COUNT (2u*WM_POLY)
static uint8_t record_undo_pending;
static uint8_t record_replace[2][WM_STEPS];
static uint32_t capture_serial;
/* Capture ownership is separate from the four sound voices: voice stealing cannot
 * lose a key release or sustain-pedal release. Each lane captures four held notes. */
static struct {uint32_t start,serial;uint8_t active,owner,note,step,slot,sustained,cleared;} capture[CAPTURE_COUNT];
static uint32_t capture_count(void)
{uint32_t n=0;for(uint32_t c=0;c<CAPTURE_COUNT;c++)n+=capture[c].active!=0;return n;}
static uint8_t capture_holds(uint32_t lane,uint32_t step,uint32_t slot);
static void capture_update(uint32_t c)
{
    if(!capture[c].active)return;
    uint32_t lane=c/WM_POLY;
    wm_phrase_t *p=lane?&wm.layer:&wm.memory;
    uint32_t frames=audio_frames-capture[c].start,limit=step_samples*wm.loop_steps;
    if(frames>limit)frames=limit;
    uint32_t ticks=(frames*256u+step_samples/2u)/step_samples;
    if(ticks<32u)ticks=32u;
    if(ticks>wm.loop_steps*256u)ticks=wm.loop_steps*256u;
    wm_at(p,capture[c].step,capture[c].slot)->gate=(uint16_t)ticks;
    /* Clear only covered repeats of this pitch; other chord/overdub voices survive.
     * Visit each crossed step once, keeping the audio callback's work bounded. */
    uint32_t crossed=(ticks-1u)/256u;
    if(crossed>=wm.loop_steps)crossed=wm.loop_steps-1u;
    while(capture[c].cleared<crossed){
        uint32_t step=(capture[c].step+(++capture[c].cleared))%wm.loop_steps;
        for(uint32_t v=0;v<WM_POLY;v++){
            wm_event_t *e=wm_at(p,step,v);
            if(e->velocity&&e->note==capture[c].note&&!capture_holds(lane,step,v))e->velocity=0;
        }
    }
}
static void capture_finish(uint32_t c)
{capture_update(c);capture[c].active=0;}
static void capture_finish_all(void)
{for(uint32_t c=0;c<CAPTURE_COUNT;c++)capture_finish(c);}
static uint8_t capture_holds(uint32_t lane,uint32_t step,uint32_t slot)
{
    for(uint32_t c=lane*WM_POLY;c<(lane+1u)*WM_POLY;c++)
        if(capture[c].active&&capture[c].step==step&&capture[c].slot==slot)return 1;
    return 0;
}
static uint32_t capture_age(uint32_t c)
{return capture_serial-capture[c].serial;}
static void capture_on_at(uint32_t note,uint32_t velocity,uint32_t owner,uint32_t at)
{
    if(note>127u||!velocity||velocity>127u||owner>33u||at>=wm.loop_steps)return;
    uint32_t lane=layer_focus,slot=WM_POLY;
    wm_phrase_t *p=lane?&wm.layer:&wm.memory;
    if(record_undo_pending){
        wm_mark();
        if(!wm_has_notes(&wm.memory)&&!wm_has_notes(&wm.layer))wm.transpose_base=wm.root;
        record_undo_pending=0;
    }
    /* A re-struck input owns one capture even across target changes. */
    for(uint32_t c=0;c<CAPTURE_COUNT;c++)
        if(capture[c].active&&capture[c].owner==owner&&capture[c].note==note)capture_finish(c);
    for(uint32_t v=0;v<WM_POLY;v++){
        wm_event_t e=wm_get(p,at,v);
        if(e.velocity&&e.note==note){slot=v;break;}
    }
    if(slot==WM_POLY)for(uint32_t v=0;v<WM_POLY;v++)if(!wm_get(p,at,v).velocity){slot=v;break;}
    if(slot==WM_POLY){
        /* Prefer an old released position before shortening a currently held note. */
        uint32_t start=record_replace[lane][at]%WM_POLY;
        for(uint32_t n=0;n<WM_POLY;n++){
            uint32_t v=(start+n)%WM_POLY;
            if(!capture_holds(lane,at,v)){slot=v;break;}
        }
        if(slot==WM_POLY){
            uint32_t oldest=lane*WM_POLY;
            for(uint32_t c=oldest+1u;c<(lane+1u)*WM_POLY;c++)
                if(capture_age(c)>capture_age(oldest))oldest=c;
            slot=capture[oldest].slot;
        }
        record_replace[lane][at]=(uint8_t)(slot+1u);
    }
    uint32_t pick=lane*WM_POLY,oldest_age=0;
    for(uint32_t c=lane*WM_POLY;c<(lane+1u)*WM_POLY;c++){
        if(capture[c].active&&capture[c].step==at&&capture[c].slot==slot)capture_finish(c);
    }
    for(uint32_t c=lane*WM_POLY;c<(lane+1u)*WM_POLY;c++){
        if(!capture[c].active){pick=c;break;}
        if(capture_age(c)>=oldest_age){oldest_age=capture_age(c);pick=c;}
    }
    capture_finish(pick);
    *wm_at(p,at,slot)=(wm_event_t){(uint8_t)note,(uint8_t)velocity,32u};
    if(!lane)ww_clear_slot(at,slot);
    capture[pick].start=audio_frames;capture[pick].serial=++capture_serial;capture[pick].active=1;
    capture[pick].owner=owner;capture[pick].note=note;capture[pick].step=at;capture[pick].slot=slot;
    capture[pick].sustained=capture[pick].cleared=0;
    capture_update(pick);
}
static void capture_on(uint32_t note,uint32_t velocity,uint32_t owner)
{capture_on_at(note,velocity,owner,capture_step());}
static void capture_off(uint32_t note,uint32_t owner,uint8_t sustain)
{
    for(uint32_t c=0;c<CAPTURE_COUNT;c++)if(capture[c].active&&capture[c].note==note&&capture[c].owner==owner){
        if(sustain)capture[c].sustained=1;else capture_finish(c);
    }
}
static volatile uint32_t cue_left,cue_phase,cue_step;
static volatile uint32_t cue_follow_step,cue_total;
static volatile uint8_t cue_stage,cue_priority;
static volatile uint8_t running,recording,anchor_view,vol=100;
static volatile uint8_t page_dirty=1;
static uint8_t octave,held[27],confirm,focus,focus_value;
static uint8_t settings_open,settings_index,settings_page;
static uint32_t ui_pending;
static uint8_t ui_chord_latched;
static uint8_t zoom_chord_latched;
static uint32_t focus_until;
static uint8_t notice;
static uint32_t notice_until;
static uint32_t fx_down,arp_down;static uint8_t arp_pending;
static uint32_t home_down,anchor_down,save_down;
static uint32_t octave_up_down,octave_down_down,play_down,performance_pending;
static uint8_t play_tap,play_recall,anchor_pending,sel_can_open,edit_layer_done;
static uint8_t save_pending;
static uint8_t project_slot,cues_enabled=1;
/* Metronome: 0 off, 1 while recording (with count-in), 2 whenever playing.
 * click_level 0..3 = 25..100%. The click joins the output after every effect,
 * so it is never printed, scattered or sent to the reverb. */
static uint8_t metronome_on,click_level=2;
static uint32_t click_left,click_phase,click_step;static int32_t click_env,click_amp;
static volatile uint32_t count_in_pos,count_in_total;
static uint8_t count_in_shown;
static void click_trigger(uint8_t accent)
{
    click_left=FS/10u;click_phase=0;click_step=accent?0x095c0000u:0x063d0000u; /* ~1.6 kHz / ~1.07 kHz */
    click_env=32767;click_amp=(accent?11000:7000)*(int32_t)(click_level+1u)/4;
}
static uint8_t metronome_wants(void){return (uint8_t)(metronome_on==2u||(metronome_on==1u&&recording));}
static uint8_t display_hold,cue_detail=1,cue_level=64,zoom_steady=1;
static uint32_t access_signature(void)
{return (uint32_t)display_hold|((uint32_t)cue_detail<<2)|((uint32_t)zoom_steady<<3)|((uint32_t)cue_level<<4)|((uint32_t)metronome_on<<11)|((uint32_t)color_theme<<13)|((uint32_t)click_level<<16);}
static uint8_t ui_time_before(uint32_t deadline)
{return !deadline||(int32_t)(fm1_ms-deadline)<0;}
static uint32_t display_deadline(void)
{
 static const uint16_t ms[3]={2000,5000,10000};
 if(display_hold==3u)return 0;
 uint32_t deadline=fm1_ms+ms[display_hold<3u?display_hold:0u];
 return deadline?deadline:1u;
}
static void cue_stop(void){cue_left=0;cue_stage=0;cue_priority=0;}
static uint8_t sustain_pedal[32];
static int32_t master_smooth=25600;
static uint32_t prev_notes;
/* A slot presence cache is read once at boot, then updated after explicit I/O.
 * Browsing a slot never reads flash while music is running. */
static uint8_t slot_presence[NSLOT]; /* 0 unknown/write problem, 1 no valid project, 2 valid */
static uint8_t project_origin_slot=255u,project_dirty,preferences_pending;
static uint8_t clean_level=100u,clean_cues=1u;
static uint32_t clean_access,access_undo;
static wm_state_t project_checkpoint=WM_DEFAULTS,checkpoint_undo=WM_DEFAULTS;
static uint8_t origin_undo=255u,level_undo=100u,cues_undo=1u,prefs_undo;
static uint32_t project_save_serial,undo_save_serial,last_project_poll;
static union {wm_disk_t current;wm_disk_v14_t v14;wm_disk_v13_t v13;wm_disk_v11_t v11;wm_disk_v10_t v10;wm_disk_v8_t v8;wm_disk_v7_t v7;wm_disk_v5_t v5;wm_disk_v4_t v4;wm_disk_v3_t v3;wm_disk_legacy_t old;} project_read_buffer;
static uint8_t project_read_slot(uint32_t slot,wm_state_t *out)
{
    if(slot>=NSLOT)return 0;
    uint8_t current_read=(uint8_t)!slot_load(slot,&project_read_buffer.current,sizeof project_read_buffer.current);
    if(current_read&&wm_valid(&project_read_buffer.current)){
        if(out)*out=project_read_buffer.current.state;
        return 1;
    }
    if(!slot_load(slot,&project_read_buffer.v14,sizeof project_read_buffer.v14)){
        const wm_state_t *m=wm_migrate_v14_ptr(&project_read_buffer.v14);if(m){if(out)*out=*m;return 1;}
    }
    if(!slot_load(slot,&project_read_buffer.v13,sizeof project_read_buffer.v13)){
        const wm_state_t *m=wm_migrate_v13_ptr(&project_read_buffer.v13);if(m){if(out)*out=*m;return 1;}
    }
    if(!slot_load(slot,&project_read_buffer.v11,sizeof project_read_buffer.v11)){
        const wm_state_t *m=wm_migrate_v11_v12_ptr(&project_read_buffer.v11);if(m){if(out)*out=*m;return 1;}
    }
    if(!slot_load(slot,&project_read_buffer.v10,sizeof project_read_buffer.v10)){
        const wm_state_t *m=wm_migrate_v9_ptr(&project_read_buffer.v10);if(m){if(out)*out=*m;return 1;}
    }
    if(!slot_load(slot,&project_read_buffer.v8,sizeof project_read_buffer.v8)){
        const wm_state_t *migrated=wm_migrate_v8_ptr(&project_read_buffer.v8);
        if(migrated){if(out)*out=*migrated;return 1;}
    }
    if(!slot_load(slot,&project_read_buffer.v7,sizeof project_read_buffer.v7)&&wm_valid_v7(&project_read_buffer.v7)){
        if(out)*out=*wm_migrate_v7_ptr(&project_read_buffer.v7);return 1;
    }
    if(!slot_load(slot,&project_read_buffer.v5,sizeof project_read_buffer.v5)&&wm_valid_v5(&project_read_buffer.v5)){
        if(out)*out=*wm_migrate_v5_ptr(&project_read_buffer.v5);return 1;
    }
    if(!slot_load(slot,&project_read_buffer.v4,sizeof project_read_buffer.v4)&&wm_valid_v4(&project_read_buffer.v4)){
        if(out)*out=*wm_migrate_v4_ptr(&project_read_buffer.v4);
        return 1;
    }
    if(!slot_load(slot,&project_read_buffer.v3,sizeof project_read_buffer.v3)&&wm_valid_v3(&project_read_buffer.v3)){
        if(out)*out=*wm_migrate_v3_ptr(&project_read_buffer.v3);
        return 1;
    }
    if(!slot_load(slot,&project_read_buffer.old,sizeof project_read_buffer.old)&&wm_valid_legacy(&project_read_buffer.old)){
        if(out)*out=*wm_migrate_legacy_ptr(&project_read_buffer.old);
        return 1;
    }
    return 0;
}
static void project_mark(void)
{
    checkpoint_undo=project_checkpoint;origin_undo=project_origin_slot;
    level_undo=clean_level;cues_undo=clean_cues;prefs_undo=preferences_pending;
    access_undo=clean_access;
    undo_save_serial=project_save_serial;
}
static void project_undo(void)
{
    /* Saving commits a new reference. Undo after Save edits that reference,
     * whereas Undo Load/New Session restores the previous project's identity. */
    if(undo_save_serial==project_save_serial){
        project_checkpoint=checkpoint_undo;project_origin_slot=origin_undo;
        clean_level=level_undo;clean_cues=cues_undo;preferences_pending=prefs_undo;
        clean_access=access_undo;
    }
}
static void project_poll(void)
{
    fm1_irq_off();
    uint8_t dirty=(uint8_t)(memcmp(&wm,&project_checkpoint,sizeof wm)!=0||
        vol!=clean_level||cues_enabled!=clean_cues||access_signature()!=clean_access||preferences_pending);
    fm1_irq_on();
    if(dirty!=project_dirty){project_dirty=dirty;page_dirty=1;}
    last_project_poll=fm1_ms;
}
static void project_initialise(void)
{
    project_checkpoint=wm;project_origin_slot=255u;project_dirty=preferences_pending=0;
    clean_level=vol;clean_cues=cues_enabled;project_save_serial=0;
    clean_access=access_signature();
    for(uint32_t i=0;i<NSLOT;i++)slot_presence[i]=project_read_slot(i,0)?2u:1u;
}
static const uint32_t INC_C9[12]={815363807u,863847862u,915214929u,969636441u,1027294024u,1088380105u,
  1153098554u,1221665363u,1294309365u,1371273005u,1452813141u,1539201906u};
static uint32_t tone_step(uint32_t note){if(note>127u)note=127u;return INC_C9[note%12u]>>(10u-note/12u);}
static uint8_t wave_band(uint32_t inc)
{
    if(inc<0x7fffffffu/12u)return 4u;
    if(inc<0x7fffffffu/8u)return 3u;
    if(inc<0x7fffffffu/4u)return 2u;
    if(inc<0x7fffffffu/2u)return 1u;
    return 0u;
}
static int32_t wave_sample(uint32_t ph,uint32_t wave,uint32_t band)
{
    uint32_t pos=ph>>24,frac=(ph>>16)&255u;
    int32_t a=WANDERER_WAVE[wave][band][pos];
    int32_t b=WANDERER_WAVE[wave][band][(pos+1u)&255u];
    return a+(((b-a)*(int32_t)frac)>>8);
}
static int32_t sine(uint32_t ph)
{
    int32_t x=(int32_t)ph>>16,ax=x<0?-x:x,y,ay;
    y=(x*(32768-ax))>>13;ay=y<0?-y:y;
    y+=((((y*ay)>>15)-y)*7373)>>15;
    return y>32767?32767:y< -32767?-32767:y;
}
static int32_t osc_sample(uint32_t ph,uint32_t wave,uint32_t band)
{
    if(wave<3u)return wave_sample(ph,wave,band);
    if(wave==WAVE_SINE)return (sine(ph)*13)>>4;
    uint32_t x=ph>>16;
    int32_t tri=(int32_t)(x<32768u?x:65535u-x)-16384;
    return (tri*13)>>3;
}
static void cue(uint32_t pitch)
{
    if(!cues_enabled||!cue_level)return;
    cue_stop();cue_phase=0;cue_step=tone_step(pitch);cue_total=FS/24u;
    cue_priority=2;cue_left=cue_total;
}
static void cue_pair(uint32_t first,uint32_t second,uint8_t priority)
{
 if(!cues_enabled||!cue_level)return;
 cue_stop();cue_phase=0;cue_step=tone_step(first);cue_follow_step=tone_step(second);
 cue_stage=1;cue_priority=priority;cue_total=FS/32u;cue_left=cue_total;
}
static void cue_toggle(uint8_t on)
{
 if(cue_detail)cue_pair(on?64u:76u,on?76u:64u,2);
 else cue(on?76u:64u);
}
static uint32_t cue_boundary_ms;static uint8_t cue_boundary_kind,cue_boundary_valid;
static void cue_boundary(uint32_t old,uint32_t value,uint32_t lo,uint32_t hi)
{
 if(!cues_enabled||!cue_detail||!cue_level||old==value||hi<=lo)return;
 uint8_t kind=value==lo?1u:value==hi?3u:0u;
 uint32_t mid=lo+(hi-lo+1u)/2u;
 if(!kind&&hi-lo>=32u&&((old<mid&&value>=mid)||(old>mid&&value<=mid)))kind=2u;
 if(!kind||(cue_left&&cue_priority>1u))return;
 if(cue_boundary_valid&&kind==cue_boundary_kind&&fm1_ms-cue_boundary_ms<120u)return;
 cue_boundary_valid=1;cue_boundary_kind=kind;cue_boundary_ms=fm1_ms;
 if(kind==1u)cue_pair(60u,48u,1);
 else if(kind==3u)cue_pair(72u,84u,1);
 else{cue(72u);cue_priority=1;}
}
static int32_t cue_sample(void)
{
 if(!cues_enabled||!cue_level){cue_stop();return 0;}
 int32_t sample=cue_step?sine(cue_phase)>>5:0;
 sample=(sample*(int32_t)cue_level)/127;
 uint32_t elapsed=cue_total-cue_left;
 if(elapsed<96u)sample=(sample*(int32_t)elapsed)/96;
 if(cue_left<128u)sample=(sample*(int32_t)cue_left)/128;
 cue_phase+=cue_step;
 if(!--cue_left){
  if(cue_stage==1u){cue_stage=2;cue_step=0;cue_total=FS/100u;cue_left=cue_total;}
  else if(cue_stage==2u){cue_stage=3;cue_phase=0;cue_step=cue_follow_step;cue_total=FS/32u;cue_left=cue_total;}
  else{cue_stage=cue_priority=0;}
 }
 return sample;
}
#include "wanderer_sound.c"
#include "wanderer_wander.h"
/* Owner 0: sequencer; 1: local keys; 2..33: USB/TRS source and channel. */
static uint32_t note_on_lane(uint32_t note,uint32_t vel,uint32_t owner,uint8_t lane)
{
    uint32_t v,pick=0,best=0xffffffffu;
    uint8_t source=lane&&ws_layer_own()?PATCH_LAYER:
        !lane&&anchor_view&&wm.has_anchor?PATCH_ANCHOR:PATCH_MEMORY;
    uint32_t legacy_index=source==PATCH_LAYER?wm.layer_sound-1u:source==PATCH_ANCHOR?wm.anchor_sound:wm.sound;
    const wm_preset_t *p=&WM_PRESETS[legacy_index<WM_LEGACY_PRESET_COUNT?legacy_index:0u];
    if(note>127u||!vel||vel>127u||owner>34u)return NV;
    if(owner)for(v=0;v<NV;v++)if(voice[v].gate&&voice[v].live==owner&&voice[v].note==note){pick=v;goto chosen;}
    for(v=0;v<NV;v++){
        uint32_t score=(!voice[v].gate&&!voice[v].amp)?0u:
          (!voice[v].gate)?1u+(uint32_t)voice[v].amp:
          0x80000000u+voice[v].age;
        if(score<best){best=score;pick=v;}
    }
chosen:
    voice[pick].splice=voice[pick].amp?voice[pick].last:0;
    voice[pick].splice_left=32u;
    voice[pick].note=(uint8_t)note;voice[pick].age=++ages;
    voice[pick].inc=tone_step(note);voice[pick].gate=(int32_t)(vel*64u*256u);
    voice[pick].patch=(uint8_t)legacy_index;
    /* Q4 oscillator ratio: 8 = sub, 16 = unison, 32 = octave.
     * High-register intervals return to unison before phase wrap aliases. */
    uint32_t ratio=note>84u&&p->ratio>16u?16u:p->ratio;
    voice[pick].inc2=(voice[pick].inc>>4)*ratio+
      (voice[pick].inc>>12)*p->detune;
    voice[pick].band1=wave_band(voice[pick].inc);
    voice[pick].band2=wave_band(voice[pick].inc2);
    voice[pick].ph=voice[pick].ph3=0u;
    voice[pick].ph2=ages*2654435761u;voice[pick].lfo=ages*119304647u;
    voice[pick].tone=(int32_t)p->cutoff*256;
    voice[pick].amp=voice[pick].f1=voice[pick].f2=0;
    voice[pick].brightness=voice[pick].fm_brightness=32767*256;
    voice[pick].stage=0u;
    voice[pick].remain=owner?0u:step_samples*3u/4u;
    voice[pick].live=(uint8_t)owner;voice[pick].sustained=0;
    voice[pick].patch_source=source;
    ws_start(pick,note,vel);
    return pick;
}
static uint32_t note_on(uint32_t note,uint32_t vel,uint32_t owner)
{return note_on_lane(note,vel,owner,layer_focus);}
static void note_off(uint32_t note,uint32_t owner)
{
    uint32_t v;
    for(v=0;v<NV;v++)if(voice[v].live==owner&&voice[v].note==note){
        if(owner>=2u&&owner<=33u&&sustain_pedal[owner-2u])voice[v].sustained=1;
        else voice[v].gate=0;
    }
}
#include "wanderer_scatter.h"
#include "wanderer_print.c"
#include "wanderer_arp.c"
/* Key changes made after recording follow the project's root. */
static wm_event_t step_transpose(wm_event_t e,uint32_t lane)
{
    if(!anchor_view&&!(lane&&ws_layer_own()?wm.layer_legacy:wm.legacy)){
        int32_t note=(int32_t)e.note+wm.root-wm.transpose_base;e.note=(uint8_t)ws_clamp(note,0,127);
    }
    return e;
}
static void sequence_event(wm_event_t e,uint32_t lane)
{
    if(!e.velocity)return;
    for(uint32_t v=0;v<NV;v++)if(!voice[v].live&&voice[v].seq_lane==lane&&voice[v].note==e.note)voice[v].gate=0;
    uint32_t v=note_on_lane(e.note,e.velocity,0,(uint8_t)lane);
    if(v<NV){
        uint32_t ticks=e.gate?e.gate:192u;
        if(ticks>wm.loop_steps*256u)ticks=wm.loop_steps*256u;
        voice[v].remain=(ticks*step_samples)/256u;
        voice[v].seq_lane=(uint8_t)lane;
    }
}
static void step_play(void)
{
    uint32_t i=step_index;
    if(!i)wp_boundary();
    /* A completed loop moves Memory one step further (paused while recording,
     * listening to the Anchor or playing a printed RAM loop). */
    if(!i&&cycle_index&&!anchor_view&&!recording&&!wp_active)ww_loop();
    if(metronome_wants()&&!(i&3u))click_trigger((uint8_t)!(i&15u)); /* beat one of every bar is accented */
    scatter_plan(wm.decay,(uint8_t)(wm.legacy||anchor_view||wp_dry_view),i,cycle_index);
    /* Scatter processes audio history, so original held-note gates keep advancing. */
    if(scatter.kind&&wm.repeat_style==1u){static const uint8_t kinds[3]={SC_STUTTER,SC_HOLD,SC_RUSH};scatter.kind=kinds[scatter.offset%3u];}
    if(scatter.kind&&wm.repeat_style==2u){static const uint8_t kinds[3]={SC_HOP,SC_JUMP,SC_DROP};scatter.kind=kinds[scatter.offset%3u];}
    if(scatter.kind&&wm.repeat_style>=3u){
      static const uint8_t kinds[5]={SC_REVERSE,SC_TAPE,SC_CLOUD,SC_PITCH,SC_RUSH};
      scatter.kind=kinds[wm.repeat_style-3u];
    }
    wp_scatter_begin();
    if(!wp_active){
        const wm_phrase_t *main=anchor_view?&wm.anchor:&wm.memory;
        uint32_t main_count=wm_step_notes(main,i),layer_count=!anchor_view&&wm.layer_enabled?wm_step_notes(&wm.layer,i):0;
        uint32_t main_limit=main_count,layer_limit=layer_count;
        if(main_count&&layer_count&&main_count+layer_count>NV){
            main_limit=main_count>3u?3u:main_count;layer_limit=NV-main_limit;
        }
        for(uint32_t lane=0;lane<2u;lane++){
            uint32_t remaining=lane?layer_limit:main_limit;
            const wm_phrase_t *phrase=lane?&wm.layer:main;
            for(uint32_t v=0;v<WM_POLY&&remaining;v++){
                wm_event_t e=wm_get(phrase,i,v);
                if(!e.velocity)continue;
                remaining--;
                if(capture_holds(lane,i,v))continue;
                if(!lane&&!anchor_view)e=wm.legacy?wm_evolve(e,i,cycle_index):ww_apply(e,i,v);
                sequence_event(step_transpose(e,lane),lane);
            }
        }
        /* Melted echoes drip into Memory's empty steps. */
        if(!anchor_view&&ww_active()&&main_count==0u&&layer_count<NV){
            wm_event_t drip=ww_drip_event(i);
            if(drip.velocity)sequence_event(step_transpose(drip,0),0);
        }
    }
    step_index=(i+1u)&(wm.loop_steps-1u);
    if(!step_index)cycle_index++;
}
#include "wanderer_fx.c"
#include "wanderer_scope.c"
static void app_audio_core(int32_t *out,uint32_t n)
{
    audio_frames+=n;for(uint32_t c=0;c<CAPTURE_COUNT;c++)capture_update(c);
    /* Melt timbre deepens loop by loop (wander) and colours Memory's voices.
     * Layer 2 stays the steady companion; the Anchor is heard as recorded. */
    uint32_t v,i;int32_t melt=anchor_view?0:(int32_t)wm.branch_melt+(wm.legacy?wm.melt:ww.timbre);
    if(melt>127)melt=127;
    for(v=0;v<NV;v++){
        /* Silent slots can retain a modern 100..149 index after a reset. Never
         * dereference the frozen 100-entry legacy bank for an inactive slot. */
        if(!voice[v].amp&&!voice[v].gate)continue;
        if(ws_voice[v].modern){ws_render(v,out,n,voice[v].patch_source==PATCH_MEMORY?melt:0);continue;}
        uint32_t ph=voice[v].ph,ph2=voice[v].ph2,ph3=voice[v].ph3,inc=voice[v].inc,
          inc2=voice[v].inc2,remain=voice[v].remain,patch=voice[v].patch;
        int32_t amp=voice[v].amp,g=voice[v].gate,f1=voice[v].f1,
          f2=voice[v].f2,tone=voice[v].tone,bright=voice[v].brightness,
          fmbright=voice[v].fm_brightness;
        uint8_t stage=voice[v].stage;
        uint32_t splice_left=voice[v].splice_left;
        int32_t last=voice[v].last,splice=voice[v].splice;
        const wm_preset_t *p=&WM_PRESETS[patch];
        int32_t sustain=(g*(int32_t)p->sustain)>>7;
        int32_t motion=sine(voice[v].lfo)>>8;
        voice[v].lfo+=p->lfo_step*n;
        /* Subtle vibrato, per patch; no universal pitch wobble. */
        int32_t pitch_mod=(motion*(int32_t)p->vibrato)>>4;
        inc=(uint32_t)((int32_t)inc+((int32_t)(inc>>16)*pitch_mod)/1731);
        inc2=(uint32_t)((int32_t)inc2+((int32_t)(inc2>>16)*pitch_mod)/1731);
        uint32_t pulse_width=(uint32_t)(128+((motion*(int32_t)p->pwm)>>7))<<24;
        for(i=0;i<n;i++){
            int32_t s,cut;
            if(remain&&!--remain){g=0;voice[v].gate=0;}
            if(!g)stage=3u;
            if(stage==0u){
                amp+=p->attack;
                if(amp>=g){amp=g;stage=1u;}
            }else if(stage==1u){
                int32_t delta=(((amp-sustain)>>6)*(int32_t)p->decay)>>10;
                amp-=delta>0?delta:1;
                if(amp<=sustain+64){amp=sustain;stage=2u;}
            }else if(stage==2u)amp=sustain;
            else{
                int32_t delta=((amp>>6)*(int32_t)p->release)>>10;
                amp-=delta>0?delta:1;if(amp<64)amp=0;
            }
            ph+=inc;ph2+=inc2;ph3+=inc>>1;
            uint32_t offset=0u;
            if(p->fm&&voice[v].note<=96u){
                uint32_t fm=(uint32_t)(((fmbright>>8)*(int32_t)p->fm)>>15);
                if(voice[v].note>84u)fm>>=1u;
                offset=(uint32_t)(sine(ph2)*(int32_t)fm)*256u;
            }
            int32_t a=p->osc1==WAVE_PULSE?
                (wave_sample(ph,0,voice[v].band1)-wave_sample(ph+pulse_width,0,voice[v].band1))/2:
                osc_sample(ph+offset,p->osc1,voice[v].band1);
            int32_t b=p->osc2==WAVE_PULSE?
                (wave_sample(ph2,0,voice[v].band2)-wave_sample(ph2+pulse_width,0,voice[v].band2))/2:
                osc_sample(ph2,p->osc2,voice[v].band2);
            s=(a*(128-p->mix)+b*p->mix)>>7;
            if(p->sub)s=(s*(128-p->sub)+(sine(ph3)*p->sub*13>>4))>>7;
            /* Exponential filter/FM contours with sub-sample amplitude precision. */
            if(bright>0){int32_t d=((bright>>6)*(int32_t)p->contour_decay)>>10;bright-=d>0?d:1;}
            if(fmbright>0){int32_t d=((fmbright>>6)*(int32_t)p->fm_decay)>>10;fmbright-=d>0?d:1;}
            cut=p->cutoff+(((bright>>8)*(int32_t)p->contour)>>15)+(melt>>2)+
                ((motion*(int32_t)p->lfo_filter)>>7)+((int32_t)voice[v].note-60)/3;
            /* Velocity subtly opens timbre, rather than only changing loudness. */
            cut+=(voice[v].gate/16384-100)/8;
            if(cut<0)cut=0;
            if(cut>220)cut=220;
            tone+=((cut*256-tone)>>6);cut=tone>>8;
            /* Bounded soft saturation into a resonant topology-preserving SVF. */
            s=(s*(128+p->drive))>>7;
            if(s>28000)s=28000;
            if(s< -28000)s=-28000;
            s-=((((s*s)>>15)*s)>>15)/3;
            int32_t cf=FILTER_G[cut],h=FILTER_H[p->resonance][cut];
            int32_t v1=((f1+((cf*(s-f2))>>14))*h)>>14;
            int32_t v2=f2+((cf*v1)>>14);
            f1=2*v1-f1;f2=2*v2-f2;
            if(f1>65535)f1=65535;
            if(f1< -65535)f1=-65535;
            if(f2>65535)f2=65535;
            if(f2< -65535)f2=-65535;
            s=(v2*(amp>>8))>>15;
            s=(s*(int32_t)p->level)>>7;
            s=(s*600)>>9;
            /* A 32-sample splice keeps retriggers/voice steals continuous. */
            if(splice_left){s=(s*(int32_t)(32u-splice_left)+splice*(int32_t)splice_left)>>5;splice_left--;}
            last=s;
            out[i*2u]+=s;
        }
        voice[v].ph=ph;voice[v].ph2=ph2;voice[v].ph3=ph3;voice[v].tone=tone;voice[v].amp=amp;
        voice[v].f1=f1;voice[v].f2=f2;voice[v].brightness=bright;
        voice[v].fm_brightness=fmbright;voice[v].stage=stage;
        voice[v].remain=remain;
        voice[v].last=last;voice[v].splice_left=(uint8_t)splice_left;
    }
    wp_input(out,n);
    fx_process(out,n);
    wp_output(out,n);
    for(i=0;i<n;i++){
        int32_t left=out[i*2u],right=out[i*2u+1u];
        int32_t target=(int32_t)vol*256,diff=target-master_smooth;
        if(diff> -64&&diff<64)master_smooth=target;
        else master_smooth+=diff>>6;
        if(scope_open&&!scope_hold){
            int32_t probe_left=fx_limit((left*master_smooth)/25600);
            int32_t probe_right=fx_limit((right*master_smooth)/25600);
            scope_feed((probe_left+probe_right)/2);
            if(!cue_left&&!click_left){
                out[i*2u]=probe_left;out[i*2u+1u]=probe_right;
                continue;
            }
        }
        if(cue_left){
            int32_t audible=cue_sample();left+=audible;right+=audible;
        }
        if(click_left){
            if(!metronome_on)click_left=0;
            else{
                /* Woodblock-like tick: a short noise transient and a sine body
                 * with an exponential decay, never a hard-edged beep. */
                int32_t body=(int32_t)(((int64_t)wn_sine_i(click_phase)*click_env)>>15);
                int32_t tick=(body*click_amp)>>15;
                if(FS/10u-click_left<48u)tick+=(int32_t)((click_phase*2654435761u)>>21)-1024;
                click_phase+=click_step;click_env-=click_env>>9;click_left--;
                if(click_env<64)click_left=0;
                left+=tick;right+=tick;
            }
        }
        out[i*2u]=fx_limit((left*master_smooth)/25600);
        out[i*2u+1u]=fx_limit((right*master_smooth)/25600);
    }
}
static void app_idle(void); /* defined with local performance input below */

/* Glyphs are blended into the band, with measured advances and no scale-up jaggies. */
static uint32_t letter(const font_t *f,uint8_t c)
{
    uint32_t i;
    if(c>='a'&&c<='z')c-=32;
    for(i=0;i<f->n;i++)if((uint8_t)f->chars[i]==c)return i;
    for(i=0;i<f->n;i++)if(f->chars[i]=='?')return i;
    return 0;
}
/* Split at musical boundaries: repeat timing never depends on callback size. */
static void count_in_finish(void);
static void app_audio(int32_t *out,uint32_t n)
{
 /* Count-in: one bar of clicks before recording starts on the next downbeat. */
 if(count_in_total&&!running){
  uint32_t beat=step_samples*4u;
  while(n&&count_in_total){
   if(!(count_in_pos%beat))click_trigger((uint8_t)!count_in_pos);
   uint32_t chunk=beat-count_in_pos%beat;if(chunk>n)chunk=n;if(chunk>256u)chunk=256u;
   app_audio_core(out,chunk);out+=chunk*2u;n-=chunk;count_in_pos+=chunk;
   if(count_in_pos>=count_in_total)count_in_finish();
  }
 }
 while(n){
  arp_update_config();
  if(arp_gate_left==0u&&arp_note<128u)arp_release();
  arp_tick();
  if(running){
   if(!step_primed){step_play();step_primed=1;}
   if(step_clock>=step_samples){step_clock-=step_samples;step_play();}
  }
  uint32_t chunk=n>256u?256u:n;
  if(running){uint32_t left=step_samples-step_clock;if(chunk>left)chunk=left;}
  if(arp_until&&chunk>arp_until)chunk=arp_until;
  if(arp_gate_left&&chunk>arp_gate_left)chunk=arp_gate_left;
  if(!chunk)continue;
  app_audio_core(out,chunk);
  if(arp_until)arp_until-=chunk;
  if(arp_gate_left)arp_gate_left-=chunk;if(running)step_clock+=chunk;
  out+=chunk*2u;n-=chunk;
 }
}
#include "wanderer_edit.c"
#include "wanderer_settings.h"
static int32_t width(const font_t *f,const char *s)
{int32_t w=0;while(*s)w+=f->adv[letter(f,(uint8_t)*s++)];return w;}
/* LCD data is already byte-swapped. Whole opaque runs need no read or blend. */
static void alpha_span(int32_t x,int32_t y,uint32_t count,uint32_t alpha,uint16_t fg)
{
    int32_t end=x+(int32_t)count;
    if(x<0)x=0;
    if(end>(int32_t)cv_w)end=(int32_t)cv_w;
    if(x>=end)return;
    uint16_t *pixel=cv_px+(uint32_t)y*cv_w+(uint32_t)x;
    uint16_t *stop=pixel+(end-x);
    if(alpha==15u){
        uint16_t color=swap16(fg);
        while(pixel<stop)*pixel++=color;
        return;
    }
    uint32_t inv=15u-alpha;
    uint32_t fr=(fg>>11)*alpha+7u,fgreen=((fg>>5)&63u)*alpha+7u,fb=(fg&31u)*alpha+7u;
    while(pixel<stop){
        uint16_t old=swap16(*pixel);
        uint32_t r=((old>>11)*inv+fr)/15u;
        uint32_t g=(((old>>5)&63u)*inv+fgreen)/15u;
        uint32_t b=((old&31u)*inv+fb)/15u;
        *pixel++=swap16((r<<11)|(g<<5)|b);
    }
}
static void aa_text(int32_t x,int32_t y,const font_t *f,const char *s,uint16_t fg)
{
#ifdef DINGHY_HOST
    if(x-FONT_PAD<0||x+width(f,s)>238||y<0||y+(int32_t)f->h>240){
        fprintf(stderr,"Wanderer text outside screen: %s x=%d width=%d y=%d\n",s,x,width(f,s),y);
        exit(2);
    }
#endif
    /* Text outside the current LCD strip has no visible pixels. */
    if(y+cv_oy>= (int32_t)cv_h||y+cv_oy+(int32_t)f->h<=0)return;
    for(;*s;s++){
        uint32_t k=letter(f,(uint8_t)*s),gy,gx=0,run=0,alpha=0;
        const uint8_t *d=f->data+f->off[k];
        uint32_t w=f->bw[k];
        for(gy=0;gy<f->h;gy++)for(gx=0;gx<w;){
            uint32_t count;
            if(!run){uint32_t b=*d++;alpha=b>>4;run=(b&15u)+1u;if(run==16u)run+=*d++;}
            count=run<w-gx?run:w-gx;
            int32_t py=y+(int32_t)gy+cv_oy;
            if(alpha&&(uint32_t)py<cv_h)
                alpha_span(x-FONT_PAD+(int32_t)gx,py,count,alpha,fg);
            gx+=count;run-=count;
        }
        x+=f->adv[k];
    }
}
#include "wanderer_screen.c"
static void focus_show(uint8_t id,uint8_t v){if(notice_success())notice=0;focus=id;focus_value=v;focus_until=display_deadline();page_dirty=1;}
static void show_notice(uint8_t id){focus=0;notice=id;notice_until=notice_sticky()?0u:display_deadline();page_dirty=1;}
static void display_expire(void)
{
 if(focus&&focus_until&&!ui_time_before(focus_until)){focus=0;page_dirty=1;}
 if(notice&&notice_until&&!ui_time_before(notice_until)){notice=0;page_dirty=1;}
}
static void release_sequence(void)
{for(uint32_t v=0;v<NV;v++)if(!voice[v].live)voice[v].gate=0;}
static void reset_performance(void)
{
    for(uint32_t c=0;c<CAPTURE_COUNT;c++)capture[c].active=0;
    for(uint32_t v=0;v<NV;v++)voice[v].gate=voice[v].amp=0;
    for(uint32_t k=0;k<32u;k++)sustain_pedal[k]=0;
    arp_reset();click_left=0;
    cue_stop();wp_cancel();wp_repeat_on=0;wp_scatter_mix=0;scatter_clear();fx_reset();
}
static void panic(void)
{
    fm1_irq_off();count_in_total=0;capture_finish_all();record_undo_pending=0;running=recording=0;reset_performance();fm1_irq_on();
    performance_pending=0;play_tap=play_recall=anchor_pending=0;
    save_pending=0;save_down=0;confirm=focus=notice=0;wp_cancel();edit_pending=rec_pending=arp_pending=0;ui_pending&=~(1u<<BTN_HOME);page_dirty=1;
}
static uint32_t capture_step(void)
{
    if(!step_primed)return 0;
    return step_clock<step_samples/2u?(step_index+wm.loop_steps-1u)%wm.loop_steps:step_index;
}
typedef struct {
    uint32_t magic;uint16_t version,size;
    uint8_t light,zoom,still,level,cues,slot,reserved[2];
} wm_preferences_t;
static uint8_t preferences_valid(const wm_preferences_t *p)
{
    return p->magic==WM_MAGIC&&(p->version==1||p->version==2)&&p->size==sizeof *p&&
      p->light<(p->version==1?2u:THEME_COUNT)&&p->zoom<2u&&p->still<2u&&p->level<=127u&&p->cues<2u&&p->slot<NSLOT&&
      ((p->reserved[1]&128u)?(p->reserved[0]&0x30u)!=0x30u:(!p->reserved[0]&&!p->reserved[1]));
}
static void preferences_load(void)
{
    wm_preferences_t p;
    if(!settings_load(&p,sizeof p)&&preferences_valid(&p)){
        color_theme=p.version==1?(p.light?THEME_LIGHT:0u):p.light;
        wm.light=color_theme==THEME_LIGHT;wm.extra_large=p.zoom;wm.reduced_motion=p.still;
        vol=p.level;cues_enabled=p.cues;project_slot=p.slot;
        if(p.reserved[1]&128u){
            display_hold=p.reserved[0]&3u;cue_detail=(p.reserved[0]>>2)&1u;
            zoom_steady=(p.reserved[0]>>3)&1u;metronome_on=(uint8_t)(((p.reserved[0]>>4)&1u)|(((p.reserved[0]>>5)&1u)<<1));
            if(metronome_on>2u)metronome_on=2u;click_level=(p.reserved[0]>>6)&3u;cue_level=p.reserved[1]&127u;
        }else{display_hold=0;cue_detail=zoom_steady=1;cue_level=64;}
        master_smooth=(int32_t)vol*256;
    }
}
static int preferences_save(void)
{
    wm_preferences_t p={WM_MAGIC,2,sizeof p,color_theme,wm.extra_large,
        wm.reduced_motion,vol,cues_enabled,project_slot,
        {(uint8_t)(display_hold|(cue_detail<<2)|(zoom_steady<<3)|((metronome_on&1u)<<4)|((metronome_on>>1)<<5)|(click_level<<6)),(uint8_t)(128u|cue_level)}},verify;
    if(settings_save(&p,sizeof p)||settings_load(&verify,sizeof verify))return -1;
    return memcmp(&p,&verify,sizeof p)?-1:0;
}
static void transport_locked(void)
{
    capture_finish_all();record_undo_pending=0;running=!running;recording=0;step_index=step_clock=cycle_index=0;step_primed=0;
    if(running)scatter_seed((fm1_ms+1u)*747796405u^audio_frames^scatter.rng);else scatter_clear();
    if(!running){arp_reset();release_sequence();wp_cancel();wp_repeat_on=0;}
}
/* Audio callback: the count-in has finished, so play and record together. */
static void count_in_finish(void)
{
    count_in_total=0;
    transport_locked();recording=1;record_undo_pending=1;
    for(uint32_t lane=0;lane<2u;lane++)for(uint32_t i=0;i<WM_STEPS;i++)record_replace[lane][i]=0;
    page_dirty=1;
}
static void transport(void)
{
    fm1_irq_off();count_in_total=0;transport_locked();fm1_irq_on();
    cue_toggle(running);page_dirty=1;
}
static void tempo_set(uint8_t bpm)
{
    if(wp_timing_locked()){show_notice(N_AUDIO_LOCK);return;}
    if(bpm==wm.bpm)return;
    fm1_irq_off();
    capture_finish_all();
    uint32_t duration=(FS*15u)/bpm;
    step_clock=step_clock*duration/step_samples;
    step_samples=duration;wm.bpm=bpm;
    fm1_irq_on();
}
static void recording_end(void)
{capture_finish_all();recording=0;record_undo_pending=0;}
static void listening_set(uint8_t anchor)
{
    if(anchor_view==anchor)return;
    fm1_irq_off();recording_end();release_sequence();anchor_view=anchor;fm1_irq_on();
    if(!anchor&&notice==N_ANCHOR_LISTEN)notice=0;
}
static void anchor_preview(void)
{
    if(!(wm.has_anchor||wp_has_anchor)){
        /* A first comparison can protect a completed Memory take. Leave an
         * in-progress take alone and never take Layer 2 as the Anchor. */
        if(recording||!wm_memory_playable()){
            cue(48u);show_notice(N_CATCH_FIRST);return;
        }
        fm1_irq_off();wm_catch();fm1_irq_on();
    }
    focus=notice=0;
    listening_set((uint8_t)!anchor_view);
    cue(anchor_view?79u:67u);page_dirty=1;
}
static void recording_toggle(void)
{
    if(anchor_view){cue(48u);show_notice(N_ANCHOR_LISTEN);return;}
    if(notice==N_CATCH_FIRST)notice=0;
    if(wp_active){
        fm1_irq_off();uint8_t ok=wp_arm();fm1_irq_on();
        if(ok&&!running)transport();
        if(!ok)show_notice(N_NO_PHRASE);
        page_dirty=1;return;
    }
    if(count_in_total){fm1_irq_off();count_in_total=0;fm1_irq_on();cue(48u);page_dirty=1;return;}
    if(!recording&&!running&&metronome_on){
        /* From stop, count one bar in; recording begins on the downbeat. */
        fm1_irq_off();count_in_pos=0;count_in_total=step_samples*16u;fm1_irq_on();
        count_in_shown=0;page_dirty=1;return;
    }
    fm1_irq_off();
    if(recording)recording_end();
    else{
        if(!running){fm1_irq_on();transport();fm1_irq_off();}
        recording=1;record_undo_pending=1;
        for(uint32_t lane=0;lane<2u;lane++)for(uint32_t i=0;i<WM_STEPS;i++)record_replace[lane][i]=0;
    }
    fm1_irq_on();cue_toggle(recording);page_dirty=1;
}
/* Caller holds the audio lock. Both screens restore the same scheduler state. */
static uint8_t undo_events_locked(void)
{
    notice=0;
    if(!wm_can_undo)return 0;
    if(wp_timing_locked()&&(wm.bpm!=wm_undo.bpm||wm.loop_steps!=wm_undo.loop_steps)){
        show_notice(N_AUDIO_LOCK);return 0;
    }
    recording_end();
    uint8_t old_length=wm.loop_steps;
    uint32_t old_samples=step_samples;
    wm_undo_once();ww_reset();
    step_samples=(FS*15u)/wm.bpm;
    if(old_length!=wm.loop_steps){step_index=step_clock=cycle_index=0;step_primed=0;}
    else if(old_samples!=step_samples)step_clock=step_clock*step_samples/old_samples;
    if(!(wm.has_anchor||wp_has_anchor))anchor_view=0;
    editor_check_page();
    reset_performance();
    return 1;
}
static void apply_confirm(void)
{
    static wm_disk_t disk;
    static wm_disk_t verify;
    static wm_state_t loaded;
    if((confirm==2||confirm==3||confirm==5)&&(running||recording)){
        cue(45u);show_notice(N_STOP_FIRST);confirm=0;return;
    }
    if(confirm==1){
        fm1_irq_off();wm_mark();wm_clear_phrase(&wm.layer);for(uint32_t v=0;v<NV;v++)if(!voice[v].live&&voice[v].seq_lane==1u)voice[v].gate=0;fm1_irq_on();
        show_notice(N_LAYER_CLEARED);
    }else if(confirm==4){
        wm_state_t fresh=WM_NEW_DEFAULTS;
        fresh.light=wm.light;fresh.extra_large=wm.extra_large;fresh.reduced_motion=wm.reduced_motion;
        fm1_irq_off();wm_mark();wm=fresh;running=recording=anchor_view=layer_focus=0;
        step_clock=step_index=cycle_index=0;step_primed=0;
        step_samples=(FS*15u)/wm.bpm;
        reset_performance();wp_reset();ww_reset();project_checkpoint=wm;project_origin_slot=255u;
        fm1_irq_on();project_poll();cue(76u);show_notice(N_NEW_SESSION);
    }else if(confirm==5){
        if(wm.has_anchor||wp_has_anchor){
            fm1_irq_off();wm_mark();if(wm.has_anchor){wm.memory=wm.anchor;wm.sound=wm.anchor_sound;
                wm.legacy=wm.anchor_legacy;wm.preset_bank=wm.anchor_preset_bank;wm.patch_valid=wm.anchor_patch_valid;if(wm.patch_valid)wm.patch=wm.anchor_patch;}
            wm.has_branch=wm.branch_melt=wm.melt=wm.drift=wm.decay=0;
            anchor_view=0;reset_performance();ww_reset();if(wp_has_anchor)wp_restore();fm1_irq_on();
            cue(79u);show_notice(N_ORIGINAL_SAFE);
        }
    }else if(confirm==2){
        /* Flash erases pause audio: release tails before writing. */
        fm1_irq_off();reset_performance();fm1_irq_on();
        disk.magic=WM_MAGIC;disk.version=WM_DISK_VERSION;disk.size=(uint16_t)sizeof wm;
        disk.state=wm;
        if(slot_save(project_slot,&disk,sizeof disk)||slot_load(project_slot,&verify,sizeof verify)||
           memcmp(&disk,&verify,sizeof disk)){slot_presence[project_slot]=0;cue(45u);show_notice(N_SAVE_FAILED);}
        else {
            uint8_t prefs_ok=preferences_save()==0;
            fm1_irq_off();project_checkpoint=wm;project_origin_slot=project_slot;project_save_serial++;
            if(prefs_ok){clean_level=vol;clean_cues=cues_enabled;clean_access=access_signature();preferences_pending=0;}
            else preferences_pending=1;
            fm1_irq_on();slot_presence[project_slot]=2u;project_poll();
            cue(prefs_ok?79u:45u);show_notice(prefs_ok?N_SAVED:N_PREF_FAILED);
        }
    }else if(confirm==3){
        uint8_t valid=project_read_slot(project_slot,&loaded);
        slot_presence[project_slot]=valid?2u:1u;
        if(!valid){cue(45u);show_notice(N_NO_VALID_SAVE);}
        else{
            fm1_irq_off();wm_mark();wm=loaded;
            running=recording=anchor_view=record_undo_pending=layer_focus=0;
            step_index=step_clock=cycle_index=0;step_primed=0;
            step_samples=(FS*15u)/wm.bpm;
            reset_performance();wp_reset();ww_reset();project_checkpoint=wm;project_origin_slot=project_slot;
            fm1_irq_on();project_poll();cue(79u);show_notice(N_LOADED);
        }
    }
    confirm=0;page_dirty=1;
}
#include "wanderer_midi.c"
#include "wanderer_leds.c"
#ifdef DINGHY_HOST
static void host_verify_app(void);
#endif
/* Called by the Settings dispatcher with audio interrupts disabled. */
static void length_adjust_locked(int32_t d)
{
    if(wp_timing_locked()){show_notice(N_AUDIO_LOCK);return;}
    if(running||recording){show_notice(N_STOP_FIRST);return;}
    capture_finish_all();
    if(wm_set_length(d>0?32u:16u)){
        step_index=step_clock=cycle_index=0;step_primed=0;release_sequence();
    }
    page_dirty=1;
}
static void layer_target_locked(uint8_t target);
static void layer_playback_locked(uint8_t enabled)
{
 if(wm.layer_enabled==enabled)return;
 wm_mark();
 wm.layer_enabled=enabled;
 if(!enabled){
  for(uint32_t v=0;v<NV;v++)if(!voice[v].live&&voice[v].seq_lane==1u)voice[v].gate=0;
 }
 cue_toggle(enabled);page_dirty=1;
}
#include "wanderer_settings.c"
/* Release-dispatched HOME/SEL avoid either chord order executing a solo action.
 * A held HOME still panics from its original press time, without a release action. */
static uint32_t ui_edges(uint32_t b,uint32_t released)
{
    const uint32_t sel=1u<<BTN_SEL,pair=(1u<<BTN_HOME)|sel;
    uint32_t ready=0,held=fm1_in.buttons;
    const uint32_t zoom_pair=(1u<<BTN_HOME)|(1u<<BTN_EDIT);
    /* HOME and EDIT dispatch on release, so either press order is safe.
     * Do not reinterpret an already-held Scope chord or a three-button chord. */
    if(!ui_chord_latched&&!zoom_chord_latched&&!edit_layer_done&&
       (home_down||(b&(1u<<BTN_HOME)))&&(held&zoom_pair)==zoom_pair&&!(held&sel)){
        zoom_chord_latched=1;wm.extra_large=(uint8_t)!wm.extra_large;
        cue_toggle(wm.extra_large);page_dirty=1;
    }
    if(zoom_chord_latched){
        ui_pending&=~pair;home_down=edit_down=sel_down=0;
        edit_pending=0;
        if(!(held&(zoom_pair|(1u<<BTN_SEL))))zoom_chord_latched=0;
        return b&~(zoom_pair|(1u<<BTN_SEL));
    }
    if(b&(1u<<BTN_HOME))home_down=fm1_ms+1u;
    if(b&sel){sel_down=fm1_ms;sel_can_open=(uint8_t)(!confirm&&!scope_open);}
    ui_pending|=b&pair;b&=~pair;
    if((held&pair)==pair&&!ui_chord_latched){
        ui_chord_latched=1;ui_pending&=~pair;home_down=fx_down=0;anchor_pending=0;
        scope_set_open((uint8_t)!scope_open);focus=0;cue(78u);page_dirty=1;
    }
    if(ui_chord_latched){
        ui_pending&=~pair;home_down=0;
        if(!(held&pair))ui_chord_latched=0;
        return b;
    }
    ready=ui_pending&released&pair;ui_pending&=~ready;
    return b|ready;
}
/* Short SAVE retains Catch. Holding it offers the actual project Save dialog,
 * without catching an Anchor on the way. The original SEQ shortcut remains. */
static uint32_t save_edges(uint32_t b,uint32_t released)
{
    const uint32_t bit=1u<<BTN_SAVE;
    if(b&bit){
        b&=~bit;
        if(!scope_open&&!settings_open&&!confirm){save_down=fm1_ms+1u;save_pending=1;}
    }
    if(save_pending&&(scope_open||settings_open||confirm||b))save_pending=0,save_down=0;
    if(save_pending&&(fm1_in.buttons&bit)&&fm1_ms+1u-save_down>=900u){
        fm1_irq_off();recording_end();fm1_irq_on();
        confirm=2;focus=notice=0;save_pending=0;save_down=0;
        cue(57u);page_dirty=1;
    }
    if((released&bit)&&save_pending){save_pending=0;save_down=0;b|=bit;}
    return b;
}
/* Tap GLO keeps the wander; hold GLO for 0.9 s to Return Memory to its
 * recorded source. The Anchor and the recorded notes are never changed. */
static uint32_t glo_down;static uint8_t glo_pending;
static uint32_t glo_edges(uint32_t b,uint32_t released)
{
    const uint32_t bit=1u<<BTN_GLO;
    uint8_t home=(uint8_t)(!scope_open&&!settings_open&&!editor_open&&!confirm&&!wp_active);
    if(b&bit){
        if(home){b&=~bit;glo_down=fm1_ms+1u;glo_pending=1;}
    }
    if(glo_pending&&!home)glo_pending=0;
    if(glo_pending&&(fm1_in.buttons&bit)&&fm1_ms+1u-glo_down>=900u){
        glo_pending=0;
        fm1_irq_off();uint8_t had=ww_return();fm1_irq_on();
        cue_pair(76u,64u,2);show_notice(had?N_RETURNED:N_NOTHING_TO_KEEP);
    }
    if((released&bit)&&glo_pending){glo_pending=0;b|=bit;}
    return b;
}
static void layer_target_locked(uint8_t target)
{
 if(layer_focus==target)return;
 /* A held note keeps its original lane and duration. */
 layer_focus=target;editor_mark_ms=0;
 editor_check_page();
 cue_toggle(target);page_dirty=1;
}
static uint32_t detail_edges(uint32_t b,uint32_t released)
{
 const uint32_t edit=1u<<BTN_EDIT,rec=1u<<BTN_REC,sel=1u<<BTN_SEL,arp=1u<<BTN_ARP;
 if(b&arp){arp_down=fm1_ms+1u;arp_pending=1;b&=~arp;}
 if((fm1_in.buttons&arp)&&arp_pending&&fm1_ms+1u-arp_down>=900u){
  arp_pending=0;scope_set_open(0);fm1_irq_off();recording_end();fm1_irq_on();
  editor_open=confirm=focus=notice=0;settings_open=1;settings_page=6;settings_index=0;page_dirty=1;
 }
 if(released&arp){if(arp_pending)b|=arp;arp_pending=0;}
 uint8_t edit_available=(uint8_t)(!scope_open&&!settings_open&&!confirm&&(!notice_error()||notice==N_CATCH_FIRST));
 if(!edit_available)edit_pending=0;
 if(b&edit){edit_down=fm1_ms;edit_pending=edit_available;b&=~edit;}
 if(b&rec){rec_down=fm1_ms+1u;rec_pending=1;b&=~rec;}
 if(((fm1_in.buttons|released)&edit)&&edit_pending&&fm1_ms-edit_down>=900u){
  edit_pending=0;edit_layer_done=1;
  fm1_irq_off();layer_target_locked((uint8_t)!layer_focus);fm1_irq_on();
  focus=notice=0;
 }
 if((fm1_in.buttons&rec)&&rec_pending&&fm1_ms+1u-rec_down>=900u){
  rec_pending=0;scope_set_open(0);fm1_irq_off();recording_end();fm1_irq_on();editor_open=1;editor_page=EDIT_PRINT;settings_open=confirm=focus=notice=0;page_dirty=1;
 }
 if(released&edit){if(edit_pending)b|=edit;edit_pending=edit_layer_done=0;}
 if(released&rec){if(rec_pending)b|=rec;rec_pending=0;}
 /* A long SELECT consumes the release even in a confirmation or Scope. */
 if(((ui_pending|b)&sel)&&fm1_ms-sel_down>=900u){
  ui_pending&=~sel;b&=~sel;
  if(sel_can_open&&!confirm&&!scope_open){
   editor_open=0;settings_open=1;settings_page=settings_index=0;focus=notice=0;page_dirty=1;
  }
 }
 if(released&sel)sel_down=0;
 if(ui_chord_latched){sel_down=0;edit_pending=rec_pending=0;}
 if(zoom_chord_latched){sel_down=0;edit_pending=0;}

 const uint32_t anchor=1u<<BTN_LFO;
 uint8_t anchor_available=(uint8_t)(!scope_open&&!settings_open&&!confirm);
 if(!anchor_available)anchor_pending=0;
 if(b&anchor){b&=~anchor;anchor_down=fm1_ms;anchor_pending=anchor_available;}
 if(anchor_pending&&((fm1_in.buttons|released)&anchor)&&fm1_ms-anchor_down>=1200u){
  anchor_pending=0;
  if(!(wm.has_anchor||wp_has_anchor))show_notice(N_CATCH_FIRST);
  else if(running||recording){cue(45u);show_notice(N_STOP_FIRST);}
  else {confirm=5;focus=notice=0;cue(55u);page_dirty=1;}
 }
 if(released&anchor){if(anchor_pending)b|=anchor;anchor_pending=0;}
 return b;
}
/* Long presses consume their tap. A running PLAY/STOP still stops on
 * contact. From idle, play starts on release so a hold never starts the loop. */
static uint32_t performance_edges(uint32_t b,uint32_t released)
{
 const uint32_t oct=1u<<BTN_OCT_UP,down=1u<<BTN_OCT_DOWN,play=1u<<BTN_PLAY;
 uint32_t held=fm1_in.buttons;
 uint8_t on_home=(uint8_t)(!scope_open&&!settings_open&&!editor_open&&!confirm&&!notice_error());
 uint32_t oct_edges=held|released|b;
 if(b&oct){octave_up_down=fm1_ms;performance_pending|=oct;b&=~oct;}
 if(b&down){octave_down_down=fm1_ms;performance_pending|=down;b&=~down;}
 if((oct_edges&(oct|down))==(oct|down)||zoom_chord_latched)performance_pending&=~(oct|down);
 if(released&down){b|=performance_pending&down;performance_pending&=~down;}
 if((performance_pending&oct)&&fm1_ms-octave_up_down>=1500u){
  performance_pending&=~oct;wm.extra_large=!wm.extra_large;
  cue_toggle(wm.extra_large);page_dirty=1;
 }
 /* Hold OCT- for 1.5 s: the next colour theme, for changing room light. */
 if((performance_pending&down)&&fm1_ms-octave_down_down>=1500u){
  performance_pending&=~down;
  color_theme=(uint8_t)((color_theme+1u)%THEME_COUNT);wm.light=color_theme==THEME_LIGHT;
  cue(64u+color_theme*3u);show_notice(N_THEME);
 }
 if(released&oct){b|=performance_pending&oct;performance_pending&=~oct;}
 if(!on_home)play_recall=0;
 if(b&play){
  play_down=fm1_ms;performance_pending|=play;play_tap=(uint8_t)!running;
  play_recall=on_home;
  if(play_tap)b&=~play;
 }
 if((performance_pending&play)&&fm1_ms-play_down>=2000u){
  performance_pending&=~play;play_tap=0;
  if(running)transport();
  if(play_recall){
   if(anchor_view&&!layer_focus)listening_set(0);
   notice=0;
   fm1_irq_off();wm_mark();ws_select(0);fm1_irq_on();
   focus_show(7,1);cue(76u);
  }
 }
 if(released&play){if(play_tap)b|=play;play_tap=play_recall=0;performance_pending&=~play;}
 return b;
}
static void local_notes(uint32_t keys,uint32_t down,uint32_t up,uint32_t base)
{
    if(!(down|up))return;
    uint32_t k;
    /* All local onsets detected in this pass share one quantization decision.
     * Keep audio enabled between individual keys, including in four-note chords. */
    fm1_irq_off();uint32_t local_step=capture_step();fm1_irq_on();
    for(k=0;k<27u;k++){
        if(down>>k&1u){
            held[k]=(uint8_t)(wm.keyboard_scale?wm_scale_move((uint8_t)(base-base%12u+wm.root),(int32_t)k):base+k);
            fm1_irq_off();performance_on(held[k],100u,1);
            /* A latched tap that ended during a redraw still plays, then releases. */
            if(!(keys>>k&1u))for(uint32_t v=0;v<NV;v++)
                if(voice[v].live==1u&&voice[v].note==held[k])voice[v].remain=FS/32u;
            if(!wm.arp_on&&recording&&!anchor_view&&!confirm&&!settings_open&&!wp_active){
                capture_on_at(held[k],100u,1u,local_step);
                if(!(keys>>k&1u))capture_off(held[k],1u,0);
            }
            fm1_irq_on();page_dirty=1;
        }
        if(up>>k&1u){fm1_irq_off();capture_off(held[k],1u,0);performance_off(held[k],1u,0);fm1_irq_on();}
    }
}
/* Called during LCD DMA and between render bands. Only performance input is
 * serviced here; menu/button commands remain in the outer control pass. */
static void app_idle(void)
{
    app_midi();
    uint32_t keys=fm1_in.notes,down=(keys&~prev_notes)|fm1_input_note_edges(),up=prev_notes&~keys;
    prev_notes=keys;
    if(down||up)splash_dismiss();
    local_notes(keys,down,up,(uint32_t)((int32_t)KEY0+(int8_t)octave*12));
}
/* The first loop that wanders protects the source as the Anchor (without
 * touching Undo), so there is always a way home. Redraw after each loop. */
static uint8_t wander_seen_loops;
static void wander_service(void)
{
    if(count_in_total){
        uint8_t beat=(uint8_t)(1u+count_in_pos/(step_samples*4u));
        if(beat!=count_in_shown){count_in_shown=beat;page_dirty=1;}
    }else if(count_in_shown){count_in_shown=0;page_dirty=1;}
    if(ww.want_anchor){
        ww.want_anchor=0;
        if(!wm.has_anchor&&!recording&&!wp_has_anchor){
            fm1_irq_off();uint8_t ok=wm_protect_source();fm1_irq_on();
            if(ok)cue_pair(67u,79u,1);
        }
    }
    if(ww.moved||(uint8_t)ww.loops!=wander_seen_loops){
        if(ww.moved&&ww.distance)led_tide_until=fm1_ms+160u;
        ww.moved=0;wander_seen_loops=(uint8_t)ww.loops;page_dirty=1;
    }
}
static uint32_t screen_refresh_interval(void)
{
    if(scope_open)return scope_hold?0u:wm.reduced_motion?250u:80u;
    if(wm.reduced_motion)return 0;
    if(running&&!settings_open&&!editor_open&&!confirm&&!notice&&!focus&&!splash_active)
        return 250u;
    return 0;
}
static void app_main(void)
{
    uint32_t last_ui=0;
    /* Slot 1 is only read after explicit LOAD; never overwrite another firmware's data on boot. */
    lcd_fill(0,0,240,240,DARK);
    preferences_load();project_initialise();
    splash_active=1;splash_started=fm1_ms;page_dirty=1;
    for(;;){
        uint32_t b,rel,keys,down,up,k,base,transition;
        int32_t d;
        uint8_t prior_recording=recording;
        platform_poll();splash_tick();
        app_midi();
        b=fm1_input_edges(&rel);
        if(b)splash_dismiss();
        b=ui_edges(b,rel);b=save_edges(b,rel);b=glo_edges(b,rel);b=detail_edges(b,rel);b=performance_edges(b,rel);
        if(!scope_open&&notice&&notice_sticky()&&(b&((1u<<BTN_HOME)|(1u<<BTN_SEL)|(1u<<BTN_FX)))){
            notice=0;b&=~((1u<<BTN_HOME)|(1u<<BTN_SEL)|(1u<<BTN_FX));page_dirty=1;
        }
        if(notice_success()&&b){notice=0;page_dirty=1;}
        transition=b&((1u<<BTN_HOME)|(1u<<BTN_REC)|(1u<<BTN_SAVE)|(1u<<BTN_EDIT)|
          (1u<<BTN_LFO)|(1u<<BTN_GLO)|(1u<<BTN_ENV)|(1u<<BTN_FX)|
          (1u<<BTN_SEQ)|(1u<<BTN_ARP)|(1u<<BTN_SEL));
        keys=fm1_in.notes;down=(keys&~prev_notes)|fm1_input_note_edges();up=prev_notes&~keys;prev_notes=keys;
        if(down||up)splash_dismiss();
        if(down&&notice_success()){notice=0;page_dirty=1;}
        if(scope_open){
            if(b&(1u<<BTN_HOME)){scope_set_open(0);focus=0;page_dirty=1;}
            else if(b&(1u<<BTN_SEL)){scope_set_hold((uint8_t)!scope_hold);cue_toggle(scope_hold);page_dirty=1;}
            if(b&(1u<<BTN_PLAY))transport();
            if((b&(1u<<BTN_REC))&&!settings_open&&!confirm){
                recording_toggle();
                page_dirty=1;
            }
        }else if(b&(1u<<BTN_HOME)){
            if(confirm)confirm=0;
            else if(editor_open)editor_open=0;
            else if(settings_open)settings_open=0;
            else {confirm=0;focus=notice=0;listening_set(0);}
            cue(72u);page_dirty=1;
        }
        else if(confirm){
            if(b&(1u<<BTN_PLAY))transport();
            if(b&(1u<<BTN_SEL))apply_confirm();
            else if(b&(1u<<BTN_FX)){confirm=0;page_dirty=1;}
        }
        else if(editor_open){
            if(b&(1u<<BTN_PLAY))transport();
            if(b&(1u<<BTN_EDIT)){editor_open=0;page_dirty=1;}
            if(b&(1u<<BTN_SEL)){editor_open=0;settings_open=1;settings_page=SETTINGS_MUSIC;settings_index=0;page_dirty=1;}
            if(b&(1u<<BTN_REC)){
                if(editor_page==EDIT_PRINT){fm1_irq_off();recording_end();uint8_t ok=wp_arm();fm1_irq_on();
                    if(ok&&!running)transport();if(!ok)show_notice(N_NO_PHRASE);page_dirty=1;
                }else recording_toggle();
            }
            if(b&(1u<<BTN_ENV)){
                uint8_t print=editor_page==EDIT_PRINT;
                fm1_irq_off();uint8_t ok=print?wp_undo():undo_events_locked();fm1_irq_on();
                cue(ok?71u:48u);page_dirty=1;
                if(!print&&notice!=N_AUDIO_LOCK)show_notice(ok?N_UNDO:N_NO_UNDO);
            }
            if(b&(1u<<BTN_LFO))anchor_preview();
        }
        else if(settings_open){
            if(b&(1u<<BTN_PLAY))transport();
            if(b&(1u<<BTN_SEL)){fm1_irq_off();settings_press();fm1_irq_on();page_dirty=1;}
        }
        else{
            if(b&(1u<<BTN_PLAY))transport();
            if(b&(1u<<BTN_REC))recording_toggle();
            if(b&(1u<<BTN_SAVE)){fm1_irq_off();capture_finish_all();uint8_t ok=wm_catch();fm1_irq_on();cue(ok?84u:48u);show_notice(ok?N_ANCHORED:wm.has_anchor?N_ANCHOR_SAFE:N_CATCH_FIRST);}
            if(b&(1u<<BTN_EDIT)){editor_open=1;editor_page=editor_selected=0;focus=notice=0;page_dirty=1;}
            if(b&(1u<<BTN_LFO))anchor_preview();
            if(b&(1u<<BTN_GLO)){
                /* Tap GLO: Keep where Memory has wandered (or arm Print in RAM view). */
                fm1_irq_off();capture_finish_all();
                uint8_t ok=wp_active?wp_arm():wm.legacy?wm_branch_now(cycle_index):(wm.has_anchor||ww.distance)?ww_keep():0;
                fm1_irq_on();cue(ok?88u:48u);
                if(!wp_active)show_notice(ok?N_BRANCHED:wm.has_anchor?N_NOTHING_TO_KEEP:N_CATCH_FIRST);else page_dirty=1;
            }
            if(b&(1u<<BTN_ENV)){
                fm1_irq_off();
                uint8_t ok;
                if(wp_armed||wp_writing||wp_active){
                    notice=0;capture_finish_all();ok=wp_undo();if(ok)reset_performance();
                }
                else ok=undo_events_locked();
                fm1_irq_on();
                cue(ok?71u:48u);if(notice!=N_AUDIO_LOCK)show_notice(ok?N_UNDO:N_NO_UNDO);
            }
            if(b&(1u<<BTN_FX)){confirm=1;fx_down=fm1_ms;page_dirty=1;cue(55u);}
            if(b&(1u<<BTN_SEQ)){confirm=2;page_dirty=1;cue(57u);}
            if(b&(1u<<BTN_ARP)){confirm=3;page_dirty=1;cue(59u);}
            if(b&(1u<<BTN_SEL)){settings_open=1;recording=0;settings_index=0;
                settings_page=SETTINGS_MUSIC;focus=notice=0;cue(78u);page_dirty=1;}
        }
        if(prior_recording&&(!recording||anchor_view||confirm||settings_open)){
            fm1_irq_off();recording_end();fm1_irq_on();
        }
        if(home_down&&(fm1_in.buttons&(1u<<BTN_HOME))&&fm1_ms+1u-home_down>=1000u){panic();home_down=0;}
        if(!(fm1_in.buttons&(1u<<BTN_HOME)))home_down=0;
        if(fx_down&&!scope_open&&confirm==1&&(fm1_in.buttons&(1u<<BTN_FX))&&fm1_ms-fx_down>=1600u){
            confirm=4;fx_down=0;cue(47u);page_dirty=1;
        }
        if(!(fm1_in.buttons&(1u<<BTN_FX)))fx_down=0;
        if(b&(1u<<BTN_OCT_DOWN)){if((int8_t)octave> -2)octave=(uint8_t)((int8_t)octave-1);page_dirty=1;}
        if(b&(1u<<BTN_OCT_UP)){if((int8_t)octave<2)octave=(uint8_t)((int8_t)octave+1);page_dirty=1;}
        base=(uint32_t)((int32_t)KEY0+(int8_t)octave*12);
        local_notes(keys,down,up,base);
        if(transition)for(k=0;k<7u;k++)(void)fm1_enc_take(k);
        for(k=0;k<3u;k++){
            d=fm1_enc_take(ENC_KNOB1+k);
        if(d)splash_dismiss();
            if(d&&scope_open){
                if(k==0u)scope_gain=(uint8_t)ws_clamp((int32_t)scope_gain+d,0,4);
                else if(k==1u&&!scope_hold)scope_time=(uint8_t)ws_clamp((int32_t)scope_time+d,0,5);
                else if(k==2u)scope_set_hold((uint8_t)(d>0));
                page_dirty=1;
            }else if(d&&!scope_open&&editor_open&&!confirm){if(anchor_view&&!layer_focus)listening_set(0);fm1_irq_off();editor_adjust(k,d);fm1_irq_on();}
            else if(d&&!scope_open&&settings_open&&!confirm){fm1_irq_off();settings_adjust(k,d);fm1_irq_on();
            }else if(d&&!confirm&&!settings_open){
                uint8_t *p=k==0?&wm.melt:k==1?&wm.drift:&wm.decay;
                uint32_t old=*p;int32_t n=(int32_t)*p+d*4;
                *p=(uint8_t)(n<0?0:n>127?127:n);
                focus_show((uint8_t)(k+1u),*p);
                cue_boundary(old,*p,0,127);
            }
        }
        d=fm1_enc_take(ENC_KNOB4);
        if(d)splash_dismiss();
        if(d&&scope_open){
            uint32_t old=vol;int32_t n=(int32_t)vol+d*4;vol=(uint8_t)(n<0?0:n>127?127:n);cue_boundary(old,vol,0,127);page_dirty=1;
        }else if(d&&editor_open&&!confirm){if(anchor_view&&!layer_focus)listening_set(0);fm1_irq_off();editor_adjust(3u,d);fm1_irq_on();
        }else if(d&&settings_open&&!confirm){fm1_irq_off();settings_adjust(3u,d);fm1_irq_on();page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            uint32_t old=vol;int32_t n=(int32_t)vol+d*4;vol=(uint8_t)(n<0?0:n>127?127:n);cue_boundary(old,vol,0,127);
            focus_show(5u,vol);
        }
        d=fm1_enc_take(ENC_ALGORITHM);
        if(d)splash_dismiss();
        if(d&&scope_open){
            /* Parameter editing belongs to the four numbered knobs. */
        }else if(d&&!confirm&&!settings_open&&!editor_open&&wp_timing_locked()){show_notice(N_AUDIO_LOCK);
        }else if(d&&!confirm&&!settings_open&&!editor_open){
            uint32_t old=wm.bpm;int32_t n=(int32_t)wm.bpm+d;
            tempo_set((uint8_t)ws_clamp(n,60,180));
            cue_boundary(old,wm.bpm,60,180);
            focus_show(4u,wm.bpm);
        }
        d=fm1_enc_take(ENC_SELECT);
        if(d)splash_dismiss();
        if(d&&scope_open){
            /* Scope has no hidden parameter controls on SELECT. */
        }else if(d&&editor_open&&!confirm){editor_selected=(uint8_t)(((int32_t)editor_selected+d%(int32_t)editor_count()+(int32_t)editor_count())%(int32_t)editor_count());page_dirty=1;
        }else if(d&&(confirm==2u||confirm==3u)){
            project_slot=(uint8_t)(((int32_t)project_slot+d%8+8)%8);page_dirty=1;
            cue(60u+project_slot*2u);
        }else if(d&&settings_open){
            settings_index=(uint8_t)(((int32_t)settings_index+d%(int32_t)settings_count()+(int32_t)settings_count())%(int32_t)settings_count());
            cue(66u+settings_index*3u);page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            /* Bank jump preserves position within each group of ten. Zoom lives in settings. */
            fm1_irq_off();ws_select(wm_preset_move(ws_edit_sound(),d,1u));fm1_irq_on();
            focus_show(7u,ws_edit_sound()+1u);cue(68u+ws_edit_sound()/10u);
        }
        d=fm1_enc_take(ENC_PRESETS);
        if(d)splash_dismiss();
        if(d&&scope_open){
            /* Scope is a monitor. Preset changes belong to Home. */
        }else if(d&&editor_open&&!confirm){editor_move(d);
        }else if(d&&settings_open){
            settings_page=settings_move(d);
            settings_index=0;cue(77u);page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            fm1_irq_off();ws_select(wm_preset_move(ws_edit_sound(),d,0u));fm1_irq_on();
            focus_show(7u,ws_edit_sound()+1u);
            cue(60u+ws_edit_sound()/10u*2u);
        }
#ifdef DINGHY_HOST
        if(step_samples!=(FS*15u)/wm.bpm){fprintf(stderr,"Wanderer tempo and scheduler diverged\n");exit(3);}
#endif
        wander_service();
        display_expire();
        if(page_dirty||fm1_ms-last_project_poll>=250u)project_poll();
        app_leds();
#ifdef DINGHY_HOST
        host_verify_app();
#endif
        uint32_t refresh=screen_refresh_interval();
        if(page_dirty||(refresh&&fm1_ms-last_ui>=refresh)){
            page_dirty=0;screen();last_ui=fm1_ms;
        }
    }
}
#ifdef DINGHY_HOST
#include "../../tests/wanderer_observe.h"
#endif
