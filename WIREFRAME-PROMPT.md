# Wireframe-Auftrag: „Denkzettel" — Quick-Capture-Notiz-App für KDE Plasma

## Kontext

Denkzettel ist eine schnelle, leichtgewichtige Notiz-App für den Linux-Desktop
(KDE Plasma, Breeze-Designsprache). Kernidee: Ein globales Tastenkürzel öffnet ein
zentriertes, rahmenloses Eingabefenster; ein Gedanke wird eingetippt und mit
Strg+Enter weggespeichert — null Zeremonie. Ein zweites Kürzel öffnet ein
Mini-Aufnahmefenster für Sprachnotizen, die lokal per Whisper transkribiert werden.
Ein Hintergrund-Dienst klassifiziert die Notizen per KI (TODOs, Ideen, CLI-Befehle,
persönliche Notizen, Software-Ideen), vergibt Tags und Kategorien, baut einen
Suchindex und legt dem Nutzer Vorschläge zur Bestätigung vor: thematisch gebündelte
Exporte in einen Obsidian-Vault sowie aus TODO-Notizen extrahierte Tasks für
Taskwarrior.

## Auftrag

Erstelle Wireframes (Desktop; 16:9-Bildschirmkontext, wo die Fensterposition
relevant ist) für die sechs Oberflächen unten. Fokus auf Layout, Hierarchie und
Bedienlogik — keine ausgearbeitete Farbwelt nötig; gern dezent an KDE Breeze
angelehnt (klare Flächen, wenig Chrome, Systemschrift). Alle Beschriftungen
auf Deutsch.

## Screen 1: Capture-Fenster (das Herzstück)

- Kleines, zentriertes, rahmenloses Fenster über dem Desktop (ca. 500–600 px
  breit), bekommt sofort den Fokus
- Inhalt: NUR ein mehrzeiliges Texteingabefeld plus eine dezente Fußzeile
  „Esc verwirft · Strg+Enter speichert"
- Kein Menü, keine Toolbar, keine Formatierungsleiste — radikal minimal
- Zwei Zustände zeigen: leer (mit Platzhaltertext) und mit getipptem Text
- Optional: App-Name klein und unaufdringlich

## Screen 2: Bibliothek

- Normales Fenster: Verwaltungs- und Stöberansicht
- Linke Sidebar: KI-erstellte Kategorien mit Zählern
  (Alle, TODOs, Ideen, CLI-Befehle, Persönlich, Software-Ideen)
- Hauptbereich: chronologische Notizliste — Zeitstempel, erste Zeile(n),
  KI-Tags als kleine Chips; Sprachnotizen zusätzlich mit Abspielsymbol und Dauer
- Oben: Volltextsuchfeld (versteht Operatoren wie `tag:`, `typ:`, `vor:`);
  Button „Vorschläge" mit Badge (Anzahl wartender Vorschläge)
- Auswahl einer Notiz zeigt Vorschau/Detail (Lese- und Bearbeiten-Ansicht,
  Löschen-Aktion); bei Sprachnotizen zusätzlich ein schlichter Audio-Player
  über dem Transkript

## Screen 3: Vorschlags-Review

- Dialog oder Ansicht innerhalb der Bibliothek; ein Ort für zwei Vorschlagsarten
- **Obsidian-Bündel:** Titel des Bündels (z. B. „7 Notizen zur
  Denkzettel-Entwicklung"), Liste der enthaltenen Notizen mit
  Abwahl-Checkboxen, Markdown-Vorschau der Zieldatei,
  Zielhinweis („→ Obsidian _INBOX")
- **Taskwarrior-Task:** Karte pro erkanntem TODO — editierbare Felder für
  Beschreibung, project, tags, due, priority; darunter der Notiz-Volltext als
  Annotation-Vorschau; Zielhinweis („→ Taskwarrior")
- Aktionen je Vorschlag: „Übernehmen", „Später", „Verwerfen"
- Beide Arten in einer Liste zeigen (z. B. ein Bündel + zwei Task-Karten)

## Screen 4: Einstellungen

- KDE-typischer Einstellungsdialog mit Seitenliste links:
  - **KI-Provider:** Ollama (lokal/Cloud, URL), openrouter.ai (API-Key),
    OpenAI (API-Key), Modellwahl (LLM + Embedding),
    „Verbindung testen"-Button
  - **Analyse:** Zeitpunkt wählbar — sofort / periodisch (mit Intervall) /
    auf Abruf
  - **Export:** Vault-Pfad, Volllauf-Schwelle
  - **Sprachnotizen:** Whisper-Backend als Auswahl (whisper.cpp ·
    WhisperX-ROCm/GPU), Modellgröße
  - **Kürzel:** Anzeige/Änderung der globalen Shortcuts (Text und Sprache)
- Für das Wireframe reichen die Seiten „KI-Provider", „Analyse" und „Sprachnotizen"

## Screen 5: Tray-Menü

- Kompaktes Kontextmenü am Systray-Icon: Capture öffnen (mit Kürzel-Hinweis),
  Sprachnotiz aufnehmen (mit Kürzel-Hinweis), Bibliothek, Analyse jetzt,
  Vorschläge (mit Zähler), Einstellungen, Beenden
- Icon-Zustände: normal / „Vorschlag wartet" (Badge)

## Screen 6: Aufnahmefenster (Sprachnotiz)

- Gleiche Machart wie das Capture-Fenster: klein, zentriert, rahmenlos,
  sofort fokussiert — die Aufnahme läuft beim Öffnen bereits
- Inhalt: dezenter Aufnahme-Indikator (Punkt/Pegel), laufende Zeitanzeige,
  Fußzeile „Esc verwirft · Strg+Enter speichert"
- Kein Wellenform-Editor, keine Pause-Taste, keine Transkript-Anzeige —
  die Transkription passiert später im Hintergrund

## Leitplanken

- Der Capture-Weg wird niemals mit UI belastet — Geschwindigkeit ist der Kern
  der App
- Keine Cloud-Sync-UI, kein Onboarding-Assistent, kein Rich-Text-Editor
- Desktop-App, kein Mobile/Responsive
- Deutsch, Breeze-nah; hell UND dunkel denkbar (ein Screen in Dunkel reicht)
