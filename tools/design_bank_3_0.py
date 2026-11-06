"""Wanderer 3.0 factory bank: 150 new sounds in 15 families of ten.

Leads, basses, modern basses, poly keys, pads, voices and textures are written
fresh, many on the 3.0 SUPER (seven-voice stereo unison) and VOICE (formant)
engines. FM, FM6, string and modal voices start from the tested 2.0 physical
recipes and are revoiced (envelopes, tone, motion, Melt). Every sound has its
own Melt journey (morph: cutoff, shape, damping, pulse, drive, detune), chosen
so that wandering Memory changes timbre musically.

Levels come from assets/wanderer_levels_3_0.json, written by
tools/level_bank_3_0.py from rendered audio. The 2.0 bank stays in firmware for
older projects. Deterministic.
"""
import copy
import json
from pathlib import Path

import design_bank_1_5 as d

R = Path(__file__).resolve().parents[1]
MARKER = '/* Wanderer 3.0 factory bank */'
LP12, LP24, LADDER, BAND = 3, 4, 0, 1
SUPER, VOICE = 6, 7
OLD = {r['name']: r['patch'] for r in json.loads((R / 'assets/wanderer_presets_2_0.json').read_text())}


def env(a, dcy, s, r):
    return d.env(a, dcy, s, r)


def base(engine, amp, **kw):
    p = d.base(engine, amp)
    p.update(filter=LP24, cutoff=96, resonance=18, envdepth=20, drive=10, level=96,
             fenv=env(4, 420, 40, 300), morph=[0, 0, 0, 0, 0, 0])
    p.update(kw)
    return p


def sup(wave, cut, amp, *, spread=60, voices=96, width=90, sub=0, air=0, res=20,
        depth=20, fenv=(4, 500, 50, 400), filt=LP24, drive=12, pulse=64, track=40, **kw):
    return base(SUPER, amp, wave=wave, cutoff=cut, detune=spread, mix=voices, shape=width,
                sub=sub, inharm=air, resonance=res, envdepth=depth, fenv=env(*fenv),
                filter=filt, drive=drive, pulse=pulse, position=track, **kw)


def voc(vowel, cut, amp, *, src=0, formant=64, focus=70, blend=110, breath=10, choir=40,
        sub=0, pulse=64, depth=8, drive=8, **kw):
    return base(VOICE, amp, wave=src, shape=vowel, position=formant, feedback=focus, mix=blend,
                inharm=breath, detune=choir, sub=sub, pulse=pulse, cutoff=cut, filter=LP12,
                resonance=10, envdepth=depth, drive=drive, **kw)


def ana(wave, cut, amp, *, res=20, depth=24, fenv=(3, 380, 30, 300), mix=60, detune=10,
        sub=0, pulse=64, shape=0, ratio=16, filt=LP24, drive=12, track=40, noise=0, **kw):
    p = base(0, amp, wave=wave, cutoff=cut, resonance=res, envdepth=depth, fenv=env(*fenv),
             mix=mix, detune=detune, sub=sub, pulse=pulse, shape=shape, filter=filt, drive=drive,
             position=track, inharm=noise, **kw)
    p['ratio'] = [16, ratio, 32, 48]
    return p


def pd(bend, cut, amp, *, wave=0, line2=1, mix=50, detune=8, depth=20, fenv=(3, 400, 30, 300), **kw):
    return base(2, amp, wave=wave, algorithm=line2, shape=bend, mix=mix, detune=detune,
                cutoff=cut, envdepth=depth, fenv=env(*fenv), filter=LP12, **kw)


def old(name, **kw):
    p = copy.deepcopy(OLD[name])
    p.update(kw)
    p['_from'] = name
    return p


def motion(p, dest, depth, rate):
    p.update(lfo_dest=dest, lfo_depth=depth, lfo_rate=rate)
    return p


# Melt journeys by family: cutoff, shape, damping, pulse, drive, detune.
MELT = {
    'lead': [-22, 18, 0, 10, 16, 18], 'bass': [-26, 10, 0, 8, 24, 6],
    'reese': [-18, 22, 0, 10, 18, 26], 'poly': [-14, 16, 0, 8, 6, 18],
    'keys': [-18, -12, 10, 0, 4, 6], 'organ': [-12, 12, 0, 6, 10, 14],
    'voice': [-8, 40, 0, 6, 4, 16], 'ensemble': [12, 20, 0, 6, 4, 24],
    'pad': [16, 24, 0, 10, 4, 26], 'texture': [10, 30, 6, 14, 8, 22],
    'pluck': [-12, 8, 26, 0, 6, 0], 'string': [-14, 10, 22, 0, 8, 0],
    'mallet': [-10, 16, 20, 0, 4, 0], 'bell': [-8, 22, 18, 0, 0, 0],
    'perc': [-16, 12, 24, 0, 10, 0],
}
FAMILY = ['lead', 'bass', 'reese', 'poly', 'keys', 'organ', 'voice', 'ensemble', 'pad', 'texture',
          'pluck', 'string', 'mallet', 'bell', 'perc']


def bank():
    S, V = sup, voc
    rows = [
        # 0 Leads
        ('AFTERGLOW', S(0, 92, (6, 420, 112, 420), spread=46, voices=86, width=80, sub=24, res=40, depth=24, drive=24)),
        ('SOLAR WIRE', motion(ana(8, 102, (2, 300, 110, 260), res=30, shape=70, depth=30, drive=22), 2, 3, 40)),
        ('GLASS SIREN', motion(S(3, 108, (8, 600, 108, 500), spread=40, voices=70, width=70, res=24), 2, 4, 42)),
        ('CANDLE LEAD', motion(ana(0, 88, (4, 400, 100, 300), res=46, mix=50, detune=12, depth=30, drive=18), 2, 3, 38)),
        ('HUMMING BIRD', motion(V(108, 112, (20, 500, 110, 400), formant=80, focus=90, choir=0, blend=120), 2, 5, 44)),
        ('NEON RIBBON', motion(pd(96, 110, (3, 400, 100, 320), wave=4, line2=1, mix=60, detune=14), 2, 3, 40)),
        ('BRIGHT COMET', S(1, 104, (2, 350, 104, 300), spread=34, voices=80, width=60, pulse=40, res=34, drive=20)),
        ('KITE STRING', motion(ana(4, 110, (10, 300, 112, 380), res=10, mix=40, detune=6, depth=10, sub=20), 2, 5, 34)),
        ('SILVER WHISTLE', motion(ana(3, 116, (16, 300, 116, 500), res=6, mix=30, ratio=32, depth=6), 2, 4, 46)),
        ('STORM LEAD', S(2, 96, (2, 380, 108, 300), spread=70, voices=100, width=96, res=38, depth=30, drive=40)),
        # 1 Basses
        ('DEEP HARBOR', ana(3, 70, (2, 500, 118, 200), res=8, sub=70, depth=10, mix=20, ratio=8, drive=26, filt=LP12)),
        ('RUBBER TIDE', ana(0, 62, (1, 260, 80, 160), res=60, depth=46, fenv=(1, 220, 0, 120), filt=LADDER, sub=30, drive=20)),
        ('FELT FLOOR', ana(2, 66, (2, 420, 100, 180), res=14, pulse=50, depth=22, sub=40, filt=LP24)),
        ('SUB MOTH', ana(4, 74, (3, 600, 110, 220), res=6, sub=80, depth=8, mix=30, filt=LP12)),
        ('PLUCK HULL', old('PLUCK ROOT')),
        ('WOOD DOCK', old('WOOD KNOCK')),
        ('GROWL KEEL', pd(110, 82, (1, 300, 90, 160), wave=5, line2=2, mix=40, detune=4, depth=40, fenv=(1, 260, 10, 120))),
        ('ACID SAND', ana(0, 54, (1, 200, 60, 120), res=96, depth=70, fenv=(1, 180, 0, 90), filt=LADDER, sub=10, drive=34)),
        ('LADDER ROPE', ana(1, 68, (1, 340, 96, 160), res=40, depth=34, filt=LADDER, sub=36, detune=6)),
        ('ORBIT BASS', old('ANGLED BASS')),
        # 2 Modern basses
        ('NIGHT REESE', motion(S(0, 64, (3, 500, 116, 220), spread=34, voices=110, width=50, sub=60, res=24, depth=16, drive=34, filt=LP24), 0, 8, 18)),
        ('WIDE HULL', S(0, 72, (2, 420, 110, 200), spread=52, voices=90, width=110, sub=70, res=18, depth=24, drive=24)),
        ('DUSK REESE', motion(S(2, 70, (4, 600, 116, 260), spread=40, voices=100, width=70, sub=50, res=30, drive=30), 0, 12, 14)),
        ('FM WOBBLE', motion(old('COPPER GROWL'), 0, 22, 30)),
        ('SYNC DIVE', ana(8, 76, (1, 300, 100, 160), res=34, shape=96, depth=44, fenv=(1, 260, 10, 120), sub=40, drive=28)),
        ('PULSE RUNNER', ana(2, 70, (1, 220, 70, 120), res=36, pulse=36, depth=40, fenv=(1, 200, 0, 100), sub=30)),
        ('SQUARE DEPTH', ana(2, 64, (2, 500, 112, 180), res=12, pulse=64, sub=64, depth=14, filt=LP12)),
        ('REED KEEL', ana(6, 72, (2, 400, 104, 180), res=22, depth=26, sub=40, drive=24)),
        ('VOX BASS', V(112, 74, (4, 400, 110, 180), formant=40, focus=80, blend=100, sub=70, choir=10)),
        ('DISTANT ENGINE', motion(S(1, 60, (6, 700, 118, 300), spread=60, voices=110, width=90, sub=60, pulse=30, res=40, drive=40), 1, 30, 10)),
        # 3 Poly
        ('SOFT POLY', S(0, 84, (10, 700, 100, 600), spread=40, voices=80, width=96, res=16, depth=18, drive=8)),
        ('DUSK CHORDS', motion(S(1, 86, (6, 600, 96, 520), spread=44, voices=90, width=100, pulse=48, res=22), 1, 20, 22)),
        ('PRISM POLY', ana(1, 96, (4, 500, 96, 500), res=24, detune=14, depth=26, drive=10)),
        ('PAPER POLY', pd(70, 98, (3, 450, 80, 420), wave=1, line2=4, mix=60, detune=12)),
        ('VELVET STAB', S(0, 90, (1, 320, 60, 280), spread=50, voices=100, width=96, res=30, depth=40, fenv=(1, 200, 0, 200))),
        ('BRASS POLY', ana(0, 82, (20, 500, 100, 380), res=20, depth=46, fenv=(30, 400, 50, 300), detune=10, drive=16)),
        ('CHORD CLOUD', motion(S(3, 96, (30, 900, 100, 900), spread=60, voices=100, width=120, res=14, depth=10), 0, 12, 18)),
        ('GLASS POLY', S(3, 110, (4, 600, 90, 600), spread=30, voices=70, width=80, res=26, depth=16)),
        ('HOUSE STAB', ana(1, 78, (1, 200, 20, 220), res=40, depth=50, fenv=(1, 160, 0, 160), detune=16, drive=20)),
        ('WARM KEYS', ana(0, 80, (3, 900, 60, 500), res=12, depth=30, fenv=(2, 700, 20, 400), detune=8, mix=50)),
        # 4 Keys and tines (2.0 FM / modal recipes, revoiced)
        ('SATIN EP', old('ROUND EP')),
        ('RAIN TINES', old('CHROME TINES')),
        ('BELL EP', old('FELT TINES')),
        ('REED EP', old('REED KEYS')),
        ('SILK CLAV', old('HAMMER CLAV')),
        ('HAMMER KEYS', old('STAGE PIANO')),
        ('CELESTE NIGHT', old('SILVER CELESTA')),
        ('FELT UPRIGHT', old('WOOD IVORY')),
        ('KALIMBA KEYS', old('NYLON KEYS')),
        ('TOY PIANO', old('PAPER TINES')),
        # 5 Organs
        ('CHAPEL DRAWBAR', old('DRAWBAR LIGHT')),
        ('SMOKE ORGAN', motion(old('DRAWBAR DEEP'), 0, 8, 30)),
        ('REED ORGAN', old('ROUND HARMONIUM')),
        ('PIPE MIST', old('DIGITAL PIPE')),
        ('PULSE ORGAN', old('PULSE ORGAN')),
        ('HOLLOW CHAPEL', old('HOLLOW ORGAN')),
        ('AIR ORGAN', motion(S(3, 104, (12, 400, 120, 300), spread=16, voices=60, width=60, res=6, depth=0), 2, 2, 50)),
        ('ACCORDION SUN', old('WAX ACCORDION')),
        ('PHASE CHAPEL', old('PHASE REED')),
        ('STREET ORGAN', ana(6, 100, (6, 300, 116, 200), res=10, depth=4, mix=50, ratio=32)),
        # 6 Voices (VOICE engine)
        ('AAH CHOIR', motion(V(0, 104, (120, 900, 116, 900), choir=60, focus=70), 2, 3, 30)),
        ('OOH CHOIR', motion(V(126, 96, (140, 900, 116, 900), choir=60, focus=80), 2, 3, 28)),
        ('EE CHOIR', motion(V(64, 108, (120, 900, 116, 900), choir=60, focus=70, formant=70), 2, 3, 32)),
        ('VOWEL PAD', motion(V(40, 100, (300, 1200, 116, 1400), choir=80, focus=60), 1, 40, 12)),
        ('WHISPER', V(20, 110, (200, 800, 100, 1200), src=3, breath=90, choir=30, focus=50, blend=120)),
        ('MONK CHANT', V(100, 86, (160, 900, 120, 1000), formant=30, focus=90, choir=70, sub=40)),
        ('SOFT SOPRANO', motion(V(10, 112, (90, 700, 112, 700), formant=96, focus=96, choir=20), 2, 6, 44)),
        ('ROBOT VOWEL', motion(V(32, 100, (2, 300, 110, 200), src=1, pulse=40, focus=110, choir=0), 1, 50, 30)),
        ('BREATH FLUTE', motion(V(118, 116, (40, 500, 110, 500), src=3, breath=70, formant=100, choir=0, focus=60), 2, 4, 46)),
        ('SINGING SAW', motion(V(80, 108, (60, 600, 116, 700), src=2, formant=84, focus=70, choir=30, blend=90, drive=30), 2, 6, 40)),
        # 7 Strings, brass and winds
        ('SILK SECTION', S(0, 90, (260, 1000, 112, 1200), spread=46, voices=100, width=110, res=10, depth=10)),
        ('CELLO SECTION', S(0, 76, (180, 900, 112, 900), spread=36, voices=96, width=90, sub=20, res=14, depth=12)),
        ('VIOLIN SWELL', motion(S(1, 100, (400, 1200, 110, 1200), spread=30, voices=90, width=100, pulse=50, res=10), 2, 4, 40)),
        ('PIZZ SECTION', old('HARP ENSEMBLE')),
        ('BRASS SECTION', S(2, 78, (40, 500, 104, 400), spread=26, voices=90, width=80, res=22, depth=50, fenv=(60, 500, 50, 300), drive=22)),
        ('SOFT HORN', motion(ana(0, 70, (60, 500, 108, 400), res=10, depth=40, fenv=(80, 600, 40, 400), detune=6), 2, 3, 36)),
        ('WARM TRUMPET', motion(ana(1, 84, (20, 400, 108, 300), res=18, depth=50, fenv=(30, 300, 50, 200), drive=24), 2, 4, 42)),
        ('FLUTE AIR', motion(ana(3, 120, (40, 400, 112, 400), res=4, noise=20, depth=10, mix=20), 2, 4, 44)),
        ('NIGHT CLARINET', old('HOLLOW CLARINET')),
        ('OBOE DUSK', old('PHASE HORN')),
        # 8 Pads
        ('NORTHERN LIGHTS', motion(S(0, 88, (500, 2000, 116, 2000), spread=70, voices=110, width=127, res=20, depth=6), 0, 16, 10)),
        ('WARM BLANKET', motion(ana(0, 72, (600, 1500, 116, 1800), res=16, detune=16, depth=10, mix=50, sub=20), 0, 10, 14)),
        ('CLOUD CHOIR', motion(V(50, 96, (700, 1500, 116, 2400), choir=100, focus=60), 1, 30, 8)),
        ('DEEP OCEAN', motion(S(3, 70, (800, 2000, 120, 2400), spread=60, voices=110, width=120, sub=40, res=30), 0, 18, 8)),
        ('SLOW DAWN', S(0, 80, (1600, 3000, 116, 2400), spread=56, voices=100, width=120, res=24, depth=30, fenv=(2000, 2000, 80, 2000))),
        ('VELVET NIGHT', motion(S(1, 82, (500, 1500, 112, 1800), spread=48, voices=100, width=110, pulse=44), 1, 24, 12)),
        ('AMBER FIELD', old('AMBER TIDE')),
        ('GLASS HALO', motion(S(3, 112, (600, 1500, 110, 2000), spread=40, voices=80, width=120, res=20), 2, 3, 30)),
        ('OPEN SKY', motion(ana(5, 96, (500, 1500, 116, 2000), res=12, detune=30, depth=8), 0, 14, 12)),
        ('FOG BANK', motion(V(90, 80, (900, 2000, 116, 2600), src=3, breath=60, choir=90, focus=40), 0, 20, 6)),
        # 9 Textures
        ('TIDE POOL', motion(V(60, 100, (300, 1200, 112, 1600), choir=50, focus=70), 1, 70, 6)),
        ('SHIFTING SAND', motion(pd(100, 96, (300, 1200, 110, 1400), wave=6, line2=5, mix=70, detune=20), 1, 50, 10)),
        ('RADIO DUST', motion(ana(7, 100, (100, 800, 110, 1000), res=30, depth=10, filt=LP12, drive=50, mix=90), 0, 40, 8)),
        ('MAGNETIC', motion(old('LANTERN PHASE'), 0, 30, 10)),
        ('WANDERING SIGNAL', motion(S(1, 90, (200, 1000, 110, 1400), spread=60, voices=100, width=110, pulse=30), 1, 60, 8)),
        ('BREATHING WALL', motion(V(110, 90, (600, 2000, 116, 2400), src=3, breath=110, choir=80, focus=50), 0, 30, 4)),
        ('FROST FIELD', motion(old('DARK GLASS'), 1, 20, 12)),
        ('STARLIGHT DRIFT', motion(S(3, 116, (400, 1600, 104, 2400), spread=80, voices=100, width=127, res=40, air=10), 0, 20, 10)),
        ('MOTH WINGS', motion(ana(9, 100, (200, 900, 100, 1200), res=20, ratio=24, depth=6), 1, 40, 20)),
        ('ECHO GARDEN', motion(old('CLOUD HARMONICS'), 0, 18, 10)),
        # 10 Plucks
        ('SUPER PLUCK', S(0, 100, (1, 300, 0, 400), spread=40, voices=90, width=100, res=30, depth=50, fenv=(1, 200, 0, 300))),
        ('BUBBLE PLUCK', ana(3, 80, (1, 220, 0, 260), res=70, depth=60, fenv=(1, 140, 0, 160), filt=LP12)),
        ('GLASS PLUCK', S(3, 112, (1, 500, 0, 600), spread=24, voices=70, width=90, res=20, depth=30, fenv=(1, 300, 0, 400))),
        ('CEDAR HARP', old('CEDAR PLUCK')),
        ('NYLON NIGHT', old('NYLON HARP')),
        ('STEEL LINE', old('STEEL THREAD')),
        ('DARK KOTO', old('DARK KOTO')),
        ('PALM SHADOW', old('PALM MUTE')),
        ('OPEN KORA', old('OPEN KORA')),
        ('DULCET WIRE', old('DULCIMER WIRE')),
        # 11 Guitars and strings
        ('THUMB HARP', old('THUMB PIANO')),
        ('LOW CEDAR', old('LOW CEDAR')),
        ('FELT HARP', old('FELT HARP')),
        ('FROST PLUCK', old('FROST PLUCK')),
        ('PAPER GUITAR', old('PAPER GUITAR')),
        ('RUBBER WIRE', old('RUBBER WIRE')),
        ('LONG ZITHER', old('LONG ZITHER')),
        ('METAL BRUSH', old('METAL BRUSH')),
        ('TAPE HARP', old('TAPE HARP')),
        ('OPEN WIRE', old('OPEN WIRE')),
        # 12 Mallets
        ('DEEP MARIMBA', old('DEEP MARIMBA')),
        ('WOOD MALLET', old('WOOD MALLET')),
        ('SOFT VIBES', motion(old('SOFT VIBES'), 0, 6, 34)),
        ('FELT BAR', old('FELT BAR')),
        ('BAMBOO BLOCK', old('BAMBOO BLOCK')),
        ('CERAMIC DROP', old('CERAMIC DROP')),
        ('FM MARIMBA', old('FM MARIMBA')),
        ('SOFT HAMMER', old('SOFT HAMMER')),
        ('PICK MALLET', old('PICK MALLET')),
        ('ORBIT CHIME', old('ORBIT CHIME')),
        # 13 Bells
        ('BRONZE BOWL', old('BRONZE BOWL')),
        ('SMALL GONG', old('SMALL GONG')),
        ('GLASS BELL', old('GLASS BELL')),
        ('COPPER CUP', old('COPPER CUP')),
        ('IRON PLATE', old('IRON PLATE')),
        ('HANGING GLASS', old('HANGING GLASS')),
        ('TEMPLE BELL', old('TEMPLE BELL')),
        ('METAL RAIN', old('METAL RAIN')),
        ('FM CHIME', old('FM CHIME')),
        ('PHASE CRYSTAL', old('PHASE CRYSTAL')),
        # 14 Percussion and effects
        ('SKIN TOM', old('SKIN TOM')),
        ('CLAY DRUM', old('CLAY DRUM')),
        ('SHORT LOG', old('SHORT LOG')),
        ('NOISE SNARE', ana(7, 110, (1, 180, 0, 160), res=10, depth=30, fenv=(1, 120, 0, 120), filt=LP12, drive=30)),
        ('LOW VESSEL', old('LOW VESSEL')),
        ('PULSE TICK', old('PULSE TICK')),
        ('WIRE SNAP', old('WIRE SNAP')),
        ('RESONANT TAP', old('RESONANT TAP')),
        ('RISING TIDE', motion(S(0, 60, (2000, 3000, 120, 1500), spread=90, voices=110, width=127, res=50, depth=60, fenv=(3000, 3000, 127, 2000)), 0, 30, 6)),
        ('DRIFTING WIRE', old('DRIFTING WIRE')),
    ]
    assert len(rows) == 150 and len({n for n, _ in rows}) == 150
    levels = {}
    lp = R / 'assets/wanderer_levels_3_0.json'
    if lp.exists():
        levels = json.loads(lp.read_text())
    out = []
    for index, (name, p) in enumerate(rows):
        assert len(name) <= 16, name
        family = FAMILY[index // 10]
        wobble = [((index * 7 + k * 3) % 9) - 4 for k in range(6)]
        morph = [max(-60, min(60, m + (w if m else 0))) for m, w in zip(MELT[family], wobble)]
        if p['engine'] in (3, 4):
            morph[3] = morph[5] = 0
        if p['engine'] == 5:
            morph[3] = 0
        p['morph'] = morph
        source = p.pop('_from', None)
        # Revoiced 2.0 recipes keep their tuned 2.0 level; new sounds are rendered and matched.
        if not source:
            p['level'] = levels.get(name, p.get('level', 96))
        # Field order must match ws_patch_t.
        order = list(OLD['AMBER TIDE'].keys())
        p = {k: p[k] for k in order}
        out.append({'name': name, 'family': family, 'from_2_0': source, 'patch': p})
    return out


def main():
    rows = bank()
    path = R / 'firmware/src/wanderer_sound_data.h'
    original = path.read_text().split(MARKER)[0].rstrip()

    def c(value):
        return '{' + ','.join(map(c, value)) + '}' if isinstance(value, list) else str(value)

    names = ','.join(json.dumps(r['name']) for r in rows)
    patches = ',\n'.join(c(list(r['patch'].values())) for r in rows)
    path.write_text(f'{original}\n{MARKER}\n'
                    f'static const char *const WS_NAMES_3[WS_PRESET_COUNT]={{{names}}};\n'
                    f'static const ws_patch_t WS_PRESETS_3[WS_PRESET_COUNT]={{\n{patches}\n}};\n')
    (R / 'assets/wanderer_presets_3_0.json').write_text(json.dumps(rows, indent=2) + '\n')
    engines = {}
    for r in rows:
        engines[r['patch']['engine']] = engines.get(r['patch']['engine'], 0) + 1
    print(f'{len(rows)} Wanderer 3.0 sounds; engines {dict(sorted(engines.items()))}')


if __name__ == '__main__':
    main()
