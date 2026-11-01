/* SPDX-License-Identifier: GPL-3.0-only
 * Original fixed-point chorus, tempo delay and diffuse stereo room for Wanderer.
 * All buffers are statically allocated in the FM-1's pool region.
 */
#define DELAY_MASK 65535u
static int16_t delay_line[65536] __attribute__((section(".pool")));
static uint32_t delay_write,delay_frames_smooth;
static int32_t delay_tone;
static int16_t verb_a[5003] __attribute__((section(".pool")));
static int16_t verb_b[5903] __attribute__((section(".pool")));
static int16_t verb_c[6607] __attribute__((section(".pool")));
static int16_t verb_d[7307] __attribute__((section(".pool")));
static int16_t diffuse_l[1133] __attribute__((section(".pool")));
static int16_t diffuse_r[1423] __attribute__((section(".pool")));
static uint16_t verb_pos[4],diffuse_pos[2];
static int32_t verb_tone[4];
static int32_t mix_smooth[3];

static int16_t fx_bound(int32_t x)
{
    return (int16_t)(x>32767?32767:x< -32767?-32767:x);
}
static int32_t fx_diffuse(int16_t *line,uint32_t len,uint16_t *pos,int32_t input)
{
    uint32_t at=*pos;
    int32_t tap=line[at];
    int32_t write=input+(tap>>1);
    line[at]=fx_bound(write);
    *pos=(uint16_t)(at+1u==len?0u:at+1u);
    return tap-(write>>1);
}
static void fx_room(int32_t input,int32_t feedback,int32_t damping,
                    int32_t *left,int32_t *right)
{
    int16_t *lines[4]={verb_a,verb_b,verb_c,verb_d};
    const uint16_t lengths[4]={5003u,5903u,6607u,7307u};
    int32_t taps[4],sum=0;
    for(uint32_t k=0;k<4u;k++){
        taps[k]=lines[k][verb_pos[k]];
        verb_tone[k]+=((taps[k]-verb_tone[k])*(160-damping))>>8;
        sum+=verb_tone[k];
    }
    /* A four-line orthogonal feedback mix spreads one impulse among all paths. */
    for(uint32_t k=0;k<4u;k++){
        int32_t reflected=verb_tone[k]-(sum>>1);
        lines[k][verb_pos[k]]=fx_bound(input+((reflected*feedback)>>7));
        verb_pos[k]=(uint16_t)(verb_pos[k]+1u==lengths[k]?0u:verb_pos[k]+1u);
    }
    *left=fx_diffuse(diffuse_l,1133u,&diffuse_pos[0],
                     (taps[0]+taps[2])>>1);
    *right=fx_diffuse(diffuse_r,1423u,&diffuse_pos[1],
                      (taps[1]+taps[3])>>1);
}
static int32_t fx_limit(int32_t x)
{
    if(x>29000)x=29000+((x-29000)>>3);
    if(x< -29000)x=-29000+((x+29000)>>3);
    return x>32767?32767:x< -32767?-32767:x;
}
static void fx_process(int32_t *out,uint32_t n)
{
    uint32_t i;
    uint32_t times[4]={step_samples,step_samples*2u,step_samples*3u,step_samples*4u};
    uint32_t delay_frames=times[wm.delay_div&3u];
    int32_t feedback=wm.delay_feedback*86/127; /* limited to 67 percent */
    int32_t room_feedback=70+wm.reverb_size*42/127;
    int32_t damping=wm.reverb_damp>>1;
    if(delay_frames>64000u)delay_frames=64000u;
    if(!delay_frames_smooth)delay_frames_smooth=delay_frames;
    for(i=0;i<n;i++){
        uint32_t at=ambience_write,dt=delay_write;
        int32_t dry=out[i*2u],left=dry,right=dry;
        uint32_t tri=(motion_phase>>21)&2047u;
        int32_t sweep=((int32_t)(tri<1024u?tri:2047u-tri)-512)/8;
        int32_t chorus_l=ambience[(at-701u-(uint32_t)sweep)&(AMBIENCE_SIZE-1u)];
        int32_t chorus_r=ambience[(at-907u+(uint32_t)sweep)&(AMBIENCE_SIZE-1u)];
        if(delay_frames_smooth<delay_frames)
            delay_frames_smooth+=delay_frames-delay_frames_smooth>8u?8u:delay_frames-delay_frames_smooth;
        else if(delay_frames_smooth>delay_frames)
            delay_frames_smooth-=delay_frames_smooth-delay_frames>8u?8u:delay_frames_smooth-delay_frames;
        int32_t echo_l=delay_line[(dt-delay_frames_smooth)&DELAY_MASK];
        int32_t echo_r=delay_line[(dt-delay_frames_smooth-521u)&DELAY_MASK];
        int32_t rv_l,rv_r,verb_in;
        int32_t targets[3]={
            (wm.fx_on&1u)?(int32_t)wm.chorus_mix<<8:0,
            (wm.fx_on&2u)?(int32_t)wm.delay_mix<<8:0,
            (wm.fx_on&4u)?(int32_t)wm.reverb_mix<<8:0
        };
        for(uint32_t k=0;k<3u;k++)
            mix_smooth[k]+=(targets[k]-mix_smooth[k])>>7;
        ambience[at]=fx_bound(dry);
        ambience_write=(at+1u)&(AMBIENCE_SIZE-1u);
        delay_tone+=(echo_l-delay_tone)>>2;
        delay_line[dt]=fx_bound(dry+
          ((delay_tone*((wm.fx_on&2u)?feedback:0))>>7));
        delay_write=(dt+1u)&DELAY_MASK;
        verb_in=(wm.fx_on&4u)?(dry+((wm.fx_on&2u)?echo_l>>2:0))>>1:0;
        fx_room(verb_in,(wm.fx_on&4u)?room_feedback:0,damping,&rv_l,&rv_r);
        left+=((chorus_l*(mix_smooth[0]>>8))>>7);
        right+=((chorus_r*(mix_smooth[0]>>8))>>7);
        left+=((echo_l*(mix_smooth[1]>>8))>>8);
        right+=((echo_r*(mix_smooth[1]>>8))>>8);
        left+=((rv_l*(mix_smooth[2]>>8))>>7);
        right+=((rv_r*(mix_smooth[2]>>8))>>7);
        motion_phase+=24348u;
        /* A gentle output knee catches pathological four-voice + tail peaks. */
        out[i*2u]=fx_limit(left);out[i*2u+1u]=fx_limit(right);
    }
}
