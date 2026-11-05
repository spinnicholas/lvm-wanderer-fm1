# Wanderer 1.0 RC2 for M-VAVE FM-1

Wanderer is a four-voice instrument with 100 original sounds, a protected Anchor phrase, an evolving Memory phrase and a second Layer. It records notes and velocities at sixteen positions in a repeating bar. It does not record incoming audio. This compiled 1.0 candidate is ready for installation testing; hardware acceptance of these new bytes is pending.

## A first session

Turn PRESETS to choose a sound from 1 to 100. Turn the SELECT encoder to move between banks of ten while keeping your position within a bank. Play the keys. ALGORITHM sets tempo from 60 to 180 BPM. The first three parameter knobs change Melt, Drift and Decay. The fourth parameter knob changes Wanderer's master level from 0 to 127. The physical MASTER control remains the overall hardware output control.

Press REC to begin recording; playback starts if needed. Press REC again to leave recording while keeping playback going. Press PLAY to stop or restart the bar from its beginning. A recording pass is one Undo action, so ENV can recover the phrase from before the pass. Notes are placed at the nearest sixteenth-note position; one note per lane can occupy each position.

When Memory has notes, press SAVE to catch your Anchor. SAVE is the Anchor action, not the project-saving button. An existing Anchor cannot be overwritten by another catch. LFO switches between listening to Anchor and listening to Memory with its enabled Layer. HOME returns to Memory. When leaving settings, HOME closes settings first.

EDIT turns the Layer on and selects it for recording. With the Layer on, REC writes the second lane and playback includes that lane. With the Layer off, REC writes Memory and Layer events are muted; their data remains available when you turn it on again. A note already triggered can finish its gate and release. LFO's Anchor listening mode excludes the Layer.

## Evolving and recovering a phrase

Melt opens the tone, Drift moves selected notes by small intervals, and Decay lowers velocity over repeated cycles with a floor. The stored Anchor remains intact. GLO commits the current evolved Memory to a branch and returns those three evolution controls to zero. ENV undoes the last supported musical action once. Parameter turns are not individually stored as Undo steps.

To replace Memory with the Anchor, stop playback and hold LFO for about 1.2 seconds. The Anchor confirmation symbol appears. Press SEL to restore, or HOME/FX to cancel. Restoration resets evolution and the last branch while keeping the Layer and selected sound. ENV can undo the restoration.

Press FX for Clear Layer, then SEL to confirm. Hold FX for about 1.6 seconds to change that dialog to New Session, then SEL to confirm. HOME or FX cancels. New Session stops the instrument and clears the musical session. It keeps your current light theme, zoom, reduced-motion preference and master level. ENV can undo it during the same powered session.

Hold HOME for one second for panic. Playback and recording stop, all voice gates close, and the effect buffers clear in bounded chunks. You can use panic while a note key is held; release and press that key again to retrigger. There is no panic beep. Anchor and Layer indicators on Home still describe the retained session.

## Projects and remembered preferences

There are eight project slots. While stopped, press SEQ for Save or ARP for Load. Turn SELECT to choose slot 1–8; the large number appears beside the save or load symbol. Press SEL to confirm. HOME or FX cancels. Save and Load are refused during playback or recording. Saving silences live notes before flash is written.

A project includes Anchor, Memory, Layer, branch, evolution controls, tempo, sound number, effect settings and the project's visual settings. The record format remains the same as 0.8/0.9. Older Wanderer records are validated and migrated on an explicit Load. No SLOOP projects are imported. Existing 0.9 slot 1 remains slot 1.

A successful Save also remembers master level, cue enable, current visual preferences and the selected slot for the next boot. Boot reads those preferences without writing flash. Project data is loaded only when you explicitly confirm Load. A malformed project is rejected without replacing the current session.

Use a spare slot before overwriting your only copy of a phrase. A power interruption during Save can invalidate the target slot; saving is not a power-loss-proof journal. Other firmware may overwrite the shared storage, and returning to Wanderer does not guarantee that its projects survive. This version has no external project-export feature. Record an audio performance or transcribe important phrases before changing firmware.

## Three settings pages

Press SEL on Home to open settings. Turn PRESETS to cycle through the three pages. The underline at the top identifies the current page. Turn SELECT to choose a row. Press HOME to return. Entering settings ends recording, while existing playback and live notes continue.

The System page contains a divided light/dark circle for theme, magnifier for the larger single-phrase view, and stillness symbol for reduced motion. SEL toggles the selected row. Reduced motion pauses regular redraw animation; it does not freeze musical state or LED updates.

The Effects page contains chorus, tempo delay and stereo room. SELECT chooses an effect and SEL toggles it. The fourth parameter knob changes the selected effect's wet amount. With Delay selected, ALGORITHM chooses one, two, three or four sixteenth-note delay lengths, and the third parameter knob sets feedback. With Room selected, ALGORITHM changes room size and the third parameter knob changes damping. A dedicated editor shows the parameter you changed, its own symbol and a matching value/gauge. Zero-valued details are shown too. The detail remains until you change the relevant control or navigate away. Feedback is bounded so tails decay. Chorus uses fractional delay taps for smoother movement.

The Performance page contains the speaker for master level, bell for earcon enable, and save symbol for the current project slot. On the speaker row, turn the fourth parameter knob to set level. On the bell row, press SEL to enable or disable cues. On the save row, turn the fourth parameter knob to select the project slot. Save a project to retain these preferences for the next boot. Master level affects voices, effect tails and cues, with smoothing; zero produces silence after the short smoothing interval.

## Meaningful key and button lights

Bright means a currently active state or the current confirmation action. Dim means a relevant action is available. The LEDs use brightness and position, not red/green distinctions. All button and note indicators are rebuilt from the current state. Only changed lamps are sent to the hardware, which avoids clearing and relighting everything every loop.

| Context | Meaning |
|---|---|
| Every view | PLAY is bright while playback runs. A key light shows a gated sounding note within the current keyboard octave, whether from the keyboard, MIDI or sequencer. Releases and panic clear it. A release tail can remain audible after its gate light goes out. |
| Home | REC shows recording. SAVE is bright for an existing Anchor and dim when a first catch is available. EDIT shows Layer enabled. LFO is bright when listening to Anchor and dim when an Anchor is available. GLO is dim when branching is available. ENV is dim when Undo is available. OCT− or OCT+ shows a transposition below or above the starting octave. |
| Settings | SEL identifies the active settings context; HOME is dim as the exit. Home-only Anchor, Layer, octave and Undo lamps are cleared. Playback and sounding-key indicators can remain on because their audio continues. |
| A confirmation | The relevant action button and SEL are bright; HOME is dim as cancel. Save uses SEQ, Load uses ARP, Clear/New uses FX, and Restore Anchor uses LFO. Unrelated status lamps are cleared. |

When you change octave, key indicators remap to the sounding pitches within the new visible key range. Notes outside that range have no key indicator. Brightness and viewing angle still need confirmation on the physical FM-1; the host tests verify the logical lamp masks.

## External MIDI

USB MIDI and the TRS MIDI input play notes on all sixteen channels. Source and channel ownership are independent, so a USB note-off cannot release a matching local or TRS note. Velocity-zero note-ons release notes. Sustain CC64, All Sound Off CC120 and All Notes Off CC123 are handled per source/channel. CC1 controls Melt. Program Change 0–99 chooses displayed sounds 1–100; higher program numbers are ignored.

External notes can be recorded into the current lane while recording on Home. Recording is suspended by settings and confirmation dialogs. There is no MIDI echo or MIDI note output. Pitch bend, MPE, external MIDI clock sync and realtime MIDI transport are not implemented in 1.0 RC2; use the internal tempo and panel transport. Four voices are shared between local keys, MIDI and both event lanes. Voice stealing and retriggering use a short splice to reduce abrupt changes. Existing voices keep their selected sound until retriggered.

## Status symbols

HOME and SEL short actions occur on release so the HOME + SEL/SCL oscilloscope combo works in either order. Read UI_AND_SCOPE_RC2.md for the waveform monitor and the complete visual update.

A check means an action succeeded. A cross means it was refused, unavailable or failed. A stop symbol with a cross means stop first. A load symbol with a cross means the selected slot has no valid project. A save symbol with a cross means a write or readback check failed; if the project write succeeded but the preferences write failed, the same broken-file error symbol appears, so inspect the saved slot before relying on it. Cues supplement these visual states and can be disabled.

## Installation and return to other firmware

Read INSTALL_AND_RECOVERY.md before installing. The new package identifies as FM-1_930. Keep the included, unchanged 0.9 package and your genuine stock v15 file available offline. A known-good 0.9 binary is included with its original source release.

The development prompt, complete test report, SHA-256 checksums and forty-five screenshots accompany this release. Errors remain until HOME, SEL or FX acknowledges them; acknowledgment does not also activate a menu or confirmation.

The screenshots come from the running production renderer at the FM-1's 240×240 resolution, enlarged by whole pixels. They are host captures, not photographs of a physical device.


# Wanderer 1.0 RC2: visual controls, oscilloscope and startup

This candidate implements the UI review and adds a live waveform monitor. The musical engine, 100-preset bank, project format, preferences format and protected recovery foundation are retained. RC1 and RC2 share the FM-1_930 family identity; use the release filename and SHA-256 to distinguish the actual package.

## Open the oscilloscope

Press HOME and SEL/SCL together, in either order, while the firmware is running. SEL is the button that the emulator's FM-1 panel map calls SCL, not the SELECT encoder. The combo opens or closes the oscilloscope. Release both buttons before repeating it. You can do this from Home, settings or a confirmation; closing the window restores the previous context. Opening a scope over a Save confirmation never confirms that Save.

HOME and SEL now perform their individual short actions on release, allowing either order of the combo without triggering a standalone action first. Holding HOME alone for one second still invokes panic. While one of the combo buttons is held, the partner's dim indicator makes the available pairing visible. The combo must be formed before a one-second HOME hold has already invoked panic.

In the scope, SELECT changes display gain from 1× to 16×. ALGORITHM changes the displayed window from roughly 6 to 186 milliseconds. Turn it down for faster waveforms and up for slower bass notes. The clock number describes the time span of the displayed capture, and the magnifier number is the display multiplier. Gain and time controls change the display only. The time readout follows the published capture's metadata, so it does not relabel an older frame with a newly requested timebase.

Tap SEL to freeze or resume the trace. A pause symbol appears and SEL lights brightly when frozen. Audio keeps running, and its transport status stays visible separately. Gain can still enlarge a frozen trace; timebase changes wait until you resume. Tap HOME to return to the previous view. PLAY remains available. Recording can be toggled with REC when the scope was opened from Home, and local keys and MIDI continue to work. Home's sound and evolution controls remain available there; a scope opened over settings or a confirmation preserves that parent context.

The trace is sampled from the stereo average after synthesis, effects and Wanderer's digital master, before earcons and before the platform's physical MASTER scaling. Stereo averaging can change the shape of wide effects. This is a musical visual monitor rather than a calibrated measurement instrument. At high display gains, the drawing can reach its vertical limits without the audio clipping. A zero digital master produces a flat trace after smoothing. Muting the later physical MASTER control does not mute the monitor's tap.

The display uses a rising zero-crossing trigger, with a free-running fallback that replaces old traces with a flat line during silence. Three capture buffers keep the audio writer away from the published frame and any frame being copied by the display. The audio callback never waits for rendering. Refresh is limited to about twelve updates per second during ordinary scope operation, or four with reduced motion, with immediate updates for control changes. The frozen trace remains visible through panic until resumed; a live trace becomes flat after panic.

## Earcons

Open settings with SEL, turn PRESETS to the Performance page, turn SELECT to the bell row and tap SEL to toggle earcons. The bell has its own identity, separate from the waveform symbol used for sound selection. Switch position and knob geometry indicate on/off without relying on color. Turning earcons off clears any earcon already playing. Voices and effect tails continue normally.

A confirmed project Save also stores the earcon preference, master level, visual preferences and current slot for the next boot. There are no new automatic flash writes. The saved preferences record remains compatible with RC1, and the existing 0.8/0.9 project record remains unchanged. Earcons are excluded from the oscilloscope even when enabled.

## Visual changes

Home retains the WANDERER name. Editors instead use their header for a context symbol and persistent transport/record indicators. Numeric editors use substantially larger digits, with a further enlargement when Zoom is enabled. Decorative orbits have been removed. Grouped, thicker step timelines represent actual phrase timing. Anchor, Memory and Layer keep their turquoise, lavender and gold identities; settings use neutral focus frames rather than repurposing those colors for unrelated rows.

One strong white frame identifies the currently monitored main phrase. A play triangle identifies its listening role, a record dot identifies the lane receiving notes, and Layer's position switch identifies whether it is enabled. Anchor's lock and empty-file badges distinguish a protected source from an uncaught Anchor. A slash identifies a disabled Layer while retaining visibility of its saved events. Selection, recording, protection and enablement no longer share the same highlight treatment.

Secondary boundaries have stronger contrast in both themes. Toggle state uses position and outline/fill geometry. Home-only button lamps remain cleared in settings and confirmations; scope lights show its own controls plus current sound/transport state.

Amounts are displayed as normalized percentages while the stored control values remain 0–127. An old digital-master setting of 100 therefore reads approximately 79 percent; the sound level itself has not been changed. Presets and project slots retain their ordinary numbers. Preset selection uses a bank-position strip rather than an amount gauge.

Delay timing appears as 1/16, 1/8, 3/16 or 1/4. Turning ALGORITHM on Delay opens the timing editor. The third parameter knob opens its feedback editor, displayed on the bounded feedback scale up to 67 percent. On Room, ALGORITHM opens size and the third parameter knob opens damping. Size and damping percentages describe the positions of those controls, not physical room dimensions. The fourth parameter knob returns to the selected effect's amount. A detail remains visible until another relevant control or navigation action changes it, including when its value is zero. Each number, icon and gauge describes the same parameter.

Zoom applies to settings, effect details, values and confirmations as well as Home. Zoomed settings present one large selected row; SELECT still moves among rows and PRESETS changes pages. Theme now uses a divided light/dark circle. Earcons use a bell, Memory uses a living-loop symbol, Delay uses diminishing echoes, Room uses a dispersed decay, and reduced motion uses a static framed point.

Save/Load confirmations show a storage tile and a large slot number. Clear Layer, New Session and Restore Anchor have distinct target diagrams. Successes use a check without a decorative countdown. Errors use distinct empty/broken/stop-first symbols and remain visible until dismissed with HOME, SEL or FX; that acknowledgment does not also execute a second action. Normal deliberate transport and musical actions remain available. A later operation can replace the notice.

## Refined startup splash

The included waveform W, tilted orbit, star, Wanderer wordmark and LVM signature are the refined artwork from the separate splash package. Both themes match its native pixels exactly. Startup uses the saved theme. The static splash expires after 1100 milliseconds and the first button, note, encoder or valid MIDI channel message dismisses it without consuming that action. There is no new startup sound, heap allocation, framebuffer or flash write. The protected boot/rescue path runs before this application artwork and remains unchanged.

## Installation and acceptance

Use LVM-Wanderer-1.0-RC2-FM1.fwsc with the included validated installer, following INSTALL_AND_RECOVERY.md. The unchanged 0.9 recovery file and its original source are included. Authentic stock v15 is still supplied separately and validated by the installer's --stock-v15 route. The larger art/font assets fit inside the existing application slot; memory, package and RAM-only flash-code checks remain enforced.

The candidate is software-tested and compiled. Real FM-1 display readability, control feel, worst-case timing, speaker/headphone operation, save/reboot/load and physical rollback of these new bytes remain the gate for stable 1.0. The test report separates those pending checks from sanitizer, host, real-package loader and compiled-emulator evidence.
