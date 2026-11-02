#!/usr/bin/env python3
"""Fresh production-source builds; keep every failure and log for review."""
from pathlib import Path
import concurrent.futures,json,os,subprocess,time
R=Path(__file__).resolve().parents[1];O=R/'build/smoke-1.3';O.mkdir(parents=True,exist_ok=True)
for folder in ['host','screens']:(R/'build'/folder).mkdir(parents=True,exist_ok=True)
env=os.environ|{'ASAN_OPTIONS':'detect_leaks=0'}
flags=['cc','-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-Wno-unused-function','-Ifirmware/hal','-Ifirmware/src','-Ifirmware/gen']
def run(name,args):
 t=time.monotonic();p=subprocess.run(args,cwd=R,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True);(O/(name+'.log')).write_text(p.stdout)
 return dict(test=name,status='passed' if not p.returncode else 'FAILED',seconds=round(time.monotonic()-t,2),exit_code=p.returncode)
def ctest(name):
 r=run(name+'-compile',flags+['tests/'+name+'.c','-o',str(O/name)])
 return [r]+([run(name,[str(O/name)])] if r['status']=='passed' else [])
names=['wanderer_memory_test','wanderer_release_test','wanderer_scope_test','wanderer_ui_test','wanderer_splash_test','wanderer_1_1_test','wanderer_1_1_stress','wanderer_1_2_test','wanderer_1_2_rc2_test','wanderer_stop_led_test','ui_rows_test','wanderer_1_3_test','wanderer_editor_test','wanderer_timing_1_3','wanderer_operations_1_3','wanderer_range_1_3','wanderer_scatter_test']
results=[]
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as ex:
 for group in ex.map(ctest,names):results.extend(group)
for name in ['ota_test','ldr_test']:
 results.append(run(name+'-compile',flags+['tests/'+name+'.c','-o',str(O/name)]))
new='build/dinghy-'+env.get('WANDERER_RELEASE','1.3-RC4')+'.fwsc';old='recovery/LVM-Wanderer-0.9-FM1.fwsc'
results+=[run('ota',[str(O/'ota_test'),new]),run('loader-up',[str(O/'ldr_test'),old,new]),run('loader-down',[str(O/'ldr_test'),new,old])]
results.append(run('host-compile',flags+['-DDINGHY_HOST','-DAPP_VER="1.3-RC4"','firmware/src/dinghy.c','-o','build/host/dinghy']))
env['WANDERER_EXPECT']='tests/wanderer-1.3-navigation.expect'
results.append(run('new-navigation',['build/host/dinghy','build/host/final-navigation','tests/wanderer-1.3-navigation.txt']))
env['WANDERER_EXPECT']='tests/wanderer-1.3-editor-save.expect'
results.append(run('editor-save-navigation',['build/host/dinghy','build/host/editor-save','tests/wanderer-1.3-editor-save.txt']))
env.pop('WANDERER_EXPECT',None)
for name in ['icon_rle_test','install_test','stock_install_test','ui_assets_audit','splash_input_audit']:
 results.append(run(name,['python3','tests/'+name+'.py']))
(O/'summary.json').write_text(json.dumps(dict(results=results),indent=2)+'\n')
print(json.dumps([r for r in results if r['status']!='passed'],indent=2));print(f"{sum(r['status']=='passed' for r in results)}/{len(results)} passed")
