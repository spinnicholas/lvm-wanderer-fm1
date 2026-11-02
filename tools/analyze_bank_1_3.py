#!/usr/bin/env python3
from pathlib import Path
import csv,json,wave
import numpy as np
R=Path(__file__).resolve().parents[1];O=R/'build';rows=list(csv.DictReader((O/'bank-1.3.csv').open()))
x=np.fromfile(O/'bank-1.3.raw',dtype='<i2').reshape(100,44100,2).mean(axis=2)
assert np.isfinite(x).all() and abs(x).max()<32767
rms=np.sqrt((x*x).mean(axis=1));assert (rms>2).all()
# Compare coarse log spectral energy and five-window envelope, rather than phase/random seeds.
f=np.fft.rfftfreq(8192,1/44100);edges=np.geomspace(40,20000,49);features=[]
for clip in x:
 spec=np.zeros(4097)
 for start in [0,4096,8192,16384]:spec+=abs(np.fft.rfft(clip[start:start+8192]*np.hanning(8192)))**2
 bands=np.array([spec[(f>=a)&(f<b)].sum() for a,b in zip(edges[:-1],edges[1:])]);bands=np.sqrt(bands/(bands.sum()+1))
 features.append(bands)
y=np.array(features);sim=y@y.T;np.fill_diagonal(sim,-1)
nearest=[{'preset':i+1,'name':rows[i]['name'],'rms':round(float(rms[i]),1),'peak':int(rows[i]['peak']),'nearest':int(sim[i].argmax()+1),'spectral_cosine':round(float(sim[i].max()),4)} for i in range(100)]
report={'all_audible':True,'all_bounded':True,'min_rms':round(float(rms.min()),1),'max_rms':round(float(rms.max()),1),'peak':int(abs(x).max()),'nearest_spectral_matches':nearest,'limits':'Spectral similarity is diagnostic, not a subjective sound-quality rating. Decay, attack, modulation and playing register also distinguish patches.'}
(O/'bank-1.3-analysis.json').write_text(json.dumps(report,indent=2)+'\n')
with wave.open(str(O/'Wanderer-1.3-All-100-Presets.wav'),'wb') as w:w.setnchannels(2);w.setsampwidth(2);w.setframerate(44100);w.writeframes((O/'bank-1.3.raw').read_bytes())
picks=[0,1,2,7,20,21,24,30,40,41,44,49,60,61,62,65,80,81,85,99]
stereo=np.fromfile(O/'bank-1.3.raw',dtype='<i2').reshape(100,44100,2)
with wave.open(str(O/'Wanderer-1.3-Sound-Sampler.wav'),'wb') as w:
 w.setnchannels(2);w.setsampwidth(2);w.setframerate(44100)
 for i in picks:w.writeframes(stereo[i].tobytes());w.writeframes(np.zeros((4410,2),dtype='<i2').tobytes())
(O/'sampler-order.json').write_text(json.dumps([dict(index=i+1,name=rows[i]['name']) for i in picks],indent=2)+'\n')
print(json.dumps({k:v for k,v in report.items() if k!='nearest_spectral_matches'},indent=2))
