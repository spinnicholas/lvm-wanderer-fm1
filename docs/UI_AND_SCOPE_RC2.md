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
