#!/usr/bin/env python3
"""Exact v15 validation and simulated full host transactions, including loader resume rules."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parent.parent/'tools'))
import fm1_install as I
root=Path(__file__).resolve().parent.parent
import ast
source=(root/'tests/install_test.py').read_text()
tree=ast.parse(source)
# Load definitions, imports and constants, without executing its standalone suite.
skip={'wire','installs','errors','smk','against_js','starrykey','print'}
def suite_call(node):
    if not isinstance(node,ast.Expr) or not isinstance(node.value,ast.Call):return False
    f=node.value.func
    return (isinstance(f,ast.Name) and f.id in skip) or (isinstance(f,ast.Attribute) and isinstance(f.value,ast.Name) and f.value.id=='sys' and f.attr=='exit')
tree.body=[node for node in tree.body if not suite_call(node)]
scope={'__file__':str(root/'tests/install_test.py'),'__name__':'installer_fixture'}
exec(compile(tree,str(root/'tests/install_test.py'),'exec'),scope)
Fake=scope['FakeFM1'];cli=scope['cli']
p=root/'build/stock-v15-verified.fwsc'
product,image=I.load_package(str(p),stock_v15=True)
assert product=='FM-1_015'
try:I.load_package(str(p))
except I.InstallError:pass
else:raise AssertionError('stock restore must require an explicit flag')
dev=Fake(image,identity='FM-1_929',name='Felucca',after_write='FM-1_015',loader=('ota-FM-1_015','FM-1 Update'))
rc,out,err=cli([str(p),'--stock-v15','--yes'],dev)
assert rc==0 and dev.bad==0 and dev.upgrades==2,(rc,out,err)
print('running 1.2 -> stock v15: checked file, staged loader, wrote and verified returned identity (simulated)')
dev=Fake(image,identity='ota-FM-1_015',name='FM-1 Update',after_write='FM-1_015')
rc,out,err=cli([str(p),'--stock-v15','--yes'],dev)
assert rc==0 and dev.upgrades==1,(rc,err)
print('interrupted official-v15 loader resumes only with the verified v15 file')
dev=Fake(image,identity='ota-FM-1_900',name='Felucca Update')
rc,out,err=cli([str(p),'--stock-v15','--yes'],dev)
assert rc==6 and dev.upgrades==0 and 'finish the interrupted' in err,(rc,err)
print('unfinished custom update refuses stock; finish original package first')
bad=bytearray(p.read_bytes());bad[-1]^=1
q=root/'build/v15-damaged.fwsc';q.write_bytes(bad)
dev=Fake(image)
rc,out,err=cli([str(q),'--stock-v15','--yes'],dev)
assert rc==2 and dev.upgrades==0 and not dev.sent
q.unlink()
print('one-byte-corrupt v15 refused before probing or writing a device')
for name in ['dinghy-1.2-rc2.fwsc','dinghy-1.2-rc1.fwsc','dinghy-1.1-rc2.fwsc','dinghy-1.1-rc1.fwsc','dinghy-1.0-rc2.fwsc','dinghy-0.9.fwsc']:
    q=root/'build'/name
    if not q.exists():continue
    product,image=I.load_package(str(q))
    for start in ['FM-1_929','FM-1_930','FM-1_931','FM-1_932','FM-1_900']:
        dev=Fake(image,identity=start,after_write=product)
        rc,out,err=cli([str(q),'--yes'],dev)
        assert rc==0 and dev.bad==0,(name,start,rc,err)
    print(f'{name}: host install/verify from 0.9, 1.0 and a custom identity pass (simulated)')
print('STOCK AND RELEASE INSTALLER TESTS PASSED')
