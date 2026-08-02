# Recherche: Fertige Lösungen für eine tippfehlertolerante Suche

**Auftrag:** Kundenwort vom 02.08.2026 (Abnahme Sprint 3): „Das Thema Fuzzy
Search soll noch einmal recherchiert werden. Da gibt es doch bestimmt schon
was Fertiges." — Anlass sind zwei belegte Fehlfälle (#51, #52): `pruefen`
findet „prüfen" nicht (Schreibvariante), `prüfem` findet „prüfen" nicht
(Tippfehler auf der Nachbartaste). Vorgehen nach der Werkzeug-Regel: lesen,
urteilen, vorlegen — **nichts wurde installiert oder ausgeführt.**

## Gesichtete Quellen (02.08.2026)

- SQLite-Projektdoku „The Spellfix1 Virtual Table", Stand 2025-07-12
  (sqlite.org/spellfix1.html) — vollständig gelesen
- sqlean-Doku `docs/fuzzy.md` (github.com/nalgeon/sqlean) — vollständig gelesen
- Suchlage zu FTS5-Fuzzy-Ansätzen (u. a. sqlite.org/fts5.html,
  Erfahrungsberichte)

## Befund 1: Es gibt genau ein „Fertiges" für unseren Fall — spellfix1

**spellfix1** ist die offizielle SQLite-Erweiterung für die Suche nach
ähnlichen Wörtern in einem Vokabular. Sie ist dafür gebaut, Suchbegriffe
vor einer Volltextsuche zu korrigieren — exakt unser Anwendungsfall.

Arbeitsweise (aus der Doku, nicht vermutet): Wörter werden intern doppelt
abgelegt — als ASCII-Transliteration (`k1`; die eingebaute Tabelle bildet
**„ß"→„ss"** bereits ab) und als phonetischer Schlüssel (`k2`). Eine Anfrage
sucht erst über den phonetischen Schlüssel Kandidaten und bewertet sie dann
per Editierdistanz; Ausgabe sind die 20 besten Treffer samt Distanz.

Der entscheidende Fund für uns ist die **konfigurierbare Editierdistanz
`editdist3`**: Eine Kostentabelle in der eigenen Datenbank legt fest, was
welche Ersetzung kostet — mehrzeichenfähig und UTF-8-fähig. Die Doku führt
als Beispiele wörtlich deutsche Regeln vor:

```sql
INSERT INTO editcost(iLang, cFrom, cTo, iCost) VALUES(0, 'a',  'ä', 5);
INSERT INTO editcost(iLang, cFrom, cTo, iCost) VALUES(0, 'ss', 'ß', 8);
```

Mit Regeln wie `ue→ü`, `ae→ä`, `oe→ö`, `ss→ß` zu Kosten nahe null löst
**derselbe Mechanismus beide offenen Issues**: #52 (Tippfehler über die
Standardkosten) und #51 (Schreibvarianten über die Kostentabelle).

**Die Bedingung dazu** (Bauart „Bedingung, ohne die eine Festlegung nicht
gilt", B9): spellfix1 ist **in keinem Standard-Build enthalten** — weder in
der SQLite-Amalgamation noch als ausgeliefertes Modul. Es ist eine einzelne
C-Datei im SQLite-Quellbaum (`ext/misc/spellfix.c`, Public Domain wie SQLite
selbst), die Anwendungen einkompilieren und am Datenbank-Handle registrieren.
Für uns hieße das: Datei ins Repo übernehmen, beim Öffnen der Datenbank über
das SQLite-Handle des Qt-Treibers initialisieren. Ob das mit dem
Qt-SQLite-Treiber auf dem Zielsystem reibungslos geht, ist **per Spike zu
verifizieren, nicht anzunehmen** — das ist die eine offene Bedingung.

## Befund 2: Die Alternativen fallen begründet ab

- **sqlean-fuzzy** (verbreitete Erweiterungssammlung): liefert nur
  Distanz-/Phonetik-**Funktionen**, nur ASCII, kein Index — jede Anfrage
  müsste den ganzen Wortbestand durchrechnen. Die sqlean-Doku selbst
  verweist für unseren Fall („search a large vocabulary for close
  matches") **auf spellfix**. Damit ist sie als Kandidat aus dem eigenen
  Munde erledigt.
- **SymSpell-Portierungen (C++):** schnell, aber eine externe Bibliothek
  mit eigener Wörterbuch-Pflege außerhalb der Datenbank — mehr
  Integrationsfläche als eine einkompilierte C-Datei, ohne Mehrwert für
  unsere Bestandsgröße.
- **Trigramm-Näherung auf dem vorhandenen Index** (Anfrage-Trigramme
  ODER-verknüpft, Ranking nach Trefferzahl): keine neue Abhängigkeit, aber
  Ranking-Logik im Anwendungscode und laut Erfahrungsberichten mäßige
  Qualität bei kurzen Wörtern — als Notlösung möglich, nicht als Lösung.
- **Externe Suchdienste** (Meilisearch u. ä.): eigener Serverprozess —
  für ein lokales Werkzeug außer Verhältnis.

## Befund 3: Eine Integrationsfalle ist jetzt schon sichtbar

spellfix1 braucht ein **Wortvokabular**. Unser FTS5-Index nutzt den
`trigram`-Tokenizer — sein Vokabular (`fts5vocab`) besteht aus
Drei-Zeichen-Schnipseln, **nicht aus Wörtern**. Die Wortliste muss also
anderswo herkommen: entweder ein zweiter, kleiner FTS5-Index mit
`unicode61` nur als Wortquelle, oder eine per Trigger gepflegte Worttabelle.
Das ist kein Hindernis, aber ein Stück Arbeit, das in die Schätzung gehört —
es stand in keiner bisherigen Überlegung.

## Empfehlung

**Option A (empfohlen):** spellfix1 einkompilieren, `editdist3` mit
deutscher Kostentabelle, zweistufige Suche (erst wie heute; liefert ein
Begriff zu wenig, werden korrigierte Begriffe nachgesucht). Deckt #51 und
#52 gemeinsam. Vorher ein kleiner Spike, der die eine offene Bedingung
prüft (Registrierung am Qt-Treiber-Handle) und die Vokabularfrage
entscheidet.

**Option B:** Nur #51 lösen, per Varianten-Erweiterung der Anfrage im
Anwendungscode (`pruefen` → zusätzlich `prüfen` suchen) — billig,
deterministisch, keine Abhängigkeit; #52 bliebe offen.

**Option C:** Zurückstellen, bis die KI-Elemente (M3) entschieden sind —
falls die überlagerte semantische Suche ohnehin kommt, könnte sie
Tippfehler mit abdecken. Dagegen spricht: Sie bräuchte dafür ein laufendes
Modell; die Suche muss auch ohne KI tragen (Kundenlinie vom 01.08.).

Die Entscheidung zwischen A, B und C liegt beim Kunden; die Umsetzung von A
wäre eine Story mit vorgelagertem Spike (Schätzung im Planning).
