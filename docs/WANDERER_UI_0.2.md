# Wanderer 0.2 screen refinement

The 240 × 240 display is rendered as four independent 240 × 60 DMA bands. The header gives the instrument or active parameter, with its status or value below. Anchor and Memory each show a named phrase, number of occupied steps, a 16-step guide, and a pitch contour for captured events. Layer has a compact phrase rail and a full-width action or confirmation label. The current step uses a vertical marker when transport runs. Both palettes retain high-contrast text and separate bands.

The glyphs remain antialiased Atkinson Hyperlegible from the bundled SIL OFL font. The display uses the same band buffer and adds no animation, full framebuffer, or audio-thread work. Pitch positions are clamped to the C3–C6 display range; stored and played notes are unchanged. Empty steps have faint guides and captured steps have solid five-pixel marks. Reduced motion still avoids decorative movement.

## Verification

The host-compiled app rendered the tour, maximum parameter values, light theme, large-text toggle, new-session confirmation, tempo restore, and adversarial save scenarios without triggering its text-bound assertions. The event-memory unit test passed under AddressSanitizer and UndefinedBehaviorSanitizer. The PNG previews are actual host framebuffer captures at 2× nearest-neighbor scale.

This 0.2 source revision was not cross-compiled or run on the native instruction emulator because the pinned JieLi toolchain and AC79 SDK were not present in this session. It was not flashed or inspected on physical FM-1 hardware. The 0.1 hardware-untested package is an earlier binary and does not contain this screen.
