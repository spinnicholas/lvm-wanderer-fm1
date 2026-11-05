#!/usr/bin/env python3
"""Exact compiled package: enable seven effects, hold four MSFA voices, browse."""
import json,shutil,subprocess,sys
from pathlib import Path
R=Path(__file__).resolve().parents[1];O=R/'build/compiled-1.7';(O/'work').mkdir(parents=True,exist_ok=True)
keys='1100-2100:SCL,2200:enc6:+9,2450:enc2:+1,2600:enc6:+1,2750:enc2:+1,2900:enc6:+1,3050:enc4:+1,3200:enc6:+1,3350:enc2:+1,3500:enc6:+1,3650:enc2:+1,3800:enc6:-11,4000:enc2:+1,4200:HOME,4500:midi:c01a,4560:enc6:+1,4650-7500:n0,4650-7500:n2,4650-7500:n4,4650-7500:n7,8000:midi:b00000,8050:midi:b02001,8100:midi:c015,8600:enc6:+1'
shutil.copyfile(R/'build/dinghy-1.7-RC1.fwsc',O/'candidate.fwsc')
args=[sys.argv[1],'--fwsc','candidate.fwsc','--ms','9500','--keys',keys,'--wav','target.wav','--snap','2500,2800,3100,3400,3700,4100,4900,8250,8950']
(O/'scenario.json').write_text(json.dumps(args,indent=2)+'\n')
with (O/'target.log').open('w') as f:subprocess.run(args,cwd=O,stdout=f,stderr=subprocess.STDOUT,check=True)
print('Compiled-package seven-effect / four-FM6-voice run completed')
