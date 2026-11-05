# LVM Wanderer 2.0 RC11

Use the `.fwsc` with your usual FM-1 updater. The package uses the permanent
device identity `FM-1_929`. Native build and simulated upgrade and rollback
passed. Test boot, response to the physical buttons, audio and worst-case
CPU load on your FM-1 before release.

## Performance controls

Hold **OCT+** for **1.5 seconds** to toggle Zoom from the current screen.
A tap still raises the octave. The existing **HOME + EDIT** gesture also
toggles Zoom.

On Home, hold the shared **PLAY/STOP** button for **2 seconds** to recall **Afterglow**,
the first preset. A tap starts a stopped transport on release. During
playback the button stops immediately on contact. Recalling Afterglow
changes the selected lane's sound and preserves the recorded notes.

Finish a recording on Layer 1, then tap **LFO**. If there is no Anchor
yet, the first tap protects the completed Memory loop and listens to it.
Tap **LFO** again to return to the working layers. A short **SAVE** can
still capture the first Anchor without changing what you hear.
With transport stopped, hold **LFO** for 1.2 seconds to open Restore Anchor. The hold leaves
listening and recording alone. In a confirmation, tap **SELECT** to
confirm or **HOME** to cancel. A long SELECT leaves the prompt in place.
Scope shows the signal and keeps preset selection on Home. If you press
REC while listening to Anchor, tap LFO to return to the working layers.
Print can still capture Anchor through its own page.

## Display and menus

Normal Edit and Settings pages show the knob number and label above the value
in each row. Zoom enlarges one parameter at a time and
keeps the same knob assignments. Use **Settings → Access Feel** for cue mode
and level. **Settings → Output** now uses knobs 1, 2 and 3 for Level, Slot
and Rec Click. Save and Load show slot status with **HOME** and **SELECT**
labeled below the action icons. Save identifies an occupied slot as
Overwrite. Undo in both Home and Edit restores the same tempo and pattern
length, and preserves Zoom and other display preferences.

Fresh sessions start with Afterglow, chorus off, and the 512 ms nominal
Scatter size. Reverb remains available from **Settings → Reverb**.
Loading a saved project restores its own effect and grain settings.

## Presets and the second layer

The 150-patch bank opens with ten leads. Afterglow uses three detuned
saws, an octave-under sine, light drive and an LP 24 envelope. Other
voicings include the Night Drive Reese bass, Prism Keys, Amber Tide pad,
Chrome Tines and Frost Pluck. The prior factory tables stay available to
older projects so saved preset indexes keep their original meaning.

Hold **EDIT** for about one second to flip the numbered **1 / 2** switch
on Home. The highlighted side is the layer you play, edit, and record
into. Both layers keep playing through the switch. A held note stays
with the layer it started on, and the next note goes to the selected
layer. Turn **PRESET** to browse its sounds, turn **SELECT** on Home to
jump by bank, or tap **EDIT** for its synth parameters. The Anchor and
Memory cards occupy the same positions in Normal and Zoom. Zoom enlarges
both icons. The Memory card shows the selected layer’s pattern and its
recording or mute status. **OCT−** and **OCT+** taps change octave.

The Anchor card shows a lock once an Anchor is stored and a listening
icon during preview. The Memory card shows RAM, ARM, or WRITE during
Print. Use **PRESET** to see a preset name.

Hold **SELECT** for Settings, then use **PRESET** to find **Layer**.

| Knob | Control | Behavior |
| --- | --- | --- |
| 1 | Edit | Choose Memory or Layer, just like holding EDIT. |
| 2 | Playback | Play or mute the recorded Layer pattern. The keys still audition it when selected. |
| 3 | Patch | Choose Independent or Follow Memory. Returning to Independent restores the retained Layer patch. |

Picking a preset or editing synth parameters while Layer is selected
automatically makes its sound independent. Both lanes share four voices
and the same effects. Anchor monitoring and Print RAM playback retain
their existing routing.

The 39-second host audition has sections at 0:00 Afterglow, 0:06 Night Drive,
0:12 Prism Keys, 0:18 Amber Tide, 0:24 Frost Pluck and 0:30 Afterglow plus
Amber Tide in a two-lane loop with Melt and Drift.

## Phrase motion and Scatter

**Melt** softens and shortens offbeats while accenting landmarks over a
four-loop arc, alongside the sound's timbre changes. Held notes keep their
duration. **Drift** starts with occasional one-scale-degree answers near
phrase endings. Higher values add answers on other weak beats. Downbeats
and root notes stay put. Every fourth loop returns to the original pitches.
Printed audio uses a crossfaded rhythmic shift at the same phrase
positions. The protected Anchor is outside these transformations.

In **Edit**, **Scatter Grid** is the next page after **Scatter**. Scatter's
**Mode** selects the sound and **Size** selects the grain. Scatter Grid's
**Grid** selects when events happen. The timing option formerly labeled
**Late Beats** is now **Late**. **Rush** is a sound mode. **Bar End**
is the timing pattern for the last two steps. Reverse playback belongs to
the sound modes. Variation changes how often the grain offset wanders.

## Reverb

Reverb Mix keeps the original gain through the lower half of the knob.
The upper half boosts the return and fades the direct synth. At full Mix
the reverb return reaches three times its previous level before the
stereo limiter and the direct synth is silent. Other effects retain
their own mix settings. Ambient has a much longer, diffused decay. Size
at minimum gives a shorter wash. At full Size and the default Damping,
a test chord still registers in the wet return one minute later. The
return decays gradually and switching Reverb off clears it.

## MIDI and compatibility

**Settings → MIDI** controls note input (**USB + TRS**, **USB**, or **TRS**),
channel (**Omni** or **1–16**), **Arp Clock** (**Internal**, **USB**, or
**TRS**), and Program Change. Clock is independent of the selected note
input. MIDI Start and Stop from the selected clock port control transport.
The event loop uses the BPM setting, while the arpeggiator follows the
incoming clock. MIDI Program Change targets the currently selected lane.
The MIDI routing options are project settings.

Existing v13 and v14 projects load without writing the old slot. They keep
their stored Layer playback and patch behavior. New saves use v15 to
store independent Layer patch edits. Save to a spare slot if you also
want to open the original project in RC2 or earlier firmware.

The control design drew on [Ableton Drift](https://www.ableton.com/en/live-manual/11/live-instrument-reference/),
[Mutable Instruments Marbles](https://pichenettes.github.io/mutable-instruments-documentation/modules/marbles/manual/),
and [Ableton's note and velocity chance](https://help.ableton.com/hc/en-us/articles/360019144299-Note-and-Velocity-Chance-in-Live-11-FAQ).
Wanderer uses its own fixed-point implementations and preset recipes.

## Build and verification

Print Drift now computes its read offset once per step. Print playback
skips the unused event-note scan. The synth caches four-operator FM index,
stacked-saw detune and steady modal rotation coefficients. The host audition
matched sample for sample before and after the final oscillator caches.

| Target region | Used | Free |
| --- | ---: | ---: |
| Application slot | 549,576 B | 31,988 B |
| Data and BSS | 92,576 B | 5,728 B |
| Audio pool | 342,536 B | 1,528 B |

RC11 passed the native build and stack checks. Static pages and held Scope redraw on
changes. Home retains its moving playhead during playback, and live
Scope has a slower refresh when Reduced Motion is enabled.

Host checks covered 336 Home states across six themes, both sizes, both
layers, both lengths, and Anchor and Print states. Layout checks covered
1,000 preset name and size combinations and 3,735 control values.
The host suite covers live layer switching, hold conflicts, flash staging,
simulated upgrade and rollback against RC10, and a 16,000-callback
adversarial run with address and undefined-behavior sanitizers.

The Anchor control tests cover the SAVE result clearing when preview begins,
first-tap capture with transport running, Layer 2-only takes, an active
recording and hold-to-restore isolation.

The installer package is 612,196 bytes. SHA-256 is
`4a8f1a10157415bce5d4373f8b9cf44e68e0cfc09450273eba1a97f89dfd5f96`.

## RC2 audit

* MIDI clock ticks update the arpeggiator clock without forcing a full
  redraw. Source changes clear old clock estimates, and MIDI transport
  updates run under one interrupt lock.
* Tempo changes preserve the current step's fractional progress. Tempo
  and length stay locked while Print is armed or capturing. A MIDI
  restart discards the incomplete Print generation.
* Layer audition, Memory and Anchor use explicit voice patch sources.
  Editing Memory leaves already sounding Anchor and Layer voices alone.
* Long-press handling covers late release, timer wrap, panic cancellation
  and confirmation screens. Octave changes apply before new key onsets
  from the same input pass.
* AddressSanitizer and UndefinedBehaviorSanitizer exercised 16,000
  randomized audio callbacks, preset/effect changes, split lanes, Print,
  undo, and malformed project slots. The exact native package passed
  staging and simulated upgrade and rollback against 1.9 RC1 and 2.0 RC1.
  The loader test harness now rejects truncated fixture files cleanly.

Run the host smoke suite with
`WANDERER_RELEASE=2.0-RC11 WANDERER_BASELINE_PACKAGE=/path/to/prior.fwsc python3 tools/smoke_1_7.py`
after the native build. The stock-restore check also uses a verified
stock v15 package at `build/stock-v15-verified.fwsc`.

To reproduce the build, run `sh tools/compile_scatter_1_8.sh` with the
JieLi toolchain and AC79 SDK from [README-DINGHY.md](../README-DINGHY.md).
