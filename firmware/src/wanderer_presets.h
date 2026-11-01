/* Original LVM patch bank; generated from assets/wanderer_presets.csv. */
#pragma once
#define WAVE_SINE 3u
#define WAVE_TRI 4u
typedef struct {
    uint8_t osc1,osc2,mix,ratio,detune,attack,decay,sustain,release;
    uint8_t cutoff,contour,contour_decay,fm,fm_decay;
} wm_preset_t;
static const wm_preset_t WM_PRESETS[WM_PRESET_COUNT] = {
    {WAVE_SAW,WAVE_HOLLOW,38,16,7,8,4,92,2,30,64,5,0,1}, /* 001 Warm Wander */
    {WAVE_GLASS,WAVE_HOLLOW,26,16,2,72,12,58,4,100,40,12,0,1}, /* 002 Glass Lantern */
    {WAVE_HOLLOW,WAVE_SAW,54,16,12,28,8,80,3,58,52,7,0,1}, /* 003 Hollow Beacon */
    {WAVE_TRI,WAVE_SAW,35,16,4,3,3,104,1,65,30,3,0,1}, /* 004 Soft Current */
    {WAVE_SINE,WAVE_GLASS,20,16,0,26,9,85,3,170,12,18,0,1}, /* 005 Clear Tone */
    {WAVE_HOLLOW,WAVE_SINE,12,8,0,12,6,98,2,26,44,8,0,1}, /* 006 Dark Pulse */
    {WAVE_SAW,WAVE_TRI,34,16,5,20,3,107,2,81,55,5,0,1}, /* 007 Dawn Edge */
    {WAVE_GLASS,WAVE_SINE,17,32,1,80,24,0,7,122,38,56,0,1}, /* 008 Little Glow */
    {WAVE_TRI,WAVE_HOLLOW,30,24,2,14,5,82,3,50,38,6,0,1}, /* 009 Velvet Fifth */
    {WAVE_SAW,WAVE_GLASS,27,16,14,2,1,110,1,70,32,2,0,1}, /* 010 Blue Horizon */
    {WAVE_SAW,WAVE_SAW,51,16,18,3,3,109,2,86,52,5,0,1}, /* 011 Wide Saw */
    {WAVE_SAW,WAVE_HOLLOW,28,16,6,2,2,111,1,63,43,3,0,1}, /* 012 Chorus Silk */
    {WAVE_SAW,WAVE_TRI,40,16,12,1,2,112,1,46,66,2,0,1}, /* 013 Soft Strings */
    {WAVE_SAW,WAVE_HOLLOW,47,8,1,4,3,104,2,54,48,4,0,1}, /* 014 Subensemble */
    {WAVE_HOLLOW,WAVE_GLASS,33,16,13,1,2,114,1,66,24,3,0,1}, /* 015 Orbit Choir */
    {WAVE_SAW,WAVE_SAW,39,16,5,8,7,92,4,74,84,12,0,1}, /* 016 Poly Brass */
    {WAVE_HOLLOW,WAVE_HOLLOW,58,16,20,4,4,91,2,92,26,6,0,1}, /* 017 Starlit PWM */
    {WAVE_TRI,WAVE_SAW,27,8,3,1,2,105,1,41,36,2,0,1}, /* 018 Old Film */
    {WAVE_SAW,WAVE_GLASS,46,16,10,11,4,101,2,108,48,5,0,1}, /* 019 Golden Stack */
    {WAVE_HOLLOW,WAVE_TRI,25,32,1,70,5,119,2,93,16,4,0,1}, /* 020 Winter Organ */
    {WAVE_SINE,WAVE_SAW,33,16,3,80,6,120,4,121,25,8,0,1}, /* 021 Round Lead */
    {WAVE_TRI,WAVE_SAW,45,16,11,60,9,107,5,80,38,7,0,1}, /* 022 Velvet Lead */
    {WAVE_HOLLOW,WAVE_SINE,24,16,2,100,7,104,3,75,34,13,0,1}, /* 023 Reedy Lead */
    {WAVE_SAW,WAVE_SAW,48,16,6,120,5,116,3,131,68,8,0,1}, /* 024 Hot Saw */
    {WAVE_SAW,WAVE_TRI,36,32,2,115,12,106,4,123,48,7,0,1}, /* 025 Octave Torch */
    {WAVE_GLASS,WAVE_SINE,15,16,0,95,14,90,3,174,10,14,0,1}, /* 026 Thin Wire */
    {WAVE_TRI,WAVE_GLASS,20,16,4,45,11,114,6,91,18,4,0,1}, /* 027 Soft Solo */
    {WAVE_SAW,WAVE_HOLLOW,40,8,1,90,6,113,3,72,56,9,0,1}, /* 028 Sub Voice */
    {WAVE_SINE,WAVE_GLASS,31,32,1,67,18,87,4,145,27,22,16,8}, /* 029 Glass Lead */
    {WAVE_HOLLOW,WAVE_SAW,28,16,16,34,5,123,6,109,28,4,0,1}, /* 030 Siren Thread */
    {WAVE_SINE,WAVE_TRI,42,8,0,70,20,90,3,38,26,14,0,1}, /* 031 Round Sub */
    {WAVE_SAW,WAVE_HOLLOW,56,8,1,90,14,96,2,34,80,20,0,1}, /* 032 Thick Root */
    {WAVE_SAW,WAVE_TRI,22,16,4,100,36,49,4,43,90,43,0,1}, /* 033 Bouncy Low */
    {WAVE_HOLLOW,WAVE_SAW,32,8,3,80,43,38,4,27,85,34,0,1}, /* 034 Dark Rubber */
    {WAVE_SINE,WAVE_SAW,28,8,0,110,16,92,3,64,34,19,14,32}, /* 035 Sub Circuit */
    {WAVE_SAW,WAVE_GLASS,19,16,1,100,28,55,4,91,115,58,0,1}, /* 036 Acid Wink */
    {WAVE_TRI,WAVE_HOLLOW,32,8,0,46,24,65,3,28,40,18,0,1}, /* 037 Felt Bass */
    {WAVE_SAW,WAVE_SAW,49,8,2,110,7,100,3,62,47,9,0,1}, /* 038 Low Octave */
    {WAVE_SINE,WAVE_SINE,21,16,0,120,40,35,3,108,22,57,56,45}, /* 039 FM Thumb */
    {WAVE_HOLLOW,WAVE_GLASS,28,8,1,64,13,93,2,52,68,30,25,21}, /* 040 Deep Grain */
    {WAVE_GLASS,WAVE_TRI,35,16,2,110,42,0,6,95,75,56,0,1}, /* 041 Amber Pluck */
    {WAVE_SINE,WAVE_GLASS,42,32,0,95,18,22,5,180,10,31,20,40}, /* 042 Pearl Key */
    {WAVE_TRI,WAVE_GLASS,27,24,0,85,26,0,7,82,48,45,0,1}, /* 043 Thumb Harp */
    {WAVE_SINE,WAVE_SINE,16,48,0,120,48,0,8,183,8,32,70,63}, /* 044 Digital Kalimba */
    {WAVE_GLASS,WAVE_SINE,22,32,0,75,17,24,6,125,26,26,28,24}, /* 045 Soft Tines */
    {WAVE_SAW,WAVE_TRI,29,16,2,100,53,0,5,79,70,62,0,1}, /* 046 Pizz Sun */
    {WAVE_HOLLOW,WAVE_TRI,18,8,0,110,68,0,4,50,95,80,0,1}, /* 047 Wood Knock */
    {WAVE_SINE,WAVE_GLASS,31,16,0,96,24,26,5,112,25,41,32,36}, /* 048 Muted Electric */
    {WAVE_SINE,WAVE_SINE,20,56,0,110,30,0,9,190,9,29,90,62}, /* 049 Chime Pick */
    {WAVE_GLASS,WAVE_HOLLOW,30,32,0,100,38,12,5,155,35,55,12,33}, /* 050 Arcade Key */
    {WAVE_SINE,WAVE_SINE,15,48,0,115,23,0,9,184,0,1,74,28}, /* 051 Clear Bell */
    {WAVE_SINE,WAVE_SINE,12,56,0,110,14,0,12,205,0,1,104,16}, /* 052 Iron Bell */
    {WAVE_SINE,WAVE_SINE,23,16,0,65,14,28,7,171,0,1,36,28}, /* 053 Soft EP */
    {WAVE_SINE,WAVE_SINE,29,32,0,90,19,33,8,197,0,1,52,25}, /* 054 Shimmer EP */
    {WAVE_SINE,WAVE_SINE,9,64,0,120,52,0,7,150,0,1,64,70}, /* 055 Marimba Glass */
    {WAVE_SINE,WAVE_SINE,14,24,0,74,17,9,10,202,0,1,86,20}, /* 056 Inharmonic Air */
    {WAVE_SINE,WAVE_SINE,25,32,0,70,12,0,12,181,0,1,42,13}, /* 057 Sweet Chime */
    {WAVE_SINE,WAVE_SINE,16,64,0,120,80,0,8,215,0,1,98,92}, /* 058 Crystal Drops */
    {WAVE_SINE,WAVE_SINE,33,48,0,110,4,118,2,185,0,1,30,2}, /* 059 FM Organ */
    {WAVE_SINE,WAVE_SINE,18,40,0,115,34,0,6,195,0,1,69,48}, /* 060 Silver Mallet */
    {WAVE_GLASS,WAVE_SINE,26,48,1,48,9,86,6,158,24,16,32,15}, /* 061 Bit Shimmer */
    {WAVE_HOLLOW,WAVE_SINE,31,32,2,50,12,98,5,99,31,12,39,12}, /* 062 Phase Reed */
    {WAVE_SINE,WAVE_GLASS,31,24,0,16,5,109,5,143,17,6,48,5}, /* 063 Soft Mod */
    {WAVE_GLASS,WAVE_GLASS,39,32,8,36,11,100,4,164,30,21,0,1}, /* 064 Arc Light */
    {WAVE_SINE,WAVE_HOLLOW,34,48,0,75,12,76,8,128,22,23,54,14}, /* 065 Prism Stack */
    {WAVE_TRI,WAVE_GLASS,48,16,3,70,27,63,5,133,40,36,0,1}, /* 066 Polygon */
    {WAVE_SINE,WAVE_GLASS,25,40,0,2,4,100,2,190,12,4,62,8}, /* 067 Phase Cloud */
    {WAVE_GLASS,WAVE_SINE,37,24,0,104,33,32,5,178,14,41,36,44}, /* 068 Glass Machine */
    {WAVE_HOLLOW,WAVE_GLASS,35,32,6,55,18,84,5,106,33,26,10,10}, /* 069 Plastic Wind */
    {WAVE_SINE,WAVE_SINE,32,64,0,28,16,77,7,207,0,1,82,29}, /* 070 Orbit Spark */
    {WAVE_TRI,WAVE_SAW,37,16,13,1,1,116,1,42,50,2,0,1}, /* 071 Quiet Shore */
    {WAVE_SAW,WAVE_HOLLOW,46,16,19,1,1,121,1,58,35,2,0,1}, /* 072 Wide Mist */
    {WAVE_GLASS,WAVE_TRI,32,16,10,1,2,106,1,103,17,3,0,1}, /* 073 Glass Tides */
    {WAVE_HOLLOW,WAVE_SINE,23,16,5,1,1,117,1,60,38,2,0,1}, /* 074 Dusk Choir */
    {WAVE_TRI,WAVE_GLASS,28,8,2,2,1,109,1,48,27,3,0,1}, /* 075 Blue Tape */
    {WAVE_SINE,WAVE_GLASS,33,24,1,1,1,119,1,132,14,2,16,2}, /* 076 Slow Aurora */
    {WAVE_SAW,WAVE_TRI,42,16,14,1,1,127,1,79,56,1,0,1}, /* 077 Bloom Field */
    {WAVE_HOLLOW,WAVE_TRI,29,32,1,2,1,124,1,91,9,3,0,1}, /* 078 Cloud Organ */
    {WAVE_GLASS,WAVE_SINE,26,16,0,1,2,110,1,110,15,2,19,3}, /* 079 Moonlit Air */
    {WAVE_TRI,WAVE_SAW,25,24,7,1,1,118,1,70,43,2,0,1}, /* 080 Far Horizon */
    {WAVE_SAW,WAVE_HOLLOW,31,16,5,10,10,98,3,56,78,11,0,1}, /* 081 Soft Brass */
    {WAVE_SAW,WAVE_SAW,39,16,3,18,6,111,4,98,94,7,0,1}, /* 082 Open Horn */
    {WAVE_HOLLOW,WAVE_TRI,34,16,0,90,3,120,2,88,15,7,0,1}, /* 083 Reed Organ */
    {WAVE_HOLLOW,WAVE_SINE,26,16,0,75,8,112,3,61,41,15,0,1}, /* 084 Nasal Reed */
    {WAVE_SINE,WAVE_TRI,21,32,0,5,7,104,3,183,6,4,0,1}, /* 085 Air Flute */
    {WAVE_SAW,WAVE_HOLLOW,45,8,2,8,9,100,4,48,100,10,0,1}, /* 086 Dark Horn */
    {WAVE_GLASS,WAVE_HOLLOW,34,16,0,40,13,97,4,74,32,12,0,1}, /* 087 Soft Oboe */
    {WAVE_HOLLOW,WAVE_SAW,38,24,1,26,4,112,2,85,22,6,0,1}, /* 088 Harmonium */
    {WAVE_TRI,WAVE_SINE,24,16,0,20,10,81,3,102,16,12,0,1}, /* 089 Wood Flute */
    {WAVE_SAW,WAVE_GLASS,42,16,3,7,10,99,4,114,105,15,0,1}, /* 090 Golden Brass */
    {WAVE_SINE,WAVE_SINE,15,56,0,120,57,0,12,160,0,1,124,46}, /* 091 Copper Dust */
    {WAVE_SINE,WAVE_GLASS,22,40,0,28,20,83,7,180,14,24,105,21}, /* 092 Shifting Alloy */
    {WAVE_GLASS,WAVE_SINE,23,64,0,115,64,0,9,205,18,71,45,60}, /* 093 Shattered Glass */
    {WAVE_HOLLOW,WAVE_GLASS,41,24,9,16,8,115,3,78,76,8,20,8}, /* 094 Hollow Machine */
    {WAVE_SINE,WAVE_SINE,10,24,0,1,15,82,1,193,0,1,98,6}, /* 095 Tidal Bell */
    {WAVE_SINE,WAVE_SINE,17,56,0,2,5,111,1,176,0,1,118,3}, /* 096 Distant Metal */
    {WAVE_HOLLOW,WAVE_SINE,32,48,1,36,20,65,5,65,95,30,55,34}, /* 097 Warped Reed */
    {WAVE_GLASS,WAVE_GLASS,38,32,0,120,90,0,5,190,29,75,0,1}, /* 098 Pixel Pluck */
    {WAVE_SINE,WAVE_GLASS,29,24,0,3,6,122,1,166,14,5,68,8}, /* 099 Celestial Alloy */
    {WAVE_TRI,WAVE_SINE,37,16,11,1,3,106,1,76,29,5,24,12}, /* 100 Last Light */
};
