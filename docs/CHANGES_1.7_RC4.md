# Wanderer 1.7 RC4: Scatter size up to 512 ms

Scatter Size now offers Auto, 4, 8, 16, 32, 64, 128, 256 and 512 ms. Tap EDIT, turn PRESET left twice from AMP to SCATTER, and use knob 2 (SIZE). Existing indices 0–6 keep their meanings; 256 and 512 ms are appended. The saved state layout and project version remain unchanged. Older firmware will reject projects saved with the two new size indices.

Manual windows retain the existing ±25% per-gesture variation; 512 ms is the nominal control value and needs up to 640 ms of source history. The live history ring therefore grows to 10,240 samples at 16 kHz. It moves to ordinary RAM, and the unchanged 4 KiB Print decode cache moves to the freed audio pool. The three Print/Anchor/Undo arenas keep their full capacity. Existing sizes retain the previous hop/jump lookback. Every ring read/write wraps at the expanded capacity; only grain-phase arithmetic retains the 4,095 fractional mask. Scatter stays before effects.

The build fits: flash image 537,940 / 581,564 bytes; ordinary RAM 89,248 / 98,304; pool 338,944 / 344,064. The loader and package header match the accepted baseline and all 33 protected platform/recovery source hashes are unchanged. The boot stack audit passes.

The dedicated sanitizer check passes 360 new-size gesture/offset/wrap cases, complete fractional/reverse-window reads, history warmup and rollover, save/load, editor endpoints and all six themes in normal/Zoom. The displayed 512 ms control was visually checked. The broader release suite covers all sizes in timing, Scatter, Print/Undo/Redo and Anchor tests.

Package: LVM-Wanderer-1.7-RC4-FM1.fwsc. Identity FM-1_929; 612,196 bytes; SHA-256 edb95653f2b1c122f0e22f6f2c66faf706d80c85ee3cb126f2f7ce80799da51f.

Install using the usual FM-1 updater. This remains a candidate for physical acceptance; the range and memory tests are software checks. No other requested controls, theme choices or sound-engine settings are changed by this revision.
