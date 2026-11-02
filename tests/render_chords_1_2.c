/* SPDX-License-Identifier: GPL-3.0-only
 * Record through the production capture path, commit/reload, then render its sequencer. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static void frames(FILE *f,uint32_t count)
{
    while(count){uint32_t n=count>256u?256u:count;int32_t out[512]={0};int16_t pcm[512];app_audio(out,n);
        for(uint32_t j=0;j<n*2u;j++){assert(abs(out[j])<32767);pcm[j]=(int16_t)out[j];}
        if(f)assert(fwrite(pcm,2,n*2u,f)==n*2u);count-=n;
    }
}
static void chord(uint8_t a,uint8_t b,uint8_t c)
{
    midi_message(1,0x90,a,105);midi_message(1,0x90,b,95);midi_message(1,0x90,c,100);
    frames(0,step_samples*10);midi_message(1,0x80,b,0);
    frames(0,step_samples*2);midi_message(1,0x80,a,0);
    frames(0,step_samples*2);midi_message(1,0x80,c,0);
    frames(0,step_samples*2);
}
int main(void)
{
    wm=(wm_state_t)WM_DEFAULTS;wm.loop_steps=32;wm.sound=72;wm.fx_on=5;
    wm.chorus_mix=22;wm.reverb_mix=35;cues_enabled=0;vol=100;step_samples=FS*15u/wm.bpm;
    project_initialise();recording_toggle();chord(60,63,67);chord(53,56,60);recording_end();transport();
    assert(wm_step_notes(&wm.memory,0)==3&&wm_step_notes(&wm.memory,16)==3);
    wm_phrase_t expected=wm.memory;project_slot=7;confirm=2;apply_confirm();assert(notice==N_SAVED);
    wm_clear_phrase(&wm.memory);confirm=3;apply_confirm();assert(notice==N_LOADED&&!memcmp(&expected,&wm.memory,sizeof expected));
    FILE *f=fopen("build/chords-1.2.raw","wb");assert(f);
    const uint8_t patches[]={72,42};
    for(uint32_t p=0;p<2;p++){
        reset_performance();wm.sound=patches[p];frames(0,FS/2);
        /* A half-second pause introduces each recorded phrase. */
        frames(f,FS/2);transport();frames(f,step_samples*32u*2u);transport();frames(f,FS*2);
    }
    fclose(f);puts("two-bar three-note captured chords retain independent gates after v5 Save/Load; sequencer demo rendered with Slow Aurora and Electric Mist");
}
