# LVM Wanderer 1.2 RC2 for M-VAVE FM-1

Chord recording with independent held durations, consistent HOME/Anchor release, coherent local chord quantization and age-aware full-step replacement, retaining the musical 1.1 synth, licensed symbols, oscilloscope and approved splash. This is the identity-corrected FM-1_929 build of RC2. Nicholas confirmed the original RC2 hardware installation and rollback routes; Sven requested a short V15 install-and-return re-check for these corrected bytes. See docs/CANDIDATE_STATUS.md and docs/IDENTITY_CORRECTION.md.

Read docs/USER_GUIDE_1.2.md, docs/INSTALL_AND_RECOVERY.md and docs/TEST_REPORT_1.2.md. docs/DEVELOPMENT_PROMPT_1.2.md and docs/ANTICIPATED_FEEDBACK_1.2.md record the design scope and decisions. Upstream credits remain in LICENSING.md, LICENSE and README-DINGHY.md. Earlier version-specific docs are historical.

Set JIELI_TOOLCHAIN and AC79_SDK to the pinned dependencies described in README-DINGHY.md, then run `sh build.sh --release 1.2-rc2`. Run `STOCK_V15=/path/to/authentic/FM-1.fwsc BASELINE_09=/path/to/original-0.9-release.zip sh tests/run_release_tests.sh` for software gates. These tests never flash a connected device. Emulator validation additionally requires fm1-emu 0.3.0. Inkscape and Pillow generate the licensed icon masks offline.

New v5 projects retain up to four events per step per lane, held durations, root, scale and 32-step storage. Historical v1/v2/v3/v4 imports are supported. 1.1 and earlier cannot open new v5 projects; preserve older slots. Four synth voices remain shared by live keys, MIDI and both lanes.

For the identity-only comparison, obtain the original RC2 firmware from its historical release and run `python3 tests/identity_correction_test.py /path/to/original-RC2.fwsc dist/LVM-Wanderer-1.2-RC2-ID929-FM1.fwsc` with AC79_SDK set. It validates the exact original SHA-256, both package identities and CRCs, the permanent runtime identity, unchanged loader/head and full flash equivalence when only the runtime identity is restored.
