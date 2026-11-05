# Wanderer 1.7: the JIANT and Skiff source audit

The audit used [JIANT](https://github.com/juanjiant-bit/JIANT/tree/07d3d70de1a1bf19480119f607402c106989da54)
and [Skiff](https://github.com/illuminus5/skiff/tree/48e77fda5916e9a5ba9e7339161f719d8877ce49),
including their analog, FM6, voice, effects and licensing files. The source
cannot establish a listener's preference, but it exposes useful engineering
differences behind that preference.

| Finding | Change in this build |
| --- | --- |
| Both use the MSFA/Dexed six-operator core, with logarithmic envelopes, level/rate scaling and velocity response | Added the actual Apache-2.0 C core as an additional FM6 engine, with 32 native algorithms and 20 original voices |
| JIANT's analog voices include sync, ring interaction and moving waveform shapes; Skiff rounds hard-sync resets with BLEP correction | Added band-limited sync and ring models; authored Sync Solo, Sync Torch and Ring Fifth; opened pad filters and revised PWM ensembles |
| Their factory voices deliberately combine sustained operator body, faster transient operators and modulation | Six independent contours, native operator levels, exact quarter-unit ratios, keyboard/velocity scaling and role-specific articulation; native sustain is mapped logarithmically from Wanderer's amplitude controls |
| Skiff's chorus and delay use separate storage; its reverb has input diffusion | Separated chorus, digital delay and tape storage, added a three-tap stereo chorus and two reverb diffusers |
| Richness comes from the signal chain as well as the oscillator | Added independent wavefolder, bitcrusher, stereo phaser and multi-head tape echo; delay returns can enter reverb |

The FM6 port retains the original algorithm, envelope, feedback, keyboard
scaling and velocity laws. Wanderer's wrapper maps its existing ADSR controls
into six native contours. It is not a full DX patch editor or SysEx bank
importer. Ratios use quarter units, with a high-resolution log-frequency table
so the displayed ratios do not acquire coarse/fine rounding errors. Operator
levels use the core's native 0–99 scale. Factory voices are original recipes;
no manufacturer bank was copied. Other modern engines and the old four-op FM
engine remain available. There are still four simultaneous sound voices.

## FM6 editing

Select FM6 on Oscillators. Its algorithm range is 1–32. FM Ratios and FM Levels
control operators 1–4; Color exposes ratios and levels for operators 5–6.
OP 1–4 ENV retain their pages. The usual AMP page becomes OP 6 ENV, and FILTER
ENV becomes OP 5 ENV. Routing exposes velocity response and keyboard scaling.
Index changes modulator level while leaving carrier level alone. Feedback,
filter, modulation and Drive remain available. The patch record preserves all
six operators and their controls in project and Anchor snapshots.

## Effects and controls

Hold SELECT to open settings, then turn PRESET to the page. Existing pages
keep their indices; the new pages follow Access Feel. HOME + EDIT still toggles
Zoom globally. Every effect uses the same four-knob layout in normal and Zoom.

| Page | Four knobs |
| --- | --- |
| Chorus | On, Mix, Rate, Depth |
| Delay | On, Mix, synced Time, Feedback |
| Reverb | On, Mix, Size, Damping |
| Bitcrush | On, Mix, 1–16 Bits, effective Rate in Hz |
| Tape Echo | On, Mix, Time in milliseconds, Feedback |
| Tape Color | Tone, Wow/flutter depth, Single/Double/Triple heads, Free/1/8/3/16/1/4 sync |
| Phaser | On, Mix, Rate, Depth |
| Wavefold | On, Mix, Drive, Tone |

Tape time is 40–1000 ms. Turning Time switches to Free from the current synced
time; the knob therefore always changes what is heard. Tape feedback is
darkened, DC-filtered and softly saturated. Fractional reads, wow/flutter and
three available head configurations create changing repeats; its wet path is
12 kHz, intentionally bandwidth-limited. Digital delay remains independently
available. Chorus and phaser run at the full 44.1 kHz audio rate. Phaser uses
separate six-stage stereo sweeps. Wavefold shapes harmonics before bitcrush,
then the phaser and time/space buses. Scatter is upstream of these effects.

The room reverb remains a compact damped feedback network with added input
diffusion, running at 16 kHz. These are practical fixed-point effects, not
measured circuit emulations. Bitcrush and strong folding deliberately create
distortion. Smooth mixes and bounded feedback prevent parameter changes from
becoming uncontrolled integer overflows.

## Listening and compatibility

The collection contains 39 analog, 25 phase, 32 string, 30 modal, four four-op
FM and twenty FM6 patches. Factory levels are trimmed offline. The ten
before/after pairs play 1.6 then 1.7, matched in RMS over the held portion with
attenuation only. The separate effects demo plays dry then wet. The full
preset book retains the actual factory levels. These are listening aids, not
objective sound-quality scores.

Project versions 1–10 import without rewriting their slots. New effects get
transparent defaults on import; saved notes, patch values and Anchors survive.
The revised effects change wet playback, while browsing factory sounds loads
the new recipes. New saves use v11, which earlier firmware cannot read. Keep
spare slots and earlier firmware when retaining a rollback project.

The sound and timing still require physical FM-1 acceptance. Software routing
checks around G3 are retained and extended to FM6; the physical-key cause from
the earlier video remains unconfirmed. Source revisions, adaptations and full
licences are recorded in `assets/dsp/SOURCES.md` and `LICENSING.md`.
