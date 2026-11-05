#!/bin/sh
# Build and validate a REAL flashable FM-1 firmware with the pinned JieLi target compiler.
#   RELEASE=3.0-RC1 sh tools/compile_native.sh
# Runs the host regression suite first, then the native build (which itself refuses
# oversize images, RAM/pool overflow, ROM calls and register access outside hal/).
set -eu
cd "$(dirname "$0")/.."
RELEASE="${RELEASE:-3.0-RC1}"
for dep in "${JIELI_TOOLCHAIN:-$HOME/.jieli/toolchain}/pi32v2/bin/clang" "${AC79_SDK:-$HOME/fw-AC79_AIoT_SDK}/cpu/wl82/tools/uboot.boot"; do
  if [ ! -e "$dep" ]; then
    echo "Missing native build prerequisite: $dep" >&2
    echo 'Follow README-DINGHY.md or run the GitHub Actions native build workflow.' >&2
    exit 2
  fi
done
sh tests/run_core_tests.sh
sh build.sh --release "$RELEASE" | tee build/native-build.log
python3 - "$RELEASE" <<'PY'
import sys,hashlib
from pathlib import Path
sys.path.insert(0, 'tools')
from fm1_install import load_package
rel=sys.argv[1]
p=Path(f'build/dinghy-{rel}.fwsc')
identity,image=load_package(str(p),False)
if identity!='FM-1_929' or p.stat().st_size<500000:raise SystemExit('unexpected firmware package')
print(f'FLASHABLE BUILD VALIDATED: {p}  {p.stat().st_size} B  SHA256={hashlib.sha256(p.read_bytes()).hexdigest()}')
PY
