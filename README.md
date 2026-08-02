# Denkzettel

Ein Notizzettel für KDE Plasma, der aufgeht, wenn man ihn braucht, und
verschwindet, wenn man fertig ist.

![Das Erfassungsfenster: ein Textfeld, darunter der Hinweis „Esc verwirft · Strg+Enter speichert"](docs/bilder/erfassungsfenster.png)

`Meta+N` drücken, tippen, `Strg+Enter`. Die Notiz ist gespeichert, das Fenster
ist weg. Kein Dateiname, kein Speichern-Dialog, keine Frage, wohin damit.

Ich habe Denkzettel gebaut, weil mir beim Arbeiten ständig Sachen einfallen,
die woanders hingehören: eine Idee, eine offene Frage, ein Kommandozeilen-Fund.
Wenn ich dafür erst eine Datei anlegen muss, ist der Gedanke weg. Was sich
lohnt, wandert später in den Obsidian-Vault.

## Was es kann

Das Erfassungsfenster ist der halbe Punkt. Die andere Hälfte ist die
Bibliothek: alle Notizen nach Tagen gruppiert, wie ein Posteingang.

![Die Bibliothek: links die nach Tagen gegliederte Notizliste, rechts der Lesebereich](docs/bilder/bibliothek.png)

Die Suche ist nachsichtig. „bucher" findet „Bücher", „grafieren" findet
„fotografieren" — Umlaute und Wortanfänge muss man nicht treffen.

Notizen lassen sich bearbeiten. Wer den Editor verlässt, ohne zu speichern,
wird gefragt.

Denkzettel läuft im Hintergrund, sitzt im Systemabschnitt der Kontrollleiste
und startet mit der Sitzung. Für Skripte gibt es eine D-Bus-Schnittstelle:

```
qdbus6 org.denkzettel.Daemon /Daemon AddNote "Text der Notiz"
```

Symbole und Beschriftungen kommen aus dem System, ein Wechsel des Farbschemas
wird sofort übernommen.

Auf der Liste stehen noch: Sprachnotizen mit Transkription, eine KI, die
sortiert und Vorschläge macht, Export nach Obsidian und Taskwarrior, runde
Fensterecken. Was gerade ansteht, steht in den
[Issues](https://github.com/hnsstrk/denkzettel/issues).

## Bauen

Gebraucht werden CMake ab 3.20, Qt 6.7 (DBus, Widgets, Sql), die KDE
Frameworks 6 (Config, DBusAddons, GlobalAccel, I18n, Notifications,
StatusNotifierItem, WidgetsAddons, WindowSystem) und ECM. Auf Arch und
CachyOS:

```
sudo pacman -S cmake extra-cmake-modules qt6-base kconfig kdbusaddons \
    kglobalaccel ki18n knotifications kstatusnotifieritem kwidgetsaddons kwindowsystem
```

```
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build
```

Zum Installieren:

```
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
sudo cmake --install build
```

Das Präfix `/usr` ist nicht kosmetisch: Der Kürzeldienst von Plasma findet die
Aktion nur, wenn die Desktop-Datei systemweit liegt. Danach `denkzetteld`
starten oder einmal neu anmelden.

## Wo die Notizen liegen

In `~/.local/share/denkzettel/denkzettel.db`, einer SQLite-Datei. Nichts
verlässt den Rechner. Ändert ein Update das Schema, wird der Bestand beim
ersten Start umgewandelt; was sich ändert, steht im [Changelog](CHANGELOG.md).

## Entwicklung

Zwei Linter laufen auf Anforderung, nicht beim normalen Bauen:

```
cmake --build build --target lint-tidy    # clang-tidy
cmake --build build --target lint-clazy   # clazy, Qt-Semantik
```

Beide sehen nur `src/` und `tests/` und melden bloß, sie ändern nichts.
Bekannte Lücke: clazy prüft `tr()`, wir benutzen aber `i18n()`.

Wie hier gearbeitet wird — Rollen, Definition of Done, Sprint-Protokolle und
Prüfberichte mit Bildern — steht unter [`docs/scrum/`](docs/scrum/). Die
Spezifikation ist [`SPEC.md`](SPEC.md).

## Wie hier geschätzt wird

![Schätzkegel: Revisionsfaktor (Endwert ÷ Erstwert) über dem Abstand in Sprints zwischen Erstschätzung und Umsetzung; 9 Punkte, Stand Sprint 5](docs/scrum/diagramme/kegel.svg)

Denkzettel entsteht in Sprints mit geschätzten Stories. Das Bild zeigt, wie
stark einzelne Schätzungen später korrigiert wurden, aufgetragen über den
Abstand zwischen Schätzung und Umsetzung. Bisher wird die Kurve nach rechts
nicht enger, aber auch nicht durchgehend weiter; bei neun Punkten ist das
wenig Beleg, und die Einschränkungen stehen unter dem Bild. Gemessen wird die
Korrektur der Schätzung, nicht der Abstand zum tatsächlichen Aufwand — den
erheben wir gar nicht. Daten und Generator liegen in
[`docs/scrum/diagramme/`](docs/scrum/diagramme/).

## Name

Ein Denkzettel ist eine Gedächtnisstütze, und wer einen verpasst bekommt,
vergisst die Sache so schnell nicht. Am 31.07.2026 gegen rund 400 bestehende
Notiz-Apps sowie AUR, crates.io, PyPI, Flathub und GitHub geprüft: frei.

## Lizenz

Steht noch aus. Bis dahin sind alle Rechte vorbehalten: Der Code ist
öffentlich lesbar, aber ohne Lizenz darf ihn niemand weitergeben oder darauf
aufbauen.
