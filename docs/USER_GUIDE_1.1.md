# Wanderer 1.1 RC2

This candidate focuses on musical evolution, held-note recording and a new synth bank. It is software-tested; acceptance of these new firmware bytes on a physical FM-1 is still pending.

## Start with the new sounds

PRESETS chooses sounds 1–100. SELECT, the rotary control, jumps by ten. The separate SEL/SCL button opens settings. ALGORITHM sets tempo on Home. Parameter knobs one, two and three control Melt, Drift and Decay; knob four controls digital level. The physical MASTER remains your overall hardware output control.

Try 011 Wide Saw, 023 Reedy Lead, 032 Thick Root, 036 Acid Willow, 043 Electric Mist, 050 Moon Celeste, 051 Glass Bell, 067 Liquid Grid, 073 Slow Aurora, 077 Golden Weather and 080 Homeward Light. Hold pad notes for a couple of seconds so their attacks can develop. Plucks, keys and bells deliberately decay even if you hold a key: that is their envelope design, not a recording limitation.

The synth now uses longer-range envelopes, exponential decay/release, a resonant lowpass, velocity-sensitive brightness, optional sub weight, pulse-width motion, patch-specific vibrato/filter motion and gentle drive. These are original sounds inspired by broad synthesis approaches, not sampled or exact brand replicas. Four synth voices are shared by live keys, MIDI and the two sequenced lanes.

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

## Record held notes

Press REC, play a short phrase and hold each note for the duration you want. REC starts playback if necessary. Release a key to finish that recorded gate. Press REC again to end recording while playback continues. A gate can extend across several steps and across the loop boundary, up to one whole loop. Very brief taps have a small minimum gate rather than a zero-length note.

Onsets still snap to the nearest sixteenth. Gate duration follows the time held, with resolution of 1/256 of a step and input/audio-block timing limits. Holding a note replaces covered onsets in that lane. Each lane remains monophonic: a new recorded note ends the preceding captured note. This is not chord recording. Live playing remains four-voice polyphonic.

Turning recording off while a key is still down finishes its recorded duration at that moment; the live key continues sounding until you release it. Stopping transport, entering settings/dialogs, switching listening/recording lanes, branching or catching an Anchor finishes pending captured notes. A tempo adjustment also finishes the pending capture before adopting the new tempo. Notes already held before REC do not create a new recorded onset; release and play them again.

USB and TRS MIDI preserve source/channel ownership. The recording follows MIDI note-off and CC64 sustain release independently of voice stealing. An unrelated source cannot finish a captured note. MIDI CC120/123 closes captures owned by that source/channel. Gates are clamped to the chosen loop length during playback, even if a longer stored gate came from a previously longer loop.

PLAY stops or restarts from the beginning. Hold HOME alone for one second to panic, stopping transport and clearing audio. Release and press held keys again afterward.

## Evolve a musical phrase

SAVE catches the first Anchor once Memory contains notes. It cannot overwrite an existing Anchor. SAVE is not the project storage button. LFO switches between protected Anchor and evolving Memory with enabled Layer; HOME returns to Memory.

Drift makes small moves through the chosen scale, normally one degree and at its highest setting sometimes two. More Drift affects more events. Evolution follows a repeating eight-cycle phrase and returns to the performed pitches on its first cycle. It never progressively walks away from the original notes. Free retains chromatic changes, still bounded by that phrase structure. Changes affect playback, leaving the stored source recoverable.

Melt smoothly opens the resonant tone without changing pitch. The amount a sound responds depends on its patch design. Decay reduces velocity over repeated cycles, with a floor. GLO commits the currently evolved Memory as a branch and resets the evolution knobs. Its pitches and durations remain stored; later evolution works from that new phrase. ENV provides one supported musical Undo; knob movements are not individual Undo steps.

To restore Anchor into Memory, stop playback and hold LFO about 1.2 seconds, then confirm with SEL/SCL. HOME or FX cancels. Anchor restore keeps the Layer and selected sound and can be undone during the same powered session.

EDIT enables Layer and selects it for recording. Turning it off preserves its notes, stops its sequenced held gates so releases can fade, and selects Memory for recording. Anchor listening excludes Layer. FX opens Clear Layer confirmation. Holding FX about 1.6 seconds changes it to New Session. SEL/SCL confirms; HOME or FX cancels. Save important work before beginning a new session.

## Save and load

Stop playback. Press SEQ for Save, turn SELECT to choose slot 1–8, and press SEL/SCL to confirm. ARP opens Load with the same slot/confirmation controls. HOME or FX cancels. Successful Save also remembers display preferences, digital level, earcon choice and current slot. Boot does not automatically load a project; explicitly Load it.

1.1 writes version-four projects with durations, 32-step storage, key and scale. It imports Wanderer 0.7/0.8/0.9/1.0 records using their exact older layouts. Imported events keep their original three-quarter-step gates, sixteen-step length, patch numbers and effect values. Imports select Free to avoid imposing a newly chosen scale. The new engine and bank change how those patch numbers sound.

Older Wanderer firmware cannot read a project saved in 1.1. Use a spare slot when converting music you still need to open in an older version. An unoverwritten older slot is read only when you deliberately Load; importing alone does not rewrite it. Firmware rollback and project compatibility are separate. Other firmware may overwrite shared storage, and there is no external project-export feature. Avoid interrupting a Save.

## Settings, waveform and light feedback

Settings now has four pages: System, Effects, Performance and Music. PRESETS selects a page and SELECT selects its row. System toggles theme, Zoom and reduced motion with SEL/SCL. Zoom applies to editors and confirmations as well as Home. Music always presents one large selected parameter.

Effects retains chorus, tempo delay and stereo room. Select a row with SELECT, toggle it with SEL/SCL, and set wet amount with parameter knob four. Delay's ALGORITHM selects 1/16, 1/8, 3/16 or 1/4 timing, while knob three sets feedback. On Room, ALGORITHM sets size and knob three sets damping. Details use normalized percentages, not physical room measurements.

Performance retains digital level, the bell for earcons, and current storage slot. Select the bell and tap SEL/SCL to toggle cues. Save a project to remember that choice after reboot. No startup or panic beep is added.

HOME + SEL/SCL opens/closes the real-time oscilloscope in either order. Release both before repeating it. SELECT changes visual gain, ALGORITHM changes time span, SEL/SCL freezes/resumes, and HOME returns. Audio and transport continue. The trace is the stereo average after effects and digital level, before earcons and physical MASTER scaling. A physical MASTER mute need not flatten it. Display gain changes no sound. Reduced motion lowers its refresh rate.

Bright lamps indicate current state or confirmation; dim lamps indicate relevant available actions. Settings/dialogs clear unrelated Home lamps. PLAY and sounding-key lamps can remain lit because their audio continues. An illuminated key represents an active gate, not the entire release tail. The selected listening phrase has a strong frame; record dots identify the recording lane. The Layer switch, Anchor lock and scope Freeze symbol keep distinct meanings.

HOME and SEL/SCL short actions happen on release to support their two-button scope combo. Error notices persist until HOME, SEL/SCL or FX acknowledges them; that acknowledgment does not execute another action. A stop-first notice can appear when changing length or saving during playback.

## Listen before installing

The 48-second showcase contains twelve four-second excerpts: 001 Warm Wander, 011 Wide Saw, 023 Reedy Lead, 032 Thick Root, 036 Acid Willow, 043 Electric Mist, 050 Moon Celeste, 051 Glass Bell, 067 Liquid Grid, 073 Slow Aurora, 077 Golden Weather and 080 Homeward Light. They use the production DSP with restrained chorus and room. The full dry-bank tour gives each of the 100 presets three seconds: two seconds held and one second after release. Slow releases may be truncated by the next audition. These renders provide a listening reference; hardware speakers/headphones and personal taste still need evaluation.


## Refined symbols in RC2

Anchor uses a nautical anchor, Memory repeat arrows, Layer stacked planes, Save a floppy disk and Load an open folder. On normal Home, headphones identifies the monitored phrase; the header play/stop symbol alone describes transport. Root uses musical notes, Scale a keyboard and Length paired measures. Existing controls, 1.1 sounds and the project format remain unchanged. The symbol key in UI_REVIEW_RC2.md explains the full mapping. Original SVGs and their Lucide/Feather/Tabler notices accompany the source.
