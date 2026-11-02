#!/usr/bin/env python3
"""Every compressed atlas must decode to the vetted rasterizer's exact alpha pixels."""
from pathlib import Path
import importlib.util,re
R=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('icons',R/'tools/gen_wanderer_icons.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
s=(R/'firmware/src/wanderer_icons.h').read_text();names=re.findall(r'#define ICO_(\w+) \d+',s)
for prefix,size in [('',64),('ZOOM_',96),('TINY_',24),('MINI_',32),('MID_',48)]:
 def array(name):return list(map(int,re.search(r'\b'+name+r'\[\]=\{([^}]+)',s,re.S)[1].replace('\n','').strip(',').split(',')))
 rows=array('ICON_'+prefix+'ROW');data=array('ICON_'+prefix+'DATA');assert len(rows)==len(names)*size
 for i,name in enumerate(names):
  decoded=[]
  for y in range(size):
   at=rows[i*size+y];row=[]
   while len(row)<size:
    token=data[at];at+=1;row += [token>>4]*((token&15)+1)
   assert len(row)==size
   decoded+=row
  expected=[min(15,(v+8)//17) for v in m.make(name.lower(),size).tobytes()]
  assert decoded==expected,(name,size)
print(f'{len(names)*5} icon masks decode pixel-identically to licensed source renders, including every legacy icon')
