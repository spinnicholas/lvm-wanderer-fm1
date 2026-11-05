# Wanderer 1.7 RC3: brighter menus and six color themes

CONTRAST is the default: black backgrounds, nearly black cards, white text and bright neutral labels. DARK retains the previous blue-black appearance with substantially brighter secondary text. LIGHT keeps pale panels and dark text. AMBER, OCEAN and VIOLET offer warm gold, deep blue/cyan and muted purple backgrounds with bright labels. Text and symbols convey state without depending on color.

Hold SELECT to open settings. Turn PRESET to ACCESSIBILITY. Knob 1 selects THEME from six named choices; this is a multi-value control rather than a light/dark switch. Save normally to retain the choice. A fresh device or an imported old dark preference starts in CONTRAST. An existing saved light preference remains LIGHT. Your saved new theme is restored on later boots. Theme preference is global: project loads do not override it.

The palettes apply to main/Zoom screens, accessibility and effects menus, patch editors, scope, confirmations and notices. The supplied splash artwork stays intact. The palette lookup is outside audio rendering; this change adds no audio processing, buffers or effect work. RC2's input/CPU improvements, optional recording click and grouped effects remain.

## Validation

All 72 software build/execution checks pass, plus the screenshot-pixel/contrast audit. The exact packaged firmware completed a 1,500 ms emulator boot/menu run; the captured accessibility menu shows CONTRAST selected and was visually inspected.

All six themes were rendered across every settings page, every row, normal and Zoom modes, the patch editor pages, home, scope and representative messages. The host renderer checks text bounds. The actual RGB565 screenshot pixels are checked against palette text/background colors, so this validates palette application as well as the color definitions. Text/secondary-label contrast on the solid panel backgrounds ranges from 9.31:1 minimum in LIGHT to 16.82:1 in CONTRAST. These are palette contrast measurements; antialiased glyph edges naturally blend with their backgrounds. Symbols/accent colors exceed 6:1 in every palette. The six normal-size accessibility captures and the Contrast Zoom capture were visually inspected.

The new preference record remains 16 bytes and uses version 2 to distinguish six theme values from the old light/dark flag. Version 1 records migrate. Invalid theme IDs are rejected. Project format remains v11, sound data is unchanged, and dry/all-seven-effects four-note output hashes match RC2 for all 150 presets. Saving v2 preferences then rolling back to RC2 resets global preferences to that firmware's defaults because it does not recognize the new preference version; project records are unaffected by this theme change.

The final image is 538,068 of 581,564 bytes; ordinary RAM 72,864 of 98,304, pool 343,040 of 344,064. The protected platform/recovery manifest and baseline loader/header checks pass. The package remains FM-1_929, 612,196 bytes. SHA-256: c6df3cae9d57c1956c642e789c56b748a2c1924b7dae96d0e30bb0da4047c783.

Install LVM-Wanderer-1.7-RC3-FM1.fwsc using the usual updater. This remains a candidate for on-device testing, including confirmation that RC2's responsiveness changes resolve the earlier lag/distortion. It has not been published as a stable release.
