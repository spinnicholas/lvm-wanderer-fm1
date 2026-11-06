#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Encode the supplied brand artwork for the animated 3.1 splash.

The 240x240 fit of assets/wanderer-splash.png is split in two:
  * the "LVM" mark (everything above the wordmark) as a lossless 128-colour
    palette run stream, as before (SPLASH_PALETTE / SPLASH_ROW / SPLASH_DATA);
  * the eight letters of "Wanderer", each a cropped 4-bit alpha mask in the
    icon run format (token = alpha << 4 | (count - 1)), with its final
    position, so the firmware can slide them in one by one.
Letters are separated by connected shapes at a firm threshold; every softer
edge pixel joins its nearest letter, so nothing is lost or doubled.
"""
from pathlib import Path
from PIL import Image, ImageOps
import sys, hashlib, json
from collections import deque

R = Path(__file__).resolve().parents[1]
D = Path(sys.argv[1]) if len(sys.argv) > 1 else R / 'build/splash-art'
D.mkdir(parents=True, exist_ok=True)
src = R / 'assets/wanderer-splash.png'
art = Image.open(src).convert('RGB')
fit = ImageOps.contain(art, (240, 240), Image.Resampling.LANCZOS)
im = Image.new('RGB', (240, 240), 'black')
im.paste(fit, ((240 - fit.width) // 2, (240 - fit.height) // 2))
px = im.load()

# The blue mark ends above this row; the white wordmark lies below it.
WORD_TOP = 146
def alpha(x, y):
    return min(px[x, y])
band = {(x, y): alpha(x, y) for x in range(240) for y in range(WORD_TOP, 240)}

# Firm cores, then nearest-core assignment for every visible pixel.
label, n = {}, 0
for (x, y), v in sorted(band.items(), key=lambda kv: (kv[0][0], kv[0][1])):
    if v > 128 and (x, y) not in label:
        n += 1; label[(x, y)] = n; stack = [(x, y)]
        while stack:
            cx, cy = stack.pop()
            for q in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                if q in band and band[q] > 128 and q not in label:
                    label[q] = n; stack.append(q)
sizes = {}
for v in label.values():
    sizes[v] = sizes.get(v, 0) + 1
cores = [c for c, s in sizes.items() if s > 40]
assert len(cores) == 8, f'expected 8 letters, found {len(cores)}'
queue = deque(p for p, c in label.items() if c in cores)
owner = {p: label[p] for p in queue}
while queue:
    cx, cy = queue.popleft()
    for q in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
        if q in band and band[q] > 6 and q not in owner:
            owner[q] = owner[(cx, cy)]; queue.append(q)
order = sorted(cores, key=lambda c: min(x for (x, y), o in owner.items() if o == c))

letters = []
for c in order:
    pts = [p for p, o in owner.items() if o == c]
    x0 = min(x for x, y in pts); x1 = max(x for x, y in pts)
    y0 = min(y for x, y in pts); y1 = max(y for x, y in pts)
    w, h = x1 - x0 + 1, y1 - y0 + 1
    own = set(pts)
    rows = []
    for y in range(y0, y1 + 1):
        x = x0; row = []
        while x <= x1:
            a = min(15, (band[(x, y)] + 8) // 17) if (x, y) in own else 0
            end = x + 1
            while end <= x1 and end - x < 16 and (min(15, (band[(end, y)] + 8) // 17) if (end, y) in own else 0) == a:
                end += 1
            row.append((a << 4) | (end - x - 1)); x = end
        rows.append(row)
    letters.append(dict(x=x0, y=y0, w=w, h=h, rows=rows))
    for p in pts:
        px[p] = (0, 0, 0)          # remove the letter from the static image
# Wordmark colour from its solid pixels.
ink = [p for p, v in band.items() if v > 200]
ref_img = Image.new('RGB', (240, 240), 'black'); ref_img.paste(fit, ((240 - fit.width) // 2, (240 - fit.height) // 2))
rp = ref_img.load()
r = sum(rp[p][0] for p in ink) // len(ink); g = sum(rp[p][1] for p in ink) // len(ink); b = sum(rp[p][2] for p in ink) // len(ink)
word_color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
for y in range(WORD_TOP, 240):
    for x in range(240):
        px[x, y] = (0, 0, 0)

# The mark: palette run stream.
q = im.quantize(colors=128, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
pal = q.getpalette()
colors = [((pal[i * 3] >> 3) << 11) | ((pal[i * 3 + 1] >> 2) << 5) | (pal[i * 3 + 2] >> 3) for i in range(128)]
row_at, data = [], []
for y in range(240):
    row_at.append(len(data)); x = 0
    while x < 240:
        c = q.getpixel((x, y)); end = x + 1
        while end < 240 and end - x < 255 and q.getpixel((end, y)) == c:
            end += 1
        data += [end - x, c]; x = end
row_at.append(len(data))

ldata, loff, lrow = [], [], []
for L in letters:
    loff.append(len(lrow))
    for row in L['rows']:
        lrow.append(len(ldata)); ldata += row
lrow.append(len(ldata))
h = ['/* Supplied LVM artwork: the mark as a palette-run stream and the eight letters of',
     ' * "Wanderer" as 4-bit alpha masks for the animated splash (tools/gen_wanderer_splash.py). */',
     '#pragma once',
     'static const uint16_t SPLASH_PALETTE[128]={' + ','.join(map(str, colors)) + '};',
     'static const uint32_t SPLASH_ROW[241]={' + ','.join(map(str, row_at)) + '};',
     'static const uint8_t SPLASH_DATA[]={' + ','.join(map(str, data)) + '};',
     f'#define SPLASH_LETTERS {len(letters)}u',
     f'#define SPLASH_WORD_TOP {min(L["y"] for L in letters)}',
     f'#define SPLASH_WORD_BOTTOM {max(L["y"] + L["h"] for L in letters)}',
     f'#define SPLASH_WORD_COLOR {word_color}u',
     'static const uint8_t SPLASH_LX[SPLASH_LETTERS]={' + ','.join(str(L['x']) for L in letters) + '};',
     'static const uint8_t SPLASH_LY[SPLASH_LETTERS]={' + ','.join(str(L['y']) for L in letters) + '};',
     'static const uint8_t SPLASH_LW[SPLASH_LETTERS]={' + ','.join(str(L['w']) for L in letters) + '};',
     'static const uint8_t SPLASH_LH[SPLASH_LETTERS]={' + ','.join(str(L['h']) for L in letters) + '};',
     'static const uint16_t SPLASH_LFIRST[SPLASH_LETTERS]={' + ','.join(map(str, loff)) + '};',
     'static const uint16_t SPLASH_LROW[]={' + ','.join(map(str, lrow)) + '};',
     'static const uint8_t SPLASH_LDATA[]={' + ','.join(map(str, ldata)) + '};']
(R / 'firmware/src/wanderer_splash_art.h').write_text('\n'.join(h) + '\n')
size = len(data) + len(row_at) * 4 + 256 + len(ldata) + len(lrow) * 2 + 8 * 6
ref_img.save(D / 'Wanderer-New-Splash.png')
(D / 'source.json').write_text(json.dumps({'source_sha256': hashlib.sha256(src.read_bytes()).hexdigest(),
    'fit': [fit.width, fit.height], 'cropped': False, 'palette_colors': 128, 'letters': len(letters),
    'encoded_bytes': size}, indent=2) + '\n')
print('Animated splash:', size, 'bytes (mark + 8 letters)')
