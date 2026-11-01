> Historical build/design notes. Current 1.2 behavior and verification are documented in USER_GUIDE_1.2.md and TEST_REPORT_1.2.md. User hardware reports cover 0.9, not the 1.2 candidate.

# Source review and architecture decision

This is an original instrument with a quiet protected source and a changing musical copy. The practical first implementation captures **note and velocity events at 16 time positions**, not audio. An Anchor is set once for a session. Changes to the working memory are non-destructive until a deliberate branch commits them; even then the Anchor stays intact. A layer offers a second simultaneous event phrase. The three controls have different roles: Melt blends a second harmonic into the synth voice, Drift changes the pitch of selected events by one or two semitones, and Decay reduces their velocity over repeat cycles with a floor. The branch captures current note and velocity differences and the base Melt amount.

## Source-level review, 2026-10-25

| Source | Inspected code and useful finding | Decision |
|---|---|---|
| Existing LVM Access 0.6.1 at SLOOP d893642 | The recovered app builds at 580,452 of 581,564 bytes. Its renderer, input handling, storage tests and 0.6.1 reliability work were examined; the inherited groovebox is too full for a new instrument. | Preserve it as a separate recovery branch; do not silently graft changes onto it. |
| Dinghy 0.4, zvenson | `firmware/src/platform.c`, `store.h`, `gfx.c`, `dinghy.c`, host simulation and build checks. The unchanged FM-1 app uses 56,320 bytes; the platform provides update/rescue, input, display, audio, MIDI plumbing, CRC slots and hard memory checks. | Use as GPL-3.0-only hardware foundation, but replace the musical app. Current Dinghy 0.4 was host tested by its author; its 0.3 hardware trial does not validate this fork. |
| Felucca / felucca-bsp | Hardware, engines, loader and GPL notices in the inherited platform and Felucca source. | Keep credited platform and update safety. Do not copy its synthesis UI or preset bank. |
| Tonitor f51bd68 | `tools/gen_font.py` produces run-length 4-bit alpha glyphs from Atkinson Hyperlegible; `gfx.c` blends them on an LCD band. Tonitor's own app has a different ambient workflow. | Adapt its offline font generator and keep the font plus OFL notice; implement a separate graphics language of labelled Anchor, Memory and Layer contours. |
| Ravel, De-Pattern | `firmware/app/rv_tape.c` allocates a shared audio tape and captures sound into Beads-compatible formats, with host scenario tests and an independent platform split. Its reported app is around 200 KB with a 120 KB tape. | The tape is technically plausible in a later phase. Copy no tape or Beads code now; a protected original plus a mutable copy needs explicit memory and persistence measurements first. |
| Rainbow Mode 0baf362 | `ui_say()` in `firmware/src/ui.c` sets a timed on-screen message; the `SMK37` references are keyboard support. | Do not mistake display text for onboard speech. |
| Speak37 81fa061 | Its fixed-point LPC speech and encoded vocabulary are real, but the FM-1 word library is reported as thousands of words and hundreds of KB. | Speech remains separate until audio cost, storage and intelligibility are measured. |
| FM1 Open, M-VAVE | The GPL app uses an SDK linking exception and a separately licensed binary engine. | Do not import its binary engine or infer GPL compatibility from the application label alone. |
| Quest, hericdk | SLOOP-based graphical RPG vocabulary with the groovebox still present. | Learn that a distinctive visual metaphor can carry musical state without copying its assets or narrative. |
| FM1 emulator | Executes the compiled .fwsc and captures the panel and audio, but has unknown MMIO and no physical timing guarantee. | Use for execution evidence; actual framebuffer host captures are the main screenshot source. |

Source repositories: https://github.com/zvenson/dinghy ; https://github.com/hugelton/Felucca ; https://github.com/nzimas/tonitor ; https://github.com/De-Pattern/Ravel_FM1 ; https://github.com/joshbartlettband-web/felucca-rainbow ; https://github.com/m-vave/fm1_open ; https://github.com/hericdk/fm1-quest .

## Other architecture options

Keeping the SLOOP fork would retain a mature four-track music system and old project compatibility, but 1,112 bytes remained in its app slot. Removing inherited sound engines and features to fit the new instrument would be a high-risk refactor. Rewriting from Felucca directly could work but repeats the recent Dinghy platform extraction and host harness. The selected architecture is a distinct GPL application on Dinghy, with the previous SLOOP source recoverable independently. This first result is not project-format compatible with 0.6.1.

## Memory and audio representation

Four 16-event phrases (Anchor, working memory, layer, last branch), a one-step Undo and the save record use only a few hundred bytes. The platform slot limit is 16 KB; the saved record is 152 bytes. The anchor is never modified by Melt, Drift, Decay or Branch. It can be cleared by held FX → new-session symbol → SEL or restored to its pre-Catch state by Undo; an explicitly confirmed Load can replace the whole session. HOME never clears it.

The 0.1 DSP was a four-voice sine plus second harmonic blend. In 0.8, the voice uses band-limited wavetables, a two-stage filter and a separate chorus, tempo delay and feedback room. Notes are still scheduled in the audio callback at 256-frame boundaries; the main loop captures key events into a quantized phrase. A future audio memory design must allocate separate original and living buffers, account for flash persistence and measured CPU load, and preserve safe return to the source. See `WANDERER_SOUND_0.8.md` for the current engine.

## Typography and screen

Atkinson Hyperlegible Bold is rendered offline to 34 px and 23 px cropped capital glyphs with 4-bit alpha runs. The source font and OFL notice are in `assets/`. A 240×60 RGB565 band is reused four times, avoiding a full framebuffer. Dark and light palettes preserve contrast, and each state has a word and geometry in addition to colour. A width assertion in the host renderer rejects clipped strings. Reduced motion holds the graphics steady until a control or state change.

The 0.8 release package identifies as `FM-1_928`; the development build identifies as `FM-1_920`. The FM-1 Firmware Hub's live identity list did not show `FM-1_928` on 2026-10-31. The identity is provisional for this private experimental build and should be rechecked before public distribution. No physical FM-1 was flashed.
