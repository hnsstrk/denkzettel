#!/usr/bin/env python3
"""Bringen die installierten Desktop-Themes eigene Schriftfarben mit?

Liest je Theme die colors-Datei und stellt Textfarbe gegen Flaechenfarbe.
Reine Dateilesung, kein Bau, keine Aenderung.
"""
import configparser
import pathlib

# Flaechenfarbe, die der native Weg je Theme zeichnet — aus
# docs/scrum/vorberichte/83-native-huelle/messung-b-themefarbe.txt
GEZEICHNET = {
    "default": (30, 34, 51, 216),
    "CachyOS-Nord-round": (30, 34, 51, 255),
    "breeze-dark": (32, 35, 38, 216),
    "breeze-light": (240, 240, 241, 216),
    "Iridescent-round": (0, 0, 0, 51),
    "cachyos-emerald": (0, 0, 0, 7),
    "cachyos-emerald-color": (0, 0, 0, 7),
    "cachyos-emerald-light": (0, 0, 0, 7),
}

# Farbschema des Kunden: WindowText
SCHEMA_TEXT = (102, 194, 242)


def leuchtdichte(rgb):
    def kanal(c):
        c = c / 255
        return c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4
    r, g, b = (kanal(x) for x in rgb)
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def kontrast(a, b):
    la, lb = leuchtdichte(a), leuchtdichte(b)
    hell, dunkel = max(la, lb), min(la, lb)
    return (hell + 0.05) / (dunkel + 0.05)


def lies(wert):
    teile = [t.strip() for t in wert.split(",")]
    return tuple(int(t) for t in teile[:3])


basis = pathlib.Path("/usr/share/plasma/desktoptheme")
print(f"{'Theme':<24} {'colors?':<8} {'Textfarbe':<16} {'gegen Theme-Flaeche':<20} {'Schemaschrift':<14}")
print("-" * 88)

for name in GEZEICHNET:
    ordner = basis / name
    datei = ordner / "colors"
    r, g, b, a = GEZEICHNET[name]
    flaeche = (r, g, b)
    schema_wert = kontrast(SCHEMA_TEXT, flaeche)

    if not datei.exists():
        print(f"{name:<24} {'nein':<8} {'—':<16} {'—':<20} {schema_wert:>6.2f}:1")
        continue

    cfg = configparser.ConfigParser(strict=False)
    cfg.read(datei, encoding="utf-8")
    if not cfg.has_section("Colors:Window"):
        print(f"{name:<24} {'ja':<8} {'keine Window-Gruppe':<16} {'—':<20} {schema_wert:>6.2f}:1")
        continue

    text = lies(cfg["Colors:Window"].get("ForegroundNormal", "0,0,0"))
    eigen = kontrast(text, flaeche)
    print(f"{name:<24} {'ja':<8} {str(text):<16} {eigen:>6.2f}:1{'':<12} {schema_wert:>6.2f}:1")

print()
print("Textfarbe = ForegroundNormal aus [Colors:Window] der Theme-eigenen colors-Datei")
print(f"Schemaschrift = WindowText des Kundenschemas {SCHEMA_TEXT} auf derselben Flaeche")
print("Flaechenfarbe deckend gerechnet; die Deckung steht in messung-b-themefarbe.txt")
