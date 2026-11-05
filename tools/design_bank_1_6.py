#!/usr/bin/env python3
"""150 role-led patches revoiced for the rebuilt oscillator/physical engines."""
import json
from pathlib import Path
import design_bank_1_5 as old
R=Path(__file__).resolve().parents[1]
def bank():
 rows=old.bank()
 phase={
 'RUBBER STEP':(1,0,84,32),'HOLLOW LOW':(1,1,112,8),'ANGLED BASS':(0,2,102,24),'BROKEN TRI':(3,0,64,22),
 'ROUND WHISTLE':(0,0,0,0),'PRISM LEAD':(2,0,104,16),'BENT RIBBON':(4,1,97,18),
 'WAX KEYS':(0,0,68,52),'GLASS POLY':(6,0,100,22),'PAPER IVORY':(3,0,82,26),
 'WAX PIANO':(0,0,35,84),'ROUND CELESTA':(6,3,112,12),'PHASE REED':(1,1,116,8),'WAX ACCORDION':(1,4,108,14),
 'PHASE HORN':(0,0,78,42),'COPPER WIND':(4,1,109,16),'PRISM STRINGS':(0,4,96,16),'SOFT PHASE BOW':(1,0,90,14),
 'MOSS CHOIR':(1,3,98,12),'LOW ORBIT':(3,0,53,10),'SILVER CHOIR':(4,6,101,14),'PHASE DUSK':(0,3,48,10),
 'LANTERN PHASE':(5,0,91,24),'RIBBON CURRENT':(4,1,98,14),'SLOW PRISM':(3,0,81,22),'WAX WAVES':(1,4,109,10),
 'SOFT HAMMER':(3,0,40,65),'PHASE CRYSTAL':(6,7,110,15),'PULSE TICK':(2,0,119,8),'BROKEN GLASS':(7,6,114,13)}
 wire={'STEEL THREAD','DULCIMER WIRE','BRIDGE GLASS','METAL BRUSH','OPEN WIRE','WIRE SNAP'}
 hammer={'THUMB PIANO','SOFT KALIMBA','PICK MALLET','WOOD CHIRP'}
 sine={'FELT STRINGS','FELT HARP','NYLON KEYS','BAMBOO AIR'}
 for x in rows:
  p=x['patch'];name=x['name'];e=p['engine'];strike=p['position'];p['drive']=0;p['level']=90;p['position']=64;p['morph'][4]=0
  if e==0:
   p['shape']=0;p['position']=70;p['inharm']=0
   if 'STRINGS' in name or name in ['AMBER HORIZON','PAPER SKY','SOFT REESE','WARM POLY']:p['wave']=5;p['detune']=9 if 'BASS' in x['bank'].upper() else 15
   if name in ['COPPER SOLO','MUTED HORN','SOFT FLUTE']:p['inharm']=3 if name=='SOFT FLUTE' else 1
   if name in ['ROUND SUB','SOFT FLUTE']:p['drive']=0;p['shape']=0
   if name in ['ACID POCKET','RESONANT TAP']:p['resonance']=104;p['drive']=12
   if name=='COPPER GROWL':p['wave']=6;p['drive']=10
   p['morph'][1]=0;p['morph'][3]=18 if p['wave']==2 else 0
  elif e==1:
   p['cutoff']=127;p['envdepth']=0;p['position']=0;p['wave']=0;p['mix']=32;p['sub']=0
   p['shape']=min(90,p['shape']+12);p['morph']=[0,30,0,0,0,0]
   if name in ['FELT TINES','GLASS TINES']:p['sub']=1;p['ratio'][2:]=[8,112];p['mix']=22;p['shape']=53
   if name=='ROUND EP':p['ratio']=[16,16,16,48];p['mix']=35;p['shape']=34
   if name=='REED KEYS':p['wave']=1;p['shape']=27;p['op_level'][1]=38
   if name=='WOOD KNOCK':p['algorithm']=0;p['shape']=39;p['ratio']=[16,16,48,80]
   if name=='NEON REED':p['algorithm']=6;p['wave']=3;p['feedback']=10;p['shape']=45
   if name=='GOLDEN HORN':p['algorithm']=3;p['shape']=38
   if name=='FM SHIMMER':p['algorithm']=5;p['ratio']=[16,32,48,16];p['shape']=43
   # Each held role needs a held modulation body. A zero-sustain modulator
   # makes unrelated reed, brass and keyboard patches converge to sine waves.
   fm_body={'SINE WEIGHT':(0,0,85,60),'WOOD KNOCK':(0,0,75,42),
    'NEON REED':(72,48,520,280),'CLEAN DIGITAL':(24,8,480,190),
    'FELT TINES':(12,4,850,320),'ROUND EP':(22,7,1300,520),
    'GLASS TINES':(24,12,1050,600),'REED KEYS':(92,46,1100,650),
    'GOLDEN HORN':(88,64,900,750),'DARK GLASS':(54,38,3200,2500),
    'FM SHIMMER':(76,48,2600,1800),'FM MARIMBA':(0,0,100,55),
    'FM CHIME':(22,12,2800,1800)}
   if name in fm_body:
    a,b,da,db=fm_body[name];p['op_env'][1]=old.env(1,da,a,180);p['op_env'][3]=old.env(1,db,b,140)
   if name=='FM CHIME':p['shape']=83;p['op_level'][1]=92;p['op_level'][3]=74;p['mix']=48
   if name=='GLASS TINES':p['op_level'][2]=100;p['mix']=40;p['shape']=74
   if name=='FELT TINES':p['op_level'][2]=96;p['op_level'][3]=58;p['mix']=35
  elif e==2:
   wave,line,dcw,depth=phase[name];p.update(wave=wave,algorithm=line,shape=dcw,envdepth=depth,cutoff=127,position=30,drive=0,inharm=0)
   p['morph']=[0,18,0,0,0,3]
   p['mix']={'HOLLOW LOW':0,'PRISM LEAD':14,'BENT RIBBON':42,'WAX KEYS':0,'GLASS POLY':20,'PAPER IVORY':40,'PHASE REED':0,'WAX ACCORDION':48,'PHASE HORN':0,'PRISM STRINGS':45,'MOSS CHOIR':48,'LOW ORBIT':18,'SILVER CHOIR':65,'PHASE DUSK':28,'LANTERN PHASE':12,'RIBBON CURRENT':50,'WAX WAVES':55,'PHASE CRYSTAL':74,'BROKEN GLASS':100}.get(name,p['mix'])
   if name in ['BROKEN GLASS','PHASE CRYSTAL']:p['inharm']=1
   if name=='ROUND WHISTLE':p['mix']=0;p['detune']=0;p['morph'][1]=40
  elif e==3:
   p['wave']=1 if name in wire else 2 if name in hammer else 3 if name in sine else 0
   p['shape']=old.time(8000 if name in ['LONG ZITHER','OPEN WIRE','STRING HALO','DRIFTING WIRE'] else 500 if name in ['PALM MUTE','MUTED WIRE','WIRE SNAP','WOOD CHIRP'] else 3600)
   p['position']={'NYLON HARP':48,'CEDAR PLUCK':20,'DULCIMER WIRE':8,'BRIDGE GLASS':5,'PALM MUTE':44,'TAPE HARP':82}.get(name,strike)
   p['pulse']={'CEDAR PLUCK':24,'NYLON HARP':8,'DARK KOTO':42,'PAPER GUITAR':80,'RUBBER WIRE':10,'LONG ZITHER':38,'OPEN KORA':18,'DARK BRIDGE':68,'DRIFTING WIRE':110}.get(name,20)
   p['cutoff']=min(127,p['cutoff']+18);p['envdepth']=0;p['mix']=90 if 'FELT' in name else 0 if name in wire else 30
   if name=='NYLON KEYS':p['wave']=0;p['pulse']=5;p['position']=53;p['damp']=44
   if name=='FELT HARP':p['wave']=0;p['pulse']=0;p['position']=85;p['damp']=78
   if name=='BAMBOO AIR':p['wave']=1;p['filter']=1;p['cutoff']=82;p['resonance']=35;p['mix']=60;p['damp']=95
   p['morph']=[10,10,-18,0,0,0]
  else:
   p['cutoff']=127;p['envdepth']=0;p['position']=strike;p['shape']=75;p['morph']=[0,18,-12,0,0,0]
   p['wave']=4 if p['amp'][2]>60 else 1 if name in ['SKIN TOM','CLAY DRUM','LOW VESSEL','CLAY LOW'] else 0 if x['bank']=='Tuned mallets' else 2
   p['damp']=p['damp']//4
   if name=='DEEP MARIMBA':p['ratio']=[16,44,86,127];p['damp']=12
   if name in ['BRONZE BOWL','SMALL GONG','IRON PLATE','TEMPLE BELL','TUNED CAN']:p['shape']=112;p['op_level'][1:]=[min(127,round(a*1.35)) for a in p['op_level'][1:]]
  model=['BLEP oscillators and feedback ladder','four independently enveloped FM operators','two phase-distortion lines','fractional-delay resonating string','damped resonator modes' if p['wave']!=4 else 'held additive partials'][e]
  x['description']+=' Uses '+model+'.'
 return rows
def main():
 rows=bank();levels=R/'assets/wanderer_levels_1_6.json'
 if levels.exists():
  trims=json.loads(levels.read_text())
  for x in rows:x['patch']['level']=trims.get(x['name'],x['patch']['level'])
 fields=list(rows[0]['patch']);prior=(R/'firmware/src/wanderer_sound_data.h').read_text();tables=prior[prior.index('/* 1..8000 ms;'):].rstrip()
 def c(v):return '{'+','.join(c(a) for a in v)+'}' if isinstance(v,list) else str(v)
 lines=[old.TYPE,'#define WS_PRESET_COUNT 150u','static const char * const WS_NAMES[WS_PRESET_COUNT]={'+','.join(json.dumps(x['name']) for x in rows)+'};','static const ws_patch_t WS_PRESETS[WS_PRESET_COUNT]={']
 lines+=['{'+','.join(c(x['patch'][f]) for f in fields)+'},' for x in rows];lines+=['};',tables]
 (R/'firmware/src/wanderer_sound_data.h').write_text('\n'.join(lines)+'\n');(R/'assets/wanderer_presets_1_6.json').write_text(json.dumps(rows,indent=2)+'\n')
 (R/'build/bank-1.6').mkdir(parents=True,exist_ok=True)
 (R/'build/bank-1.6-registers.inc').write_text(','.join(str(x['register']) for x in rows)+'\n')
 print('150 patches revoiced for BLEP/ladder, four-op FM, eight-wave PD, fractional strings and modal resonators')
if __name__=='__main__':main()
