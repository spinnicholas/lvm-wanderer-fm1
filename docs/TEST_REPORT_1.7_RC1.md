# Wanderer 1.7 RC1 verification

FM-1 candidate built November 2, 2026. Physical acceptance is still required.

Firmware: `LVM-Wanderer-1.7-RC1-FM1.fwsc`

Identity: `FM-1_929`. Size: 612,196 bytes.

SHA-256: `7891fff2915d9efb59bb679d483de0f3355d7268ef68c4365b981f8cffdec8c7`

## Source and signal checks

`python3 tools/smoke_1_7.py` passed all **68 build/execution checks**. C tests
use AddressSanitizer and UndefinedBehaviorSanitizer with recovery disabled.
Coverage includes project import/save/Anchor/undo, release tails, callback
segmentation, MIDI addressing, Print/Scatter/arp operations, Zoom, cues, OTA,
and loader upgrade/downgrade. The navigation fixture was updated for the added
settings pages and the entire suite rerun successfully.

The focused routing test passed **4,590 exact comparisons**: local versus MIDI
at matching velocity, all 27 keys through five octaves and four voice slots in
all six engines; plus all 150 presets across slots around G3. It also passed
76 model/waveform cases at pitch extremes. A separate test passed 64 native
MSFA algorithm/register/feedback/release cases and v9/v10 project/Anchor import.
No physical G-key repair is claimed by these software comparisons.

Effects checks established exact bypass below the master limiter knee,
16-bit/maximum-rate transparency, 4-bit quantization, earlier multi-head tape
taps, separation of chorus storage from echo feedback, identical output across
five callback sizes, bounded seven-effect extremes, reset and silence. All new
effect-control endpoints validate in project records.

All **150 auditions plus 1,350 register/Melt probes** passed, with nonzero
output and no digital clipping; maximum peak 12,739. The full factory stress
passed **450 four-voice/seven-effect cases** with extreme feedback, mixes,
Master, and Melt; peak 28,133. The hostile parameter test passed 480 older
engine/filter/routing/register corners; peak 28,175. The limiter deliberately
bounds overload, so bounded peaks alone do not establish clean or pleasing
sound. Listening remains necessary.

The editor passed 3,072 engine/page/control/endpoint/theme/Zoom renders without
clipping or label collisions. Steady numeric Zoom passed 46,080 range renders.
Color and effect pages were captured in both themes and sizes; nine final
compiled-device captures were inspected.

## Target build and execution

Image: **537,572 / 581,564 bytes**. Ordinary RAM: **72,800 / 98,304 bytes**.
Pool: **343,040 / 344,064 bytes**. The build checks all limits. The conservative
boot/migration stack bound is **2,964 / 4,096 bytes**; slot-scanner frame 20
bytes. The pool is close to its fixed limit, so future buffer additions require
another memory audit.

All 33 protected platform/recovery source files match the verified baseline
manifest. The loader entry and package head match the exact accepted 1.4 RC6
package. Package validation, OTA and loader tests passed. The manifest makes
the source audit reproducible from the source ZIP without needing Git history.

The exact candidate above completed **9,500 simulated milliseconds** in
fm1-emu 0.3.0. The script enabled all four new effects, enabled digital delay
alongside the default chorus/reverb, and held an F3/G3/A3/C4 chord on Neon Reed
FM6. Captures confirmed each effect enabled and preset 28 sounding; MIDI bank
one followed by PRESET wrapped the factory selection to 1. The script is
`tools/audit_emulator_1_7.py`.

Audio: **411,648 stereo frames at 44.1 kHz**, peak 9,033. The emulator drew 244
LCD frames. Average ISR instruction load was **30.3%**, with 0.7% nested. This
is an average scripted emulation result, not a physical worst-case deadline
measurement. Incomplete MMIO modeling remains reported by the emulator.

## Listening files and reproduction

The preset book embeds 150 dry factory-level previews. The ten A/B pairs play
1.6 then 1.7, matched in RMS over the held portion with attenuation only. The
effects demo is 28.08 seconds: four dry/wet pairs from the production DSP.
Embedded preset metadata/audio, complete WAV length and JavaScript syntax were
checked. These previews do not prove subjective superiority.

Build with the pinned toolchain/SDK in `README-DINGHY.md` using
`sh build.sh --release 1.7-RC1`. Run the smoke suite, the dedicated full-bank
renderer and seven-effect stress test, then the safety and stack audits. The
emulator script accepts the fm1-emu executable path. `VERIFICATION_1.7_RC1.json`
records the final values. Hardware audio, touch behavior, USB/MIDI timing and
sound quality remain physical acceptance items before publication.
