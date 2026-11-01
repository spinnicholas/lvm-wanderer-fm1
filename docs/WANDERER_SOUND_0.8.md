# Wanderer 0.8: sound characters, delay and room

## The instrument

Three deliberately small sound characters share a four-voice engine: 1 is a warm, slowly opening pair of saw and hollow oscillators; 2 is a quicker glass tone; 3 is a more forward hollow/saw blend. They use original 256-point tables generated at five harmonic limits. The selected band keeps partials below Nyquist for the note's increment, and the table is interpolated between points. Every voice has two oscillators with a slight detune, a velocity-scaled amplitude contour, a decaying brightness contour, and a two-stage low-pass filter. Melt raises filter brightness without replacing the fundamental. Existing notes retain their sound character when PRESETS changes; newly triggered notes use the new character.

The effects are separate from the evolving note memory. Chorus uses two gently moving 16- and 21-ms reads of a 16 KiB line. Delay has a 128 KiB line, damped feedback capped at about 67 percent, a short right-channel offset, and four divisions: sixteenth, eighth, dotted eighth, quarter. It follows the current BPM. The stereo room is a four-line feedback delay network with unequal paths, damping and two short diffusion stages. The mix ramps when a switch or amount changes. The output has a gentle high-level knee. All effect buffers have fixed sizes in the FM-1 pool; no allocation occurs in the audio callback.

## Controls

Turn PRESETS on Home to cycle sounds 1–3. SEL opens settings; PRESETS switches between the original system page and the new effects page. SELECT picks one of the three large effect symbols. SEL switches Chorus, Delay or Reverb on/off. KNOB 4 changes its amount from 0 to 127. On Delay, ALGORITHM chooses division 1–4 and KNOB 3 adjusts feedback. On Reverb, ALGORITHM adjusts room size and KNOB 3 adjusts damping. HOME returns to performance. The screen continues to use WANDERER as its only word; the symbols and numeric values identify effects. Delay starts off, while subtle chorus and reverb start on.

Slot format 3 saves the sound and effect settings. Confirmed Load accepts old version 1 and 2 Wanderer slots at their original 152-byte length, validates them, and migrates them to the new defaults; a new slot is 160 bytes. No old SLOOP project is imported. Anchor, Layer, Melt, Drift and Decay retain their memory behavior.

## Research decisions

The goal is a coherent voice and effect path suited to Wanderer's evolving phrase, not an emulation or copy of any manufacturer's instruments. Sequential's Take 5 documentation describes variable oscillator shapes, envelopes, modulation and dedicated reverb/multi-effects. Korg documents a two-oscillator plus multi-engine voice, filter and simultaneous modulation/reverb/delay. Roland emphasizes expressive oscillator shaping and chorus as a defining part of the JUNO family. The FM-1 Felucca project confirms tempo delay and a dense stereo feedback reverb are practical on this board, and its release notes call out click-free send ramps and buffer clearing. DaisySP's delay-line source illustrates fixed storage and interpolated reads for embedded DSP. The code here uses its own fixed-point tables and effect implementation rather than importing those projects.

Primary sources: https://sequential.com/hybrid-analog/take-5-compact-polysynth/ ; https://www.korg.com/us/products/synthesizers/minilogue_xd/index.php ; https://www.roland.com/us/products/juno-x/support/ ; https://github.com/hugelton/Felucca/wiki/Features ; https://github.com/hugelton/Felucca/releases ; https://github.com/daisyaudio/DaisySP/blob/master/Source/Utility/delayline.h ; https://github.com/joakimeriksson/fm1-emu/blob/master/PLAN.md .

## Verification and limits

The 13.5-second host example exercises capture, Melt/Drift/Decay, switching on Delay, Layer, Anchor revisit and all three sounds. It peaked at 10,190/32,767 with no clipped samples; the maximum-Melt, maximum-level four-note stress render peaked at 23,298 with no clipping. A separate maximum-wet stress render peaked at 28,084 without clipping; its tail fell from 2,297 RMS at 3–4 seconds to 21 RMS at 8–9 seconds. The effect page, settings, new-session, tempo-restore, adversarial and boundary scripts passed. The renderer and audio callback passed ASan/UBSan, and slot migration and exact record sizes passed the event-memory test.

The 0.8 device package was cross-compiled with the pinned JieLi toolchain and AC79 SDK. Its real binary booted in fm1-emu and produced nonzero audio when a key was pressed; see `RELEASE_0.8.md`. Host and emulator tests cannot prove FM-1 audio interrupt headroom, speaker quality, physical controls or reliable flash writes. A controlled hardware trial is needed before calling the sound or performance finished. This engine remains a 16-step note-memory synth, not an audio looper, sampler or full replacement for a commercial polysynth.
