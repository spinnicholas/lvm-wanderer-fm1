/* SPDX-License-Identifier: GPL-3.0-only
 * MIDI performance input. No MIDI echo: connecting a DAW cannot form a loop.
 * Every source/channel owns its notes and sustain independently of the local keys.
 */
/* Independent pending banks per port/channel. CC0=0, CC32=0/1, then PC0..127.
 * Only a valid Program Change changes the sound; banks above our 150 are ignored. */
static uint8_t midi_bank_msb[32],midi_bank_lsb[32];
static uint8_t midi_route_allows(uint32_t source,uint32_t channel)
{
 return (!wm.midi_input||wm.midi_input==source)&&(!wm.midi_channel||wm.midi_channel==channel);
}
static void arp_realtime(uint32_t source,uint8_t status)
{
 if(!wm.arp_clock||source!=wm.arp_clock)return;
 if(status!=0xf8u&&status!=0xfau&&status!=0xfbu&&status!=0xfcu)return;
 fm1_irq_off();
 if(status==0xf8u){
  uint32_t gap=audio_frames-arp_clock_frame;
  if(arp_clock_seen&&gap>=FS/200u&&gap<=FS/12u){
   uint32_t q=gap*24u;
   arp_clock_quarter=arp_clock_quarter?(arp_clock_quarter*3u+q)/4u:q;
  }
  arp_clock_frame=audio_frames;arp_clock_last=fm1_ms;arp_clock_seen=1;
  fm1_irq_on();
  return;
 }
 uint8_t was_running=running;
 if(status==0xfcu){
  if(running)transport_locked();else arp_reset();
 }else{
  if(!running)transport_locked();
  if(status==0xfau){
   if(wp_writing)wp_cancel();
   step_clock=step_index=cycle_index=step_primed=0;
   arp_release();arp_cursor=arp_until=arp_tick_serial=0;
   release_sequence();
  }
  arp_clock_last=fm1_ms;
 }
 fm1_irq_on();
 if(was_running!=running)cue_toggle(running);
 page_dirty=1;
}
static void midi_message(uint32_t source,uint8_t st,uint8_t d1,uint8_t d2)
{
    uint32_t kind=st&0xf0u,owner=2u+(source-1u)*16u+(st&15u);
    if(source<1u||source>2u)return;
    if(st>=0xf8u){arp_realtime(source,st);return;}
    if(st<0x80u||st>=0xf0u||d1>127u||d2>127u)return;
    if(!midi_route_allows(source,(st&15u)+1u))return;
    splash_dismiss();
    if(kind==0x90u&&d2&&notice_success()){notice=0;page_dirty=1;}
    fm1_irq_off();
    if(kind==0x90u&&d2){
        performance_on(d1,d2,owner);
        if(!wm.arp_on&&recording&&!wp_active&&!anchor_view&&!confirm&&!settings_open)capture_on(d1,d2,owner);
    }else if(kind==0x80u||(kind==0x90u&&!d2)){capture_off(d1,owner,sustain_pedal[owner-2u]);performance_off(d1,owner,sustain_pedal[owner-2u]);}
    else if(kind==0xb0u){
        if((d1==0u||d1==32u)&&!confirm){
            if(d1==0u)midi_bank_msb[owner-2u]=d2;else midi_bank_lsb[owner-2u]=d2;
        }else if(d1==64u){
            sustain_pedal[owner-2u]=(uint8_t)(d2>=64u);
            if(d2<64u)arp_pedal_up(owner);
            if(d2<64u)for(uint32_t lane=0;lane<CAPTURE_COUNT;lane++)
                if(capture[lane].active&&capture[lane].owner==owner&&capture[lane].sustained)capture_finish(lane);
            if(d2<64u)for(uint32_t v=0;v<NV;v++)
                if(voice[v].live==owner&&voice[v].sustained)voice[v].gate=0,voice[v].sustained=0;
        }else if(d1==120u||d1==123u){
            sustain_pedal[owner-2u]=0;arp_forget_owner(owner);
            for(uint32_t lane=0;lane<CAPTURE_COUNT;lane++)if(capture[lane].active&&capture[lane].owner==owner)capture_finish(lane);
            for(uint32_t v=0;v<NV;v++)if(voice[v].live==owner){
                voice[v].gate=0;voice[v].sustained=0;
                if(d1==120u)voice[v].amp=0;
            }
        }else if(d1==1u&&!confirm)wm.melt=d2;
    }else if(kind==0xc0u&&!confirm&&!wm.midi_ignore_program){
        uint32_t index=(uint32_t)midi_bank_lsb[owner-2u]*128u+d1;
        if(!midi_bank_msb[owner-2u]&&index<WM_PRESET_COUNT){ws_select((uint8_t)index);editor_check_page();page_dirty=1;}
    }
    fm1_irq_on();
}
static void app_midi(void)
{
    uint8_t st,d1,d2;int source;
    if(wm.arp_clock&&arp_clock_seen&&fm1_ms-arp_clock_last>1000u){fm1_irq_off();arp_reset();arp_clock_seen=0;arp_clock_quarter=0;fm1_irq_on();page_dirty=1;}
    /* Bound each pass so a MIDI flood cannot starve controls or the display. */
    for(uint32_t k=0;k<64u&&(source=midi_get(&st,&d1,&d2));k++)
        midi_message((uint32_t)source,st,d1,d2);
}
