"""Audit actual target prologues: the real-device boot failure was a stack guard trap."""
from pathlib import Path
import re,json
R=Path(__file__).resolve().parents[1]
def frame(text,name):
 block=text.split('\n'+name+':\n',1)[1].split('\n\n',1)[0];prologue=block.split('call ',1)[0];size=0
 for line in prologue.splitlines():
  m=re.search(r'sp \+= -(\d+)',line)
  if m:size+=int(m[1])
  m=re.search(r'\[--sp\] = \{([^}]+)\}',line)
  if m:
   for item in m[1].split(','):
    item=item.strip();r=re.fullmatch(r'r(\d+)-r(\d+)',item)
    size+=4*(abs(int(r[1])-int(r[2]))+1) if r else 4
 if re.search(r'sp = r\d+',prologue):
  constants=re.findall(r'r\d+ = -(\d+)',prologue);assert constants,'unresolved dynamic stack frame';size+=int(constants[0])
 return size
s=(R/'build/dinghy.dis').read_text();names=['app_main','project_read_slot','wm_valid_v7','wm_migrate_v7_ptr','wm_valid_v5','wm_valid_v4','wm_valid_v3','wm_valid_legacy','wm_migrate_v5_ptr','wm_migrate_v4_ptr','wm_migrate_v3_ptr','wm_migrate_legacy_ptr','wm_migrate_phrase','wm_migrate_phrase_v4'];frames={n:frame(s,n) for n in names}
assert frames['project_read_slot']<=64
assert max(frames.values())<=3072
bound=frames['app_main']+frames['project_read_slot']+max(frames[n] for n in names if n.startswith('wm_valid_'))+max(frames[n] for n in names if '_ptr' in n)+max(frames[n] for n in names if n.startswith('wm_migrate_phrase'))
assert bound<4096
old=R/'build/rc1-crash.dis'
if old.exists():assert frame(old.read_text(),'project_read_slot')>24064
report={'frames_bytes':frames,'conservative_boot_migration_chain_bytes':bound,'scanner_frame_limit':64,'boot_chain_limit':4096,'rc1_fault_pc':'0x020062AC','rc1_return_pc':'0x02008BC0','rc1_scanner_local_allocation':26488}
(R/'build/rc2-stack-audit.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
