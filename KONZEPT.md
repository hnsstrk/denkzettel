# Denkzettel — Konzeptnotizen

Stand: 2026-07-31, 19:15 — Rohfassung aus dem Konzeptgespräch; drei Design-Interviews
eingearbeitet (Grundentscheidungen · Sprachnotizen + Taskwarrior · Wireframe-Details +
Restfragen inkl. KI-Architektur). Alle Ursprungs-Streitfragen entschieden, es bleiben
drei Spec-Detailfragen. Noch keine verabschiedete Spec.

## Kerngedanke

**Schnell und leichtgewichtig.** Volle KDE-Integration.

## Architektur: zwei Komponenten

1. **Capture-Fenster** (Vordergrund) — die schnelle Eingabe
2. **Sammel- und Analyse-Dienst** (Hintergrund) — sammelt, analysiert, räumt auf

## Capture-Fenster

- Aufruf über **globalen Shortcut**, muss unter **KDE Wayland** funktionieren
  (bekannt problematisch für Fremd-Apps; Lösungsweg für native KDE-Apps:
  KGlobalAccel bzw. XDG-GlobalShortcuts-Portal — für Qt/KDE-Programme sauber machbar,
  das Electron-Problem betrifft uns nicht)
- Fenster öffnet **mitten auf dem Desktop** und bekommt sofort den **Fokus**
- Eingabe abschicken → Fenster weg, Notiz gespeichert
- Inhalt: **vermutlich nur Text** — noch offen, Vorschläge erwünscht (→ Offene Fragen)

## Speicherung

- Eine Notiz = eine Datei, benannt nach **Datum + Uhrzeit im ISO-Format**
  (z. B. `2026-07-31T13-32-05.md`)
- Ablage als **Text oder Markdown**
- **SQLite diskutabel**, wenn gut begründet (Entscheidung offen; Hybrid denkbar:
  Markdown-Dateien als Quelle der Wahrheit + SQLite nur für Index/Metadaten)

## Hintergrund-Dienst

- Sammelt alle Notizen, **analysiert und räumt auf**
- **KI-Klassifikation** — herausfinden, was eine Notiz ist:
  - Kommandozeilen-Skripte oder -Befehle
  - Persönliche Notizen
  - TODOs
  - Ideen
  - Ideen für Softwareentwicklung
  - (Liste erweiterbar)
- Baut im Hintergrund einen **Suchindex** auf
- Erstellt per KI **Verknüpfungen, Tags, Kategorien**

## KI-Anbindung (konfigurierbar, mehrere Provider)

- **Ollama** — lokal oder Cloud
- **openrouter.ai**
- **OpenAI** — ursprünglich als „Anmeldung über ChatGPT" gedacht; Recherche
  31.07.2026: nur per API-Key machbar (siehe drittes Interview und SPEC 7.5)

## Obsidian-Überführung (Volllauf-Schutz)

- Die App darf **nicht mit Notizen volllaufen**
- Es muss einen Weg geben, Notizen **sinnvoll gebündelt** nach Obsidian zu überführen
  (Ziel-Vault: Vault Obsidian, dort `_INBOX/`-Staging als natürlicher Andockpunkt)

## Entschieden (Design-Interview 31.07.2026)

- **Name:** Denkzettel (Namensprüfung 31.07.2026 gegen ~400 Apps + AUR/crates.io/PyPI/Flathub/GitHub: frei)
- **Fenstermodell:** zentriertes Schwebefenster mit Fokus
- **Zielplattform:** KDE Plasma unter Wayland (Entwicklungsrechner: Plasma 6.7, Qt 6.11)
- **Capture-Fenster:** radikal pur — nur ein Textfeld. Strg+Enter speichert, Esc verwirft.
  Keine Marker, keine Anhänge. Klassifikation ist allein Sache der KI im Hintergrund.
- **Speicherung: SQLite.** Denkzettel ist Durchlauf-Speicher, der Klartext-Ort bleibt Obsidian.
  Auflage: ein simpler **Voll-Export** (alle Notizen als Markdown) als Rettungsweg,
  damit nichts je in der DB gefangen ist.
- **Stack:** so KDE-nativ wie möglich, primärer Fokus schnell → Capture-Fenster C++/Qt/KF6
  mit echtem KGlobalAccel. Sprache des Hintergrund-Dienstes: im dritten Interview
  entschieden — ebenfalls C++/Qt/KF6 (siehe unten).
- **KI-Analyse-Zeitpunkt: konfigurierbar in den Einstellungen** (sofort / periodisch
  mit Intervall / auf Abruf) → die App braucht einen Einstellungen-Dialog.
- **Obsidian-Export: KI-Vorschlag + Bestätigung.** Der Dienst bündelt thematisch und legt
  ein Paket vor; erst nach Bestätigung wird Markdown nach `_INBOX/` geschrieben
  („Melden, nicht heilen").
- **Bibliotheks-Fenster:** ja — Kategorien-Sidebar mit Zählern, Notizliste, Volltextsuche,
  Vorschlags-Review (siehe zweites Interview).
- **Rückmeldung: Tray-Icon dauerhaft** — Status, Menü (Capture / Bibliothek / Analyse jetzt /
  Vorschläge), Benachrichtigung bei fertigem Vorschlag (Export-Bündel oder
  Taskwarrior-Task).

## Entschieden (zweites Design-Interview 31.07.2026): Sprachnotizen + Taskwarrior

- **Sprachnotizen: ja, über zweiten globalen Shortcut.** Eigenes Mini-Aufnahmefenster
  (gleiche Machart wie das Capture-Fenster: zentriert, rahmenlos), die Aufnahme läuft
  sofort los. Strg+Enter stoppt und speichert, Esc verwirft. Das Text-Capture-Fenster
  bleibt unangetastet — radikal pur.
- **Transkription lokal, asynchron im Hintergrund-Dienst** — das Aufnahmefenster
  speichert nur die Aufnahme und schließt sich sofort (kein Warten auf Whisper,
  kein Korrektur-Editor im Capture-Weg). Das Transkript durchläuft danach dieselbe
  Pipeline wie Text (Klassifikation, Tags, Index); eine Sprachnotiz kann also
  z. B. auch als TODO erkannt werden.
- **Whisper-Backend wählbar in den Einstellungen** (Vault-Recherche 31.07.2026,
  Quellnotizen: „RPG Audio-Erfassung und Transkription" Abschn. 6.1,
  „RPG-Audio Projektplan" Stufe 2):
  Auf dem Entwicklungsrechner sind zwei GPU-taugliche Wege belegt — **whisper.cpp** mit ROCm-/
  Vulkan-Backend (auf der RX 7900 XTX / gfx1100 verifiziert, PoC transkribiert
  echte Sessions) und der **WhisperX-ROCm-Stack aus dem RPG-Audio-Projekt**
  (Whisper large-v3 + pyannote 3.1; die Sprecher-Diarisierung braucht Denkzettel
  nicht). Beide stehen in den Einstellungen zur Auswahl.
  *(Korrektur 31.07.2026 abends, Befund der Schätzklausur: WhisperX ist auf
  Entwicklungsrechner noch **nicht installiert** — geplant im RPG-Audio-Projekt, der PoC
  lief mit whisper.cpp. Die Auswahl bleibt, die Installation ist Vorbedingung;
  Details SPEC 12.)*
- **Audio bleibt an der Notiz**, solange sie in Denkzettel lebt — in der Bibliothek
  abspielbar, Transkriptionsfehler sind nachhörbar. Export oder Löschung der Notiz
  nimmt das Audio mit weg (Durchlauf-Charakter).
- **Taskwarrior-Überführung: KI-Vorschlag + Bestätigung** (gleiches Muster wie der
  Obsidian-Export). Aus TODO-Notizen extrahiert die KI einen Task-Vorschlag —
  Beschreibung, project, tags, ggf. due —, der vor der Bestätigung editierbar ist.
  Erst nach Bestätigung läuft `task add`.
- **Nach Übernahme wird die Notiz gelöscht.** Ist sie länger als die Task-Beschreibung,
  hängt der Volltext als Taskwarrior-Annotation am Task — nichts geht verloren.
- **Vorschlags-Review statt Export-Review:** ein Ort für beide Vorschlagsarten
  (Obsidian-Bündel und Taskwarrior-Tasks), ein Tray-Badge zählt beides.

## Entschieden (drittes Design-Interview 31.07.2026): Wireframe-Details + Restfragen

- **Capture-Fenster wächst mit dem Text** (bis ~8 Zeilen, dann Scrollbalken) —
  aus dem Wireframe übernommen.
- **Bibliothek: Bearbeiten erlaubt** — Abweichung vom Wireframe (dort „Nur-Lesen"):
  Notizen, vor allem fehlerhafte Whisper-Transkripte, sind in der Detailansicht
  editierbar, bevor sie exportiert werden.
- **Task-Karte bekommt das priority-Feld** (aus dem Wireframe übernommen; die
  Feldliste des zweiten Interviews kannte es noch nicht). Die KI füllt es nur bei
  klarem Signal in der Notiz („dringend!"), sonst bleibt es leer.
- **Standard-Kürzel: Meta+N (Text-Capture), Meta+Umschalt+N (Sprachnotiz).**
  Auf dem Entwicklungsrechner gegen die KDE-Belegung geprüft (31.07.2026): beide frei; Meta-Kürzel
  fängt der Compositor vor allen Anwendungen ab. In den Einstellungen änderbar.
- **Suche mit Operatoren:** Die Volltextsuche versteht Syntax wie `tag:backup`,
  `vor:2026-07`, `typ:audio` direkt im Suchfeld — keine zusätzliche Filter-UI.
- **Bündelung: ab 3 thematisch zusammenhängenden Notizen** (Schwelle einstellbar);
  Zielnotiz ist eine Sammelnotiz pro Thema mit Datums-Abschnitten (wie die
  Markdown-Vorschau im Wireframe).
- **Volllauf-Schutz: Anzahl + Alter, das Tray mahnt.** Zwei einstellbare Kriterien
  (z. B. mehr als 200 Notizen ODER älteste unexportierte Notiz älter als 30 Tage).
  Bei Überschreiten Tray-Hinweis und bevorzugte Export-Vorschläge — nie
  automatischer Export („Melden, nicht heilen").
- **OpenAI: OAuth („Sign in with ChatGPT") von Anfang an** — kein
  API-Key-Zwischenschritt. Machbarkeit und Flow für Desktop-Apps sind als
  Rechercheauftrag Teil der Spec (→ Offene Fragen).
  **Nachtrag — Recherchevorbehalt ausgelöst (31.07.2026):** „Sign in with
  ChatGPT" ist ein reines Identitäts-Login ohne Modellzugriff; der
  inoffizielle Codex-Abo-Weg ist eine ToS-Grauzone und bietet keine
  Embeddings. Konsequenz gemäß Vorbehalt: **v1 nutzt den manuellen API-Key**,
  ein Abo-Weg bleibt spätere Option. Beleg und Details:
  `recherche/2026-07-31-openai-oauth-machbarkeit.md`, SPEC 7.5.
- **Hintergrund-Dienst in C++/Qt/KF6** — ein Stack mit dem Capture-Fenster,
  Kopplung über D-Bus, KI-REST und OAuth via Qt Network; Whisper-Engines laufen
  als Subprozesse.
- **Wayland-Shortcut über KGlobalAccel direkt** — die Kürzel erscheinen in den
  Plasma-Systemeinstellungen; die Portal-Route entfällt.
- **Whisper-Anbindung: whisper.cpp mit Vulkan-Backend aus dem AUR** (keine
  ROCm-Stack-Kopplung, übersteht ROCm-Updates); **WhisperX über konfigurierbaren
  Pfad** — Installation ist Vorbedingung (siehe Korrektur im zweiten Interview).
  Beide laufen als Subprozess des Dienstes.
- **Audio-Ablage: Datei + DB-Referenz.** Audio unter
  `~/.local/share/denkzettel/audio/`, SQLite hält nur den Verweis; Löschen der
  Notiz räumt die Datei mit ab. Der **Voll-Export-Rettungsweg** wird dadurch
  trivial: ein Ordner mit einer `.md` je Notiz plus `audio/`-Unterordner.
- **KI-Architektur: LLM + Embedding-Modell.** Embeddings clustern die Notizen zu
  Bündel-Kandidaten (ein Vektor je Notiz, als BLOB in SQLite; Brute-Force-Vergleich
  reicht bei diesem Bestand, keine Vektor-DB), das LLM übernimmt Klassifikation,
  Tags, Task-Extraktion und Bündel-Formulierung. Lokale Defaults via Ollama:
  `qwen3:8b` (LLM) und `bge-m3` (mehrsprachiges Embedding); bei Cloud-Providern
  deren Embedding-API. Beides in den Einstellungen änderbar.

## Anwendungseinstellungen

Konsolidierter Umfang des Einstellungen-Dialogs (KDE-typisch, Seitenliste links):

- **KI-Provider:** Provider-Wahl, LLM- und Embedding-Modell, „Verbindung testen"
  (Provider-Liste → „KI-Anbindung" oben)
- **Analyse:** Zeitpunkt der KI-Analyse — sofort / periodisch (mit Intervall) /
  auf Abruf
- **Export:** Obsidian-Vault-Pfad, Volllauf-Schwelle
- **Sprachnotizen:** Whisper-Backend als Auswahl (whisper.cpp ROCm/Vulkan ·
  WhisperX-ROCm aus dem RPG-Audio-Bestand), Modellgröße
- **Kürzel:** die beiden globalen Shortcuts (Meta+N Text-Capture,
  Meta+Umschalt+N Sprachnotiz)

## Offene Fragen

Keine mehr. Die acht Ursprungsfragen wurden durch das dritte Design-Interview
entschieden; die drei danach verbliebenen Spec-Detailfragen sind beantwortet:

1. Themen-Clustering → SPEC 7.3 (Single-Linkage über Cosine-Ähnlichkeit,
   Schwelle 0,60 intern; „Vermischtes"-Bündel für clusterlose Altbestände)
2. OAuth „Sign in with ChatGPT" → Recherche abgeschlossen, nicht machbar;
   v1 per API-Key (Nachtrag im dritten Interview, SPEC 7.5,
   `recherche/2026-07-31-openai-oauth-machbarkeit.md`)
3. Suchoperator-Syntax → SPEC 6 (tag:/kat:/typ:/vor:/nach:/Phrase,
   UND-verknüpft)
