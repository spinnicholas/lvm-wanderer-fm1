/* SPDX-License-Identifier: GPL-3.0-only
 * RGB565 palettes. Each musical element keeps its colour on every screen:
 * Anchor teal, Memory violet, Layer gold; coral and sky are page accents.
 * Text and labels stay >= 7:1 against every surface, accents >= 3:1
 * (tests/theme_contrast_test.py). Theme choice is a global preference.
 * 3.1: WANDERER (after the 1.2-1.4 look) is the default; Contrast is a choice.
 */
#define THEME_COUNT 6u
#define THEME_LIGHT 2u
static uint8_t color_theme;
typedef struct {const char *name;uint16_t bg,panel,memory,text,label,anchor,evolve,layer,coral,sky;} wm_theme_t;
static const wm_theme_t WM_THEMES[THEME_COUNT]={
 {"WANDERER",RGB(7,15,30),RGB(15,30,52),RGB(26,26,62),RGB(244,248,255),RGB(208,222,238),RGB(84,226,206),RGB(178,162,255),RGB(255,198,92),RGB(255,142,122),RGB(112,198,255)},
 {"CONTRAST",RGB(0,0,0),RGB(5,5,5),RGB(8,8,8),RGB(255,255,255),RGB(235,235,235),RGB(180,255,238),RGB(224,224,255),RGB(255,234,170),RGB(255,196,184),RGB(186,228,255)},
 {"LIGHT",RGB(238,246,246),RGB(255,255,255),RGB(247,247,255),RGB(13,29,35),RGB(48,69,78),RGB(8,89,90),RGB(49,64,139),RGB(139,72,8),RGB(160,44,24),RGB(12,82,150)},
 {"AMBER",RGB(10,7,2),RGB(20,14,4),RGB(24,17,5),RGB(255,246,224),RGB(245,226,184),RGB(255,211,107),RGB(255,241,188),RGB(255,191,166),RGB(255,168,128),RGB(240,226,190)},
 {"OCEAN",RGB(2,10,16),RGB(4,18,27),RGB(5,21,33),RGB(242,250,255),RGB(205,234,248),RGB(140,244,238),RGB(188,214,255),RGB(255,227,166),RGB(255,170,150),RGB(150,212,255)},
 {"VIOLET",RGB(9,5,16),RGB(17,10,27),RGB(23,13,34),RGB(252,246,255),RGB(231,218,245),RGB(220,192,255),RGB(162,238,255),RGB(255,218,171),RGB(255,170,192),RGB(172,222,255)}
};
static const wm_theme_t *theme_palette(void){return &WM_THEMES[color_theme<THEME_COUNT?color_theme:0];}
