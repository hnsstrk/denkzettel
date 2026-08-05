#!/usr/bin/env python3
"""Bildmessungen des UI-Reviews zu Sprint 7.

Alle Zahlen des Berichts, die aus einem Bild stammen, entstehen hier — an den
Bildern im Nachbarordner `bilder/`, die dieser Review selbst erzeugt hat.

Aufruf: python3 docs/scrum/reviews/sprint-07-ui-review/messungen/bildvergleich.py
"""

from pathlib import Path

from PIL import Image

BILDER = Path(__file__).resolve().parent.parent / "bilder"
GRUND = 128  # der glatte Untergrund, den die Sonde `weichzeichner … krunner` legt


def lum(c):
    def f(v):
        v = v / 255
        return v / 12.92 if v <= 0.04045 else ((v + 0.055) / 1.055) ** 2.4

    return 0.2126 * f(c[0]) + 0.7152 * f(c[1]) + 0.0722 * f(c[2])


def kontrast(a, b):
    la, lb = lum(a), lum(b)
    hoch, tief = max(la, lb), min(la, lb)
    return (hoch + 0.05) / (tief + 0.05)


def kanten_der_zeile(im, y):
    """Erste und letzte deutlich dunkle Spalte einer Zeile."""
    erste = letzte = None
    for x in range(im.size[0]):
        if im.getpixel((x, y))[0] < 100:
            erste = x if erste is None else erste
            letzte = x
    return erste, letzte


def kantenlauf(im, xlinks, y0, richtung, n=12):
    """Erste dunkle Spalte je Zeile, relativ zur linken Kante — der Eckbogen."""
    lauf = []
    for i in range(n):
        y = y0 + richtung * i
        for x in range(xlinks - 6, xlinks + 70):
            if im.getpixel((x, y))[0] < 100:
                lauf.append(x - xlinks)
                break
    return lauf


def hellster(im, x0, x1, y0, y1):
    return max(im.getpixel((x, y))[0] for y in range(y0, y1) for x in range(x0, x1))


def huelle_gegen_krunner():
    im = Image.open(BILDER / "ux-83-fenster-neben-krunner.png").convert("RGB")
    print("=== Hülle des Erfassungsfensters gegen KRunner (eigener Sitzungslauf) ===")
    print(f"Bild {im.size[0]}x{im.size[1]}, Untergrund {im.getpixel((520, 400))}")

    kl, kr = kanten_der_zeile(im, 30)  # KRunner
    dl, dr = kanten_der_zeile(im, 1050)  # Erfassungsfenster
    print(f"\nKRunner            waagerecht x {kl}..{kr}")
    print(f"Erfassungsfenster  waagerecht x {dl}..{dr}")

    print(f"\nFläche    KRunner {im.getpixel((100, 55))}   Erfassung {im.getpixel((dl + 400, 1050))}")
    print(f"Randpunkt KRunner {im.getpixel((kl, 30))}   Erfassung {im.getpixel((dl, 1050))}")

    print("\nSchattenverlauf nach außen (Rotkanal, Grund = 128)")
    print("  KRunner  :", [im.getpixel((kl - i, 30))[0] for i in range(1, 14)])
    print("  Erfassung:", [im.getpixel((dl - i, 1050))[0] for i in range(1, 14)])

    # untere Kanten
    y = 5
    while im.getpixel((80, y))[0] < 100:
        y += 1
    kunten = y - 1
    y = 1050
    while im.getpixel((50, y))[0] < 100:
        y += 1
    dunten = y - 1
    y = 1050
    while im.getpixel((50, y))[0] < 100:
        y -= 1
    doben = y + 1

    print("\nKantenlauf am Eckbogen (Spalte je Zeile)")
    print("  KRunner   unten links:", kantenlauf(im, kl, kunten, -1))
    print("  Erfassung oben  links:", kantenlauf(im, dl, doben, +1))
    print("  Erfassung unten links:", kantenlauf(im, dl, dunten, -1))
    print(f"  Fenstermaß im Bild: {dr - dl + 1}x{dunten - doben + 1} Bildpunkte")

    print("\nHellster Bildpunkt am Bogen (AK 13 — nichts darf heller sein als der Grund 128)")
    for name, kasten in (
        ("Erfassung oben links ", (25, 70, 885, 920)),
        ("Erfassung oben rechts", (975, 1020, 885, 920)),
        ("Erfassung unten links", (25, 70, 1165, 1200)),
        ("KRunner unten links  ", (55, 95, 45, 80)),
    ):
        print(f"  {name}: {hellster(im, *kasten)}")

    flaeche = im.getpixel((500, 1080))
    print("\nKontraste auf der Kundeneinstellung (Fläche", flaeche, ")")
    for name, kasten in (
        ("App-Name ", (66, 300, 925, 952)),
        ("Notiztext", (70, 700, 968, 1000)),
        ("Fußzeile ", (380, 700, 1140, 1168)),
    ):
        x0, x1, y0, y1 = kasten
        punkte = [im.getpixel((x, y)) for y in range(y0, y1) for x in range(x0, x1)]
        tinte = max(punkte, key=lambda v: abs(lum(v) - lum(flaeche)))
        print(f"  {name}: Tinte {tinte}  {kontrast(tinte, flaeche):.2f} : 1")

    print("\nEinheitlichkeit der Fläche (4b: eine durchgehende Fläche)")
    treffer = gesamt = 0
    for y in range(910, 1176, 3):
        for x in range(48, 996, 7):
            gesamt += 1
            treffer += im.getpixel((x, y)) == (47, 50, 52)
    print(f"  {100 * treffer / gesamt:.2f} % der Stichproben genau (47, 50, 52) — der Rest ist Schrift")


def fremdes_theme():
    im = Image.open(BILDER / "ux-83-fremdes-theme-notiztext-unsichtbar.png").convert("RGB")
    flaeche = im.getpixel((500, 200))
    punkte = [im.getpixel((x, y)) for y in range(66, 84) for x in range(70, 600)]
    tinte = max(punkte, key=lambda v: abs(lum(v) - lum(flaeche)))
    print("\n=== Fremdes Desktop-Theme (breeze-dark) unter hellem Farbschema — #85 ===")
    print(f"Fläche {flaeche}, hellste Tinte des Notiztextes {tinte}")
    print(f"Kontrast Notiztext : {kontrast(tinte, flaeche):.2f} : 1")
    punkte = [im.getpixel((x, y)) for y in range(174, 190) for x in range(230, 420)]
    tinte = max(punkte, key=lambda v: abs(lum(v) - lum(flaeche)))
    print(f"Kontrast Fußzeile  : {kontrast(tinte, flaeche):.2f} : 1")


def rollwert_nach_dem_klick():
    print("\n=== #71: bewegt sich die Liste nach dem Klick? (eigene Bilder 11a/11b) ===")
    for name in ("ux-71-11a-klick-auf-angeschnittene-zeile.png", "ux-71-11b-nach-dem-nachlauf.png"):
        im = Image.open(BILDER / name).convert("RGB")
        ys = [
            y
            for y in range(im.size[1])
            if im.getpixel((200, y))[2] > 180 and im.getpixel((200, y))[0] < 120
        ]
        print(f"  {name}: Markierung y {min(ys)}..{max(ys)}")


if __name__ == "__main__":
    huelle_gegen_krunner()
    fremdes_theme()
    rollwert_nach_dem_klick()
