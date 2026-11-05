/* SPDX-License-Identifier: GPL-3.0-only
 * Bounded held-note arpeggiator. Input ownership, sounding voices and recording
 * ownership are distinct. The ISR splits at every onset and gate boundary.
 */
#define ARP_HELD 64u
#define ARP_OWNER 34u
static struct {uint8_t note,velocity,owner,held;} arp_keys[ARP_HELD];
static uint32_t arp_count,arp_cursor,arp_until,arp_gate_left,arp_rng=0x615af4e7u;
static uint32_t arp_note=128u,arp_tick_serial;
static uint8_t arp_configured[8],arp_clock_seen;
static uint32_t arp_clock_last,arp_clock_quarter,arp_clock_frame;
static const char * const ARP_MODES[WM_ARP_MODES]={"UP","DOWN","UP DOWN","DOWN UP","UP UP","DOWN DOWN","PLAYED","RANDOM","CONVERGE","DIVERGE","UP REPEAT","DOWN REPEAT","CHORD"};
static const char * const ARP_RATES[WM_ARP_RATES]={"1/4","1/4 DOT","1/4 TRI","1/8","1/8 DOT","1/8 TRI","1/16","1/16 DOT","1/16 TRI","1/32","1/32 DOT","1/32 TRI"};
static void arp_release(void)
{
 for(uint32_t v=0;v<NV;v++)if(voice[v].live==ARP_OWNER)voice[v].gate=0;
 for(uint32_t c=0;c<CAPTURE_COUNT;c++)if(capture[c].active&&capture[c].owner==0u)capture_finish(c);
 arp_note=128u;arp_gate_left=0;
}
static void arp_reset(void)
{
 arp_release();arp_count=arp_cursor=arp_until=arp_tick_serial=0;
 memset(arp_keys,0,sizeof arp_keys);
}
static void arp_remove(uint32_t i)
{if(i<arp_count){for(uint32_t j=i+1u;j<arp_count;j++)arp_keys[j-1u]=arp_keys[j];arp_count--;}}
static void arp_forget_owner(uint32_t owner)
{
 for(uint32_t i=arp_count;i>0;i--)if(arp_keys[i-1u].owner==owner)arp_remove(i-1u);
 if(!arp_count){arp_release();arp_cursor=arp_until=0;}
}
static void arp_update_config(void);
static void performance_on(uint32_t note,uint32_t velocity,uint32_t owner)
{
 arp_update_config();
 if(!wm.arp_on){note_on(note,velocity,owner);return;}
 if(note>127u||!velocity||velocity>127u||owner<1u||owner>33u)return;
 /* A new chord replaces the old latch once every physical key has been released. */
 uint8_t any=0;for(uint32_t i=0;i<arp_count;i++)if(arp_keys[i].held==1u)any=1;
 if(wm.arp_latch&&!any){arp_count=0;arp_cursor=0;}
 for(uint32_t i=0;i<arp_count;i++)if(arp_keys[i].note==note&&arp_keys[i].owner==owner){arp_keys[i].held=1;arp_keys[i].velocity=(uint8_t)velocity;return;}
 if(arp_count==ARP_HELD)arp_remove(0);
 arp_keys[arp_count].note=(uint8_t)note;arp_keys[arp_count].velocity=(uint8_t)velocity;
 arp_keys[arp_count].owner=(uint8_t)owner;arp_keys[arp_count].held=1;arp_count++;
 if(arp_count==1u)arp_until=arp_cursor=arp_tick_serial=0;
}
static void performance_off(uint32_t note,uint32_t owner,uint8_t sustain)
{
 if(!wm.arp_on){note_off(note,owner);return;}
 for(uint32_t i=arp_count;i>0;i--)if(arp_keys[i-1u].note==note&&arp_keys[i-1u].owner==owner){
  if(wm.arp_latch||sustain)arp_keys[i-1u].held=sustain?2u:0u;else arp_remove(i-1u);
 }
 if(!arp_count){arp_release();arp_until=arp_cursor=0;}
}
static void arp_pedal_up(uint32_t owner)
{
 for(uint32_t i=arp_count;i>0;i--)if(arp_keys[i-1u].owner==owner&&arp_keys[i-1u].held==2u){
  if(wm.arp_latch)arp_keys[i-1u].held=0;else arp_remove(i-1u);
 }
 if(!arp_count)arp_release();
}
static void arp_update_config(void)
{
 const uint8_t *config=&wm.arp_on;uint8_t changed=0;
 for(uint32_t i=0;i<8u;i++)if(config[i]!=arp_configured[i])changed=1;
 if(!changed)return;
 uint8_t was_on=arp_configured[0];
 arp_release();arp_cursor=arp_until=arp_tick_serial=0;
 if(!wm.arp_on||!was_on){
  arp_count=0;
  /* Enabling never leaves an old direct voice hanging. */
  for(uint32_t v=0;v<NV;v++)if(voice[v].live)voice[v].gate=0;
  capture_finish_all();
 }
 if(!wm.arp_latch)for(uint32_t i=arp_count;i>0;i--)if(!arp_keys[i-1u].held)arp_remove(i-1u);
 if(config[7]!=arp_configured[7]){arp_clock_seen=0;arp_clock_quarter=0;}
 memcpy(arp_configured,config,8);
}
static uint32_t arp_period(void)
{
 uint32_t q=wm.arp_clock?arp_clock_quarter:step_samples*4u;
 if(!q)q=step_samples*4u;
 uint32_t div=1u<<(wm.arp_rate/3u),n=q/div;
 if(wm.arp_rate%3u==1u)n=n*3u/2u;else if(wm.arp_rate%3u==2u)n=n*2u/3u;
 /* Pairwise swing changes neither the length of a pair nor transport tempo. */
 uint32_t shift=n*wm.arp_swing/100u;
 n=arp_tick_serial&1u?n-shift:n+shift;
 return n<64u?64u:n;
}
static uint32_t arp_index(uint32_t n)
{
 uint32_t c=arp_cursor,k=c%n;
 switch(wm.arp_mode){
 case 1:return n-1u-k;
 case 2:case 3:if(n>1u){uint32_t t=c%(2u*n-2u);k=t<n?t:2u*n-2u-t;}return wm.arp_mode==3?n-1u-k:k;
 case 4:return (c/2u)%n;
 case 5:return n-1u-(c/2u)%n;
 case 7:arp_rng^=arp_rng<<13;arp_rng^=arp_rng>>17;arp_rng^=arp_rng<<5;return arp_rng%n;
 case 8:return k&1u?n-1u-k/2u:k/2u;
 case 9:return k&1u?(n-1u)/2u+(k+1u)/2u:(n-1u)/2u-k/2u;
 case 10:case 11:{uint32_t t=c%(2u*n);k=t<n?t:2u*n-1u-t;return wm.arp_mode==11?n-1u-k:k;}
 default:return k;
 }
}
static void arp_emit(uint32_t note,uint32_t velocity,uint32_t gate)
{
 uint32_t v=note_on(note,velocity,ARP_OWNER);
 if(v<NV)voice[v].remain=gate;
 arp_note=note;
 if(recording&&!wp_active&&!anchor_view&&!confirm&&!settings_open)capture_on(note,velocity,0u);
}
static void arp_tick(void)
{
 if(!wm.arp_on||!arp_count||arp_until)return;
 if(wm.arp_clock&&!arp_clock_seen)return;
 arp_release();
 uint8_t notes[ARP_HELD],vels[ARP_HELD];uint32_t count=0;
 for(uint32_t i=0;i<arp_count;i++){
  uint8_t note=arp_keys[i].note;uint32_t at=count;
  if(wm.arp_mode!=6u)for(at=0;at<count&&notes[at]<note;at++);
  uint8_t exists=0;for(uint32_t k=0;k<count;k++)if(notes[k]==note){exists=1;break;}
  if(exists)continue;
  for(uint32_t k=count;k>at;k--){notes[k]=notes[k-1u];vels[k]=vels[k-1u];}
  notes[at]=note;vels[at]=arp_keys[i].velocity;count++;
 }
 if(!count)return;
 uint32_t n=count*wm.arp_octaves,index=arp_index(n),period=arp_period();
 arp_gate_left=period*wm.arp_gate/100u;arp_until=period;
 if(wm.arp_mode==12u){
  uint32_t oct=(arp_cursor/count)%wm.arp_octaves;
  for(uint32_t k=0;k<count&&k<NV;k++)arp_emit(notes[k]+oct*12u>127u?notes[k]:notes[k]+oct*12u,vels[k],arp_gate_left);
 }else{
  uint32_t note=notes[index%count]+12u*(index/count);
  /* Fold at the boundary instead of accumulating repeated clipped MIDI 127s. */
  while(note>127u)note-=12u;
  arp_emit(note,vels[index%count],arp_gate_left);
 }
 arp_cursor++;arp_tick_serial++;
}
