# Install Wanderer 1.5 RC1

Use **LVM-Wanderer-1.5-RC1-FM1.fwsc** from this package with your usual FM-1 firmware update method. Its permanent firmware identity remains **FM-1_929**. This is the new 150-preset candidate, ready for a physical-device audition.

The included command-line installer is in `source/tools/fm1_install.py`. With a USB data cable connected, open a terminal in the extracted `source` directory and run:

```sh
python3 -m pip install mido python-rtmidi
python3 tools/fm1_install.py --info
python3 tools/fm1_install.py /absolute/path/to/LVM-Wanderer-1.5-RC1-FM1.fwsc
```

Follow the installer prompts and keep the cable connected while writing. Keep your known-working 1.4 firmware and the recovery files you normally use. The source includes the same recovery loader and Python installer as the previous release.

New project saves use **v9**. Existing versions 1–8 import without rewriting their original slots. Old custom patches and Anchor snapshots are retained; 1.3 and 1.4 factory fallbacks and names remain available to imported projects. Save into a spare slot if you want to return to 1.4, since 1.4 cannot read a new v9 save.

PRESET moves through all 150 new sounds. SELECT moves through fifteen groups of ten. Browse the supplied preset book to hear dry previews and find the engine and intended use of each sound. Hold notes for the sustained leads, brass, organs, strings and pads; plucked and struck models decay naturally. Try Melt from zero through its range, then add your usual Scatter and effects.

The three-second previews are production-DSP host renders. Software checks and emulator execution do not replace listening on your FM-1. Useful audition points are sustained chord body, pluck attack versus tail, transitions between patches, low and high registers, four-voice playing, and loading an existing 1.4 project.
