# Wanderer 1.3 RC4

RC1 is withdrawn after a physical startup failure. The tested RC2 boot fix is retained; RC3 added Scatter and the supplied splash; RC4 adds grain size. See BOOT_FIX_1.3_RC2.md.

Wanderer is a four-voice instrument with a protected Anchor, a working Memory and an optional note Layer. Version 1.3 adds five synthesis families, 100 named patches, sound editing, randomized Scatter and destructive audio Print. The permanent firmware identity remains **FM-1_929**.

## Start playing

Choose a named sound with PRESET. SELECT jumps by ten sounds. PLAY starts or stops the loop; REC records notes into Memory, or into Layer when Layer is enabled. Held keys and four-note chords retain their durations. Tap SAVE to catch the first performed Memory as the protected event Anchor. Tap LFO to compare Anchor with the working loop. Hold HOME for panic.

## Controls

| Control | Action |
|---|---|
| PLAY | Start/stop. Stop clears sequencer gates and refreshes every keyboard LED. A held live key retains its normal feedback. |
| REC, in Event view | Record/stop note capture. |
| REC, when an audio loop is active | Arm/cancel one destructive Print pass. |
| Hold REC, 0.9 seconds | Open Print without starting note recording on the way. |
| EDIT | Open/close the sound editor. |
| Hold EDIT, 0.9 seconds | Toggle the note Layer. |
| SCL | Go directly to Music: root, scale and 16/32-step length. |
| Hold SCL, 0.9 seconds | Open System settings. |
| HOME + SCL | Open/close the live waveform scope. Either press order works. |
| HOME | Return/close a view; leave Anchor listening. |
| Hold HOME, 1 second | Panic: stop, release voices and captures, cancel pending Print, clear effect tails. |
| SAVE | Catch the event Anchor once. |
| Hold SAVE, 0.9 seconds; or SEQ | Open project Save. Choose a slot with SELECT, confirm with SCL, cancel with HOME/FX. Stop first. |
| ARP | Open Load. Choose a slot and confirm with SCL. Stop first. |
| LFO | Compare with Anchor. In Print, compare the protected audio Anchor. |
| Hold LFO, 1.2 seconds | Offer Anchor restore; stop first, then confirm with SCL. |
| GLO | Branch the evolving notes. When audio is active, arm/cancel Print. |
| ENV | Undo the latest event/edit action; when audio is active, cancel a pending pass or Undo/Redo the completed Print. |
| FX | Offer Layer clear. Hold 1.6 seconds for New Session. SCL confirms; HOME cancels. |
| Knobs 1 / 2 / 3, at Home | Melt / Drift / Scatter. Imported legacy projects retain their original Decay behavior. |
| Knob 4, at Home | Digital volume. |
| ALGORITHM, at Home | Tempo, 60–180 BPM. Audio loops retain their recording tempo. |
| PRESET, at Home | Named sound. MIDI programs 0–99 select the same modern bank. |
| SELECT, at Home | Jump between groups of ten sounds. |
| Octave buttons | Shift the live keyboard; held keys still release their original pitch. |

Short actions dispatch on release so holding a shortcut does not fire the tap action first.

## Sound editing

EDIT opens the editor. PRESET moves between pages; SELECT chooses a control to inspect. Knobs 1–4 edit controls 1–4, including in Zoom. Scatter has three controls; knob 4 is unused there. Normal view uses spacious rows; Zoom shows the selected parameter. Pages follow the selected engine, so FM-only pages do not crowd an analog or string patch.

| Page | Controls |
|---|---|
| AMP | Attack, decay, sustain, release |
| FILTER | Cutoff frequency, resonance, low/band/high-pass type, envelope depth |
| FILTER ENV | Independent filter attack, decay, sustain, release |
| OSCILLATORS | Engine and the oscillator/physical parameters relevant to it |
| TIMBRE | Rounding/pulse/sub/drive, FM index, phase bend, string damping/pick position, or modal partial balance |
| MODULATION | LFO rate/depth/destination and patch level |
| OP 1–4 ENV | Independent FM operator or modal partial ADSR |
| FM / MODAL RATIOS | Four frequency ratios, in sixteenths: 16/16 is unison, 24/16 a fifth, 32/16 an octave |
| FM LEVELS / PARTIAL LVLS | Four operator/partial levels |
| ROUTING, FM | Eight algorithms, feedback, quick access to operator 1/4 ratios |
| PRINT | Print status, action reminders, Event/RAM source selection |
| PLAY | Scatter style, scale-following keyboard, root and scale |
| SCATTER | Style, nominal grain size and amount |

Envelope times are logarithmically spaced from 1 ms to 8 seconds. Attack is linear; decay and release are exponential time constants, so a tail can take several displayed time constants to become silent. Sustain and other unlabelled amounts use 0–127. LFO rate spans approximately 0.04–12 Hz. Frequency ratios fold into a playable upper register rather than overflowing phase increments.

FM algorithm numbers: 1 four-operator chain; 2 three parallel modulators into one carrier; 3 two independent pairs; 4 a chain plus a parallel modulator; 5 a three-operator chain plus a free carrier; 6 a shared modulator chain feeding two carriers; 7 one modulator feeding three carriers; 8 four additive carriers.

Edits apply to the current patch and its held voices. Selecting another preset starts that preset's factory patch; existing release voices retain their sound. Project Save stores the edited patch and the event Anchor's patch snapshot. ENV undoes an editing gesture. The legacy bank remains available through imported projects; editing or choosing a modern preset moves that working patch to the new engine while retaining its protected Anchor.

## Melt, Drift and Scatter

Melt moves toward endpoints designed into each patch: filter, oscillator rounding or FM index/phase bend, damping, pulse, drive and detune. It preserves the synthesis topology. On printed audio, Melt becomes a warm filter/drive insert that can be rerecorded.

Drift reads the performed motif rather than accumulating random transpositions. It protects roots and four-step landmarks, moves other notes in scale degrees, keeps velocity/duration, and resolves every eighth cycle. Chord voices share the same directional decision. On printed audio, Drift rearranges beat-aligned slices while retaining their recorded pitch; it resolves to the source timeline every eighth cycle.

Scatter replaces the predictable Repeat fill. Knob 3 controls the probability and density of bounded audio gestures: micro-stutters, nearby hops, earlier-fragment jumps, hesitations, brief dropouts and accelerating repeats. Low settings leave long clean stretches; high settings create more frequent clusters with breathing room. Gestures vary their slice size and can start slightly after the step boundary. Each performance seeds a new pseudorandom sequence.

SCATTER offers Mixed (all gestures), Slices (stutter/hold/rush) and Hops (hop/jump/dropout). PLAY also retains the style shortcut. These replace the previous Fill/Stutter/Both choices. The control acts on recent audio history in an event loop and reads fragments of the current generation in a printed loop. The underlying notes, their held gates and the transport clock keep running normally. Turning the knob to zero crossfades back to normal playback in about a millisecond. Legacy v1–v5 projects retain their original Decay semantics.

Tap EDIT, then turn PRESET left once from AMP to reach SCATTER. Knob 1 chooses Style, knob 2 chooses Size and knob 3 adjusts Amount. SELECT chooses among these three controls in Zoom. Size is **Auto, 4, 8, 16, 32, 64 or 128 ms**. Auto keeps the tempo-derived windows. Manual sizes vary by up to ±25% per gesture; Rush can halve a slice again. Small sizes make tight buzzes; larger sizes make longer, recognizable fragments. Size also bounds hop/jump windows and sets dropout length.

The signal path is **synth/playback → Scatter → chorus/delay/reverb → Print/output**. Printed-audio Melt sits between Scatter and the effects. Delay and reverb tails continue across Scatter gaps and can soften their edges. Existing printed audio may already contain effects from an earlier Print.

Scatter never edits the stored note phrases or audio generations during ordinary playback. Anchor comparison bypasses it. Only an explicit Print bakes its audible result into a new generation; the protected audio Anchor stays unchanged. There is no short-reverse gesture in this release.

## Destructive Print

1. Record a note loop, or play an existing Memory/Layer.
2. Hold REC to open Print, then tap REC to arm. A stopped transport starts; a running transport waits for the next loop boundary.
3. The first pass captures a protected **dry audio input Anchor** and a processed working loop together. This Anchor represents the input to the first audio capture, including the chosen sound and performed note evolution. It is separate from the event Anchor caught with SAVE.
4. Later passes rerecord the current audio, live playing and insert effects into a spare arena. A completed pass becomes the new working loop at the next boundary. An unfinished pass never replaces the current loop.
5. ENV cancels an armed/in-progress pass. After completion, ENV switches Undo/Redo. The first Print can Undo to its dry audio Anchor. LFO compares the audio Anchor; Print from that comparison makes a new working generation without overwriting the Anchor.
6. Knob 4 on the Print page selects Event (left) or RAM audio (right), once audio exists. Stop cancels pending recording and silences sequenced notes. New Session or loading a project clears the RAM audio history.

Print is deliberately colored: **16 kHz mono, 4-bit block ADPCM**, with antialias filtering before capture. The synth/dry output remains at 44.1 kHz; the compact stereo wet bus runs at 16 kHz. Each arena holds up to 8.192 seconds: 32 steps at 60 BPM fit. Effect tails are included inside the captured loop, not stored beyond its boundary. Melt and effect settings remain active after a Print; further passes can accumulate their character.

There are three arenas: protected Anchor, current generation and one Undo/workspace. Starting the actual next pass consumes the previous Undo workspace. Cancel keeps the completed current generation; it cannot recover an Undo that the new pass has already overwritten. The Anchor is never a write target after its first capture.

**RAM only: Save stores notes and patch settings, not printed audio.** Record the audio output externally if you want to keep it. Audio retains its captured pitch/tempo; root/scale and preset changes affect event playback/new notes, not the pitch of existing audio. Tempo/length changes are locked while a RAM loop exists; return to a saved event project or New Session to establish a different loop duration. An event Layer can be printed together with Memory; extra live notes can become audio overdubs on a later Print.

## Accessibility and compatibility

Dark/light themes, Zoom, reduced motion, the waveform view and optional earcons remain available. Selection uses thick outlines and location, not color alone. Text uses Atkinson Hyperlegible. Existing Lucide/Tabler geometry and licenses are retained; the new Repeat symbol follows the same SVG → raster → verified mask workflow.

The new project format is v7; Size is saved with the project and participates in Undo. Existing v6 projects load with Size set to Auto and preserve their sound and Scatter style. Imports from v1–v5 preserve notes, gates, the legacy sound bank and old Decay semantics. Import reads without rewriting the original slot. Save a converted project into a spare slot; older firmware cannot read v7. Other firmware can overwrite shared storage.

This is a software-tested release candidate. The exact 1.3 file still needs physical installation, listening and timing checks on an FM-1. Keep the previously tested recovery files and follow INSTALL_AND_RECOVERY.md.
