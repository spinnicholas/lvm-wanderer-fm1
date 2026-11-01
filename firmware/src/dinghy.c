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

/* Atkinson Hyperlegible, SIL OFL. Offline generated 4-bit run-length glyphs. */
typedef struct {uint8_t h,n;const char *chars;const uint8_t *adv,*bw;const uint32_t *off;const uint8_t *data;} font_t;
#include "wanderer_font.h"
#include "wanderer_icons.h"
#include "wanderer_waves.h"
#include "wanderer_presets.h"
#include "wanderer_filter.h"
#define NV 4u
#define KEY0 53u
#define DARK RGB(7,19,24)
#define PALE RGB(231,245,242)
#define INK RGB(13,29,35)
#define CYAN RGB(105,232,218)
#define BLUE RGB(166,175,250)
#define GOLD RGB(255,202,113)
enum {
    N_NONE, N_ORIGINAL_SAFE, N_ANCHORED, N_ANCHOR_SAFE, N_NO_PHRASE,
    N_BRANCHED, N_CATCH_FIRST, N_UNDO, N_NO_UNDO, N_STILL, N_LIVE,
    N_LAYER_CLEARED, N_NEW_SESSION, N_SAVE_FAILED, N_SAVED,
    N_NO_VALID_SAVE, N_LOADED, N_STOP_FIRST, N_PREF_FAILED
};
static struct {
    volatile uint32_t inc;volatile int32_t gate;
    uint32_t ph,ph2,ph3,lfo,inc2,age,remain;
    int32_t amp,f1,f2,tone,brightness,fm_brightness,last,splice;
    uint8_t note,live,patch,band1,band2,stage,sustained,splice_left,seq_lane;
} voice[NV];
/* One short mono feedback line, read at two unequal times for a quiet stereo tail.
 * 8192 x 16-bit samples = 16 KiB; no allocation or division in the audio ISR. */
#define AMBIENCE_SIZE 8192u
static int16_t ambience[AMBIENCE_SIZE] __attribute__((section(".pool")));
static uint32_t ambience_write;
static uint32_t motion_phase;
static uint32_t ages,step_clock,step_index,cycle_index,step_samples=5906u;
static uint8_t step_primed;
static uint32_t audio_frames;
static uint32_t capture_step(void);
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
    uint32_t lane=wm.layer_enabled?1u:0u,slot=WM_POLY;
    wm_phrase_t *p=lane?&wm.layer:&wm.memory;
    if(record_undo_pending){wm_mark();record_undo_pending=0;}
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
        if(capture[c].active&&capture[c].owner==owner&&capture[c].note==note)capture_finish(c);
    }
    for(uint32_t c=lane*WM_POLY;c<(lane+1u)*WM_POLY;c++){
        if(!capture[c].active){pick=c;break;}
        if(capture_age(c)>=oldest_age){oldest_age=capture_age(c);pick=c;}
    }
    capture_finish(pick);
    *wm_at(p,at,slot)=(wm_event_t){(uint8_t)note,(uint8_t)velocity,32u};
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
static volatile uint8_t running,recording,anchor_view,vol=100;
static uint8_t octave,held[27],page_dirty=1,confirm,focus,focus_value;
static uint8_t settings_open,settings_index,settings_page,settings_detail;
static uint8_t settings_detail_kind;
static uint32_t ui_pending;
static uint8_t ui_chord_latched;
static uint32_t focus_until;
static uint8_t notice;
static uint32_t notice_until;
static uint32_t fx_down;
static uint32_t home_down,anchor_down,save_down;
static uint8_t save_pending;
static uint8_t project_slot,cues_enabled=1;
static uint8_t sustain_pedal[32];
static int32_t master_smooth=25600;
static uint32_t prev_notes;
/* A slot presence cache is read once at boot, then updated after explicit I/O.
 * Browsing a slot never reads flash while music is running. */
static uint8_t slot_presence[NSLOT]; /* 0 unknown/write problem, 1 no valid project, 2 valid */
static uint8_t project_origin_slot=255u,project_dirty,preferences_pending;
static uint8_t clean_level=100u,clean_cues=1u;
static wm_state_t project_checkpoint=WM_DEFAULTS,checkpoint_undo=WM_DEFAULTS;
static uint8_t origin_undo=255u,level_undo=100u,cues_undo=1u,prefs_undo;
static uint32_t project_save_serial,undo_save_serial,last_project_poll;
static union {wm_disk_t current;wm_disk_v4_t v4;wm_disk_v3_t v3;wm_disk_legacy_t old;} project_read_buffer;
static uint8_t project_read_slot(uint32_t slot,wm_state_t *out)
{
    if(slot>=NSLOT)return 0;
    if(!slot_load(slot,&project_read_buffer.current,sizeof project_read_buffer.current)&&wm_valid(&project_read_buffer.current)){
        if(out)*out=project_read_buffer.current.state;
        return 1;
    }
    if(!slot_load(slot,&project_read_buffer.v4,sizeof project_read_buffer.v4)&&wm_valid_v4(&project_read_buffer.v4)){
        if(out)*out=wm_migrate_v4(&project_read_buffer.v4);
        return 1;
    }
    if(!slot_load(slot,&project_read_buffer.v3,sizeof project_read_buffer.v3)&&wm_valid_v3(&project_read_buffer.v3)){
        if(out)*out=wm_migrate_v3(&project_read_buffer.v3);
        return 1;
    }
    if(!slot_load(slot,&project_read_buffer.old,sizeof project_read_buffer.old)&&wm_valid_legacy(&project_read_buffer.old)){
        if(out)*out=wm_migrate_legacy(&project_read_buffer.old);
        return 1;
    }
    return 0;
}
static void project_mark(void)
{
    checkpoint_undo=project_checkpoint;origin_undo=project_origin_slot;
    level_undo=clean_level;cues_undo=clean_cues;prefs_undo=preferences_pending;
    undo_save_serial=project_save_serial;
}
static void project_undo(void)
{
    /* Saving commits a new reference. Undo after Save edits that reference,
     * whereas Undo Load/New Session restores the previous project's identity. */
    if(undo_save_serial==project_save_serial){
        project_checkpoint=checkpoint_undo;project_origin_slot=origin_undo;
        clean_level=level_undo;clean_cues=cues_undo;preferences_pending=prefs_undo;
    }
}
static void project_poll(void)
{
    fm1_irq_off();
    uint8_t dirty=(uint8_t)(memcmp(&wm,&project_checkpoint,sizeof wm)!=0||
        vol!=clean_level||cues_enabled!=clean_cues||preferences_pending);
    fm1_irq_on();
    if(dirty!=project_dirty){project_dirty=dirty;page_dirty=1;}
    last_project_poll=fm1_ms;
}
static void project_initialise(void)
{
    project_checkpoint=wm;project_origin_slot=255u;project_dirty=preferences_pending=0;
    clean_level=vol;clean_cues=cues_enabled;project_save_serial=0;
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
    if(!cues_enabled)return;
    cue_phase=0;cue_step=tone_step(pitch);cue_left=FS/24u;
}
/* Owner 0: sequencer; 1: local keys; 2..33: USB/TRS source and channel. */
static uint32_t note_on(uint32_t note,uint32_t vel,uint32_t owner)
{
    uint32_t v,pick=0,best=0xffffffffu;
    const wm_preset_t *p=&WM_PRESETS[wm.sound];
    if(note>127u||!vel||vel>127u||owner>33u)return NV;
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
    voice[pick].patch=wm.sound;
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
    return pick;
}
static void note_off(uint32_t note,uint32_t owner)
{
    uint32_t v;
    for(v=0;v<NV;v++)if(voice[v].live==owner&&voice[v].note==note){
        if(owner>=2u&&sustain_pedal[owner-2u])voice[v].sustained=1;
        else voice[v].gate=0;
    }
}
static void sequence_event(wm_event_t e,uint32_t lane)
{
    if(!e.velocity)return;
    for(uint32_t v=0;v<NV;v++)if(!voice[v].live&&voice[v].seq_lane==lane&&voice[v].note==e.note)voice[v].gate=0;
    uint32_t v=note_on(e.note,e.velocity,0);
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
            if(!lane&&!anchor_view)e=wm_evolve(e,i,cycle_index);
            sequence_event(e,lane);
        }
    }
    step_index=(i+1u)%wm.loop_steps;
    if(!step_index)cycle_index++;
}
#include "wanderer_fx.c"
#include "wanderer_scope.c"
static void app_audio(int32_t *out,uint32_t n)
{
    audio_frames+=n;for(uint32_t c=0;c<CAPTURE_COUNT;c++)capture_update(c);
    uint32_t v,i;int32_t melt=anchor_view?0:(int32_t)wm.branch_melt+wm.melt;
    if(melt>127)melt=127;
    if(running){
        if(!step_primed){step_play();step_primed=1;}
        step_clock+=n;
        if(step_clock>=step_samples){step_clock-=step_samples;step_play();}
    }
    for(v=0;v<NV;v++){
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
        if(!amp&&!g)continue;
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
    fx_process(out,n);
    for(i=0;i<n;i++){
        int32_t left=out[i*2u],right=out[i*2u+1u];
        int32_t target=(int32_t)vol*256,diff=target-master_smooth;
        if(diff> -64&&diff<64)master_smooth=target;
        else master_smooth+=diff>>6;
        if(scope_open&&!scope_hold){
            int32_t probe_left=fx_limit((left*master_smooth)/25600);
            int32_t probe_right=fx_limit((right*master_smooth)/25600);
            scope_feed((probe_left+probe_right)/2);
        }
        if(cue_left){
            int32_t cue_sample=sine(cue_phase)>>6;
            if(cue_left<256u)cue_sample=(cue_sample*(int32_t)cue_left)>>8;
            cue_phase+=cue_step;cue_left--;
            left+=cue_sample;right+=cue_sample;
        }
        out[i*2u]=fx_limit((left*master_smooth)/25600);
        out[i*2u+1u]=fx_limit((right*master_smooth)/25600);
    }
}
static void app_idle(void){} /* audio and input stay independent of LCD DMA */

/* Glyphs are blended into the band, with measured advances and no scale-up jaggies. */
static uint32_t letter(const font_t *f,uint8_t c)
{
    uint32_t i;
    if(c>='a'&&c<='z')c-=32;
    for(i=0;i<f->n;i++)if((uint8_t)f->chars[i]==c)return i;
    for(i=0;i<f->n;i++)if(f->chars[i]=='?')return i;
    return 0;
}
static int32_t width(const font_t *f,const char *s)
{int32_t w=0;while(*s)w+=f->adv[letter(f,(uint8_t)*s++)];return w;}
static void aa_text(int32_t x,int32_t y,const font_t *f,const char *s,uint16_t fg)
{
#ifdef DINGHY_HOST
    if(x-FONT_PAD<0||x+width(f,s)>238||y<0||y+(int32_t)f->h>240){
        fprintf(stderr,"Wanderer text outside screen: %s x=%d width=%d y=%d\n",s,x,width(f,s),y);
        exit(2);
    }
#endif
    for(;*s;s++){
        uint32_t k=letter(f,(uint8_t)*s),gy,gx=0,run=0,alpha=0;
        const uint8_t *d=f->data+f->off[k];
        uint32_t w=f->bw[k];
        for(gy=0;gy<f->h;gy++)for(gx=0;gx<w;){
            uint32_t count;
            if(!run){uint32_t b=*d++;alpha=b>>4;run=(b&15u)+1u;if(run==16u)run+=*d++;}
            count=run<w-gx?run:w-gx;
            if(alpha)for(uint32_t t=0;t<count;t++){
                int32_t px=x-FONT_PAD+(int32_t)gx+(int32_t)t,py=y+(int32_t)gy+cv_oy;
                if((uint32_t)px<cv_w&&(uint32_t)py<cv_h){
                    uint16_t old=swap16(cv_px[(uint32_t)py*cv_w+(uint32_t)px]);
                    uint32_t r=(((old>>11)* (15u-alpha)+(fg>>11)*alpha)+7u)/15u;
                    uint32_t g=((((old>>5)&63u)*(15u-alpha)+((fg>>5)&63u)*alpha)+7u)/15u;
                    uint32_t b=(((old&31u)*(15u-alpha)+(fg&31u)*alpha)+7u)/15u;
                    cv_px[(uint32_t)py*cv_w+(uint32_t)px]=swap16((r<<11)|(g<<5)|b);
                }
            }
            gx+=count;run-=count;
        }
        x+=f->adv[k];
    }
}
#include "wanderer_screen.c"
static void focus_show(uint8_t id,uint8_t v){if(notice_success())notice=0;focus=id;focus_value=v;focus_until=fm1_ms+1200u;page_dirty=1;}
static void show_notice(uint8_t id){focus=0;notice=id;notice_until=notice_sticky()?0u:fm1_ms+1800u;page_dirty=1;}
static void release_sequence(void)
{for(uint32_t v=0;v<NV;v++)if(!voice[v].live)voice[v].gate=0;}
static void reset_performance(void)
{
    for(uint32_t c=0;c<CAPTURE_COUNT;c++)capture[c].active=0;
    for(uint32_t v=0;v<NV;v++)voice[v].gate=voice[v].amp=0;
    for(uint32_t k=0;k<32u;k++)sustain_pedal[k]=0;
    cue_left=0;fx_reset();
}
static void panic(void)
{
    fm1_irq_off();capture_finish_all();record_undo_pending=0;running=recording=0;reset_performance();fm1_irq_on();
    save_pending=0;save_down=0;confirm=focus=notice=0;ui_pending&=~(1u<<BTN_HOME);page_dirty=1;
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
    return p->magic==WM_MAGIC&&p->version==1&&p->size==sizeof *p&&
      p->light<2u&&p->zoom<2u&&p->still<2u&&p->level<=127u&&p->cues<2u&&p->slot<NSLOT;
}
static void preferences_load(void)
{
    wm_preferences_t p;
    if(!settings_load(&p,sizeof p)&&preferences_valid(&p)){
        wm.light=p.light;wm.extra_large=p.zoom;wm.reduced_motion=p.still;
        vol=p.level;cues_enabled=p.cues;project_slot=p.slot;
        master_smooth=(int32_t)vol*256;
    }
}
static int preferences_save(void)
{
    wm_preferences_t p={WM_MAGIC,1,sizeof p,wm.light,wm.extra_large,
        wm.reduced_motion,vol,cues_enabled,project_slot,{0,0}},verify;
    if(settings_save(&p,sizeof p)||settings_load(&verify,sizeof verify))return -1;
    return memcmp(&p,&verify,sizeof p)?-1:0;
}
static void transport(void)
{
    fm1_irq_off();
    capture_finish_all();record_undo_pending=0;running=!running;recording=0;step_index=step_clock=cycle_index=0;step_primed=0;
    if(!running)release_sequence();
    fm1_irq_on();
    cue(running?76u:64u);page_dirty=1;
}
static void recording_end(void)
{capture_finish_all();recording=0;record_undo_pending=0;}
static void listening_set(uint8_t anchor)
{
    if(anchor_view==anchor)return;
    fm1_irq_off();recording_end();release_sequence();anchor_view=anchor;fm1_irq_on();
}
static void recording_toggle(void)
{
    fm1_irq_off();
    if(recording)recording_end();
    else{
        if(!running){fm1_irq_on();transport();fm1_irq_off();}
        recording=1;record_undo_pending=1;
        for(uint32_t lane=0;lane<2u;lane++)for(uint32_t i=0;i<WM_STEPS;i++)record_replace[lane][i]=0;
    }
    fm1_irq_on();page_dirty=1;
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
        wm_state_t fresh=WM_DEFAULTS;
        fresh.light=wm.light;fresh.extra_large=wm.extra_large;fresh.reduced_motion=wm.reduced_motion;
        fm1_irq_off();wm_mark();wm=fresh;running=recording=anchor_view=0;
        step_clock=step_index=cycle_index=0;step_primed=0;
        step_samples=(FS*15u)/wm.bpm;
        reset_performance();project_checkpoint=wm;project_origin_slot=255u;
        fm1_irq_on();project_poll();cue(76u);show_notice(N_NEW_SESSION);
    }else if(confirm==5){
        if(wm.has_anchor){
            fm1_irq_off();wm_mark();wm.memory=wm.anchor;
            wm.has_branch=wm.branch_melt=wm.melt=wm.drift=wm.decay=0;
            anchor_view=0;reset_performance();fm1_irq_on();
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
            if(prefs_ok){clean_level=vol;clean_cues=cues_enabled;preferences_pending=0;}
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
            running=recording=anchor_view=record_undo_pending=0;
            step_index=step_clock=cycle_index=0;step_primed=0;
            step_samples=(FS*15u)/wm.bpm;
            reset_performance();project_checkpoint=wm;project_origin_slot=project_slot;
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
static void music_adjust(int32_t d)
{
    if(settings_index==0u)wm.root=(uint8_t)(((int32_t)wm.root+d%12+12)%12);
    else if(settings_index==1u)wm.scale=(uint8_t)(((int32_t)wm.scale+d%WM_SCALE_COUNT+WM_SCALE_COUNT)%WM_SCALE_COUNT);
    else{
        if(running||recording){show_notice(N_STOP_FIRST);return;}
        uint8_t length=d>0?32u:16u;
        fm1_irq_off();capture_finish_all();
        if(wm_set_length(length)){step_index=step_clock=cycle_index=0;step_primed=0;release_sequence();}
        fm1_irq_on();
    }
    page_dirty=1;
}
/* Release-dispatched HOME/SEL avoid either chord order executing a solo action.
 * A held HOME still panics from its original press time, without a release action. */
static uint32_t ui_edges(uint32_t b,uint32_t released)
{
    const uint32_t pair=(1u<<BTN_HOME)|(1u<<BTN_SEL);
    uint32_t ready=0,held=fm1_in.buttons;
    if(b&(1u<<BTN_HOME))home_down=fm1_ms+1u;
    ui_pending|=b&pair;b&=~pair;
    if((held&pair)==pair&&!ui_chord_latched){
        ui_chord_latched=1;ui_pending&=~pair;home_down=anchor_down=fx_down=0;
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
        uint8_t prior_recording=recording,prior_lane=wm.layer_enabled;
        platform_poll();splash_tick();
        app_midi();
        b=fm1_input_edges(&rel);
        if(b)splash_dismiss();
        b=ui_edges(b,rel);b=save_edges(b,rel);
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
        base=(uint32_t)((int32_t)KEY0+(int8_t)octave*12);
        if(scope_open){
            if(b&(1u<<BTN_HOME)){scope_set_open(0);focus=0;page_dirty=1;}
            else if(b&(1u<<BTN_SEL)){scope_set_hold((uint8_t)!scope_hold);cue(72u);page_dirty=1;}
            if(b&(1u<<BTN_PLAY))transport();
            if((b&(1u<<BTN_REC))&&!settings_open&&!confirm&&!anchor_view){
                recording_toggle();
                page_dirty=1;
            }
        }else if(b&(1u<<BTN_HOME)){
            if(settings_open)settings_open=0;
            else {confirm=0;focus=notice=0;listening_set(0);}
            cue(72u);page_dirty=1;
        }
        else if(settings_open){
            if(b&(1u<<BTN_PLAY))transport();
            if(b&(1u<<BTN_SEL)){
                if(settings_page==3u)music_adjust(settings_index==2u?(wm.loop_steps==16u?1:-1):1);
                else if(settings_page==2u){if(settings_index==1u){cues_enabled=!cues_enabled;cue_left=0;}}
                else if(settings_page==1u)wm.fx_on^=(uint8_t)(1u<<settings_index);
                else if(settings_index==0)wm.light=!wm.light;
                else if(settings_index==1)wm.extra_large=!wm.extra_large;
                else wm.reduced_motion=!wm.reduced_motion;
                cue(72u+settings_index*4u);page_dirty=1;
            }
        }
        else if(confirm){
            if(b&(1u<<BTN_PLAY))transport();
            if(b&(1u<<BTN_SEL))apply_confirm();
            else if(b&(1u<<BTN_FX)){confirm=0;page_dirty=1;}
        }else{
            if(b&(1u<<BTN_PLAY))transport();
            if(b&(1u<<BTN_REC)){
                if(anchor_view){cue(48u);show_notice(N_ORIGINAL_SAFE);}
                else{recording_toggle();cue(recording?81u:69u);page_dirty=1;}
            }
            if(b&(1u<<BTN_SAVE)){fm1_irq_off();capture_finish_all();uint8_t ok=wm_catch();fm1_irq_on();cue(ok?84u:48u);show_notice(ok?N_ANCHORED:wm.has_anchor?N_ANCHOR_SAFE:N_NO_PHRASE);}
            if(b&(1u<<BTN_EDIT)){wm.layer_enabled=!wm.layer_enabled;if(!wm.layer_enabled)for(uint32_t v=0;v<NV;v++)if(!voice[v].live&&voice[v].seq_lane==1u)voice[v].gate=0;cue(wm.layer_enabled?74u:62u);page_dirty=1;}
            if(b&(1u<<BTN_LFO)){
                anchor_down=fm1_ms+1u;
                if(wm.has_anchor){listening_set((uint8_t)!anchor_view);cue(anchor_view?79u:67u);}
                else{cue(48u);show_notice(N_CATCH_FIRST);}
                page_dirty=1;
            }
            if(b&(1u<<BTN_GLO)){fm1_irq_off();capture_finish_all();uint8_t ok=wm_branch_now(cycle_index);fm1_irq_on();cue(ok?88u:48u);show_notice(ok?N_BRANCHED:N_CATCH_FIRST);}
            if(b&(1u<<BTN_ENV)){fm1_irq_off();capture_finish_all();uint8_t ok=wm_undo_once();step_samples=(FS*15u)/wm.bpm;if(ok)reset_performance();fm1_irq_on();cue(ok?71u:48u);show_notice(ok?N_UNDO:N_NO_UNDO);}
            if(b&(1u<<BTN_FX)){confirm=1;fx_down=fm1_ms;page_dirty=1;cue(55u);}
            if(b&(1u<<BTN_SEQ)){confirm=2;page_dirty=1;cue(57u);}
            if(b&(1u<<BTN_ARP)){confirm=3;page_dirty=1;cue(59u);}
            if(b&(1u<<BTN_SEL)){settings_open=1;recording=0;settings_index=0;
                settings_page=settings_detail=settings_detail_kind=0;focus=notice=0;cue(78u);page_dirty=1;}
        }
        if(prior_recording&&(!recording||anchor_view||confirm||settings_open||prior_lane!=wm.layer_enabled)){
            fm1_irq_off();recording_end();fm1_irq_on();
        }
        if(home_down&&(fm1_in.buttons&(1u<<BTN_HOME))&&fm1_ms+1u-home_down>=1000u){panic();home_down=0;}
        if(!(fm1_in.buttons&(1u<<BTN_HOME)))home_down=0;
        if(anchor_down&&!scope_open&&!settings_open&&!confirm&&(fm1_in.buttons&(1u<<BTN_LFO))&&fm1_ms+1u-anchor_down>=1200u){
            if(wm.has_anchor){confirm=5;cue(55u);page_dirty=1;}
            else show_notice(N_CATCH_FIRST);
            anchor_down=0;
        }
        if(!(fm1_in.buttons&(1u<<BTN_LFO)))anchor_down=0;
        if(fx_down&&!scope_open&&confirm==1&&(fm1_in.buttons&(1u<<BTN_FX))&&fm1_ms-fx_down>=1600u){
            confirm=4;fx_down=0;cue(47u);page_dirty=1;
        }
        if(!(fm1_in.buttons&(1u<<BTN_FX)))fx_down=0;
        if(b&(1u<<BTN_OCT_DOWN)){if((int8_t)octave> -2)octave=(uint8_t)((int8_t)octave-1);page_dirty=1;}
        if(b&(1u<<BTN_OCT_UP)){if((int8_t)octave<2)octave=(uint8_t)((int8_t)octave+1);page_dirty=1;}
        /* All local onsets detected in this pass share one quantization decision.
         * Keep audio enabled between individual keys, including in four-note chords. */
        fm1_irq_off();uint32_t local_step=capture_step();fm1_irq_on();
        for(k=0;k<27u;k++){
            if(down>>k&1u){
                held[k]=(uint8_t)(base+k);
                fm1_irq_off();note_on(held[k],100u,1);
                /* A latched tap that ended during a redraw still plays, then releases. */
                if(!(keys>>k&1u))for(uint32_t v=0;v<NV;v++)
                    if(voice[v].live==1u&&voice[v].note==held[k])voice[v].remain=FS/32u;
                if(recording&&!anchor_view&&!confirm&&!settings_open){
                    capture_on_at(held[k],100u,1u,local_step);
                    if(!(keys>>k&1u))capture_off(held[k],1u,0);
                }
                fm1_irq_on();page_dirty=1;
            }
            if(up>>k&1u){fm1_irq_off();capture_off(held[k],1u,0);note_off(held[k],1u);fm1_irq_on();}
        }
        if(transition)for(k=0;k<7u;k++)(void)fm1_enc_take(k);
        for(k=0;k<3u;k++){
            d=fm1_enc_take(ENC_KNOB1+k);
        if(d)splash_dismiss();
            if(d&&!scope_open&&settings_open&&settings_page==1u&&k==2u&&settings_index){
                uint8_t *p=settings_index==1u?&wm.delay_feedback:&wm.reverb_damp;
                int32_t n=(int32_t)*p+d*4;
                *p=(uint8_t)(n<0?0:n>127?127:n);
                settings_detail=*p;settings_detail_kind=settings_index==1u?2u:4u;page_dirty=1;
            }else if(d&&!confirm&&!settings_open){
                uint8_t *p=k==0?&wm.melt:k==1?&wm.drift:&wm.decay;
                int32_t n=(int32_t)*p+d*4;
                *p=(uint8_t)(n<0?0:n>127?127:n);
                focus_show((uint8_t)(k+1u),*p);
                cue(60u+k*4u+*p/32u);
            }
        }
        d=fm1_enc_take(ENC_KNOB4);
        if(d)splash_dismiss();
        if(d&&scope_open){
            int32_t n=(int32_t)vol+d*4;vol=(uint8_t)(n<0?0:n>127?127:n);page_dirty=1;
        }else if(d&&settings_open&&settings_page==3u){
            music_adjust(d);
        }else if(d&&settings_open&&settings_page==2u){
            if(settings_index==0u){int32_t n=(int32_t)vol+d*4;vol=(uint8_t)(n<0?0:n>127?127:n);}
            else if(settings_index==2u)project_slot=(uint8_t)(((int32_t)project_slot+d%8+8)%8);
            page_dirty=1;
        }else if(d&&settings_open&&settings_page==1u){
            uint8_t *p=settings_index==0u?&wm.chorus_mix:
              settings_index==1u?&wm.delay_mix:&wm.reverb_mix;
            int32_t n=(int32_t)*p+d*4;
            *p=(uint8_t)(n<0?0:n>127?127:n);
            settings_detail=settings_detail_kind=0;page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            int32_t n=(int32_t)vol+d*4;vol=(uint8_t)(n<0?0:n>127?127:n);
            focus_show(5u,vol);
        }
        d=fm1_enc_take(ENC_ALGORITHM);
        if(d)splash_dismiss();
        if(d&&scope_open){
            if(!scope_hold){int32_t n=(int32_t)scope_time+d;scope_time=(uint8_t)(n<0?0:n>5?5:n);page_dirty=1;}
        }else if(d&&settings_open&&settings_page==1u&&settings_index==1u){
            int32_t n=(int32_t)wm.delay_div+d;
            wm.delay_div=(uint8_t)(n<0?0:n>3?3:n);
            settings_detail=(uint8_t)(wm.delay_div+1u);
            settings_detail_kind=1u;page_dirty=1;
        }else if(d&&settings_open&&settings_page==1u&&settings_index==2u){
            int32_t n=(int32_t)wm.reverb_size+d*4;
            wm.reverb_size=(uint8_t)(n<0?0:n>127?127:n);
            settings_detail=wm.reverb_size;
            settings_detail_kind=3u;page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            int32_t n=(int32_t)wm.bpm+d;
            capture_finish_all();
            wm.bpm=(uint8_t)(n<60?60:n>180?180:n);
            step_samples=(FS*15u)/wm.bpm;
            focus_show(4u,wm.bpm);
        }
        d=fm1_enc_take(ENC_SELECT);
        if(d)splash_dismiss();
        if(d&&scope_open){
            int32_t n=(int32_t)scope_gain+d;scope_gain=(uint8_t)(n<0?0:n>4?4:n);page_dirty=1;
        }else if(d&&(confirm==2u||confirm==3u)){
            project_slot=(uint8_t)(((int32_t)project_slot+d%8+8)%8);page_dirty=1;
            cue(60u+project_slot*2u);
        }else if(d&&settings_open){
            settings_index=(uint8_t)(((int32_t)settings_index+d%3+3)%3);
            settings_detail=settings_detail_kind=0;
            cue(66u+settings_index*3u);page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            /* Bank jump preserves position within each group of ten. Zoom lives in settings. */
            wm.sound=wm_preset_move(wm.sound,d,1u);
            focus_show(7u,(uint8_t)(wm.sound+1u));cue(68u+(uint32_t)wm.sound/10u);
        }
        d=fm1_enc_take(ENC_PRESETS);
        if(d)splash_dismiss();
        if(d&&scope_open){
            if(!settings_open&&!confirm){wm.sound=wm_preset_move(wm.sound,d,0u);page_dirty=1;}
        }else if(d&&settings_open){
            settings_page=(uint8_t)(((int32_t)settings_page+d%4+4)%4);
            settings_index=settings_detail=settings_detail_kind=0;cue(77u);page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            wm.sound=wm_preset_move(wm.sound,d,0u);
            focus_show(7u,(uint8_t)(wm.sound+1u));
            cue(60u+(uint32_t)wm.sound/10u*2u);
        }
#ifdef DINGHY_HOST
        if(step_samples!=(FS*15u)/wm.bpm){fprintf(stderr,"Wanderer tempo and scheduler diverged\n");exit(3);}
#endif
        if(focus&&fm1_ms>=focus_until){focus=0;page_dirty=1;}
        if(notice&&notice_until&&fm1_ms>=notice_until){notice=0;page_dirty=1;}
        if(page_dirty||fm1_ms-last_project_poll>=250u)project_poll();
        app_leds();
#ifdef DINGHY_HOST
        host_verify_app();
#endif
        if(page_dirty||fm1_ms-last_ui>(scope_open&&!wm.reduced_motion?80u:250u)){
            if(page_dirty||!wm.reduced_motion||scope_open){screen();last_ui=fm1_ms;}
            page_dirty=0;
        }
    }
}
#ifdef DINGHY_HOST
#include "../../tests/wanderer_observe.h"
#endif
