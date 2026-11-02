#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Encode the supplied brand artwork without cropping into the native LCD palette."""
from pathlib import Path
from PIL import Image,ImageOps
import sys,hashlib,json
R=Path(__file__).resolve().parents[1];D=Path(sys.argv[1]) if len(sys.argv)>1 else R/'build/splash-art';D.mkdir(parents=True,exist_ok=True)
src=R/'assets/wanderer-splash.png';art=Image.open(src).convert('RGB');fit=ImageOps.contain(art,(240,240),Image.Resampling.LANCZOS)
im=Image.new('RGB',(240,240),'black');im.paste(fit,((240-fit.width)//2,(240-fit.height)//2));q=im.quantize(colors=128,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
pal=q.getpalette();colors=[((pal[i*3]>>3)<<11)|((pal[i*3+1]>>2)<<5)|(pal[i*3+2]>>3) for i in range(128)]
rows=[];data=[]
for y in range(240):
 rows.append(len(data));x=0
 while x<240:
  c=q.getpixel((x,y));end=x+1
  while end<240 and end-x<255 and q.getpixel((end,y))==c:end+=1
  data += [end-x,c];x=end
rows.append(len(data))
h=['/* Supplied LVM artwork; generated lossless palette-run stream. */','#pragma once','static const uint16_t SPLASH_PALETTE[128]={'+','.join(map(str,colors))+'};','static const uint32_t SPLASH_ROW[241]={'+','.join(map(str,rows))+'};','static const uint8_t SPLASH_DATA[]={'+','.join(map(str,data))+'};']
(R/'firmware/src/wanderer_splash_art.h').write_text('\n'.join(h)+'\n')
# Reference contains exactly the RGB565 pixels the production renderer sends.
ref=Image.new('RGB',(240,240));px=ref.load()
for y in range(240):
 for x in range(240):
  c=colors[q.getpixel((x,y))];px[x,y]=(((c>>11)&31)*255//31,((c>>5)&63)*255//63,(c&31)*255//31)
ref.save(D/'Wanderer-New-Splash.png');(D/'source.json').write_text(json.dumps({'source_sha256':hashlib.sha256(src.read_bytes()).hexdigest(),'fit':[fit.width,fit.height],'cropped':False,'palette_colors':128,'encoded_bytes':len(data)+len(rows)*4+256},indent=2)+'\n');print('Supplied splash:',len(data)+len(rows)*4+256,'bytes, full artwork retained')
