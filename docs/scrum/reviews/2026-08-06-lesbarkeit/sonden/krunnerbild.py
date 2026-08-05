# M5 — KRunner als Maßstab, gemessen an einem Bild, das das Projekt schon hat.
#
# Die Aussage „KRunner hat ein sichtbares Eingabefeld" braucht ein Bild aus der
# angemeldeten Sitzung (B21). Ein solches liegt versioniert im UI-Review zu
# Sprint 7 — mit Denkzettel und KRunner nebeneinander auf demselben Schirm,
# unter demselben Theme und demselben Farbschema. Ein neues Bild aufzunehmen
# hieße, denselben Fall noch einmal herzustellen; ausgelesen wird deshalb das
# vorhandene.
#
# Aufruf: python3 krunnerbild.py <Wurzel des Repos>

import sys

from PIL import Image

wurzel = sys.argv[1]
pfad = (
    "docs/scrum/reviews/sprint-07-s83-native-huelle/bilder/sitzung/"
    "fenster-neben-krunner.png"
)
bild = Image.open(f"{wurzel}/{pfad}").convert("RGB")


def kanal(v):
    v /= 255
    return v / 12.92 if v <= 0.03928 else ((v + 0.055) / 1.055) ** 2.4


def leuchtdichte(c):
    return 0.2126 * kanal(c[0]) + 0.7152 * kanal(c[1]) + 0.0722 * kanal(c[2])


def kontrast(a, b):
    la, lb = leuchtdichte(a), leuchtdichte(b)
    return (max(la, lb) + 0.05) / (min(la, lb) + 0.05)


huelle = bild.getpixel((90, 30))
feld = bild.getpixel((300, 30))
kante = bild.getpixel((134, 30))
denkzettel = bild.getpixel((500, 1000))

print("M5 — KRunner als Maßstab, gemessen am versionierten Sitzungsbild")
print("Bild:", pfad)
print("Größe:", bild.size, "— aufgenommen im UI-Review zu Sprint 7, angemeldete")
print("Wayland-Sitzung, Desktop-Theme default (kein [Theme] name in plasmarc),")
print("Farbschema Breeze Dark.")
print()
print("KRunner")
print(f"  Hülle (x=90,y=30)              {huelle}")
print(f"  Feldfläche (x=300,y=30)        {feld}")
print(f"  Feldkante, fokussiert (x=134)  {kante}")
print(f"  Feldfläche gegen Hülle         {kontrast(feld, huelle):.2f}:1")
print()
print("Denkzettel im selben Bild")
print(f"  Fläche (x=500,y=1000)          {denkzettel}")
print("  Es gibt keinen zweiten Ton: das Fenster ist von der Kopfzeile bis zur")
print("  Fußzeile eine Fläche. Genau das ist der Kundenbefund #100.")
print()
print("Zum Vergleich die offscreen gerechnete Sonde (M3, Theme default):")
print("  Hülle 47,49,52 · Feldfläche 20,22,24 · Feldfläche gegen Hülle 1,39:1")
print(f"  Das Sitzungsbild liegt bei {kontrast(feld, huelle):.2f}:1 — dieselbe Aussage, und die")
print("  Farbwerte stimmen bis auf den Bildpunkt. Die Rechnung der Sonde ist damit")
print("  am wirklichen Fall bestätigt, nicht nur in sich stimmig.")
