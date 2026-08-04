#!/usr/bin/env python3
"""Messsonde zum Kernsatz des Kunden — „Der weiße Hintergrund ragt über den
Rahmen hinaus."

Gemessen wird an seinen beiden Eckaufnahmen, waagerecht durch eine Zeile
mitten im Bogen: vom Schatten außen über die Kontur bis in die Fläche. Die
Frage ist eine Zahlenfrage — steht zwischen Schatten und Kontur ein Bildpunkt,
der **heller** ist als der Grund weit draußen? Dann ist dort weder Fenster noch
Schatten, und genau das sieht der Kunde.

Die Gegenprobe ist die native Ecke derselben Sitzung: Findet sich dort dasselbe,
ist es Plasmas Normalzustand; findet sich dort nichts, ist es unseres.

Aufruf: python3 eckhelligkeit.py
"""

import os
import sys

from PIL import Image

HIER = os.path.dirname(os.path.abspath(__file__))
BILDER = os.path.join(HIER, "..", "kundenbilder")

# Datei, Name, Zeilen mitten im Bogen, Spalten bis einschließlich Fläche.
AUFNAHMEN = [
    ("03-ecke-denkzettel.png", "Denkzettel", [14, 15, 16, 17], 14),
    ("04-ecke-nativ.png", "natives Fenster", [28, 29, 30, 31], 26),
]


def main():
    print("=== Helligkeit quer durch den Bogen ===")
    print("Gelesen wird der Grauwert (Mittel aus R, G, B) je Bildpunkt, von außen nach innen.\n")

    for datei, name, zeilen, breite in AUFNAHMEN:
        bild = Image.open(os.path.join(BILDER, datei)).convert("RGB")
        px = bild.load()
        print(f"--- {name}  ({datei})")
        for y in zeilen:
            werte = [sum(px[x, y]) / 3 for x in range(breite)]
            aussen = werte[0]
            zeile = "  ".join(f"{v:5.1f}" for v in werte)
            print(f"    Zeile {y:2d}:  {zeile}")
            # Der hellste Punkt zwischen dem Grund außen und der Fläche innen.
            spitze = max(range(1, breite), key=lambda x: werte[x])
            if werte[spitze] > aussen:
                print(f"              → hellster Punkt bei x={spitze}: {werte[spitze]:.1f}"
                      f"  =  {werte[spitze] - aussen:+.1f} über dem Grund außen ({aussen:.1f})")
            else:
                print(f"              → kein Punkt heller als der Grund außen ({aussen:.1f})")
        print()

    return 0


if __name__ == "__main__":
    sys.exit(main())
