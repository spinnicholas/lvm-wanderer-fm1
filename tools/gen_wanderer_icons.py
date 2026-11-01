#!/usr/bin/env python3
"""Rasterize vetted, vendored SVG icons to the FM-1 4-bit alpha tables.

Lucide: ISC, with inherited Feather MIT notices. Tabler: MIT.
Sources and untouched originals are retained in assets/icons/.
Only stroke width is adapted (2.25 units on the original 24-unit grid).
No remote fetch is performed during a build. Inkscape renders each source once;
Pillow downsamples that 384px alpha to the five actual display sizes.
"""
from pathlib import Path
from PIL import Image
from io import BytesIO
import hashlib,subprocess,sys,xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[1]
SIZE=64
MAP={
 'anchor':'lucide/anchor', 'memory':'lucide/repeat-2',
 'layer':'lucide/layers-2', 'play':'lucide/play', 'stop':'lucide/square',
 'record':'lucide/circle', 'melt':'lucide/droplet', 'drift':'lucide/shuffle',
 'decay':'lucide/hourglass', 'tempo':'tabler/metronome',
 'level':'lucide/volume-2', 'size':'lucide/search',
 'save':'lucide/save', 'load':'lucide/folder-open', 'undo':'lucide/undo-2',
 'branch':'lucide/git-branch', 'erase':'lucide/trash', 'new':'lucide/circle-plus',
 'check':'lucide/check', 'close':'lucide/x', 'safe':'lucide/shield-check',
 'sun':'lucide/contrast', 'still':'lucide/circle-pause', 'sound':'lucide/list-music',
 'chorus':'lucide/audio-waveform', 'delay':'lucide/chart-no-axes-column-decreasing',
 'reverb':'lucide/box', 'bell':'lucide/bell', 'lock':'lucide/lock-keyhole',
 'clock':'lucide/clock', 'feedback':'lucide/rotate-ccw', 'room':'lucide/maximize-2',
 'damp':'tabler/filter-down', 'scope':'lucide/square-activity',
 'controls':'lucide/sliders-horizontal', 'empty':'lucide/file-minus',
 'broken':'lucide/file-x', 'pause':'lucide/pause', 'root':'tabler/music',
 'scale':'tabler/piano', 'length':'lucide/columns-2', 'listen':'lucide/headphones',
 'edited':'lucide/file-pen-line', 'stored':'lucide/file-check', 'fresh':'lucide/file-plus', 'mute':'lucide/volume-x',
}
CACHE={}
def make(name,size=SIZE):
    if name not in CACHE:
        original=ROOT/'assets/icons'/f'{MAP[name]}.svg'
        tree=ET.fromstring(original.read_text())
        tree.set('stroke-width','2.25')
        tree.set('stroke','#ffffff')
        # Raster cache is keyed by the adapted bytes, so source edits cannot reuse old masks.
        adapted=ET.tostring(tree)
        key=hashlib.sha256(adapted).hexdigest()[:20]
        folder=ROOT/'build/icon-cache';folder.mkdir(parents=True,exist_ok=True)
        png=folder/f'{name}-{key}.png'
        if not png.exists():
            svg=folder/f'{name}-{key}.svg';svg.write_bytes(adapted)
            result=subprocess.run(['inkscape',str(svg),'--export-type=png',
                f'--export-filename={png}','--export-width=384','--export-height=384'],
                capture_output=True,text=True)
            if result.returncode:raise RuntimeError(result.stderr)
        CACHE[name]=Image.open(png).convert('RGBA').getchannel('A')
    if size==24:
        # Keep a guaranteed one-pixel quiet margin at the smallest status size.
        out=Image.new('L',(24,24),0)
        out.paste(CACHE[name].resize((22,22),Image.Resampling.LANCZOS),(1,1))
        return out
    return CACHE[name].resize((size,size),Image.Resampling.LANCZOS)


def main(out):
    names="anchor memory layer play stop record melt drift decay tempo level size save load undo branch erase new check close safe sun still sound chorus delay reverb bell lock clock feedback room damp scope controls empty broken pause root scale length listen edited stored fresh mute".split()
    data=[]
    for name in names:
        data.extend(min(15,(v+8)//17) for v in make(name).tobytes())
    packed=[(data[i]<<4)|data[i+1] for i in range(0,len(data),2)]
    lines=["/* Lucide ISC / Feather MIT and Tabler MIT icons, raster-adapted for Wanderer. See assets/icons licenses and sources.json. */",
           "#pragma once", f"#define ICON_SIZE {SIZE}"]
    lines += [f"#define ICO_{n.upper()} {i}" for i,n in enumerate(names)]
    lines.append(f"static const uint8_t ICON_ALPHA[{len(packed)}] = {{")
    for i in range(0,len(packed),24):
        lines.append("    "+", ".join(f"0x{x:02x}" for x in packed[i:i+24])+",")
    lines.append("};")
    zoom=[]
    for name in names:
        zoom.extend(min(15,(v+8)//17) for v in make(name,96).tobytes())
    zoom=[(zoom[i]<<4)|zoom[i+1] for i in range(0,len(zoom),2)]
    lines.append("#define ZOOM_SIZE 96")
    lines.append(f"static const uint8_t ICON_ZOOM_ALPHA[{len(zoom)}] = {{")
    for i in range(0,len(zoom),24):
        lines.append("    "+", ".join(f"0x{x:02x}" for x in zoom[i:i+24])+",")
    lines.append("};")
    small=[min(15,(v+8)//17) for v in make("layer",32).tobytes()]
    small=[(small[i]<<4)|small[i+1] for i in range(0,len(small),2)]
    lines.append("#define SMALL_SIZE 32")
    lines.append(f"static const uint8_t ICON_LAYER_SMALL_ALPHA[{len(small)}] = {{")
    for i in range(0,len(small),24):
        lines.append("    "+", ".join(f"0x{x:02x}" for x in small[i:i+24])+",")
    lines.append("};")
    for label,size in [("TINY",24),("MINI",32),("MID",48)]:
        pixels=[]
        for name in names:pixels.extend(min(15,(v+8)//17) for v in make(name,size).tobytes())
        packed_size=[(pixels[i]<<4)|pixels[i+1] for i in range(0,len(pixels),2)]
        lines.append(f"static const uint8_t ICON_{label}_ALPHA[{len(packed_size)}] = {{")
        for i in range(0,len(packed_size),24):lines.append("    "+", ".join(f"0x{x:02x}" for x in packed_size[i:i+24])+",")
        lines.append("};")
    Path(out).write_text("\n".join(lines)+"\n")
    print(f"{len(names)} source-backed symbols, {len(packed)+len(zoom)+len(small)} packed bytes")

if __name__=="__main__":
    main(sys.argv[1])
