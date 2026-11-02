#!/usr/bin/env python3
"""Create a reviewable local release bundle; never publishes or flashes a device."""
from pathlib import Path
import hashlib,json,subprocess,shutil,zipfile,re
R=Path(__file__).resolve().parents[1];O=R/'artifacts';O.mkdir(exist_ok=True)
pkg=json.loads((R/'build/1.3-package.json').read_text());emu=json.loads((R/'build/1.3-compiled.json').read_text());smoke=json.loads((R/'build/smoke-1.3/summary.json').read_text())['results'];assert all(x['status']=='passed' for x in smoke);assert emu['firmware_sha256']==pkg['sha256']
regions=pkg['regions'];MD=f'''# Wanderer 1.3 RC1 verification

## Exact candidate

Identity **FM-1_929**. Package **612,196 bytes**. SHA-256: `{pkg['sha256']}`.

The target was built with the pinned JieLi 20260730.1 toolchain and the SHA-verified AC79 SDK files. No new firmware was flashed or publicly published.

| Region | Used | Limit | Free |
|---|---:|---:|---:|
'''
for key,label in [('app','Application'),('ram','Ordinary RAM'),('pool','Large-buffer pool')]:
 v=regions[key];MD+=f"| {label} | {v['used']:,} B | {v['limit']:,} B | {v['free']:,} B |\n"
MD+='''
## Software checks

'''+f"**{len(smoke)} fresh compile/execute/protocol checks passed.** AddressSanitizer and UndefinedBehaviorSanitizer were enabled for host C tests. LeakSanitizer alone was disabled for this managed runtime.\n\n"+'''- Legacy v1–v5 imports, malformed records, all eight project slots, patch/Anchor snapshots, source-slot preservation, dirty/Undo behavior and Save readback.
- Local keys, sustained chords, voice stealing, quantized capture, two-bar wraps, separate MIDI source/channel ownership, sustain, panic and exact note release after transposition.
- All 100 modern patches audible; 12,800 preset/register pairs; full four-voice/effects stress; 480 hostile engine/filter/routing/register cases after effect-buffer warmup. All outputs bounded, with no sanitizer overflow.
- Phrase-preserving Drift and resolution, low-density phrase-ending Repeat, 12 callback-size runs with identical repeat audio/printed bytes.
- Three-arena Print, two longest 8-second passes, first Undo to dry Anchor, completed-generation Undo/Redo, cancel preserving current generation and immutable audio Anchor.
- Save from inside the editor, modal priority, confirmation cancellation, persistent Save results, complete event sound restoration and audio Anchor restoration surviving Home.
- Stop refreshes stale bright/dim note LEDs in normal and editor/Print scenes; 256 repeated starts/stops; live-key release remains correct.
- Simulated OTA staging, custom loader upgrade/downgrade, authentic-v15 transactions, malformed package rejection and installer protocol behavior.

## Visual verification

964 new native production-renderer captures cover engine contexts, every preset name, both themes, Zoom/control selection, Print states and the audio-duration lock. 2,400 parameter-range/theme/Zoom editor renders pass bounds and label-collision checks; legacy settings range tests remain intact. Contact sheets were inspected for text, spacing, strong selection borders, palette and recovery cues.

58 of 66 existing screens are pixel-identical. The eight intentional differences are Repeat instead of Decay, preset 100's name, three changed modern-synth scope traces and three named-scale views. The licensed icon pipeline independently decodes all 235 masks to the exact source-rendered alpha pixels, including every legacy icon. Only the new Repeat glyph was added; Lucide/Tabler and OFL notices remain present.

## Compiled firmware

'''+f"The exact package above completed **8,800 ms** in fm1-emu 0.3.0, including four FM voices, record, two Print passes, Undo/Redo and stop. Audio: {emu['frames']:,} stereo frames at 44.1 kHz, peak {emu['peak']:,}; seven actual LCD captures were visually checked. Average emulated ISR instruction load: {emu['average_isr_percent']}%.\n\n"+'''The earlier script that sent Record during startup did not capture a loop; it was replaced by the after-startup sequence used for this completed run. The final scenario script/log and exact input firmware hash are retained.

Average emulated load does not establish a worst-case physical audio deadline. The emulator reports incomplete MMIO modeling. Real-device installation/rollback, DAC/speaker listening, button/LED hardware and the 5.805 ms callback deadline remain pending on a physical FM-1. Subjective improvement over stock requires level-matched listening there.

## Protected foundation and limits

All 33 protected platform, HAL, loader, flash layout and recovery source files match the prior baseline. The compressed loader and retained first 16 KiB package head are byte-identical to the hardware-tested 0.9 foundation. The permanent identity is unchanged. New synth/UI code is not covered by old hardware results.

Print is 16 kHz mono block ADPCM, up to 8.192 seconds, with a 44.1 kHz synth/output and compact 16 kHz stereo wet bus. Printed audio lives in RAM and is not saved to flash; project Save stores notes and patch snapshots. Starting an actual next Print consumes the old Undo workspace, while keeping current and Anchor safe. Tempo/length are locked while audio exists. Imported legacy patches retain their voice bank and Decay semantics; ambience is implemented by the new compact effects engine.

## Reproduce

Use the dependencies and original v15 fixture described in README.md and INSTALL_AND_RECOVERY.md. Build with `sh build.sh --release 1.3-RC1`. Run `python3 tools/smoke_1_3.py`; compile/run `tests/capture_1_3.c` and `tests/capture_screens.c`, then `python3 tools/review_screens_1_3.py`. `tools/audit_release_1_3.py` verifies linked limits, identity and unchanged recovery files. The complete source, licensed original assets, generated tables and evidence accompany this candidate.
'''
(R/'docs/TEST_REPORT_1.3.md').write_text(MD)
# Keep README's current byte count/hash synchronized with the exact candidate.
p=R/'README.md';s=p.read_text();s=re.sub(r'The app is [\d,]+ /',f"The app is {regions['app']['used']:,} /",s);s=re.sub(r'SHA-256: `[0-9a-f]+`',f"SHA-256: `{pkg['sha256']}`",s);p.write_text(s)
subprocess.run(['python3','tools/make_1_3_docs.py'],cwd=R,check=True)
E=R/'test-evidence/1.3';E.mkdir(parents=True,exist_ok=True)
for f in ['1.3-package.json','1.3-compiled.json','1.3-screen-review.json','1.3-screen-comparison.json','bank-1.3-analysis.json','1.3-target.log','1.3-editor-save.log','1.3-range.log']:
 shutil.copy2(R/'build'/f,E/f)
for p in (R/'build/smoke-1.3').glob('*.log'):shutil.copy2(p,E/p.name)
shutil.copy2(R/'build/smoke-1.3/summary.json',E/'summary.json');shutil.copy2(R/'build/final-emulator-1.3/compiled.log',E/'compiled-emulator.log')
firmware=R/'dist/LVM-Wanderer-1.3-RC1-FM1.fwsc';shutil.copy2(firmware,O/firmware.name)
shutil.copy2(R/'build/Wanderer-1.3-Sound-Sampler.wav',O/'Wanderer-1.3-Sound-Sampler.wav')
# A source commit may be made after this staging step; the archive is rebuilt afterward.
bundle=O/'LVM-Wanderer-1.3-RC1-Complete.zip';count=0
with zipfile.ZipFile(bundle,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 def add(p,name):
  global count
  z.write(p,'LVM-Wanderer-1.3-RC1/'+name);count+=1
 for p in O.glob('*'):
  if p.suffix in ['.html','.fwsc','.wav']:add(p,p.name)
 for p in [R/'docs/USER_GUIDE_1.3.md',R/'docs/TEST_REPORT_1.3.md',R/'docs/INSTALL_AND_RECOVERY.md']:add(p,p.name)
 for name in ['Wanderer-1.3-All-100-Presets.wav','sampler-order.json','bank-1.3.csv']:add(R/'build'/name,'Auditions/'+name)
 z.writestr('LVM-Wanderer-1.3-RC1/START_HERE.md','''# Wanderer 1.3 RC1

Read Wanderer-1.3-Guide.html for the instrument controls and Print limits. Read Wanderer-1.3-Verification.html for exact checks and pending physical testing.

The installable file is LVM-Wanderer-1.3-RC1-FM1.fwsc, permanent identity FM-1_929. INSTALL_AND_RECOVERY.md explains installation with tools/fm1_install.py and the included known-good recovery/LVM-Wanderer-0.9-FM1.fwsc.

Printed audio is RAM-only. Save stores notes and patches. The exact new firmware still needs physical installation/listening/timing checks on an FM-1.

source/ contains the complete corresponding GPL source, assets, licenses, tests and evidence. Native-Screens/ contains every new native LCD capture. Auditions/ contains all 100 dry preset auditions and the sampler order.
''')
 add(firmware,'source/dist/'+firmware.name)
 for p in (R/'tools').glob('fm1_install.py'):add(p,'tools/'+p.name)
 add(R/'recovery/LVM-Wanderer-0.9-FM1.fwsc','recovery/LVM-Wanderer-0.9-FM1.fwsc')
 for p in sorted(R.rglob('*')):
  if not p.is_file():continue
  rel=p.relative_to(R)
  if any(part in ['.git','build','artifacts','__pycache__'] for part in rel.parts) or rel.parts[0]=='dist':continue
  add(p,'source/'+str(rel))
 pattern=re.compile(r'(editor-e\d-t\d-z\d-p\d\d-k\d|preset-t\d-z\d-p\d\d|print-t\d-z\d-state\d|audio-lock-t\d)\.png$')
 for p in sorted((R/'build/screens-1.3').glob('*.png')):
  if pattern.fullmatch(p.name):add(p,'Native-Screens/'+p.name)
print(json.dumps(dict(bundle_bytes=bundle.stat().st_size,files=count,firmware_sha256=pkg['sha256']),indent=2))
