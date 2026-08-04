# Vorprüfung #71 — Messung Bearbeiter A (`denkzettel-dev`)

**Gegenstand:** Issue #71, „Bibliothek: Klick auf angeschnittene Zeile wählt die
Nachbarzeile (UI-Review S5, B2)" · **Datum:** 04.08.2026, Ganymed ·
**Quellstand:** `main` @ `6acc87e` · **Belege:** `messungen/`, Sonde in
`sonden/klicksonde.cpp`, wiederholbar über
`bash docs/scrum/vorberichte/71-klick-nachbarzeile/pruefen.sh`

Dieser Bericht trägt die Felder **1, 2, 4 und 5**. **Feld 3 (Ready-Urteil)
fällt der Scrum Master**, Feld 6 steht als „Offene Fragen" am Ende.

**Stand der Werkzeuge** (B17): qt6-base 6.11.1, Plasma 6.7.3, Sitzung Wayland.
Gemessen wurde offscreen **und** in der angemeldeten Sitzung; die Ausgaben
unterscheiden sich in **einer** Zeile, dem Plattformnamen
(`diff messungen/klicksonde-offscreen.txt messungen/klicksonde-wayland.txt`).

---

## Die entscheidende Messfrage vorweg: nicht dieselbe Ursache wie #57

**#57 war der Kopf-Vorlauf, #71 ist das Nachrücken zur Auswahl selbst.** Beide
sitzen in `showNote()`, aber in verschiedenen Zeilen, und die Heilung von #57
kann diesen Fall bauartbedingt nicht erreichen:

| | #57 (geheilt, Sprint 5) | #71 (offen) |
|---|---|---|
| Zeile | `librarywindow.cpp:789` — `scrollTo(head)` | `librarywindow.cpp:792` — `scrollTo(index)` |
| Bedingung | vier Bedingungen, darunter `!m_selectionFollowsAPress` | **keine** — läuft immer |
| Auslöser | Auswahl überschreitet eine Gruppengrenze | jede Auswahländerung |
| Größe des Sprungs | bis 387 px | gemessen 72 px, in zwei Fällen 99 bzw. 107 px |

**Gemessen, nicht geschlossen** (Abschnitt C der Sonde): Der Fehler tritt
**auch dann auf, wenn keine Gruppengrenze überschritten wird** — dann käme der
Kopf-Vorlauf selbst ohne den Mausdruck-Merker aus #57 gar nicht in Frage.
Vorauswahl Zeile 13, Ziel Zeile 14, dieselbe Gruppe: aktuelle Zeile 14,
markiert 15, Versatz −72 px. Der Merker aus #57 wirkt hier also nicht, weil
die Zeile, die er schützt, gar nicht die schuldige ist.

**Die Ursache ist positiv belegt, nicht nur plausibel** (Abschnitte E und F):
An einem **blanken** `QListView` mit demselben Modell und demselben Delegate,
ohne `LibraryWindow`, tritt der Fehler **nicht** auf (Auswahl 14, Versatz
0 px). Wird demselben blanken View die eine Verbindung
`currentChanged → scrollTo(index, EnsureVisible)` gegeben, tritt er **auf**
(aktuell 14, markiert 15, Versatz −72 px) — Zeile für Zeile dasselbe Bild wie
im `LibraryWindow`. Damit ist die Ursache nicht erschlossen, sondern
ein- und ausgeschaltet worden.

**Der Mechanismus in einem Satz:** `QAbstractItemView::mousePressEvent` setzt
erst die aktuelle Zeile und wählt **danach** über ein Rechteck aus
(`setSelection`); der `scrollTo` läuft synchron aus `currentChanged` dazwischen
und verschiebt die Zeilen unter diesem Rechteck. Der Detailbereich hängt an der
**aktuellen** Zeile (richtig), die Markierung an der **verschobenen**
(falsch). Qt schaltet für genau diesen Zweck während des Mausdrucks sein
eigenes Autoscrollen ab — ein ausdrücklicher `scrollTo` umgeht diese Sperre.

---

## Feld 1 — Dateimenge (am Code vermessen, Notation nach B13)

| | **#71** — Klick auf die angeschnittene Zeile |
|---|---|
| **Issue** | **#71** (`epic:M2`, `typ:bug`) |
| **Zweig** | `story/71-klick-nachbarzeile` |
| **Quellen & Tests** | `src/ui/librarywindow.cpp` — **eine Funktion**, `showNote()` `:692–799`, darin die Zeile `:792`. Je nach Lesart (unten) zusätzlich **entweder** nichts weiter (Lesart 1 und 2) **oder** der Listenaufbau `:183–188` (Lesart 3). `src/ui/librarywindow.h` — **nur wenn** eine Lesart ein neues Feld braucht; die drei gemessenen Wege brauchen keines, `m_selectionFollowsAPress` steht bereits.<br>`tests/librarytest.cpp` — ein neuer Test bei den #57-Nachbarn: Deklaration bei `:143`, Rumpf bei `:1558`. **Nur bei Lesart 3** zusätzlich die vier Aufbauten `:1406`, `:1459`, `:1532`, `:1595`, die den Rollwert als **Zeilennummer** setzen. |
| **Build** | **Nichts.** Keine neue Bibliothek, kein neues Ziel, kein neuer Läufer. |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-NN-s71-klick-nachbarzeile/` — neu anzulegen, mit eigenem `pruefen.sh`. **Wiederverwendbar, nicht neu zu erfinden:** `sonden/klicksonde.cpp` dieser Vorprüfung misst den Fall bereits über beide Plattformen, beide Rollmodi und alle Rollwerte; sie ist der fertige Bauplan des Vorher-Nachher-Belegs. Bildbeleg über `libraryshots` (DoD 3, siehe Feld 4). |
| **Fachliche Quellen** | **SPEC 9** (`:513–518`) — der Absatz „**Ein Mausklick tut das nicht**" spricht heute nur vom **Kopf-Vorlauf**. Dass auch das Nachrücken zur Auswahl selbst unter dem Mausdruck steht, ist eine **entdeckte Bedingung** und zieht die SPEC nach (DoD 4 in der Fassung nach B9). Wireframe 3b, Fall 4 — nur zu lesen, nicht zu ändern. |
| **Ausdrücklich nicht** | `src/capture/*`, `src/shell/*`, `src/store/*`, `src/ui/notelistdelegate.{h,cpp}` (**die Zeilenhöhe ist gemessen unbeteiligt**, siehe F4), `src/ui/notelistmodel.*`, `tests/capturetest.cpp`, `tests/captureshots.cpp`, `tests/editshots.cpp`, `tests/searchshots.cpp`, `wireframes/`, alle SPEC-Abschnitte außer 9, sowie Belegordner fremder Sprints. `tests/libraryshots.cpp` ist **nur bei Lesart 3** in der Menge (`:373` setzt den Rollwert auf 4). |

### Kollisionsfläche gegen #83 — **null, sie laufen nebeneinander**

| | #83 | #71 |
|---|---|---|
| Quellen | `src/capture/capturewindow.{h,cpp}` | `src/ui/librarywindow.cpp` |
| Tests | `tests/capturetest.cpp`, `tests/captureshots.cpp` | `tests/librarytest.cpp` (`tests/libraryshots.cpp` nur bei Lesart 3) |
| SPEC | 3.1, 3.2, 15, 16 | 9 |

**Kein gemeinsamer Pfad, keine gemeinsame Bibliothek** (`denkzettelcapture`
gegen `denkzettelui`, `src/CMakeLists.txt` unverändert), **kein gemeinsamer
Bildläufer** (`captureshots` gegen `libraryshots`), **kein gemeinsamer
SPEC-Abschnitt**. Der einzige geteilte Gegenstand ist `build/` der
Repositoriumswurzel und, am Sprint-Ende, `/usr` — beides taktet ohnehin der PO.

### Kollisionsfläche gegen #72 — **dieselben zwei Dateien, getrennte Bereiche**

#72 (Tooltips) und #71 fassen `src/ui/librarywindow.cpp` und
`tests/librarytest.cpp` an. Gemessen an den Zeilenbereichen:

| | #72 (Tooltips) | #71 | kleinster Abstand |
|---|---|---|---|
| `librarywindow.cpp` | Konstruktor: `:173` (Rückgängig-Aktion), `:361`, `:368` (die beiden Knöpfe) | `showNote()` `:792` | **317 Zeilen**, verschiedene Funktionen |
| `librarytest.cpp`, Slot-Liste | Anmeldung nahe `:191` (Bearbeiten-Block) | Anmeldung nahe `:143` (#57-Block) | **48 Zeilen** |
| `librarytest.cpp`, Rümpfe | im Bearbeiten-Teil ab `:3099` | bei `:1558` | über 1.500 Zeilen |

**Urteil: Sie können nebeneinander laufen.** Kein gemeinsames Feld, keine
gemeinsame Funktion, kein gemeinsamer Test; alle Abstände liegen weit über
Gits Mischbreite von drei Zeilen. **Eine Bedingung:** Lesart 3 (unten) zöge
`tests/libraryshots.cpp` und damit die **Bildreihe der Bibliothek** in die
Dateimenge. Auch dann kollidiert #72 nicht — es fasst die Bildreihe nicht an —,
aber die neun Bilder wären dann ein Beleg, den nur ein Strang gleichzeitig neu
erzeugen darf.

**Wichtiger als der Abstand:** #72 ändert Tooltips, #71 ändert das
Auswahlverhalten. Sie berühren dieselbe Ansicht, aber keine gemeinsame Aussage.

---

## Feld 2 — Gemessene Fallen (die Zeilen für den Spawn-Auftrag)

**F1 — Der Fehler tritt an *jedem* Rollwert auf, an dem eine Zeile
angeschnitten ist: 11 von 11.** Nicht ein Sonderfall, sondern der Regelfall
(Abschnitt I). *Beleg:* `messungen/klicksonde-offscreen.txt`, Abschnitt I.

**F2 — Es gibt zwei Fehlerbilder, und das Issue nennt nur eines.** Neben
„markiert die Nachbarzeile" (8 Fälle) steht **„markiert gar nichts"** (3
Fälle, Rollwerte 0, 9 und 10): Die Auswahl verschwindet, während der
Detailbereich die geklickte Notiz zeigt. Ursache ist, dass unter dem Zeiger
nach dem Rücken ein **Gruppenkopf** oder der Leerraum unter der Liste liegt —
beides ist nicht wählbar. Ein AK, das nur von der Nachbarzeile spricht, deckt
den zweiten Fall nicht ab. *Beleg:* Abschnitt I, Zeilen „markiert keine".

**F3 — Nur der **untere** Rand schneidet an, nie der obere.** Bei
`ScrollPerItem` setzt die Liste stets zeilenbündig ab: **0 von 12** Rollwerten
zeigen eine oben angeschnittene Zeile. Wer einen Testfall „oben
angeschnitten" bauen will, sucht umsonst — er existiert im heutigen Rollmodus
nicht. *Beleg:* Abschnitt D.

**F4 — Die Zeilenhöhe aus `NoteListDelegate::sizeHint()` ist die *Größe* des
Sprungs, nicht seine *Ursache*.** Gemessen: Notizzeile 72 px, Gruppenkopf
27 px; der Versatz beträgt −72 px (eine Notizzeile), −99 px (Notiz + Kopf) und
−107 px am Listenende. Der Delegate gehört deshalb **nicht** in die
Dateimenge: An einem blanken View mit demselben Delegate tritt der Fehler
nicht auf (Abschnitt E). *Beleg:* Abschnitte A, I, E.

**F5 — `indexAt()` ist unschuldig.** Vor dem Druck liefert es die geklickte
Zeile richtig (14), nach dem Druck die Nachbarzeile (15) — weil sich die
Zeilen unter dem stehenden Zeiger bewegt haben. Die aktuelle Zeile, die aus
`indexAt()` stammt, ist **immer korrekt**; falsch ist allein die Markierung,
die Qt danach über ein Rechteck setzt. Wer bei `indexAt()` sucht, sucht
umsonst. *Beleg:* Abschnitte B und C, Zeile „Zeile unter dem Zeiger".

**F6 — Offscreen und Wayland liefern Zeile für Zeile dasselbe.** Der `diff`
über die vollständigen Ausgaben hat **eine** Zeile Unterschied: den
Plattformnamen. Der Fall ist offscreen vollständig messbar — anders als bei
#83, wo Compositor-Eigenschaften im Spiel waren (B21 betrifft Hülle, Rundung,
Kontur, Schatten, Dekoration; hier geht es um Geometrie und Farbrolle).
*Beleg:* `pruefen.sh`, Ausgabe des `diff`.

**F7 — Der naheliegende Prüfaufbau misst nichts, und das ist gemessen.**
Klickt eine Messschleife nacheinander auf mehrere Rollwerte, ohne die
**aktuelle Zeile vorher wegzulegen**, dann trifft sie zweimal dieselbe Zeile,
`currentChanged` feuert nicht, `scrollTo` läuft nicht — und der Fall meldet
„stimmt". In der ersten Fassung dieser Sonde sahen so **2 von 11** Fällen wie
ein Freispruch aus; nach dem Einbau eines `setCurrentIndex()` vor jedem Klick
sind es **11 von 11**. Das ist genau die Bauart, die `CLAUDE.md` als
Nicht-Test bezeichnet. **Der neue Unit-Test muss die Vorauswahl ausdrücklich
setzen und zusichern, dass sie nicht das Ziel ist.** *Beleg:* die Korrektur
ist im Quelltext der Sonde kommentiert (Abschnitte I und H).

**F8 — Der Mausdruck-Merker aus #57 wird nur von einer Taste *auf der Liste*
gelöscht** (`librarywindow.cpp:530–532`). Er bleibt nach einem Listenklick
stehen, wenn der Nutzer als nächstes ins Suchfeld tippt oder einen Knopf
drückt. Heute reicht das nur bis zum Kopf-Vorlauf; **eine Lesart, die den
Merker auch auf das Nachrücken zur Auswahl anwendet (Lesart 2), weitet diese
Klebrigkeit auf jede programmatische Auswahländerung aus** — Löschen,
Rückgängig, Neuladen. *Das ist am Code gelesen, nicht gemessen* (siehe „Was
ich nicht klären konnte"); wer Lesart 2 baut, misst es nach.

**F9 — Die Skalierung ändert an dieser Messung nichts.** `QT_SCALE_FACTOR=1.6`
(die Sitzungsskalierung des Kunden, #83 F3) liefert offscreen eine **Zeichen
für Zeichen identische** Ausgabe: Der Fall lebt in logischen Bildpunkten.
Für den **Bildbeleg** gilt die Skalierungspflicht aus DoD 3 trotzdem — sie
betrifft das Bild, nicht die Zahl. *Beleg:*
`diff messungen/klicksonde-offscreen.txt messungen/klicksonde-offscreen-skala-1.6.txt`
ist leer.

---

## Die drei Lesarten, jede gemessen

Der UI-Review S5 nannte zwei Lesarten; die Messung hat eine dritte
hinzugefügt. Alle drei sind an der Sonde durchgespielt.

| | Weg | AK 1 (Auswahl = Anzeige) | AK 2 (Bild) | Kosten | Beleg |
|---|---|---|---|---|---|
| **Lesart 1** | `scrollTo` **nachreichen** (`Qt::QueuedConnection` oder `QTimer::singleShot(0)`) | **erfüllt** (14/14) | **nicht erfüllt** — das Bild rückt weiterhin um 72 px, und die Zeile unter dem Zeiger wechselt von 14 auf 15 | eine Zeile | Abschnitt G |
| **Lesart 2** | beim Mausdruck **gar nicht** nachrücken — konsequent zu #57 | **erfüllt** (14/14) | **erfüllt** — Versatz 0 px, Zeile unter dem Zeiger bleibt 14 | eine Zeile; Nebenwirkung F8 | Abschnitt E |
| **Lesart 3** | `setVerticalScrollMode(ScrollPerPixel)` | **erfüllt**, 0 von 32 Fällen gehen auseinander | **erfüllt** — die Zeile rückt nur um den verdeckten Betrag (23–70 px) und bleibt dabei unter dem Zeiger; zusätzlich wird sie ganz sichtbar | vier Testaufbauten und eine Bildszene, siehe unten | Abschnitt H |

**Warum Lesart 3 den Fehler prinzipiell ausschließt und nicht nur zufällig:**
Bei `ScrollPerPixel` ist der Versatz genau der verdeckte Teil der Zeile. Der
Zeiger steht auf dem **sichtbaren** Teil, also näher am oberen Rand der Zeile
als der sichtbare Teil hoch ist — nach dem Rücken um den verdeckten Betrag
liegt er darum weiterhin innerhalb derselben Zeile. Bei `ScrollPerItem` ist der
Versatz dagegen die volle Höhe der Zeile, die oben hinausrollt (72 px), also
immer mindestens eine ganze Zeile. Die Sonde prüft das am oberen **und** am
unteren Rand des sichtbaren Streifens, über 32 Fälle mit Streifenbreiten von
2 bis 68 px — keiner geht auseinander.

**Was Lesart 3 kostet, gemessen** (Abschnitt J): Dieselbe Zahl bedeutet in
beiden Rollmodi Verschiedenes — der Rollbereich ist `0..11` gegen `0..662`.
`setValue(8)` stellt Zeile 9 einmal bei `y=72` **ins Bild**, einmal bei
`y=595` **außerhalb**. Die vier Testaufbauten in `tests/librarytest.cpp`
(`:1406`, `:1459`, `:1532`, `:1595`) setzen den Rollwert auf eine
**Zeilennummer** und wären neu zu rechnen; dazu `tests/libraryshots.cpp:373`
und die davon abhängige Bildszene `09-ruhiges-bild-innerhalb-der-gruppe.png`.
Alle vier Aufbauten sichern ihre Voraussetzung anschließend zu
(`QVERIFY(!viewport.intersects(visualRect(head)))`), zwei davon mit der
Begründung „der Fall verlangt ihn außerhalb" (`:1407`, `:1460`) — sie würden
also **laut** fallen, nicht still grün bleiben. Das ist die gute Nachricht an dieser Stelle; die Arbeit bleibt
trotzdem.

**Dev-Empfehlung, ohne die Entscheidung zu nehmen:** **Lesart 2**, weil sie
beide Akzeptanzkriterien wörtlich erfüllt, eine Zeile groß ist und dieselbe
Regel weiterschreibt, die SPEC 9 schon trägt („Wer zeigt, erwartet, dass die
gezeigte Stelle bleibt"). Lesart 3 ist die schönere Bedienung — die
angeschnittene Zeile wird ganz sichtbar, **ohne** unter dem Zeiger
wegzurutschen —, kostet aber die Testumstellung und ändert nebenbei das
Verhalten des Mausrads. Das ist eine Produktentscheidung, keine Messfrage.
**Lesart 1 erfüllt AK 2 nicht** und sollte ausscheiden.

---

## Feld 4 — Prüfmittel je Akzeptanzkriterium, und was ein Agent nicht kann

| AK | Prüfmittel | Grenze |
|---|---|---|
| **1** — Klick auf eine angeschnittene Zeile wählt genau diese Zeile; Auswahl und Detailbereich stimmen überein | **Unit-Test in `tests/librarytest.cpp`, offscreen** — `QTest::mouseClick` auf den sichtbaren Streifen, dann `QCOMPARE(list->currentIndex(), target)` **und** `QCOMPARE(list->selectionModel()->selectedIndexes(), {target})` **und** der sichtbare Text des Detailbereichs. Der Fall ist offscreen vollständig reproduzierbar (F6). Drei Zusicherungen zum Aufbau sind Pflicht (F7): die Zielzeile ist angeschnitten, der Klickpunkt liegt im sichtbaren Streifen, die Vorauswahl ist **nicht** die Zielzeile | **Der Agent klickt nicht wirklich.** Sowohl `QTest::mouseClick` als auch die Sonde stellen die Ereignisse selbst zu; eine echte Zeigerbewegung durch den Compositor ist nicht auslösbar. Gemessen ist, dass der Weg durch `QAbstractItemView::mousePressEvent` derselbe ist und offscreen wie unter Wayland dasselbe liefert — **dass ein echter Mausklick des Kunden denselben Weg nimmt, ist geschlossen, nicht gemessen** |
| **2** — das Nachrücken ist weg oder die geklickte Zeile bleibt unter dem Zeiger | **Der Weg ist der Prüfgegenstand, nicht das Ziel** (B3): gemessen wird `visualRect(target).y()` **vor** und **nach** dem Klick sowie `indexAt(punkt)` vor und nach. Zwei Zahlen, kein Bild. Die Sonde druckt beide bereits | Welche der beiden Alternativen gelten soll, entscheidet die Lesart — das AK lässt beide zu, und ein Test kann nur eine prüfen. **Das ist vor dem Ziehen zu entscheiden** (Feld 6) |
| **beide, Zustandsbeleg** | **Bild je Zustand** (DoD 3): der Zustand „angeschnittene Zeile geklickt" mit Markierung und Detailbereich im selben Bild, über `libraryshots` mit `QT_QPA_PLATFORMTHEME=kde` und `QT_SCALE_FACTOR` auf der Kundenskalierung. Vorher **`cmake --build build --target libraryshots`** — ein veralteter Läufer schreibt ein altes Bild mit frischem Zeitstempel | **B21 verlangt hier kein Sitzungsbild:** Kein Kriterium spricht über Hülle, Rundung, Kontur, Schatten, Dekoration oder Durchsichtigkeit. Markierungsfarbe und Textsatz sind Farbrolle und Geometrie — was offscreen belegbar ist |
| **Rückschrittsschutz** | Der vorhandene #57-Test `leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked` (`:1482`) muss grün bleiben; er prüft den benachbarten Fall der **ganz sichtbaren** Zeile | keine |
| **Mutationsprobe** | Heilung entfernen, den neuen Test rot werden lassen — die Sonde liefert die Vorher-Zahlen dafür bereits (`messungen/klicksonde-*.txt`) | keine |

**Was ein Agent an dieser Story grundsätzlich nicht prüfen kann:**

1. **Einen echten Mausklick.** Alle Belege beruhen auf zugestellten Ereignissen
   (siehe oben). Ein Sichtlauf durch den Kunden ist der einzige Weg, das zu
   schließen — er ist billig und sollte in die Abnahme.
2. **Ob das Ergebnis sich ruhig anfühlt.** Ob eine angeschnittene Zeile stehen
   bleiben *soll* oder ganz ins Bild rücken *soll*, ist Geschmack des Kunden;
   die Messung sagt nur, was jede Lesart tut.
3. **Ob die Klebrigkeit des #57-Merkers (F8) im Betrieb stört.** Nicht
   gemessen, siehe unten.

---

## Feld 5 — Größenklasse: **`size:s`** (bedingt)

Bedeutung laut `PROZESS.md`: *„läuft nebenher — wenige Dateien, kein neuer
Prüfweg"*. Beides trifft für die Lesarten 1 und 2 zu:

- **Wenige Dateien:** zwei — `src/ui/librarywindow.cpp` (eine Zeile in einer
  Funktion) und `tests/librarytest.cpp` (ein Test). Keine Kopfdatei, kein
  Build, kein neues Ziel.
- **Kein neuer Prüfweg:** Der Klick-Test steht bereits als Muster in derselben
  Datei (`:1541`, `QTest::mouseClick` auf den Viewport), die Sonde ist gebaut,
  der Bildläufer existiert.
- **Keine Kollision** mit #83 (Feld 1), also neben einer `size:l` zulässig.

**Die Bedingung, unter der es `size:m` wird — und damit aus Sprint 7 fällt:**
Entscheidet der Kunde sich für **Lesart 3** (`ScrollPerPixel`), kommen vier
Testaufbauten, eine Bildszene und ein geändertes Mausradverhalten dazu; die
Story fasst dann `tests/libraryshots.cpp` an und erzeugt neun Bilder neu. Das
ist kein Nebenher mehr. **Neben `size:l` (#83) steht nur `size:s` — Lesart 3
schließt #71 für Sprint 7 aus.** Deshalb gehört die Lesart **vor** den Schnitt
entschieden, nicht in ihn hinein.

**Wofür nicht `size:m` in jedem Fall:** Der Fehler ist vollständig vermessen,
die Ursache ein- und ausgeschaltet, das Prüfmittel gebaut und die Heilung eine
Zeile. Was an dieser Story Arbeit war, ist mit dieser Vorprüfung getan.

---

## Feld 6 — Offene Fragen an PO oder Kunde

1. **Welche Lesart?** Das ist die Frage, an der die Größenklasse hängt (Feld 5)
   und damit die Sprint-Fähigkeit. Lesart 2 empfehle ich; Lesart 3 ist die
   schönere Bedienung und passt nicht neben #83.
2. **AK 2 lässt zwei Ergebnisse zu und ist deshalb nicht einzeln prüfbar.**
   „Entweder weg oder rückt so, dass die Zeile unter dem Zeiger bleibt" — ein
   Test prüft eines von beiden. Vorschlag, nachdem die Lesart entschieden ist:
   *„Nach dem Klick steht die geklickte Zeile an derselben Stelle wie davor
   (Versatz 0 px)"* für Lesart 2, oder *„…der Zeiger liegt nach dem Klick
   weiterhin über der geklickten Zeile"* für Lesart 3.
3. **AK 1 deckt den zweiten gemessenen Fehlerfall nicht ab** (F2): In 3 von 11
   Fällen ist danach **gar nichts** markiert. Vorschlag, AK 1 zu ergänzen:
   *„…und es bleibt genau eine Zeile markiert."*
4. **Gehört die entdeckte Bedingung in SPEC 9?** Der Absatz `:513–518` sagt
   heute, ein Mausklick hole den Gruppenkopf nicht ins Bild. Dass er auch das
   Nachrücken zur Auswahl selbst unterbindet (bei Lesart 2), ist eine neue
   Aussage. *Dev-Empfehlung: ja* — DoD 4 verlangt es, und ohne den Satz baut
   der nächste Strang die Zeile wieder ein.
5. **Soll der Kunde einmal selbst klicken?** Der einzige Beleg, den kein Agent
   führen kann (Feld 4, Grenze 1). Zwei Minuten in der Abnahme.

---

## Was ich **nicht** klären konnte

- **Ob ein echter Mausklick denselben Weg nimmt** wie ein zugestelltes
  Ereignis. Offscreen und Wayland liefern identisch, aber beide über
  `sendEvent`.
- **Die Klebrigkeit des #57-Merkers (F8)** — am Code gelesen
  (`librarywindow.cpp:517–535`), nicht gemessen. Ein beobachtbarer Nachweis
  ließ sich nicht bauen: Jeder Weg, auf dem der Merker nach einem Klick
  stehenbliebe, führt über ein Neuladen, das die Liste ohnehin an den Anfang
  stellt — dort ist ein unterbliebener `scrollTo` nicht von einem
  ausgeführten zu unterscheiden. Wer Lesart 2 baut, prüft es an der echten
  Klasse nach.
- **Warum bei Rollwert 9 und 10 gar nichts markiert wird** statt der
  Nachbarzeile. Gemessen ist das *Ob*; dass es am Listenende und am
  Gruppenkopf liegt, ist erschlossen.
- **Ob `ScrollPerPixel` das Mausrad angenehmer oder unangenehmer macht.**
  Nicht gemessen, und keine Frage für eine Sonde.
- **Der Aufwand in Zeit.** Wird in diesem Projekt nicht erhoben.

---

## Befehle, mit denen ich gemessen habe

```
bash docs/scrum/vorberichte/71-klick-nachbarzeile/pruefen.sh     # alle Messungen
QT_QPA_PLATFORM=offscreen QT_SCALE_FACTOR=1.6 build/sonden/klicksonde   # F9
gh issue view 71 ; gh issue view 72 ; gh issue view 83
grep -n "^void LibraryWindow::" src/ui/librarywindow.cpp
grep -n "scrollTo\|eventFilter\|selectionFollowsAPress" src/ui/librarywindow.cpp
grep -n "verticalScrollBar" tests/librarytest.cpp tests/libraryshots.cpp
sed -n 500,520p SPEC.md
```

**Nicht getan:** nichts committet, nichts gepusht, nichts nach `/usr`
installiert, keine Zeile unter `src/`, `tests/`, `SPEC.md` oder `wireframes/`
geändert, den Fehler **nicht geheilt**. Der Bauplatz liegt unter
`docs/scrum/vorberichte/71-klick-nachbarzeile/build/` und ist von `.gitignore`
gedeckt; `build/` der Repositoriumswurzel wurde nicht angefasst. Die Sonde
arbeitet auf einer eigenen temporären Datenbank und mit eigenem
`XDG_CONFIG_HOME`, die Notizen und die Fenstergeometrie des Kunden sind
unberührt.
