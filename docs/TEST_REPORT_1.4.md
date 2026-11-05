# Wanderer 1.4 RC1 verification

Candidate SHA-256: `d4a8440b1bc13f4508c3862cb4f020dc4ff911e6c6c5b6f152535a4c8171f8b8`. Package: 612,196 bytes; identity FM-1_929. This is a software-verified test candidate, not a physically accepted release.

The complete source was recovered from the previous session. Its unmodified RC4 build reproduced the user's tested SHA-256 `91633c4d0e3de94fd15722fce062504243c0d623437317be96434165c3b116ce` exactly. Development started in an isolated worktree at commit `2131515`.

| Check | Result |
|---|---|
| Fresh sanitizer, host, MIDI/input, state, Print, scope, editor and installation/loader suite | 49/49 passed |
| Additional 1.4 integration | All 13 patterns and 12 rates, gates/swing/octaves, 64-entry overflow, MIDI ownership/sustain/latch, generated recording, selected clock, malformed control bounds and real v7 migration passed |
| Scatter/printed source | Every mode × grain size, both edge extremes, protected audio Anchor, Print/Undo/Redo passed |
| Callback segmentation | 1,092 comparisons; blocks 1, 17, 64, 255, 256, 389 and 1,024 produce identical arp output and recordings |
| Sustained host run | 600 seconds with both event layers, max effects, four shared voices, changing Scatter/arp settings, immutable Anchor and clean panic |
| Engine extremes | Existing 480 engine/filter/routing/register cases pass without sanitizer overflow |
| Interface | 296 production captures; text bounds and two-line row widths checked; dark/light/Zoom and exact-package emulator screenshots visually inspected |
| Target code | 398,272 / 581,564 bytes |
| Ordinary RAM | 63,600 / 98,304 bytes |
| Audio pool | 315,392 / 344,064 bytes |
| Boot stack audit | Scanner 20 bytes; conservative migration chain 2,964 / 4,096 bytes |
| Recovery integrity | All 33 protected source files, compressed loader and package head identical to RC4 |
| Exact final package emulation | 5,200 ms, 221,952 audio samples, arp controls/held notes/panic; 10.9% average emulated ISR, 0.2% nested |

The emulator was deliberately bounded by 1,248,000,000 executed instructions; its reported time reached 5,200 ms. This is a simulated timing observation, not a physical worst-case ISR/deadline measurement. The emulator reports its known unsupported MMIO bring-up accesses. A longer 14-second development run exercised Scatter, recording and Print and averaged 22.3% ISR; it preceded the final gain/spacing adjustments and is supporting development evidence rather than an exact-file acceptance result.

The 100-preset single-note bank render has peak 8,090 / 32,767 at the audition's test note/velocity; one-second RMS spans 240–3,142 because short percussion and sustained sounds differ. The before/after WAV preserves actual levels instead of normalizing away the 6 dB gain change. These are production-DSP host renders, not recordings from an FM-1. Sound quality remains a listening judgment.

The bundled 26-second Scatter/arp demo uses two seconds per segment: clean, Mixed, Slices, Hops, Reverse, Tape, Cloud, Pitched, Fills, then Up, Down, Up Down and Chord arp examples. The before/after timeline is supplied separately.

Physical acceptance remains: installation and rollback, real output/headphone levels, listening to bass/pads/chords, encoder feel, rapid menu switching, long playing with both layers/effects/Print, and external MIDI timing during LCD redraws. External clock follows the arp; it does not turn the event-memory/Print transport into a full external-clock slave. No public release or Firmware Hub update was made.

Run `python3 tools/smoke_1_4.py` for the integrated suite. Additional tests are `tests/wanderer_1_4_timing.c`, `tests/wanderer_1_4_stress.c`, `tests/capture_1_4.c`, `tests/target_boot_stack_test.py` and `tests/safety_1_4.py`. Compile host C tests with the include paths and sanitizer flags in the smoke runner. Safety comparison needs the verified RC4 file; stock installation simulation needs the verified original V15 fixture, which is not redistributed.
