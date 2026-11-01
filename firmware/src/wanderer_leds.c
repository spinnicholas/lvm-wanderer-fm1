/* SPDX-License-Identifier: GPL-3.0-only
 * One complete LED scene per control pass. No view inherits another view's lamps.
 * Bright = active state/current confirmation; dim = a relevant available action.
 * PLAY and sounding notes remain visible in every view because audio continues there.
 */
static uint64_t led_previous,glow_previous;
static uint8_t leds_initialised;
static void app_leds(void)
{
    uint32_t k,base=(uint32_t)((int32_t)KEY0+(int8_t)octave*12);
    uint64_t bright=0,glow=0;
    for(k=0;k<NV;k++)if(voice[k].gate&&voice[k].note>=base&&voice[k].note<base+27u)
        bright|=(uint64_t)1u<<(KEY_LED(voice[k].note-base));
    if(running)bright|=(uint64_t)1u<<(BTN_PLAY);
    if(scope_open){
        if(recording)bright|=(uint64_t)1u<<BTN_REC;
        if(scope_hold)bright|=(uint64_t)1u<<BTN_SEL;
        glow|=(uint64_t)1u<<BTN_HOME;glow|=(uint64_t)1u<<BTN_SEL;
    }else if(settings_open){
        bright|=(uint64_t)1u<<(BTN_SEL);glow|=(uint64_t)1u<<(BTN_HOME);
    }else if(confirm){
        uint32_t button=confirm==1u||confirm==4u?BTN_FX:
            confirm==2u?BTN_SEQ:confirm==3u?BTN_ARP:BTN_LFO;
        bright|=(uint64_t)1u<<(button);bright|=(uint64_t)1u<<(BTN_SEL);
        glow|=(uint64_t)1u<<(BTN_HOME);
    }else if(notice_sticky()){
        /* Persistent outcome/error pages are their own scene, not hidden Home.
         * Recording can still be real after a refused action, so retain its lamp. */
        if(recording)bright|=(uint64_t)1u<<BTN_REC;
        bright|=(uint64_t)1u<<BTN_SEL;
        if(notice==N_SAVED||notice==N_SAVE_FAILED||notice==N_PREF_FAILED)bright|=(uint64_t)1u<<BTN_SEQ;
        if(notice==N_LOADED||notice==N_NO_VALID_SAVE)bright|=(uint64_t)1u<<BTN_ARP;
        glow|=((uint64_t)1u<<BTN_HOME)|((uint64_t)1u<<BTN_FX);
    }else{
        if(recording)bright|=(uint64_t)1u<<(BTN_REC);
        if(wm.has_anchor)bright|=(uint64_t)1u<<(BTN_SAVE);
        if(!wm.has_anchor&&wm_has_notes(&wm.memory))glow|=(uint64_t)1u<<(BTN_SAVE);
        if(wm.layer_enabled)bright|=(uint64_t)1u<<(BTN_EDIT);
        if(anchor_view)bright|=(uint64_t)1u<<(BTN_LFO);
        if(wm.has_anchor&&!anchor_view)glow|=(uint64_t)1u<<(BTN_LFO);
        if(wm.has_anchor)glow|=(uint64_t)1u<<(BTN_GLO);
        if(wm_can_undo)glow|=(uint64_t)1u<<(BTN_ENV);
    }
    if(!scope_open&&!settings_open&&!confirm&&!notice_sticky()){
        if((int8_t)octave<0)bright|=(uint64_t)1u<<(BTN_OCT_DOWN);
        if((int8_t)octave>0)bright|=(uint64_t)1u<<(BTN_OCT_UP);
    }
    if(save_pending&&!settings_open&&!confirm&&!scope_open)glow|=(uint64_t)1u<<BTN_SEQ;
    if(ui_pending&&!ui_chord_latched)glow|=((uint64_t)1u<<BTN_HOME)|((uint64_t)1u<<BTN_SEL);
    for(k=0;k<41u;k++){
        uint64_t bit=(uint64_t)1u<<k;
        if(!leds_initialised||((bright^led_previous)&bit))fm1_led_key(k,(bright&bit)!=0);
        if(!leds_initialised||((glow^glow_previous)&bit))fm1_led_key_glow(k,(glow&bit)!=0);
    }
    led_previous=bright;glow_previous=glow;leds_initialised=1;
}
