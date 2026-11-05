# Wanderer 1.3 RC3 — Scatter and supplied splash

Knob 3 is now Scatter. It makes weighted pseudorandom choices between stutters, short hops, earlier-fragment jumps, holds, accelerating slices and brief gaps. Gesture rates, timing offsets and breathing room vary with the amount. Each transport start seeds a fresh sequence. Zero fades back to the normal loop in about a millisecond.

PLAY offers Mixed, Slices and Hops. Scatter uses audio history in event playback and reads fragments from the current audio generation in Print playback. The underlying transport, performed notes and held gates keep advancing normally, so a gesture reconnects with the correct point in the loop. It never directly rewrites Memory or Anchor. An explicit Print can bake the effect into a new generation, with the existing Undo/Redo and protected audio Anchor retained. Reverse fragments are not included.

The supplied LVM/Wanderer artwork replaces the boot splash. Its full aspect ratio, colors and black background are retained without cropping. Native production-renderer captures in both themes match the RGB565 reference pixel for pixel. Startup timeout and dismissal behavior are unchanged. No new control icons were added; the existing licensed symbol workflow is retained.

The RC2 boot fix, which the user confirmed works and is tested, is retained. The actual target stack audit still measures a 20-byte scanner frame and a conservative 2,916-byte boot migration chain. Safety guards remain enabled.

47 fresh compile/execution/protocol checks pass. Dedicated Scatter tests cover all six gestures and clean gaps, low/high density, held-note state equality, zero/panic, bounded output, unmodified note/audio Anchors, Print/Undo/Redo and identical results across callback sizes with a fixed seed. A sub-LSB release-state bookkeeping corner exposed by the timing test was corrected so release completion is callback-size independent.

Final linked regions: application 394,400 / 581,564 bytes; ordinary RAM 57,712 / 98,304; large-buffer pool 315,392 / 344,064. No new large audio pool buffer was added. Package size 612,196 bytes; permanent identity FM-1_929. SHA-256: f6b4f7a3790c75f7de0c79d31be2d69239346fb0dcf5952b14077e565667fdb0.

The audition is 16 seconds: 0–4 clean, 4–8 gentle Scatter, 8–16 dense Scatter. It is rendered by the production DSP, not recorded from a physical FM-1. Printed audio remains RAM-only. The exact RC3 candidate requires physical confirmation; RC2's hardware result does not automatically validate the new bytes.

The exact final package completed 6,500 ms in fm1-emu with the new splash, four FM voices, Scatter at maximum, Print, Undo/Redo and Stop. Average emulated ISR load was 16.9%; that is not a physical worst-case deadline measurement. Five actual LCD captures were inspected.
