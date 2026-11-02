#!/usr/bin/env python3
"""Inspect the final package's completed compiled-firmware emulator session."""
from pathlib import Path
from PIL import Image,ImageDraw
import hashlib,json,re,wave,numpy as np
R=Path(__file__).resolve().parents[1];D=R/'build/final-emulator-1.3';log=(D/'compiled.log').read_text();assert 'time 8800 ms' in log and 'display_on=true' in log and 'halt:' not in log
fw=D/'firmware.fwsc';assert fw.read_bytes()==(R/'dist/LVM-Wanderer-1.3-RC1-FM1.fwsc').read_bytes()
with wave.open(str(D/'compiled-print.wav')) as w:
 assert w.getframerate()==44100 and w.getnchannels()==2
 samples=np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').reshape(-1,2)
assert len(samples)>370000 and 100<abs(samples.astype(np.int32)).max()<32767
# Production screenshots distinguish Print from an empty/no-phrase error scene.
# Expected labels are verified visually on the contact sheet; hashes preserve exact reviewed bytes.
labels={1800:'Tempo and four FM voices',3700:'Print page',4500:'First Print in progress',5500:'Completed / next Print',7300:'Second Print in progress',8170:'Undo completed audio',8500:'Transport stopped'}
images=[];captures=[]
for ms,label in labels.items():
 p=D/f'work/lcd_{ms}.bmp';im=Image.open(p).convert('RGB');assert im.size==(240,240) and len(im.getcolors(100000))>10
 out=R/f'docs/screenshots-1.3/compiled-{ms}.png';im.save(out);images.append((im,label));captures.append(dict(ms=ms,label=label,sha256=hashlib.sha256(p.read_bytes()).hexdigest()))
sheet=Image.new('RGB',(4*250,2*280),'#d8e1e2');draw=ImageDraw.Draw(sheet)
for i,(im,label) in enumerate(images):x=i%4*250;y=i//4*280;sheet.paste(im,(x,y));draw.text((x+3,y+242),label,fill='#12222a')
sheet.save(R/'docs/screenshots-1.3/compiled-review.png')
cpu=re.search(r'cpu0: ([0-9.]+)% in an ISR',log)
report=dict(firmware_sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),emulator='fm1-emu 0.3.0',emulated_ms=8800,frames=len(samples),peak=int(abs(samples.astype(np.int32)).max()),rms=round(float(np.sqrt(np.mean(samples.astype(float)**2))),2),average_isr_percent=float(cpu[1]) if cpu else None,captures=captures,physical_hardware=False,limitations='Emulated average ISR instruction load is not a physical worst-case audio deadline measurement. The emulator reports incomplete MMIO modeling; installation/DAC/hardware LED behavior remains unqualified.')
(R/'build/1.3-compiled.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k!='captures'},indent=2))
