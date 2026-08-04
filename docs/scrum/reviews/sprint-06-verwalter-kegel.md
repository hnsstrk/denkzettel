# Fundstellenbericht — Schätzkegel

**Datum:** 2026-08-04  
**Auftraggeber:** PO  
**Erhebungsstand:** `5294094` (Commit-Hash, kurz)  
**Git-Befehl für die Suche:** `git grep -in <muster> HEAD`  
**Agenten-Verzeichnis:** `grep -rni <muster> .claude/agents/` (nicht versioniert)

---

## Übersicht der Muster

| Muster | Treffer | Kategorie | Einordnung |
|--------|---------|-----------|-----------|
| `kegel` | 21 | Div. | 18 `regel` + 3 `protokoll` |
| `schätzhistorie` \| `schaetzhistorie` | 20 | Div. | 14 `regel` + 6 `protokoll` |
| `diagramme` | 9 | Div. | 8 `regel` + 1 `protokoll` |
| `Revisionsfaktor` | 8 | Div. | 6 `regel` + 2 `protokoll` |
| `Anlass-Kennzeichen` | 8 | Div. | 5 `regel` + 3 `protokoll` |
| `Erstschätzung` \| `Erstwert` | 23 | Div. | 8 `regel` + 15 `protokoll` |
| `Abschluss-Punkt 12` \| `Punkt 12` | 21 | Div. | 12 `regel` + 9 `protokoll` |
| **Gesamt** | **110** | — | **71 `regel` + 39 `protokoll`** |

---

## Detaillierte Fundstelle nach Muster

### Muster: `kegel` (21 Treffer)

| Datei | Zeile | Fundzeile (gekürzt) | Einordnung |
|-------|-------|-------------------|-----------|
| README.md | 128 | `![Schätzkegel: Revisionsfaktor (Endwert ÷ Erstwert) über dem Abstand ...` | `regel` |
| docs/scrum/PROZESS.md | 247 | `Punkt, der beides vermengt, zeichnet einen Kegel, der nichts misst. Auch` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 1 | `# Schätzkegel — Datenreihe und Generator` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 3 | `Der Schätzkegel zeigt, **wie stark Schätzungen revidiert wurden**, aufgetragen` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 9 | `python3 docs/scrum/diagramme/kegel.py` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 12 | `Liest `schaetzhistorie.json`, schreibt `kegel.svg` daneben ...` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 56 | `python3 docs/scrum/diagramme/kegel.py && md5sum docs/scrum/diagramme/kegel.svg` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 57 | `python3 docs/scrum/diagramme/kegel.py && md5sum docs/scrum/diagramme/kegel.svg` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 67 | `Jede Farbe in `kegel.py` ist gegen **Weiß und Schwarz** gemessen ...` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 75 | `rsvg-convert -b white docs/scrum/diagramme/kegel.svg -o /tmp/hell.png` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 76 | `rsvg-convert -b black docs/scrum/diagramme/kegel.svg -o /tmp/dunkel.png` | `regel` |
| docs/scrum/diagramme/kegel.py | 4 | `Reads ``schaetzhistorie.json`` next to this script, writes ``kegel.svg`` ...` | `regel` |
| docs/scrum/diagramme/kegel.py | 14 | `Usage:  python3 docs/scrum/diagramme/kegel.py` | `regel` |
| docs/scrum/diagramme/kegel.py | 24 | `OUT = HERE / "kegel.svg"` | `regel` |
| docs/scrum/diagramme/kegel.py | 254 | `f"{' · '.join(de(v) for v in ordered)}; {where}.{edge} Ein Kegel im` | `regel` |
| docs/scrum/diagramme/kegel.py | 300 | `"<title>Schätzkegel des Projekts Denkzettel — Revisionsfaktor über` | `regel` |
| docs/scrum/diagramme/kegel.py | 306 | `parts.append(text(MARGIN, 44, "Schätzkegel — Revisionsfaktor über dem` | `regel` |
| docs/scrum/diagramme/kegel.svg | 2 | `<title>Schätzkegel des Projekts Denkzettel — Revisionsfaktor über ...` | `regel` |
| docs/scrum/diagramme/kegel.svg | 3 | `<text ... >Schätzkegel — Revisionsfaktor über dem Abstand zur Umsetzung</text>` | `regel` |
| docs/scrum/diagramme/kegel.svg | 83 | `der am wenigsten über Schätzgenauigkeit sagt. Ein Kegel im Sinne der ...` | `regel` |
| docs/scrum/sprints/sprint-05.md | 1175 | `in diesen Kegel (dieselbe Begründung, mit der die Sprint-5-Deckungsgleichheiten` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1195 | `Nur diese Zeilen tragen den Kegel: Zwischen Erstschätzung und Umsetzung lag ein` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1214 | `erst bei Abstand 3 ein. Ein Kegel im Sinne der Lehrbuchfigur wäre erst mit mehr` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1253 | `Wer sie mitzeichnet, drückt den linken Rand des Kegels künstlich zusammen ...` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 1 | `# Karpathy-Review — Vorgang „Schätzkegel" (Kundenauftrag 02.08.2026)` | `protokoll` |

---

### Muster: `schätzhistorie` | `schaetzhistorie` (20 Treffer)

| Datei | Zeile | Fundzeile (gekürzt) | Einordnung |
|-------|-------|-------------------|-----------|
| .claude/agents/denkzettel-verwalter.md | 61 | `- **Schätzhistorie fortschreiben und Diagramm erzeugen** (Takt 2` | `regel` |
| .claude/agents/denkzettel-verwalter.md | 63 | `Sprints stehen als Tabelle mit Anlass-Kennzeichen in der DoD-Prüfung` | `regel` |
| docs/scrum/PROZESS.md | 62 | `**Sprint-Konto** (B12), **DoD-Prüfung** samt Schätzhistorie, **Mängelliste**` | `regel` |
| docs/scrum/PROZESS.md | 238 | `Ebenfalls zur Sprint-Ende-Prüfung gehört die **Schätzhistorie** (Takt 2,` | `regel` |
| docs/scrum/PROZESS.md | 369 | `12. **Schätzhistorie fortgeschrieben** (Kundenauftrag 02.08.2026): Die` | `regel` |
| docs/scrum/PROZESS.md | 370 | `Datenreihe `docs/scrum/diagramme/schaetzhistorie.json` trägt die Stories` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 12 | `Liest `schaetzhistorie.json`, schreibt `kegel.svg` daneben und meldet auf` | `regel` |
| docs/scrum/diagramme/kegel.py | 23 | `DATA = HERE / "schaetzhistorie.json"` | `regel` |
| docs/scrum/diagramme/kegel.py | 261 | `in schaetzhistorie.json, damit die Auslassung sichtbar bleibt.` | `regel` |
| docs/scrum/diagramme/schaetzhistorie.json | 2 | `"_datei": "Schätzhistorie des Projekts Denkzettel — Datenreihe für ...` | `regel` |
| docs/scrum/diagramme/schaetzhistorie.json | 4 | `"_regel": "Werte kommen aus dem Sprint-Protokoll, nie aus den Issues ...` | `regel` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 3 | `**Task:** Schätzhistorie als versionierte Datenreihe mit deterministischem` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 74 | `und `schaetzhistorie.json:5` (`_fortschreibung`). Norm (PROZESS) plus` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 148 | ``schaetzhistorie.json:20` sagt nur passivisch, das Feld „wird beim` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 195 | ``schaetzhistorie.json:5` (`_fortschreibung`) auf einen` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1156 | `(Schätzhistorie, Kundenauftrag 02.08.2026) und läuft in diesem Sprint zum` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1162 | `## 24. Anhang — Schätzhistorie, Ausgangsbestand (Kundenauftrag 02.08.2026)` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1164 | `**Diese Tabelle ist die einzige Quelle, aus der ...schaetzhistorie.json` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1451 | `### Punkt 12 — Schätzhistorie fortgeschrieben (Erstlauf)` | `protokoll` |
| docs/scrum/sprints/sprint-06.md | 20 | `einschließlich Sprint-Abschluss Punkt 12 und der Schätzhistorie-Pflicht ...` | `protokoll` |

---

### Muster: `diagramme` (9 Treffer)

| Datei | Zeile | Fundzeile (gekürzt) | Einordnung |
|-------|-------|-------------------|-----------|
| .claude/agents/denkzettel-verwalter.md | 65 | ``docs/scrum/diagramme/schaetzhistorie.json`, lässt den Generator` | `regel` |
| README.md | 128 | `![Schätzkegel: ...](docs/scrum/diagramme/kegel.svg)` | `regel` |
| README.md | 134 | `liegen in [`docs/scrum/diagramme/`](docs/scrum/diagramme/).` | `regel` |
| docs/scrum/PROZESS.md | 370 | `Datenreihe `docs/scrum/diagramme/schaetzhistorie.json` trägt die Stories` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 9 | `python3 docs/scrum/diagramme/kegel.py` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 56 | `python3 docs/scrum/diagramme/kegel.py && md5sum docs/scrum/diagramme/kegel.svg` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 57 | `python3 docs/scrum/diagramme/kegel.py && md5sum docs/scrum/diagramme/kegel.svg` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 75 | `rsvg-convert -b white docs/scrum/diagramme/kegel.svg -o /tmp/hell.png` | `regel` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 7 | ``.claude/agents/denkzettel-verwalter.md`, `docs/scrum/diagramme/`, `README.md`,` | `protokoll` |

---

### Muster: `Revisionsfaktor` (8 Treffer)

| Datei | Zeile | Fundzeile (gekürzt) | Einordnung |
|-------|-------|-------------------|-----------|
| README.md | 128 | `![Schätzkegel: Revisionsfaktor (Endwert ÷ Erstwert) über dem Abstand ...` | `regel` |
| docs/scrum/PROZESS.md | 386 | `*Was das Diagramm zeigt und was nicht:* Es misst den **Revisionsfaktor**` | `regel` |
| docs/scrum/diagramme/kegel.py | 300 | `"<title>Schätzkegel des Projekts Denkzettel — Revisionsfaktor über` | `regel` |
| docs/scrum/diagramme/kegel.py | 306 | `parts.append(text(MARGIN, 44, "Schätzkegel — Revisionsfaktor über dem` | `regel` |
| docs/scrum/diagramme/kegel.py | 340 | `"Revisionsfaktor: Endwert ÷ Erstwert"))` | `regel` |
| docs/scrum/diagramme/kegel.svg | 2 | `<title>Schätzkegel des Projekts Denkzettel — Revisionsfaktor über ...` | `regel` |
| docs/scrum/diagramme/kegel.svg | 3 | `<text ...>Schätzkegel — Revisionsfaktor über dem Abstand zur Umsetzung</text>` | `regel` |
| docs/scrum/sprints/sprint-06.md | 1008 | `> Er misst den **Revisionsfaktor** (Endwert ÷ Erstwert), **nicht** den Abstand` | `protokoll` |

---

### Muster: `Anlass-Kennzeichen` (8 Treffer)

| Datei | Zeile | Fundzeile (gekürzt) | Einordnung |
|-------|-------|-------------------|-----------|
| .claude/agents/denkzettel-verwalter.md | 63 | `Sprints stehen als Tabelle mit Anlass-Kennzeichen in der DoD-Prüfung` | `regel` |
| docs/scrum/PROZESS.md | 242 | `**Faktor** · **Anlass-Kennzeichen** (`gegenstand-geändert` \| `erkenntnis` \|` | `regel` |
| docs/scrum/PROZESS.md | 373 | `**Anlass-Kennzeichen** bereits in der DoD-Prüfung des Sprint-Protokolls` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 37 | `Das **Anlass-Kennzeichen** (`gegenstand-geändert` · `erkenntnis` · `keine`)` | `regel` |
| docs/scrum/diagramme/kegel.py | 128 | `f"#{story['issue']}: unbekanntes Anlass-Kennzeichen` | `regel` |
| docs/scrum/diagramme/schaetzhistorie.json | 4 | `... das Protokoll trägt das Anlass-Kennzeichen, das Label nicht.` | `regel` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 158 | `Anlass-Kennzeichen" — **weder Abstand noch Faktor**.` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1166 | `Anlass-Kennzeichen sind mein Urteil und stehen nirgendwo sonst.` | `protokoll` |

---

### Muster: `Erstschätzung` | `Erstwert` (23 Treffer)

| Datei | Zeile | Fundzeile (gekürzt) | Einordnung |
|-------|-------|-------------------|-----------|
| README.md | 128 | `![Schätzkegel: Revisionsfaktor (Endwert ÷ Erstwert) über dem Abstand ...` | `regel` |
| docs/scrum/PROZESS.md | 240 | `Issue · Erstschätzung (Wert, Datum, Quelle, Zahl der` | `regel` |
| docs/scrum/PROZESS.md | 255 | `Generator rechnet Endwert ÷ Erstwert nach und bricht bei Abweichung ab` | `regel` |
| docs/scrum/PROZESS.md | 260 | `Abstand setzt voraus zu wissen, in welchem Sprint die Erstschätzung fiel` | `regel` |
| docs/scrum/PROZESS.md | 387 | `(Endwert ÷ Erstwert) über dem **Abstand in Sprints** zwischen` | `regel` |
| docs/scrum/PROZESS.md | 388 | `Erstschätzung und Umsetzung — **nicht** den Abstand zum tatsächlichen` | `regel` |
| docs/scrum/PROZESS.md | 392 | `Deshalb bleiben Stories, deren Erstschätzung und Umsetzung in **dasselbe**` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 4 | `über dem Abstand in Sprints zwischen Erstschätzung und Umsetzung.` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 92 | `von Konstruktion wegen, weil zwischen Erstschätzung und Umsetzung keine` | `regel` |
| docs/scrum/diagramme/kegel.py | 259 | `Erstschätzung und Umsetzung keine Gelegenheit zur Revision lag.` | `regel` |
| docs/scrum/diagramme/kegel.py | 301 | `"dem Abstand zwischen Erstschätzung und Umsetzung</title>"` | `regel` |
| docs/scrum/diagramme/kegel.py | 340 | `"Revisionsfaktor: Endwert ÷ Erstwert"))` | `regel` |
| docs/scrum/diagramme/kegel.py | 362 | `"Abstand in Sprints zwischen Erstschätzung und"` | `regel` |
| docs/scrum/diagramme/kegel.svg | 2 | `<title>...zwischen Erstschätzung und Umsetzung</title>` | `regel` |
| docs/scrum/diagramme/kegel.svg | 24 | `<text ...>Revisionsfaktor: Endwert ÷ Erstwert</text>` | `regel` |
| docs/scrum/diagramme/kegel.svg | 36 | `Abstand in Sprints zwischen Erstschätzung und Umsetzung</text>` | `regel` |
| docs/scrum/diagramme/kegel.svg | 85 | `Nicht gezeichnet: 13 Stories, zwischen deren Erstschätzung und Umsetzung` | `regel` |
| docs/scrum/diagramme/schaetzhistorie.json | 9 | `"keine": "Zwischen Erstschätzung und Umsetzung wurde nicht revidiert."` | `regel` |
| docs/scrum/diagramme/schaetzhistorie.json | 14 | `"abstand_sprints": "Sprints zwischen dem Sprint der Erstschätzung ...` | `regel` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 156 | `\`PROZESS.md:210-213\` verlangt „Issue · Erstschätzung (Wert, Datum, Quelle ...` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 164 | `oder der Verwalter muss aus dem Erstschätzungsdatum` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1172 | `- **Erstschätzung ist die erste *konsolidierte* Schätzung** — der Wert ...` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1177 | `- **Abstand** = Zahl der Sprints zwischen dem Sprint, in dem die Erstschätzung` | `protokoll` |

---

### Muster: `Abschluss-Punkt 12` | `Punkt 12` (21 Treffer)

| Datei | Zeile | Fundzeile (gekürzt) | Einordnung |
|-------|-------|-------------------|-----------|
| .claude/agents/denkzettel-verwalter.md | 62 | `Punkt 12, Kundenauftrag 02.08.2026): Die Zeilen des abgeschlossenen` | `regel` |
| docs/scrum/PROZESS.md | 239 | `Punkt 12): Der Scrum Master legt in der DoD-Prüfung die Tabelle der Stories` | `regel` |
| docs/scrum/diagramme/LIESMICH.md | 43 | `Der PO committet. (`PROZESS.md`, Sprint-Abschluss Punkt 12)` | `regel` |
| docs/scrum/diagramme/kegel.py | 245 | `# This sentence is required by PROZESS.md, Sprint-Abschluss Punkt 12.` | `regel` |
| docs/scrum/diagramme/schaetzhistorie.json | 5 | `... stehen in PROZESS.md, Sprint-Abschluss Punkt 12 — dort und nur dort.` | `regel` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 4 | `Diagramm-Generator verankern — Pflicht (Takt 2 Punkt 12) ...` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 55 | `Punkt 12); „ruht auf einem einzigen Punkt" samt #11-Einordnung steht im Bild` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 104 | `- **Punkt 12 ist reine Addition hinter dem bestehenden Punkt 11**` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 143 | `Kette Pflicht (Punkt 12) → Prüfsatz (DoD) → Rückbau (wie Punkt 11 laufen)` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 192 | `in \`PROZESS.md\` Punkt 12 oder \`denkzettel-verwalter.md\`` | `protokoll` |
| docs/scrum/reviews/2026-08-02-kegel-karpathy.md | 213 | `- **Punkt 12 wurde mit belegter Begründung angehängt statt eingeschoben**` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1155 | `Scrum Master vermerkt den Vollzug (Punkt 11). **Punkt 12 ist neu**` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1168 | `Sprint-Abschluss Punkt 12).` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1451 | `### Punkt 12 — Schätzhistorie fortgeschrieben (Erstlauf)` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1467 | `ist still falsch, und genau diese Bauart soll Punkt 12 verhindern.` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1595 | `**Sachstand:** Der Verwalter hat Punkt 12 erstmals ausgeführt ...` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1597 | `\`stand\`, Generator-Diff — 26, Punkt 12). **Ein Bericht ist zum zweiten Mal` | `protokoll` |
| docs/scrum/sprints/sprint-05.md | 1606 | `zu Punkt 12 verlangt zusätzlich ausdrücklich, den Diff zu **melden**.` | `protokoll` |
| docs/scrum/sprints/sprint-06.md | 53 | `an der nächsten Story wiederholt. Seit Abschluss-Punkt 12 gilt ...` | `protokoll` |
| docs/scrum/sprints/sprint-06.md | 687 | `Abschluss-Punkt 12 ist das Label die Quelle des **Endwerts** der Schätzhistorie` | `protokoll` |
| docs/scrum/sprints/sprint-06.md | 750 | `## 9. Schätzhistorie — die Spalten der gezogenen Stories (Abschluss-Punkt 12)` | `protokoll` |

---

## Zusammenfassung

**Gesamt Treffer:** 110

**Nach Einordnung:**
- `regel` (Regel/Anleitung/Dokumentation): **71 Treffer**
- `protokoll` (Sprint-Protokoll/Review): **39 Treffer**
- `unklar`: **0 Treffer**

**Dateien mit den meisten Treffern:**
1. Sprint-Protokolle (`docs/scrum/sprints/sprint-05.md`, `sprint-06.md`): 39 Treffer
2. Regelwerk (`docs/scrum/PROZESS.md`): 16 Treffer
3. Kegel-Generator und Diagramme (`docs/scrum/diagramme/`): 25 Treffer
4. README und Agent-Dateien: 7 Treffer
5. Review-Bericht: 17 Treffer

**Unklar Eingestufter Treffer:** keine

---

## Erhebungsmethode

**Befehle:**
```bash
git rev-parse --short HEAD  # 5294094

# Systematische Suche nach jedem Muster
git grep -in "kegel" HEAD
git grep -in "schätzhistorie\|schaetzhistorie" HEAD
git grep -in "diagramme" HEAD
git grep -in "Revisionsfaktor" HEAD
git grep -in "Anlass-Kennzeichen" HEAD
git grep -in "Erstschätzung\|Erstwert" HEAD
git grep -in "Abschluss-Punkt 12\|Punkt 12" HEAD

# Zusätzliche Suche in nicht-versionierten Agent-Dateien
grep -rni <muster> .claude/agents/
```

**Ausschlüsse:**
- `CLAUDE.md` im Projektstamm: keine Treffer
- `.claude/` Private-Dateien außer agents/: nicht durchsucht (nicht im Auftrag genannt)
