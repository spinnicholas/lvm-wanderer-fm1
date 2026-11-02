#!/usr/bin/env python3
"""Deterministic, original 100-patch bank; every entry has a named musical role.
Run to regenerate coefficients and the generated JSON catalog. No runtime floats.
"""
import json, math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
NAMES=[
'VELVET REED','COPPER BASS','DUSK STRINGS','HOLLOW WOOD','PAPER ORGAN','SOFT EDGE','SLOW GLASS','NARROW LIGHT','WARM BRASS','SUB CURRENT',
'SILK PULSE','BROKEN RADIO','LOW TIDE','GOLDEN FIFTH','DUST FLUTE','WIDE HORIZON','ROUND SQUARE','MOSS PAD','SILVER SAW','NIGHT CHORD',
'FELT TINES','BRONZE BELL','ICE KEYS','RUBBER BASS','WIRE CHOIR','BAMBOO FM','CRYSTAL DROP','METAL REED','WOODEN GONG','NEON PLUCK',
'QUIET RHODES','TUNED PLATE','GLASS ORGAN','SOFT MALLET','CLOCK CHIME','DEEP MARIMBA','ELECTRIC HARP','AIR COLUMN','COLD BRASS','IRON DRUM',
'FOLDED SINE','PRISM KEYS','ANGLED BASS','BENT REED','PHASE CHOIR','WAX LEAD','SOFT TEETH','CERAMIC','WAVING GLASS','SLOPE ORGAN',
'VECTOR PAD','PAPER EDGE','ROUND FOLD','GHOST HORN','RIBBON','LANTERN','SPLIT LIGHT','THIN ICE','CHALK PULSE','SHADOW SYNC',
'CEDAR STRING','PALM MUTE','NYLON HARP','STEEL THREAD','DARK KOTO','RUBBER WIRE','PAPER GUITAR','BRASS WIRE','GLASS STRING','BASS WOOD',
'PICKED FIFTH','BAMBOO STRING','TAPE HARP','METAL BRUSH','FELT STRING','DRY KALIMBA','OPEN KORA','LOW CEDAR','DULCIMER','THUMB PIANO',
'CLAY BOWL','BRONZE PLATE','WOOD BLOCK','GLASS BOWL','HOLLOW DRUM','SMALL GONG','STONE BELL','WATER CUP','TUNED CAN','SKIN DRUM',
'BELL TREE','SOFT STEEL','MARBLE DROP','COPPER CUP','PAPER FRAME','LOW GONG','HANGING GLASS','SHORT LOG','METAL RAIN','DEEP VESSEL']
TYPE='''/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#define WS_ENGINES 5u
/* All panel values 0..127, topology fields validated separately. Times log spaced. */
typedef struct {
 uint8_t engine,wave,mix,sub,detune,pulse,shape,damp,position,inharm;
 uint8_t amp[4],fenv[4],cutoff,resonance,filter,envdepth,drive,level;
 uint8_t lfo_rate,lfo_depth,lfo_dest,algorithm,feedback;
 uint8_t ratio[4],op_level[4],op_env[4][4];
 int8_t morph[6]; /* cutoff, shape/index, damping, pulse, drive, detune */
} ws_patch_t;
'''
def bank():
 out=[]
 for i,name in enumerate(NAMES):
  e=i//20;j=i%20; role=j%5
  amp=([2,55,0,38] if role==0 else [1,38,0,22] if role==1 else [9,62,78,65] if role==2 else [1,70,36,46] if role==3 else [65,80,103,85])
  p=dict(engine=e,wave=j%5,mix=[24,70,105,45,90][role],sub=[12,48,0,20,0][role],detune=[3,0,11,4,17][role],pulse=25+(j*13)%78,
   shape=22+(j*17)%100,damp=25+(j*23)%99,position=15+(j*19)%100,inharm=(j*7)%110,amp=amp,
   fenv=[2+(j%4)*9,36+(j*7)%50,20+(j*11)%80,38+(j*3)%55],cutoff=[83,46,68,76,54][role]+j//5*8,
   resonance=[12,42,22,64,8][role],filter=j//7,envdepth=[63,72,32,48,21][role],drive=[8,30,4,15,0][role],level=84,
   lfo_rate=14+j*3,lfo_depth=[3,0,14,5,20][role],lfo_dest=(j//5)%3,algorithm=j%8,feedback=8+(j*7)%65,
   ratio=[[16,16,32,48],[16,32,80,112],[16,24,48,64],[8,16,32,16],[16,48,96,127]][role],
   op_level=[110,42+(j*5)%65,25+(j*9)%80,15+(j*13)%85],
   op_env=[amp,[1,22+(j*5)%60,20+(j*7)%55,28],[2,35+(j*11)%65,10,35],[1,18+(j*3)%55,0,24]],
   morph=[(-22 if j%2 else 28),(-35 if j%3==0 else 38),(-28 if j%2 else 30),(-20 if j%3 else 28),12,(-3 if j%2 else 7)])
  if e==1:p.update(cutoff=112,resonance=5,envdepth=8,sub=0,level=100)
  if e==3:p.update(amp=[0,85,110 if j%3 else 0,53],cutoff=105,envdepth=12,level=110)
  if e==4:p.update(amp=[0,85,0,65],cutoff=112,envdepth=10,level=100)
  # Explicit subtractive recipes: the name describes its oscillator, interval and articulation.
  if e==0:
   recipes=[
    (1,16,30,8,3,75,20,[8,62,78,50]),(0,16,70,48,0,46,42,[0,57,45,35]),
    (0,16,105,0,11,58,12,[60,80,94,80]),(1,16,45,20,4,56,35,[0,58,0,32]),
    (1,32,90,0,0,80,5,[0,0,105,20]),(0,16,63,12,7,100,22,[2,62,65,40]),
    (5,16,82,0,8,90,8,[75,83,86,90]),(2,16,48,0,0,80,40,[1,48,80,40]),
    (0,16,90,10,3,46,24,[16,64,78,44]),(3,16,10,64,0,55,0,[1,60,105,34]),
    (2,16,24,8,5,70,12,[12,55,80,60]),(2,16,75,0,22,57,64,[0,40,60,20]),
    (4,8,40,35,5,52,8,[85,80,99,95]),(0,24,76,0,3,70,28,[6,64,76,40]),
    (3,32,10,0,0,65,0,[15,45,105,60]),(0,16,95,0,17,66,10,[65,85,90,95]),
    (1,16,65,8,0,73,12,[3,70,80,45]),(4,16,40,15,11,44,8,[85,85,95,90]),
    (0,16,80,0,6,30,22,[1,70,36,46]),(1,24,70,45,9,50,10,[70,90,104,90])]
   wave,ratio,mix,sub,detune,cut,res,amp=recipes[j]
   p.update(wave=wave,mix=mix,sub=sub,detune=detune,cutoff=cut,resonance=res,amp=amp,filter=2 if j==18 else 1 if j in (7,11) else 0)
   p['ratio'][1]=ratio
   if j==7:p['pulse']=22
   if j==11:p.update(pulse=10,drive=70)
   if j==16:p['shape']=114
  else:p['filter']=0
  if e==1:
   ratios=[[16,16,112,16],[16,45,93,127],[16,48,80,112],[8,16,8,16],[16,32,48,64],
    [16,16,32,16],[16,64,96,112],[16,24,80,48],[16,23,37,85],[16,48,16,80],
    [16,16,16,112],[16,55,91,127],[16,32,64,96],[16,16,48,32],[16,64,112,127],
    [16,32,16,48],[16,48,80,16],[16,16,48,64],[16,32,80,112],[8,13,47,109]]
   p['ratio']=ratios[j];p['algorithm']=[0,2,1,0,6,3,0,5,4,2,3,2,7,5,1,4,3,6,1,2][j]
   if j in (0,1,2,5,6,8,9,10,11,13,14,15,16,19):p['amp']=[1,70+j%7,0,40+j%15]
   if j in (4,12,17,18):p['amp']=[35+j,80,90,75]
   p['op_env'][0]=p['amp'][:]
  if e==2:
   p['shape']=[18,105,35,87,64,117,44,99,12,75,28,112,55,91,39,82,23,120,49,97][j]
   if j in (4,8,10,14,15,17):p['amp']=[55+j,83,90,80]
  if e==3:
   p['position']=[22,8,47,31,17,64,85,12,42,26,74,36,92,57,15,51,78,19,103,44][j]
   p['damp']=[45,112,30,15,72,98,85,24,10,68,35,58,75,40,108,62,18,82,27,51][j]
   p['shape']=[98,45,112,118,73,65,54,105,123,87,111,82,69,95,40,90,116,76,119,103][j]
  if e==4:
   p['inharm']=[11,57,5,39,18,91,26,73,48,9,105,64,21,83,14,119,44,2,98,68][j]
   p['ratio']=[
    [16,44,85,117],[16,32,49,94],[16,44,86,127],[16,38,71,109],[16,25,34,47],
    [16,23,43,69],[16,42,83,120],[16,30,57,93],[16,36,63,105],[16,24,34,45],
    [16,31,63,127],[16,40,70,103],[16,39,73,121],[16,34,66,114],[16,28,40,59],
    [8,17,31,55],[16,41,77,113],[16,43,84,122],[16,37,89,127],[8,19,37,61]][j]
   decay=[100,105,55,98,65,112,94,92,79,63,99,103,77,95,60,117,107,51,101,111][j]
   p['amp']=[0,decay,0,80 if decay>90 else 52]
   p['op_env'][0]=[0,decay+min(10,127-decay),0,80]
   p['op_env'][1][1]=95+j%14;p['op_env'][2][1]=85+j%16;p['op_env'][3][1]=75+j%21
   p['op_level']=[90 if j%3 else 75,110,90 if j%2 else 110,65+j%7*8]

  out.append(dict(name=name,patch=p))
 return out

def main():
 out=bank();(ROOT/'assets/wanderer_presets_1_3.json').write_text(json.dumps(out,indent=2)+'\n')
 fields=list(out[0]['patch']);h=[TYPE,'static const char * const WS_NAMES[100]={'+','.join('"'+x['name']+'"' for x in out)+'};','static const ws_patch_t WS_PRESETS[100]={']
 def c(v):return '{'+','.join(c(x) for x in v)+'}' if isinstance(v,list) else str(v)
 for entry in out:h.append('{'+','.join(c(entry['patch'][f]) for f in fields)+'},')
 h+=['};','/* 1..8000 ms; smoothing coefficient per 16 audio frames, Q16. */']
 times=[max(1,round(8000**(i/127))) for i in range(128)]
 h+=['static const uint16_t WS_MS[128]={'+','.join(map(str,times))+'};',
 'static const uint16_t WS_RATE[128]={'+','.join(str(min(65535,max(1,round((1-math.exp(-16/(t*44.1)))*65536)))) for t in times)+'};',
 'static const uint32_t WS_ATTACK[128]={'+','.join(str(max(1,round(16777216*16/(t*44.1))))+'u' for t in times)+'};']
 h+=['static const uint32_t WS_LFO[128]={'+','.join(str(round(.04*300**(i/127)*2**32*16/44100))+'u' for i in range(128))+'};']
 import re
 g=list(map(int,re.search(r'FILTER_G\[221\]=\{([^}]+)',(ROOT/'firmware/src/wanderer_filter.h').read_text())[1].split(',')))
 h+=['static const uint16_t WS_HZ[128]={'+','.join(str(round(math.atan(g[i*220//127]/16384)*44100/math.pi)) for i in range(128))+'};']
 (ROOT/'firmware/src/wanderer_sound_data.h').write_text('\n'.join(h)+'\n')
if __name__=='__main__':main()
