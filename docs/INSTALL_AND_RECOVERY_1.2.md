# Installing Wanderer 1.2 RC2 and returning to other firmware

This is an installable FM-1 release candidate, identity FM-1_929. It is not yet a hardware-validated stable 1.2. Nicholas confirmed all four V15/SLOOP installation and return routes for the original RC2. This identity-corrected build awaits the short V15 install-and-return re-check requested by Sven. Host tests and emulation cannot establish that every FM-1 will recover from every interrupted write.

## Before changing firmware

Back up your current firmware's projects using its own supported method. Wanderer and other firmware share flash storage; successful firmware rollback does not imply project preservation. Keep stable USB power, a working data cable, the exact candidate package, the included unchanged 0.9 package and your original M-VAVE v15 file offline. Finish any interrupted update before starting a different firmware transition.

## Install with the included Python tool

On a computer with Python 3, install its MIDI dependencies with `python3 -m pip install mido python-rtmidi`. From the extracted release folder, use `python3 tools/fm1_install.py --info` to inspect the connected FM-1 identity. Close other MIDI applications first. On Windows, `py` can replace `python3`.

Run `python3 tools/fm1_install.py LVM-Wanderer-1.2-RC2-ID929-FM1.fwsc`. The tool validates package CRCs, checks the model, serves the update, and verifies the returned identity. Confirm the displayed package and device when asked. Leave the USB cable connected until it completes. If your system assigned a generic port name, use the tool's `--port` option with the observed name.

A browser installer must explicitly support arbitrary local custom FM-1 packages. A page that only offers its own firmware is not an installer for Wanderer. Firmware Hub listing, Android installation and browser behavior with this corrected build have not been qualified for this candidate. The provided computer installer is the documented path; Android users should retain their already-tested 0.9 route until 1.2 browser installation is verified.

## Return to Wanderer 0.9 or a compatible custom package

Run `python3 tools/fm1_install.py recovery/LVM-Wanderer-0.9-FM1.fwsc` to reinstall the included known-good 0.9. The file's SHA-256 is `c6c0a2829a5c1fe5959cb0d302c4de152b97fe3f29e469c400d1fbaefa236dc9`. This exact 0.9 binary is unchanged.

Compatible Felucca-family custom packages with the recognised update loader can be served by the same tool. Use each project's own installation requirements. Fresh 1.2 software tests exercise real Wanderer 0.9 and 1.1 RC2 packages. Earlier 1.0 RC2 evidence also exercised SLOOP 2.8.4 with this unchanged loader; that does not qualify every community firmware with 1.2. The 1.2 candidate's custom loader is byte-identical to 0.9 and writes only its allowed application area. It does not rewrite the retained flash head.

## Return to authentic stock v15

Obtain the unmodified M-VAVE FM-1 v15 `FM-1.fwsc` file. The version name alone is insufficient: verify identity `FM-1_015`, size `699956` bytes and SHA-256 `db1642b2b6fa5c2cccb11ffd13878068bb28601678d3644049f99dc40e7edb8a`. The vendor's current download at the old filename now serves v16; it is not the pinned v15 file. Keep your existing v15 copy. Vendor firmware is not redistributed in this package.

From a running Wanderer, run `python3 tools/fm1_install.py --stock-v15 /path/to/FM-1.fwsc`. The new flag accepts only the exact verified v15 bytes. It retains CRC checks, hardware-model checks and the firmware's official-loader validation. The firmware compares the device's retained first 16 KiB with v15 before staging its official loader. A mismatched head is rejected before staging erases. The stock loader can write the full stock image; the custom loader's narrow write window is not used as a shortcut.

If the device is in an unfinished custom update, this stock operation is refused. Finish the custom update with its original package, boot it normally, then request v15. If an official-v15 update was interrupted and its `ota-FM-1_015` loader remains active, the same verified v15 file can resume it. Do not use `--force` for stock recovery or rename another firmware to imitate v15.

M-VAVE's M-UPGRADE is an alternative official update tool. The SLOOP maintainer currently also documents Clipper's Return to the official firmware page for a device that M-UPGRADE does not list: https://isod89.github.io/clipper-fm1/webapp/installer/ . Those external tools are maintained separately; this candidate has not physically validated them with FM-1_929. Close other MIDI apps before connecting.

## USB rescue and an interrupted write

If the application does not start normally, switch off and hold OCT− alone while switching on. The inherited application-level USB rescue starts without the normal audio/UI session when startup and its USB code remain intact. Reinstall the candidate or known-good 0.9 with the included tool. Rescue intentionally reports a distinct recovery identity rather than pretending that the normal application booted successfully.

If an update was interrupted after the custom loader started, reconnect and rerun the installer with the same custom package. The loader keeps an unfinished update record and verifies rewritten sectors. The host tests cover retry, corruption, missing confirmation and a later resumed session.

This is not a second independent firmware bank. If power loss or damage prevents startup, the stock boot path or application rescue may be unavailable. Follow the FM-1 Transporter recovery project's current instructions: https://github.com/Quixotic7/MvaveFM1Unbricker . Do not improvise flash offsets or erase the first 16 KiB.

## Physical acceptance for stable 1.2

Test the candidate's boot, speakers/headphones, all keys and encoders, release behavior, MIDI, LED meaning and brightness, settings, project Save, power-cycle and explicit Load. Confirm 0.9→1.2→0.9, then 1.2→authentic v15 and installation back from v15. Confirm the desired custom firmware transition, such as SLOOP, in both directions. Restore any backed-up music with its originating firmware. Keep 1.2 RC2 labelled a candidate until these checks pass on a real FM-1.

1.2 changes application chord recording, project feedback and interaction transitions. The 1.1 DSP, presets, effects, scope and approved splash are retained. The protected update/recovery foundation remains identical to 0.9. HOME + SEL/SCL is a runtime combination; OCT− alone at power-on retains its original rescue function. Earcon preferences remain compatible. New version-five projects cannot be opened by older Wanderer firmware; save conversions into spare slots when retaining old-version projects is important. The complete release also includes the unchanged 1.1 RC2 and 1.0 RC2 packages as an application rollback option.

1.2 imports 1.1 version-four projects without rewriting the original slot. A project explicitly saved in 1.2 uses version five, and cannot be loaded by 1.1. Keep a spare slot for conversion and keep the original 1.1 package for application rollback.

The 1.2 RC2 package also retains the exact 1.2 RC1 binary. Both use the same v5 project layout, so projects saved in RC2 remain readable in RC1. The archived RC1 and original RC2 used FM-1_932 incorrectly. This corrected RC2 uses Wanderer's permanent FM-1_929 identity. Use the full filename and SHA-256 to distinguish the bytes.

## Identity-correction re-check

Install the corrected file from authentic V15, confirm normal boot and running USB identity FM-1_929, then return to V15. Record the exact corrected file hash from CANDIDATE_STATUS.md with the result. Sven confirmed this is sufficient for the identity-only correction; the earlier four-route RC2 hardware report is retained separately.
