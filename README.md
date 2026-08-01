# Denkzettel

Ein flüchtiger Scratchpad für KDE Plasma — ein Tastendruck hin, ein Tastendruck weg.

Ein durchgehender Puffer statt vieler Dateien: kein Dateiname, kein Speichern-Dialog, keine Ablage-Entscheidung. Gedanken beim Arbeiten und Testen festhalten; was wichtig wird, wandert von Hand ins Hauptbuch (den Obsidian-Vault).

**Status:** In Entwicklung — Daemon mit Tray, Capture-Fenster, globales Kürzel, Autostart und Bibliotheksfenster sind gebaut (Meilensteine M1/M2, Sprint 2 in der Kundenabnahme).

## Linter

Zwei CMake-Targets prüfen den Quellcode auf Anforderung; der normale Build enthält keine Analyse:

```
cmake --build build --target lint-tidy    # clang-tidy: bugprone-*, performance-*, misc-const-correctness
cmake --build build --target lint-clazy   # clazy: Qt-Semantik, Stufen level0 und level1
```

Beide lesen die Compile-Datenbank des Build-Verzeichnisses und sehen nur `src/` und `tests/`, keine erzeugten Dateien. `lint-tidy` prüft über `.clang-tidy` zusätzlich, dass der Rückgabewert einer Registrierung bei einem fremden Dienst nicht ignoriert wird. Beide melden nur und ändern nichts — clazys Fixit-Automatik bleibt bewusst aus. Bekannter blinder Fleck: clazy kennt nur `tr()`-Checks und prüft unsere `i18n()`-Aufrufe nicht.

## Name

Ein Denkzettel ist eine Gedächtnisstütze — und wer einen verpasst bekommt, vergisst die Sache nicht so schnell. Der Name wurde am 31.07.2026 gegen ein Inventar von rund 400 existierenden Notiz-Apps sowie AUR, crates.io, PyPI, Flathub und GitHub geprüft: frei.
