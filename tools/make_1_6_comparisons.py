#!/usr/bin/env python3
"""Listening A/B with RMS matched over the held portion, never peak boosted."""
import json,subprocess,wave
from pathlib import Path
import numpy as np
R=Path(__file__).resolve().parents[1];O=R/'deliverables-1.6-RC1';O.mkdir(exist_ok=True)
rows=json.loads((R/'assets/wanderer_presets_1_6.json').read_text())
picks=[20,40,54,91,100,106,120,130,138]
audio=[];order=[];t=0
for p in picks:
 clips=[np.fromfile(R/f'build/bank-{v}/{p+1:03}.raw',dtype='<i2').astype(np.float64) for v in ['1.5','1.6']]
 assert all(len(a)==44100*3*2 for a in clips)
 rms=[np.sqrt(np.mean(a[:44100*2*2]**2)) for a in clips];target=min(2500,*rms)
 for v,a,r in zip(['1.5','1.6'],clips,rms):
  order.append(f'{t:05.1f}s  {v}  {p+1:03} {rows[p]["name"]}')
  audio.append(np.rint(a*(target/max(1,r))).astype('<i2').tobytes());audio.append(bytes(4410*4));t+=3.1
 audio.append(bytes(13230*4));t+=.3
raw=b''.join(audio);wav=O/'Wanderer-1.6-RC1-Before-After.wav'
with wave.open(str(wav),'wb') as f:f.setparams((2,2,44100,0,'NONE','not compressed'));f.writeframes(raw)
subprocess.run(['ffmpeg','-y','-loglevel','error','-i',str(wav),'-codec:a','libmp3lame','-b:a','192k',str(wav.with_suffix('.mp3'))],check=True)
(O/'Comparison-Order.txt').write_text('Each pair: 1.5 then 1.6. Velocity 110, two seconds held and one released, dry. RMS matched over the held portion, with attenuation only. Listening comparison, not a quality score.\n\n'+'\n'.join(order)+'\n')
g=O/'Wanderer-1.6-RC1-G-Neighbours.wav'
with wave.open(str(g),'wb') as f:f.setparams((2,2,44100,0,'NONE','not compressed'));f.writeframes((R/'build/G-neighbours-1.6.raw').read_bytes())
subprocess.run(['ffmpeg','-y','-loglevel','error','-i',str(g),'-codec:a','libmp3lame','-b:a','192k',str(g.with_suffix('.mp3'))],check=True)
(O/'G-Neighbours-Order.txt').write_text('Each engine plays F3, F#3, G3, G#3, A3, with G3 third.\n0.0s Analog: Velvet Saw\n3.5s FM: Felt Tines\n7.0s Phase: Phase Reed\n10.5s String: Cedar Pluck\n14.0s Modal: Bronze Bowl\nFixed velocity 100, Melt zero, effects off, factory levels.\n')
print('Nine RMS-matched old/new pairs and five-engine G-neighbour comparisons created')
