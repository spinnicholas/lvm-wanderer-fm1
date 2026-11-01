#!/usr/bin/env python3
"""Compare immutable safety files and the compressed loader to the known 0.9 package."""
import hashlib
import json
from pathlib import Path
import sys
import zipfile
root=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(root/'tools'))
import fm1_install as I
baseline=Path(sys.argv[1]);release=Path(sys.argv[2])
with zipfile.ZipFile(baseline) as z:
    names=[n for n in z.namelist() if n.startswith(('source/firmware/hal/','source/firmware/loader/')) and not n.endswith('/')]
    names += ['source/firmware/src/'+n for n in ['platform.c','usb.c','ota.c','recovery.c','bootguard.h','store.h','libc.c']]
    names += ['source/firmware/crt0.S','source/firmware/app.ld']
    for n in names:
        p=root/n.removeprefix('source/')
        assert p.read_bytes()==z.read(n),f'safety source changed: {n}'
    old=z.read('LVM-Wanderer-0.9-FM1.fwsc')
def entries(raw):
    image=I.deinterleave(raw)[0];head=I._dec(image[:64]);count=int.from_bytes(head[8:10],'little')
    out={}
    for k in range(count):
        e=I._dec(image[64+k*80:64+(k+1)*80]);off=int.from_bytes(e[8:12],'little');size=int.from_bytes(e[12:16],'little')
        out[int.from_bytes(e[:2],'little')]=image[off:off+size]
    return out
new=release.read_bytes();oe,ne=entries(old),entries(new)
assert oe[100]==ne[100], 'compressed loader differs from hardware-tested 0.9'
assert oe[0][:0x4000]==ne[0][:0x4000], 'package head changed'
identity,_=I.load_package(str(release))
assert identity=='FM-1_932'
report={'baseline_sha256':hashlib.sha256(old).hexdigest(),'release_sha256':hashlib.sha256(new).hexdigest(),
 'identity':identity,'unchanged_safety_files':len(names),'loader_sha256':hashlib.sha256(ne[100]).hexdigest(),
 'loader_identical_to_0.9':True,'package_head_identical_to_0.9':True,
 'physical_1.2_test':'pending; inherited 0.9 tests do not validate new app bytes'}
(root/'build/evidence-1.2/safety.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
