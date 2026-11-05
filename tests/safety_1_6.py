"""Audit the candidate against the exact hardware-accepted 1.4 RC6 package."""
import hashlib,json,subprocess,sys
from pathlib import Path
R=Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'));import fm1_install as I
base='3fb54aa'
paths=subprocess.check_output(['git','ls-tree','-r','--name-only',base,'firmware/hal','firmware/loader'],cwd=R,text=True).splitlines()
paths+=['firmware/src/'+n for n in ['platform.c','usb.c','ota.c','recovery.c','bootguard.h','store.h','libc.c']]+['firmware/crt0.S','firmware/app.ld']
for name in paths:assert (R/name).read_bytes()==subprocess.check_output(['git','show',base+':'+name],cwd=R),name
def entries(raw):
 image=I.deinterleave(raw)[0];head=I._dec(image[:64]);count=int.from_bytes(head[8:10],'little');out={}
 for k in range(count):
  e=I._dec(image[64+k*80:64+(k+1)*80]);off=int.from_bytes(e[8:12],'little');size=int.from_bytes(e[12:16],'little');out[int.from_bytes(e[:2],'little')]=image[off:off+size]
 return out
old=(R/'dist/LVM-Wanderer-1.4-RC6-FM1.fwsc').read_bytes();new=(R/'build/dinghy-1.6-RC1.fwsc').read_bytes()
assert hashlib.sha256(old).hexdigest()=='9135b62169409efb73a6492561673bb5465979563824605956ab3316c50d5b32'
a,b=entries(old),entries(new);assert a[100]==b[100] and a[0][:0x4000]==b[0][:0x4000]
identity,_=I.load_package(str(R/'build/dinghy-1.6-RC1.fwsc'));assert identity=='FM-1_929'
report={'identity':identity,'unchanged_protected_files':len(paths),'loader_identical_to_accepted_1_4':True,'head_identical_to_accepted_1_4':True,'sha256':hashlib.sha256(new).hexdigest(),'bytes':len(new)}
(R/'build/safety-1.6.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
