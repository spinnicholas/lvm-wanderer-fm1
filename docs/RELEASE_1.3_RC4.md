# Wanderer 1.3 RC4 — FM-1 hardware release notes

Wanderer is an experimental event-memory synthesizer for the M-VAVE FM-1, built by Nicholas James / Low Vision Music with **low-vision musicians in mind**. Its large symbols, position-based controls, accessible visual presentation and audible confirmations aim to make musical exploration easier without relying solely on small labels or color.

## This build

**Version:** 1.3 RC4 (Grains)  
**Firmware identity:** `FM-1_929` (permanent Wanderer identity)  
**Compiled file:** `LVM-Wanderer-1.3-RC4-Grains-FM1.fwsc`  
**Size:** 612,196 bytes  
**SHA-256:** `91633c4d0e3de94fd15722fce062504243c0d623437317be96434165c3b116ce`

Do not install a file with a different checksum while following these RC4 release notes.

## New in RC4

On the SCATTER editor page, the first three knobs set Style, Size and Amount. Scatter grain size offers Auto and 4, 8, 16, 32, 64 and 128 ms options. Manual grain lengths may vary per gesture for less predictable stutters, hops and dropouts. Scatter comes **before chorus, delay and reverb** in the signal path, allowing those effects to soften glitch edges. Project saves now use version 7; existing version-6 projects import with grain size set to Auto without rewriting the original slot.

The previous RC2 boot/scanner fix is retained. The firmware uses identity `FM-1_929` rather than IDs reserved by sloopDX. In the build report, 47 software regression checks, 2,520 UI collision checks and a compiled emulator scenario passed.

## Physical testing and firmware transitions

**Author report, November 1, 2026:** The developer physically tested the latest 1.3 RC4 firmware on an FM-1 and confirmed it was ready for publication. The reported installation and return paths include:

- Earlier Wanderer version ↔ latest Wanderer
- Official V15 ↔ latest Wanderer
- SLOOP ↔ latest Wanderer

Here, ↔ records the author's report that installation and return were tested in both directions. The report does not establish that every intermediate Wanderer release or every unrelated third-party firmware is compatible. Wider hardware testing by independent FM-1 owners is welcome.

## Installation and data safety

Use the FM-1 firmware installation workflow and **the compiled `.fwsc` file**, not the source ZIP. Back up any important projects first and retain the exact, working firmware image needed to return to your starting firmware. Do not interrupt power or USB during flashing. Flashing firmware from a different project does not automatically convert that project's saved music; restore its projects with the originating firmware.

Wanderer project-format migration is one-way for newly saved projects: 1.3 RC4 writes v7 records. Older releases will not necessarily open those records. To preserve rollback compatibility, import older slots but save migrated projects into spare slots rather than overwriting originals.

## About the project

This is a new synthesizer/sequencer application built on the FM-1 platform, not a SLOOP update. It explores Anchor memory, evolving phrases, original presets and controlled unpredictability. It is designed around the needs of musicians with low vision, but the screen is not yet entirely optional.

Project home: https://github.com/spinnicholas/lvm-wanderer-fm1  
Low Vision Music: https://www.lowvisionmusic.info

Firmware remains experimental. Installation and rollback success on one tested device do not guarantee compatibility with all FM-1 revisions.