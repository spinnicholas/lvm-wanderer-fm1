# Wanderer 1.2 RC2 — permanent FM-1 identity correction

On November 1, 2026, Firmware Hub maintainer Sven Trogus confirmed that Wanderer must use **FM-1_929 for every version**. The identity identifies the firmware rather than its release. FM-1_930 through FM-1_940 belong to sloopDX; the original RC2's FM-1_932 identity was incorrect.

The corrected installable file is **LVM-Wanderer-1.2-RC2-ID929-FM1.fwsc**. It is 612196 bytes. Its SHA-256 is `c4fb1140ef05769ef443c23aca9b7220b096a9a9cd4ff2c7aa2be27f7656ee64`. The application version stays `1.2-rc2`; its running USB identity and package identity are both FM-1_929. The prepared release tag is `v1.2.0-rc2-id929`; public GitHub publication is pending permission.

The build script uses the same permanent FM-1 identity for development and release builds. Version numbers no longer select identities in another firmware's reserved range. The SMK build behavior is outside this FM-1 correction and is unchanged.

## Verification

The complete software release gates pass, including sanitizer regression/stress tests, all 100 preset checks, navigation/layout checks, package CRCs, OTA/loader staging and simulated installer transactions. The simulations include installation into an already-running FM-1_929 and returning to verified authentic stock V15; identical firmware identities do not prevent replacement.

The dedicated identity-correction comparison verifies the original RC2's exact published hash, both packages' CRCs and identities, the corrected running USB identity and exact equality of the compressed loader and retained flash head. Decrypting the new application and changing only its FM-1_929 runtime string back to FM-1_932 reproduces the entire original RC2 flash image byte for byte, including regenerated checksums. This establishes an identity-only change: the synth, UI, controls, effects and project data layout are unchanged.

Compiled emulation of the corrected package also passes the existing 7000 ms chord-recording, Save/Load, Anchor playback and HOME transition scenario, producing 301312 stereo frames at 44100 Hz with no halt. All six framebuffer captures are byte-identical to the original RC2 captures.

The rebuilt release is byte-identical to the first corrected build. The 33 protected platform/recovery sources, loader and retained head also pass comparison against the original 0.9 foundation. Detailed fresh logs are in `test-evidence/identity929/`; original RC2 evidence remains historical.

## Hardware record and remaining re-check

Nicholas confirmed all four V15/SLOOP installation and return routes on his FM-1 for the original RC2 bytes. That record is preserved. The corrected file has a different hash, and its short V15 install-and-return re-check has not yet been reported. Sven explicitly said this short check is sufficient for the identity-only correction. Confirm normal boot and running identity FM-1_929, then return to V15 using the verified original file.

Sven said a corrected public release is picked up automatically by the Hub within three hours. Hub refresh/activation is a separate observation, not a completed check claimed here.

## Reproduce

With the pinned JieLi toolchain and AC79 SDK configured, build using `sh build.sh --release 1.2-rc2`. Run the existing release gates in `tests/run_release_tests.sh`. For the exact identity-only proof, obtain the original RC2 package from its historical release and run `python3 tests/identity_correction_test.py /path/to/original-RC2.fwsc dist/LVM-Wanderer-1.2-RC2-ID929-FM1.fwsc` with AC79_SDK set. The comparison refuses any original input whose SHA-256 does not match the published RC2.
