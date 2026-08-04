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

Damit verengte sich die Frage: Entweder hat dieser Lauf tatsächlich
geklungen — dann gibt es einen zweiten Auslöser, den der Code nicht zeigt —,
oder die Messung hat dem Prozess etwas zugeschrieben, das kein Klang aus
unserem Code war. Auffällig: Um 10:04:25 lief gerade der **Löschweg**
(`KMessageWidget` vom Typ `Warning`, `src/ui/librarywindow.cpp:231`) — eine
Bauart, die der Erstbericht nie am Quelltext geprüft hat. Beide Stränge sind
unten geschlossen: der Löschweg per Quelltext (dieser Abschnitt), der ganze
Läufer per Messung (Abschnitt 2).

### Nochmals am Code geprüft (ohne Ton)

- `tests/libraryshots.cpp` vollständig gelesen: kein `KMessageDialog`, kein
  `QMessageBox`, kein `exec()`, kein `beep()`.
- Die einzigen `KNotification`-Aufrufe des Projekts liegen in
  `src/shell/globalshortcuts.cpp:109,124` (Kürzel-Konflikte). Sie werden
  ausschließlich aus dem Daemon-`main()` (`src/main.cpp:83`) erreicht; die
  Bibliothek `denkzettelui`, gegen die `libraryshots` linkt, ruft sie nicht.
- **`KMessageWidget` — die vom Erstbericht nie geprüfte Bauart des Löschwegs —
  löst keine Benachrichtigung aus.** Quelle kwidgetsaddons v6.28.0
  (`pacman -Q`-geprüft), `src/kmessagewidget.cpp` von invent.kde.org: kein
  `beep()`, kein `KMessageBox::notifyInterface`, kein `sendNotification`,
  kein `KNotification`. `animatedShow()` ist eine reine Höhenanimation
  (QTimeLine, 500 ms), `event()` behandelt nur Polish/Show/LayoutRequest für
  das Layout. Anders als `KMessageDialog` überschreibt die Klasse nicht
  einmal `showEvent()` (Symbolliste `nm -D /usr/lib/libKF6WidgetsAddons.so.6`:
  nur `paintEvent`, `resizeEvent`, `event`).
- `libraryshots` setzt `QCoreApplication::setApplicationName("denkzettel")`
  (`tests/libraryshots.cpp:157`) — ein Klang aus diesem Prozess über den
  bekannten Mechanismus trüge als Anwendungsname mutmaßlich „denkzettel“,
  nicht „libraryshots“. Welche Namensquelle die Messung des Kunden anzeigte
  (Programmname des Prozesses oder gesetzter Anwendungsname), ist ohne den
  rohen Mitschnitt nicht entscheidbar.

---

## 2. Der Messlauf: `libraryshots` klingt nicht — belegt

Vom Kunden freigegeben (er saß im Raum und war vorab über den erwarteten
Klang unterrichtet); ausgeführt 12:52–12:53 Uhr.

### Aufbau

Nachbau des historischen Standes ohne Stille (Begründung und Nachweise in
Abschnitt 4), Umgebung wie beim V1-Lauf um 10:04 (`QT_QPA_PLATFORM=offscreen
QT_QPA_PLATFORMTHEME=kde`). Mitgeschnitten mit `pactl subscribe`, jede Zeile
mit Zeitstempel auf die Millisekunde; lautlose Positivkontrolle
(`paplay --volume=0 …/dialog-warning.oga`) **vor, zwischen und nach** den
Läufen. Vollständiger Mitschnitt:
`2026-08-04-testklaenge-nachtrag/pactl-nachtrag.log`.

### Beobachtung

| Zeit | Ereignis | Audio-Streams |
|---|---|---|
| 12:52:56,7 | Kontrolle 1 (`paplay`) | 1 — Abonnent fängt |
| **12:52:57,3–12:53:04,4** | **Lauf A: `libraryshots`**, 13 Bilder geschrieben, rc=0 | **0** |
| 12:53:04,4 | Kontrolle 2 | 1 |
| 12:53:05,0–12:53:07,0 | Lauf B: `editshots`, 5 Bilder, rc=0 | **1** um 12:53:05,814 |
| 12:53:07,0 | Kontrolle 3 | 1 |

Zur Zuordnung auf die Zehntelsekunde, mit derselben Technik wie in
Abschnitt 1:

- In Lauf A entstand `03-meldungszustand.png` (der Löschweg, dieselbe Stelle,
  die um 10:04:25,50 lief) um 12:52:58,90 — der Mitschnitt zeigt dort
  **nichts**. Über den gesamten Lauf A erschien kein Stream und **nicht einmal
  eine PipeWire-Client-Registrierung**: `libraryshots` tritt gegenüber
  PipeWire überhaupt nicht in Erscheinung.
- Der eine Stream in Lauf B liegt mitten im Lauf, exakt beim Wächterdialog
  (der Dialog als Bildbeleg:
  `2026-08-04-testklaenge-nachtrag/03-waechterdialog.png`). Er war der einzige
  hörbare Klang der Messung.

### Schlussfolgerung

1. **`libraryshots` klingt nicht von sich aus.** Nicht vermutet, gemessen —
   und die Null ist beweiskräftig, weil im selben Aufbau, am selben Bau, mit
   demselben Mitschnitt der `editshots`-Dialog seinen Stream erzeugt hat
   (Attributionskontrolle) und alle drei `paplay`-Kontrollen anschlugen.
2. **Ein zweiter Auslöser existiert nach allem Geprüften nicht:** Wächterdialog
   (Erstbericht), `KMessageWidget`-Löschweg (Quelltext, Abschnitt 1) und der
   ganze Läufer empirisch (Lauf A) sind klanglos.
3. **Die Zuschreibung des Kundenmitschnitts um 10:04:25 ist damit nicht
   reproduzierbar.** `libraryshots` lief zu dieser Sekunde (Abschnitt 1), aber
   der identische Code erzeugt keinerlei PipeWire-Ereignis — auch keinen
   Client-Eintrag, an dem ein Monitor den Namen hätte ablesen können. Was der
   Mitschnitt um 10:04:25 wirklich zeigte, ließe sich nur an seinen
   Rohdaten klären (liegen dem Team nicht vor); denkbar bleibt ein zeitlich
   benachbarter Stream einer anderen Quelle, der beim Notieren dem gerade
   laufenden Läufer zugeordnet wurde — etwa ein nicht mehr belegbarer
   `librarytest`-Direktlauf eines parallelen Strangs (Abschnitt 3). **Das ist
   Möglichkeit, kein Beleg.**

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
Projektbaums. Neu sind allein dieser Bericht und der Belegordner
`2026-08-04-testklaenge-nachtrag/` (Messmitschnitt und Dialogbild — flüchtige
Belege sofort gesichert, B7).

---

## Nachtrag des PO (04.08.2026, nach Abnahme des Berichts)

**Der Rest aus Abschnitt 3 lässt sich enger fassen, als der Bericht es tut.**
Der Bericht nennt es als *Möglichkeit*, dass ein Klang aus `libraryshots`
mutmaßlich gar nicht den Namen „libraryshots" getragen hätte. Am Quelltext
geprüft, ist das keine Mutmaßung mehr:

| Programm | Anwendungsname |
|---|---|
| `tests/librarytest.cpp` | **kein** `setApplicationName` — fällt auf den Programmnamen zurück |
| `tests/libraryshots.cpp` | `setApplicationName("denkzettel")` |
| `tests/editshots.cpp` | `setApplicationName("denkzettel")` |

**Die Positivkontrolle steckt im Mitschnitt des Kunden selbst.** Dort erschienen
die 98 Streams als `application.name = "librarytest"` — genau der Fall ohne
gesetzten Namen. Der Name kommt also aus dem Anwendungsnamen und fällt sonst auf
den Programmnamen zurück.

**Daraus folgt:** Ein Stream aus `libraryshots` hätte als **„denkzettel"**
erscheinen müssen, nie als „libraryshots". Die Zeile im Kundenmitschnitt kann
deshalb **nicht** aus einem Stream abgelesen sein — sie ist mit hoher
Wahrscheinlichkeit eine Zuordnung nach dem gerade laufenden Prozess, nicht nach
dem Namen eines Klangs. Zusammen mit dem gemessenen Befund, dass `libraryshots`
**nicht einmal einen PipeWire-Client anmeldet**, ist die Sache damit so weit
geklärt, wie sie ohne die Rohdaten von damals zu klären ist.

**Was offen bleibt, und zwar wirklich:** die rund sechs überzähligen Klänge in
der Zwei-Sekunden-Spitze. Sie hingen am Protokoll des 10:09-Laufs, das um 11:22
überschrieben wurde. Das ist nicht mehr zu belegen, und die Arithmetik
7 × 14 = 98 bleibt eine Rechnung, kein Beweis.
