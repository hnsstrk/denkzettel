# Vorprüfung #70 — Messung B (Scrum Master)

Gemessen am 2026-08-05, 18:32–18:40 CEST, Stand `581dacc`. Unabhängig von
Bearbeiter A; dessen Messung war zum Zeitpunkt dieser Arbeit nicht gelesen.

Messbelege: `messungen-b/m1` … `m7` in diesem Ordner.

**Vorbemerkung zur Messgrundlage.** Der Arbeitsbaum war während dieser Messung
fremdverändert (`git status`: ` M tests/librarytest.cpp`, dazu `build-repro71/`
und `build-vor70/` — Sonden paralleler Vorprüfungen). Gemessen wurde deshalb
durchgehend gegen `git show HEAD:`, nicht gegen die Arbeitskopie. Zwei
Prüfsätze `reproIssue71PlainClick`/`reproIssue71PressMoveRelease`, die in der
Arbeitskopie stehen, gehören **nicht** zum Bestand (`m2`).

---

## Feld 1 — Dateimenge (am Code vermessen, Notation nach B13)

| | **#70** |
|---|---|
| **Quellen und Tests** | `src/ui/librarywindow.cpp` — **nur** `showNote()` `:692–799`, darin die Kopf-Hol-Bedingung `:780–793`.<br>`tests/librarytest.cpp` — die Slot-Deklarationen `:134–145` und drei neue Prüfsätze im Muster von `:1425–1480` (Tastatur) und `:1482–1556` (Klick).<br>`tests/libraryshots.cpp` — eine neue Szene nach dem Muster von Szene 8 (`:309–340`); der Helfer `walkUp(list, steps)` steht bereits bei `:115–121`. |
| **Build** | **keine Änderung.** `librarytest` und `libraryshots` sind beide in `tests/CMakeLists.txt` eingerichtet, keine neue Abhängigkeit, keine neue Zieldatei. |
| **Belege und Prüfmittel** | `docs/scrum/reviews/sprint-07-s70-…/` — Bericht, Bild der neuen Szene, Rollwerttabelle (B7). Kein neues Prüfmittel zu bauen. |
| **Fachliche Quellen** | `SPEC.md` Abschnitt 9, die Kopfregel `:513–518` — sie regelt heute nur den Grenzübertritt und ist um den neuen Fall zu ergänzen (AK 6). Wireframe 3b, Fall 4 als **Referenz**. |
| **Ausdrücklich nicht** | `src/ui/librarywindow.h` (siehe Falle 2 — es braucht keinen neuen Helfer), `src/ui/notelistmodel.*`, `src/ui/notelistdelegate.*`, der Aktions- und Knopfblock `:168–176`, `:249–265`, `:355–372` (Fläche #72), `src/capture/*` und `tests/capturetest.cpp`/`tests/captureshots.cpp` (Fläche #83), `src/shell/*`, `src/store/*`, `wireframes/`. |

**Zu `wireframes/`:** Wireframe 3b beschreibt Fall 4 als Grenzübertritt. Ob
die neue Regel dort nachzuziehen ist, ist eine UX-Frage und keine der Story —
sie ist zu **melden**, nicht zu heilen.

---

## Feld 2 — Gemessene Fallen

1. **Die Bedingung wird erweitert, nicht ersetzt** (`m1`). Heute steht in
   `:785`:
   `if (head.isValid() && crossesAGroupBoundary && !m_selectionFollowsAPress)`.
   Der Grenzübertritt-Zweig muss bleiben — an ihm hängen vier Prüfsätze. Neu
   ist ein **Oder**-Zweig „das Ziel ist die erste Notiz seiner Gruppe".
2. **Die erste Notiz einer Gruppe braucht keinen neuen Helfer.** `groupHeadOf()`
   (`:677–689`) sucht den nächsten Kopf aufwärts; steht er genau eine Zeile
   höher, ist `index` die erste Notiz seiner Gruppe — `head.row() == index.row() - 1`.
   Damit bleibt `librarywindow.h` unangetastet und die Story ohne
   Kopfdatei-Änderung.
3. **`!m_selectionFollowsAPress` muss im selben `if` bleiben — und keiner der
   heutigen Prüfsätze fängt es, wenn es das nicht tut** (`m3`). Wer einen
   zweiten `if`-Block danebenstellt und die Marke vergisst, öffnet #57 wieder.
   Der einzige Klick-Prüfsatz mit Kopf-Zusicherung
   (`leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked`, `:1482`)
   zielt auf die **letzte** Notiz von „Gestern". Ein Klick auf die **erste**
   Notiz einer Gruppe kommt in keinem Prüfsatz vor. Deshalb gehört ein solcher
   Prüfsatz zur Story (AK 3).
4. **Der Fehler ist heute grün** (`m3`). Von sieben `Key_Up`-Stellen im Bestand
   trifft keine den Fall: sechs überschreiten eine Gruppengrenze, die siebte
   (`leavesTheHeadOutsideWhereItCannotFitWithTheSelection`, `:1717`) geht
   innerhalb einer Gruppe auf eine Notiz, die nicht die erste ist. Ein
   Testaufbau, in dem der Fehler nicht auftreten kann, ist kein Test — hier ist
   der Aufbau schlicht nicht vorhanden.
5. **Die Platzbedingung gilt weiter.** `:788` lässt den Kopf draußen, wenn Kopf
   und Auswahl nicht zusammen ins Bild passen. Für die erste Notiz ist sie fast
   immer erfüllt (der Kopf steht direkt darüber), aber sie darf durch die
   Umstellung nicht wegfallen —
   `leavesTheHeadOutsideWhereItCannotFitWithTheSelection` ist ihr Wächter.
6. **Kein Schwellwert, keine Sichtbarkeitsbedingung.** Das Issue verbietet es,
   und `bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready` (`:1425`) ist der
   Prüfsatz dagegen. Das ist die einzige der hier genannten Fallen, die bereits
   einen Wächter hat.
7. **Die 35 px aus dem Issue gehören in keinen Prüfsatz.** Sie stammen aus der
   Sitzung des Kunden; offscreen misst der Läufer andere Zeilenhöhen (der
   Testkommentar bei `:1296–1301` nennt Kopf 47 px, Eintrag 65 px). Geprüft
   wird, **dass** der Kopf im Bild ist, nicht um wie viel er es verfehlte.
8. **Vor dem Bildbeleg bauen**: `cmake --build build --target libraryshots`,
   und `QT_QPA_PLATFORMTHEME=kde` setzen. Der Läufer ist seit dem 04.08.2026
   nicht mehr `EXCLUDE_FROM_ALL`, aber wer ihn ohne Bauen startet, bekommt
   weiter ein plausibles Bild eines alten Standes.

---

## Feld 3 — AK-Urteil: **ready = nein**

Prüfbarkeit je Kriterium, gemessen:

| AK | prüfbar? | Begründung |
|---|---|---|
| 1 — Kopf ganz im Bild nach Aufwärtsschritt | **ja** | Rollwert und `visualRect(head)` sind messbar; Muster liegt vor (`:1425–1480`). Wortlaut „y ≥ 0" ist schwächer als die bestehende Zusicherung `viewport()->rect().contains(...)` — nachzuschärfen, kein Blocker. |
| 2 — Prüfsatz aus Sprint 3 bleibt grün | **ja** | Der wörtlich zitierte Prüfsatz ist eindeutig `bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready`. Der Name gehört ins Kriterium. |
| 3 — Klickpfad bleibt ruhig | **ja, aber unvollständig** | Messbar über den Rollwert vor/nach. Der Fall „Klick auf die **erste** Notiz einer Gruppe" ist von keinem Prüfsatz gedeckt (`m3`) — genau der Fall, den die Story neu behandelt. |
| 4 — kein zusätzliches Rollen innerhalb der Gruppe | **ja** | `staysPutWhileTheSelectionMovesWithinItsGroup` und `leavesTheHeadOutsideWhereItCannotFitWithTheSelection` decken es. |
| 5 — Bildbeleg und Rollwerte | **ja** | Läufer und Muster vorhanden. Ablageort und Läufer sind zu benennen (B7). |
| 6 — SPEC 9 nachziehen | **ja** | `git diff SPEC.md`, Fundstelle `:513–518`. Die Aussage „heute sagt die SPEC dazu nichts" trifft für den neuen Fall zu (`m4`). |

**Das Urteil hängt nicht an den Kriterien, sondern an zwei selbstdeklarierten
offenen Punkten** — und die DoR vom 04.08.2026 ist genau dagegen gefasst:

- **Abschnitt „Nachbarschaft":** *„Wer #70 später baut, hält den dann geltenden
  Stand von #59 dagegen … **Vor dem Ziehen prüfen.**"*
- **Abschnitt „Nicht geschätzt":** *„Vor dem Ziehen zwei unabhängige
  Schätzungen; das `sp:`-Label wird im selben Zug gesetzt."* Das Verfahren ist
  am 04.08.2026 beendet, das `sp:`-Label gibt es nicht mehr. Die Zeile ist eine
  Arbeitsanweisung, die ins Leere führt.

Hinzu kommt: **AK 3 ist nach der Entscheidung über #71 neu zu fassen** (Frage 1
unten). Solange offen ist, ob #71 im selben Sprint läuft, sagt AK 3 nicht
eindeutig, welcher Klickpfad ruhig bleiben soll.

### Was der PO ergänzen muss — zum Übernehmen

**Streichen:**

1. Der Abschnitt **„Nicht geschätzt"** ganz.
2. Im Abschnitt „Nachbarschaft" der Satz „**Vor dem Ziehen prüfen.**" samt der
   beiden Mutmaßungen; an seine Stelle das Messergebnis:

   > **#59 ist gemessen und berührt diese Story nicht** (`084385c`,
   > Vorprüfung #70, `messungen-b/m6`). Die Heilung sitzt in `changeEvent()`
   > und `regroupList()` und hängt das Neugruppieren an den Kalendertag;
   > `showNote()` und die Kopf-Hol-Bedingung sind unverändert. Weder billiger
   > noch kollidierend.

**Kriterien neu fassen (Wortlaut prüfbar, zum Einsetzen):**

3. **AK 1:**
   > Wandert die Auswahl per Pfeiltaste aufwärts auf die **erste Notiz einer
   > Gruppe**, liegen danach **Gruppenkopf und Auswahl ganz im Bild**
   > (`viewport()->rect().contains(visualRect(...))` für beide), nachgewiesen
   > durch Messung des Rollwerts vor und nach dem Tastendruck, nicht durch
   > Bildvergleich.

4. **AK 2:**
   > Der Prüfsatz aus Sprint 3 —
   > `bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready` in
   > `tests/librarytest.cpp` — bleibt grün, und mit ihm
   > `staysPutWhileTheSelectionMovesWithinItsGroup` und
   > `leavesTheHeadOutsideWhereItCannotFitWithTheSelection`.

5. **AK 3** (Fassung ohne #71 im Sprint):
   > Der Klickpfad aus #57 bleibt ruhig, **auch bei einem Klick auf die erste
   > Notiz einer Gruppe**: Der Rollwert ist vor und nach dem Klick derselbe,
   > und die geklickte Zeile steht danach an derselben Stelle. Heute prüft das
   > kein Prüfsatz — der einzige Klicktest mit Kopf-Zusicherung zielt auf die
   > letzte Notiz einer Gruppe.

   (Fassung **mit** #71 im Sprint: „Der Klickpfad **in der Fassung nach #71**
   bleibt ruhig …" — siehe Frage 1.)

6. **Neues AK — die Grenze der Regel, damit sie nicht später als Fehler
   gemeldet wird:**
   > Passen Gruppenkopf und Auswahl nicht zusammen ins Bild, bleibt der Kopf
   > draußen. Die bestehende Bedingung
   > `selected.bottom() - heading.top() <= viewport()->height()` gilt für den
   > neuen Fall unverändert.

7. **AK 5:**
   > Bildbeleg aus `tests/libraryshots.cpp` (neue Szene nach dem Muster von
   > Szene 8), abgelegt unter `docs/scrum/reviews/` (B7); die gemessenen
   > Rollwerte stehen im Bericht. **Offscreen genügt** — das Kriterium
   > behauptet nichts über Hülle, Rundung, Kontur, Schatten, Dekoration oder
   > Durchsichtigkeit (B21).

---

## Feld 4 — Prüfmittel, und was ein Agent nicht prüfen kann

| AK | Prüfmittel |
|---|---|
| 1 | Neuer Prüfsatz in `tests/librarytest.cpp` nach dem Muster `:1425–1480`: drei Gruppen, Liste auf eine Zeile innerhalb der mittleren Gruppe gerollt, `QTest::keyClick(list, Qt::Key_Up)` auf die **zweite** Notiz dieser Gruppe, dann `viewport()->rect().contains(visualRect(head))`. Rollwert über `list->verticalScrollBar()->value()` vorher/nachher. Offscreen. |
| 2 | `ctest --test-dir build -R librarytest` — die drei benannten Prüfsätze bleiben grün. |
| 3 | Neuer Klick-Prüfsatz nach dem Muster `:1482–1556`, Ziel ist die **erste** Notiz einer Gruppe. Gemessen wird der **Rollwert vor und nach dem Klick**, nicht der Endzustand — der Grund steht im Kommentar dieses Prüfsatzes (`:1502–1505`): das nachfolgende `scrollTo(index)` lässt beide Fälle im Bild gleich aussehen. |
| 4 | Bestandsprüfsätze plus ein Aufwärtsschritt auf eine Notiz, die nicht die erste ihrer Gruppe ist: `QCOMPARE(scrollBar->value(), rolledTo)`. |
| 5 | `cmake --build build --target libraryshots` (Falle 8), dann `QT_QPA_PLATFORMTHEME=kde ./build/bin/libraryshots <Ordner>`. Neue Szene nach Muster `:309–340`, Helfer `walkUp` vorhanden. |
| 6 | `git diff SPEC.md` gegen `sprint-07-basis`; Fundstelle Abschnitt 9, `:513–518`. |

**Grenzen der Prüfbarkeit, ausgesprochen:**

- **Der Betrag der Verfehlung ist nicht offscreen prüfbar.** Die 35 px stammen
  aus der Kundensitzung; offscreen gelten andere Zeilenhöhen (Falle 7). Prüfbar
  ist das Vorzeichen, nicht der Betrag.
- **Ein Bild aus der angemeldeten Sitzung ist hier nicht gefordert.** Kein
  Kriterium behauptet etwas über Hülle, Rundung, Kontur, Schatten, Dekoration
  oder Durchsichtigkeit; B21 greift nicht.
- **Kein Fensterwechsel im Spiel.** Die Wayland-Grenze aus Sprint 6 §16.1
  (M-B1) — ein Agent kann sich den Fokus nicht zurückholen — betrifft diese
  Story nicht: Alle Eingaben gehen an ein Fenster, das den Fokus behält.

---

## Feld 5 — Größenklasse: **`size:s`**

Gemessene Fläche: **eine** Bedingung in einer bestehenden `if`-Zeile (`:785`),
**keine** Kopfdatei-Änderung (Falle 2), **keine** Build-Änderung, **kein** neuer
Prüfweg — Prüfsätze und Bildläufer bestehen samt Muster und Helfer. Dazu drei
Prüfsätze, eine Bildszene und ein SPEC-Absatz von vier Zeilen.

Das ist „läuft nebenher — wenige Dateien, kein neuer Prüfweg".

**Gegenprobe gegen `size:m`** („trägt einen Strang aus"): Sie träfe zu, wenn
eine neue Datei, ein neuer Prüfweg oder eine Build-Änderung nötig wäre. Keines
davon ist der Fall. **Der einzige Weg, auf dem #70 zu `m` würde**, ist die
Kopplung mit #71 — siehe Frage 1; dann ist es aber nicht mehr #70, sondern ein
gemeinsamer Strang.

**Label:** noch **nicht** gesetzt. Es wird im selben Zug mit dem konsolidierten
`bericht.md` gesetzt, nicht mit einer Einzelmessung — die Klasse von A steht
noch aus, und bei Abweichung gilt die höhere.

---

## Feld 6 — Offene Fragen

**An den PO:**

1. **Die sieben Punkte aus Feld 3.** Sie sind der Grund für „ready = nein" und
   allesamt Textarbeit am Issue; die Story wird davon nicht größer.
2. **#71 im selben Sprint — ja oder nein?** Davon hängt der Wortlaut von AK 3
   ab (Frage 1 unten). Solange das offen ist, kann AK 3 nicht endgültig
   formuliert werden.
3. **Prozessbeobachtung, kein Mangel:** Von den fünf am 04.08.2026 vermessenen
   Kandidaten trägt nur #72 sein Klassen-Label; #71, #73, #61 und #76 tragen
   keines, obwohl die Klasse in der Commit-Botschaft von `581dacc` steht. Ein
   konsolidiertes `bericht.md` gibt es zu keinem von ihnen. In der einzigen
   Quelle der Wahrheit steht die gemessene Klasse damit nirgends.

**An den Kunden:** keine.

---

## Die drei Sprint-Fragen

### 1. #70 gegen #71 — ein gemeinsamer Strang

**Urteil: ein gemeinsamer Strang, und zwar #71 zuerst.** Zwei parallele Stränge
sind ausgeschlossen; verschiedene Sprints sind zulässig und für Sprint 7 die
bessere Wahl (Frage 3).

Drei Messgründe, aufsteigend nach Gewicht:

- **Der Textabstand ist null** (`m4`). #70 arbeitet in `:780–793`, #71 an
  `:792` — dieselbe 14-Zeilen-Bedingung in einer Funktion von 108 Zeilen.
- **Die Kriterien widersprechen einander.** #70 AK 3 macht die Ruhe des
  Klickpfads aus #57 zur Zusicherung; #71 ändert genau diesen Pfad. Läuft #71
  parallel, ist AK 3 in dem Augenblick falsch, in dem #71 abgenommen wird.
- **Der schwerste Grund, und er stand in keinem der beiden Issues** (`m5`):
  #71 zielt nach der A-Messung auf `setVerticalScrollMode(ScrollPerPixel)`.
  Damit **wechselt die Einheit des Rollwerts** — heute steht die Liste im
  Voreinstellungsmodus `ScrollPerItem`, und `verticalScrollBar()->value()` ist
  eine Zeilennummer. Vier Prüfsätze stellen ihren Fall über
  `setValue(noteRow(list, N).row())` her; unter `ScrollPerPixel` heißt dieselbe
  Zeile „roll 8 Bildpunkte weit". **Drei dieser vier sind genau die Prüfsätze,
  auf die #70 in AK 2, AK 3 und AK 4 baut** (`:1406`, `:1459`, `:1532`). Sie
  werden davon nicht rot — sie prüfen etwas anderes. Das ist die Bauart „vier
  grüne Tests, die nichts prüfen" aus `CLAUDE.md`, hier vorab gemessen.

**Reihenfolge: #71 vor #70**, in einem Strang, zwei Commits. #71 legt die
Maßeinheit fest, in der #70 seine Zusicherungen schreibt; umgekehrt wären alle
#70-Prüfsätze nach #71 erneut zu prüfen.

**Neu zu fassen ist danach AK 3 von #70:**

> Der Klickpfad **in der Fassung nach #71** bleibt ruhig: Ein Klick auf eine
> sichtbare Notiz — auch auf die **erste** Notiz einer Gruppe — lässt die
> geklickte Zeile an ihrer Stelle stehen und wählt genau sie aus. Gemessen
> werden Rollwert und `visualRect(target).y()` vor und nach dem Klick, in der
> Einheit, die der Rollmodus nach #71 hat.

Zusätzlich, wenn #71 auf `ScrollPerPixel` geht: **alle vier Aufbauten aus `m5`
sind auf die neue Einheit umzustellen** — das gehört in #71, nicht in #70, und
es ist Umfang, kein Nebenher.

### 2. Passt #70 neben #83?

**Ja.** #70 ist `size:s`, #83 ist `size:l`; die Regel „neben einer `size:l`
steht nur `size:s`" ist erfüllt. Die Flächen sind vollständig disjunkt (`m7`):
`src/capture/` gegen `src/ui/`, `capturetest`/`captureshots` gegen
`librarytest`/`libraryshots`, keine gemeinsame `CMakeLists.txt`. Einzige
Berührung ist `SPEC.md` — #83 in den Abschnitten 3, 15 und 16, #70 in
Abschnitt 9 (`:513–518`); nächstgelegen sind das Ende von Abschnitt 9 (`:604`)
und der Anfang von Abschnitt 15 (`:719`): **115 Zeilen Abstand**. Git mischt
mit drei Zeilen Kontext, ein Textkonflikt ist ausgeschlossen.

**Nicht daneben passt #70 zusammen mit #71.** Der gemeinsame Strang aus Frage 1
trägt zwei Issues, den Wechsel des Rollmodus und die Umstellung von vier
bestehenden Prüfsätzen. Das ist keine `s` mehr.

### 3. Der Zuschnitt, den ich vertrete

**Sprint 7 = #83 (`l`) + #72 (`s`) + #70 (`s`) — drei Issues.**

**Sprint-Konto (B12), Ausgangsstand:** `3 Issues · 1×l, 2×s`. Beide Grenzen mit
Luft eingehalten: drei von höchstens vier Issues, eine `l` und daneben nur `s`.

| | **Strang A** | **Strang B** |
|---|---|---|
| **Issues** | #83 (`l`) | #72 (`s`), #70 (`s`) |
| **Zweig** | `story/83-native-huelle` | `story/72-tooltips` |
| **Quellen und Tests** | `src/capture/capturewindow.{h,cpp}`, `tests/capturetest.cpp`, `tests/captureshots.cpp` | `src/ui/librarywindow.cpp` — `:168–176`, `:249–265`, `:355–372` (#72) und `:780–793` (#70); `tests/librarytest.cpp`, `tests/libraryshots.cpp` |
| **Build** | `CMakeLists.txt` (Wurzel), `src/CMakeLists.txt`, `tests/CMakeLists.txt` | **keine** |
| **Fachliche Quellen** | `SPEC.md` Abschnitte 3 (`:124–214`), 15 (`:719–748`) und 16 (`:749–809`) | `SPEC.md` Abschnitt 9 (`:491–604`) |

**Kleinster gemessener Abstand zwischen den Strängen (`m7`): 115 Zeilen**, und
er liegt in `SPEC.md` — in Code und Tests teilen die Stränge **keine einzige
Datei**. Zwei Worktrees sind damit nicht nur zulässig, sondern angezeigt.

**Innerhalb von Strang B** (ein Strang, also ohnehin konfliktfrei; die Zahlen
belegen, dass sich #72 und #70 auch fachlich nicht berühren): 408 Zeilen
Abstand in `librarywindow.cpp`, über 1360 Zeilen zwischen den Prüfsatz-Körpern
in `librarytest.cpp`; einzig die Slot-Deklarationen liegen rund 50 Zeilen
auseinander.

**Warum #71 nicht mitkommt** — zwei Gründe, beide gemessen:

1. **Seine Größenklasse steht unter einer offenen Bedingung.** Beide Messungen
   sagen `size:s`, beide **bedingt**: Braucht der Nachbarzeilen-Befund eine
   Sitzungsprobe, entsteht ein neuer Prüfweg und die Story wird `size:m` — dann
   passt sie nicht neben #83. Diese Messung steht aus und gehört vor die
   Freigabe.
2. **Mit #71 wären es vier Issues an der oberen Grenze, und einer der Stränge
   trüge den Rollmodus-Wechsel samt Umstellung von vier bestehenden Prüfsätzen**
   (`m5`). Die Klassenregel wäre formal gehalten (1×l, 3×s) und die Sache
   trotzdem falsch geschnitten — genau die Lage, gegen die die Dateimenge in
   Feld 1 die Last der weggefallenen Punktzahl trägt.

**Wenn der Kunde das Paket „ruhige Liste" früher will**, ist der saubere
Tausch: **#83 heraus, Sprint 7 = #70 + #71 (ein Strang, #71 zuerst) + #72**.
Das ist eine Prioritätsentscheidung des Kunden, keine des Scrum Masters — ich
lege sie vor, ich treffe sie nicht.

**Was vor der Freigabe noch fehlt:** Nach heutigem Stand ist **allein #83**
ready. #72 und #70 tragen je ein „nein" aus reiner Textarbeit am Issue (für
#72 siehe `docs/scrum/vorberichte/72-tooltips/messung-b.md`, für #70 Feld 3
oben). Ohne diese Nacharbeit ist der vorgeschlagene Zuschnitt nicht ziehbar.

---

## Nachtrag 05.08.2026 — korrigierte Prämisse

Angehängt am 05.08.2026, 18:50 CEST, Stand `7afe022`. **Der Text oberhalb
dieser Linie bleibt unverändert** (B17: ein überholter Beleg wird geankert,
nicht geglättet). Messbeleg: `messungen-b/m8-nachtrag-lesart2.txt`.

### Was überholt ist

**Fund 2 aus Frage 1 fällt weg.** Ich hatte geschrieben, die #71-Heilung ziele
auf `setVerticalScrollMode(ScrollPerPixel)`, und daraus den Wechsel der
Rollwert-Einheit hergeleitet (`m5`). Das war **Lesart 3** von dreien; die
A-Messung empfahl Lesart 2, und der PO hat am 05.08.2026 Lesart 2 entschieden
(`docs/scrum/vorberichte/71-klick-nachbarzeile/bericht.md`, Feld 6). Nachgeprüft
und bestätigt (`m8`, Abschnitt 1): eine Bedingung an `librarywindow.cpp:792`,
kein Rollmodus-Wechsel. Damit bleibt `ScrollPerItem`, die vier Aufbauten in
`:1406`, `:1459`, `:1532`, `:1595` behalten ihre Einheit, und die drei
Prüfsätze hinter #70 AK 2, AK 3 und AK 4 messen weiter, was sie messen sollen.
`m5` bleibt als Messung stehen und beschreibt, was unter Lesart 3 gegolten
hätte — nicht, was gilt.

**Der Vorbehalt zu #71 ist ebenfalls erledigt** (`reproduktion.md`, 05.08.2026):
Der Fall ist im vorhandenen offscreen-Aufbau reproduzierbar, kein neuer
Prüfweg, `size:s` bestätigt.

**Was davon unberührt bleibt:** das Urteil „ein gemeinsamer Strang, #71
zuerst". Es hing nie allein an Fund 2 — der Textabstand null in `:780–793` und
der Kriterien-Widerspruch tragen es für sich. Und es kommen zwei neu gemessene
Gründe hinzu (siehe (a)).

### (a) Trägt der Vier-Issue-Schnitt? **Ja.**

**#83 (`l`) + #71 (`s`) + #70 (`s`) + #72 (`s`), zwei Stränge, Strang B in der
Reihenfolge #71 → #70 → #72.** Beide Einwände, die ich gegen die Aufnahme von
#71 geführt hatte, sind durch Messung erledigt, und keiner der drei Gründe
gegen den Schnitt ist übrig geblieben:

- Die Größenklasse von #71 stand unter einer Bedingung → gemessen, `s`.
- Der Strang trüge den Rollmodus-Wechsel samt Umstellung von vier Prüfsätzen →
  entfällt mit Lesart 2.
- Die Klassenregel ist gehalten: eine `l`, daneben nur `s`.

**Zwei Gründe kommen hinzu, beide neu gemessen** (`m8`) — sie stützen den
Schnitt stärker als die Argumentation, mit der er vorgelegt wurde:

1. **#70 und #71 überschreiben denselben SPEC-Absatz.** #70 AK 6 zieht die
   Kopfregel nach (`SPEC.md:513–514`), #71 AK 7 die Mausklick-Bedingung
   (`:514–518`) — **gemeinsame Zeile 514**. In `SPEC.md` ist der Abstand
   ebenso null wie im Code. Getrennt in zwei Sprints würde derselbe Absatz
   zweimal umgeschrieben, und die erste Fassung wäre nach der zweiten ein
   überholter Beleg mit Ankerpflicht.
2. **Beide hängen an derselben Marke.** #71 AK 6 heilt die Klebrigkeit von
   `m_selectionFollowsAPress`; #70 hängt seinen neuen Auslöser hinter genau
   diese Marke (`:785`). #71 zuerst heilt sie **einmal für beide Auslöser**.
   Wer #70 zuerst baut, hängt einen zweiten Auslöser an eine Marke, deren
   Fehler bekannt und noch nicht behoben ist.

**Die Überlegung des Team-Leads greift, aber nicht ganz so weit, wie sie
formuliert ist.** „Trennt man beide auf zwei Sprints, wird ein vom PO
abgenommenes Kriterium im Folgesprint falsch" — das gilt für die Reihenfolge
**#70 vor #71**. In der umgekehrten Trennung (#71 in Sprint 7, #70 in Sprint 8)
entsteht der Widerspruch nicht: #70 AK 3 würde dann von vornherein gegen den
gelieferten Stand geschrieben. Der Vier-Issue-Schnitt trägt also nicht, *weil*
die Trennung unmöglich wäre, sondern weil sie zweimal dieselben vierzehn Zeilen
und denselben SPEC-Absatz aufmacht, ohne dafür etwas einzusparen.

**Zwei Bedingungen, unter denen ich den Schnitt vertrete:**

1. **Strang B ist ein Strang** — ein Worktree, ein Agent, drei Commits in der
   Reihenfolge #71 → #70 → #72. Nicht drei Stränge und keine Parallelarbeit
   innerhalb von B. Der Umfang trägt das (`m8`, Abschnitt 4): rund sechs
   geänderte Zeilen in `librarywindow.cpp`, fünf bis sechs neue Prüfsätze, zwei
   Bildszenen, ein SPEC-Absatz, keine Build-Änderung, kein neuer Läufer.
2. **Das Sprint-Konto steht bei der Freigabe auf `4 Issues · 1×l, 3×s` — die
   obere Grenze der Issue-Zahl ist damit erreicht, nicht nur berührt.** Das
   gehört dem Kunden bei der Vorlage gesagt: **Jeder** Zugang nach der Freigabe
   ist dann eine Grenzüberschreitung nach B12 und ihm als solche vorzulegen,
   nicht nur als Story. Bei drei Issues wäre das nicht so, und genau diese
   Blindstelle hat Sprint 3 gekostet.

**Und eine Empfehlung, die keine Bedingung ist:** #72 steht zu Recht am Ende.
Es ist der einzige der drei, der 408 Zeilen von den anderen entfernt liegt und
keinen ihrer Prüfsätze berührt. Läuft Strang B lang, ist #72 das Stück, das
ohne Schaden an der Beweislage der anderen herausfällt.

### (b) Entfällt — der Schnitt trägt.

Zur Vollständigkeit, falls der Kunde die Vier zurückweist: Der einzige
Ersatzschnitt, der den Kriterien-Widerspruch **nicht** erzeugt, ist
**#83 + #71 + #72** mit #70 im Folgesprint. **#83 + #70 + #72** mit #71 später
ist die Variante, die der Team-Lead zu Recht ablehnt: Sie nimmt #70 AK 3 ab und
macht die Zusicherung im nächsten Sprint falsch.

### (c) #70 AK 3 in der Fassung „nach #71", unter Lesart 2

Meine Fassung oben in Feld 3 („in der Einheit, die der Rollmodus nach #71 hat")
ist mit Lesart 2 gegenstandslos. Sie wird ersetzt durch:

> Der Klickpfad bleibt ruhig, in der Fassung nach #71: Ein Klick auf eine
> sichtbare Notiz lässt das Bild stehen — **auch dann, wenn die geklickte Notiz
> die erste Notiz ihrer Gruppe ist**. Rollwert und `visualRect(target).y()`
> sind vor und nach dem Klick gleich, und der Gruppenkopf wird **nicht**
> hereingeholt. Heute deckt kein Prüfsatz diesen Fall: Der einzige Klicktest
> mit Kopf-Zusicherung
> (`leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked`) zielt
> auf die **letzte** Notiz einer Gruppe.

**Warum das nach #71 ein anderes Kriterium ist als vorher:** #71 nimmt das
Nachrücken vom Mausdruck ganz weg. Der Satz „kein Sprung bei einem Klick" ist
danach von #71 AK 2 zugesichert und in #70 eine Wiederholung. Was #70 dann
allein zu sichern hat, ist, dass sein **neuer** Auslöser — die erste Notiz
einer Gruppe — nicht auf den Mauspfad durchschlägt. Genau das steht oben, und
genau diese Lücke ist gemessen (`m3`): Ein Klick auf die erste Notiz einer
Gruppe kommt in keinem Prüfsatz vor.

**Anschlusspflicht, damit die beiden Kriterien nicht auseinanderlaufen:** #71
AK 5 nennt `leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked`
und `keepsTheHeadFetchAfterAClickThatSelectedNothing` als Prüfsätze, die grün
bleiben müssen. Beide zielen auf die **letzte** Notiz von „Gestern" und werden
von #70 nicht berührt — geprüft, kein Konflikt.
