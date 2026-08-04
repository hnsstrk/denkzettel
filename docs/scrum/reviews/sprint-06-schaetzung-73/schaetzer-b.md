# Unabhängige Schätzung B — Issue #73 (AppStream-Metainfo)

**Datum:** 04.08.2026 · **Rolle:** Entwickler (Schätzer B) · **Grundlage:**
`gh issue view 73`, `/usr/share/ECM/kde-modules/appstreamtest.cmake`,
`/usr/share/ECM/kde-modules/KDECMakeSettings.cmake`, `CMakeLists.txt`,
`CHANGELOG.md`, `.github/workflows/ci.yml`, `docs/scrum/PROZESS.md`
(Abschluss-Punkte 9/10, Sprint-Mechanik), eigene Messungen mit
`appstreamcli 1.1.5`.

Der Bericht von Schätzer A lag beim Schreiben im selben Ordner und wurde
auftragsgemäß **nicht** gelesen.

## Ergebnis

**3 Story Points.**

## Messungen (nicht Vermutungen)

Alle Messläufe fanden außerhalb des Arbeitsbaums statt; am Repo wurde nichts
geändert, nichts installiert.

### M1 — Der Test ist heute wirkungslos, obwohl installiert wurde

```
$ ctest --test-dir build -R appstreamtest -V
1: Test command: /usr/bin/cmake "-DAPPSTREAMCLI=/usr/bin/appstreamcli" \
   "-DINSTALL_FILES=…/build/install_manifest.txt" -P …/appstreamtest.cmake
1/1 Test #1: appstreamtest ....................   Passed    0.01 sec
```

`build/install_manifest.txt` existiert und führt fünf Zeilen (Binärdatei,
zwei `.desktop`-Kopien, zwei Icons). Keine davon endet auf `.metainfo.xml`
oder `.appdata.xml` — der Validierungsblock des ECM-Skripts wird
übersprungen. Der Befund des Issues ist damit am laufenden System bestätigt,
nicht nur aus dem Skript gelesen.

### M2 — Der Test hängt an der **Installation**, nicht am Arbeitsbaum

`KDECMakeSettings.cmake:177` setzt
`INSTALL_FILES=${CMAKE_BINARY_DIR}/install_manifest.txt`. Das Skript beginnt
mit `file(GLOB install_done "${INSTALL_FILES}")`; fehlt die Datei, gibt es
`Not installed yet, skipping` und endet grün. Geprüft wird also stets die
**installierte Kopie**, nie die Datei im Repo.

**Folge für den automatischen Testlauf:** `ci.yml` installiert `appstream`
(der Test wird registriert), führt aber **kein** `cmake --install` aus. Dort
gibt es kein `install_manifest.txt`. Der Test ist in CI heute *und nach der
Story* wirkungslos, solange kein Installationsschritt dazukommt.
Akzeptanzkriterium 5 („bleibt grün") ist damit trivial erfüllt und misst
nichts.

### M3 — Ein vollständiger Entwurf validiert auf Anhieb

Ein realistischer Entwurf (`component type="desktop-application"`, `id`,
`name`, `summary`, deutsche zweiabsätzige `description`, `project_license
MIT`, `metadata_license CC0-1.0`, `developer` mit `id`, `launchable`, zwei
`url`, `content_rating type="oars-1.1"`, `categories`, ein `screenshot` mit
Roh-URL aus dem eigenen Repo, ein `release 0.1.0` vom 02.08.2026):

```
✔ Validierung war erfolgreich: Pedantisch: 1     (Rückgabewert 0)
```

Der Inhalt ist also **nicht** der teure Teil. Deutsche Texte ohne
`xml:lang` erzeugen keinen Befund; Screenshot-URLs werden mit `--no-net`
nicht abgerufen.

### M4 — Die Mutationsprobe funktioniert, und schon Warnungen färben rot

Drei Mutationen gegen `appstreamcli validate --no-net`:

| Mutation | Befund | Rückgabewert |
|---|---|---|
| `metadata_license` entfernt | `E: metadata-license-missing` | 3 |
| schließendes `</component>` entfernt | Fehler: 1 | 3 |
| `date="2026-08-02"` → `date="nichts"` | `W: invalid-iso8601-date` | 3 |

Dasselbe durch das ECM-Skript geschickt (Manifest mit einer Zeile auf die
jeweilige Datei):

```
gute Datei     → Rückgabewert 0
Datum kaputt   → FATAL_ERROR, Rückgabewert 1
```

Zwei Schlüsse: Die Mutationsprobe ist mechanisch nachweisbar — **und schon
eine Warnung**, nicht erst ein Fehler, macht den Lauf rot. Das ist für die
Pflege wichtiger als für die Umsetzung: Ein künftig hinzugefügtes
`<release>` mit schiefem Datum kippt den gesamten `ctest`.

## Anteile des Aufwands

| Anteil | Einschätzung | Begründung |
|---|---|---|
| Die XML-Datei schreiben | klein | M3: ~40 Zeilen, validiert sofort |
| `install(FILES … ${KDE_INSTALL_METAINFODIR})` | sehr klein | eine Zeile, gleiche Bauart wie die `.desktop`-Zeilen |
| Mutationsprobe durchführen und belegen | **mittel** | siehe Unsicherheit unten — das Verfahren, nicht die Prüfung, kostet |
| Inhaltsentscheidungen einholen | klein, aber blockierend | Sprache, Screenshots, `developer id` |
| Fortschreibungsregel (AK 3) | klein, **fremde Fläche** | ändert `PROZESS.md`, nicht Code |
| CI scharf machen (empfohlen) | klein | ein `cmake --install build` vor `ctest`; der Container läuft als root |

Vergleichsanker aus dem geschlossenen Bestand: **#43** (App- und Tray-Icon,
`sp:2`) und **#6** (Autostart, `sp:2`) haben dieselbe Bauart — eine
Datei/zwei Dateien plus Installationskette plus Prüfung. #73 liegt darüber,
weil AK 4 ein eigenes Nachweisverfahren verlangt und drei Entscheidungen
offen sind. Unter **#62** (Spike, `sp:3`) und **#5** (Kürzel + D-Bus, `sp:3`)
liegt es nicht — also **3**, nicht 2.

## Größte Unsicherheit — ausdrücklich benannt

**Wie die Mutationsprobe (AK 4) durchgeführt wird, ohne den getakteten
`/usr`-Install zu stören.** Ich habe das ECM-Skript isoliert nachgestellt
(M4) und damit belegt, *dass* es rot wird. Den echten Weg über `ctest` konnte
ich nicht messen, weil er eine Installation verlangt und der Auftrag mir
jede Änderung untersagt. Drei Wege, alle mit einem Haken:

1. **Installierte Datei unter `/usr/share/metainfo/` beschädigen** — braucht
   Wurzelrechte und verfälscht kurzzeitig den Stand, an dem laut DoD 2 alle
   anderen prüfen. Bei parallel arbeitenden Agenten die schlechteste Wahl.
2. **In ein temporäres Präfix installieren** (`cmake --install build
   --prefix …`) — kein Neubau nötig, aber es **überschreibt
   `build/install_manifest.txt`** mit den temporären Pfaden. Danach muss neu
   nach `/usr` installiert werden, sonst prüft der nächste `ctest` still ein
   Verzeichnis, das niemand mehr betrachtet. Genau die Fehlerklasse, die
   dieses Issue behandelt.
3. **Zweites Build-Verzeichnis** — sauber und kollisionsfrei, kostet aber
   einen vollständigen Neubau.

Ich halte Weg 3 für richtig und habe ihn eingepreist. Wird stattdessen Weg 2
gewählt, muss die Rückstellung Teil des Belegs sein, nicht eine mündliche
Zusicherung.

## Teilbarkeit

Teilbar, aber bei 3 Punkten lohnt der Schnitt nicht. Falls doch geteilt
wird, liegt die Naht zwischen **Artefakt** und **Prozess**:

- **A (2):** Datei, Installationszeile, Mutationsprobe, Belegablage.
- **B (1):** AK 3 — Fortschreibung im Sprint-Abschluss — plus die
  CI-Schärfung. Teil B gehört ohnehin überwiegend dem Scrum Master.

## Was die Zahl kippen würde

**Auf 5:**
- Der Kunde verlangt **englische Standardtexte mit deutschen
  `xml:lang`-Varianten**. Das Projekt hat kein `po/`-Verzeichnis und keine
  Übersetzungsanbindung; die Varianten müssten von Hand doppelt gepflegt
  werden, und die Frage, warum die `.desktop`-Datei dann weiter unmarkiert
  deutsch ist, kommt gleich mit.
- Die Mutationsprobe erweist sich als nur mit Wurzelrechten führbar und
  muss in den vom PO getakteten Installationsabschnitt eingeplant werden.

**Auf 2:**
- AK 3 (Fortschreibungsregel) wird herausgeschnitten, Screenshots entfallen,
  und die Mutationsprobe darf über ein zweites Build-Verzeichnis laufen.

## Zuschnitt — was vor dem Ziehen zu klären ist

**Vier Punkte, davon zwei für den Kunden:**

1. **(Kunde) Sprache der Metainfo-Texte.** Deutsch unmarkiert — wie die
   `.desktop`-Datei — oder englischer Standard mit deutschen
   `xml:lang`-Varianten? AppStream deutet den unmarkierten Text als
   englisch; ein Software-Zentrum zeigt deutsche Texte dann auch
   englischsprachigen Nutzern. Für ein bewusst deutschsprachiges Werkzeug
   ist die unmarkierte Fassung vertretbar, aber es ist keine
   Entwicklerentscheidung.
2. **(Kunde) Screenshots.** Der Nutzen, den das Issue selbst begründet
   („ohne Beschreibung, ohne Bild"), fällt ohne Screenshots zur Hälfte aus.
   AppStream verlangt dafür **absolute URLs**; die naheliegende Quelle sind
   die Roh-URLs der Bilder unter `docs/bilder/` im öffentlichen Repo. Das
   schreibt das Repo als dauerhaften Bildhoster fest und bindet die Metainfo
   an den Zweignamen `main`. Bewusst zu entscheiden, nicht nebenbei.
3. **(PO) AK 2 ist unvollständig.** `metadata_license` fehlt in der Liste
   und ist Pflicht — M4 zeigt, dass die Validierung ohne es mit
   `metadata-license-missing` **fehlschlägt**. `categories` fehlt ebenfalls
   (nicht validierungspflichtig, aber die Einsortierung im Software-Zentrum
   hängt daran). Beide gehören in AK 2, sonst schreibt der Entwickler ein
   Kriterium mit, das der Test ohnehin erzwingt.
4. **(PO/SM) AK 3 und AK 5 treffen nicht.**
   - AK 3 verlangt, dass der Sprint-Abschluss die Datei mitführt. Das ist
     eine Änderung an `PROZESS.md`, Punkte 9/10 — **Fläche des Scrum
     Masters**, nicht des Entwicklers. Inhaltlich unkritisch: Punkt 10 ist
     bis #61 ausgesetzt, die Einträge sammeln sich unter
     `[Unveröffentlicht]`, und `releases` führt vorerst genau einen Eintrag
     (0.1.0, 02.08.2026) — passend zu `project(denkzettel VERSION 0.1.0)`.
     Die Regel muss also die Aussetzung mitschreiben, sonst steht ab #61
     eine zweite Stelle, an der die Version gepflegt werden will.
   - AK 5 („automatischer Lauf bleibt grün") ist nach M2 **trivial erfüllt
     und wertlos**: In CI wird nie installiert, der Test überspringt sich
     dort weiterhin selbst. Vorschlag: AK 5 auf *„der automatische Lauf
     installiert und validiert die Metainfo tatsächlich"* schärfen. Der
     Container läuft als root, der Zusatz ist ein Schritt mit einer Zeile
     vor `ctest`. Ohne diese Schärfung repariert die Story den blinden Test
     nur auf der Entwicklungsmaschine und nur dann, wenn dort zufällig
     installiert wurde — und genau diese Bedingung ist es, die ihn heute
     blind macht.

**Empfehlung:** ziehbar mit 3 Punkten, sobald Punkt 1 und 2 vom Kunden
entschieden sind. Punkt 3 und 4 kann der PO ohne Rückfrage schärfen.
