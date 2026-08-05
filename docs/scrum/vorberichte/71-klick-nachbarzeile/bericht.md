# Vorprüfbericht #71 — Klick auf angeschnittene Zeile

**Konsolidiert vom PO am 05.08.2026** aus `messung-a.md` (Bearbeiter A,
`denkzettel-dev`, Stand `6acc87e`), `messung-b.md` (Bearbeiter B, Scrum Master,
Stand `6acc87e`) und `reproduktion.md` (Nachmessung, Stand `581dacc`). Die
beiden Messungen entstanden unabhängig voneinander.

**Ergebnis: `size:s`, ready — nach Nachschärfung der Kriterien durch den PO und
nach der Reproduktionsmessung, die B ausdrücklich vor der Freigabe verlangt
hatte.**

---

## 1. Der Streitpunkt zwischen A und B — und wie er entschieden wurde

Beide kamen unabhängig auf `size:s`. **B stufte aber unter einem ausdrücklichen
Vorbehalt ein** und benannte die Messung, die vor der Freigabe fällig sei:

> „Der Nachbarzeilen-Teil ist im Testaufbau möglicherweise gar nicht auslösbar.
> `QTest::mouseClick` sendet Press und Release **ohne** dazwischenliegendes
> Move-Ereignis … Das ist die entscheidende Messung dieser Story, und sie steht
> aus."

Wäre der Vorbehalt eingetreten, wäre ein neuer Prüfweg entstanden (Sitzungsprobe)
und #71 hätte als `size:m` **nicht mehr neben #83 gepasst**. B hat die Story
trotzdem nicht vorsorglich hochgestuft, sondern die Messung gefordert — mit der
Begründung, wer aus Vorsicht auf `m` gehe, verliere den einzigen kleinen
Kandidaten des Laufs auf eine Vermutung hin. Das war die richtige Reihenfolge.

**Die Messung ist am 05.08.2026 gelaufen. Der Vorbehalt ist nicht eingetreten,
und der vermutete Mechanismus war falsch** (`reproduktion.md`):

- Beide Befunde des Issues treten im vorhandenen offscreen-Testaufbau auf —
  14 von 16 Rollwerten zeigen eine angeschnittene Zeile, in **allen 14** rückt
  das Bild, in **13** sitzt die Markierung falsch.
- **Ein Move-Ereignis ist für den Fehler weder nötig noch hilfreich.** Die
  Auswahl ist bereits nach dem **Druck** falsch. Der Fehler entsteht
  *innerhalb* der Behandlung des Druckereignisses, nicht zwischen Druck und
  Loslassen.
- Es entsteht **kein neuer Prüfweg**: kein neuer Läufer, kein neues Bauziel,
  keine angemeldete Sitzung, kein echter Zeiger.

**`size:s` bestätigt.** Der Vorbehalt aus Feld 5 von B ist erledigt.

## 2. Was nur **ein** Bearbeiter gefunden hat

**A hat die Ursache ein- und ausgeschaltet, statt sie zu erschließen.** An einem
blanken `QListView` mit demselben Modell und demselben Delegate tritt der Fehler
nicht auf; mit der einen Verbindung `currentChanged → scrollTo(index)` tritt er
auf — Zeile für Zeile dasselbe Bild wie im `LibraryWindow`. Das ist der
Unterschied zwischen einer plausiblen und einer belegten Ursache.

**A hat drei Lesarten durchgemessen**, darunter eine, die im UI-Review nicht
stand (`ScrollPerPixel`). Ohne diese Messung wäre die Produktentscheidung ohne
Alternative gefallen.

**A hat den zweiten Fehlerfall gefunden, den das Issue nicht nennt:** In
mehreren Fällen ist danach **gar nichts** markiert, weil unter dem Zeiger nach
dem Rücken ein Gruppenkopf oder der Leerraum liegt. Ein Kriterium, das nur von
der Nachbarzeile spricht, deckt diesen Fall nicht ab.

**B hat den Tastaturpfad geschützt.** Die Heilung liegt drei Zeilen neben dem
Kopf-Vorlauf aus #57/#59. Fehlt der Satz, ist eine Regression zulässig, die
niemand als Bruch eines Kriteriums bemerken würde. B hat den fehlenden Satz
prüfbar formuliert geliefert; er steht jetzt als Kriterium im Issue.

**B hat die Kollision mit #70 gemeldet** — der wichtigste Punkt seiner Messung
und die Lücke im Kandidatenfeld des PO. Siehe Abschnitt 5.

**Beide haben unabhängig festgestellt, dass die Verdachtszeile des Issues
veraltet ist** (`:783` statt `:792`).

## 3. Die drei Fallen für den Testaufbau — jede hätte einen grünen Test ohne
Prüfwert erzeugt

1. **Ohne Vorauswahl feuert `currentChanged` nicht.** In der ersten Fassung der
   Sonde sahen so 2 von 11 Fällen wie ein Freispruch aus (A, F7).
2. **Ein festverdrahteter Rollwert kann den einen treffen, an dem der Fehler
   unter der Schwelle bleibt.** Bei einem der 16 gemessenen Rollwerte rückt das
   Bild nur um 35 px statt 72; der Klickpunkt liegt danach noch in derselben
   Zeile (Reproduktion, „Die eine Ausnahme").
3. **Der Vergleich „markiert == aktuell" belegt nichts.** Wird bei gedrückter
   Taste ein Move zugestellt, zieht Qt die aktuelle Zeile nach — dann stimmen
   beide überein, auf der falschen Zeile. Unter diesem Kriterium hätte ein Lauf
   in 9 von 14 Fällen „stimmt" gemeldet (Reproduktion, Versuch 2).

**Falle 3 ist am 05.08.2026 als Punkt 5 in die Liste „Rückgabewerte und Läufe,
die nichts belegen" der Dev-Agentendatei aufgenommen worden** — sie ist
allgemeiner als dieser Fall: Wer zwei Größen vergleicht, die derselbe Fehler
gemeinsam verschieben kann, misst nichts.

## 4. Die sechs Felder

**Feld 1 — Dateimenge.** A und B decken sich.

| | |
|---|---|
| **Quellen und Tests** | `src/ui/librarywindow.cpp` — **eine Funktion**, `showNote()`, darin die Zeile `:792`.<br>`tests/librarytest.cpp` — ein neuer Test bei den #57-Nachbarn |
| **Build** | **nichts** — keine neue Bibliothek, kein neues Ziel, kein neuer Läufer |
| **Belege und Prüfmittel** | Belegordner des Sprints; `sonden/klicksonde.cpp` dieser Vorprüfung ist der fertige Bauplan, `reproduktion-testfall.diff` der fertige Testfall |
| **Fachliche Quellen** | **SPEC 9** — der Absatz „Ein Mausklick tut das nicht" spricht heute nur vom Kopf-Vorlauf. Dass auch das Nachrücken zur Auswahl selbst unter dem Mausdruck steht, ist eine **entdeckte Bedingung** und zieht die SPEC nach (DoD 4/B9) |
| **Ausdrücklich nicht** | `src/capture/*`, `src/shell/*`, `src/store/*`, `src/ui/notelistdelegate.*` (die Zeilenhöhe ist gemessen **unbeteiligt** — sie ist die Größe des Sprungs, nicht seine Ursache), `src/ui/notelistmodel.*`, `wireframes/`, alle SPEC-Abschnitte außer 9 |

**Feld 2 — gemessene Fallen.** Die drei Testaufbau-Fallen aus Abschnitt 3, dazu:
die veraltete Verdachtszeile; der Fehler tritt **auch ohne Gruppengrenze** auf,
der #57-Merker schützt also nicht die schuldige Zeile; **nur der untere Rand
schneidet an**, nie der obere (bei `ScrollPerItem` setzt die Liste stets
zeilenbündig ab — wer einen Testfall „oben angeschnitten" bauen will, sucht
umsonst); und die Klebrigkeit des #57-Merkers, die Lesart 2 auf jede
programmatische Auswahländerung ausweiten würde.

**Feld 3 — AK-Urteil.** Ausgangsfassung **nicht ready** (Urteil B): Kopf trug
„Entwurf", kein Kriterium schützte den Tastaturpfad, AK 1 nannte kein
Prüfmittel, AK 2 ließ zwei Ergebnisse zu und war damit nicht einzeln prüfbar.
**Behoben durch den PO am 05.08.2026:** acht Kriterien, jedes gegen eine
gemessene Falle geschnitten. **Damit ready.**

**Feld 4 — Prüfmittel.** Unit-Test in `tests/librarytest.cpp`, offscreen:
`QTest::mouseClick` auf den sichtbaren Streifen, dann Zusicherungen auf
`currentIndex()`, `selectionModel()->selectedIndexes()`, den Text des
Detailbereichs, `visualRect(target).y()` und den Rollwert. Mutationsprobe über
das Entfernen der Heilung. Bildbeleg über `libraryshots`.
**Grenze:** Der Agent klickt nicht wirklich — alle Belege beruhen auf
zugestellten Ereignissen. Dass ein echter Mausklick denselben Weg nimmt, ist
geschlossen, nicht gemessen. Ein Blick des Kunden schließt das.
**B21 nicht einschlägig:** Gegenstand sind Rollwert, Zeilenlage und
Auswahlzustand.

**Feld 5 — Größenklasse: `size:s`.** Beide Bearbeiter unabhängig; der Vorbehalt
von B ist durch die Reproduktionsmessung ausgeräumt. Die Bedingung, unter der es
`m` würde, war Lesart 3 (`ScrollPerPixel`) — **durch die PO-Entscheidung für
Lesart 2 entfallen**.

**Feld 6 — offene Fragen: alle vom PO entschieden.**

| Frage | Entscheidung |
|---|---|
| Welche Lesart? | **Lesart 2** — beim Mausdruck gar nicht nachrücken. Erfüllt beide Kriterien wörtlich, eine Zeile groß, schreibt die Regel aus SPEC 9 weiter. Lesart 1 fällt aus (Bild springt weiter); Lesart 3 ist die schönere Bedienung, ändert aber nebenbei das Mausradverhalten — das hat niemand verlangt |
| AK 2 ist nicht einzeln prüfbar | Auf Lesart 2 festgelegt: „Versatz 0 px" |
| AK 1 deckt den zweiten Fehlerfall nicht | Ergänzt: „und es bleibt genau eine Zeile markiert" |
| SPEC 9 nachziehen? | **Ja**, DoD 4/B9 |
| Soll der Kunde selbst klicken? | **Ja**, in der Abnahme — der einzige Beleg, den kein Agent führen kann |
| Reihenfolge gegen #70 | **Ein gemeinsamer Strang.** Siehe Abschnitt 5 |

## 5. #70 — die Lücke, die B gefunden hat

B hat gemeldet, dass **#70 im Kandidatenfeld des PO fehlte**, und das mit einem
Sachargument belegt: #70 arbeitet in **denselben Zeilen**, und **#70 AK 3
verlangt ausdrücklich, dass der Klickpfad aus #57 unverändert ruhig bleibt** —
genau der Pfad, den #71 ändert. Die beiden sind nicht nur kollidierend, sie
können einander widersprechen.

**Der PO hat den Einwand angenommen.** #70 ist am 05.08.2026 vorgeprüft worden;
der Sprint-7-Schnitt behandelt beide in **einem** Strang. Der Beleg dafür, dass
der Einwand nötig war: #71 nennt #70 im eigenen Text unter „Nachbarschaft" —
„die drei zusammen wären ein natürliches ‚ruhige Liste'-Paket" —, und dieser
Satz stand da, bevor die Auswahl getroffen wurde.
