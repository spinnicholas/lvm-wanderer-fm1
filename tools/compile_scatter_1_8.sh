#!/bin/sh
# Build and validate a REAL flashable firmware with the pinned JieLi target compiler.
# Never relabel an RC4 .fwsc as 1.8 or skip linker/RAM/SDK hash checks.
set -eu
cd "$(dirname "$0")/.."
for dep in "${JIELI_TOOLCHAIN:-$HOME/.jieli/toolchain}/pi32v2/bin/clang" "${AC79_SDK:-$HOME/fw-AC79_AIoT_SDK}/cpu/wl82/tools/uboot.boot"; do
  if [ ! -e "$dep" ]; then
    echo "Missing native build prerequisite: $dep" >&2
    echo 'Follow README-DINGHY.md or run the GitHub Actions candidate build workflow.' >&2
    exit 2
  fi
done
mkdir -p build/ci build/rc4
for test in wanderer_adversarial_2_0 wanderer_2_0_test wanderer_filter_test wanderer_reverb_test wanderer_layer_workflow_test wanderer_scatter_patterns_test wanderer_scatter_test wanderer_grain_512_test wanderer_fx_1_7_test wanderer_release_test wanderer_memory_test; do
  "${CC:-cc}" -O1 -Wno-unused-function -Ifirmware/hal -Ifirmware/src -Ifirmware/gen "tests/$test.c" -o "build/ci/$test"
  "build/ci/$test"
done
sh build.sh --release 2.0-RC11
python3 - <<'PY'
import sys,hashlib
from pathlib import Path
sys.path.insert(0, 'tools')
from fm1_install import load_package
p=Path('build/dinghy-2.0-RC11.fwsc')
identity,image=load_package(str(p),False)
if identity!='FM-1_929' or p.stat().st_size<500000:raise SystemExit('unexpected firmware package')
print(f'FLASHABLE BUILD VALIDATED: {p}  {p.stat().st_size} B  SHA256={hashlib.sha256(p.read_bytes()).hexdigest()}')
PY
