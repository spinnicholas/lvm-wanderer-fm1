# Wanderer 1.6 RC1 verification

Candidate built on November 2, 2026. Physical listening is still required.

Filename: `LVM-Wanderer-1.6-RC1-FM1.fwsc`

Identity: `FM-1_929`

Bytes: 612,196

SHA-256: `c781efe21a524607d84d0bc94fb5a816ac97106da5fcba529f883085edaeab5e`

## Production-source checks

`python3 tools/smoke_1_6.py` passed all **64 build/execution checks**. C test
executables use AddressSanitizer and UndefinedBehaviorSanitizer with recovery
disabled. The suite covers note timing, release tails, callback segmentation,
project versions and snapshots, MIDI addressing, Print/Scatter/arp operations,
accessibility shortcuts/cues/steady Zoom, UI assets, OTA and loader upgrade and
downgrade between the candidate and the recovery firmware.

The focused engine test passed **4,050 exact output comparisons**: local/MIDI
onsets at matching velocity on all 27 keys across five octaves, through every
voice slot in five engine families; and every factory patch across slots at
F-sharp3/G3/G-sharp3. It also passed 60 new waveform/model cases at MIDI pitch
extremes with maximum modulation, plus v9-to-v10 project/Anchor import and
malformed-file rejection.

These are software comparisons with matching onset state. They do not test the
physical touch sensor or establish the exact cause heard in the submitted
video. The selected preset was not identifiable from that video.

## Audio coverage

All **150 production-DSP auditions plus 1,350 register/Melt probes** passed,
with nonzero output and no digital clipping. The probes use the playing note
and adjacent octaves at Melt 0, 64 and 127. Their maximum peak was 12,739.

The separate full-bank stress run passed **450 four-voice cases** at maximum
effects, feedback, Master and Melt endpoints/midpoint; peak 28,124. The hostile
parameter test passed **480 engine/filter/FM-routing/register corners** with
four voices and maximum effects; peak 28,177. No sanitizer overflow was
reported. The master soft limiter is deliberately active under overload, so
bounded peaks alone are not evidence of clean sound or subjective quality.

The supplied low-G audio plays F3/F-sharp3/G3/G-sharp3/A3 through Velvet Saw,
Felt Tines, Phase Reed, Cedar Pluck and Bronze Bowl at fixed velocity. Spectral
estimates of the fundamental in those 25 renders were within 1.1 cents of
nominal, including the analog patch's deliberate small detune. No isolated G3
pitch excursion appeared. This is an offline estimate, not a hardware tuning
measurement or a comprehensive timbre test.

The preset book contains 150 unnormalized factory-level audio previews. Its
embedded records and MP3 payloads were validated; its JavaScript passed syntax
checking. The nine before/after pairs are matched in RMS over the held portion
with attenuation only. Their order is recorded in `Comparison-Order.txt`.
Loudness matching applies only to that comparison, not the bank previews.

## Display, target build and protection

The UI checks passed 2,560 engine/page/knob/endpoint/theme/Zoom editor renders
without clipping or label/value collisions, plus 38,400 steady numeric Zoom
range renders. Eighty Color-page captures, sixteen bank-name views and seven
compiled LCD captures were produced; the review sheets were visually checked.

The target build passed: image **520,068 / 581,564 bytes**, ordinary RAM
**69,280 / 98,304 bytes**, pool **315,392 / 344,064 bytes**. The conservative
boot/migration stack bound was **3,116 / 4,096 bytes**, including the new v9
import path. Slot-scanner frame: 20 bytes.

All **33 protected platform/recovery files**, the update-loader entry and
package head matched the exact hardware-accepted 1.4 RC6 baseline. Package
parsing/identity, OTA, and loader tests passed. Use `tests/safety_1_6.py` and
`tests/target_boot_stack_test.py` to repeat those audits after a build.

## Exact compiled package execution

The exact package hash above completed **8,500 simulated milliseconds** in
fm1-emu 0.3.0. It played four-note F3/G3/A3/C4 chords in Analog, FM, Phase,
String and Modal, selected preset 150 through MIDI bank one, and wrapped from
150 to 1 with PRESET. The inspected LCD captures showed the expected sound
names and numbers. Audio contained **367,616 stereo frames at 44.1 kHz**,
peak **9,548**; 196 LCD frames were drawn.

Average emulated ISR instruction load was **29.8%**, with **0.6% nested**.
This is an average of one scripted run, not a physical worst-case deadline
measurement. The emulator reported 317 accesses to 17 incompletely modeled
MMIO addresses; it did not stop on an execution fault. Hardware audio, touch
behaviour, USB/MIDI timing and final sound quality remain physical acceptance
items. This candidate has not received that acceptance and is not the public
stable release.

## Reproduction

Use the pinned SDK/toolchain in `README-DINGHY.md`, then build with
`sh build.sh --release 1.6-RC1`. Run the smoke suite and the dedicated
`render_bank_1_6.c`, `wanderer_bank_1_5_stress.c`, `render_g_1_6.c` and
`capture_1_6.c` programs with the host include paths used by the smoke suite.
Generate listening material with `tools/make_1_6_docs.py` and
`tools/make_1_6_comparisons.py`; the latter also needs baseline 1.5 audition
raws. `docs/VERIFICATION_1.6_RC1.json` records the final measured values.
