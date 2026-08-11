# Changelog

Alle nennenswerten Änderungen an Denkzettel stehen in dieser Datei —
aus Nutzersicht geschrieben, nach [Keep a Changelog](https://keepachangelog.com/de/1.1.0/)
gegliedert. Quelle sind die geschlossenen Issues des jeweiligen
Sprint-Milestones; rein technische Einträge bleiben draußen, jede Änderung
am Datenbank-Schema wird immer genannt. Die Versionszählung folgt
0.x-SemVer (Festlegung vom 02.08.2026; seit #61 sichtbar über
`denkzetteld --version`).

## [0.3.0] — noch nicht abgenommen

Sprint 9. **Zwei Befunde des Kunden aus der Durchsicht nach Sprint 8** — beide
betrafen die Lesbarkeit, und beide sind geheilt.

### Geändert

- **Das Erfassungsfenster zeigt, wo man tippt.** Der Textbereich bekommt Fläche
  und Kante aus der Grafik des Desktop-Themes (`widgets/lineedit`) — dieselbe
  Quelle wie die Fensterhülle, eine Ebene tiefer. Bis hierher war das Fenster
  ein durchgehender Farbblock, in dem der Eingabebereich nicht zu erkennen war.
  In der Sitzung hebt sich das Feld unter `default` um 1,79 : 1 von der Hülle ab
  — KRunners Feld liegt bei 1,41 : 1 (#100)
- **Die Notizliste trennt Einträge und Gruppen.** Zwischen zwei Notizen
  derselben Gruppe steht eine auf die Textkante eingerückte Haarlinie, über
  jedem Gruppenkopf außer dem ersten dieselbe Linie über die volle Breite.
  Gleiche Farbe, verschiedene Länge — die Rangfolge Notiz/Gruppe entsteht aus
  der Ausdehnung des Strichs. Kein Maß der Liste ändert sich dadurch (#101)

### Bekannte Grenze

- **Unter fünf der acht geprüften Desktop-Themes bleibt das Eingabefeld
  unsichtbar** — dort zeichnet die Theme-Grafik nur einen Hauch (Deckung 15 von
  255). Auf der Voreinstellung greift der Rückfall `default`, und dort trägt es.
  Die Heilung ist als #102 erfasst: Alle acht Themes führen einen Fokuszustand
  mit sichtbarer Kante, der das Feld unter allen sichtbar machen würde

### Anmerkung

Die Bilder in der README zeigen weiterhin den Stand vom 04.08.2026 — der
Bildläufer erzeugt seit der nativen Hülle ein unbrauchbares Bild (#96).

## [0.2.0] — 2026-08-05

Abgenommen am 05.08.2026 (Sprints 6 bis 8; die Abnahme lag beim Product Owner,
der Kunde sieht das Gesamtergebnis danach an). **Die erste Fassung mit einer
Versionsnummer** — bis hierher gab es keine, weil die Zahl die Anwendung nicht
erreichte.

### Hinzugefügt

- **Das Erfassungsfenster ist eine native Plasma-Überlagerung.** Rundung,
  Kontur und Schatten kommen aus dem Desktop-Theme, nicht aus eingebauten
  Werten; der Grund dahinter wird weichgezeichnet wie bei KRunner und den
  Benachrichtigungen. Gemessen ist die Fläche mit KRunner **bildpunktgleich**
  (#83)
- **Die Schrift kommt aus derselben Quelle wie die Fläche.** Bringt das
  Desktop-Theme eigene Farben mit, gelten sie — für den Notiztext und für die
  gedämpften Texte. Unter `breeze-light` steigt der Kontrast des Notiztextes
  von 1,1 : 1 auf 13,4 : 1 (#85)
- **`denkzetteld --version` und `--help`.** Beide antworten auch, während der
  Dienst läuft (#61)
- **Tooltips mit Tastenkürzel** an „Bearbeiten", „Löschen" und „Rückgängig"
  in der Bibliothek (#72)

### Behoben

- **Ein Klick auf eine angeschnittene Zeile wählt jetzt diese Zeile.** Vorher
  rückte das Bild und markierte die Nachbarzeile, während der Lesebereich die
  geklickte Notiz zeigte — Auswahl und Anzeige gingen auseinander (#71)
- **Die erste Notiz einer Gruppe holt ihren Tageskopf ins Bild.** Vorher stand
  bei einer Notiz von gestern „08:00" und nichts sagte, von welchem Tag (#70)

### Geändert

- Die Linterschwelle steht auf null und wird bei jedem öffentlichen Lauf
  geprüft; 88 Befunde sind geheilt, 37 mit Begründung stehengeblieben (#76)

**Keine Änderung am Datenbank-Schema.**

## [Unveröffentlicht]

Stand aus den Sprints 4 und 5, abgenommen am 02.08.2026 (Kundenabnahme). Eine
Versionsnummer bekamen diese Abnahmen nicht — die Regel wirkt erst ab #61, und
rückwirkend wird nicht nummeriert.

### Hinzugefügt

- **Notizen bearbeiten:** In der Bibliothek öffnet „Bearbeiten" oder F2
  den Editor; Speichern (Strg+Enter) und Abbrechen (Esc) mit Nachfrage
  bei ungespeicherten Änderungen — nichts wird still verworfen oder
  geschrieben. Die geänderte Notiz bleibt in der Trefferliste sichtbar,
  auch wenn sie nicht mehr zum Suchbegriff passt (#11)
- **Tray-Menü überarbeitet:** Alle Einträge auf Deutsch („Notiz
  erfassen" statt „Capture öffnen") und mit Symbolen; „Beenden" steht
  abgesetzt am Ende, getrennt von den Arbeitswegen (#60)
- **Symbole in der Bibliothek:** „Bearbeiten", „Löschen", „Speichern",
  „Abbrechen" und „Rückgängig" tragen jetzt Symbole aus dem Systemthema;
  die Nachfrage vor ungespeicherten Änderungen ebenso, samt Warnsymbol
  (#66, #67)

### Geändert

- Der Menüeintrag zeigt das Tastenkürzel Meta+N an; die Umbenennung gilt
  überall, auch in den Systemeinstellungen (Kurzbefehle) und im
  Startermenü (#60)
- Der Wächterdialog ist auf die KDE-Bauart umgestellt; „Speichern" bleibt
  die vorausgewählte Antwort (#66)

### Behoben

- Der Klick auf eine sichtbare Notiz einer anderen Tagesgruppe lässt die
  Liste nicht mehr springen (#57)
- Zeitstempel und Hinweise der Bibliothek folgen einem Wechsel des
  Farbschemas jetzt ohne Neustart (#58)

## [0.1.0] — 2026-08-02

Erster abgenommener Gesamtstand, erarbeitet in den Sprints 1–3
(Git-Tag `sprint-03-abschluss`).

### Hinzugefügt

- **Erfassungsfenster:** Ein Tastendruck (Meta+N) öffnet ein schlankes
  Eingabefeld, Enter speichert, Escape verwirft — keine Dateinamen, keine
  Dialoge (#4, #5, #42)
- **Dauerhafte lokale Ablage:** Notizen liegen in einer SQLite-Datenbank
  im Nutzerprofil — kein Cloud-Zwang, nichts verlässt den Rechner (#3)
- **Bibliothek:** Fenster mit Notizliste und Lesebereich; Notizen löschen
  mit Rückgängig-Weg (#7)
- **Posteingangs-Gliederung:** Die Notizliste gruppiert nach Heute ·
  Gestern · Diese Woche · Letzte Woche · Älter, die erste Zeile jeder
  Notiz dient als Betreff (#46)
- **Volltextsuche:** Das Suchfeld der Bibliothek findet Wortteile auch
  mitten im Wort und verzeiht fehlende Umlaute — „bucher" findet
  „Bücher", „grafieren" findet „fotografieren" (#8)
- **Tray-Symbol** mit eigenem Icon; Linksklick öffnet das Menü wie der
  Rechtsklick (#2, #43, #44)
- **Autostart:** Der Hintergrunddienst startet mit der Anmeldung; beim
  Erststart wird das Tastenkürzel eingerichtet (#6)

### Geändert

- **Datenbank-Schema auf Version 2** für die Volltextsuche. Ein
  bestehender Notizbestand wird beim ersten Start einmalig übernommen;
  alle Notizen bleiben erhalten (#8, #9)

### Behoben

- Kleintexte im Erfassungsfenster folgen jetzt einem Themewechsel, statt
  in der alten Farbe stehenzubleiben (#54)
