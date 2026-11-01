# Wanderer 0.7: sound refinement

The original engine used a pure sine that Melt gradually replaced with a second harmonic. At high Melt, the note lost its fundamental. This revision holds the second harmonic between approximately 8 and 47 percent of the oscillator blend, so even a fully Melted phrase retains its pitch center. A one-pole low-pass contour rounds off high notes. The attack rises over several milliseconds and the release lasts roughly 70 ms at a typical captured velocity, allowing notes to meet naturally across the step grid.

A 16 KiB mono feedback buffer is placed in the existing pool section and read at unequal 94 ms and 142 ms taps. Those taps are mixed at low levels into left and right, making a narrow stereo field and a short tail. The delay is processed after the four voices, so it adds no per-voice buffer cost. Confirmation cues stay centered and fade over their final 256 samples. The voice mix has extra headroom for chords and feedback.

The engine remains an event-memory synthesizer, not an audio looper or resampler. Anchor still bypasses the evolving Melt amount; Layer, Drift, Decay and Branch keep their prior meanings. The screen and session storage format did not change.

On the host, the same 12.5-second scripted phrase reached 7,568 peak amplitude and had no samples near 16-bit clipping; a four-note maximum-level, maximum-Melt stress case reached 15,678 with no clipped samples. The stereo difference measured 217 RMS in the musical example, compared with zero for the earlier dual-mono engine. The maximum adjacent-sample jump in that example fell from 1,110 to 679. These are engineering checks, not proof of preference or hardware timing. Settings, new-session, tempo-restore, adversarial and boundary host scenarios passed. The audio callback and event-memory code passed AddressSanitizer and UndefinedBehaviorSanitizer.

The JieLi cross-compiler and AC79 SDK were unavailable, so no 0.7 FM-1 binary exists from this work. Check worst-case callback time, speaker balance and the real stereo output on a controlled device before distribution.
