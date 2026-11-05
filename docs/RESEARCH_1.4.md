# Musical design references

Chase Bliss describes MOOD MKII as two cooperating channels, with short sampling/looping on one side and real-time spatial effects on the other. Its official page describes blending, smoothing and harmonized clock steps: https://www.chasebliss.com/mood-mkii

Hologram describes Microcosm as sampling, pitch-shifting, delay and looping, with rhythmic micro-loops and diffuse textures. The official product page and manual entry point are https://www.hologramelectronics.com/products/microcosm and https://www.hologramelectronics.com/pages/microcosm-manual

Julius O. Smith's *Spectral Audio Signal Processing*, Weighted Overlap Add, explains synthesis windows and complementary overlap conditions: https://www.dsprelated.com/freebooks/sasp/Weighted_Overlap_Add.html

Design inference for Wanderer: preserve hard stutters, but also offer bounded windowed fragments, complementary overlap, deliberate phrase rests, octave-related playback and an effects stage after slicing. The implementation is a small time-domain dual-head history reader, not an FFT/WOLA processor, transparent time stretcher, reverse-engineered pedal or copied proprietary algorithm.
