# Hängt den Zusammenzug an M4 an: je Trennmittel der schlechteste und der beste
# Fall über alle gemessenen Schemata sowie die Zahl der Schemata unter zwei
# Schwellen. Die Schwellen sind keine Norm — 1,10 : 1 und 1,20 : 1 sind die
# Gegend, in der die Fußzeilen-Linie am 01.08.2026 gescheitert ist, und genau
# darum geht es beim Vergleich.
#
# Aufruf: python3 zusammenzug.py <messungen/m4-farbtafel.txt>

import re
import sys

text = open(sys.argv[1], encoding="utf8").read()


def werte(muster):
    return [float(x) for x in re.findall(muster, text)]


mittel = [
    ("Trennlinie (frameContrast)", werte(r"Trennlinie gegen Base ([\d.]+):1")),
    ("Abwechselnde Zeilenfarbe", werte(r"AlternateBase gegen Base ([\d.]+):1")),
    ("Fläche hinter dem Kopf (Header)", werte(r"Header-Fläche gegen Base ([\d.]+):1")),
]

print()
print()
print("=== Zusammenzug über alle 18 Schemata ===")
print("Jedes Mittel gegen die Fläche, auf der die Liste steht (Base, gemessen in M1).")
print()
print(f"{'Trennmittel':30s}{'schlechtester':>14s}{'bester':>9s}{'<1,10:1':>9s}{'<1,20:1':>9s}")
print("-" * 71)
for name, v in mittel:
    unter10 = sum(1 for x in v if x < 1.10)
    unter20 = sum(1 for x in v if x < 1.20)
    print(f"{name:30s}{min(v):11.2f}:1{max(v):7.2f}:1{unter10:9d}{unter20:9d}")
print()
print("Zum Vergleich die vier Rollen, an denen die Fußzeilen-Linie am 01.08.2026")
print("gescheitert ist (aus M1, gegen denselben Grund): Mid 1,00:1 · Midlight 1,00:1 ·")
print("Dark 1,01:1 · Shadow 1,06:1 im jeweils schlechtesten Fall.")
