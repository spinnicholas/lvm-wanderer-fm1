/* SPDX-License-Identifier: GPL-3.0-only; the same numbered four-knob grammar everywhere. */
#define SETTINGS_PAGES 9u
#define SETTINGS_ACCESS_FEEL 8u
#define SETTINGS_MUSIC 4u
static const char * const SETTINGS_TITLES[SETTINGS_PAGES]={"ACCESSIBILITY","CHORUS","DELAY","REVERB","MUSIC","OUTPUT","ARPEGGIATOR","ARP FEEL","ACCESS FEEL"};
static const char * const SETTINGS_LABELS[SETTINGS_PAGES][4]={
 {"THEME","ZOOM","MOTION",""},{"ON","MIX","",""},{"ON","MIX","TIME","FEEDBACK"},
 {"ON","MIX","SIZE","DAMPING"},{"ROOT","SCALE","LENGTH","KEYBOARD"},
 {"LEVEL","CUES","SLOT",""},{"ON","PATTERN","RATE","OCTAVES"},{"GATE","SWING","LATCH","CLOCK"},
 {"DISPLAY","CUES","CUE LEVEL","STEADY"}};
static uint32_t settings_count(void){return settings_page==0u||settings_page==5u?3u:settings_page==1u?2u:4u;}
static void settings_press(void);
static void settings_adjust(uint32_t k,int32_t d);
static const char *settings_value(uint32_t k,char *buf);
static void screen_parameters(uint16_t fg,uint16_t pa,uint16_t guide);
