/* SPDX-License-Identifier: GPL-3.0-only */
static uint8_t settings_fx_bit(void)
{return settings_page==9?1:settings_page==10?2:settings_page==12?4:settings_page==13?8:0;}
static uint8_t *settings_fx_param(uint32_t k)
{
 uint8_t *p[5][4]={{0,&wm.crush_mix,&wm.crush_bits,&wm.crush_rate},
  {0,&wm.tape_mix,&wm.tape_time,&wm.tape_feedback},{&wm.tape_tone,&wm.tape_wow,&wm.tape_heads,&wm.tape_sync},
  {0,&wm.phaser_mix,&wm.phaser_rate,&wm.phaser_depth},{0,&wm.fold_mix,&wm.fold_drive,&wm.fold_tone}};
 return p[settings_page-9u][k];
}
static uint8_t settings_bool(uint32_t k)
{
 return (settings_page==SETTINGS_MIDI&&k==3u)||(settings_page==SETTINGS_LAYER)||(settings_page>=9u&&k==0u&&settings_fx_bit())||(settings_page==0u&&k!=0u)||(settings_page>=1u&&settings_page<SETTINGS_REVERB&&k==0u)||
 (settings_page==4u&&k==3u)||
 (settings_page==6u&&k==0u)||(settings_page==7u&&k==2u)||(settings_page==SETTINGS_ACCESS_FEEL&&k==3u);
}
static uint32_t settings_get(uint32_t k)
{
 if(settings_page==SETTINGS_MIDI)return k==0u?wm.midi_input:k==1u?wm.midi_channel:k==2u?wm.arp_clock:!wm.midi_ignore_program;
 if(settings_page==SETTINGS_LAYER)return k==0u?layer_focus:k==1u?wm.layer_enabled:ws_layer_own();
 if(settings_page==SETTINGS_SYSTEM)return k==0u?cpu_load:k==1u?cpu_peak:k==2u?cpu_lean:0u;
 if(settings_page>=9){uint8_t *p=settings_fx_param(k);return p?*p:(wm.fx_new_on&settings_fx_bit())!=0;}
 if(settings_page==0u)return k==0?color_theme:k==1?wm.extra_large:wm.reduced_motion;
 if(settings_page>=1u&&settings_page<=3u){
  uint32_t effect=settings_page==1u?1u:settings_page==2u?2u:4u;
  if(!k)return settings_page==SETTINGS_REVERB?((wm.fx_on&4u)?wm.reverb_type+1u:0u):(wm.fx_on&effect)!=0;
  if(k==1u)return settings_page==1u?wm.chorus_mix:settings_page==2u?wm.delay_mix:wm.reverb_mix;
  if(settings_page==1u)return k==2u?wm.chorus_rate:wm.chorus_depth;
  return settings_page==2u?(k==2u?wm.delay_div:wm.delay_feedback):(k==2u?wm.reverb_size:wm.reverb_damp);
 }
 if(settings_page==4u)return k==0u?wm.root:k==1u?wm.scale:k==2u?wm.loop_steps:wm.keyboard_scale;
 if(settings_page==SETTINGS_OUTPUT)return k==0u?vol:k==1u?project_slot:k==2u?metronome_on:click_level;
 if(settings_page==6u)return k==0u?wm.arp_on:k==1u?wm.arp_mode:k==2u?wm.arp_rate:wm.arp_octaves;
 if(settings_page==SETTINGS_ACCESS_FEEL)return k==0u?display_hold:k==1u?(cues_enabled?(cue_detail?2u:1u):0u):k==2u?cue_level:zoom_steady;
 return k==0u?wm.arp_gate:k==1u?wm.arp_swing:wm.arp_latch;
}
static const char *settings_value(uint32_t k,char *buf)
{
 uint32_t v=settings_get(k);
 if(settings_page==SETTINGS_ACCESS_FEEL){
  if(k==0u){static const char * const names[4]={"2 S","5 S","10 S","HOLD"};return names[v];}
  if(k==1u){static const char * const names[3]={"OFF","ACTIONS","FULL"};return names[v];}
  if(k==2u){number(percent(v),buf);uint32_t n=0;while(buf[n])n++;buf[n++]='%';buf[n]=0;return buf;}
 }
 if(settings_page==0u){
  if(k==0u)return WM_THEMES[v].name;
  if(k==2u)return v?"REDUCED":"FULL";
 }
 if(settings_page==SETTINGS_SYSTEM){
  if(k<2u){number(v>100u?100u:v,buf);uint32_t n=0;while(buf[n])n++;buf[n++]='%';buf[n]=0;return buf;}
  if(k==2u){static const char *const names[3]={"READY","LIGHTER","LIGHTEST"};return names[v<3u?v:2u];}
  return "PREPARE";
 }
 if(settings_page==SETTINGS_OUTPUT&&k==2u){static const char *const names[3]={"OFF","REC","ALWAYS"};return names[v<3u?v:0u];}
 if(settings_page==SETTINGS_OUTPUT&&k==3u){static const char *const names[4]={"25%","50%","75%","100%"};return names[v&3u];}
 if(settings_page==SETTINGS_REVERB&&!k){
  static const char *const names[RV_COUNT+1]={"OFF","ROOM","HALL","PLATE","SPRING","CAVERN","DEEP SPACE","AMBIENT"};
  return names[v];
 }
 if(settings_page==SETTINGS_LAYER){
  if(!k)return v?"LAYER":"MEMORY";
  if(k==1u)return v?"PLAY":"MUTE";
  return v?"INDEPENDENT":"FOLLOW MEMORY";
 }
 if(settings_page==SETTINGS_MIDI){
  static const char *const inputs[]={"USB + TRS","USB","TRS"};
  static const char *const clocks[]={"INTERNAL","USB","TRS"};
  if(!k)return inputs[v];
  if(k==1u&&!v)return "OMNI";
  if(k==2u)return clocks[v];
 }
 if(settings_bool(k))return v?"ON":"OFF";
 if(settings_page==9u&&k==3u)return number(FS/FX_HOLD[v],buf);
 if(settings_page==10u&&k==2u)return number(fx_tape_ms(),buf);
 if(settings_page==11u&&k==2u){const char *names[3]={"SINGLE","DOUBLE","TRIPLE"};return names[v];}
 if(settings_page==11u&&k==3u){const char *names[4]={"FREE","1/8","3/16","1/4"};return names[v];}
 if(settings_page==2u&&k==2u){static const char * const n[4]={"1/16","1/8","3/16","1/4"};return n[v];}
 if(settings_page==4u){
  if(!k){static const char * const n[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};return n[v];}
  if(k==1u){static const char * const n[8]={"FREE","MAJOR","MINOR","DORIAN","MIN PENT","MAJ PENT","MIXOLYD","HARM MIN"};return n[v];}
 }
 if(settings_page==6u){if(k==1u)return ARP_MODES[v];if(k==2u)return ARP_RATES[v];}
 return number(settings_page==SETTINGS_OUTPUT&&k==1u?v+1u:v,buf);
}
static void settings_adjust(uint32_t k,int32_t d)
{
 if(k>=settings_count()||!d)return;
 settings_index=(uint8_t)k;
 if(settings_page!=SETTINGS_LAYER&&settings_page!=0u&&settings_page!=SETTINGS_OUTPUT&&settings_page!=SETTINGS_SYSTEM&&settings_page!=SETTINGS_ACCESS_FEEL&&!(settings_page==SETTINGS_MUSIC&&k==2u))wm_mark();
 if(settings_page==10u&&k==2u&&wm.tape_sync){uint32_t ms=fx_tape_ms();wm.tape_time=(uint8_t)((ms-40u)*127u/960u);wm.tape_sync=0;}
 uint32_t old=settings_get(k);int32_t value=(int32_t)old+d;
 if(settings_bool(k))value=d>0?1:0;
 volatile uint8_t *p=0;int32_t lo=0,hi=127;
 if(settings_page==SETTINGS_MIDI){
  if(k==3u)wm.midi_ignore_program=(uint8_t)!value;
  else{
   hi=k==1u?16:2;
   p=k==0u?&wm.midi_input:k==1u?&wm.midi_channel:&wm.arp_clock;
  }
 }else if(settings_page==SETTINGS_LAYER){
  hi=1;
  if(!k)layer_target_locked((uint8_t)value);
  else if(k==1u)layer_playback_locked((uint8_t)value);
  else if(old!=(uint32_t)value){
   wm_mark();
   if(value)ws_layer_unlink();
   else wm.layer_link=1;
  }
 }else if(settings_page==SETTINGS_SYSTEM){
  /* Read-outs: turning CPU PEAK clears it; OFFICIAL FW asks before erasing. */
  if(k==1u)cpu_peak=cpu_load;
  if(k==3u&&d>0){confirm=6;focus=notice=0;cue(47u);}
  return;
 }else if(settings_page>=9u){
  p=settings_fx_param(k);
  if(!p){uint8_t bit=settings_fx_bit();if(value)wm.fx_new_on|=bit;else wm.fx_new_on&=(uint8_t)~bit;hi=1;}
  if(settings_page==9u&&k==2u){lo=1;hi=16;}
  if(settings_page==9u&&k==3u)hi=15;
  if(settings_page==11u&&k>=2u)hi=k==2u?2:3;
 }
 else if(settings_page==0u){p=k==0u?&color_theme:k==1u?&wm.extra_large:&wm.reduced_motion;hi=k==0u?THEME_COUNT-1u:1u;}
 else if(settings_page>=1u&&settings_page<=3u){
  if(settings_page==SETTINGS_REVERB&&!k){
   hi=RV_COUNT;value=ws_clamp(value,0,hi);
   if(value){wm.reverb_type=(uint8_t)(value-1);wm.fx_on|=4u;}
   else wm.fx_on&=(uint8_t)~4u;
  }
  else if(!k){uint8_t bit=(uint8_t)(1u<<(settings_page-1u));if(value)wm.fx_on|=bit;else wm.fx_on&=(uint8_t)~bit;}
  else if(k==1u)p=settings_page==1u?&wm.chorus_mix:settings_page==2u?&wm.delay_mix:&wm.reverb_mix;
  else if(settings_page==1u)p=k==2u?&wm.chorus_rate:&wm.chorus_depth;
  else if(settings_page==2u){p=k==2u?&wm.delay_div:&wm.delay_feedback;if(k==2u)hi=3;}
  else p=k==2u?&wm.reverb_size:&wm.reverb_damp;
 }
 else if(settings_page==4u){
  if(k==2u)length_adjust_locked(d);
  else {p=k==0u?&wm.root:k==1u?&wm.scale:&wm.keyboard_scale;hi=k==0u?11:k==1u?7:1;}
 }
 else if(settings_page==SETTINGS_OUTPUT){p=k==0u?&vol:k==1u?&project_slot:k==2u?&metronome_on:&click_level;hi=k==0u?127:k==1u?NSLOT-1u:k==2u?2u:3u;}
 else if(settings_page==6u){p=k==0u?&wm.arp_on:k==1u?&wm.arp_mode:k==2u?&wm.arp_rate:&wm.arp_octaves;lo=k==3u?1:0;hi=k==0u?1:k==1u?WM_ARP_MODES-1:k==2u?WM_ARP_RATES-1:4;}
 else if(settings_page==7u){p=k==0u?&wm.arp_gate:k==1u?&wm.arp_swing:&wm.arp_latch;lo=k==0u?5:0;hi=k==0u?100:k==1u?50:1;}
 else{
  if(k==0u){p=&display_hold;hi=3;}
  else if(k==1u){hi=2;uint32_t mode=(uint32_t)ws_clamp(value,0,2);cues_enabled=mode!=0u;if(mode)cue_detail=mode==2u;}
  else if(k==2u)p=&cue_level;
  else{p=&zoom_steady;hi=1;}
 }
 if(p)*p=(uint8_t)ws_clamp(value,lo,hi);
 if(settings_page==0u&&k==0u)wm.light=color_theme==THEME_LIGHT;
 if(settings_page==6u||settings_page==7u||settings_page==SETTINGS_MIDI)arp_update_config();
 if(settings_page==SETTINGS_MIDI&&k<2u&&old!=settings_get(k)){
  uint32_t prior_count=arp_count;
  for(uint32_t source=1;source<=2;source++)for(uint32_t ch=0;ch<16;ch++){
   if(!midi_route_allows(source,ch+1u)){
    uint32_t owner=2u+(source-1u)*16u+ch;
    sustain_pedal[owner-2u]=0;arp_forget_owner(owner);
    for(uint32_t v=0;v<NV;v++)if(voice[v].live==owner)voice[v].gate=0,voice[v].sustained=0;
    for(uint32_t c=0;c<CAPTURE_COUNT;c++)if(capture[c].active&&capture[c].owner==owner)capture_finish(c);
   }
  }
  if(arp_count!=prior_count){arp_release();arp_until=0;}
 }
 uint32_t now=settings_get(k);
 if(!cues_enabled||!cue_level)cue_stop();
 else if(old!=now&&!(settings_page==SETTINGS_LAYER&&k<2u)){
  if(settings_bool(k)||(settings_page==SETTINGS_REVERB&&!k&&(!old||!now)))cue_toggle((uint8_t)(now!=0));
  else if(settings_page==SETTINGS_ACCESS_FEEL&&k==2u)cue(72u);
  else if(settings_page==SETTINGS_ACCESS_FEEL&&k==1u)cue_toggle(1);
  else cue_boundary(old,now,(uint32_t)lo,(uint32_t)hi);
 }
 page_dirty=1;
}
static void settings_press(void)
{
 if(settings_page==SETTINGS_REVERB&&!settings_index){
  int32_t selected=(int32_t)wm.reverb_type+1;
  settings_adjust(0,(wm.fx_on&4u)?-selected:selected);
 }else if(settings_page==SETTINGS_SYSTEM){
  if(settings_index==3u)settings_adjust(3,1);else if(settings_index==1u)settings_adjust(1,1);
 }else if(settings_page==SETTINGS_OUTPUT&&settings_index>=2u){
  /* SELECT steps through OFF, REC, ALWAYS (and the four click levels), then wraps. */
  uint32_t top=settings_index==2u?2u:3u,v=settings_get(settings_index);
  settings_adjust(settings_index,v>=top?-(int32_t)top:1);
 }else settings_adjust(settings_index,settings_bool(settings_index)&&settings_get(settings_index)?-1:1);
}
static void screen_parameters(uint16_t fg,uint16_t pa,uint16_t guide)
{
 /* Page groups: access sky, effects coral, music/arp/MIDI gold, Layer gold, system teal. */
 const wm_theme_t *tp=theme_palette();
 uint16_t accent=settings_page==0u||settings_page==SETTINGS_ACCESS_FEEL?tp->sky:
    (settings_page>=1u&&settings_page<=3u)||(settings_page>=9u&&settings_page<=13u)?tp->coral:
    settings_page==SETTINGS_SYSTEM?tp->anchor:tp->layer;
 aa_text(10,7,&FONT_T,SETTINGS_TITLES[settings_page],fg);transport_symbol(209,6,fg);section_rule(accent,guide);
 uint32_t count=settings_count();
 if(wm.extra_large){
  uint32_t k=settings_index;char buf[12];const char *v=settings_value(k,buf);
  if(settings_page==0u){
   zoom_text(12,42,216,40,34,SETTINGS_LABELS[0][k],0,fg);
   zoom_text(12,88,216,88,104,v,1,fg);
  }else{
   uint8_t digits=settings_page==9u&&k==3u?5u:(settings_page==SETTINGS_OUTPUT&&k==1u)||(settings_page==6u&&k==3u)?1u:
    (settings_page==4u&&k==2u)||(settings_page==7u&&k==1u)?2u:3u;
   panel(8,39,224,164,pa);
   zoom_parameter_range(SETTINGS_LABELS[settings_page][k],v,"",digits,0,fg);
  }
  int32_t y=settings_page==0u?185:208;
  knob_tabs(count,k,y,fg,pa,guide);
  if(settings_page==0u&&settings_index==1u)text_center(219,&FONT_T,"HOLD OCT UP",fg);
 }else{
  for(uint32_t k=0;k<count;k++){
   char buf[12];const char *value=settings_value(k,buf);
   control_row(k,37+(int32_t)k*50,SETTINGS_LABELS[settings_page][k],value,"",value==buf,
       k==settings_index,fg,pa,guide,accent);
  }
  if(settings_page==0u){
   static const char * const help[3]={"SCREEN COLORS","LARGER TEXT","LESS MOVEMENT"};
   text_center(191,&FONT_T,help[settings_index],guide);
   if(settings_index==1u)text_center(215,&FONT_T,"HOLD OCT UP",fg);
  }
  uint32_t position=settings_position();
  for(uint32_t i=0;i<SETTINGS_PAGES;i++)cv_rect(12+(int32_t)(i*216u/SETTINGS_PAGES),237,216u/SETTINGS_PAGES-4u,position==i?3:1,position==i?fg:guide);
 }
}
