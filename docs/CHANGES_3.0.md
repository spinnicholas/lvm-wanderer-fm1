# LVM Wanderer 3.0 RC2

3.0 is a major overhaul built on 2.0 RC11. The layout, the button names and
the Anchor/Memory idea are unchanged, as are your saved projects and the
`FM-1_929` identity. What changes is what happens while you listen: Memory
now travels a little further from the Anchor every loop.

**Status:** compiled with the pinned JieLi toolchain and passes the host test suite.
It has **not yet been tested on an FM-1.** See [Build and test status](#build-and-test-status).

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

## Metronome

Settings → Output, knobs 3 and 4:

| Knob | Setting | Behaviour |
| --- | --- | --- |
| 3 CLICK | OFF / REC / ALWAYS | OFF is silent. REC clicks while you record. ALWAYS clicks whenever the loop plays. SELECT steps through them. |
| 4 CLICK LEVEL | 25 / 50 / 75 / 100% | Click volume, separate from the music. |

* **Count-in.** With CLICK on REC or ALWAYS, tapping **REC** from stop counts in one bar
  of four clicks. Recording and the loop start together on the next downbeat.
  The screen shows a huge 4, 3, 2, 1 with four beat dots, and the REC light flashes on
  each beat. Tap REC again (or PLAY, or hold HOME) to cancel.
* **A better click:** a short woodblock-like tick with a natural decay, instead of
  the old 12 ms beep that only played while recording. The first beat of every bar
  is higher and louder.
* The click joins the output after every effect, so it is never printed,
  scattered or sent to the reverb.
* CLICK and CLICK LEVEL are saved with your preferences. CLICK starts OFF, as
  before, so REC from stop starts immediately until you turn it on.

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

* **Home pictures the melody.** Each card has a mini piano roll: one bar per
  note at its pitch and length, with thick connecting lines. The Memory card
  draws Memory as you hear it over a faint line of the Anchor, so you can see
  it wandering away and coming home. Melted drips show as dots. A thick
  line marks the playhead.
* Cards have a soft top-lit gradient in every theme.
* Every message shows a word as well as a symbol.
* In Edit, the row for the knob you are turning is a solid colour. **Each
  row has a thick bar showing where the knob sits in its range.**
* Melt, Drift, Scatter, Tempo and Level show a large number, a word, and a
  meter made of 16 large segments.

## Sound engine

### Two new engines

* **SUPER** (Edit → Oscillators → Engine): a modern unison voice. It plays a
  centre oscillator plus six detuned copies, and each copy can be a saw, a pulse,
  a saw+pulse, or a pure "glass" sine at the centre. **Spread** sets the
  detune: fine near zero, wide at the top. **Voices** sets how loud the six
  copies are. **Width** spreads them across real stereo. The side signal goes
  through the same filter as the centre, so the sound stays even across the
  stereo field. Sub and Air (noise) add weight and breath. Melt widens and
  detunes it over the loops.
* **VOICE:** a formant engine. A saw, pulse or breathy source, plus
  a second detuned "choir" partner, goes through three resonances that shape
  it into vowels. **Vowel** moves smoothly through A-E-I-O-U, **Formant**
  makes the voice sound smaller or larger, **Focus** makes the vowels clearer,
  and **Breath** adds air. Melt moves the vowel, so a wandering Memory
  slowly changes the vowel it sings.
* On the host computer both cost within about 15% of the Analog engine, which
  already runs on the FM-1.

### Every engine

* **Velocity shapes tone as well as volume.** Velocity 100 (the FM-1 keys)
  sounds as before; softer notes, including those Melt softens, are darker.
* **Analog and Phase oscillators drift a few cents**, more with Detune.
* **Voices spread gently in stereo.** Scatter and printed audio fade the
  spread with the dry voice.
* Fixed an overflow, also present in 2.0, where a very loud voice could
  wrap the DC blocker.

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

## Presets: a new 3.0 bank

150 sounds in 15 families of ten: leads, basses, modern basses, poly,
keys and tines, organs, voices, strings/brass/winds, pads, textures, plucks,
guitars, mallets, bells, and percussion and effects.

* **76 are brand-new recipes:** 28 on SUPER, 16 on VOICE, 28 on Analog
  and 4 on Phase. Examples: Afterglow (now a SUPER lead), Night Reese, Wide
  Hull, Northern Lights, Slow Dawn, Aah, Ooh and Ee Choir, Monk Chant, Whisper,
  Robot Vowel, Silk Section, Brass Section, Tide Pool, Rising Tide.
* **74 keep the tuned 2.0 recipes** for the FM, FM6, string and modal sounds
  (keys, organs, plucks, guitars, mallets, bells, drums). Each has a new Melt
  journey suited to its family, and several gain movement. 34 have new names.
* Afterglow is still preset 1, the startup sound and the hold-PLAY recall.
* New sounds are level-matched by rendering them, against the loudness of
  the 2.0 bank. Sustained sounds are matched by average level, plucked and
  struck ones by their attack. Carried-over sounds keep their tuned 2.0 levels.
  `tools/level_bank_3_0.py` repeats the measurement.
* Projects saved with 2.0 keep the 2.0 bank, and v13 and v14 Layer sounds keep
  their 2.0 sounds.

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

**Native FM-1 build** (pinned, checksum-verified JieLi toolchain and AC79 SDK, GitHub
Actions run 42137785068, source `701031f`):

| Region | Used | Limit |
| --- | ---: | ---: |
| App image | 570,584 B | 581,564 B |
| RAM (.data + .bss) | 94,672 B | 98,304 B |
| Audio pool | 342,536 B | 344,064 B |

RAM code makes no calls, and hardware registers are only touched in `hal/`.
Package `LVM-Wanderer-3.0-RC2-FM1.fwsc`: 612,196 bytes, identity `FM-1_929`, SHA-256
`78d807b81714ce5463ccc28d79e26f182e274af23aee243a650dc4ddc033de1f`.

**Host tests:** `tests/run_core_tests.sh` passes. It covers the 2.0 regressions, the
1.2 product tests, the engine comparisons, the LED tests, `wanderer_3_0_test.c` and
`wanderer_engines_3_0_test.c`. Every other host test program that passes on RC11 also
passes. The wander engine, metronome, new engines and adversarial runs also pass under
AddressSanitizer and UndefinedBehaviorSanitizer.

Not run, because they also fail on unmodified RC11: `wanderer_1_4_timing`,
`wanderer_scatter_audition`, `wanderer_theme_test`, the old `render_bank_*` and
screenshot programs, and the `1.1-navigation`, `led-navigation` and `rc2-navigation`
scripts. They fail on old expected results or missing output folders.

**Not yet done:** testing on an FM-1. CPU load on the device is not measured; on the
host, SUPER and VOICE cost within about 15% of Analog. Before release, check:

* boot, and every tap and hold
* four voices with every effect at full, listening for crackles
* wandering, with Keep and Return
* the count-in
* save and load
* switching back to 2.0 RC11 and to the stock firmware
