#!/usr/bin/env python3
"""Build the offline preset book and unnormalized production-audio previews."""
import base64
import csv
import html
import io
import json
import subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

R=Path(__file__).resolve().parents[1]
O=R/'deliverables-1.7-RC1'
O.mkdir(exist_ok=True)
A=R/'build/bank-1.7'
rows=json.loads((R/'assets/wanderer_presets_1_7.json').read_text())
def encode(row):
    src=A/f"{row['number']:03d}.raw";out=src.with_suffix('.mp3')
    subprocess.run(['ffmpeg','-y','-loglevel','error','-f','s16le','-ar','44100','-ac','2',
        '-i',str(src),'-codec:a','libmp3lame','-b:a','128k',str(out)],check=True)
with ThreadPoolExecutor(max_workers=4) as pool:list(pool.map(encode,rows))
def data(path,mime):return 'data:'+mime+';base64,'+base64.b64encode(path.read_bytes()).decode()

for x in rows:
    x['audio']=data(A/f"{x['number']:03d}.mp3",'audio/mpeg')

banks=list(dict.fromkeys(x['bank'] for x in rows))
styles='''
:root{color-scheme:dark;--bg:#09171e;--fg:#edf8f5;--muted:#b0c9cf;--accent:#8ce8da;--line:#54747c;--card:#11252f;font-size:22px}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--fg);font:1rem/1.7 system-ui,sans-serif}
main{max-width:1040px;margin:auto;padding:1.2rem}a{color:var(--accent)}h1,h2,h3{line-height:1.2;overflow-wrap:anywhere}h1{font-size:2.5rem;margin:.6rem 0}h2{margin-top:2.3rem}h3{margin:.2rem 0 .6rem;font-size:1.3rem}
button,input,select{font:inherit;color:inherit;background:var(--bg);border:2px solid var(--line);border-radius:8px;padding:.5rem .7rem;min-height:48px;max-width:100%}button{cursor:pointer}button:hover{border-color:var(--accent)}:focus-visible{outline:4px solid #ffd581;outline-offset:4px}
.tools{position:sticky;top:0;z-index:2;background:var(--bg);border-bottom:2px solid var(--line);padding:.5rem;display:flex;gap:.4rem;flex-wrap:wrap}.tools button{font-size:.9rem}p{max-width:74ch}.eyebrow,.meta{color:var(--muted)}.intro{font-size:1.1rem}.filters{display:grid;gap:.8rem;grid-template-columns:repeat(auto-fit,minmax(230px,1fr));margin:1.3rem 0}label{display:flex;flex-direction:column;gap:.3rem}.preset{padding:1.2rem;border:2px solid var(--line);border-radius:14px;background:var(--card);margin:1rem 0;scroll-margin-top:6rem}.preset p{margin:.5rem 0}.number{font-variant-numeric:tabular-nums;color:var(--accent)}audio{width:100%;max-width:650px;margin-top:.6rem}img{display:block;width:100%;height:auto}.review{max-width:760px;margin:1.2rem auto}.jump{display:flex;gap:.5rem;flex-wrap:wrap}.jump a{border:1px solid var(--line);padding:.3rem .6rem;border-radius:7px}summary{cursor:pointer;padding:.5rem 0}footer{border-top:2px solid var(--line);padding:1rem;margin-top:2rem}table{border-collapse:collapse;width:100%;font-size:.9rem}td,th{border-bottom:1px solid var(--line);padding:.5rem;text-align:left}code{overflow-wrap:anywhere}@media(max-width:500px){:root{font-size:20px}main{padding:1rem}h1{font-size:2rem}.preset{padding:.9rem}}
'''
body='''<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Wanderer 1.7 · 150 sounds</title><style>'''+styles+'''</style></head><body>
<nav class="tools" aria-label="Reading controls"><button id="bigger">Larger text</button><button id="smaller">Smaller text</button><button id="theme">Light / dark</button><a href="#catalog">Preset catalog</a></nav>
<main><p class="eyebrow">LOW VISION MUSIC · FM-1 · 1.7 RC1</p><h1>150 sounds.<br>Rebuilt from the voice up.</h1>
<p class="intro">A new factory bank built around full-bodied basses and leads, playable keys, sustained brass and strings, slow pads, physical plucks, resonant mallets, bells and tuned percussion.</p>
<p>The collection uses 39 analog, 25 phase-distortion, 32 string, 30 modal, four four-operator FM and twenty native MSFA six-operator FM patches. Each sound has a deliberate envelope, register and Melt response. The designs use the firmware’s synthesis engines and are not sampled instruments or exact replicas of other synthesizers.</p>
<h2>Finding a sound</h2><p>PRESET moves one sound at a time and wraps between 1 and 150. SELECT moves through fifteen groups of ten, keeping the position within each group. The fifteen marks at the bottom of the sound-name screen show the current group. The same controls work in Zoom.</p>
<p>Hold the key on pads, strings, brass, organs and sustained basses. Plucks and struck sounds naturally darken or decay while you hold. Start with Melt down to hear the original recipe, then bring it up to explore its assigned changes. Scatter and effects stay under your existing controls; choosing a preset does not replace your effect or arpeggiator settings.</p>
<h2>Saved projects</h2><p>Older project files import without rewriting their original slots. Their saved patch values, notes and Anchor snapshots remain intact, but the expanded effects change wet playback, and browsing the new factory bank selects its revised voices. Older factory definitions remain available for imported projects. New saves use v11 and cannot be read by 1.6 or earlier; save into spare slots. Keep the earlier firmware for exact earlier playback.</p>
<h2>MIDI selection</h2><p>Bank Select is remembered independently for each USB/TRS input and MIDI channel. Send CC0 = 0 and CC32 = 0, followed by Program Change 0–127, for presets 1–128. Send CC0 = 0 and CC32 = 1, followed by Program Change 0–21, for presets 129–150. Some controllers display program numbers one higher. Unsupported banks and programs leave the current sound unchanged. Bank Select alone does not change a sound.</p>
<h2>Listen and compare</h2><p>Every player below is a three-second render from the production firmware DSP: two seconds held, then one second of release, at velocity 110 and Master 100. Melt and effects are off. The previews have no loudness normalization. The suggested note is a starting point; try nearby octaves and four-note voicings on the FM-1. RC1 still needs your physical-device audition.</p>
<details><summary>See the current sound-name screens</summary><img class="review" src="'''+data(R/'docs/screenshots-1.7/bank-review.png','image/png')+'''" alt="Sixteen current preset views, including bass, keys, pad, string, mallet and percussion names; the last preset is Drifting Wire, number 150."></details>
<section id="catalog"><h2>Preset catalog</h2><div class="filters"><label>Search names and uses<input id="search" type="search" placeholder="Try pad, bass, soft, glass…"></label><label>Group<select id="bank"><option value="">All fifteen groups</option>'''
body+=''.join('<option>'+html.escape(b)+'</option>' for b in banks)
body+='''</select></label><label>Engine<select id="engine"><option value="">All six engines</option>'''+''.join('<option>'+e+'</option>' for e in ['Analog','FM','FM6','Phase','String','Modal'])+'''</select></label></div><p id="count" role="status" aria-live="polite">150 sounds</p><div id="sounds"></div></section>
<footer><p>Nicholas James / Low Vision Music · Rebuilt sound engines and original bank · GPL-3.0-only</p><p><a href="https://www.lowvisionmusic.info/">Low Vision Music</a></p></footer></main><script>
const presets='''+json.dumps(rows,separators=(',',':')).replace('</','<'+chr(92)+'/')+''';
const host=document.querySelector('#sounds');
function draw(){host.replaceChildren();let found=0;const q=document.querySelector('#search').value.toLowerCase(),bank=document.querySelector('#bank').value,engine=document.querySelector('#engine').value;
 for(const p of presets){if(bank&&p.bank!==bank||engine&&p.engine!==engine||![p.name,p.description,p.bank,p.engine].join(' ').toLowerCase().includes(q))continue;found++;
 const card=document.createElement('article');card.className='preset';card.id='preset-'+p.number;const title=document.createElement('h3');const num=document.createElement('span');num.className='number';num.textContent=String(p.number).padStart(3,'0')+'  ';title.append(num,document.createTextNode(p.name));card.append(title);
 for(const [text,cls] of [[p.bank+' · '+p.engine,'meta'],[p.description,''],['Suggested MIDI note: '+p.register+' · Factory level: '+p.patch.level+'/127','meta']]){const el=document.createElement('p');el.textContent=text;el.className=cls;card.append(el);}
 const audio=document.createElement('audio');audio.controls=true;audio.preload='none';audio.src=p.audio;audio.setAttribute('aria-label','Listen to '+p.name);audio.addEventListener('play',()=>document.querySelectorAll('audio').forEach(a=>{if(a!==audio)a.pause()}));card.append(audio);host.append(card);}
 document.querySelector('#count').textContent=found+' sounds';}
for(const id of ['search','bank','engine'])document.querySelector('#'+id).addEventListener('input',draw);
document.querySelector('#bigger').onclick=()=>document.documentElement.style.fontSize=Math.min(38,parseFloat(getComputedStyle(document.documentElement).fontSize)*1.12)+'px';
document.querySelector('#smaller').onclick=()=>document.documentElement.style.fontSize=Math.max(18,parseFloat(getComputedStyle(document.documentElement).fontSize)/1.12)+'px';
let light=false;document.querySelector('#theme').onclick=()=>{light=!light;const s=document.documentElement.style;for(const [k,v] of Object.entries(light?{bg:'#ffffff',fg:'#10262d',muted:'#365661',accent:'#006252',line:'#78979f',card:'#eff7f7'}:{bg:'#09171e',fg:'#edf8f5',muted:'#b0c9cf',accent:'#8ce8da',line:'#54747c',card:'#11252f'}))s.setProperty('--'+k,v);s.colorScheme=light?'light':'dark';};draw();
</script></body></html>'''
(O/'Wanderer-1.7-RC1-Preset-Book.html').write_text(body)

lines=['# Wanderer 1.7 RC1 · 150 factory sounds','','PRESET browses 1–150. SELECT moves through fifteen groups of ten. New saves use v11. Imported modern projects retain their saved patch values and earlier factory definitions but play through the rebuilt engines, so their audio changes.','','| No. | Group | Name | Engine | Suggested MIDI note | Character |','|---:|---|---|---|---:|---|']
for x in rows:lines.append(f"| {x['number']} | {x['bank']} | {x['name']} | {x['engine']} | {x['register']} | {x['description']} |")
(R/'docs/PRESETS_1.7.md').write_text('\n'.join(lines)+'\n')
with (O/'Wanderer-1.7-Preset-Catalog.csv').open('w') as f:
    writer=csv.DictWriter(f,fieldnames=['number','bank','name','engine','register','description']);writer.writeheader();writer.writerows({k:x[k] for k in writer.fieldnames} for x in rows)
# Fifteen representative bank openings plus contrasting digital and string selections.
picks=[0,3,20,30,40,44,50,60,70,80,86,91,100,110,120,130,140,149]
raw=b''.join((A/f'{i+1:03d}.raw').read_bytes()+bytes(4410*4) for i in picks)
subprocess.run(['ffmpeg','-y','-loglevel','error','-f','s16le','-ar','44100','-ac','2','-i','pipe:0',
    '-codec:a','libmp3lame','-b:a','192k',str(O/'Wanderer-1.7-RC1-Sampler.mp3')],input=raw,check=True)
(O/'Sampler-Order.txt').write_text('\n'.join(f"{k*3.1:05.1f}s  {i+1:03d} {rows[i]['name']}" for k,i in enumerate(picks))+'\n')
print('Offline preset book with 150 embedded audio previews, catalog, and 18-sound sampler created.')
