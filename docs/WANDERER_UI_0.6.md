# Wanderer 0.6: visual refinement

This pass keeps the 0.5 hierarchy and controls. It makes the sixteen-step rings genuinely circular at the 240 × 240 framebuffer resolution, gives selected phrase and menu cards a consistent top and left accent, adds a quiet header rule across views, and gives each large switch a visible inset center. The full-width Layer card carries the same accent treatment when enabled.

The Zoom setting now uses a magnifier; reduced motion uses a paused orbit. Both are drawn from the same original 24-unit, thick-stroke symbol geometry as the Anchor, Memory, Layer and sun icons. The three menu rows still show the 64-pixel icon and the real state of a large switch. The user-facing symbol key and 0.5 controls remain in the companion documents.

The on-device display still contains only **WANDERER** as a word, plus parameter numbers. Settings remain available from SEL and close with HOME; playback and keyboard performance continue while it is open. No session format or audio behavior changed.

The host framebuffer was inspected for dark and light Home, menu selected and toggled states, Zoom, parameter, and confirmation views. Settings, UI tour, new-session, tempo-restore, boundary and adversarial host scripts passed. The host renderer and event-memory test also passed AddressSanitizer and UndefinedBehaviorSanitizer. The pinned JieLi cross-compiler and AC79 SDK are not available here, so there is no 0.6 FM-1 binary or physical-device validation.
