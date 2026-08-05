# Vorprüfbericht #70 — Erste Notiz einer Gruppe holt ihren Tageskopf ins Bild

**Konsolidiert vom PO am 05.08.2026** aus `messung-a.md` (Bearbeiter A,
`denkzettel-dev`) und `messung-b.md` samt Nachtrag (Bearbeiter B, Scrum Master),
beide gegen Stand `581dacc`/`7afe022`. Die Messungen entstanden unabhängig
voneinander.

**Ergebnis: `size:s`, ready — nach Nachschärfung der Kriterien durch den PO.**

---

## 1. Wo die beiden Messungen übereinstimmen

Beide kommen unabhängig auf **`size:s`** und auf dieselbe Kernaussage: **#70 und
#71 gehören nicht nebeneinander, sondern in einen Strang.** Beide messen den
Abstand in `librarywindow.cpp` als praktisch null — A: „eine Zeile, derselbe
Rumpf"; B: „dieselbe 14-Zeilen-Bedingung `:780–793`". Beide stellen fest, dass
die Story **keine Kopfdatei-Änderung, keinen Build-Eingriff und keinen neuen
Prüfweg** braucht.

## 2. Wo sie auseinandergehen — die Reihenfolge

| | Vorschlag | Begründung |
|---|---|---|
| **A** | #70 zuerst, #71 darauf | „kostet weniger als zwei Stränge mit Rebase" — ohne gemessenen Grund für die Richtung |
| **B** | **#71 zuerst**, #70 darauf | zunächst mit einem Argument, das sich als falsch erwies (siehe §3); im Nachtrag durch zwei neu gemessene ersetzt |

**PO-Entscheidung: #71 zuerst.** Zwei Gründe, beide von B im Nachtrag gemessen:

1. **Beide hängen an derselben Marke.** #71 AK 6 heilt die Klebrigkeit von
   `m_selectionFollowsAPress`; #70 hängt seinen neuen Auslöser hinter genau
   diese Marke. **#71 zuerst heilt sie einmal für beide Auslöser.** Wer #70
   zuerst baut, hängt einen zweiten Auslöser an eine Marke, deren Fehler bekannt
   und noch nicht behoben ist.
2. **#70 AK 3 wird dadurch endgültig statt vorläufig.** Es sichert etwas über
   den Klickpfad zu; #71 ändert diesen Pfad. In der Reihenfolge #71 → #70 wird
   das Kriterium gegen den gelieferten Stand geschrieben.

## 3. Ein Befund, der sich als falsch erwies — und warum er trotzdem im Bericht steht

B hatte als **schwersten** Grund für den gemeinsamen Strang gemessen, dass #71
die **Einheit des Rollwerts** wechsle: Unter `setVerticalScrollMode(ScrollPerPixel)`
wäre `verticalScrollBar()->value()` keine Zeilennummer mehr, und **drei der vier
Prüfsätze, auf die #70 in AK 2, AK 3 und AK 4 baut, wären grün geblieben und
hätten etwas anderes geprüft** — die Bauart „vier grüne Tests, die nichts
prüfen" aus `CLAUDE.md`, vorab gemessen.

**Die Prämisse stimmte nicht.** `ScrollPerPixel` ist **Lesart 3** von dreien;
Bearbeiter A hatte **Lesart 2** empfohlen, und der PO hat Lesart 2 entschieden.
Der PO hat B korrigiert; B hat nachgeprüft statt übernommen und den Fund im
Nachtrag als gegenstandslos gekennzeichnet. **Die Messung `m5` bleibt stehen**
und beschreibt, was unter Lesart 3 gegolten *hätte* — nicht, was gilt (B17: ein
überholter Beleg wird geankert, nicht geglättet).

**Warum das hier festgehalten wird:** Der Fund war nicht wertlos. Er beschreibt
eine reale Falle, die eine andere Lesart ausgelöst hätte, und er hat die
Lesart-Entscheidung des PO im Nachhinein bestätigt. Ein Bericht, der nur die
zutreffenden Funde führt, verschweigt, wie das Urteil zustande kam.

## 4. Die vier gemessenen Fallen

1. **Die Bedingung wird erweitert, nicht ersetzt.** Der Grenzübertritt-Zweig in
   `:785` muss bleiben — an ihm hängen vier Prüfsätze. Neu ist ein
   **Oder**-Zweig (A und B unabhängig).
2. **`!m_selectionFollowsAPress` muss im selben `if` bleiben — und kein heutiger
   Prüfsatz fängt es, wenn es das nicht tut** (B, `m3`). Wer einen zweiten
   `if`-Block danebenstellt und die Marke vergisst, **öffnet #57 wieder, ohne
   dass ein Test rot wird.**
3. **Der Fehler ist heute grün** (B, `m3`). Von sieben `Key_Up`-Stellen im
   Bestand trifft keine den Fall; ein Klick auf die **erste** Notiz einer Gruppe
   kommt in keinem Prüfsatz vor. Der Testaufbau, in dem der Fehler auftreten
   könnte, ist schlicht nicht vorhanden.
4. **Der Betrag der Verfehlung gehört in keinen Prüfsatz** (B). Die Pixelzahl
   des ursprünglichen Befunds stammt aus der Kundensitzung; offscreen gelten
   andere Zeilenhöhen. Prüfbar ist das Vorzeichen, nicht der Betrag.

**Dazu zwei Funde von A, die je einen Fehlversuch ersparen:**

- **AK 1 braucht keine Passbedingung.** Kopf und erste Notiz messen zusammen
  106 px, das flachste erreichbare Fenster lässt 149 px Liste — die
  Passbedingung ist bei jeder Fenstergröße erfüllt. Eine Bedingung hätte es
  gebraucht, wenn das Kriterium eine beliebige Notiz meinte.
- **Es gibt bereits eine Kandidatenfassung**, `flicken/a-70.patch`: +5/−1 Zeilen
  in einem Rumpf, heilt den Befund und hält die volle Testauflage (108/108).

## 5. Zwei Fehler in der Vorlage des PO, von A gefunden

1. **Die SPEC-Angabe im Issue war tot.** Die Story berief sich auf
   „Zeichnung 3b, `SPEC.md:405–409`"; dort steht heute Abschnitt 7.3
   (Themen-Clustering). Der gemeinte Zeitstempel-Satz liegt bei `:499–503`. Die
   **Begründung** der Story hängt an dieser Fundstelle.
2. **Das ursprüngliche AK 6 hatte eine falsche Prämisse.** „heute sagt die SPEC
   dazu nichts" trifft nicht zu — `SPEC.md:513–518` trägt die
   Grenzübertrittsregel. Zu tun ist eine **Ergänzung**. Ohne die
   Richtigstellung hätte jemand einen Absatz neu geschrieben, der schon dasteht.

Beides ist am 05.08.2026 im Issue berichtigt.

## 6. Die sechs Felder

**Feld 1 — Dateimenge.**

| | |
|---|---|
| **Quellen und Tests** | `src/ui/librarywindow.cpp` — **eine Stelle**, der Bedingungskopf des Kopfholens. Rumpf und das folgende `scrollTo(index)` bleiben unverändert.<br>`tests/librarytest.cpp` — Slotliste und neue Zusicherungen bei den vorhandenen Kopf-Zusicherungen.<br>`tests/libraryshots.cpp` — **Szene 7**; sie ist bereits genau dieser Fall |
| **Build** | **keine Änderung** — beide Läufer sind eingerichtet, keine neue Abhängigkeit |
| **Belege und Prüfmittel** | Belegordner des Sprints (B7); kein neues Prüfmittel zu bauen |
| **Fachliche Quellen** | `SPEC.md` Abschnitt 9, die Kopfregel `:513–518` — **ergänzen, nicht ersetzen**. Wireframe 3b als **Referenz** |
| **Ausdrücklich nicht** | `src/ui/librarywindow.h` (es braucht keinen neuen Helfer), `notelistmodel.*`, `notelistdelegate.*`, der Aktions- und Knopfblock (Fläche #72), `src/capture/*` und dessen Tests (Fläche #83), `src/shell/*`, `src/store/*`, `wireframes/` |

**Feld 2 — gemessene Fallen:** die vier aus §4 plus die zwei Funde von A.

**Feld 3 — AK-Urteil.** A schlug **ready ja** vor (mit einer Auflage am Schnitt
und einer Berichtigung an AK 6), B urteilte **ready nein** — nicht wegen der
Kriterien, sondern wegen **zwei selbstdeklarierter offener Punkte**: „Vor dem
Ziehen prüfen" im Abschnitt Nachbarschaft und der ganze Abschnitt „Nicht
geschätzt", der auf das am 04.08.2026 beendete Verfahren und das gelöschte
`sp:`-Label verweist.
**B trägt das Urteil**, und es ist das strengere zu Recht: Der DoR-Zusatz vom
04.08.2026 ist genau gegen diese Bauart gefasst.
**Behoben durch den PO am 05.08.2026:** beide Abschnitte gestrichen bzw. durch
das Messergebnis ersetzt, sieben Sätze übernommen, sieben Kriterien einzeln
prüfbar gefasst. **Damit ready.**

**Feld 4 — Prüfmittel.** Neue Zusicherungen in `tests/librarytest.cpp` nach dem
Muster von `staysPutWhileTheSelectionMovesWithinItsGroup`: Rollwert vor und nach
dem Tastendruck plus `viewport()->rect().contains(visualRect(head))`. **Das
Endbild allein trennt die Fälle nicht** — es sieht in beiden richtig aus.
Bildbeleg über Szene 7 von `libraryshots`, **offscreen genügt** (B21 nicht
einschlägig: Gegenstand ist Geometrie, nicht Hülle, Rundung, Kontur, Schatten
oder Dekoration), mit `QT_SCALE_FACTOR` auf der Kundenskalierung.
**Grenzen, ausgesprochen:** Ob der Handel „Lesbarkeit gegen Bewegung" im
täglichen Gebrauch aufgeht, kann ein Agent messen, aber nicht bewerten — das hat
in Sprint 3 und Sprint 5 je erst der Kunde am laufenden Fenster gesagt, beide
Male gegen die vorherige Messlage. Und ob Zeichnung 3b nachzuziehen ist, ist
eine UX-Frage, die zu **melden** und nicht in dieser Story zu heilen ist.

**Feld 5 — Größenklasse: `size:s`.** Beide Bearbeiter unabhängig, ohne
Abweichung. **Ausdrücklich:** Die Klasse gilt für #70 **allein** und ist kein
Freibrief für das Paket mit #71.

**Feld 6 — offene Fragen: alle vom PO entschieden.**

| Frage | Entscheidung |
|---|---|
| Schnitt #70/#71 | **Ein Strang, #71 zuerst** (§2) |
| AK 6 richtigstellen | **Ja** — Ergänzung statt Neuschrift, im Issue vollzogen |
| Tote SPEC-Angabe | **Berichtigt** auf `:499–503`, mit Vermerk |
| Falscher Kommentar an Szene 7 des Bildläufers | Als **Bestandsbefund** ins Issue aufgenommen: Wird #70 gebaut, wird der Kommentar wahr. Wird #70 nicht gezogen, bleibt eine versionierte Behauptung stehen, die ihr eigenes Bild widerlegt — dann eigenes Issue |
| Wireframe 3b nachziehen? | **Nicht Gegenstand dieser Story.** UX-Frage, gemeldet |

## 7. Der Sprint-7-Schnitt

**#83 (`l`) + #71 (`s`) + #70 (`s`) + #72 (`s`)** — vier Issues, zwei Stränge.
B vertritt ihn im Nachtrag unter zwei Bedingungen, die der PO übernommen hat:

1. **Strang B ist *ein* Strang** — ein Worktree, ein Agent, drei Commits in der
   Reihenfolge #71 → #70 → #72. Keine Parallelarbeit innerhalb von B.
2. **Das Sprint-Konto steht bei der Freigabe auf `4 Issues · 1×l, 3×s`** — die
   obere Grenze der Issue-Zahl ist **erreicht, nicht nur berührt**. Jeder Zugang
   danach ist eine Grenzüberschreitung nach B12.

**Die Begründung des PO für vier statt drei Issues war zu weit gefasst, und B
hat sie berichtigt.** Der PO hatte argumentiert, eine Trennung mache ein
abgenommenes Kriterium im Folgesprint falsch. Das gilt nur für die Reihenfolge
#70 vor #71; in der umgekehrten Trennung entstünde der Widerspruch nicht.
**Der tragende Grund ist ein anderer:** Eine Trennung macht **zweimal dieselben
vierzehn Zeilen und denselben SPEC-Absatz** auf — #70 AK 7 zieht die Kopfregel
nach (`SPEC.md:513–514`), #71 AK 7 die Mausklick-Bedingung (`:514–518`),
**gemeinsame Zeile 514** — ohne dafür etwas einzusparen.
