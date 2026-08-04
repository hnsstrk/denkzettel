# BM25 — Ist-Stand am eigenen Code

**Datum:** 04.08.2026 · **Auftrag:** PO · **Art:** Ermittlung, keine Änderung

Der Kunde fragt, ob BM25 die Suche verbessern könnte. Dieser Bericht klärt
**nicht**, was BM25 allgemein leistet (das tut die parallele Recherche),
sondern **was es für diesen Code konkret hieße**.

**Grenzen dieses Berichts, eingehalten:** keine Datei geändert außer dieser;
kein Messlauf, kein SQL gegen eine Datenbank, keine Installation, kein Commit.
Wo eine Frage nur durch Messung zu entscheiden ist, steht sie in Abschnitt 6
als Vorschlag — ungemessen und als solches gekennzeichnet.

Jeder Abschnitt trennt **Beobachtung** (am Code belegt) von **Schluss**
(Bewertung des Berichtenden).

---

## 1. Der Suchpfad heute — die Kette

### Beobachtung

| # | Ort | Was geschieht |
|---|---|---|
| 1 | `src/ui/librarywindow.cpp:325–332` | Das Suchfeld ist ein schlichtes `QLineEdit`; `textChanged` ist mit `LibraryWindow::searchChanged()` verbunden — gesucht wird bei **jedem Tastendruck**. |
| 2 | `src/ui/librarywindow.cpp:616–625` | `searchChanged()` führt zuerst eine laufende Löschung aus (`m_deletion->flush()`), dann `reload(Selection::Keep)`. |
| 3 | `src/ui/librarywindow.cpp:591–594` | `reload()` ruft **ausschließlich** `m_store->search(m_search->text())`. Volle Liste und Trefferliste sind derselbe Codeweg; das leere Feld ist kein Sonderfall. |
| 4 | `src/store/store.cpp:414–419` | `Store::search()` zerlegt die Eingabe mit `searchTerms()`; ohne Term liefert es `notes()` zurück. |
| 5 | `src/store/store.cpp:162–178` | `searchTerms()` ist der **einzige „Parser" von heute**: Alles, was nicht Buchstabe oder Ziffer ist, trennt Terme. Damit erreicht kein Zeichen des Nutzers FTS5 als Syntax — und weder die FTS5-Phrase noch das `LIKE`-Muster brauchen Maskierung. |
| 6 | `src/store/store.cpp:425–433`, `:147–148` | Aufteilung an der Schwelle `trigramLength = 3`: Terme ab drei Zeichen werden zu FTS5-Phrasen `"term"`, kürzere landen in `shortTerms`. |
| 7 | `src/store/store.cpp:435–441` | Aus beiden entstehen **WHERE-Bedingungen**: `id IN (SELECT rowid FROM notes_fts WHERE notes_fts MATCH :match)` für die Phrasen, je ein `content LIKE :shortN` für jeden kurzen Term. |
| 8 | `src/store/store.cpp:443–450` | Das erzeugte SQL: `SELECT <noteColumns> FROM notes WHERE <bedingungen mit AND> ORDER BY created_at DESC, id DESC`. **`notes_fts` steht nur in der Unterabfrage der WHERE-Klausel**, nicht in der FROM-Klausel. |
| 9 | `src/store/store.cpp:451–458` | Bindung: alle Phrasen mit Leerzeichen verbunden (FTS5 liest eine Phrasenfolge als UND), die kurzen Terme als `%term%`. |
| 10 | `src/store/store.cpp:465–469`, `:181–199` | Die Zeilen werden über `noteFromQuery()` gelesen — **spaltenweise über den Namen** (`query.value("id")` usw.), nicht über den Index. |
| 11 | `src/ui/notelistmodel.cpp:13–33` | `buildRows()` baut aus der Notizliste die Zeilen mit Tagesköpfen. Der Kommentar `:18–20` nennt die Voraussetzung ausdrücklich: *„The notes arrive newest first and the groups are ordered the same way, so a group is done the moment the next note falls into another one."* |

Die Rolle von `notes` gegen `notes_fts`: `notes_fts` ist eine
**external-content-Tabelle** (`content='notes'`, `content_rowid='id'`,
`src/store/store.cpp:54–58`) und hält keinen eigenen Text; drei Trigger
(`:64–75`) halten sie nach. Die Abfrage benutzt sie ausschließlich als
**Mengenlieferant für IDs** — sie liefert nie eine ausgegebene Spalte.

### Schluss

Der Suchpfad ist eine gerade Kette ohne Zwischenschicht: ein Feld, eine
Store-Methode, ein SQL-Statement, ein Modell. Wer die Sortierung ändern will,
muss genau zwei Stellen anfassen — `Store::search()` und alles, was die
Reihenfolge voraussetzt (Abschnitt 4). Einen Suchoperator-Parser gibt es
**noch nicht**; er ist offene Story S7 (Issue #10), was `src/store/store.h:75–77`
und der UI-Review `docs/scrum/reviews/sprint-03-s6-trefferliste.md:105–112`
festhalten.

---

## 2. Was BM25 technisch verlangen würde

### Beobachtung

**`bm25()` ist eine FTS5-Hilfsfunktion.** Sie darf nur in einer Abfrage
vorkommen, die selbst ein `MATCH` gegen **diese** FTS-Tabelle trägt. Heute
(`src/store/store.cpp:448–450`) steht `notes_fts` nur in einer nicht
korrelierten Unterabfrage der WHERE-Klausel; die äußere Abfrage kennt weder
die FTS-Tabelle noch einen Rangwert. **`ORDER BY bm25(...)` an der heutigen
Abfrage ist deshalb keine Ein-Zeilen-Änderung, sondern unmöglich.**

Der Kommentar `src/store/store.cpp:444–446` nennt als Grund gegen einen Join,
dass `notes` und `notes_fts` beide eine Spalte `content` führen und der Join
mehrdeutig wäre. **Am Code geprüft trägt dieser Einwand nur den naiven Join.**
Er entfällt, wenn die FTS-Seite als abgeleitete Tabelle eingebunden wird, die
nur zwei Spalten nach außen gibt — Rowid und Rangwert, kein `content`:

```
FROM notes
JOIN (SELECT rowid AS fts_id, bm25(notes_fts) AS score
        FROM notes_fts WHERE notes_fts MATCH :match) AS ranked
  ON ranked.fts_id = notes.id
```

Was das im Einzelnen berührt:

- `noteColumns()` (`src/store/store.cpp:141–145`) bliebe **unverändert** — die
  abgeleitete Tabelle liefert keine gleichnamige Spalte, `noteFromQuery()`
  (`:181–199`) liest weiter über Spaltennamen.
- Die `LIKE`-Bedingungen der kurzen Terme (`:439–441`) blieben, wo sie sind:
  in der WHERE-Klausel über `notes.content`.
- Die Indexdefinition (`:54–58`) setzt **weder `detail=none` noch
  `columnsize=0`**; Positionen und Spaltenlängen liegen also im Index, und
  BM25 hätte die Zahlen, die es braucht.
- Die FTS-Tabelle hat **genau eine Spalte** (`content`, `:54–55`). Die
  Spaltengewichte von `bm25()` haben hier folglich nichts zu gewichten.

### Schluss

1. **Verfügbar machen ist klein, entscheiden ist es nicht.** Die Umstellung
   auf einen rangfähigen Join ist in `Store::search()` eine Sache weniger
   Zeilen. Der Aufwand steckt nicht dort (Abschnitt 4 und 5).
2. **BM25 kann auf diesem Schema nicht das, was man intuitiv von ihm erwartet.**
   Weil `notes_fts` nur eine Spalte hat, kann kein Treffer bevorzugt werden,
   der in der **Betreffzeile** steht — die „erste Zeile als Betreff" (SPEC 9,
   `SPEC.md:405`) ist keine eigene Spalte, sondern Anzeigelogik der Oberfläche.
   Wer „Treffer im Betreff zuerst" will, bekommt das von BM25 hier **nicht**;
   das wäre eine zweite FTS-Spalte und damit ein Schema-Umbau.
3. **Der Tokenizer verschiebt die Bedeutung des Rangs.** Bei
   `trigram` sind die Indexterme Drei-Zeichen-Folgen; die „Dokumentlänge", mit
   der BM25 normiert, ist damit rund die **Zeichenzahl** der Notiz. Das ist
   eine Ableitung aus der Tokenizer-Definition, **nicht gemessen** — sie steht
   als Messvorschlag M1 in Abschnitt 6. Wenn sie zutrifft, rangiert BM25 hier
   vor allem **kurze Notizen vor langen** und nur nachgeordnet nach
   Trefferhäufigkeit.

---

## 3. Sonderfälle, an denen eine Relevanzsortierung zerbricht

### Beobachtung

**a) Terme unter drei Zeichen haben überhaupt keinen Rang.**
Sie stehen prinzipbedingt nicht im Trigramm-Index (`src/store/store.cpp:421–424`,
`SPEC.md:246–252`) und werden über `content LIKE '%…%'` auf `notes` gesucht
(`:439–441`, `:455–458`). Eine Notiz, die **nur** über diesen Weg gefunden
wird, kommt in keiner FTS-Abfrage vor — für sie existiert kein `bm25()`-Wert,
auch kein schlechter. Zwei Fälle:

- **Nur kurze Terme** (Test `searchFindsTermsShorterThanThreeCharacters`,
  `tests/storetest.cpp:370–397`: „KI", „ad", „ki"): `phrases` ist leer, es gibt
  **kein `MATCH`** und damit keine Tabelle, über die gerankt werden könnte. Der
  Rang müsste ersatzlos entfallen — die Liste fiele auf die chronologische
  Ordnung zurück.
- **Gemischt** („ad sprech", `tests/storetest.cpp:391`): Der Rang käme
  **allein vom langen Term**; der kurze Term wirkt als Filter, aber nicht auf
  die Reihenfolge.

**b) Suchen ohne Volltextterm haben nichts zu ranken.**
Der Operator-Parser (S7, Issue #10) ist **noch nicht gebaut**
(`src/store/store.h:75–77`). Nach SPEC 6 (`SPEC.md:212–217`) sind aber
`tag:backup`, `kat:todos`, `typ:audio`, `vor:2026-07` zulässige und für sich
allein vollständige Suchen. Eine solche Suche enthält kein `MATCH` — es gibt
keinen Text, dessen Relevanz man messen könnte.

**c) Die leere Suche.** `Store::search()` gibt bei termloser Eingabe
`notes()` zurück (`src/store/store.cpp:417–419`, Test
`tests/storetest.cpp:427–435`) — die volle Bibliothek, per Definition
chronologisch.

### Schluss

Die Relevanzsortierung stünde in **einer Minderheit der Abfrageformen** zur
Verfügung: nur wenn mindestens ein Term drei Zeichen oder länger ist. In allen
anderen Fällen — kurze Terme allein, reine Filtersuche nach S7, leeres Feld —
gäbe es keinen Rang.

Damit entsteht ein Verhalten, das der Bericht für den ernstesten Befund hält:
**Dieselbe Liste im selben Fenster wäre je nach Eingabe nach zwei verschiedenen
Kriterien sortiert, ohne dass irgendetwas das anzeigt.** „KI" sortierte
chronologisch, „Pipeline" nach Relevanz. Das ist keine Umsetzungslücke, die man
schließen kann — es folgt aus der Entscheidung zum Trigramm-Index und der
Teilstring-Rettung für kurze Terme (beide Kundenentscheidung 01.08.2026,
Issue #8, `SPEC.md:236–260`).

Ein einheitlicher Ersatzrang wäre eine erfundene Zahl: Ob eine `LIKE`-Notiz
vor oder hinter einer BM25-Notiz steht, hat keine sachliche Antwort.

---

## 4. Was an der heutigen Reihenfolge hängt

### Beobachtung

**Zusicherungen in SPEC und Zeichnung**

| Ort | Aussage |
|---|---|
| `SPEC.md:261–262` | „Die Trefferliste behält die Ordnung der Bibliothek (neueste zuerst, 9.) statt der FTS5-Relevanzsortierung — nur so trägt sie deren Tagesgruppen." |
| `SPEC.md:399–404` | SPEC 9: chronologische Liste in Tagesgruppen, innerhalb der Gruppen neueste zuerst. |
| `SPEC.md:407–410` | Der Zeitstempel eines Eintrags **folgt seiner Gruppe** (Heute/Gestern nur Uhrzeit). |
| `SPEC.md:411–416` | Tastensprung über eine Gruppengrenze holt den Kopf ins Bild (Issue #57). |
| `SPEC.md:435–438` | K2: die gespeicherte Notiz **bleibt in der laufenden Trefferliste stehen**. |
| `wireframes/Denkzettel Wireframes.dc.html:650` | „Denkzettel hat **genau eine Sortierung** (chronologisch, neueste zuerst, SPEC 9). Die Gliederung ist damit nur die lesbare Form dieser einen Ordnung." — Begründung dafür, dass es **keinen** Umschalter „In Gruppen anzeigen" gibt. |
| `wireframes/Denkzettel Wireframes.dc.html:651` | Begründung gegen einklappbare Gruppenköpfe: bei neueste-zuerst liegt das Gesuchte „fast immer oben". |
| `wireframes/Denkzettel Wireframes.dc.html:453, 663` | Die Trefferliste zeigt dieselbe Gliederung wie die Bibliothek. |

**Code**

| Ort | Abhängigkeit |
|---|---|
| `src/store/store.cpp:448–450` | Das `ORDER BY` selbst, samt begründendem Kommentar `:444–447`. |
| `src/store/store.cpp:398–400` | `notes()` — dieselbe Ordnung, Kommentar zum Tiebreak über `id`. |
| `src/store/store.h:52`, `:56` | Zugesichert in der Schnittstelle („newest first"). |
| `src/ui/notelistmodel.cpp:13–33` | **Die tragende Stelle.** `buildRows()` öffnet einen Kopf, sobald die Gruppe wechselt. Der Algorithmus setzt voraus, dass gleiche Gruppen **zusammenhängend** ankommen. |
| `src/ui/notelistmodel.h:11` | „The notes of the library list, newest first". |
| `src/ui/notelistmodel.cpp:111–132` | `takeNote()` — die Erkennung „letzte Notiz ihrer Gruppe" liest die **Nachbarzeilen**. |
| `src/ui/librarywindow.cpp:591–594` | Volle Liste und Trefferliste teilen sich einen Codeweg. |

**Tests, deren Zusicherung die Reihenfolge ist**

| Ort | Was bräche |
|---|---|
| `tests/storetest.cpp:302–327` | `findsNotesByFullText` — `QCOMPARE(searchContents("Bücher"), {books, backup})` mit dem Kommentar `:320–321`: *„the newest comes first — a result list is ordered like the library list, not by relevance (SPEC 9)"*. **Der einzige Test, der die Frage direkt zusichert.** |
| `tests/storetest.cpp:291–299` | `searchContents()` vergleicht grundsätzlich **geordnete** `QStringList` — jede Mehrtrefferzusicherung im Store-Test hängt daran. |
| `tests/storetest.cpp:336–346`, `:362–365`, `:384–391`, `:406–418` | Weitere geordnete Vergleiche (Umlaute, Wortteile, kurze Terme, Literalität). Bei einem Treffer je Vergleich ordnungsblind, bei mehreren nicht. |
| `tests/librarytest.cpp:1968–2000` | `groupsTheSearchResultsLikeTheLibrary` — vergleicht die **volle Zeilenfolge mit Köpfen** vor und nach der Suche. Bricht bei jeder Umsortierung. |
| `tests/librarytest.cpp:1939–1966` | `filtersTheListWithTheSearchField` — greift `noteRow(list, 0)`. |
| `tests/librarytest.cpp:3237–3277` | `keepsTheSavedNoteInTheResultListUntilTheSearchChanges` (K2) — baut `expected` aus der **vorherigen** Zeilenfolge und prüft Auswahlzeile und Rollstand. |
| `tests/librarytest.cpp:703–730`, `:731–763` | `takesAndReinsertsANote`, `dropsTheHeadWithTheLastNoteOfItsGroup` — Löschen/Undo über **Indexpositionen** in einer gruppierten Liste. |
| `tests/librarytest.cpp:783–795` | Der Testhelfer `storedNote()` staffelt jede Notiz eine Sekunde älter, *„so the list order is the order the notes were added in"* — die Grundannahme fast aller Bibliothekstests. |
| `tests/librarytest.cpp:1241–1292`, `:1293–1374`, `:1624–…` | Tastenlauf über Gruppenköpfe und Kopf-ins-Bild-Holen (Issue #57). |

**Bildbelege**

| Ort | Abhängigkeit |
|---|---|
| `tests/searchshots.cpp:99–114` | Die Bildreihe der Suche; Bild 2 heißt `2-trefferliste-mit-gruppen.png`. |
| `docs/scrum/reviews/sprint-03-s6-trefferliste.md:44–57` | Abgenommener UI-Review: der Fall „**kopflastige Trefferliste**" — fünf Köpfe zu fünf Einträgen — wurde als noch tragbar beurteilt, mit der Begründung, ohne Kopf stünde bei einem Treffer von heute nirgends, dass er von heute ist. |
| `docs/scrum/reviews/sprint-04-s8-ui-review/uxshots.cpp:425–442` | Bildbeleg zu K2, ausdrücklich „Die einzige Notiz der Trefferliste steht unter ihrem Gruppenkopf". |
| `CHANGELOG.md:20` | Veröffentlichte Zusage zu K2. |

### Schluss

Die Sortierung ist in diesem Produkt **keine Eigenschaft der Suche, sondern
die Voraussetzung der Listendarstellung**. `buildRows()` ist kein Sortierer,
sondern ein Gruppierer, der eine sortierte Eingabe erwartet. Bekäme er eine
nach Relevanz geordnete Liste, entstünde kein Fehler und kein Absturz —
sondern eine Liste, in der Tagesköpfe **mehrfach und in wechselnder Folge**
auftreten („Heute · Älter · Heute · Gestern"). Der Grenzfall, den der Review
von Sprint 3 gerade noch für tragbar hielt (fünf Köpfe zu fünf Einträgen),
würde damit zum **Normalfall**.

Das ist der Grund, warum `SPEC.md:261–262` die Entscheidung nicht mit
Geschmack, sondern mit einer Abhängigkeit begründet: *„nur so trägt sie deren
Tagesgruppen."* Eine BM25-Sortierung ist deshalb keine Änderung an der Suche,
sondern eine Änderung an der **Bibliotheksdarstellung** — und berührt zwei
weitere abgenommene Festlegungen: den fehlenden Sortierumschalter und die
fehlenden einklappbaren Köpfe, die beide ausdrücklich damit begründet sind,
dass es **genau eine** Sortierung gibt
(`wireframes/Denkzettel Wireframes.dc.html:650–651`).

---

## 5. Größenordnung des Eingriffs

Grobe Einschätzung, **keine Story-Point-Schätzung** — die gehört in die
Schätzklausur.

**Was eine Zeile wäre:** nichts. Es gibt keine Ein-Zeilen-Fassung dieser
Änderung; `ORDER BY bm25(...)` scheitert an der heutigen Abfrageform
(Abschnitt 2).

**Klein (Stunden, `store.cpp` und Store-Tests):** den Rang überhaupt
**verfügbar** machen — Join auf eine abgeleitete Tabelle, Rangspalte
mitführen. Rund fünf bis zehn Zeilen in `Store::search()`
(`src/store/store.cpp:435–458`) plus eine Verzweigung für den Fall ohne
Phrase. Solange die Ausgabereihenfolge unverändert bleibt, bricht davon
nichts.

**Mittel (die eigentliche Frage):** die **Ausgabe** danach ordnen. Dann fallen
an: die SPEC-Zusicherung `SPEC.md:261–262` und ihre Begründung, die
Zeilenfolge-Zusicherungen in mindestens fünf Tests (Abschnitt 4), die
Bildreihe `tests/searchshots.cpp` samt neuem Bildbeleg, und eine Antwort auf
die Sonderfälle aus Abschnitt 3 — insbesondere darauf, dass kurze Terme und
Filtersuchen **keinen** Rang haben.

**Umbau:** alles, was die Gruppenköpfe rettet. Jede Variante — Rang nur
**innerhalb** der Tagesgruppen, ein Umschalter „nach Relevanz", eine
Gliederung, die bei Relevanzsortierung entfällt — berührt `NoteListModel`,
die Tastennavigation über Gruppengrenzen (Issue #57), die Zeitstempelform je
Gruppe (`SPEC.md:406–410`) und die Zeichnungen 2c/3a/3b. Dazu käme die
Wechselwirkung mit S7 (Issue #10), das heute noch offen ist.

**Hinweis, nicht geprüft:** Wenn der Kunde „ich sehe nicht, warum das ein
Treffer ist" meint, ist BM25 möglicherweise gar nicht das Mittel. Der UI-Review
`docs/scrum/reviews/sprint-03-s6-trefferliste.md:112–119` hat für genau diese
Beanstandung die **Hervorhebung der Fundstelle** als offene Notiz vermerkt.
Ob das Kundenanliegen Rang oder Sichtbarkeit betrifft, ist eine Frage an den
Kunden, keine Ermittlung am Code.

---

## 6. Vorgeschlagene Messungen — nicht ausgeführt

**M1 — Trägt der Join den Rang, und was misst BM25 unter `trigram`?**
Entscheidet zwei offene Punkte aus Abschnitt 2: ob `bm25()` durch eine
abgeleitete Tabelle über einen external-content-Index hindurchreicht, und ob
der Rang bei Trigramm-Tokens von der Notizlänge beherrscht wird.

```sql
CREATE TABLE notes(id INTEGER PRIMARY KEY, content TEXT NOT NULL);
CREATE VIRTUAL TABLE notes_fts USING fts5(
  content, content='notes', content_rowid='id',
  tokenize='trigram remove_diacritics 1');
INSERT INTO notes(content) VALUES
  ('Backup prüfen'),
  ('Backup Backup Backup und dazu noch sehr viel weiterer Text, damit die Notiz deutlich länger wird als die erste');
INSERT INTO notes_fts(notes_fts) VALUES('rebuild');
SELECT n.id, r.score, n.content
  FROM notes n
  JOIN (SELECT rowid AS fts_id, bm25(notes_fts) AS score
          FROM notes_fts WHERE notes_fts MATCH '"backup"') AS r
    ON r.fts_id = n.id
 ORDER BY r.score;
```

Aufruf: `sqlite3 :memory:` mit obigem Skript. **Kosten:** Sekunden, keine
Installation, keine Änderung am Projekt. **Einschränkung:** misst die
System-`sqlite3`, nicht die von Qt geladene Bibliothek. Soll die Messung am
Bibliotheksstand der Anwendung erfolgen, ist ein wegwerfbarer Läufer unter
`tests/` nötig — Vorbild: `tests/spellfixspike.cpp`; **Kosten:** ein Build
plus wenige Minuten, und er wäre eine Codeänderung, die dieser Auftrag
ausschließt.

**M2 — Wie kopflastig wird eine rangsortierte Trefferliste?**
Entscheidet die Bewertung aus Abschnitt 4 am Bild statt an der Vermutung.
Verlangt einen abgewandelten `searchshots`-Lauf gegen eine rangsortierte
`Store::search()` — also eine **Codeänderung** und damit einen eigenen Spike
mit PO-Freigabe. Ohne dieses Bild ist die Aussage „die Köpfe zerfallen" eine
begründete Erwartung, kein Beleg.

**Nicht messbar, weil Entscheidung:** wie eine Notiz, die nur über den
`LIKE`-Weg gefunden wurde, gegen eine BM25-bewertete Notiz einzuordnen wäre
(Abschnitt 3). Das ist eine Kundenfrage.
