# Changelog

Alle nennenswerten Änderungen an Denkzettel stehen in dieser Datei —
aus Nutzersicht geschrieben, nach [Keep a Changelog](https://keepachangelog.com/de/1.1.0/)
gegliedert. Quelle sind die geschlossenen Issues des jeweiligen
Sprint-Milestones; rein technische Einträge bleiben draußen, jede Änderung
am Datenbank-Schema wird immer genannt. Die Versionszählung folgt
0.x-SemVer (Festlegung vom 02.08.2026; sichtbar in der Anwendung mit #61).

## [Unveröffentlicht]

Stand aus den Sprints 4 und 5, abgenommen am 02.08.2026 (Kundenabnahme; die
Versionsnummer folgt mit #61).

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
