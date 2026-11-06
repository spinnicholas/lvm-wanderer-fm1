/* SPDX-License-Identifier: GPL-3.0-only
 * Wanderer 3.0 wander engine. Memory (Layer 1) moves a little further from its
 * recorded source at the end of every loop; the Anchor never moves.
 *
 *  - DRIFT walks individual notes through the current scale, one degree at a time.
 *    Low settings protect the downbeat and root notes; high settings allow wider
 *    walks and occasional octave leaps. Gravity always pulls notes back home, and
 *    with DRIFT at zero every note returns to its recorded pitch over a few loops.
 *  - MELT softens weak beats, lets notes lengthen toward legato, drips quiet
 *    one-degree-lower echoes into the empty step after a note, and deepens the
 *    patch's Melt timbre a little more each loop. With MELT at zero these relax.
 *
 * All offsets are stored relative to the recorded notes, so recording, overdubs
 * and Undo keep working on the source. The state is runtime only: KEEP (GLO)
 * bakes what you hear into Memory, RETURN (hold GLO) goes straight home.
 * Work per loop is bounded (32 steps x 4 slots) and runs at the loop boundary
 * in the audio callback. No division by a variable that can be zero.
 */
#pragma once
#define WW_LEN_UNIT 32u /* gate offset unit in ticks: an eighth of a step */
typedef struct {
 int8_t deg[WM_STEPS][WM_POLY];  /* scale-degree offset */
 int8_t vel[WM_STEPS][WM_POLY];  /* velocity offset */
 int8_t len[WM_STEPS][WM_POLY];  /* gate offset in WW_LEN_UNIT ticks */
 uint8_t drip[WM_STEPS];         /* echo velocity dripped into an empty step */
 uint8_t drip_note[WM_STEPS];
 uint32_t rng;
 uint16_t loops;                 /* loops wandered since the last Keep/Return */
 uint8_t timbre;                 /* cumulative Melt timbre, follows the Melt knob */
 uint8_t distance;               /* 0..100, how far Memory is from its source */
 uint8_t moved;                  /* set at each loop that changed something */
 uint8_t want_anchor;            /* ISR asks the main loop to protect the source */
} ww_state_t;
static ww_state_t ww={.rng=0x9e3779b9u};
static uint32_t ww_random(void)
{uint32_t x=ww.rng;if(!x)x=0x9e3779b9u;x^=x<<13;x^=x>>17;x^=x<<5;return ww.rng=x;}
static uint8_t ww_active(void){return 1u;}
static void ww_reset(void)
{
 uint32_t seed=ww.rng;memset(&ww,0,sizeof ww);ww.rng=seed?seed:0x9e3779b9u;
}
/* Recording into a slot starts that note's journey again. */
static void ww_clear_slot(uint32_t step,uint32_t slot)
{
 if(step>=WM_STEPS||slot>=WM_POLY)return;
 ww.deg[step][slot]=ww.vel[step][slot]=ww.len[step][slot]=0;
 if(!slot)ww.drip[step]=0;
}
static uint32_t ww_scale_size(void)
{uint32_t m=WM_SCALES[wm.scale<WM_SCALE_COUNT?wm.scale:0],n=0;while(m){n+=m&1u;m>>=1;}return n?n:12u;}
static int32_t ww_toward(int32_t x,int32_t target,int32_t step)
{if(x<target)return x+step>target?target:x+step;if(x>target)return x-step<target?target:x-step;return x;}
/* The event as it sounds now. Layer 2 and the Anchor are never passed here. */
static wm_event_t ww_apply(wm_event_t e,uint32_t step,uint32_t slot)
{
 if(!e.velocity||step>=WM_STEPS||slot>=WM_POLY)return e;
 int32_t d=ww.deg[step][slot];
 if(d)e.note=wm_scale_move(e.note,d);
 int32_t v=(int32_t)e.velocity+ww.vel[step][slot];
 e.velocity=(uint8_t)(v<4?0:v>127?127:v); /* a fully melted note dissolves */
 int32_t g=(int32_t)e.gate+(int32_t)ww.len[step][slot]*(int32_t)WW_LEN_UNIT;
 int32_t top=(int32_t)wm.loop_steps*256;
 e.gate=(uint16_t)(g<24?24:g>top?top:g);
 return e;
}
static uint8_t ww_step_empty(uint32_t step)
{return (uint8_t)!wm_step_notes(&wm.memory,step%wm.loop_steps);}
/* A dripped echo plays in an otherwise empty step. */
static wm_event_t ww_drip_event(uint32_t step)
{
 wm_event_t e={0,0,0};
 if(step<WM_STEPS&&ww.drip[step]&&ww_step_empty(step))e=(wm_event_t){ww.drip_note[step],ww.drip[step],160u};
 return e;
}
static void ww_measure(void)
{
 uint32_t sum=0,events=0;
 for(uint32_t s=0;s<wm.loop_steps;s++){
  for(uint32_t v=0;v<WM_POLY;v++){
   if(!wm_get(&wm.memory,s,v).velocity)continue;
   events++;
   int32_t d=ww.deg[s][v],q=ww.vel[s][v],l=ww.len[s][v];
   sum+=(uint32_t)(d<0?-d:d)*24u+(uint32_t)(q<0?-q:q)/2u+(uint32_t)(l<0?-l:l);
  }
  if(ww.drip[s]&&ww_step_empty(s))sum+=12u;
 }
 /* About two degrees on every note, or the equivalent, reads as 100%. */
 uint32_t full=events*48u+1u;
 uint32_t pct=sum*100u/full;
 ww.distance=(uint8_t)(pct>100u?100u:pct);
}
/* One loop of wandering. Called by step_play at the start of a new loop. */
static void ww_loop(void)
{
 if(!ww_active())return;
 const int32_t D=wm.drift,M=wm.melt;
 const wm_phrase_t *p=&wm.memory;
 uint32_t L=wm.loop_steps,count=0;
 static uint8_t list_step[WM_STEPS*WM_POLY],list_slot[WM_STEPS*WM_POLY]; /* ISR only */
 for(uint32_t s=0;s<L;s++)for(uint32_t v=0;v<WM_POLY;v++)
  if(wm_get(p,s,v).velocity){list_step[count]=(uint8_t)s;list_slot[count]=(uint8_t)v;count++;}
 if(!count){ww.distance=0;return;}
 uint32_t before=ww.distance;
 if((D||M)&&!wm.has_anchor)ww.want_anchor=1;
 /* Gravity: notes beyond the Drift range always step home; within it they
  * step home with a chance that falls as Drift rises. */
 int32_t range=1+D/26,octave=(int32_t)ww_scale_size();
 uint32_t home_chance=D?(uint32_t)(96-D*3/4):128u; /* of 128 */
 for(uint32_t i=0;i<count;i++){
  uint32_t s=list_step[i],v=list_slot[i];int32_t d=ww.deg[s][v],a=d<0?-d:d;
  if(!d)continue;
  if(a>octave+range||(a>range&&a<octave-range)||(ww_random()&127u)<home_chance)
   ww.deg[s][v]=(int8_t)(d>0?d-1:d+1);
 }
 /* Drift moves: about one note in four per loop at full Drift, at least the
  * occasional single move at low settings. */
 if(D){
  uint32_t moves=(count*(uint32_t)D+(ww_random()&255u)*2u)/512u;
  if(!moves&&(ww_random()&127u)<(uint32_t)D)moves=1;
  for(uint32_t m=0;m<moves;m++){
   uint32_t i=ww_random()%count,s=list_step[i],v=list_slot[i];
   wm_event_t e=wm_get(p,s,v);
   if(D<48&&(s==0u||e.note%12u==wm.root))continue; /* landmarks stay put */
   int32_t d=ww.deg[s][v],step;
   if(D>=100&&(ww_random()&7u)==0u)step=d>0?-octave:octave;   /* octave leap */
   else step=d?((ww_random()&3u)?(d>0?1:-1):(d>0?-1:1)):((ww_random()&1u)?1:-1);
   int32_t next=d+step,a=next<0?-next:next;
   if(a>range&&a!=octave&&!(a>octave-range&&a<octave+range))next=d-step;
   ww.deg[s][v]=(int8_t)ws_clamp(next,-octave-range,octave+range);
  }
 }
 /* Melt: landmarks lean in, weak beats sink and lengthen. Changes are gradual. */
 int32_t rate=1+M/20;
 for(uint32_t i=0;i<count;i++){
  uint32_t s=list_step[i],v=list_slot[i];
  wm_event_t e=wm_get(p,s,v);
  uint8_t strong=(uint8_t)((s&3u)==0u),off=(uint8_t)(s&1u);
  int32_t vel_target=strong?M/14:off?-(M*48)/127:-(M*20)/127;
  if(M>=96&&off&&s&&(ww_random()&15u)==0u)vel_target=-(int32_t)e.velocity; /* dissolve */
  int32_t len_target=strong?M/32:M/14;          /* up to about a step longer */
  if(e.gate>(uint32_t)wm.loop_steps*128u)len_target=0;  /* long notes keep their duration */
  int32_t jitter=M?(int32_t)(ww_random()%3u)-1:0;
  ww.vel[s][v]=(int8_t)ws_clamp(ww_toward(ww.vel[s][v],vel_target+jitter,M?rate:6),-127,40);
  ww.len[s][v]=(int8_t)ws_clamp(ww_toward(ww.len[s][v],len_target,M?1:2),-8,24);
 }
 /* Drips fall into the empty step after a note; they evaporate without Melt. */
 for(uint32_t s=0;s<L;s++){
  uint32_t prev=(s+L-1u)%L;
  wm_event_t src=wm_get(p,prev,0);
  if(M>=40&&src.velocity&&ww_step_empty(s)&&(ww_random()&127u)<(uint32_t)(M-32)){
   int32_t target=(int32_t)src.velocity*M/300;
   ww.drip_note[s]=wm_scale_move(ww_apply(src,prev,0).note,-1);
   ww.drip[s]=(uint8_t)ws_clamp(ww_toward(ww.drip[s],target,4+M/16),0,90);
  }else if(ww.drip[s]&&(M<40||!src.velocity))ww.drip[s]=(uint8_t)(ww.drip[s]>8u?ww.drip[s]-8u:0u);
 }
 /* Timbre follows Melt over several loops instead of jumping. */
 ww.timbre=(uint8_t)ws_clamp(ww_toward(ww.timbre,M,M>ww.timbre?1+M/12:8),0,127);
 if(ww.loops<9999u)ww.loops++;
 ww_measure();
 if(ww.distance!=before||D||M)ww.moved=1;
}
/* Keep: what you hear becomes the new Memory source. Undo restores it. */
static uint8_t ww_keep(void)
{
 if(!ww_active()||!wm_memory_playable())return 0;
 wm_mark();
 static wm_phrase_t next;
 next=wm.memory;
 for(uint32_t s=0;s<WM_STEPS;s++)for(uint32_t v=0;v<WM_POLY;v++){
  wm_event_t e=wm_get(&wm.memory,s,v);
  if(e.velocity)*wm_at(&next,s,v)=ww_apply(e,s,v);
 }
 for(uint32_t s=0;s<wm.loop_steps;s++){
  wm_event_t d=ww_drip_event(s);
  if(d.velocity)*wm_at(&next,s,0)=d;
 }
 wm.memory=next;wm.has_branch=1;
 wm.branch_melt=(uint8_t)ws_clamp((int32_t)wm.branch_melt+ww.timbre/2,0,127);
 ww_reset();return 1;
}
/* Return: Memory goes straight home to its recorded source. */
static uint8_t ww_return(void)
{
 uint8_t had=(uint8_t)(ww.distance||ww.timbre||ww.loops);
 ww_reset();return had;
}
