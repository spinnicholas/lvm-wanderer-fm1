"""Factory voicing pass. Keep the 1.x bank intact for saved projects."""
from copy import deepcopy
import json
import design_bank_1_5 as design
import design_bank_1_7 as previous

R = previous.R
MARKER = '/* Wanderer 2.0 factory bank */'


def bank():
    rows = deepcopy(previous.bank())
    trims = json.loads((R / 'assets/wanderer_levels_1_7.json').read_text())
    for index, row in enumerate(rows):
        p = row['patch']
        p['level'] = min(112, trims.get(row['name'], 90) + 7)
        engine = p['engine']
        group = index // 10
        if p['filter'] == 0 and engine != 5:
            p['filter'] = 4 if engine == 0 and group < 4 else 3
        if engine == 0:
            p.update(cutoff=min(122, max(60, p['cutoff'] + 9)),
                     resonance=min(80, p['resonance'] + 14),
                     envdepth=min(37, p['envdepth'] + 5))
            if p['wave'] in (0, 1, 5):
                p.update(detune=max(p['detune'], 8 + index % 11),
                         mix=max(37, min(93, p['mix'])), drive=max(12, p['drive']))
            if group < 2:
                p['sub'] = max(p['sub'], 32)
            if group in (7, 8, 9):
                p.update(lfo_dest=0, lfo_depth=10+index % 14, lfo_rate=20+index % 13)
        elif engine == 2:
            p.update(cutoff=max(p['cutoff'], 92), shape=min(114, p['shape'] + 12),
                     mix=max(p['mix'], 31), detune=max(p['detune'], 6 + index % 9))
        elif engine in (1, 5):
            p.update(cutoff=max(p['cutoff'], 106), shape=min(110, p['shape'] + 10))
        elif engine in (3, 4):
            p.update(cutoff=max(p['cutoff'], 108), drive=max(p['drive'], 9))
    rows = rows[20:30] + rows[:20] + rows[30:]
    rows[0]['name'] = 'AFTERGLOW'
    rows[0]['description'] = 'Detuned saw lead with octave support, fast LP24 envelope and a warm driven center.'
    rows[0]['patch'].update(wave=5, ratio=[16,16,16,16], sub=24, mix=62, detune=14,
                            filter=4, cutoff=90, resonance=57, envdepth=23, position=34,
                            drive=30, level=92, lfo_dest=0, lfo_depth=4, lfo_rate=31,
                            amp=design.env(7,440,112,390), fenv=design.env(4,310,52,280),
                            morph=[-25,26,0,8,25,21])
    edits = {
        'SINGING PULSE': ('PLASMA PULSE', dict(pulse=47, sub=34, mix=50, detune=13, drive=22, resonance=63, cutoff=94)),
        'ROUND WHISTLE': ('SOFT LASER', dict(shape=94, detune=10, mix=44, lfo_depth=7, resonance=53, cutoff=111)),
        'FIFTH BEACON': ('SKYLINE FIFTH', dict(wave=5, ratio=[16,24,16,16], mix=62, detune=12, sub=22, drive=27, cutoff=97)),
        'SOFT CHANT': ('AFTERIMAGE', dict(cutoff=111, resonance=28, drive=16, shape=79, lfo_dest=0, lfo_depth=14)),
        'NEON REED': ('VIOLET REED', dict(filter=3, cutoff=120, resonance=38, shape=81, lfo_depth=6)),
        'BENT RIBBON': ('FRACTAL LEAD', dict(shape=108, detune=19, cutoff=108, drive=21, mix=43)),
        'SYNC TORCH': ('ION TORCH', dict(shape=117, mix=91, detune=54, envdepth=23, drive=29, resonance=57)),
        'ROUND SUB': ('SUB CURRENT', dict(wave=3, cutoff=73, sub=49, drive=24, resonance=19)),
        'PULSE ANCHOR': ('PULSE PRESSURE', dict(pulse=44, mix=70, sub=54, drive=36, resonance=56, cutoff=78)),
        'SOFT REESE': ('NIGHT DRIVE', dict(wave=5, mix=70, detune=34, sub=45, cutoff=79, drive=31, resonance=34)),
        'SYNC SOLO': ('NEON SYNC', dict(shape=102, mix=103, cutoff=101, resonance=55, drive=24)),
        'PRISM LEAD': ('PRISM EDGE', dict(shape=106, mix=55, detune=11, cutoff=111, resonance=32)),
        'DUST CHORDS': ('DUSK CHORDS', dict(cutoff=87, resonance=55, drive=21, lfo_rate=28)),
        'GLASS POLY': ('PRISM KEYS', dict(cutoff=111, envdepth=16, resonance=49, mix=60)),
        'AMBER HORIZON': ('AMBER TIDE', dict(cutoff=92, resonance=39, lfo_depth=18, lfo_rate=20)),
        'PAPER SKY': ('SOFT STATIC', dict(wave=5, detune=29, mix=62, cutoff=86, lfo_depth=24)),
        'HOLLOW TIDE': ('SLOW SIGNAL', dict(shape=106, cutoff=101, detune=19, mix=61)),
        'FM SHIMMER': ('NIGHT SHIMMER', dict(shape=82, cutoff=118, filter=3, resonance=37, lfo_depth=9)),
        'GLASS TINES': ('CHROME TINES', dict(shape=88, cutoff=112, filter=3, resonance=32)),
        'BRIDGE GLASS': ('FROST PLUCK', dict(cutoff=113, drive=18, resonance=46)),
        'STONE CHIME': ('ORBIT CHIME', dict(cutoff=108, resonance=41, drive=16)),
    }
    for row in rows:
        if row['name'] in edits:
            row['name'], patch = edits[row['name']]
            row['patch'].update(patch)
    return rows


def main():
    rows = bank()
    path = R / 'firmware/src/wanderer_sound_data.h'
    original = path.read_text().split(MARKER)[0].rstrip()
    def c(value):
        return '{' + ','.join(map(c, value)) + '}' if isinstance(value, list) else str(value)
    names = ','.join(json.dumps(row['name']) for row in rows)
    patches = ',\n'.join(c(list(row['patch'].values())) for row in rows)
    path.write_text(f'{original}\n{MARKER}\nstatic const char *const WS_NAMES_2[WS_PRESET_COUNT]={{{names}}};\n'
                    f'static const ws_patch_t WS_PRESETS_2[WS_PRESET_COUNT]={{\n{patches}\n}};\n')
    (R / 'assets/wanderer_presets_2_0.json').write_text(json.dumps(rows, indent=2)+'\n')
    print(f'{len(rows)} Wanderer 2.0 factory patches; first preset {rows[0]["name"]}')


if __name__ == '__main__':
    main()
