#!/usr/bin/env python3
from pathlib import Path
from PIL import Image,ImageDraw
import re,json,numpy as np
R=Path(__file__).resolve().parents[1];O=R/'build/screens-1.3';D=R/'docs/screenshots-1.3';D.mkdir(parents=True,exist_ok=True)
valid=re.compile(r'(editor-e\d-t\d-z\d-p\d\d-k\d|preset-t\d-z\d-p\d\d|print-t\d-z\d-state\d|audio-lock-t\d)$')
paths=[p for p in sorted(O.glob('*.ppm')) if valid.fullmatch(p.stem)]
for p in paths:Image.open(p).convert('RGB').save(p.with_suffix('.png'))
def sheet(files,dest,cols=5):
 rows=(len(files)+cols-1)//cols;im=Image.new('RGB',(cols*250,rows*265),'#d8e1e2');d=ImageDraw.Draw(im)
 for i,p in enumerate(files):x=i%cols*250;y=i//cols*265;im.paste(Image.open(p).convert('RGB'),(x,y));d.text((x+2,y+243),p.stem,fill='#12222a')
 im.save(dest)
for e in range(5):
 for t in range(2):
  for z in range(2):sheet(sorted(O.glob(f'editor-e{e}-t{t}-z{z}-*-k0.png')),D/f'editor-e{e}-t{t}-z{z}.png')
for t in range(2):
 for z in range(2):
  sheet(sorted(O.glob(f'print-t{t}-z{z}-state*.png')),D/f'print-t{t}-z{z}.png',4)
  presets=sorted(O.glob(f'preset-t{t}-z{z}-p*.png'))
  for j in range(4):sheet(presets[j*25:(j+1)*25],D/f'names-t{t}-z{z}-{j}.png')
# Regressions in old screen geometry are compared, with explicit intentional changes.
old=R.parent/'research/screens';now=R/'build/screens';comparison=[]
for p in sorted(now.glob('*.ppm')):
 im=Image.open(p).convert('RGB');im.save(p.with_suffix('.png'));q=old/p.with_suffix('.png').name
 if q.exists():
  ref=Image.open(q).convert('RGB')
  if ref.size!=im.size:ref=ref.resize(im.size,Image.Resampling.NEAREST)
  comparison.append(dict(name=p.stem,equal=bool(np.array_equal(np.array(im),np.array(ref)))))
(R/'build/1.3-screen-comparison.json').write_text(json.dumps(comparison,indent=2)+'\n')
report={'current_native_views':len(paths),'contact_sheets':len(list(D.glob('*.png'))),'old_comparison':comparison,'limits':'Contact sheets reviewed visually; full native captures and range/overlap tests provide finer coverage. Intentional changes include Repeat, preset names, scale names, modern scope output and corrected labels.'}
(R/'build/1.3-screen-review.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k!='old_comparison'},indent=2))
