#!/usr/bin/env python3
"""Schneidet aus einem Sitzungsbild das Fenster heraus — alles, was vom
Sitzungshintergrund abweicht. Aufruf: ausschnitt.py <ziel> <bild>...
"""
import os
import sys
from PIL import Image

ziel = sys.argv[1]
os.makedirs(ziel, exist_ok=True)

for pfad in sys.argv[2:]:
    bild = Image.open(pfad).convert("RGB")
    grund = bild.getpixel((5, 5))
    punkte = [(x, y) for y in range(bild.height) for x in range(bild.width)
              if bild.getpixel((x, y)) != grund]
    xs = [p[0] for p in punkte]
    ys = [p[1] for p in punkte]
    kasten = (min(xs), min(ys), max(xs) + 1, max(ys) + 1)
    name = os.path.basename(pfad)
    print(f"{name}  {kasten}")
    bild.crop(kasten).save(os.path.join(ziel, name))
