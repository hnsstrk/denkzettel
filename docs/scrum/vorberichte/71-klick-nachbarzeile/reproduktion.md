# Reproduktion #71 im vorhandenen offscreen-Testaufbau

**Gegenstand:** Issue #71, „Bibliothek: Klick auf angeschnittene Zeile wählt die
Nachbarzeile" · **Datum:** 05.08.2026, Ganymed · **Quellstand:** `main` @
`581dacc` · **Bauplatz:** `build-repro71` (Debug), Ziel `librarytest`
**Belege:** `reproduktion.txt` (rohe Terminalausgabe beider Läufe),
`reproduktion-testfall.diff` (der Wegwerf-Testfall im Wortlaut)

**Die Frage:** Lässt sich der Befund mit den Mitteln reproduzieren, die
`tests/librarytest.cpp` schon benutzt — QTest-Mausereignisse auf dem Viewport,
offscreen —, oder braucht er eine Sitzungsprobe?

**Die Antwort in einem Wort: ja.** Beide Befunde des Issues treten im
vorhandenen Prüfweg auf. Es entsteht kein neuer Prüfweg: kein neuer Läufer,
kein neues Bauziel, keine angemeldete Sitzung, kein echter Zeiger.

---

## Aufbau

Der Wegwerf-Testfall übernimmt die Bestückung von
`leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked`
(`tests/librarytest.cpp:1505–1524`) unverändert: 8 Notizen von heute, 3 von
gestern, 8 von letzter Woche, `LibraryWindow` auf 900×600, `showLibrary()`.
Daraus ergibt sich ein Viewport von 279×552 bei einer Zeilenhöhe von 72 px und
ein Rollbereich 0..15 (`ScrollPerItem`).

Zwei Dinge unterscheiden ihn vom Bestandstest — und genau daran liegt es, dass
die Suite den Fehler bisher nicht sieht:

| | Bestandstest `:1541` | Wegwerf-Testfall |
|---|---|---|
| Zielzeile | eine **ganz sichtbare** Zeile | die **unten angeschnittene** Zeile |
| Klickpunkt | `visualRect(target).center()` | 5 px unter der Oberkante, im sichtbaren Streifen |

Die angeschnittene Zeile wird zur Laufzeit gesucht (oben im Bild, unten darüber
hinaus, Kopfzeilen übersprungen), und jeder Rollwert wird einzeln geklickt.
Gemessen wird vor und nach dem Klick: `verticalScrollBar()->value()`,
`visualRect(target).y()`, `currentIndex().row()` und die Zeilen aus
`selectionModel()->selectedIndexes()`.

---

## Befund 1 — der Rollsprung: reproduziert

Von 16 Rollwerten zeigen 14 eine angeschnittene Zeile. **In allen 14 rückt das
Bild beim Klick.** Der Rollwert springt um eine Zeile, in zwei Fällen um zwei.

| Versatz | Fälle | Rollwerte |
|---|---|---|
| **−72 px** (genau eine Zeilenhöhe) | 11 | 1, 2, 3, 4, 6, 7, 8, 10, 11, 12, 14 |
| −99 px | 1 | 0 |
| −107 px | 1 | 13 |
| −35 px | 1 | 9 |

Die im Issue genannten **72 px** sind der Regelfall und decken sich mit der
Sonde der Vorprüfung (`messungen/klicksonde-offscreen.txt`). Der Sprung ist
kein Sonderfall einzelner Stellen, sondern tritt an jeder Stelle auf, an der
eine Zeile angeschnitten steht.

## Befund 2 — die falsche Markierung: reproduziert

**13 von 14 angeschnittenen Fällen markieren nicht die geklickte Zeile.** Der
Detailbereich hängt an `currentIndex` und zeigt die richtige Notiz; die
Markierung sitzt woanders — Auswahl und Anzeige gehen auseinander, wie im
Issue beschrieben.

Es sind zwei Fehlerbilder, und das Issue nennt nur eines:

| Fehlerbild | Fälle | Rollwerte |
|---|---|---|
| **Nachbarzeile markiert** (aktuell *n*, markiert *n+1*) | 8 | 2, 3, 6, 7, 8, 10, 11, 12 |
| **gar nichts markiert** — unter dem Zeiger liegt nach dem Rücken ein Gruppenkopf oder der Leerraum | 5 | 0, 1, 4, 13, 14 |

Das bestätigt F2 der Vorprüfung von Bearbeiter A unabhängig und im anderen
Aufbau.

### Die eine Ausnahme — und was sie für das AK bedeutet

**Rollwert 9 kommt richtig heraus** (aktuell 17, markiert 17). Nicht weil der
Fehler dort fehlt: Das Bild rückt auch hier, nur um **35 px** statt um 72. Die
Zeile ist 72 px hoch und der Klick saß 5 px unter ihrer Oberkante — nach einem
Rücken um 35 px liegt der Punkt immer noch in derselben Zeile. Der Fehler
bleibt unter der Schwelle, ab der er sichtbar wird.

Daraus folgt eine Falle für die Umsetzung: **Ein Test, der seinen Rollwert
festverdrahtet, kann ausgerechnet diesen Wert erwischen und wäre dann grün,
ohne etwas zu prüfen.** Der Aufbau sollte die angeschnittene Zeile suchen und
über die Rollwerte streuen, wie hier gemessen — nicht eine Stelle festlegen.

Die Vorprüfung nennt in F1 „11 von 11" und damit *jeden* angeschnittenen
Rollwert. Für die dortige Bestückung stimmt das; für diese, mit einem Rollwert
mehr am Ende, gilt es nicht ausnahmslos. Am Urteil ändert das nichts, an der
Bauart des Tests schon.

---

## Der geprüfte Verdacht: `QTest::mouseClick` braucht kein Move

Der Verdacht des Auftrags war, `QTest::mouseClick` sende Druck und Loslassen
ohne dazwischenliegendes Move-Ereignis, weshalb `QListView` das Rücken der
Liste nicht bemerke und die Auswahl richtig bliebe. **Gemessen trifft das
nicht zu** — die Auswahl ist bereits nach dem Druck falsch (Versuch 2 gibt den
Zustand zwischen Druck und Loslassen einzeln aus: „nach dem Druck: aktuell 10,
markiert 11").

Der Grund liegt darin, dass der Fehler **innerhalb** der Behandlung des
Druckereignisses entsteht, nicht zwischen Druck und Loslassen:
`QAbstractItemView::mousePressEvent` setzt erst die aktuelle Zeile, wodurch
synchron `currentChanged → showNote() → scrollTo(index)`
(`src/ui/librarywindow.cpp:792`) läuft und die Liste rückt; **danach** wählt
dieselbe Funktion über das beim Druck gemerkte Rechteck aus — das nun auf die
nachgerückte Zeile zeigt. Ein Move-Ereignis ist für den Fehler weder nötig noch
hilfreich.

## Versuch 2 — Druck, Move, Loslassen einzeln: der schlechtere Prüfweg

Er wurde trotzdem gefahren, weil der Auftrag ihn vorsah. Ergebnis: **Das Move
bei gedrückter Taste ist eine Zieh-Auswahl und verschlimmert den Fall.**

| | Versuch 1 (Klick) | Versuch 2 (Druck, Move, Loslassen) |
|---|---|---|
| Rollsprung (Rollwert 6) | −72 px | **−144 px** — die Liste rückt ein zweites Mal |
| aktuelle Zeile am Ende | 14 (geklickt: 14) | **15** — auch `currentIndex` wandert mit |
| markiert am Ende | 15 | 15 |
| „markiert == aktuell"? | AUSEINANDER | **stimmt** |
| geklickte Zeile getroffen? | VERFEHLT | **VERFEHLT** |

In 8 von 14 Fällen zieht das Move die aktuelle Zeile auf die Nachbarzeile
nach, und dann *stimmen* Auswahl und Anzeige wieder überein — auf der
**falschen** Zeile. Der Detailbereich zeigt dann ebenfalls die Nachbarnotiz;
das Auseinandergehen verschwindet, der eigentliche Schaden bleibt und wächst.

**Das ist zugleich ein Lauf, der nichts belegt hätte.** Das erste Messkriterium
dieser Reproduktion war „markiert == aktuell" — genau der Vergleich, den das
Issue nahelegt. Unter diesem Kriterium hätte Versuch 2 in 9 von 14 Fällen
„stimmt" gemeldet und wie ein sauberes Ergebnis ausgesehen. Erst die
zusätzliche Spalte „ist die **geklickte** Zeile markiert?" zeigt, dass 13 von
14 Fällen daneben liegen. Empfehlung an den PO: Dieser Fall gehört in die
Liste der Rückgabewerte und Läufe, die nichts belegen
(`.claude/agents/denkzettel-dev.md`) — ich ändere die Datei nicht, sie liegt
außerhalb dieser Messung.

**Für die Umsetzung heißt das:** Der Test von #71 nimmt den schlichten Klick
und prüft gegen die **geklickte Zeile**, nicht gegen die Übereinstimmung von
Auswahl und aktueller Zeile.

---

## Skalierung und Grenzen

**`QT_SCALE_FACTOR=1.6` (Skalierung des Kunden) ändert kein einziges Zeichen
der Ausgabe** — beide Läufe stehen in `reproduktion.txt`. Das ist kein
Widerspruch zu B21, sondern seine Kehrseite: Gemessen werden logische
Koordinaten, und `resize(900, 600)` ist eine logische Größe. Ein **Bild** wäre
skalierungsabhängig; diese Zahlen sind es nicht.

B21 trifft hier auch sonst nicht zu. Der Gegenstand von #71 sind Rollwert,
Zeilenlage und Auswahlzustand — Geometrie und Zustand, also genau das, was
offscreen belegbar ist. Hülle, Rundung, Kontur, Schatten und Dekoration
behauptet weder das Issue noch diese Messung. Die Vorprüfung von Bearbeiter A
hat den Fall zusätzlich in der angemeldeten Wayland-Sitzung gemessen; die
Ausgabe unterschied sich in einer Zeile, dem Plattformnamen.

**Was diese Messung nicht leistet:** Sie sagt nichts über den Bildbeleg, den
die Abnahme von #71 nach DoD 3 braucht — der läuft über `libraryshots` und ist
Sache der Umsetzung, nicht der Vorprüfung. Und sie prüft keine Heilung: Sie
zeigt den Fehler, nicht seine Behebung.

## Ablage und Rückstellung

Der Wegwerf-Testfall stand vorübergehend in `tests/librarytest.cpp` (zwei
Slots, ein anonymer Namensraum mit zwei Hilfsfunktionen) und ist als
`reproduktion-testfall.diff` in diesem Ordner abgelegt. Die Datei ist auf ihren
Ausgangsstand zurückgestellt:

```
$ git status --short tests/
(leer)
```

Gebaut und gelaufen ist alles in `build-repro71`; `build/` der
Repositoriumswurzel wurde nicht angefasst, nach `/usr` wurde nichts
installiert.

---

**`size:s` bestätigt** — beide Befunde reproduzieren im vorhandenen
offscreen-Testaufbau mit den Mitteln, die `tests/librarytest.cpp` schon
benutzt; es entsteht kein neuer Prüfweg.
