"""Check emulated output; captures still require visual review of the actual LCD."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import wave,struct,math,json,hashlib
root=Path(__file__).resolve().parents[1]
out=root/'build/compiled';log=(root/'build/evidence-1.2/compiled.log').read_text()
assert 'time 7000 ms' in log and 'halt:' not in log and 'display_on=true' in log
with wave.open(str(out/'compiled-chords.wav')) as w:
    frames=w.getnframes();rate=w.getframerate();x=struct.unpack('<'+'h'*frames*2,w.readframes(frames))
assert frames>250000 and rate==44100 and 100<max(map(abs,x))<32767
labels={1800:'Recording held chord',4100:'Save destination',4400:'Verified Save result',5300:'Verified Load result',6400:'Anchor chord playback',6800:'HOME return with live key'}
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',20)
sheet=Image.new('RGB',(1800,610),'#071318');d=ImageDraw.Draw(sheet)
for i,(ms,label) in enumerate(labels.items()):
    im=Image.open(out/f'work/lcd_{ms}.bmp').convert('RGB');assert im.size==(240,240)
    assert len(im.getcolors(100000))>10
    im.resize((720,720),Image.Resampling.NEAREST).save(out/f'Wanderer-1.2-Compiled-{ms}.png')
    sheet.paste(im.resize((280,280),Image.Resampling.NEAREST),(i*300+10,0))
    for j,line in enumerate(label.split(' ',1)):d.text((i*300+10,300+30*j),line,font=font,fill='white')
sheet=sheet.crop((0,0,1800,380));sheet.save(out/'Wanderer-1.2-Compiled-Review.png')
r={'firmware':'FM-1_932','package_sha256':hashlib.sha256((root/'build/dinghy-1.2-rc2.fwsc').read_bytes()).hexdigest(),'emulated_ms':7000,'frames':frames,'rate':rate,'peak':max(map(abs,x)),'rms':round(math.sqrt(sum(v*v for v in x)/len(x)),2),'captures':list(labels),'physical_hardware':'pending','limitations':'Emulation and unknown MMIO are not physical timing/flash/DAC qualification.'}
(root/'build/evidence-1.2/compiled.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r,indent=2))
