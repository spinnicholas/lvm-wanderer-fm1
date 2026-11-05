# Wanderer 1.4 RC2 spacing correction

RC2 increases the normal settings cards from 43 to 48 pixels high and gives the label and value separate padded lines. Font sizes stay unchanged. The page indicator moves below the cards. Numbered Zoom buttons in settings, sound editing and Print gain extra vertical padding.

Only `wanderer_settings.c` and `wanderer_edit_screen.c` change firmware behavior from RC1, and only their drawing coordinates change. Synthesis, Scatter, arpeggiator behavior, project format and controls retain RC1 behavior.

Candidate SHA-256: `c1c34c11961cc1f05a7bb06536233b6d6ec83a7b174729110149c13fb451d5b1`. Package: 612,196 bytes. Identity: FM-1_929.

Fresh RC2 verification: target compilation passes flash, RAM, audio-pool and register-access limits; 296 Home/Scatter/settings production captures in dark/light and Zoom complete without screen-bound failures; all six representative screens visually inspected; 2,400 editor engine/page/control/extreme/theme/Zoom renders pass clipping and label-collision checks; 2,048 settings-range renders pass; all 33 protected platform/recovery source files and loader/package-head bytes match verified RC4; boot migration stack bound remains 2,964 / 4,096 bytes.

The broader 49-check regression, ten-minute DSP stress run, callback-size comparisons, audio demos and compiled-package emulation belong to RC1 and were not repeated in this drawing-only correction. RC1's report remains included as historical evidence. This candidate still needs physical FM-1 installation, listening and control testing. Start at a lower master level and save v8 projects into spare slots for rollback to RC4.
