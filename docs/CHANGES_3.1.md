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
  top right, and a violet bar beside the Memory symbol shows how far it has gone.
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

## Replace the Anchor

Before, tap SAVE only set an Anchor when there was none. Wanderer also makes the
Anchor for you the first time Memory wanders, so SAVE usually just said ANCHOR IS
SAFE, and no control could make a new one.

* **Now, with an Anchor already set, tap SAVE asks NEW ANCHOR / REPLACE ANCHOR.**
* **SELECT makes Memory, as you hear it now, the new Anchor.** Any wander you haven't
  kept is included. The wander count and distance start again from zero.
* **HOME cancels, and Undo (ENV) brings back the old Anchor and Memory together.**
* **It works while the loop plays.** It isn't offered while you're listening to the
  Anchor (LFO), or with printed audio in RAM. Those cases still say ANCHOR IS SAFE.
* **Hold SAVE** still opens project Save, as before.
* **Wording:** the NO ANCHOR screen says RECORD MEMORY, not RECORD LAYER 1. Elsewhere,
  LAYER 1/2 is now MEMORY and LAYER.
* `tests/wanderer_anchor_replace_test.c` covers all of this, under the sanitizers
  too.

## Zoom: larger sound names, centred knob numbers

* **The sound screen has its own Zoom layout.** The name gets the whole width and
  most of the height, on two lines for two-word names, and the number is 46 px
  instead of 21. Of the 150 names, 99 now show at 46 px (all were 23 or 34 px
  before), 43 at 34 px, and the 8 longest single words at a new 28 px size.
* **A new 28 px font fills the gap between 23 and 34 px.** It adds 7,516 B. Other
  Zoom pages also use it where a value was too long for 34 px.
* **The knob-number tabs are centred as a group.** Pages with three knobs
  (Accessibility, Arp Feel, Layer) used to sit to the left. Each digit is also
  centred in its tab.
* **Home:** MEMORY clears the card edge and the distance meter, the meter draws as a
  clean bar, and the Layer switch is wider, so ON and OFF never touch the knob.

## Clearer controls

A review of Home, Edit and Settings for controls that said one thing and did another:

* **The Layer switch can be flipped from Home: tap FX.** Before, the only way was
  Settings → LAYER, knob 2, so the switch seemed stuck on ON.
  * The switch acts when you let go, so holding FX for 1.6 seconds still asks New
    session without also flipping the Layer.
  * You see LAYER ON or LAYER OFF, and hear a rising or falling cue.
  * FX glows while the Layer is on.
  * An OFF Layer's melody line turns grey.
  * Undo reverses the switch.
* **Clear Layer has moved to Settings → LAYER, knob 4 (CLEAR).** Before, a tap of FX
  opened it. It still asks first, Undo brings the notes back, and it says EMPTY when
  there is nothing to clear.
* **Same words everywhere:** knob 2 on the Layer page says ON/OFF, like the switch,
  not PLAY/MUTE.
* **NO ANCHOR now says TAP SAVE TO SET.** Before, it said TAP LFO TO LISTEN, but with
  no Anchor, LFO only shows NO ANCHOR again.
* **LFO glows only when there is an Anchor to hear.**
* **Edit pages show a violet Memory or gold Layer symbol at the top right.** Before,
  a bare "1" or "2" there looked like a page number.
* **Guide fixes:**
  * Saving is "hold SEQ", not "tap SEQ". Tap SEQ is the click.
  * Settings opens with a tap of SEL. Holding SEL opens Accessibility.
* `tests/wanderer_layer_switch_test.c` covers the switch, the hold, the Layer page,
  CLEAR and the lamps, under the sanitizers too.

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
* **Flash freed:** removing old-project support freed about 68 KB of app image and
  23,280 B of RAM. The 3.1 additions (the 28 px font and the Anchor and Zoom
  changes) use about 8 KB of that, so the net saving is 59,784 B. See
  [Build and test status](#build-and-test-status).

### Tests removed, and why

| Test | Why |
| --- | --- |
| `wanderer_1_1_test`, `wanderer_1_1_stress` | Exercised the 1.1 CLASSIC engine and v3–v5 project migration, both removed. |
| `wanderer_1_3_test`, `wanderer_operations_1_3`, `wanderer_timing_1_3` | Exercised 1.3-format projects and the legacy wavetable voice. |
| `wanderer_bank_1_5_test`, `compare_factory_compat_1_5` | Compared the 1.5 factory bank and its compatibility mapping, now removed. |
| `capture_1_5`, `capture_1_6`, `capture_1_7`, `render_bank_1_3`, `render_bank_1_5`, `render_bank_1_6`, `render_bank_1_7` | Screenshot and audio renderers for the 1.x banks, which no longer exist. |

The tests that remain were moved to the current format. New tests cover:

* a blank unit;
* a same-version restart that keeps saves;
* a new version that erases everything;
* an interrupted erase that repeats;
* a damaged settings record;
* rejection of legacy flags, old banks and the old magic number;
* the animated splash: full first frame then strip only, timing, Reduced Motion,
  a key skip, and a skip during an erase.

### Old failures fixed

The 3.0 notes listed several programs as not run because they also failed on
RC11. All of them now run and pass:

* **`wanderer_theme_test`.** The test stored the 32-bit settings signature in a
  16-bit variable, which cut off the theme and click-level bits. It now uses 32 bits
  and checks the 3.1 rules: WANDERER is the default, and old records or records from
  another build are ignored. It is now in the core suite.
* **`wanderer_1_4_timing`** checks that audio and recorded notes are identical
  whatever the audio callback size. It failed because the effects' start-up memory
  clear was still running during the test, and that clear advances once per call.
  With the clear finished first, all 1,092 comparisons match. On the FM-1 the clear
  finishes during start-up and callbacks are always 256 frames, so the device's
  sound was never affected. It is now in the core suite.
* **Tools that needed setup.** The screenshot programs needed their output folders;
  `ota_test`, `ldr_test` and `wanderer_scatter_audition` needed file arguments; and
  `smk_flash_test` needs the SMK-37 build flag. With those, all pass. `ota_test`
  accepts the 3.1 package, and `ldr_test` passes for the RC3 → 3.1 update.
* **`render_bank_3_0`** renders only the current bank. `tools/level_bank_3_0.py`
  keeps the 2.0 loudness targets as fixed numbers, measured on RC3.

## Sound and performance

The audio code is the same as RC3, except two exact rewrites in SUPER's
six-voice loop:

* the square height comes from a small table instead of chained comparisons;
* the six detuned voices are summed first and multiplied once (4 multiplies per
  sample instead of 12). The constant terms cancel exactly in the stereo side.

**The same audio.** For each of the 150 presets, with four voices and every effect
on, the rendered output is identical to RC3's.

**Instruction counts.** The table shows the worst preset (preset 22 at `-O2`,
preset 18 at `-Os` for 3.1) and the mean of all 150. These are Valgrind
instruction counts on a computer, not timings on the FM-1. Both versions were
measured with the same harness.

| Build | RC3 worst | 3.1 worst | RC3 mean | 3.1 mean |
| --- | --- | --- | --- | --- |
| `-Os` (the device's setting) | 57.98M | **57.39M** | 49.12M | **48.55M** |
| `-O2` (as used for the 53.3M figure in 3.0's notes) | 53.28M | **52.98M** | 47.47M | 47.80M |

* **The worst case costs less than RC3's at both settings.**
* **At `-Os`, the average costs less too.**
* **At `-O2`, the average is 0.7% higher.** The audio is identical, so this comes
  from the host compiler's code layout in unchanged code, not from extra work.
* **The load guard from RC3 is kept.**

## Build and test status

Native build of source commit `f22f110dcc07` by the
[FM-1 native build workflow](https://github.com/spinnicholas/lvm-wanderer-fm1/actions/runs/42403454757),
with the pinned JieLi toolchain and AC79 SDK.

* **Package:** `dist/LVM-Wanderer-3.1-RC1-FM1.fwsc`, 612,196 B, identity `FM-1_929`.
  The build reported "FLASHABLE BUILD VALIDATED".
* **SHA-256:** `da603e50e629c7388a545425cc905dbda33c5812aa2dfe2bdafb63ebd98b32bb`

| | Limit | 3.0 RC3 | 3.1 RC1 | Change |
| --- | --- | --- | --- | --- |
| App image | 581,564 B | 571,888 B | **512,104 B** | −59,784 B (−10.5%) |
| RAM (.data + .bss) | 98,304 B | 94,752 B | **71,472 B** | −23,280 B |
| Audio pool | 344,064 B | 342,536 B | 342,536 B | 0 |
| Loader | — | 10,456 B | 10,456 B | 0 |

The `.fwsc` container is a fixed 612,196 B for both versions. The savings are inside
it: 69,460 B of headroom in the app image, up from 9,676 B.

**Host tests** run in CI before the compile and passed:

* the core suite;
* the palette contrast check.

**Also passed locally:**

* the full sweep: all 60 test programs in `tests/`;
* AddressSanitizer and UndefinedBehaviorSanitizer runs of the 3.0, adversarial,
  engines, release, editor, splash, memory and layer workflow tests.

**Not yet tested on an FM-1.** Things to watch on hardware:

* the first-start erase, and that a same-version restart keeps your saves;
* the splash timing and key skip;
* theme readability in real room light;
* the CPU readout on heavy SUPER and VOICE sounds with all effects on;
* a clean start of v15 after OFFICIAL FW.
