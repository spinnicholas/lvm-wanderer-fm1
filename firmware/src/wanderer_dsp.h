/* SPDX-License-Identifier: GPL-3.0-only
 * Felucca-Engines oscillator, saturation and phase-distortion routines:
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hugelton Instruments, MIT.
 * OberVave trapezoidal ladder:
 * Copyright (C) 2026 zvenson, GPL-3.0-only.
 * Adapted for Wanderer; pinned sources and full notices in assets/dsp/.
 */
#pragma once
#include "wanderer_dsp_tables.h"
static inline int32_t wn_clampi(int32_t v,int32_t a,int32_t b){return v<a?a:v>b?b:v;}
static inline int32_t wn_sine_i(uint32_t ph)
{
    uint32_t i = ph >> 22;
    int32_t a = WN_SINE[i], b = WN_SINE[(i + 1u) & 1023u];
    return a + (((b - a) * (int32_t)((ph >> 7) & 0x7FFFu)) >> 15);
}
static inline int32_t wn_osc_sine(uint32_t ph) { return wn_sine_i(ph); }

/* polyBLEP residual (Q15) around a wrap of a phase accumulator */
static inline int32_t wn_blep(uint32_t ph, uint32_t inc)
{
    uint32_t d = inc >> 15;
    int32_t x;
    if (!d)
        return 0;
    if (ph < inc) {
        x = (int32_t)(ph / d);                       /* 0..32767 */
        return x + x - ((x * x) >> 15) - 32768;
    }
    if (ph > 0xFFFFFFFFu - inc) {
        x = -(int32_t)((0xFFFFFFFFu - ph) / d);       /* -32767..0 */
        return ((x * x) >> 15) + x + x + 32768;
    }
    return 0;
}

static inline int32_t wn_osc_saw(uint32_t ph, uint32_t inc)
{
    return ((int32_t)(ph >> 16) - 32768) - wn_blep(ph, inc);
}

static inline int32_t wn_osc_pulse(uint32_t ph, uint32_t inc, uint32_t pw)
{
    return (wn_osc_saw(ph, inc) - wn_osc_saw(ph + pw, inc)) >> 1;
}

static inline int32_t wn_osc_tri(uint32_t ph)
{
    int32_t s = (int32_t)(ph >> 15);                 /* 0..131071 */
    return s < 65536 ? s - 32768 : 98303 - s;
}

/* tanh(x / 32768) * 32767, linearly interpolated (a plain lookup gives
 * 256-unit steps, audible on quiet signals) */
static inline int32_t wn_softclip(int32_t x)
{
    int32_t a = x < 0 ? -x : x, i = a >> 8, y;
    if (i >= 256)
        y = WN_TANH[256];                           /* continuous with the table */
    else
        y = WN_TANH[i] + (((WN_TANH[i + 1] - WN_TANH[i]) * (a & 255)) >> 8);
    return x < 0 ? -y : y;
}


static inline int32_t wn_pd_cos(uint32_t ph16)             /* -cos, Q15, from the 16-bit PD phase */
{
    return -wn_sine_i(((ph16 & 0xFFFFu) << 16) + 0x40000000u);
}

/* one wave's bend for a block: break points and the slopes of the bent phase
 * (Q16 reciprocals), so a sample costs a multiply instead of a divide (the
 * bent phase comes out the same or 1 lower in 65536) */
typedef struct {
    uint32_t w, x, peak, rez;
    uint32_t k0, k1;
} wn_pd_t;

static inline uint32_t wn_pd_slope(uint32_t span, uint32_t len) { return (span << 16) / (len ? len : 1u); }

static void wn_pd_setup(wn_pd_t *b, uint32_t w, uint32_t dcw)
{
    uint32_t x;
    b->w = w;
    switch (w) {
    case 0:                                                  /* SAW */
        x = 32768u + ((dcw * 32767u) >> 16);
        b->k0 = wn_pd_slope(32768u, x);
        b->k1 = wn_pd_slope(32767u, 65535u - x);
        break;
    case 1:                                                  /* SQUARE */
        x = 32768u - ((dcw * 31120u) >> 16);
        b->k0 = wn_pd_slope(32768u, x);
        b->k1 = wn_pd_slope(32767u, x);
        break;
    case 2:                                                  /* PULSE */
        x = 65535u - ((dcw * 63487u) >> 16);
        b->k0 = wn_pd_slope(65535u, x);
        break;
    case 3:                                                  /* DOUBLE WN_SINE */
        x = 65535u - ((dcw * 49151u) >> 16);
        b->k0 = wn_pd_slope(65535u, x);
        b->k1 = wn_pd_slope(65535u, 65535u - x);
        break;
    case 4:                                                  /* SAW-PULSE */
        x = 65535u - ((dcw * 32767u) >> 16);
        b->peak = (x * (32768u + ((dcw * 29491u) >> 16))) >> 16;
        if (!b->peak)
            b->peak = 1;
        b->k0 = wn_pd_slope(32768u, b->peak);
        b->k1 = wn_pd_slope(32767u, x - b->peak);
        break;
    default:                                                 /* RESONANCE */
        x = 0;
        b->rez = (dcw * 44000u) >> 16;
        break;
    }
    b->x = x;
}

/* one sample at phase ph (16 bit): bipolar Q15 */
static int32_t wn_pd_wave(const wn_pd_t *b, uint32_t ph)
{
    uint32_t pd, x = b->x;
    switch (b->w) {
    case 0:                                                  /* SAW */
        pd = ph < x ? (ph * b->k0) >> 16 : 32768u + (((ph - x) * b->k1) >> 16);
        break;
    case 1:                                                  /* SQUARE */
        if (ph < x)
            pd = (ph * b->k0) >> 16;
        else if (ph < 32768u)
            pd = 32768u;
        else if (ph < 32768u + x)
            pd = 32768u + (((ph - 32768u) * b->k1) >> 16);
        else
            pd = 65535u;
        break;
    case 2:                                                  /* PULSE */
        pd = ph < x ? (ph * b->k0) >> 16 : 65535u;
        break;
    case 3:                                                  /* DOUBLE WN_SINE */
        pd = ph < x ? (ph * b->k0) >> 16 : ((ph - x) * b->k1) >> 16;
        break;
    case 4:                                                  /* SAW-PULSE */
        if (ph >= x)
            pd = 65535u;
        else if (ph < b->peak)
            pd = (ph * b->k0) >> 16;
        else
            pd = 32768u + (((ph - b->peak) * b->k1) >> 16);
        break;
    default: {                                               /* RESONANCE: windowed sine, rez = pitch of the core */
        uint32_t rp = (ph + ((ph * b->rez) >> 16) * 7u) & 0xFFFFu, win;
        int32_t core = (wn_pd_cos(rp) * 5) >> 3;
        if (b->w == 5)
            win = 65535u - ph;                               /* saw window */
        else if (b->w == 6)
            win = ph < 32768u ? ph << 1 : (65535u - ph) << 1; /* triangle window */
        else
            win = ph < 16384u ? ph << 2 : ph > 49152u ? (65535u - ph) << 2 : 65535u;   /* trapezoid */
        if (win > 65535u)
            win = 65535u;
        return (core * (int32_t)win) >> 16;                  /* |core| <= 20479: fits 32 bits */
    }
    }
    return wn_pd_cos(pd);
}



static inline int32_t wn_sat_tanh(int32_t x){return wn_softclip(x/2)*2;}
typedef struct { int32_t G, G2, G3, G4, k, inv, gin; } wn_lad_coef_t;   /* G.. Q12, k Q10 (4.0 = 4096 self-oscillates), inv gin Q12 */
typedef struct { int32_t s[4]; } wn_lad_state_t;

static inline void wn_lad_coef(wn_lad_coef_t *c, int32_t cut, int32_t k10)
{
    int32_t i, g, G;
    cut = wn_clampi(cut, 0, 136 << 8);
    i = cut >> 8;
    g = WN_SVF_G[i];
    if (i < 136)
        g += ((WN_SVF_G[i + 1] - g) * (cut & 255)) >> 8;
    G = (int32_t)(((uint32_t)g << 12) / (uint32_t)(4096 + g));
    c->G = G;
    c->G2 = (G * G) >> 12;
    c->G3 = (c->G2 * G) >> 12;
    c->G4 = (c->G3 * G) >> 12;
    c->k = k10;
    c->inv = (int32_t)((4096u << 12) / (uint32_t)(4096 + ((k10 * c->G4) >> 10)));
    c->gin = 4096 + 2 * k10;                   /* 1 + k/2: the bass stays, the loop is pushed harder as the resonance rises */
}

static inline __attribute__((always_inline)) int32_t wn_lad_step(const wn_lad_coef_t *c, wn_lad_state_t *s, int32_t in)
{
    int32_t G = c->G, *st = s->s, v, y, u;
    int32_t S1 = st[0] - ((G * st[0]) >> 12), S2 = st[1] - ((G * st[1]) >> 12);
    int32_t S3 = st[2] - ((G * st[2]) >> 12), S4 = st[3] - ((G * st[3]) >> 12);
    int32_t x = (in * c->gin) >> 12;
    int32_t acc = ((c->G3 * S1 + c->G2 * S2 + G * S3) >> 12) + S4;
    int32_t y4 = (int32_t)(((int64_t)(((c->G4 * x) >> 12) + acc) * c->inv) >> 12);   /* the loop's output, linear */
    u = wn_sat_tanh(x - ((c->k * wn_clampi(y4, -131072, 131071)) >> 10));
    v = ((u - st[0]) * G) >> 12; y = v + st[0]; st[0] = y + v;
    v = ((y - st[1]) * G) >> 12; y = v + st[1]; st[1] = y + v;
    v = ((y - st[2]) * G) >> 12; y = v + st[2]; st[2] = y + v;
    v = ((y - st[3]) * G) >> 12; y = v + st[3]; st[3] = y + v;
    return y;
}
