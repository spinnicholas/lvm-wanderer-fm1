# Wanderer 1.4 RC4 accessibility labels and global Zoom shortcut

The former System page is now titled ACCESSIBILITY. THEME shows DARK or LIGHT; ZOOM shows OFF or ON; MOTION shows FULL or REDUCED. Normal view uses its spare space for SCREEN COLORS, LARGER TEXT or LESS MOVEMENT. Both normal and Zoom views show ZOOM HOME+EDIT as the shortcut reminder. Parameter positions and knob assignments stay the same.

Press HOME and EDIT together, then release both, to toggle Zoom. Either press order works. It toggles once until the combination has been fully released, suppressing the solo HOME and EDIT actions. The page, parameter selection, dialog, Layer, notes and performance state stay in place. The existing HOME + SELECT scope shortcut takes precedence when already held. Display settings change without a flash write and are saved with the existing project/preferences workflow. HOME and EDIT still have their normal solo/hold actions when pressed separately; do not hold either by itself before adding the other. The oscilloscope keeps its waveform layout while the Zoom setting changes for applicable views.

Candidate SHA-256: `1565edadd36a33296de62ab24de717f36c20a78195fdece8c4df4852f25d425e`. Package: 612,196 bytes. Identity: FM-1_929.

Fresh verification: 53/53 sanitizer-backed regression/build/navigation/loader checks pass. The new shortcut test exercises 48 context/state/press-order combinations, extended holds, partial releases/represses, exact state preservation during recording and layering, both Scope conflict orders, solo-key behavior and an unrelated pending REC action. Adaptive fitting's 1,444 cases still pass. 296 production screenshots pass screen bounds checks; six Accessibility normal/Zoom views were visually inspected.

The exact package completed 2,700 ms in fm1-emu with recording, held notes, a Scatter selection, both shortcut press orders, and the Accessibility page. Compiled screenshots confirm normal Scatter, Zoom Scatter, normal Scatter again and Zoom Accessibility without a page jump. The emulator reports 12.4% in an ISR, 0.3% nested (30.3 M ISR instructions per emulated s). This average simulation does not establish physical worst-case latency. Known unsupported MMIO bring-up accesses are still reported.

Target image: 454,240 / 581,564 bytes. Ordinary RAM: 63,856 / 98,304 bytes. Audio pool: 315,392 / 344,064 bytes. Protected source/package-head/loader checks pass; the conservative boot migration bound stays 2,964 / 4,096 bytes.

Synthesis, Scatter, arpeggiator and project format retain RC3 behavior. Physical FM-1 installation, listening, button feel and timing still require testing. Audio comparisons in the package were generated for RC1 and remain historical listening examples. Start with lower master volume and save v8 projects into spare slots if retaining rollback to RC4 from version 1.3.
