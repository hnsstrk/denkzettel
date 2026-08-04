# Vorprüfung #71 — Messung B (Scrum Master)

Gemessen am 2026-08-04, 20:30–20:38 CEST, Stand `6acc87e`. Unabhängig von
Bearbeiter A; dessen Messung war zum Zeitpunkt dieser Arbeit nicht gelesen.

Messbeleg: `messung-b-bestand.txt` in diesem Ordner.

## Feld 1 — Dateimenge (so weit ohne A gemessen)

| | **#71** |
|---|---|
| **Quellen und Tests** | `src/ui/librarywindow.cpp` — der Block `:753–795` (`showNote()`, Kopf-Fetch und das unbedingte `scrollTo`), dazu `eventFilter()` `:517–535` (die Marke `m_selectionFollowsAPress`).<br>`tests/librarytest.cpp` — die Slot-Deklarationsliste (Klassenkopf bis `:297`) und ein bis zwei neue Fälle im Muster von `:1482–1557`. |
| **Belege und Prüfmittel** | Rollwertmessung im Test; ein Bildbeleg ist **nicht** nötig (siehe Feld 4). |
| **Fachliche Quellen** | `SPEC.md` Abschnitt 9 — trägt heute nichts zur Rollstelle beim Klick; ob eine Bedingung entdeckt wird, entscheidet sich beim Bauen (DoD 4/B9). |
| **Ausdrücklich nicht** | `src/ui/notelistmodel.*`, `src/ui/notelistdelegate.*`, `src/capture/`, `tests/libraryshots.cpp`. |

## Feld 2 — Fallen (Teilmessung)

1. **Die Verdachtszeile des Issues stimmt nicht mehr.** Das Issue nennt „ein
   unbedingtes `scrollTo` in `librarywindow.cpp:783`". Zeile `:783` trägt heute
   `const bool crossesAGroupBoundary = …`. Das unbedingte `scrollTo` steht in
   **`:792`**; `:789` trägt das bedingte für den Gruppenkopf. Der Befund stammt
   aus Sprint 5, seither sind #57 und #59 durch diese Fläche gegangen.
2. **Die Heilung darf den Tastaturpfad nicht mitnehmen.** `:785–791` holt den
   Gruppenkopf ins Bild, und zwar nur bei Tastatur (`!m_selectionFollowsAPress`).
   Wer das `scrollTo` in `:792` an eine Bedingung hängt, verändert **beide**
   Pfade. Vier bestehende Zusicherungen stehen dagegen, darunter
   `leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked`
   (`:1482–1557`) und `keepsTheHeadFetchAfterAClickThatSelectedNothing`
   (`:1559 ff.`).
3. **Der Nachbarzeilen-Teil ist im Testaufbau möglicherweise gar nicht
   auslösbar.** `QTest::mouseClick` sendet Press und Release **ohne**
   dazwischenliegendes Move-Ereignis (so nutzt es der bestehende Test in
   `:1541`). Rückt die Liste zwischen Press und Release, verschiebt sich die
   Zeile unter dem stehenden Zeiger — ohne Move-Ereignis sieht `QListView` das
   aber nicht, und die Auswahl bliebe korrekt. Der Befund „wählt die
   Nachbarzeile" hängt dann an einer Mausbewegung, die der Kunde macht und der
   Test nicht. **Das ist die entscheidende Messung dieser Story, und sie steht
   aus** (Feld 4). Der andere Teil des Befunds — die Liste rückt um 72 px — ist
   im vorhandenen Muster ohne Weiteres messbar.

## Feld 3 — AK-Urteil: **ready = nein**

**Der Kopf sagt es selbst:** „Akzeptanzkriterien (Entwurf, Schätzung im
Planning)". Nach dem DoR-Zusatz vom 04.08.2026 ist ein Issue mit
selbstdeklarierten offenen Punkten nicht ready. Der Zusatz greift hier nicht
formal, sondern trifft die Sache — die beiden Kriterien haben inhaltlich drei
Lücken:

1. **Kein Kriterium schützt den Tastaturpfad.** Die Heilung liegt drei Zeilen
   neben dem Kopf-Fetch aus #57/#59 (Falle 2). Fehlt der Satz, ist eine
   Regression zulässig, die niemand als Bruch eines Kriteriums bemerken würde.
   Fehlender Satz, prüfbar: *„Der Kopf-Fetch auf dem Tastaturpfad bleibt
   unverändert; die bestehenden Zusicherungen bleiben grün."*
2. **AK 1 nennt kein Prüfmittel für die Nachbarzeile.** Nach Falle 3 ist offen,
   ob dieser Teil offscreen überhaupt reproduzierbar ist. Ist er es nicht,
   braucht die Story eine Sitzungsprobe — und dann ist sie eine andere Story
   (siehe Feld 5).
3. **Die Verdachtszeile ist veraltet** (Falle 1). Kein Ready-Blocker für sich,
   aber sie steht als Arbeitsanweisung im Issue und führt in die Irre.

**AK 2 ist in Ordnung** und ich vermerke das ausdrücklich: Die Oder-Form
(„entweder weg oder rückt so, dass die geklickte Zeile unter dem Zeiger bleibt")
sieht unbestimmt aus, ist es aber nicht — beide Zweige sind einzeln messbar
(Rollwert unverändert bzw. `visualRect(target).y()` unverändert), und die Wahl
zwischen ihnen ist eine technische Entscheidung im Story-Rahmen.

**Behebung (PO):** die zwei fehlenden Sätze ergänzen, die Zeilenangabe auf
`:792` nachziehen, „(Entwurf)" streichen.

## Feld 4 — Prüfmittel, und was ein Agent nicht prüfen kann

**Vorhanden, nichts Neues zu bauen:** `QTest::mouseClick(list->viewport(), …,
list->visualRect(target).center())` mit `verticalScrollBar()->value()` und
`visualRect(…).y()` vorher/nachher — genau das Muster von `:1538–1548`. Für eine
**angeschnittene** Zeile ist nur der Klickpunkt anders zu wählen (ein Punkt im
sichtbaren Teil statt `center()`).

**Grenze, ausgesprochen:** Ob ein Klick ohne Mausbewegung die Auswahl auf die
Nachbarzeile wandern lässt, ist nicht abgeleitet, sondern zu messen. Ergibt die
Messung, dass es ein Move-Ereignis braucht, gibt es zwei Wege — ein Test, der
Press, Move und Release einzeln sendet (bleibt im vorhandenen Prüfweg), oder
eine Sitzungsprobe (neuer Prüfweg). **Der erste Weg ist zu versuchen, bevor der
zweite beauftragt wird.**

Ein Bildbeleg nach B21 ist **nicht** einschlägig: Es geht um Rollposition und
Auswahl, nicht um Hülle, Rundung, Kontur, Schatten oder Dekoration.

## Feld 5 — Größenklasse: **`size:s`** — unter einer benannten Bedingung

Wenige Dateien (zwei), kein neuer Prüfweg (Feld 4), keine neue Abhängigkeit,
keine SPEC-Festlegung im Weg, kein Compositor. Die Änderung selbst ist eine
Bedingung an `:792` samt Testfall.

**Die Bedingung, unter der das gilt:** dass sich der Befund im vorhandenen
Testaufbau reproduzieren lässt (Falle 3). Braucht der Nachbarzeilen-Teil eine
Sitzungsprobe, entsteht ein neuer Prüfweg und die Story ist **`size:m`** —
dann passt sie nach der Klassenregel nicht mehr neben #83. **Das ist die
Messung, die vor der Sprint-Freigabe zu machen ist**, nicht danach; sie kostet
einen Testlauf.

Ich stufe trotz dieser Unsicherheit auf `s` ein und nicht vorsorglich auf `m`,
weil der billigere Weg zuerst zu versuchen ist und ein Press-Move-Release-Test
im vorhandenen Muster bleibt. Wer aus Vorsicht auf `m` ginge, verlöre den
einzigen kleinen Kandidaten dieses Laufs auf eine Vermutung hin.

## Feld 6 — Offene Fragen

**An den PO:**

1. **#70 fehlt in der Kandidatenliste, und das ist der wichtigste Punkt dieser
   Messung.** #70 („Erste Notiz einer Gruppe holt ihren Tageskopf ins Bild,
   Tastaturpfad") arbeitet in **denselben Zeilen** `:785–792`. Mehr noch: **#70
   AK 3 verlangt ausdrücklich, dass der Klickpfad aus #57 unverändert ruhig
   bleibt** — das ist genau der Pfad, den #71 ändert. Die beiden sind nicht nur
   kollidierend, sie können einander widersprechen. Sie gehören in **einen**
   Strang oder in **verschiedene** Sprints; zwei parallele Stränge auf dieser
   Fläche sind ausgeschlossen. Das Issue selbst nennt #70 unter „Nachbarschaft",
   die Auswahl des PO hat es nicht aufgenommen.
2. **#71 und #72 in einem Strang.** Beide schreiben in `librarywindow.cpp` und
   in die Slot-Liste von `librarytest.cpp`. Der Textabstand ist groß (`:361`
   gegen `:792`), aber die Slot-Deklarationen liegen im selben Klassenkopf.
   Zwei Worktrees lohnen dafür nicht.

**Kundenentscheidung:** keine offen. Der Befund ist gemessen und als Bestand
ausgewiesen, keine Regression von #57.
