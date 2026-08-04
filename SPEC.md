# Denkzettel — Spezifikation

Stand: 2026-07-31 — abgeleitet aus KONZEPT.md (drei Design-Interviews, alle
Grundsatzfragen entschieden) und den Wireframes (`wireframes/`). Diese Spec ist
die Bau-Grundlage; wo sie das Konzept präzisiert oder davon abweicht, ist das
ausdrücklich als solches markiert.

## 1. Ziel

Denkzettel ist ein Quick-Capture-Werkzeug für KDE Plasma (Wayland): Ein globales
Kürzel öffnet ein zentriertes Eingabefenster, ein Gedanke wird getippt oder
gesprochen und ist gespeichert — null Zeremonie. Ein Hintergrund-Dienst
klassifiziert per KI, vergibt Tags, baut einen Suchindex und legt dem Nutzer
Vorschläge zur Bestätigung vor: thematische Bündel für den Obsidian-Vault und
Tasks für Taskwarrior. Denkzettel ist Durchlauf-Speicher — der Klartext-Ort
bleibt Obsidian, das Task-System bleibt Taskwarrior.

Leitplanken (aus dem Konzept, unverhandelbar):

- Der Capture-Weg wird niemals mit UI belastet — Geschwindigkeit ist der Kern.
- Überführungen passieren nur nach Bestätigung („Melden, nicht heilen").
- Die App darf nicht volllaufen; ein Voll-Export-Rettungsweg existiert immer.

## 2. Architektur

### 2.1 Prozessmodell — Präzisierung gegenüber dem Konzept

Das Konzept nennt „zwei Komponenten" (Capture-Fenster, Hintergrund-Dienst).
Diese Spec präzisiert: **ein Prozess zur Laufzeit** (`denkzetteld`), in dem
Capture-/Aufnahmefenster als vorinstanziierte, versteckte Fenster leben.

Begründung: Der Dienst läuft ohnehin dauerhaft (Tray, Shortcuts, Analyse).
Ein vorgehaltenes Fenster erscheint auf Shortcut-Druck ohne Prozessstart —
schneller geht es nicht, und Geschwindigkeit ist Kernziel Nr. 1. Die Trennung
Capture ↔ Dienst bleibt als **Modulgrenze im Code** erhalten (eigenes
Verzeichnis, keine Abhängigkeit des Capture-Moduls auf Analyse-Code).

Die im dritten Interview entschiedene **D-Bus-Kopplung** bleibt real: Der
Prozess exportiert `org.denkzettel.Daemon` als externe Schnittstelle (siehe
2.3) — nutzbar von CLI, Skripten und für Einzelinstanz-Erzwingung.

### 2.2 Module

| Modul | Aufgabe |
|---|---|
| `capture` | Text-Capture-Fenster, Aufnahmefenster (Sprachnotiz) |
| `store` | SQLite-Zugriff, Datenmodell, FTS-Index, Audio-Dateiverwaltung |
| `analysis` | KI-Pipeline: Klassifikation, Tags, Embeddings, Clustering, Task-Extraktion |
| `transcribe` | Whisper-Backends (whisper.cpp, WhisperX) als Subprozesse, Job-Queue |
| `proposals` | Vorschlags-Erzeugung und -Ausführung (Obsidian-Export, Taskwarrior) |
| `ui` | Bibliothek, Vorschlags-Review, Einstellungen |
| `shell` | Tray (KStatusNotifierItem), KGlobalAccel-Registrierung, KNotifications, D-Bus-Adaptor |

### 2.3 D-Bus-Schnittstelle `org.denkzettel.Daemon`

| Methode | Wirkung |
|---|---|
| `ShowCapture()` | Capture-Fenster zeigen (auch Ziel der KGlobalAccel-Aktion) |
| `ShowRecorder()` | Aufnahmefenster zeigen, Aufnahme startet sofort |
| `AddNote(text) → id` | Notiz ohne UI anlegen (CLI/Skripte) |
| `AnalyzeNow()` | Analyse-Lauf anstoßen |
| `ShowLibrary()`, `ShowProposals()` | Fenster öffnen |
| `Quit()` | Beenden |

Ein zweiter Prozessstart erkennt die belegte D-Bus-Registrierung und ruft
stattdessen `ShowCapture()` (Einzelinstanz).

### 2.4 Globale Kürzel

- `Meta+N` → `ShowCapture()` · `Meta+Umschalt+N` → `ShowRecorder()`
- Registrierung über **KGlobalAccel** (KF6); die Kürzel erscheinen in den
  Plasma-Systemeinstellungen und sind dort wie in den App-Einstellungen änderbar.
- Belegung auf dem Entwicklungsrechner geprüft (31.07.2026, inkl. Mehrfachbelegungen): beide frei.
- **Konflikterkennung (T1-Befund):** Eine KGlobalAccel-Registrierung kann
  unsichtbar fehlschlagen — der Eintrag entsteht und `invokeShortcut`
  funktioniert, aber der echte Tastendruck geht weiter an den bestehenden
  Besitzer. Beim Erststart und bei Kürzel-Änderung prüft Denkzettel die
  Sequenz gegen die bestehende Belegung (inkl. Mehrfachbelegungen) und
  meldet Konflikte sichtbar statt still zu scheitern.
- **Auslösung über Desktop Actions (entdeckte Bedingung, Befund 01.08.2026):**
  Bei installierter Anwendung endet der Komponentenname auf `.desktop`; damit
  behandelt kglobalacceld die Komponente als *Service-Action-Komponente* und
  startet beim Tastendruck die gleichnamige **Desktop Action** der
  `.desktop`-Datei (ApplicationLauncherJob), statt ein D-Bus-Signal an den
  laufenden Prozess zu senden. Fehlt die Gruppe, protokolliert der Dienst einen
  Fehler und bricht ab — die Registrierung liegt vor, `isActive` ist wahr, und
  der Tastendruck verpufft. Deshalb gilt: **Je Kürzel deklariert die
  Desktop-Datei eine Gruppe `[Desktop Action <Aktions-Id>]` mit eigener
  `Exec`-Zeile, und die Id steht in `Actions=`.** Die Aktions-Id ist zugleich
  der `objectName` der `QAction` und muss ein gültiger XDG-Bezeichner sein
  (Buchstaben, Ziffern, Bindestrich — kein Unterstrich; `desktop-file-validate`
  weist ihn sonst zurück). Die `Exec`-Zeile startet `denkzetteld`; die
  Einzelinstanz-Weiche aus 2.3 macht daraus den Aufruf des Fensters.
- **Rücklesen der Registrierung (entdeckte Bedingung, Befund 01.08.2026;
  Retro-Beschluss B5):** `KGlobalAccel::setGlobalShortcut()` kann einen
  Fehlschlag des Dienstes nicht melden — der Aufruf setzt seine D-Bus-Nachricht
  ab, ohne die Antwort zu lesen, und liefert auch dann `true`, wenn
  kglobalacceld nichts behalten hat. Deshalb fragt Denkzettel nach jeder
  Registrierung beim Dienst nach, welche Sequenz er für die Aktion hält, und
  prüft zugleich, ob die Desktop-Datei die zugehörige Gruppe deklariert. Bleibt
  eines von beiden aus, meldet Denkzettel das sichtbar — **bei jedem Start, nicht
  nur beim ersten**: Anders als ein Konflikt lässt dieser Fehlschlag gar kein
  wirkendes Kürzel zurück. Die Meldung nennt das betroffene Kürzel und einen
  ausführbaren Schritt. **Das gilt je Kürzel:** `Meta+Umschalt+N` durchläuft
  dieselbe Prüfung wie `Meta+N` — ohne sie wiederholt es dessen Fehlschlag, und
  „in den Systemeinstellungen sichtbar“ ist gerade der Zustand, den ein still
  gescheitertes Kürzel erzeugt.

### 2.5 Autostart und Erststart (Ergänzung aus der Schätzklausur)

- `denkzetteld` startet über einen **XDG-Autostart-Eintrag** mit der
  Plasma-Sitzung (das Paket installiert die .desktop-Datei; Deaktivierung
  über die Plasma-Systemeinstellungen). Ohne laufenden Dienst gäbe es keine
  Kürzel — Autostart ist Grundfunktion, keine Politur.
- **Erststart**: legt Datenverzeichnis, DB (aktuelle Schemaversion) und
  Default-Konfiguration an; erkennt die optionalen Werkzeuge (ffmpeg,
  whisper-cli, task, Ollama-Erreichbarkeit) und zeigt Fehlendes in den
  Einstellungen an (vgl. Abschnitt 15).
- **Aufräum-Kontrolle**: Beim Dienststart werden Audio-Dateien ohne
  DB-Verweis (abgebrochene Aufnahmen, unterbrochene Löschungen) entfernt
  und im Log vermerkt — eindeutiger, harmloser, wiederkehrender Fall,
  daher zulässige Selbstheilung im Sinne der Loop-Konventionen.

## 3. Capture-Fenster (Text)

- Rahmenloses Fenster, sofortiger Fokus, immer im Vordergrund.
- **Fokus-Mechanik (T1-Befund, Issue #1):** Vor jedem Zeigen wird das Fenster
  neu gemappt — `hide()` zerstört die Wayland-Surface, `show()` erzeugt ein
  frisches Toplevel, das vom Compositor regulär den Fokus erhält. Der
  XDG-Activation-Token-Weg trägt nachweislich nicht (KGlobalAccel liefert
  kein Token, Zeitstempel immer 0) und wird nicht gebaut.
- **Zentrierung (PO-Entscheidung nach T1):** KWin-Standardplatzierung —
  Plasma 6.7 zentriert standardmäßig, auf dem Entwicklungsrechner verifiziert. Ein
  Wayland-Client kann sich nicht selbst positionieren; weicht die
  Platzierungsrichtlinie des Nutzers ab, ist Layer-Shell (Overlay,
  `AnchorNone`, `KeyboardInteractivityOnDemand`) der gemessene Rückfallweg —
  dokumentiert, in v1 nicht gebaut.
- Inhalt: App-Name klein, mehrzeiliges Textfeld (Platzhalter „Gedanke
  festhalten …"), Fußzeile „Esc verwirft · Strg+Enter speichert".
- **Mitwachsend**: Starthöhe ~5 Zeilen (Sprint-1-Abnahme: 3 waren dem Kunden
  zu wenig), wächst mit dem Text bis ~8 Zeilen, danach Scrollbalken.
- Strg+Enter: Notiz speichern (`store`), Fenster verstecken, Feld leeren.
  Esc: verwerfen, Fenster verstecken. Fokusverlust: Fenster bleibt (kein
  Datenverlust durch versehentlichen Klick daneben).
- Kein Button, kein Menü, keine Formatierung.

## 4. Aufnahmefenster (Sprachnotiz)

- Gleiche Machart wie Capture; die **Aufnahme läuft ab Fensteröffnung** —
  kein Start-Knopf.
- Inhalt: Aufnahme-Indikator (roter Punkt), einfacher Pegel, laufende
  Zeitanzeige, gleiche Fußzeile.
- Strg+Enter: Aufnahme stoppen, Audio speichern, Notiz vom Typ `audio`
  anlegen, Transkriptions-Job einreihen, Fenster verstecken. Esc: Aufnahme
  verwerfen (Datei löschen).
- Technik: QtMultimedia (PipeWire-Backend), Format **Opus in OGG**
  (`audio/*.ogg`), mono, 48 kHz — klein und von Qt direkt abspielbar.
- Obergrenze 15 Minuten (Schutz vor vergessener Aufnahme); Hinweis in der
  Zeitanzeige ab Minute 14.

## 5. Datenhaltung

### 5.1 SQLite (eine DB: `~/.local/share/denkzettel/denkzettel.db`)

```sql
notes(id INTEGER PK, created_at TEXT ISO8601, type TEXT 'text'|'audio',
      content TEXT,            -- Text bzw. Transkript
      audio_path TEXT NULL,    -- relativ zu audio/, nur type='audio'
      audio_duration_s INTEGER NULL,
      category TEXT NULL,      -- KI-Kategorie, NULL = unanalysiert
      state TEXT 'neu'|'transkribiert'|'analysiert',
      needs_reembed INTEGER NOT NULL DEFAULT 0,  -- nach Bearbeitung (Abschn. 9)
      analysis_attempts INTEGER NOT NULL DEFAULT 0,  -- Fehlerzähler 7.2
      analysis_last_error TEXT NULL)
tags(note_id FK, tag TEXT)
embeddings(note_id FK PK, model TEXT, vector BLOB)  -- float32-Array
proposals(id INTEGER PK, kind TEXT 'bundle'|'task', created_at TEXT,
          status TEXT 'offen'|'zurueckgestellt',
          payload TEXT JSON)   -- Bündel: Titel+Markdown; Task: Felder
proposal_notes(proposal_id FK, note_id FK)
transcribe_jobs(note_id FK PK, enqueued_at TEXT, attempts INTEGER,
                last_error TEXT NULL)
meta(key TEXT PK, value TEXT)  -- Schema-Version u. Ä.
```

- Volltextindex: **FTS5**-Tabelle `notes_fts(content)`, per Trigger synchron.
  Sie hält **keinen eigenen Text**, sondern verweist mit `content='notes'`,
  `content_rowid='id'` auf die Notiztabelle (Schemaversion 2, Issue #8) — der
  Notiztext existiert genau einmal. Daraus folgt eine Bedingung, ohne die der
  Index still verwahrlost: Die Trigger für Ändern und Löschen müssen FTS5 den
  **alten** Text mitgeben (`'delete'`-Kommando mit `old.content`). Mit dem
  neuen Text bleiben die alten Wörter auffindbar, und weder ein Fehler noch
  FTS5s `integrity-check` zeigen das an — nur eine Suche nach dem alten Wort
  (siehe `StoreTest::keepsSearchIndexInSync()`).
- Audio liegt als Datei unter `audio/` (Name = Notiz-ISO-Zeitstempel), die DB
  hält den Verweis. Löschen einer Notiz löscht Tags, Embedding, FTS-Eintrag,
  `proposal_notes`-Verweise und Audio-Datei in einer Transaktion +
  Dateisystem-Aufräumen.

### 5.2 Einstellungen und Geheimnisse

- Einstellungen: **KConfig** (`~/.config/denkzettelrc`).
- API-Keys (openrouter, OpenAI): **KWallet** — nie im Klartext in
  Config-Dateien. (OAuth-Tokens erst, falls der spätere
  Codex-App-Server-Zusatzpfad aus 7.5 je gebaut wird.)

## 6. Suche

Volltextsuche über FTS5 mit **Operatoren im Suchfeld** (Entscheidung drittes
Interview). Syntax-Umfang (damit ist offene Frage 3 des Konzepts beantwortet):

| Operator | Bedeutung |
|---|---|
| `tag:backup` | Notizen mit KI-Tag `backup` |
| `kat:todos` | Kategorie (alle, todos, ideen, cli, persoenlich, software) |
| `typ:text` / `typ:audio` | Notiztyp |
| `vor:2026-07` / `vor:2026-07-15` | erstellt vor Datum (Monat oder Tag) |
| `nach:2026-06` | erstellt nach Datum |
| `"exakte Phrase"` | Phrasensuche (FTS5-Phrase) |
| freier Text | FTS5-Volltext (UND-verknüpfte Terme) |

- Alle Bestandteile sind **UND-verknüpft**; kein OR, keine Klammern (V1).
- Unbekannte `xyz:`-Präfixe werden als Volltext behandelt (kein Fehler).
- Parser ist reine Funktion `QString → SearchQuery` — unit-testbar.
- FTS5-Tokenizer: **`trigram remove_diacritics 1`** (Kundenentscheidung
  01.08.2026, Issue #8). Ein Suchbegriff findet **Wortteile an jeder Stelle**:
  „grafieren" findet „fotografieren", „bahn" findet „Straßenbahn", „sprech"
  findet „Besprechung". „bucher" findet „Bücher" — die Umlaut-Toleranz bleibt
  Kernanforderung und ist mit `remove_diacritics 1` erhalten.
  - **Damit ist die Präfixsuche keine eigene Festlegung mehr.** Sie ist im
    Teilstring-Verhalten enthalten. Die Abfrage hängt **kein** `*` an: Am
    System gemessen sind `"foto"` und `"foto"*` beim trigram-Tokenizer
    identisch — er erzeugt ausschließlich vollständige Drei-Zeichen-Tokens,
    an denen ein Präfixzeichen nichts erweitern kann. Ein `prefix=`-Index
    wird ebenfalls nicht angelegt (Beschluss E2).
  - **Preis, gemessen an 20 000 Notizen:** Der trigram-Index ist rund
    **sechsmal** so groß wie ein `unicode61`-Index (1,8 MiB → 10,9 MiB) und
    damit gut dreimal so groß wie der Rohtext selbst. Für den erwarteten
    Bestand ist das tragbar; bei sechsstelligen Notizzahlen wäre es neu zu
    bewerten.
  - **Grenze (Befund Issue #8, SQLite 3.53.4):** Der Tokenizer entfernt
    diakritische Zeichen. `ß` trägt keines — es ist ein eigener Buchstabe und
    bleibt stehen. „strassenbahn" findet „Straßenbahn" deshalb **nicht**,
    „grosse" nicht „Größe". Gilt für `unicode61` wie für `trigram`; die
    ß/ss-Faltung verlangt einen eigenen Tokenizer und ist eigene Story (S30).
- **Suchbegriffe unter drei Zeichen (Entscheidung Issue #8):** Ein
  trigram-Index kann sie prinzipbedingt nicht enthalten — ein Trigramm ist
  drei Zeichen lang. Solche Begriffe werden deshalb **als Teilstring direkt
  auf `notes.content` verglichen** (`LIKE '%…%'`), die übrigen weiterhin über
  den Index; beide Wege sind UND-verknüpft. Begründung: „KI", „PO" oder „ad"
  sind echte Suchbegriffe, und eine Suche, die dabei wortlos leer bleibt,
  wäre ein Fehler, den niemand als solchen erkennt. Der Alternativweg — ein
  Hinweis im Leerzustand — würde eine reine Umsetzungsgrenze zur Regel
  machen, die der Nutzer lernen muss. **Kosten gemessen** (20 000 Notizen):
  3 ms je Abfrage, weniger als die Indexabfrage selbst (9 ms) — der
  Einwand des vollen Tabellendurchlaufs trägt in dieser Größenordnung nicht.
  - Grenze dieses Wegs: Er ignoriert Groß-/Kleinschreibung nur für ASCII
    („ki" findet „KI"), faltet aber keine diakritischen Zeichen („u" findet
    kein „ü") und keine Groß-/Kleinschreibung darüber hinaus („ü" findet
    kein „Ü"). Betrifft ausschließlich Begriffe mit ein oder zwei Zeichen.
- Die Trefferliste behält die Ordnung der Bibliothek (neueste zuerst, 9.)
  statt der FTS5-Relevanzsortierung — nur so trägt sie deren Tagesgruppen.
  - **BM25 ist am 04.08.2026 geprüft und verworfen** (Kundenentscheidung;
    Belege unter `docs/scrum/reviews/2026-08-04-bm25/`). Der Grund ist **nicht**
    der Trigramm-Tokenizer — die Vermutung, ein in Trigramme zerfallender
    Suchbegriff verzerre die Formel, ist widerlegt: FTS5 summiert BM25 über
    **Phrasen**, nicht über Tokens, und die Abfrage ist phrasenweise gebaut.
    Verworfen wurde BM25 aus drei anderen Gründen:
    1. **Bei kurzen Notizen entartet es zu „kürzeste Notiz zuerst".** Ein
       gesuchter Begriff steht dort fast immer genau einmal (`f=1`); bei einem
       einzelnen Suchbegriff kürzt sich der IDF-Anteil heraus, und übrig bleibt
       die Längennormierung. Eine Einzeilernotiz, in der „Backup" beiläufig
       vorkommt, stünde vor der ausführlichen Notiz über Backups. `k1` und `b`
       sind in FTS5 **nicht einstellbar** — es gibt keinen Regler dagegen.
    2. **Zwei Suchwege hätten gar keinen Rang:** Begriffe mit ein oder zwei
       Zeichen laufen über `content LIKE` und stehen nicht im FTS-Index; reine
       Filtersuchen (`tag:`, `kat:`, `vor:`) enthalten keinen Volltextterm.
    3. **Die Sortierung ist keine Eigenschaft der Suche, sondern die
       Voraussetzung der Listendarstellung.** Der Zeilenbau gruppiert, er
       sortiert nicht; nach Rang geordnete Eingabe erzeugt Tagesköpfe mehrfach
       und in wechselnder Folge. Dazu hängt der Zeitstempel je Eintrag an seiner
       Gruppe (9., Zeichnung 3b): Ohne Kopf verlöre jeder Treffer von heute und
       gestern seine Tagesangabe. Und da bei jedem Tastendruck gesucht wird,
       spränge die Liste, während der Nutzer noch tippt.
  - **Was stattdessen fehlt, ist die Lesbarkeit des Treffers**, nicht seine
    Rangfolge: Der gefundene Begriff wird in der Liste nicht hervorgehoben, und
    die Fundstelle liegt oft im abgeschnittenen Teil (Hinweis 2 des
    S6-Reviews, Sprint 3). Eigene Story.

## 7. KI-Pipeline

### 7.1 Provider-Abstraktion

Interface `AiProvider` mit zwei Fähigkeiten: `chat(prompt) → text/json` und
`embed(text) → vector`. Implementierungen:

- **Ollama** (lokal/eigene URL): `/api/chat` + `/api/embed`.
  Defaults: LLM `qwen3:8b`, Embedding `bge-m3` (mehrsprachig).
- **openrouter.ai**: OpenAI-kompatible API, API-Key aus KWallet — nur `chat`.
- **OpenAI**: per Platform-API-Key (siehe 7.5) — nur `chat`.

**Embeddings kommen in v1 immer aus Ollama** (Präzisierung nach Befund der
Schätzklausur 31.07.2026: openrouter bietet keinen Embedding-Endpunkt).
Der LLM-Provider ist frei wählbar, das Embedding-Modell läuft lokal —
einheitlich, kostenlos, und die Cluster-Schwelle (7.3) bleibt an ein
Modell gebunden. Ohne erreichbares Ollama degradiert Denkzettel sichtbar:
Klassifikation über den gewählten Provider funktioniert weiter, Themen-
Bündel entfallen (Hinweis in Einstellungen und Tray-Tooltip).

Alle Aufrufe über Qt Network, asynchron, mit Timeout (30 s) und einem
Wiederholungsversuch. „Verbindung testen" in den Einstellungen macht je einen
Mini-`chat`- und (bei Ollama) `embed`-Aufruf und zeigt Latenz oder Fehler.

### 7.2 Analyse-Lauf

Auslöser je Einstellung: **sofort** (nach Speichern/Transkription),
**periodisch** (Intervall, Default 30 min) oder **auf Abruf** (Tray/D-Bus).
Ein Lauf verarbeitet alle Notizen mit `state != 'analysiert'` sowie —
nur für Schritt 2 — solche mit `needs_reembed = 1`; höchstens 50 Notizen
pro Lauf (Budget, Abschnitt 14), der Rest folgt im nächsten Lauf:

1. **Klassifikation + Tags** (ein LLM-Aufruf pro Notiz, JSON-Schema:
   `{category, tags[], is_todo, task?}`): Kategorie aus fester Liste (TODOs,
   Ideen, CLI-Befehle, Persönlich, Software-Ideen), 1–4 Tags kleingeschrieben.
   Für `is_todo=true` extrahiert derselbe Aufruf die Task-Felder
   (`description, project, tags, due, priority` — `due`/`priority` nur bei
   klarem Signal, sonst null).
2. **Embedding** (ein `embed`-Aufruf pro Notiz) → `embeddings`-Tabelle;
   setzt `needs_reembed` zurück.
3. **Clustering + Vorschlags-Erzeugung** (7.3/7.4).

Fehlerbehandlung nach Loop-Konventionen: Fehlversuche werden persistent
gezählt (`notes.analysis_attempts`/`analysis_last_error`, überlebt
Daemon-Neustarts; Erfolg setzt zurück). Ab dem zweiten Fehlschlag wird die
Notiz übersprungen und der Fehler im Tray-Tooltip + Log gemeldet — kein
endloses Wiederholen, keine Selbstheilung.

### 7.3 Themen-Clustering (beantwortet offene Frage 1)

- Grundlage: Cosine-Ähnlichkeit der Embeddings aller **unexportierten,
  analysierten** Notizen (Brute-Force-Paarvergleich; bei Volllauf-Schwelle
  ~200 Notizen sind das ≤ 20k Vergleiche — unkritisch, keine Vektor-DB).
- Verfahren: Single-Linkage-Verkettung — Notizenpaare mit Ähnlichkeit ≥
  **0,60** (interne Konstante, kalibrierbar, kein User-Setting) landen im
  selben Cluster.
- Cluster mit ≥ **Bündel-Schwelle** Notizen (Einstellung, Default **3**)
  werden dem LLM vorgelegt: Es benennt das Thema, darf offensichtliche
  Ausreißer entfernen (Plausibilisierung) und erzeugt die Sammelnotiz
  (siehe 8.1). Ergebnis: ein `bundle`-Vorschlag.
- **Notizen ohne Cluster** bleiben einfach im Bestand. Reißt die
  Alters-Schwelle des Volllauf-Schutzes, erzeugt der Dienst zusätzlich ein
  Bündel „Vermischtes vom <Zeitraum>" aus den ältesten clusterlosen Notizen —
  ebenfalls nur als Vorschlag, jede Notiz abwählbar.
- Bereits vorgeschlagene, aber zurückgestellte Notizen (`status =
  'zurueckgestellt'`) werden beim nächsten Lauf erneut geclustert — ein
  „Später" verschiebt nur, es versteckt nichts dauerhaft.

### 7.4 Task-Vorschläge

Für jede als TODO klassifizierte Notiz entsteht ein `task`-Vorschlag mit den
extrahierten Feldern. Kein Auto-`task add` — Ausführung erst nach Bestätigung
im Review (Abschnitt 9).

### 7.5 OpenAI-Anbindung

Die Entscheidung des dritten Interviews (OAuth „Sign in with ChatGPT" von
Anfang an) stand ausdrücklich unter Recherchevorbehalt. Die Recherche
(31.07.2026, vollständig mit Quellen:
`recherche/2026-07-31-openai-oauth-machbarkeit.md`) hat den Vorbehalt
ausgelöst:

- **„Sign in with ChatGPT" ist ein reines Identitätsverfahren** (Beta, sechs
  kuratierte Partner, keine öffentliche Selbstregistrierung). Die App erhielte
  Name, E-Mail und Profilbild — laut OpenAI-Doku ausdrücklich keine Tokens
  und keinen Modellzugriff.
- Der inoffizielle **Codex-OAuth-Abo-Weg** (Codex CLI, OpenClaw) ist technisch
  einsehbar, aber für Dritt-Apps nie freigegeben (ToS-Grauzone, von OpenAI
  seit 12/2025 mehrfach unbeantwortet) — und liefert nur Chat-Endpunkte:
  **keine Embeddings**, die Denkzettels KI-Architektur (7.1/7.3) zwingend
  braucht.

**Konsequenz für v1: OpenAI per manuellem Platform-API-Key** (Ablage in
KWallet), gleichberechtigt neben Ollama und openrouter. Die
Einstellungen-Seite zeigt bei OpenAI einen kurzen Hinweistext, warum es kein
„Mit ChatGPT anmelden" gibt. Ein Abo-Weg über den Codex App Server
(JSON-RPC/stdio) bleibt als optionaler späterer Zusatzpfad denkbar, wird für
v1 aber nicht gebaut.

## 8. Überführungen

### 8.1 Obsidian-Export (Bündel)

- Zielnotiz: **eine Sammelnotiz pro Thema** in `<Vault>/_INBOX/`, Dateiname
  `Denkzettel <Thema> <YYYY-MM-DD>.md`.
- Aufbau: Vault-konformes Frontmatter (`type`, `tags`, `created` — beim Bauen
  gegen die Vault-CLAUDE.md-Konventionen verifizieren), `# <Thema>`, dann
  `## <YYYY-MM-DD>`-Abschnitte mit den Notizen als Absätze/Bullets
  (chronologisch), wie die Markdown-Vorschau im Wireframe.
- Sprachnotizen exportieren ihr **Transkript**; die Audio-Datei wird beim
  Export gelöscht (Konzept-Entscheidung: Audio lebt nur solange die Notiz).
- Nach bestätigtem Export: Notizen in einer Transaktion löschen (inkl. Audio,
  Tags, Embeddings, FTS) und den **Vorschlag samt `proposal_notes`-Verweisen
  entfernen** — Übernehmen und Verwerfen enden gleich, nur dass Übernehmen
  vorher ausführt. Ein „übernommen"-Zustand existiert nicht (Durchlauf-
  Speicher, keine Vorschlags-Historie).

### 8.2 Taskwarrior

- Ausführung per `QProcess`: `task add <description> project:<p> +tag1 +tag2
  due:<d> priority:<p>` (nur gesetzte Felder), danach bei längerem Notiztext
  `task <uuid> annotate <volltext>` (UUID aus `task add`-Ausgabe).
- Fehlerfall (task-Binary fehlt, Exit ≠ 0): Vorschlag bleibt offen, Fehler
  wird an der Karte angezeigt — nichts geht verloren.
- Nach Erfolg: Notiz löschen und Vorschlag entfernen (wie 8.1).

### 8.3 Voll-Export (Rettungsweg)

- Menüpunkt in der Bibliothek: exportiert **alle** Notizen als Ordner
  `denkzettel-export-<datum>/` mit einer `.md` je Notiz (ISO-Name, Frontmatter
  mit Kategorie/Tags/Typ) plus `audio/`-Unterordner mit den Originaldateien.
- Rein lesend — der Bestand bleibt unverändert. Kein KI-Aufruf nötig.

## 9. Bibliothek und Vorschlags-Review

- **Bibliothek** (Fenster): Sidebar mit KI-Kategorien + Zählern, chronologische
  Notizliste, wie ein Posteingang in Tagesgruppen gegliedert (**Heute ·
  Gestern · Diese Woche · Letzte Woche · Älter**; „Woche" ist die
  Kalenderwoche, ihr Anfang folgt der Locale des Systems —
  `QLocale::firstDayOfWeek`, in Deutschland Montag), innerhalb der Gruppen
  neueste zuerst.
  Ein Eintrag zeigt Zeitstempel, die erste Zeile als Betreff, den Folgetext
  als Vorschau und Tag-Chips; Sprachnotizen zusätzlich ▶ und Dauer. Der
  Zeitstempel folgt der Gruppe: in Heute/Gestern die Uhrzeit, in den
  Wochengruppen Wochentag und Datum, in Älter das absolute Datum; im
  Detailbereich die volle Form. Die Gliederung ist fest — kein Umschalter,
  keine einklappbaren Gruppen (Wireframes 3a/3b).
  Die Gruppen werden beim Aufbau der Liste und bei jeder Fensteraktivierung
  nachgerechnet — es gibt keinen Mitternachtszeitgeber (Wireframe 3b).
  **Neu gruppiert wird dabei nur, wenn der Kalendertag ein anderer ist als
  beim letzten Aufbau** (entdeckt bei der Umsetzung, DoD 4): Neugruppieren
  setzt das Modell zurück und stellt die Auswahl wieder her, was die Liste zu
  ihr scrollt — ohne Tageswechsel warf ein Alt-Tab den Leser um 459 px auf
  seine Auswahl zurück (Issue #59, gemessen 04.08.2026). Der Kalendertag
  genügt als Bedingung, weil alle vier Gruppengrenzen Tagesgrenzen sind.
  Springt die Auswahl **per Taste** über eine Gruppengrenze, holt die Liste den
  Kopf der neuen Gruppe ins Bild (Wireframe 3b, Fall 4). **Ein Mausklick tut das
  nicht**: Wer zeigt, erwartet, dass die gezeigte Stelle bleibt, und ein
  Vorscrollen risse sie ihm unter dem Zeiger weg (gemessen 387 px, Issue #57).
  Der Tag geht dabei nicht verloren — der Detailbereich trägt den vollen
  Zeitstempel.
  Dazu Suchfeld (Abschnitt 6) und Button „Vorschläge" mit Badge.
- Detailansicht: **Lese- und Bearbeiten-Ansicht** (Entscheidung drittes
  Interview — v. a. für fehlerhafte Transkripte). Bearbeiten behält
  Kategorie/Tags und `state`, setzt aber `needs_reembed = 1` — der nächste
  Analyse-Lauf erneuert nur das Embedding (7.2), denn es veraltet mit dem
  Text. Löschen-Aktion mit 5-Sekunden-Undo (Spec-Ergänzung, nicht im
  Konzept: rein client-seitig verzögertes Löschen, kein Soft-Delete-Zustand
  in der DB).
- **Bedingungen des Bearbeiten-Zustands** (S8; die letzten beiden entdeckt
  bei der Umsetzung, DoD 4):
  - Ungespeicherte Änderungen werden **nie ohne Nachfrage** geschrieben oder
    verworfen. Auswahlwechsel, Fensterschließen, Esc und „Abbrechen“ führen
    denselben Dialog mit **Speichern · Verwerfen · Abbrechen**. Das weicht
    bewusst vom Capture-Fenster ab, wo Esc still verwirft (3): dort steht ein
    nie gespeicherter Entwurf, hier eine bereits gespeicherte Notiz.
    „Abbrechen“ liegt mit im Dialog, weil Schaltfläche und Kürzel dieselbe
    Handlung sind — ein Fehlklick auf sie ist genau der Fall, gegen den der
    Dialog gefasst ist.
  - Die gespeicherte Notiz **bleibt in der laufenden Trefferliste stehen**,
    auch wenn ihr neuer Text nicht mehr auf den Suchbegriff (6) passt; erst
    die nächste Änderung des Suchbegriffs liest den Store neu. Sonst
    verschwände die Notiz unter der Hand, die sie eben berichtigt hat.
  - Das **Suchfeld ist währenddessen abgeschaltet**. Eine Suche baut die
    Liste neu auf; die Notiz unter dem Editor kann dabei aus ihr
    herausfallen, und dann hat der Dialog keine Zeile mehr, auf die er die
    Auswahl zurücknehmen könnte.
  - **Bauart des Dialogs (entschieden in Sprint 5, #66):** Der Wächter ist
    ein **`KMessageDialog`** vom Typ `WarningTwoActionsCancel` mit
    `KStandardGuiItem`-Symbolen; **Vorgabeantwort ist „Speichern"**.
    Grund ist die in Sprint 4 entdeckte Bedingung (DoD 4/B9): Unter der
    KDE-Plattformintegration (`QT_QPA_PLATFORMTHEME=kde`) beantwortet das
    System einen gebauten `QMessageBox` mit einem **eigenen Meldungsfenster
    samt eigenen Knopfobjekten** — es übernimmt Beschriftung, Rollen und
    Reihenfolge, aber nichts, was nachträglich am `QPushButton` gesetzt wird
    (Symbole, Vorgabe-/Escape-Knopf). Ein `KMessageDialog` ist ein
    gewöhnlicher `QDialog` und bleibt der eigene. Daraus folgt für die
    Prüfung: Der Dialogtest misst den Dialog, den die Anwendung **zeigt**
    (`QApplication::activeModalWidget()`), unter gesetztem Plattform-Thema —
    ein Test ohne Plattform-Thema misst einen Dialog, den kein
    KDE-Sitzungsnutzer sieht.
  - **Bedingungen dieser Bauart, alle am 02.08.2026 gemessen** (DoD 4/B9):
    - `KMessageDialog` kennt **keinen Zweittext** (`informativeText`); Frage
      und Erläuterung stehen in einem Text, durch eine Leerzeile getrennt.
    - Die Antwortrollen sind `Yes` · `No` · `Reject` statt
      `Accept` · `Destructive` · `Reject`. **Zugesichert ist die Bedeutung**
      (Speichern schreibt und führt die Handlung aus, Verwerfen führt sie
      ohne Schreiben aus, Abbrechen bleibt im Editor), nicht die Rolle und
      nicht die Reihenfolge.
    - Die Vorgabeantwort **folgt dem Fokus**: Unter selbstvorgabefähigen
      Knöpfen macht der Fokuswechsel den fokussierten Knopf zur Vorgabe. Die
      KDE-Bauart gibt beim Sichtbarwerden Fokus und Vorgabe an „Abbrechen";
      „Speichern" muss deshalb **nach** dem Anzeigen Fokus *und* Vorgabe
      erhalten, und eine Zusicherung darüber ist erst gültig, wenn sie am
      **sichtbaren** Dialog gemessen wird.
    - Wird der Dialog von Hand angezeigt, ist er **nicht mehr modal** durch
      ein späteres `exec()`; die Modalität ist dann selbst zu setzen.
    - Das **Warnsymbol** (`dialog-warning`) wird **ausdrücklich gesetzt**.
      `KMessageDialog::setIcon()` sagt zwar zu, bei leerem Symbol eines nach
      Dialogtyp zu wählen — gemessen kommt keines, und der Dialog trägt dann
      gar kein Bildetikett. Ein Dialog über drohenden Datenverlust ist der
      Kernfall des Warnsymbols (PO-Entscheidung 02.08.2026; Zeichnung 2a,
      Zustand C nachgezogen).
    - **Die Bauart klingt** (entdeckt am 04.08.2026): `showEvent()` meldet bei
      jedem Anzeigen das KNotification-Ereignis `messageWarning`, dem
      `plasma_workspace.notifyrc` den Systemklang `dialog-warning` zuordnet —
      abgespielt im eigenen Prozess über libcanberra. Das ist
      KDE-Plattformstandard und **bleibt so**: Lautstärke und Stummschaltung
      regelt der Nutzer im System. `KMessageDialog::setNotifyEnabled(false)`
      würde den Klang abschalten; **genau das ist bewusst unterlassen**
      (Kundenentscheidung 04.08.2026), damit ihn niemand später für ein
      Versehen hält und wegmacht. Still sind allein die Test- und Bildläufer:
      sie lenken libcanberra vor `main()` auf den Null-Treiber
      (`tests/testsilence.cpp`) — oberhalb des Audiogeräts bleibt alles
      unverändert, und kein Test misst Klang.
- Steckt die Notiz in einem **offenen Vorschlag**, verwirft Bearbeiten oder
  Löschen diesen Vorschlag (seine Vorschau wäre veraltet); der nächste
  Analyse-Lauf erzeugt ihn auf aktuellem Stand neu.
- Bei Sprachnotizen: Audio-Player (Play/Pause, Fortschritt, Zeit) über dem
  Transkript.
- **Vorschlags-Review**: Liste offener Vorschläge beider Arten. Bündel-Karte:
  Titel, Notizliste mit Abwahl-Checkboxen, Markdown-Vorschau, Ziel „→ Obsidian
  _INBOX". Task-Karte: editierbare Felder (Beschreibung, project, tags, due,
  priority), Annotation-Vorschau, Ziel „→ Taskwarrior". Aktionen je Karte:
  **Übernehmen · Später · Verwerfen** (Verwerfen löscht nur den Vorschlag,
  nie Notizen).

## 10. Tray und Benachrichtigungen

- **KStatusNotifierItem**, dauerhaft. **Ein** Menü, in drei Gruppen; Wortlaut
  und Symbolnamen sind verbindlich (Wireframe 5a, Issue #60). Die Symbole
  kommen ausnahmslos aus dem Symbol-Thema: Nur ein Symbol aus dem Thema trägt
  einen Namen, und über das Tray-Protokoll geht der **Name**, nicht das Bild.

  | Eintrag | Symbolname | Zustand | Kürzel-Hinweis |
  |---|---|---|---|
  | Notiz erfassen | `document-edit` | aktiv | Meta+N |
  | Sprachnotiz aufnehmen | `audio-input-microphone` | inaktiv bis M4 | Meta+Umschalt+N ab M4 |
  | *— Trenner —* | | | |
  | Bibliothek öffnen | `view-list-text` | aktiv | — |
  | Jetzt analysieren | `system-run` | inaktiv bis M5 | — |
  | Vorschläge (Zähler) | `tools-wizard` | inaktiv bis M5 | — |
  | *— Trenner —* | | | |
  | Denkzettel einrichten … | `configure` | **erst mit #16** | — |
  | Beenden | `application-exit` | aktiv | — |

  **„Beenden" steht abgesetzt in der letzten Gruppe** und nie neben dem
  häufigsten Eintrag: Es beendet den Dienst und mit ihm das Kürzel. Bis der
  Einstellungs-Dialog (#16) steht, ist „Denkzettel einrichten …" **gar kein
  Eintrag** — ein dauerhaft ausgegrauter erklärt dem Nutzer nicht, warum er
  grau ist (KDE HIG).
- **Der Kürzel-Hinweis ist ein Hinweis.** Meta+N steht als Text am Eintrag und
  darf die Registrierung bei KGlobalAccel nicht doppeln; das Kürzel der
  Menü-Aktion trägt deshalb `Qt::WidgetShortcut` und erreicht nur das Fenster
  seines Menüs — und das hat keines, plasmashell zeichnet es.
- **Linksklick auf das Tray-Icon öffnet dasselbe Menü wie der Rechtsklick**
  (`ItemIsMenu`). Das weicht bewusst vom KDE-Standard ab, der den Linksklick
  für eine Hauptaktion vorsieht: Denkzettel hat kein Hauptfenster, sondern
  mehrere gleichrangige Wege, und die Recherche zum KDE-Verhalten wurde dem
  Kunden vorgelegt. Kundenentscheidung vom 01.08.2026, belegt in Issue #44, am
  02.08.2026 nach erneuter Vorlage bestätigt — **bei HIG- oder UI-Reviews kein
  Befund.**
- **Entdeckte Bedingung (Messung 02.08.2026, Issue #60): Getrennte Menüs für
  Links- und Rechtsklick sind unter Plasma/Wayland nicht zu haben.** Sie
  hießen `ItemIsMenu=false` plus ein eigenes Menü im
  `activateRequested`-Handler; das Menü müsste dann Denkzettel selbst
  zeichnen. Als Popup wird es erzeugt und zwei Millisekunden später wieder
  geschlossen — ein `Qt::Popup` braucht unter Wayland eine Elternfläche und
  einen Eingabe-Grab, und ein Klient mit nichts als einem Tray-Symbol hat
  beides nicht. Als gewöhnliches Fenster bleibt es stehen, aber die
  gewünschte Lage wird verworfen und KWin setzt es in die Bildschirmmitte.
  Deshalb bleibt es bei einem Menü; der Kunde hat den Rückfall am 02.08.2026
  entschieden. Beleg: `docs/scrum/reviews/sprint-04-s33-traymenues/`. Von den
  drei in Wireframe 5a benannten HIG-Abweichungen bleibt damit **nur A1**
  (Linksklick öffnet ein Menü); A2 (zwei verschiedene Menüs) und A3 („Beenden"
  nur über den Rechtsklick) entfallen ersatzlos, weil es die zweite Liste
  nicht gibt.
- **Zweite entdeckte Bedingung (ebenda): Die Symbolnamen erreichen Plasma nur,
  solange ein Symbol-Thema auflösbar ist.** Ohne Plattform-Thema enthalten die
  Suchpfade nichts als die Qt-Ressource, `QIcon::fromTheme()` liefert ein
  leeres Symbol ohne Namen, und das Menü käme unbebildert an. Unter Plasma ist
  das Thema da; für Testläufe ist `QT_QPA_PLATFORMTHEME=kde` deshalb
  Voraussetzung, nicht Zierde.
- Icon-Zustände: normal · „Vorschlag wartet" (Badge) · Fehlerzustand
  (Analyse-/Transkriptionsfehler, Tooltip nennt Ursache).
- Icons in v1 aus dem Breeze-Bestand (App- und Tray-Icon abgeleitet, Badge
  als Overlay gezeichnet) — keine eigene Grafikarbeit.
- **KNotification** bei: neuem Vorschlags-Paket, Volllauf-Mahnung,
  wiederholtem Fehler. Keine Benachrichtigung für Routineläufe.

## 11. Volllauf-Schutz

- Zwei Kriterien, beide einstellbar: **Anzahl** unexportierter Notizen
  (Default 200) ODER **Alter** der ältesten unexportierten Notiz (Default
  30 Tage).
- Bei Überschreiten: Tray-Mahnung + Benachrichtigung; der nächste Analyse-Lauf
  erzeugt bevorzugt Bündel (inkl. „Vermischtes", 7.3). **Nie** automatischer
  Export.

## 12. Transkription

- Job-Queue (`transcribe_jobs`), seriell abgearbeitet (eine GPU); überlebt
  Neustarts (Queue in der DB).
- **whisper.cpp** (Default): Vulkan-Build aus dem AUR (`whisper.cpp-vulkan`
  o. ä.; Paketname bei Umsetzung prüfen). Aufruf als Subprozess:
  Audio per `ffmpeg` nach 16-kHz-Mono-WAV (temporär), dann
  `whisper-cli -m <modell> -f <wav> -l de -oj` → JSON-Transkript.
  Modellgröße einstellbar (Default `small`, Auswahl tiny–large-v3; Download
  der GGML-Modelle beim ersten Gebrauch mit Fortschritt, Ablage unter
  `~/.local/share/denkzettel/models/`).
- **WhisperX (ROCm/GPU)**: konfigurierbarer Aufruf-Pfad, Subprozess mit
  `--language de`, JSON-Auswertung, ohne Diarisierung. **Vorbedingung**
  (Befund Schätzklausur 31.07.2026): Auf dem Entwicklungsrechner ist WhisperX noch nicht
  installiert — der PoC des RPG-Audio-Projekts lief mit whisper.cpp. Die
  Anbindung wird erst gebaut/abgenommen, wenn die Installation existiert
  (entsteht im RPG-Audio-Projekt); bis dahin ist whisper.cpp der einzige
  aktive Weg.
- Fehlerpfad: 2 Fehlversuche → Job pausiert, Tray-Fehlerzustand, Notiz bleibt
  als `audio`-Notiz ohne Transkript sichtbar/abspielbar (nichts geht verloren).

## 13. Einstellungen (Dialog)

Seitenliste gemäß Konzept: **KI-Provider** (Provider-Wahl, LLM- und
Embedding-Modell, Verbindung testen), **Analyse** (sofort/periodisch mit
Intervall/auf Abruf), **Export** (Vault-Pfad mit Ordner-Wahl, Volllauf-Schwellen
Anzahl + Tage, Bündel-Schwelle), **Sprachnotizen** (Backend whisper.cpp/
WhisperX, Modellgröße, WhisperX-Pfad), **Kürzel** (KKeySequenceWidget für
beide Shortcuts).

## 14. Fehlerbehandlung und Loop-Disziplin

Der periodische Analyse-Lauf ist ein Loop im Sinne der Loop-Konventionen:

- **goal met**: alle Notizen `analysiert`, Vorschläge erzeugt → Lauf endet.
- **budget**: ein Lauf verarbeitet max. 50 Notizen (Rest im nächsten Lauf).
- **stalled**: gleicher Fehler 2× → betroffene Notiz überspringen, melden.
- **needs a human**: alle Überführungen sind ohnehin bestätigungspflichtig.

Meldewege: Tray-Zustand + Tooltip (leise), KNotification (wichtig), Logdatei
`~/.local/share/denkzettel/denkzettel.log` (Details, mit Rotation).

## 15. Build, Abhängigkeiten, Paketierung

- **CMake** + ECM (Extra CMake Modules), C++20.
- Qt 6: Widgets, Sql, Network, Multimedia, **DBus**. KF6: KGlobalAccel,
  KConfig, KNotifications, KStatusNotifierItem, KWallet (Framework: KWallet),
  **KDBusAddons** (KDBusService/Einzelinstanz), **KWidgetsAddons**
  (KMessageWidget — Meldungen im Fenster; KMessageDialog samt
  KStandardGuiItem — Wächterdialog, Abschnitt 9), **KWindowSystem**
  (KWindowConfig — Fenstergröße über Sitzungen; die Position setzt ein
  Wayland-Client nicht selbst, siehe Abschnitt 3), KI18n (App-Sprache Deutsch;
  `i18n()`-Aufrufe sind KDE-Standardpraxis für alle sichtbaren Strings —
  keine Übersetzungs-Roadmap, nur Konvention). UI-Fließtexte (Platzhalter,
  Hinweise, Dialoge) sprechen den Nutzer in unpersönlicher Infinitivform an
  („Zum Lesen links eine Notiz auswählen.") — einmal app-weit festgelegt
  statt je Fenster (PO-Entscheidung 31.07.2026, Gestaltungsauftrag S8).
- Laufzeit-Abhängigkeiten: `ffmpeg` (Audio-Konvertierung), optional
  `whisper.cpp` (Vulkan) und `task` (Taskwarrior) — beides wird zur Laufzeit
  erkannt; fehlt eines, sind nur die betroffenen Funktionen deaktiviert
  (mit Hinweis in den Einstellungen), die App bleibt nutzbar.
- Paketierung: zunächst lokales `cmake --install` — mit
  `-DCMAKE_INSTALL_PREFIX=/usr`, denn nur dann landet der
  XDG-Autostart-Eintrag in `/etc/xdg/autostart`; unter dem
  CMake-Standard `/usr/local` liest ihn keine Plasma-Sitzung
  (Sprint-2-Befund, Issue #6). PKGBUILD/AUR nach Stabilisierung.

## 16. Teststrategie

- **Unit (QTest)**: Suchoperator-Parser, Clustering (mit synthetischen
  Vektoren), Prompt-/JSON-Schema-Verarbeitung (Provider gemockt),
  Export-Formatter (Sammelnotiz-Markdown), Taskwarrior-Kommandozeilen-Bau,
  Dateinamens-/Pfadlogik.
- **Integration**: Store-Schicht gegen echte SQLite (Tempfile), FTS-Trigger,
  Lösch-Transaktion inkl. Audio-Datei.
- **Manuell (Checkliste je Meilenstein)**: Shortcut-Weg unter Wayland,
  Fokusverhalten, Aufnahme mit echtem Mikrofon, Whisper-Durchlauf auf der
  7900 XTX, Export in einen **Test-Vault** (nie der echte), Taskwarrior
  gegen ein **eigenes `TASKDATA`-Testverzeichnis** (nie der Produktivbestand).
- **Migrationstest**: Sobald die erste reale Schema-Migration existiert,
  prüft ein Test das Upgrade einer Bestands-DB von Version n auf n+1.
- **Bedingung für Symbol- und Dialogzusicherungen** (entdeckt bei #60,
  bestätigt bei #66/#67 — DoD 4/B9): Tests, die Symbolnamen oder das
  Aussehen eines Meldungsdialogs zusichern, laufen mit
  `QT_QPA_PLATFORM=offscreen` **und `QT_QPA_PLATFORMTHEME=kde`. Ohne das
  Plattform-Thema** löst `QIcon::fromTheme()` nichts auf und liefert ein
  Symbol **ohne Namen** — die Zusicherung ist dann rot, ohne dass am Bau
  etwas fehlt —, und die Plattformintegration baut den Meldungsdialog nicht
  so, wie ein Sitzungsnutzer ihn sieht (Abschnitt 9). Das gilt derzeit für
  `shelltest` und `librarytest`. **Es ersetzt keine Plasma-Sitzung:** Der
  Bildnachweis am installierten Stand bleibt.
- KI-Qualität (Klassifikation/Clustering) wird nicht automatisiert getestet —
  der Vorschlags-Review ist die menschliche Kontrollinstanz.

## 17. Meilensteine

1. **M1 Capture-Kern**: Daemon, Tray, KGlobalAccel, Text-Capture, SQLite-Store.
   *Nutzbar: Gedanken festhalten.*
2. **M2 Bibliothek + Suche**: Fenster, Liste, Detail (lesen/bearbeiten/löschen),
   FTS + Operatoren.
3. **M3 KI-Basis**: Provider-Abstraktion (Ollama), Klassifikation + Tags,
   Kategorien-Sidebar, Einstellungen-Seiten KI/Analyse.
4. **M4 Sprachnotizen**: Aufnahmefenster, Queue, whisper.cpp-Backend,
   Player in der Bibliothek; WhisperX-Anbindung.
5. **M5 Vorschläge**: Embeddings + Clustering, Bündel- und Task-Vorschläge,
   Review-UI, Obsidian- und Taskwarrior-Ausführung, Volllauf-Schutz,
   Voll-Export.
6. **M6 Provider-Ausbau**: openrouter, OpenAI per API-Key (gemäß 7.5), KWallet.
7. **M7 Politur**: Icon-Zustände, Benachrichtigungs-Feinschliff, Logging,
   PKGBUILD.

Jeder Meilenstein endet mit der manuellen Checkliste (16) und einem
lauffähigen Stand.
