"""Check RGB565 palette contrast and that the actual menu uses those colors."""
from pathlib import Path
import json
from PIL import Image,ImageDraw
R=Path(__file__).resolve().parents[1];O=R/'build/rc3'
def rgb(v):return ((v>>11)*255//31,((v>>5)&63)*255//63,(v&31)*255//31)
def luminance(v):
 c=[x/255 for x in rgb(v)];c=[x/12.92 if x<=.04045 else ((x+.055)/1.055)**2.4 for x in c]
 return .2126*c[0]+.7152*c[1]+.0722*c[2]
def ratio(a,b):
 a,b=sorted([luminance(a),luminance(b)]);return (b+.05)/(a+.05)
results=[];canvas=Image.new('RGB',(768,584),'#101010');draw=ImageDraw.Draw(canvas)
for line in (O/'theme.log').read_text().splitlines():
 if not line.startswith('PALETTE '):continue
 a=line.split();t=int(a[1]);bg,pa,pm,fg,label,*acc=map(int,a[3:]);im=Image.open(O/f'theme-{t}.ppm')
 counts=im.getcolors(240*240);colors={color:count for count,color in counts}
 for role in [bg,pa,fg,label]:assert colors.get(rgb(role),0)>=20,(a[2],role,'palette not rendered')
 minimum=min(ratio(x,y) for x in [fg,label] for y in [bg,pa,pm]);accent=min(ratio(x,y) for x in acc for y in [bg,pa,pm])
 assert minimum>=7,(a[2],minimum);assert accent>=3,(a[2],accent)
 results.append(dict(name=a[2],minimum_text_contrast=round(minimum,2),minimum_accent_contrast=round(accent,2)))
 im.save(O/f'theme-{t}.png');x=(t%3)*256+8;y=(t//3)*292+30;canvas.paste(im,(x,y));draw.text((x,y-22),a[2],fill='white')
assert len(results)==6
canvas.save(O/'theme-preview.png');(O/'contrast.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results))
