/* Production note routing, voice-slot consistency, new models and v9 import. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
#define FRAMES 4096
static int32_t ref[FRAMES*2],out[FRAMES*2];
static void prepare(uint32_t preset,uint32_t slot)
{
 running=recording=anchor_view=0;reset_performance();wp_reset();
 wm=(wm_state_t)WM_DEFAULTS;ws_select(preset);wm.arp_on=0;wm.fx_on=0;
 arp_update_config();ages=0;cues_enabled=0;memset(voice,0,sizeof voice);memset(ws_voice,0,sizeof ws_voice);
 for(uint32_t v=0;v<NV;v++)if(v!=slot){voice[v].gate=1;voice[v].amp=10;voice[v].age=1;}
}
static void render(uint32_t preset,uint32_t slot,uint32_t note,uint8_t midi,int32_t *dst)
{
 prepare(preset,slot);
 if(midi)midi_message(1,0x90,(uint8_t)note,100);else performance_on(note,100,1);
 assert(voice[slot].note==note&&voice[slot].gate&&voice[slot].live==(midi?2:1));
 memset(dst,0,FRAMES*2*sizeof *dst);ws_render(slot,dst,FRAMES,0);
 if(midi)midi_message(1,0x80,(uint8_t)note,0);else performance_off(note,1,0);
 assert(!voice[slot].gate);
}
int main(void)
{
 uint32_t cases=0;
 /* G3 is key index 2. Compare the actual local performance entry with MIDI
  * on every key and voice slot, at matching velocity and onset state. */
 for(uint32_t engine=0;engine<WS_ENGINES;engine++){
  uint32_t p=0;while(WS_PRESETS[p].engine!=engine)p++;
  for(int32_t oct=-2;oct<=2;oct++)for(uint32_t key=0;key<27;key++){
   uint32_t note=(uint32_t)((int32_t)KEY0+oct*12+(int32_t)key);render(p,0,note,0,ref);
   for(uint32_t slot=0;slot<NV;slot++){
    render(p,slot,note,1,out);assert(!memcmp(ref,out,sizeof ref));cases++;
   }
  }
 }
 /* Every factory patch, G3 and both immediate semitone neighbours. A slot
  * change must never change the rendered timbre with matching onset state. */
 for(uint32_t p=0;p<WM_PRESET_COUNT;p++)for(uint32_t note=54;note<=56;note++){
  render(p,0,note,0,ref);
  for(uint32_t slot=1;slot<NV;slot++){render(p,slot,note,0,out);assert(!memcmp(ref,out,sizeof ref));cases++;}
 }
 /* New waveform models at both pitch limits and maximum legal modulation. */
 const uint8_t waves[6]={10,5,8,4,5,6};uint32_t corners=0;
 for(uint32_t e=0;e<WS_ENGINES;e++)for(uint32_t w=0;w<waves[e];w++)for(uint32_t high=0;high<2;high++){
  prepare(0,0);wm.patch.engine=e;wm.patch.wave=w;wm.patch.algorithm=7;
  wm.patch.shape=wm.patch.drive=wm.patch.feedback=wm.patch.resonance=127;
  wm.patch.inharm=wm.patch.detune=wm.patch.envdepth=wm.patch.lfo_depth=127;
  wm.patch.cutoff=high?127:0;wm.patch.sub=127;wm.patch.amp[0]=0;wm.patch.amp[2]=127;
  for(uint32_t k=0;k<4;k++)wm.patch.ratio[k]=high?127:1;
  note_on(high?127:0,127,1);memset(out,0,sizeof out);ws_render(0,out,FRAMES,127);
  for(uint32_t i=0;i<FRAMES*2;i++)assert(out[i]>-65536&&out[i]<65536);corners++;
 }
 wm=(wm_state_t)WM_DEFAULTS;ws_select(149);wm_record(0,55,100);assert(wm_catch());
 wm_disk_v10_t v9={WM_MAGIC,9,sizeof(wm_state_v10_t),{0}};memcpy(&v9.state,&wm,sizeof v9.state);
 const wm_state_t *p=wm_migrate_v9_ptr(&v9);assert(p&&!memcmp(p,&wm,sizeof wm));
 assert(!slot_save(2,&v9,sizeof v9));static wm_state_t loaded;
 assert(project_read_slot(2,&loaded)&&!memcmp(&loaded,&wm,sizeof wm));
 v9.magic=0;assert(!wm_migrate_v9_ptr(&v9));v9.magic=WM_MAGIC;
 v9.size--;assert(!wm_migrate_v9_ptr(&v9));v9.size++;
 v9.state.patch.wave=8;assert(!wm_migrate_v9_ptr(&v9));
 printf("%u exact route/voice comparisons (27 keys across five octaves, 150 presets around G3), %u new-model pitch extremes, v9 snapshot/Anchor import and malformed files pass\n",cases,corners);
}
