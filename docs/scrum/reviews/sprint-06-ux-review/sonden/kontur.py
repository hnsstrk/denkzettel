#!/usr/bin/env python3
"""Prüft die Kontur (4b: Mischung Window/WindowText mit frameContrast 0,20,
die einzige Linie im Fenster) an allen vier Kanten und in den vier Ecken."""
from PIL import Image
import numpy as np

FRAME = 24
BASE = "/home/hnsstrk/Projekte/denkzettel/docs/scrum/reviews/sprint-06-ux-review/bilder/"


def load(name):
    a = np.asarray(Image.open(BASE + name).convert("RGB")).astype(int)
    ph, pw = a.shape[:2]
    W, H = pw - 2 * FRAME, ph - 2 * FRAME
    return a[FRAME:FRAME + H, FRAME:FRAME + W], W, H


def show(name, surface, contour):
    win, W, H = load(name)
    print(f"== {name}  Fenster {W}x{H}")
    print(f"   Soll: Flaeche {surface}  Kontur {contour}")
    mid_y, mid_x = H // 2, W // 2
    for label, pixels in [
        ("oben   (Mittelspalte, 0..3)", [tuple(win[k, mid_x]) for k in range(4)]),
        ("unten  (Mittelspalte, H-1..H-4)", [tuple(win[H - 1 - k, mid_x]) for k in range(4)]),
        ("links  (Mittelzeile, 0..3)", [tuple(win[mid_y, k]) for k in range(4)]),
        ("rechts (Mittelzeile, W-1..W-4)", [tuple(win[mid_y, W - 1 - k]) for k in range(4)]),
    ]:
        print(f"   {label}: {pixels}")

    # Diagonale der linken oberen Ecke: der Punkt, an dem die Rundung sitzt
    print("   Ecke oben links, Diagonale (y=x=0..9):",
          [tuple(win[k, k]) for k in range(10)])
    print("   Ecke unten rechts, Diagonale:",
          [tuple(win[H - 1 - k, W - 1 - k]) for k in range(10)])

    # Wie nah kommt die dunkelste Stelle der Eckdiagonale an die Konturfarbe?
    diag = np.array([win[k, k] for k in range(12)])
    d_contour = np.abs(diag - np.array(contour)).max(axis=1)
    print(f"   geringste Abweichung von der Konturfarbe auf der Eckdiagonale: {d_contour.min()}"
          f" (Kanten erreichen 0)")


mix = lambda a, b, t: tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))
light_surface = (0xef, 0xf0, 0xf1)
light_text = (0x23, 0x26, 0x29)
dark_surface = (0x20, 0x23, 0x26)
dark_text = (0xfc, 0xfc, 0xfc)

show("01-rand-schmal-hell-leer.png", light_surface, mix(light_surface, light_text, 0.20))
show("07-rand-breit-hell-leer.png", light_surface, mix(light_surface, light_text, 0.20))
show("10-rand-breit-dunkel-leer.png", dark_surface, mix(dark_surface, dark_text, 0.20))
