/* SPDX-License-Identifier: GPL-3.0-only
 * MIDI performance input. No MIDI echo: connecting a DAW cannot form a loop.
 * Every source/channel owns its notes and sustain independently of the local keys.
 */
static void midi_message(uint32_t source,uint8_t st,uint8_t d1,uint8_t d2)
{
    uint32_t kind=st&0xf0u,owner=2u+(source-1u)*16u+(st&15u);
    if(source<1u||source>2u||st<0x80u||st>=0xf0u||d1>127u||d2>127u)return;
    splash_dismiss();
    if(kind==0x90u&&d2&&notice_success()){notice=0;page_dirty=1;}
    fm1_irq_off();
    if(kind==0x90u&&d2){
        note_on(d1,d2,owner);
        if(recording&&!anchor_view&&!confirm&&!settings_open)capture_on(d1,d2,owner);
    }else if(kind==0x80u||(kind==0x90u&&!d2)){capture_off(d1,owner,sustain_pedal[owner-2u]);note_off(d1,owner);}
    else if(kind==0xb0u){
        if(d1==64u){
            sustain_pedal[owner-2u]=(uint8_t)(d2>=64u);
            if(d2<64u)for(uint32_t lane=0;lane<CAPTURE_COUNT;lane++)
                if(capture[lane].active&&capture[lane].owner==owner&&capture[lane].sustained)capture_finish(lane);
            if(d2<64u)for(uint32_t v=0;v<NV;v++)
                if(voice[v].live==owner&&voice[v].sustained)voice[v].gate=0,voice[v].sustained=0;
        }else if(d1==120u||d1==123u){
            sustain_pedal[owner-2u]=0;
            for(uint32_t lane=0;lane<CAPTURE_COUNT;lane++)if(capture[lane].active&&capture[lane].owner==owner)capture_finish(lane);
            for(uint32_t v=0;v<NV;v++)if(voice[v].live==owner){
                voice[v].gate=0;voice[v].sustained=0;
                if(d1==120u)voice[v].amp=0;
            }
        }else if(d1==1u&&!confirm)wm.melt=d2;
    }else if(kind==0xc0u&&d1<WM_PRESET_COUNT&&!confirm){
        wm.sound=d1;page_dirty=1;
    }
    fm1_irq_on();
}
static void app_midi(void)
{
    uint8_t st,d1,d2;int source;
    /* Bound each pass so a MIDI flood cannot starve controls or the display. */
    for(uint32_t k=0;k<64u&&(source=midi_get(&st,&d1,&d2));k++)
        midi_message((uint32_t)source,st,d1,d2);
}
