# Wanderer 1.0 RC2 verification

This report covers the compiled UI/oscilloscope/splash candidate. Software tests, real-package simulations and emulator execution are separate from physical acceptance. No connected FM-1 was flashed during this work.

## Exact package

| Item | Result |
|---|---|
| File | LVM-Wanderer-1.0-RC2-FM1.fwsc |
| Identity | FM-1_930, shared family identity with RC1 |
| Package size | 612196 bytes |
| SHA-256 | 980fe16310d23dda2ecb00dd79eef4836b446600018f096de098f25fc584ac81 |
| App image | 404484 / 581564 bytes |
| RAM .data + .bss | 19488 / 98304 bytes |
| Static pool | 232928 / 344064 bytes |
| Custom loader | 9040 bytes; identical to 0.9 |
| Custom loader SHA-256 | dac30791235ae6339f7b4b7179fbb59fb59ee6a05aa8442697b5f6407e4ef8dc |
| Known-good 0.9 SHA-256 | c6c0a2829a5c1fe5959cb0d302c4de152b97fe3f29e469c400d1fbaefa236dc9 |

The build uses the same JieLi jieli-linux-toolchains-20260730.1 and AC79 SDK tag AC79NN_SDK_V1.2.1_2023-12-13, commit d179b4484759423312073f5fbb232501aa491047. SDK digests are enforced. A repeated final build produced byte-identical firmware, including regeneration of the selected splash masks. HAL register-access checks and RAM-only flash execution checks pass (916 RAM-text instructions, no calls).

## Application and regression gates

The full Wanderer suite passes with AddressSanitizer and UndefinedBehaviorSanitizer on the host. It covers Anchor preservation, recording, Layer playback, branching, Undo, save validation/migration, boundaries, settings, tempo restoration, adversarial controls, effects, output headroom, OTA, loader and installer behavior. The inherited platform suite also passes, including FM-1/SMK host checks, flash-map tests, MIDI, keyboard layers and update behavior. SMK firmware is not newly delivered or hardware-qualified.

The real application core passes all eight independent slot save/load paths, stopped-only saving, readback comparison, invalid-load preservation, preferences validation, nearest-step recording and Anchor restoration. The 0.8/0.9 project record and RC1 preferences record are unchanged. The 100-preset audition remains audible and distinct: RMS 361–4099, peak 6358/32767. A sanitizer stress run covers all 12800 preset/MIDI-note pairs at velocity 127 with maximum effect controls, complete master mute and panic silence.

Ten thousand direct LED scene transitions pass, including clearing deliberately dirty masks, context changes, note release and panic. The original timed LED/navigation scenario passes with release-dispatched HOME/SEL observations. A second timed scenario asserts both scope chord orders, recording continuity, parent-context restoration, freeze, timebase/gain controls, prevention of accidental Save, sticky-error acknowledgment and rapid earcon on/off. Same-timestamp short taps still sound and release normally.

## Scope isolation and capture

Scope off/on tests render the same musical events through the same production DSP. Their musical PCM matches byte-for-byte: 387072 bytes, SHA-256 593f864df9360ab2dd4129bc7ece9dc39ceace331f6d324cf6a3173ce9ea8e06. The scenario changes scope gain/time while notes continue. Opening and closing the scope does not reset transport, recording, the project, sound selection or the parent view.

Sanitizer tests verify real sample capture, zero-crossing/free-running behavior, frozen-frame preservation, reader-buffer protection across repeated publications, silent flatline, live musical output and panic flatline. A held read buffer remains unchanged while the writer publishes many more frames. Ten thousand scope open/close cycles preserve musical state. New capture timebases are latched per buffer, and the displayed time follows the buffer metadata. Resume discards a partial frozen capture. Capture uses fixed storage and no FFT, heap allocation, renderer calls or waits in audio.

Earcons are excluded from the probe: an earcon-only output produces audible PCM with a zero scope trace. Turning earcons off clears any active cue; new cue requests remain suppressed. Both saved on/off preference round trips pass. The parent Save confirmation is not executed by either order of HOME + SEL/SCL. Holding HOME alone still invokes panic without a second release action.

The probe is the stereo average after Wanderer's digital master and effects, before earcons and the platform's physical MASTER scaling. It is a musical display with manual gain and selectable time span, not a calibrated oscilloscope. Reduced motion lowers refresh frequency, and explicit Freeze pauses only the displayed trace.

## Visual and startup verification

Three hundred renderer states pass native 240×240 text bounds under both sanitizers. They cover both themes, normal/large views, settings pages, selected rows, zero/maximum values, all delay divisions, all detail types, numeric editors, confirmations, persistent errors and extreme scope values/gains. Forty-five production-renderer screenshots cover every major page/state, including live/frozen/silent scope and both startup themes. A 48-frame scope GIF is generated from real app_audio output and the production renderer.

The revised UI removes decorative rings, strengthens focus/boundaries, preserves lane colors, separates listen/record/enable states, enlarges numeric editors and extends Zoom to settings and confirmations. Effect details pair the correct parameter symbol, value and gauge; zero-valued details no longer disappear. Presets remain categorical, while ordinary amounts use normalized percentages. Errors persist until deliberate acknowledgment and do not trigger a second action on acknowledgment.

The refined splash is imported from LVM-Wanderer-Boot-Splash.zip. Both C-rendered themes are pixel-identical to its refined native PNGs. The code uses row-indexed four-bit masks and the existing 60-row display buffer, with no new framebuffer, heap allocation, startup sound, decorative LED state or flash writes. Tests pass for mask bounds/RLE widths, 1100 ms timeout, timer wrap, idempotent dismissal, preserved state/LEDs and MIDI note-through. Twenty-four main-loop first-input cases cover all fourteen buttons, seven encoders, a local key, USB MIDI and TRS MIDI; their original actions remain effective. The no-input timeout also passes without startup saving.

## Compiled firmware execution

fm1-emu 0.3.0 (631b790b1) runs the actual final .fwsc for 4000 emulated milliseconds, with panel interaction to reach the oscilloscope and four simultaneous notes held from 2200–3500 ms. A compiled LCD snapshot visibly shows the real scope. Memory inspection confirms scope_open=1, 140 published waveform captures, splash dismissed and settings closed.

The run reports 38666 interrupts, 660 audio halves and 532 LCD transfers. Audio contains 168960 stereo frames at 44100 Hz, peak 9930/32767 and RMS 1290.5. The emulator reports 14.4 percent of CPU0 instructions inside an ISR, 0.3 percent nested, and 35.4 million ISR instructions per emulated second for this scenario. These are emulator observations rather than worst-case physical CPU or latency measurements. Its script also requests high effect controls; exact maximum-effect arithmetic/headroom is established separately by the host stress test, not inferred from every emulated knob click.

The emulator reports 37 unknown MMIO accesses at 17 addresses. It does not establish physical DAC behavior, display readability, LED brightness, battery operation, USB compatibility or power-loss recovery. The compiled scope snapshot is kept separately from host-renderer gallery screenshots.

## Recovery and real-package checks

Thirty-three protected platform/recovery files are byte-identical to the 0.9 source baseline. The custom loader and generated package head are also identical to 0.9. The original 0.9 baseline was previously rebuilt byte-for-byte to Nicholas's distributed file. RC2 changes application code and art rather than the protected flash driver, loader, bootguard or OCT− rescue path.

The preserved loader core passes real-package application replacements in both directions between RC2 and 0.9, and RC2 and SLOOP 2.8.4 (SLOOP SHA-256 5a6a0843c868650dba6718cf85e43e719aec18c3dc5cc803c57c38b382158d4e). These simulate flash initialized from each package using the Wanderer loader core; they do not execute SLOOP's own physical loader. Head preservation, chip-key rejection before erase, corruption repair/refusal, missing completion and resumed sessions pass.

Authentic v15 is verified as FM-1_015, 699956 bytes, SHA-256 db1642b2b6fa5c2cccb11ffd13878068bb28601678d3644049f99dc40e7edb8a. Its real official-loader staging passes with the retained stock head, including comparison before erases, staged-loader readback and commit only after host confirmation. A mismatched head is rejected. The generated custom package head differs from stock; a compatible custom installation retains the device's original head, which is the head used in this test.

Full host-side simulated transactions pass for RC2→official v15, resuming the exact official-v15 loader, refusing stock from an unfinished custom loader, rejecting corrupt v15 before device probing, and installing/verifying RC2 from 0.9, 1.0 and a custom identity. The official loader's physical full-chip write and reboot are not executed by the host simulation. The vendor v15 file is a private test input and is not redistributed.

## Reproduce and accept

Set JIELI_TOOLCHAIN and AC79_SDK to the pinned dependencies, then run `STOCK_V15=/path/to/genuine/FM-1.fwsc sh tests/run_release_tests.sh`. Stock checks explicitly skip when that file is unavailable. The suite builds the final RC2 candidate, runs the regression/core/scope/UI/splash tests, asserts real-loop navigation, compares musical PCM and captures the screens. No test script flashes a connected device. tests/safety_audit.py compares the 0.9 protected source and loader bytes; tests/ldr_test.c exercises real custom-package replacement; tools/make_release_docs.py builds the guide/gallery. Raw results accompany the source release.

This remains 1.0 RC2 until the new bytes pass physical installation, all controls, speakers/headphones, live rendering/timing, LED evaluation, save/power-cycle/load, v15 rollback and a chosen custom-firmware round trip on an FM-1. Prior physical testing covered 0.9 only. Shared project storage can be overwritten by other firmware. External project export, audio recording/resampling, external MIDI clock/transport, pitch bend, MPE and onboard speech remain outside the implemented feature set.
