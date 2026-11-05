> Author-tested on November 1, 2026: Nicholas confirms that the latest 1.2 RC2 passed installation from V15 and SLOOP and return to each on his FM-1. This hardware follow-up supersedes the development-session pending-acceptance language below. The release remains experimental.

# Wanderer 1.2 RC2

This candidate adds chord recording, clearer project status and more predictable recording transitions to the 1.1 synth and licensed icon system. It is software-tested; acceptance of these new firmware bytes on a physical FM-1 is still pending.

## Start with the new sounds

PRESETS chooses sounds 1–100. SELECT, the rotary control, jumps by ten. The separate SEL/SCL button opens settings. ALGORITHM sets tempo on Home. Parameter knobs one, two and three control Melt, Drift and Decay; knob four controls digital level. The physical MASTER remains your overall hardware output control.

Try 011 Wide Saw, 023 Reedy Lead, 032 Thick Root, 036 Acid Willow, 043 Electric Mist, 050 Moon Celeste, 051 Glass Bell, 067 Liquid Grid, 073 Slow Aurora, 077 Golden Weather and 080 Homeward Light. Hold pad notes for a couple of seconds so their attacks can develop. Plucks, keys and bells deliberately decay even if you hold a key: that is their envelope design, not a recording limitation.

The synth uses longer-range envelopes, exponential decay/release, a resonant lowpass, velocity-sensitive brightness, optional sub weight, pulse-width motion, patch-specific vibrato/filter motion and gentle drive. These are original sounds inspired by broad synthesis approaches, not sampled or exact brand replicas. Four synth voices are shared by live keys, MIDI and the two sequenced lanes.

## Choose a key, scale and loop length

Tap SEL/SCL. Turn PRESETS three page positions forward from System to Music, identified by the keyboard symbol. SELECT chooses among musical notes (root), keyboard (scale) and paired measures (loop length). The fourth parameter knob changes the selected value. SEL/SCL advances root/scale, or switches 16/32. HOME exits.

Root uses large note notation such as C or F#. Scale uses a large number and a twelve-position interval diagram. Filled positions are allowed pitch classes, counted from your selected root.

|Scale number|Meaning|
|---:|---|
|0|Free / chromatic|
|1|Major|
|2|Natural minor|
|3|Dorian|
|4|Minor pentatonic|
|5|Major pentatonic|
|6|Mixolydian|
|7|Harmonic minor|

New sessions start at C natural minor, sixteen steps and 112 BPM. The scale controls generated Drift notes. Your played notes, stored original phrase and Anchor are not automatically quantized to that scale. Choose a matching key for your music. An unchanged performed note can remain outside the chosen scale; a generated pitch move goes to a permitted scale note.

Sixteen means one 4/4 bar of sixteenth-note steps; thirty-two means two bars. Stop playback before changing length. Shortening hides the second bar without deleting it. Returning to 32 reveals those events again. ENV can undo the length change after returning Home. Hidden events persist in the project. The normal and Zoom Home views show a second timeline row for a two-bar phrase; duration marks extend across covered steps.

## Record chords and held notes

Press REC, play a short phrase and hold each note for the duration you want. REC starts playback if necessary. Release a key to finish that recorded gate. Press REC again to end recording while playback continues. A gate can extend across several steps and across the loop boundary, up to one whole loop. Very brief taps have a small minimum gate rather than a zero-length note.

Onsets still snap to the nearest sixteenth. Gate duration follows the time held, with resolution of 1/256 of a step and input/audio-block timing limits. Each lane stores up to four notes per step, with an independent duration for each. Play a chord, then release its notes at different times: those durations replay independently. Holding a note clears only covered repeats of that same pitch; other recorded pitches survive. Overdubs fill free positions, and a repeated pitch on the same step replaces its existing event. When a step is full, an additional pitch first replaces a released position. If all four positions are still held, it replaces the oldest held onset. Retriggering a note makes it recent again. The synth and each step still have a four-note limit.

The synth still has four voices total, shared by live keys, MIDI, Memory and Layer. When both sequenced lanes start chords on the same step, Memory receives up to three voices and Layer the remaining voices. Later onsets can steal earlier sounding voices. Two lanes do not mean eight simultaneous synth voices. Local onsets detected together use one quantization decision, so a scheduler boundary during processing cannot split that batch. Notes arriving in separate scans or MIDI messages still quantize at their own arrival times.

Turning recording off while a key is still down finishes its recorded duration at that moment; the live key continues sounding until you release it. Stopping transport, entering settings/dialogs, switching listening/recording lanes, branching or catching an Anchor finishes pending captured notes. A tempo adjustment also finishes the pending capture before adopting the new tempo. Notes already held before REC do not create a new recorded onset; release and play them again.

USB and TRS MIDI preserve source/channel ownership. The recording follows MIDI note-off and CC64 sustain release independently of voice stealing. An unrelated source cannot finish a captured note. MIDI CC120/123 closes captures owned by that source/channel. Gates are clamped to the chosen loop length during playback, even if a longer stored gate came from a previously longer loop.

PLAY stops or restarts from the beginning. Hold HOME alone for one second to panic, stopping transport and clearing audio. Release and press held keys again afterward.

## Evolve a musical phrase

A short SAVE press catches the first Anchor once Memory contains notes. It cannot overwrite an existing Anchor. Hold SAVE for about 0.9 seconds to open project Save instead; the hold does not Catch an Anchor first. Short SAVE now acts on release. SEQ remains the direct Save shortcut. LFO switches between protected Anchor and evolving Memory with enabled Layer; HOME returns to Memory and releases the previous sequenced gates. Switching the listening phrase ends recording and releases the previous sequenced gates, allowing natural envelope/effect tails. Pressing LFO without an Anchor explains that Catch is needed and keeps the current recording running.

Drift makes small moves through the chosen scale, normally one degree and at its highest setting sometimes two. More Drift affects more events. Evolution follows a repeating eight-cycle phrase and returns to the performed pitches on its first cycle. It never progressively walks away from the original notes. Free retains chromatic changes, still bounded by that phrase structure. Changes affect playback, leaving the stored source recoverable.

Melt smoothly opens the resonant tone without changing pitch. The amount a sound responds depends on its patch design. Decay reduces velocity over repeated cycles, with a floor. GLO commits the currently evolved Memory as a branch and resets the evolution knobs. Its pitches and durations remain stored; later evolution works from that new phrase. ENV provides one supported musical Undo; knob movements are not individual Undo steps.

To restore Anchor into Memory, stop playback and hold LFO about 1.2 seconds, then confirm with SEL/SCL. HOME or FX cancels. Anchor restore keeps the Layer and selected sound and can be undone during the same powered session.

EDIT enables Layer and selects it for recording. Turning it off preserves its notes, stops its sequenced held gates so releases can fade, and selects Memory for recording. Anchor listening excludes Layer. FX opens Clear Layer confirmation. Holding FX about 1.6 seconds changes it to New Session. SEL/SCL confirms; HOME or FX cancels. Save important work before beginning a new session.

## Save and load

Stop playback. Hold SAVE about 0.9 seconds, or press SEQ for Save, turn SELECT to choose slot 1–8, and press SEL/SCL to confirm. ARP opens Load with the same slot/confirmation controls. HOME or FX cancels. Successful Save also remembers display preferences, digital level, earcon choice and current slot. Boot does not automatically load a project; explicitly Load it. PLAY also works inside a confirmation, so you can stop there before confirming Save.

Home has a persistent bottom status strip. A file with a plus means an untouched, untitled session; a file with a pen means unsaved changes; a file with a check means the assigned project matches its saved reference. A slot number appears only after a successful Save or Load. The paired-measure icon and 16/32 show the loop length. The crossed-out speaker appears at digital level zero; it cannot detect the physical MASTER position.

Save/Load dialogs show whether the chosen slot contains a valid Wanderer project (file/check), no valid Wanderer project (file/minus), or an unverified write result (file/cross). “No valid project” does not mean the slot is empty or safe to overwrite: another firmware may use it. The separate file/pen symbol says the current session has unsaved changes. Browse another slot with SELECT; this does not alter the assigned Home slot.

Save and Load success screens remain visible until acknowledged. HOME, SEL/SCL or FX acknowledges them without executing a second action. Other controls or a new note dismiss a success while performing the chosen action. Errors remain until acknowledged. If the project saves but preferences fail, the two-row result shows Save/slot/check and Preferences/cross. Your music is saved; the session remains marked edited until a successful retry. A failed or unverified write never produces a Saved result.

Undo after Save returns the prior music and marks it edited against the newly saved project. Undo immediately after Load restores the previous session and its project identity. Starting and stopping REC without playing a note does not consume the previous Undo. Undo preserves your current theme, Zoom and reduced-motion choices.

1.2 writes version-five projects, including all chord events, durations, root, scale and two-bar storage. It reads version-four 1.1 projects at their original length, preserving their performed notes and held durations. It also imports Wanderer 0.7/0.8/0.9/1.0 v1/v2/v3 records using their exact older layouts; those events keep their original three-quarter-step gates and sixteen-step length, and select Free to avoid imposing a scale. The 1.1/1.2 engine changes how old patch numbers sound.

1.1 and earlier firmware cannot read a project saved in 1.2. Use a spare slot when converting music you still need to open in an older version. An unoverwritten older slot is read only when you deliberately Load; importing alone does not rewrite it. Firmware rollback and project compatibility are separate. Other firmware may overwrite shared storage, and there is no external project-export feature. Avoid interrupting a Save.

## Settings, waveform and light feedback

Settings has four pages: System, Effects, Performance and Music. PRESETS selects a page and SELECT selects its row. System toggles theme, Zoom and reduced motion with SEL/SCL. Zoom applies to editors and confirmations as well as Home. Music always presents one large selected parameter.

Effects retains chorus, tempo delay and stereo room. Select a row with SELECT, toggle it with SEL/SCL, and set wet amount with parameter knob four. Delay's ALGORITHM selects 1/16, 1/8, 3/16 or 1/4 timing, while knob three sets feedback. On Room, ALGORITHM sets size and knob three sets damping. Details use normalized percentages, not physical room measurements.

Performance retains digital level, the bell for earcons, and current storage slot. Select the bell and tap SEL/SCL to toggle cues. Save a project to remember that choice after reboot. No startup or panic beep is added.

HOME + SEL/SCL opens/closes the real-time oscilloscope in either order. Release both before repeating it. SELECT changes visual gain, ALGORITHM changes time span, SEL/SCL freezes/resumes, and HOME returns. Audio and transport continue. The trace is the stereo average after effects and digital level, before earcons and physical MASTER scaling. A physical MASTER mute need not flatten it. Display gain changes no sound. Reduced motion lowers its refresh rate.

Bright lamps indicate current state or confirmation; dim lamps indicate relevant available actions. Settings, dialogs and persistent result/error pages clear unrelated Home lamps. On persistent results/errors, SEL/SCL is bright for acknowledgment and HOME/FX are dim alternatives; SEQ or ARP identifies a storage result. PLAY, sounding notes and REC remain lit only when those activities continue. PLAY and sounding-key lamps can remain lit because their audio continues. An illuminated key represents an active gate, not the entire release tail. The selected listening phrase has a strong frame; record dots identify the recording lane. The Layer switch, Anchor lock and scope Freeze symbol keep distinct meanings.

HOME and SEL/SCL short actions happen on release to support their two-button scope combo. Save/Load outcomes and error notices persist until HOME, SEL/SCL or FX acknowledges them; that acknowledgment does not execute another action. A stop-first notice can appear when changing length or saving during playback.

## Listen before installing

The 48-second showcase contains twelve four-second excerpts: 001 Warm Wander, 011 Wide Saw, 023 Reedy Lead, 032 Thick Root, 036 Acid Willow, 043 Electric Mist, 050 Moon Celeste, 051 Glass Bell, 067 Liquid Grid, 073 Slow Aurora, 077 Golden Weather and 080 Homeward Light. They use the production DSP with restrained chorus and room. The full dry-bank tour gives each of the 100 presets three seconds: two seconds held and one second after release. Slow releases may be truncated by the next audition. These renders provide a listening reference; hardware speakers/headphones and personal taste still need evaluation.


## Licensed symbols and consistent feedback

Anchor uses a nautical anchor, Memory repeat arrows, Layer stacked planes, Save a floppy disk and Load an open folder. On normal Home, headphones identifies the monitored phrase; the header play/stop symbol alone describes transport. Root uses musical notes, Scale a keyboard and Length paired measures. The 1.1 sound bank, effects, scope and approved splash remain unchanged. The status strip adds four more icons from the same licensed Lucide family. UI_REVIEW_1.2.md explains the new status symbols and screenshot review. Original SVGs and their Lucide/Feather/Tabler notices accompany the source.

## Chord recording listening check

The separate 1.2 chord demo is rendered by the production sequencer from recorded captures, not by directly playing live voices. It includes a three-note chord with staggered releases, then the saved/reloaded two-bar phrase using a sustained pad and a key patch. A half-second pause introduces each example. The inherited twelve-preset showcase and dry bank tour remain references for the unchanged synth.
