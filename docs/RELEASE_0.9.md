# Wanderer 0.9 for FM-1 — 100-sound build

This is an experimental, flashable FM-1 `.fwsc` candidate with a new 100-sound engine. No physical FM-1 was connected or flashed during development.

## Exact package

| Property | Value |
|---|---|
| File | `dinghy-0.9.fwsc` (distributed as `LVM-Wanderer-0.9-FM1.fwsc`) |
| Target and identity | FM-1 / `FM-1_929` |
| Size | 612,196 bytes |
| Logical image | 612,176 bytes |
| SHA-256 | `c6c0a2829a5c1fe5959cb0d302c4de152b97fe3f29e469c400d1fbaefa236dc9` |

## Build

Built on Linux x86_64 with JieLi `jieli-linux-toolchains-20260730.1` and AC79 SDK tag `AC79NN_SDK_V1.2.1_2023-12-13`, commit `d179b4484759423312073f5fbb232501aa491047`:

```sh
JIELI_TOOLCHAIN=/path/to/jieli/toolchain AC79_SDK=/path/to/fw-AC79_AIoT_SDK sh build.sh --release 0.9
JIELI_TOOLCHAIN=/path/to/jieli/toolchain AC79_SDK=/path/to/fw-AC79_AIoT_SDK sh tests/run_wanderer_tests.sh
```

The 123,316-byte app is within the 581,564-byte slot; `.data + .bss` is 12,768 / 98,304 bytes and pool buffers are 232,928 / 344,064 bytes. The 9,040-byte loader is included. RAM code, HAL register and package checks passed. The complete Wanderer test script passed; it builds and tests a separate development package (`FM-1_920`). The release file above was independently parsed and verified.

The generated table has exactly 100 unique names and parameter sets. The host sweep rendered each patch without digital clipping or silence (RMS 361–4,099, peak 7,176/32,767) and checked 100 distinct auditions. The compiled release booted in fm1-emu 0.3.0 for 750 ms, reported 100 audio halves and 16 LCD frames, and responded to a 300–500 ms key press with 25,600 nonzero stereo PCM frames (peak 2,015, RMS 589). It reported 32 unknown MMIO accesses at 17 addresses. An emulated run is not a physical timing or sound-quality measurement.

## Installing and playing

Back up projects with your current firmware and keep a working recovery package. You can select the `.fwsc` from a browser installer that accepts *arbitrary local custom packages*, or use the included `tools/fm1_install.py` via USB-MIDI:

```sh
python3 -m pip install mido python-rtmidi
python3 tools/fm1_install.py --info
python3 tools/fm1_install.py /path/to/LVM-Wanderer-0.9-FM1.fwsc
```

On Home, PRESETS moves through sounds 1–100 and SELECT jumps banks of ten. The display shows the large sound number, and the [companion catalog](WANDERER_PRESETS_0.9.md) names each number. Zoom is in the system settings page. Confirmed slot 1 Save (SEQ, SEL) while stopped keeps the sound and effect settings. The record format remains compatible with the previous 0.8 slot; older Wanderer slots migrate on Load. The three former sounds are represented by numbers 1–3. None of these slots import SLOOP projects.

Subsequent author report: I have physically tested Wanderer 0.9 on my FM-1 and successfully rolled back to another firmware. Wider hardware coverage and detailed per-feature results are still being documented. See [HARDWARE_TESTING.md](HARDWARE_TESTING.md) for the recorded scope.

## Public testing release

See [STATUS.md](STATUS.md) for current limitations and [PUBLICATION_CHECKS.json](PUBLICATION_CHECKS.json) for the additional host and package checks. This is not a Firmware Hub listing or approval. The author has since confirmed a physical FM-1 test and successful rollback; see [HARDWARE_TESTING.md](HARDWARE_TESTING.md).
