> Hardware follow-up, November 1, 2026: Nicholas confirmed that the latest 1.2 RC2 passed the same real-FM-1 installation and rollback tests: V15 → Wanderer, SLOOP → Wanderer, Wanderer → V15, and Wanderer → SLOOP. This supersedes the pending-acceptance label retained below from the development-session report. See CANDIDATE_STATUS.md.

# Wanderer 1.2 RC2 verification

These are fresh software checks on the 1.2 application. No connected FM-1 was flashed. Physical installation/rollback reports inherited from the user concern 0.9, and do not establish that these new application bytes are ready for a stable label.

## Exact candidate

|Item|Result|
|---|---|
|Package|LVM-Wanderer-1.2-RC2-FM1.fwsc|
|Identity|FM-1_932|
|Size|612196 bytes|
|SHA-256|e1a18e8f2d700399f6cac736e9587031d4a91cc7f286cc32172b45d0489ef378|
|Application|520752 / 581564 bytes|
|RAM data + BSS|35680 / 98304 bytes|
|Static pool|232928 / 344064 bytes|
|Custom loader|9040 bytes; identical to 0.9|
|RAM-only flash code|916 instructions, no calls|
|Project record|v5: 2084 bytes, including 2076-byte state|
|Imports|v1/v2 legacy; v3 160-byte record; v4 548-byte record|

The pinned JieLi Linux toolchain 20260730.1 and AC79NN SDK V1.2.1 build the final package. The build checks pinned SDK input digests, HAL-only register access and flash-code placement. A second build is byte-identical to the recorded final package.

## Additional RC2 regressions

RC1's full-step cyclic replacement could discard a newly retriggered note while an older held onset survived. It also preferred an arbitrary held position over an available released position. Both cases now preserve the expected notes: released positions yield first, otherwise the oldest held onset yields. Capture-pool recency comparison remains correct through unsigned serial wrap. The three reproducers fail against the immutable RC1 source and pass against RC2 with AddressSanitizer and UndefinedBehaviorSanitizer.

Local keys detected together now share one quantization decision sampled before individual note processing. A targeted test moves the scheduler across the half-step boundary between captured notes and verifies all three notes occupy the same onset and replay together. It adds no input waiting window. Notes from separate input scans and MIDI messages still follow their own arrival times.

HOME returning from Anchor now ends pending capture and releases the old sequenced gates, matching LFO. Live key gates remain held, and natural envelope/effect tails may fade. The actual main-loop test reproduces RC1 leaving two gates after HOME where one live gate should remain; RC2 leaves the expected live gate, which releases with its key. The direct listening-transition test also covers both sequenced lanes and capture finalization.

These refinements do not change v5 project layout, synth/effects, display layout, scope, splash, or update/recovery code. RC1 and RC2 share identity FM-1_932 and must be distinguished by filename/SHA-256. Both read each other's v5 projects. Exact RC1 binaries and sources remain preserved.

## Recording and project behavior

AddressSanitizer and UndefinedBehaviorSanitizer exercise the actual application. Three captured notes retain separate release times and replay as three, then two, then zero held voices. Anchor, branch and scale evolution retain every chord event and gate. Five/six-note overflow leaves the four most recent onset positions. Old note releases cannot close a replacement capture. Held overdubs clear covered repeats of the same pitch while preserving different pitches. Source/channel-aware MIDI sustain and release, independent capture ownership, loop-boundary gates and simultaneous Memory/Layer allocation pass. The four-voice sound budget is unchanged.

An 8192-block rapid MIDI test mixes note-ons, releases and sustain across both inputs and multiple channels, with repeated two-bar wraps and maximum wet effects, feedback, room size and digital level. Captures stay within the four-note lane budget, complete event records remain valid, filters remain bounded and every capture can be released. This supplements the deterministic independent-duration cases; it is not a hardware jitter benchmark.

Empty recording passes preserve prior Undo. Saving establishes the correct clean reference. Undo after Save marks prior music edited; Undo after Load restores the previous session/reference. Current accessibility choices survive Undo. The slot shown on Home follows a successful Save/Load, independently of slot browsing. Boot probes validity without writes or automatic project loading.

Injected slot-write and readback faults never claim Saved, even when bytes may have reached storage. A project commit followed by a preferences-write fault shows distinct music-success/preferences-failure status, retains a pending edited state and permits a successful retry. All eight slots round-trip. Exact v4 import retains root, scale, patch, 32-step notes and durations, and leaves old bytes unchanged. Bounds checks reject invalid note, velocity and gate fields in every primary/extra position across all four phrases. Historical v1/v2/v3 import tests also pass. New v5 projects deliberately require 1.2.

A timed actual-main-loop scenario records and releases a chord, stops, holds SAVE, confirms, acknowledges, undoes, reloads, listens to Anchor and clears/cancels Layer. Assertions verify every state and dirty/reference transition. A no-Anchor LFO request preserves recording. PLAY works inside confirmations. Short SAVE remains Catch on release; the 900ms hold opens Save without Catch. Inherited short-tap and scope chord-order scenarios pass.

## UI, lights, scope and startup

All 66 renderer captures are reviewed: every major menu, row/detail, confirmation, error/outcome, dark/light, Zoom, scope, two-bar/chord timeline and approved splash. A full-range 2048-render settings test checks both themes and sizes. Existing 300-state UI and 1152-state Music tests pass. New footer and project-outcome renders cover both themes and sizes.

46 original licensed SVGs have source hashes and full Lucide/Feather/Tabler notices. All 230 generated size masks pass opacity, nonempty content and border-padding checks. A one-pixel margin fixes faint border pixels at the new 24px status size; the existing larger symbol geometry is retained. Layer card/timeline spacing was corrected after inspecting the first screenshot set.

Ten thousand LED context transitions pass complete scene replacement, stale-lamp cleanup and note release. Main-loop assertions cover Home, settings, dialogs and persistent results/errors. Dedicated status-scene assertions retain real PLAY/REC while clearing Anchor, Layer and octave lamps from hidden Home. Acknowledgment restores Home feedback. Sounding-note and PLAY lamps remain meaningful when audio continues. Save-hold hints and recording indicators follow their current context. Physical brightness/readability and red/green accessibility remain user acceptance items.

Scope off/on musical PCM is byte-identical: 387072 bytes, SHA-256 a0228cf4ba582b5b45cf1bc020d876e9d6d479af890e9cae1ff770f321713d0c. Both shortcut orders, freeze, return to parent context, sample ownership, silence, earcon exclusion, preference persistence and 10000 scope cycles pass. The approved splash stays pixel-exact. Twenty-four first-input cases and no-input timeout pass without startup writes.

## Audio and unchanged synth

The production effect engine, preset coefficients, oscillator tables, filter implementation, scope and splash source are byte-identical to 1.1 RC2. 12800 preset/note combinations retain output bounds, master mute and panic silence. Four held voices on each of the 100 patches for 2.5 seconds at maximum effects, Melt, feedback/size and output pass without signed overflow, clipping or unstable filters: peak 30145/32767. The dry 100-patch audition remains audible, peak 4625, RMS 311–2768, with sustained pads and intentionally decaying keys differentiated.

The 22.14-second chord demo, rerendered and byte-identical to RC1, records through production MIDI capture, commits/reloads v5, then plays two passes of a two-bar three-note phrase using Slow Aurora and Electric Mist. Its peak is 9081/32767 and RMS 1222.29. Independent captured gates are 2560, 3072 and 3584 ticks. It demonstrates actual sequencer output. The inherited 48-second preset showcase and 300-second dry tour are also included; they are software renders, not recordings of the physical DAC or speakers. Listening preference and real-instrument balance still require feedback.

## Compiled-firmware execution

The final package, SHA-256 e1a18e8f2d700399f6cac736e9587031d4a91cc7f286cc32172b45d0489ef378, runs for 7000 emulated milliseconds in fm1-emu 0.3.0. Actual LCD captures show held chord recording, Save destination, verified Save success, verified Load success, Anchor playback and HOME return with a live key. The new Home status strip is visible, and Save/Load use the expected persistent results. The run produces 301312 stereo frames at 44100 Hz, peak 2504 and RMS 304.96.

The emulator reports 69182 interrupts and 1177 audio halves. Its ISR instruction percentage and nested fraction are recorded in compiled.log; those are observations of this scenario, not a physical worst-case CPU or latency measurement. There are 392 unknown MMIO accesses at 17 addresses, including flash-controller accesses in this save/load scenario. Emulation cannot qualify physical flash reliability, DAC output, LED brightness or USB recovery.

## Update/recovery evidence

All 33 protected platform/HAL/loader/recovery sources are byte-identical to 0.9. The compressed loader SHA-256 remains dac30791235ae6339f7b4b7179fbb59fb59ee6a05aa8442697b5f6407e4ef8dc; package head is identical. Fresh actual-package loader tests cover 0.9, 1.1 RC2 and 1.2 RC1 to/from this RC2, corruption retry, missing completion, next-session resume and protected-range enforcement. The unchanged platform has a passing inherited RC1 platform-suite result, including FM-1/SMK host input, MIDI, storage maps, update layout, loader screen and installer checks. No SMK firmware qualification is claimed.

The authentic stock-v15 package is verified by exact size, identity and SHA-256 and tested separately without redistribution. Staging checks the retained head, validates its official loader, and rejects a mismatch before erase. The host installer simulation covers running 1.2 to v15 and official-loader resume. It refuses stock during an unfinished custom update and rejects a damaged v15 before probing/writing. Exact 0.9, 1.0 RC2 and 1.1 RC2 host installs are exercised from current and earlier custom identities. Prior SLOOP tests used the same unchanged loader; fresh 1.2 cross-package simulations here use 0.9 and 1.1 RC2.

Rollback is preserved as a supported path, not an unconditional guarantee against power loss or unsupported firmware. This instrument does not have two independently bootable firmware banks. Firmware rollback does not make v5 music readable in older versions or preserve data another firmware overwrites. Keep exact old packages, the authentic v15 file and older project slots.

## Reproduce and complete physical acceptance

Set JIELI_TOOLCHAIN and AC79_SDK as described in README-DINGHY.md. Run `STOCK_V15=/path/to/verified-v15.fwsc BASELINE_09=/path/to/original-0.9-release.zip sh tests/run_release_tests.sh`. Additional platform checks use `sh tests/run_tests.sh`; compiled execution uses `FM1_EMU=/path/to/fm1-emu-0.3.0 sh tests/run_compiled_1_2.sh`. Render the chord reference with tests/render_chords_1_2.c. Raw final evidence accompanies the release. LeakSanitizer is disabled for this environment; AddressSanitizer and UndefinedBehaviorSanitizer remain active.

Before a stable release, exercise actual FM-1 cold boot, headphones/speakers, all controls and lights, sustained chords and MIDI, maximum practical polyphony/effects, audible clicks/timing, scope, storage across power cycles and the documented 0.9/1.1/v15 transitions. Evaluate low-vision readability and musical usefulness on the user's instrument. Host tests and emulation cannot establish physical USB reliability, flash endurance, analog output quality, battery behavior or worst-case CPU timing.
