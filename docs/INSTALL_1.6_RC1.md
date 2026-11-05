# Install Wanderer 1.6 RC1

Use `LVM-Wanderer-1.6-RC1-FM1.fwsc` with your usual FM-1 firmware updater.
Permanent identity remains `FM-1_929`. This is a listening candidate; keep the
hardware-accepted 1.4 firmware available.

The complete source includes the existing recovery loader and installer. From
the extracted source directory, the command-line method is:

```sh
python3 -m pip install mido python-rtmidi
python3 tools/fm1_install.py --info
python3 tools/fm1_install.py /absolute/path/to/LVM-Wanderer-1.6-RC1-FM1.fwsc
```

Follow the installer prompts and keep the cable connected while writing.

Project versions 1–9 import. Saved patch values, notes and Anchor snapshots are
retained, but modern projects sound different through the rebuilt engines.
New saves use v10, which older firmware cannot read. Use spare slots when
retaining a rollback project. Importing alone does not rewrite an old slot.

PRESET browses 150 sounds. SELECT moves through fifteen groups of ten. In the
editor, PRESET also reaches the new Color page with model-specific controls.
HOME + EDIT continues to toggle Zoom from anywhere. The existing accessibility
controls are retained.

The preset book contains dry previews of every patch. The comparison plays
1.5 followed by 1.6 for each listed sound, matched in RMS over the held portion
for listening comparison only. The full bank previews retain factory levels.
The G-neighbour clip plays F3, F-sharp3, G3, G-sharp3 and A3, in that order,
through Analog, FM, Phase, String and Modal at fixed velocity with effects off.

On the FM-1, first compare the low G with adjacent keys at a fixed patch and
Melt setting, then try four-note chords, fast retriggers, sustained notes and
existing project/Anchor playback. The physical-key cause from the video has
not been conclusively identified by software checks.
