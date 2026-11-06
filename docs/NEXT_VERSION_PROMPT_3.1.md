# Development prompt: LVM Wanderer 3.1

Copy everything below the line into a new request.

---

You are continuing work on **LVM Wanderer**, my accessible firmware for the M-VAVE FM-1, in
`spinnicholas/lvm-wanderer-fm1`. Start from the latest commit on
`claude/fm1-firmware-overhaul-33i8f8`, which is 3.0 RC3. Name this release **3.1 RC1** (`RELEASE`
file). It is for musicians with low vision, not blindness. Keep the Anchor/Memory idea, the
button layout and the spirit of earlier releases. This release is mainly a **visual overhaul,
an animated splash, and a fresh start on every new firmware version**.

## Facts from the current code (check them, don't re-derive them)

- **Budget at RC3** (native build of `bda56f2`):
  - App image: 571,888 of 581,564 B, so only **9,676 B free**.
  - RAM (.data + .bss): 94,752 of 98,304 B.
  - Audio pool: 342,536 of 344,064 B.
  - Any new feature has to make room first.
- **Compatibility with old projects costs about 36 KB of flash.** It consists of:
  - five older preset tables: `WS_PRESETS_1_3`, `WS_PRESETS_1_4`, `WS_PRESETS` (1.9),
    `WS_PRESETS_2`, and their name tables;
  - the 100-patch classic bank `WM_PRESETS` (`wanderer_presets.h`, `wanderer_factory_legacy.h`);
  - the legacy voice path in `app_audio_core` (the non-`modern` branch);
  - `wm_evolve`'s legacy branch;
  - migration code for project formats v3–v14 in `wanderer_memory.h` and `project_read_slot()`.
  - The current bank is `WS_PRESETS_3` / `WS_NAMES_3`.
- **Icons are the largest flash item, about 170 KB.** The atlases are `ICON_ZOOM`, `ICON`, `ICON_MID`,
  `ICON_MINI` and `ICON_TINY`, made by `tools/gen_wanderer_icons.py` from Lucide/Tabler SVGs.
  The builder needs Inkscape to regenerate them; GitHub's runner has none, so it reuses the
  checked-in `wanderer_icons.h`. If you change icons, regenerate them here and commit the header.
- **The splash** is your artwork `assets/wanderer-splash.png` (1448×1086: the blue "LVM" mark
  with a gold note, and the white "Wanderer" wordmark underneath).
  - `tools/gen_wanderer_splash.py` stores it as one 240×240 image with a 128-colour palette,
    run-length coded: about 28 KB (`SPLASH_DATA`).
  - It shows for `WANDERER_SPLASH_MS` = 1100 ms, and any key skips it (`wanderer_splash.c`).
  - Scaled to the screen, the wordmark sits at about rows 148–186 and is about 225 px wide.
- **Screen**:
  - 240×240 ST7789 on SPI at 12 MHz (`LCD_BAUD 4`).
  - Drawn in four 60-row bands. A full frame is 115,200 B, about 77 ms (roughly 13 fps).
  - Redrawing only a strip about 40 rows tall should allow about 25–30 fps.
- **Themes** (`wanderer_themes.h`): Contrast, Dark, Light, Amber, Ocean, Violet. `color_theme`
  defaults to **0 (Contrast)**, so that is what boots.
  - Versions 1.2–1.4 defaulted to the **Dark** look: navy background, a **teal anchor**, a
    **lavender memory** symbol, and a **gold stacked-squares layer symbol** (`lucide/layers-2`,
    `ICO_LAYER`) in its own Home row, with an on/off switch for Layer playback.
  - See `docs/screenshots-1.2/01-home-dark.png` and `docs/screenshots-1.4/home-large.png`.
  - 3.0 shrank the layer to a small "1 2" switch in the header.
- **Contrast test** (`tests/theme_contrast_test.py`): text and labels must be at least **7:1**
  against every background and panel colour; accents at least **3:1**.
- **Saved data on the FM-1** (`store.h`): slots at `0x97000` (8 slots × 4 sectors) and settings at
  `0xFC000`. The official v15 firmware uses this area too. RC3's Settings → SYSTEM → OFFICIAL FW
  erases exactly these sectors.
- **What old data can and can't do to the sound:**
  - At boot, `project_initialise()` reads every slot through all old formats to show which
    slots are full. A project is only fully decoded when someone loads it.
  - So old projects make boot slower and the code bigger. They are **not** a likely cause of
    glitches while playing.
  - RC2's glitches came from audio-interrupt load, which RC3 measures and limits.
  - Keep this distinction in your reasoning; don't promise that erasing data fixes audio.
- **Rules of the chip** (`docs/RULES.md`):
  - No floating point and no 64-bit division.
  - Hardware registers only in `hal/`.
  - No constants that look like ROM addresses.
  - The audio interrupt has 5.8 ms per 256 frames.
  - Erasing flash stalls everything for about 50 ms per 4 KB sector.

## 1. A fresh start for every new firmware version

When a **new firmware version** starts for the first time, erase all saved Wanderer data before
anything else reads it:

- Erase all eight project slots, including any Anchor, Memory, Layer and loops in them.
- Erase the settings sector.
- Then write the new version's identity.

Rebooting **the same version** must keep its own saves and settings, or Save would be useless.

- Identify a version with a 32-bit value made from the release string (and the source commit,
  if available at build time). Store it inside the settings record.
- A missing, invalid or different identity means a fresh start.
- Do the erase behind the splash screen (see 3), **one sector per animation frame**, so the
  animation keeps moving and the wait feels like real loading.
- Never erase anything outside `SETTINGS_AT` and the slot sectors.
- Keep Settings → SYSTEM → OFFICIAL FW for going back to v15.
- **Remove old-project compatibility entirely:**
  - the older preset tables and their names;
  - the classic 100-patch bank and legacy voice path;
  - legacy `wm_evolve`;
  - all migration and validation code for v3–v14;
  - fields only they need.
- Start a new, clean project format (v16) with its own magic number, so other firmwares and older
  Wanderer versions reject it cleanly.
- Update or delete tests that only exist to check old formats. Add tests for: the first-boot
  erase, keeping data on a same-version reboot, a changed version, a corrupt settings record, and
  power loss partway through the erase (the next boot must finish the job).
- Report how much flash this frees.

## 2. A complete visual overhaul with colour

The UI looks plain. Bring back the colour and character of 1.2–1.4 while keeping 3.0's
accessibility.

- **Default theme:** boot into a new **WANDERER** theme modelled on the 1.2–1.4 Dark look.
  - Navy background, slightly lighter panels.
  - Teal Anchor, lavender/violet Memory, warm gold Layer, white text.
  - Each element keeps its colour on every screen: the Anchor is always teal, Memory always violet,
    Layer always gold.
  - **Contrast stays a choice in Settings → Accessibility**, as do the other themes.
  - Every theme must pass `theme_contrast_test.py` (7:1 text, 3:1 accents).
- **Home:**
  - Large, colourful Anchor and Memory symbols.
  - Bring back a **Layer row** with the gold stacked-squares symbol and a clear on/off switch
    for Layer playback, as in 1.4.
  - Keep the 1/2 edit focus visible and obvious.
  - Keep the melody pictures (piano rolls), the wander distance bar and loop count, but give
    them their element's colour.
  - Use tasteful depth: the existing top-lit gradients, coloured card edges, a soft glow on the
    selected card.
  - Never let colour be the only signal: keep a word or shape for every state.
- **Every other screen gets the same visual language:**
  - Edit pages: each page group has its own accent colour (sound, envelopes, FM, Scatter, Print).
  - Settings pages, the Melt/Drift/Scatter/Tempo/Level focus screens, preset names, messages,
    confirmations, Save/Load, Scope, and the count-in.
  - Keep the knob position bars and the segmented meters, coloured to their element.
- **Normal and Zoom** both look finished. Zoom keeps its larger symbols, and text never clips;
  the host renderer already fails on clipped text.
- **Icons:** recolour freely. Only add or change icons if flash allows, and keep the atlas sizes
  in mind.
- Before the native build, render a **screen gallery** (every screen, Normal and Zoom, the
  WANDERER, Contrast and Light themes) with the host tools. Send me the gallery so I can review
  the look.

## 3. An animated splash

- Show the "LVM" mark first, exactly as in the artwork.
- Then the **eight letters of "Wanderer" slide in from the right, one after another**. Each one
  eases out and settles in its correct place. All eight have landed within about **two seconds**.
- Hold the finished logo briefly; the whole splash lasts about **2.5–3 seconds**.
- Optionally, add a subtle final touch, such as the gold note brightening once.
- **Implementation:**
  - Change `gen_wanderer_splash.py` to cut the wordmark out of the static image.
  - Store the eight letters as separate cropped masks (one colour, 4-bit alpha, run-length coded,
    like the fonts and icons) with their final positions.
  - Keep the "LVM" part as the palette image. Removing the wordmark rows from it should make the
    total smaller than today's 28 KB; measure it.
  - Redraw only the strip that contains the letters each frame, at about 25 fps if the screen
    allows, using `lcd_window` and blits within the existing band buffer.
  - Use integer easing, no allocation, and no extra frame buffer.
- **Accessibility:**
  - Any key skips the splash, as now. Skipping during a first-boot erase only skips the
    animation, not the erase.
  - With Settings → Accessibility → MOTION set to REDUCED, show the finished logo without
    movement. After a fresh-start erase the setting is back to its default, which is fine.
- The audio callback keeps running during the splash. Nothing in the animation goes into the
  audio interrupt.

## Constraints

- **Audio cost must not rise.** Re-run the Valgrind instruction-count comparison across all 150
  presets, four voices with every effect on (the method used for RC3). The worst preset must
  stay at or below RC3's 53.3M, and the load guard must stay in place.
- **Flash:** report the new image size. Most of the overhaul should be paid for by section 1.
- **Tests:**
  - All host tests that pass at RC3 must still pass, except those deleted with the old formats;
    say which ones and why.
  - The wander, metronome, engine and adversarial tests must also pass under AddressSanitizer
    and UndefinedBehaviorSanitizer.
  - `theme_contrast_test.py` must pass.
- **Native build:** push to the branch so `.github/workflows/fm1-native-build.yml` builds it. Send
  me the `.fwsc` with its SHA-256 and the size table. Don't call anything "tested on hardware";
  only I can do that.
- **Guide:** update `docs/guide-3.0.html` into a 3.1 guide covering the colours, the default theme
  and how to switch to Contrast, the Layer row, the splash (and that any key skips it), and the
  fresh start on update ("updating erases your saved projects"). Update `docs/CHANGES` as well.
- **Commits:** commit in logical steps and push to the same branch. No pull request unless I ask.

## Hardware test checklist to give me at the end

1. Install 3.1 over 3.0 RC3. You should see the splash with the letters arriving, then a blank
   project with the WANDERER theme.
2. Save to slot 1, power off and on. Slot 1 is still there.
3. Reinstall the same file. Slot 1 is still there, because it's the same version.
4. Hold any key during the splash: it skips.
5. Set MOTION to REDUCED, then reboot: the logo shows without movement.
6. Check every theme for readability in your room light.
7. Play heavy SUPER and VOICE sounds with all effects on, and watch Settings → SYSTEM.
8. Erase with Settings → SYSTEM → OFFICIAL FW, then install v15. v15 should start clean.
