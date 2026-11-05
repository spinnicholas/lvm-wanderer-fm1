# Anticipated feedback and decisions

|Likely player response|Implemented refinement|Evidence|
|---|---|---|
|I can play a chord but the loop loses notes.|Four-note event cells and independent captured gates; every musical transform preserves all events.|Three-note capture/playback, staggered release, sustain, overflow and legacy import cases.|
|Overdubbing a long note erased my other music.|Only covered repetitions of the same pitch are removed.|Different-pitch overdub preservation regression.|
|I cannot tell whether this is saved or which project I am editing.|Persistent edited/saved/untitled symbol and actual assigned slot; dirty comparison against a committed reference.|Undo Save/Load checkpoints and screenshot states.|
|SAVE seems to do something different from its label.|Short-release Catch remains; 900ms hold opens actual project Save, with SEQ retained.|Unit and timed-navigation tests verify no intermediate Catch.|
|The success screen disappeared before I could inspect it.|Save/Load results persist and acknowledgment consumes its button action.|Sticky-result and navigation checks.|
|It said Save failed; did I lose the music?|Separate music commit and preferences outcome, and no clean indication after a preference fault.|Injected preference/slot-write failures.|
|I switched to Anchor and old notes kept sounding.|End recording and release the previous sequenced gates on a valid listening change.|Capture/playback tests and navigation assertions.|
|I accidentally pressed REC twice and lost Undo.|Create the recording Undo checkpoint only on the first captured note.|Empty-pass regression.|
|Lights from a different page are still on.|Persistent outcome/error pages receive complete LED scenes, retaining only actual audio/recording and current acknowledgment/context lights.|Status-scene mask assertions plus inherited 10000-transition checks.|
|The instrument is silent and I do not know why.|Persistent digital-zero speaker symbol and retained large level editor.|Both themes/sizes and master mute regression.|
|Can I go back to my previous firmware and open old music?|Retain recovery foundation and exact prior packages; import older layouts without rewriting; document that v5 needs 1.2.|Protected-source/loader/head comparison, OTA/installer simulations and format tests.|

Deliberately deferred: autosave, automatic startup loading, favorites, arpeggiator, extra effects and further DSP changes. They would add storage or control behavior without resolving the most immediate recording and trust problems. A stable label awaits physical audio, timing, screen, storage and rollback acceptance. Personal listening feedback is still needed to judge the existing 100-preset bank; automated audio measurements cannot establish taste.

## RC2 follow-through

Further playing review found two ordinary cases that needed correction: returning with HOME did not use the same gate release as LFO, and cyclic full-cell replacement could discard a newly retriggered note while an older one survived. New regressions fail against RC1 and pass with RC2. Released positions now yield before held positions. Capture-age comparison also remains chronological through counter wrap. A shared local-onset step prevents scheduler-boundary splits within one detected batch, without adding input delay or changing separate MIDI messages.
