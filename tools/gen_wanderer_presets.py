#!/usr/bin/env python3
"""Compile original 1.1 designs to bounded fixed-point runtime coefficients."""
from pathlib import Path
import csv,math,sys
R=Path(__file__).resolve().parents[1]
def coeff(ms):return max(1,min(65000,round((1-math.exp(-6.907755/(max(.3,ms)*44.1)))*65536)))
def main(header,catalog):
    rows=list(csv.DictReader((R/'assets/wanderer_presets.csv').open()))
    assert len(rows)==100 and len({p['name'] for p in rows})==100
    out=['/* Original LVM 1.1 bank. Millisecond designs compiled offline. */','#pragma once','#define WAVE_SINE 3u','#define WAVE_TRI 4u','#define WAVE_PULSE 5u','typedef struct {','uint8_t osc1,osc2,mix,ratio,detune,sustain,cutoff,contour,fm,resonance,sub,drive,lfo_filter,vibrato,pwm,level;','uint16_t attack,decay,release,contour_decay,fm_decay;','uint32_t lfo_step;','} wm_preset_t;','static const wm_preset_t WM_PRESETS[WM_PRESET_COUNT]={']
    waves={'S':'WAVE_SAW','H':'WAVE_HOLLOW','G':'WAVE_GLASS','N':'WAVE_SINE','T':'WAVE_TRI','P':'WAVE_PULSE'}
    lines=['# Wanderer 1.1: 100 original presets','','PRESETS moves by one; SELECT jumps by ten. Durations below describe the amplitude envelope. Held notes sustain where sustain is nonzero. These are original synthesis designs, not sampled or exact brand replicas.','','|No.|Bank|Sound|Attack ms|Release ms|Sustain /127|','|---:|---|---|---:|---:|---:|']
    for p in rows:
        def n(k):return int(p[k])
        assert 0<=n('resonance')<=7 and 0<=n('sub')<=64 and n('pwm')<=48
        assert n('mix')<=127 and n('sustain')<=127 and 0<n('ratio')<=64
        assert 0<n('attack_ms')<=4000 and 0<n('release_ms')<=5000
        assert 0<n('cutoff')<=220 and n('contour')<=150 and n('fm')<=127
        keys='mix ratio detune sustain cutoff contour fm resonance sub drive lfo_filter vibrato pwm level'.split()
        vals=[waves[p['osc1']],waves[p['osc2']]]+[str(n(k)) for k in keys]
        vals += [str(max(1,round(8128*256/(n('attack_ms')*44.1))))]+[str(coeff(n(k))) for k in ['decay_ms','release_ms','contour_ms','fm_ms']]
        vals += [str(round(n('lfo_rate')*.05*2**32/44100))+'u']
        out.append('{'+','.join(vals)+'}, /* '+p['number']+' '+p['name']+' */')
        lines.append(f"|{p['number']}|{p['bank']}|{p['name']}|{p['attack_ms']}|{p['release_ms']}|{p['sustain']}|")
    out+=['};']
    Path(header).write_text('\n'.join(out)+'\n');Path(catalog).write_text('\n'.join(lines)+'\n')
    # TPT state-variable filter coefficients: g=tan(pi*f/fs), h=1/(1+g*(g+k)).
    # Resonance stays below self-oscillation. No transcendental calls in audio.
    g=[math.tan(math.pi*(40*(300**(i/220)))/44100) for i in range(221)]
    damp=[1.9-i*.22 for i in range(8)]
    table=['/* Original TPT coefficient tables: Q14. Generated offline. */','#pragma once','static const int32_t FILTER_G[221]={'+','.join(str(round(x*16384)) for x in g)+'};','static const int32_t FILTER_H[8][221]={']
    for k in damp:table.append('{'+','.join(str(round(16384/(1+x*(x+k)))) for x in g)+'},')
    table.append('};')
    (R/'firmware/src/wanderer_filter.h').write_text('\n'.join(table)+'\n')
    print('100 patch coefficients and stable resonant filter tables compiled')
if __name__=='__main__':main(*sys.argv[1:])
