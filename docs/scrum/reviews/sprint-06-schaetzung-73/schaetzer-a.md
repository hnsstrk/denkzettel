# Schätzung A zu #73 — AppStream-Metainfo, `appstreamtest` scharf machen

Stand: 2026-08-04 · Schätzer A (`denkzettel-dev`), unabhängig erstellt.
Kein Produktivcode geändert; einzige Änderung dieses Auftrags ist diese Datei.

## Zahl

**3 Story Points.**

## Der wichtigste Befund zuerst — die Bedingung, ohne die AK 4 nicht erfüllbar ist

Die Frage der Story ist nicht, wie lange man eine XML-Datei schreibt. Sie
lautet: *wann wird dieser Test überhaupt scharf?* Ich habe das gemessen statt
gelesen.

`appstreamtest` liest `${CMAKE_BINARY_DIR}/install_manifest.txt` und validiert
die dort genannten Dateien **an ihrem Installationsort**. Das Manifest schreibt
`cmake --install` — auch der Lauf mit `DESTDIR`. Genau das tut `installtest`
(`tests/installtest.cmake`) bei jedem `ctest`: Er installiert in
`build/tests/installtest-root`, und CMake schreibt dabei
`build/install_manifest.txt`. Das Manifest führt die Pfade jedoch **ohne
DESTDIR-Präfix**:

```
/usr/bin/denkzetteld
/usr/share/applications/org.denkzettel.Denkzettel.desktop
/etc/xdg/autostart/org.denkzettel.Denkzettel.desktop
/usr/share/icons/hicolor/scalable/apps/denkzettel.svg
/usr/share/icons/hicolor/scalable/status/denkzettel-tray.svg
```

Die Dateien liegen aber unter dem Staging-Verzeichnis. Der Test sucht sie also
in `/usr` — und findet sie nur, wenn dort **wirklich** installiert wurde.

### Vier Messungen am ECM-Skript

Ich habe das ECM-Skript viermal direkt aufgerufen, mit selbstgebauten
Manifesten und einer Kandidaten-Metainfo (Ablage: Sitzungs-Scratchpad, deshalb
hier nur die Ergebnisse):

| Fall | Manifest zeigt auf | Ausgang |
|---|---|---|
| A | vorhandene, gültige Metainfo | `EXIT=0`, „Validierung war erfolgreich" |
| B | vorhandene, beschädigte Metainfo | `EXIT=1`, `FATAL_ERROR` — **rot** |
| C | Pfad in `/usr`, Datei existiert dort nicht | `EXIT=0`, nur `CMake Warning: Could not find …` — **grün** |
| D | Manifest gibt es nicht | `EXIT=0`, „Not installed yet, skipping" — **grün** |

Fall **C** ist die Fehlerklasse, die die Story sonst übersieht: Das Skript
meldet die fehlende Datei als *Warnung*, nicht als Fehler, und endet grün. Wer
die Metainfo einbaut, aber nicht nach `/usr` installiert, hat den Test nicht
scharf gemacht — er hat ihn nur von „still leer" auf „still warnend"
umgestellt. Beides sieht in `ctest` gleich aus.

### Daraus folgen zwei harte Bedingungen

**1. Im automatischen Testlauf wird der Test durch diese Story nicht scharf —
unter keiner Umsetzung, die nur AK 1–3 erfüllt.** In einem frischen
Arbeitsbaum gibt es beim ersten `ctest` noch kein `install_manifest.txt`, und
`appstreamtest` ist Test **#1**, `installtest` Test **#7** — die Reihenfolge
liegt fest, das Manifest entsteht erst nach dem Test, der es bräuchte (Fall D).
Selbst wenn es existierte, griffe Fall C, weil der Container kein `/usr` dieses
Projekts hat. AK 5 („automatischer Lauf bleibt grün") ist damit erfüllt — aber
trivial, aus demselben Grund wie heute.

**2. Die Mutationsprobe (AK 4) verlangt zwei Installationen nach `/usr`.**
Beschädigt wird die Datei im Repo; scharf ist aber die Kopie in `/usr`. Der
Ablauf lautet: beschädigen → `pkexec cmake --install` → `ctest` (rot, Beleg) →
heilen → `pkexec cmake --install` → `ctest` (grün, Beleg). Das kollidiert mit
der Regel aus `CLAUDE.md`: Es gibt nur ein `/usr`, und der PO taktet die
Installation. Zwei getaktete Root-Installationen mitten im Sprint sind der
teuerste Einzelposten dieser Story — nicht die XML-Datei.

### Der Ausweg, den ich empfehle (und der eine PO-Entscheidung braucht)

Das Repo hat das Muster bereits: `tests/installtest.cmake` ist ein
projekteigener CMake-Test, der über `DESTDIR` in den Bauordner installiert und
das Ergebnis prüft — **ohne Root, ohne `/usr`, überall scharf, auch im
Container**. Ein Zwilling davon, der `appstreamcli validate --no-net` auf die
Metainfo im Arbeitsbaum bzw. im Staging-Verzeichnis wirft, macht die Prüfung
an jedem Ort scharf, an dem sie gebraucht wird, und die Mutationsprobe wird
zu einem gewöhnlichen `ctest`-Lauf ohne Passworteingabe.

Das ECM-`appstreamtest` bleibt daneben stehen, wie es ist. Es zu reparieren
wäre Heilen statt Melden — es ist fremder Code.

**Diese Ergänzung steht so nicht in den AK.** Ohne sie erfüllt die Story ihr
eigenes AK 4 nur auf der Entwicklungsmaschine, nach zwei Root-Installationen,
und der automatische Lauf prüft die Datei weiterhin nie.

## Was die Zahl trägt — Anteile

| Anteil | Aufwand | Bemerkung |
|---|---|---|
| Metainfo-Datei verfassen | ~15 % | gemessen billig, siehe unten |
| Installationsregel + `KDE_INSTALL_METAINFODIR` | ~5 % | eine Zeile; die Variable existiert (`KDEInstallDirsCommon.cmake:353`) |
| Projekteigener Validierungstest (Zwilling zu `installtest`) | ~30 % | inkl. Verdrahtung in `tests/CMakeLists.txt` |
| Mutationsprobe durchführen und versioniert belegen | ~25 % | zwei Läufe, Belege nach `docs/scrum/reviews/` |
| `releases`-Block aus `CHANGELOG.md` + Fortschreibungsregel | ~15 % | `PROZESS.md`, Sprint-Abschluss; DoD 4 zieht die SPEC nach |
| Installierter Stand prüfen (DoD 2), Taktung abstimmen | ~10 % | |

### Die XML selbst ist der billigste Teil — gemessen

Ich habe einen vollständigen Kandidaten aus `.desktop`, `README.md`, `SPEC.md`
Abschnitt 1 und `CHANGELOG.md` geschrieben (`id`, `name`, `summary`,
`description`, `launchable`, `project_license` MIT, `metadata_license`,
`developer`, `content_rating`, `url`, `releases` mit `0.1.0`/2026-08-02) und
validiert:

```
✔ Validierung war erfolgreich: Pedantisch: 1
EXIT=0
```

**Beim ersten Versuch sauber.** Der eine pedantische Hinweis ist
`cid-contains-uppercase-letter` — der Großbuchstabe in
`org.denkzettel.Denkzettel`. Er ist unvermeidlich, weil die Id zum bestehenden
`.desktop`-Dateinamen passen muss, und er lässt den Lauf grün.

Drei Mutationen zeigen, dass die Prüfung wirklich greift und wie leicht sie
rot wird:

| Mutation | Ausgang |
|---|---|
| `</component>` entfernt | `E: xml-markup-invalid`, `EXIT=3` |
| `<summary>` entfernt | `E: component-summary-missing`, `EXIT=3` |
| Datum als `02.08.2026` | `W: invalid-iso8601-date`, `EXIT=3` |

Wichtig: **auch eine bloße Warnung färbt rot** (`EXIT=3`). Die Mutationsprobe
ist damit billig und eindeutig — sobald der Test überhaupt scharf ist.

`appstreamcli` ist auf der Entwicklungsmaschine vorhanden (Version 1.1.5) und
im automatischen Lauf ebenfalls (Paket `appstream` in der Abhängigkeitszeile
von `.github/workflows/ci.yml`). Von dieser Seite droht nichts.

## Was ich nicht einschätzen kann

1. **Ob der PO den projekteigenen Zwilling will.** Das ist der Punkt, an dem
   die Zahl kippt (siehe Risiken). Ich kann die Bedingung benennen, nicht die
   Entscheidung vorwegnehmen.
2. **Wie teuer die Taktung der Root-Installation im laufenden Sprint wird.**
   Sie hängt an Fremdterminen (Passworteingabe des Kunden) und daran, wie viele
   Agenten gleichzeitig auf `/usr` warten. Das ist keine Codegröße; es ist
   Wartezeit, die ich nicht messen kann.
3. **Ob `<screenshots>` verlangt werden.** Die AK nennen sie nicht, das Issue
   erwähnt „ohne Bild" in der Begründung. Bilder brauchen erreichbare URLs;
   `--no-net` prüft sie nicht, ein Software-Zentrum schon. Käme das dazu, wäre
   eine Ablage für die Bilder zu klären (Repo-Rohlinks über GitHub) — plus
   etwa ein halber Punkt.

## Teilbarkeit

Ja, an einer sauberen Naht — dort, wo die Zuständigkeit wechselt:

- **Teil A (2 SP) — die Auslieferung.** AK 1–3: Datei im Repo, Installation
  nach `KDE_INSTALL_METAINFODIR`, `releases` aus dem Changelog, Fortschreibung
  in `PROZESS.md` verankert. Prüfbar ohne Root über `installtest`-Staging.
- **Teil B (2 SP) — die Schärfe.** AK 4–5: projekteigener Validierungstest,
  Mutationsprobe mit versioniertem Beleg, DoD-2-Lauf am installierten Stand.

Teil B hat ohne A keinen Gegenstand; A ohne B liefert eine Metainfo, deren
Richtigkeit niemand prüft. **Ich würde nicht teilen** — vier Punkte für das,
was zusammen drei sind, und die Naht erzeugt genau die Zwischenstufe („Datei
da, Test weiterhin blind"), die dieses Issue anprangert.

## Risiken, die die Zahl kippen

- **↑ 5 SP, wenn AK 4 wörtlich am ECM-Test hängen bleibt.** Dann steht die
  Story auf zwei getakteten Root-Installationen, und ein Fehlschlag im ersten
  Anlauf kostet einen dritten Takt.
- **↑ 5 SP, wenn Screenshots dazukommen** (Ablage, URLs, Zuschnitte).
- **↑, wenn die Fortschreibungsregel aus AK 3 zur Prozessarbeit auswächst.**
  „Changelog und Metainfo bleiben in Deckung" ist leicht gesagt und trifft
  `PROZESS.md`, die SPEC (DoD 4: entdeckte Bedingungen ziehen die SPEC nach)
  und #61, das die Versionsanzeige noch offen hat. Wer die Versionszählung
  gleich mit entscheidet, verschiebt Umfang aus #61 hierher.
- **↓ 2 SP**, falls der PO auf die Mutationsprobe als eigenen Beleg verzichtet
  und den projekteigenen Test durch einen roten Erstlauf (Test vor Datei) für
  ausreichend hält. Das wäre allerdings eine Abschwächung von AK 4.

## Empfehlung zur Ziehbarkeit

**Ziehbar — aber nicht mit dem AK-Satz 4 in der jetzigen Fassung.**

Vor dem Ziehen ist eine Sache zu entscheiden, und es ist keine
Entwicklerentscheidung: *Woran wird die Schärfe des Tests gemessen — am
geerbten ECM-Test unter `/usr`, oder an einem projekteigenen Test, der überall
läuft?* Bleibt AK 4 wie es ist, kauft der Sprint eine Story, deren Beleg an der
Passworteingabe des Kunden und der `/usr`-Taktung hängt und die im
automatischen Lauf trotzdem nichts prüft.

Mein Vorschlag für die Schärfung von AK 4 und 5:

> 4. Ein **projekteigener** Test validiert die Metainfo mit `appstreamcli`
>    unabhängig von einer Installation nach `/usr`; nach absichtlicher
>    Beschädigung der Datei wird `ctest` rot (Mutationsprobe, Beleg unter
>    `docs/scrum/reviews/`).
> 5. Der geerbte `appstreamtest` bleibt unverändert; im Sprint-Protokoll wird
>    festgehalten, dass er weiterhin nur nach einer echten Installation nach
>    `/usr` etwas prüft — und dass eine fehlende Datei dort nur eine Warnung
>    erzeugt, keinen roten Lauf.

Mit dieser Fassung ist die Story eine saubere 3, ohne offene Fremdabhängigkeit,
und sie schließt die Lücke wirklich statt sie zu verschieben.

## Belegte Fundstellen

- `/usr/share/ECM/kde-modules/appstreamtest.cmake` — Sammellogik, `EXISTS`-Fall
  als `message(WARNING)` (Zeile 21), `FATAL_ERROR` nur bei Validierungsfehler
  (Zeile 38)
- `/usr/share/ECM/kde-modules/KDECMakeSettings.cmake:172–181, 195` —
  Registrierung des Tests, `INSTALL_FILES` zeigt auf
  `${CMAKE_BINARY_DIR}/install_manifest.txt`
- `/usr/share/ECM/kde-modules/KDEInstallDirsCommon.cmake:353` —
  `KDE_INSTALL_METAINFODIR` unterhalb `DATAROOTDIR`
- `/home/hnsstrk/Projekte/denkzettel/tests/installtest.cmake` — vorhandenes
  Muster eines projekteigenen, installationsprüfenden CMake-Tests
- `/home/hnsstrk/Projekte/denkzettel/tests/CMakeLists.txt:122–129` —
  seine Verdrahtung
- `/home/hnsstrk/Projekte/denkzettel/CMakeLists.txt:37–54` — heutige
  Installationsregeln für `.desktop` und Symbole
- `/home/hnsstrk/Projekte/denkzettel/.github/workflows/ci.yml` — Paket
  `appstream` vorhanden, **kein** Installationsschritt
