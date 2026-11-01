# Wanderer 0.3: the symbol screen

The screen is a 240 × 240 RGB565 display rendered in four 240 × 60 bands. It contains one word, **WANDERER**. The top row shows transport with a triangle, square, or solid circle. Its sixteen short pulse bars reflect the working memory. Below, the literal anchor is the protected original phrase, paired arrows are the working memory, and stacked sheets are the layer. The phrase diagrams have sixteen positions, every fourth position slightly larger, with connected pitch nodes for occupied steps. A vertical marker follows the playing step. The selected Anchor or Memory mode has a bright left rail. The Layer switch is filled and shifted right when enabled; it is gray and shifted left when disabled. A palette change affects color, while shape and position remain the same.

Turning a parameter briefly changes the top row: droplet = Melt, paired drifting arrows = Drift, hourglass = Decay, metronome = tempo, speaker = level, and nested sizes = large text. The numeric value and a horizontal meter are shown beside its symbol. The footer normally shows the Anchor-safe or branch state and whether the original or working memory is heard. During a notice it shows a context symbol with a check for success or X for refusal. During a confirmation it shows the requested action, then X and check; HOME cancels and SEL confirms. Auditory cues and the physical controls are unchanged.

## Symbol key

| Symbol | Meaning |
|---|---|
| Anchor | Captured original phrase, protected from changes |
| Paired arrows | Living working memory |
| Stacked sheets | Layer; the adjacent switch shows its enable state |
| Triangle / square / circle | Play / stop / record |
| Droplet / paired drifting arrows / hourglass | Melt / Drift / Decay |
| Metronome / speaker / nested sizes | Tempo / level / text size |
| Down arrow into tray / up arrow from tray | Save / load |
| Curved arrow / fork / trash / circled plus | Undo / branch / erase / new session |
| Check / X / shield check | Success or confirm / refusal or cancel / protected |

## Design and assets

Research considered Apple's glanceable small-screen guidance, Google's bold and geometric 24-pixel icon keylines, and Lucide's consistent open icon family. Lucide's ISC license allows reuse, but none of its artwork is embedded. The 21 original symbols in `tools/gen_wanderer_icons.py` were redrawn as one coherent 24-pixel family with a common stroke, rendered at 4× then reduced to 4-bit alpha. They cost 6,048 packed bytes in the generated header. The Atkinson Hyperlegible font is now generated only for the name and digits, reducing the two glyph data tables from 15,286 to 6,240 bytes. The icon and font generators are part of both the host and target build.

References:

- Apple, Designing for watchOS: https://developer.apple.com/design/human-interface-guidelines/designing-for-watchos/
- Material Design, System icons: https://m2.material.io/design/iconography/system-icons.html
- Lucide icons and ISC license: https://lucide.dev/ and https://lucide.dev/license
- Lucide accessibility guidance: https://github.com/lucide-icons/lucide/blob/main/docs/how-to/accessibility.md

## Verification and limits

Actual host framebuffer captures include stopped and playing states, the layer switch in both positions, parameter extremes, confirmations, original mode, and light theme. Host text-bound checks and the event-memory sanitizer test pass. The symbols are deliberately explained in this companion key because Melt, Drift, and Decay are concepts without universally understood icons. The screen does not speak values. The 0.3 source was not cross-compiled or physically tested because the pinned JieLi toolchain and AC79 SDK were unavailable in this session. The earlier 0.1 firmware binary does not contain this UI.
