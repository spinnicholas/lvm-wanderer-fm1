"""Wanderer 3.0 factory bank: give every sound its own Melt journey and movement.

The 2.0 bank used five engine-wide Melt (morph) templates and left 111 of 150
sounds without LFO motion. Melt now deepens loop by loop, so each sound gets a
Melt target that suits its family: leads and basses darken and thicken, keys
round off, ensembles and pads bloom open and widen, plucked and struck bodies
soften their damping and inharmonicity. Pads, strings, organs and winds gain a
slow filter breath; leads and brass a light vibrato. Names, levels and the
basic voicing are unchanged, and the 2.0 table stays in firmware for older
projects. Deterministic: run it again and the same bank results.
"""
import json
from pathlib import Path

R = Path(__file__).resolve().parents[1]
MARKER = '/* Wanderer 3.0 factory bank */'

# morph order: cutoff, shape/index, damping, pulse, drive, detune
FAMILY = {
    0: ('lead', [-22, 14, 0, 10, 18, 14]),
    1: ('bass', [-26, 10, 0, 8, 22, 6]),
    2: ('bass', [-20, 16, 0, 12, 20, 8]),
    3: ('keys', [-16, -10, 6, 6, 6, 10]),
    4: ('keys', [-18, -14, 10, 0, 4, 8]),
    5: ('organ', [-12, 10, 0, 6, 10, 16]),
    6: ('brass', [-18, 12, 0, 8, 16, 12]),
    7: ('strings', [14, 14, 0, 6, 4, 22]),
    8: ('pad', [16, 18, 0, 10, 4, 24]),
    9: ('pad', [12, 20, 0, 12, 8, 20]),
    10: ('pluck', [-12, 8, 26, 0, 6, 0]),
    11: ('pluck', [-14, 10, 22, 0, 8, 0]),
    12: ('mallet', [-10, 16, 20, 0, 4, 0]),
    13: ('bell', [-8, 22, 18, 0, 0, 0]),
    14: ('perc', [-16, 12, 24, 0, 10, 0]),
}
# LFO: (destination, depth range, rate range); destination 0 cutoff, 2 pitch
MOTION = {
    'lead': (2, (3, 5), (36, 46)),
    'brass': (2, (2, 4), (34, 42)),
    'organ': (0, (6, 12), (24, 34)),
    'strings': (0, (8, 16), (14, 26)),
    'pad': (0, (10, 20), (10, 24)),
    'keys': (0, (4, 8), (18, 28)),
}


def bank():
    rows = json.loads((R / 'assets/wanderer_presets_2_0.json').read_text())
    for index, row in enumerate(rows):
        p = row['patch']
        family, morph = FAMILY[index // 10]
        engine = p['engine']
        # A small, repeatable variation keeps neighbours distinct.
        wobble = [((index * 7 + k * 3) % 9) - 4 for k in range(6)]
        new = [max(-60, min(60, m + (w if m else 0))) for m, w in zip(morph, wobble)]
        if engine == 5:      # FM6: Melt opens the modulation index, not a pulse width
            new[3] = 0
        if engine in (3, 4):  # strings and modal bodies have no pulse or detune voice
            new[3] = new[5] = 0
        if engine == 1:
            new[1] = abs(new[1])
        p['morph'] = new
        if family in MOTION and p['lfo_depth'] == 0 and engine not in (3, 4):
            dest, depth, rate = MOTION[family]
            p['lfo_dest'] = dest
            p['lfo_depth'] = depth[0] + index % (depth[1] - depth[0] + 1)
            p['lfo_rate'] = rate[0] + (index * 5) % (rate[1] - rate[0] + 1)
        row['description'] = row.get('description', '')
        row['melt'] = family
    return rows


def main():
    rows = bank()
    path = R / 'firmware/src/wanderer_sound_data.h'
    original = path.read_text().split(MARKER)[0].rstrip()

    def c(value):
        return '{' + ','.join(map(c, value)) + '}' if isinstance(value, list) else str(value)

    patches = ',\n'.join(c(list(row['patch'].values())) for row in rows)
    path.write_text(f'{original}\n{MARKER}\n'
                    f'static const ws_patch_t WS_PRESETS_3[WS_PRESET_COUNT]={{\n{patches}\n}};\n')
    (R / 'assets/wanderer_presets_3_0.json').write_text(json.dumps(rows, indent=2) + '\n')
    moving = sum(1 for r in rows if r['patch']['lfo_depth'])
    templates = len({tuple(r['patch']['morph']) for r in rows})
    print(f'{len(rows)} Wanderer 3.0 patches; {templates} distinct Melt targets; {moving} with motion')


if __name__ == '__main__':
    main()
