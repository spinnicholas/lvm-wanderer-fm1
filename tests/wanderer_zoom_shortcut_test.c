/* Global display shortcuts must not dispatch HOME/EDIT or touch performance. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
static uint32_t edges(uint32_t held,uint32_t pressed,uint32_t released)
{
 fm1_in.buttons=held;
 uint32_t b=ui_edges(pressed,released);b=save_edges(b,released);return detail_edges(b,released);
}
static void clear_input(void)
{
 ui_pending=ui_chord_latched=zoom_chord_latched=0;
 home_down=sel_down=edit_down=save_down=0;
 edit_pending=rec_pending=save_pending=sel_held_action=0;
 fm1_in.buttons=0;fm1_ms=100;
}
int main(void)
{
 const uint32_t home=1u<<BTN_HOME,edit=1u<<BTN_EDIT,sel=1u<<BTN_SEL;
 uint32_t cases=0;cues_enabled=0;
 for(uint32_t context=0;context<8;context++)for(uint32_t zoom=0;zoom<2;zoom++)for(uint32_t order=0;order<3;order++){
  clear_input();wm=(wm_state_t)WM_DEFAULTS;wm.extra_large=zoom;wm.layer_enabled=1;
  settings_open=context==1;settings_page=7;settings_index=2;
  editor_open=context==2||context==3;editor_page=context==3?EDIT_PRINT:EDIT_SCATTER;editor_selected=3;
  scope_open=context==4;confirm=context==5?2:0;notice=context==6?N_SAVE_FAILED:0;
  focus=context==7?1:0;focus_until=9000;running=recording=1;octave=1;
  wm_state_t expected=wm;expected.extra_large=!zoom;
  if(order==0)assert(!edges(home|edit,home|edit,0));
  else {
   uint32_t first=order==1?home:edit;
   assert(!edges(first,first,0));fm1_ms+=50;
   assert(!edges(home|edit,(home|edit)^first,0));
  }
  assert(wm.extra_large==!zoom&&zoom_chord_latched);
  fm1_ms+=3000;assert(!edges(home|edit,0,0));
  assert(!home_down&&!edit_pending&&!sel_down);
  assert(!edges(edit,0,home));
  assert(!edges(home|edit,home,0)); /* Partial release/repress cannot toggle again. */
  assert(wm.extra_large==!zoom);
  assert(!edges(home,0,edit));assert(!edges(0,0,home));assert(!zoom_chord_latched);
  assert(!memcmp(&wm,&expected,sizeof wm));assert(running&&recording&&octave==1);
  assert(settings_open==(context==1)&&settings_page==7&&settings_index==2);
  assert(editor_open==(context==2||context==3)&&editor_selected==3);
  assert(scope_open==(context==4)&&confirm==(context==5?2:0));
  assert(notice==(context==6?N_SAVE_FAILED:0)&&focus==(context==7?1:0));
  assert(!edges(home|edit,home|edit,0));assert(wm.extra_large==zoom);
  assert(!edges(0,0,home|edit));cases++;
 }
 clear_input();scope_open=0;
 assert(!edges(home|sel,home|sel,0));assert(scope_open&&ui_chord_latched);
 uint8_t zoom=wm.extra_large;
 assert(!edges(home|sel|edit,edit,0));assert(wm.extra_large==zoom);
 assert(!edges(0,0,home|sel|edit));
 clear_input();scope_open=0;
 assert(!edges(home|edit,home|edit,0));
 assert(!edges(home|edit|sel,sel,0));assert(!scope_open);
 assert(!edges(sel,0,home|edit));assert(zoom_chord_latched);
 fm1_ms+=2000;assert(!edges(sel,0,0));assert(!sel_down);
 assert(!edges(0,0,sel));assert(!zoom_chord_latched);
 clear_input();assert(!edges(home,home,0));assert(edges(0,0,home)==home);
 clear_input();assert(!edges(edit,edit,0));assert(edges(0,0,edit)==edit);
 clear_input();rec_pending=1;rec_down=101;
 assert(!edges(home|edit,home|edit,0));assert(rec_pending&&rec_down==101);
 puts("48 context/state/order combinations: single-toggle latch, partial releases, active recording/layers, exact state preservation, Scope conflicts, solo keys and unrelated REC pending pass");
}
