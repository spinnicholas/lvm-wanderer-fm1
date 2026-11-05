#!/usr/bin/env python3
"""High-resolution sine, smooth filter, tuning and physical-decay coefficients."""
from pathlib import Path
import math
R=Path(__file__).resolve().parents[1]
def arr(kind,name,values):return 'static const '+kind+' '+name+'['+str(len(values))+']={'+','.join(map(str,values))+'};'
times=[max(1,round(8000**(i/127))) for i in range(128)]
h=['/* Generated offline; no floating point in the firmware. */','#pragma once',
 arr('int16_t','WN_SINE',[round(32767*math.sin(2*math.pi*i/1024)) for i in range(1024)]),
 arr('int32_t','WN_SINE_HI',[round((1<<30)*math.sin(2*math.pi*i/1024)) for i in range(1024)]),
 arr('int16_t','WN_TANH',[round(32767*math.tanh(i/128)) for i in range(257)]),
 arr('int32_t','WN_SVF_G',[round(4096*math.tan(math.pi*min(15000,8.1757989156*2**(i/12))/44100)) for i in range(138)]),
 arr('uint32_t','WN_CENTS',[round(65536*2**((i-127)/1200)) for i in range(255)]),
 arr('uint16_t','WN_RADIUS',[min(65532,round(65536*math.exp(-6.907755/(t*44.1)))) for t in times]),
 # Modal envelopes share the other envelopes' time-constant convention.
 # Using a T60 here made upper modes die almost seven times too quickly.
 arr('uint32_t','WN_RADIUS24',[min(16777215,round(16777216*math.exp(-1/(t*44.1)))) for t in times]),
 arr('uint16_t','WN_HZ',[round(min(15000,8.1757989156*2**((i*137/127)/12))) for i in range(128)]),
 arr('uint16_t','WN_LP_HZ',[round(8.1757989156*2**((25+(i*105*256//127)/256)/12)) for i in range(128)]),
 arr('uint16_t','WN_ATTACK_RATE',[min(65535,round(65536*(1-math.exp(-math.log(5)*16/(t*44.1))))) for t in times]),
 arr('uint16_t','WN_PD_DEPTH',[round(56000*min(1,700/(440*2**((n-69)/12)))) for n in range(128)]),
 arr('uint16_t','WN_FM_BAND',[round(32767*min(1,1600/(440*2**((n-69)/12)))) for n in range(128)])]
h.append(arr('uint32_t','WN_FX_INC',[round((2**32)*.03*(200**(v/127))/44100) for v in range(128)]))
h.append(arr('uint32_t','WN_SYNC_RATIO',[round(65536*2**(2*v/127)) for v in range(128)]))
# Loss per complete string round trip for the chosen note and T60, not per sample.
h+=['static const uint16_t WN_STRING_LOSS[128][128]={']
for note in range(128):
 period=min(4094,44100/(440*2**((note-69)/12)))
 h.append('{'+','.join(str(min(32766,round(32768*math.exp(-6.907755*period/(max(40,t)*44.1))))) for t in times)+'},')
h+=['};']
(R/'firmware/src/wanderer_dsp_tables.h').write_text('\n'.join(h)+'\n')
print('Interpolated sine, saturation, continuous cutoff, cents and physical decay tables generated')
