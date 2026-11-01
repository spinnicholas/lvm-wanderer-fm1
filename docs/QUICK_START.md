# Wanderer 0.9 quick start

Work in progress. Read [STATUS.md](STATUS.md) and [installation notes](RELEASE_0.9.md) first.

## Five-minute tour

PLAY starts a 16-step loop at 112 BPM. REC enables quantized note capture; play a few keys and press REC again to finish. SAVE catches the phrase as the Anchor. Turn KNOB 1, 2, and 3 for MELT, DRIFT, and DECAY. EDIT selects the layer; REC and play keys to overdub it. GLO bakes a new branch, preserving the Anchor. LFO plays the original, then returns to the living memory. HOME clears a dialog or restores orientation without clearing the phrase or stopping playback.

| Physical control | Wanderer action |
|---|---|
| PLAY | Start or stop the loop |
| REC | Toggle quantized capture into memory or the selected layer |
| SAVE | Catch the first working phrase as the protected Anchor |
| EDIT | Toggle layer recording target; the layer keeps playing when off |
| GLO | Commit an evolved branch, keeping the Anchor |
| LFO | Revisit original Anchor; press again for living memory |
| HOME | Return to living Home, cancel a prompt or close settings; leave music playing |
| ENV | Undo one recent capture, Catch, layer erase, branch, load, or new session |
| FX, then SEL | Clear the layer after confirmation |
| Hold FX for 1.6 seconds, then SEL | Start a new session after confirmation; HOME cancels |
| SEQ, then SEL | Save slot 1 while stopped |
| ARP, then SEL | Load slot 1 while stopped |
| SEL | Open settings; confirm a prompt; toggle the selected setting while inside settings |
| SELECT encoder | Move among settings; outside settings, jump between ten sound banks. Zoom remains in settings |
| PRESETS encoder | Browse sounds 001–100; in settings, switch between system and effects pages |
| ALGORITHM encoder | Tempo, 60–180 BPM; in effects, delay division or reverb size |
| KNOB 1–3 | Melt, Drift, Decay (0–127) |
| KNOB 4 | Master level; in effects, amount for the selected effect |
| OCT− / OCT+ | Keyboard octave |

In settings, turn SELECT to choose a row and SEL to switch it. PRESETS moves between the system page (light palette, Zoom, reduced motion) and the effects page (Chorus, Delay, Reverb). On the effects page, KNOB 4 sets the selected amount. ALGORITHM sets delay division or reverb size; KNOB 3 sets delay feedback or reverb damping. The temporarily displayed number shows that secondary value. HOME closes settings. PLAY and keyboard performance continue while settings is open. Settings and sound choice are included when you explicitly save the slot while stopped; toggling never writes flash during performance. Older Wanderer 0.1–0.7 slots are migrated on Load with the new effect defaults.

Every important action has a short audible cue and a distinct visual symbol. These cues do not speak note names or values. The screen remains important for complete information; this is not yet a fully screen-independent instrument. Each phrase orbit shows sixteen positions; bright nodes are occupied steps and the larger open node is the play position. Colour is supplemented by separate symbol shapes and fixed positions. The layer shows its sixteen steps on a horizontal rail.

Saving is explicit and refused during playback or recording because flash writes are not qualified under audio load. Slot 1 uses a new LVM magic, version and the platform's CRC storage. It does not import old SLOOP projects. A saved slot is only loaded after confirmation.

