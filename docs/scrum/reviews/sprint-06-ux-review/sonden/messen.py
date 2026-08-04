#!/usr/bin/env python3
"""Misst die Bilder der UX-Prüfung gegen Wireframe 4a/4b. Liest nur, schreibt nichts."""
import sys
from PIL import Image
import numpy as np

FRAME = 24
HATCH = [(0xf2, 0xf0, 0xeb), (0xe9, 0xe7, 0xe2)]


def is_hatch(px):
    return any(abs(int(px[0]) - h[0]) <= 1 and abs(int(px[1]) - h[1]) <= 1
               and abs(int(px[2]) - h[2]) <= 1 for h in HATCH)


def measure(path):
    im = Image.open(path).convert("RGB")
    a = np.asarray(im).astype(int)
    ph, pw = a.shape[:2]
    W, H = pw - 2 * FRAME, ph - 2 * FRAME
    win = a[FRAME:FRAME + H, FRAME:FRAME + W]

    hatchmask = np.zeros((H, W), dtype=bool)
    for h in HATCH:
        hatchmask |= (np.abs(win - np.array(h)).max(axis=2) <= 1)

    # Eckanlauf: wie viele Pixel der obersten Fensterzeile bleiben unbeansprucht
    top = hatchmask[0]
    corner_left = int(np.argmin(top)) if not top.all() else W
    # Randanlauf am linken Rand analog (oberste Spalte)
    left = hatchmask[:, 0]
    corner_top = int(np.argmin(left)) if not left.all() else H

    # Flaechenfarbe: Mitte des Fensters, weit weg von Text
    mid = win[H // 2, W // 2]

    # Farbe hinter dem Textfeld gegen Farbe daneben (auf gleicher Hoehe)
    # Zeile knapp unter dem App-Namen, links im Textbereich vs. ganz rechts
    def rowcolors(y):
        return win[y, 30], win[y, W - 30]

    # Konturring: erste nicht-hatch Zeile in der Fenstermitte-Spalte
    col = hatchmask[:, W // 2]
    first = int(np.argmin(col))
    ring = [tuple(win[first + k, W // 2]) for k in range(0, 4)]

    # Tinte: Zeilen mit Pixeln, die deutlich von der Flaeche abweichen
    surface = mid
    dev = np.abs(win - surface).max(axis=2)
    inkmask = dev > 40
    # Rand ausschliessen: nur Innenbereich betrachten
    inner = inkmask[6:H - 6, 6:W - 6]
    rows = np.where(inner.any(axis=1))[0] + 6
    bands = []
    if len(rows):
        start = rows[0]
        prev = rows[0]
        for r in rows[1:]:
            if r - prev > 2:
                bands.append((start, prev))
                start = r
            prev = r
        bands.append((start, prev))

    print(f"== {path.split('/')[-1]}")
    print(f"   Bild {pw}x{ph}   Fenster {W}x{H}")
    print(f"   Eckanlauf oben-links: horizontal {corner_left} px, vertikal {corner_top} px")
    print(f"   Flaeche Mitte: {tuple(mid)}")
    print(f"   Ringfarben ab Oberkante (Mittelspalte): {ring}")
    print(f"   Tintenbaender (y innerhalb des Fensters):")
    for (s, e) in bands:
        # linkeste Tintenspalte dieses Bandes
        seg = inkmask[s:e + 1, :]
        cols = np.where(seg.any(axis=0))[0]
        print(f"      y {s}..{e}  (Hoehe {e - s + 1})  x {cols.min()}..{cols.max()}")
    return bands, W, H


for p in sys.argv[1:]:
    measure(p)
    print()
