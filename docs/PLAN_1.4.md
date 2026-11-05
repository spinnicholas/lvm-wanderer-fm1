# Wanderer 1.4 implementation plan

Baseline: `2131515`, the complete working 1.3 RC4 source recovered from the previous session. Keep its partition, platform, loader, identity, Anchor ownership, Print arenas and recovery paths.

1. Improve synthesis at its source: continuous phase distortion, independently phased and balanced subtractive oscillators, useful sub bass, gentle nonlinear drive, smoothed control changes and a broader bank with more subtractive basses, pads, leads and keys. Compare production-DSP audio and levels, including high notes and four-voice chords.
2. Make Scatter phrase-aware and selectable: Mixed, Slices, Hops, Reverse, Tape, Cloud, Pitched and Fills. Keep grain size and amount and add edge smoothing on one consolidated Scatter page. Wet blend, tone and variation use conservative internal defaults. Use complementary crossfades, fractional reads, bounded tonal transpositions, downsampling protection and deliberate clean downbeats. Route before effects and retain protected source audio.
3. Remove Scatter from PLAY and consolidate all parameter editing around knobs 1–4 matching rows 1–4. PRESET changes pages, SELECT chooses a row, HOME returns. System, music, effects and arp use the same layout in normal and Zoom modes. Retain established Save/Load buttons.
4. Add an arpeggiator with up/down/bounce, endpoint repeats, double-note, played, random, converge/diverge and chord modes. Quarter through thirty-second notes, dotted/triplet rates, one to four octaves, gate, swing, latch and internal/USB/TRS clock. Record generated notes with gates, retain ownership across inputs, and release on panic, mode changes and MIDI stop.
5. Add positional/activity LED feedback and review real 240×240 renders in both themes and Zoom. Test migration, Undo, Print, both layers, malformed input, parameter extremes, callback-size invariance, held notes, long playback, target compilation, memory and boot stack limits.

Deliver a test candidate, source, production-DSP listening comparison, screenshots and an honest validation report. Physical FM-1 timing and listening remain the final acceptance step; do not publish this candidate automatically.
