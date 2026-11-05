# Wanderer 1.4 RC3 adaptive Zoom

Zoom now measures the current label, value and optional units against reserved screen rectangles, chooses the largest supported pre-rendered font, and wraps values at word boundaries when a larger two-line layout fits. Short values use more of the screen; long values retain all their text. Header and knob positions stay fixed. Normal parameter pages and Home remain as in RC2. Print Zoom state/hint lettering also uses measured fitting.

Three larger full-alphabet Atkinson Hyperlegible fonts are generated at build time. Four cached fit results add 256 bytes of target RAM; unchanged screens reuse the measurements across LCD strips and redraws. There is no allocation or fitting in audio callbacks. Drawing skips text and glyph runs outside the current LCD strip. More visible lettering still costs rendering time; physical worst-case latency is not measured by these tests.

Candidate SHA-256: `c67318dd6f47382130970bd14db299cde2fc1878957ca948005bda2689cf94cf`. Package: 612,196 bytes. Identity: FM-1_929.

Fresh verification: 51/51 sanitizer-backed regression/build/navigation/loader checks pass, including 1,444 fitting cases spanning settings labels, all arp patterns/rates, numeric ranges, cache hits, units and every editor engine/page/control extreme. 296 production captures complete without text bounds errors. Six representative Zoom screens were visually inspected, and 150 normal/Home captures are pixel-identical to RC2. Protected source, package-head and loader integrity checks pass. Boot migration bound remains 2,964 / 4,096 bytes.

Target image: 453,376 / 581,564 bytes. Ordinary RAM: 63,856 / 98,304 bytes. Audio pool: 315,392 / 344,064 bytes. Relative to RC2, the image adds 55,104 bytes and ordinary RAM adds 256 bytes; the audio pool stays the same.

The exact package completed 4,800 ms in fm1-emu with held notes, arpeggiator playback, Zoom enabled and pattern changes. Compiled screenshots of CHORD and DOWN REPEAT were inspected. Emulator observation: 13.3% in an ISR, 0.3% nested (32.5 M ISR instructions per emulated s). This is an average simulated ISR observation, not a physical worst-case deadline result. Known unsupported MMIO bring-up accesses remain reported by the emulator.

Synthesis, Scatter, arpeggiator and project format retain RC2 behavior. The long DSP stress run and before/after WAVs were produced for RC1 and are included as historical evidence; they were not repeated for this UI change. Physical FM-1 installation, listening and worst-case input/audio timing still require testing. Use lower starting master volume and spare project slots for rollback to RC4.
