# LVM Wanderer 3.0 RC1

3.0 is a major overhaul built on 2.0 RC11. The layout, the button names and
the Anchor/Memory idea are unchanged, as are your saved projects and the
`FM-1_929` identity. What changes is what happens while you listen: Memory
now travels a little further from the Anchor every loop.

**Status:** passes the host test suite. It is **not yet compiled for or tested on
an FM-1.** See [Build and test status](#build-and-test-status).

## Melt and Drift now wander

In 2.0, Melt and Drift recalculated the same four-loop pattern and
snapped back to the original every fourth loop. In 3.0, each completed loop
moves Memory one more step from what you recorded, so the Anchor becomes a
real home to return to.

* **Drift** (knob 1 on Home is Melt, knob 2 is Drift) walks single notes up
  or down one step of the current scale. Low settings move about one note
  per loop and never move the first beat or root notes. High settings walk
  further and can leap an octave. A gravity pulls every note home: notes
  past the Drift range always step back, and with Drift at zero all notes
  return to their recorded pitches over a few loops.
* **Melt** works gradually: weak beats get softer, notes stretch toward
  legato, and quiet echoes one step lower drip into the empty step after a
  note. The patch's own Melt timbre deepens over several loops. Each
  sound now has its own Melt character, see [Presets](#presets).
  At very high Melt, an occasional off-beat note dissolves completely.
  Turn Melt down and all of this relaxes.
* Wandering pauses while you record, while you listen to the Anchor and
  while a printed RAM loop plays. Layer 2 does not wander; it stays steady
  under Memory.
* **The first loop that wanders protects your recording as the Anchor**,
  if you have not caught one yet. This does not use up Undo.
* Recording a new note into Memory starts that note's wander over.

### New on Home

| Action | Result |
| --- | --- |
| Tap **GLO** | **Keep.** Saves the Memory you hear now as the new Memory. Undo (ENV) reverses it. The Anchor is unchanged. |
| Hold **GLO** about 1 second | **Return.** Puts Memory back to what you recorded. The screen shows BACK HOME. |
| Tap **LFO** | Compare with the Anchor, as before. |
| Hold **OCT−** 1.5 seconds | Switch to the next colour theme, for when the room light changes. A tap still lowers the octave. |

On Home, the Memory card shows **how many loops** Memory has wandered (top
right) and a thick **distance bar** that fills as Memory moves away from
what you recorded. It is drawn in both Normal and Zoom.

## LEDs

* **PLAY** flashes bright on each beat and stays dim between beats, so you
  can see the tempo without reading the screen. **REC** does the same while
  recording.
* **Every C key (or the key of your chosen root) glows dimly on Home** as a
  landmark when the arpeggiator is off.
* **GLO** glows when there is something to Keep and flashes briefly each
  time Memory moves at the end of a loop.
* **LFO** flashes on the first beat of the loop once Memory is at least
  halfway along the distance bar, a reminder to compare with the Anchor.

## Screens

* Every message now shows a word as well as a symbol, for example KEPT,
  BACK HOME, UNDONE, NOTHING TO UNDO, STOP FIRST, or the name of the new
  theme.
* In Edit and Settings, the row for the knob you are turning is a solid
  colour with dark text, instead of only an outline.
* The Melt, Drift and Scatter screens add a word for the setting: DRY,
  SOFT, MELTING or LIQUID for Melt; STILL, GENTLE, WANDERING or FAR for Drift;
  CLEAN, SPARSE, BUSY or WILD for Scatter.

## Sound engine

* **Velocity changes the tone as well as the volume.** Velocity 100, the
  FM-1 keys, sounds the same as before. Softer notes, including those Melt
  softens, sound darker. Louder MIDI notes are a little brighter.
* **Analog and phase-distortion oscillators drift slowly** by a few cents,
  more on patches with more Detune.
* **Voices are spread slightly in stereo**, even with every effect off.
  Scatter and printed audio fade the spread with the dry voice.

## Reverb

* Each delay line's feedback is set from its length, so all lines die away
  at the same rate and the tail is smoother, with less metallic ringing.
* Hall, Plate, Cavern and Deep Space slowly modulate two lines in the
  feedback loop, which keeps long tails from ringing at fixed pitches.
* A low cut inside the loop keeps long tails from building up bass. It
  stops at exact silence, so it never leaves a hum.
* Extra output taps widen the stereo image.
* Every type except Ambient fades to exact digital silence. Ambient keeps
  its very long tail. Switching Reverb off still clears it.

## Scatter

* Gestures repeat as a **motif**. The same step gets the same gesture the
  next time through, so Scatter plays a rhythm rather than random noise.
  **Variation** (Scatter Grid, knob 2) sets how often a new motif replaces
  it at the start of a loop: 0 keeps it forever, full changes it every loop.
* In Free, the first beat stays clean below high Amounts, and the last two
  steps of the loop get more gestures, like a drum fill.
* Stutter and Rush repeats get quieter with each repeat, like a real echo,
  instead of staying at the same level.

## Presets

The current bank (bank 4) keeps the 150 2.0 names and voicings, with two
changes:

* **Each sound has its own Melt character.** In 2.0, 135 sounds shared five
  Melt settings. Now leads and basses get darker and thicker, keys get rounder,
  string and pad sounds open up and widen, and plucked, mallet and bell
  sounds soften. 142 of the 150 Melt settings are different.
* **69 sounds have slow movement.** Pads, strings, organs and keys move
  their filter slowly; leads and brass have light vibrato.

Projects saved with 2.0 keep the 2.0 bank. v13 and v14 Layer sounds also
keep their 2.0 sounds.

## Compatibility

* Projects are still saved as v15. 3.0 opens every project that 2.0 RC11
  can open.
* Projects saved with 3.0 use the new bank number, which 2.0 does not
  recognise. 2.0 will reject these projects without changing them. To open
  a 3.0 project in 2.0, keep a copy in a spare slot.
* Projects from before 3.0 (legacy and classic) Melt and Drift exactly as
  before.
* Wandering is not saved. Tap GLO (Keep) before saving if you want to save
  where Memory has wandered to.

## Cleanup

* Removed the four-loop Melt/Drift code for current projects. Older projects
  still use it.
* Sequenced notes and Melt drips are transposed by one shared helper.
* The Anchor is now copied by one function, which Catch and the automatic
  Anchor both use.
* Fixed a test bug where right-channel samples from the last block were
  reused. The firmware's audio buffer was already cleared before each block.

## Build and test status

Done on the host computer:

* `tests/run_core_tests.sh`: the 2.0 regression suite, the 1.2 product
  tests, the engine voice comparisons, the LED tests and the new
  `tests/wanderer_3_0_test.c`. It covers wander growth, bounds, gravity and
  landmarks, Keep/Return/Undo, the automatic Anchor, playback pausing, the
  GLO and OCT− holds, notice words in every theme and size, beat LEDs,
  Scatter motifs, reverb silence, stereo spread, the bank, and 6,000
  adversarial loops under AddressSanitizer and UndefinedBehaviorSanitizer.
* All 42 host test programs in `tests/` that pass on RC11 also pass here. Not run:
  `wanderer_1_4_timing`, `wanderer_scatter_audition`, `wanderer_theme_test`,
  the `render_bank_*` programs and the screenshot capture programs. They
  fail the same way on unmodified RC11 because of old expected values or
  missing output folders. The `1.1-navigation`, `led-navigation` and
  `rc2-navigation` scripts also fail on unmodified RC11 because their
  expected results are old.
* Estimated size: about 15 KB more flash, against about 32 KB free in
  RC11, and under 1 KB more RAM. The audio pool is unchanged. The native
  build checks these limits exactly.

**Native FM-1 build:** the JieLi compiler is not available in this
environment. `.github/workflows/fm1-native-build.yml` builds the version
named in `RELEASE` with the pinned, checksum-verified JieLi toolchain and
the AC79 SDK. It runs the host tests, checks the package and commits
`dist/LVM-Wanderer-3.0-RC1-FM1.fwsc` with a size report. You can also run it
locally: `RELEASE=3.0-RC1 sh tools/compile_native.sh`.

Before release, test on hardware: boot, every button and hold, audio under
four voices with all effects at full, Keep/Return while playing, save and
load, and switching back to 2.0 RC11 and to the stock firmware.
