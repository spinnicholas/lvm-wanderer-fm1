# Wanderer 1.4

Wanderer 1.4 is ready to install on M-VAVE FM-1. Nicholas confirmed that the exact RC6 build passes his standard physical-device tests on November 2, 2026. This release publishes those same bytes; the file and embedded label retain RC6 so the tested build remains identifiable.

Wanderer is designed with low-vision musicians in mind: large high-contrast symbols and text, adaptive and steady Zoom, consistent four-knob parameter controls, meaningful LED feedback and optional audio cues. HOME + EDIT toggles Zoom globally. Accessibility uses clear Theme, Zoom and Motion labels. Access Feel adds display hold, cue mode, cue level and steady numeric sizing.

The musical update brings a fuller 100-preset synth, eight Scatter modes with grain size/amount/smoothing before effects, and an arpeggiator with 13 patterns, 12 rates, gate, swing, latch, octave range and USB/TRS clock. Record generated notes into Memory or Layer, preserve an Anchor, and Print evolving phrases to RAM audio with Undo/Redo.

Download `LVM-Wanderer-1.4-RC6-FM1.fwsc` for installation. `Wanderer-1.4-Release.zip` includes matching source, the guide, screenshots, cue preview, test evidence and installer. [Quick start](https://github.com/spinnicholas/lvm-wanderer-fm1/blob/main/docs/QUICK_START_1.4.md) and [installation/recovery](https://github.com/spinnicholas/lvm-wanderer-fm1/blob/main/docs/INSTALL_AND_RECOVERY.md) are available without downloading the bundle.

The binary is 612,196 bytes, identity FM-1_929, SHA-256 `9135b62169409efb73a6492561673bb5465979563824605956ab3316c50d5b32`. Sixty build/regression checks, 35,840 steady-Zoom range renders, cue/Print isolation checks, target memory/stack audits and exact-package emulation passed before the author’s hardware acceptance.

Use spare slots for v8 saves if retaining rollback to older Wanderer project formats. Printed audio is RAM-only. Four synth voices are shared by loops, live playing and the arpeggiator. External clock follows the arp, while event loops and printed audio retain their internal clock. The modern engine is 6 dB louder than 1.3 RC4; start with a lower Master setting.

Nicholas James / [Low Vision Music](https://www.lowvisionmusic.info). GPL-3.0-only with included third-party notices.
