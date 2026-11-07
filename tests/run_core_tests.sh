#!/bin/sh
# Host regression suite used before every native build. Plain C, no hardware needed.
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/ci build/rc4 build/screens
CC="${CC:-cc}"
for test in wanderer_3_0_test wanderer_engines_3_0_test wanderer_splash_test wanderer_adversarial_2_0 wanderer_2_0_test wanderer_filter_test wanderer_reverb_test wanderer_layer_workflow_test wanderer_scatter_patterns_test wanderer_scatter_test wanderer_grain_512_test wanderer_fx_1_7_test wanderer_release_test wanderer_memory_test wanderer_1_2_test wanderer_engine_1_6_test wanderer_stop_led_test wanderer_theme_test wanderer_1_4_timing wanderer_anchor_replace_test; do
  [ -f "tests/$test.c" ] || continue
  "$CC" -O1 -Wno-unused-function -Ifirmware/hal -Ifirmware/src -Ifirmware/gen "tests/$test.c" -o "build/ci/$test" -lm
  echo "== $test"
  "build/ci/$test" > "build/ci/$test.log"
  tail -n 2 "build/ci/$test.log"
done
python3 tests/theme_palette_contrast.py
echo "CORE HOST TESTS PASSED"
