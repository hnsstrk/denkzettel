#!/usr/bin/env python3
"""Schneidet eine Bildschirmaufnahme am Rahmen des Hintergrundfensters zu.

Die Sonde legt unter das Prüffenster ein eigenes Fenster mit magentafarbenem
Rahmen. Was innerhalb dieses Rahmens steht, hat die Sonde selbst gezeichnet —
so kommt der Schatten des Compositors ins Bild, ohne dass vom Schreibtisch des
Kunden ein Bildpunkt in das öffentliche Repository gerät.

Aufruf: zuschnitt.py <Vollaufnahme> <Zieldatei>
"""

import sys

from PIL import Image


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 2

    picture = Image.open(sys.argv[1]).convert("RGB")
    pixels = picture.load()
    width, height = picture.size

    columns: list[int] = []
    rows: list[int] = []
    for y in range(0, height, 2):
        for x in range(0, width, 2):
            red, green, blue = pixels[x, y]
            if red > 200 and green < 70 and blue > 200:
                columns.append(x)
                rows.append(y)

    if not columns:
        print("Kein Rahmen gefunden — lag das Hintergrundfenster im Bild?")
        return 1

    box = (min(columns), min(rows), max(columns) + 1, max(rows) + 1)
    picture.crop(box).save(sys.argv[2])
    print(f"Zuschnitt {box} -> {sys.argv[2]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
