#!/usr/bin/env python3
"""Nachmessung zur PO-Rückfrage vom 05.08.2026: Ist unsere Ecke runder als die
von KRunner?

Der Kantenlauf einer Fensterecke hängt davon ab, **wo man die Kante ansetzt**.
Zwischen Hülle und Untergrund liegt der Schatten, und der hat seinen eigenen,
viel weicheren Bogen. Eine Zahl ohne die Schwelle, aus der sie stammt, ist
deshalb nicht vergleichbar — dieses Programm misst über einen Fächer von
Schwellen und legt zusätzlich die Grauwerte selbst nebeneinander.

Aufruf: python3 docs/scrum/reviews/sprint-07-ui-review/messungen/eckenvergleich.py
"""

from pathlib import Path

from PIL import Image

REVIEWS = Path(__file__).resolve().parents[2]
EIGENES = REVIEWS / "sprint-07-ui-review" / "bilder" / "ux-83-fenster-neben-krunner.png"
STRANG_A = REVIEWS / "sprint-07-s83-native-huelle" / "bilder" / "sitzung" / "fenster-neben-krunner.png"
SITZUNGS_GRAB = REVIEWS / "sprint-07-ui-review" / "bilder" / "ux-83-sitzung-fenster-ruhe.png"


def grau(c):
    return (c[0] * 299 + c[1] * 587 + c[2] * 114) // 1000


def zeilenkanten(im, yband, xband, schwelle):
    """Je Zeile des Bandes die erste und letzte Spalte unter der Schwelle.

    Der Zeilenscan über die volle Breite ist nötig: Ein Aufwärtslauf von einem
    Punkt im Fenster bleibt an der Schrift hängen und liefert einen zu kleinen
    Kasten — diese Falle hat den ersten Anlauf dieser Messung verdorben.
    """
    px = im.load()
    zeilen = {}
    for y in range(*yband):
        erste = letzte = None
        for x in range(*xband):
            if grau(px[x, y]) < schwelle:
                erste = x if erste is None else erste
                letzte = x
        zeilen[y] = (erste, letzte)
    return zeilen


def kasten(zeilen):
    belegt = {y: v for y, v in zeilen.items() if v[0] is not None}
    return (
        min(v[0] for v in belegt.values()),
        min(belegt),
        max(v[1] for v in belegt.values()),
        max(belegt),
    )


def lauf(zeilen, reihen, kante, seite):
    out = []
    for y in reihen:
        erste, letzte = zeilen.get(y, (None, None))
        if erste is None:
            out.append(None)
        else:
            out.append(erste - kante if seite == "l" else kante - letzte)
    return out


def faecher(pfad, name, dzband, krband):
    im = Image.open(pfad).convert("RGB")
    breite = (0, im.size[0])
    print("=" * 74)
    print(f"{name}  ({im.size[0]}x{im.size[1]})")
    for schwelle in (100, 110, 118, 124):
        zd = zeilenkanten(im, dzband, breite, schwelle)
        zk = zeilenkanten(im, krband, breite, schwelle)
        dl, do, dr, du = kasten(zd)
        kl, ko, kr, ku = kasten(zk)
        was = "nur die Hülle" if schwelle == 100 else "Hülle und Schatten"
        print(f"\n  -- Grauwert < {schwelle} ({was})")
        print(f"     Erfassung  x {dl}..{dr}  y {do}..{du}")
        print(f"     KRunner    x {kl}..{kr}  y {ko}..{ku}")
        print("     Erfassung oben  links :", lauf(zd, range(do, do + 8), dl, "l"))
        print("     Erfassung unten links :", lauf(zd, range(du, du - 8, -1), dl, "l"))
        print("     Erfassung unten rechts:", lauf(zd, range(du, du - 8, -1), dr, "r"))
        print("     KRunner   unten links :", lauf(zk, range(ku, ku - 8, -1), kl, "l"))
        print("     KRunner   unten rechts:", lauf(zk, range(ku, ku - 8, -1), kr, "r"))


def gitter(im, x0, y0, dx, dy, n=8):
    px = im.load()
    return [[grau(px[x0 + dx * j, y0 + dy * i]) for j in range(n)] for i in range(n)]


def zeige(g, titel):
    print(f"\n  {titel}")
    for r in g:
        print("     " + " ".join(f"{v:4d}" for v in r))


def uebergang(g):
    """Je Zeile die Spalte, in der der Wert erstmals unter 100 fällt."""
    return [next((j for j, v in enumerate(r) if v < 100), None) for r in g]


def grauwerte(pfad):
    im = Image.open(pfad).convert("RGB")
    print("=" * 74)
    print("Grauwerte am Eckscheitel — ohne Schwelle, Zahl gegen Zahl")
    # Hüllenkästen bei Schwelle 100: Erfassung x 42..1001 y 904..1181,
    # KRunner x 72..972 y 0..60
    dz_oben = gitter(im, 42, 904, +1, +1)
    dz_unten = gitter(im, 42, 1181, +1, -1)
    kr_unten = gitter(im, 72, 60, +1, -1)
    kr_unten_r = gitter(im, 972, 60, -1, -1)
    zeige(dz_oben, "Erfassungsfenster, obere linke Ecke")
    zeige(dz_unten, "Erfassungsfenster, untere linke Ecke (senkrecht gespiegelt)")
    zeige(kr_unten, "KRunner, untere linke Ecke (senkrecht gespiegelt)")
    print("\n  Spalte des Übergangs je Zeile (erster Wert unter 100)")
    print("     Erfassung oben :", uebergang(dz_oben))
    print("     Erfassung unten:", uebergang(dz_unten))
    print("     KRunner  unten :", uebergang(kr_unten))
    print("     KRunner  unten rechts (Eigenkontrolle):", uebergang(kr_unten_r))


def grab_asymmetrie(pfad):
    """Warum die untere Ecke eine Zeile kürzer ausläuft als die obere."""
    im = Image.open(pfad).convert("RGBA")
    W, H = im.size
    px = im.load()
    print("=" * 74)
    print(f"Sitzungs-Grab des Erfassungsfensters: {W}x{H}")
    print(f"600x174 logisch bei 1,6 wären {600 * 1.6:.1f} x {174 * 1.6:.1f}")
    for name, y in (
        ("oberste Zeile   ", 0),
        ("zweitoberste    ", 1),
        ("zweitunterste   ", H - 2),
        ("unterste Zeile  ", H - 1),
    ):
        print(f"  Alpha {name}:", [px[x, y][3] for x in range(10)])


if __name__ == "__main__":
    faecher(EIGENES, "Mein eigenes Bild", (860, 1220), (0, 100))
    faecher(STRANG_A, "Bild von Strang A", (860, 1220), (0, 100))
    grauwerte(EIGENES)
    if SITZUNGS_GRAB:
        grab_asymmetrie(SITZUNGS_GRAB)
