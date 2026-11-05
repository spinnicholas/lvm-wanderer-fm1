# Wanderer 1.6 RC1: rebuilt instruments

This candidate replaces the modern sound renderer and revoices all 150 factory
patches. It is intended for listening on the FM-1 before publication. The 1.4
release remains the hardware-accepted fallback.

## Research and implementation

The implementation was informed by the actual FM-1 firmware sources in
[OberVave](https://github.com/zvenson/obervave/tree/099ccd6f30a83591cea3521efaec6d3546004036),
[Felucca-Engines](https://github.com/hugelton/Felucca-Engines/tree/0f015add84d77d8ae0ead08de901b7ecf3686f18)
and [S1Vave](https://github.com/AndrewGG88/S1Vave/tree/e1fa9aedf30ec5df635b0fac4c035d658e1c962f).
Felucca's MIT oscillator/phase routines and OberVave's GPL ladder are adapted
with their notices retained in `assets/dsp/`. S1Vave was a design reference;
no manufacturer patches or recorded samples were imported.

| Instrument | New implementation | Sound-design use |
| --- | --- | --- |
| Analog | PolyBLEP saw and pulse; three-saw stack; sine, triangle, reed and noise; resonant four-stage feedback ladder; independent filter envelope | Sub bass, acid, PWM, octave/fifth leads, brass, detuned ensembles |
| FM | Interpolated four-operator phase modulation, eight routings, individual smoothed envelopes, five operator waves, averaged feedback, pair balance and upper-pair octave control | Tine attack/body, electric keys, reeds, additive organs, inharmonic chimes |
| Phase | Eight waveforms on two detuned lines; saw, square, pulse, double sine, saw-pulse and three windowed resonant shapes; independent DCW envelope; blend or ring mode | Hollow bass, reed sustain, digital keys, animated resonances, glassy attacks |
| String | Fractional-delay feedback string with delay compensation, note/time-based loss, position-sensitive pluck, noise, bipolar hammer or soft harmonic excitation; body damping | Nylon, bridge wire, muted plucks, hammer/tine accents and long rings |
| Modal | Damped complex resonator modes with pitch-independent decay coefficients; bar, skin and bowl damping; strike position and hardness; pipe/organ held partials | Mallets, vessels, bowls, bells and additive sustained textures |

The new Color editor page exposes engine-specific model controls using the
same four knobs in normal and Zoom views. String Ring now displays time.
The bank uses 39 analog, 16 FM, 30 phase, 32 string and 33 modal recipes, in
fifteen groups of ten. Existing Scatter and effect settings stay under the
player's control when a preset changes.

## What caused convergence

The old shared saturation stage reduced differences between sources. The new
renderer applies drive only when requested. FM carriers and modulators now have
separate, interpolated contours and sustained roles retain a modulation body.
Phase patches use distinct waveforms and stronger, deliberately different DCW
settings rather than variations of one bend mixed with a sine. String presets
retain their designed pick positions. Modal decay uses the same time-constant
convention as the other envelopes; using T60 there prematurely removed upper
modes. Modal rotation coefficients are normalized so interpolation does not
produce a different decay at a particular key.

High operator ratios fade near Nyquist instead of folding down by an octave.
FM index and phase depth taper smoothly in the upper register. This reduces
abrupt pitch-dependent changes, but does not make nonlinear synthesis free of
all aliasing. A gentle master overload limiter has unity gain below its knee;
it does not change ordinary quiet signals. Factory levels are trimmed offline.

## Low G

The submitted video does not identify the selected preset, so it cannot prove
one exact cause. G3 is MIDI 55, local key index 2. Regression tests compare the
local performance route and MIDI at matching velocity/onset state, on all 27
keys through all five keyboard octaves and all four voice slots. A second pass
compares every factory patch across slots at F-sharp3, G3 and G-sharp3.
The supplied neighbour-note audio plays F3, F-sharp3, G3, G-sharp3 and A3 through
each engine. These checks can establish software consistency; the physical
touch sensor, audio electronics and particular preset from the video still
require confirmation on the device. No physical-key repair is claimed.

## Projects and limits

Versions 1–9 import without rewriting their original slots. Notes, settings,
patch values and Anchor snapshots are retained. All modern projects play
through the rebuilt engine and therefore sound different. The old v1–v5 legacy
renderer remains available. New saves use v10 and cannot be read by 1.5 or
earlier. Save into spare slots and keep earlier firmware for exact earlier
modern playback.

There are four voices, four FM operators, four modal partials and Q4 ratios.
This is a family of playable synthetic instruments, not a measured clone of a
particular acoustic instrument or vintage synthesizer. Strings use a finite
initial excitation, not continuous bowing or breath. The 4096-sample string
buffer cannot accurately tune the lowest five MIDI notes; the full physical
keyboard's octave range fits. Integer maths and offline tables keep the audio
path free of allocation and floating point. Software tests and emulation do
not replace physical audio and timing checks.
