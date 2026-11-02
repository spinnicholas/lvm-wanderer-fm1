#!/usr/bin/env python3
"""Self-contained accessible HTML from the release's Markdown, with native LCD examples."""
from pathlib import Path
import re,html,base64
from html.parser import HTMLParser
R=Path(__file__).resolve().parents[1];O=R/'artifacts';O.mkdir(exist_ok=True)
def inline(s):
 tokens=[];pos=0
 for m in re.finditer(r'\[([^\]]+)\]\(([^)]+)\)|\*\*([^*]+)\*\*|`([^`]+)`',s):
  tokens.append(html.escape(s[pos:m.start()]));tokens.append('<a href="'+html.escape(m[2],quote=True)+'">'+html.escape(m[1])+'</a>' if m[1] else '<strong>'+html.escape(m[3])+'</strong>' if m[3] else '<code>'+html.escape(m[4])+'</code>');pos=m.end()
 return ''.join(tokens)+html.escape(s[pos:])
def render(source):
 lines=source.splitlines();blocks=[];nav=[];para=[];i=0;section=False;listtag=None
 def flush():
  if para:blocks.append('<p>'+inline(' '.join(para))+'</p>');para.clear()
 def close_list():
  nonlocal listtag
  if listtag:blocks.append('</'+listtag+'>');listtag=None
 while i<len(lines):
  line=lines[i]
  if line.startswith('# '):flush();close_list();blocks.append('<h1>'+inline(line[2:])+'</h1>')
  elif line.startswith('## '):
   flush();close_list()
   if section:blocks.append('</section>')
   label=line[3:];ident='section-'+re.sub('[^a-z0-9]+','-',label.lower()).strip('-');nav.append('<a href="#'+ident+'">'+html.escape(label)+'</a>');blocks.append('<section id="'+ident+'" class="chapter"><h2>'+inline(label)+'</h2>');section=True
  elif line.startswith('|'):
   flush();close_list();rows=[]
   while i<len(lines) and lines[i].startswith('|'):
    cells=[x.strip() for x in lines[i].strip().strip('|').split('|')]
    if not all(re.fullmatch(':?-+:?',x) for x in cells):rows.append(cells)
    i+=1
   blocks.append('<div class="table-wrap" tabindex="0" role="region" aria-label="Scrollable reference table"><table><thead><tr>'+''.join('<th scope="col">'+inline(c)+'</th>' for c in rows[0])+'</tr></thead><tbody>'+''.join('<tr>'+''.join('<td>'+inline(c)+'</td>' for c in row)+'</tr>' for row in rows[1:])+'</tbody></table></div>');continue
  elif re.match(r'(- |\d+\. )',line):
   flush();tag='ul' if line.startswith('- ') else 'ol'
   if listtag!=tag:close_list();blocks.append('<'+tag+'>');listtag=tag
   blocks.append('<li>'+inline(re.sub(r'^(- |\d+\. )','',line))+'</li>')
  elif not line.strip():flush();close_list()
  else:close_list();para.append(line)
  i+=1
 flush();close_list()
 if section:blocks.append('</section>')
 return ''.join(nav),''.join(blocks)
font=base64.b64encode((R/'assets/AtkinsonHyperlegible-Bold.ttf').read_bytes()).decode()
css='''@font-face{font-family:Wanderer;src:url(data:font/ttf;base64,FONT) format('truetype');font-weight:700}*{box-sizing:border-box}:root{font-size:22px;color-scheme:dark;--bg:#081119;--fg:#f0f9f5;--panel:#10202b;--edge:#7fa1ad;--link:#69e8da}body{margin:0;background:var(--bg);color:var(--fg);font-family:Arial,sans-serif;line-height:1.65}body.light{color-scheme:light;--bg:#eef6f6;--fg:#0d1d23;--panel:#fff;--edge:#46646e;--link:#08595a}body.contrast{--bg:#000;--fg:#fff;--panel:#000;--edge:#fff;--link:#ffca71}main,nav,header{max-width:1120px;margin:auto;padding:1rem 1.2rem}h1,h2,button,nav{font-family:Wanderer,Arial,sans-serif;font-weight:700}h1{font-size:2rem;line-height:1.25}h2{font-size:1.45rem;line-height:1.35;margin-top:2.2rem}p,li{max-width:78ch}a{color:var(--link);text-underline-offset:.2em;text-decoration-thickness:2px}#controls{position:sticky;top:0;z-index:5;display:flex;justify-content:center;align-items:center;gap:.4rem;flex-wrap:wrap;padding:.5rem;background:var(--panel);border-bottom:2px solid var(--edge)}button,input{font:inherit;color:var(--fg);background:var(--bg);padding:.35rem .6rem;border:2px solid var(--edge);border-radius:.35rem}button{cursor:pointer}input{width:10rem;max-width:70vw}nav{display:flex;gap:.5rem;flex-wrap:wrap}nav a{border:1px solid var(--edge);border-radius:.3rem;padding:.3rem .6rem}a:focus-visible,button:focus-visible,input:focus-visible,[tabindex]:focus-visible{outline:4px solid #ffca71;outline-offset:4px}.skip{position:absolute;top:-20rem}.skip:focus{top:0;z-index:20;background:var(--bg);padding:.5rem}code{font-size:.86em;overflow-wrap:anywhere}.table-wrap{overflow:auto}table{width:100%;border-collapse:collapse;min-width:600px}th,td{border:1px solid var(--edge);padding:.65rem;text-align:left;vertical-align:top}th{background:var(--panel)}section{scroll-margin-top:7rem}[hidden]{display:none!important}.examples{display:flex;gap:1rem;flex-wrap:wrap}figure{margin:0;max-width:480px}figure img{width:100%;max-width:480px;image-rendering:pixelated;border:2px solid var(--edge)}figcaption{font-size:.9rem}details{margin-top:2rem}pre{white-space:pre-wrap;font-size:.75rem}footer{padding:2rem;text-align:center;border-top:1px solid var(--edge)}@media print{#controls,nav,.skip{display:none}:root{font-size:14pt}body{--bg:#fff;--fg:#000;--panel:#eee;--edge:#555;--link:#000}main{max-width:none}table{min-width:0}h2{break-after:avoid}}'''.replace('FONT',font)
js='''let size=22;const body=document.body;document.getElementById('larger').onclick=()=>{size=Math.min(40,size+2);document.documentElement.style.fontSize=size+'px'};document.getElementById('smaller').onclick=()=>{size=Math.max(18,size-2);document.documentElement.style.fontSize=size+'px'};for(const id of ['light','contrast'])document.getElementById(id).onclick=e=>{body.classList.toggle(id);e.target.setAttribute('aria-pressed',body.classList.contains(id))};document.getElementById('find').oninput=e=>{let n=0;const q=e.target.value.toLowerCase().trim();document.querySelectorAll('.chapter').forEach(s=>{s.hidden=q&&!s.textContent.toLowerCase().includes(q);if(!s.hidden)n++});document.getElementById('status').textContent=q?n+' matching sections':''};'''
def make(md,name,examples=False):
 nav,content=render(md);extra=''
 if examples:
  extra='<section class="chapter" id="screens"><h2>Actual firmware screens</h2><div class="examples">'
  for file,label in [('editor-e1-t0-z0-p01-k0.png','Independent filter controls'),('editor-e4-t1-z0-p10-k0.png','Modal ratios in light theme'),('preset-t1-z1-p75.png','Named patch in Zoom'),('print-t0-z0-state2.png','Print in progress'),('scatter-t0-z0-size6.png','Scatter size at 128 ms'),('scatter-t1-z1-size6.png','Scatter size in light Zoom')]:
   data=base64.b64encode((R/'build/screens-1.3'/file).read_bytes()).decode();extra+='<figure><img alt="'+label+'" src="data:image/png;base64,'+data+'"><figcaption>'+label+'</figcaption></figure>'
  extra+='</div></section>'
 license=(R/'assets/AtkinsonHyperlegible-OFL.txt').read_text()
 doc='<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Wanderer 1.3 RC4</title><style>'+css+'</style></head><body><a class="skip" href="#content">Skip to content</a><div id="controls" aria-label="Reading controls"><button id="smaller" aria-label="Decrease text size">A−</button><button id="larger" aria-label="Increase text size">A+</button><button id="light" aria-pressed="false">Light theme</button><button id="contrast" aria-pressed="false">High contrast</button><label>Find <input id="find" type="search"></label><a href="#top">Top</a></div><header id="top">Low Vision Music · November 1, 2026</header><nav aria-label="Contents">'+nav+'</nav><main id="content"><p id="status" role="status" aria-live="polite"></p>'+content+extra+'<details><summary>Typeface license</summary><pre>'+html.escape(license)+'</pre></details></main><footer>Wanderer · FM-1_929</footer><script>'+js+'</script></body></html>'
 class Check(HTMLParser):
  def __init__(self):super().__init__();self.ids=[];self.refs=[]
  def handle_starttag(self,tag,attrs):
   a=dict(attrs)
   if 'id'in a:self.ids.append(a['id'])
   if tag=='a' and a.get('href','').startswith('#'):self.refs.append(a['href'][1:])
 c=Check();c.feed(doc);assert len(c.ids)==len(set(c.ids)) and set(c.refs)<=set(c.ids)
 (O/name).write_text(doc);print(name,len(doc.encode()))
make((R/'docs/USER_GUIDE_1.3.md').read_text(),'Wanderer-1.3-Guide.html',True)
report=R/'docs/TEST_REPORT_1.3.md'
if report.exists():make(report.read_text(),'Wanderer-1.3-Verification.html')
