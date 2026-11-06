"""Match the loudness of the 3.0 bank to the 2.0 bank by rendering both.

Writes assets/wanderer_levels_3_0.json (name -> level) and reruns
design_bank_3_0.py. Two passes converge; each preset is compared with the
2.0 median of its own register (bass families vs the rest)."""
import json, statistics, subprocess, sys
from pathlib import Path
R = Path(__file__).resolve().parents[1]
EXE = R / 'build/render_bank_3_0'

def render():
    subprocess.run(['cc', '-O2', '-w', '-Ifirmware/hal', '-Ifirmware/src', '-Ifirmware/gen',
                    'tests/render_bank_3_0.c', '-o', str(EXE), '-lm'], check=True, cwd=R)
    out = subprocess.run([str(EXE)], check=True, cwd=R, capture_output=True, text=True).stdout
    return [tuple(map(float, l.split())) for l in out.splitlines()]

def main():
    # Loudness targets: medians of the 2.0 factory bank, measured with this
    # renderer on 3.0 RC3 (the 2.0 bank itself was removed in 3.1).
    targets = {'low': 1931.4, 'rest': 3170.5, 'peak': 13187.0}
    print('targets', {k: round(v) for k, v in targets.items()})
    for _ in range(3):
        rows = json.loads((R / 'assets/wanderer_presets_3_0.json').read_text())
        now = render()
        levels = {}
        for i, row in enumerate(rows):
            if row.get('from_2_0'):
                continue
            if i >= 100:   # decaying sounds: match the attack peak instead of the average
                lvl = row['patch']['level'] * targets['peak'] / max(now[i][2], 1.0)
            else:
                lvl = row['patch']['level'] * targets['low' if 10 <= i < 30 else 'rest'] / max(now[i][1], 1.0)
            levels[row['name']] = int(max(24, min(127, round(lvl))))
        (R / 'assets/wanderer_levels_3_0.json').write_text(json.dumps(levels, indent=1) + '\n')
        subprocess.run([sys.executable, 'tools/design_bank_3_0.py'], check=True, cwd=R)
    now = render()
    rows = json.loads((R / 'assets/wanderer_presets_3_0.json').read_text())
    for i, (r, n) in enumerate(zip(rows, now)):
        if r.get('from_2_0'):
            flag = '  (2.0 level)'
        elif i >= 100:
            flag = '' if 0.7 < n[2] / targets['peak'] < 1.4 else '  <-- check'
        else:
            t = targets['low' if 10 <= i < 30 else 'rest']
            flag = '' if 0.7 < n[1] / t < 1.4 else '  <-- check'
        print(f"{i+1:3} {r['name']:<16} eng {r['patch']['engine']} lvl {r['patch']['level']:3} rms {n[1]:7.0f} peak {n[2]:6.0f} zcr {n[3]:.3f}{flag}")

if __name__ == '__main__':
    main()
