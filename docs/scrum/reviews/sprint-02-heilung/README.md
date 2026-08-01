# Heilungslauf zu den Sprint-2-Kundenbefunden (01.08.2026)

Belege der Selbst-Sichtprüfung des Entwicklers (Beschlüsse B1, B3, B5, B7 aus
`docs/scrum/sprints/sprint-02.md`, Abschnitt 9.5) zu den beiden Befunden der
gescheiterten Abnahme vom 01.08.2026. Die Issues #5 und #7 bleiben offen — die
Kundenabnahme steht aus.

## Befund 2 — Bibliotheks-Layout (#7)

| Datei | Inhalt |
|---|---|
| `layout-tests-rot.txt` | Testlauf vor der Heilung: Kopfzeile 300 px bei 900×600 und 400 px bei 1200×800 gegen einen `sizeHint` von 41 px; mit Meldung nur 265 px für den Splitter |
| `meldungsband-rot.txt` | Nachheilung zum UI-Review: das Band vor `setWordWrap(false)` — Text bei y=11, Knopf bei y=34, also zwei Zeilen |
| `layout-tests-gruen.txt` | beide Geometriefunktionen nach Stretch-Faktor und Nachheilung |
| `layout-geometrie.txt` | gemessene Geometrie des geheilten Fensters in allen fünf gezeigten Zuständen |
| `heilung-900x600.png` | Normalfall, 900×600, Breeze, offscreen |
| `heilung-1200x800.png` | Normalfall, 1200×800 — die zweite Fenstergröße aus B2 |
| `heilung-leerzustand-900x600.png` | Leerzustand 1 (Wireframe 2c): noch keine Notiz |
| `heilung-ohne-auswahl-900x600.png` | Leerzustand 2 (Wireframe 2c): Notizen da, keine ausgewählt |
| `heilung-meldung-900x600.png` | Meldungszustand nach dem Löschen, Band einzeilig (49 px) |
| `shot.cpp` | Helfer, der die fünf Bilder erzeugt; Bau- und Aufrufbefehl stehen im Kopfkommentar |

## Befund 1 — Meta+N ohne Wirkung (#5)

| Datei | Inhalt |
|---|---|
| `kuerzel-vorher.txt` | roher Lauf des ungeheilten Daemons am echten Sitzungsbus: keine Komponente bei kglobalacceld, keine Meldung |
| `kuerzel-nachweis.txt` | die ganze Kette — Journal vor und nach der Heilung, beide Meldezweige, Mitschnitt der `KNotification` auf dem Bus, Gegenprobe gegen fremde Komponenten |
| `kuerzel-probe.cpp` | Gegenprobe: was `KGlobalAccel::globalShortcut()` für bekannte Komponenten liefert und was für unsere |

Zu wissen wert: `qWarning()` landet auf Ganymed im Journal, nicht auf stderr.
Ein Terminalmitschnitt des Daemons ist deshalb kein Beleg — geprüft wird mit
`journalctl --user -t denkzetteld`.

## Nachheilung zum UI-Review vom 01.08.2026

Der UI-Review (`docs/scrum/reviews/2026-08-01-ui-review-s5-heilung.md`) hat
kein `fail` gefunden, aber zwei `warn` und einen Textbefund. Alle drei sind
eingearbeitet: einzeiliges Meldungsband (B3), die beiden fehlenden
Geometrie-Zusicherungen (B5) und der Meldungstext des seltenen
Kürzel-Zweigs (B9). Die Bildstrecke und die Messwerte oben stammen aus dem
Stand **nach** dieser Nachheilung; zwei Randbemerkungen des Reviews sind dabei
mit erledigt — der Helfer setzt jetzt das Fenstersymbol (B6), und Wireframe 2c
hat beide Leerzustände als Bild (B7).
