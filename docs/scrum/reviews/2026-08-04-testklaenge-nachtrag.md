# Nachtrag zur Klangdiagnose: das 10:04:25-Rätsel und die Zahl 98

Datum: 04.08.2026 · **Nachtrag zu `2026-08-04-testklaenge.md`** — zweiter
Diagnoseauftrag des PO nach Kundenentscheidung vom 04.08.2026. Der Erstbericht
ließ zwei Punkte ausdrücklich offen: das dem Läufer `libraryshots` um 10:04:25
zugeschriebene Klangmuster und die im Messfenster 10:00–10:10 gezählten
98 Klänge. Keine Änderung am Projekt; dieser Bericht ist die einzige neue Datei.

Beobachtung und Schlussfolgerung sind in jedem Abschnitt getrennt.

---

## 1. `libraryshots` lief wirklich um 10:04:25 — keine Verwechslung

### Beobachtung

Der V1-Strang (Bericht `2026-08-04-v1-bildlaeufer.md`, Abschnitt 2.3) hat am
Vormittag alle vier Bildläufer von Hand ausgeführt — am Stand `9d222d2`
(committet 09:13 Uhr), also **vor** dem Einbau der Stille (`d30f5d0`,
11:07 Uhr). Die dabei geschriebenen Bilder liegen noch im Arbeitsordner des
Laufs; ihre Änderungszeiten (`find -printf '%T+'`) ergeben die Reihenfolge:

| Läufer | erstes Bild | letztes Bild |
|---|---|---|
| `searchshots` | 10:04:21,4 | 10:04:23,9 |
| **`libraryshots`** | **10:04:24,5** | **10:04:31,0** |
| `editshots` | 10:04:31,3 | 10:04:32,9 |
| `readmeshots` | 10:04:33,3 | 10:04:33,7 |

Das Bild `03-meldungszustand.png` — der Zustand nach der Entf-Taste, mit dem
Meldungsband der schwebenden Löschung — schrieb `libraryshots` um
**10:04:25,50**.

### Schlussfolgerung

Die erste der beiden Erklärungen des Erstberichts — **eine Verwechslung der
Läufer in der Messung — ist widerlegt.** Um 10:04:25 lief exakt
`libraryshots`; `editshots` (der einzige Bildläufer mit Dialog) lief sechs
Sekunden später. Die Zuschreibung des Kunden passt sekundengenau auf einen
echten `libraryshots`-Prozess, der ohne die heutige Stille lief.

Damit verengt sich die Frage: Entweder hat dieser Lauf tatsächlich geklungen —
dann gibt es einen zweiten Auslöser, den der Code nicht zeigt —, oder die
Messung hat dem Prozess etwas zugeschrieben, das kein Klang war. Auffällig,
aber nur eine Koinzidenz, solange sie nicht gemessen ist: Um 10:04:25 lief
gerade der **Löschweg** (`KMessageWidget` vom Typ `Warning`,
`src/ui/librarywindow.cpp:231`), dem der Erstbericht Klanglosigkeit
bescheinigt.

### Nochmals am Code geprüft (ohne Ton)

- `tests/libraryshots.cpp` vollständig gelesen: kein `KMessageDialog`, kein
  `QMessageBox`, kein `exec()`, kein `beep()`.
- Die einzigen `KNotification`-Aufrufe des Projekts liegen in
  `src/shell/globalshortcuts.cpp:109,124` (Kürzel-Konflikte). Sie werden
  ausschließlich aus dem Daemon-`main()` (`src/main.cpp:83`) erreicht; die
  Bibliothek `denkzettelui`, gegen die `libraryshots` linkt, ruft sie nicht.
- `libraryshots` setzt `QCoreApplication::setApplicationName("denkzettel")`
  (`tests/libraryshots.cpp:157`) — welche Namensquelle die Messung des Kunden
  anzeigt (Programmname des Prozesses oder gesetzter Anwendungsname), ist
  aus dem Code nicht entscheidbar und blieb als Unschärfe stehen.

---

## 2. Der Messlauf

*(Abschnitt wird nach der Freigabe des PO gefüllt — der Lauf ist möglicherweise
hörbar und wurde vorab angekündigt.)*

---

## 3. Die Zahl 98: drei Läufe belegt, vier nicht mehr belegbar

### Beobachtung

Ein voller `librarytest`-Lauf zeigt 14 Wächterdialoge (Erstbericht,
Abschnitt 2, aus dem Code gezählt). Im Messfenster 10:00–10:10 sind heute noch
**drei** Suitenläufe belegbar:

| Lauf | Beleg | `librarytest` |
|---|---|---|
| Arbeitsordner `bn` | `LastTest.log`, mtime 10:01:00 | Start 10:00 |
| Arbeitsordner `bend` | `LastTest.log`, mtime 10:08:52 | Start 10:08, Dauer 5,75 s |
| Haupt-`build/` | nur noch über den Erstbericht („belegt einen Suitenlauf um 10:09“) — das Log wurde um 11:22 vom nächsten Lauf überschrieben | um 10:09 |

Macht 3 × 14 = **42** belegte Dialoganzeigen, dazu **1** von `editshots`
(10:04:31) — zusammen 43 von 98.

**Warum der Rest nicht mehr belegbar ist:** Die Messskripte des V1-Strangs
(`messung.sh`, `wdh.sh`, `wdh4.sh`) beginnen jeden Durchgang mit `rm -rf` auf
ihren Bauordner. Von den mindestens neun Bauordnern des Vormittags (`bv`,
`bv2`, `bn`, `bci`, `bspike`, `bj4v`, `bj4n`, `bend`, Hauptbaum) tragen nur
noch zwei ein `LastTest.log`; jeder weitere `ctest`-Lauf in einem später
neu aufgesetzten Ordner hat sein Log mitgenommen. Direkte Aufrufe
(`./bin/librarytest` ohne `ctest`) hinterlassen ohnehin keins.

Zur dichten Stelle des Kundenmitschnitts (15 Klänge 10:08:50–10:08:51): Der
`bend`-Lauf endete 10:08:52; `librarytest` lief darin bis etwa 10:08:50,
und seine 3×3-Matrix (9 Dialoge in schneller Folge) liegt am Testende — sie
fällt also genau in diese Sekunden. Die übrigen ~6 Klänge derselben zwei
Sekunden verlangen eine zweite, überlappende Quelle; der Haupt-`build`-Lauf
„um 10:09“ (Minutengranularität) kann sie gewesen sein, beweisbar ist es
nach dem Überschreiben des Logs nicht mehr.

### Schlussfolgerung

**98 = 7 × 14 bleibt Arithmetik.** Drei der sieben Läufe sind belegt, vier
nicht mehr — ihre Spuren wurden von den `rm -rf`-Schleifen der
Vormittagsmessungen zerstört, bevor dieser Auftrag begann. Die Zahl der Läufe
im Fenster ist damit **nicht rekonstruierbar**; die 98 sind mit den belegten
Läufen vereinbar, aber nicht durch sie bewiesen.

---

## 4. Nebenbefund zur Methodik des Übergehens der Stille

Die Vorgabe, die Stille „je Lauf über die Umgebung“ zu übergehen, trägt nicht:
`tests/testsilence.cpp:36` ruft `qputenv("CANBERRA_DRIVER", "null")` vor
`main()` — **`qputenv` überschreibt eine außen gesetzte Variable**, nicht
umgekehrt. Ein Messlauf am heutigen Stand ist daher nicht per Umgebung
lauter zu machen. Stattdessen wurde für Abschnitt 2 der historische Stand
`9d222d2` per `git archive` in einen Arbeitsordner exportiert und dort gebaut
(Quellen von `libraryshots` und `editshots` seit `9d222d2` unverändert, per
`git log` geprüft; das Binary enthält nachweislich kein Stille-Symbol,
per `nm` geprüft). Das Repository blieb unberührt.

---

## 5. Nichts verändert

Produktivcode, Testcode, Buildsystem, `tests/testsilence.cpp`,
Systemeinstellungen: unberührt. Kein Commit, kein Push. Alle Bau- und
Messläufe dieses Auftrags liefen in einem Arbeitsordner außerhalb des
Projektbaums.
