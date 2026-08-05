# Vorprüfung #70 — Messung Bearbeiter A (`denkzettel-dev`)

**Gegenstand:** Issue #70, „Bibliothek: Erste Notiz einer Gruppe holt ihren
Tageskopf ins Bild (Tastaturpfad)" · **Datum:** 05.08.2026, Ganymed ·
**Quellstand:** `main` @ `7afe022` — `src/`, `tests/` und `SPEC.md` sind darin
Zeile für Zeile dieselben wie in `581dacc`, gegen das die ersten Läufe gingen ·
**Belege:** `messungen/`, Sonden in `sonden/`, Kandidatenfassung als
`flicken/a-70.patch` · **wiederholbar über**
`bash docs/scrum/vorberichte/70-gruppenkopf-tastatur/pruefen.sh`

Dieser Bericht trägt die Felder **1, 2, 4, 5 und 6** und legt zu **Feld 3**
einen Vorschlag vor. **Das Ready-Urteil fällt der Scrum Master.**

**Stand der Werkzeuge** (B17): qt6-base 6.11.1, Bau als `Debug` in
`build-vor70/`. Nach `/usr` ist nichts installiert worden. `src/` ist nicht
angefasst worden: Die Kandidatenfassung entsteht als Kopie unter `kandidat/`.

---

## Feld 1 — Dateimenge (am Code vermessen, Notation nach B13)

| | **#70** — Die erste Notiz einer Gruppe holt ihren Kopf |
|---|---|
| **Issue** | **#70** (`epic:M2`, `typ:bug`) |
| **Zweig** | `story/70-gruppenkopf-tastatur` |
| **Quellen & Tests** | `src/ui/librarywindow.cpp` — **eine Stelle**, der Bedingungskopf des Kopfholens `:783–785`. Der Rumpf `:786–790` und das `scrollTo(index)` `:792` bleiben, wie sie sind (gemessen, Feld 2, F3). Umfang der Kandidatenfassung: **+6 Zeilen, −1 Zeile**, alles innerhalb eines Rumpfes (`flicken/a-70.patch`).<br>`tests/librarytest.cpp` — neue Zusicherungen in der Nachbarschaft der sieben vorhandenen Kopf-Zusicherungen; die Slotliste `:136–144`, die Rümpfe ab `:1294`. Der neue Fall gehört zwischen `staysPutWhileTheSelectionMovesWithinItsGroup` (`:1376`) und `bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready` (`:1425`) — beide sind seine Nachbarn im Sachverhalt.<br>`tests/libraryshots.cpp` — **Szene 7** `:278–306`. Sie ist bereits genau der Fall dieser Story und behauptet heute etwas Falsches (F7); ihr Kommentar `:278–280` wird wahr, nicht neu geschrieben.<br>`src/ui/librarywindow.h` — **nicht nötig.** Der Kandidat kommt ohne neue Methode aus; ein privater Helfer wäre eine Ermessensfrage, kein Zwang. |
| **Build** | **Nichts.** Keine neue Bibliothek, keine neue Quelldatei, keine CMake-Zeile. |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-NN-s70-gruppenkopf/`, neu. **Wiederverwendbar, nicht neu zu erfinden:** `sonden/kopfsonde.cpp` dieser Vorprüfung misst die vier Fälle der Akzeptanzkriterien (B, D, E, I) und ist der fertige Bauplan des Rollwert-Nachweises; `messungen/libraryshots-1.6/07-fall4-uebergang-beim-scrollen.png` ist der Bildbeleg zu AK 5, erzeugt mit `QT_SCALE_FACTOR=1.6`. |
| **Fachliche Quellen** | **SPEC 9**, `SPEC.md:513–518` — der Satz, den AK 6 meint. Er ist **vorhanden**, nicht abwesend; er deckt den Grenzübertritt und nicht die erste Notiz (F8). **Zeichnung 3b, Fall 4** (`wireframes/Denkzettel Wireframes.dc.html`) — ihr Begründungssatz *„die Auswahl steht nie ohne ihre Überschrift da"* beschreibt bereits den Zielzustand dieser Story; ihr Mechanismussatz nennt nur den Grenzübertritt (F9). **`wireframes/` bleibt UX**, der Dev liest sie nur. **UI-Review Sprint 5**, `docs/scrum/reviews/sprint-05-ui-review/bericht.md` — Herkunft von N2 und von #71. |
| **Ausdrücklich nicht** | `src/capture/*`, `src/shell/*`, `src/store/*`, `src/analysis/*`, `src/transcribe/*`; `src/ui/notelistmodel.*`, `notelistdelegate.*`, `elidedlines.*`, `pendingdeletion.*`, `timestampformat.*`; in `librarywindow.cpp` alles außerhalb von `showNote()` — insbesondere `changeEvent` `:538–556` und `regroupList()` `:612–620` (**das ist #59, bereits geliefert**) sowie `eventFilter` `:517–536`; **`m_list->scrollTo(index, …)` in `:792` — das ist #71** (F6); `wireframes/`; `SPEC.md` außer dem Satz `:513–518`. |

### Kollisionsfläche — **#71 gehört nicht neben #70**

| Vorgang | Berührung | Kleinster Abstand | Urteil |
|---|---|---|---|
| **#59** (Rollstelle bei Aktivierung, `084385c`, **geliefert**) | dieselbe Datei, **keine gemeinsame Funktion**: #59 sitzt in `changeEvent`, `reload`, `regroupList` und im Header, #70 in `showNote` | rund 130 Zeilen; kein gemeinsamer Rumpf | **erledigt, keine Auflage** — Wirkung siehe F5 |
| **#71** (Klick auf angeschnittene Zeile) | dieselbe Datei, **derselbe Rumpf**, dasselbe `if (index.isValid())`-Block | **eine Zeile.** #70 ändert `:781–788`, #71-Lesart 2 ändert `:789–795` (`messungen/sonde2-mischprobe.txt`) | **nicht nebeneinander** — Begründung unten |
| **#72** (Tooltips) | dieselbe Datei, andere Rümpfe (`:168–369`) | rund 400 Zeilen | **parallel möglich** |

**Warum #71 nicht neben #70 steht — gemessen, nicht vermutet.** Der UI-Review
S5 legt für #71 **zwei Lesarten** vor und sagt ausdrücklich, welche gilt, sei
Produktentscheidung (`bericht.md:253–258`). Die Mischprobe hat beide gegen die
Kandidatenfassung von #70 gehalten:

- **Lesart 2** („`scrollTo` respektiert denselben Merker"): `git merge-file`
  **Rückgabewert 0** — die Hunks liegen eine Zeile auseinander und mischen
  sauber.
- **Lesart 1** („die Bewegung geschieht nach der Auswahlverarbeitung"):
  `git merge-file` **Rückgabewert 1** — ein Konflikt. Sie schiebt beide
  `scrollTo`-Aufrufe in einen aufgeschobenen Aufruf und schluckt dabei genau
  den `if`-Rumpf, den #70 ändert.

Zum Zeitpunkt des Schnitts ist **nicht entschieden**, welche Lesart #71 nimmt.
Damit ist auch nicht entschieden, ob die beiden Stränge kollidieren — und das
ist der teurere Teil: **#70 AK 3 verlangt, dass der Klickpfad ruhig bleibt,
während #71 genau diesen Pfad ändert.** Ein Strang würde seinen eigenen
Nachweis gegen einen Stand führen, den der andere gerade verschiebt. Beide in
**einem** Strang, in dieser Reihenfolge (#70 zuerst, #71 darauf), kosten weniger
als zwei Stränge mit Rebase und doppeltem Nachmessen. Sonst in verschiedene
Sprints.

---

## Feld 2 — Gemessene Fallen

**F1 — Der Befund liegt bei genau −35 px, unverändert nach #59.** Aufwärts auf
die erste Notiz der Gruppe „Gestern": Rollwert 11 → 10, Kopf danach bei
`y=−35 h=35`, also mit der Unterkante exakt auf der Bildkante. Der Wert aus dem
Issue stimmt am heutigen Stand, in einer 8er-Gruppe wie in einer 3er-Gruppe.
*Beleg:* `messungen/sonde1-heutiger-stand-offscreen.txt`, Abschnitte B und I.

**F2 — Die Ursache ist die Gruppenbedingung, nicht das Rollen.** `showNote()`
holt den Kopf nur bei `crossesAGroupBoundary` (`librarywindow.cpp:783`). Beim
Aufwärtsgang auf die erste Notiz ist der Vorgänger die **zweite** Notiz
derselben Gruppe — kein Übertritt, keine Bedingung erfüllt, kein Holen. Der
tragfähige Weg ist deshalb **keine neue Rollmechanik**, sondern eine zweite
Auslöserbedingung neben dem Übertritt.

**F3 — Die vorhandenen zwei `scrollTo`-Zeilen erledigen die Sache
vollständig; die zweite rollt nicht zurück.** Im Zustand des Befundes von Hand
nachgefahren: `scrollTo(Kopf)` Rollwert 10 → 9, danach `scrollTo(Auswahl)`
**Δ 0**. Ergebnis Kopf `y=0`, Auswahl `y=35`, beide ganz im Bild.
**Die Story braucht keine Zeile im Rumpf.**
*Beleg:* `sonde1-…-offscreen.txt`, Abschnitt G.

**F4 — Die Verbotszone wird nicht berührt, wenn die Bedingung strukturell
gestellt wird.** `head.row() == index.row() − 1` ist exakt gleichbedeutend mit
„erste Notiz ihrer Gruppe": `NoteListModel::buildRows()`
(`notelistmodel.cpp:23–29`) schreibt einen Kopf und **unmittelbar danach** die
erste Notiz — es gibt keine andere Zeilenfolge. Kein Pixel, kein Schwellwert,
keine Sichtbarkeitsfrage. Die Sichtbarkeitsprüfung selbst macht Qt:
`scrollTo(EnsureVisible)` bewegt **Δ 0**, wenn die Zeile ganz im Bild steht, und
genau **Δ −1**, wenn sie angeschnitten ist.
*Beleg:* `sonde1-…-offscreen.txt`, Abschnitt H.

**F4a — Die vorhandene Passbedingung in `:788` ist keine Sichtbarkeitsprüfung
und darf bleiben.** `selected.bottom() − heading.top()` ist **rollinvariant**:
bei den Rollwerten 11, 10 und 9 misst sie dreimal denselben Wert **106**. Sie
fragt, ob Kopf und Auswahl *zusammen* ins Bild passen, nicht ob eines von
beiden gerade sichtbar ist. Wer sie beim Aufräumen für den verbotenen
Schwellwert hält und entfernt, reißt
`leavesTheHeadOutsideWhereItCannotFitWithTheSelection` ein.
*Beleg:* `sonde1-…-offscreen.txt`, Abschnitte B, G und J.

**F5 — #59 macht die Sache weder billiger noch teurer; es räumt eine Falle
weg.** #59 (`084385c`) hat `changeEvent`, `reload`, `regroupList` und den Header
angefasst, **nicht `showNote`**. Wirkung auf diese Fläche: Vor #59 lief bei
**jeder** Fensteraktivierung ein `regroupList()`, dessen `setCurrentIndex()` ein
`showNote()` mit ungültigem Vorgänger auslöste — dort ist
`previousGroup.has_value() == false`, der Kopf wurde also **immer** geholt. Ein
Prüflauf, der zwischendurch das Fenster aktiviert, hätte den Befund heilen
lassen, ohne dass etwas geheilt war. Seit #59 tritt das ohne Tageswechsel nicht
mehr ein. **Der Bau wird dadurch nicht kleiner, aber der Nachweis wird
belastbar.**

**F6 — Zeile `:792` gehört #71 und darf von #70 nicht angefasst werden.** Der
UI-Review S5 benennt das unbedingte `m_list->scrollTo(index, EnsureVisible)`
als Verdachtszeile von B2 (`bericht.md:246–247`). Wer bei #70 „gleich mit
aufräumt", baut #71 mit — und hebelt AK 3 aus, das die Ruhe genau dieses Pfades
zusichert.

**F7 — Der Bildläufer behauptet heute das Gegenteil dessen, was er zeigt.**
`tests/libraryshots.cpp:278–280` kommentiert Szene 7 mit *„The selection walks
up to the first note of ‚Letzte Woche'; its head comes along and the entry
stays whole"*. Nachgestellt und gemessen: Die Auswahl landet auf Zeile 10, ihr
Kopf „Letzte Woche" steht bei `y=−35` — **er kommt nicht mit**. Das Bild im
Repositorium
(`docs/scrum/reviews/sprint-05-s-verhalten/bilder/07-fall4-uebergang-beim-scrollen.png`)
und der frische Lauf bei `QT_SCALE_FACTOR=1.6` zeigen beide die Auswahl ohne
Überschrift. Szene 7 ist **der Fall dieser Story**, nicht ein Nachbarfall.
Gegen die Kandidatenfassung kommt der Kopf mit (`y=0`).
*Beleg:* `sonde1-…-offscreen.txt` Abschnitt K gegen
`messungen/sonde3-kandidat-gegen-testauflage.txt` Abschnitt C;
Bild `messungen/libraryshots-1.6/07-fall4-uebergang-beim-scrollen.png`.
**Meldung an den PO, nicht geheilt:** Ein falscher Kommentar an einem
versionierten Bildbeleg ist ein Befund für sich; er fällt hier nur deshalb in
die Dateimenge, weil dieselbe Story ihn wahr macht.

**F8 — AK 6 unterstellt eine leere SPEC, und die SPEC ist nicht leer.**
`SPEC.md:513–518` trägt bereits: *„Springt die Auswahl **per Taste** über eine
Gruppengrenze, holt die Liste den Kopf der neuen Gruppe ins Bild."* Zu
**ergänzen** ist der Fall der ersten Notiz, nicht neu zu schreiben. **Ebenfalls
gemessen:** Die Passbedingung aus `:788` steht **nirgends** in der SPEC (Treffer
0) — eine DoD-4-Lücke, die älter ist als diese Story und im selben Absatz
liegt.
*Beleg:* `messungen/sonde4-spec-stand.txt`.

**F9 — Die Zeichnung sagt bereits, was gebaut werden soll.** Zeichnung 3b,
Fall 4 begründet mit *„die Auswahl steht nie ohne ihre Überschrift da"* — das
ist der Zielzustand von #70, nicht der heutige. Ihr Mechanismussatz nennt nur
den Grenzübertritt. Die Zeichnung ist **UX-Fläche**; ob ihr Text nachgezogen
wird, entscheidet nicht der Dev (Feld 6).

**F10 — Qt rollt vor uns, und das ist der Grund, warum es überhaupt geht.**
`showNote` hängt an `QItemSelectionModel::currentChanged`
(`librarywindow.cpp:286`), verdrahtet **nach** dem eigenen Slot der View. Beim
Tastendruck rollt Qt die Auswahl zuerst ins Bild (Rollwert 11 → 10), erst
danach läuft `showNote`. Deshalb hat das Kopfholen das letzte Wort. Wer diese
Reihenfolge umbaut — eigenes Auswahlmodell, aufgeschobener Aufruf —, verliert
sie: das ist genau, was Lesart 1 von #71 täte.

**F11 — Die Kandidatenfassung hält die volle Testauflage.** Dieselbe
unveränderte `tests/librarytest.cpp` gegen beide Stände:
**108 passed, 0 failed** hier wie dort. Darin die drei Zusicherungen, die
AK 2, AK 3 und AK 4 tragen.
*Beleg:* `messungen/sonde3-kandidat-gegen-testauflage.txt`, Abschnitte A und B.

**F12 — Der Preis ist genau eine Zeilenhöhe.** Die Liste bewegt sich im
Befundfall um **einen Rollschritt = 35 px** mehr als heute (`ScrollPerItem`,
Kopfzeile 35 px hoch). Das ist der im Issue bewusst getragene Preis, gemessen
und nicht geschätzt.
*Beleg:* `sonde1-…-offscreen.txt`, Abschnitt G und F.

---

## Feld 3 — AK-Urteil (Vorschlag; das Urteil fällt der Scrum Master)

**Vorschlag: ready ja, mit einer Auflage am Schnitt und einer Berichtigung an
AK 6.** Das Issue führt keine selbstdeklarierten offenen Punkte, die Kriterien
sind einzeln prüfbar, und zu jedem steht in Feld 4 ein Prüfmittel.

| AK | Prüfbar? | Begründung |
|---|---|---|
| **1** Aufwärts auf die erste Notiz → Kopf ganz im Bild (`y ≥ 0`), **am Rollwert** gemessen | **ja** | Der Nachweisweg steht als `sonden/kopfsonde.cpp` Abschnitt B fertig da. Das Kriterium gilt **ohne Zusatzbedingung**: Kopf und erste Notiz messen zusammen 106 px, das flachste erreichbare Fenster lässt 149 px Liste — die Passbedingung ist bei jeder Fenstergröße erfüllt (F4a, Abschnitt J). Es hätte eine Bedingung gebraucht, wenn das Kriterium eine beliebige Notiz meinte. |
| **2** Prüfsatz aus Sprint 3 bleibt grün | **ja** | `bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready` (`librarytest.cpp:1425`) — läuft gegen die Kandidatenfassung grün (F11), zusätzlich als Sondenfall I gemessen. |
| **3** Klickpfad aus #57 bleibt ruhig | **ja, mit Auflage** | `leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked` (`:1482`) plus Sondenfall E (Δ 0). **Auflage:** Das Kriterium ist nur so lange prüfbar, wie #71 den Klickpfad nicht gleichzeitig verschiebt. Siehe Kollisionsfläche. |
| **4** Innerhalb der Gruppe auf eine Nicht-erste Notiz → kein Zusatzrollen | **ja** | `staysPutWhileTheSelectionMovesWithinItsGroup` (`:1376`) plus Sondenfall D (Δ 0 vor und nach der Kandidatenfassung). |
| **5** Bildbeleg des Zustands nach dem Tastendruck + Rollwerte im Bericht | **ja** | Szene 7 von `libraryshots` ist bereits dieser Zustand (F7). B21 verlangt hier **kein** Sitzungsbild: Das Kriterium behauptet nichts über Hülle, Rundung, Kontur, Schatten, Dekoration oder Durchsichtigkeit, sondern über Geometrie — offscreen ist die zulässige Belegform, mit `QT_SCALE_FACTOR` auf der Skalierung des Kunden. |
| **6** SPEC 9 trägt die Regel nach | **ja, Prämisse falsch** | „heute sagt die SPEC dazu nichts" trifft nicht zu: `SPEC.md:513–518` trägt die Grenzübertrittsregel. Zu tun ist eine **Ergänzung**. Der PO sollte den Wortlaut des Kriteriums richtigstellen, damit niemand einen Absatz neu schreibt, der schon dasteht (melden, nicht heilen). |

---

## Feld 4 — Prüfmittel, und was ein Agent nicht prüfen kann

**Womit der Nachweis geführt wird**

| Gegenstand | Mittel |
|---|---|
| AK 1, 4 | Neue Zusicherungen in `tests/librarytest.cpp`, gebaut wie `staysPutWhileTheSelectionMovesWithinItsGroup`: **Rollwert vor und nach dem Tastendruck**, dazu `viewport()->rect().contains(visualRect(head))`. Das Endbild allein trennt die Fälle nicht — es sieht in beiden richtig aus (Kommentar `:1502–1504`). |
| AK 2, 3 | Die bestehenden Zusicherungen `:1425` und `:1482`, unverändert. Nachweis ist der grüne Gesamtlauf `ctest --test-dir build -R librarytest`. |
| AK 5 | `cmake --build build --target libraryshots` **frisch**, dann `QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.6 ./build/bin/libraryshots <ziel>`, Bild `07-fall4-uebergang-beim-scrollen.png` nach `docs/scrum/reviews/`. Die Rollwerte kommen aus dem Testlauf, nicht aus dem Bild. |
| AK 6 | `SPEC.md:513–518`, ergänzt; Gegenprobe `grep -n "Springt die Auswahl" SPEC.md`. |
| Rückfall | `sonden/kopfsonde.cpp` dieser Vorprüfung, gegen den fertigen Stand gebaut — sie misst alle vier Bewegungen in einem Lauf. |

**Was ein Agent an dieser Story nicht prüfen kann**

1. **Ob die 35 px im täglichen Gebrauch stören.** Die Story kauft Lesbarkeit
   mit Bewegung. Ob der Handel aufgeht, hat in Sprint 3 und in Sprint 5 jeweils
   erst der Kunde am laufenden Fenster gesagt — beide Male gegen die vorherige
   Messlage. Ein Agent kann den Preis messen, nicht bewerten.
2. **Den Fensterwechsel.** Der #59-Pfad (`changeEvent`) ist von dieser Story
   nicht betroffen, aber wer ihn im selben Lauf mitprüfen will, kann sich unter
   Wayland den Fokus nicht selbst zurückholen; `activateWindow()` genügt nicht.
   Der tragfähige Weg ist, das obenauf liegende Fenster zu **schließen**.
   Innerhalb der Testauflage ist der Fall über `QEvent::ActivationChange`
   ohnehin erreichbar (`librarytest.cpp:1835`) — das ist die Grenze eines
   *Sicht*laufs, nicht der Zusicherung.
3. **Ob Zeichnung 3b nachgezogen werden muss.** UX-Fläche; siehe Feld 6.

---

## Feld 5 — Größenklasse: **`size:s`**

**Begründung, am Code vermessen:**

- **Eine Datei, eine Stelle, +6/−1 Zeilen** — die Kandidatenfassung liegt als
  `flicken/a-70.patch` vor und ist vollständig gemessen: sie heilt den Befund
  (F1 → Kopf bei `y=0`) und hält die volle Testauflage (F11, 108/108).
- **Kein neuer Prüfweg.** Die vier Fälle werden mit den Mitteln geprüft, die
  seit Sprint 3 in `librarytest.cpp` stehen; der Bildbeleg mit einer Szene, die
  der Bildläufer bereits zeichnet. Nichts ist zu erfinden.
- **Kein Build, keine Bibliothek, keine Kopfdatei.**
- Was dazukommt, ist Fleißarbeit mit bekanntem Muster: zwei bis drei
  Zusicherungen, ein SPEC-Satz, ein Bild.

**Was die Klasse nicht abdeckt, ausdrücklich:** `size:s` gilt für #70 **allein**.
Wird #70 mit #71 zu einem Strang zusammengelegt — was ich empfehle —, ist das
Paket zu bewerten, nicht diese Story; #71 trägt eine offene Produktentscheidung
und einen zweiten Befundpfad. Die Klasse dieses Berichts ist kein Freibrief für
das Paket.

---

## Feld 6 — Offene Fragen

**An den PO**

1. **Schnitt #70/#71.** Ein Strang für beide (Reihenfolge #70 → #71) oder
   verschiedene Sprints? Die Messung sagt: nicht nebeneinander (Feld 1). Die
   Entscheidung ist Ihre.
2. **AK 6 richtigstellen.** Der Satz „heute sagt die SPEC dazu nichts" ist
   falsch (F8). Bitte im Issue korrigieren, bevor der Strang gezogen wird —
   sonst schreibt jemand einen Absatz neu, der schon dasteht.
3. **Die veraltete SPEC-Angabe im Issue.** Das Issue beruft sich auf
   „Zeichnung 3b, `SPEC.md:405–409`"; dort steht heute Abschnitt 7.3
   (Themen-Clustering). Der gemeinte Zeitstempel-Satz liegt bei
   `SPEC.md:499–503`. Kosmetisch, aber die Begründung der Story hängt daran.
4. **Falscher Kommentar an Szene 7 des Bildläufers** (F7) — gemeldet, nicht
   geheilt. Er verschwindet, wenn #70 gebaut wird; wird #70 **nicht** gezogen,
   bleibt eine versionierte Behauptung stehen, die ihr eigenes Bild widerlegt.
5. **Passbedingung fehlt in der SPEC** (F8, zweiter Teil). `librarywindow.cpp:788`
   und `leavesTheHeadOutsideWhereItCannotFitWithTheSelection` sichern eine
   Bedingung zu, die in SPEC 9 nicht steht — eine DoD-4-Lücke aus Sprint 3. Sie
   liegt in demselben Absatz, den AK 6 ohnehin anfasst. Mitnehmen oder eigenes
   Issue?

**An UX (über den PO)**

6. **Zeichnung 3b, Fall 4.** Ihr Begründungssatz beschreibt bereits den
   Zielzustand („die Auswahl steht nie ohne ihre Überschrift da"), ihr
   Mechanismussatz nur den Grenzübertritt. Wird der Mechanismussatz nachgezogen,
   oder gilt die Zeichnung als Absicht und die SPEC als Mechanik?

**An den Kunden — keine.** Die Produktfrage dieser Story ist am 04.08.2026
entschieden, samt Preis.
