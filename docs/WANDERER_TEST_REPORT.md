# LVM Wanderer 0.1 test and resource report

Compiled 2026-10-25 from the GPL source in this bundle. This is a host and emulator experiment, not a physical instrument validation.

| Build | App bytes / 581,564-byte limit | App headroom | RAM data + bss / 98,304 | Pool / 344,064 |
|---|---:|---:|---:|---:|
| Recovered LVM Access 0.6.1 SLOOP application | 580,452 | 1,112 | report in earlier release | report in earlier release |
| Unmodified Dinghy 0.4 local baseline | 56,320 | 525,244 | 16,480 | 30,720 |
| LVM Wanderer 0.1 event-memory application | 51,204 | 530,360 | 12,384 | 30,720 |

The new app is smaller than the Dinghy example because it replaces rather than adds to its original app. Its glyph data are 9,208 and 6,078 encoded bytes before linker layout. A music slot is 152 bytes. The build retains the 581,564-byte app limit and the inherited RAM, protected-flash, loader and register-access checks.

The single-command `tests/run_wanderer_tests.sh` passed: target build, ASan/UBSan event-memory tests, scripted host audio and screens, boundary text assertions, new-session confirmation and cancellation, tempo restoration across Load, Undo and new session, a save-while-playing refusal, OTA simulation, loader update simulation, installer parser tests and package CRC/identity validation. The recorded tour produced audible samples after capture and after layering, with no saturated samples in the checked WAV. Actual screen frames from the host and native instruction emulator are included.

The exact compiled package identified itself as `FM-1_920` and reached Home in fm1-emu. The 1,000 ms emulation reported 143 audio half-buffer interrupts and 20 LCD transfers. It also reported 32 unknown MMIO accesses at 17 addresses. That run does **not** measure the worst-case audio callback, touch latency, flash-cache placement effects or physical LCD appearance. No FM-1 was connected or flashed.

Current limits: a monophonic event in each of 16 slots per phrase; no audio recording, resampling, microphone or SD-card import; one persistent slot; no incoming MIDI note handling in the new app; no onboard or connected speech integration; no proven worst-case CPU deadline under dense voices or flash operations. The FM-1 hardware screen, speaker, encoders, save and recovery path, as well as low-vision usability, still need a controlled trial. The inherited SLOOP groovebox project format is deliberately incompatible with this distinct application.

The code and visual system are a first playable vertical slice. It makes Anchor preservation, audible evolution, a layer, a branch and safe return concrete. More synthesis depth, tactile mapping refinement, independent speech and an audio-memory engine remain future work.
