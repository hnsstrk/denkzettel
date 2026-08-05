# Strang B, Sprint 7 — #71, #70, #72

**Zweig:** `story/71-ruhige-liste` · **Arbeitsbaum:** `denkzettel-71` ·
**Ausgangsstand:** `sprint-07-basis` (`7afe022`) · **Datum:** 05.08.2026

**Vier Commits statt drei.** #71, #70, #72 — dazu ein Nachtrag zu #71, der bei
der Sichtprüfung entstanden ist und einen eigenen Commit verdient, weil er eine
Aussage des ersten zurücknimmt (Abschnitt „Was bei der Sichtprüfung herauskam").

**Volle Testauflage: 113 von 113 grün**, `ctest` 7 von 7, Bau warnungsfrei.
Nach `/usr` wurde nichts installiert; der laufende `denkzetteld` (PID 4029,
`/usr/bin/denkzetteld`, kein `(deleted)`) ist der Stand des PO und wurde nicht
angefasst.

Alles hier ist mit `pruefen.sh` in diesem Ordner wiederholbar.

---

## 1. Was gebaut wurde

| Issue | Codeänderung | Größe |
|---|---|---|
| **#71** | `showNote()`: das `scrollTo(index)` steht unter `!m_selectionFollowsAPress`. Dazu im `eventFilter`: der Merker endet mit dem Loslassen | 4 + 3 Zeilen |
| **#70** | `showNote()`: ein Oder-Zweig `isFirstOfItsGroup` im Bedingungskopf des Kopfholens | 2 Zeilen |
| **#72** | drei `setToolTip()`, eine Hilfsfunktion `tooltipNaming()` | 3 + 4 Zeilen |

Berührte Dateien: `src/ui/librarywindow.cpp`, `tests/librarytest.cpp`,
`tests/libraryshots.cpp`, `SPEC.md` (Abschnitt 9, ein Absatz, einmal
geschrieben). `librarywindow.h` blieb unberührt, wie die Vorprüfung von #70
vorgesehen hatte.

---

## 2. #71 — Kriterium für Kriterium

| AK | Prüfmittel | Ergebnis | Mutationsprobe |
|---|---|---|---|
| **1** Klick wählt die geklickte Zeile, genau eine markiert, Detailbereich passt | `selectsTheClippedRowThatWasClickedAndLeavesThePictureWhereItIs`, gestreut über **alle 16 Rollwerte**, geprüft in den 12 mit angeschnittener Zeile | grün | **M1** — Heilung entfernt: rot, `selectedRows` = „keine" statt „8" |
| **2** Zeile steht nach dem Klick, wo sie stand (Rollwert und `visualRect().y()`) | derselbe Prüfsatz, gemessen vor und nach dem Klick | grün, Versatz 0 px | **M3** — Heilung entfernt, Auswahlzusicherungen ausgesetzt: rot, Rollwert 2 statt 0. **M6** — zusätzlich Rollwert ausgesetzt: rot, `y` 444 statt 549 |
| **3** Aufbau sichert seine Voraussetzungen, **sucht** die Zeile, streut über Rollwerte | `bottomClippedRow()` sucht; `QVERIFY2` auf Vorauswahl ≠ Ziel, sichtbarer Streifen ≥ 2 px; Wächter `checked >= 10` | grün, 12 Fälle geprüft | **M4** — Suche liefert nichts: rot, „Nur 0 angeschnittene Rollwerte geprüft" |
| **4** Mutationsprobe im Bericht belegt | dieser Abschnitt | erfüllt | — |
| **5** Tastaturpfad unverändert, die zwei genannten Prüfsätze bleiben grün | volle Auflage | grün | **M8** — Grenzübertritt-Zweig entfernt: **4** Prüfsätze rot, darunter beide genannten |
| **6** Merker klebt nicht | `dropsTheMarkOfAPressThatSelectedNothingWhenItEnds` | grün | **M2** — Loslass-Zweig entfernt: rot, „Kopf bei y=-39, Bild 559 hoch" |
| **7** SPEC 9 nachgezogen | `SPEC.md:513–531` | erfüllt | — |
| **8** Bildbeleg | `libraryshots`, Szene 11a/11b, `QT_QPA_PLATFORMTHEME=kde`, `QT_SCALE_FACTOR=1.6`, Läufer vorher gebaut | `bilder-1.6/11a…`, `11b…` | — |

**Zwei Zusicherungen tragen den Beweis nicht, und das ist gemessen, nicht
vermutet:** `currentIndex() == target` und der Text des Detailbereichs bleiben
in **M5** grün, wenn nur die Auswahlzusicherung ausgesetzt wird. Sie sind
Regressionsschutz, keine Fehlerfänger — im Fehlerbild von #71 sind beide
richtig, das war ja der Befund („Auswahl und Anzeige gehen auseinander").

**Zu AK 6 ein Befund, der die Begründung des Kriteriums berichtigt.** Das
Kriterium erwartete, dass ein klebender Merker das **Nachrücken zur Auswahl**
unterdrückt. Gemessen tut er das nicht: `QAbstractItemView::setCurrentIndex`
rollt von sich aus zu dem, was es auswählt, und deckt den Fall doppelt. Der
Merker klebt aber wirklich — er stand nach dem Klick auf einen Gruppenkopf noch,
als die Löschung zwei Aufrufe später ihre Auswahl setzte. Wirksam wird er am
**Kopf-Vorlauf**, der keine zweite Deckung hat: Ohne den Loslass-Zweig bleibt
der Gruppenkopf bei einer programmatischen Auswahländerung draußen (M2). Das ist
ein **Bestandsfehler**, unabhängig von #71 — der Kopf-Vorlauf hing schon vorher
an diesem Merker. Der Prüfsatz misst deshalb den Kopf und nicht die Auswahl; ein
Test auf die Auswahl wäre grün gewesen, ohne etwas zu prüfen.

## 3. #70 — Kriterium für Kriterium

| AK | Prüfmittel | Ergebnis | Mutationsprobe |
|---|---|---|---|
| **1** Pfeiltaste auf die erste Notiz einer Gruppe: Kopf **und** Auswahl im Bild | `bringsTheHeadAlongWhenTheSelectionReachesTheFirstNoteOfItsGroup`, `viewport()->rect().contains(...)` für beide, Rollwert vor und nach dem Tastendruck | grün | **M7** — Oder-Zweig entfernt: rot, „Kopf bei y=-39" |
| **2** Die drei genannten Bestandsprüfsätze bleiben grün | volle Auflage | grün | **M8** — Grenzübertritt-Zweig entfernt: rot, darunter `bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready` |
| **3** Klick auf die **erste** Notiz einer Gruppe lässt das Bild stehen, Kopf kommt nicht | `leavesThePictureWhereItIsWhenTheFirstNoteOfAGroupIsClicked`, gemessen vor und nach dem Klick | grün | **M9** — `!m_selectionFollowsAPress` aus dem `if` genommen: rot, zusammen mit `leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked` |
| **4** Bewegung innerhalb der Gruppe auf eine Notiz, die nicht die erste ist, rollt nicht | `staysPutWhileTheSelectionMovesWithinItsGroup` (Bestand, Ziel ist die dritte Notiz von „Gestern") | grün | in M8 mit erfasst |
| **5** Passen Kopf und Auswahl nicht zusammen ins Bild, bleibt der Kopf draußen | Code unverändert; die Bedingung steht im selben Rumpf und gilt für beide Zweige | erfüllt | **M10 trifft nicht** — siehe unten |
| **6** Bildbeleg aus `libraryshots` | Szene 7, Läufer vorher gebaut, Theme und Skalierung gesetzt | `bilder-1.6/07…` gegen `messungen/70-szene7-vor-der-heilung.png` | — |
| **7** SPEC 9 als **Ergänzung** nachgezogen | `SPEC.md:513–522` | erfüllt | — |

**AK 5 hat keine wirksame Mutationsprobe, und das ist eine Grenze, keine
Nachlässigkeit.** M10 entfernt die Passbedingung — **112 von 112 bleiben grün**.
Zwei Gründe, beide gemessen:

1. Der Bestandsprüfsatz `leavesTheHeadOutsideWhereItCannotFitWithTheSelection`
   bewegt sich **innerhalb** einer Gruppe auf eine Notiz, die nicht die erste
   ist — die Bedingung wird gar nicht betreten. Kein Prüfsatz des Bestands
   fängt das Entfernen der Passbedingung. **Bestandsbefund, gemeldet.**
2. Für den **neuen** Zweig ist der Fall nicht herstellbar: Im flachsten
   erreichbaren Fenster (447×168, Viewport 127 px) messen Kopf und erste Notiz
   zusammen 113 px. Es bleiben **14 px Reserve** — deutlich weniger als die 43,
   die die Vorprüfung angab, aber die Aussage trägt. Ein längerer Betreff oder
   eine größere Schrift könnte sie aufbrauchen; dann wäre der Fall erreichbar
   und prüfbar. **Als Impediment gemeldet.**

**Der Bestandsbefund aus dem Issue ist eingetreten:** Szene 7 des Bildläufers
war schon immer genau dieser Fall, und ihr Kommentar behauptete, der Kopf komme
mit. Das Bild vor der Heilung widerlegt ihn (`messungen/70-szene7-vor-der-heilung.png`,
kein Kopf „Letzte Woche"), das Bild danach zeigt ihn. Die Szene wurde nicht
angefasst.

## 4. #72 — Kriterium für Kriterium

| AK | Prüfmittel | Ergebnis | Mutationsprobe |
|---|---|---|---|
| **1** Drei Flächen tragen je einen Tooltip, „Rückgängig" über die `QAction` | `namesTheShortcutInTheTooltipOfEachActionSurface` | grün | **M11** — alle drei entfernt: rot |
| **2** Kürzel zur Laufzeit gelesen, nirgends als Literal; Lauf unter `LANG=C` grün | Erwartungswert aus `shortcut().toString(NativeText)` derselben Aktion | beide Läufe 113/113 grün | **M14** — Kürzel als Literal `"Entf"`: unter `LANG=C` rot, „Notiz bearbeiten (Entf)" statt „(F2)" |
| **3** Wortlaut `<Infinitiv> (<Kürzel>)` über einen Platzhalterstring | `i18nc("@info:tooltip", "%1 (%2)", …)` in `tooltipNaming()`, keine `+`-Verkettung | „Notiz bearbeiten (F2)", „Notiz löschen (Entf)", „Löschen rückgängig machen (Strg+Z)" | **M13** — Kürzelteil weggelassen: rot |
| **4** Keine Zusicherung „nicht leer"; geprüft wird der vollständige Text | drei `QCOMPARE` gegen den zusammengesetzten Erwartungstext | erfüllt | **M12** zeigt, warum: ohne `setToolTip` liefert die Aktion „Rückgängig" — **nicht leer**, und ein solcher Test wäre von Anfang an grün gewesen |
| **5** „Rückgängig" am erzeugten `QToolButton`, über `findChild` und `defaultAction()` | ebenda | grün | **M12** — nur diese Zeile entfernt: rot |
| **6** Zustand wird hergestellt, nicht vorausgesetzt | Meldungszeile über `Key_Delete` erzeugt, `QTRY_VERIFY` auf das Ende der Animation; Tooltips zusätzlich im Zustand mit ausgewählter Notiz geprüft | grün | — |

**Gebietsabhängigkeit belegt, nicht behauptet** (`messungen/72-kuerzeltext-je-gebiet.txt`):

```
LANG=de_DE.UTF-8   [Notiz bearbeiten (F2)] [Notiz löschen (Entf)]
LANG=C             [Notiz bearbeiten (F2)] [Notiz löschen (Del)]
```

Ohne diese Messung wäre der Doppellauf ein Lauf, der nichts belegt — zwei grüne
Ergebnisse sagen nichts, solange offen ist, ob sich überhaupt etwas unterscheidet.

**Eine Annahme des Kriteriums stimmte nicht.** AK 6 setzt voraus, dass „beide
Schaltflächen abgeschaltet starten". Gemessen sind es die beiden **Aktionen**;
die Knöpfe sind von Anfang an eingeschaltet und mit den Aktionen nur über
`clicked`/`trigger` verbunden, nicht über `setDefaultAction`. Vom Schirm hält
sie in diesem Zustand die Platzhalterseite vor dem Detailbereich. Der Prüfsatz
sichert deshalb den Zustand der **Aktion** zu und prüft die Tooltips zusätzlich
in dem Zustand, in dem die Knöpfe sichtbar sind. **Gemeldet.**

**Kein Bildbeleg, wie im Issue festgelegt:** Ein Tooltip ist ein eigenes
Fenster, `QWidget::grab()` zeigt ihn nie. `tests/libraryshots.cpp` wurde für
#72 nicht angefasst.

---

## 5. Was bei der Sichtprüfung herauskam — der Befund, der einen Nachtrag wert war

Szene 11 kam zweimal verschieden heraus. Nachgemessen
(`messungen/71-nachlaufender-autoscroll.txt`, Rollwert alle 50 ms nach dem
Klick):

```
Doppelklick-Intervall: 400 ms
nach    0 ms: Rollwert 6, markiert 14
…
nach  500 ms: Rollwert 6, markiert 14
nach  550 ms: Rollwert 7, markiert 14
…
nach  900 ms: Rollwert 7, markiert 14
```

**`QAbstractItemView` startet beim Mausdruck einen verzögerten Autoscroll und
holt die angeschnittene Zeile eine halbe Sekunde später doch ins Bild.** Die
Markierung wandert nicht mit — sie bleibt durchgehend auf der geklickten Zeile.

Was das für #71 heißt, ohne Beschönigung:

- **AK 1 ist erfüllt und bleibt es.** Der Auswahlfehler, der Gegenstand der
  Story war, ist geheilt — dauerhaft, nicht für eine halbe Sekunde.
- **AK 2 ist wörtlich erfüllt** („vor und nach dem Klick"), aber die Ruhe hält
  nur bis zum Doppelklick-Intervall. Wer den Kunden fragt, was er sieht, bekommt
  „die Zeile rutscht kurz darauf ganz ins Bild" — praktisch das Verhalten von
  Lesart 3, nur verzögert und ohne deren Wirkung auf das Mausrad.
- Die Sache **zu schließen** hieße, das Autoscrollen der Liste abzuschalten
  (`setAutoScroll(false)`). Das wirkt auch auf das Rollen beim Ziehen an den
  Rand und ist eine Produktentscheidung. **Deshalb gemeldet, nicht geheilt.**

Der Testkommentar in `librarytest.cpp` sagt die Grenze jetzt ausdrücklich —
sonst wäre der Prüfsatz für den nächsten Leser eine Falle: grün, und mit einer
Zusicherung, die weniger trägt als ihr Name verspricht.

Szene 11 nimmt seither **zwei** Bilder, beide über drei Läufe bitgleich:
`11a` unmittelbar nach dem Klick (Zeile angeschnitten, richtig markiert, Bild
unbewegt) und `11b` nach dem Nachlauf. Der Läufer bricht ab, wenn der Rollwert
schon vor 11a gewandert ist.

---

## 6. Befunde und Impediments für den PO

| # | Sache | Fläche | Vorschlag |
|---|---|---|---|
| **B-1** | **Nachlaufender Autoscroll** (Abschnitt 5). AK 2 von #71 hält nur bis zum Doppelklick-Intervall | `librarywindow.cpp`, aber Produktentscheidung | Eigenes Issue: Autoscroll der Liste abschalten oder das Verhalten als gewollt festschreiben. Der Kunde sieht es in zwei Sekunden |
| **B-2** | **Die Passbedingung des Kopfholens ist ungeprüft.** M10 entfernt sie, 112/112 bleiben grün; der Prüfsatz, der sie zu halten scheint, betritt die Bedingung gar nicht | `librarytest.cpp`, außerhalb der drei Stories | Eigenes Issue. Ein Prüfsatz mit Grenzübertritt in einer sehr hohen Gruppe fasst sie |
| **B-3** | **AK 5 von #70 ist strukturell nicht prüfbar** — im flachsten Fenster bleiben 14 px Reserve, der Fall tritt nie ein | — | Als Grenze festhalten. Wächst die Zeilenhöhe (längerer Betreff, größere Schrift), wird er erreichbar |
| **B-4** | **AK 6 von #72 setzte Falsches voraus** („beide Schaltflächen starten abgeschaltet") — es sind die Aktionen | — | Zur Kenntnis; der Prüfsatz ist an den gemessenen Bestand angepasst |
| **B-5** | **Szene 10c/10d des Bildläufers sind nicht bitgenau wiederholbar** — der Textcursor im Editor blinkt, die Aufnahme trifft mal die eine Phase, mal die andere | `libraryshots.cpp`, außerhalb der drei Stories | Eigenes Issue, falls Bildvergleiche automatisiert werden sollen |
| **B-6** | **Der Vorprüfbericht zu #70 ist unversioniert.** `docs/scrum/vorberichte/70-gruppenkopf-tastatur/` steht im Hauptarbeitsbaum als `??` und fehlt auf `sprint-07-basis`; gelesen wurde er von dort | `docs/` | Einchecken — ein unversionierter Beleg ist kein Beleg (B7) |
| **B-7** | **Der Daemon meldet bei jedem Start** `Failed to register with host portal … Connection already associated with an application ID` (Journal, vier Bootvorgänge) | `src/shell/`, außerhalb | Zur Kenntnis |

**Eine Unsauberkeit im eigenen Haus:** Der Nachtrag-Commit `018a7f9` hat die
Belegdateien von #72 mit aufgenommen (`72-*.txt`, `m11`–`m14`), weil der
Belegordner als Ganzes gestaged wurde. Nur Messausgaben, kein Code — aber die
Zuordnung Commit/Issue stimmt für diese Dateien nicht.

---

## 7. Was nicht geprüft ist

- **Ein echter Mausklick.** Alle Belege beruhen auf zugestellten Ereignissen.
  Dass ein Klick des Kunden denselben Weg durch
  `QAbstractItemView::mousePressEvent` nimmt, ist geschlossen, nicht gemessen.
  Das Issue nennt diese Grenze und legt sie in die Abnahme.
- **Der Tooltip nach Verweilzeit.** Ein Prozess bewegt unter Wayland den Zeiger
  nicht. Belegt ist der hinterlegte Text an allen drei Flächen; dass der Zeiger
  ihn nach der üblichen Verweilzeit zeigt, bleibt dem Blick des Kunden.
- **Der installierte Stand.** Nach `/usr` wurde nicht installiert; das taktet
  der PO. Alle Läufe liefen am gebauten Stand in `denkzettel-71/build`.
- **Ob der Handel „Lesbarkeit gegen Bewegung" (#70) im täglichen Gebrauch
  aufgeht.** Ein Agent kann das messen, nicht bewerten.

---

## Nachtrag 05.08.2026 — K3

**Der karpathy-Review hat an der Beweislage oben einen Mangel gefunden, und er
trägt.** Der Satz „Alles hier ist mit `pruefen.sh` in diesem Ordner
wiederholbar" stimmte nicht: Das Skript listete die abgelegten Ausgaben, aber
**der Eingriff jeder Probe stand nirgends**. Damit war nachprüfbar nur, dass
irgendetwas rot wurde — nicht, dass die genannte Zeile es war. Eine
Mutationsprobe ist der einzige Beleg dafür, dass eine Zusicherung überhaupt
etwas prüft; ohne ihren Eingriff ist sie eine Behauptung mit Anlage. Dazu sagte
`pruefen.sh` zweimal „zwölf", während vierzehn Läufe abgelegt sind.

**Ergänzt:**

- **`mutationsproben.sh`** in diesem Ordner, ausführbar, nach dem Muster von
  Strang A (`../sprint-07-s83-native-huelle/mutationsproben.sh`). Es arbeitet
  auf einer Kopie des Arbeitsbaums unter `/tmp`, nimmt je Probe den Eingriff
  selbst vor, fährt den Lauf, nennt das erwartete Ergebnis und baut zurück.
  Alle vierzehn stehen darin — **keine läuft von Hand.**
  Beleg des ersten vollständigen Laufs: `messungen/mutationsproben-lauf.txt`
  (14 Proben, **12 rot, 2 erwartungsgemäß grün**).
- **`pruefen.sh`** berichtigt: die beiden „zwölf" sind weg, der Abschnitt
  verweist auf das Nachbarskript statt die Ausgaben aufzuzählen.

**Zwei Dinge, die beim Schreiben des Skripts genauer wurden als oben:**

1. **M3, M4, M5 und M6 greifen in den Test, nicht in den Code** — das steht
   oben nur zwischen den Zeilen. Der Grund gehört dazu: QTest hält beim ersten
   Fehlschlag an, also fällt immer nur die vorderste Zusicherung. Wer die
   hinteren belegen will, muss die vorderen aussetzen. Im Skript steht das als
   Kommentar über den vier Proben.
2. **M12 meldet unter `LANG=C` „Ctrl+Z", nicht „Strg+Z".** Der Bericht oben
   nennt die deutsche Form, weil er den Wortlaut der deutschen Sitzung
   beschreibt. Beides ist richtig — und es ist dieselbe Gebietsabhängigkeit,
   gegen die AK 2 von #72 geschnitten ist. Das Skript sagt es an der Probe.

**Was hier nicht mehr offen ist** (Stand nach der Lieferung, zur Kenntnis):
**B-6** — der Vorprüfbericht zu #70 ist seit `5a7de67` eingecheckt. **B-1** ist
als **#89** gebucht und dem Blick des Kunden vorbehalten, **B-2** als **#90**,
**B-5** als **#91**, **B-7** als **#92**. Die Tabelle in Abschnitt 6 bleibt
stehen, wie sie war — sie beschreibt den Stand ihres Tages (B17).
