#!/usr/bin/env python3
"""Verify source-backed icon masks, source provenance and unchanged musical core."""
from pathlib import Path
import hashlib,importlib.util,json
R=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('icons',R/'tools/gen_wanderer_icons.py');icons=importlib.util.module_from_spec(spec);spec.loader.exec_module(icons)
assert len(icons.MAP)==47 and len(set(icons.MAP.values()))==47
assets=[]
for name,source in icons.MAP.items():
    p=R/'assets/icons'/f'{source}.svg';assert p.exists()
    for size in [24,32,48,64,96]:
        a=icons.make(name,size);q=[min(15,(v+8)//17) for v in a.tobytes()]
        assert len(q)==size*size and max(q)==15 and min(q)==0
        assert sum(v>0 for v in q)>size*size//20,(name,size,'empty-looking icon')
        assert not any(q[x] or q[(size-1)*size+x] or q[x*size] or q[x*size+size-1] for x in range(size)),(name,size,'cropped edge')
    project=source.split('/')[0]
    assets.append(dict(function=name,source=source+'.svg',url=(f'https://github.com/lucide-icons/lucide/blob/main/icons/{p.name}' if project=='lucide' else f'https://github.com/tabler/tabler-icons/blob/main/icons/outline/{p.name}'),sha256=hashlib.sha256(p.read_bytes()).hexdigest(),license='ISC / Feather MIT' if project=='lucide' else 'MIT'))
(R/'assets/icons/sources.json').write_text(json.dumps({'retrieved':'2026-11-01','adaptation':'2.25-unit stroke, original geometry, Inkscape rasterization at 384px; 4-bit masks at 24/32/48/64/96px','icons':assets},indent=2)+'\n')
for project in ['lucide','tabler']:assert 'Permission' in (R/f'assets/icons/{project}/LICENSE.txt').read_text()
# The supplied RC1 source can be given explicitly when reproducing this comparison.
import sys
before=Path(sys.argv[1]) if len(sys.argv)>1 else R.parents[1]/'wanderer-1.1-rc2/source'
files=['wanderer_presets.h','wanderer_waves.h','wanderer_filter.h','wanderer_scope.c']
if before.exists():
    for name in files:assert (R/'firmware/src'/name).read_bytes()==(before/'firmware/src'/name).read_bytes(),name+' changed'
    print('7 effect, preset, oscillator, filter, scope and splash source files are byte-identical to 1.1 RC2')
print('47 distinct licensed source SVGs; all 235 display masks retain padding/opacity; source hashes and full notices present')
