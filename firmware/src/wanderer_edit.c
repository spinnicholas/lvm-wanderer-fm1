/* SPDX-License-Identifier: GPL-3.0-only
 * Four knobs, one parameter each; identical mapping in normal and Zoom views.
 */
#define EDIT_PRINT 13u
#define EDIT_SCATTER 14u
#define EDIT_PAGES 15u
static uint8_t editor_open,editor_page,editor_selected;
static uint32_t editor_count(void){return 4u;}
static uint32_t editor_mark_ms,edit_down,rec_down,sel_down;
static uint8_t edit_pending,rec_pending,sel_held_action;
static const char * const EDIT_TITLES[EDIT_PAGES]={"AMP","FILTER","FILTER ENV","OSCILLATORS","TIMBRE","MODULATION","OP 1 ENV","OP 2 ENV","OP 3 ENV","OP 4 ENV","FM RATIOS","FM LEVELS","ROUTING","PRINT","SCATTER"};
static const char * const WS_ENGINE_NAMES[5]={"ANALOG","FM","PHASE","STRING","MODAL"};
static const char * const EDIT_LABELS[EDIT_PAGES][4]={
 {"ATTACK","DECAY","SUSTAIN","RELEASE"},{"CUTOFF","RESONANCE","TYPE","ENV DEPTH"},{"ATTACK","DECAY","SUSTAIN","RELEASE"},
 {"ENGINE","WAVE","RATIO","DETUNE"},{"SHAPE","DAMPING","POSITION","DRIVE"},{"RATE","DEPTH","TARGET","SUB"},
 {"ATTACK","DECAY","SUSTAIN","RELEASE"},{"ATTACK","DECAY","SUSTAIN","RELEASE"},{"ATTACK","DECAY","SUSTAIN","RELEASE"},{"ATTACK","DECAY","SUSTAIN","RELEASE"},
 {"OP 1","OP 2","OP 3","OP 4"},{"OP 1","OP 2","OP 3","OP 4"},{"ALGORITHM","FEEDBACK","PULSE","INHARM"},
 {"REC PRINT","ENV UNDO","LFO ANCHOR","EVENT / RAM"},{"MODE","SIZE","AMOUNT","SMOOTH"}};
static uint8_t *editor_param(uint32_t k)
{
 if(editor_page==EDIT_SCATTER){uint8_t *sc[4]={&wm.repeat_style,&wm.grain_size,&wm.decay,&wm.scatter_edge};return sc[k];}
 ws_patch_t *p=&wm.patch;
 if(editor_page==0)return &p->amp[k];if(editor_page==2)return &p->fenv[k];
 if(editor_page>=6&&editor_page<=9)return &p->op_env[editor_page-6][k];
 if(editor_page==10)return &p->ratio[k];if(editor_page==11)return &p->op_level[k];
 uint8_t *osc[5][4]={
  {&p->engine,&p->wave,&p->ratio[1],&p->detune},{&p->engine,&p->algorithm,&p->ratio[0],&p->feedback},
  {&p->engine,&p->shape,&p->mix,&p->detune},{&p->engine,&p->shape,&p->damp,&p->position},{&p->engine,&p->inharm,&p->damp,&p->position}};
 uint8_t *timbre[5][4]={
  {&p->shape,&p->pulse,&p->sub,&p->drive},{&p->shape,&p->feedback,&p->op_level[0],&p->drive},
  {&p->shape,&p->detune,&p->mix,&p->drive},{&p->shape,&p->damp,&p->position,&p->drive},{&p->inharm,&p->damp,&p->op_level[3],&p->drive}};
 if(editor_page==3)return osc[p->engine][k];if(editor_page==4)return timbre[p->engine][k];
 uint8_t *row[5][4]={{&p->cutoff,&p->resonance,&p->filter,&p->envdepth},{&p->lfo_rate,&p->lfo_depth,&p->lfo_dest,&p->level},
 {&p->algorithm,&p->feedback,&p->ratio[0],&p->ratio[3]},{0,0,0,0},{&wm.repeat_style,&wm.keyboard_scale,&wm.root,&wm.scale}};
 uint32_t r=editor_page==1?0:editor_page==5?1:editor_page==12?2:editor_page==14?4:3;
 return row[r][k];
}
static const char *editor_label(uint32_t k)
{
 uint32_t engine=ws_current_patch()->engine;
 static const char * const osc[5][4]={{"ENGINE","WAVE","RATIO","DETUNE"},{"ENGINE","ALGORITHM","RATIO 1","FEEDBACK"},{"ENGINE","BEND","MIX","DETUNE"},{"ENGINE","TENSION","DAMPING","POSITION"},{"ENGINE","INHARM","DAMPING","POSITION"}};
 static const char * const timbre[5][4]={{"ROUND","PULSE","SUB","DRIVE"},{"INDEX","FEEDBACK","OP 1 LVL","DRIVE"},{"BEND","DETUNE","MIX","DRIVE"},{"TENSION","DAMPING","POSITION","DRIVE"},{"INHARM","DAMPING","PARTIAL 4","DRIVE"}};
 if(editor_page==3)return osc[engine][k];if(editor_page==4)return timbre[engine][k];
 if(editor_page==5&&k==3)return "LEVEL";
 if(editor_page==12&&k>=2)return k==2?"RATIO 1":"RATIO 4";
 return EDIT_LABELS[editor_page][k];
}
static void editor_move(int32_t d)
{
 int32_t dir=d<0?-1:1;uint32_t count=(uint32_t)(d<0?-d:d)%EDIT_PAGES;
 while(count--){do{editor_page=(uint8_t)((editor_page+EDIT_PAGES+dir)%EDIT_PAGES);}while(
  (editor_page>=6&&editor_page<=12)&&!(wm.patch.engine==1||(wm.patch.engine==4&&editor_page!=12)));
 }
 editor_selected=0;editor_mark_ms=0;page_dirty=1;
}
static void editor_ensure(void)
{if(!wm.patch_valid){wm.patch=WS_PRESETS[wm.sound];wm.patch_valid=1;}}
static void editor_adjust(uint32_t k,int32_t d)
{
 if(k>=editor_count())return;
 editor_selected=(uint8_t)k;
 if(editor_page==EDIT_PRINT){
  if(k==3&&wp_has_anchor){wp_active=(uint8_t)(d>0);wp_ring_count=0;wp_pos=wp_phase=0;for(uint32_t v=0;v<NV;v++)if(!voice[v].live)voice[v].gate=0;}
  page_dirty=1;return;
 }
 if(!editor_mark_ms||fm1_ms-editor_mark_ms>650u)wm_mark();editor_mark_ms=fm1_ms;
 editor_ensure();wm.legacy=0;
 uint8_t *value=editor_param(k);if(!value)return;int32_t max=127,step=2;
 ws_patch_t *p=&wm.patch;
 if(value==&p->filter||value==&p->lfo_dest){max=p->engine==3&&value==&p->lfo_dest?1:2;step=1;}
 if(value==&p->engine||value==&p->wave){max=value==&p->wave?5:4;step=1;}
 if(value==&p->algorithm){max=7;step=1;}
 if(editor_page==EDIT_SCATTER){max=k==0?WM_SCATTER_MODES-1u:k==1?WM_GRAIN_COUNT-1u:127;step=1;}

 uint32_t old=*value;*value=(uint8_t)ws_clamp((int32_t)*value+d*step,0,max);
 cue_boundary(old,*value,0,(uint32_t)max);
 for(uint32_t v=0;v<NV;v++)if(voice[v].gate&&ws_voice[v].modern&&voice[v].patch==wm.sound){
  if(ws_voice[v].p.engine!=wm.patch.engine){voice[v].splice=voice[v].last;voice[v].splice_left=32;ws_start(v,voice[v].note,(uint32_t)voice[v].gate/16384u);}
  else{ws_voice[v].p=wm.patch;for(uint32_t o=0;o<4;o++){
    uint32_t inc=ws_ratio_inc(voice[v].inc,wm.patch.ratio[o]);while(inc>0x60000000u)inc>>=1;ws_voice[v].inc[o]=inc;
  }if(wm.patch.engine!=1)ws_voice[v].inc[0]=voice[v].inc;}
 }
 page_dirty=1;
}
