#!/usr/bin/env python3
"""Produce the screenshot gallery and self-contained accessible release guide."""
from pathlib import Path
import base64,html,re,sys
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parent.parent
out=Path(sys.argv[1]) if len(sys.argv)>1 else root/'build/delivery'
out.mkdir(parents=True,exist_ok=True)
screens=root/'build/screens'
for p in sorted(screens.glob('*.ppm')):
    Image.open(p).resize((720,720),Image.Resampling.NEAREST).save(p.with_suffix('.png'))
files=sorted(p for p in screens.glob('*.png') if p.stem[:2].isdigit());assert len(files)==66
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',19)
head=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',30)
sheet=Image.new('RGB',(1500,100+((len(files)+2)//3)*540),'#071318');d=ImageDraw.Draw(sheet)
d.text((30,25),'WANDERER 1.2 • RC1 • Every major page and state',fill='#f0f9f5',font=head)
figures=[]
for i,p in enumerate(files):
    x=(i%3)*500;y=100+(i//3)*540
    sheet.paste(Image.open(p).resize((460,460),Image.Resampling.NEAREST),(x+20,y))
    label=p.stem[3:].replace('-',' ')
    d.text((x+20,y+468),label,fill='#b7e9e3',font=font)
    data=base64.b64encode(p.read_bytes()).decode()
    figures.append(f'<figure><img width="720" height="720" loading="lazy" src="data:image/png;base64,{data}" alt="Wanderer 1.2: {html.escape(label)}"><figcaption>{html.escape(label)}</figcaption></figure>')
sheet.save(out/'Wanderer-1.2-Screenshot-Gallery.png')

def inline(s):
    s=html.escape(s)
    s=re.sub(r'`([^`]+)`',r'<code>\1</code>',s)
    s=re.sub(r'\[([^]]+)\]\((https?://[^)]+)\)',r'<a href="\2">\1</a>',s)
    return s

def render(md,prefix):
    lines=md.splitlines();parts=[];toc=[];para=[];table=[]
    def flush():
        if para:parts.append('<p>'+inline(' '.join(para))+'</p>');para.clear()
    def flush_table():
        if not table:return
        rows=[[c.strip() for c in row.strip('|').split('|')] for row in table]
        parts.append('<div class="tablewrap"><table><thead><tr>'+''.join('<th scope="col">'+inline(c)+'</th>' for c in rows[0])+'</tr></thead><tbody>')
        for row in rows[2:]:parts.append('<tr>'+''.join('<td>'+inline(c)+'</td>' for c in row)+'</tr>')
        parts.append('</tbody></table></div>');table.clear()
    for line in lines+['']:
        if line.startswith('|'):
            flush();table.append(line);continue
        flush_table()
        if line.startswith('#'):
            flush();level=len(line)-len(line.lstrip('#'));title=line[level:].strip();anchor=prefix+'-'+re.sub('[^a-z0-9]+','-',title.lower()).strip('-')
            parts.append(f'<h{min(level+1,4)} id="{anchor}">{inline(title)}</h{min(level+1,4)}>')
            if level<=2:toc.append((anchor,title))
        elif not line.strip():flush()
        else:para.append(line.strip())
    return ''.join(parts),toc

sections=[];toc=[]
for j,name in enumerate(['USER_GUIDE_1.2.md','UI_REVIEW_1.2.md','INSTALL_AND_RECOVERY.md','CHANGELOG_1.2.md','ANTICIPATED_FEEDBACK_1.2.md','TEST_REPORT_1.2.md','WANDERER_PRESETS.md']):
    content,links=render((root/'docs'/name).read_text(),f'd{j}');toc.extend(links)
    sections.append('<section class="contentblock">'+content+'</section>')
nav=''.join(f'<a href="#{a}">{html.escape(t)}</a>' for a,t in toc)
css='''
:root{color-scheme:dark;--bg:#071318;--fg:#f0f9f5;--panel:#102b33;--line:#83bdb5;--link:#9af5e6;font-size:22px}
*{box-sizing:border-box}html{scroll-padding-top:6rem}body{margin:0;background:var(--bg);color:var(--fg);font:1rem/1.65 Arial,sans-serif}
body.light{color-scheme:light;--bg:#fffdf7;--fg:#102b33;--panel:#eff3f2;--line:#436d6a;--link:#005e60}
body.contrast{--bg:#000;--fg:#fff;--panel:#000;--line:#fff;--link:#ffec80}
header,main,nav{max-width:1150px;margin:auto;padding:1rem}header{padding-top:6rem}h1{font-size:2rem;line-height:1.2}h2{font-size:1.6rem;margin-top:2rem}h3{font-size:1.3rem;margin-top:2rem}a{color:var(--link)}
nav{display:flex;flex-wrap:wrap;gap:.5rem}nav a{padding:.45rem .75rem;border:1px solid var(--line);border-radius:8px}a:focus-visible,button:focus-visible,input:focus-visible{outline:4px solid #ffcb71;outline-offset:3px}
#toolbar{position:fixed;inset:0 0 auto;background:var(--panel);border-bottom:2px solid var(--line);padding:.5rem;display:flex;gap:.4rem;flex-wrap:wrap;z-index:4;justify-content:center}
button,input{font:inherit;padding:.3rem .6rem;background:var(--bg);color:var(--fg);border:1px solid var(--line);border-radius:6px}input{width:10rem}button{cursor:pointer}code{font-size:.9em;overflow-wrap:anywhere}
.tablewrap{overflow-x:auto}table{border-collapse:collapse;width:100%;margin:1.5rem 0}th,td{border:1px solid var(--line);padding:.6rem;vertical-align:top;text-align:left}th{background:var(--panel)}
.gallery{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:1.5rem}figure{margin:0;background:var(--panel);padding:.6rem;border:1px solid var(--line)}img{display:block;width:100%;height:auto;image-rendering:pixelated}figcaption{padding:.5rem .1rem;font-weight:bold;text-transform:capitalize}
.skip{position:fixed;left:1rem;top:-5rem;z-index:10;background:var(--bg);padding:1rem}.skip:focus{top:0}[hidden]{display:none!important}
@media(max-width:700px){:root{font-size:20px}.gallery{grid-template-columns:1fr}header{padding-top:9rem}html{scroll-padding-top:9rem}nav{display:block}nav a{display:block;margin:.5rem 0}}
@media print{#toolbar,.skip{display:none}header{padding-top:0}body{--bg:#fff;--fg:#000;--panel:#eee;--line:#333;--link:#000}.gallery{grid-template-columns:repeat(2,1fr)}figure{break-inside:avoid}}
'''
script='''
let size=22;const body=document.body;
document.getElementById('bigger').onclick=()=>{size=Math.min(40,size+2);document.documentElement.style.fontSize=size+'px'};
document.getElementById('smaller').onclick=()=>{size=Math.max(16,size-2);document.documentElement.style.fontSize=size+'px'};
document.getElementById('theme').onclick=()=>{body.classList.toggle('light');document.getElementById('theme').setAttribute('aria-pressed',body.classList.contains('light'))};
document.getElementById('contrast').onclick=()=>{body.classList.toggle('contrast');document.getElementById('contrast').setAttribute('aria-pressed',body.classList.contains('contrast'))};
document.getElementById('find').oninput=e=>{let q=e.target.value.toLowerCase();document.querySelectorAll('.contentblock,figure').forEach(n=>n.hidden=!n.textContent.toLowerCase().includes(q));document.getElementById('results').textContent=q?'Showing matching sections and screenshots. Clear search to show everything.':''};
'''
def page(title,content,nav=''):
    return f'''<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>{title}</title><style>{css}</style></head><body><a class="skip" href="#content">Skip to content</a><div id="toolbar" role="region" aria-label="Reading controls"><button id="smaller" aria-label="Decrease text size">A−</button><button id="bigger" aria-label="Increase text size">A+</button><button id="theme" aria-pressed="false">Light theme</button><button id="contrast" aria-pressed="false">High contrast</button><label>Find <input id="find" type="search" aria-label="Filter sections and screenshots"></label><a href="#top">Top</a></div><header id="top"><h1>{title}</h1><p>Installable 1.2 RC2. Physical FM-1 acceptance remains pending. These are captures from the production renderer, enlarged from 240 × 240 pixels.</p><p id="results" role="status" aria-live="polite"></p></header><nav aria-label="Contents">{nav}</nav><main id="content">{content}</main><script>{script}</script></body></html>'''
audio_data=base64.b64encode((root/'build/Wanderer-1.2-Recorded-Chords.mp3').read_bytes()).decode()
audio_section='<section class="contentblock"><h2 id="listen">Listen to the recorded chords</h2><p>A two-bar three-note phrase captured through the production MIDI recording path, saved and reloaded, then rendered by the sequencer with Slow Aurora and Electric Mist. The half-second pauses separate the examples. Playback starts only when you choose Play.</p><audio controls preload="none" aria-label="Wanderer 1.2 recorded chord showcase" style="width:100%" src="data:audio/mpeg;base64,'+audio_data+'"></audio></section>'
gallery='<section class="contentblock"><h2 id="screenshots">All sixty-six screens</h2><p>Home, playback, recording, Anchor, zoom, both themes, System, Effects, Performance, Music, two-bar timelines, held durations, parameter details, confirmations, persistent results/errors, live/frozen/silent oscilloscope and both startup splash themes.</p></section><div class="gallery">'+''.join(figures)+'</div>'
(out/'Wanderer-1.2-Guide.html').write_text(page('Wanderer 1.2 RC2 — guide and screenshots',audio_section+''.join(sections)+gallery,'<a href="#listen">Audio showcase</a>'+nav+'<a href="#screenshots">Screenshots</a>'))
(out/'Wanderer-1.2-Screenshots.html').write_text(page('Wanderer 1.2 — every major screen',gallery))
print('66 PNG captures, labelled gallery, standalone guide and screenshot viewer created')
