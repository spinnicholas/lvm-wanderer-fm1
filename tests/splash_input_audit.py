#!/usr/bin/env python3
"""First-input startup dismissal must preserve the normal action, including chord-safe releases."""
from pathlib import Path
import os,subprocess
root=Path(__file__).resolve().parent.parent
cases=[(f'button-{i}',f'10 btn {i}',{}) for i in range(14)]
actions={12:{'running':1},13:{'recording':1},3:{'settings':1},11:{'confirm':2},10:{'confirm':3},2:{'confirm':1},6:{'editor':1}}
cases=[(n,e,actions.get(int(n.split('-')[1]),{})) for n,e,_ in cases]
for i in range(7):cases.append((f'encoder-{i}',f'10 enc {i} 1',{0:{'sound':10},1:{'bpm':113},5:{'volume':104},6:{'sound':1}}.get(i,{})))
cases += [('local-key','10 keys 128',{'gates':1}),('usb-note','10 midi 144 60 100',{'gates':1}),('trs-note','10 trs 144 60 100',{'gates':1})]
for name,event,fields in cases:
    script=root/f'build/splash-{name}.txt';script.write_text(event+'\n80 end\n')
    expect=root/f'build/splash-{name}.expect';expect.write_text('20 splash 0\n'+''.join(f'20 {k} {v}\n' for k,v in fields.items()))
    env=dict(os.environ,WANDERER_EXPECT=str(expect),ASAN_OPTIONS='detect_leaks=0')
    result=subprocess.run([str(root/'build/host/dinghy'),str(root/f'build/splash-{name}'),str(script)],env=env,capture_output=True,text=True)
    assert result.returncode==0,(name,result.stdout,result.stderr)
script=root/'build/splash-timeout.txt';script.write_text('1200 end\n')
expect=root/'build/splash-timeout.expect';expect.write_text('100 splash 1\n1102 splash 0\n')
result=subprocess.run([str(root/'build/host/dinghy'),str(root/'build/splash-timeout'),str(script)],env=dict(os.environ,WANDERER_EXPECT=str(expect),ASAN_OPTIONS='detect_leaks=0'),capture_output=True,text=True)
assert result.returncode==0,(result.stdout,result.stderr)
assert 'saved' not in result.stdout
print(f'{len(cases)} first-input cases and the 1100 ms no-input timeout pass; keyboard/MIDI/controls preserved and no startup saves')
