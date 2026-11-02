#!/usr/bin/env python3
"""Package, unchanged recovery foundation and exact linked-region accounting."""
from pathlib import Path
import hashlib,json,re,subprocess,sys,os
R=Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'));import fm1_install as I
release=os.environ.get('WANDERER_RELEASE','1.3-RC4');new=R/f'build/dinghy-{release}.fwsc';identity,_=I.load_package(str(new));assert identity=='FM-1_929'
def entries(raw):
 image=I.deinterleave(raw)[0];head=I._dec(image[:64]);out={}
 for k in range(int.from_bytes(head[8:10],'little')):
  e=I._dec(image[64+k*80:64+(k+1)*80]);off=int.from_bytes(e[8:12],'little');size=int.from_bytes(e[12:16],'little');out[int.from_bytes(e[:2],'little')]=image[off:off+size]
 return out
old=entries((R/'recovery/LVM-Wanderer-0.9-FM1.fwsc').read_bytes());current=entries(new.read_bytes());assert old[100]==current[100] and old[0][:0x4000]==current[0][:0x4000]
paths=list((R/'firmware/hal').glob('*'))+list((R/'firmware/loader').glob('*'))
paths += [R/'firmware/src'/n for n in ['platform.c','usb.c','ota.c','recovery.c','bootguard.h','store.h','libc.c']]+[R/'firmware/crt0.S',R/'firmware/app.ld']
protected=[]
for p in paths:
 if not p.is_file():continue
 rel=str(p.relative_to(R));before=subprocess.check_output(['git','show','e89e2ccd02206eb84015d659064e05ee405d1b5a:'+rel],cwd=R);assert before==p.read_bytes(),rel
 protected.append(dict(path=rel,sha256=hashlib.sha256(before).hexdigest()))
log=(R/f'build/1.3-{release.split("-")[-1].lower()}-target.log').read_text();m=re.search(r'image (\d+) B of (\d+); RAM .data\+.bss (\d+) B of (\d+); pool (\d+) B of (\d+)',log);assert m
vals=list(map(int,m.groups()));report=dict(identity=identity,package_bytes=new.stat().st_size,sha256=hashlib.sha256(new.read_bytes()).hexdigest(),regions={k:dict(used=vals[i*2],limit=vals[i*2+1],free=vals[i*2+1]-vals[i*2]) for i,k in enumerate(['app','ram','pool'])},protected_files=protected,compressed_loader_identical_to_09=True,package_head_identical_to_09=True,physical_hardware='not tested',firmware_release=release)
for r in report['regions'].values():assert r['free']>0
(R/'build/1.3-package.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k!='protected_files'},indent=2));print(len(protected),'protected source files unchanged')
