#!/usr/bin/env python3
"""Prove the corrected package differs from original RC2 only in its identity/CRCs."""
from pathlib import Path
import hashlib,json,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import fm1_install as I
import fm1pkg_make as M

def entries(raw):
    image=I.deinterleave(raw)[0];head=I._dec(image[:64]);out={}
    for k in range(int.from_bytes(head[8:10],'little')):
        e=I._dec(image[64+k*80:64+(k+1)*80])
        off=int.from_bytes(e[8:12],'little');size=int.from_bytes(e[12:16],'little')
        out[int.from_bytes(e[:2],'little')]=image[off:off+size]
    return out

old_path,new_path=map(Path,sys.argv[1:3])
old,new=old_path.read_bytes(),new_path.read_bytes()
assert hashlib.sha256(old).hexdigest()=='e1a18e8f2d700399f6cac736e9587031d4a91cc7f286cc32172b45d0489ef378'
assert I.load_package(str(old_path))[0]=='FM-1_932'
assert I.load_package(str(new_path))[0]=='FM-1_929'
x,y=entries(old),entries(new)
assert x[100]==y[100], 'update loader changed'
assert x[0][:0x4000]==y[0][:0x4000], 'retained flash head changed'
region=bytearray(y[0][0x4000:]);M.sfc(region,0,len(region),0,M.KEY)
app=bytes(region[0x120:0x120+M.APP_SLOT])
assert app.count(b'FM-1_929')==1 and b'FM-1_932' not in app
# Restoring only the previous runtime identity must exactly reproduce original flash.
assert M.flash_image(app.replace(b'FM-1_929',b'FM-1_932'),M.KEY)==x[0]
assert M.ufw(y[0],y[100],'FM-1_929')==new
print(json.dumps({'identity':'FM-1_929','sha256':hashlib.sha256(new).hexdigest(),
 'bytes':len(new),'identity_only_change':True,'runtime_identity_verified':True,
 'loader_identical':True,'retained_head_identical':True,'crc_validation':'passed'},indent=2))
