/* SPDX-License-Identifier: GPL-3.0-only
 * RGB565 palettes: text and secondary labels stay bright on every dark surface.
 * Theme selection is a global preference, independent of project sound state.
 */
#define THEME_COUNT 6u
#define THEME_LIGHT 2u
static uint8_t color_theme;
typedef struct {const char *name;uint16_t bg,panel,memory,text,label,anchor,evolve,layer;} wm_theme_t;
static const wm_theme_t WM_THEMES[THEME_COUNT]={
 {"CONTRAST",RGB(0,0,0),RGB(5,5,5),RGB(8,8,8),RGB(255,255,255),RGB(235,235,235),RGB(180,255,238),RGB(224,224,255),RGB(255,234,170)},
 {"DARK",RGB(8,17,25),RGB(16,32,43),RGB(23,31,49),RGB(240,249,245),RGB(205,225,231),RGB(105,232,218),RGB(166,175,250),RGB(255,202,113)},
 {"LIGHT",RGB(238,246,246),RGB(255,255,255),RGB(247,247,255),RGB(13,29,35),RGB(48,69,78),RGB(8,89,90),RGB(49,64,139),RGB(139,72,8)},
 {"AMBER",RGB(10,7,2),RGB(20,14,4),RGB(24,17,5),RGB(255,246,224),RGB(245,226,184),RGB(255,211,107),RGB(255,241,188),RGB(255,191,166)},
 {"OCEAN",RGB(2,10,16),RGB(4,18,27),RGB(5,21,33),RGB(242,250,255),RGB(205,234,248),RGB(140,244,238),RGB(188,214,255),RGB(255,227,166)},
 {"VIOLET",RGB(9,5,16),RGB(17,10,27),RGB(23,13,34),RGB(252,246,255),RGB(231,218,245),RGB(220,192,255),RGB(162,238,255),RGB(255,218,171)}
};
static const wm_theme_t *theme_palette(void){return &WM_THEMES[color_theme<THEME_COUNT?color_theme:0];}
