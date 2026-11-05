# Install Wanderer 1.7 RC1

Use `LVM-Wanderer-1.7-RC1-FM1.fwsc` with your usual FM-1 updater. Its permanent
identity remains `FM-1_929`. This is a listening candidate, not the public
hardware-accepted release.

The complete source includes the existing recovery loader and installer:

```sh
python3 -m pip install mido python-rtmidi
python3 tools/fm1_install.py --info
python3 tools/fm1_install.py /absolute/path/to/LVM-Wanderer-1.7-RC1-FM1.fwsc
```

Follow the installer prompts and keep the cable connected while writing.
Keep your accepted 1.4 firmware available for rollback.

Projects 1–10 import. New saves use v11 and cannot be read by older firmware;
save into spare slots. Notes, settings, patches and Anchor snapshots are
retained. Imported new-effect switches start off. The upgraded effects alter
wet playback; browsing new factory sounds selects the revised recipes.

PRESET browses all 150 sounds; SELECT moves through fifteen groups of ten.
Hold SELECT for settings. Turn PRESET through the existing pages to Bitcrush,
Tape Echo, Tape Color, Phaser and Wavefold. HOME + EDIT toggles Zoom anywhere.
FM6 editing and each effect's four controls are explained in SOUND_DESIGN.md.

Before-After plays 1.6 followed by 1.7 at matched RMS. Effects plays dry followed
by wet. The preset book contains 150 dry factory-level previews. On the FM-1,
compare held notes and attacks, play four-note chords, change patches while
tails ring, try the effect switches and test an imported project and Anchor.
