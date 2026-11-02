#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
#include <limits.h>
int main(void)
{
 for(uint32_t mode=0;mode<3;mode++){
  display_hold=mode;fm1_ms=100;focus_show(7,1);
  uint32_t duration=mode==0?2000:mode==1?5000:10000;
  assert(focus_until==100+duration);fm1_ms+=duration-1;display_expire();assert(focus==7);
  uint32_t refreshed=fm1_ms;focus_show(7,2);assert(focus_until==refreshed+duration);
  fm1_ms=refreshed+duration-1;display_expire();assert(focus==7);
  fm1_ms++;display_expire();assert(!focus);
  fm1_ms=500;show_notice(N_ANCHORED);assert(notice_until==500+duration);
  fm1_ms+=duration;display_expire();assert(!notice);
  fm1_ms=UINT32_MAX-999;focus_show(1,127);fm1_ms+=duration-1;display_expire();assert(focus==1);
  fm1_ms++;display_expire();assert(!focus);
 }
 display_hold=0;fm1_ms=UINT32_MAX-1999;focus_show(1,127);assert(focus_until==1);
 fm1_ms=0;display_expire();assert(focus);fm1_ms=1;display_expire();assert(!focus);
 display_hold=3;fm1_ms=100;focus_show(7,50);assert(!focus_until);
 fm1_ms+=1000000;display_expire();assert(focus==7);show_notice(N_ANCHORED);
 fm1_ms+=1000000;display_expire();assert(notice==N_ANCHORED&&!notice_until);
 for(uint32_t mode=0;mode<4;mode++){
  display_hold=mode;show_notice(N_SAVE_FAILED);assert(!notice_until);fm1_ms+=20000;display_expire();assert(notice==N_SAVE_FAILED);
  show_notice(N_SAVED);assert(!notice_until);fm1_ms+=20000;display_expire();assert(notice==N_SAVED);
 }
 puts("2/5/10 seconds, refresh, HOLD, persistent critical/results notices and uint32 rollover pass");
 _Static_assert(sizeof(wm_preferences_t)==16,"existing preference size must stay compatible");
 wm_preferences_t old={WM_MAGIC,1,sizeof old,1,1,1,37,0,3,{0,0}};
 assert(!settings_save(&old,sizeof old));display_hold=3;cue_detail=zoom_steady=0;cue_level=127;
 preferences_load();assert(wm.light&&wm.extra_large&&wm.reduced_motion&&vol==37&&!cues_enabled&&project_slot==3);
 assert(!display_hold&&cue_detail&&zoom_steady&&cue_level==64);
 for(uint32_t hold=0;hold<4;hold++)for(uint32_t detail=0;detail<2;detail++)for(uint32_t steady=0;steady<2;steady++)for(uint32_t level=0;level<128;level+=127){
  display_hold=hold;cue_detail=detail;zoom_steady=steady;cue_level=level;cues_enabled=1;
  assert(!preferences_save());display_hold=0;cue_detail=0;zoom_steady=0;cue_level=1;preferences_load();
  assert(display_hold==hold&&cue_detail==detail&&zoom_steady==steady&&cue_level==level);
 }
 old.reserved[0]=32;old.reserved[1]=128;assert(!preferences_valid(&old));
 old.reserved[0]=0;old.reserved[1]=1;assert(!preferences_valid(&old));
 uint16_t sig=access_signature();assert(!settings_save(&old,sizeof old));preferences_load();assert(access_signature()==sig);
 host_fault_settings_save=1;assert(preferences_save());
 puts("legacy defaults, all extension bits, cue-level extremes, unchanged 16-byte size, malformed records and failed writes pass");
 settings_page=SETTINGS_ACCESS_FEEL;project_initialise();uint32_t writes=host_store_writes;
 uint8_t prior=display_hold;settings_adjust(0,prior==3?-1:1);project_poll();assert(project_dirty&&host_store_writes==writes);
 settings_adjust(0,prior==3?1:-1);project_poll();assert(!project_dirty);
 assert(host_store_writes==writes);puts("unsaved indicator detects preference changes/reverts; turning controls never writes flash");
}
