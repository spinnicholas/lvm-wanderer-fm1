/* SPDX-License-Identifier: GPL-3.0-only */
static uint8_t settings_bool(uint32_t k)
{
 return (settings_page==0u)||(settings_page>=1u&&settings_page<=3u&&k==0u)||
 (settings_page==4u&&k==3u)||(settings_page==5u&&k==1u)||
 (settings_page==6u&&k==0u)||(settings_page==7u&&k==2u)||(settings_page==SETTINGS_ACCESS_FEEL&&k==3u);
}
static uint32_t settings_get(uint32_t k)
{
 if(settings_page==0u)return k==0?wm.light:k==1?wm.extra_large:wm.reduced_motion;
 if(settings_page>=1u&&settings_page<=3u){
  uint32_t effect=settings_page==1u?1u:settings_page==2u?2u:4u;
  if(!k)return (wm.fx_on&effect)!=0;
  if(k==1u)return settings_page==1u?wm.chorus_mix:settings_page==2u?wm.delay_mix:wm.reverb_mix;
  return settings_page==2u?(k==2u?wm.delay_div:wm.delay_feedback):(k==2u?wm.reverb_size:wm.reverb_damp);
 }
 if(settings_page==4u)return k==0u?wm.root:k==1u?wm.scale:k==2u?wm.loop_steps:wm.keyboard_scale;
 if(settings_page==5u)return k==0u?vol:k==1u?cues_enabled:project_slot;
 if(settings_page==6u)return k==0u?wm.arp_on:k==1u?wm.arp_mode:k==2u?wm.arp_rate:wm.arp_octaves;
 if(settings_page==SETTINGS_ACCESS_FEEL)return k==0u?display_hold:k==1u?(cues_enabled?(cue_detail?2u:1u):0u):k==2u?cue_level:zoom_steady;
 return k==0u?wm.arp_gate:k==1u?wm.arp_swing:k==2u?wm.arp_latch:wm.arp_clock;
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
  if(k==0u)return v?"LIGHT":"DARK";
  if(k==2u)return v?"REDUCED":"FULL";
 }
 if(settings_bool(k))return v?"ON":"OFF";
 if(settings_page==2u&&k==2u){static const char * const n[4]={"1/16","1/8","3/16","1/4"};return n[v];}
 if(settings_page==4u){
  if(!k){static const char * const n[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};return n[v];}
  if(k==1u){static const char * const n[8]={"FREE","MAJOR","MINOR","DORIAN","MIN PENT","MAJ PENT","MIXOLYD","HARM MIN"};return n[v];}
 }
 if(settings_page==6u){if(k==1u)return ARP_MODES[v];if(k==2u)return ARP_RATES[v];}
 if(settings_page==7u&&k==3u){static const char * const n[3]={"INTERNAL","USB","TRS"};return n[v];}
 return number(settings_page==5u&&k==2u?v+1u:v,buf);
}
static void settings_adjust(uint32_t k,int32_t d)
{
 if(k>=settings_count()||!d)return;
 settings_index=(uint8_t)k;
 if(settings_page!=0u&&settings_page!=5u&&settings_page!=SETTINGS_ACCESS_FEEL)wm_mark();
 uint32_t old=settings_get(k);int32_t value=(int32_t)old+d;
 if(settings_bool(k))value=d>0?1:0;
 volatile uint8_t *p=0;int32_t lo=0,hi=127;
 if(settings_page==0u){p=k==0u?&wm.light:k==1u?&wm.extra_large:&wm.reduced_motion;hi=1;}
 else if(settings_page>=1u&&settings_page<=3u){
  if(!k){uint8_t bit=(uint8_t)(1u<<(settings_page-1u));if(value)wm.fx_on|=bit;else wm.fx_on&=(uint8_t)~bit;}
  else if(k==1u)p=settings_page==1u?&wm.chorus_mix:settings_page==2u?&wm.delay_mix:&wm.reverb_mix;
  else if(settings_page==2u){p=k==2u?&wm.delay_div:&wm.delay_feedback;if(k==2u)hi=3;}
  else p=k==2u?&wm.reverb_size:&wm.reverb_damp;
 }
 else if(settings_page==4u){
  if(k==2u){uint8_t save=settings_index;settings_index=2;music_adjust(d);settings_index=save;}
  else {p=k==0u?&wm.root:k==1u?&wm.scale:&wm.keyboard_scale;hi=k==0u?11:k==1u?7:1;}
 }
 else if(settings_page==5u){p=k==0u?&vol:k==1u?&cues_enabled:&project_slot;hi=k==0u?127:k==1u?1:7;}
 else if(settings_page==6u){p=k==0u?&wm.arp_on:k==1u?&wm.arp_mode:k==2u?&wm.arp_rate:&wm.arp_octaves;lo=k==3u?1:0;hi=k==0u?1:k==1u?WM_ARP_MODES-1:k==2u?WM_ARP_RATES-1:4;}
 else if(settings_page==7u){p=k==0u?&wm.arp_gate:k==1u?&wm.arp_swing:k==2u?&wm.arp_latch:&wm.arp_clock;lo=k==0u?5:0;hi=k==0u?100:k==1u?50:k==2u?1:2;}
 else{
  if(k==0u){p=&display_hold;hi=3;}
  else if(k==1u){hi=2;uint32_t mode=(uint32_t)ws_clamp(value,0,2);cues_enabled=mode!=0u;if(mode)cue_detail=mode==2u;}
  else if(k==2u)p=&cue_level;
  else{p=&zoom_steady;hi=1;}
 }
 if(p)*p=(uint8_t)ws_clamp(value,lo,hi);
 if(settings_page==6u||settings_page==7u)arp_update_config();
 uint32_t now=settings_get(k);
 if(!cues_enabled||!cue_level)cue_stop();
 else if(old!=now){
  if(settings_bool(k))cue_toggle((uint8_t)(now!=0));
  else if(settings_page==SETTINGS_ACCESS_FEEL&&k==2u)cue(72u);
  else if(settings_page==SETTINGS_ACCESS_FEEL&&k==1u)cue_toggle(1);
  else cue_boundary(old,now,(uint32_t)lo,(uint32_t)hi);
 }
 settings_detail=settings_detail_kind=0;page_dirty=1;
}
static void settings_press(void)
{settings_adjust(settings_index,settings_bool(settings_index)&&settings_get(settings_index)?-1:1);}
static void screen_parameters(uint16_t fg,uint16_t pa,uint16_t guide)
{
 aa_text(10,7,&FONT_T,SETTINGS_TITLES[settings_page],fg);transport_symbol(209,6,fg);cv_rect(12,33,216,2,guide);
 uint32_t count=settings_count();
 if(wm.extra_large){
  uint32_t k=settings_index;char buf[12];const char *v=settings_value(k,buf);
  if(settings_page==0u){
   zoom_text(12,42,216,40,34,SETTINGS_LABELS[0][k],0,fg);
   zoom_text(12,88,216,88,104,v,1,fg);
  }else{
   uint8_t digits=(settings_page==5u&&k==2u)||(settings_page==6u&&k==3u)?1u:
    (settings_page==4u&&k==2u)||(settings_page==7u&&k==1u)?2u:3u;
   zoom_parameter_range(SETTINGS_LABELS[settings_page][k],v,"",digits,0,fg);
  }
  int32_t y=settings_page==0u?185:208;
  for(uint32_t j=0;j<count;j++){int32_t x=18+(int32_t)j*55;card(x,y,40,30,pa,j==k?fg:guide,j==k);char b[12];aa_text(x+12,y+6,&FONT_T,number(j+1u,b),fg);}
  if(settings_page==0u&&settings_index==1u)text_center(219,&FONT_T,"ZOOM HOME+EDIT",fg);
 }else{
  for(uint32_t k=0;k<count;k++){
   /* Two full-size lines need clearance from the three-pixel selection frame. */
   int32_t y=38+(int32_t)k*49;card(8,y,224,48,pa,k==settings_index?fg:guide,k==settings_index);
   char buf[12],n[12];aa_text(16,y+15,&FONT_T,number(k+1u,n),guide);
   aa_text(40,y+5,&FONT_T,SETTINGS_LABELS[settings_page][k],guide);
   const char *v=settings_value(k,buf);aa_text(220-width(&FONT_T,v),y+25,&FONT_T,v,fg);
  }
  if(settings_page==0u){
   static const char * const help[3]={"SCREEN COLORS","LARGER TEXT","LESS MOVEMENT"};
   text_center(191,&FONT_T,help[settings_index],guide);
   if(settings_index==1u)text_center(215,&FONT_T,"ZOOM HOME+EDIT",fg);
  }
  for(uint32_t i=0;i<SETTINGS_PAGES;i++)cv_rect(19+(int32_t)i*23,237,17,settings_page==i?3:1,settings_page==i?fg:guide);
 }
}
