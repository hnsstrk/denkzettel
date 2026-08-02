# UI-Review S8 (#11) — Bearbeiten-Ansicht

**Modus:** UI-Review · **Sprint 4** · **02.08.2026** · Prüfer: `denkzettel-ux`

**Prüfgegenstand:** Zweig `story/11-bearbeiten`, Stand `646804a`, im Arbeitsbaum
`.claude/worktrees/agent-acb5c3d37bd37a7c8`.

**Maßstab:** Wireframe 2a (Zustände A/B/C) und 2b/2c in
`wireframes/Denkzettel Wireframes.dc.html` · SPEC 9 **in der Fassung des
Zweigs** · Akzeptanzkriterien aus `gh issue view 11` · KDE Human Interface
Guidelines.

## Wie die Bilder entstanden sind

Die Bilder in diesem Ordner sind **meine eigenen** (DoD 3, Beschluss B3); die
Bildstrecke des Entwicklers unter `sprint-04-s8-bearbeiten/` ist die Behauptung,
nicht der Prüfgegenstand. Gelaufen ist ein eigener Bildläufer, out-of-source
gegen `denkzettelui` gelinkt — Quelle und CMake-Datei liegen als `uxshots.cpp`
und `CMakeLists.txt` daneben, damit der Beleg reproduzierbar bleibt (B7):

```
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DWORKTREE=<Arbeitsbaum>
cmake --build build
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_FORCE_STDERR_LOGGING=1 \
  ./build/uxshots docs/scrum/reviews/sprint-04-s8-ui-review
```

Fenster 900×600 (Bild 14: 1400×900), Bezugszeit Freitag 31.07.2026 16:00 wie in
`libraryshots` und `editshots`. Kategorie und Tags sind von Hand in die
Prüfdatenbank geschrieben (Prüfmittel-Vermerk K3); die Datenbank ist ein
`QTemporaryDir`, nie der Bestand des Kunden.

Die **Dialogbilder 09 und 10 sind zusammengesetzt**: `QWidget::grab()` erwischt
immer nur ein Fenster, Bibliothek und Dialog wurden im selben Augenblick
aufgenommen und übereinandergelegt. Hinzugefügt ist nichts — insbesondere ist
das Fenster hinter dem Dialog nicht abgedunkelt, weil es das im Bau auch nicht
ist.

| Bild | Zeigt |
|---|---|
| `01-lesen.png` | Zustand A — Leseansicht mit „Bearbeiten“ und „Löschen“ |
| `02-kopfzeile-lesen.png` | Kopfzeile im Lesezustand (Ausschnitt) |
| `03-bearbeiten-frisch.png` | Zustand B unmittelbar nach F2 |
| `04-kopfzeile-bearbeiten.png` | Kopfzeile im Bearbeiten-Zustand (Ausschnitt) |
| `05-bearbeiten-geaendert.png` | Zustand B nach der Berichtigung „Fold“ → „Vault“ |
| `06-leeres-feld.png` | leeres Textfeld, „Speichern“ gesperrt |
| `07-lesen-nach-speichern.png` | Rückkehr in den Lesezustand, keine Erfolgsmeldung |
| `08-ohne-analyse.png` | Zustand B einer Notiz ohne Analyse — Kategorie/Tags „—“ |
| `09-waechter-abbrechen.png` | Wächterdialog, ausgelöst über den **Abbrechen-Knopf** |
| `10-waechter-auswahlwechsel.png` | Wächterdialog, ausgelöst über den **Auswahlwechsel** |
| `11-k2-treffer-vorher.png` | Trefferliste „Fold“ vor dem Bearbeiten |
| `12-k2-treffer-nachher.png` | dieselbe Liste nach dem Speichern (K2) |
| `13-k2-nach-suchwechsel.png` | nach der nächsten Änderung des Suchbegriffs |
| `14-bearbeiten-1400x900.png` | Raumaufteilung im größeren Fenster |
| `15-kurze-textnotiz.png` | kurze Textnotiz im Bearbeiten-Zustand (AK 10) |
| `16-sprung-bearbeiten.png` / `17-sprung-lesen.png` | Beleg zu Befund 1 |

## Gesamtverdikt

**warn — abnahmefähig mit vier Nachbesserungen, keine davon blockierend.**

Die Ansicht deckt Wireframe 2a in allen drei Zuständen, die elf
Akzeptanzkriterien sind am Bild oder am Messwert belegt, und die drei
PO-Festlegungen sind umgesetzt. Vier Befunde stehen dagegen: ein gemessener
Layoutsprung beim Zustandswechsel, eine Auswahl, die dem Wächterdialog
vorauseilt, ein deutscher Satz mit eingebautem Zeitstempel und ein
abgeschaltetes Suchfeld ohne Begründung.

## Befunde

### 1 · Der Textbereich springt beim Zustandswechsel um 19 px — **warn**

**Fundstelle:** `src/ui/librarywindow.cpp:292–305` (Kopfzeile des
Detailbereichs) · Bilder `16-sprung-bearbeiten.png` und `17-sprung-lesen.png`.

Gemessen am selben Fenster, am Textstapel selbst:

```
Textbereich oben 102 → 83 (Sprung -19 px), Höhe 486 → 436
Knopfhöhe 34, Kennzeichenhöhe 15
```

Der Notiztext rutscht beim Öffnen des Editors um 19 px nach oben und beim
Verlassen wieder nach unten. Ursache ist die Kopfzeile: „Bearbeiten“ und
„Löschen“ sind 34 px hoch, das Kennzeichen „wird bearbeitet“ 15 px — die Zeile
schrumpft mit ihnen. Der Kommentar im Quelltext sagt das Gegenteil zu („one row
for both states, so the pane keeps its height when the state changes“,
`librarywindow.cpp:292–294`), und der Test
`putsTheEditingBadgeWhereTheButtonsStand` prüft nur die Sichtbarkeit, nicht die
Höhe — deshalb ist es niemandem aufgefallen.

Nach den KDE HIG darf ein Moduswechsel den Inhalt nicht verschieben: der Blick
steht beim Drücken von F2 auf der Stelle, die berichtigt werden soll, und wandert
mit dem Text weg.

**Korrekturvorschlag:** Das Kennzeichen auf die Höhe der Knöpfe bringen, etwa
`m_editingBadge->setMinimumHeight(m_editButton->sizeHint().height())`, oder der
Kopfzeile eine feste Mindesthöhe geben. Ein Prüfsatz dazu gehört in
`keepsTheMeasuresOfTheEditState`: der Textstapel beginnt in beiden Zuständen bei
derselben y-Koordinate.

### 2 · Die Auswahl wandert vor dem Wächterdialog — **warn**

**Fundstelle:** `src/ui/librarywindow.cpp:563–595` (`showNote()`) · Bild
`10-waechter-auswahlwechsel.png`, im Vergleich mit `09-waechter-abbrechen.png`.

Beim Auswahlwechsel steht der Dialog vor einer Liste, in der **bereits die neue
Notiz hervorgehoben ist** („restic-Backup“, oben), während der Dialog nach der
alten fragt („Die Notiz von Heute 11:05“) und der Detailbereich dahinter
ebenfalls die alte zeigt. Wireframe 2a, Zustand C, zeichnet es andersherum: dort
bleibt die bearbeitete Notiz hervorgehoben (Zeile 286 der Zeichnung).

Der Zustand ist vorübergehend und wird nach „Abbrechen“ zurückgenommen (gemessen:
Auswahl zurück auf Zeile 2, Editor offen) — aber in genau dem Augenblick, in dem
der Nutzer entscheiden soll, zeigen Liste und Frage auf verschiedene Notizen. Das
Bild über den Abbrechen-Knopf (09) zeigt, wie es aussehen soll.

**Korrekturvorschlag:** Die Auswahl vor dem Öffnen des Dialogs zurücknehmen
(`m_restoringSelection` steht dafür schon bereit) und die Antwort erst danach
ausführen; oder den Wächter vor den Auswahlwechsel ziehen, statt ihn aus
`currentChanged` heraus zu führen.

### 3 · „Die Notiz von Heute 11:05 wurde geändert.“ — **warn**

**Fundstelle:** `src/ui/librarywindow.cpp:793–795` · Bilder
`09-waechter-abbrechen.png`, `10-waechter-auswahlwechsel.png`.

Der Zusatztext des Dialogs setzt den Listen-Zeitstempel in einen Satz ein. Das
ergibt „Die Notiz von **Heute 11:05** wurde geändert.“ — großes „Heute“ mitten im
Satz, kein „Uhr“. Bei älteren Notizen liest es sich als „Die Notiz von Di,
28. Juli wurde geändert.“ Der Wireframe schreibt „Die Notiz von heute, 11:05 Uhr
wurde geändert.“ (Zeile 295). Die UI-Sprache der App ist sonst durchweg sauberes
Deutsch; das hier ist der einzige Satz, in dem ein Formatwert die Grammatik
bestimmt.

**Korrekturvorschlag:** Entweder eine eigene Satzform für den Zeitstempel, oder
die Klammer: „Die bearbeitete Notiz (Heute 11:05) wurde geändert.“ Der
Zeitstempel darf auch ganz entfallen — im Bearbeiten-Zustand gibt es nur eine
Notiz, über die geredet wird.

### 4 · Das abgeschaltete Suchfeld sagt nicht, warum — **warn**

**Fundstelle:** `src/ui/librarywindow.cpp:844` · Bilder
`02-kopfzeile-lesen.png` gegen `04-kopfzeile-bearbeiten.png`.

Die PO-Festlegung selbst steht nicht zur Debatte; geprüft ist ihre **sichtbare**
Umsetzung. **Dass** das Feld abgeschaltet ist, erkennt man: der Rahmen verliert
die Akzentfarbe, der Platzhalter wird blasser. **Warum**, erfährt man nicht —
gemessen: `toolTip()` ist leer, der Platzhalter bleibt „Volltextsuche …“.

Die KDE HIG raten von Bedienelementen ab, die ohne erkennbaren Grund gesperrt
sind; das Projekt hat für denselben Fall schon einmal einen Tooltip vorgesehen
(Wireframe 2c, Festlegung „Deaktiviertes Suchfeld“, damals für S5). Der
Unterschied: damals war die Sperre dauerhaft, hier endet sie mit dem
Bearbeiten-Zustand — die Kurzhilfe kostet trotzdem nichts.

**Korrekturvorschlag:** Kurzhilfe am Feld, solange bearbeitet wird: „Während des
Bearbeitens ist die Suche abgeschaltet.“ Beim Verlassen des Zustands wieder
entfernen, wie S6 es mit der alten Umhüllung gehalten hat.

### 5 · Die Knöpfe des Wächterdialogs tragen keine Symbole — **warn (gering)**

**Fundstelle:** `src/ui/librarywindow.cpp:800–802` · Bild
`09-waechter-abbrechen.png`.

Die drei Knöpfe sind mit `addButton(QString, Rolle)` gebaut und bleiben deshalb
ohne Symbol. In KDE-Anwendungen tragen genau diese drei Standardhandlungen
üblicherweise `document-save`, `dialog-cancel` und ein Verwerfen-Symbol; das
Symbol ist bei einem Dialog, der drei ähnlich lange deutsche Wörter
nebeneinanderstellt, die schnellste Unterscheidung. Der destruktive Knopf
(„Verwerfen“) ist als solcher nicht gekennzeichnet.

**Korrekturvorschlag:** `QMessageBox::Save | QMessageBox::Discard |
QMessageBox::Cancel` verwenden oder `KStandardGuiItem` auf die drei Knöpfe legen.
Die Rollen und damit die Reihenfolge bleiben dabei unverändert.

### 6 · Im Lesezustand fehlen die Tag-Chips der Zeichnung — **warn (nicht S8)**

**Fundstelle:** Wireframe 2a Zustand A (Zeile 252) und 2b (Zeile 370) · Bild
`01-lesen.png`.

Beide Zeichnungen setzen unter den Notiztext eine Reihe Tag-Chips. Gebaut ist sie
nicht. Das stammt aus S5 und ist **kein Befund gegen diese Story** — mit S8
entsteht daraus aber eine Schieflage, die vorher nicht da war: Kategorie und Tags
sieht man **nur im Bearbeiten-Zustand** (Bild 03), also gerade dort, wo man sie
nicht ändern darf. Wer sie lesen will, muss den Editor öffnen.

**Melden, nicht heilen:** Das geht an den PO. Entweder die Merkmalszeile auch im
Lesezustand zeigen (dann deckt sich der Bau wieder mit 2a/2b), oder die Chips in
den Zeichnungen streichen. Solange in M2 niemand Kategorien und Tags füllt, ist
beides folgenlos — mit M3 wird es sichtbar.

## Deckung mit dem Wireframe, Fläche für Fläche

Jede gezeichnete Fläche erzeugt genau eine Prüffrage (Beschluss B2).

### Wireframe 2a, Zustand A — Leseansicht · Bild `01-lesen.png`

| Fläche der Zeichnung | Befund | Verdikt |
|---|---|---|
| Kopfzeile: Zeitstempel links | „Heute 11:05“, gedämpft, links | **ok** |
| Kopfzeile: „Bearbeiten“ (betont) und „Löschen“ rechts | beide da, Reihenfolge wie gezeichnet; „Bearbeiten“ ist nicht optisch betont, was ohne Vorgabeknopf-Rolle in einem Fenster auch nicht üblich ist | **ok** |
| Player-Zeile über dem Transkript | in M2 nicht gebaut — mit K1 aus S8 herausgelöst, kommt mit S16 (#26) | **ok** |
| Notiztext | volle Breite, rahmenlos, füllt die Resthöhe | **ok** |
| Tag-Chips unter dem Text | nicht gebaut | **warn** (Befund 6) |
| F2 als Beschleuniger zur sichtbaren Schaltfläche | gemessen: F2 aus der Liste heraus öffnet den Editor | **ok** |
| Doppelklick öffnet **nicht** | kein Doppelklick-Weg im Code; Wortauswahl im `QTextBrowser` bleibt | **ok** |

### Wireframe 2a, Zustand B — Bearbeiten · Bilder `03`, `05`, `08`, `15`

| Fläche der Zeichnung | Befund | Verdikt |
|---|---|---|
| Kennzeichen „wird bearbeitet“ rechts im Kopf, dort wo die Knöpfe standen | vorhanden, in Akzentfarbe (`QPalette::Link`, folgt einem Farbschemawechsel); die Zeichnung zeichnet eine Pille mit Umriss, gebaut ist reiner Text — die Kennzeichnung trägt trotzdem | **ok** |
| … und die Zeile behält dabei ihre Höhe | **nein**, 19 px Sprung | **warn** (Befund 1) |
| gedämpfte Player-Zeile | entfällt in M2 (K1) | **ok** |
| Textfeld mit Rahmen, editierbar | `QPlainTextEdit` mit Rahmen — der Unterschied zum rahmenlosen Leser ist die deutlichste Anzeige des Zustands | **ok** |
| Cursor am Textende, nichts markiert | gemessen: Cursor bei 184 von 184, Auswahl 0, Fokus im Editor | **ok** |
| Merkmalszeile „Kategorie … | Tags …“ als reine Anzeige | vorhanden, Marken gedämpft, Werte in Textfarbe, keine Eingabefelder | **ok** |
| Fußzeile: Kürzel-Hinweis links | „Esc bricht ab · Strg+Enter speichert“ — dieselbe Machart wie im Erfassungsfenster („Esc verwirft · Strg+Enter speichert“), der Unterschied im Verb ist richtig, weil Esc hier fragt | **ok** |
| Fußzeile: [Abbrechen] [Speichern] rechts | gebaut ist [Speichern] [Abbrechen] — die `QDialogButtonBox` erbt die KDE-Reihenfolge (Accept zuerst). Das ist dieselbe Begründung, die die Zeichnung für Zustand C ausdrücklich gibt; **die Zeichnung von Zustand B ist an dieser Stelle nachzuziehen** | **ok** |
| gestrichelte Trennlinie über der Fußzeile | nicht gebaut; der Abstand trennt ausreichend, und eine zweite Linie im Detailbereich hätte keine Entsprechung | **ok** |
| kurze Textnotiz verhält sich gleich, nur ohne Player | Bild 15: gleiche Aufteilung, gleicher Editor | **ok** |

### Wireframe 2a, Zustand C — Wächterdialog · Bilder `09`, `10`

| Fläche der Zeichnung | Befund | Verdikt |
|---|---|---|
| Dialog mit Titelzeile „Änderungen speichern?“ | Fenstertitel „Ungespeicherte Änderungen“, Haupttext „Änderungen speichern?“ | **ok** |
| Erläuterungssatz darunter | vorhanden, Wortlaut mit Grammatikfehler | **warn** (Befund 3) |
| drei Knöpfe mit den Rollen Accept/Destructive/Reject | gemessen: Speichern (Accept), Verwerfen (Destructive), Abbrechen (Reject); Reihenfolge im Fenster Speichern · Verwerfen · Abbrechen = KDE-Plattformreihenfolge, wie die Zeichnung sie zusichert | **ok** |
| … ohne Symbole | üblich wären welche | **warn** (Befund 5) |
| Fokus/Vorgabe auf der harmlosen Handlung | gemessen: der Tastaturfokus steht auf „Speichern“, Enter speichert also. (`defaultButton()` und `escapeButton()` meldeten zur Laufzeit trotz der Aufrufe in Zeile 804/806 keinen Knopf — eine Qt-Eigenheit ohne Wirkung: Esc über dem Dialog ergibt gemessen die harmlose Antwort — Fenster bleibt offen, Editor bleibt offen, Änderung erhalten, Speicher unverändert) | **ok** |
| derselbe Dialog bei Auswahlwechsel, Fensterschließen und Esc | alle drei geführt (Bilder 09/10, Messung zum Fensterschließen) | **ok** |
| …und zusätzlich beim **Abbrechen-Knopf** (PO-Festlegung 1) | Bild 09 — derselbe Dialog | **ok** |
| Liste hinter dem Dialog: bearbeitete Notiz bleibt hervorgehoben | Auswahl ist schon gewandert | **warn** (Befund 2) |
| abgedunkeltes Fenster hinter dem Dialog | zeichnerisches Mittel; Qt dunkelt nicht ab, und keine KDE-Anwendung tut es. **Die Zeichnung ist hier nicht bindend** und sollte den Vermerk tragen | **ok** |

### Raumaufteilung · Bilder `01`, `03`, `14`

Gemessen bei 900×600 und 1400×900, im Lese- wie im Bearbeiten-Zustand:

```
Lesen 900x600      Kopfzeile h=48 · Liste b=300 h=552 · Leser y=102 h=486
Bearbeiten 900x600 Kopfzeile h=48 · Liste b=300 h=552 · Editor y=83 h=436
Lesen 1400x900     Kopfzeile h=48 · Liste b=300 h=852 · Leser y=102 h=786
Bearbeiten 1400x900 Kopfzeile h=48 · Liste b=300 h=852 · Editor y=83 h=736
```

| Prüfsatz (2c, „Raumaufteilung“) | Befund | Verdikt |
|---|---|---|
| Kopfzeile wächst nicht mit dem Fenster (rund 48 px) | 48 px bei beiden Größen und in beiden Zuständen | **ok** |
| Liste behält ihre Breite, der Detailbereich wächst | Liste 300 px bei 900 wie bei 1400 px Fensterbreite | **ok** |
| Liste und Detail füllen die volle Resthöhe | 552 bzw. 852 px, kein freier Streifen | **ok** |
| Merkmalszeile und Fußzeile bleiben einzeilig, der Text bekommt den Überschuss | Editor 436 von 486 px bzw. 736 von 786 px | **ok** |
| Der Zustandswechsel verschiebt den Text nicht | 19 px Sprung | **warn** (Befund 1) |

## Die drei PO-Festlegungen

| Festlegung | Sichtbare Umsetzung | Verdikt |
|---|---|---|
| Der **Abbrechen-Knopf** führt denselben Wächterdialog wie Esc | Bild 09: Klick auf „Abbrechen“ mit offener Änderung öffnet den Dialog; „Abbrechen“ im Dialog kehrt in den Editor zurück (gemessen: Editor offen, Änderung erhalten). Der Fußzeilen-Hinweis „Esc bricht ab“ deckt beide Wege, weil der Knopf der Beschleuniger desselben ist | **ok** |
| Das **Suchfeld ist im Bearbeiten-Zustand abgeschaltet** | Dass — ja (Bilder 02/04). Warum — nein | **warn** (Befund 4) |
| **Kategorie und Tags zeigen „—“**, solange keine Analyse gelaufen ist | Bild 08: „Kategorie —  Tags —“, in derselben Zeile und Machart wie die gefüllte Fassung; die Zeile bleibt gleich hoch, das Layout wackelt nicht | **ok** |

## KDE HIG

| Prüfpunkt | Befund | Verdikt |
|---|---|---|
| Dialogrollen und Knopfreihenfolge | Rollen gesetzt, Reihenfolge von der Plattform geerbt, nicht selbst gestellt | **ok** |
| Symbole an den Dialogknöpfen | fehlen | **warn** (Befund 5) |
| Kürzel ergänzen sichtbare Bedienelemente, ersetzen sie nicht | F2/Strg+Enter/Esc haben je eine sichtbare Entsprechung; der Hinweis in der Fußzeile nennt die beiden, die man nicht sieht | **ok** |
| Klarheit des Zustandswechsels | vier gleichzeitige Anzeichen: Kennzeichen, Rahmen um den Text, Merkmalszeile, Fußzeile. Reichlich, aber nicht zu viel | **ok** |
| Rückmeldung nach dem Speichern | nur der Rückwechsel in die Leseansicht, keine Erfolgsmeldung (Bild 07) — genau wie die Zeichnung es festlegt | **ok** |
| Gesperrte Bedienelemente mit erkennbarem Grund | „Speichern“ bei leerem Feld: der Grund steht daneben, das Feld ist leer (Bild 06). Suchfeld: siehe Befund 4 | **ok** / **warn** |
| Kein Datenverlust ohne Nachfrage | vier Wege geprüft (Abbrechen, Esc, Auswahlwechsel, Fensterschließen), keiner schreibt oder verwirft still | **ok** |
| Deutsche UI-Sprache, Wortfamilie der App | durchgehend deutsch mit korrekten Umlauten; Wortwahl deckt sich mit dem Erfassungsfenster | **ok** außer Befund 3 |

## K2 — sichtbar geprüft

**Frage:** Fällt die gespeicherte Notiz aus der laufenden Trefferliste, bleibt sie
sichtbar und ausgewählt, ohne dass die Liste springt?

**Aufbau:** Suche „Fold“ (Bild 11) → Notiz bearbeiten, „Fold“ durch „Vault“
ersetzen → speichern (Bild 12) → Suchbegriff auf „Fol“ ändern (Bild 13).

**Messung:**

```
Suche „Fold“: 2 Zeile(n)
nach dem Speichern: 2 Zeile(n), Auswahl auf Zeile 1, Rollstand 0 → 0,
                    Auswahlzeile y 27 → 27, Suchfeld „Fold“, Suchfeld frei 1
im Speicher: „Fold“ 0 Treffer, „Vault“ 1 Treffer
nach Änderung des Suchbegriffs auf „Fol“: 0 Zeile(n)
```

**Befund:** Die Bilder 11 und 12 sind bis auf den Text im Detailbereich
deckungsgleich — gleicher Gruppenkopf, gleiche Zeile, gleiche Auswahlmarkierung,
gleiche Höhe. Kein Sprung, kein Aufblitzen einer leeren Liste, kein Wechsel auf
„Keine Treffer“. Die nächste Änderung des Suchbegriffs räumt auf (Bild 13:
„Keine Treffer“, Detailbereich leer), wie SPEC 9 es in der Fassung des Zweigs
festlegt. **Verdikt: ok.**

Eine Beobachtung ohne Verdikt: Im Bild 12 steht „Fold“ im Suchfeld, während die
angezeigte Notiz das Wort nicht mehr enthält. Nichts weist darauf hin. Das ist
die gewollte Folge der Festlegung und hier harmlos, weil der Nutzer die Änderung
gerade selbst vorgenommen hat — es sei nur benannt, damit es später nicht als
Fehler gemeldet wird.

## SPEC-Änderung des Zweigs (DoD 4)

`SPEC.md` 9 ist um drei Bedingungen ergänzt: den Wächterdialog über alle vier
Auswege, das Stehenbleiben der Notiz in der Trefferliste und das abgeschaltete
Suchfeld. Alle drei sind am Bau belegt (Bilder 09/10, 11/12, 04). Die letzten
beiden sind als bei der Umsetzung entdeckt gekennzeichnet — das ist die Fassung
nach B9, und die Begründung („dann hat der Dialog keine Zeile mehr, auf die er
die Auswahl zurücknehmen könnte“) ist die tatsächliche. **Verdikt: ok.**

## Offene Punkte

1. **Zwei Nachbesserungen für diesen Sprint** (Befunde 1 und 2): der Layoutsprung
   und die vorauseilende Auswahl. Beide sind kleine, klar umrissene Eingriffe in
   `librarywindow.cpp`.
2. **Zwei für den PO zu entscheiden** (Befunde 4 und 5): Kurzhilfe am
   abgeschalteten Suchfeld, Symbole an den Dialogknöpfen.
3. **Befund 3** (Grammatik des Dialogsatzes) ist eine Textkorrektur — sie kostet
   eine Zeile und sollte nicht in den Backlog wandern.
4. **Befund 6** geht an den PO: Merkmalszeile auch im Lesezustand, oder Tag-Chips
   aus den Zeichnungen 2a/2b streichen.
5. **Zwei Nachträge an den Zeichnungen**, die ich als Gestalter selbst führe,
   sobald der PO sie beauftragt: die Knopfreihenfolge in Zustand B (Plattform
   statt Zeichnung) und ein Vermerk an Zustand C, dass die Abdunklung ein
   zeichnerisches Mittel ist und nicht gebaut wird.
6. **Nicht geprüft, weil außerhalb der Story:** der Zweig steht auf dem Stand vor
   den Tray-Menü-Commits von `main`; die Zeichnung 5a unterscheidet sich deshalb
   im Arbeitsbaum von der in `main`. Das ist keine Änderung dieser Story — beim
   Zusammenführen ist der Stand von `main` der richtige.

## Testlauf

`ctest --test-dir build`: 7 von 7 bestanden (4,78 s). Die Bildprüfung ersetzt die
Tests nicht und die Tests ersetzen sie nicht — Befund 1 ist genau der Fall, den
kein Test bemerkt hat.
