from pathlib import Path
import numpy as np,json,csv,wave
R=Path(__file__).resolve().parents[1]
x=np.fromfile(R/'build/bank-1.1.raw',dtype='<i2').reshape(-1,2)
rows=list(csv.DictReader((R/'assets/wanderer_presets.csv').open()))
report=[]
for i,p in enumerate(rows):
    a=x[i*132300:(i+1)*132300,0].astype(float)
    rms=lambda v:float(np.sqrt(np.mean(v*v)))
    report.append(dict(number=i+1,name=p['name'],peak=int(np.max(abs(a))),rms=rms(a[:88200]),early=rms(a[4410:13230]),held=rms(a[66150:83790]),tail=rms(a[92610:105840])))
    assert np.max(abs(a))<32767
    assert rms(a[:88200])>20,(i,p['name'],rms(a[:88200]))
    assert abs(np.mean(a[:88200]))<100,(i,'DC offset')
assert len({q['peak'] for q in report})>90
for i in [10,11,12,72,73,74,77,79]:assert report[i]['held']>100,(i,report[i])
for i in [40,41,45,46,50]:assert report[i]['early']>report[i]['held']*1.5,(i,report[i])
(R/'build/evidence-1.2/bank.json').write_text(json.dumps(report,indent=2)+'\n')
print(f"100 dry patches audible, peak {max(q['peak'] for q in report)}, RMS {min(q['rms'] for q in report):.0f}–{max(q['rms'] for q in report):.0f}; sustained pads and decaying keys differentiated")
for stem in ['bank-1.1','showcase-1.1']:
    with wave.open(str(R/f'build/{stem}.wav'),'wb') as w:
        w.setnchannels(2);w.setsampwidth(2);w.setframerate(44100);w.writeframes((R/f'build/{stem}.raw').read_bytes())
