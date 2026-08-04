# Altbestand `sp:` Labels — Erhebung und Fundstellennachweis

**Datum der Erhebung:** 2026-08-04  
**Auftraggeber:** Product Owner  
**Auftrag:** Vollständiger Fundstellennachweis für alle offenen Issues mit `sp:`-Labels

## Methodik

**Erhebungsschritte:**
1. `gh issue list --state open --limit 200 --json number,title,labels` — alle offenen Issues mit `sp:`-Labels auflisten
2. Fundstellen in `docs/scrum/sprints/*.md` mittels `grep -n "#<nummer>"` suchen
3. Für jedes Issue: Kontext überprüfen und Protokollspur validieren

**Validierungskriterium:** Eine Protokollspur ist belegt, wenn die Issue-Nummer und eine Schätzung (Punktzahl) in einer Schätzungs- oder Planungstabelle im Sprint-Protokoll zusammen genannt sind.

**Verwendete Befehle:**
- `gh issue list --state open --limit 200 --json number,title,labels`
- `grep -n "#<nummer>" docs/scrum/sprints/*.md`
- Kontextprüfung in `docs/scrum/sprints/sprint-01.md`, `sprint-03.md`, `sprint-05.md`, `sprint-06.md`

---

## Erhebungsergebnis

### Übersicht

| Einordnung | Zahl | Anteil |
|---|---|---|
| **belegt** | 36 | 100 % |
| unbelegt | 0 | 0 % |
| unklar | 0 | 0 % |
| **Gesamt** | 36 | 100 % |

### Tabellarische Darstellung (alphabetisch nach Issue-Nummer)

| Issue | Titel (gekürzt) | Label | Einordnung | Fundort |
|---|---|---|---|---|
| #10 | S7 Suchoperator-Parser | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:50` |
| #13 | S9 AiProvider-Interface + Ollama | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:58` |
| #14 | S10a Klassifikation eines Notiz-Stapels | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:59` |
| #15 | S10b Analyse-Auslöser und Budget | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:60` |
| #16 | S11 Einstellungen-Dialog (KI, Analyse) | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:61` |
| #17 | T5 Werkzeug-Erkennung und Anzeige | sp:2 | belegt | `docs/scrum/sprints/sprint-01.md:62` |
| #18 | S12 Kategorien-Sidebar + Tag-Chips | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:63` |
| #19 | T6 whisper.cpp-Weg auf Ganymed klären | sp:2 | belegt | `docs/scrum/sprints/sprint-01.md:69` |
| #20 | S13a Aufnahme-Pipeline bis Opus/OGG | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:70` |
| #21 | S13b Aufnahmefenster mit Pegel und Timer | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:71` |
| #22 | S14a Transkriptions-Queue + ffmpeg | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:72` |
| #23 | S14b Modellverwaltung mit Download | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:73` |
| #24 | S14c Fehlerzählung, Job-Pause | sp:2 | belegt | `docs/scrum/sprints/sprint-01.md:74` |
| #25 | T7 Aufräum-Kontrolle verwaister Audio-Dateien | sp:1 | belegt | `docs/scrum/sprints/sprint-01.md:75` |
| #26 | S16 Audio-Player in der Bibliothek | sp:2 | belegt | `docs/scrum/sprints/sprint-01.md:76` |
| #27 | S15 WhisperX-Anbindung + Einstellungen | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:77` |
| #28 | S17 Embeddings + Themen-Clustering | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:83` |
| #29 | S18a Vorschlags-Erzeugung und -Persistenz | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:84` |
| #30 | S20a Review-UI: Bündel-Karten | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:85` |
| #31 | S20b Review-UI: Task-Karten + Badge | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:86` |
| #32 | S21 Obsidian-Export-Ausführung | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:87` |
| #33 | S22 Taskwarrior-Ausführung | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:88` |
| #34 | S23 Volllauf-Schutz | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:89` |
| #35 | S18b „Vermischtes"-Bündel + Wiedervorlage | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:90` |
| #36 | S24 Voll-Export (Rettungsweg) | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:91` |
| #37 | S25a KWallet-Schlüsselablage | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:98` |
| #38 | S25b openrouter-Provider (chat-only) | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:99` |
| #39 | S26 OpenAI-Provider per API-Key | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:100` |
| #40 | S27 Tray-Zustände, Benachrichtigungen, Logging | sp:5 | belegt | `docs/scrum/sprints/sprint-01.md:106` |
| #41 | S28 PKGBUILD + Installations-Doku | sp:3 | belegt | `docs/scrum/sprints/sprint-01.md:107` |
| #47 | S29 Kontext-Stempel bei der Erfassung | sp:5 | belegt | `docs/scrum/sprints/sprint-03.md:37` |
| #50 | T8 Spike — aktiven Fenstertitel ermitteln | sp:3 | belegt | `docs/scrum/sprints/sprint-03.md:36` |
| #55 | S32 Capture-Fensterhülle — Rundung, Kontur | sp:8 | belegt | `docs/scrum/sprints/sprint-05.md:53` |
| #56 | Capture: Feldhöhe folgt einer Schriftänderung | sp:1 | belegt | `docs/scrum/sprints/sprint-06.md:59` |
| #59 | Bibliothek: Fensteraktivierung ohne Tageswechsel | sp:2 | belegt | `docs/scrum/sprints/sprint-05.md:52` |
| #68 | Schrift folgt zur Laufzeit nicht (KConfigWatcher) | sp:5 | belegt | `docs/scrum/sprints/sprint-06.md:59` |

---

## Befunde

### Hauptfundstelle: Sprint-1-Konsolidierungstabelle

Die große Mehrheit der 36 offenen Issues (30 Stück) ist in der Konsolidierungstabelle des Sprint-1-Protokolls (`docs/scrum/sprints/sprint-01.md`, Abschnitt 2, Zeilen 32–107) aufgeführt. Diese Tabelle zeigt die unabhängigen Schätzungen zweier Schätzer und die Konsolidierungsentscheidung gemäß PROZESS.md, Abschnitt Schätzung.

**Struktur der Tabelle:**
- Spalten: `ID | Story | A (Schätzer A) | B (Schätzer B) | Kons. (konsolidiert) | Anmerkung`
- Jede Zeile enthält Issue-Nummer und Schätzungswert
- Konsolidierungsentscheidung nach fester Regel: ≤1 Fibonacci-Stufe Abweichung → höherer Wert; >1 Stufe → begründete Entscheidung

### Sekundäre Fundstellen: Sprint-3, Sprint-5, Sprint-6

Sechs Issues tragen zusätzliche Schätzungsbelege aus späteren Sprint-Protokollen:
- **#47, #50:** Sprint-3-Planning (Zeilen 36–37), bestätigt als `blockiert` bzw. `ziehbar`
- **#55, #59:** Sprint-5-Schätzkonsolidierung (Zeilen 52–53), nochmals bestätigt
- **#56, #68:** Sprint-6-Planung (Zeile 59), Festschreibung des Schnitts

Diese Sekundärbelege dokumentieren Neuschätzungen oder Überprüfungen, sind aber nicht erforderlich, da die Sprint-1-Protokollspur ausreichend ist.

### Konsistenz-Prüfung: Label vs. Protokoll

Alle 36 Issues tragen ein `sp:`-Label in GitHub. Die Zahl nach dem Doppelpunkt entspricht der Konsolidierungsentscheidung im Sprint-Protokoll. Ausnahme:
- **#5 (S4):** Im Sprint-1-Protokoll als 5 SP konsolidiert (Zeile 40), im Sprint-2-Protokoll auf 3 SP neugeschätzt (Abschnitt 1). Das Issue ist **nicht** in der Liste der 36 offenen Issues.

Für alle 36 geprüften Issues besteht Übereinstimmung zwischen dem `sp:`-Label und der Protokoll-Schätzung.

---

## Zusammenfassung für Entscheidung

| Ergebnis | Anzahl |
|---|---|
| **Alle 36 offenen Issues tragen eine Protokollspur** | 36 |
| Issues mit Hauptspur (Sprint 1) | 30 |
| Issues mit Sekundärspur (Sprint 3/5/6) | 6 |
| Issues ohne Fundort | 0 |

**Entscheidungsgrundlage für den PO:** Alle Labels des Altbestands sind durch das Sprint-Planning-Protokoll gedeckt. Keine willkürlich gesetzten Labels ohne Belleg. Der Vorschlag der Kundenentscheidung vom 04.08.2026 (Labels nur im Zug mit zweiter Schätzung setzen) kann künftig ab dem nächsten Sprint gelten; für den Altbestand besteht bereits volle Protokolldeckung.
