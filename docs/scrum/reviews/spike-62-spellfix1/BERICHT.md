# Spike #62 — spellfix1-Einbindung und Vokabularquelle

**Datum:** 02.08.2026 · **Rechner:** Ganymed · **Zweig:** `spike/62-spellfix1`
**Auftrag:** Issue #62, vier Prüffragen · **Ergebnis:** alle vier beantwortet,
Empfehlung am Ende.

Belege: `lauf.txt` (Messlauf), `ctest.txt` (Gegenprobe Schalter/Lint).
Nachvollziehen:

```sh
cmake -B build/spike62 -S . -DCMAKE_BUILD_TYPE=Debug -DDENKZETTEL_SPIKE_SPELLFIX=ON
cmake --build build/spike62 --target spellfixspike
./build/spike62/bin/spellfixspike
```

**Der Spike ändert kein Produktverhalten.** Der Schalter
`DENKZETTEL_SPIKE_SPELLFIX` steht auf `OFF`; ohne ihn wird `spellfix.c` nicht
einmal übersetzt (`grep -c spellfix build/normal/compile_commands.json` → `0`),
und `ctest` läuft mit denselben sieben Tests grün wie zuvor.

---

## Prüffrage 1 — Einbindung und Registrierung: **trägt**

`ext/misc/spellfix.c` aus SQLite 3.53.4 (3084 Zeilen, Public Domain,
SHA-256 `f679ae6f…dea2`, Herkunft in `third_party/spellfix/HERKUNFT.md`) lässt
sich mit `-DSQLITE_CORE=1` einkompilieren und am Handle des Qt-Treibers
registrieren. Belegt in beide Richtungen, auf einer eigens dafür geöffneten
Verbindung:

```
vor der Registrierung:  no such function: editdist3
nach der Registrierung: editdist3('prüfem', 'prüfen') = 150, SQLite im Prozess: 3.53.4
```

Nicht nur die Funktionen kommen an, auch das Modul: `CREATE VIRTUAL TABLE …
USING spellfix1` gelingt und liefert auf `MATCH 'prufen'` das Wort `prüfen`.

Warum es überhaupt geht — die Voraussetzung, nicht die Vermutung: Das
Qt-Plugin `libqsqlite.so` ist gegen `libsqlite3.so.0` gelinkt, hat SQLite also
nicht statisch eingebaut. Die einkompilierte Datei ruft dieselbe Bibliothek an,
die auch der Treiber benutzt. Wäre das Plugin gegen ein eigenes SQLite
gebunden, gäbe es zwei Bibliotheken im Prozess und die Registrierung ginge ins
Leere. **Das gehört als Bedingung in die SPEC** (DoD 4 in der Fassung nach B9).

**Zweite Bedingung, im Lauf entdeckt: die Registrierung gilt je Verbindung,
nicht je Datei.** Das Testprogramm hatte spellfix1 auf seiner Arbeitsverbindung
schon registriert; auf der zweiten Verbindung zur *selben Datei* meldete SQLite
trotzdem `no such function: editdist3`. Für die Umsetzung heißt das: Die
Registrierung gehört in `Store::open()`, und zwar in jedes `open()` — nicht
einmalig in die Migration.

## Prüffrage 2 — Vokabularquelle: **Weg A**, zweiter FTS5-Index mit `unicode61`

Gemessen am echten Schema (`Store::open()` legt es an, die FTS-Trigger der
Migration 2 laufen mit), Korpus per rekursivem CTE: 20 000 Notizen,
6 259 416 Zeichen Rohtext, 17 030 verschiedene Wörter, 500 000 Wortvorkommen.
Größen aus `dbstat`, also je Datenbankobjekt, nicht aus der Dateigröße.

| Objekt | Größe | Aufbau |
|---|---|---|
| `notes` (Rohtext) | 7,37 MiB | — |
| `notes_fts` (trigram, Bestand heute) | 17,96 MiB | — |
| **A:** `notes_words` (`unicode61`) | **2,13 MiB** | 0,1 s |
| **B:** Worttabelle per Trigger | **0,36 MiB** | 0,5 s |
| `spellfix1`-Tabelle (beide Wege gleich) | 0,93 MiB | 0,1 s |
| Bestand heute (`notes` + `notes_fts`) | 25,33 MiB | — |
| Aufschlag Weg A + spellfix1 | 3,06 MiB | +12,1 % |
| Aufschlag Weg B + spellfix1 | 1,29 MiB | +5,1 % |

Beide Wege liefern dieselben 17 030 Wörter — sonst verglichen die Zahlen zwei
verschiedene Dinge. Eine Korrekturabfrage kostet 0 ms (unter der Messschwelle).

**Die Größe spricht für B, die Entscheidung fällt trotzdem auf A.** Der Grund
ist nicht Geschmack, sondern eine Messung, die es fast nicht gegeben hätte: Der
Korpus oben besteht aus leerzeichengetrennten Wörtern — eine Form, in der der
handgeschriebene Wortsplitter von B gar nicht scheitern *kann*. Also ist eine
einzige Notiz mit echten Satzzeichen nachgeschoben worden
(`Rechnung prüfen: Straße 7 — „dringend"! (siehe E-Mail vom 3.8.)`). Die Wörter,
die beide Wege daraus neu aufnehmen:

```
A (unicode61): 3 | 7 | 8 | dringend | e | mail | siehe | vom
B (Trigger):   (siehe | 3.8.) | 7 | e-mail | prüfen: | vom | — | „dringend"!
```

B nimmt `prüfen:`, `„dringend"!`, `(siehe` und `—` als Vokabeln auf. Das sind
genau die Wörter, die spellfix1 dem Nutzer später als Korrektur anböte. Um das
zu heilen, müsste der Trigger einen Tokenizer in SQL nachbauen — Satzzeichen,
Bindestriche, Ziffern, Groß-/Kleinschreibung über ASCII hinaus. `unicode61`
kann das fertig. Dazu kommt: B braucht eine eigene Referenzzählung, damit ein
Wort verschwindet, wenn die letzte Notiz mit diesem Wort gelöscht wird; A
bekommt Löschen und Ändern von FTS5 geschenkt, mit demselben Dreier-Trigger-
Muster, das `notes_fts` schon benutzt.

1,77 MiB Unterschied bei 20 000 Notizen — gegen einen trigram-Index, der an
derselben Stelle 17,96 MiB kostet. Der Aufpreis für A ist ein Zehntel dessen,
was die Suche heute ohnehin zahlt.

**Bedingung, ohne die Weg A nicht gilt: `remove_diacritics 0`.** Der
Bestandsindex läuft mit `remove_diacritics 1` (SPEC 6). Übernähme der
Wortindex diese Einstellung, stünde `prufen` im Vokabular statt `prüfen` — die
Korrektur böte dem Nutzer ein Wort an, das in keiner Notiz vorkommt. Gehört in
die SPEC.

**Grenze der Messung, ausdrücklich benannt:** Der Korpus ist synthetisch
(Grundwortschatz plus Komposita). Die Zahlen sind *untereinander* vergleichbar,
weil sie in einem Lauf am selben Korpus entstanden sind — gegen die Zahlen in
SPEC 6 sind sie es nicht: Dort misst der trigram-Index 10,9 MiB bei 3,6 MiB
Rohtext, hier 17,96 MiB bei 7,37 MiB. Der Korpus ist doppelt so umfangreich und
hat mehr lange Komposita. Wer die 17,96 gegen die 10,9 hält, vergleicht zwei
Korpora, nicht zwei Indizes.

## Prüffrage 3 — deutsche Kostentabelle: **beide Kundenfälle getroffen**

```sql
CREATE TABLE editcost (iLang INT, cFrom TEXT, cTo TEXT, iCost INT);
INSERT INTO editcost VALUES (0,'ue','ü',1), (0,'ü','ue',1),
                            (0,'ae','ä',1), (0,'ä','ae',1),
                            (0,'oe','ö',1), (0,'ö','oe',1),
                            (0,'ss','ß',1), (0,'ß','ss',1);
INSERT INTO <spellfix-tabelle>(command) VALUES('edit_cost_table=editcost');
```

Jeder Fall einzeln belegt, und jeder zweimal: an einer kleinen Wortliste, wo er
zu sehen ist, und am vollen Vokabular von 17 030 Wörtern, wo er sich gegen
Konkurrenz behaupten muss.

| Fall | Eingabe | Bester Treffer | Distanz (klein / groß) |
|---|---|---|---|
| #51 Schreibvariante | `pruefen` | `prüfen` | 1 / 1 |
| #52 Tippfehler | `prüfem` | `prüfen` | 150 / 150 |

**Der Risikohinweis aus dem Planning traf die richtige Stelle, aber mit der
falschen Annahme.** spellfix1 transliteriert nicht `ü`→`u`, sondern
`ü`→`ue`, `ä`→`ae`, `ö`→`oe`, `ß`→`ss` (Tabelle `translit[]` in `spellfix.c`,
Zeilen 1348, 1353, 1371, 1377). Der Ablauf zerfällt damit in zwei Stufen, die
verschiedene Zeichenketten sehen:

1. **Kandidatensuche** — transliteriert. spellfix1 bildet den phonetischen
   Schlüssel `k2` aus der Transliteration `k1` und sucht darüber (Zeile 2597).
   `pruefen` und `prüfen` haben denselben `k1`, deshalb wird das Wort
   überhaupt gefunden.
2. **Bewertung** — nicht transliteriert. `editDist3Core()` bekommt den
   unveränderten Suchbegriff und die unveränderte Wortspalte (Zeile 2465,
   `pMatchStr3` ist im Quelltext als „Original unicode string" kommentiert).
   Erst hier greift die Kostentabelle.

Deshalb war das Trennen der beiden Fälle keine Formalie: `pruefen` kommt auf
Distanz 1, weil eine Regel greift; `prüfem` auf 150, weil keine greift und die
Standard-Ersetzungskosten zählen. Vom einen auf den anderen zu schließen wäre
schiefgegangen.

**Entdeckte Bedingung: Distanzen sind Kosten, keine Zeichen.** Mit geladener
Kostentabelle gelten Einfügen 100, Löschen 100, Ersetzen 150
(`spellfix.c`, Zeilen 780–782). Ein Schwellwert `distance <= 2`, wie man ihn
von Levenshtein gewohnt ist, würde #52 aussperren. Gehört in die SPEC.

**Zweite Bedingung: Vokabular und Suchbegriff müssen dieselbe Schreibung
haben.** `editdist3` vergleicht die Originale ohne Groß-/Kleinfaltung.
`unicode61` liefert kleingeschriebene Wörter — der Suchbegriff muss vor der
Korrektur ebenso kleingeschrieben werden.

## Prüffrage 4 — Empfehlung für den Schnitt der Umsetzungs-Story

**Eine Story, nicht zwei.** #51 (Schreibvariante) und #52 (Tippfehler) sind
kein getrennter Aufwand mehr: Dieselbe Kostentabelle an derselben
spellfix1-Tabelle löst beide. Sie getrennt zu ziehen hieße, dieselbe Migration
zweimal zu bauen.

Umfang der Story, in der Reihenfolge, in der sie prüfbar ist:

1. **`spellfix.c` fest einbauen** — der Spike-Schalter fällt weg. Registrierung
   in `Store::open()`, je Verbindung, mit **Rückleseprüfung**: Nach
   `sqlite3_spellfix_init()` wird `editdist3` abgefragt, statt dem Rückgabewert
   zu glauben. Schlägt es fehl, muss die Suche weiterlaufen wie heute und der
   Fehlschlag sichtbar gemeldet werden.
2. **Schema-Version 3** — `notes_words` (`unicode61 remove_diacritics 0`) mit
   den drei Triggern und `rebuild`; `editcost` mit den acht Regeln; die
   spellfix1-Tabelle und ihre erste Befüllung aus `fts5vocab`.
3. **Zweistufige Suche in `Store::search()`** — erst der Weg von heute; liefert
   ein Term keinen Treffer, wird der korrigierte Term nachgesucht.
4. **Rückmeldung an den Nutzer** — er muss sehen, dass korrigiert wurde, sonst
   wirkt die Trefferliste falsch. Das ist eine UX-Frage; Entscheidung bei PO
   und `denkzettel-ux`, nicht im Spike.

### Was vor der Schätzung entschieden gehört

- **Nachführung der spellfix1-Tabelle.** Neue Wörter kommen laufend hinzu, und
  spellfix1 kennt kein Trigger-Hook. Der Vollaufbau aus 17 030 Wörtern dauert
  0,1 s — bei jedem Start neu aufzubauen ist also bezahlbar; ein inkrementeller
  Abgleich aus `fts5vocab` wäre sparsamer. **Im Spike nicht geprüft** — das ist
  keine Fußnote, sondern ein offener Punkt der Story.
- **Schwellwert und Zahl der Vorschläge.** Über 150 muss er liegen (siehe
  oben); wie weit darüber, entscheidet, wie oft die Suche etwas findet, das der
  Nutzer nicht gemeint hat. Am Bestand zu messen, nicht zu setzen.
- **Verhältnis zu S30 (ß/ss).** Die Regel `ss`→`ß` steht in derselben
  Kostentabelle, wirkt aber nur auf die **Korrektur**, nicht auf die
  **Indexsuche**: `strassenbahn` fände `Straßenbahn` nur, wenn die Korrektur
  greift, also nur wenn der Term sonst nichts findet. Ob S30 damit erledigt
  oder nur halb erledigt ist, ist im Spike nicht geprüft.

### An den PO gemeldet, nicht selbst geändert

Die fünf oben benannten Bedingungen gehören nach DoD 4 in die SPEC, sobald die
Story gezogen wird: gemeinsame libsqlite3 mit dem Qt-Treiber · Registrierung je
Verbindung · `remove_diacritics 0` für den Wortindex · Distanzen sind Kosten,
kein Zeichenmaß · gleiche Schreibung von Vokabular und Suchbegriff. Der Spike
ändert weder SPEC noch Produktivcode; die Aufnahme entscheidet der PO.

---

## Zustand des Zweigs

| Datei | Was |
|---|---|
| `third_party/spellfix/spellfix.c` | Fremdcode, unverändert, 3084 Zeilen |
| `third_party/spellfix/HERKUNFT.md` | Bezugsquelle, Fassung, Prüfsumme, Lizenz |
| `third_party/spellfix/.clang-tidy` | Lint-Ausschluss für die Fremddatei |
| `src/CMakeLists.txt` | Schalter `DENKZETTEL_SPIKE_SPELLFIX` (Standard `OFF`) |
| `tests/spellfixspike.cpp` | Prototyp, beantwortet Prüffrage 1–3 messend |
| `tests/CMakeLists.txt` | Prototyp nur bei eingeschaltetem Schalter |
| `docs/scrum/reviews/spike-62-spellfix1/` | dieser Bericht, `lauf.txt`, `ctest.txt` |

Nicht gepusht, nicht gemerged — das entscheidet der PO.
