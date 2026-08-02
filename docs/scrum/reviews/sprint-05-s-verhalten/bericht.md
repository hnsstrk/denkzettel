# Strang B, Sprint 5 — Übergabebericht zu #57 (Klick-Sprung) und #58 (Palettenrolle)

**Rolle:** Entwickler (`denkzettel-dev`) · **Datum:** 02.08.2026 ·
**Zweig:** `story/57-58-verhalten` · **Ausgangsstand:** `a322b86`

Belege in diesem Ordner: `testlauf-1-rot.txt`, `testlauf-2-gegenprobe.txt`,
`testlauf-3-gruen.txt`, `messung-szenen-vorher.txt`,
`messung-szenen-nachher.txt`, `messung-szenen-unterschied.txt`,
`messung-farben.txt`, `selbststart.txt`, `bilder/`.

## 1. Was gebaut wurde

**#57 — die Regel lautet: Zeigen ist nicht Tippen.** Ein neuer `eventFilter`
auf `m_list->viewport()` setzt vor jedem Mausdruck einen Merker; `showNote()`
liest ihn als vierte Bedingung des Vorscrollens. Beim Tastendruck bleibt alles
wie in Sprint 3 geheilt, beim Klick steht das Bild still.

Drei Fallen, alle im Code benannt:

1. `QListView::pressed` taugt nicht — es kommt **nach** `currentChanged`, also
   nachdem die Liste sich schon bewegt hat. Deshalb der Ereignisfilter.
2. Der Merker wird von genau dem `showNote()`-Aufruf verbraucht, zu dem er
   gehört — über einen `QScopeGuard`, der an **jedem** der vier Ausgänge
   greift. Ein per `singleShot` gepostetes Zurücksetzen wäre im verschachtelten
   Ereignisumlauf des Wächterdialogs gelaufen und der Merker vor der Antwort
   des Nutzers verschwunden.
3. Ein Druck, der **nichts** auswählt (Gruppenkopf, Leerraum), ließe den Merker
   liegen und würde den nächsten Tastendruck einfärben. Der nächste Tastendruck
   löscht ihn deshalb. Dass diese Zeile trägt, ist gemessen und nicht behauptet
   — siehe `testlauf-2-gegenprobe.txt`.

**#58 — die eine Zeile.** `subtleLabel()` in `librarywindow.cpp` setzt statt
einer eingefrorenen Farbe die Rolle: `setForegroundRole(QPalette::PlaceholderText)`,
zeichengleich zum Vorbild `capturewindow.cpp:36`.

## 2. Rot zuerst

`testlauf-1-rot.txt` ist gegen den **unveränderten** Produktivcode gefahren
(`git checkout -- src/ui/librarywindow.{h,cpp}`, Tests unverändert):

```
FAIL!  : leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked()
         Actual   (list->verticalScrollBar()->value()): 9
         Expected (rolledTo)                          : 10
FAIL!  : textsFollowAColourSchemeChange()
         („Zum Lesen links eine Notiz auswählen.“ malt in #000000, das Schema sagt #232629)
```

Der dritte neue Test (`keepsTheHeadFetchAfterAClickThatSelectedNothing`) ist
**nicht** rot gewesen und wird auch nicht als Rot-Nachweis ausgegeben: Er
sichert eine Eigenschaft des neuen Mechanismus, die es vorher nicht gab. Damit
er kein Test ist, in dem der Fehler gar nicht auftreten *kann*, ist er gegen
den geheilten Stand **ohne** die Tastendruck-Rücksetzung gefahren worden:

```
FAIL!  : keepsTheHeadFetchAfterAClickThatSelectedNothing()
         'list->viewport()->rect().contains(list->visualRect(head))' returned FALSE. (Kopf bei y=-39)
```

## 3. Die Umkehrung des bestehenden Tests (K5)

`bringsTheHeadAlongWhenAVisibleNoteOfAnotherGroupIsClicked` (`:1398`) sicherte
das Verhalten zu, das diese Story beseitigt. Er ist **umgekehrt**, nicht
ergänzt und nicht gelöscht: neuer Name
`leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked`, und sein
Kommentar trägt die überholte Fassung samt ihres eigenen Schlusssatzes
weiter — *„Should it grate in daily use, this is the test that says where the
decision was made."* Es hat gestört; die Spur, wo entschieden wurde, bleibt
lesbar.

## 4. Rollwert-Messung — der Weg, nicht das Ziel (AK 3, K4)

Gefahren wurde **das Szenenprogramm des UI-Reviewers selbst**
(`docs/scrum/reviews/sprint-03-s5a-ak7-nachpruefung/ux-nachpruefung-ak7.cpp`,
nur gelesen und gegen die gebaute `libdenkzettelui.a` gebunden), einmal vor
und einmal nach der Heilung, `QT_QPA_PLATFORM=offscreen`,
`QT_QPA_PLATFORMTHEME=kde`, `-style breeze`, Bezugszeitpunkt 31.07.2026 16:00.

Der Vorher-Lauf reproduziert die Werte des Reviewers aus
`nach-der-warn-heilung/messung.txt` **zeilengleich**. Von zwölf Szenen ändert
sich danach **genau eine Zeile**:

```
71c71
<   Nach dem Klick: Auswahl Zeile 6, Rollwert 6 → 0, Kopf y=0
---
>   Nach dem Klick: Auswahl Zeile 6, Rollwert 6 → 6, Kopf y=-387
```

| Szene | vorher | nachher |
|---|---|---|
| **N11** Klick auf sichtbare Zeile anderer Gruppe | Rollwert 6 → **0**, das Bild springt 387 px | Rollwert 6 → **6**, das Bild steht |
| N1, N7 abwärts über die Grenze | +2 / +1 | **unverändert** |
| N2 aufwärts auf die erste Notiz der Gruppe | 11 → 10, Kopf y=−35 draußen | **unverändert** |
| N3 aufwärts über die Grenze | 10 → 8 | **unverändert** |
| N4, N5 kleine Gruppe | 8 → 7 | **unverändert** |
| N6, N8 Rad, dann Pfeiltaste | +0 | **unverändert** |
| N10 Grenzübertritt auf sichtbare Zeile | 8 → 7, Kopf y=0 im Bild | **unverändert** |
| N12 Löschen und Undo | Rollwert 1 | **unverändert** |

Das ist zugleich die Antwort auf Risiko 3 des Plannings („an beiden Fällen
zugleich messen"): Der zweite Datenpunkt des Issues — **N2**, die 35 px, die zu
Unrecht unterbleiben — ist gemessen und **unverändert**. Die Regel verschiebt
den Fehler also nicht, sie nimmt ihn an einer Stelle weg und lässt die andere,
wie sie war. **N2 bleibt offen** (siehe 8).

## 5. Bilder zu #58 (AK 2, K3)

`tests/libraryshots.cpp` ist um Strecke 10 erweitert — **ein Lauf, ein Fenster,
kein Neuaufbau**, vier Bilder, beide Richtungen:

| Bild | Zustand |
|---|---|
| `bilder/10a-schema-hell-lesen.png` | helles Schema, Lesezustand, „Gestern 21:48" |
| `bilder/10b-schema-dunkel-lesen.png` | nach dem Wechsel auf dunkel, dasselbe Fenster |
| `bilder/10c-schema-dunkel-bearbeiten.png` | Bearbeiten-Zustand: „Kategorie", „Tags", „Esc bricht ab · Strg+Enter speichert" |
| `bilder/10d-schema-hell-bearbeiten.png` | zurück auf hell, dasselbe Fenster |

Der Bearbeiten-Zustand ist mit im Bild, weil dort **vier** `subtleLabel`-Stellen
zugleich sichtbar sind; die Zeitstempel **in der Liste** malt der Delegate und
folgen schon immer — die Story sitzt an den QLabel-Stellen, und die zeigt das
Bild (UX-Ergänzung im Issue).

Dazu zwei Bilder des **ungeheilten** Stands (`10b-ungeheilt-…`,
`10c-ungeheilt-…`) und die Farbablesung in `messung-farben.txt`:

| Lauf | Hinweistext nach dem Wechsel auf dunkel | Kontrast |
|---|---|---|
| ungeheilt | `#707d8a` — die Farbe des **hellen** Schemas, eingefroren | 3,75:1 |
| geheilt | `#a1a9b1` — die des dunklen | 6,64:1 |

**Warum die Farbablesung dazugehört:** In der Gegenrichtung (zurück auf hell)
sind beide Stände gleich — dort ist der Fehler unsichtbar, weil die
eingefrorene Farbe zufällig die richtige ist. Ein Bildpaar in dieser Richtung
ließe einen ungeheilten Stand geheilt aussehen.

**Kein weiteres Vorkommen der Bauart** (AK 4): `grep -rn "setPalette" src/`
liefert nach der Heilung keinen Treffer mehr; die beiden verbleibenden Stellen
im Verzeichnis sind `setForegroundRole`-Aufrufe (`capturewindow.cpp:36`,
`librarywindow.cpp:299`).

## 6. Testbilanz (Prüfmittel nach Planning 5.3.2)

| Lauf | vorher | nachher |
|---|---|---|
| `QT_QPA_PLATFORM=offscreen` | 102 / 0 | **104 / 0** |
| `+ QT_QPA_PLATFORMTHEME=kde` | 101 / 1 | **103 / 1** |
| `ctest --test-dir build` | 7 / 7 | **7 / 7** |

Der eine rote Test unter dem Plattformthema ist
`namesTheThreeAnswersOfTheGuardDialog` — er war es am Ausgangsstand ebenso
(Planning 3.3), gehört zu #66 und damit **Strang A**. Nicht angefasst
(melden, nicht heilen). Die beiden neuen Zusicherungen von Strang B sind in
**beiden** Umgebungen grün; sie messen relativ (Rollwert vorher/nachher,
Zeilenlage vorher/nachher) und hängen an keinem absoluten Pixelwert.

## 7. Was #59 angeht (K6)

Nichts verbaut. Die Heilung von #59 sitzt in `regroupList()` bzw. in der
Bedingung des `changeEvent` — mein Merker wird ausschließlich von einem
Mausdruck **in der Liste** gesetzt und ist auf dem Aktivierungsweg immer
`false`. Belegt: `regroupsWhenTheWindowIsActivated` ist unverändert grün, und
Szene N12 des Reviewers (Löschen/Undo, also der Weg über `setCurrentIndex` ohne
Maus) ist zeilengleich. Wird #59 später geheilt, entfällt auf diesem Weg der
`showNote()`-Aufruf ganz — der Merker steht dem nicht im Weg.

## 8. Offen — ausdrücklich nicht geheilt

**Der zweite Datenpunkt aus #57 (Szene N2) bleibt bestehen:** Wandert die
Auswahl per Pfeiltaste **innerhalb** einer Gruppe aufwärts bis zu deren erster
Notiz, steht der Kopf danach bei y = −35, also um seine eigene Höhe
abgeschnitten. Das ist regelkonform (es findet kein Grenzübertritt statt) und
durch diese Story unverändert — gemessen, nicht vermutet.

Er ist hier **nicht** mitgeheilt, weil der Auftrag die Tastatur-Seite
ausdrücklich unangetastet lässt und jede Regel dafür entweder einen Schwellwert
bräuchte (im Issue zu Recht abgelehnt) oder eine neue Regel wäre („die erste
Notiz einer Gruppe holt ihren Kopf"). Das ist eine Produktentscheidung, keine
Entwicklerentscheidung. **Vorschlag an den PO:** als eigenes Issue führen, zu
#59 benachbart.

## 9. Berührte Dateien

| Datei | Was |
|---|---|
| `src/ui/librarywindow.h` | `eventFilter`-Deklaration, Feld `m_selectionFollowsAPress` |
| `src/ui/librarywindow.cpp` | `subtleLabel()` (#58); Filter installiert, `eventFilter()`, Merker-Verbrauch und vierte Bedingung in `showNote()` (#57) |
| `tests/librarytest.cpp` | Umkehrung von `:1398`, zwei neue Zusicherungen, Deklarationen im Block bei `:140` und `:163` |
| `tests/libraryshots.cpp` | Strecke 10 (Schemawechsel), Helfer `buttonNamed`, `breezePalette`, `applyScheme` |
| `SPEC.md` | 9, Listenteil: die entdeckte Bedingung (DoD 4/B9) — Kopf beim Grenzübertritt **per Taste**, nicht beim Klick |
| `docs/scrum/reviews/sprint-05-s-verhalten/` | dieser Bericht und die Belege |

`tests/CMakeLists.txt` ist **nicht** angefasst (gehört Strang A), `editshots.cpp`
und der Wächterdialog ebenso wenig. Nach `/usr` wurde nichts installiert.

**Abweichung von der Dateimengen-Tabelle des Plannings (5.2):** Der Belegordner
heißt `sprint-05-s-verhalten/` statt `sprint-05-s57-s58/` — so lautete der
Spawn-Auftrag des PO. Sonst keine.

---
**Nachtrag (UI-Review B1, PO, 02.08.2026):** Die Bilder 10a–10d zeigen den
Zweigstand — ohne die #67-Symbole, die erst der Merge brachte. Für den
#58-Nachweis (Farbfolge) tragen sie; als Bild des Auslieferungsstands nicht.
Das Bild des gemergten Standes liegt unter `../sprint-05-ui-review/`.
