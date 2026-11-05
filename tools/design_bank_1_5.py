#!/usr/bin/env python3
"""Original 150-sound bank, authored for Wanderer's five actual DSP engines.

Recipes are musical designs, not imported patches or random parameter permutations.
Times are quantized to the existing 1..8000 ms envelope table. DSP is unchanged.
"""
import json
import math
from pathlib import Path
from design_bank_1_3 import TYPE

ROOT = Path(__file__).resolve().parents[1]
ENGINES = ['Analog', 'FM', 'Phase', 'String', 'Modal']
TIMES = [max(1, round(8000**(i/127))) for i in range(128)]

def time(ms):
    return min(range(128), key=lambda i: abs(math.log(TIMES[i]/max(1, ms))))

def env(a, d, s, r):
    return [time(a), time(d), s, time(r)]

def base(engine, amp):
    return dict(engine=engine, wave=0, mix=64, sub=0, detune=0, pulse=64,
        shape=32, damp=40, position=36, inharm=0, amp=env(*amp),
        fenv=env(2, 350, 0, 160), cutoff=94, resonance=8, filter=0,
        envdepth=0, drive=6, level=100, lfo_rate=25, lfo_depth=0,
        lfo_dest=0, algorithm=2, feedback=0, ratio=[16, 16, 32, 48],
        op_level=[127, 45, 95, 30],
        op_env=[env(1, 1800, 110, 500), env(1, 160, 0, 100),
                env(1, 1400, 95, 400), env(1, 100, 0, 70)],
        morph=[24, 0, 0, 0, 8, 0])

def analog(wave, cut, depth, res, amp, *, ratio=16, mix=66, detune=3,
           sub=0, pulse=64, shape=20, fd=300, fs=0, fa=2, ft=0,
           drive=10, rate=25, motion=0, dest=0, morph=None):
    p=base(0,amp)
    p.update(wave=wave,cutoff=cut,envdepth=depth,resonance=res,mix=mix,
        detune=detune,sub=sub,pulse=pulse,shape=shape,filter=ft,drive=drive,
        fenv=env(fa,fd,fs,400),lfo_rate=rate,lfo_depth=motion,lfo_dest=dest)
    p['ratio'][1]=ratio
    p['morph']=morph or [28,12,0,10,12,3]
    return p

def phase(bend, mix, cut, amp, *, detune=2, depth=0, fd=300,
          motion=0, rate=24, ft=0, drive=4, morph=None):
    p=base(2,amp)
    p.update(shape=bend,mix=mix,cutoff=cut,detune=detune,envdepth=depth,
        fenv=env(3,fd,0,450),lfo_dest=1,lfo_depth=motion,lfo_rate=rate,
        filter=ft,drive=drive)
    # Moving toward the midpoint rounds the wave; moving away adds harmonics.
    p['morph']=morph or [12,24 if bend>=64 else -24,0,0,4,2]
    return p

def string(tension, damp, position, cut, amp, *, depth=0, fd=200,
           drive=5, morph=None):
    p=base(3,amp)
    p.update(shape=tension,damp=damp,position=position,cutoff=cut,
        envdepth=depth,fenv=env(1,fd,0,220),drive=drive,level=115)
    p['morph']=morph or [14,6,-24,0,7,0]
    return p

def fm(ratios, levels, index, amp, *, alg=2, mod1=170, mod2=90,
       sustain=0, feedback=0, cut=109, motion=0, rate=30, morph=None):
    p=base(1,amp)
    p.update(ratio=ratios,op_level=levels,shape=index,algorithm=alg,
        feedback=feedback,cutoff=cut,lfo_dest=1,lfo_depth=motion,lfo_rate=rate)
    # Carriers hold their body; modulators lose brightness sooner.
    p['op_env']=[env(1,2400,110,amp[3]),env(1,mod1,sustain,120),
                 env(1,2100,100,amp[3]),env(1,mod2,sustain//2,90)]
    if alg==7:
        p['op_env']=[env(1,1600,127,amp[3]) for _ in range(4)]
    p['morph']=morph or [6,28,0,0,4,0]
    return p

def modal(ratios, levels, decay, damp, position, cut, *, release=750,
          inharm=0, attack=1, sustain=0, motion=0, rate=23, morph=None):
    p=base(4,(attack,6500,sustain,release))
    p.update(ratio=ratios,op_level=levels,damp=damp,position=position,
        cutoff=cut,inharm=inharm,lfo_depth=motion,lfo_rate=rate)
    p['op_env']=[env(attack,d,sustain if k==0 else sustain//2,release)
                 for k,d in enumerate(decay)]
    p['morph']=morph or [10,0,-26,0,5,0]
    return p

A=analog; P=phase; S=string; F=fm; M=modal

def bank():
    out=[]
    def group(title, entries):
        assert len(entries)==10,title
        for name,p,register,note in entries:
            out.append(dict(number=len(out)+1,bank=title,name=name,
                engine=ENGINES[p['engine']],register=register,description=note,patch=p))

    group('Foundation bass',[
      ('ROUND SUB', A(3,45,12,0,(4,600,105,130),mix=18,sub=52,shape=8),36,'Rounded sustained foundation; Melt opens its upper body.'),
      ('LADDER BASS', A(0,31,65,25,(2,320,78,110),sub=38,fd=190,drive=26),36,'Fast filter pluck over a sustained low end.'),
      ('PULSE ANCHOR', A(2,43,42,12,(3,450,94,150),pulse=43,sub=30,fd=280),36,'Weighty pulse bass with a quieter nasal edge.'),
      ('RUBBER STEP', P(27,18,55,(2,210,44,100),depth=33,fd=120),36,'Bent-phase rubber attack, rounded decay.'),
      ('SINE WEIGHT', F([16,16,16,32],[127,24,85,16],21,(4,800,105,160),mod1=85,mod2=60,cut=66),36,'Clean FM body with a brief low harmonic knock.'),
      ('OCTAVE FLOOR', A(0,38,46,9,(4,500,86,190),ratio=32,mix=30,sub=54,fd=340),36,'Octave bite above a stable sub.'),
      ('SOFT REESE', A(0,48,18,6,(12,750,100,250),detune=9,sub=20,motion=3,rate=16),36,'Slow beating bass for sustained notes.'),
      ('PLUCK ROOT', S(114,74,23,70,(1,550,72,170),depth=18),36,'Picked string bass with a damped edge.'),
      ('HOLLOW LOW', P(104,35,60,(5,600,94,180),detune=1,depth=14),36,'Hollow phase body for sparse bass phrases.'),
      ('FELT BASS', M([16,32,48,64],[127,45,22,10],[1800,380,180,100],54,58,64,release=200),36,'Soft harmonic mallet bass with a short upper transient.'),
    ])
    group('Character bass',[
      ('ACID POCKET', A(0,28,81,65,(1,190,35,80),fd=145,drive=35,sub=18),36,'Resonant bite for short sequences.'),
      ('NARROW PULSE', A(2,58,32,22,(2,280,77,100),pulse=18,mix=41,sub=22),36,'Nasal pulse bass with retained fundamental.'),
      ('WOOD KNOCK', F([16,16,32,48],[127,55,78,34],47,(1,260,18,100),mod1=75,mod2=42),36,'A wooden FM transient above a rounded low body.'),
      ('ANGLED BASS', P(112,13,63,(2,360,60,140),depth=35,fd=180,drive=13),36,'Firm phase bite for syncopated lines.'),
      ('SAW PRESSURE', A(0,50,45,16,(2,350,90,140),detune=1,sub=42,drive=30),36,'Solid saw bass without wide detuning.'),
      ('MUTED WIRE', S(91,114,12,77,(1,240,35,80),drive=12),36,'Short damped-string attack for rhythmic bass.'),
      ('BROKEN TRI', P(15,48,56,(5,450,79,190),depth=10),36,'Sine blend softens a strongly bent phase wave.'),
      ('BRIDGE BASS', S(120,42,8,89,(1,900,80,230)),36,'Bright near-bridge pluck with a long body.'),
      ('COPPER GROWL', A(1,45,52,48,(5,700,76,170),shape=42,pulse=29,drive=24),36,'Reedy bass with a resonant filter opening.'),
      ('CLAY LOW', M([16,25,35,47],[127,72,50,24],[1400,650,280,180],35,30,82,release=250),36,'Low struck body with nonharmonic overtones.'),
    ])
    group('Leads',[
      ('VELVET SAW', A(0,83,24,10,(12,400,105,240),detune=3,sub=9),60,'Full sustained lead with a softened attack.'),
      ('SINGING PULSE', A(2,76,28,22,(18,550,105,290),pulse=46,motion=2,dest=2,rate=70),60,'Pulse lead with restrained vibrato.'),
      ('ROUND WHISTLE', P(58,68,92,(22,600,112,260),detune=0,morph=[10,-25,0,0,3,0]),72,'Near-sine lead; Melt adds a stronger phase edge.'),
      ('COPPER SOLO', A(1,67,47,32,(15,470,96,220),shape=34,fd=350,fa=40),60,'Reedy lead with an opening filter attack.'),
      ('PRISM LEAD', P(21,24,91,(8,480,100,240),detune=3,motion=2,rate=65),60,'Bright phase lead with a subtle moving edge.'),
      ('FIFTH BEACON', A(0,86,18,8,(10,650,96,330),ratio=24,mix=32,detune=1),60,'Open-fifth lead for single-note lines.'),
      ('SOFT CHANT', M([16,32,48,80],[127,68,36,15],[5000,3600,1900,1000],15,30,86,attack=70,sustain=90,release=420),60,'Sustained harmonic partials with a vocal-like contour.'),
      ('NEON REED', F([16,32,16,48],[127,38,82,25],31,(9,750,100,280),mod1=450,mod2=180,sustain=32),60,'Two FM pairs retain body after the transient.'),
      ('BENT RIBBON', P(101,40,78,(35,700,105,440),motion=4,rate=34),60,'Gentle phase movement for lyrical lines.'),
      ('OCTAVE TORCH', A(0,77,35,15,(6,440,90,230),ratio=32,mix=38,sub=8,drive=18),60,'Octave-led saw tone for hooks.'),
    ])
    group('Poly keys',[
      ('WARM POLY', A(0,70,27,7,(15,700,88,520),detune=6,fd=520),60,'Rounded chord patch with a sustained center.'),
      ('PULSE COMP', A(2,73,36,13,(3,350,72,220),pulse=38,detune=4,fd=220),60,'Compact pulse chords for rhythmic playing.'),
      ('WAX KEYS', P(30,30,83,(4,700,54,420),depth=18,fd=360),60,'Soft phase keys with a declining bright edge.'),
      ('DUST CHORDS', A(4,80,15,4,(12,850,63,700),mix=42,detune=3,shape=12),60,'Muted triangle-family chords with a long release.'),
      ('GLASS POLY', P(111,24,95,(3,1050,56,700),detune=4),60,'Clear phase chord keys with a singing tail.'),
      ('BRASS COMP', A(0,61,49,19,(24,520,84,340),fd=450,fa=60),60,'Short swelling brass-like chords.'),
      ('CLEAN DIGITAL', F([16,16,32,64],[127,28,78,16],22,(3,1100,64,620),mod1=190,mod2=90),60,'Rounded digital keys rather than a constant metallic index.'),
      ('VELVET FIFTH', A(1,74,16,8,(20,900,90,650),ratio=24,mix=23,shape=28),60,'A quiet fifth adds color to open voicings.'),
      ('PAPER IVORY', P(45,48,80,(6,850,43,400),depth=16,fd=250),60,'Dry rounded keys for close chords.'),
      ('HARMONIC KEYS', M([16,32,64,96],[127,65,30,16],[1600,800,360,170],26,48,98,release=500),60,'Harmonic struck keys with progressively shorter upper modes.'),
    ])
    group('Soft keys',[
      ('FELT TINES', F([16,16,16,112],[127,43,55,26],38,(3,1400,0,750),mod1=300,mod2=110),60,'Body and tine use independent modulation decays.'),
      ('ROUND EP', F([16,16,32,48],[127,34,70,22],27,(4,1700,0,800),mod1=420,mod2=150,cut=98),60,'Mellow electric keys with a fuller lower pair.'),
      ('GLASS TINES', F([16,32,16,96],[127,39,62,31],45,(2,1200,0,650),mod1=240,mod2=160),60,'Brighter tine color for sparse melodic playing.'),
      ('REED KEYS', F([16,32,32,64],[127,30,64,17],25,(5,1550,20,650),mod1=650,mod2=300,sustain=10,cut=95),60,'Woody digital keys with a held reed body.'),
      ('WAX PIANO', P(23,32,85,(2,1400,0,600),depth=24,fd=420),60,'Phase-based struck keys with a softer sine layer.'),
      ('FELT STRINGS', S(111,96,34,74,(1,1500,56,540)),60,'Damped plucked strings for intimate chord voicings.'),
      ('PAPER TINES', M([16,32,69,110],[127,56,23,13],[1900,1000,370,180],28,40,99,release=680),60,'Quiet nonharmonic sparkle above a strong fundamental.'),
      ('WOOD IVORY', M([16,44,86,121],[127,45,20,9],[1650,460,210,100],52,67,94,release=550),60,'Soft mallet keys with rapidly darkening partials.'),
      ('ROUND CELESTA', P(108,54,94,(2,1200,0,700),detune=1,depth=16,fd=300),72,'Clear struck phase keys with a rounded center.'),
      ('NYLON KEYS', S(118,62,53,88,(1,1800,78,700)),60,'Long nylon-like plucks for broken chords.'),
    ])
    group('Organs and reeds',[
      ('DRAWBAR LIGHT', F([16,32,48,64],[127,84,42,24],1,(3,200,127,120),alg=7,cut=100),60,'Four additive sine partials, held evenly.'),
      ('DRAWBAR DEEP', F([8,16,32,48],[127,115,40,17],1,(4,200,127,160),alg=7,cut=88),60,'Sub-octave foundation with quieter upper drawbars.'),
      ('HOLLOW ORGAN', A(1,88,0,3,(4,200,120,130),ratio=32,mix=22,detune=0,shape=8),60,'Stable hollow-family organ without detuned beating.'),
      ('PULSE ORGAN', A(2,91,0,3,(3,200,120,120),ratio=32,mix=38,detune=0,pulse=64),60,'Octave pulse organ with a firm gate.'),
      ('PHASE REED', P(14,24,92,(24,600,108,210),detune=0,motion=2,rate=58),60,'Bright reed-like phase sustain.'),
      ('SOFT FLUTE', A(3,83,9,0,(55,450,108,260),mix=12,detune=0,shape=4,motion=2,dest=2,rate=67),72,'Rounded flute-like line with shallow vibrato.'),
      ('ODD CHOIR', M([16,48,80,112],[127,55,24,11],[5500,4200,2500,1400],16,45,94,attack=45,sustain=100,release=320),60,'Held odd partials for a hollow choral color.'),
      ('ROUND HARMONIUM', A(1,76,12,9,(55,850,110,330),ratio=32,mix=18,shape=18,detune=2),60,'Slow reed chords with an octave reinforcement.'),
      ('DIGITAL PIPE', F([16,32,64,96],[127,80,34,15],1,(15,250,127,220),alg=7,cut=104),72,'Clean harmonic organ for high-register lines.'),
      ('WAX ACCORDION', P(115,36,82,(30,700,108,280),detune=3,motion=2,rate=60),60,'Detuned phase reeds with sustained body.'),
    ])
    group('Brass and winds',[
      ('OPEN BRASS', A(0,58,63,15,(35,650,100,300),fa=65,fd=480,fs=32,detune=4),60,'Amp and filter rise together, then settle.'),
      ('MUTED HORN', A(1,56,38,24,(28,600,94,260),shape=32,fa=45,fd=430),60,'Narrower reedy horn color.'),
      ('OCTAVE BRASS', A(0,66,52,12,(30,750,100,420),ratio=32,mix=28,fa=80,fd=560),60,'Octave weight for wide brass chords.'),
      ('SOFT TROMBONE', A(0,48,43,9,(80,900,105,380),sub=12,fa=110,fd=750,detune=1),48,'Dark swelling brass body.'),
      ('PHASE HORN', P(24,18,72,(45,650,100,290),depth=43,fd=500),60,'Bent phase edge through an opening filter.'),
      ('COPPER WIND', P(110,28,78,(55,850,104,400),depth=25,fd=620,motion=2,rate=66),60,'Gentle vibratory edge for sustained wind phrases.'),
      ('BAMBOO AIR', S(122,110,61,92,(12,1800,90,600),morph=[10,4,-35,0,3,0]),60,'Soft damped string color for breath-like plucked phrases.'),
      ('HOLLOW CLARINET', A(2,61,30,9,(42,700,108,260),pulse=64,mix=24,detune=0,fa=40,fd=400),60,'Odd-harmonic wind-like sustain.'),
      ('GOLDEN HORN', F([16,16,32,32],[127,40,80,23],29,(35,1000,100,390),mod1=700,mod2=390,sustain=28,cut=91),60,'Warm FM pairs retain a controlled sustained index.'),
      ('CERAMIC WIND', M([16,32,48,80],[127,38,20,8],[5200,2500,1100,600],22,50,84,attack=80,sustain=85,release=450),60,'Soft additive wind tone with quieter upper modes.'),
    ])
    group('Ensemble strings',[
      ('SILK STRINGS', A(0,73,18,7,(350,1600,104,1400),detune=7,motion=3,rate=17),60,'Sustained saw ensemble with a gentle filter drift.'),
      ('WARM CELLOS', A(0,58,22,5,(180,1500,106,1100),detune=4,sub=12),48,'Darker low-register ensemble.'),
      ('PULSE ENSEMBLE', A(2,77,12,5,(260,1800,108,1500),pulse=43,detune=6),60,'Soft pulse strings with a slow amplitude edge.'),
      ('HIGH VIOLAS', A(0,85,12,4,(220,1600,100,1300),detune=5,motion=2,dest=2,rate=64),72,'Upper ensemble with shallow pitch motion.'),
      ('CHAMBER STRINGS', A(4,88,10,4,(300,1900,106,1800),mix=82,detune=4,shape=13),60,'Muted triangle-family ensemble for close chords.'),
      ('PRISM STRINGS', P(30,18,84,(400,2100,100,1600),detune=4,motion=3,rate=18),60,'Phase ensemble with slow timbre movement.'),
      ('SOFT PHASE BOW', P(106,48,79,(650,2500,104,2100),detune=3,motion=4,rate=12),60,'Long phase swell for legato pads.'),
      ('HARP ENSEMBLE', S(124,38,47,91,(1,2300,100,1400)),60,'Long plucked-string bed for rolling chords.'),
      ('DARK ZITHER', S(121,86,29,76,(2,2100,92,1200)),48,'Damped low strings with a restrained attack.'),
      ('HARMONIC VEIL', M([16,32,48,64],[127,65,32,12],[6000,4800,3400,2100],10,60,88,attack=500,sustain=92,release=2000),60,'Sustained harmonic ensemble without FM sidebands.'),
    ])
    group('Slow pads',[
      ('AMBER HORIZON', A(0,61,32,6,(1100,3200,104,2800),detune=6,fa=1800,fd=3200,fs=40,motion=3,rate=9),60,'Slow amp and filter blooms with a warm body.'),
      ('MOSS CHOIR', P(93,48,73,(900,3000,106,2600),detune=2,motion=4,rate=11),60,'Soft hollow phase pad with restrained motion.'),
      ('WARM CURRENT', A(1,68,15,5,(700,2600,104,2400),shape=24,detune=5,motion=4,rate=12),60,'Rounded reedy pad for held chords.'),
      ('LOW ORBIT', P(39,38,66,(1400,3500,110,3200),detune=2,motion=5,rate=7),48,'Dark phase swell with a long release.'),
      ('CLOUD HARMONICS', M([16,32,64,96],[127,56,32,16],[7000,6000,4700,3200],8,46,92,attack=950,sustain=100,release=2600),60,'Slow harmonic partials rather than a sharp strike.'),
      ('SILVER CHOIR', P(116,60,86,(1000,3200,100,2800),detune=4,motion=3,rate=15),60,'Bright phase layer softened by a strong sine blend.'),
      ('DARK GLASS', F([16,16,32,48],[127,24,72,14],18,(1000,3000,103,2500),mod1=2600,mod2=1800,sustain=32,motion=2,rate=10,cut=82),60,'Low-index sustained FM pad, without a bell attack.'),
      ('PAPER SKY', A(4,83,8,3,(1500,4200,110,3600),mix=80,detune=5,shape=10),60,'Very slow soft ensemble for wide voicings.'),
      ('BOWL HAZE', M([16,27,43,69],[127,42,23,10],[7000,4900,3100,1800],12,65,80,attack=1600,sustain=82,release=3200),60,'Nonharmonic partials entering without a percussive onset.'),
      ('PHASE DUSK', P(17,65,70,(1250,3800,108,3000),detune=1,motion=4,rate=8),60,'A sine-rich pad around a quiet bent-phase edge.'),
    ])
    group('Motion pads',[
      ('TIDAL FILTER', A(0,64,15,26,(500,2100,100,1700),detune=5,motion=11,rate=24),60,'Slow resonant filter movement.'),
      ('LANTERN PHASE', P(27,25,86,(600,2400,100,1900),motion=14,rate=21),60,'Clearly audible phase-bend movement.'),
      ('RIBBON CURRENT', P(100,35,81,(480,2200,100,1800),motion=11,rate=33),60,'Faster phase animation for held chord patterns.'),
      ('HOLLOW TIDE', A(1,76,14,12,(420,1800,104,1600),shape=62,pulse=30,motion=12,dest=1,rate=19),60,'Motion changes the pulse blend inside a hollow oscillator.'),
      ('SLOW PRISM', P(44,48,82,(1000,3000,108,2600),motion=18,rate=12),60,'Broad, slow phase movement near a round waveform.'),
      ('BELL AURORA', M([16,31,49,79],[127,45,25,12],[6500,5000,3200,1900],11,55,92,attack=750,sustain=75,motion=8,rate=14,release=2500),60,'Nonharmonic sustained partials with moving filter color.'),
      ('FM SHIMMER', F([16,32,16,80],[127,29,63,15],22,(650,2500,95,1900),mod1=1900,mod2=950,sustain=24,motion=6,rate=22),60,'Quiet sideband movement over two carriers.'),
      ('WAX WAVES', P(109,52,87,(550,2300,104,2100),motion=13,rate=27),60,'Rounded moving phase pad with detuned body.'),
      ('STRING HALO', S(125,30,58,95,(1,3200,115,2300),morph=[15,2,-20,0,4,0]),60,'Long resonating pluck suited to Scatter and reverb.'),
      ('COPPER SWELL', M([16,29,47,77],[127,48,22,11],[6500,4300,2700,1600],14,43,88,attack=1100,sustain=76,motion=12,rate=20,release=2400),60,'Slow struck-metal colors with a softened onset.'),
    ])
    group('Plucked strings',[
      ('CEDAR PLUCK', S(116,52,26,91,(1,1600,92,650)),60,'Balanced wooden pluck with a clear body.'),
      ('NYLON HARP', S(123,71,48,87,(1,2100,102,900)),60,'Soft string edge and a long ringing center.'),
      ('STEEL THREAD', S(125,19,18,109,(1,2300,108,1000)),60,'Bright steel-like pluck with little damping.'),
      ('DARK KOTO', S(112,67,13,82,(1,1500,84,520),depth=18),60,'Near-bridge pluck through a darkening filter.'),
      ('PALM MUTE', S(76,119,31,85,(1,260,18,90)),48,'Damped short pluck for rhythmic playing.'),
      ('OPEN KORA', S(124,36,73,96,(1,2400,108,1200)),60,'Open pluck with a different excitation comb.'),
      ('DULCIMER WIRE', S(122,26,9,110,(1,1900,90,750)),60,'Bright bridge pluck for repeated melodic patterns.'),
      ('THUMB PIANO', S(110,88,55,94,(1,1000,70,480)),60,'Compact damped tine-like string articulation.'),
      ('LOW CEDAR', S(120,64,24,77,(1,2000,100,700)),48,'Lower-register body for bass and arpeggios.'),
      ('FELT HARP', S(119,101,43,75,(2,1900,96,850)),60,'Soft muted string bed for broken chords.'),
    ])
    group('String mutations',[
      ('BRIDGE GLASS', S(126,10,5,118,(1,2500,105,1200)),60,'Very bright bridge excitation for granular phrases.'),
      ('PAPER GUITAR', S(102,106,67,83,(1,800,48,280),drive=14),60,'Damped papery pluck with a dry body.'),
      ('RUBBER WIRE', S(94,91,41,89,(1,650,54,250)),48,'Short soft tension setting for rubbery bass phrases.'),
      ('LONG ZITHER', S(126,22,37,101,(1,3500,116,1800)),60,'Long ringing plucks; leaves room for an effect tail.'),
      ('METAL BRUSH', S(103,18,94,115,(1,650,30,200),drive=19),72,'Bright short wire attack with a rough edge.'),
      ('TAPE HARP', S(122,81,85,72,(1,2000,97,1100),drive=15),60,'Darkened string pluck with gentle drive.'),
      ('WOOD CHIRP', S(84,116,17,99,(1,220,12,70)),72,'Brief damped excitation for high rhythmic accents.'),
      ('OPEN WIRE', S(127,12,61,115,(1,4200,118,2000)),60,'Bright long wire body for sparse ambient figures.'),
      ('DARK BRIDGE', S(117,93,7,73,(1,1600,83,540)),48,'Bridge articulation softened by damping and cutoff.'),
      ('SOFT KALIMBA', S(108,97,51,87,(1,1200,62,500)),60,'Soft attack and a compact tine-like ring.'),
    ])
    group('Tuned mallets',[
      ('DEEP MARIMBA', M([16,44,86,125],[127,55,23,9],[1500,420,170,80],52,54,101,release=400),48,'Strong root with bar-like upper modes fading quickly.'),
      ('WOOD MALLET', M([16,43,83,120],[127,48,18,8],[900,220,110,50],65,69,103,release=270),60,'Dry wood strike with a short upper click.'),
      ('SOFT VIBES', M([16,44,87,125],[127,55,25,12],[3600,1500,640,260],21,38,109,release=1200),60,'Long mallet fundamental with shorter bright modes.'),
      ('FELT BAR', M([16,42,80,117],[127,32,12,5],[1800,450,180,80],72,87,83,release=600),60,'Very soft muted bar strike.'),
      ('BAMBOO BLOCK', M([16,39,70,112],[127,50,28,12],[500,180,90,45],78,72,106,release=180),60,'Short tuned block for rhythmic sequences.'),
      ('CERAMIC DROP', M([16,38,73,117],[127,62,29,15],[1900,700,320,130],38,38,112,release=700),72,'Clear small-resonator strike with a bright opening.'),
      ('FM MARIMBA', F([16,16,32,48],[127,58,72,22],42,(1,700,0,300),mod1=90,mod2=45),60,'Fast modulator decay leaves a wooden low body.'),
      ('SOFT HAMMER', P(35,54,87,(2,850,0,360),depth=25,fd=140),60,'Rounded phase mallet with a closing filter.'),
      ('PICK MALLET', S(99,103,38,94,(1,650,34,240)),60,'Short plucked tine between wood and string.'),
      ('STONE CHIME', M([16,35,65,103],[127,52,21,10],[2200,950,400,170],41,58,98,release=800),60,'Muted stone-like ringing body.'),
    ])
    group('Bells and metals',[
      ('BRONZE BOWL', M([16,27,43,69],[127,84,48,28],[5200,3500,2100,1100],12,25,115,release=1800),60,'Bowl-like nonharmonic modes with a long tail.'),
      ('SMALL GONG', M([16,23,39,61],[127,75,52,31],[4200,3000,2200,1500],18,20,110,release=1600,inharm=14),48,'Lower clustered modes for a compact gong color.'),
      ('GLASS BELL', M([16,40,68,113],[127,69,34,19],[3900,2100,1000,450],20,41,117,release=1400),72,'Bright glass-like ring with a clear root.'),
      ('COPPER CUP', M([16,29,52,83],[127,74,38,24],[2700,1900,950,420],29,31,110,release=1000,inharm=6),60,'Compact metal cup with a strong second mode.'),
      ('IRON PLATE', M([16,34,66,107],[127,83,56,31],[3000,2400,1600,800],24,17,116,release=1200,inharm=18),48,'Dense plate-like partials for low resonant accents.'),
      ('HANGING GLASS', M([16,41,77,119],[127,61,31,17],[4800,2400,1200,550],15,49,117,release=1900),72,'Long glass fundamental with lighter upper sparkle.'),
      ('TEMPLE BELL', M([16,32,47,73],[127,79,42,24],[5600,3900,2500,1400],9,27,110,release=2100,inharm=8),60,'Long metal strike with several slowly decaying modes.'),
      ('METAL RAIN', M([16,37,89,125],[115,88,64,37],[1600,1100,650,350],27,15,120,release=800,inharm=20),72,'Bright clustered strike for overlapping high figures.'),
      ('FM CHIME', F([16,45,16,93],[127,58,75,35],55,(1,1900,0,950),mod1=700,mod2=340),72,'Two inharmonic FM pairs with different transient lengths.'),
      ('PHASE CRYSTAL', P(119,14,112,(1,1700,0,850),detune=1,depth=10,fd=650),72,'Bright phase strike offers a contrasting bell color.'),
    ])
    group('Textures and percussion',[
      ('SKIN TOM', M([16,26,34,43],[127,74,45,22],[850,410,220,120],58,45,91,release=240),48,'Membrane-like nonharmonic ratios for pitched tom patterns.'),
      ('CLAY DRUM', M([16,25,37,49],[127,85,49,25],[650,300,140,80],65,36,94,release=180),48,'Dry tuned vessel strike.'),
      ('SHORT LOG', M([16,43,85,123],[127,48,20,7],[280,100,50,25],85,82,105,release=110),60,'Very short bar-like knock.'),
      ('TUNED CAN', M([16,30,57,97],[127,92,58,32],[1200,800,420,200],47,21,115,release=400,inharm=23),60,'Compact bright resonator for tuned percussion.'),
      ('LOW VESSEL', M([16,24,35,47],[127,70,43,23],[2300,1250,700,300],33,40,85,release=700),36,'Low hollow struck body for sparse accents.'),
      ('PULSE TICK', P(12,8,104,(1,100,0,60),depth=10,fd=50,drive=10),72,'Short pitched phase click for repeating patterns.'),
      ('WIRE SNAP', S(62,121,11,113,(1,90,0,45),drive=24),72,'Damped noisy string onset with little lingering tone.'),
      ('RESONANT TAP', A(0,36,96,81,(1,170,0,75),mix=23,fd=70,drive=20),60,'Brief filter sweep for tuned electronic accents.'),
      ('BROKEN GLASS', P(124,3,119,(1,360,0,140),detune=18,motion=8,rate=80,drive=18),72,'A rough bright phase strike for Scatter textures.'),
      ('DRIFTING WIRE', S(127,7,106,105,(2,4400,119,2600),morph=[18,-10,32,0,10,0]),48,'Long bright wire; Melt darkens and damps the ringing body.'),
    ])
    assert len(out)==150 and len({x['name'] for x in out})==150
    return out

def main():
    out=bank()
    # Offline level trims come from production-DSP register auditions, keyed by name.
    trims=ROOT/'assets/wanderer_levels_1_5.json'
    if trims.exists():
        levels=json.loads(trims.read_text())
        for x in out:x['patch']['level']=levels.get(x['name'],x['patch']['level'])
    fields=list(out[0]['patch'])
    assert fields==['engine','wave','mix','sub','detune','pulse','shape','damp','position','inharm',
      'amp','fenv','cutoff','resonance','filter','envdepth','drive','level','lfo_rate','lfo_depth',
      'lfo_dest','algorithm','feedback','ratio','op_level','op_env','morph']
    def flatten(v):return sum((flatten(x) for x in v),[]) if isinstance(v,list) else [v]
    for x in out:
        p=x['patch']
        assert all(0<=n<=127 for f in fields[:-1] for n in flatten(p[f])),x['name']
        assert all(-127<=n<=127 for n in p['morph'])
        assert len(x['name'])<=19 and max(map(len,x['name'].split()))<=10
    def c(v):return '{'+','.join(c(x) for x in v)+'}' if isinstance(v,list) else str(v)
    prior=(ROOT/'firmware/src/wanderer_sound_data.h').read_text()
    tables=prior[prior.index('/* 1..8000 ms;'):].rstrip()
    h=[TYPE,'#define WS_PRESET_COUNT 150u',
       'static const char * const WS_NAMES[WS_PRESET_COUNT]={'+','.join(json.dumps(x['name']) for x in out)+'};',
       'static const ws_patch_t WS_PRESETS[WS_PRESET_COUNT]={']
    h += ['{'+','.join(c(x['patch'][f]) for f in fields)+'},' for x in out]
    h += ['};',tables]
    (ROOT/'firmware/src/wanderer_sound_data.h').write_text('\n'.join(h)+'\n')
    (ROOT/'assets/wanderer_presets_1_5.json').write_text(json.dumps(out,indent=2)+'\n')
    auditions=ROOT/'build/bank-1.5'
    auditions.mkdir(parents=True,exist_ok=True)
    (ROOT/'build/bank-1.5-registers.inc').write_text(','.join(str(x['register']) for x in out)+'\n')
    counts={e:sum(x['engine']==e for x in out) for e in ENGINES}
    print(json.dumps(dict(presets=len(out),engines=counts)))

if __name__=='__main__':main()
