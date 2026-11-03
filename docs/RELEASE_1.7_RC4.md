# Wanderer 1.7 RC4 — author-tested release record

**Firmware identity:** `FM-1_929`  
**Version:** `1.7-RC4`  
**Hardware testing:** Author-confirmed on November 2, 2026 (Kentucky time). Nicholas reports having thoroughly tested this version on his physical M-VAVE FM-1. This is the author's report, not independent hardware certification.  
**GitHub distribution:** The exact RC4 binary and matching source were supplied and locally validated, but the 1.7 RC4 assets have **not yet been attached to a GitHub release**. Until they are uploaded, the public 1.4 download remains the newest download hosted by this repository. Do not download 1.4 and treat it as 1.7.

## Files verified for this release

- Firmware: `LVM-Wanderer-1.7-RC4-FM1.fwsc`, 612,196 bytes
- Firmware SHA-256: `edb95653f2b1c122f0e22f6f2c66faf706d80c85ee3cb126f2f7ce80799da51f`
- Matching source archive: `LVM-Wanderer-1.7-RC4-Source.zip`, 15,022,272 bytes
- Source archive SHA-256: `69a9ffb5b77b844586e86d5e356dced733564527faa18c20f544349ee9b477bb`
- Integrity: the archive passes ZIP integrity testing; its embedded RC4 .fwsc has the identical SHA-256 to the separate firmware upload.
- Included original software verification: 72 checks passed, including 360 grain/Scatter cases, FM-1 platform identity and recovery-source invariants.

## What's new since 1.4

Wanderer is an experimental four-voice synth and musical-memory instrument, designed with low-vision musicians in mind by Nicholas James / [Low Vision Music](https://www.lowvisionmusic.info). Keep an idea in an Anchor, evolve a working layer with Drift and Melt, use Scatter before effects, then Print the result or save a project.

In the 1.7 development line, a revised 150-preset bank uses analog, FM, phase, string and modal sound design. The effects include chorus, delay, reverb, bitcrush, tape echo, phaser and wavefolder. RC2 improved note responsiveness during screen updates and reduced DSP work; RC3 introduced six themes and a high-contrast default. RC4 extends Scatter Size beyond 128 ms with **256 ms and 512 ms** selections, preserving the musical signal path before effects.

### Accessibility

- Boot into high-contrast black/white settings and controls, with Dark, Light, Amber, Ocean and Violet alternatives.
- Normal and enlarged Zoom interfaces, readable parameter values, optional audio cues, adjustable display hold and physical-control continuity.
- The 512 ms size control was checked in software in normal and Zoom and across all six themes.

### Installation and backward compatibility

Use the actual RC4 .fwsc only, with an established FM-1 updater. Keep a known-good older installer file and your verified original stock firmware for recovery. The FM-1 identity stays `FM-1_929` across Wanderer versions.

New 1.7 project saves use project format v11. Older projects can be imported, but older firmware cannot read those new v11 saves. Save conversion experiments into spare slots and keep independent project backups when rolling back. New Scatter size indices 256/512 ms are not understood by older firmware.

## Remaining distribution tasks

- Attach exact firmware and source archive to a new GitHub release/tag such as `v1.7.0-rc4`; include the SHA-256 values above.
- Publish a current guide and screenshots of the Anchor Home page, new contrast/theme menus, Scatter 512 ms control and splash, all based on genuine renderer captures.
- After the attachments and download links work, update main README, GitHub Pages and `fmhub.json` to identify 1.7 RC4 as current. Explicitly mention its low-vision design in the Hub listing.
- Confirm Firmware Hub picked up the release. Preserve older releases as rollback references.
