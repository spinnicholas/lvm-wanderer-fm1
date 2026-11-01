#!/usr/bin/env python3
"""Run identical musical events with scope off/on; rendered PCM must match exactly."""
from pathlib import Path
import subprocess,hashlib,wave,struct
root=Path(__file__).resolve().parent.parent
prefix=['0 btn 3','30 enc 6 2','60 enc 0 1','90 btn 3','130 btn 8']
music=['200 keys 18561','850 keys 0','1100 midi 144 36 127','1400 midi 128 36 0','2200 end']
scopes=['350 btndown 8','360 btndown 3','370 btnup 8','380 btnup 3','500 enc 0 2','550 enc 1 3']
outputs=[]
for active in [False,True]:
    lines=prefix+music+(scopes if active else [])
    lines.sort(key=lambda x:int(x.split()[0]))
    script=root/f'build/scope-compare-{int(active)}.txt';script.write_text('\n'.join(lines)+'\n')
    out=root/f'build/scope-compare-{int(active)}'
    subprocess.run([str(root/'build/host/dinghy'),str(out),str(script)],check=True,stdout=subprocess.DEVNULL)
    with wave.open(str(out.with_suffix('.wav'))) as f:outputs.append(f.readframes(f.getnframes()))
assert outputs[0]==outputs[1], 'scope display or controls altered musical PCM'
assert any(outputs[0]),'musical scenario silent'
print(f'Scope off/on PCM is byte-identical: {len(outputs[0])} bytes, SHA-256 {hashlib.sha256(outputs[0]).hexdigest()}')
