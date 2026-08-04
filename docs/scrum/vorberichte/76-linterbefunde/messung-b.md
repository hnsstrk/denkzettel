# Vorprüfung #76 — Messung B (Scrum Master)

Gemessen am 2026-08-04, 20:30–20:38 CEST, Stand `6acc87e`. Unabhängig von
Bearbeiter A; dessen Messung war zum Zeitpunkt dieser Arbeit nicht gelesen.

Belege: `messung-b-tidy.txt`, `messung-b-clazy.txt`,
`messung-b-auswertung.txt` (alle in diesem Ordner). Der Lauf ging über eine
eigene Kopie der gefilterten Compile-Datenbank im Scratchpad, nicht über
`build/lint/` — fünf Agenten arbeiten heute parallel im selben `build`.

## Feld 1 — Dateimenge (so weit ohne A gemessen)

**20 Dateien tragen Befunde**, gemessen, nicht geschätzt:

| Datei | Befunde |
|---|---|
| `tests/librarytest.cpp` | 42 (+2 clazy) |
| `src/store/note.h` | 36 |
| `src/ui/librarywindow.h` | 14 |
| `src/ui/timestampformat.h` | 9 |
| `tests/shelltest.cpp` | 7 (+1 clazy) |
| `tests/capturetest.cpp` | 5 |
| `src/ui/notelistmodel.h` | 4 |
| `src/shell/shortcutregistration.h`, `src/main.cpp`, `src/capture/capturewindow.cpp` | je 3 |
| `tests/editshots.cpp`, `src/shell/trayicon.cpp`, `src/shell/shortcutregistration.cpp`, `src/shell/globalshortcuts.cpp` | je 2 |
| `tests/searchshots.cpp`, `tests/readmeshots.cpp`, `tests/libraryshots.cpp`, `tests/captureshots.cpp`, `src/ui/notelistdelegate.cpp`, `src/ui/librarywindow.cpp` | je 1 |

Dazu: `.github/workflows/ci.yml` (AK 3), ggf. `.clang-tidy` (siehe Feld 6).

**Das ist praktisch das ganze Repository.** Von den 20 Dateien liegen 6 in
`src/`, 4 sind Header mit Kern-Datentypen, 5 sind Bildläufer.

## Feld 2 — Fallen (Teilmessung)

- **Der Ausgangsstand im Issue ist überholt.** Das Issue nennt (gemessen an
  `009c471`) **72 eindeutige Befunde aus 130 Rohzeilen**. Mein Lauf an `6acc87e`
  liefert **81 eindeutige Befunde aus 140 Rohzeilen** — und eine Prüfung, die
  das Issue gar nicht führt: **`bugprone-narrowing-conversions`, 3 Befunde**.
  Wer die Tabelle des Issues abarbeitet, ist am Ende nicht bei null.
- **clazy: 3 Befunde, unverändert**, aber die Zeilennummern sind gewandert:
  `tests/librarytest.cpp:2387` und `:2393` (Issue: `:2336`/`:2342`),
  `tests/shelltest.cpp:361` (unverändert). Wer nach Zeilennummer sucht, sucht
  falsch.
- **`clazy-standalone` gibt `rc=0` auch mit Befunden** — von mir nachgemessen
  (`rc=0` bei drei Warnungen). Der Rückgabewert taugt für AK 2 nicht als
  Nachweis; es zählt die Zahl der `warning:`-Zeilen. In `ci.yml` ist das bereits
  so gelöst und im Kommentar begründet.
- **`performance-enum-size` (66) und `misc-const-correctness` (60) machen 126
  der 140 Rohzeilen aus.** Beide sind mechanisch — aber `performance-enum-size`
  trifft mit 36 Befunden `src/store/note.h`, den Kern-Datentyp des Projekts.
  Das ist keine Formatierung, das sind Basistypen von `enum`s in einer Datei,
  die jede andere einbindet.

## Feld 3 — AK-Urteil: **ready = ja**

Alle fünf Kriterien sind einzeln prüfbar, und jedes hat ein Prüfmittel, das
existiert: `cmake --build build --target lint-tidy` und `lint-clazy` sind in
`CMakeLists.txt:120–150` definiert und laufen (heute belegt),
`.github/workflows/ci.yml` liegt im Repo, `ctest` läuft.

Die drei DoR-Zusätze vom 04.08.2026 greifen nicht: B21 ist nicht einschlägig
(keine Aussage über Hülle, Rundung, Schatten); alle genannten Prüfmittel zeigt
`git ls-files`.

**Zum Abschnitt „Ein Befund, der eine Entscheidung braucht":** Er sieht wie ein
selbstdeklarierter offener Punkt aus, ist aber keiner im Sinne des
DoR-Zusatzes. Der Unterschied zu #83 ist der Zeitpunkt: Dort stand „vor dem
Ziehen zu entscheiden", hier legt der Entwickler **innerhalb** der Story vor und
der PO entscheidet — und **AK 5 fängt beide Wege auf** („Wo ein Befund bewusst
stehenbleibt, steht ein `NOLINT` mit Begründung"). Die Story ist damit
vollständig, auch wenn die Entscheidung noch aussteht.

**Ich vermerke ausdrücklich, was hier gelungen ist:** AK 4 verlangt den Nachweis
je Änderung in `src/`, nicht nur einen grünen `ctest`. Das ist genau die
Bauart, an der dieses Projekt seine vier wertlosen Tests gefunden hat. Von den
fünf Kandidaten dieses Laufs ist #76 das einzige Issue, das die DoR ohne
Nacharbeit besteht.

## Feld 5 — Größenklasse: **`size:m`** — an der oberen Kante

**Nicht `size:s`:** 20 Dateien sind nicht „wenige Dateien". Und AK 3 nimmt
`lint-tidy` **neu** in `ci.yml` auf — das ist ein neuer Prüfweg im Wortsinn der
Klassendefinition, dazu eine Schwellenänderung an einer laufenden Wache.

**Nicht `size:l`:** Die Arbeit ist mechanisch und ohne Entwurfsanteil; es gibt
keine offene Gestaltungsfrage, keine Bildprüfung, keinen Compositor, keine
Kundenentscheidung im Weg. Der Aufwand steckt in der Zahl und in AK 4, nicht in
der Schwierigkeit.

**Zur Ehrlichkeit der Einstufung:** `m` heißt hier „trägt einen Strang aus", und
das trifft es — aber der Strang berührt jede andere Fläche des Repositories
(siehe Kollisionen). Die Klasse beschreibt den Aufwand, nicht die
Verträglichkeit; die steht in Feld 1.

## Kollisionen — der Kern meines Auftrags

**#76 kollidiert mit allen vier anderen Kandidaten und mit #83.** Gemessen an
den Dateien, die Befunde tragen:

| Gegen | Kollisionsfläche | Urteil |
|---|---|---|
| **#83** | `src/capture/capturewindow.cpp` (3), `tests/capturetest.cpp` (5), `tests/captureshots.cpp` (1) — die drei Kerndateien von #83 | **nicht parallel** |
| **#71 / #72** | `src/ui/librarywindow.cpp` (1), `src/ui/librarywindow.h` (14), `tests/librarytest.cpp` (42+2), `tests/libraryshots.cpp` (1) | **nicht parallel** |
| **#61** | `src/main.cpp` (3), `CMakeLists.txt` | Naht, auflösbar |
| **#73** | keine gemeinsame Datei | frei |

**Der Fall #83 ist der schwerste, und er ist mehr als eine Textkollision.** Nach
meinem eigenen konsolidierten Bericht (`83-native-huelle/bericht.md`, Feld 1)
schreibt #83 `capturewindow.cpp` in weiten Teilen **neu** — `mixed()` und
`tinted()` (`:69–94`) fallen weg, `paintEvent()` kommt neu hinzu. Wer dort
vorher `const`-Korrektheit und `enum`-Basistypen heilt, heilt Code, den #83
löscht. Das ist nicht nur Mischkonflikt, das ist verworfene Arbeit.

## Feld 6 — Offene Fragen

**An den PO:**

1. **`performance-enum-size` in `src/store/note.h` (36 Befunde) ist eine
   Entscheidung, keine Heilung.** Die Prüfung verlangt, `enum`s einen kleineren
   Basistyp zu geben. In der zentralen Datentyp-Datei des Projekts ist das eine
   Änderung an der öffentlichen Form, die jede andere Übersetzungseinheit sieht.
   Vorzulegen ist, ob geheilt oder die Prüfung abgewählt wird — dieselbe Frage
   wie beim `bugprone-unused-return-value`, nur 36-mal.
2. **Darf `.clang-tidy` geändert werden?** Das Issue lässt bei einem Befund
   ausdrücklich beide Wege offen. Eine Prüfung abzuwählen ändert die Messlatte
   selbst — dann misst der spätere Nachweis „0 Befunde" etwas anderes als heute.
   Vorschlag zur Formulierung: Abwahl nur mit Begründung im
   `.clang-tidy`-Kommentar und Nennung im Sprint-Protokoll.
3. **Der Ausgangsstand im Issue gehört nachgezogen** (72 → 81, plus
   `bugprone-narrowing-conversions`). Nicht als Ready-Blocker — AK 1 sagt „0",
   und 0 bleibt 0 —, aber die Tabelle wird sonst als Arbeitsliste gelesen und
   ist dann unvollständig. Nach B17 wird der alte Stand nicht überschrieben,
   sondern mit Datum und Commit angehängt.

**Kundenentscheidung:** keine offen. „Heilen statt einfrieren" ist am
04.08.2026 entschieden.
