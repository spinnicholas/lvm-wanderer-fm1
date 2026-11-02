# Installing Wanderer 1.4

Wanderer 1.4 publishes the exact RC6 build accepted by Nicholas after his standard physical FM-1 tests on November 2, 2026. The package identity is FM-1_929. Download LVM-Wanderer-1.4-RC6-FM1.fwsc from the current GitHub release; its SHA-256 is 9135b62169409efb73a6492561673bb5465979563824605956ab3316c50d5b32.

## Install

Keep your existing firmware and projects backed up. Use the FM-1 installation method you normally use with a USB data cable and stable power. The Firmware Hub mirrors published releases on its own refresh schedule; check that its Wanderer card says 1.4 before choosing its Install button. A stale 0.9 card still refers to the older build.

The included Python installer supports the package directly. Install Python 3 and its MIDI dependencies with `python3 -m pip install mido python-rtmidi`. Close other MIDI applications, connect the FM-1, then run `python3 tools/fm1_install.py --info` from the source folder.

Run `python3 tools/fm1_install.py /absolute/path/to/LVM-Wanderer-1.4-RC6-FM1.fwsc`. The tool validates the package, checks the model, serves the update and verifies the returned identity. Confirm the displayed device and package when prompted, and leave the cable connected until completion. On Windows, `py` can replace `python3`.

## Projects and previous firmware

New project saves use format v8. Older Wanderer projects can be imported without rewriting their original slots. Save converted work into spare slots if you need to return to earlier versions. Different firmware families share flash storage but do not necessarily share project formats. Printed audio remains RAM-only; record it externally before powering off or loading another project.

The release source includes the prior 0.9 rollback package at `recovery/LVM-Wanderer-0.9-FM1.fwsc`. Install it with the same Python tool if you need that version. For SLOOP or another compatible custom package, follow that project's own update method and retain its exact firmware file.

## Authentic V15 restoration

Use your original unmodified V15 file with identity FM-1_015, 699,956 bytes, SHA-256 db1642b2b6fa5c2cccb11ffd13878068bb28601678d3644049f99dc40e7edb8a. The provided tool's explicit restoration path is `python3 tools/fm1_install.py --stock-v15 /path/to/FM-1.fwsc`. It accepts that verified V15 package and retains the firmware's model, CRC and official-loader checks. Vendor firmware is not redistributed here.

## USB rescue

OCT− alone at power-on retains the inherited USB rescue path. Reconnect and use the same package to finish an interrupted custom update. HOME + SELECT is the runtime scope shortcut; HOME + EDIT is the runtime Zoom shortcut. Neither replaces startup rescue.

[Current hardware confirmation](HARDWARE_TESTING.md) · [Release notes](RELEASE_1.4.md) · [Quick start](QUICK_START_1.4.md)
