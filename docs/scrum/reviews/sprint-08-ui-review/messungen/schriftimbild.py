#!/usr/bin/env python3
"""Misst die Schrift **im Bild** — nicht an der Palette.

Warum das ein eigenes Prüfmittel ist: Die Belegsonde des Strangs liest die
Farben aus der Palette des Textfeldes und rechnet sie gegen die Fläche aus der
Aufnahme. Damit ist belegt, welche Farbe das Fenster *tragen soll*. Ob es
tatsächlich mit ihr zeichnet — und wie die Schrift nach dem Kantenausgleich auf
dieser Fläche ankommt —, steht in derselben Zahl nicht. Dieses Skript nimmt
allein die Aufnahme und kennt keine Palette.

Vorgehen je Bild:
  * Die Fläche kommt aus dem Bild des **Leerzustands**, aus einem Band unterhalb
    des Platzhaltertextes und oberhalb der Fußzeile — dort steht keine Schrift.
  * Der Ausschnitt enthält Schatten und blanken Grund. Beide werden über den
    **Zeilen- und Spaltenmedian** herausgeschnitten: Schrift ist in ihrer Zeile
    eine Minderheit und verschiebt den Median nicht, der Grund verschiebt ihn um
    ein Vielfaches. Ohne diesen Schritt meldet das Skript bei dunkler Hülle über
    hellem Grund **ein** Band über die ganze Bildhöhe — der erste Lauf tat das
    an vier Bildern, und die Zahl daraus sah aus wie eine Schriftmessung.
  * Die Textbänder werden über das Zeilenprofil gesucht: je Zeile die Zahl der
    Bildpunkte, die um mehr als `SCHWELLE` von der Fläche abweichen.
  * Je Band ist die Schriftfarbe der Mittelwert des oberen Drittels **der
    abweichenden** Bildpunkte — der Kern der Glyphe, nicht ihr
    ausgeglichener Rand. Der Rand zöge jede Zahl zur Fläche hin und machte die
    Schrift besser aussehend, als sie ist.

Wo Hülle und Grund fast dieselbe Farbe haben (`cachyos-emerald` über Weiß:
Fläche 248 gegen Grund 255), trennt kein Medianschnitt mehr — dort deckt die
Hülle zu wenige Prozent, und das ist selbst der Befund. Solche Bilder sind an
der Zahl der gefundenen Bänder zu erkennen und werden nicht zurechtgelegt.

Aufruf: schriftimbild.py <Bilderordner>
"""

import sys
import pathlib
import numpy as np
from PIL import Image

SCHWELLE = 30


def kanal(c):
    v = c / 255.0
    return np.where(v <= 0.03928, v / 12.92, ((v + 0.055) / 1.055) ** 2.4)


def leuchtdichte(rgb):
    r, g, b = kanal(np.array(rgb, dtype=float))
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def kontrast(a, b):
    la, lb = leuchtdichte(a), leuchtdichte(b)
    return (max(la, lb) + 0.05) / (min(la, lb) + 0.05)


def baender(abweichung, mindestbreite=8):
    """Zusammenhängende Zeilenbänder, in denen Schrift steht."""
    zeilen = (abweichung > SCHWELLE).sum(axis=1)
    aktiv = zeilen > mindestbreite
    ergebnis, start = [], None
    for y, an in enumerate(aktiv):
        if an and start is None:
            start = y
        elif not an and start is not None:
            if y - start >= 4:
                ergebnis.append((start, y))
            start = None
    if start is not None:
        ergebnis.append((start, len(aktiv)))
    return ergebnis


def schriftfarbe(bild, flaeche, y0, y1):
    """Der Kern der Glyphen dieses Bandes, oder None.

    Ausgewählt wird **relativ zur Schwelle** und nicht als fester Anteil der
    Zeile. Ein fester Anteil trägt, solange die Zeile voll Text ist, und kippt
    bei kurzem: Der erste Lauf nahm die obersten 5 % aller Bildpunkte des Bandes
    und maß für den App-Namen — zehn Zeichen auf 957 Bildpunkten Breite —
    überwiegend Fläche. Herausgekommen ist 1,04:1 für eine Schrift, die in
    Wahrheit bei 4,06:1 steht.
    """
    teil = bild[y0:y1].reshape(-1, 3).astype(float)
    abstand = np.abs(teil - flaeche).sum(axis=1)
    schrift = teil[abstand > SCHWELLE]
    if schrift.shape[0] < 20:
        return None
    abstaende = abstand[abstand > SCHWELLE]
    kern = schrift[abstaende >= np.quantile(abstaende, 0.70)]
    return kern.mean(axis=0)


def fensterinneres(leer, flaeche):
    """Der Bereich des Ausschnitts, in dem die Fläche des Fensters steht."""
    h, b = leer.shape[:2]
    kern = leer[:, int(b * 0.30):int(b * 0.70)].astype(float)
    zeile = np.abs(np.median(kern, axis=1) - flaeche).sum(axis=1) < 20
    ys = np.flatnonzero(zeile)
    if ys.size == 0:
        return 0, h, 0, b
    y0, y1 = int(ys[0]), int(ys[-1]) + 1

    kern2 = leer[y0:y1].astype(float)
    spalte = np.abs(np.median(kern2, axis=0) - flaeche).sum(axis=1) < 20
    xs = np.flatnonzero(spalte)
    if xs.size == 0:
        return y0, y1, 0, b
    return y0, y1, int(xs[0]), int(xs[-1]) + 1


def auswerten(normal, leer):
    n = np.array(Image.open(normal).convert("RGB"))
    l = np.array(Image.open(leer).convert("RGB"))
    if n.shape != l.shape:
        return None

    # Die Fläche aus dem Leerbild, aus dem schriftfreien Band der unteren Hälfte.
    h = n.shape[0]
    band = l[int(h * 0.45):int(h * 0.70), int(n.shape[1] * 0.3):int(n.shape[1] * 0.7)]
    flaeche = np.median(band.reshape(-1, 3), axis=0)

    y0, y1, x0, x1 = fensterinneres(l, flaeche)
    innen = n[y0:y1, x0:x1]
    abweichung = np.abs(innen.astype(float) - flaeche).sum(axis=2)
    gefunden = [(a + y0, b + y0) for a, b in baender(abweichung)]
    return flaeche, gefunden, n[:, x0:x1], (y0, y1, x0, x1)


def main():
    ordner = pathlib.Path(sys.argv[1])
    print("### Schrift im Bild gemessen — UI-Review Sprint 8, #85")
    print("Fläche aus dem Leerbild, Schriftfarbe aus dem Glyphenkern (oberes"
          " Drittel der abweichenden Bildpunkte).\n")
    for normal in sorted(ordner.glob("fenster-*.png")):
        if normal.stem.endswith("-leer"):
            continue
        leer = normal.with_name(normal.stem + "-leer.png")
        if not leer.exists():
            continue
        ergebnis = auswerten(normal, leer)
        if ergebnis is None:
            print(f"{normal.name}: Bildgrößen gehen auseinander — übersprungen")
            continue
        flaeche, gefunden, n, rahmen = ergebnis
        y0, y1, x0, x1 = rahmen
        print(f"--- {normal.name}")
        print(f"    Fläche im Bild: {tuple(int(v) for v in flaeche)}"
              f"   Fensterinneres y {y0}–{y1}, x {x0}–{x1}"
              f"   Textbänder: {len(gefunden)}")
        # Erwartet sind drei: App-Name, Notiztext, Fußzeile. Was mehr oder
        # weniger findet, wird gemeldet und nicht stillschweigend zurechtgelegt.
        namen = ["App-Name", "Notiztext", "Fußzeile"]
        for i, (y0, y1) in enumerate(gefunden):
            farbe = schriftfarbe(n, flaeche, y0, y1)
            name = namen[i] if len(gefunden) == 3 else f"Band {i + 1}"
            if farbe is None:
                print(f"    {name:<10} y {y0:>3}–{y1:<3} zu wenige abweichende "
                      f"Bildpunkte — keine Zahl")
                continue
            k = kontrast(farbe, flaeche)
            marke = "   UNTER 4,5:1" if k < 4.5 else ""
            print(f"    {name:<10} y {y0:>3}–{y1:<3} "
                  f"Farbe {tuple(int(v) for v in farbe)!s:<18} "
                  f"Kontrast {k:5.2f}:1{marke}")
        print()


if __name__ == "__main__":
    main()
