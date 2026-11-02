/* SPDX-License-Identifier: GPL-3.0-only
 * A read-only monitor of the post-FX, post-Wanderer-master stereo average.
 * Earcons are excluded. No allocation, FFT, display calls or waiting in audio.
 * Three buffers prevent the writer from reusing a published/reader-owned frame.
 */
#define SCOPE_N 256u
static volatile uint8_t scope_open,scope_hold;
static uint8_t scope_time=1u,scope_gain=2u; /* 11.6 ms, 4x display gain */
static int16_t scope_frames[3][SCOPE_N];
static int16_t scope_trace[SCOPE_N];
static uint8_t scope_frame_time[3],scope_display_time=1u,scope_capture_time;
static volatile uint32_t scope_ready=1u,scope_reader=3u,scope_generation;
static uint32_t scope_writer,scope_pos,scope_decimate,scope_wait;
static int32_t scope_previous;
static uint8_t scope_capturing;
static uint32_t scope_peak;
static void scope_set_open(uint8_t on)
{
    if(!on){scope_open=0;return;}
    /* Audio ignores these buffers until publication of scope_open below. */
    scope_open=0;
    for(uint32_t b=0;b<3u;b++)for(uint32_t i=0;i<SCOPE_N;i++)scope_frames[b][i]=0;
    for(uint32_t i=0;i<SCOPE_N;i++)scope_trace[i]=0;
    scope_ready=1;scope_reader=3;scope_writer=scope_generation=scope_pos=0;
    scope_decimate=scope_wait=scope_previous=scope_peak=scope_capturing=0;
    for(uint32_t b=0;b<3u;b++)scope_frame_time[b]=scope_time;
    scope_display_time=scope_time;scope_hold=0;scope_open=1;
}
static void scope_feed(int32_t sample)
{
    if(!scope_open||scope_hold)return;
    if(!scope_capturing){
        /* Positive-going trigger; free-run after 40 ms so silence becomes flat. */
        if((scope_previous<=0&&sample>0)||++scope_wait>=FS/25u){
            scope_capturing=1;scope_capture_time=scope_time;scope_pos=scope_decimate=scope_wait=0;
        }
    }
    scope_previous=sample;
    if(!scope_capturing)return;
    if(scope_decimate){scope_decimate--;return;}
    scope_decimate=(1u<<scope_capture_time)-1u;
    scope_frames[scope_writer][scope_pos++]=(int16_t)sample;
    if(scope_pos==SCOPE_N){
        scope_frame_time[scope_writer]=scope_capture_time;scope_ready=scope_writer;scope_generation++;
        for(uint32_t b=0;b<3u;b++)if(b!=scope_ready&&b!=scope_reader){scope_writer=b;break;}
        scope_capturing=0;
    }
}
static void scope_snapshot(void)
{
    uint32_t at;
    if(scope_hold)return;
    fm1_irq_off();at=scope_ready;scope_reader=at;fm1_irq_on();
    for(uint32_t i=0;i<SCOPE_N;i++)scope_trace[i]=scope_frames[at][i];
    scope_display_time=scope_frame_time[at];
    fm1_irq_off();scope_reader=3u;fm1_irq_on();
    scope_peak=0;
    for(uint32_t i=0;i<SCOPE_N;i++){
        int32_t s=scope_trace[i];uint32_t a=(uint32_t)(s<0?-s:s);
        if(a>scope_peak)scope_peak=a;
    }
}

static void scope_set_hold(uint8_t on)
{
    if(on){scope_snapshot();scope_hold=1;}
    else{
        fm1_irq_off();scope_capturing=scope_pos=scope_wait=scope_decimate=0;
        scope_previous=0;scope_hold=0;fm1_irq_on();
    }
}
