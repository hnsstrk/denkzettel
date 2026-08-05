# -*- coding: utf-8 -*-
"""Kontrast beider Textklassen je Desktop-Theme — Bearbeiter B, Vorpruefung #85.

Gedreht wird die Theme-Achse bei festem Farbschema des Kunden. Gerechnet wird
dreierlei, und die Unterscheidung ist der Punkt:

  deckend  — Textfarbe gegen die Flaechenfarbe, als waere sie undurchsichtig.
             Das ist die Rechnung aus `po-themeschrift.py`; bei 2,7 % Deckung
             rechnet sie gegen eine Flaeche, die es nicht gibt.
  ungst.   — Flaeche mit ihrem gemessenen Alpha ueber weissem bzw. schwarzem
             Grund zusammengesetzt, davon der schlechtere Wert. Methode aus
             `ux-beratung/sonden/kleintext.py`, nur mit der anderen Achse.

Reine Dateilesung: /usr/share/plasma/desktoptheme/<theme>/colors und die
Flaechenwerte aus messung-b-themefarbe.txt. Kein Bau, keine Aenderung.
"""
import configparser
import pathlib

# Flaeche, die der native Weg je Theme zeichnet — RGB und Alpha.
# Quelle: docs/scrum/vorberichte/83-native-huelle/messung-b-themefarbe.txt
# (durchscheinende Fassung, also der Fall der angemeldeten Kundensitzung;
# ohne weichzeichnenden Compositor greift seit #83 der Auswahlpfad `opaque`).
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

# Farbschema des Kunden, CachyOSNordLightly, Gruppe [Colors:Window].
# Quelle: docs/scrum/vorberichte/83-native-huelle/sm-textrolle.txt
SCHEMA_WINDOWTEXT = (102, 194, 242)
SCHEMA_FOREGROUNDINACTIVE = (102, 106, 115)

BASIS = pathlib.Path("/usr/share/plasma/desktoptheme")


def lum(c):
    def ch(v):
        v = v / 255.0
        return v / 12.92 if v <= 0.03928 else ((v + 0.055) / 1.055) ** 2.4
    return 0.2126 * ch(c[0]) + 0.7152 * ch(c[1]) + 0.0722 * ch(c[2])


def kontrast(a, b):
    la, lb = lum(a), lum(b)
    return (max(la, lb) + 0.05) / (min(la, lb) + 0.05)


def ueber(front, dahinter, alpha):
    a = alpha / 255.0
    return tuple(front[i] * a + dahinter[i] * (1 - a) for i in range(3))


def unguenstigst(text, flaeche, alpha):
    hell = kontrast(text, ueber(flaeche, (255, 255, 255), alpha))
    dunkel = kontrast(text, ueber(flaeche, (0, 0, 0), alpha))
    return min(hell, dunkel)


def themefarben(name):
    """(ForegroundNormal, ForegroundInactive) der Theme-eigenen colors-Datei."""
    datei = BASIS / name / "colors"
    if not datei.exists():
        return None, None
    cfg = configparser.ConfigParser(strict=False)
    cfg.read(datei, encoding="utf-8")
    if not cfg.has_section("Colors:Window"):
        return None, None
    grp = cfg["Colors:Window"]

    def lies(schluessel):
        wert = grp.get(schluessel)
        if not wert:
            return None
        return tuple(int(t.strip()) for t in wert.split(",")[:3])

    return lies("ForegroundNormal"), lies("ForegroundInactive")


def zeile(werte):
    return " | ".join(werte)


print("Kontrast beider Textklassen je Desktop-Theme")
print("Farbschema fest: CachyOSNordLightly (Kunde) — WindowText %s, ForegroundInactive %s"
      % (SCHEMA_WINDOWTEXT, SCHEMA_FOREGROUNDINACTIVE))
print()
print("A) Notiztext (WindowText / im Code QPalette::Text von m_text)")
print()
kopf = ("%-22s %6s %8s | %9s %8s | %9s %8s" %
        ("Theme", "Alpha", "colors?", "heute", "ungst.", "nach AK1", "ungst."))
print(kopf)
print("-" * len(kopf))
for name, (r, g, b, a) in GEZEICHNET.items():
    flaeche = (r, g, b)
    fg, _ = themefarben(name)
    heute = kontrast(SCHEMA_WINDOWTEXT, flaeche)
    heute_u = unguenstigst(SCHEMA_WINDOWTEXT, flaeche, a)
    quelle = fg if fg else SCHEMA_WINDOWTEXT
    neu = kontrast(quelle, flaeche)
    neu_u = unguenstigst(quelle, flaeche, a)
    print("%-22s %6d %8s | %8.2f: %7.2f: | %8.2f: %7.2f:" %
          (name, a, "ja" if fg else "nein", heute, heute_u, neu, neu_u))

print()
print("B) Kleintext (ForegroundInactive / im Code QPalette::PlaceholderText,")
print("   App-Name, Fusszeile und der Platzhaltertext des Eingabefeldes)")
print()
print(kopf.replace("nach AK1", "Themequelle"))
print("-" * len(kopf))
for name, (r, g, b, a) in GEZEICHNET.items():
    flaeche = (r, g, b)
    _, inact = themefarben(name)
    heute = kontrast(SCHEMA_FOREGROUNDINACTIVE, flaeche)
    heute_u = unguenstigst(SCHEMA_FOREGROUNDINACTIVE, flaeche, a)
    quelle = inact if inact else SCHEMA_FOREGROUNDINACTIVE
    neu = kontrast(quelle, flaeche)
    neu_u = unguenstigst(quelle, flaeche, a)
    print("%-22s %6d %8s | %8.2f: %7.2f: | %8.2f: %7.2f:" %
          (name, a, "ja" if inact else "nein", heute, heute_u, neu, neu_u))

print()
print("C) Die Farbsaetze der colors-Dateien — unterscheiden sie sich?")
print()
saetze = ["Colors:Window", "Colors:View", "Colors:Button", "Colors:Complementary",
          "Colors:Header", "Colors:Tooltip", "Colors:Selection"]
for name in GEZEICHNET:
    datei = BASIS / name / "colors"
    if not datei.exists():
        continue
    cfg = configparser.ConfigParser(strict=False)
    cfg.read(datei, encoding="utf-8")
    werte = {}
    for satz in saetze:
        if cfg.has_section(satz):
            wert = cfg[satz].get("ForegroundNormal")
            if wert:
                werte[satz] = wert.strip()
    verschieden = len(set(werte.values()))
    print("%-22s ForegroundNormal in %d Saetzen, %d verschiedene Werte"
          % (name, len(werte), verschieden))
    for satz, wert in werte.items():
        print("    %-22s %s" % (satz, wert))
    print()
