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
#include "wanderer_memory.h"

/* Atkinson Hyperlegible, SIL OFL. Offline generated 4-bit run-length glyphs. */
typedef struct {uint8_t h,n;const char *chars;const uint8_t *adv,*bw;const uint32_t *off;const uint8_t *data;} font_t;
#include "wanderer_font.h"
#include "wanderer_icons.h"
#include "wanderer_waves.h"
#include "wanderer_presets.h"
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
    N_NO_VALID_SAVE, N_LOADED, N_STOP_FIRST
};
static struct {
    volatile uint32_t inc;volatile int32_t gate;
    uint32_t ph,ph2,inc2,age,remain;
    int32_t amp,f1,f2,brightness,fm_brightness;
    uint8_t note,live,patch,band1,band2,stage;
} voice[NV];
/* One short mono feedback line, read at two unequal times for a quiet stereo tail.
 * 8192 x 16-bit samples = 16 KiB; no allocation or division in the audio ISR. */
#define AMBIENCE_SIZE 8192u
static int16_t ambience[AMBIENCE_SIZE] __attribute__((section(".pool")));
static uint32_t ambience_write;
static uint32_t motion_phase;
static uint32_t ages,step_clock,step_index,cycle_index,step_samples=5906u;
static uint8_t step_primed;
static volatile uint32_t cue_left,cue_phase,cue_step;
static volatile uint8_t running,recording,anchor_view,vol=100;
static uint8_t octave,held[27],page_dirty=1,confirm,focus,focus_value;
static uint8_t settings_open,settings_index,settings_page,settings_detail;
static uint32_t settings_detail_until;
static uint32_t focus_until;
static uint8_t notice;
static uint32_t notice_until;
static uint32_t fx_down;
static uint32_t prev_notes;
static const uint32_t INC_C9[12]={815363807u,863847862u,915214929u,969636441u,1027294024u,1088380105u,
  1153098554u,1221665363u,1294309365u,1371273005u,1452813141u,1539201906u};
static uint32_t tone_step(uint32_t note){return INC_C9[note%12u]>>(10u-note/12u);}
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
    cue_phase=0;cue_step=tone_step(pitch);cue_left=FS/16u;
}
static void note_on(uint32_t note,uint32_t vel,uint32_t sequence)
{
    uint32_t v,pick=0,best=0xffffffffu;
    const wm_preset_t *p=&WM_PRESETS[wm.sound];
    if(note>127u||!vel)return;
    for(v=0;v<NV;v++){
        uint32_t score=(!voice[v].gate&&!voice[v].amp)?0u:
          (!voice[v].gate)?1u+(uint32_t)voice[v].amp:
          (voice[v].live&&!sequence&&voice[v].note==note)?2u:
          0x80000000u+voice[v].age;
        if(score<best){best=score;pick=v;}
    }
    voice[pick].note=(uint8_t)note;voice[pick].age=++ages;
    voice[pick].inc=tone_step(note);voice[pick].gate=(int32_t)(vel*64u);
    voice[pick].patch=wm.sound;
    /* Q4 oscillator ratio: 8 = sub, 16 = unison, 32 = octave.
     * High-register intervals return to unison before phase wrap aliases. */
    uint32_t ratio=note>84u&&p->ratio>16u?16u:p->ratio;
    voice[pick].inc2=(voice[pick].inc>>4)*ratio+
      (voice[pick].inc>>12)*p->detune;
    voice[pick].band1=wave_band(voice[pick].inc);
    voice[pick].band2=wave_band(voice[pick].inc2);
    voice[pick].ph=voice[pick].ph2=0u;
    voice[pick].amp=voice[pick].f1=voice[pick].f2=0;
    voice[pick].brightness=voice[pick].fm_brightness=32767;
    voice[pick].stage=0u;
    voice[pick].remain=sequence?step_samples*3u/4u:0u;
    voice[pick].live=(uint8_t)!sequence;
}
static void note_off(uint32_t note)
{
    uint32_t v;
    for(v=0;v<NV;v++)if(voice[v].live&&voice[v].note==note)voice[v].gate=0;
}
static void step_play(void)
{
    wm_event_t e;
    uint32_t i=step_index;
    e=anchor_view?wm.anchor.e[i]:wm_evolve(wm.memory.e[i],i,cycle_index);
    if(e.velocity)note_on(e.note,e.velocity,1);
    if(!anchor_view){e=wm.layer.e[i];if(e.velocity)note_on(e.note,e.velocity,1);}
    step_index=(i+1u)&15u;
    if(!step_index)cycle_index++;
}
#include "wanderer_fx.c"
static void app_audio(int32_t *out,uint32_t n)
{
    uint32_t v,i;int32_t melt=anchor_view?0:(int32_t)wm.branch_melt+wm.melt;
    int32_t motion=sine(motion_phase)>>12;
    if(melt>127)melt=127;
    if(running){
        if(!step_primed){step_play();step_primed=1;}
        step_clock+=n;
        if(step_clock>=step_samples){step_clock-=step_samples;step_play();}
    }
    for(v=0;v<NV;v++){
        uint32_t ph=voice[v].ph,ph2=voice[v].ph2,inc=voice[v].inc,
          inc2=voice[v].inc2,remain=voice[v].remain,patch=voice[v].patch;
        int32_t amp=voice[v].amp,g=voice[v].gate,f1=voice[v].f1,
          f2=voice[v].f2,bright=voice[v].brightness,
          fmbright=voice[v].fm_brightness;
        uint8_t stage=voice[v].stage;
        const wm_preset_t *p=&WM_PRESETS[patch];
        int32_t sustain=(g*(int32_t)p->sustain)>>7;
        if(!amp&&!g)continue;
        for(i=0;i<n;i++){
            int32_t s,cut;
            if(remain&&!--remain){g=0;voice[v].gate=0;}
            if(!g)stage=3u;
            if(stage==0u){
                amp+=p->attack;
                if(amp>=g){amp=g;stage=1u;}
            }else if(stage==1u){
                /* Table value is a contour speed, scaled for musical tens of ms. */
                amp-=(p->decay+31u)>>5;
                if(amp<=sustain){amp=sustain;stage=2u;}
            }else if(stage==2u)amp=sustain;
            else{amp-=p->release;if(amp<0)amp=0;}
            ph+=inc;ph2+=inc2;
            uint32_t offset=0u;
            if(p->fm&&voice[v].note<=96u){
                uint32_t fm=(uint32_t)((fmbright*(int32_t)p->fm)>>15);
                if(voice[v].note>84u)fm>>=1u;
                offset=(uint32_t)(sine(ph2)*(int32_t)fm)*256u;
            }
            s=(osc_sample(ph+offset,p->osc1,voice[v].band1)*(128-p->mix)+
               osc_sample(ph2,p->osc2,voice[v].band2)*p->mix)>>7;
            if(bright>0){bright-=(p->contour_decay+3u)>>2;if(bright<0)bright=0;}
            if(fmbright>0){fmbright-=p->fm_decay;if(fmbright<0)fmbright=0;}
            cut=p->cutoff+((bright*(int32_t)p->contour)>>15)+(melt>>2)+motion;
            if(cut<18)cut=18;
            if(cut>220)cut=220;
            f1+=((s-f1)*cut)>>8;
            f2+=((f1-f2)*cut)>>8;
            s=f2;
            s=(s*amp)>>15;
            s=(s*(int32_t)vol*6)>>9;
            out[i*2u]+=s;
        }
        voice[v].ph=ph;voice[v].ph2=ph2;voice[v].amp=amp;
        voice[v].f1=f1;voice[v].f2=f2;voice[v].brightness=bright;
        voice[v].fm_brightness=fmbright;voice[v].stage=stage;
        voice[v].remain=remain;
    }
    fx_process(out,n);
    for(i=0;i<n;i++){
        int32_t left=out[i*2u],right=out[i*2u+1u];
        if(cue_left){
            int32_t cue_sample=sine(cue_phase)>>5;
            if(cue_left<256u)cue_sample=(cue_sample*(int32_t)cue_left)>>8;
            cue_phase+=cue_step;cue_left--;
            left+=cue_sample;right+=cue_sample;
        }
        out[i*2u]=left;out[i*2u+1u]=right;
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
static void focus_show(uint8_t id,uint8_t v){focus=id;focus_value=v;focus_until=fm1_ms+1200u;page_dirty=1;}
static void show_notice(uint8_t id){focus=0;notice=id;notice_until=fm1_ms+1200u;page_dirty=1;}
static void transport(void)
{
    running=!running;recording=0;step_index=step_clock=cycle_index=0;step_primed=0;
    cue(running?76u:64u);page_dirty=1;
}
static void apply_confirm(void)
{
    static wm_disk_t disk;
    static wm_disk_legacy_t old_disk;
    if((confirm==2||confirm==3)&&(running||recording)){
        cue(45u);show_notice(N_STOP_FIRST);confirm=0;return;
    }
    if(confirm==1){
        fm1_irq_off();wm_mark();for(uint32_t i=0;i<WM_STEPS;i++)wm.layer.e[i].velocity=0;fm1_irq_on();
        show_notice(N_LAYER_CLEARED);
    }else if(confirm==4){
        wm_state_t fresh=WM_DEFAULTS;
        fm1_irq_off();wm_mark();wm=fresh;running=recording=anchor_view=0;
        step_clock=step_index=cycle_index=0;step_primed=0;
        step_samples=(FS*15u)/wm.bpm;
        for(uint32_t v=0;v<NV;v++)voice[v].gate=0;
        fm1_irq_on();cue(76u);show_notice(N_NEW_SESSION);
    }else if(confirm==2){
        disk.magic=WM_MAGIC;disk.version=3;disk.size=(uint16_t)sizeof wm;
        disk.state=wm;
        if(slot_save(0,&disk,sizeof disk)){cue(45u);show_notice(N_SAVE_FAILED);}
        else {cue(79u);show_notice(N_SAVED);}
    }else if(confirm==3){
        uint8_t modern=!slot_load(0,&disk,sizeof disk)&&wm_valid(&disk);
        uint8_t legacy=modern?0u:
          (uint8_t)(!slot_load(0,&old_disk,sizeof old_disk)&&wm_valid_legacy(&old_disk));
        if(!modern&&!legacy){cue(45u);show_notice(N_NO_VALID_SAVE);}
        else{
            fm1_irq_off();wm_mark();wm=modern?disk.state:wm_migrate_legacy(&old_disk);
            running=recording=anchor_view=0;
            step_index=step_clock=cycle_index=0;step_primed=0;
            step_samples=(FS*15u)/wm.bpm;
            for(uint32_t v=0;v<NV;v++)voice[v].gate=0;
            fm1_irq_on();cue(79u);show_notice(N_LOADED);
        }
    }
    confirm=0;page_dirty=1;
}
static void app_main(void)
{
    uint32_t last_ui=0;
    /* Slot 1 is only read after explicit LOAD; never overwrite another firmware's data on boot. */
    lcd_fill(0,0,240,240,DARK);
    for(;;){
        uint32_t b,rel,keys,down,up,k,base,transition;
        int32_t d;
        platform_poll();
        b=fm1_input_edges(&rel);
        transition=b&((1u<<BTN_HOME)|(1u<<BTN_REC)|(1u<<BTN_SAVE)|(1u<<BTN_EDIT)|
          (1u<<BTN_LFO)|(1u<<BTN_GLO)|(1u<<BTN_ENV)|(1u<<BTN_FX)|
          (1u<<BTN_SEQ)|(1u<<BTN_ARP)|(1u<<BTN_SEL));
        keys=fm1_in.notes;down=keys&~prev_notes;up=prev_notes&~keys;prev_notes=keys;
        base=(uint32_t)((int32_t)KEY0+(int8_t)octave*12);
        if(b&(1u<<BTN_HOME)){
            if(settings_open)settings_open=0;
            else {confirm=0;focus=0;anchor_view=0;}
            cue(72u);page_dirty=1;
        }
        else if(settings_open){
            if(b&(1u<<BTN_PLAY))transport();
            if(b&(1u<<BTN_SEL)){
                if(settings_page)wm.fx_on^=(uint8_t)(1u<<settings_index);
                else if(settings_index==0)wm.light=!wm.light;
                else if(settings_index==1)wm.extra_large=!wm.extra_large;
                else wm.reduced_motion=!wm.reduced_motion;
                cue(72u+settings_index*4u);page_dirty=1;
            }
        }
        else if(confirm){
            if(b&(1u<<BTN_SEL))apply_confirm();
            else if(b&(1u<<BTN_FX)){confirm=0;page_dirty=1;}
        }else{
            if(b&(1u<<BTN_PLAY))transport();
            if(b&(1u<<BTN_REC)){
                if(anchor_view){cue(48u);show_notice(N_ORIGINAL_SAFE);}
                else{recording=!recording;if(recording&&!running)transport(),recording=1;cue(recording?81u:69u);page_dirty=1;}
            }
            if(b&(1u<<BTN_SAVE)){fm1_irq_off();uint8_t ok=wm_catch();fm1_irq_on();cue(ok?84u:48u);show_notice(ok?N_ANCHORED:wm.has_anchor?N_ANCHOR_SAFE:N_NO_PHRASE);}
            if(b&(1u<<BTN_EDIT)){wm.layer_enabled=!wm.layer_enabled;cue(wm.layer_enabled?74u:62u);page_dirty=1;}
            if(b&(1u<<BTN_LFO)){anchor_view=wm.has_anchor?!anchor_view:0;cue(anchor_view?79u:67u);page_dirty=1;}
            if(b&(1u<<BTN_GLO)){fm1_irq_off();uint8_t ok=wm_branch_now(cycle_index);fm1_irq_on();cue(ok?88u:48u);show_notice(ok?N_BRANCHED:N_CATCH_FIRST);}
            if(b&(1u<<BTN_ENV)){fm1_irq_off();uint8_t ok=wm_undo_once();step_samples=(FS*15u)/wm.bpm;fm1_irq_on();cue(ok?71u:48u);show_notice(ok?N_UNDO:N_NO_UNDO);}
            if(b&(1u<<BTN_FX)){confirm=1;fx_down=fm1_ms;page_dirty=1;cue(55u);}
            if(b&(1u<<BTN_SEQ)){confirm=2;page_dirty=1;cue(57u);}
            if(b&(1u<<BTN_ARP)){confirm=3;page_dirty=1;cue(59u);}
            if(b&(1u<<BTN_SEL)){settings_open=1;settings_index=0;
                settings_page=settings_detail=0;focus=notice=0;cue(78u);page_dirty=1;}
        }
        if(fx_down&&confirm==1&&(fm1_in.buttons&(1u<<BTN_FX))&&fm1_ms-fx_down>=1600u){
            confirm=4;fx_down=0;cue(47u);page_dirty=1;
        }
        if(!(fm1_in.buttons&(1u<<BTN_FX)))fx_down=0;
        if(b&(1u<<BTN_OCT_DOWN)){if((int8_t)octave> -2)octave=(uint8_t)((int8_t)octave-1);page_dirty=1;}
        if(b&(1u<<BTN_OCT_UP)){if((int8_t)octave<2)octave=(uint8_t)((int8_t)octave+1);page_dirty=1;}
        for(k=0;k<27u;k++){
            if(down>>k&1u){
                held[k]=(uint8_t)(base+k);
                fm1_irq_off();note_on(held[k],100u,0);
                if(recording&&!anchor_view&&!confirm){
                    wm_mark();wm_record(step_index,held[k],100u);
                }
                fm1_irq_on();page_dirty=1;
            }
            if(up>>k&1u){fm1_irq_off();note_off(held[k]);fm1_irq_on();}
            fm1_led_key(KEY_LED(k),keys>>k&1u);
        }
        fm1_led_key(BTN_PLAY,running);
        fm1_led_key(BTN_REC,recording);
        fm1_led_key(BTN_SAVE,wm.has_anchor);
        fm1_led_key(BTN_EDIT,wm.layer_enabled);
        fm1_led_key(BTN_LFO,anchor_view);
        if(transition)for(k=0;k<7u;k++)(void)fm1_enc_take(k);
        for(k=0;k<3u;k++){
            d=fm1_enc_take(ENC_KNOB1+k);
            if(d&&settings_open&&settings_page&&k==2u&&settings_index){
                uint8_t *p=settings_index==1u?&wm.delay_feedback:&wm.reverb_damp;
                int32_t n=(int32_t)*p+d*4;
                *p=(uint8_t)(n<0?0:n>127?127:n);
                settings_detail=*p;settings_detail_until=fm1_ms+1300u;page_dirty=1;
            }else if(d&&!confirm&&!settings_open){
                uint8_t *p=k==0?&wm.melt:k==1?&wm.drift:&wm.decay;
                int32_t n=(int32_t)*p+d*4;
                *p=(uint8_t)(n<0?0:n>127?127:n);
                focus_show((uint8_t)(k+1u),*p);
                cue(60u+k*4u+*p/32u);
            }
        }
        d=fm1_enc_take(ENC_KNOB4);
        if(d&&settings_open&&settings_page){
            uint8_t *p=settings_index==0u?&wm.chorus_mix:
              settings_index==1u?&wm.delay_mix:&wm.reverb_mix;
            int32_t n=(int32_t)*p+d*4;
            *p=(uint8_t)(n<0?0:n>127?127:n);
            settings_detail=0;page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            int32_t n=(int32_t)vol+d*4;vol=(uint8_t)(n<0?0:n>127?127:n);
            focus_show(5u,vol);
        }
        d=fm1_enc_take(ENC_ALGORITHM);
        if(d&&settings_open&&settings_page&&settings_index==1u){
            int32_t n=(int32_t)wm.delay_div+d;
            wm.delay_div=(uint8_t)(n<0?0:n>3?3:n);
            settings_detail=(uint8_t)(wm.delay_div+1u);
            settings_detail_until=fm1_ms+1300u;page_dirty=1;
        }else if(d&&settings_open&&settings_page&&settings_index==2u){
            int32_t n=(int32_t)wm.reverb_size+d*4;
            wm.reverb_size=(uint8_t)(n<0?0:n>127?127:n);
            settings_detail=wm.reverb_size;
            settings_detail_until=fm1_ms+1300u;page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            int32_t n=(int32_t)wm.bpm+d;
            wm.bpm=(uint8_t)(n<60?60:n>180?180:n);
            step_samples=(FS*15u)/wm.bpm;
            focus_show(4u,wm.bpm);
        }
        d=fm1_enc_take(ENC_SELECT);
        if(d&&settings_open){
            settings_index=(uint8_t)(((int32_t)settings_index+d%3+3)%3);
            settings_detail=0;
            cue(66u+settings_index*3u);page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            /* Bank jump preserves position within each group of ten. Zoom lives in settings. */
            wm.sound=wm_preset_move(wm.sound,d,1u);
            focus_show(7u,(uint8_t)(wm.sound+1u));cue(68u+(uint32_t)wm.sound/10u);
        }
        d=fm1_enc_take(ENC_PRESETS);
        if(d&&settings_open){
            settings_page=(uint8_t)(1u-settings_page);
            settings_index=settings_detail=0;cue(77u);page_dirty=1;
        }else if(d&&!confirm&&!settings_open){
            wm.sound=wm_preset_move(wm.sound,d,0u);
            focus_show(7u,(uint8_t)(wm.sound+1u));
            cue(60u+(uint32_t)wm.sound*5u);
        }
#ifdef DINGHY_HOST
        if(step_samples!=(FS*15u)/wm.bpm){fprintf(stderr,"Wanderer tempo and scheduler diverged\n");exit(3);}
#endif
        if(focus&&fm1_ms>=focus_until){focus=0;page_dirty=1;}
        if(notice&&fm1_ms>=notice_until){notice=0;page_dirty=1;}
        if(page_dirty||fm1_ms-last_ui>250u){
            if(page_dirty||!wm.reduced_motion){screen();last_ui=fm1_ms;}
            page_dirty=0;
        }
    }
}
