# Recherche: Trägt BM25 für die Suche von Denkzettel?

Datum: 04.08.2026 · Auftrag: Product Owner · Art: Sachrecherche, **kein** Messlauf

**Grenzen dieser Arbeit.** Nichts installiert, kein SQL ausgeführt, keine
Datei ausser dieser geändert. Wo eine Messung eine Frage entscheiden würde,
steht ein Vorschlag in Abschnitt 7 — ausgeführt wurde er nicht.

**Lesart.** „Beobachtung" ist belegt (Doku-Zitat oder Fundstelle im
SQLite-Quelltext). „Rechnung" ist aus der zitierten Formel abgeleitet und
ohne Messung nachprüfbar. „Schluss" ist meine Bewertung und kann falsch sein.

---

## 1. Was BM25 ist

**Beobachtung.** BM25 bewertet, wie gut eine Zeile zu einer Volltextabfrage
passt, aus drei Grössen: der Häufigkeit des Suchbegriffs in der Zeile (TF),
seiner Seltenheit im Gesamtbestand (IDF) und der Länge der Zeile im Verhältnis
zur Durchschnittslänge. Zwei Konstanten steuern das Verhalten: `k1` sättigt
die Termhäufigkeit (der zehnte Treffer zählt weniger als der zweite), `b`
regelt, wie stark lange Dokumente bestraft werden.

Die in FTS5 umgesetzte Fassung steht als ausführbarer Code in
`ext/fts5/fts5_aux.c`, Funktion `fts5Bm25Function` (Zeilen 693–747):

```c
const double k1 = 1.2;          /* Constant "k1" from BM25 formula */
const double b = 0.75;          /* Constant "b" from BM25 formula */
...
    for(i=0; i<pData->nPhrase; i++){
      score += pData->aIDF[i] * (
          ( aFreq[i] * (k1 + 1.0) ) /
          ( aFreq[i] + k1 * (1 - b + b * D / pData->avgdl) )
      );
    }
    sqlite3_result_double(pCtx, -1.0 * score);
```

---

## 2. Die FTS5-Umsetzung

Alle Aussagen dieses Abschnitts sind **Beobachtungen** aus
<https://sqlite.org/fts5.html> und dem Quelltext.

### 2.1 Aufruf

```sql
SELECT * FROM ft WHERE ft MATCH ? ORDER BY bm25(ft);
```

Die Argumente nach dem Tabellennamen sind **Spaltengewichte**:

> „The first argument passed to bm25() following the table name is the weight
> assigned to the leftmost column of the FTS5 table. The second is the weight
> assigned to the second leftmost column, and so on." (FTS5-Doku 5.1.1)

Im Quelltext wirkt das Gewicht **auf die Termhäufigkeit**, nicht auf den
fertigen Wert (`fts5_aux.c:722–723`):

```c
      double w = (nVal > ic) ? sqlite3_value_double(apVal[ic]) : 1.0;
      aFreq[ip] += w;
```

### 2.2 `ORDER BY rank`

> „All FTS5 tables feature a special hidden column named 'rank' … in a
> full-text query, column rank contains by default the same value as would be
> returned by executing the bm25() auxiliary function with no trailing
> arguments." (FTS5-Doku 5.2)

Die Zuordnung lässt sich dauerhaft (`INSERT INTO ft(ft, rank) VALUES('rank',
'bm25(10.0, 5.0)')`, Doku 6.11) oder je Abfrage (`rank MATCH
'bm25(10.0, 5.0)'`) ändern.

### 2.3 Vorzeichen

> „the FTS5 implementation of BM25 multiplies the result by -1 before
> returning it, ensuring that better matches are assigned numerically lower
> scores." (FTS5-Doku 5.1.1; im Code `sqlite3_result_double(pCtx, -1.0 * score)`)

**Folge:** `ORDER BY rank` **aufsteigend** liefert die besten Treffer zuerst.
Ein `DESC` dreht die Liste vom Besten weg — ein Fehler, der keine Meldung
erzeugt und nur an der Trefferqualität auffällt.

### 2.4 Sind `k1` und `b` einstellbar?

**Nein.**

> „_k1_ and _b_ are both constants, hard-coded at 1.2 and 0.75 respectively."
> (FTS5-Doku 5.1.1; bestätigt in `fts5_aux.c:700–701`)

### 2.5 Wo bm25() überhaupt stehen darf

> „Auxiliary functions … may only be used within full-text queries (those that
> use the MATCH operator, or LIKE/GLOB with the trigram tokenizer) on an FTS5
> table."

---

## 3. Die zentrale Frage: Was misst BM25, wenn die Terme Trigramme sind?

Das war der Auftrag, und die Antwort fällt anders aus als die naheliegende
Sorge erwarten lässt.

### 3.1 Beobachtung: FTS5 rechnet über **Phrasen**, nicht über Tokens

Die Doku definiert die Symbole der Formel ausdrücklich phrasenweise:

- `nPhrase` — „the number of phrases in the query"
- `IDF(qi)` — „the inverse-document-frequency of query phrase _i_"
- `n(qi)` — „the total number of rows that contain at least one instance of
  phrase _i_"
- `f(qi,D)` — „the phrase frequency of phrase _i_", voreingestellt „the number
  of occurrences of the phrase within the current row"

Der Quelltext bestätigt es: `Fts5Bm25Data` hält `int nPhrase` und je Phrase
**einen** IDF-Wert (`fts5_aux.c:592–598`); die IDF-Schleife läuft
`for(i=0; i<nPhrase; i++)` (Zeile 656); die Häufigkeitsschleife summiert die
Instanzen **auf die Phrase** (`aFreq[ip] += w`, Zeile 723), wobei `ip` die von
`xInst()` gelieferte Phrasennummer ist.

### 3.2 Beobachtung: Denkzettel fragt bereits phrasenweise

`src/store/store.cpp:429` klammert jeden Suchbegriff:

```cpp
            phrases.append(QStringLiteral("\"%1\"").arg(term));
```

und fügt sie mit Leerzeichen zusammen (Zeile 453). Aus „foto backup" wird
`"foto" "backup"` — **zwei** Phrasen, nicht vier oder mehr Trigramme.

### 3.3 Beobachtung: ein Trigramm-Phrasentreffer ist ein Teilstring-Treffer

Der Trigramm-Tokenizer schiebt ein Fenster von drei Zeichen um **je ein
Zeichen** weiter und gibt bei jeder Position ein Token aus
(`fts5_tokenize.c:1387–1418`: `xToken(...)` in der Schleife, danach das erste
Zeichen aus dem Puffer entfernen und das nächste anhängen). Eine Phrase aus
aufeinanderfolgenden Trigrammen trifft daher genau dann, wenn der Suchtext als
Teilstring vorkommt — das ist genau das in SPEC 6 zugesicherte Verhalten.

### 3.4 Antwort auf die Auftragsfrage

**Schluss.** Die befürchtete Verzerrung — „ein Suchwort zerfällt in mehrere
überlappende Trigramme, deren stark korrelierte Häufigkeiten mehrfach in die
Summe eingehen" — **tritt bei Denkzettel nicht auf**. Sie träte auf, wenn
jedes Trigramm eine eigene Phrase wäre. Es ist aber jeder **Suchbegriff** eine
Phrase, und FTS5 summiert die Formel über Phrasen. Damit gilt:

| Grösse | Bedeutung bei Wort-Tokenizer | Bedeutung bei Denkzettels Trigramm-Index |
|---|---|---|
| `f(qi,D)` | wie oft das Wort in der Notiz steht | wie oft der **Teilstring** in der Notiz steht |
| `n(qi)` für IDF | in wie vielen Notizen das Wort steht | in wie vielen Notizen der **Teilstring** steht |
| `D`, `avgdl` | Wortzahl der Notiz / Mittel | **Zeichenzahl** der Notiz / Mittel |

Die ersten beiden Zeilen sind sinnvolle Grössen: „wie oft kommt der gesuchte
Teilstring vor" und „wie selten ist er im Bestand" sind genau die Fragen, die
TF und IDF beantworten sollen.

Die dritte Zeile sieht nach einem Bruch aus, ist aber keiner: `D` geht
ausschliesslich als Verhältnis `D / avgdl` in die Formel ein
(`fts5_aux.c:740`). Zähler und Nenner stehen in derselben Einheit, also kürzt
sich der Massstab. Die Längennormierung misst dann Zeichenlänge statt
Wortlänge — für deutsche Fliesstexte praktisch derselbe Ordnungsbegriff.

**Die Trigramm-Eigenschaft ist also nicht der Grund, BM25 hier abzulehnen.**
Die Gründe stehen im nächsten Abschnitt und haben mit dem Tokenizer wenig zu
tun.

---

## 4. Fallstricke

### 4.1 Der wichtigste: BM25 hat bei Denkzettel keinen einzigen Regler

**Beobachtung.** `k1` und `b` sind hartcodiert (2.4). Der einzige
Einstellpunkt von `bm25()` sind Spaltengewichte (2.1). `notes_fts` hat
**genau eine Spalte** (`store.cpp:54–58`), und Notizen haben keinen Titel.

**Rechnung.** Bei einer Spalte multipliziert ein Gewicht `w` jede
Termhäufigkeit mit demselben Faktor. Ein gemeinsamer Faktor ändert die
Reihenfolge nicht — nur die Zahlenwerte.

**Schluss.** Der in jedem Ratgeber genannte Haupthebel („Titel höher
gewichten als Fliesstext") existiert für Denkzettel nicht. BM25 wäre hier ein
**Schalter, kein Regler**: Man bekommt genau eine Rangfolge, und wenn sie
nicht gefällt, gibt es innerhalb von FTS5 nichts zu drehen. Der übliche Weg
aus schlechter Relevanz ist damit von vornherein versperrt.

### 4.2 Bei kurzen Notizen entartet BM25 zu „kürzeste Notiz zuerst"

**Rechnung** aus der Formel in Abschnitt 1. In einer kurzen persönlichen
Notiz steht ein gesuchter Teilstring fast immer **genau einmal**: `f(qi,D)=1`
für nahezu jeden Treffer. Setzt man `f=1` ein, ist der Bruch

```
(1 · 2,2) / (1 + 1,2 · (0,25 + 0,75 · D/avgdl))
```

nur noch von `D` abhängig, und zwar **monoton fallend**: je kürzer die Notiz,
desto höher der Beitrag. Bei einem einzelnen Suchbegriff ist der IDF-Anteil
für alle Treffer identisch (derselbe Begriff), er kürzt sich aus dem Vergleich
heraus.

**Schluss.** Für den häufigsten Fall — ein Suchwort, kurze Notizen — ist
BM25 hier **der Länge nach sortiert, aufsteigend**. Eine Einzeilernotiz, in
der „Backup" beiläufig vorkommt, landet vor der ausführlichen Notiz über
Backups. Ob der Kunde das als Verbesserung empfindet, ist keine technische
Frage; ich halte es für zweifelhaft. Der TF-Anteil, der BM25 seinen Wert
gibt, kann erst greifen, wenn Begriffe mehrfach vorkommen — das setzt längere
Texte voraus, als Denkzettel sie erwartet.

### 4.3 Der Kurzbegriff-Pfad hat keinen Wert, den man einsortieren könnte

**Beobachtung.** `Store::search()` teilt die Suche in zwei Wege
(`store.cpp:424–441`): Begriffe ab drei Zeichen gehen über den Index,
Begriffe mit ein oder zwei Zeichen über `content LIKE '%…%'` direkt auf
`notes` — beides UND-verknüpft. Auxiliary-Funktionen gibt es nur im
FTS5-Zweig (2.5).

**Beobachtung.** Die Abfrage nutzt heute `id IN (SELECT rowid FROM notes_fts
WHERE notes_fts MATCH :match)` (`store.cpp:437`) und sortiert erst aussen.
Ein Rangwert aus der Unterabfrage steht dort nicht zur Verfügung.

**Schluss.** BM25 einzubauen ist nicht „`ORDER BY rank` ergänzen". Es
verlangt, die Abfrage umzubauen, damit der Wert nach aussen kommt, **und**
eine Antwort auf die Frage, welchen Rang eine Notiz bekommt, deren Suche
teils über `LIKE` lief. Jede Antwort darauf ist erfunden. Die Suche nach
„KI Backup" mischt dann einen gerechneten mit einem erfundenen Wert.

### 4.4 Weitere, kleinere Befunde

**IDF-Abschneidung bei häufigen Begriffen.** `fts5_aux.c:669–674`:

```c
        ** The problem with this is that if (N < 2*nHit), the IDF is
        ** negative. Which is undesirable. So the minimum allowable IDF is
        ** (1e-6) ...  */
        double idf = log( (nRow - nHit + 0.5) / (nHit + 0.5) );
        if( idf<=0.0 ) idf = 1e-6;
```

**Rechnung:** Die Klammer greift, sobald ein Begriff in mehr als der Hälfte
der Notizen vorkommt — dann fällt sein Beitrag nicht auf „klein", sondern auf
praktisch **null**, eine Stufe statt eines Übergangs. Über einem
Teilstring-Index wird diese Schwelle leichter erreicht als über einem
Wortindex, weil ein Dreizeichen-Teilstring häufiger ist als ein
Dreizeichen-Wort. **Schluss:** Für Begriffe, die ein Mensch tatsächlich
eintippt, dürfte das selten sein; ich führe es als Randbefund, nicht als
Haupteinwand.

**Kosten der IDF-Berechnung.** `fts5Bm25GetData` ruft je Phrase
`xQueryPhrase` (`fts5_aux.c:658`), was laut `fts5.h:141–145` eine vollständige
zweite Abfrage über die Tabelle ausführt:

> „a query equivalent to: `... FROM ftstable WHERE ftstable MATCH $p ORDER BY
> rowid` … is executed"

Das geschieht einmal je Abfrage, nicht je Zeile (`xSetAuxdata`,
`fts5_aux.c:682`). Dazu kommen `xInstCount`/`xInst` je Trefferzeile.
**Nicht belegt:** wie viel das kostet — siehe 6.1 und den Messvorschlag 7.1.

**Was hier *kein* Problem ist.** Die beiden Konfigurationsfallen greifen bei
Denkzettel nicht: `columnsize` ist nicht gesetzt, also Vorgabe 1 — die Doku
warnt nur für `columnsize=0`, dass `xColumnSize` „still works, but runs much
more slowly". Und `detail` ist nicht gesetzt, also `full` — bei
`detail=none`/`detail=column` wären Phrasenabfragen gar nicht verfügbar und
`xInstCount`/`xInst` „quite slow" (`fts5.h:115–118`, `132–133`).

### 4.5 Der Konflikt, der über allem steht

**Beobachtung.** SPEC 6, letzter Punkt, hält bereits eine Entscheidung fest:

> „Die Trefferliste behält die Ordnung der Bibliothek (neueste zuerst, 9.)
> statt der FTS5-Relevanzsortierung — nur so trägt sie deren Tagesgruppen."

Der Kommentar in `store.cpp:444–447` wiederholt sie.

**Schluss.** BM25 ist eine **Relevanzsortierung**. Sie einzuführen heisst,
die chronologische Ordnung und damit die Tagesgruppen der Trefferliste
aufzugeben — oder eine zweite Ansicht zu bauen. Das ist keine technische
Frage, sondern eine Produktentscheidung, die schon einmal getroffen wurde.
Sie gehört vor jede Umsetzungsdiskussion und ist Sache des Kunden.

---

## 5. Alternativen zur Relevanzsortierung

Geordnet nach meinem Eindruck von Nutzen je Aufwand. Alle drei erhalten die
Tagesgruppen, weil sie die Reihenfolge nicht anfassen.

**a) Fundstelle zeigen statt Reihenfolge ändern.** FTS5 bringt `highlight()`
und `snippet()` mit (Doku 5.1.2/5.1.3). Bei kurzen Notizen ist die Frage
selten „welcher Treffer ist der beste" — die Liste ist überschaubar —,
sondern „warum ist dieser Treffer dabei". **Nicht belegt / Vorbehalt:**
`snippet()` misst seine Länge in **Tokens** mit Obergrenze 64 („This must be
greater than zero and equal to or less than 64"); bei Trigrammen sind 64
Tokens rund 64 **Zeichen**, also ein sehr kurzer Ausschnitt. Ob das reicht,
ist zu prüfen. `highlight()` liefert die ganze Spalte mit Markierungen und
hat diese Grenze nicht.

**b) Semantische Suche über die vorhandenen Embeddings.** SPEC 5.1 und 7.2:
Denkzettel legt je Notiz ein Embedding an (`bge-m3`, lokal über Ollama). BM25
kann Synonyme prinzipiell nicht — „Auto" findet „Wagen" nicht, und daran
ändert kein Parameter etwas. Das ist die Schwäche, an der bei kurzen Notizen
mehr hängt als an der Rangfolge. **Vorbehalt:** eigene Story, eigener
Aufwand, und M5-Sache; ich nenne es als Richtung, nicht als Vorschlag für
jetzt.

**c) Status quo.** Chronologisch mit Tagesgruppen. Bei einigen tausend
Notizen und einer eingegrenzten Suche ist die Trefferliste meist kurz genug,
dass eine Rangfolge wenig hinzufügt. Das ist die geltende Entscheidung und
sie kostet nichts.

---

## 6. Was ich **nicht** belegen konnte

Ausdrücklich benannt, weil eine benannte Lücke mehr wert ist als eine
plausible Herleitung.

**6.1 Keinerlei Erfahrungsberichte zu BM25 *speziell mit* dem
Trigramm-Tokenizer.** Zwei gezielte Suchläufe brachten ausschliesslich
allgemeine FTS5-Einführungen und Beiträge zu Spaltengewichten bei
wortbasierten Tokenizern. Ich habe **keine** Quelle gefunden, die die
Rangfolgequalität von BM25 über einem Trigramm-Index untersucht, bewertet
oder auch nur diskutiert. Meine Aussagen in Abschnitt 3 stützen sich deshalb
allein auf Doku und Quelltext, nicht auf fremde Erfahrung.

**6.2 Keine Zahl zu den Laufzeitkosten.** Ich weiss aus dem Quelltext, dass
BM25 eine zusätzliche vollständige Abfrage je Phrase auslöst (4.4). Wie sich
das zu den in SPEC 6 verzeichneten 9 ms Indexabfrage bei 20 000 Notizen
verhält, ist **ungemessen**. Jede Zahl, die ich hier nennen würde, wäre
geraten.

**6.3 Keine Aussage zur tatsächlichen Trefferqualität.** Abschnitt 4.2 ist
eine Rechnung aus der Formel, keine Beobachtung an echten Notizen. Dass BM25
bei einem Suchbegriff auf „kürzeste Notiz zuerst" hinausläuft, folgt aus der
Formel; **ob** das die Suche schlechter macht, ist ein Urteil und kein Befund.

**6.4 Wie viele Suchen überhaupt mehr als einen Begriff haben.** Bei mehreren
Begriffen kommt der IDF-Anteil wieder ins Spiel und 4.2 greift schwächer.
Nutzungsdaten dazu liegen nicht vor.

**6.5 Das Zusammenspiel von `snippet()` mit Trigrammen.** Die
Token-Obergrenze 64 ist belegt; dass sie bei Trigrammen rund 64 Zeichen
bedeutet, ist meine Ableitung aus 3.3 und **nicht durch eine Quelle bestätigt**.

---

## 7. Vorgeschlagene Messungen — **nicht ausgeführt**

Zwei Messungen würden die offenen Punkte entscheiden. Beide gehören in einen
eigenen Auftrag mit eigener Kundenzusage.

**7.1 Was kostet BM25?** (entscheidet 6.2)
Den vorhandenen Bestandsaufbau aus `tests/spellfixspike.cpp` (20 000 Notizen)
nachnutzen und je Abfrage einmal mit und einmal ohne `ORDER BY rank` messen.
Kosten: ein Testläufer-Build und ein Lauf in der Grössenordnung der
bestehenden Spike-Tests; keine Änderung an Produktivcode.

**7.2 Wird die Trefferliste besser?** (entscheidet 6.3 und 6.4)
Zehn bis fünfzehn echte Suchen des Kunden auf einem echten Bestand, Trefferliste
einmal chronologisch und einmal nach `rank`, beide Listen nebeneinander zur
Beurteilung durch den Kunden. Das ist die einzige Messung, die die eigentliche
Frage beantwortet — sie braucht den Kunden, nicht einen Agenten.

**Reihenfolge:** 7.2 vor 7.1. Trägt die Rangfolge fachlich nicht, ist ihr
Preis gleichgültig.

---

## 8. Quellen

- SQLite FTS5 Extension — <https://sqlite.org/fts5.html>
  (Abschnitte 4.3.4 Trigram-Tokenizer, 5.1.1 bm25(), 5.1.2 highlight(),
  5.1.3 snippet(), 5.2 Sorting by Auxiliary Function Results, 6.11 rank)
- `ext/fts5/fts5_aux.c` — <https://raw.githubusercontent.com/sqlite/sqlite/master/ext/fts5/fts5_aux.c>
  (Zeilen 588–747: `Fts5Bm25Data`, `fts5Bm25GetData`, `fts5Bm25Function`)
- `ext/fts5/fts5.h` — <https://raw.githubusercontent.com/sqlite/sqlite/master/ext/fts5/fts5.h>
  (Zeilen 76–167: `xColumnSize`, `xPhraseCount`, `xInstCount`, `xInst`, `xQueryPhrase`)
- `ext/fts5/fts5_tokenize.c` — <https://raw.githubusercontent.com/sqlite/sqlite/master/ext/fts5/fts5_tokenize.c>
  (Zeilen 1349–1421: `fts5TriTokenize`)
- Projektintern: `SPEC.md` Abschnitte 5.1, 6, 7.2 · `src/store/store.cpp`
  Zeilen 54–58 (Tabellendefinition), 414–470 (`Store::search`)

Die Suchläufe zu Erfahrungsberichten (6.1) lieferten nur allgemeine
FTS5-Einführungen ohne Bezug zur gestellten Frage; sie sind deshalb nicht
einzeln aufgeführt.
