# Research: ready-made solutions for a typo-tolerant search

**Task:** the customer's word of 02.08.2026 (sprint 3 acceptance):

> Das Thema Fuzzy Search soll noch einmal recherchiert werden. Da gibt es doch
> bestimmt schon was Fertiges.
>
> (The subject of fuzzy search is to be researched once more. There is bound to
> be something ready-made for it already.)

The occasion is two documented failure cases (#51, #52): `pruefen` does not
find "prüfen" (spelling variant), `prüfem` does not find "prüfen" (typo on the
neighboring key). Procedure per the tool rule: read, judge, present —
**nothing was installed or executed.**

## Sources reviewed (02.08.2026)

- SQLite project documentation "The Spellfix1 Virtual Table", as of 2025-07-12
  (sqlite.org/spellfix1.html) — read in full
- sqlean documentation `docs/fuzzy.md` (github.com/nalgeon/sqlean) — read in full
- Search results on FTS5 fuzzy approaches (among them sqlite.org/fts5.html,
  field reports)

## Finding 1: there is exactly one "ready-made" thing for our case — spellfix1

**spellfix1** is the official SQLite extension for searching a vocabulary for
similar words. It is built to correct search terms ahead of a full-text
search — exactly our use case.

How it works (from the documentation, not guessed): words are stored twice
internally — as an ASCII transliteration (`k1`; the built-in table already
maps **"ß"→"ss"**) and as a phonetic key (`k2`). A query first looks for
candidates over the phonetic key and then scores them by edit distance; the
output is the 20 best matches together with their distance.

The decisive find for us is the **configurable edit distance `editdist3`**: a
cost table in the database itself defines what each substitution costs —
multi-character capable and UTF-8 capable. The documentation demonstrates
German rules verbatim as its examples:

```sql
INSERT INTO editcost(iLang, cFrom, cTo, iCost) VALUES(0, 'a',  'ä', 5);
INSERT INTO editcost(iLang, cFrom, cTo, iCost) VALUES(0, 'ss', 'ß', 8);
```

With rules like `ue→ü`, `ae→ä`, `oe→ö`, `ss→ß` at costs close to zero,
**the same mechanism solves both open issues**: #52 (typos over the default
costs) and #51 (spelling variants over the cost table).

**The condition attached** (of the kind "condition without which a commitment
does not hold", B9): spellfix1 is **contained in no standard build** — neither
in the SQLite amalgamation nor as a shipped module. It is a single C file in
the SQLite source tree (`ext/misc/spellfix.c`, public domain like SQLite
itself) that applications compile in and register on the database handle. For
us that would mean: take the file into the repository, initialize it on the
SQLite handle of the Qt driver when opening the database. Whether that works
smoothly with the Qt SQLite driver on the target system is **to be verified by
a spike, not assumed** — that is the one open condition.

## Finding 2: the alternatives fall short for stated reasons

- **sqlean-fuzzy** (widely used extension collection): provides only distance
  and phonetic **functions**, ASCII only, no index — every query would have
  to compute over the entire word stock. For our case ("search a large
  vocabulary for close matches") the sqlean documentation itself points **to
  spellfix**. That settles it as a candidate, out of its own mouth.
- **SymSpell ports (C++):** fast, but an external library with its own
  dictionary maintenance outside the database — more to integrate than a
  compiled-in C file, with no gain at our data volume.
- **Trigram approximation on the existing index** (query trigrams
  OR-combined, ranked by number of hits): no new dependency, but ranking
  logic in the application code and, according to field reports, mediocre
  quality for short words — possible as a stopgap, not as a solution.
- **External search services** (Meilisearch and the like): a server process of
  its own — out of proportion for a local tool.

## Finding 3: one integration pitfall is visible already

spellfix1 needs a **word vocabulary**. Our FTS5 index uses the
`trigram` tokenizer — its vocabulary (`fts5vocab`) consists of
three-character fragments, **not of words**. So the word list has to come from
somewhere else: either a second, small FTS5 index with `unicode61` purely as a
word source, or a word table maintained by triggers. That is no obstacle, but
a piece of work that belongs in the estimate — it appeared in none of the
considerations so far.

## Recommendation

**Option A (recommended):** compile spellfix1 in, `editdist3` with a
German cost table, a two-stage search (first as today; if a term yields too
few results, corrected terms are searched afterwards). Covers #51 and
#52 together. Beforehand a small spike that checks the one open condition
(registration on the Qt driver handle) and decides the vocabulary question.

**Option B:** solve #51 only, by expanding the query with variants in the
application code (`pruefen` → additionally search for `prüfen`) — cheap,
deterministic, no dependency; #52 would remain open.

**Option C:** defer until the AI elements (M3) are decided — if the
overlaid semantic search is coming anyway, it could cover typos as well.
Against it: it would need a running model for that; the search has to hold up
without AI too (customer line of 01.08.).

The decision between A, B and C is the customer's; implementing A would be a
story with a spike ahead of it (estimate in planning).
