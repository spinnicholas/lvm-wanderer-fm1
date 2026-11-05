#!/usr/bin/env python3
"""150 instruments: original MSFA FM6 voices, sync/ring analog and open ensembles."""
import json
from pathlib import Path
import design_bank_1_6 as prior
import design_bank_1_5 as d
R=Path(__file__).resolve().parents[1]
def bank():
 rows=prior.bank()
 # Algorithm is the native DX-style number minus one; ratios are quarter units.
 # Levels are native MSFA 0..99, not linear amplitude. Every voice is authored here.
 fm={
 'WOOD KNOCK':(0,[4,4,12,4,7,28],[96,78,67,73,59,70],'wood'),
 'NEON REED':(21,[4,4,8,12,4,4],[94,78,86,82,74,76],'reed'),
 'CLEAN DIGITAL':(4,[4,4,8,4,12,4],[94,74,80,67,68,70],'keys'),
 'FELT TINES':(4,[4,4,4,56,4,4],[97,76,80,77,66,65],'ep'),
 'ROUND EP':(0,[4,4,4,12,4,4],[97,72,86,70,64,64],'ep'),
 'GLASS TINES':(4,[4,8,4,56,4,12],[95,82,92,94,66,69],'ep'),
 'REED KEYS':(18,[4,4,8,4,12,4],[94,84,83,79,72,77],'reed'),
 'GOLDEN HORN':(18,[4,4,8,4,12,4],[96,82,90,85,77,76],'brass'),
 'DARK GLASS':(7,[4,4,8,12,4,7],[92,76,87,72,78,68],'pad'),
 'FM SHIMMER':(15,[4,8,12,4,7,16],[95,76,81,74,70,68],'pad'),
 'FM MARIMBA':(0,[4,4,12,4,4,28],[96,84,74,77,68,75],'wood'),
 'FM CHIME':(4,[4,11,4,23,4,39],[94,87,86,84,70,77],'bell'),
 'PAPER IVORY':(4,[4,4,12,4,8,4],[96,84,76,70,66,73],'clav'),
 'WAX PIANO':(4,[4,4,4,28,4,8],[97,76,82,83,71,70],'piano'),
 'ROUND CELESTA':(4,[4,12,4,28,8,4],[94,84,82,78,68,70],'bell'),
 'COPPER WIND':(21,[4,4,8,12,4,12],[94,86,82,80,73,75],'reed'),
 'SILVER CHOIR':(18,[4,4,8,12,4,28],[92,74,83,70,72,69],'pad'),
 'BELL AURORA':(4,[4,7,8,15,12,23],[92,78,85,75,76,73],'pad'),
 'CERAMIC WIND':(21,[4,4,12,20,4,8],[94,80,78,73,66,70],'reed'),
 'TUNED CAN':(4,[4,9,4,21,4,35],[96,89,85,85,78,81],'metal'),
 }
 rename={'PAPER IVORY':'HAMMER CLAV','WAX PIANO':'STAGE PIANO','ROUND CELESTA':'SILVER CELESTA','COPPER WIND':'BREATH REED','SILVER CHOIR':'DIGITAL CHOIR','CERAMIC WIND':'HOLLOW WIND'}
 for x in rows:
  p=x['patch'];name=x['name'];p['level']=90
  if p['engine']==0:
   if x['bank'] in ['Slow pads','Motion pads','Ensemble strings']:p['cutoff']=min(115,p['cutoff']+14)
   if name in ['SILK STRINGS','WARM CELLOS','HIGH VIOLAS','CHAMBER STRINGS']:
    p.update(wave=2,pulse=50,detune=8,lfo_dest=1,lfo_depth=24,lfo_rate=31)
   if name in ['COPPER SOLO','OCTAVE TORCH']:
    p.update(wave=8,detune=40 if name=='COPPER SOLO' else 63,shape=82,mix=99,cutoff=99,envdepth=12,resonance=20)
    x['name']='SYNC SOLO' if name=='COPPER SOLO' else 'SYNC TORCH'
    x['description']='Band-limited oscillator restart with an envelope-driven sync sweep.'
   if name=='VELVET FIFTH':
    p.update(wave=9,cutoff=111,ratio=[16,24,16,16],mix=127,sub=0,envdepth=0,detune=1)
    x['name']='RING FIFTH';x['description']='Saw-times-sine ring modulation at a fifth, for metallic chord colour.'
   if name=='PAPER SKY':p.update(wave=5,detune=22,cutoff=94,shape=0)
  if name in fm:
   alg,ratios,levels,role=fm[name]
   p.update(engine=5,wave=4,algorithm=alg,ratio=ratios[:4],detune=ratios[4],pulse=ratios[5],op_level=levels[:4],sub=levels[4],mix=levels[5],shape=64,cutoff=127,envdepth=0,position=54,inharm=28,drive=0,feedback=18 if role in ['reed','metal'] else 0,lfo_depth=0,lfo_dest=2)
   if role=='pad':
    envs=[d.env(900,3500,104,2300),d.env(1400,2800,78,1700),d.env(1300,4100,100,2600),d.env(700,3200,70,1500),d.env(1800,5000,96,2900),d.env(2000,3700,60,2100)]
    p.update(lfo_depth=5,lfo_rate=34,position=36,shape=68)
   elif role in ['reed','brass']:
    envs=[d.env(28,600,110,280),d.env(42,700,86,220),d.env(38,800,102,340),d.env(60,480,73,180),d.env(80,1100,94,330),d.env(110,900,64,270)]
    p.update(lfo_depth=4 if role=='reed' else 0,lfo_rate=60,shape=60 if role=='reed' else 65)
   elif role in ['bell','metal']:
    envs=[d.env(1,2600,0,1600),d.env(1,1800,0,900),d.env(1,1900,0,1100),d.env(1,1200,0,700),d.env(1,1500,0,850),d.env(1,650,0,500)]
    p.update(shape=67,inharm=50)
   elif role in ['wood','clav']:
    envs=[d.env(1,350 if role=='wood' else 650,0,180),d.env(1,70,0,65),d.env(1,230,0,120),d.env(1,42,0,40),d.env(1,180,0,95),d.env(1,28,0,35)]
    p.update(shape=64,inharm=42)
   else:
    envs=[d.env(2,1800,0,700),d.env(1,650,12,180),d.env(1,900,0,400),d.env(1,220,0,120),d.env(1,2200,0,800),d.env(1,95,0,80)]
    if role=='keys':envs[0]=d.env(2,1200,72,420);envs[2]=d.env(2,900,58,350)
    if role=='piano':envs[1]=d.env(1,1100,0,260);envs[3]=d.env(1,320,0,160)
   p['op_env']=envs[:4];p['fenv']=envs[4];p['amp']=envs[5];p['morph']=[0,22,0,0,0,0]
   x['engine']='FM6';x['name']=rename.get(name,name)
   x['description']=f'Original six-operator {role} voice: native logarithmic envelopes, keyboard/velocity scaling, algorithm {alg+1}, and separate body/transient operators.'
 return rows
def main():
 rows=bank();trims=R/'assets/wanderer_levels_1_7.json'
 if trims.exists():
  levels=json.loads(trims.read_text())
  for x in rows:x['patch']['level']=levels.get(x['name'],90)
 prior_header=(R/'firmware/src/wanderer_sound_data.h').read_text().split("/* Wanderer 2.0 factory bank */")[0];tables=prior_header[prior_header.index('/* 1..8000 ms;'):].rstrip()
 def c(v):return '{'+','.join(c(a) for a in v)+'}' if isinstance(v,list) else str(v)
 fields=list(rows[0]['patch']);header=[d.TYPE.replace('WS_ENGINES 5u','WS_ENGINES 6u'),'#define WS_PRESET_COUNT 150u','static const char * const WS_NAMES[WS_PRESET_COUNT]={'+','.join(json.dumps(x['name']) for x in rows)+'};','static const ws_patch_t WS_PRESETS[WS_PRESET_COUNT]={']
 header+=['{'+','.join(c(x['patch'][f]) for f in fields)+'},' for x in rows];header+=['};',tables]
 (R/'firmware/src/wanderer_sound_data.h').write_text('\n'.join(header)+'\n');(R/'assets/wanderer_presets_1_7.json').write_text(json.dumps(rows,indent=2)+'\n')
 (R/'build/bank-1.7').mkdir(parents=True,exist_ok=True);(R/'build/bank-1.7-registers.inc').write_text(','.join(str(x['register']) for x in rows)+'\n')
 print('150 presets; '+str(sum(x['patch']['engine']==5 for x in rows))+' authored MSFA FM6 voices, band-limited sync/ring and brighter ensembles')
if __name__=='__main__':main()
