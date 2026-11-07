/* SPDX-License-Identifier: GPL-3.0-only
 * One complete LED scene per control pass. No view inherits another view's lamps.
 * Bright = active state/current confirmation; dim = a relevant available action.
 * PLAY and sounding notes remain visible in every view because audio continues there.
 * 3.0: PLAY (and REC while recording) pulse bright on every beat, so the tempo can be
 * seen without reading the screen. Root-note keys glow as landmarks on Home. GLO
 * flashes when Memory wanders at a loop boundary; LFO pulses on the bar once
 * Memory is far from its Anchor, inviting a comparison.
 */
static uint32_t led_tide_until;
static uint64_t led_previous,glow_previous;
static uint8_t leds_initialised,leds_was_running;
static void app_leds(void)
{
    uint32_t k,base=(uint32_t)((int32_t)KEY0+(int8_t)octave*12);
    uint64_t bright=0,glow=0;uint8_t key_notes[27];uint32_t arp_notes[4]={0,0,0,0};
    for(uint32_t key=0;key<27u;key++)key_notes[key]=(uint8_t)(wm.keyboard_scale?wm_scale_move((uint8_t)(base-base%12u+wm.root),(int32_t)key):base+key);
    for(uint32_t i=0;i<arp_count;i++)arp_notes[arp_keys[i].note>>5]|=1u<<(arp_keys[i].note&31u);
    /* Stop is a full refresh: clear stale hardware lamps even if they no
     * longer match the delta cache. Released sequence tails are not held keys. */
    uint8_t refresh=(uint8_t)(!leds_initialised||(leds_was_running&&!running));
    for(k=0;k<NV;k++)if(voice[k].gate&&(running||voice[k].live)){
        for(uint32_t key=0;key<27u;key++){
            uint32_t note=key_notes[key];
            if((voice[k].live==1u&&(fm1_in.notes>>key&1u)?held[key]:note)==voice[k].note)bright|=(uint64_t)1u<<KEY_LED(key);
        }
    }
    if(wp_armed||wp_writing)bright|=(uint64_t)1u<<BTN_REC;
    if(count_in_total&&!running){
        /* Count-in: REC flashes on each of the four beats. */
        uint32_t beat=step_samples*4u;
        if(beat&&count_in_pos%beat<beat/4u)bright|=(uint64_t)1u<<BTN_REC;else glow|=(uint64_t)1u<<BTN_REC;
    }
    uint32_t played=(step_index+wm.loop_steps-1u)&(wm.loop_steps-1u);
    uint8_t on_beat=(uint8_t)(running&&step_primed&&(played&3u)==0u&&step_clock<step_samples*3u/4u);
    if(running){if(on_beat)bright|=(uint64_t)1u<<BTN_PLAY;else glow|=(uint64_t)1u<<BTN_PLAY;}
    if(scope_open){
        if(recording)bright|=(uint64_t)1u<<BTN_REC;
        if(scope_hold)bright|=(uint64_t)1u<<BTN_SEL;
        glow|=(uint64_t)1u<<BTN_HOME;glow|=(uint64_t)1u<<BTN_SEL;
    }else if(editor_open){
        bright|=(uint64_t)1u<<BTN_EDIT;glow|=(uint64_t)1u<<BTN_HOME;
        if(editor_page==EDIT_PRINT){glow|=((uint64_t)1u<<BTN_REC)|((uint64_t)1u<<BTN_ENV);if(wp_armed||wp_writing)bright|=(uint64_t)1u<<BTN_REC;}
    }else if(settings_open){
        bright|=(uint64_t)1u<<(BTN_SEL);glow|=(uint64_t)1u<<(BTN_HOME);
    }else if(confirm){
        uint32_t button=confirm==1u||confirm==4u?BTN_FX:
            confirm==2u?BTN_SEQ:confirm==3u?BTN_ARP:confirm==6u?BTN_SEL:confirm==7u?BTN_SAVE:BTN_LFO;
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
        if(recording){if(on_beat||!running)bright|=(uint64_t)1u<<BTN_REC;else glow|=(uint64_t)1u<<BTN_REC;}
        uint8_t anchored=(uint8_t)(wm.has_anchor||wp_has_anchor);
        uint8_t memory_ready=(uint8_t)(!anchored&&wm_memory_playable());
        if(anchored)bright|=(uint64_t)1u<<BTN_SAVE;
        if(memory_ready)glow|=(uint64_t)1u<<BTN_SAVE;
        if(layer_focus)bright|=(uint64_t)1u<<(BTN_EDIT);
        if(anchor_view)bright|=(uint64_t)1u<<(BTN_LFO);
        if(!anchor_view&&(anchored||(!recording&&memory_ready)))glow|=(uint64_t)1u<<BTN_LFO;
        if(!anchor_view&&anchored&&ww.distance>=50u&&on_beat&&played==0u)bright|=(uint64_t)1u<<BTN_LFO;
        if(wm.has_anchor||wp_has_anchor||ww.distance)glow|=(uint64_t)1u<<(BTN_GLO);
        if(led_tide_until&&(int32_t)(fm1_ms-led_tide_until)<0)bright|=(uint64_t)1u<<BTN_GLO;
        /* Root keys glow as landmarks while the arpeggiator is off. */
        if(!wm.arp_on)for(uint32_t key=0;key<27u;key++)
            if(key_notes[key]%12u==wm.root)glow|=(uint64_t)1u<<KEY_LED(key);
        if(wm_can_undo)glow|=(uint64_t)1u<<(BTN_ENV);
        if(metronome_on)glow|=(uint64_t)1u<<BTN_SEQ; /* the click is on: tap SEQ to stop it */
    }
    if(!scope_open&&!settings_open&&!confirm&&!notice_sticky()){
        if((int8_t)octave<0)bright|=(uint64_t)1u<<(BTN_OCT_DOWN);
        if((int8_t)octave>0)bright|=(uint64_t)1u<<(BTN_OCT_UP);
    }
    /* Position and pulse, never red/green alone: ARP = running gate; GLO = Scatter. */
    if(wm.arp_on){
        glow|=(uint64_t)1u<<BTN_ARP;
        if(arp_gate_left&&arp_note<128u)bright|=(uint64_t)1u<<BTN_ARP;
        for(uint32_t key=0;key<27u;key++)if((arp_notes[key_notes[key]>>5]>>(key_notes[key]&31u))&1u)glow|=(uint64_t)1u<<KEY_LED(key);
    }
    if(wp_scatter_mix&&scatter.kind)bright|=(uint64_t)1u<<BTN_GLO;
    if(save_pending&&!settings_open&&!confirm&&!scope_open)glow|=(uint64_t)1u<<BTN_SEQ;
    if(ui_pending&&!ui_chord_latched)glow|=((uint64_t)1u<<BTN_HOME)|((uint64_t)1u<<BTN_SEL);
    for(k=0;k<41u;k++){
        uint64_t bit=(uint64_t)1u<<k;
        if(refresh||((bright^led_previous)&bit))fm1_led_key(k,(bright&bit)!=0);
        if(refresh||((glow^glow_previous)&bit))fm1_led_key_glow(k,(glow&bit)!=0);
    }
    led_previous=bright;glow_previous=glow;leds_initialised=1;leds_was_running=running;
}
