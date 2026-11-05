# Wanderer 1.1 RC2 verification

RC2 refines the display and icon assets. This is a compiled, software-tested candidate. No connected FM-1 was flashed. The earlier RC1 report is preserved below as inherited engine/recovery evidence, clearly separate from the fresh RC2 checks.

|Item|RC2 result|
|---|---|
|File|LVM-Wanderer-1.1-RC2-FM1.fwsc|
|Identity|FM-1_931, the same family as RC1; distinguish exact files by SHA-256|
|Package|612196 bytes|
|SHA-256|532fc5731000ef5a36025f690028133d1e1ddbfffde2682fe73403d4c656fc11|
|Application|463692 / 581564 bytes|
|RAM data + BSS|21280 / 98304 bytes|
|Pool|232928 / 344064 bytes|
|Loader|9040 bytes, identical to 0.9|

## Fresh RC2 verification

All 55 RC1 screenshots were reviewed, including Home, both themes, Zoom, recording, settings, details, confirmations, errors, scope, splash and Music. The changed UI was recaptured from the production renderer and visually inspected. A complete before/after review records each page and a linked icon key. The approved splash is unchanged.

Forty-two distinct licensed SVG icons are used, thirty-eight from Lucide and four from Tabler. All 168 display-size masks pass nonempty/full-opacity/padding checks. Per-source hashes and complete ISC/Feather MIT/Tabler MIT notices are retained. Geometry is original source geometry, with a consistent adapted 2.25-unit stroke. The asset builder uses local files and Inkscape/Pillow; it does not fetch assets during compilation.

AddressSanitizer and UndefinedBehaviorSanitizer pass the inherited 300-state UI test, 1152 Music states and 2048 full-range normal/Zoom value-row renders, including zero and 100 percent. The real application capture produces 55 major views. Timed LED, short-tap, RC2 scope/cue and 1.1 musical navigation scenarios pass. The real musical recording, project migration, Anchor/Undo and core MIDI/DSP tests also pass. Twenty-four first-input cases and the splash timeout pass.

Eleven production files for synthesis, memory, MIDI, effects, presets, oscillators/filter tables, scope, LEDs and splash are byte-identical to 1.1 RC1. The timed production-audio scenario is byte-identical between RC1 and RC2: 387072 PCM bytes, SHA-256 a0228cf4ba582b5b45cf1bc020d876e9d6d479af890e9cae1ff770f321713d0c. Scope off/on PCM also remains identical. The UI revision does not alter the sound bank, captured durations, 16/32-step behavior or v4 project format.

The actual compiled package runs in fm1-emu 0.3.0 for 4000 emulated milliseconds. Panel actions select Music, switch to 32 steps, return Home and play a note. Actual LCD snapshots show the new keyboard tab, 32-step view, real anchor, repeat Memory, headphones badge and stacked layers. Audio contains 168960 stereo frames at 44100 Hz, peak 1170 and RMS 246.6. The emulator reports 38665 interrupts, 660 audio halves and 80 LCD frames, with 12.3 percent of CPU0 instructions in an ISR and 0.2 percent nested for this light scenario. This is not a maximum-load or physical CPU measurement. Thirty-seven unknown MMIO accesses remain at seventeen addresses.

The safety audit verifies 33 protected files, compressed loader bytes and package head against the hardware-tested 0.9 baseline. Real-package loader simulations pass RC1→RC2 and RC2→RC1. The prior 1.1 v15/0.9/RC2 recovery results below concern unchanged protected code; they are inherited evidence rather than newly executed physical transitions. Hardware install, actual LCD/LED readability, timing and rollback acceptance remain pending.

Run tests/ui_assets_audit.py /path/to/RC1/source for source/mask comparisons; ui_rows_test.c covers the new settings ranges. The existing release gate remains available, plus raw fresh results in test-evidence/. The UI review and symbol key are in UI_REVIEW_RC2.md and the self-contained review HTML.

## Inherited 1.1 RC1 engine and recovery report

# Wanderer 1.1 RC1 verification

This report distinguishes software evidence from physical acceptance and musical taste. No connected FM-1 was flashed. Prior physical rollback reports concern 0.9; these new 1.1 bytes remain a candidate.

## Exact firmware

|Item|Result|
|---|---|
|File|LVM-Wanderer-1.1-RC1-FM1.fwsc|
|Identity|FM-1_931|
|Package size|612196 bytes|
|SHA-256|9178b60d135459d0f6bd2a0482a882fccf2bdd76c4dadca9f71eb97a4ef6c65f|
|Application|455260 / 581564 bytes|
|RAM data + BSS|21280 / 98304 bytes|
|Static pool|232928 / 344064 bytes|
|Custom loader|9040 bytes, identical to hardware-tested 0.9|
|Project format|Version 4; 548-byte record, including 540-byte state|
|Older imports|v1/v2 legacy layouts and exact 160-byte v3 record|

The pinned JieLi toolchain and AC79 SDK used for RC2 were reused. The build validates SDK digests, HAL-only register access and RAM-only flash code; 916 RAM-text instructions contain no calls. Rebuilding the final firmware produced byte-identical packages.

## Musical recording and evolution

AddressSanitizer and UndefinedBehaviorSanitizer pass the new recording/evolution tests and existing app/DSP/MIDI/LED tests. A held local note records about 700 milliseconds and plays for more than four steps before releasing. Tests cover a held gate crossing the two-bar boundary, second-bar onsets, shortening without deletion, Undo, the one-loop duration cap, different MIDI sources, sustain pedal release, a replacement recorded note and transport finalization. A real main-loop scenario toggles recording off before key release, checks the stored long gate, saves/loads it, changes scale/root/length, refuses a running length change and undoes a stopped length change.

98,304 combinations of eight scales, twelve roots, 128 MIDI pitches and eight evolution phases verify generated-note membership, bounded moves, periodic return to the performed pitch and gate preservation. The allowed scales are Free, major, natural minor, Dorian, minor pentatonic, major pentatonic, Mixolydian and harmonic minor. Anchor data stays immutable through evolution and branching. There are still two monophonic event lanes, with four shared synth voices; chords are playable live but not stored as simultaneous lane events.

All eight current-format slots save and load durations, key, scale and two-bar data. Explicit old-format imports preserve their notes, patch numbers, old 192/256-step gates and sixteen-step loops. Invalid roots, scales, lengths, event bounds, gates and headers are rejected before replacing the current session. The old formats are not rewritten by Load. New v4 projects are deliberately not claimed to be readable in old firmware.

The earlier Wanderer regression suite passes, including Anchor/branch/Undo, settings, boundaries, new session, tempo restore, adversarial controls, effect tails/headroom, preset audition, updater staging, loader and host installer. The inherited platform suite passes its FM-1/SMK host and flash-layout tests. No SMK firmware or physical SMK qualification is delivered.

## Sound-engine evidence

The bank is rebuilt around millisecond envelope specifications, exponential decay/release, a resonant TPT state-variable lowpass, bounded input saturation, optional sub oscillator, band-limited pulse width modulation, velocity-sensitive brightness and patch-specific movement. Coefficients and filter tables are generated offline. The audio callback uses fixed storage and no allocation or transcendental calculations.

A dry three-second rendering of each of 100 patches yields RMS 311–2768 and maximum peak 4625/32767 for the specified audition. Tests distinguish sustained pads from intentionally decaying keys and bells. A slow pad remains sustained after 1.6 seconds, fades gradually after note-off and reaches silence. Mean DC stays below the audition bound. Another short-input preset tour is also audible and unclipped. These checks establish useful behavior, not a subjective claim that every sound is professionally finished.

12,800 preset/MIDI-note pairs pass full-output bounds. A stronger sanitizer stress test holds four voices on each of the 100 patches for 2.5 seconds with maximum effects, Melt, feedback/room size and digital level. Filter states and envelope levels remain bounded, no signed overflow is reported, and the largest output is 30145/32767 without clipping. Master mute and panic silence pass. Note range, fixed voice budget and high-register modulation safeguards remain.

The 48-second showcase renders twelve musical excerpts using the actual production DSP with modest chorus and room. The 300-second tour renders all 100 sounds dry, two seconds held plus one second released. Neither is a recording of the physical DAC, speakers or headphones. Subjective listening on the user's instrument is still needed, especially for balance, bass response, brightness, clicks and patch character.

## UI, LEDs, scope and startup

The inherited 300-state renderer test passes both themes, Zoom, numeric/detail limits, errors and scope extremes. An additional 1,152 Music settings renders cover roots, scales and selected rows in both themes/Zoom states. Two-bar Home views also fit 240×240. Fifty-five screenshots show every major page, including Music, root notation, scale intervals, loop length and duration timelines. The new settings and timeline captures were visually inspected. Their numeric scale codes require the documented scale mapping; they do not add normal words to the device UI.

Ten thousand LED context changes pass stale-mask cleanup and release handling. Timed navigation confirms the original light/transport/dialog behavior plus new musical controls. A disabled Layer now releases its sequenced held gates while allowing tails. Settings still clears unrelated Home lamps; sounding-note and PLAY indications can remain active.

Scope/cue tests pass both chord orders, release isolation, parent-context restoration, freeze, sample-buffer ownership, silent flatline, earcon exclusion, saved cue on/off and 10,000 open/close cycles. Scope off/on musical PCM is byte-identical: 387072 bytes, SHA-256 a0228cf4ba582b5b45cf1bc020d876e9d6d479af890e9cae1ff770f321713d0c. Existing startup art remains pixel-exact in both themes. Twenty-four first-input cases and timeout pass without startup saving. LeakSanitizer is disabled in this execution environment; AddressSanitizer and UndefinedBehaviorSanitizer remain enabled.

## Compiled-firmware execution

fm1-emu 0.3.0 runs the final package for 8000 emulated milliseconds, selecting a new pad and holding four notes with the oscilloscope open. The actual LCD capture shows a live waveform. The run produces 345344 stereo frames at 44100 Hz, peak 3952 and RMS 754.0. It reports 79354 interrupts, 1349 audio halves, 500 LCD frames, 22.4 percent of CPU0 instructions in an ISR and 0.5 percent nested, or 55.1 million ISR instructions per emulated second for this scenario.

Those instruction proportions are emulator observations, not a worst-case physical CPU utilization or latency measurement. The emulator reports 37 unknown MMIO accesses at seventeen addresses. Four-voice maximum-effect arithmetic is established by the separate sanitizer stress test, not inferred from the emulator's pad scenario. The run does not establish physical display visibility, LED brightness, DAC behavior, battery operation or USB reliability.

## Reversibility

The safety audit verifies 33 protected platform/HAL/USB/flash/recovery files byte-for-byte against the original 0.9 release. The compressed loader and generated flash head remain identical. Loader SHA-256 is dac30791235ae6339f7b4b7179fbb59fb59ee6a05aa8442697b5f6407e4ef8dc; unchanged 0.9 firmware SHA-256 is c6c0a2829a5c1fe5959cb0d302c4de152b97fe3f29e469c400d1fbaefa236dc9.

Real-package loader simulations pass both directions between 1.1 and 0.9, and between 1.1 and RC2. They cover protected-head preservation, damaged/foreign package refusal, corruption/retry, missing completion and resumed sessions. Earlier RC2/0.9 tests with SLOOP 2.8.4 provide inherited evidence for the unchanged loader; that package was not available for a fresh 1.1-to-SLOOP simulation in this turn. This does not assert universal compatibility with every custom firmware.

Authentic stock v15 is checked as FM-1_015, 699956 bytes, SHA-256 db1642b2b6fa5c2cccb11ffd13878068bb28601678d3644049f99dc40e7edb8a. Official-loader staging passes with the retained stock head, while a mismatched head is rejected before erase. Full simulated host transactions pass 1.1→v15, exact official-loader resume, unfinished custom-loader refusal and corrupt-v15 rejection. Host installation/identity verification of 1.1 passes from 0.9, RC2, 1.1 and another custom identity. Vendor firmware is a private test input and is not redistributed.

Firmware rollback does not preserve shared projects by itself. New v4 records cannot be opened by older Wanderer; preserve older slots or another supported backup before conversion. The original head/rescue foundation is retained, but the FM-1 has no independent second application bank and no guarantee against every interrupted write.

## Reproduce and accept

Set JIELI_TOOLCHAIN and AC79_SDK to the pinned dependencies. Run STOCK_V15=/path/to/genuine/FM-1.fwsc BASELINE_09=/path/to/original-0.9-release.zip sh tests/run_release_tests.sh. This rebuilds and exercises the application, renderer, new musical behavior, host navigation, scope/startup and sound bank. Raw evidence is included. tests/safety_audit.py compares the baseline; tests/ldr_test.c exercises actual packages. The compiled emulator was additionally run with fm1-emu 0.3.0. No script automatically flashes hardware.

Before promoting this candidate, test physical boot/installation, held-note record/replay, both lengths, root/scale changes, all controls, speaker/headphone/MIDI audio, display/LED clarity, high-load timing, Save/power-cycle/Load and real v15/custom rollback. Aesthetic sound acceptance remains part of that musician-led test. External audio recording/resampling, polyphonic phrase storage, external MIDI clock/transport, pitch bend, MPE and speech are not implemented.

## DSP design references

The filter equations were checked against Andrew Simper's primary derivation, Solving the continuous SVF equations using trapezoidal integration, https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf . The fixed-point tables and bounded implementation here are original project code, not a claim to reproduce Cytomic products. Envelope gate/state and exponential-contour behavior were checked against Nigel Redmon's EarLevel Engineering article, https://www.earlevel.com/main/2013/06/01/envelope-generators/ . The implementation retains a linear attack with exponential decay and release.
