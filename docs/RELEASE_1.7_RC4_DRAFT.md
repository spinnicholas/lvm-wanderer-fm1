# Wanderer 1.7 RC4 — release preparation (NOT YET PUBLISHED)

**Status:** Candidate release information staged only. No 1.7 RC4 download is available from this branch. The [public 1.4 release](https://github.com/spinnicholas/lvm-wanderer-fm1/releases/tag/v1.4.0) remains the latest independently confirmed hardware-tested release. Do not replace its firmware or re-label 1.4 binaries as 1.7 RC4.

## Proposed release summary

Wanderer is experimental firmware for the M-VAVE FM-1, designed from a low-vision musician's perspective by Nicholas James / [Low Vision Music](https://www.lowvisionmusic.info). It offers protected Anchor memory, musical evolution, low-vision display settings, optional audible controls, a four-voice instrument and hands-on performance.

Compared with 1.4, 1.7 RC4 work includes:

- **Scatter size up to 512 ms.** Options at 256 ms and 512 ms extend the earlier 128 ms ceiling. The expanded audio buffer was checked for range, wraparound, save/load and normal/Zoom display behavior in the reported RC4 software tests.
- **High-contrast by default.** RC3 introduced six selectable appearance themes: Contrast (boot default), Dark, Light, Amber, Ocean and Violet, with stronger menu text/background contrast.
- **150 presets** and a revised sound/effects experience. Seven effects reported for the 1.7 line: chorus, delay, reverb, bitcrush, tape echo, stereo phaser and wavefolder.
- **Interface and performance improvements.** RC2 grouped effect/arp controls and added an optional recording click. Reported emulation showed a lower audio interrupt load than RC1.
- **Project-file compatibility note.** Existing projects can be imported; fresh 1.7 saves use format v11 and are not readable by earlier Wanderer firmware. Back up projects and save converted projects into spare slots where practical.

## Release gates / outstanding work

- [ ] Recover and verify the original compiled `LVM-Wanderer-1.7-RC4-FM1.fwsc` from the development session. Do **not** substitute the 1.4 binary or rebuild from the 1.4 codebase and call it RC4.
- [ ] Recover its matching `LVM-Wanderer-1.7-RC4-Source.zip` and test evidence.
- [ ] Validate identity `FM-1_929`, size, internal package checks and SHA-256; document exact bytes and build provenance.
- [ ] Ensure source, binary, guides, quick start, screenshots (including Anchor home screen and all six themes), recovery instructions and license notices match.
- [ ] Complete physical FM-1 testing of these exact RC4 bytes, including installer and rollback paths, or prominently label the GitHub release **untested release candidate** rather than stable.
- [ ] Publish a new GitHub release/tag with attached **actual matching assets**. Keep the historical 1.4 release intact.
- [ ] Only then update root `README.md`, `docs/index.html` (GitHub Pages), and `fmhub.json` (Firmware Hub listing) to point at the real new release. Make the Firmware Hub description explicitly say **designed with low vision musicians in mind**.
- [ ] Update `fmhub.json` preview image to a verified current Anchor/Home capture; retain other genuine UI screenshots and splash, never an unrelated mockup. Verify GitHub Pages and the Hub refresh.

## Important distinction

Software test reports for the 1.7 RC4 candidate are not equivalent to manual hardware acceptance. A physical acceptance record exists for **1.4 RC6**, not for these exact 1.7 RC4 bytes. This document is a working release checklist and **not** a firmware install instruction.
