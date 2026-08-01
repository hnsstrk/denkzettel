# Nachweis zu Issue #54 — Capture-Texte folgen dem Themewechsel

**Datum:** 01.08.2026 · **Branch:** `fix/54-theme-farben` · **Rolle:** Entwickler

Geprüft wurde der **installierte** Stand (`DESTDIR=<sandbox>/root cmake
--install build`, Präfix `/usr`), nicht das Build-Verzeichnis. Eine
systemweite Installation nach `/usr` war nicht möglich — sie braucht das
Passwort des Kunden und läuft einmal am Sprint-Ende.

## Wie gemessen wurde

Verschachtelte KWin-Sitzung mit **eigenem D-Bus** und **eigenem
`XDG_CONFIG_HOME`** (`sitzung.sh` → `nachweis.sh`); die Plasma-Sitzung des
Kunden wurde nicht angefasst, sein `~/.config/kdeglobals` blieb unverändert
(Stand 15:01, alle Läufe ab 23:07). Innerhalb der Sandbox wechselt
`plasma-apply-colorscheme` das Schema auf dem üblichen Weg, der Dienst hört
darüber wie in jeder KDE-Sitzung.

Je Lauf: Schema setzen → Dienst **einmal** starten → `ShowCapture` über D-Bus
→ Bild → Schema wechseln → Bild → Schema zurück → Bild. **Kein Neustart
zwischen den Bildern**; die Fenster-ID ist in allen drei Bildern dieselbe, und
`pgrep -a denkzetteld` zeigt genau einen Prozess der Sandbox.

Farben stammen aus den Bildern (`messen.py`): Hintergrund = häufigste Farbe
des Streifens, Textfarbe = der Bildpunkt mit dem größten Abstand davon (Kern
der Glyphe). Durch die Kantenglättung fallen die Werte etwas milder aus als
die direkt aus der Palette gelesenen der UX-Untersuchung.

## Ergebnis

| Lauf | Zustand | Kopf „Denkzettel" | Fußzeile „Esc verwirft …" |
|---|---|---|---|
| ungeheilt | Start unter BreezeDark | `#95a6a6` / 6,23:1 | `#9ca2a4` / 6,10:1 |
| ungeheilt | **nach Wechsel auf BreezeLight** | `#a8abb6` / **2,01:1** | `#a4adb7` / **1,99:1** |
| ungeheilt | zurück auf BreezeDark | `#95a6a6` / 6,23:1 | `#9ca2a4` / 6,10:1 |
| geheilt | Start unter BreezeDark | `#95a6a6` / 6,23:1 | `#9ca2a4` / 6,10:1 |
| geheilt | **nach Wechsel auf BreezeLight** | `#7b8092` / **3,44:1** | `#758394` / **3,39:1** |
| geheilt | zurück auf BreezeDark | `#95a6a6` / 6,23:1 | `#9ca2a4` / 6,10:1 |

Der ungeheilte Stand reproduziert den Befund der UX-Untersuchung (dort 2,09:1
aus der Palette gelesen): der Kleintext bleibt beim Wechsel auf dem dunklen
Wert stehen. Der geheilte Stand folgt in beide Richtungen; die Bilder 1 und 3
sind byte-gleich, das Fenster kehrt also exakt in den Ausgangszustand zurück.
(Byte-gleich sind sie auch im ungeheilten Lauf — die eingefrorene Farbe ist ja
die dunkle. Unter BreezeDark fällt der Fehler nicht auf, und genau deshalb
braucht der Nachweis den hellen Zwischenzustand.)

Bilder: `ungeheilt-*` (Gegenprobe), `geheilt-leer-*` (Leerzustand),
`geheilt-getippt-*` (mit Text — belegt, dass auch Textfarbe und
Feldhintergrund folgen).
