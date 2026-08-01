#!/usr/bin/env python3
"""Misst aus den Fensterbildern die Farbe der Kleintexte und den Kontrast.

Hintergrund = haeufigste Farbe im Streifen, Textfarbe = der Bildpunkt mit dem
groessten Abstand davon (der Kern der Glyphe, von der Kantenglaettung
unberuehrt).
"""
import sys
from collections import Counter
from PIL import Image


def leuchtdichte(rgb):
    def kanal(v):
        v = v / 255.0
        return v / 12.92 if v <= 0.04045 else ((v + 0.055) / 1.055) ** 2.4
    r, g, b = (kanal(c) for c in rgb)
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def kontrast(a, b):
    la, lb = leuchtdichte(a), leuchtdichte(b)
    hell, dunkel = max(la, lb), min(la, lb)
    return (hell + 0.05) / (dunkel + 0.05)


def streifen(bild, y0, y1):
    punkte = [bild.getpixel((x, y))[:3]
              for y in range(y0, y1) for x in range(bild.width)]
    grund = Counter(punkte).most_common(1)[0][0]
    text = max(punkte, key=lambda p: sum((p[i] - grund[i]) ** 2 for i in range(3)))
    return grund, text


def hexfarbe(rgb):
    return "#%02x%02x%02x" % rgb


for pfad in sys.argv[1:]:
    bild = Image.open(pfad).convert("RGB")
    h = bild.height
    print(f"{pfad.split('/')[-1]}  ({bild.width}x{h})")
    for name, y0, y1 in (('Kopf  "Denkzettel"', 4, 20),
                         ('Fuss  "Esc verwirft …"', h - 20, h - 4)):
        grund, text = streifen(bild, y0, y1)
        print(f"  {name:24s} Grund {hexfarbe(grund)}  Text {hexfarbe(text)}"
              f"  Kontrast {kontrast(grund, text):.2f}:1")
