#!/usr/bin/env python3
"""Fresh production-source builds; keep every failure and log for review."""
from pathlib import Path
import concurrent.futures,json,os,subprocess,time
R=Path(__file__).resolve().parents[1]
release=os.environ.get('WANDERER_RELEASE','1.7-RC1')
O=R/'build'/('smoke-1.7' if release.startswith('1.7') else 'smoke-'+release)
O.mkdir(parents=True,exist_ok=True)
for folder in ['host','screens','rc3','rc4']:(R/'build'/folder).mkdir(parents=True,exist_ok=True)
env=os.environ|{'ASAN_OPTIONS':'detect_leaks=0'}
flags=['cc','-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-Wno-unused-function','-Ifirmware/hal','-Ifirmware/src','-Ifirmware/gen']
def run(name,args,expected=0):
 t=time.monotonic();p=subprocess.run(args,cwd=R,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True);(O/(name+'.log')).write_text(p.stdout)
 return dict(test=name,status='passed' if p.returncode==expected else 'FAILED',seconds=round(time.monotonic()-t,2),exit_code=p.returncode)
def ctest(name):
 r=run(name+'-compile',flags+['tests/'+name+'.c','-o',str(O/name)])
 return [r]+([run(name,[str(O/name)])] if r['status']=='passed' else [])
names=['wanderer_memory_test','wanderer_release_test','wanderer_scope_test','wanderer_ui_test','wanderer_splash_test','wanderer_1_1_test','wanderer_1_1_stress','wanderer_1_2_test','wanderer_1_2_rc2_test','wanderer_stop_led_test','ui_rows_test','wanderer_1_3_test','wanderer_editor_test','wanderer_timing_1_3','wanderer_operations_1_3','wanderer_range_1_3','wanderer_scatter_test','wanderer_1_4_test','wanderer_zoom_fit_test']
names.extend(['wanderer_zoom_shortcut_test','wanderer_bank_1_5_test','wanderer_engine_1_6_test','wanderer_1_7_test','wanderer_fx_1_7_test','wanderer_rc2_response_test','wanderer_theme_test','wanderer_grain_512_test'])
names.extend(['wanderer_accessibility_test','wanderer_cue_test','wanderer_steady_zoom_test'])
if release.startswith('2.'):
 names.extend(['wanderer_2_0_test','wanderer_adversarial_2_0','wanderer_filter_test',
               'wanderer_reverb_test','wanderer_layer_workflow_test','wanderer_scatter_patterns_test','wanderer_bank_1_7_stress'])
results=[]
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as ex:
 for group in ex.map(ctest,names):results.extend(group)
for name in ['ota_test','ldr_test']:
 results.append(run(name+'-compile',flags+['tests/'+name+'.c','-o',str(O/name)]))
new='build/dinghy-'+release+'.fwsc'
old=env.get('WANDERER_BASELINE_PACKAGE','recovery/LVM-Wanderer-0.9-FM1.fwsc')
results+=[run('ota',[str(O/'ota_test'),new]),run('loader-up',[str(O/'ldr_test'),old,new]),run('loader-down',[str(O/'ldr_test'),new,old])]
if release.startswith('2.'):
 truncated=O/'truncated.fwsc'
 truncated.write_bytes((R/new).read_bytes()[:0x500])
 results.append(run('reject-truncated-loader-fixture',[str(O/'ldr_test'),str(truncated),new],2))
results.append(run('host-compile',flags+['-DDINGHY_HOST',f'-DAPP_VER="{release}"','firmware/src/dinghy.c','-o','build/host/dinghy']))
env['WANDERER_EXPECT']='tests/wanderer-1.3-navigation.expect'
results.append(run('new-navigation',['build/host/dinghy','build/host/final-navigation','tests/wanderer-1.3-navigation.txt']))
env['WANDERER_EXPECT']='tests/wanderer-1.3-editor-save.expect'
results.append(run('editor-save-navigation',['build/host/dinghy','build/host/editor-save','tests/wanderer-1.3-editor-save.txt']))
env['WANDERER_EXPECT']='tests/wanderer-access-feel.expect'
results.append(run('access-feel-navigation',['build/host/dinghy','build/host/access-feel','tests/wanderer-access-feel.txt']))
if release.startswith('2.'):
 for script in ['wanderer-rc2-listening','wanderer-anchor-shortcut']:
  env['WANDERER_EXPECT']='tests/'+script+'.expect'
  results.append(run(script,['build/host/dinghy','build/host/'+script,'tests/'+script+'.txt']))
 env['WANDERER_EXPECT']='tests/wanderer-2.0-holds.expect'
 results.append(run('performance-holds',['build/host/dinghy','build/host/performance-holds','tests/wanderer-2.0-holds.txt']))
 env['WANDERER_EXPECT']='tests/wanderer-layer-live.expect'
 results.append(run('layer-live-navigation',['build/host/dinghy','build/host/layer-live','tests/wanderer-layer-live.txt']))
 env['WANDERER_EXPECT']='tests/wanderer-layer-playback.expect'
 results.append(run('layer-selector-and-playback',['build/host/dinghy','build/host/layer-playback','tests/wanderer-layer-playback.txt']))
 env['WANDERER_EXPECT']='tests/wanderer-control-conflicts.expect'
 results.append(run('control-conflicts',['build/host/dinghy','build/host/control-conflicts','tests/wanderer-control-conflicts.txt']))
env.pop('WANDERER_EXPECT',None)
for name in ['icon_rle_test','install_test','stock_install_test','ui_assets_audit','splash_input_audit']:
 results.append(run(name,['python3','tests/'+name+'.py']))
(O/'summary.json').write_text(json.dumps(dict(results=results),indent=2)+'\n')
print(json.dumps([r for r in results if r['status']!='passed'],indent=2));print(f"{sum(r['status']=='passed' for r in results)}/{len(results)} passed")

raise SystemExit(any(r['status']!='passed' for r in results))
