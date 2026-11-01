# Wanderer 1.1 RC2: complete UI review and symbol key

All 55 RC1 production-renderer screenshots were inspected before this refinement. The revised versions were captured from the same production renderer and inspected again at native 240×240 proportions and enlarged views. These are executable UI captures, not appearance mockups.

## Findings and design decisions

The old anchor lacked pointed flukes and had a blunt bowl. Memory resembled a crescent with stars. Save/Load looked like clipboards, root resembled a trident, loop length resembled a battery, and several effect symbols looked improvised. Inconsistent internal gaps made some outlines look melted together. Small percentages sat on a different line from their values, faint interval circles became visual noise, and a permanent Play badge on a stopped phrase blurred listening status with running transport.

The revision uses actual licensed SVG geometry rather than invented decorative metaphors. Thirty-eight icons come from Lucide, four from Tabler. All share a 24-unit canvas, rounded caps/joins and a consistent adapted 2.25-unit stroke. Original SVGs, copyright notices, source URLs and hashes are included. Display masks are made at 32, 48, 64 and 96 pixels. The source projects are references and asset authors, not endorsers of Wanderer.

Anchor is now a proper nautical anchor; Memory is repeat arrows; Layer is stacked planes; Save is a floppy disk; Load is an open folder; root is musical notes; scale is a keyboard; and length is paired measures. Headphones marks the monitored phrase on normal Home while the header alone states running/stopped transport. Record dots retain their lane role. The closed lock marks the protected Anchor and the plus badge marks an uncaught Anchor.

Page focus uses a quiet tab background with its underline. Row focus remains a thick high-contrast frame. Normal settings align percentage signs with values; full-width Performance rows use their available right-hand space. Clear rectangular interval cells replace tiny rounded cells. Loop length uses two groups of four beat marks, filled or hollow for one/two bars. Row location uses three short bars instead of sparkly dots. Directional confirmation arrows are thicker. The gain multiplier sits nearer its scope value. Existing lane colors, position cues, Zoom and large type are retained.

A few concepts still require learning: no familiar symbol fully explains the unique Melt/Drift/Decay behavior. Their symbol and physical-control mapping is documented below. An optional text-label mode could reduce that learning further, but this revision preserves the established on-device word constraint. Source icons improve recognition; they cannot guarantee universal interpretation.

## Research references

Lucide's icon selection guide recommends familiar meanings, consistent visual language and avoiding redundant decorative icons: https://lucide.dev/how-to/icon-guide . Its design language describes consistent canvas, stroke, spacing and visual weight: https://lucide.dev/contribute/icons/design-principles . This firmware adapts stroke width for its display; it does not claim to meet Lucide's contribution specification unchanged. Lucide's license and inherited Feather notice are at https://lucide.dev/license . Tabler sources and its MIT license are at https://github.com/tabler/tabler-icons and https://tabler.io/license . The exact vendored originals and SHA-256 values are in assets/icons/sources.json.

## Every screen reviewed

|Screen|Review and refinement|
|---|---|
|01-home-dark|Uncaught Anchor uses a plus badge; proper anchor, repeat Memory and headphones distinguish source, loop and monitoring.|
|02-home-phrase-layer|Closed padlock, repeat arrows and stacked layers replace the ambiguous source shapes.|
|03-home-playing|Playback stays in the transport header; headphones identifies the listening lane independently.|
|04-home-recording|Record dots retain their original roles, distinct from headphones and the selected frame.|
|05-anchor-listening|Anchor monitoring is indicated by headphones in its card, without a false running triangle.|
|06-anchor-zoom|Zoom gives the sourced anchor enough room for its ring, arms and flukes to stay distinct.|
|07-memory-zoom|Zoomed Memory uses a recognized repeat symbol instead of the crescent/dots drawing.|
|08-home-light|The same new symbols keep their roles and contrast in the light theme.|
|09-zoom-light|Light Zoom retains its large single phrase and strong focus frame.|
|10-settings-system|System uses contrast, magnifier and paused-motion symbols; page focus is a subtle tab.|
|11-settings-effects|Waveform, diminishing echoes and room cube have consistent strokes; amounts and percentages share a baseline.|
|12-settings-performance|Speaker, bell and floppy disk distinguish level, earcons and storage; values align to the right.|
|13-delay-division|Delay timing keeps its musical fraction and clock; row location uses three clear bars.|
|14-reverb-size|Room size uses outward arrows while a smaller cube preserves effect context.|
|15-cues-disabled|Earcon off is shown by switch position, rather than a different or decorative bell.|
|16-settings-light|The light System page shares the same hierarchy and geometric on/off states.|
|17-confirm-clear-layer|Stacked layers and a conventional trash can make the destructive target explicit.|
|18-confirm-save-slot8|A familiar floppy disk replaces the clipboard-like Save symbol.|
|19-confirm-load-slot8|An open folder distinguishes Load from Save.|
|20-confirm-new-session|Anchor, Memory and Layer retain their identities above the New Session trash target.|
|21-confirm-restore-anchor|A thicker directional arrow connects Anchor to Memory during Restore.|
|22-melt|A plain droplet replaces the embellished Melt shape.|
|23-drift|A conventional shuffle/variation symbol replaces the improvised Drift arrows.|
|24-decay|A clean hourglass keeps Decay distinct from filter damping.|
|25-tempo|The actual Tabler metronome symbol replaces the improvised tempo drawing.|
|26-master|Master uses a conventional speaker with volume waves.|
|27-preset100|A musical list identifies preset selection, distinct from the oscilloscope waveform.|
|28-saved|Save success keeps the slot number and check with the proper storage symbol.|
|29-stop-first|Stop-first uses familiar transport shapes connected by a legible arrow.|
|30-invalid-save|An empty-file symbol replaces the generic blank rectangle.|
|31-settings-zoom|Zoom retains its large magnifier and unambiguous position switch.|
|32-feedback-zero-zoom|Feedback uses a circular return arrow, distinct from the rectangular repeat Memory symbol.|
|33-damping-zoom|Filter damping uses a filter/down symbol; room context remains visible.|
|34-delay-timing-zoom|Zoomed timing retains its large fraction and consistent row locator.|
|35-save-normal|Normal Save confirmation uses the new floppy disk, unchanged slot and actions.|
|36-save-zoom|Zoom Save retains its number size while sharing the sourced storage symbol.|
|37-scope-live|The live scope uses the standard monitor/waveform context symbol and a quieter gain multiplier.|
|38-scope-held|Freeze remains conventional pause bars, separate from transport status.|
|39-scope-light|The light scope retains the same waveform and high-contrast grid.|
|40-scope-silence|Silence stays a real flat trace with no decorative waveform.|
|41-write-failure|Write failure uses a file with an X instead of the irregular broken-page shape.|
|42-empty-slot|An empty slot retains a file/minus shape, distinct from write failure.|
|43-stop-first-persistent|Persistent Stop-first shares the same thicker connection and familiar transport shapes.|
|44-splash-dark|The approved dark splash is retained pixel-for-pixel.|
|45-splash-light|The approved light splash is retained pixel-for-pixel.|
|46-two-bar-held-notes|Two-bar durations and playhead marks are retained with the new lane identities.|
|47-two-bar-zoom|Zoomed two-bar timelines remain legible beneath the repeat Memory symbol.|
|48-music-root-fsharp|Musical notes identify root pitch, replacing the trident-like tuning fork.|
|49-music-minor|A keyboard identifies scale; filled/hollow interval cells replace faint little circles.|
|50-music-free|Free keeps all twelve interval cells filled, alongside the existing scale number.|
|51-music-length32|Paired measure frames and two groups of four beats clarify a two-bar length.|
|52-music-length16|One filled group and one hollow group clarify sixteen steps without deleting hidden events.|
|53-music-root-zoom-light|Root pitch stays large and readable in light Zoom, using the musical note symbol.|
|54-music-dorian-light|Dorian retains its exact interval pattern in the clearer light-theme cells.|
|55-music-length-light|The light loop-length page shares the same one/two-bar geometry and large number.|

## Symbol meanings and controls

|Meaning|New symbol|Control or interpretation|
|---|---|---|
|Anchor|Nautical anchor|SAVE catches the first protected phrase; LFO listens to it.|
|Memory|Repeat arrows|The evolving recorded phrase; HOME returns to it.|
|Layer|Stacked planes|EDIT enables the second lane and selects it for recording.|
|Listening|Headphones|The phrase currently being monitored on normal Home; independent of running transport.|
|Recording|Record dot|The lane receiving recorded notes; REC toggles capture.|
|Protected|Closed padlock|Anchor exists and cannot be overwritten by Catch.|
|Catch available|Plus in a circle|Anchor has not yet been caught.|
|Melt|Droplet|Parameter knob 1 smoothly opens tone.|
|Drift|Shuffle arrows|Parameter knob 2 increases scale-aware pitch variation.|
|Decay|Hourglass|Parameter knob 3 controls velocity aging across cycles.|
|Tempo|Metronome|ALGORITHM on Home sets BPM.|
|Sound bank|Musical list|PRESETS browses sounds; SELECT jumps by ten.|
|Master|Speaker and waves|Parameter knob 4 on Home or the Performance level row.|
|Chorus|Layered waveform|Effects row 1.|
|Delay|Diminishing vertical echoes|Effects row 2; its clock detail sets musical timing.|
|Room/reverb|Room cube|Effects row 3.|
|Feedback|Circular return arrow|Delay knob 3.|
|Room size|Outward arrows|Room ALGORITHM.|
|Damping|Filter/down|Room knob 3.|
|Earcons|Bell|Performance bell row; SEL/SCL toggles.|
|Save project|Floppy disk|SEQ, SELECT slot, SEL/SCL confirm while stopped.|
|Load project|Open folder|ARP, SELECT slot, SEL/SCL confirm while stopped.|
|Theme|Contrast circle|System row 1.|
|Zoom|Magnifier|System row 2.|
|Reduced motion|Pause in a circle|System row 3; does not stop music.|
|Root|Musical notes|Music row 1; parameter knob 4 changes root.|
|Scale|Keyboard|Music row 2; parameter knob 4 changes the scale number.|
|Loop length|Paired measure frames|Music row 3; choose 16/32 steps while stopped.|
|Waveform monitor|Waveform in a frame|HOME + SEL/SCL opens/closes it.|
|Trace freeze|Pause bars|SEL/SCL inside the scope; sound continues.|
|Empty project|File with minus|No valid project in the selected slot.|
|Write problem|File with X|Save or preferences write/readback failed.|

## Validation and limits

The musical/control/recording/project/LED/scope/splash source files are byte-identical to RC1. The same production audio scenario produces 387072 identical PCM bytes. The fixed project format and performance mappings remain unchanged. The approved splash pixels are unchanged in both themes.

The checks cover 42 distinct sourced icons and all 168 size masks, including nonempty masks, clear padding, full opacity and source/license provenance. Existing 300-state layout checks, 1152 Music states and 2048 full-range value-row renders pass bounds under AddressSanitizer and UndefinedBehaviorSanitizer. Timed navigation, LED scene cleanup, scope/cues, first input, project/recording and protected recovery checks pass. The complete release retains candidate status for physical display and hardware acceptance.
