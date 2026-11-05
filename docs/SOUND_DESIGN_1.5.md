# Wanderer 1.5: the 150-sound bank

The factory bank is rebuilt around playing roles: Foundation Bass, Character Bass, Leads, Poly Keys, Soft Keys, Organs and Reeds, Brass and Winds, Ensemble Strings, Slow Pads, Motion Pads, Plucked Strings, String Mutations, Tuned Mallets, Bells and Metals, and Textures and Percussion. Each group contains ten original sounds.

The collection uses 39 analog, 30 phase-distortion, 32 string, 33 modal and 16 FM patches. A new number means a new recipe, rather than a renamed copy of an earlier preset. Different envelope lengths, held-note behavior, oscillator intervals, spectral balances and Melt responses make the sounds useful for different jobs.

## Sound design

The analog patches use saw, pulse, hollow, sine and triangle-family sources, with deliberate unison, octave or fifth blends. Sub oscillators provide low-end support where useful. Filter envelopes are independent of amplitude envelopes: a bass can lose brightness while retaining its low body, and a brass patch can open gradually while the note swells. Slow detuning and restrained modulation give ensembles movement without applying pitch wobble to every sound. The existing cascaded four-pole low-pass path remains the sound-shaping filter; this is not a transistor-ladder circuit emulation.

The phase patches use different bend amounts, sine blends, detuning and phase-bend LFO depths. Short phase keys, full held leads and long evolving pads have different amplitude envelopes. Melt moves the bend away from the midpoint for a stronger edge on most phase sounds. Round Whistle deliberately begins close to a sine, with Melt adding more distortion. This uses Wanderer’s piecewise phase bend, not the CZ’s full dual-line, eight-stage DCW architecture.

FM patches separate sustained carrier body from the modulator transient. Electric-key and wooden-mallet sounds shed their bright onset before the body decays. FM leads retain a modest sustained index. Drawbar patches use the additive algorithm and harmonic partial levels. Only sixteen patches use the FM engine; metal and digital colors also come from the modal and phase engines.

String patches vary excitation position, damping, tension/loss, cutoff, articulation and Melt behavior. They span bass plucks, nylon-like harps, bright bridge wires, palm-muted phrases, long resonating strings and short noisy snaps. This engine has an initial pluck excitation; a long amplitude envelope does not turn it into a continuously bowed-string model. Ensemble and bowed-style sustained colors therefore use the analog and phase engines.

Modal patches use deliberate harmonic or nonharmonic partial ratios, individual partial levels and separate decay times. Upper modes generally lose energy before the fundamental. Harmonic settings supply additive organs and soft sustained pads. Bar-like, membrane-like and bowl-like settings create distinct mallets and struck bodies. Four partials and Q4 ratios limit physical accuracy; the names describe useful synthetic colors rather than exact acoustic reproductions.

Factory level trims are compiled offline. They use the active part of production-DSP auditions so long pads do not jump in level beside plucks. Short percussion retains its transient contrast. No new normalization, floating-point calculation, sample streaming or allocation is added to the audio callback. Existing Scatter, effect and arpeggiator settings remain under the player’s control when choosing a sound.

## Browsing and compatibility

PRESET wraps through 1–150. SELECT moves through fifteen groups of ten. MIDI uses CC0 = 0 and CC32 = 0/1 before Program Change, with separate pending banks for every input and channel. Bank zero supplies presets 1–128; bank one supplies 129–150 through program values 0–21. Invalid combinations do not select a different sound.

New saves use v9. Versions 1–8 are read without rewriting their slots. Snapshot patches and Anchors are retained. Frozen 1.3 and 1.4 factory data keep old fallback sounds and names available. Browsing a preset selects the new bank. Save to a spare slot when retaining a 1.4 rollback project.

## References

The techniques are adapted to the controls Wanderer actually implements. No manufacturer preset data or audio is included.

Moog’s [Sub 37 manual](https://api.moogmusic.com/sites/default/files/2018-09/SUB_37_MANUAL_v1.1_0.pdf) explains oscillator spectra, pulse width, sub support and envelope-driven filtering. These inform the analog recipes and their different articulations.

Yamaha’s [How FM Tone Generation Works](https://usa.yamaha.com/products/contents/music_production/synth_50th/anecdotes/004.html) explains the relationship between carrier/modulator ratios, modulation level and changing brightness over an envelope. These inform the FM body/transient designs.

Arturia’s [CZ V synthesis explanation](https://support.arturia.com/hc/en-us/articles/4405748202002-CZ-V-General-Questions) describes how nonuniform phase scanning adds harmonics. Wanderer applies that principle through its existing phase-bend and sine-mix controls.

Ableton’s [Live instrument reference](https://www.ableton.com/en/manual/live-instrument-reference/), especially Tension and Collision, describes excitation position, damping, resonator materials and decay. These inform the physical-string and modal recipes, within Wanderer’s smaller models.

The MIDI Association’s [MIDI message guide](https://midi.org/about-midi-part-3midi-messages) and [controller table](https://midi.org/midi-1-0-control-change-messages) describe Bank Select and Program Change addressing beyond 128 sounds.
