#!/usr/bin/env python3
"""Validate the hand-curated Wanderer patch bank and emit target tables/catalog."""
import csv
import sys
from pathlib import Path

FIELDS = ("osc1", "osc2", "mix", "ratio", "detune", "attack", "decay",
          "sustain", "release", "cutoff", "contour", "contour_decay", "fm", "fm_decay")
WAVES = {"S": "WAVE_SAW", "H": "WAVE_HOLLOW", "G": "WAVE_GLASS",
         "N": "WAVE_SINE", "T": "WAVE_TRI"}


def main(header, catalog):
    path = Path(__file__).resolve().parent.parent / "assets/wanderer_presets.csv"
    with path.open(newline="") as f:
        rows = list(csv.DictReader(f))
    assert len(rows) == 100, f"expected 100 patches, got {len(rows)}"
    assert len({r["name"] for r in rows}) == 100, "patch names must be unique"
    assert len({tuple(r[k] for k in FIELDS) for r in rows}) == 100, "patch data must be unique"
    assert len({r["bank"] for r in rows}) == 10
    output = [
        "/* Original LVM patch bank; generated from assets/wanderer_presets.csv. */",
        "#pragma once",
        "#define WAVE_SINE 3u",
        "#define WAVE_TRI 4u",
        "typedef struct {",
        "    uint8_t osc1,osc2,mix,ratio,detune,attack,decay,sustain,release;",
        "    uint8_t cutoff,contour,contour_decay,fm,fm_decay;",
        "} wm_preset_t;",
        "static const wm_preset_t WM_PRESETS[WM_PRESET_COUNT] = {",
    ]
    lines = [
        "# Wanderer 0.9 — 100 original sound presets",
        "",
        "Turn PRESETS for the next or previous sound; turn SELECT to move by ten.",
        "The large on-device number is the preset number below. SEL opens settings;",
        "Zoom is available on its system page. Save slot 1 while stopped (SEQ, SEL)",
        "to remember the selected sound. Existing notes keep their old sound until",
        "retriggered. These are original patches inspired by broad synthesis styles,",
        "not exact replicas or sampled sounds from the named manufacturers.",
        "",
        "| No. | Bank | Preset | Sound idea |",
        "|---:|---|---|---|",
    ]
    descriptions = {
        "Foundations": "A versatile oscillator voice",
        "Vintage Poly": "Warm layered analog-style tones",
        "Mono & Lead": "Direct, singing leads",
        "Bass Lab": "Low-end voices and short filter contours",
        "Plucks & Keys": "Short attacks and decaying keys",
        "FM Bells": "Two-operator phase-modulated tones",
        "Digital Motion": "Hybrid digital harmonics and motion",
        "Pads & Atmospheres": "Slow, soft sustained textures",
        "Brass & Organic": "Breathy and reed-like contours",
        "Experimental": "Unusual ratios, attacks and timbres",
    }
    for index, row in enumerate(rows, 1):
        assert int(row["number"]) == index
        assert row["osc1"] in WAVES and row["osc2"] in WAVES
        assert row["bank"] in descriptions
        assert (index - 1) // 10 == list(descriptions).index(row["bank"])
        values = []
        for field in FIELDS:
            if field.startswith("osc"):
                values.append(WAVES[row[field]])
            else:
                n = int(row[field])
                assert 0 <= n <= 255, f"{index}: {field} out of byte range"
                values.append(str(n))
        assert 0 <= int(row["mix"]) <= 127
        assert 1 <= int(row["ratio"]) <= 64
        assert 1 <= int(row["attack"]) <= 127
        assert 1 <= int(row["decay"]) <= 127
        assert 1 <= int(row["release"]) <= 127
        assert 1 <= int(row["contour_decay"]) <= 127
        assert 1 <= int(row["fm_decay"]) <= 127
        assert 18 <= int(row["cutoff"]) <= 220
        output.append("    {" + ",".join(values) + "}, /* %03d %s */" % (index, row["name"]))
        lines.append(f'| {index:03d} | {row["bank"]} | {row["name"]} | {descriptions[row["bank"]]} |')
    output.append("};")
    Path(header).write_text("\n".join(output) + "\n")
    Path(catalog).write_text("\n".join(lines) + "\n")
    print("100 curated patch definitions and numbered catalog")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
