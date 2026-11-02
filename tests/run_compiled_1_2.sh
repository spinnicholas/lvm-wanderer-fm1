#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Execute final FM-1 package in pinned fm1-emu 0.3.0; never flashes hardware.
set -eu
cd "$(dirname "$0")/.."
: "${FM1_EMU:?Set FM1_EMU to the fm1-emu 0.3.0 executable}"
mkdir -p build/compiled/work build/evidence-1.2
cd build/compiled
"$FM1_EMU" --fwsc ../dinghy-1.2-rc2.fwsc --ms 7000 --steps 1680000000 \
  --keys '1200-1260:REC,1400-2200:n7,1400-2000:n10,1400-2400:n14,2500-2560:REC,2700-2760:PLAY,3000-4000:SAVE,4200-4260:SCL,4500-4560:HOME,4800-4860:ARP,5100-5160:SCL,5400-5460:HOME,5600-5660:PLAY,5650-5680:SAVE,5700-5760:HOME,5850-5910:PLAY,6000-6060:LFO,6100-6160:PLAY,6500-6900:n22,6550-6610:HOME' \
  --wav compiled-chords.wav --snap 1800,4100,4400,5300,6400,6800 \
  > ../evidence-1.2/compiled.log 2>&1
cd ../..
python3 tests/audit_compiled_1_2.py
