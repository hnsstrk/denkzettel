#!/usr/bin/env python3
"""Eckprofil und Lupe (Wireframe 4a zeichnet die linke obere Ecke 4-fach)."""
import sys
from PIL import Image, ImageDraw
import numpy as np

FRAME = 24
HATCH = [(0xf2, 0xf0, 0xeb), (0xe9, 0xe7, 0xe2)]
BASE = "/home/hnsstrk/Projekte/denkzettel/docs/scrum/reviews/sprint-06-ux-review/bilder/"


def hatchmask(win):
    m = np.zeros(win.shape[:2], dtype=bool)
    for h in HATCH:
        m |= (np.abs(win - np.array(h)).max(axis=2) <= 1)
    return m


def profile(path, rows=14):
    im = Image.open(BASE + path).convert("RGB")
    a = np.asarray(im).astype(int)
    ph, pw = a.shape[:2]
    W, H = pw - 2 * FRAME, ph - 2 * FRAME
    win = a[FRAME:FRAME + H, FRAME:FRAME + W]
    m = hatchmask(win)
    out = []
    for y in range(rows):
        row = m[y]
        out.append(int(np.argmin(row)) if not row.all() else -1)
    return out, W, H


for name in ["01-rand-schmal-hell-leer.png", "07-rand-breit-hell-leer.png"]:
    p, W, H = profile(name)
    print(f"{name}: Fenster {W}x{H}")
    print(f"  Einzug je Zeile ab Oberkante (0..13): {p}")

# Lupe: linke obere Ecke, 8-fach, beide Themes nebeneinander
SIZE = 20
ZOOM = 8
tiles = []
for name, label in [("01-rand-schmal-hell-leer.png", "schmal (breeze-dark, Rand 4)"),
                    ("07-rand-breit-hell-leer.png", "breit (CachyOS-Nord-round, Rand 8)")]:
    im = Image.open(BASE + name).convert("RGB")
    crop = im.crop((FRAME - 4, FRAME - 4, FRAME - 4 + SIZE, FRAME - 4 + SIZE))
    tiles.append((crop.resize((SIZE * ZOOM, SIZE * ZOOM), Image.NEAREST), label))

gap, pad, texth = 24, 16, 20
w = pad * 2 + tiles[0][0].width * 2 + gap
h = pad * 2 + tiles[0][0].height + texth
sheet = Image.new("RGB", (w, h), (255, 255, 255))
d = ImageDraw.Draw(sheet)
x = pad
for tile, label in tiles:
    sheet.paste(tile, (x, pad))
    d.rectangle([x - 1, pad - 1, x + tile.width, pad + tile.height], outline=(120, 120, 120))
    d.text((x, pad + tile.height + 5), label, fill=(60, 60, 60))
    x += tile.width + gap
sheet.save(BASE + "18-eckenlupe-8-fach.png")
print("geschrieben: 18-eckenlupe-8-fach.png", sheet.size)
