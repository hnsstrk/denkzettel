# Denkzettel

**Ein flüchtiger Notizblock für KDE Plasma — ein Tastendruck hin, ein Tastendruck weg.**

![Das Erfassungsfenster: ein Textfeld, darunter der Hinweis „Esc verwirft · Strg+Enter speichert"](docs/bilder/erfassungsfenster.png)

Ein durchgehender Puffer statt vieler Dateien: kein Dateiname, kein
Speichern-Dialog, keine Ablage-Entscheidung. Gedanken beim Arbeiten und Testen
festhalten; was wichtig wird, wandert später ins Hauptbuch — bei mir ein
Obsidian-Vault.

`Meta+N` öffnet das Fenster, `Strg+Enter` speichert, `Esc` verwirft. Mehr
Bedienung gibt es auf diesem Weg nicht, und das ist Absicht.

## Was gebaut ist

- **Erfassen ohne Zeremonie** — globales Kürzel, sofortiger Fokus, das Fenster
  wächst mit dem Text und verschwindet nach dem Speichern
- **Bibliothek** mit Volltextsuche und einer Notizliste, die nach Tagen
  gegliedert ist wie ein Posteingang: Heute · Gestern · Diese Woche ·
  Letzte Woche · Älter
- **Suche, die verzeiht** — findet bei fehlenden Umlauten und mitten im Wort:
  „bucher" findet „Bücher", „grafieren" findet „fotografieren"
- **Bearbeiten mit Netz** — ungespeicherte Änderungen gehen nie ohne Nachfrage
  verloren
- **Fügt sich ein** — Symbole und deutsche Beschriftungen aus dem
  Systemthema; ein Wechsel des Farbschemas wird ohne Neustart übernommen
- **Läuft im Hintergrund** — Tray-Symbol, Autostart mit der Plasma-Sitzung,
  D-Bus-Schnittstelle für Skripte

![Die Bibliothek: links die nach Tagen gegliederte Notizliste, rechts der Lesebereich](docs/bilder/bibliothek.png)

**Geplant** (siehe [Issues](https://github.com/hnsstrk/denkzettel/issues)):
Sprachnotizen mit Transkription, KI-gestützte Ordnung und Vorschläge, Export
nach Obsidian und Taskwarrior, abgerundete Fensterecken aus dem Desktop-Theme.

## Bauen

Voraussetzungen: **CMake ≥ 3.20**, **Qt 6.7** (DBus, Widgets, Sql), **KF6**
(Config, DBusAddons, GlobalAccel, I18n, Notifications, StatusNotifierItem,
WidgetsAddons, WindowSystem) und **ECM**. Auf Arch/CachyOS:

```
sudo pacman -S cmake extra-cmake-modules qt6-base kconfig kdbusaddons \
    kglobalaccel ki18n knotifications kstatusnotifieritem kwidgetsaddons kwindowsystem
```

```
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build
```

Installieren nach `/usr` (die Desktop-Datei muss systemweit liegen, sonst
findet der Kürzel-Dienst die Aktion nicht):

```
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
sudo cmake --install build
```

Danach `denkzetteld` starten oder ab- und anmelden — der Autostart-Eintrag
übernimmt es künftig.

## Wo die Notizen liegen

`~/.local/share/denkzettel/denkzettel.db` — eine SQLite-Datei, nichts
verlässt den Rechner. Ein Update, das das Schema ändert, wandelt den Bestand
beim ersten Start um; die Änderungen stehen im
[Changelog](CHANGELOG.md).

## Mitwirken und Prüfen

Zwei Linter-Targets prüfen den Quellcode auf Anforderung; der normale Build
enthält keine Analyse:

```
cmake --build build --target lint-tidy    # clang-tidy: bugprone-*, performance-*, misc-const-correctness
cmake --build build --target lint-clazy   # clazy: Qt-Semantik, Stufen level0 und level1
```

Beide lesen die Compile-Datenbank des Build-Verzeichnisses und sehen nur
`src/` und `tests/`. Beide melden nur und ändern nichts — clazys
Fixit-Automatik bleibt bewusst aus. Bekannter blinder Fleck: clazy kennt nur
`tr()`-Checks und prüft unsere `i18n()`-Aufrufe nicht.

Wie in diesem Projekt gearbeitet wird — Rollen, Definition of Done,
Sprint-Protokolle, Prüfberichte samt Bildern — steht unter
[`docs/scrum/`](docs/scrum/); die bindende Spezifikation in
[`SPEC.md`](SPEC.md).

## Wie dieses Projekt schätzt

![Schätzkegel: Revisionsfaktor (Endwert ÷ Erstwert) über dem Abstand in Sprints zwischen Erstschätzung und Umsetzung; 9 Punkte, Stand Sprint 5](docs/scrum/diagramme/kegel.svg)

Denkzettel entsteht in Sprints mit geschätzten Stories. Der Kegel zeigt, wie
stark Schätzungen später revidiert wurden, aufgetragen über den Abstand
zwischen Schätzung und Umsetzung. Die gemessene Hüllkurve ist bisher
nicht-fallend, aber **nicht durchgängig weiter werdend** — die Vorbehalte
stehen unter dem Bild, samt der dünnen Belegdichte von neun Punkten. Der
Faktor misst **nicht** den Abstand zum tatsächlichen Aufwand; der wird im
Projekt nicht erhoben. Datenreihe und Generator liegen unter
[`docs/scrum/diagramme/`](docs/scrum/diagramme/), die Werte stammen aus den
Sprint-Protokollen.

## Name

Ein Denkzettel ist eine Gedächtnisstütze — und wer einen verpasst bekommt,
vergisst die Sache nicht so schnell. Der Name wurde am 31.07.2026 gegen ein
Inventar von rund 400 existierenden Notiz-Apps sowie AUR, crates.io, PyPI,
Flathub und GitHub geprüft: frei.

## Lizenz

Noch nicht festgelegt. Bis dahin gilt: alle Rechte vorbehalten — das
Repository ist öffentlich einsehbar, aber ohne Lizenz erlaubt es weder
Weitergabe noch abgeleitete Werke.
