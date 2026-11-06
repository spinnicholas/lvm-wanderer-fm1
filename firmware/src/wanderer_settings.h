/* SPDX-License-Identifier: GPL-3.0-only; the same numbered four-knob grammar everywhere. */
#define SETTINGS_PAGES 16u
#define SETTINGS_ACCESS_FEEL 8u
#define SETTINGS_MUSIC 4u
#define SETTINGS_REVERB 3u
#define SETTINGS_OUTPUT 5u
#define SETTINGS_MIDI 14u
#define SETTINGS_LAYER 15u
static const char * const SETTINGS_TITLES[SETTINGS_PAGES]={"ACCESSIBILITY","CHORUS","DELAY","REVERB","MUSIC","OUTPUT","ARPEGGIATOR","ARP FEEL","ACCESS FEEL","BITCRUSH","TAPE ECHO","TAPE COLOR","PHASER","WAVEFOLD","MIDI","LAYER"};
static const char * const SETTINGS_LABELS[SETTINGS_PAGES][4]={
 {"THEME","ZOOM","MOTION",""},{"ON","MIX","RATE","DEPTH"},{"ON","MIX","TIME","FEEDBACK"},
 {"TYPE","MIX","SIZE","DAMPING"},{"ROOT","SCALE","LENGTH","KEYBOARD"},
 {"LEVEL","SLOT","CLICK","CLICK LEVEL"},{"ON","PATTERN","RATE","OCTAVES"},{"GATE","SWING","LATCH",""},
 {"DISPLAY","CUES","CUE LEVEL","STEADY"},{"ON","MIX","BITS","RATE HZ"},
 {"ON","MIX","TIME MS","FEEDBACK"},{"TONE","WOW","HEADS","SYNC"},{"ON","MIX","RATE","DEPTH"},{"ON","MIX","DRIVE","TONE"},{"INPUT","CHANNEL","ARP CLOCK","PROGRAM"},{"EDIT","PLAYBACK","PATCH",""}};
static uint32_t settings_count(void){return settings_page==SETTINGS_LAYER||settings_page==0u||settings_page==7u?3u:4u;}
static void settings_press(void);
static void settings_adjust(uint32_t k,int32_t d);
static const char *settings_value(uint32_t k,char *buf);
static void screen_parameters(uint16_t fg,uint16_t pa,uint16_t guide);

/* Presentation order is independent of stable internal page IDs. */
static const uint8_t SETTINGS_ORDER[SETTINGS_PAGES]={0,8,4,6,7,14,15,5,13,9,12,1,2,10,11,3};
static uint32_t settings_position(void)
{for(uint32_t i=0;i<SETTINGS_PAGES;i++)if(SETTINGS_ORDER[i]==settings_page)return i;return 0;}
static uint8_t settings_move(int32_t d)
{return SETTINGS_ORDER[((int32_t)settings_position()+d%(int32_t)SETTINGS_PAGES+(int32_t)SETTINGS_PAGES)%(int32_t)SETTINGS_PAGES];}
