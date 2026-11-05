# Wanderer 0.8 FM-1 package

This is a flashable FM-1 `.fwsc` package, built from Wanderer's 0.8 source. It is an experimental hardware candidate, not a physically qualified release.

## Exact artifact

| Property | Value |
|---|---|
| File | `dinghy-0.8.fwsc` |
| Package size | 612,196 bytes |
| Logical image size | 612,176 bytes |
| SHA-256 | `59a9fe73ce21e166fbba8c0f22fed609f91e81a6d97f2a1f371a4a5f8debe5e8` |
| Package identity | `FM-1_928` |
| Target | M-VAVE FM-1 |

## Build and checks, 2026-10-31

Built on Linux x86_64 with JieLi `jieli-linux-toolchains-20260730.1` and the AC79 AIoT SDK tag `AC79NN_SDK_V1.2.1_2023-12-13` at commit `d179b4484759423312073f5fbb232501aa491047`:

```sh
JIELI_TOOLCHAIN=/path/to/jieli/toolchain AC79_SDK=/path/to/fw-AC79_AIoT_SDK sh build.sh --release 0.8
JIELI_TOOLCHAIN=/path/to/jieli/toolchain AC79_SDK=/path/to/fw-AC79_AIoT_SDK sh tests/run_wanderer_tests.sh
```

The app binary is 121,796 / 581,564 bytes, `.data + .bss` 12,752 / 98,304 bytes, and pool 232,928 / 344,064 bytes. The 9,040-byte update loader was included. Build checks for RAM-resident code and HAL register access passed. The complete Wanderer core, layout, OTA, loader, installer and package test script passed. The test script builds a separate development package (`FM-1_920`); the release package above was independently parsed and its identity and hash checked.

The **actual release file** executed for 1,000 ms in fm1-emu 0.3.0: 143 audio half-buffer interrupts and 20 LCD frames. A second 750-ms run injected key `n7` from 300–500 ms and generated 25,600 stereo PCM frames with peak 2,098 and RMS 729. The emulator reported 32 unknown MMIO accesses across 17 addresses. These are emulation observations, not physical measurements.

## Install on an FM-1

Back up material you need from the device and retain a known working recovery firmware. Use the included installer with a USB data cable and the FM-1 connected:

```sh
python3 -m pip install mido python-rtmidi
python3 tools/fm1_install.py --info
python3 tools/fm1_install.py /path/to/dinghy-0.8.fwsc
```

The installer checks the package and connected model, asks for confirmation, transfers the image in two stages, and checks the reported identity after reboot. Keep the cable connected through writing. This local package is not listed on the FM-1 Firmware Hub; use the included Python installer for this build. Do not use the SMK-37 path for this FM-1 package.

No physical FM-1 was connected in this build. Display appearance, input mapping, worst-case audio deadline, save/load writes, installation and rollback must be tested on real hardware before wider distribution. The FM-1 Firmware Hub identity table did not show `FM-1_928` on 2026-10-31; recheck before publishing.
