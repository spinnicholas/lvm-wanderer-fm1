"""Compare protected platform/recovery bytes and package head/loader to verified RC4."""
import hashlib,json,subprocess,sys
from pathlib import Path
R=Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'));import fm1_install as I
baseline=Path(sys.argv[1]);candidate=Path(sys.argv[2])
paths=subprocess.check_output(['git','ls-tree','-r','--name-only','2131515','firmware/hal','firmware/loader'],cwd=R,text=True).splitlines()
paths+=['firmware/src/'+n for n in ['platform.c','usb.c','ota.c','recovery.c','bootguard.h','store.h','libc.c']]+['firmware/crt0.S','firmware/app.ld']
for name in paths:assert (R/name).read_bytes()==subprocess.check_output(['git','show','2131515:'+name],cwd=R),name

def entries(raw):
 image=I.deinterleave(raw)[0];head=I._dec(image[:64]);count=int.from_bytes(head[8:10],'little');out={}
 for k in range(count):
  e=I._dec(image[64+k*80:64+(k+1)*80]);off=int.from_bytes(e[8:12],'little');size=int.from_bytes(e[12:16],'little');out[int.from_bytes(e[:2],'little')]=image[off:off+size]
 return out
old,new=baseline.read_bytes(),candidate.read_bytes();assert hashlib.sha256(old).hexdigest()=='91633c4d0e3de94fd15722fce062504243c0d623437317be96434165c3b116ce';a,b=entries(old),entries(new);assert a[100]==b[100] and a[0][:0x4000]==b[0][:0x4000]
identity,_=I.load_package(str(candidate));assert identity=='FM-1_929'
report={'identity':identity,'unchanged_protected_files':len(paths),'loader_identical':True,'head_identical':True,'sha256':hashlib.sha256(new).hexdigest(),'bytes':len(new)}
(R/'build/1.4-safety.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
