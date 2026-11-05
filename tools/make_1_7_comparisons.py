#!/usr/bin/env python3
"""Listening A/B with RMS matched over the held portion, never peak boosted."""
import json,subprocess,wave
from pathlib import Path
import numpy as np
R=Path(__file__).resolve().parents[1];O=R/'deliverables-1.7-RC1';O.mkdir(exist_ok=True)
rows=json.loads((R/'assets/wanderer_presets_1_7.json').read_text())
picks=[23,37,40,42,43,44,65,70,86,138]
audio=[];order=[];t=0
for p in picks:
 clips=[np.fromfile(R/f'build/bank-{v}/{p+1:03}.raw',dtype='<i2').astype(np.float64) for v in ['1.6','1.7']]
 assert all(len(a)==44100*3*2 for a in clips)
 rms=[np.sqrt(np.mean(a[:44100*2*2]**2)) for a in clips];target=min(2500,*rms)
 for v,a,r in zip(['1.6','1.7'],clips,rms):
  order.append(f'{t:05.1f}s  {v}  {p+1:03} {rows[p]["name"]}')
  audio.append(np.rint(a*(target/max(1,r))).astype('<i2').tobytes());audio.append(bytes(4410*4));t+=3.1
 audio.append(bytes(13230*4));t+=.3
raw=b''.join(audio);wav=O/'Wanderer-1.7-RC1-Before-After.wav'
with wave.open(str(wav),'wb') as f:f.setparams((2,2,44100,0,'NONE','not compressed'));f.writeframes(raw)
subprocess.run(['ffmpeg','-y','-loglevel','error','-i',str(wav),'-codec:a','libmp3lame','-b:a','192k',str(wav.with_suffix('.mp3'))],check=True)
(O/'Comparison-Order.txt').write_text('Each pair: 1.6 then 1.7. Velocity 110, two seconds held and one released, dry. RMS matched over the held portion, with attenuation only. Listening comparison, not a quality score.\n\n'+'\n'.join(order)+'\n')
g=O/'Wanderer-1.7-RC1-Effects.wav'
with wave.open(str(g),'wb') as f:f.setparams((2,2,44100,0,'NONE','not compressed'));f.writeframes((R/'build/effects-1.7.raw').read_bytes())
subprocess.run(['ffmpeg','-y','-loglevel','error','-i',str(g),'-codec:a','libmp3lame','-b:a','192k',str(g.with_suffix('.mp3'))],check=True)
(O/'Effects-Order.txt').write_text('Each pair: three seconds dry, four seconds wet, with a short gap.\n0.00s Bitcrush — Sync Solo\n7.02s Tape Echo — Stage Piano\n14.04s Stereo Phaser — Silk Strings\n21.06s Wavefolder — Dust Chords\nNo loudness normalization on the effect examples.\n')
print('Ten 1.6/1.7 level-matched comparisons and four dry/wet effect examples created')
