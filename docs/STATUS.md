# Wanderer 0.9 — work in progress

Wanderer 0.9 is available for source review, host audition and volunteer testing. I have physically tested it on my own M-VAVE FM-1 and successfully rolled back to another firmware. This establishes an initial device test and rollback, not full coverage of every function or device revision. Detailed starting/destination firmware versions, real-time audio measurements and persistence results have not yet been recorded. Back up your work and retain known-working recovery firmware before a trial. Install at your own risk.

The unchanged release file reports `FM-1_929`. The Firmware Hub’s live checker showed no listed firmware using that ID at publication preparation. This is not a reservation or Hub approval. The initial physical test and rollback have now been completed on my FM-1. The Hub also asks for installation from official V15 and from another custom firmware; those starting conditions have not yet been recorded, so a complete Hub qualification claim has not been submitted.

See [HARDWARE_TESTING.md](HARDWARE_TESTING.md) for the latest physical-test report. The earlier statements in the original source/download bundle reflect the state before this author report.

## What was verified

The original build/emulator observations are in RELEASE_0.9.md. Publication preparation reran ten host scenarios, an audit of all 100 patches and the event-memory test with AddressSanitizer and UndefinedBehaviorSanitizer. All passed. Leak detection was disabled because this container cannot inspect process threads. The generated font, icons, oscillator and patch headers remained byte-identical to the original 0.9 source archive. Package CRC, loader marker, size, identity and SHA-256 were verified. The firmware has not been rebuilt or modified for publication.

Screenshots are host framebuffer captures; the sound tour is a host audio render. They are not photographs or recordings of physical FM-1 hardware. The original waveform tables and patches are synthesis designs, not manufacturer preset data or faithful hardware emulations.

## Scope and known limitations

This is a four-voice synth that captures note and velocity events in a 16-step phrase. It does not record audio or resample microphone/input sound. There is one explicitly saved slot. Old SLOOP projects cannot be imported. Audible cues are tones, not speech, and the screen is still required for complete information. The package is for the FM-1 only; no SMK-37 firmware is released here.

Source code and third-party notices are retained. Public-facing documentation was refreshed and a new Wanderer SVG replaces the inherited restricted Dinghy logo. The inherited release workflow is saved as docs/UPSTREAM_RELEASE_WORKFLOW.yml for reference, not enabled automatically. The source tree at the release tag is the corresponding code for the uploaded firmware.
