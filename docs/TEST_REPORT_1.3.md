# Wanderer 1.3 RC4 verification

On the SCATTER editor page, knobs 1–3 control Style, Size and Amount. Tap EDIT, then turn PRESET left once from AMP to reach it. Knob 4 is unused on this three-control page. Normal/light themes and Zoom retain the existing Atkinson Hyperlegible style.

Size offers Auto, 4, 8, 16, 32, 64 and 128 ms. Auto retains tempo-derived grain windows. Manual windows vary by up to ±25% per gesture, and Rush can halve them again. Size also controls hop/jump windows and dropout gaps. Bounds, short fades and protected history remain in place.

Scatter already preceded the effects; that routing is retained and now regression-tested through the actual app callback. Signal path: synth/playback → Scatter → chorus/delay/reverb → Print/output. Printed-audio Melt sits between Scatter and effects. Chorus, delay and reverb retain tails when Scatter stops feeding them. Earlier effects already baked into a printed generation remain part of that source.

Size is stored and undone with the project. New saves use v7. Existing v6 projects load at Auto without rewriting their source slot; old notes, patch and style are preserved. Legacy v1–v5 imports retain their sound bank and Decay. Use a spare slot for new saves if retaining rollback compatibility.

All 47 regression compile/execution/protocol checks pass, including 48 callback-size runs at Auto, 8, 32 and 128 ms, save/load and v6 migration, all styles/sizes, note and audio Anchor integrity, Print/Undo/Redo, effects tails and Stop LED cleanup. 2,520 editor range/theme/Zoom renders pass collision checks. Native Scatter screenshots in both themes and Zoom were visually inspected. Both splash themes still match the supplied image pixel for pixel. No new icons or audio pool buffers were added.

The tested RC2 boot fix is retained: target scanner frame 20 bytes, conservative migration stack chain 2,916 bytes. All 33 protected platform/recovery files, package head and compressed loader remain unchanged. Identity FM-1_929.

The exact RC4 package completed a 6,500 ms compiled emulator run with four FM voices, manual 128 ms Scatter, Print, Undo/Redo and Stop. Average emulated ISR load was 16.5%; this does not establish a physical worst-case deadline. This candidate awaits the user's manual FM-1 test.

Application: 394,864 / 581,564 bytes. Ordinary RAM: 59,936 / 98,304. Audio pool: 315,392 / 344,064. Package: 612,196 bytes.

SHA-256: `91633c4d0e3de94fd15722fce062504243c0d623437317be96434165c3b116ce`.

The 24-second production-DSP audition contains six four-second segments: clean, 4 ms, 16 ms, 64 ms, 128 ms, then 128 ms with delay/reverb. Printed audio remains RAM-only.
