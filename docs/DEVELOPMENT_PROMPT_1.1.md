# Wanderer 1.1 development brief and musical brainstorm

## Problem to solve

The 1.0 instrument stores each recorded onset without its release, forcing sequenced gates to three quarters of a sixteenth note. Pads and leads therefore turn into brief bleeps on replay. Drift makes chromatic semitone changes without harmonic context. The sound bank has limited envelope ranges and a simple nonresonant filter. Improve the instrument as something people can play musically, preserving the accessible visual language and tested recovery foundation.

## Musical directions considered

Scale-bound evolution is the first useful improvement. Let the player choose a root and scale. Move generated notes by a small number of scale degrees; leave the performed source exact. Avoid an accumulating random walk. Give an evolving phrase a periodic return to its starting motif so change has a recognizable shape. Retain Free for intentional chromatic work. Longer gates must remain attached to their events through evolution, Anchor catch, branching, Undo, save and reload.

Melt is a timbral control, so pitch quantization is not appropriate for it. Make its filter movement smooth and musically useful, with resonant tone shaping and carefully bounded saturation in the engine. Drift controls generated pitch; Decay controls repeated-cycle velocity aging. Keep each role predictable rather than letting every knob alter several unrelated musical dimensions.

Recording should follow actual key/MIDI gates, including sustain pedal and source/channel ownership. The existing lanes are monophonic event lanes; this release should not pretend to offer polyphonic phrase recording. Choose 16 or 32 sixteenth-note steps, corresponding to one or two bars of 4/4. Preserve hidden second-bar events when shortening and make length changes undoable. Require stopped transport for a length change.

Improve synthesis before redesigning the bank. Add wide-range envelopes with exponential decays/releases, a stable resonant lowpass, a sub oscillator, band-limited pulse width modulation, subtle patch-specific movement, velocity-sensitive brightness and bounded drive. Preserve four shared voices, oscillator anti-alias safeguards, click-reducing voice splices and fixed memory usage. Generate coefficients offline rather than performing transcendental calculations in the audio callback.

Design 100 original patches in ten useful banks: foundations, vintage poly, leads, basses, plucks/keys, FM bells, digital motion, pads, brass/organic and experimental voices. Include genuine held pads and leads, plus intentionally decaying keys and bells. Do not claim exact manufacturer emulation or treat unique parameter hashes as evidence of professional sound quality. Supply listening examples and leave final aesthetic judgment to the musician.

Additional promising ideas include optional live-key scale assistance, swing, controllable mutation density, harmonic changes per bar and polyphonic event memory. They require their own control, data and CPU designs and remain future work rather than untested additions to this release.

## Implementation contract

Work on a separate 1.1 source copy. Keep all platform, HAL, USB, flash driver, bootguard, rescue and custom-loader files identical to the hardware-tested 0.9 baseline. Build a real FM-1 package with identity FM-1_931. Preserve the RC2 splash, oscilloscope, cue toggle, themes, Zoom, error handling and state-derived LED behavior.

Add Music as a fourth settings page with root, scale and loop length. Keep large root notation, numeric scale selection accompanied by its interval pattern, and a large 16/32 indication with one/two-bar geometry. Use no extra prose on the device. Document the scale choices and control mapping clearly.

Use a versioned project format with 32 events per lane and explicit gates. Validate malformed records before replacing the current session. Import the exact historical v1/v2/v3 layouts, retaining their sixteen steps and original three-quarter-step gates. Older firmware cannot read new v4 records; document that distinction separately from safe firmware rollback. Do not overwrite projects on boot or auto-load one.

Test real duration capture and playback, recording interruption, long holds, loop wrap, separate MIDI owners, sustain, alternate loop lengths, undo, Anchor protection, branch preservation, all eight slots, old-record imports, invalid-record refusal, all roots/scales/MIDI pitches, sustained pads, full-output headroom, maximum effects, UI bounds, LEDs, scope isolation and splash. Exercise the real main-loop controls as well as direct core functions. Run the compiled package in the emulator. Compare protected source/loader bytes and simulate transitions to known-good 0.9, RC2, SLOOP and authentic v15. Do not flash connected hardware automatically.

Deliver a compiled candidate, complete source/recovery bundle, release guide, preset catalog, screenshots of every major view, verification report, checksums and a short audio showcase. Label physical installation, timing, readability and subjective listening acceptance as pending until tested on the FM-1.
