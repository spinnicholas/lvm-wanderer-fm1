# LVM Wanderer 1.8 Scatter development candidate

**Status: SOURCE-ONLY DEVELOPMENT BUILD. NOT A FLASHABLE RELEASE.**

This branch is derived from the **uploaded, physically tested 1.7 RC4 source package**, not the older public GitHub source. The user's original RC4 `.fwsc` file is untouched. No firmware has been flashed, uploaded, or published by this work.

## What changed

- **The original 512 ms mode is preserved** at its previous index and with its previous 75%–125% randomized grain-duration window. Two options are appended: **768 ms** (fixed maximum history length, avoiding a larger allocation). The experimental 1-second option is deferred until native memory headroom can be established. Shorter sizes and Auto are unchanged.
- The history ring increases from **10,240 samples / 20 KiB** to **12,288 samples / 24 KiB**, using a single conditional wrap. This requires **4,096 additional bytes for the history**. Three mutually exclusive, main-thread-only project-migration paths now share one workspace, recovering approximately **4.4 KiB**. RC4 had approximately 9 KiB spare in ordinary RAM; the target linker must still confirm the new total. The three Print/Anchor/Undo ADPCM arenas and the shared effects pool are unchanged. The native linker must still verify that ordinary RAM has sufficient space.
- A new **PATTERNS** editor page is appended after COLOR, leaving existing editor page indices and Scatter controls intact. Its four physical knobs control PATTERN, VARIATION, MIX, TONE, including in Zoom mode.
- 10 algorithm selections: FREE (original behavior), PULSE, EUCLID, CASCADE, DRUNK, ECHOES, REWIND, FILLS, MORPH, CHAOS. Sound choice (Pitched, Cloud, Tape, etc.) remains separate. Algorithm decisions happen once per sequencer step, never inside the sample-rate render loop.
- Pitched remains the same playback style; Cloud retains its two-head blend. Tape retains the original half-speed character but adds very subtle continuous wow and flutter using cheap triangular phase generators, updated at 16 kHz only during active Tape events.
- Steady Zoom retains stable numbers. The 768 ms option is shown in milliseconds; no new unit change occurs. All 17 page-indicator positions remain within the 240-pixel screen.
- The existing effects tape-echo engine skips multi-head interpolation and modulation when its wet mix has fully reached zero, but **continues maintaining live history** so enabling it does not reference stale tape. Active sound processing and effect-tail fades remain untouched.
- A redundant unused `% wp_repeat_size` calculation is removed from the 16 kHz loop. Scatter now computes one grain coordinate for both playheads and caches modulo/window weight across repeated 44.1 kHz samples; Print sample reading avoids `% length` when already in range. This addresses unnecessary hot-path integer divisions without changing how transport events are scheduled.
- Disk version is increased to **12**. An explicit **read-only version 11 import** is added so saved RC4 projects can load without treating the old trailing padding byte as a pattern setting. Old projects default to FREE; they are not rewritten until an explicit Save.

## Audio and performance caveats

The software uses a single bounded history, no runtime heap allocation, and the original effects and synth voice counts. There is no new 44.1 kHz oscillator or per-sample random scheduler. The source ZIP includes the already-generated C glyph header but not the standalone typeface binary; `tools/build.py` retains that header if the optional typeface file is absent. However, **true audio-interrupt timing and target RAM are not verified** without the JieLi toolchain, AC79 SDK, linked firmware map and actual FM-1 testing. Do not interpret host timing as an assurance of lag-free hardware behavior.

Before any flashable package is released: cross-compile with the pinned JieLi pi32v2 toolchain and SDK; inspect linker `.data + .bss` below 96 KiB and `.pool` below 344,064 bytes with safe margins; run host sanitizers and platform updater tests; boot in the FM-1 emulator; perform extreme polyphony and 768 ms Scatter stress tests on hardware; verify all saved project migration and upgrade/rollback routes manually.

## Code references used for research

- SLOOP: `firmware/src/slicer.c` and `firmware/src/eng_grain.c` — pulse masks, windowed repeats, Hann envelopes and bounded work. GPL-3.0-only: https://github.com/isod89/sloop-fm1
- Mutable Instruments Clouds: `clouds/dsp/grain.h`, `clouds/dsp/granular_sample_player.h` — envelope blending and probabilistic versus deterministic grain scheduling: https://github.com/pichenettes/eurorack
- DaisySP: `Source/Sampling/granularplayer.cpp` — overlapping dual readers: https://github.com/electro-smith/DaisySP
- Lunar Modulator: `engines/src/fx_warble.cc` — independent slow wow and fast flutter: https://github.com/ip2k/lunar-modulator
- B.Oops: `src/FxStutter.hpp` and its pattern sequencer — rhythmic probability, crossfades and repeat gestures: https://github.com/sjaehn/BOops

The new C code is an independent FM-1 adaptation of those ideas; it does not transplant those third-party code files.

## Reproduce the host tests

From the extracted project root:

```sh
mkdir -p build/local build/rc4
cc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all \
  -Wno-unused-function -Ifirmware/hal -Ifirmware/src -Ifirmware/gen \
  tests/wanderer_scatter_patterns_test.c -o build/local/scatter-patterns
ASAN_OPTIONS=detect_leaks=0 ./build/local/scatter-patterns
```

Additional existing test cases run successfully in this session: `wanderer_grain_512_test`, `wanderer_scatter_test`, `wanderer_1_4_test`, `wanderer_1_7_test`, `wanderer_fx_1_7_test`, `wanderer_release_test`, `wanderer_editor_test`, `wanderer_memory_test`, `wanderer_ui_test`, `wanderer_zoom_fit_test` and `wanderer_steady_zoom_test`. Host benchmarks were repeated nine times per scenario; medians for Pitched, Pitched with tape echo, Cloud, and Tape were approximately **0.101, 0.102, 0.098, 0.099 s** on this machine versus **0.111, 0.108, 0.107, 0.106 s** for RC4 on the same tests. These are short host timing microbenchmarks, not hardware CPU measurements.

The accompanying 18-second host WAV preview is split in this order: **Pitched + Cascade (512 ms)**, **Cloud + Morph (768 ms)** and **Tape + Drunk (768 ms)**. It is generated by `tests/wanderer_scatter_audition.c`, not recorded from FM-1 hardware; musical impressions may differ on the actual instrument.
