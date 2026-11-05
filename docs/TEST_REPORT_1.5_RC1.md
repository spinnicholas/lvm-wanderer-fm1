# Wanderer 1.5 RC1 verification

This candidate replaces the factory collection with 150 original presets using all five engines. It also expands panel navigation, preset names and MIDI addressing, and retains old factory data for project migration. The audio synthesis algorithms are retained apart from an early skip for inactive voices, preventing a stale modern preset number from reaching the legacy 100-entry lookup.

## Exact candidate

The installable file is `LVM-Wanderer-1.5-RC1-FM1.fwsc`, 612,196 bytes, permanent identity `FM-1_929`. SHA-256:

```text
cda435a89f7b268dcef578a2df8389cd8a4adad6ddfa3c1abfcd06289d7062ce
```

The target image is 473,588 / 581,564 bytes. Ordinary RAM is 66,160 / 98,304 bytes, and the large-buffer pool is 315,392 / 344,064 bytes. The linked boot/migration stack audit bounds the checked chain at 3,116 / 4,096 bytes; the project scanner frame is 20 bytes. The data tables and level trims are generated offline and deterministically.

## Audio and behavior

All 150 presets have distinct valid patch data and distinct production-DSP audition bytes. Their three-second dry auditions have peaks from 1,536 to 8,829, without digital clipping. All 1,350 probes across three registers and three Melt values are audible and bounded under AddressSanitizer and UndefinedBehaviorSanitizer. Different decay and sustain behavior is intentional; metrics do not establish subjective sound quality.

All 450 factory-preset/Melt cases pass with four held voices, maximum effects, maximum feedback and Master 127. Peak output is 30,380, without digital clipping or sanitizer findings. Panic releases the voices. Preset levels are compiled from active-body measurements, keeping short percussive attacks distinct from sustained tones.

All 62 compile, execution and protocol regression checks pass. They cover event memory, older project imports, current save/readback, malformed records, source-slot preservation, patch and Anchor snapshots, musical capture, arpeggiator, Scatter, Print/Undo/Redo, effects, MIDI ownership and sustain, accessible controls, Zoom, display hold, cue behavior, OTA, upgrade/downgrade simulation and installer behavior. MIDI Bank Select is tested for delayed action, invalid combinations, per-channel isolation and per-input isolation. New v9 saves include preset provenance; actual v8-sized records retain their old patches, names and fallback bank.

All 100 previous 1.4 factory sounds render byte-identically between the 1.4 source and the 1.5 imported-factory path, using identical two-note auditions, Melt 63 and note releases. Hashes are included for both runs. This verifies retained 1.4 factory behavior; it is not a claim that old 1.3 synthesis is identical to 1.3 hardware.

## Display and compiled execution

There are 1,192 new native production-renderer captures, including all 150 names in both themes and both normal/Zoom layouts. Editor, Print, Scatter and notification contexts are included. The preset contact sheet was inspected for spacing and legibility, including the fifteen bank marks and preset 150. Existing editor-range, stable numeric Zoom and screen-bound checks pass.

The exact package above completed 9,500 simulated milliseconds in fm1-emu 0.3.0, playing analog, FM, phase, string and modal sounds, selecting preset 150 through MIDI bank one, and wrapping from 150 to 1 with PRESET. The run produced 411,648 stereo frames at 44.1 kHz, peak 4,076, and seven inspected LCD captures. Average emulated ISR instruction load was 23.2%, with 0.5% nested. This average is not a physical worst-case audio deadline measurement. Known incomplete MMIO modeling remains reported.

All 33 protected platform, HAL, loader, flash-layout and recovery source files match the accepted 1.4 baseline. The compressed update loader and retained first 16 KiB package head are byte-identical. Identity, package CRC and loader checks pass.

## Physical audition

This new candidate has not yet been accepted on a physical FM-1. The prior 1.4 hardware acceptance does not qualify the new bank. The next check is the author’s listening and playing test, particularly held chords, low/high registers, patch levels, old project recall and preferred Scatter/effect combinations.

Rebuild with the pinned dependencies in `README-DINGHY.md` and `sh build.sh --release 1.5-RC1`. Run `WANDERER_RELEASE=1.5-RC1 python3 tools/smoke_1_5.py`. The additional bank, four-voice, register/Melt, previous-bank comparison and capture programs are in `tests/`; their logs and metrics are in `test-evidence/1.5-RC1/`. The preset book includes all 150 dry audio previews, and the complete source includes the authored recipes, frozen old banks and offline level trims.
