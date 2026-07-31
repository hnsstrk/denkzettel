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
- FTS5-Tokenizer: `unicode61 remove_diacritics 2` — „bucher" findet
  „Bücher"; deutsche Umlaut-Toleranz ist Kernanforderung der Suche.

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
  Notizliste (Zeitstempel, erste Zeilen, Tag-Chips; Sprachnotizen mit ▶ und
  Dauer), Suchfeld (Abschnitt 6), Button „Vorschläge" mit Badge.
- Detailansicht: **Lese- und Bearbeiten-Ansicht** (Entscheidung drittes
  Interview — v. a. für fehlerhafte Transkripte). Bearbeiten behält
  Kategorie/Tags und `state`, setzt aber `needs_reembed = 1` — der nächste
  Analyse-Lauf erneuert nur das Embedding (7.2), denn es veraltet mit dem
  Text. Löschen-Aktion mit 5-Sekunden-Undo (Spec-Ergänzung, nicht im
  Konzept: rein client-seitig verzögertes Löschen, kein Soft-Delete-Zustand
  in der DB).
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

- **KStatusNotifierItem**, dauerhaft. Menü: Capture öffnen (Meta+N),
  Sprachnotiz aufnehmen (Meta+Umschalt+N), Bibliothek, Analyse jetzt,
  Vorschläge (Zähler), Einstellungen, Beenden.
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
  **KDBusAddons** (KDBusService/Einzelinstanz), KI18n (App-Sprache Deutsch;
  `i18n()`-Aufrufe sind KDE-Standardpraxis für alle sichtbaren Strings —
  keine Übersetzungs-Roadmap, nur Konvention). UI-Fließtexte (Platzhalter,
  Hinweise, Dialoge) sprechen den Nutzer in unpersönlicher Infinitivform an
  („Zum Lesen links eine Notiz auswählen.") — einmal app-weit festgelegt
  statt je Fenster (PO-Entscheidung 31.07.2026, Gestaltungsauftrag S8).
- Laufzeit-Abhängigkeiten: `ffmpeg` (Audio-Konvertierung), optional
  `whisper.cpp` (Vulkan) und `task` (Taskwarrior) — beides wird zur Laufzeit
  erkannt; fehlt eines, sind nur die betroffenen Funktionen deaktiviert
  (mit Hinweis in den Einstellungen), die App bleibt nutzbar.
- Paketierung: zunächst lokales `cmake --install`; PKGBUILD/AUR nach
  Stabilisierung.

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
