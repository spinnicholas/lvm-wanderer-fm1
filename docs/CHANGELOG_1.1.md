# Wanderer 1.1 RC1 changes

Recording now stores held gate duration, including MIDI sustain, rather than replaying every onset with a fixed short gate. Gates are preserved through Anchor, evolution, branch, Undo and v4 project storage. Captures finish on recording/context transitions and are bounded to one loop.

Loops can contain sixteen or thirty-two sixteenth-note steps. The Music settings page contains root, eight scale choices and loop length. Shortening retains the second bar and length changes support Undo. Scale-bound Drift uses bounded degree moves in an eight-cycle phrase with periodic resolution. Performed notes are preserved. Melt has a smoothed resonant filter response.

The synth gains wide-range amplitude/filter/FM contours, exponential decay and release, a resonant topology-preserving lowpass, soft drive, sub oscillator, band-limited pulse-width modulation, velocity-sensitive timbre and patch-specific movement. The 100 original patches are redesigned around sustained pads/leads, focused basses and deliberate decaying keys/bells. Four voices and two monophonic event lanes remain.

The fourth settings page uses large note notation and scale interval geometry. Home shows both bars and held-duration lines. Layer mute releases its sequenced held gates while allowing release tails. Existing scope, earcons, accessibility settings, startup splash, error acknowledgment and LED context behavior remain.

Project version four imports historical v1/v2/v3 records without rewriting them on Load. Older firmware cannot load a newly saved v4 project. The 33 protected platform/recovery files, loader bytes and generated package head remain identical to 0.9. Identity FM-1_931 distinguishes this candidate from 1.0's FM-1_930. Physical acceptance is pending.
