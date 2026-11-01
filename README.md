# LVM Wanderer 1.2 RC2 for M-VAVE FM-1

**Author-tested on FM-1 · experimental 1.2 RC2 pre-release.**

I'm Nicholas James, a visually impaired musician and the person behind [Low Vision Music](https://www.lowvisionmusic.info). Wanderer is an event-memory synth built around a protected Anchor, an evolving working phrase and an optional Layer, with large symbols, Zoom, position-based feedback and optional earcons.

This repository provides the complete corresponding source and exact compiled candidate for Firmware Hub review. The previously hardware-tested [0.9 release](https://github.com/spinnicholas/lvm-wanderer-fm1/releases/tag/v0.9.0-wip). remains available for rollback. Version 0.9 was installed from V15 and SLOOP and returned successfully to each on my FM-1. Nicholas has now also confirmed that the latest 1.2 RC2 passed those same four installation and rollback tests on his FM-1: V15 to Wanderer, SLOOP to Wanderer, Wanderer to V15, and Wanderer to SLOOP.

[Download the compiled 1.2 RC2 firmware](https://github.com/spinnicholas/lvm-wanderer-fm1/releases/download/v1.2.0-rc2/dist/LVM-Wanderer-1.2-RC2-FM1.fwsc). Identity: FM-1_932. Size: 612196 bytes. SHA-256: e1a18e8f2d700399f6cac736e9587031d4a91cc7f286cc32172b45d0489ef378.

Read the [user guide](docs/USER_GUIDE_1.2.md), [installation and recovery instructions](docs/INSTALL_AND_RECOVERY.md), [exact verification report](docs/TEST_REPORT_1.2.md), [release status](docs/CANDIDATE_STATUS.md) and [changelog](docs/CHANGELOG_1.2.md). [All 66 major screen captures](docs/screenshots-1.2) and six actual compiled-firmware captures accompany the evidence. Source and full third-party notices are browsable in this branch.

![Wanderer 1.2 Home, production renderer](docs/card.png)

1.2 adds four-note-per-step chord capture with independent durations, MIDI sustain, 16/32-step loops, root/scale-aware Drift, a redesigned 100-preset engine, live oscilloscope, meaningful light scenes and persistent project status. RC2 refines chord replacement and quantization and makes HOME's Anchor return release the old sequence while preserving live keys. No audio resampling is provided. Four sound voices remain shared by keys, MIDI and both lanes.

The 33 protected platform/recovery source files, compressed loader and package head remain identical to the 0.9 foundation. The software suite, sanitizer stress cases, package checks, compiled emulation and rollback simulations pass. The author confirms this candidate passed real-device installation from V15 and SLOOP and return to each. Physical timing measurements are not claimed.

The new v5 project format is shared with 1.2 RC1. Older Wanderer versions cannot read v5; preserve older project slots and use spare slots for conversion. Importing an older project does not rewrite it. Other firmware can overwrite shared storage. Retain exact recovery packages and the verified original v15 file; there is no independent second firmware bank.

Build with the pinned dependencies in [README-DINGHY.md](README-DINGHY.md): `sh build.sh --release 1.2-rc2`. The [source README notes](docs/SOURCE_README_1.2.md) describe reproducible software gates. GPL-3.0-only, with applicable OFL, ISC and MIT assets; see [LICENSE](LICENSE) and [LICENSING.md](LICENSING.md). Not affiliated with M-VAVE or upstream authors.
