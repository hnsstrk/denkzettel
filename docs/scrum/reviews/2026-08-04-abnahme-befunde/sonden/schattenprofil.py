#!/usr/bin/env python3
"""Messsonde zu Kundenbefund B2 — wie stark verdunkelt welcher Schatten?

Gemessen wird an den **beiden Eckaufnahmen des Kunden** vom 04.08.2026,
`03-ecke-denkzettel.png` und `04-ecke-nativ.png`.
Sie sind das einzige Material dieser Abnahme mit ruhigem Grund: Der große
Nebeneinander-Schnappschuss liegt auf einem gestreiften Hintergrundbild, dessen
eigene Helligkeit von Zeile zu Zeile um mehr schwankt als der ganze Schatten
ausmacht — dort ist nichts zu messen (nachgerechnet, siehe Bericht).

Gemessen wird waagerecht nach links, von der linken Fensterkante weg, in einer
Zeile weit unterhalb der Ecke, damit die Rundung nicht hineinspielt.

**Zwei Grenzen dieser Messung, ausdrücklich:**

1. Beide Ausschnitte sind abgeschnitten, bevor der Schatten ausläuft. Die
   Reichweiten unten sind deshalb **Untergrenzen**, keine Reichweiten.
2. Die beiden Aufnahmen liegen auf verschiedenem Grund. Vergleichbar ist der
   Verlauf — wie steil, über wie viele Pixel —, nicht der Absolutwert.

Aufruf: python3 schattenprofil.py
"""

import os
import sys

from PIL import Image

HIER = os.path.dirname(os.path.abspath(__file__))
BILDER = os.path.join(HIER, "..", "kundenbilder")

# Datei, Zeile (unterhalb der Ecke), Spalte der äußersten Konturpixel.
AUFNAHMEN = [
    ("03-ecke-denkzettel.png", "Denkzettel (Schatten des Desktop-Themes)", 26, 8),
    ("04-ecke-nativ.png", "natives Fenster (Schatten der Dekoration)", 30, 21),
]


def main():
    print("=== Schattenprofil an der linken Kante ===")
    print("Skalierung des Bildschirms: 1,6 (kscreen-doctor) — die Zahlen sind Gerätepixel.\n")

    for datei, name, zeile, kante in AUFNAHMEN:
        bild = Image.open(os.path.join(BILDER, datei)).convert("RGB")
        px = bild.load()
        werte = [(kante - 1 - x, sum(px[kante - 1 - x, zeile]) / 3) for x in range(kante)]
        hell = werte[-1][1]  # der äußerste Bildpunkt: am wenigsten verschattet
        print(f"--- {name}")
        print(f"    {datei}  {bild.size[0]}x{bild.size[1]}, Zeile {zeile}, Kante bei x={kante}")
        print(f"    Hellster Grund im Ausschnitt (x=0): {hell:.1f}")
        print("    x   Abstand von der Kante   Helligkeit   Verdunkelung gegen x=0")
        for x, v in sorted(werte):
            abstand = kante - x
            print(f"    {x:3d}   {abstand:6d} px            {v:6.1f}      {v - hell:+6.1f}")
        innen = werte[0][1]
        print(f"    Verdunkelung unmittelbar an der Kante: {innen - hell:+.1f} von {hell:.1f}"
              f"  =  {100 * (hell - innen) / hell:.0f} %")
        print(f"    Im Ausschnitt sichtbare Länge des Verlaufs: {kante} Gerätepixel"
              f"  (= {kante / 1.6:.0f} logische) — UNTERGRENZE, der Ausschnitt endet hier.\n")

    return 0


if __name__ == "__main__":
    sys.exit(main())
