#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/wanderer-tour build/evidence
sh build.sh > build/evidence/build.log
cc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all tests/wanderer_memory_test.c -o build/wanderer_memory_test
ASAN_OPTIONS=detect_leaks=0 build/wanderer_memory_test > build/evidence/memory.log
sh tests/run_host.sh tests/wanderer-tour.txt > build/evidence/tour.log
python3 - <<'PY'
from pathlib import Path
import shutil,wave,struct,math
for p in Path('build/host').glob('dinghy-*.png'):
    if p.stem.split('-')[-1] in {'900','1100','1600','1800','2000','2450','2600','2900'}:
        shutil.copy2(p, Path('build/wanderer-tour')/p.name)
shutil.copy2('build/host/dinghy.wav','build/wanderer-tour/tour.wav')
with wave.open('build/wanderer-tour/tour.wav') as w:
    n=w.getnframes(); samples=struct.unpack('<'+'h'*2*n,w.readframes(n))
    assert n>120000
    assert max(map(abs,samples))>1000
    assert max(map(abs,samples))<32767
    for a,b in [(.9,1.2),(1.5,1.8),(1.8,2.1)]:
        block=samples[int(a*w.getframerate())*2:int(b*w.getframerate())*2]
        rms=math.sqrt(sum(x*x for x in block)/len(block))
        print(f'{a:.1f}-{b:.1f} s RMS {rms:.0f}')
PY
sh tests/run_host.sh tests/wanderer-settings.txt > build/evidence/settings.log
sh tests/run_host.sh tests/wanderer-fx.txt > build/evidence/effects-ui.log
sh tests/run_host.sh tests/wanderer-boundaries.txt > build/evidence/boundaries.log
sh tests/run_host.sh tests/wanderer-new-session.txt > build/evidence/new-session.log
sh tests/run_host.sh tests/wanderer-tempo-restore.txt > build/evidence/tempo-restore.log
sh tests/run_host.sh tests/wanderer-adversarial.txt > build/evidence/adversarial.log
sh tests/run_host.sh tests/wanderer-effects-demo.txt > build/evidence/effects-audio.log
python3 tests/preset_audit.py > build/evidence/preset-audit.log
python3 - <<'PY'
import wave,struct,math
with wave.open('build/host/dinghy.wav') as w:
    x=struct.unpack('<'+'h'*w.getnframes()*2,w.readframes(w.getnframes()))
    assert max(map(abs,x))<32767
    assert any(x[i]!=x[i+1] for i in range(0,len(x),2)), 'effects must create stereo output'
    def rms(a,b):
        q=x[int(a*w.getframerate())*2:int(b*w.getframerate())*2]
        return math.sqrt(sum(v*v for v in q)/len(q))
    assert rms(12.4,12.9)>rms(13.0,13.4)*2, 'room tail must decay'
PY
sh tests/run_host.sh tests/wanderer-audio-stress.txt > build/evidence/audio-stress.log
python3 - <<'PY'
import wave,struct
with wave.open('build/host/dinghy.wav') as w:
    x=struct.unpack('<'+'h'*w.getnframes()*2,w.readframes(w.getnframes()))
    assert max(map(abs,x))<32767, 'four-voice maximum-level audio clips'
PY
sh tests/run_host.sh tests/wanderer-fx-stress.txt > build/evidence/effects-stress.log
python3 - <<'PY'
import wave,struct,math
with wave.open('build/host/dinghy.wav') as w:
    x=struct.unpack('<'+'h'*w.getnframes()*2,w.readframes(w.getnframes()))
    assert max(map(abs,x))<32767, 'maximum wet settings clip'
    def rms(a,b):
        q=x[int(a*w.getframerate())*2:int(b*w.getframerate())*2]
        return math.sqrt(sum(v*v for v in q)/len(q))
    assert rms(8,9)<rms(3,4)/20, 'feedback tail does not decay'
PY
python3 - <<'PY'
from pathlib import Path
s=Path('build/evidence/adversarial.log').read_text()
assert 'slot 0 saved' not in s, 'save while playing must be refused'
PY
cc -O1 tests/ota_test.c -o build/ota_test
build/ota_test build/dinghy.fwsc > build/evidence/ota.log
python3 - <<'PY'
from pathlib import Path
p=bytearray(Path('build/dinghy.bin').read_bytes())
p[0x3000]^=1
Path('build/evidence/older.bin').write_bytes(p)
PY
python3 tools/fm1pkg_make.py build/evidence/older.bin build/loader/ota.bin build/evidence/older.fwsc > build/evidence/old-package.log
cc -O1 tests/ldr_test.c -o build/ldr_test
build/ldr_test build/evidence/older.fwsc build/dinghy.fwsc > build/evidence/loader.log
python3 tests/install_test.py > build/evidence/installer.log
python3 - <<'PY'
import sys,hashlib,json
from pathlib import Path
sys.path.insert(0,'tools')
from fm1_install import load_package
p=Path('build/dinghy.fwsc')
identity,image=load_package(str(p),False)
assert identity=='FM-1_929'
report={'identity':identity,'package_bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),
        'app_bytes':Path('build/dinghy.bin').stat().st_size,'app_limit':581564,
        'validation':'CRC and loader marker parsed; host tests pass','physical_device':'not tested'}
Path('build/evidence/package.json').write_text(json.dumps(report,indent=2)+'\n')
print('Wanderer core, host tour, layout bounds, OTA, loader, installer and package checks passed')
PY
