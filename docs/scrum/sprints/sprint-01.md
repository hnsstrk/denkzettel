# Sprint 1 — Planning-Protokoll

**Datum:** 2026-07-31, 19:50 (Ganymed)
**Moderation:** Scrum Master (Agent `scrum-master`)
**Teilnehmer:** Schätzer A (Agent `schaetzer-aufwand`, reiner Implementierungs-
aufwand) · Schätzer B (Agent `schaetzer-risiko`, Risiko und Integration) ·
Scrum Master · Product Owner (Claude Haupt-Session)
**Status des Sprint-Vorschlags:** wartet auf Kundenfreigabe

Grundlagen: `SPEC.md` (Stand 31.07.2026, um die PO-Entscheidungen der Klausur
ergänzt), `docs/scrum/BACKLOG.md` (Entwurf, 28 Stories), `docs/scrum/PROZESS.md`.
Beide Schätzberichte lagen unabhängig voneinander vor; keiner der Schätzer
kannte die Zahlen des anderen.

## 1. Konsolidierungsregel

Nach PROZESS.md: Abweichung ≤ 1 Fibonacci-Stufe → höherer Wert. Größere
Abweichung → begründete Entscheidung anhand der vorgetragenen Argumente.
13er-Stories werden vor dem Ziehen geteilt; hier wurden zusätzlich alle 8er
geteilt, weil beide Schätzer sie unabhängig als zweigeteilt beschrieben haben.

Abweichungen von der Regel „höherer Wert" gibt es nur dort, wo eine
PO-Entscheidung den Umfang der Story verändert hat — diese Fälle sind unter
Abschnitt 3 einzeln begründet.

## 2. Konsolidierte Schätzung

Spalten A und B sind die unabhängigen Werte; bei geteilten Stories steht der
Wert der Ursprungsstory in Klammern.

### Meilenstein 1 — Capture-Kern

| ID | Story | A | B | Kons. | Anmerkung |
|----|-------|---|---|-------|-----------|
| T1 | Wayland-Fokus-Spike | – | – | **2** | neu, siehe 4.1 |
| S1 | Projektgerüst mit Tray | 3 | 3 | **3** | einig |
| S2 | SQLite-Store (M1-Schema) | 5 | 3 | **3** | Entscheidung E2 |
| S3 | Text-Capture-Fenster | 5 | 5 | **5** | einig; Neuschätzung nach T1 (E7) |
| S4 | Globales Kürzel + D-Bus | 3 | 5 | **5** | Regel: höherer Wert; Neuschätzung nach T1 (E7) |
| T2 | Autostart und Erststart | – | – | **2** | neu, siehe 4.2 |

### Meilenstein 2 — Bibliothek und Suche

| ID | Story | A | B | Kons. | Anmerkung |
|----|-------|---|---|-------|-----------|
| S5 | Bibliotheksfenster mit Liste und Detail | 5 | 3 | **5** | Regel: höherer Wert |
| S6 | Volltextsuche (FTS5) | 3 | 3 | **3** | einig |
| T3 | Migrationstest Schemaversion 1→2 | – | – | **1** | neu, siehe 4.3 |
| S7 | Suchoperator-Parser | 3 | 2 | **3** | Regel: höherer Wert |
| S8 | Bearbeiten-Ansicht | 2 | 2 | **2** | Entscheidung E3 (bleibt eigenständig) |

### Meilenstein 3 — KI-Basis

| ID | Story | A | B | Kons. | Anmerkung |
|----|-------|---|---|-------|-----------|
| T4 | Ollama-Modelle auf Ganymed | – | – | **1** | neu, siehe 4.4 |
| S9 | AiProvider-Interface + Ollama | 5 | 5 | **5** | einig |
| S10a | Klassifikation eines Notiz-Stapels | (8) | (8) | **5** | Teilung, Entscheidung E6 |
| S10b | Analyse-Auslöser und Budget | (8) | (8) | **3** | Teilung, Entscheidung E6 |
| S11 | Einstellungen-Dialog (KI, Analyse) | 3 | 3 | **3** | einig |
| T5 | Werkzeug-Erkennung und Anzeige | – | – | **2** | neu, siehe 4.5 |
| S12 | Kategorien-Sidebar + Tag-Chips | 3 | 2 | **3** | Regel: höherer Wert |

### Meilenstein 4 — Sprachnotizen

| ID | Story | A | B | Kons. | Anmerkung |
|----|-------|---|---|-------|-----------|
| T6 | whisper.cpp-Weg auf Ganymed klären | – | – | **2** | neu, siehe 4.6 |
| S13a | Aufnahme-Pipeline bis Opus/OGG | (8) | (8) | **5** | Teilung nach B |
| S13b | Aufnahmefenster mit Pegel und Timer | (8) | (8) | **3** | Teilung nach B |
| S14a | Transkriptions-Queue + ffmpeg + whisper-cli | (8) | (13) | **5** | Teilung, Entscheidung E5 |
| S14b | Modellverwaltung mit Download | (8) | (13) | **3** | Teilung, Entscheidung E5 |
| S14c | Fehlerzählung, Job-Pause, Tray-Kopplung | (8) | (13) | **2** | Teilung, Entscheidung E5 |
| T7 | Aufräum-Kontrolle verwaister Audio-Dateien | – | – | **1** | neu, siehe 4.7 |
| S16 | Audio-Player in der Bibliothek | 2 | 2 | **2** | einig |
| S15 | WhisperX-Anbindung + Einstellungen-Seite | 3 | 8 | **3** | Entscheidung E1 — **blockiert** |

### Meilenstein 5 — Vorschläge

| ID | Story | A | B | Kons. | Anmerkung |
|----|-------|---|---|-------|-----------|
| S17 | Embeddings + Themen-Clustering | 5 | 5 | **5** | einig |
| S18a | Vorschlags-Erzeugung und -Persistenz (Bündel + Task) | (8) | (8) | **5** | Teilung + Entscheidung E4 |
| S20a | Review-UI: Bündel-Karten | (8) | (5) | **5** | Teilung nach A |
| S20b | Review-UI: Task-Karten + Badge | (8) | (5) | **3** | Teilung nach A |
| S21 | Obsidian-Export-Ausführung | 5 | 5 | **5** | einig |
| S22 | Taskwarrior-Ausführung | 3 | 5 | **5** | Regel: höherer Wert |
| S23 | Volllauf-Schutz | 3 | 3 | **3** | einig |
| S18b | „Vermischtes"-Bündel + Wiedervorlage | (8) | (8) | **3** | Teilung nach B; fachlich an S23 |
| S24 | Voll-Export (Rettungsweg) | 3 | 2 | **3** | Regel: höherer Wert |
| ~~S19~~ | ~~Task-Vorschläge~~ | 2 | 2 | **–** | aufgelöst in S18a, Entscheidung E4 |

### Meilenstein 6 — Provider-Ausbau

| ID | Story | A | B | Kons. | Anmerkung |
|----|-------|---|---|-------|-----------|
| S25a | KWallet-Schlüsselablage | (5) | (8) | **3** | Teilung, beide Berichte einig |
| S25b | openrouter-Provider (chat-only) | (5) | (8) | **3** | Teilung; AK anzupassen (5.2) |
| S26 | OpenAI-Provider per API-Key | 2 | 3 | **3** | Regel: höherer Wert |

### Meilenstein 7 — Politur

| ID | Story | A | B | Kons. | Anmerkung |
|----|-------|---|---|-------|-----------|
| S27 | Tray-Zustände, Benachrichtigungen, Logging | 5 | 5 | **5** | einig |
| S28 | PKGBUILD + Installations-Doku | 3 | 3 | **3** | einig |

### Summen

| Größe | Wert |
|---|---|
| Bestandsstories nach Neuschnitt | 34 Stories, 125 SP |
| Neue Tech-/Vorbedingungs-Stories | 7 Stories, 11 SP |
| **Gesamt** | **41 Stories, 136 SP** |
| Rohsumme Schätzer A | 116 SP (28 Stories) |
| Rohsumme Schätzer B | 129 SP (28 Stories) |

Die konsolidierte Summe liegt über beiden Einzelsummen. Zwei Ursachen, beide
erklärbar: In zehn Fällen greift die Regel „höherer Wert", und 11 SP entfallen
auf Vorbedingungs-Stories, die in keinem der beiden Berichte als Backlog-
Position mitgezählt waren. Bei ~13 SP je Sprint entspricht das rund elf
Sprints — ohne S15, die blockiert geführt wird.

## 3. Entscheidungsfälle

Sieben Fälle, in denen der Scrum Master nicht mechanisch nach Regel
konsolidieren konnte. Nur einer davon (E1) ist eine echte Schätzabweichung von
mehr als einer Stufe; die übrigen sind Schnitt- oder Scope-Entscheidungen.

**E1 — S15 WhisperX (A: 3, B: 8, konsolidiert 3).** Der einzige Fall mit
Abweichung > 1 Stufe. A schätzt den reinen Bauaufwand: zweites Backend hinter
einer bereits stehenden Abstraktion plus eine Einstellungsseite. B schätzt 8,
weil das Fundament fehlt — WhisperX ist auf Ganymed weder im PATH noch im
`backend_env` des RPG-Audio-Projekts auffindbar, und das Akzeptanzkriterium
verlangt einen Lauf gegen genau diese Installation. Entscheidung: **3 SP**,
weil der Aufschlag von B keine Bauarbeit beschreibt, sondern eine fehlende
Vorbedingung — und die ist inzwischen in SPEC 12 als solche geführt (die
Anbindung wird erst gebaut, wenn die Installation existiert). Die Story wird
**blockiert** geführt und nicht gezogen, solange die Vorbedingung offen ist.
Die von beiden genannte Reibung „Backend-Wechsel zur Laufzeit" ist in den 3 SP
enthalten.

**E2 — S2 SQLite-Store (A: 5, B: 3, konsolidiert 3).** Die Regel ergäbe 5. Der
PO hat A-Anmerkung 9 angenommen: Das Schema wächst je Meilenstein, statt in M1
schon `proposals`, `embeddings` und `transcribe_jobs` anzulegen, die erst in
M4/M5 gebraucht werden. Damit umfasst S2 nur noch `notes`, `tags` und `meta`
samt Migrationslogik, CRUD und Lösch-Transaktion. A nennt für genau diesen
reduzierten Umfang selbst 3 SP; B lag mit 3 bereits richtig. Nebenwirkung: Die
Migrationslogik wird früh real benutzt statt nur gebaut — deshalb T3.

**E3 — S8 Bearbeiten-Ansicht (beide 2, bleibt eigenständig).** Beide Schätzer
schlugen die Auflösung vor, aber aus einem Grund, der inzwischen entfallen ist:
A („Vorbedingung, die noch nirgends gebaut wird") und B („trägt eine
Schemaerweiterung, die nach S2 gehört") zielten beide auf das fehlende
Re-Embedding-Kennzeichen. `needs_reembed` steht jetzt in SPEC 5.1 und kommt mit
dem M2-Schemaschritt. Entscheidung: S8 bleibt eigenständig mit **2 SP** — eine
Auflösung in S5 würde diese auf 7 SP treiben und damit über die Teilungsgrenze
heben, ohne dass die beiden Ansichten Code teilen.

**E4 — S19 Task-Vorschläge (beide 2, aufgelöst).** Hier trugen beide Berichte
dasselbe Argument vor: Die Task-Felder entstehen bereits im
Klassifikationsaufruf (S10a), übrig bleibt reine Persistenz ohne eigene
Außenfläche. Entscheidung: **S19 wird aufgelöst**, die Task-Payload wandert in
S18a, die damit die `proposals`-Tabelle für beide `kind`-Werte einmal
verantwortet statt zweimal. S18a bleibt bei 5 SP: Der Zuwachs ist ein zweiter
Payload-Typ auf derselben Tabelle, die Feldextraktion liefert S10a.

**E5 — S14 Transkription (A: 8, B: 13, Teilung in drei).** Die Regel ergäbe 13
und damit Teilungspflicht. A teilt zweifach (Queue+whisper 5 / Download 3), B
dreifach (Queue+ffmpeg+whisper 5 / Modellverwaltung 3 / Fehlerpfad+Tray 2).
Entscheidung: **B's Dreiteilung**, weil sie den Fehlerpfad sichtbar macht, der
sonst am Ende der größten Story mitläuft und dort erfahrungsgemäß gekürzt wird.
Summe der Teile 10 SP statt 13 — der Aufschlag von B kam wesentlich aus der
fehlenden Werkzeugkette (kein aufrufbares `whisper-cli`, keine GGML-Datei), und
die ist jetzt als T6 eigenständig gebucht statt in der Story versteckt.

**E6 — S10 Klassifikations-Lauf (beide 8, gegenläufige Aufteilung).** Beide
teilen an derselben Naht, verteilen die Punkte aber gegenläufig: A gibt dem
Orchestrator 5 und dem Prompt 3, B der Klassifikation 5 und dem Scheduler 3.
Entscheidung: **B's Verteilung** — S10a Klassifikation (Prompt, JSON-Schema,
Antwort-Parsing samt Thinking-Block-Robustheit bei qwen3-Familien, Persistenz
von Kategorie/Tags/is_todo/Task-Feldern, Fehlerzählung je Notiz) 5 SP; S10b
Auslöser sofort/periodisch/Abruf mit Budget 50 und `AnalyzeNow()` 3 SP. Die
Fehlerzählung (`analysis_attempts`) sitzt an der Notiz und wird beim
Klassifikationsaufruf hochgezählt — sie gehört zu S10a, nicht zum Scheduler.
Ausschlaggebend: B's Beobachtung, dass die beiden Hälften an völlig
verschiedenen Dingen scheitern; das Prompt-Risiko ist das größere und gehört
in die größere Story.

**E7 — Wayland-Anteil in S3, S4 und S13b (Doppelzählung vermieden).** Beide
Berichte nennen den Wayland-Fokus als Querschnittsrisiko, das drei Stories
gleichzeitig treffen kann. Der Spike T1 klärt ihn einmal. Entscheidung: Die
konsolidierten Werte für S3 (5) und S4 (5) **bleiben zunächst stehen** — den
Aufschlag jetzt schon abzuziehen, hieße ein Spike-Ergebnis vorwegzunehmen, das
auch negativ ausfallen kann (dann wird S3 teurer, nicht billiger). Trägt der
xdg-activation-Weg, sind S3 und S4 beim Sprint-2-Planning **neu zu schätzen**;
die Erwartung des Scrum Masters liegt dann bei je 3 SP.

## 4. Neue Tech- und Vorbedingungs-Stories

Sieben Stories, die im Backlog-Entwurf fehlten und aus den Lückenlisten beider
Berichte sowie den Spec-Ergänzungen folgen.

**4.1 — T1 Wayland-Fokus-Spike (M1, 2 SP).** Zeitlich begrenzter Durchstich,
der belegt, wie ein rahmenloses Fenster unter Plasma/Wayland aus einer
KGlobalAccel-Aktion heraus erscheint und sofort Tastatureingaben empfängt —
vorrangig über ein XDG-Activation-Token, Fallbacks KWin-Regel oder
Layer-Shell. Ergebnis: lauffähiger Prototyp plus schriftlicher Befund, welcher
Weg trägt.

**4.2 — T2 Autostart und Erststart (M1, 2 SP).** XDG-Autostart-Eintrag für
`denkzetteld` (vom Paket installiert, in den Systemeinstellungen abschaltbar)
und ein Erststart, der Datenverzeichnis, DB mit aktueller Schemaversion und
Default-Konfiguration anlegt (SPEC 2.5). Ohne laufenden Dienst gibt es keine
Kürzel — Grundfunktion, keine Politur.

**4.3 — T3 Migrationstest Schemaversion 1→2 (M2, 1 SP).** Test, der eine
Bestands-DB der M1-Schemaversion auf die M2-Version hebt und Datenerhalt prüft
(SPEC 16). Wird zusammen mit S6 gezogen, weil die FTS5-Tabelle samt Triggern
die erste reale Migration ist.

**4.4 — T4 Ollama-Modelle auf Ganymed (M3, 1 SP).** `ollama pull qwen3:8b` und
`bge-m3`; Erreichbarkeit sowie Antwortformat von `/api/chat` und `/api/embed`
einmal von Hand belegt. Vorbedingung für S9 und S10a — beide haben
Akzeptanzkriterien, die genau diese Modelle voraussetzen.

**4.5 — T5 Werkzeug-Erkennung und Anzeige (M3, 2 SP).** Laufzeit-Erkennung von
ffmpeg, `whisper-cli`, `task` und Ollama-Erreichbarkeit; Fehlendes wird in den
Einstellungen benannt (SPEC 2.5 und 15). Hängt an S11.

**4.6 — T6 whisper.cpp-Weg auf Ganymed klären (M4, 2 SP).** Entscheidung
zwischen AUR-Vulkan-Paket und dem vorhandenen, selbstgebauten HIP/ROCm-Baum
unter `~/Projekte/rpg-audio-studio/models/whisper.cpp`; Ergebnis ist ein
aufrufbares `whisper-cli` und eine abgelegte GGML-Modelldatei (`small`), SPEC 12
wird nachgezogen. Vorbedingung für S14a.

**4.7 — T7 Aufräum-Kontrolle verwaister Audio-Dateien (M4, 1 SP).** Beim
Dienststart werden Audio-Dateien ohne DB-Verweis entfernt und im Log vermerkt
(SPEC 2.5) — der einzige Fall zulässiger Selbstheilung im Projekt, weil
eindeutig, harmlos und wiederkehrend.

## 5. Hinweise an den Product Owner

**5.1 — Verbindungstest doppelt vergeben.** Die Logik sitzt in S9, der Knopf in
S11; beide Akzeptanzkriterien nennen ihn. Vorschlag: In S9 „Testfunktion mit
Latenzmessung als aufrufbare Methode", in S11 „Knopf ruft die Testfunktion und
zeigt Latenz oder Fehler".

**5.2 — S25b Akzeptanzkriterium überholt.** Der Backlog-Entwurf verlangt für
openrouter „chat und embed funktionieren". Nach der PO-Entscheidung (Embeddings
kommen in v1 immer aus Ollama, SPEC 7.1) muss das AK auf „chat funktioniert;
embed bleibt bei Ollama, sichtbare Degradation bei fehlendem Ollama" lauten.

**5.3 — Tray-Menüeintrag in Sprint 1.** S1 sieht die Menüeinträge als Stubs
vor. Damit Sprint 1 ohne S4 (Kürzel) manuell prüfbar ist, sollte S3 den Eintrag
„Capture öffnen" real verdrahten. Kleine Scope-Präzisierung an S3, keine
Punkteänderung — Entscheidung liegt beim PO.

**5.4 — Cluster-Schwelle 0,60 ist eine unbelegte Konstante.** Sie hängt
vollständig am Embedding-Modell (Befund B zu S17). Solange bge-m3 nicht
installiert ist (T4), lässt sie sich nicht kalibrieren; bei S17 ist sie gegen
echte Notizen zu prüfen.

## 6. Impediment-Liste

**I1 — Werkzeugkette auf Ganymed unvollständig (offen).** Nicht vorhanden: die
Spec-Default-Modelle `qwen3:8b` und `bge-m3` (installiert sind `qwen3.6:27b` und
`snowflake-arctic-embed2`), ein aufrufbares `whisper-cli`, GGML-Modelldateien,
WhisperX in jeder Form. Wirkung: Die Akzeptanzkriterien von S9, S10a, S14a und
S15 sind ohne Nachinstallation nicht abnehmbar. Gegenmaßnahmen: T4 (vor S9),
T6 (vor S14a), S15 blockiert geführt bis die WhisperX-Installation im
RPG-Audio-Projekt entsteht.

**I2 — Wayland-Fokusübernahme ungeklärt (adressiert).** Querschnittsrisiko für
S3, S4 und S13b; unter Wayland kann eine Anwendung sich den Fokus nicht selbst
nehmen. Gegenmaßnahme: T1 als erste Story in Sprint 1 — einmal klären statt
dreimal scheitern. Stop-Bedingung: Trägt weder xdg-activation noch eine
KWin-Regel, ist das ein Fall „needs a human" (Meldung an PO und Kunden, nicht
weiterprobieren).

**I3 — Arbeitsmuster „Agent meldet sich untätig statt Bericht zu liefern"
(für die Retro vorgemerkt).** Während der Schätzklausur dreimal aufgetreten.
Wirkung auf den Prozess: Der PO musste nachfassen, die Klausur verzögerte sich.
Evidenz für die erste Retro nach Sprint 3 gesammelt; eine Prozess- oder
Agenten-Änderung wird dort vorgeschlagen, nicht vorab.

**I4 — Automatisierte Prüfbarkeit der UI-lastigen Stories (bekannt, ohne
Gegenmaßnahme).** S3, S13b, S20a und S20b entziehen sich weitgehend
automatisierten Tests (Befund beider Berichte). Die manuelle Checkliste je
Meilenstein (SPEC 16) ist hier die einzige Kontrollinstanz — beim
DoD-Durchgang entsprechend zu prüfen statt zu behaupten.

## 7. Sprint-1-Vorschlag

**Sprint-Ziel:** Ein getippter Gedanke landet über das rahmenlose
Capture-Fenster in der Datenbank — und der Wayland-Fokusweg ist belegt statt
vermutet.

| Reihenfolge | ID | Story | SP |
|---|---|---|---|
| 1 | T1 | Wayland-Fokus-Spike | 2 |
| 2 | S1 | Projektgerüst mit Tray | 3 |
| 3 | S2 | SQLite-Store (M1-Schema) | 3 |
| 4 | S3 | Text-Capture-Fenster | 5 |
| | | **Summe** | **13** |

**Begründung der Auswahl:**

- **Risiko zuerst.** T1 steht am Anfang, weil die Wayland-Fokusfrage drei
  spätere Stories gleichzeitig gefährdet. Fällt der Spike negativ aus, ändert
  das die Schätzung von S3 noch innerhalb desselben Sprints — und nicht erst,
  wenn UI-Arbeit hineingeflossen ist.
- **Abhängigkeiten.** S1 ist das Fundament für alles Weitere (Build, Tray,
  D-Bus-Name). S3 kann sein Akzeptanzkriterium „Strg+Enter speichert die Notiz
  in den Store" nur erfüllen, wenn S2 vorliegt — deshalb S2 vor S3.
- **Nutzen am Sprint-Ende.** Nach Sprint 1 ist Denkzettel benutzbar, wenn auch
  über das Tray-Menü statt über das Kürzel: Gedanke tippen, speichern, fertig.
  Das ist Meilenstein 1 minus Kürzel.
- **Was bewusst draußen bleibt.** S4 (Kürzel + D-Bus, 5 SP) hätte den Sprint auf
  18 SP getrieben und damit die Umfangsgrenze aus PROZESS.md gerissen. S4 ist
  die erste Story in Sprint 2 — dann mit dem Wissen aus T1 und gegebenenfalls
  neu geschätzt. T2 (Autostart) folgt ebenfalls in Sprint 2; ohne Kürzel bringt
  ein automatisch startender Dienst noch keinen Nutzen.

**Freigabe:** Dieser Vorschlag geht über den Product Owner an den Kunden. Der
Sprint startet erst nach dessen Freigabe (Freigabemodell PROZESS.md,
Kundenentscheidung 31.07.2026).

## 8. done / next

**done:** Zwei unabhängige Schätzungen über 28 Stories konsolidiert (41 Stories
und 136 SP nach Neuschnitt); sieben Entscheidungsfälle begründet; acht 8er/13er
in 15 kleinere Stories geteilt; S19 aufgelöst, S8 bewusst erhalten; sieben
Vorbedingungs-Stories ergänzt; vier Impediments erfasst; Sprint-1-Vorschlag mit
13 SP formuliert.
**next:** PO legt die GitHub-Issues an (inkl. der neuen IDs und der
AK-Korrekturen aus Abschnitt 5), holt die Kundenfreigabe für Sprint 1 und
zieht die vier Stories in den Milestone `Sprint 1`. Danach: Umsetzung
beginnend mit T1; bei negativem Spike-Befund Neuschätzung von S3 vor
Weiterarbeit.
