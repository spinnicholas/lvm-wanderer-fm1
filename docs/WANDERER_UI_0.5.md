# Wanderer 0.5: settings and Zoom

The 0.4 home design remains the default: two large phrase orbits, a full-width Layer card, and bold full-screen parameter and confirmation states. The 0.5 revision adds a settings screen and makes the previously ineffective size option useful.

Press **SEL** from the home or parameter view to open settings. The three 64-pixel symbols are sun (light palette), overlapping sizes (Zoom), and a stillness symbol (reduced motion). Their large switches show the actual values: knob left means off, knob right means on. A colored rail identifies the selected row. Turn the **SELECT encoder** to move between rows, press **SEL** to toggle, and press **HOME** to close. PLAY and keyboard performance continue while the menu is visible. HOME closes the menu without resetting the current Anchor/Memory listening mode. A confirmation still gives SEL its confirmation meaning.

Zoom replaces the two phrase cards with one larger view of the phrase currently heard. It has a 96-pixel Anchor or Memory symbol, a wide sixteen-step orbit, and a small Layer state strip. The default remains the paired 64-pixel home view. Outside settings, the SELECT encoder is a Zoom shortcut, and PRESETS remains a palette shortcut. The reduced-motion option has moved from a direct SEL toggle into settings.

Settings are part of the session state, as before. Explicit slot Save while stopped stores them with the phrase. Toggling a switch never starts a flash write during playback. A new session resets them to dark palette, paired view and motion enabled. An older experimental slot may load with its saved size flag on, which now selects Zoom; turn it off in settings to return to the paired view.

The menu contains no new words on the display. WANDERER remains the only on-device word; numeric values remain on parameter screens. The companion `WANDERER_UI_0.4.md` describes the base design and `WANDERER_UI_0.3.md` has the full symbol key. The settings icons are original assets generated from the same thick-stroke geometry as the other symbols.

The FM-1 host tour, settings navigation and toggle script, new-session, tempo-restore and adversarial scripts rendered without text-bound errors. The host application and event-memory tests passed AddressSanitizer and UndefinedBehaviorSanitizer. This source was not cross-compiled or physically tested because the pinned JieLi toolchain and AC79 SDK were unavailable in this session. The 0.1 binary does not contain this UI.
