# Refinements in 1.2 RC2

HOME now releases the previous sequenced gates when returning from Anchor to Memory, matching LFO's listening transition. Natural release/effect tails can fade; independent live keys remain held.

Local key onsets detected in one scan use a shared nearest-step decision, so the audio scheduler cannot split that batch while the keys are processed. MIDI messages and onsets detected in separate scans retain their own arrival timing.

Full recorded chord cells prefer released positions. If every position is held, the oldest captured onset yields; retriggering refreshes its age. Capture-pool age comparison also handles unsigned counter wrap correctly. The four-voice budget, v5 project layout, synth, icon system and recovery foundation are retained.

RC1 is preserved separately, including its original binary and source. RC1 and RC2 can both read v5 projects. Older releases still require their earlier-format project slots.

# Wanderer 1.2 RC2 changes

Record up to four notes per step per lane with independent gates and source-aware MIDI sustain. Anchor, branch, scale evolution, Undo and project storage preserve every chord event. Held overdubs clear covered repetitions of the same pitch while preserving other pitches. The four-voice synth budget is unchanged; simultaneous Memory/Layer chord starts share it explicitly.

Home shows untitled/edited/saved project status, the assigned slot, digital mute and 16/32-step length. Slot confirmations show cached Wanderer validity before an explicit write. New status symbols use original licensed Lucide SVG geometry with the existing rasterization system. Layer cards and their timelines are separated from the status strip in both display sizes.

Hold SAVE 0.9 seconds for project Save; a short release retains Catch and SEQ remains Save. Success outcomes stay visible. Persistent outcomes/errors have explicit acknowledgment LED scenes and clear hidden Home lamps. A project commit followed by a preferences failure receives a distinct two-row result. Undo tracks the correct saved reference, and an empty REC pass preserves Undo. PLAY works in confirmations. Listening changes finish recording and release previous sequenced gates. Failed Anchor listening preserves recording. Dialogs, settings and recording-lane changes end recording predictably.

Version-five storage reads historical versions one through four without rewriting on Load. New v5 projects require 1.2; preserve old slots for older firmware. Boot only reads project validity and never writes or auto-loads a session.

The 1.1 synth/preset bank, effect engine, oscilloscope and approved splash are retained. The protected update/recovery sources, compressed loader and package head remain identical to the 0.9 foundation. Identity FM-1_932; physical FM-1 acceptance remains pending.
