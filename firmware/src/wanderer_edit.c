/* SPDX-License-Identifier: GPL-3.0-only
 * Four knobs, one parameter each; identical mapping in normal and Zoom views.
 */
#define EDIT_PRINT 13u
#define EDIT_SCATTER 14u
#define EDIT_PATTERN 15u
#define EDIT_COLOR 16u
#define EDIT_PAGES 17u
static uint8_t editor_open,editor_page,editor_selected;
static uint32_t editor_count(void){return 4u;}
static uint32_t editor_mark_ms,edit_down,rec_down,sel_down;
static uint8_t edit_pending,rec_pending;
static const char * const EDIT_TITLES[EDIT_PAGES]={"AMP","FILTER","FILTER ENV","OSCILLATORS","TIMBRE","MODULATION","OP 1 ENV","OP 2 ENV","OP 3 ENV","OP 4 ENV","FM RATIOS","FM LEVELS","ROUTING","PRINT","SCATTER","SCATTER GRID","COLOR"};
static const char * const WS_ENGINE_NAMES[WS_ENGINES]={"ANALOG","FM","PHASE","STRING","MODAL","FM6","SUPER","VOICE"};
static const char * const EDIT_LABELS[EDIT_PAGES][4]={
 {"ATTACK","DECAY","SUSTAIN","RELEASE"},{"CUTOFF","RESONANCE","TYPE","ENV DEPTH"},{"ATTACK","DECAY","SUSTAIN","RELEASE"},
 {"ENGINE","WAVE","RATIO","DETUNE"},{"SHAPE","DAMPING","POSITION","DRIVE"},{"RATE","DEPTH","TARGET","SUB"},
 {"ATTACK","DECAY","SUSTAIN","RELEASE"},{"ATTACK","DECAY","SUSTAIN","RELEASE"},{"ATTACK","DECAY","SUSTAIN","RELEASE"},{"ATTACK","DECAY","SUSTAIN","RELEASE"},
 {"OP 1","OP 2","OP 3","OP 4"},{"OP 1","OP 2","OP 3","OP 4"},{"ALGORITHM","FEEDBACK","PULSE","INHARM"},
 {"REC PRINT","ENV UNDO","LFO ANCHOR","EVENT / RAM"},{"MODE","SIZE","AMOUNT","SMOOTH"},{"GRID","VARIATION","MIX","TONE"},{"MODEL","BALANCE","COLOR","BODY"}};
static const uint8_t *editor_param(uint32_t k,const ws_patch_t *p)
{
 if(editor_page==EDIT_SCATTER){uint8_t *sc[4]={&wm.repeat_style,&wm.grain_size,&wm.decay,&wm.scatter_edge};return sc[k];}
 if(editor_page==EDIT_PATTERN){uint8_t *pat[4]={&wm.scatter_pattern,&wm.scatter_variation,&wm.scatter_mix,&wm.scatter_tone};return pat[k];}
 if(editor_page==EDIT_COLOR){const uint8_t *color[WS_ENGINES][4]={{&p->mix,&p->inharm,&p->position,&p->shape},{&p->wave,&p->mix,&p->sub,&p->position},{&p->wave,&p->algorithm,&p->mix,&p->inharm},{&p->wave,&p->mix,&p->filter,&p->drive},{&p->wave,&p->position,&p->shape,&p->inharm},{&p->detune,&p->pulse,&p->sub,&p->mix},
  {&p->position,&p->inharm,&p->pulse,&p->sub},{&p->feedback,&p->pulse,&p->sub,&p->detune}};return color[p->engine][k];}
 if(editor_page==0)return &p->amp[k];if(editor_page==2)return &p->fenv[k];
 if(editor_page>=6&&editor_page<=9)return &p->op_env[editor_page-6][k];
 if(editor_page==10)return &p->ratio[k];if(editor_page==11)return &p->op_level[k];
 if(editor_page==12&&p->engine==5){const uint8_t *row[4]={&p->algorithm,&p->feedback,&p->position,&p->inharm};return row[k];}
 const uint8_t *osc[WS_ENGINES][4]={
  {&p->engine,&p->wave,&p->ratio[1],&p->detune},{&p->engine,&p->algorithm,&p->ratio[0],&p->feedback},
  {&p->engine,&p->shape,&p->mix,&p->detune},{&p->engine,&p->shape,&p->damp,&p->position},{&p->engine,&p->inharm,&p->damp,&p->position},{&p->engine,&p->algorithm,&p->ratio[0],&p->feedback},
  {&p->engine,&p->wave,&p->detune,&p->mix},{&p->engine,&p->wave,&p->shape,&p->position}};
 const uint8_t *timbre[WS_ENGINES][4]={
  {&p->shape,&p->pulse,&p->sub,&p->drive},{&p->shape,&p->feedback,&p->op_level[0],&p->drive},
  {&p->shape,&p->detune,&p->mix,&p->drive},{&p->shape,&p->damp,&p->position,&p->drive},{&p->inharm,&p->damp,&p->op_level[3],&p->drive},{&p->shape,&p->feedback,&p->op_level[0],&p->drive},
  {&p->shape,&p->sub,&p->inharm,&p->drive},{&p->mix,&p->inharm,&p->detune,&p->drive}};
 if(editor_page==3)return osc[p->engine][k];if(editor_page==4)return timbre[p->engine][k];
 const uint8_t *row[5][4]={{&p->cutoff,&p->resonance,&p->filter,&p->envdepth},{&p->lfo_rate,&p->lfo_depth,&p->lfo_dest,&p->level},
 {&p->algorithm,&p->feedback,&p->ratio[0],&p->ratio[3]},{0,0,0,0},{&wm.repeat_style,&wm.keyboard_scale,&wm.root,&wm.scale}};
 uint32_t r=editor_page==1?0:editor_page==5?1:editor_page==12?2:editor_page==14?4:3;
 return row[r][k];
}
static const char *editor_label(uint32_t k)
{
 uint32_t engine=ws_edit_patch()->engine;
 if(engine==5&&editor_page==12&&k>=2)return k==2?"VELOCITY":"KEY SCALE";
 if(engine==5&&editor_page==EDIT_COLOR){const char *labels[4]={"RATIO 5","RATIO 6","OP 5 LVL","OP 6 LVL"};return labels[k];}
 if(engine==5&&(editor_page==3||editor_page==4))engine=1;
 if(editor_page==EDIT_COLOR){static const char * const color[WS_ENGINES][4]={{"BALANCE","NOISE","KEY TRACK","ROUND"},{"OP WAVE","PAIR MIX","TINE OCT","KEY TRACK"},{"WAVE","LINE 2","BALANCE","LINE MODE"},{"EXCITER","BODY","FILTER","DRIVE"},{"MODEL","STRIKE","HARDNESS","INHARM"},{"","","",""},
  {"KEY TRACK","AIR","PULSE","SUB"},{"FOCUS","PULSE","SUB","CHOIR"}};return color[engine][k];}
 static const char * const osc[WS_ENGINES][4]={{"ENGINE","WAVE","RATIO","DETUNE"},{"ENGINE","ALGORITHM","RATIO 1","FEEDBACK"},{"ENGINE","BEND","MIX","DETUNE"},{"ENGINE","RING","DAMPING","POSITION"},{"ENGINE","INHARM","DAMPING","POSITION"},{"","","",""},
  {"ENGINE","WAVE","SPREAD","VOICES"},{"ENGINE","SOURCE","VOWEL","FORMANT"}};
 static const char * const timbre[WS_ENGINES][4]={{"ROUND","PULSE","SUB","DRIVE"},{"INDEX","FEEDBACK","OP 1 LVL","DRIVE"},{"BEND","DETUNE","MIX","DRIVE"},{"RING","DAMPING","POSITION","DRIVE"},{"INHARM","DAMPING","PARTIAL 4","DRIVE"},{"","","",""},
  {"WIDTH","SUB","AIR","DRIVE"},{"FORMANTS","BREATH","CHOIR","DRIVE"}};
 if(editor_page==3)return osc[engine][k];if(editor_page==4)return timbre[engine][k];
 if(editor_page==5&&k==3)return "LEVEL";
 if(editor_page==12&&k>=2)return k==2?"RATIO 1":"RATIO 4";
 return EDIT_LABELS[editor_page][k];
}
static uint8_t editor_page_supported(uint8_t page,uint8_t engine)
{
 return page<6u||page>12u||engine==1u||engine==5u||(engine==4u&&page!=12u);
}
static void editor_check_page(void)
{
 if(editor_open&&!editor_page_supported(editor_page,ws_edit_patch()->engine)){
  editor_page=editor_selected=0;
  editor_mark_ms=0;
 }
}
static void editor_move(int32_t d)
{
 int32_t dir=d<0?-1:1;uint32_t count=(uint32_t)(d<0?-d:d)%EDIT_PAGES;
 uint8_t engine=ws_edit_patch()->engine;
 while(count--){do{editor_page=(uint8_t)((editor_page+EDIT_PAGES+dir)%EDIT_PAGES);}while(
  !editor_page_supported(editor_page,engine));
 }
 editor_selected=0;editor_mark_ms=0;page_dirty=1;
}
/* Materialize the selected patch only on an actual synth edit. */
static ws_patch_t *editor_ensure(void)
{
 if(layer_focus){
  if(!ws_layer_own())ws_layer_copy_memory();
  if(!wm.layer_patch_valid){wm.layer_patch=*ws_layer_patch();wm.layer_patch_valid=1;}
  return &wm.layer_patch;
 }
 if(!wm.patch_valid){wm.patch=*ws_current_patch();wm.patch_valid=1;}
 return &wm.patch;
}
/* One place for every patch parameter's range; the knob and the screen share it. */
static int32_t editor_patch_max(uint32_t k,const ws_patch_t *p,const uint8_t *value)
{
 int32_t max=127;
 if(value==&p->filter||value==&p->lfo_dest)max=value==&p->filter?WS_FILTER_COUNT-1:p->engine==3?1:2;
 if(value==&p->engine||value==&p->wave)max=value==&p->wave?(p->engine==0?9:p->engine==2?7:p->engine==3?3:p->engine==5?5:p->engine>=6?3:4):(int32_t)WS_ENGINES-1;
 if(editor_page==EDIT_COLOR&&p->engine==1&&k==2)max=3;
 if(editor_page==EDIT_COLOR&&p->engine==2&&k==3)max=1;
 if(value==&p->algorithm)max=p->engine==5?31:7;
 if(p->engine==5&&((editor_page==11)||(editor_page==4&&k==2)||(editor_page==EDIT_COLOR&&k>=2)))max=99;
 return max;
}
/* Range of row k on the current page, or -1 for rows that are actions. */
static int32_t editor_max(uint32_t k)
{
 if(editor_page==EDIT_PRINT)return -1;
 if(editor_page==EDIT_SCATTER||editor_page==EDIT_PATTERN){
  if(k==0)return editor_page==EDIT_SCATTER?(int32_t)WM_SCATTER_MODES-1:(int32_t)WM_PATTERN_COUNT-1;
  if(k==1&&editor_page==EDIT_SCATTER)return (int32_t)WM_GRAIN_COUNT-1;
  return 127;
 }
 const ws_patch_t *p=ws_edit_patch();const uint8_t *value=editor_param(k,p);
 return value?editor_patch_max(k,p,value):-1;
}
static void editor_adjust(uint32_t k,int32_t d)
{
 if(k>=editor_count())return;
 editor_selected=(uint8_t)k;
 if(editor_page==EDIT_PRINT){
  if(k==3&&wp_has_anchor){wp_active=(uint8_t)(d>0);wp_ring_count=0;wp_pos=wp_phase=0;for(uint32_t v=0;v<NV;v++)if(!voice[v].live)voice[v].gate=0;}
  page_dirty=1;return;
 }
 if(!editor_mark_ms||fm1_ms-editor_mark_ms>650u)wm_mark();editor_mark_ms=fm1_ms;
 if(editor_page==EDIT_SCATTER||editor_page==EDIT_PATTERN){
  uint8_t *value=(uint8_t*)editor_param(k,ws_edit_patch());
  uint32_t max=127;
  if(k==0)max=editor_page==EDIT_SCATTER?WM_SCATTER_MODES-1u:WM_PATTERN_COUNT-1u;
  if(k==1&&editor_page==EDIT_SCATTER)max=WM_GRAIN_COUNT-1u;
  uint32_t old=*value;*value=(uint8_t)ws_clamp((int32_t)old+d,0,(int32_t)max);
  cue_boundary(old,*value,0,max);page_dirty=1;return;
 }
 ws_patch_t *p=editor_ensure();
 if(layer_focus)wm.layer_legacy=0;else wm.legacy=0;
 uint8_t *value=(uint8_t*)editor_param(k,p);if(!value)return;
 int32_t max=editor_patch_max(k,p,value),step=max<=31?1:2;

 uint32_t old=*value;*value=(uint8_t)ws_clamp((int32_t)*value+d*step,0,max);
 if(value==&p->engine){p->algorithm%=p->engine==5?32:8;p->wave%=p->engine==0?10:p->engine==2?8:p->engine>=6?4:6;}
 if(p->engine==5){for(uint32_t j=0;j<4;j++){if(p->ratio[j]<2)p->ratio[j]=2;if(p->op_level[j]>99)p->op_level[j]=99;}
  if(p->detune<2)p->detune=2;if(p->pulse<2)p->pulse=2;if(p->mix>99)p->mix=99;if(p->sub>99)p->sub=99;}
 cue_boundary(old,*value,0,(uint32_t)max);
 uint8_t source=layer_focus?PATCH_LAYER:PATCH_MEMORY,sound=ws_edit_sound();
 for(uint32_t v=0;v<NV;v++)if(voice[v].gate&&voice[v].patch_source==source&&ws_voice[v].modern&&voice[v].patch==sound){
  if(ws_voice[v].p.engine!=p->engine){voice[v].splice=voice[v].last;voice[v].splice_left=32;ws_start(v,voice[v].note,(uint32_t)voice[v].gate/16384u);}
  else{
   ws_voice_t *s=&ws_voice[v];
   if(s->p.filter!=p->filter){
    voice[v].splice=voice[v].last;voice[v].splice_left=32;
    memset(s->lp,0,sizeof s->lp);memset(&s->ladder_state,0,sizeof s->ladder_state);
    s->ic1=s->ic2=0;s->control=0;
    s->cut_sm=p->filter>=WS_LP12?ws_lp_cut(p->cutoff):p->cutoff*137*256/127;
   }
   s->p=*p; /* Smooth control tick updates tuning and coefficients. */
  }
 }
 page_dirty=1;
}
