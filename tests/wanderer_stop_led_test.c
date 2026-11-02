/* SPDX-License-Identifier: GPL-3.0-only
 * Playback stop must clear note lamps even if the physical LED state drifted
 * from the delta cache. Live playing while stopped keeps normal feedback.
 */
#include <assert.h>
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
static const uint64_t key_mask=(((uint64_t)1u<<27u)-1u)<<14u;
static void fresh(void)
{
    wm=(wm_state_t)WM_DEFAULTS;running=recording=anchor_view=confirm=settings_open=scope_open=notice=octave=0;
    host_leds=host_glow=led_previous=glow_previous=0;leds_initialised=0;
    step_samples=FS*15u/wm.bpm;cues_enabled=0;reset_performance();
}
int main(void)
{
    fresh();running=1;app_leds();
    /* A stale output bit is not present in the application's cached scene. */
    host_leds|=(uint64_t)1u<<KEY_LED(7);host_glow|=(uint64_t)1u<<KEY_LED(8);
    transport();app_leds();
    assert(!running&&!(host_leds&key_mask)&&!(host_glow&key_mask));
    puts("stop clears stale bright and dim keyboard lamps independently of cached scene");
    for(uint32_t view=0;view<4u;view++){
        fresh();running=1;settings_open=view==1;scope_open=view==2;confirm=view==3?2u:0u;
        sequence_event((wm_event_t){60,100,8192},0);sequence_event((wm_event_t){64,100,8192},1);app_leds();
        assert(host_leds&key_mask);transport();app_leds();
        assert(!(host_leds&key_mask)&&!(host_glow&key_mask));
        assert(!(host_leds&((uint64_t)1u<<BTN_PLAY)));
        for(uint32_t v=0;v<NV;v++)assert(!voice[v].gate);
    }
    puts("stop clears held sequence notes in Home, settings, scope and confirmation scenes");
    fresh();running=1;sequence_event((wm_event_t){60,100,8192},0);app_leds();
    running=0;app_leds();assert(!(host_leds&key_mask));
    puts("stopped LED scenes exclude sequencer gates even before voice cleanup");
    fresh();note_on(60,100,1);app_leds();assert(host_leds&key_mask);
    note_off(60,1);app_leds();assert(!(host_leds&key_mask));
    puts("live key feedback while stopped and note release remain correct");
    for(uint32_t n=0;n<256u;n++){
        fresh();running=1;sequence_event((wm_event_t){53u+n%27u,100,8192},n&1u);app_leds();
        transport();app_leds();assert(!(host_leds&key_mask)&&!(host_glow&key_mask));
    }
    puts("256 repeated loop start/stop cases leave no keyboard lamps on");
    return 0;
}
