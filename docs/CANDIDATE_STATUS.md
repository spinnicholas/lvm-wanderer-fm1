# 1.2 RC2 hardware and release status

Exact firmware: dist/LVM-Wanderer-1.2-RC2-FM1.fwsc, FM-1_932, 612196 bytes, SHA-256 e1a18e8f2d700399f6cac736e9587031d4a91cc7f286cc32172b45d0489ef378.

On November 1, 2026, Nicholas confirmed that the latest 1.2 RC2 has been tested on his own FM-1 and passed the same installation and rollback tests as the earlier release. The confirmed routes are V15 to Wanderer, SLOOP to Wanderer, Wanderer to V15, and Wanderer to SLOOP. These are author-reported physical-device results, in addition to the separately documented software and emulator checks.

The release remains an experimental prerelease, with regression/stress checks, compiled emulation and update/recovery simulations passing. The 33 protected platform/recovery sources, compressed loader and retained package head remain byte-identical to 0.9. No physical worst-case CPU or audio-latency measurements are claimed.

The candidate shares v5 project storage with 1.2 RC1. Earlier Wanderer releases cannot read newly saved v5 projects; preserve old-format slots for rollback and use spare slots for conversion. Other firmware can overwrite shared project storage. The recovery folder contains exact prior Wanderer binaries. Original stock v15 is not redistributed; INSTALL_AND_RECOVERY.md describes its verified-file restore path. This is not a dual-bank firmware or an unconditional power-loss recovery guarantee.

The development-session verification report retains the original software-test evidence. This confirmed hardware follow-up supersedes its earlier pending-acceptance label.

## Firmware Hub identity review

The Hub's live identity checker reports FM-1_932 within sloopDX's reserved range. The exact tested package is being submitted unchanged, with this conflict explicitly raised to the maintainer for resolution. Hub acceptance or activation is not claimed until the maintainer resolves it.
