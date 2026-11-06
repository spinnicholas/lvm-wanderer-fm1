#!/usr/bin/env python3
"""3.1 palette contrast check, straight from wanderer_themes.h (RGB565 as stored).
Text and labels >= 7:1 on bg, panel and memory surfaces; every accent >= 3:1;
the background colour used as text on an accent fill (selected rows) >= 4.5:1."""
import re, sys
from pathlib import Path
src = Path(__file__).resolve().parents[1] / 'firmware/src/wanderer_themes.h'
def lin(c):
    c /= 255; return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
def rgb565(r, g, b):  # what the panel shows
    r5, g6, b5 = r >> 3, g >> 2, b >> 3
    return (r5 * 255 // 31, g6 * 255 // 63, b5 * 255 // 31)
def L(c):
    r, g, b = c; return 0.2126 * lin(r) + 0.7152 * lin(g) + 0.0722 * lin(b)
def ratio(a, b):
    la, lb = sorted((L(a), L(b)), reverse=True); return (la + 0.05) / (lb + 0.05)
ok = True
for name, body in re.findall(r'\{"(\w+)",(.*?)\}', src.read_text()):
    cols = [rgb565(*map(int, t)) for t in re.findall(r'RGB\((\d+),(\d+),(\d+)\)', body)]
    bg, panel, memory, text, label, anchor, evolve, layer, coral, sky = cols
    surfaces = [bg, panel, memory]
    t = min(ratio(x, s) for x in (text, label) for s in surfaces)
    a = min(ratio(x, s) for x in (anchor, evolve, layer, coral, sky) for s in surfaces)
    on = min(ratio(bg, x) for x in (anchor, evolve, layer, coral, sky))
    good = t >= 7 and a >= 3 and on >= 4.5
    ok &= good
    print(f'{name:<9} text {t:5.2f}  accents {a:5.2f}  text-on-accent {on:5.2f}  {"ok" if good else "FAIL"}')
sys.exit(0 if ok else 1)
