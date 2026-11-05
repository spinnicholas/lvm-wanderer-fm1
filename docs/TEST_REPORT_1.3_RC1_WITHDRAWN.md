# WITHDRAWN: RC1 has a physical startup stack overflow

See BOOT_FIX_1.3_RC2.md. The following historical checks missed that hardware guard failure.

# Wanderer 1.3 RC1 verification

## Exact candidate

Identity **FM-1_929**. Package **612,196 bytes**. SHA-256: `b5f50a6608d569ba11a057026de1eb2d55c6429b797dd96709a7f3eb5212e788`.

The target was built with the pinned JieLi 20260730.1 toolchain and the SHA-verified AC79 SDK files. No new firmware was flashed or publicly published.

| Region | Used | Limit | Free |
|---|---:|---:|---:|
| Application | 362,776 B | 581,564 B | 218,788 B |
| Ordinary RAM | 39,488 B | 98,304 B | 58,816 B |
| Large-buffer pool | 315,392 B | 344,064 B | 28,672 B |

## Software checks

**45 fresh compile/execute/protocol checks passed.** AddressSanitizer and UndefinedBehaviorSanitizer were enabled for host C tests. LeakSanitizer alone was disabled for this managed runtime.

- Legacy v1–v5 imports, malformed records, all eight project slots, patch/Anchor snapshots, source-slot preservation, dirty/Undo behavior and Save readback.
- Local keys, sustained chords, voice stealing, quantized capture, two-bar wraps, separate MIDI source/channel ownership, sustain, panic and exact note release after transposition.
- All 100 modern patches audible; 12,800 preset/register pairs; full four-voice/effects stress; 480 hostile engine/filter/routing/register cases after effect-buffer warmup. All outputs bounded, with no sanitizer overflow.
- Phrase-preserving Drift and resolution, low-density phrase-ending Repeat, 12 callback-size runs with identical repeat audio/printed bytes.
- Three-arena Print, two longest 8-second passes, first Undo to dry Anchor, completed-generation Undo/Redo, cancel preserving current generation and immutable audio Anchor.
- Save from inside the editor, modal priority, confirmation cancellation, persistent Save results, complete event sound restoration and audio Anchor restoration surviving Home.
- Stop refreshes stale bright/dim note LEDs in normal and editor/Print scenes; 256 repeated starts/stops; live-key release remains correct.
- Simulated OTA staging, custom loader upgrade/downgrade, authentic-v15 transactions, malformed package rejection and installer protocol behavior.

## Visual verification

964 new native production-renderer captures cover engine contexts, every preset name, both themes, Zoom/control selection, Print states and the audio-duration lock. 2,400 parameter-range/theme/Zoom editor renders pass bounds and label-collision checks; legacy settings range tests remain intact. Contact sheets were inspected for text, spacing, strong selection borders, palette and recovery cues.

58 of 66 existing screens are pixel-identical. The eight intentional differences are Repeat instead of Decay, preset 100's name, three changed modern-synth scope traces and three named-scale views. The licensed icon pipeline independently decodes all 235 masks to the exact source-rendered alpha pixels, including every legacy icon. Only the new Repeat glyph was added; Lucide/Tabler and OFL notices remain present.

## Compiled firmware

The exact package above completed **8,800 ms** in fm1-emu 0.3.0, including four FM voices, record, two Print passes, Undo/Redo and stop. Audio: 380,672 stereo frames at 44.1 kHz, peak 3,415; seven actual LCD captures were visually checked. Average emulated ISR instruction load: 15.2%.

The earlier script that sent Record during startup did not capture a loop; it was replaced by the after-startup sequence used for this completed run. The final scenario script/log and exact input firmware hash are retained.

Average emulated load does not establish a worst-case physical audio deadline. The emulator reports incomplete MMIO modeling. Real-device installation/rollback, DAC/speaker listening, button/LED hardware and the 5.805 ms callback deadline remain pending on a physical FM-1. Subjective improvement over stock requires level-matched listening there.

## Protected foundation and limits

All 33 protected platform, HAL, loader, flash layout and recovery source files match the prior baseline. The compressed loader and retained first 16 KiB package head are byte-identical to the hardware-tested 0.9 foundation. The permanent identity is unchanged. New synth/UI code is not covered by old hardware results.

Print is 16 kHz mono block ADPCM, up to 8.192 seconds, with a 44.1 kHz synth/output and compact 16 kHz stereo wet bus. Printed audio lives in RAM and is not saved to flash; project Save stores notes and patch snapshots. Starting an actual next Print consumes the old Undo workspace, while keeping current and Anchor safe. Tempo/length are locked while audio exists. Imported legacy patches retain their voice bank and Decay semantics; ambience is implemented by the new compact effects engine.

## Reproduce

Use the dependencies and original v15 fixture described in README.md and INSTALL_AND_RECOVERY.md. Build with `sh build.sh --release 1.3-RC1`. Run `python3 tools/smoke_1_3.py`; compile/run `tests/capture_1_3.c` and `tests/capture_screens.c`, then `python3 tools/review_screens_1_3.py`. `tools/audit_release_1_3.py` verifies linked limits, identity and unchanged recovery files. The complete source, licensed original assets, generated tables and evidence accompany this candidate.
