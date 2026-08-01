# Heilungslauf zu den Sprint-2-Kundenbefunden (01.08.2026)

Belege der Selbst-Sichtprüfung des Entwicklers (Beschlüsse B1, B3, B5, B7 aus
`docs/scrum/sprints/sprint-02.md`, Abschnitt 9.5) zu den beiden Befunden der
gescheiterten Abnahme vom 01.08.2026. Die Issues #5 und #7 bleiben offen — die
Kundenabnahme steht aus.

## Befund 2 — Bibliotheks-Layout (#7)

| Datei | Inhalt |
|---|---|
| `layout-tests-rot.txt` | Testlauf vor der Heilung: Kopfzeile 300 px bei 900×600 und 400 px bei 1200×800 gegen einen `sizeHint` von 41 px; mit Meldung nur 265 px für den Splitter |
| `layout-tests-gruen.txt` | derselbe Testlauf nach dem Stretch-Faktor |
| `layout-geometrie.txt` | gemessene Geometrie des geheilten Fensters in allen vier gezeigten Zuständen |
| `heilung-900x600.png` | Normalfall, 900×600, Breeze, offscreen |
| `heilung-1200x800.png` | Normalfall, 1200×800 — die zweite Fenstergröße aus B2 |
| `heilung-leerzustand-900x600.png` | Leerzustand (Wireframe 2c) |
| `heilung-meldung-900x600.png` | Meldungszustand nach dem Löschen |
| `shot.cpp` | Helfer, der die vier Bilder erzeugt; Bau- und Aufrufbefehl stehen im Kopfkommentar |

## Befund 1 — Meta+N ohne Wirkung (#5)

| Datei | Inhalt |
|---|---|
| `kuerzel-vorher.txt` | roher Lauf des ungeheilten Daemons am echten Sitzungsbus: keine Komponente bei kglobalacceld, keine Meldung |
| `kuerzel-nachweis.txt` | die ganze Kette — Journal vor und nach der Heilung, beide Meldezweige, Mitschnitt der `KNotification` auf dem Bus, Gegenprobe gegen fremde Komponenten |
| `kuerzel-probe.cpp` | Gegenprobe: was `KGlobalAccel::globalShortcut()` für bekannte Komponenten liefert und was für unsere |

Zu wissen wert: `qWarning()` landet auf Ganymed im Journal, nicht auf stderr.
Ein Terminalmitschnitt des Daemons ist deshalb kein Beleg — geprüft wird mit
`journalctl --user -t denkzetteld`.
