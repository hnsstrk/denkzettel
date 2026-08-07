"""Zählt die Stärke jeder Trennlinie in Gerätebildpunkten (Issue #101, L9).

Die Zusicherung des Kunden lautet: „Gleiche Farbe, verschiedene Ausdehnung —
die Rangfolge entsteht aus der Länge des Strichs statt aus seiner Stärke."
Unter seiner Skalierung 1,6 belegt eine Linie von einem logischen Punkt 1,6
Gerätebildpunktzeilen; welche ganzen Zeilen daraus werden, hing vor der
Ausrichtung am Geräteraster davon ab, wo die Zeile im Raster landete. Gemessen
wurden dann mal eine, mal zwei — und eine Gruppenlinie konnte dünner ausfallen
als die Eintragslinien darunter.

Gelesen werden zwei Spalten: eine ganz am Rand, die nur die Gruppenlinie über
die volle Breite erreicht, und eine tief in der eingerückten Zone, die beide
Arten kreuzen. Was in der einen, aber nicht in der anderen steht, ist eine
Eintragslinie.

Aufruf: python3 strichstaerke.py <Bilderordner>
"""

import os
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow fehlt")

# Breeze Dark, Grund und Textfarbe im Verhältnis 0,20 — die Farbe, die der
# Delegate mischt. Steht hier als Zahl, weil dieses Skript das Bild liest und
# nicht den Code; stimmt sie nicht mehr, findet der Lauf keine Linien und sagt
# das, statt stillschweigend nichts zu melden.
LINIE = (66, 68, 70)

RAND_SPALTE = 2
INNEN_SPALTE = 200


def laeufe(px, hoehe, x):
    """Die Läufe der Linienfarbe in Spalte `x` als (Anfang, Höhe)."""
    aus, y = [], 0
    while y < hoehe:
        if px[x, y] == LINIE:
            start = y
            while y < hoehe and px[x, y] == LINIE:
                y += 1
            aus.append((start, y - start))
        else:
            y += 1
    return aus


def main():
    ordner = sys.argv[1]
    gefunden = 0

    for name in sorted(os.listdir(ordner)):
        if not name.endswith(".png"):
            continue

        bild = Image.open(os.path.join(ordner, name)).convert("RGB")
        px = bild.load()
        _, hoehe = bild.size

        rand = laeufe(px, hoehe, RAND_SPALTE)
        innen = laeufe(px, hoehe, INNEN_SPALTE)
        randlagen = {anfang for anfang, _ in rand}

        eintrag = sorted({h for anfang, h in innen if anfang not in randlagen})
        gruppe = sorted({h for _, h in rand})
        gefunden += len(eintrag) + len(gruppe)

        staerken = set(eintrag) | set(gruppe)
        print(name)
        print("  Eintragslinien: %s" % (eintrag or "keine in diesem Bild"))
        print("  Gruppenlinien : %s" % (gruppe or "keine in diesem Bild"))
        print("  Urteil        : %s"
              % ("eine einzige Stärke" if len(staerken) <= 1
                 else "MEHRERE STÄRKEN — die Stärke widerspricht der Länge"))

    # Ohne diese Zeile sähe ein Lauf, der die Linienfarbe nicht mehr trifft,
    # genauso aus wie einer über lauter einheitliche Linien.
    print()
    print("Gefundene Stärkegruppen insgesamt: %d" % gefunden)
    if gefunden == 0:
        sys.exit("Keine einzige Linie gefunden — Farbe oder Spalten stimmen nicht mehr")


if __name__ == "__main__":
    main()
