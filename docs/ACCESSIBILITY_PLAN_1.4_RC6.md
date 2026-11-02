# Wanderer 1.4 RC6 accessibility plan

Add an Access Feel page to the left of Accessibility in the page cycle. Four knobs retain the same row mapping: Display, Cues, Cue Level and Steady.

Display offers two, five or ten seconds, plus Hold until dismissed, for temporary parameter and preset overlays and transient notices. Critical/saved-result notices keep their existing persistent behavior. Timers use rollover-safe comparisons, refresh when an overlay changes, and do not change the music scheduler.

Cues offers Off, Actions and Full. Full adds quiet range landmarks and rising/falling on/off patterns, with an independent cue level. Cue processing uses bounded sample-counted envelopes with a short attack and release, no allocation, and no voice stealing. Repeated clamped knob turns remain silent; identical rapid boundary cues are rate limited. Audio is added after Print and scope capture so cues do not enter the printed loop or waveform trace. Saving preferences does not occur while turning a control.

Steady keeps numeric Zoom lettering at a size that fits the parameter's supported display range, including time/frequency unit transitions and percent values. Text modes keep adaptive word fitting and wrapping. A preference allows the previous fully adaptive numeric behavior.

Reuse the two reserved preference bytes with an extension marker, retaining the original record size and project format. Old records receive explicit defaults; old firmware can still read the original preference fields and ignore the extension. Track new settings in the unsaved indicator and validate readback on explicit Save.

Verify timer durations/Hold/rollover, modal priority, old/new/malformed preferences, failed write reporting, audio cue patterns/volume/mute/priority/rate limits/callback segmentation, Print and scope isolation, stable numeric ranges and formatted units, screen bounds and shortcut hint placement. Finish with sanitizer regression, target memory/stack/loader audits, exact-package emulation and downloadable firmware/source/screens/audio evidence. Physical timing and listening remain device acceptance.
