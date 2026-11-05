# Wanderer 1.7 RC2: responsiveness and CPU fixes

This is a hardware-test candidate responding to reported late notes and distorted multi-note playback in 1.7 RC1. It retains four voices, all six engines, all 150 presets, and all seven effects. It does not incorporate the separate Baud Girl research.

## Changes

Local notes and MIDI are serviced while LCD DMA waits and between screen bands. Previously the LCD idle callback was empty and input handling waited for a complete redraw. Notes arriving during drawing also preserve the pending redraw request.

The synth avoids initializing unused string/modal state on other engines, avoids advancing unused operator envelopes, and avoids calculating analog oscillators whose output was immediately replaced by a specialist waveform. Filter coefficients are reused when cutoff, resonance and filter mode are unchanged; envelope and modulation updates retain their rate.

Disabled wavefold, bitcrush and phaser processing is skipped after the bypass fade. Disabled chorus skips unused tap reads while keeping its input history and phase. All active effects retain their processing rates, and delay/tape/reverb histories continue to advance. Stable effect mix values avoid seven redundant per-sample smoothing updates. There is no automatic reduction of polyphony, effect availability or sample rate.

## Optional recording metronome

Hold SELECT to open settings. Turn PRESET to OUTPUT and use knob 4, REC CLICK, to switch it on. It defaults off, sounds only during event recording, ticks on quarter notes and accents the loop start. It is independent of accessibility cues and goes after Print capture and effects, so it is not baked into recorded Print audio or effects tails. It follows output level. Save normally to retain the global preference.

## Settings order

Accessibility → Access Feel → Music → Arpeggiator → Arp Feel → Output → Wavefold → Bitcrush → Phaser → Chorus → Delay → Tape Echo → Tape Color → Reverb.

All effect pages are contiguous; arpeggiator controls remain beside musical controls. Internal IDs and project effect fields remain stable. This changes page browsing order, not the audio routing.

## Verification

The 150-preset, four-voice comparison produces matching per-preset audio hashes against RC1 with effects bypassed and with all seven enabled. Each case includes a held chord and releases. The optimized control/oscillator paths therefore preserve the tested output. Host CPU benchmarks over three runs showed median render CPU time approximately 30% lower dry and 18% lower with all seven effects; these figures describe the host, not the FM-1 processor.

The dedicated sanitizer test covers four-note onset/release and latched taps through the LCD idle handler, click default/setting/persistence, record-only click timing, empty recording events, page grouping and both UI sizes. The broader release suite also covers project migration, Anchor/undo, keyboard/MIDI routing, effect boundaries, callback segmentation, OTA and loader upgrade/downgrade. Full factory stress covers 450 four-voice/max-effect/Melt cases; peak 28,133. A separate ten-minute simulated performance covers layers, Scatter and arp.

The final target image is 537,556 bytes of 581,564. Ordinary RAM is 72,864 of 98,304; the pool remains 343,040 of 344,064. All 33 protected platform/recovery files match the baseline; package head and recovery loader match the accepted 1.4 RC6 package. The boot stack audit passes. Exact package identity is FM-1_929, size 612,196 bytes, SHA-256 d1fdf8221af17856d68790b1c1b62549179629065b935e743c77e6df58bb4857.

The exact candidate completed an initial 2,800 ms compiled-device emulation with a four-note chord: 115,968 stereo frames at 44.1 kHz, bounded output and responsive note onset/release. A matched 4,200 ms run of the exact final package reduced average emulated interrupt instruction load from RC1’s 38.0% to 29.2%, with the same 177,920 stereo frames and 695 audio halves. This is an average instruction-load comparison, not a measured worst-case hardware deadline. The emulator does not reproduce hardware cache/bus timing faithfully; no physical worst-case deadline claim is made. The reported one-to-two-second delay and occasional distortion still require confirmation on the user's FM-1. No claim of repairing persistent project corruption is made from this audio report.

## Install and check

Install LVM-Wanderer-1.7-RC2-FM1.fwsc with the usual FM-1 updater. Keep the last known-good package available. Use a spare project slot: new 1.7 project saves remain version 11, which older firmware cannot read.

First compare the same problem preset with single notes and four-note chords, then add effects and operate the controls while holding notes. Try quick taps during screen changes and Zoom, releases, rapid retriggers, playback plus live notes, and the recording click on/off. These are hardware acceptance checks, not prerequisites claimed to have passed here. This candidate has not been published as the stable firmware.
