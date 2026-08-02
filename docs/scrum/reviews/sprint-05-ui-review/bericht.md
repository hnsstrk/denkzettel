# UI-Review Sprint 5 — Stories #66, #67 (Strang A) und #57, #58 (Strang B)

**Modus:** UI-Review (dritter Einsatzmodus des UI/UX-Agenten) ·
**Rolle:** UI/UX (`denkzettel-ux`) · **Datum:** 02.08.2026 ·
**Prüfgegenstand:** der gemergte Stand `main`, `5a4a83a` (Sprint-Diff
`sprint-05-basis..HEAD`, Basis `a322b86`)

**Verbindliche Bezugspunkte:** `wireframes/Denkzettel Wireframes.dc.html`
(2a Zustände A/B/C, Tafel „Symbole an den Schaltflächen", 2b/3a Meldungszeile),
`SPEC.md` 9 in der Fassung nach diesem Sprint, die Akzeptanzkriterien und
PO-Entscheidungen in `gh issue view 66/67/57/58`, KDE HIG.

## 0. Prüfmittel — wie die Bilder und Zahlen entstanden sind

Alles unten Gemessene stammt aus **eigenen Läufen**, nicht aus den Belegen der
Stränge. Die Bildläufer wurden vorher ausdrücklich frisch gebaut
(CLAUDE.md, Prüfhaltung; Vorfall dieses Sprints: `EXCLUDE_FROM_ALL` lässt einen
alten Läufer plausible Bilder des falschen Standes schreiben).

| Schritt | Befehl |
|---|---|
| Out-of-source-Bau des `main`-Standes | `cmake -B <scratch>/build-uxreview -S . -DCMAKE_BUILD_TYPE=Debug` |
| Läufer frisch | `cmake --build <scratch>/build-uxreview --target editshots libraryshots librarytest` |
| Umgebung aller Läufe | `QT_QPA_PLATFORM=offscreen`, `QT_QPA_PLATFORMTHEME=kde`, `-style breeze`, Bezugszeitpunkt Fr 31.07.2026 16:00, Fenster 900×600 |

Eigene Prüfmittel, hier abgelegt (B7 — ein unversionierter Beleg ist kein Beleg):

| Datei | Was |
|---|---|
| `ux-review-s5.cpp`, `ux-review-s5-CMakeLists.txt` | eigenes Prüfprogramm des Reviews; linkt nur gegen `libdenkzettelui.a`, kein Produktivcode |
| `messung-eigenpruefung.txt` | dessen Lauf gegen `main` |
| `messung-basisstand.txt` | **derselbe** Lauf gegen den Sprint-Basisstand `a322b86` (aus `git archive` ins Scratchpad, kein Arbeitsbaum im Repo) |
| `messung-szenen-main.txt` | Lauf des Szenenprogramms des S8-Reviews (`sprint-03-s5a-ak7-nachpruefung/ux-nachpruefung-ak7.cpp`, unverändert übernommen, md5 `873e1921…`) gegen `main` |
| `lauf-editshots.txt` | eigener Lauf von `editshots` (Symbolnamen, Vorgabeknopf, Warnsymbolgröße) |
| `bilder/` | 20 Bilder, alle aus diesen Läufen |

Zur Logausgabe, für den nächsten Prüfer: Qt schickt `qInfo()` in dieses System
an journald, sobald stderr kein Terminal ist — ohne `QT_FORCE_STDERR_LOGGING=1`
schreibt `editshots` seine Bilder, und die Messwerte daneben fehlen wortlos.

Randbefund: `librarytest` läuft am `main`-Stand unter dem Plattformthema
**107 grün / 0 rot**; der in Strang B gemeldete rote
`namesTheThreeAnswersOfTheGuardDialog` ist mit #66 grün geworden.

## 1. #67 — die sechs Symbol-Stellen gegen die Tafel

Verbindlich ist der **Name** (Tafel „Symbole an den Schaltflächen", 2a). Namen
per Struktur ausgelesen, Erscheinung im Bild geprüft.

Zur Zählung, damit sie nachvollziehbar bleibt: Die Tafel führt **fünf Namen an
sechs Stellen** — `edit-delete` trägt zwei davon. Fünf Stellen sind
Schaltflächen des Fensters (Bearbeiten, Löschen, Speichern, Abbrechen,
Rückgängig), die sechste ist „Verwerfen" im Wächterdialog; sie steht hier mit
in der Tabelle, weil sie denselben Namen prüft. Die beiden anderen
Dialogknöpfe und das Warnsymbol stehen in Abschnitt 2.

| Stelle | Tafel | gemessen | Bild | Verdikt |
|---|---|---|---|---|
| Detailkopf „Bearbeiten" | `document-edit` | `document-edit` | `bilder/s5-edit-01-lesen.png` | **ok** |
| Detailkopf „Löschen" | `edit-delete` | `edit-delete` | `bilder/s5-edit-01-lesen.png` | **ok** |
| Fußzeile „Speichern" | `document-save` | `document-save` | `bilder/s5-edit-02-bearbeiten.png` | **ok** |
| Fußzeile „Abbrechen" | `dialog-cancel` | `dialog-cancel` | `bilder/s5-edit-02-bearbeiten.png` | **ok** |
| Meldungszeile „Rückgängig" | `edit-undo` | `edit-undo` | `bilder/s5-edit-05-loeschmeldung.png` | **ok** |
| Wächterdialog „Verwerfen" | `edit-delete` (derselbe Name wie „Löschen") | `edit-delete` | `bilder/s5-edit-03-waechterdialog.png` | **ok** |

Fundstellen im Bau: `src/ui/librarywindow.cpp:173` (Rückgängig), `:361`, `:368`,
`:441`, `:444`, `:996`.

**Prüfsatz „Kein Symbol ohne Beschriftung" (Tafel): ok.** Alle sechs Stellen
tragen Text neben dem Symbol. Der einzige reine Symbolknopf im Fenster wäre der
Schließen-Knopf der Meldungszeile — er ist abgeschaltet
(`librarywindow.cpp:232`, `setCloseButtonVisible(false)`), im Bild steht rechts
nur „Rückgängig". Die Zeichnung sagt „Ein reiner Symbolknopf käme in diesem
Fenster nirgends vor" — das hält.

**Prüfsatz „Ein Name kann zwei Stellen tragen": ok.** `edit-delete` steht an
„Löschen" (Zustand A) und an „Verwerfen" (Dialog); beide stehen nie zugleich im
Bild, weil der Detailkopf während des Bearbeitens die Marke „wird bearbeitet"
zeigt (`bilder/s5-edit-02-bearbeiten.png`).

**Konsistenz mit dem Tray (#60): ok.** `document-edit` trägt im Tray „Notiz
erfassen" und hier „Bearbeiten" — die Tafel entscheidet das ausdrücklich so,
die Tray-Wahl ist unangetastet.

## 2. #66 — der Wächterdialog

Bild: `bilder/s5-edit-03-waechterdialog.png`, Zahlen: `lauf-editshots.txt`.

| Prüffrage aus Zeichnung 2a, Zustand C | Befund | Verdikt |
|---|---|---|
| Trägt der Dialog das große Warnsymbol `dialog-warning`? | ja, **64×64** gemessen, im Bild links oben | **ok** |
| Tragen die drei Antworten Symbole? | `document-save` · `edit-delete` · `dialog-cancel` | **ok** |
| Ist „Speichern" die Vorgabe (PO-Entscheidung F3)? | ja — am **sichtbaren** Dialog gemessen, im Bild am Fokusrahmen erkennbar | **ok** |
| Behalten die drei Antworten ihre Bedeutung? | Speichern schreibt, Verwerfen führt ohne Schreiben aus, Abbrechen bleibt im Editor — der Lauf endet über „Abbrechen" wieder im Bearbeiten-Zustand („Fenster nach ‚Abbrechen' noch offen: ja") | **ok** |
| Steht die Reihenfolge der Zeichnung entgegen? | nein — die Zeichnung sichert nur die Rollen zu | **ok** |
| Ist der Zweisatz zusammengezogen? | ja, ein Text mit Leerzeile (SPEC 9, `KMessageDialog` kennt keinen Zweittext) | **ok** |
| Fehlt die Abdunklung hinter dem Dialog? | ja, und das ist ausdrücklich **kein** Befund (Nachtrag von 02.08.2026 in der Zeichnung) | **ok** |

**Befund W1 (warn) — die Zeichnung trägt einen Wortlaut, den der Bau nicht mehr
hat, und eine Hierarchie, die er nicht herstellen kann.**
Gezeichnet: „Die Notiz von heute, 11:05 Uhr wurde geändert. Ohne Speichern
gehen die Änderungen verloren.", die Frage darüber in größerer Schrift.
Gebaut (`librarywindow.cpp:969`): „Änderungen speichern?" + Leerzeile + „Die
bearbeitete Notiz (Heute 11:05) hat ungespeicherte Änderungen. Ohne Speichern
gehen sie verloren." — beides in **derselben** Schriftgröße.

Der Wortlaut ist gedeckt (PO-Entscheidung 02.08.2026: Wortlaut beim Dev,
Bedeutung bleibt) und die Klammerform ist die bessere Lösung, weil der
Zeitstempel in drei Formen auftritt. Der Befund liegt bei der **Zeichnung**:
Wer sie das nächste Mal als Prüfmaßstab nimmt, misst gegen einen Satz, den es
nicht mehr gibt. Zur Hierarchie: Die KDE HIG stellen im Meldungsdialog die
Frage über die Erläuterung; ob `KMessageDialog` das ohne Zweittext hergibt,
habe ich **nicht gemessen** — das gehört zur Machbarkeitsfrage, nicht zum
Befund.
*Vorschlag:* Zustand C in 2a auf den gebauten Wortlaut nachziehen und dabei
festhalten, ob die typografische Hervorhebung der Frage gewollt ist. Beides
gehört in einen Gestaltungsauftrag, nicht in diesen Bericht — ich melde, ich
heile nicht.

## 3. #58 — Zeitstempel und Hinweise am Schemawechsel

### 3.1 Die Belege des Strangs

Geprüft: `sprint-05-s-verhalten/bilder/10a`–`10d` und die beiden ungeheilten
Vergleichsbilder, dazu `messung-farben.txt`.

* **Detailbereich sichtbar (UX-Ergänzung zu #58): ok.** 10a/10b zeigen den
  Lesezustand mit „Gestern 21:48" im Detailkopf, 10c/10d den Bearbeiten-Zustand
  mit vier `subtleLabel`-Stellen zugleich (Zeitstempel, „Kategorie", „Tags",
  Fußzeilenhinweis). Ein Fenster, ein Lauf, kein Neuaufbau — das ist die Bedingung
  aus AK 2 und sie ist eingehalten.
* **Beide Richtungen und die Farbablesung: ok.** Die Begründung des Strangs, warum
  das Bildpaar allein nicht trägt (zurück auf hell ist der Fehler unsichtbar,
  weil die eingefrorene Farbe zufällig stimmt), ist richtig und nachgemessen.
* **Befund B1 (warn) — die Bilder zeigen den Zweigstand, nicht den
  Auslieferungsstand.** In 10a–10d tragen „Bearbeiten"/„Löschen" und
  „Speichern"/„Abbrechen" **keine Symbole**: Sie entstanden auf
  `story/57-58-verhalten`, also ohne Strang A. Für #58 tragen sie trotzdem (die
  Labelfarbe hängt nicht an den Symbolen), als Bild des ausgelieferten Fensters
  nicht. Der Bericht nennt den Zweig, aber nicht diese Folge.
  *Vorschlag:* eine Zeile in `sprint-05-s-verhalten/bericht.md`, dass die Bilder
  den Zweigstand ohne #67 zeigen; das Bild des gemergten Standes liegt als
  `bilder/s5-gesamt-10c-main-dunkel-bearbeiten.png` hier daneben.

### 3.2 Eigenes Gegenbild und eigene Messung

Eigener Lauf, ein Fenster, kein Neuaufbau — und derselbe Lauf gegen den
Basisstand `a322b86` als Gegenprobe (`messung-basisstand.txt`).

| Zustand | `main` | Basisstand (ungeheilt) |
|---|---|---|
| hell, Detail-Zeitstempel | `#707d8a` auf `#eff0f1` | `#707d8a` |
| **nach dem Wechsel auf dunkel** | **`#a1a9b1`** auf `#202326` | **`#707d8a`** — eingefroren |
| dunkel, Fußzeilenhinweis | `#a0a8b0` | `#707d89` |
| dunkel, Beschriftung „Kategorie" | `#a1a9b1` | `#707d8a` |

Bilder: `bilder/s5-58-a-hell-lesen.png`, `s5-58-b-dunkel-lesen.png`,
`s5-58-c-dunkel-bearbeiten.png` gegen `s5-58-b-UNGEHEILT-dunkel-lesen.png`.

**Der eindeutige Nachweis — die Signalfarbe.** Statt eines zweiten Breeze-Schemas
habe ich `QPalette::PlaceholderText` auf `#ff00ff` gesetzt: Eine Farbe, die in
keinem Schema vorkommt, trennt „folgt der Rolle" von „sieht zufällig richtig aus".

* `bilder/s5-58-d-signalfarbe.png` (`main`): Detail-Zeitstempel, Fußzeilenhinweis,
  „Kategorie"/„Tags", die Listen-Zeitstempel und die Vorschauzeilen — **alle
  magenta**, gemessen `#ff00ff` bzw. `#ff01ff`.
* `bilder/s5-58-d-UNGEHEILT-signalfarbe.png` (Basisstand): Dieselben QLabel-Stellen
  bleiben **grau**, während die vom Delegate gemalten Listenzeilen magenta werden.
  Das ist der Fehler aus #58 in einem einzigen Bild, an genau den Stellen, die das
  Issue benennt.

**Verdikt #58: ok.** Kontrast auf dunkel von 3,75:1 (unter der WCAG-Schwelle
4,5:1) auf 6,64:1 — beide Werte des Strangs nachgerechnet und bestätigt
(WCAG-Formel, `#a1a9b1` bzw. `#707d8a` auf `#202326`). Kein weiteres Vorkommen der Bauart: `setPalette` kommt in
`src/` nicht mehr vor; verblieben sind `setForegroundRole`-Aufrufe
(`capturewindow.cpp:36`, `librarywindow.cpp:125`, `:353`).

## 4. #57 — der Weg als Prüfgegenstand

### 4.1 Nachvollzug der Rollwert-Messung

Gefahren: das Szenenprogramm des S8-Reviews, **unverändert** (md5 vor dem Lauf
geprüft), gegen den `main`-Stand.

```
diff sprint-05-s-verhalten/messung-szenen-nachher.txt  messung-szenen-main.txt
→ kein Unterschied
diff sprint-05-s-verhalten/messung-szenen-vorher.txt   messung-szenen-main.txt
→ 71c71: Rollwert 6 → 0, Kopf y=0   |   Rollwert 6 → 6, Kopf y=-387
```

* **Die geänderte Szene (N11): ok.** Der Klick auf eine sichtbare Notiz einer
  anderen Gruppe lässt den Rollwert bei 6 — das Bild steht, die geklickte Zeile
  bleibt unter dem Zeiger. Der 387-px-Sprung ist weg.
* **N10 (Tastatur, Grenzübertritt auf eine schon sichtbare Zeile): unverändert,
  ok.** „Rollwert 8 → 7, Kopf ‚Gestern' y=0 IM BILD" — zeilengleich mit dem
  Vorher-Lauf. Die AK-7-Heilung ist unangetastet.
* **Alle übrigen elf Szenen: unverändert, ok.** Auch N2 (der zweite Datenpunkt des
  Issues, Kopf bei y=−35 draußen) steht unverändert da; er bleibt offen und ist
  vom Strang als Vorschlag an den PO gemeldet.

### 4.2 Eigene Zusatzszenen — Wege, die das Szenenprogramm nicht hat

`messung-eigenpruefung.txt`, dieselbe Umgebung, Bestand zwei Gruppen zu je acht.

| Szene | Erwartung aus SPEC 9 / #57 | gemessen | Verdikt |
|---|---|---|---|
| B2 Klick auf eine sichtbare Zeile **derselben** Gruppe | Bild steht | Rollwert 3 → 3 | **ok** |
| B3 Klick, **danach** zweimal Pfeil ab über die Grenze | Kopf wird geholt | Rollwert 0 → 3, Kopf y=432 im Bild | **ok** |
| B4 Klick auf einen **Gruppenkopf** (wählt nichts), dann Taste | Merker darf nicht kleben | Kopf y=432 im Bild | **ok** |
| B5 Klick in den **Leerraum**, dann Taste | dasselbe | Kopf y=72 im Bild | **ok** |

Die drei letzten prüfen die Stelle, an der die Regel „Zeigen ist nicht Tippen"
brechen könnte: Ein Mausdruck, der nichts auswählt, färbt den nächsten
Tastendruck nicht ein. Sie halten.

### 4.3 Befund B2 (warn) — der Fall neben der Zusicherung

**Beobachtung, gemessen** (`bilder/s5-57-b1a-vor-dem-klick.png`,
`s5-57-b1b-nach-dem-klick.png`, `s5-57-b6-nach-dem-loslassen.png`):

Die unterste Zeile steht bei y = 539 im 552 px hohen Bild, also **angeschnitten**.
Ein Klick auf ihren sichtbaren Teil:

```
vor dem Druck:       Rollwert 6, aktuelle Zeile 11, markiert 11
nach dem Druck:      Rollwert 7, aktuelle Zeile 14, markiert 15
nach dem Loslassen:  Rollwert 7, aktuelle Zeile 14, markiert 15
Zeile unter dem Zeiger am Ende: 15
```

Zwei Wirkungen, beide im Bild zu sehen:

1. Das Bild rückt um **eine Zeilenhöhe (72 px)** — die angeklickte Zeile wandert
   unter dem Zeiger weg. Das ist derselbe Vorgang, gegen den #57 gefasst ist, nur
   um den Faktor fünf kleiner.
2. Weil die Liste **während** der Verarbeitung des Mausdrucks rollt, liegt danach
   eine andere Zeile unter dem Zeiger: Der `QListView` markiert Zeile 15, der
   Detailbereich zeigt Zeile 14. Im Bild `s5-57-b6-nach-dem-loslassen.png` steht
   rechts „Gestern 11:00 / Notiz 4 von gestern", während links die Zeile
   **darunter** blau markiert ist und „Notiz 4" unmarkiert bleibt.

**Einordnung — kein Rückschritt dieses Sprints.** Derselbe Lauf gegen den
Basisstand `a322b86` liefert Zeile für Zeile dasselbe
(`messung-basisstand.txt`, Abschnitt B6). Ursache ist nicht die neue Regel,
sondern das unbedingte `m_list->scrollTo(index, EnsureVisible)` in
`src/ui/librarywindow.cpp:783`, das aus `currentChanged` **synchron** mitten in
der Mausdruckverarbeitung läuft. Die Akzeptanzkriterien von #57 sind nicht
verletzt: Sie sprechen von einer *bereits vollständig sichtbaren* Zeile, und die
ist hier nicht gegeben.

*Vorschlag an den PO:* als eigenes Issue führen, neben #59 und dem offenen
N2-Fall. Zwei Lesarten sind vertretbar — „eine angeschnittene Zeile ganz ins Bild
holen" (heutiges Verhalten, aber mit falscher Markierung) oder „auch hier steht
das Bild still" (konsequent zu #57). Die zweite braucht nur, dass `scrollTo` beim
Zeigen denselben Merker respektiert wie das Vorscrollen zum Kopf; die erste
braucht, dass die Bewegung **nach** der Auswahlverarbeitung geschieht. Welche —
ist Produktentscheidung, nicht meine.

## 5. HIG-Blick auf das Gesamtbild der Bibliothek

Bilder: `bilder/s5-gesamt-01-normalfall.png`, `s5-gesamt-02-leerzustand.png`,
`s5-gesamt-10c-main-dunkel-bearbeiten.png`, dazu die fünf `s5-edit-*`.

| Prüffrage | Befund | Verdikt |
|---|---|---|
| Symbole erklären Beschriftungen, ersetzen sie nicht | überall Text neben Symbol | **ok** |
| Destruktive Handlungen erkennbar | „Löschen" und „Verwerfen" tragen das rote Breeze-Symbol, die harmlosen ein neutrales | **ok** |
| Vorgabe auf der Antwort, die nichts verliert | „Speichern" trägt Fokus und Vorgabe | **ok** |
| Rückmeldung statt Meldungslärm | Speichern wechselt sichtbar in die Leseansicht, kein Toast; die Löschung meldet sich mit Frist und Rückgängig | **ok** |
| Lesbarkeit nach einem Schemawechsel | 6,64:1 statt 3,75:1 | **ok** |
| Sichtbare Bedienelemente **und** Tastenwege | F2, Entf, Strg+Z, Strg+Enter, Esc, Strg+W (`librarywindow.cpp:249`–`282`) | **ok** |
| Raumaufteilung Liste/Detail unverändert | 300 px Liste zu 600 px Detail bei 900 px Fensterbreite (`librarywindow.cpp:46`, `:221`); Zeichnung 3a skizziert 280 zu 660, sichert aber keine Maße zu — dieselbe Gewichtung | **ok** |

**Befund H1 (warn, klein) — die Tastenwege stehen nirgends am Bedienelement.**
Einen Tooltip trägt heute nur das Suchfeld (`librarywindow.cpp:1061`).
Die KDE HIG empfehlen für Schaltflächen mit Kürzel den Hinweis im Tooltip;
„Bearbeiten (F2)", „Löschen (Entf)", „Rückgängig (Strg+Z)" wären drei Zeilen.
Der Fußzeilenhinweis „Esc bricht ab · Strg+Enter speichert" zeigt, dass die App
diesen Gedanken sonst pflegt — im Detailkopf fehlt er. Nie beauftragt, deshalb
Empfehlung und kein Mangel.

**Randnotiz ohne Verdikt (außerhalb dieses Sprints):** Die Meldungszeile zieht
„Notiz gelöscht" und die Restfrist in **einen** Text zusammen
(„Notiz gelöscht — noch 5 s"); Zeichnung 2b hat beide getrennt. Alter Stand aus
S5a, hier nur festgehalten, damit es nicht als Nebenwirkung von #67 gelesen wird.

## 6. Gesamtverdikt

**Die vier Stories sind aus UI/UX-Sicht abnahmefähig.** Alle geprüften
Zusicherungen aus Zeichnung, SPEC und Akzeptanzkriterien halten am gebauten
Stand — sechs Symbol-Stellen, Warnsymbol, Vorgabeknopf, Schemawechsel in beiden
Richtungen, Rollwert-Verhalten von Maus und Tastatur.

Drei Punkte gehen als **warn** an den PO, keiner davon blockiert:

| # | Befund | Art |
|---|---|---|
| W1 | Zeichnung 2a, Zustand C trägt einen überholten Dialogtext und eine Hierarchie, die der Bau nicht herstellt | Gestaltungsauftrag |
| B1 | Die #58-Bilder des Strangs zeigen den Zweigstand ohne die Symbole aus #67 | Belegpflege |
| B2 | Klick auf eine **angeschnittene** Zeile bewegt das Bild um 72 px und markiert die Nachbarzeile — Bestand, keine Regression | eigenes Issue |

Dazu eine Empfehlung ohne Mangelcharakter: **H1**, Tooltips mit Tastenkürzel an
den drei Schaltflächen des Detailkopfs und der Meldungszeile.
