#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# FM-1 release candidate gate; no connected hardware is touched.
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/evidence-1.2 build/screens
export ASAN_OPTIONS=detect_leaks=0
export HOST_CFLAGS='-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all'
sh tests/run_wanderer_tests.sh > build/evidence-1.2/suite.log 2>&1
cc $HOST_CFLAGS -Wall -Wno-unused-function -Ifirmware/hal -Ifirmware/src -Ifirmware/gen tests/wanderer_release_test.c -o build/wanderer_release_test
build/wanderer_release_test > build/evidence-1.2/core.log 2>&1
WANDERER_EXPECT=tests/wanderer-led-navigation.expect build/host/dinghy build/led-navigation tests/wanderer-led-navigation.txt > build/evidence-1.2/led-navigation.log 2>&1
WANDERER_EXPECT=tests/wanderer-short-tap.expect build/host/dinghy build/short-tap tests/wanderer-short-tap.txt > build/evidence-1.2/short-tap.log 2>&1
WANDERER_EXPECT=tests/wanderer-rc2-navigation.expect build/host/dinghy build/rc2-navigation tests/wanderer-rc2-navigation.txt > build/evidence-1.2/rc2-navigation.log 2>&1
WANDERER_EXPECT=tests/wanderer-1.1-navigation.expect build/host/dinghy build/music-navigation tests/wanderer-1.1-navigation.txt > build/evidence-1.2/music-navigation.log 2>&1
WANDERER_EXPECT=tests/wanderer-1.2-navigation.expect build/host/dinghy build/product-navigation tests/wanderer-1.2-navigation.txt > build/evidence-1.2/product-navigation.log 2>&1
WANDERER_EXPECT=tests/wanderer-rc2-listening.expect build/host/dinghy build/rc2-listening tests/wanderer-rc2-listening.txt > build/evidence-1.2/rc2-listening.log 2>&1
python3 tests/ui_assets_audit.py > build/evidence-1.2/ui-assets.log 2>&1
python3 tests/splash_input_audit.py > build/evidence-1.2/splash-inputs.log 2>&1
python3 tests/scope_audio_compare.py > build/evidence-1.2/audio-identical.log 2>&1
for task in wanderer_scope_test wanderer_ui_test wanderer_splash_test wanderer_1_1_test wanderer_1_1_stress wanderer_1_2_test wanderer_1_2_rc2_test wanderer_stop_led_test ui_rows_test; do
    cc $HOST_CFLAGS -Wall -Wno-unused-function -Ifirmware/hal -Ifirmware/src -Ifirmware/gen tests/$task.c -o build/$task
    build/$task > build/evidence-1.2/$task.log 2>&1
done
cc $HOST_CFLAGS -Wno-unused-function -Ifirmware/hal -Ifirmware/src -Ifirmware/gen tests/capture_screens.c -o build/capture_screens
build/capture_screens > build/evidence-1.2/screens.log 2>&1
sh build.sh --release 1.2-rc2 > build/evidence-1.2/release-build.log 2>&1
cc -O1 tests/ota_test.c -o build/ota_test
build/ota_test build/dinghy-1.2-rc2.fwsc > build/evidence-1.2/release-stage.log 2>&1
# Provide the genuine stock file separately; never redistribute vendor firmware here.
if [ -n "${STOCK_V15:-}" ]; then
    python3 - "$STOCK_V15" <<'PY'
from pathlib import Path
import sys
sys.path.insert(0,'tools');import fm1_install as I
p=Path(sys.argv[1]);product,image=I.load_package(str(p),stock_v15=True)
Path('build/stock-v15-verified.fwsc').write_bytes(p.read_bytes())
Path('build/device-head-v15-retained.bin').write_bytes(image[0x400:0x4400])
PY
    build/ota_test build/stock-v15-verified.fwsc build/device-head-v15-retained.bin > build/evidence-1.2/stock-stage.log 2>&1
    python3 -u tests/stock_install_test.py > build/evidence-1.2/stock-installer.log 2>&1
else
    printf 'SKIPPED stock-v15 gates: provide the authentic file using STOCK_V15.\n'
fi
cc -O2 -Wno-unused-function -Ifirmware/hal -Ifirmware/src -Ifirmware/gen tests/render_bank_1_1.c -o build/render-bank
build/render-bank > build/evidence-1.2/render-bank.log
python3 tests/audit_bank_1_1.py > build/evidence-1.2/bank.log
if [ -n "${BASELINE_09:-}" ]; then
    python3 tests/safety_audit.py "$BASELINE_09" build/dinghy-1.2-rc2.fwsc > build/evidence-1.2/safety.log
fi
printf 'FM-1 software release gates passed; physical acceptance remains required.\n'
