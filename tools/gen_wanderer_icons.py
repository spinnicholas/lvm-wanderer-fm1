#!/usr/bin/env python3
"""Generate bold 64-pixel, 4-bit symbols from original 24-unit vector geometry.

The source geometry is kept here so shapes and stroke weight can be revised
without hand-editing bitmap data. No external icon artwork is embedded.
"""
from pathlib import Path
from PIL import Image, ImageDraw
import sys

S = 4
VIEW = 24
SIZE = 64
STROKE = 10


def make(name, size=SIZE):
    im = Image.new("L", (VIEW*S, VIEW*S), 0)
    d = ImageDraw.Draw(im)
    def box(a): return tuple(round(v*S) for v in a)
    def line(points, w=STROKE):
        pts=[(round(x*S),round(y*S)) for x,y in points]
        d.line(pts,fill=255,width=w,joint="curve")
        r=w//2
        for x,y in (pts[0],pts[-1]): d.ellipse((x-r,y-r,x+r,y+r),fill=255)
    def rect(a,r=2,w=STROKE): d.rounded_rectangle(box(a),radius=r*S,outline=255,width=w)
    def oval(a,w=STROKE): d.ellipse(box(a),outline=255,width=w)
    def disk(a): d.ellipse(box(a),fill=255)
    if name=="anchor":
        oval((9,2,15,8),w=8)
        line([(12,8),(12,19)])
        line([(6,10),(18,10)])
        d.arc(box((3,9,21,21)),0,180,fill=255,width=STROKE)
        line([(3,15),(3,12)])
        line([(21,15),(21,12)])
    elif name=="memory":
        line([(5,8),(19,8),(16,5)])
        line([(19,8),(16,11)])
        line([(19,16),(5,16),(8,13)])
        line([(5,16),(8,19)])
    elif name=="layer":
        rect((3,4,18,16),2)
        rect((7,8,22,20),2)
    elif name=="play":
        d.polygon([(7*S,4*S),(20*S,12*S),(7*S,20*S)],fill=255)
    elif name=="stop":
        d.rounded_rectangle(box((5,5,19,19)),radius=2*S,fill=255)
    elif name=="record":
        disk((5,5,19,19))
    elif name=="melt":
        line([(12,3),(18,12),(19,15),(17,19),(12,21),(7,19),(5,15),(6,12),(12,3)])
        disk((10,15,13,18))
    elif name=="drift":
        line([(3,8),(10,8),(14,5),(21,5)])
        line([(3,16),(11,16),(15,19),(21,19)])
        d.polygon([(20*S,2*S),(23*S,5*S),(20*S,8*S)],fill=255)
        d.polygon([(20*S,16*S),(23*S,19*S),(20*S,22*S)],fill=255)
    elif name=="decay":
        line([(5,3),(19,3)])
        line([(5,21),(19,21)])
        line([(7,5),(17,5),(13,10),(12,12),(11,10),(7,5)])
        line([(7,19),(17,19),(12,13),(7,19)])
    elif name=="tempo":
        line([(4,20),(20,20)])
        line([(6,18),(10,4),(14,4),(18,18)])
        line([(12,13),(18,6)])
        disk((17,5,21,9))
    elif name=="level":
        d.polygon([(3*S,9*S),(7*S,9*S),(12*S,5*S),(12*S,19*S),(7*S,15*S),(3*S,15*S)],fill=255)
        d.arc(box((10,5,20,19)),290,70,fill=255,width=STROKE)
        d.arc(box((10,2,23,22)),295,65,fill=255,width=STROKE)
    elif name=="size":
        # Zoom: a generous lens and a short, unmistakable handle.
        oval((3,3,16,16),w=9)
        line([(15,15),(21,21)],w=11)
    elif name=="save":
        line([(12,3),(12,15)])
        line([(7,10),(12,15),(17,10)])
        line([(4,16),(4,21),(20,21),(20,16)])
    elif name=="load":
        line([(12,17),(12,5)])
        line([(7,10),(12,5),(17,10)])
        line([(4,16),(4,21),(20,21),(20,16)])
    elif name=="undo":
        d.arc(box((5,5,21,21)),200,520,fill=255,width=STROKE)
        d.polygon([(2*S,9*S),(10*S,8*S),(7*S,15*S)],fill=255)
    elif name=="branch":
        line([(5,4),(5,13),(12,13),(18,19)])
        line([(12,13),(18,7)])
        disk((3,2,7,6));disk((16,5,20,9));disk((16,17,20,21))
    elif name=="erase":
        line([(5,6),(19,6)])
        line([(9,4),(15,4)])
        rect((7,8,17,21),1)
        line([(10,11),(10,17)],6);line([(14,11),(14,17)],6)
    elif name=="new":
        oval((3,3,21,21))
        line([(12,7),(12,17)])
        line([(7,12),(17,12)])
    elif name=="check":
        line([(4,12),(10,18),(20,6)])
    elif name=="close":
        line([(5,5),(19,19)])
        line([(19,5),(5,19)])
    elif name=="safe":
        line([(12,3),(20,6),(19,15),(12,21),(5,15),(4,6),(12,3)])
        line([(8,12),(11,15),(16,9)])
    elif name=="sun":
        oval((7,7,17,17))
        for a,b in [((12,2),(12,5)),((12,19),(12,22)),
                    ((2,12),(5,12)),((19,12),(22,12)),
                    ((4,4),(6,6)),((18,18),(20,20)),
                    ((4,20),(6,18)),((18,6),(20,4))]:
            line([a,b],w=8)
    elif name=="still":
        # A paused orbit reads as stillness rather than a snowflake.
        oval((3,3,21,21),w=7)
        line([(9,8),(9,16)],w=8)
        line([(15,8),(15,16)],w=8)
    elif name=="sound":
        line([(2,13),(6,13),(8,5),(11,19),(14,8),(17,16),(19,12),(22,12)],w=8)
    elif name=="chorus":
        for y in (5,12,19):
            line([(3,y),(7,y-2),(12,y+2),(17,y-2),(21,y)],w=7)
    elif name=="delay":
        d.arc(box((3,3,21,21)),25,305,fill=255,width=STROKE)
        d.polygon([(19*S,15*S),(23*S,20*S),(17*S,20*S)],fill=255)
        disk((10,10,14,14))
    elif name=="reverb":
        disk((2,10,6,14))
        for x in (8,14,20):
            d.arc(box((x-7,4,x+4,20)),300,60,fill=255,width=7)
    else:
        raise ValueError(name)
    return im.resize((size,size),Image.Resampling.LANCZOS)


def main(out):
    names="anchor memory layer play stop record melt drift decay tempo level size save load undo branch erase new check close safe sun still sound chorus delay reverb".split()
    data=[]
    for name in names:
        data.extend(min(15,(v+8)//17) for v in make(name).tobytes())
    packed=[(data[i]<<4)|data[i+1] for i in range(0,len(data),2)]
    lines=["/* Original LVM symbols; generated by tools/gen_wanderer_icons.py. */",
           "#pragma once", f"#define ICON_SIZE {SIZE}"]
    lines += [f"#define ICO_{n.upper()} {i}" for i,n in enumerate(names)]
    lines.append(f"static const uint8_t ICON_ALPHA[{len(packed)}] = {{")
    for i in range(0,len(packed),24):
        lines.append("    "+", ".join(f"0x{x:02x}" for x in packed[i:i+24])+",")
    lines.append("};")
    zoom=[]
    for name in ("anchor","memory"):
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
    Path(out).write_text("\n".join(lines)+"\n")
    print(f"{len(names)} original symbols, {len(packed)+len(zoom)+len(small)} packed bytes")

if __name__=="__main__":
    main(sys.argv[1])
