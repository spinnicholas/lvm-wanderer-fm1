#!/usr/bin/env python3
"""Original, role-led 1.4 sound bank. Emit using the established coefficient compiler."""
import json
import design_bank_1_3 as previous
base_bank=previous.bank

def bank():
    out=base_bank()
    # More usable analog voices alongside the existing synthesis families.
    names=['FOUNDATION BASS','ROUND ACID','SAW STACK','SOFT POLY','VELVET LEAD','OCTAVE BRASS','DEEP PULSE','SLOW ORBIT','DUST KEYS','WIDE FIFTH',
           'LOW EMBER','TWIN SAW','OPEN HORIZON','REED CHOIR','NIGHT DRIVE','SOFT TRIAD','PLUSH KEYS','COPPER LEAD','CINEMA STRINGS','WARM CURRENT']
    slots=list(range(30,40))+list(range(50,60))
    for j,i in enumerate(slots):
        p=base_bank()[j%20]['patch'].copy()
        p.update(engine=0,filter=0,wave=[0,2,0,0,2,0,2,0,4,0][j%10],
                 cutoff=[44,58,82,76,86,65,48,69,80,80][j%10],resonance=[14,50,8,12,18,20,10,8,6,10][j%10],
                 mix=[72,45,70,66,52,60,42,72,38,65][j%10],sub=45 if j%10 in (0,6) else 12,
                 detune=[2,1,12,8,4,5,2,14,3,7][j%10],shape=12,drive=30 if j%10 in (0,1,4,6) else 12,
                 amp=([1,60,78,35] if j%10 in (0,1,4,6) else [58,82,98,86] if j%10 in (3,7) else [8,65,82,48]),
                 fenv=[5,66,18,45],envdepth=45 if j%10 in (0,1,4,6) else 22,level=100,
                 lfo_depth=2 if j%10 in (0,1,4,6) else 7,lfo_dest=0,lfo_rate=27)
        p['ratio']=[16,24 if j%10 in (5,9) else 16,32,48]
        out[i]=dict(name=names[j],patch=p)
    for x in out:
        p=x['patch']
        if p['engine']==0:
            p['shape']=min(p['shape'],50)
            p['lfo_depth']=min(p['lfo_depth'],10)
            p['level']=min(112,p['level']+10)
        elif p['engine']==1:
            # Less constant metallic modulation; a transient opens into a rounded fundamental.
            p['feedback']=min(34,p['feedback']);p['shape']=min(62,p['shape'])
            p['op_level'][0]=127
            for k in (1,2,3):
                p['op_level'][k]=p['op_level'][k]*3//4
                p['op_env'][k][2]=min(p['op_env'][k][2],18)
            p['cutoff']=94;p['level']=115
        elif p['engine']==2:
            p['cutoff']=min(p['cutoff'],92);p['drive']=min(p['drive'],24)
    return out

previous.bank=bank
previous.main()
(previous.ROOT/'assets/wanderer_presets_1_3.json').write_text(json.dumps(base_bank(),indent=2)+'\n')
(previous.ROOT/'assets/wanderer_presets_1_4.json').write_text(json.dumps(bank(),indent=2)+'\n')
