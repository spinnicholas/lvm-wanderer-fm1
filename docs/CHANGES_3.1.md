# LVM Wanderer 3.1 RC1

3.1 builds on 3.0 RC3. The sound is unchanged: all 150 presets render the same
audio, sample for sample. What changes is how Wanderer looks, how it starts, and
how it treats data from earlier versions.

**Status:** compiled with the pinned JieLi toolchain and passes the host test suite.
It has **not yet been tested on an FM-1.** See [Build and test status](#build-and-test-status).

> **Installing 3.1 erases your saved Wanderer projects and settings** the first
> time it starts. This is deliberate: see [Fresh start](#fresh-start-on-every-new-version).

## Colour is back

* **A new default theme, WANDERER.** Deep navy with a teal Anchor, a violet Memory
  and a gold Layer, after the look of 1.2–1.4. Coral and sky blue are used as
  accents. **Contrast is no longer the default.** It is one hold of OCT− away, next
  to Light, Amber, Ocean and Violet.
* **Each musical part keeps its colour on every screen:** Home cards, headers, focus
  screens, meters, messages, confirmations, Save/Load and the oscilloscope.
* **Home, redesigned.** Tinted Anchor and Memory cards with large colour symbols.
  An EDIT tag marks what your knobs and keys change. The wander count sits at the
  top right, and a tall violet bar on Memory's edge shows how far it has gone.
  Each card has a melody picture.
* **The Layer row returns,** with the stacked-squares symbol from the older versions.
  It is gold, shows the Layer's own melody line, and has an ON/OFF switch showing
  whether the Layer plays (set it with Settings → LAYER, knob 2). When the Layer is
  chosen, its row gets a thick gold edge and a filled LAYER tag.
* **Edit and Settings pages are coloured by group**, and each knob bar takes its page
  colour:

  | Area | Colour |
  | --- | --- |
  | Envelope pages | gold |
  | FM pages | sky |
  | Scatter and Pattern | coral |
  | Print | teal |
  | Other sound pages | violet |
  | Accessibility | sky |
  | Effects | coral |
  | SYSTEM | teal |
  | Other Settings pages | gold |
* **Every theme is checked:** text at least 7:1 against every surface it sits on,
  coloured symbols at least 3:1, and text on a coloured tag at least 4.5:1. The
  check is `tests/theme_palette_contrast.py`, and it runs with the core tests.

  | Theme | Text | Accents | Text on accent |
  | --- | --- | --- | --- |
  | WANDERER | 12.63 | 7.61 | 8.77 |
  | CONTRAST | 16.82 | 13.41 | 14.06 |
  | LIGHT | 9.31 | 6.32 | 6.32 |
  | AMBER | 14.85 | 10.18 | 11.02 |
  | OCEAN | 15.00 | 10.20 | 10.97 |
  | VIOLET | 14.35 | 10.78 | 11.41 |
* **Normal and Zoom are both finished.** A gallery harness
  (`tests/capture_gallery_3_1.c`) renders 56 screens in three themes at both sizes
  for review.

## Animated start

The LVM mark appears, then the eight letters of "Wanderer" glide in from the
right one after another and settle into the word.

* **Timing.** Each letter starts 170 ms after the one before and glides for 650 ms
  with an easing curve. Every letter has landed by about 1.85 s, and the whole
  splash lasts 2.7 s.
* **Drawing.** Only the 38-row strip holding the word is redrawn, at about 33 frames
  a second, in the existing band buffer. There is no frame buffer, no allocation and
  no work in the audio interrupt.
* **Any key skips the animation.**
* **Reduced Motion** shows the finished logo for 1.5 s, without movement.
* **Encoding.** `tools/gen_wanderer_splash.py` splits the supplied artwork into
  eight letters by connected shapes and stores each as a 4-bit alpha mask
  (21,263 B for the mark and letters together).

## Fresh start on every new version

Old project data from earlier versions was a likely source of glitches. 3.1 starts
clean:

* **On the first start of a new firmware build,** Wanderer erases its settings
  sector and all eight project slots: 33 flash sectors, one per animation frame
  behind the splash. The settings sector goes first, so an interrupted erase simply
  starts again on the next power-up.
* **A key press skips the letters, never the erase.** Home appears when the erase
  is done.
* **The build identity** is an FNV-1a hash of the version and the source commit,
  stored in the settings record (preferences format 3). Restarting the same build
  keeps everything.
* **Nothing outside Wanderer's own sectors is touched.**

Because nothing old survives, all old-project support is gone:

* the v3–v14 project formats and their migrations;
* the "CLASSIC" sound path (the 1.x engine, its wavetables and preset tables);
* the older factory banks.

The project format is now v16, with a new magic number (`LVM3`). Anything else is
refused.

* **Source removed:** `wanderer_waves.h`, `wanderer_presets.h` and
  `wanderer_factory_legacy.h`. In `firmware/src` that is 1,334 lines removed and
  489 added.
* **Flash freed:** see [Build and test status](#build-and-test-status).

### Tests removed, and why

| Test | Why |
| --- | --- |
| `wanderer_1_1_test`, `wanderer_1_1_stress` | Exercised the 1.1 CLASSIC engine and v3–v5 project migration, both removed. |
| `wanderer_1_3_test`, `wanderer_operations_1_3`, `wanderer_timing_1_3` | Exercised 1.3-format projects and the legacy wavetable voice. |
| `wanderer_bank_1_5_test`, `compare_factory_compat_1_5` | Compared the 1.5 factory bank and its compatibility mapping, now removed. |

The tests that remain were moved to the current format. New tests cover:

* a blank unit;
* a same-version restart that keeps saves;
* a new version that erases everything;
* an interrupted erase that repeats;
* a damaged settings record;
* rejection of legacy flags, old banks and the old magic number;
* the animated splash: full first frame then strip only, timing, Reduced Motion,
  a key skip, and a skip during an erase.

## Sound and performance

The audio code is the same as RC3, except one bit-exact change: SUPER's square
height now comes from a small table instead of chained comparisons.

**The same audio.** For each of the 150 presets, with four voices and every effect
on, the rendered output is identical to RC3's.

**Instruction counts.** The table below shows the worst preset (preset 22 at `-O2`,
preset 18 at `-Os` for 3.1) and the mean of all 150. These are Valgrind
instruction counts on a computer, not timings on the FM-1. Both versions were
measured with the same harness in the same session.

| Build | RC3 worst | 3.1 worst | RC3 mean | 3.1 mean |
| --- | --- | --- | --- | --- |
| `-Os` (the device's setting) | 57.98M | **57.51M** | 49.12M | **48.79M** |
| `-O2` (as used for the 53.3M figure in 3.0's notes) | 53.28M | 53.78M | 47.47M | 47.69M |

* **At the device's own optimisation level, 3.1 costs less than RC3,** both in the
  worst case and on average.
* **At `-O2`, 3.1 is about 0.9% higher.** The audio is identical, and the per-line
  profile puts the difference in unchanged lines of the SUPER loop. It comes from
  the host compiler's code layout, not from extra work.
* **The load guard from RC3 is kept.**

## Build and test status

*Filled in from the native build.*
