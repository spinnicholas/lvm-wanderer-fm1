/* SPDX-License-Identifier: GPL-3.0-only
 * Three bounded IMA ADPCM arenas. 16 kHz mono, 256 samples / independent 132 B block.
 * Anchor never aliases a write target. Source generation is swapped only on completion.
 * First Print captures the dry Anchor and processed current generation together.
 */
#define WP_BLOCKS 512u
#define WP_BLOCK 132u
#define WP_MAX (WP_BLOCKS*256u)
static uint8_t wp_arena[3][WP_BLOCKS*WP_BLOCK] __attribute__((section(".pool"),aligned(4)));
static const int16_t WP_STEP[89]={7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767};
static const int8_t WP_INDEX[16]={-1,-1,-1,-1,2,4,6,8,-1,-1,-1,-1,2,4,6,8};
typedef struct {int32_t predictor,index;uint32_t count;uint8_t arena;} wp_encoder_t;
static wp_encoder_t wp_enc,wp_anchor_enc;
static uint8_t wp_active,wp_armed,wp_writing,wp_has_anchor,wp_current=1,wp_spare=2,wp_undo_valid,wp_redo;
static uint8_t wp_undo_anchor,wp_source_anchor,wp_dry_view,wp_splice_left;
static int32_t wp_last_sample,wp_splice;
static uint32_t wp_length,wp_pos,wp_phase,wp_write_phase,wp_generation;
static int32_t wp_dry[256];
#define WP_CACHES 8u
static int16_t wp_cache[WP_CACHES][256] __attribute__((section(".pool")));static uint32_t wp_cache_block[WP_CACHES];
static uint8_t wp_cache_arena[WP_CACHES],wp_cache_valid[WP_CACHES],wp_cache_replace;
static void wp_invalidate(void){memset(wp_cache_valid,0,sizeof wp_cache_valid);}
/* A small dedicated history keeps Undo arenas untouched in Event view too. */
/* 512 ms is unchanged; 768 ms uses a fixed window at the RAM limit.
 * 12,288 samples at 16 kHz = 24 KiB history; 4 KiB above tested RC4.
 * The native RC4 RAM build had only ~9 KiB spare, so do not enlarge this
 * without reproducing the target linker map. No audio-rate division. */
#define WP_RING_SAMPLES 12288u
#define WP_RC4_RING_SAMPLES 10240u
static int16_t wp_ring[WP_RING_SAMPLES];
static uint32_t wp_ring_wrap(uint32_t at)
{return at>=WP_RING_SAMPLES?at-WP_RING_SAMPLES:at;}
static uint32_t wp_ring_count;static int32_t wp_ring_tone;
static uint32_t wp_ring_write,wp_ring_start,wp_repeat_size;static uint8_t wp_repeat_on;
static uint32_t wp_drift_offset;
static uint32_t wp_scatter_origin,wp_scatter_elapsed,wp_scatter_duration,wp_scatter_delay;
static int32_t wp_scatter_mix,wp_scatter_last,wp_scatter_splice;
static uint8_t wp_scatter_fade,wp_scatter_rushed;
static uint32_t wp_scatter_cache_elapsed=0xffffffffu,wp_scatter_cache_size,wp_scatter_cache_at,wp_scatter_cache_weight,wp_scatter_cache_repeat;
/* Ratchets fall away like a struck repeat: about -1 dB per repeat, with a floor. */
static const uint8_t WP_RATCHET[12]={255,227,203,181,162,145,130,117,106,97,90,84};
static uint32_t wp_scatter_rate=4096u,wp_scatter_window;static int32_t wp_scatter_tone;
static uint32_t wp_tape_wow,wp_tape_flutter;
static int32_t wp_tape_mod_q12;
/* Triangle wave in 20-bit phase: fast, bounded and table-free. */
static int32_t wp_tape_tri(uint32_t p)
{p&=0xfffffu;return (int32_t)(p<524288u?p:1048575u-p)-262144;}
/* Two Butterworth sections before 16 kHz capture; no floating point in firmware. */
static const int32_t WP_AA[2][5]={{1926,3851,1926,-11062,2381},{2569,5137,2569,-14756,8647}};
static int32_t wp_aa_state[2][2][4],wp_melt_f1,wp_melt_f2;
static int32_t wp_antialias(uint32_t stream,int32_t x)
{
 for(uint32_t k=0;k<2;k++){
  int32_t *z=wp_aa_state[stream][k];const int32_t *c=WP_AA[k];
  int32_t y=((c[0]*x)>>14)+((c[1]*z[0])>>14)+((c[2]*z[1])>>14)-((c[3]*z[2])>>14)-((c[4]*z[3])>>14);
  z[1]=z[0];z[0]=x;z[3]=z[2];z[2]=ws_clamp(y,-65535,65535);x=z[2];
 }
 return x;
}
static int32_t wp_decode_nibble(int32_t *predictor,int32_t *index,uint32_t code)
{
 int32_t step=WP_STEP[*index],diff=step>>3;if(code&1)diff+=step>>2;if(code&2)diff+=step>>1;if(code&4)diff+=step;
 *predictor=ws_clamp(*predictor+((code&8)?-diff:diff),-32768,32767);*index=ws_clamp(*index+WP_INDEX[code],0,88);return *predictor;
}
static void wp_encode(wp_encoder_t *e,int32_t sample)
{
 if(e->count>=WP_MAX)return;
 uint32_t at=e->count&255u;uint8_t *b=wp_arena[e->arena]+(e->count>>8)*WP_BLOCK;
 sample=ws_clamp(sample,-32767,32767);
 if(!at){e->predictor=sample;b[0]=(uint8_t)sample;b[1]=(uint8_t)((uint32_t)sample>>8);b[2]=(uint8_t)e->index;b[3]=0;}
 else{
  int32_t delta=sample-e->predictor,step=WP_STEP[e->index];uint32_t code=delta<0?8u:0u;if(delta<0)delta=-delta;
  if(delta>=step){code|=4;delta-=step;}if(delta>=(step>>1)){code|=2;delta-=step>>1;}if(delta>=(step>>2))code|=1;
  wp_decode_nibble(&e->predictor,&e->index,code);
  uint32_t byte=4+(at-1u)/2u;if(at&1u)b[byte]=(uint8_t)code;else b[byte]|=(uint8_t)(code<<4);
 }
 e->count++;
}
static int32_t wp_sample(uint8_t arena,uint32_t pos)
{
 if(!wp_length)return 0;if(pos>=wp_length)pos%=wp_length;uint32_t block=pos>>8;
 uint32_t slot;
 for(slot=0;slot<WP_CACHES;slot++)if(wp_cache_valid[slot]&&wp_cache_arena[slot]==arena&&wp_cache_block[slot]==block)break;
 if(slot==WP_CACHES){slot=wp_cache_replace;wp_cache_replace=(wp_cache_replace+1u)%WP_CACHES;
  const uint8_t *b=wp_arena[arena]+block*WP_BLOCK;int32_t p=(int16_t)((uint32_t)b[0]|((uint32_t)b[1]<<8)),index=b[2];
  wp_cache[slot][0]=(int16_t)p;uint32_t count=wp_length-block*256u;if(count>256)count=256;
  for(uint32_t k=1;k<count;k++){uint32_t code=(b[4+(k-1)/2]>>((k&1)?0:4))&15u;wp_cache[slot][k]=(int16_t)wp_decode_nibble(&p,&index,code);}
  wp_cache_arena[slot]=arena;wp_cache_block[slot]=block;wp_cache_valid[slot]=1;
 }
 return wp_cache[slot][pos&255u];
}
static void wp_cancel(void){wp_armed=wp_writing=0;wp_invalidate();}
static void wp_reset(void)
{wp_cancel();wp_active=wp_has_anchor=wp_undo_valid=wp_redo=0;wp_current=1;wp_spare=2;wp_length=wp_pos=wp_phase=wp_generation=0;wp_ring_count=wp_ring_write=0;wp_ring_tone=wp_melt_f1=wp_melt_f2=0;wp_drift_offset=0;wp_scatter_mix=wp_scatter_last=wp_scatter_tone=wp_tape_mod_q12=0;wp_tape_wow=wp_tape_flutter=0;scatter_clear();wp_repeat_on=wp_undo_anchor=wp_source_anchor=wp_dry_view=wp_splice_left=0;}
static uint8_t wp_undo(void)
{
 if(wp_armed||wp_writing){wp_cancel();return 1;}
 if(!wp_undo_valid)return 0;
 if(wp_undo_anchor)wp_dry_view=!wp_dry_view;
 else{uint8_t old=wp_current;wp_current=wp_spare;wp_spare=old;}
 wp_splice=wp_last_sample;wp_splice_left=32;wp_redo=!wp_redo;wp_invalidate();return 1;
}
static void wp_restore(void)
{
 if(!wp_has_anchor)return;wp_cancel();wp_active=1;anchor_view=0;wp_dry_view=wp_undo_anchor=wp_undo_valid=wp_redo=1;
 wp_splice=wp_last_sample;wp_splice_left=32;wp_pos=wp_phase=0;
 /* Anchor listening is read-only. A new Print from Anchor writes into spare. */
}
static uint8_t wp_timing_locked(void)
{return wp_has_anchor||wp_armed||wp_writing;}
static uint8_t wp_arm(void)
{
 if(wp_writing||wp_armed){wp_cancel();return 1;}
 if((step_samples*wm.loop_steps*160u)/441u>WP_MAX)return 0;
 if(!wp_active&&!wm_has_notes(&wm.memory)&&!(wm.layer_enabled&&wm_has_notes(&wm.layer)))return 0;
 wp_armed=1;return 1;
}
static void wp_boundary(void)
{
 if(wp_writing){
  if(wp_enc.count){wp_length=wp_enc.count;if(!wp_has_anchor){wp_has_anchor=1;wp_current=1;wp_spare=2;wp_undo_valid=1;}
   else{uint8_t old=wp_current;wp_current=wp_spare;wp_spare=old;wp_undo_valid=1;}
   wp_undo_anchor=(uint8_t)(!wp_generation||wp_source_anchor);wp_dry_view=0;
   wp_active=1;anchor_view=0;for(uint32_t v=0;v<NV;v++)if(!voice[v].live)voice[v].gate=voice[v].amp=0;wp_generation++;wp_redo=0;}
  wp_writing=0;wp_invalidate();page_dirty=1;
 }
 wp_pos=wp_phase=0;
 if(wp_armed){
  wp_armed=0;wp_source_anchor=(uint8_t)(anchor_view||wp_dry_view);wp_writing=1;wp_undo_valid=0;wp_redo=0;wp_write_phase=0;
  memset(wp_aa_state,0,sizeof wp_aa_state);
  wp_enc=(wp_encoder_t){0,0,0,wp_has_anchor?wp_spare:1};wp_anchor_enc=(wp_encoder_t){0,0,0,0};
  page_dirty=1;
 }
}
static void wp_scatter_begin(void)
{
 wp_repeat_on=(uint8_t)(scatter.kind!=SC_CLEAN);
 uint32_t step16=step_samples*160u/441u;
 if(wp_active&&wp_length){
  int32_t answer=wm_drift_answer(step_index,cycle_index);
  uint32_t hop=step16<wp_length?step16:step16%wp_length;
  uint32_t offset=answer>0?hop:answer<0?wp_length-hop:0u;
  if(offset==wp_length)offset=0;
  if(offset!=wp_drift_offset){wp_splice=wp_last_sample;wp_splice_left=32;wp_drift_offset=offset;}
 }else wp_drift_offset=0;
 wp_repeat_size=step16/(scatter.divisions?scatter.divisions:1u);
 if(scatter.kind==SC_HOLD)wp_repeat_size=step16/2u;
 if(scatter.kind==SC_CLOUD&&wp_repeat_size<1024u)wp_repeat_size=1024u;
 /* Auto retains the tempo-derived windows. Manual sizes vary +/-25% per
  * gesture using the planner's existing seed: no allocation or audio RNG. */
 if(wm.grain_size){uint32_t base=(uint32_t)WM_GRAIN_MS[wm.grain_size]*16u;
  wp_repeat_size=wm.grain_size==WM_GRAIN_COUNT-1u?base:base*(6u+(scatter.offset%5u))/8u;}
 uint32_t cap=wm.grain_size?WP_RING_SAMPLES:2048u;
 if(wp_repeat_size<32u)wp_repeat_size=32u;if(wp_repeat_size>cap)wp_repeat_size=cap;
 if(!wp_active&&wp_ring_count<wp_repeat_size)wp_repeat_on=0;
 wp_ring_start=wp_ring_wrap(wp_ring_write+WP_RING_SAMPLES-wp_repeat_size);
 if(wp_repeat_on&&!wp_active&&(scatter.kind==SC_HOP||scatter.kind==SC_JUMP)){
  /* Keep the RC4 hop/jump lookback limits for existing sizes. */
  uint32_t history_cap=wm.grain_size<=6u?4096u:wm.grain_size<WM_GRAIN_V11_COUNT?WP_RC4_RING_SAMPLES:WP_RING_SAMPLES;
  uint32_t history=wp_ring_count<history_cap?wp_ring_count:history_cap;
  uint32_t available=history-wp_repeat_size;
  uint32_t back=available?scatter.offset*137u%(available+1u):0u;
  wp_ring_start=wp_ring_wrap(wp_ring_start+WP_RING_SAMPLES-back);
 }
 wp_scatter_rate=4096u;
 if(scatter.kind==SC_TAPE)wp_scatter_rate=2048u;
 if(scatter.kind==SC_CLOUD)wp_scatter_rate=4096u;
 if(scatter.kind==SC_PITCH){
  /* Pitch-class preserving octaves; fifths only when every active scale tone permits it. */
  uint8_t fifth=1;for(uint32_t pc=0;pc<12u;pc++)if((WM_SCALES[wm.scale]>>pc&1u)&&!(WM_SCALES[wm.scale]>>((pc+7u)%12u)&1u))fifth=0;
  wp_scatter_rate=(scatter.offset&1u)?2048u:(fifth&&wm.scatter_variation>90u?6137u:8192u);
 }
 wp_scatter_window=wp_repeat_size*4096u/wp_scatter_rate;if(wp_scatter_window<32u)wp_scatter_window=32u;
 wp_scatter_elapsed=0;wp_scatter_cache_elapsed=0xffffffffu;wp_scatter_duration=step16;wp_scatter_delay=(scatter.offset%4u)*step16/16u;wp_scatter_rushed=0;
 if(wp_active&&wp_length){
  uint32_t back=wp_repeat_size;
  if(scatter.kind==SC_HOP)back=(scatter.offset&1u)?step16:wp_length-step16%wp_length;
  if(scatter.kind==SC_JUMP)back=step16*scatter.offset;
  wp_scatter_origin=(wp_pos+wp_length-back%wp_length)%wp_length;
 }
 wp_scatter_splice=wp_scatter_last;wp_scatter_fade=32;
}
/* Fractional bounded reads never borrow an Undo arena or modify the source.
 * Every wrap is inside a fixed captured window. The second head is half a window
 * apart; complementary triangular weights remove repeated hard grain edges. */
/* Caller computes interpolated source coordinate once for BOTH heads. */
static int32_t wp_scatter_read_at(uint32_t at,uint32_t frac,uint8_t arena,uint8_t reverse)
{
 if(at>=wp_repeat_size)at=wp_repeat_size-1u;
 uint32_t a=reverse?wp_repeat_size-1u-at:at;
 uint32_t b=reverse?(a?a-1u:a):(a+1u==wp_repeat_size?a:a+1u);
 int32_t x,y;
 if(wp_active){x=wp_sample(arena,wp_scatter_origin+a);y=wp_sample(arena,wp_scatter_origin+b);}
 else{x=wp_ring[wp_ring_wrap(wp_ring_start+a)];y=wp_ring[wp_ring_wrap(wp_ring_start+b)];}
 return x+(((y-x)*(int32_t)frac)>>12);
}
/* Test/utility entry point keeps the original signature but shares the renderer. */
static int32_t wp_scatter_read(uint32_t phase,uint32_t size,uint8_t arena,uint8_t reverse)
{
 (void)size;
 int32_t q=(int32_t)(phase*wp_scatter_rate+(wp_phase*wp_scatter_rate)/FS);
 if(scatter.kind==SC_TAPE)q+=wp_tape_mod_q12;
 if(q<0)q=0;
 return wp_scatter_read_at((uint32_t)q>>12,(uint32_t)q&4095u,arena,reverse);
}
static int32_t wp_scatter_audio(int32_t clean,uint8_t arena)
{
 uint8_t enabled=(uint8_t)(running&&wp_repeat_on&&wp_scatter_elapsed>=wp_scatter_delay&&wm.decay&&!wm.legacy&&!anchor_view&&!wp_dry_view);
 if(!enabled&&!wp_scatter_mix)return clean;
 int32_t affected=wp_scatter_last;
 if(enabled){
  uint32_t elapsed=wp_scatter_elapsed-wp_scatter_delay,size=wp_scatter_window;
  if(scatter.kind==SC_RUSH&&elapsed>wp_scatter_duration/2u){if(!wp_scatter_rushed){wp_scatter_splice=wp_scatter_last;wp_scatter_fade=32;wp_scatter_rushed=1;}size/=2u;if(size<32u)size=32u;}
  /* Elapsed only advances at 16 kHz, not at the 44.1 kHz callback rate.
   * Cache modulo and taper weight across the two/three repeated host samples. */
  if(wp_scatter_cache_elapsed!=elapsed||wp_scatter_cache_size!=size){
   wp_scatter_cache_repeat=elapsed/size;wp_scatter_cache_at=elapsed-wp_scatter_cache_repeat*size;
   uint32_t edge=wp_scatter_cache_at<size-wp_scatter_cache_at?wp_scatter_cache_at:size-wp_scatter_cache_at;
   uint32_t weight=edge*65536u/size;
   wp_scatter_cache_weight=weight>32768u?32768u:weight;
   wp_scatter_cache_elapsed=elapsed;wp_scatter_cache_size=size;
  }
  uint32_t at=wp_scatter_cache_at;
  if(scatter.kind==SC_DROP){
   uint32_t gap=wm.grain_size?size:wp_scatter_duration/4u;
   if(gap>wp_scatter_duration*3u/4u)gap=wp_scatter_duration*3u/4u;
   uint32_t start=(scatter.offset%4u)*(wp_scatter_duration-gap)/4u;
   affected=elapsed>=start&&elapsed<start+gap?0:clean;
   affected=wp_scatter_last+ws_clamp(affected-wp_scatter_last,-1024,1024);
  }else{
   uint8_t reverse=(uint8_t)(scatter.kind==SC_REVERSE||(scatter.kind==SC_CLOUD&&(scatter.offset&1u)));
   int32_t q=(int32_t)(at*wp_scatter_rate+(wp_phase*wp_scatter_rate)/FS);
   if(scatter.kind==SC_TAPE)q+=wp_tape_mod_q12;
   if(q<0)q=0;
   uint32_t aq=(uint32_t)q;
   int32_t one=wp_scatter_read_at(aq>>12,aq&4095u,arena,reverse);
   uint32_t edge=at<size-at?at:size-at;
   int32_t hard=edge<16u?(one*(int32_t)edge)>>4:one;
   if(wm.scatter_edge||scatter.kind==SC_CLOUD){
    uint32_t other=at+size/2u;
    if(other>=size)other-=size;
    /* Difference is signed: do not wrap into a huge source index. */
    int32_t oqs=q+((int32_t)other-(int32_t)at)*(int32_t)wp_scatter_rate;
    if(oqs<0)oqs=0;uint32_t oq=(uint32_t)oqs;
    int32_t two=wp_scatter_read_at(oq>>12,oq&4095u,arena,reverse);
    int32_t weight=(int32_t)wp_scatter_cache_weight;
    int32_t smooth=(one*weight+two*(32768-weight))>>15;
    int32_t blend=scatter.kind==SC_CLOUD?127:wm.scatter_edge;
    affected=hard+((smooth-hard)*blend)/127;
   }else affected=hard;
  }
  if(scatter.kind==SC_STUTTER||scatter.kind==SC_RUSH){
   uint32_t n=wp_scatter_cache_repeat<11u?wp_scatter_cache_repeat:11u;
   affected=(affected*(int32_t)WP_RATCHET[n])>>8;
  }
  /* Tone and level follow the wet signal; no nonlinear clipping of fragments. */
  int32_t coeff=768+wm.scatter_tone*224;
  wp_scatter_tone+=((affected-wp_scatter_tone)*coeff)>>15;
  affected=wp_scatter_tone;
  if(wp_scatter_fade){affected=(affected*(32-wp_scatter_fade)+wp_scatter_splice*wp_scatter_fade)>>5;wp_scatter_fade--;}
  wp_scatter_last=affected;
 }
 int32_t target=enabled?(int32_t)wm.scatter_mix*256/127:0;
 wp_scatter_mix+=ws_clamp(target-wp_scatter_mix,-8,8);
 return clean+(((affected-clean)*wp_scatter_mix)>>8);
}
/* Before insert effects, keep dry for the protected first-generation Anchor. */
static void wp_input(int32_t *out,uint32_t n)
{
 int16_t *ring=wp_ring;
 uint8_t capture_dry=wp_writing&&!wp_has_anchor;
 for(uint32_t i=0;i<n;i++){
  int32_t dry=out[i*2];if(capture_dry)wp_dry[i]=dry;wp_ring_tone+=(dry-wp_ring_tone)>>1;
  if(running&&wp_active){
   uint32_t pos=wp_pos;
   if(!anchor_view&&!wp_dry_view){
    pos+=wp_drift_offset;if(pos>=wp_length)pos-=wp_length;
   }
   uint8_t arena=anchor_view||wp_dry_view?0:wp_current;int32_t a=wp_sample(arena,pos),b=wp_sample(arena,pos+1);
   uint32_t frac=(wp_phase*32768u)/FS;
   int32_t sample=a+(((b-a)*(int32_t)frac)>>15);
   sample=wp_scatter_audio(sample,arena);
   uint32_t edge=wp_pos<wp_length-wp_pos?wp_pos:wp_length-wp_pos;
   if(edge<32u)sample=(sample*(int32_t)edge)>>5;
   if(wp_splice_left){sample=(sample*(32-wp_splice_left)+wp_splice*wp_splice_left)>>5;wp_splice_left--;}
   wp_last_sample=sample;
   if(wm.melt&&!anchor_view&&!wp_dry_view){
    uint32_t cutoff=220u-wm.melt/2u;int32_t cf=FILTER_G[cutoff],h=FILTER_H[0][cutoff];
    int32_t v1=((wp_melt_f1+((cf*(sample-wp_melt_f2))>>14))*h)>>14;
    int32_t v2=wp_melt_f2+((cf*v1)>>14);wp_melt_f1=ws_clamp(2*v1-wp_melt_f1,-65535,65535);wp_melt_f2=ws_clamp(2*v2-wp_melt_f2,-65535,65535);
    int32_t shaped=ws_clamp((v2*(128+wm.melt))>>7,-28000,28000);shaped-=((((shaped*shaped)>>15)*shaped)>>15)/3;
    sample=(sample*(127-wm.melt)+shaped*wm.melt)/127;
   }
   dry+=sample;
  }else{dry=wp_scatter_audio(dry,0);}
  out[i*2]=dry;
  /* Scattered or printed audio replaces the dry voice; fade its spread too. */
  if(wp_scatter_mix||(running&&wp_active))out[i*2+1]=(out[i*2+1]*(256-ws_clamp(wp_active?256:wp_scatter_mix,0,256)))>>8;
  if(running){wp_phase+=16000;if(wp_phase>=FS){wp_phase-=FS;
    if(wp_active){if(++wp_pos>=wp_length)wp_pos=0;}
    else if(!wp_repeat_on){ring[wp_ring_write]=(int16_t)ws_clamp(wp_ring_tone,-32767,32767);if(++wp_ring_write==WP_RING_SAMPLES)wp_ring_write=0;if(wp_ring_count<WP_RING_SAMPLES)wp_ring_count++;}
    if(wp_repeat_on){wp_scatter_elapsed++;}
    /* Keep tape transport phase moving while stopped from scatter, so a new
     * Tape event does not always restart at the same artificial wobble. */
    wp_tape_wow+=23u;wp_tape_flutter+=426u;
    if(wp_repeat_on&&scatter.kind==SC_TAPE)
     wp_tape_mod_q12=((wp_tape_tri(wp_tape_wow)*6144)>>18)+((wp_tape_tri(wp_tape_flutter)*1024)>>18);
  }}
 }
}
static void wp_output(int32_t *out,uint32_t n)
{
 if(!wp_writing)return;
 for(uint32_t i=0;i<n;i++){
  int32_t wet=wp_antialias(0,(out[i*2]+out[i*2+1])/2);
  int32_t dry=wp_has_anchor?0:wp_antialias(1,wp_dry[i]);
  wp_write_phase+=16000;
  if(wp_write_phase>=FS){wp_write_phase-=FS;
   if(!wp_has_anchor)wp_encode(&wp_anchor_enc,dry);
   wp_encode(&wp_enc,wet);
  }
 }
}
