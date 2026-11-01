#!/usr/bin/env python3
"""Exercise the actual host voice through all 100 sound numbers."""
from pathlib import Path
import hashlib
import math
import struct
import subprocess
import wave

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/preset-audit"
OUT.parent.mkdir(parents=True, exist_ok=True)
SCRIPT = OUT.with_suffix(".txt")
lines = [
    "# Disable the default chorus and reverb to isolate the sound designs.",
    "0 btn 3", "30 enc 6 1", "60 btn 3",
    "90 enc 0 2", "120 btn 3", "150 btn 8",
]
START, SPACING = 240, 240
for i in range(100):
    t = START + i * SPACING
    if i:
        lines.append(f"{t} enc 6 1")
    lines.extend((f"{t+30} keys 128", f"{t+170} keys 0"))
    if i % 10 == 9:
        lines.append(f"{t+70} shot")
lines.append(f"{START+100*SPACING} end")
SCRIPT.write_text("\n".join(lines) + "\n")
with (OUT.with_suffix(".log")).open("w") as log:
    subprocess.run([str(ROOT / "build/host/dinghy"), str(OUT), str(SCRIPT)],
                   cwd=ROOT, check=True, stdout=log)
with wave.open(str(OUT.with_suffix(".wav"))) as f:
    rate = f.getframerate()
    samples = struct.unpack("<" + "h" * (f.getnframes() * 2),
                            f.readframes(f.getnframes()))
    assert max(map(abs, samples)) < 32767, "preset sweep clipped"
    rms = []
    fingerprints = []
    for i in range(100):
        begin = int((START + i*SPACING + 55) * rate / 1000)
        end = int((START + i*SPACING + 160) * rate / 1000)
        segment = samples[begin*2:end*2:2]
        rms.append(math.sqrt(sum(x*x for x in segment) / len(segment)))
        fingerprints.append(hashlib.sha256(struct.pack('<'+'h'*len(segment),*segment)).digest())
    assert min(rms) > 20, f"inaudible patch {rms.index(min(rms))+1}: {min(rms):.1f}"
    assert max(rms) / min(rms) < 45, "unbalanced preset levels"
    assert len({round(x / 25) for x in rms}) >= 25, "preset dynamics do not vary"
    assert len(set(fingerprints)) == 100, "two presets rendered the same audition"
    print(f"100 audible presets: RMS {min(rms):.0f}–{max(rms):.0f}, peak {max(map(abs,samples))}")
