# Sprint 5 — Planning-Protokoll

**Datum:** 2026-08-02, 16:17 (Ganymed) · **Moderation:** Scrum Master (Agent
`scrum-master`)
**Teilnehmer:** Scrum Master · Product Owner · Schätzer Dev · UI/UX (Agent
`denkzettel-ux`, Planning-Beratung zu #66/#67 und Zweitschätzung).
**Status des Sprint-Vorschlags:** vorgelegt, Freigabe durch den Kunden steht aus.

**Grundlagen:** `CLAUDE.md`, `docs/scrum/PROZESS.md` (Stand nach den
Beschlüssen B11–B15 und den Kundenentscheidungen vom 02.08.2026),
`docs/scrum/sprints/sprint-04.md` (§15–§18), `docs/scrum/reviews/sprint-04-karpathy.md`
(Befund 3.1), `SPEC.md` (9, 16), GitHub-Issues #55, #57, #58, #59, #66, #67, #69.
**Quellstand aller Prüfungen:** `main` @ `47539f9`; Zeilennummern sind an
diesem Stand nachgezählt.

**Kundenrichtung für diesen Sprint:** „Denkzettel sieht und verhält sich wie
eine KDE-Anwendung."

Alle Aussagen über Konflikte, Prüfmittel und Risiken sind am Quellcode oder an
einer eigenen Messung geführt, nicht an den Story-Texten. Wo ich eine fremde
Angabe nur wiedergebe, steht es dabei.

## 1. Sprint-Konto (B12) — ab Zeile 1

| Buchung | Issues | Story Points | Grenzen (2–4 · ~13) |
|---|---|---|---|
| Kandidatenfeld (#66, #67, #57, #58, #59, #55) | 6 | **23** | beide gerissen |
| Nach den Vorentscheidungen des PO (#55 raus, 3) | 5 | **13** | **Story-Grenze gerissen**, SP am Anschlag |
| Vorschlag des Scrum Masters (#59 raus, 4.3) | **4** | **11** | beide gehalten, Story-Grenze **am Anschlag** |
| *Freigabe-Stand* | *einzutragen nach der Kundenentscheidung* | | |

**Der erste Befund des Kontos betrifft nicht die Punkte, sondern die Zahl der
Issues.** Der vorgeschlagene Schnitt trägt 11 von ~13 SP — zwei Punkte Luft —,
aber **4 von 4 Issues**. Die Story-Grenze ist damit ausgeschöpft: In Sprint 5
ist **jeder** Zugang nach der Freigabe eine Grenzüberschreitung und dem Kunden
nach B12 als solche vorzulegen, auch ein Ein-Punkt-Nachtrag. Sprint 4 hatte an
dieser Stelle noch ein Issue Luft; dieser Sprint hat keine. Das gehört vor die
Freigabe, nicht in die Rückschau (Sprint 3, 12.7).

*Zur Herkunft der 23:* #55 trägt 8 SP (deckungsgleich geschätzt) und ist vom PO
begründet draußen (4.1); #69 ist nicht schätzbar und deshalb gar nicht im Feld
(2.4).

## 2. Schätzstand und Konsolidierung

| Issue | Story | Dev | UX | Alt-Label | **Konsolidiert** | Regel |
|---|---|---|---|---|---|---|
| #66 | Wächterdialog KDE-Bauart | 5 | 3 | — | **5** | ≤1 Stufe → höherer Wert |
| #67 | Bibliotheks-Symbole | 2 | 1 | — | **2** | ≤1 Stufe → höherer Wert |
| #57 | Klick-Sprung | 3 | 3 | `sp:2` | **3** | deckungsgleich; Alt-Label ersetzt (2.1) |
| #58 | Palettenrolle | 1 | 1 | `sp:1` | **1** | dreifach deckungsgleich |
| #59 | Scrollstelle | 2 | 2 | `sp:2` | **2** | dreifach deckungsgleich |
| #55 | Fensterhülle | 8 | 8 | — | **8** | deckungsgleich; nicht im Schnitt (4.1) |

**Keine 13er-Story im Feld** — nichts ist teilungsbedürftig.

**Zur Anwendung der Stufenregel:** In der Fibonacci-Reihe (1, 2, 3, 5, 8, 13)
sind 3 und 5 benachbart, ebenso 1 und 2. Beide Abweichungen sind **eine Stufe**,
und die Regel greift ohne Ermessen: der höhere Wert. Eine begründete
Konsolidierung durch den Scrum Master war in diesem Planning **an keiner Stelle
nötig** — anders als in Sprint 4, wo #11 sie erzwang.

### 2.1 #57 — die beiden Schätzer ersetzen das Label, sie übernehmen es nicht

Das Label `sp:2` stammt aus der Zeit, als das Issue geschrieben wurde. Beide
Schätzer sagen heute unabhängig **3** und begründen die Erhöhung gleichlautend
mit dem, was im Issue selbst steht: Der Gegenstand ist keine Codeänderung,
sondern **eine Regelsuche**. Die naheliegende Regel („nur vorscrollen, wenn die
Auswahl nicht ohnehin sichtbar ist") ist am 01.08.2026 belegt widerlegt worden
— sie hätte den gerade geheilten AK-7-Fall wiederhergestellt. Das Issue trägt
zwei Datenpunkte mit **vertauschten Vorzeichen**: 387 px Sprung, der nicht
stattfinden soll, und 35 px Ruck, der zu Unrecht unterbleibt. Eine Lösung, die
nur einen von beiden trifft, verschiebt den Fehler.

**Entscheidung: 3 SP.** Die 2 wird nicht übernommen, sie wird ersetzt — dieselbe
Form wie bei #11 im Sprint-4-Planning (dort 2.1).

### 2.2 #58 und #59 — dreifache Deckung, keine Diskussion

Beide Schätzer bestätigen unabhängig das Alt-Label. Bei #58 ist die Heilung
zeichengenau bekannt (`setForegroundRole(QPalette::PlaceholderText)`), das
Vorbild für den Test steht in `CaptureTest::textsFollowAColourSchemeChange()`,
und die Stolperstelle (`processEvents()`) steht im AK. Das ist die seltene Lage,
in der eine 1 wirklich eine 1 ist.

### 2.3 #66 gegen #67 — warum 5 und 2 und nicht zweimal dasselbe

Beide Issues sagen „Symbole fehlen", und beide werden im selben Strang gebaut
(4.4). Trotzdem sind sie **nicht gleich groß**, und die Schätzung bildet das ab:

- **#67 sind gewöhnliche `QPushButton` im eigenen Fenster.** `QIcon::fromTheme`
  bzw. `KStandardGuiItem` wirken dort direkt; die Arbeit ist vier Zuweisungen
  plus Zusicherungen auf `icon().name()`.
- **#66 ist ein Umbau der Dialog-Bauart.** `QMessageBox` → `KMessageBox`/
  `KMessageDialog` zieht die Rollen (`Yes`/`No`/`Reject` statt
  `Accept`/`Destructive`/`Reject`), den Vorgabeknopf, den Wegfall von
  `informativeText` und **fünf `QMessageBox*`-Casts in den Tests** nach sich
  (UX-Befund F2, PO-Kommentar an #66). Die 3 des UX-Schätzers ist die
  optimistische Lesart derselben Lage; die Stufenregel existiert genau dafür.

## 2.4 Was nicht geschätzt werden konnte

**#69 (S35, spellfix1)** liegt seit dem Spike vor und benennt selbst drei
Klärungen vor der Schätzung (Nachführung der spellfix1-Tabelle, Schwellwert und
Ranking, UX-Moment der Rückmeldung). Nach der Sprint-Mechanik ist eine Story
ohne zwei unabhängige Schätzungen nicht ziehbar; hier fehlt sogar die
Schätzgrundlage. **Nicht im Kandidatenfeld** — kein Mangel, ein Zustand, der vor
dem Sprint-6-Planning zu heilen ist. Dieselbe Lage wie #62 im Sprint-4-Planning
(dort 2.3).

## 3. Konfliktanalyse am Code — mit zwei eigenen Messungen

### 3.1 Was welche Story anfasst (Fundstellen an `47539f9` nachgezählt)

| Issue | Fundstellen | Beleg |
|---|---|---|
| #66 | `src/ui/librarywindow.cpp:842–880` (Wächterdialog), Includes `:13`/`:28`; `tests/librarytest.cpp` Helfer `answerNextDialog` (`:864`), `namesTheThreeAnswersOfTheGuardDialog` (`:2538–2591`), `asksBeforeUnsavedChangesAreLost` (`:2728–2810`); `tests/CMakeLists.txt:45`; SPEC 9 (`:423–432`) | selbst gelesen |
| #67 | `src/ui/librarywindow.cpp:301–311` (Bearbeiten/Löschen), `:375–380` (Speichern/Abbrechen), `:125`/`:183` (Rückgängig an der `KMessageWidget`) | selbst gelesen |
| #57 | `src/ui/librarywindow.cpp:596–685` (`showNote`, `crossesAGroupBoundary` `:670`, `scrollTo` `:676`/`:679`); `tests/librarytest.cpp:1398–1447` | selbst gelesen |
| #58 | `src/ui/librarywindow.cpp:75–81` (`subtleLabel`), Aufrufer `:110`, `:291`, `:368`, `:371`, `:389` | selbst gelesen; Verdachtszeile aus dem #58-Kommentar bestätigt |

### 3.2 Die Kollisionsfläche — benannt, nicht wegdiskutiert

**Beide Kandidaten-Stränge schreiben in dieselben zwei Dateien**:
`src/ui/librarywindow.cpp` und `tests/librarytest.cpp`. Das ist unvermeidlich —
vier Befunde in einem Fenster lassen sich nicht in disjunkte Dateien schneiden.
Entscheidend ist deshalb nicht die Datei, sondern der **Abstand der Bereiche**:

| Datei | Strang A (#66, #67) | Strang B (#57, #58) | kleinster Abstand |
|---|---|---|---|
| `src/ui/librarywindow.cpp` | 13, 28, 125, 183, 301–311, 375–380, 842–880 | 75–81, 596–685 | **43 Zeilen** (81 → 125) |
| `tests/librarytest.cpp` | 864, 2538–2591, 2728–2810 + Deklarationen bei 179–185 | 1398–1447 + Deklarationen bei 140–141 | **38 Zeilen** (141 → 179) |

Git mischt mit drei Zeilen Kontext. Bei 38 Zeilen kleinstem Abstand ist ein
Textkonflikt **ausgeschlossen, solange jeder Strang in seinem Bereich bleibt** —
dieselbe Lage wie `tests/CMakeLists.txt` in Sprint 4, wo der Merge beider PRs
zwei getrennte, konfliktfreie Hunks erzeugte (karpathy 3.1).

**Nicht geteilt wird:** `tests/CMakeLists.txt` gehört **allein Strang A** (5.2),
`tests/editshots.cpp` allein A, `tests/libraryshots.cpp` allein B.

**SPEC 9 ist die dritte gemeinsame Datei, und diesmal derselbe Abschnitt.**
A schreibt in die Bedingungen des Bearbeiten-Zustands (`SPEC.md:405–432`);
B schreibt, **falls** es eine Bedingung entdeckt, in den Listenteil
(`:385–397`) — oberhalb von Zeile 398. Festlegung, damit es nicht erst beim
Merge auffällt.

### 3.3 Messung 1 — das Plattformthema verschiebt keine Geometriewerte

**Das benannte Risiko F2 der UX-Beratung lautet:** `QT_QPA_PLATFORMTHEME=kde` an
`librarytest` kann Geometriewerte bestehender Tests verschieben. Ich habe es
nicht abgewogen, sondern gefahren (Binär `build/bin/librarytest`, gebaut
14:38:55 aus den Quellen von 14:38:30, also aktuell):

| Lauf | Ergebnis |
|---|---|
| `QT_QPA_PLATFORM=offscreen` (heutiger Stand) | **102 passed, 0 failed** |
| `QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde` | **101 passed, 1 failed** |

Der eine rote Test ist `namesTheThreeAnswersOfTheGuardDialog`, und er fällt
**an der Symbolzusicherung**, nicht an einer Geometrie:

```
FAIL!  : LibraryTest::namesTheThreeAnswersOfTheGuardDialog()
         '!icons.contains(QString())' returned FALSE. (||)
   Loc: [tests/librarytest.cpp(2589)]
```

**Kein einziger Geometrietest wird rot** — weder `keepsTheMeasuresOfTheEditState`
(zwei Fenstergrößen), noch `keepsTheMeasuresOfTheGroupedList`, noch
`keepsTheHeaderAtTheTopAndTheRestForTheNotes`, noch die vier Rollwert-Tests der
Gruppenköpfe. **F2 ist damit für den heutigen Testbestand entkräftet**, und der
Risikoeintrag in 7 verschiebt sich auf die *neuen* Zusicherungen beider Stränge.

### 3.4 Messung 2 — der Kundenbefund aus #66 ist offscreen reproduzierbar

Die Fehlermeldung oben trägt mehr, als sie auf den ersten Blick zeigt. Die
Zusicherungen **vor** der roten Zeile sind alle durchgelaufen: drei Knöpfe,
Beschriftungen *Speichern · Verwerfen · Abbrechen*, Rollen `AcceptRole` /
`DestructiveRole` / `RejectRole`. Erst `icon().name()` liefert dreimal die leere
Zeichenkette — die Meldung zeigt sie als `||`, also drei leere Felder.

Ohne Plattformthema sind dieselben drei Namen gesetzt; **mit** ihm sind sie leer.
Das ist genau der Befund, den der Kunde am Bild gemeldet hat
(`docs/scrum/reviews/sprint-04-kundenabnahme/waechterdialog-ohne-symbole.png`).

**Folge für das Planning:** Die Testumgebungsfrage aus #66 AK 3 hat eine
gemessene Antwort — **eine Zeile in `tests/CMakeLists.txt` macht den O3-Befund
offscreen als roten Test reproduzierbar, bevor er geheilt wird.** Der Dev muss
ihn nicht erst herbeiführen. Was die Messung **nicht** sagt: dass
offscreen+`kde` dasselbe ist wie eine Plasma-Sitzung. Der Bildnachweis am echten
Plasma (#66 AK 1, #67 letztes AK) bleibt unberührt.

### 3.5 Ein Test schreibt heute fest, was #57 beseitigen soll

`tests/librarytest.cpp:1398–1447`,
`bringsTheHeadAlongWhenAVisibleNoteOfAnotherGroupIsClicked`, sichert das
**heutige** Verhalten zu: Nach dem Klick auf eine sichtbare Notiz anderer Gruppe
ist deren Kopf im Bild. Sein Kommentar sagt selbst, wo er steht:

> *„Should it grate in daily use, this is the test that says where the decision
> was made."*

Es hat gestört, der Kunde hat es gemeldet, das Issue liegt vor. **Strang B kehrt
diesen Test um, er ergänzt ihn nicht.** Wer das übersieht, baut die Heilung und
hält den daraufhin roten Test für seinen eigenen Fehler — die Bauart, gegen die
`CLAUDE.md` die Prüfhaltung fasst. Der Prüfsatz für den Ersatz steht im
#57-Kommentar und ist umsetzungsoffen formuliert:

> *Ein Mausklick auf eine bereits vollständig sichtbare Zeile darf den Rollwert
> nicht verändern.*

## 4. Sprint-5-Vorschlag

### 4.1 Die Vorentscheidungen des PO — nachvollzogen, nicht neu verhandelt

- **#55 (Fensterhülle, 8 SP) bleibt draußen.** Drei Gründe, alle belegt: beide
  Schätzer sagen unabhängig 8; die Akzeptanzkriterien wurden heute erst neu
  gefasst (A1–A9); der Gestaltungsauftrag zum 4b-Schatten-Satz läuft noch und
  ist im Issue selbst als „vor dem Ziehen" markiert. Mit 8 SP wäre daneben nur
  noch eine Ein-Punkt-Story möglich — der Sprint hätte kein Thema mehr, sondern
  ein Vorhaben. **Sprint-6-Kandidat.**
- **#69 (spellfix1) ist nicht ziehbar** — siehe 2.4.

### 4.2 Sprint-Ziel

> **Die Bibliothek sieht und verhält sich wie eine KDE-Anwendung: Jede
> Schaltfläche und jede Antwort im Nachfragedialog trägt ihr Symbol, die Liste
> springt nicht mehr unter dem Zeiger weg, und Zeitstempel und Hinweise folgen
> dem Farbschema, während der Dienst läuft.**

Nachprüfbar in vier Handgriffen ohne Werkzeug:

1. Bibliothek öffnen, eine Notiz auswählen — **„Bearbeiten" und „Löschen" tragen
   Symbole** (#67).
2. F2, etwas tippen, eine andere Notiz anklicken — die drei Knöpfe der Nachfrage
   **tragen Symbole** (#66).
3. In der Liste weit nach unten rollen, dann eine sichtbare Notiz einer anderen
   Gruppe anklicken — **das Bild bleibt stehen** (#57).
4. Farbschema wechseln, **ohne** den Dienst neu zu starten — die Uhrzeiten in der
   Liste wechseln die Farbe mit (#58).

Das Ziel zahlt in beiden Hälften auf die Kundenrichtung ein: 1 und 2 sind
*„sieht aus wie"*, 3 und 4 sind *„verhält sich wie"*.

### 4.3 Der Schnitt

| Strang | Issue | Story | SP |
|---|---|---|---|
| A | #66 | Wächterdialog auf KDE-Bauart | 5 |
| A | #67 | Symbole an den Bibliotheks-Schaltflächen | 2 |
| B | #57 | Klick auf sichtbare Notiz lässt das Bild springen | 3 |
| B | #58 | Zeitstempel folgen dem Themewechsel nicht | 1 |
| | | **Summe** | **11** |

**#59 bleibt draußen** — nicht, weil es unwichtig wäre, sondern weil es die
Story-Grenze reißt (fünf Issues, 1). Die Punkte trügen es (13 von ~13), die
Zahl der Issues nicht. Genau diese Blindstelle hat B12 erzeugt: Wer 13 im Blick
hat, sieht die 13 einhalten und übersieht die 5 (Sprint 3, 12.7).

**Der Reviewer hat empfohlen, #57 und #59 „zusammen zu entscheiden".** Diese
Empfehlung wird eingehalten, ohne #59 zu ziehen: Der Spawn-Auftrag für Strang B
legt #59 bei, und die Regel, die B für #57 findet, **darf #59 nicht verbauen**.
Das kostet nichts — die Heilung von #59 (Neugruppieren nur bei Kalendertagswechsel)
sitzt in `regroupList()` und nicht in `showNote()`, ist also von #57s Regel
unabhängig. Was sie braucht, ist ein Blick des Strangs, keine Umsetzung.

*Falls der Kunde fünf Issues zulässt:* #59 ist der Zugang, und der Schnitt
lautet 5 Issues / 13 SP — beide Grenzen am oder über dem Anschlag, ohne jede
Luft für Review-Auflagen. Sprint 2 erzeugte allein aus einem UI-Review drei
Auflagen, Sprint 3 einen `fail` und einen `warn`. **Empfehlung: vier Issues.**

### 4.4 Warum #66 und #67 ein Strang sind und #57 und #58 der andere

**A = #66 + #67:** Beide brauchen dieselbe eine Zeile in `tests/CMakeLists.txt`
(3.4), beide arbeiten am selben Gegenstand (Symbole an beschrifteten
Bedienflächen), und #67s AK verlangt ausdrücklich Konsistenz der Symbolwahl mit
#66 und dem Tray-Menü. Zwei Stränge hier hieße: zwei Agenten streiten um eine
CMake-Zeile und um dieselbe Symboltabelle. **UX bestätigt die Bündelung.**

**B = #57 + #58:** Getrennte Codebereiche (`showNote` und `subtleLabel`), aber
gemeinsam die Eigenschaft, dass ihr Prüfgegenstand **nicht das Bild ist, sondern
der Verlauf** — Rollwert vorher/nachher bei #57, Farbe vor/nach dem Schemawechsel
bei #58. Beide brauchen denselben Bildläufer (`tests/libraryshots.cpp`) und
dieselbe Sorgfalt beim Ereignisumlauf (`processEvents()`).

**Empfohlene Zahl der Dev-Agenten: zwei.** Sie folgt aus den Bereichen, nicht
aus der Erlaubnis.

## 5. Strang-Zuschnitt und Dateimengen (B13, ergänzte Notation)

### 5.1 Die Notation ist erweitert — und warum

Der karpathy-Review von Sprint 4 (Befund 3.1, `warn`) hat festgestellt: **Beide
Stränge des Sprints schrieben außerhalb ihrer notierten Dateimenge** — Bildläufer
angelegt, `tests/CMakeLists.txt` verdrahtet, Belege unter
`docs/scrum/reviews/<story>/` abgelegt. Jede dieser Zeilen war auf eine Pflicht
rückführbar (DoD 3: eigene Bilder; B7: versionierte Belege). *„Der Mangel liegt
in der zu engen Dateimengen-Notation des Plannings, nicht in Eigenmacht der
Stränge."*

Die Tabelle unten trägt deshalb **drei Spalten statt einer**: Quellen und Tests,
Belege und Prüfmittel, fachliche Quellen. Ein Strang, der seine Belegpflicht
erfüllt, ist damit nicht mehr formal im Verstoß.

**Der Prozess-Nachzug bleibt Retro-Kandidat für Sprint 6** (Sprint 4, 17.6,
Punkt 2). Dieses Planning wendet die Form an; sie in `PROZESS.md` zu schreiben
ist eine Retro-Entscheidung, keine des Scrum Masters im laufenden Planning.

### 5.2 Worktree-, Zweig- und Dateimengenzuordnung

| | **Strang A** | **Strang B** |
|---|---|---|
| **Issues** | #66, #67 (7 SP) | #57, #58 (4 SP) |
| **Zweig** | `story/66-dialog-symbole` | `story/57-listenruhe` |
| **Quellen & Tests** | `src/ui/librarywindow.cpp` — **nur** Includes (`:13`, `:28`), `:125`, `:183`, `:301–311`, `:375–380`, `:842–880`; `tests/librarytest.cpp` — Dialog- und Symbolzusicherungen, Helfer `answerNextDialog` (`:864`), Deklarationen im Block ab `:179` | `src/ui/librarywindow.cpp` — **nur** `:75–81` (`subtleLabel`), `:596–685` (`showNote`); `tests/librarytest.cpp` — Rollwert- und Palettenzusicherungen, Umkehrung von `:1398–1447`, Deklarationen im Block bei `:140` |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-05-s66-s67/` (Bericht, eigene Bilder, Messprotokolle); `tests/editshots.cpp`; `tests/CMakeLists.txt` — **nur** der `librarytest`-Block (`:43–45`) und der `editshots`-Block (`:66–73`) | `docs/scrum/reviews/sprint-05-s57-s58/` (Bericht, Bilder hell/dunkel, Rollwert-Messprotokolle); `tests/libraryshots.cpp` — **kein** CMake-Eingriff nötig, der Block (`:57–64`) besteht |
| **Fachliche Quellen** | SPEC 9 **nur** `:405–432`; SPEC 16, falls die Teststrategie nachzieht; Zeichnungen 2a/2b (Symbole) über den laufenden Gestaltungsauftrag — **nicht** durch den Dev | SPEC 9 **nur oberhalb** `:398`, und nur falls eine Bedingung entdeckt wird (DoD 4/B9) |
| **Ausdrücklich nicht** | `showNote`, `subtleLabel`, `libraryshots.cpp` | `tests/CMakeLists.txt`, `editshots.cpp`, der Wächterdialog, der Helfer `answerNextDialog` |

### 5.3 Die drei Festlegungen zur Kollisionsfläche

1. **Die `QT_QPA_PLATFORMTHEME`-Zeile setzt Strang A.** Sie gehört zu #66/#67
   (die Symbolnamen sind ohne sie nicht prüfbar), und A ist zugleich der einzige
   Strang, der den davon roten Test heilt (3.3/3.4). **Strang B fasst
   `tests/CMakeLists.txt` nicht an.**
2. **Strang B schreibt seine Zusicherungen umgebungsunabhängig.** Das ist keine
   Erschwernis, sondern das, was #57 AK 3 ohnehin verlangt: *gemessen wird der
   Rollwert vor und nach der Eingabe*, nicht eine absolute Bildkoordinate. Die
   Vorher-Werte des Reviewers (387 px, 35 px) wurden **mit**
   `QT_QPA_PLATFORMTHEME=kde` genommen; wer sie als absolute Zahlen zusichert,
   baut eine Zeitbombe, die zündet, sobald A seine Zeile setzt.
   **Prüfmittel, für beide Stränge vor der Übergabe:**
   ```
   QT_QPA_PLATFORM=offscreen                         ./build/bin/librarytest
   QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde ./build/bin/librarytest
   ```
   Beide Läufe müssen grün sein — bei A nach der Heilung, bei B in jedem Zustand.
   Heute liefern sie 102/102 und 101/102 (3.3), der Unterschied ist genau A's
   Arbeit.
3. **Merge-Reihenfolge: A vor B.** A ändert die Testumgebung und den
   gemeinsamen Testhelfer `answerNextDialog`; B ändert nur Bereiche, die A nicht
   anfasst. Landet A zuerst, rebased B auf einen Stand, dessen Umgebung
   feststeht, und fährt beide Läufe aus Punkt 2 ein letztes Mal. Umgekehrt müsste
   A nach dem Rebase B's neue Zusicherungen unter der geänderten Umgebung
   nachprüfen — dieselbe Arbeit, nur beim größeren Strang. **Gemerged wird
   ausschließlich vom PO** (B13); ein Strang, der `main` braucht, rebased.

## 6. UI-Story-Einstufung (Vorschlag an den PO)

Welche Stories UI-Stories im Sinne von DoD 3 sind, legt der PO beim Planning
fest (Kundenentscheidung 31.07.2026). Der UX-Vorschlag liegt vor; ich schließe
mich ihm an und begründe die eine Abweichung vom Regelfall.

| Issue | UI-Story? | Prüfmittel nach DoD 3 | Grund |
|---|---|---|---|
| **#66** | **ja, mit Bildpflicht** | Eigene Bilder von `denkzettel-ux` aus dem Sprint-Stand **plus** ein Bild am echten Plasma | Der Dialog ist ein **Zustand**: „Bei Zuständen ist das Bild der Prüfgegenstand, nicht die Zusicherung" (B3). Und der Gegenstand der Story ist gerade, dass die Zusicherung offscreen etwas anderes misst als der Nutzer sieht (3.4) |
| **#67** | **ja, mit Bildpflicht** | Eigene Bilder je Zustand: Lesekopf, Bearbeiten-Fußzeile, Lösch-Meldung mit „Rückgängig" | Vier beschriftete Bedienflächen, alle sichtbar; `icon().name()` prüft die Absicht, das Bild prüft das Ergebnis |
| **#58** | **ja, mit Bildpflicht** | Bilder vor/nach dem Schemawechsel aus **einem** Lauf, hell und dunkel | Das AK verlangt es bereits; `tests/libraryshots.cpp` kann es liefern (K3) |
| **#57** | **ja, aber Prüfmittel-Ersatz** | **Rollwert vor und nach der Eingabe** statt Bildvergleich, gefahren über die Szenen `n11a`/`n11b` und `n2` des UI-Reviewers | „Bei Bewegungen ist der Weg der Prüfgegenstand, nicht das Ziel" (B3). **Ein Bild zeigt hier nichts**: Beide Zustände sehen richtig aus, falsch ist die Bewegung dazwischen — das AK sagt es selbst |

**Zur Form des Ersatzes bei #57:** Das ist kein Nachlass, sondern dasselbe
Muster wie bei #44 und #60, wo `QWidget::grab()` das Menü prinzipbedingt nicht
fasste und drei benannte Ersatzmittel an seine Stelle traten. Der Ersatz gehört
**in den Review-Auftrag und in den Bericht**, damit „geprüft" nicht später als
„ohne Bild geprüft" gelesen wird.

## 7. Risiken, die diesen Schnitt kippen

1. **Neue Zusicherungen unter dem Plattformthema.** F2 ist für den *heutigen*
   Bestand gemessen und entkräftet (3.3) — für die Zusicherungen, die beide
   Stränge erst schreiben, gilt es weiter. **Wer räumt auf, wenn `librarytest`
   rot wird?** Antwort, vorab festgelegt: **Wer die rote Zusicherung geschrieben
   hat.** Ist es eine A-Zusicherung, heilt A; ist es eine B-Zusicherung, die
   unter A's Umgebung fällt, heilt B (Festlegung 5.3.2 macht das prüfbar, bevor
   übergeben wird). **Ist ein *bestehender* Test rot, der keinem der beiden
   gehört, ist es ein Impediment an den PO — kein Strang heilt fremde
   Zusicherungen** (melden, nicht heilen). Der einzige heute bekannte Fall
   dieser Art ist `namesTheThreeAnswersOfTheGuardDialog`, und er gehört A.
2. **Die 3×3-Matrix kann ihre Bedeutung verlieren.** `KMessageDialog` ordnet
   nach `Yes`/`No`/`Reject`; `asksBeforeUnsavedChangesAreLost` prüft heute
   `AcceptRole`/`DestructiveRole`/`RejectRole`. Die Umschreibung auf
   Bedeutungsebene (#66 AK 2) ist die riskanteste Zeile des Sprints: Eine
   Matrix, die nach dem Umbau grün ist, aber die Zuordnung Knopf→Handlung nicht
   mehr prüft, ist ein grüner Test, der nichts prüft — vier davon hat dieses
   Projekt an einem Abend entlarvt (`CLAUDE.md`, Prüfhaltung). **Empfehlung: Der
   karpathy-Review bekommt diese Stelle ausdrücklich als Prüfauftrag.**
3. **#57 ist eine Regelsuche und kann steckenbleiben.** Zwei Datenpunkte mit
   vertauschten Vorzeichen, die naheliegende Regel belegt falsch (2.1). Es gilt
   die Loop-Disziplin unverändert: **zweimal derselbe Fehlschlag ohne neue
   Evidenz → Stopp und Impediment**, kein dritter Anlauf. Der Strang misst an
   **beiden** Fällen zugleich; eine Lösung, die nur einen trifft, wird nicht
   übergeben.
4. **#67 hängt an einem laufenden Gestaltungsauftrag.** Die Zeichnungen 2a/2b
   sollen um die Symbole ergänzt werden (AK 4). Das ist **Startbedingung**, wie
   Wireframe 1e es in Sprint 4 für Strang B war (dort K6) — und es ist
   UX-Arbeit, nicht Dev-Arbeit. Der PO taktet.
5. **Der `/usr`-Takt.** Alle vier Stories brauchen den installierten Stand:
   #66/#67 für den Bildnachweis am echten Plasma, #58 für den Schemawechsel bei
   laufendem Dienst, #57 für den Hauptweg. **Kein Strang installiert selbst**
   (DoD 2, Präzisierung nach Sprint-3-Mangel M1); der PO taktet, und der Endstand
   wird am Sprint-Ende einmal installiert und geprüft.
6. **Story-Grenze ohne Luft** (1). Jeder Zugang ist eine Grenzüberschreitung.
   Erfahrungswert: Sprint 4 hat mitten im Sprint eine Scope-Entscheidung
   gebraucht (§14), Sprint 3 endete bei fünf Issues.
7. **Zwei Stränge, ein Fenster.** Das ist neu: In Sprint 4 waren die Flächen
   disjunkt (`src/ui/*` gegen `src/shell/*`), hier teilen sie sich zwei Dateien.
   Die Bereichsdisziplin aus 5.2 ist deshalb keine Formalie, sondern die
   tragende Annahme des Schnitts. **Fällt sie, fällt die Empfehlung „zwei
   Agenten"** — dann ist ein Strang nacheinander richtig.

## 8. Klärungspunkte vor dem Ziehen — mit Vorschlag

Die AK-Anpassungen macht der PO; der Scrum Master legt den Wortlaut vor.

**K1 — #66 AK 3: die Testumgebungsfrage ist beantwortbar, bevor der Sprint
beginnt.**
Das AK verlangt, dass der Dialogtest den Dialog misst, den der Nutzer sieht —
oder ausdrücklich benennt, warum nicht.
*Vorschlag:* Das AK auf den gemessenen Weg festlegen: `librarytest` bekommt
`QT_QPA_PLATFORMTHEME=kde`; damit ist der O3-Befund **offscreen als roter Test
reproduzierbar** (3.4, drei leere Symbolnamen). Zugleich in das AK aufnehmen,
was die Messung **nicht** hergibt: dass offscreen+`kde` keine Plasma-Sitzung
ersetzt — der Bildnachweis am echten Plasma bleibt.

**K2 — #67 AK 4: Startbedingung Gestaltungsauftrag.**
Die Zeichnungen 2a/2b sollen die Symbole tragen, bevor der Dev sie umsetzt.
*Vorschlag:* Wie bei Wireframe 1e in Sprint 4 als **Startbedingung für Strang A**
führen und im Spawn-Auftrag benennen. Ist die Zeichnung zum Spawn-Zeitpunkt
nicht fertig, beginnt A mit #66 und zieht #67 nach.

**K3 — #58 AK 2: Wer erzeugt die Bilder hell/dunkel aus einem Dienstlauf?**
*Vorschlag:* `tests/libraryshots.cpp` erweitern — der Bildläufer zeichnet bereits
die gruppierte Liste mit den Zeitstempeln, steht **nicht** in `add_test()` und
braucht keine CMake-Änderung. Den Schemawechsel im Lauf nach dem Vorbild von
`CaptureTest::textsFollowAColourSchemeChange()` (`tests/capturetest.cpp:177 ff.`)
setzen: Palette tauschen, `processEvents()`, zweites Bild. Damit ist „aus **einem**
Dienstlauf" wörtlich erfüllt. Der Kundenblick in der Abnahme kommt hinzu, ersetzt
den Beleg aber nicht.

**K4 — #57: UI-Story mit Prüfmittel-Ersatz.**
*Vorschlag:* Wie in 6 vorgeschlagen festlegen und den Ersatz in den
UI-Review-Auftrag schreiben. Ohne diese Festlegung prüft `denkzettel-ux` gegen
Bilder, in denen der Fehler nicht sichtbar sein **kann** — ein Testaufbau, in
dem der Fehler gar nicht auftreten kann, ist kein Test (`CLAUDE.md`).

**K5 — #57: Umkehrung eines bestehenden Tests.**
`bringsTheHeadAlongWhenAVisibleNoteOfAnotherGroupIsClicked` sichert heute das
Verhalten zu, das die Story beseitigt (3.5).
*Vorschlag:* In das Issue aufnehmen, dass die Umkehrung **Teil der Story** ist
und die Entscheidungsspur nicht verlorengeht: den alten Kommentar nicht löschen,
sondern die überholte Fassung benennen — die Gattung, die dieses Projekt sich
selbst gegeben hat (`646804a`, `95bcab6`, Sprint 4 §18).

**K6 — #59 als Beilage zu Strang B.**
*Vorschlag:* Kein AK, kein Scope. Ein Satz im Spawn-Auftrag: *„#59 liegt bei;
deine Regel für #57 darf seine Heilung nicht verbauen."* Damit ist die Empfehlung
des Reviewers („zusammen entscheiden") erfüllt, ohne die Story-Grenze zu reißen.

**K7 — Basis-Tag.**
Vor dem ersten Strang-Spawn setzt der PO `sprint-05-basis` auf den
Ausgangsstand (`PROZESS.md`, Sprint-Mechanik). In Sprint 4 hat der Tag den
Prüf-Diff des Sprint-Endes getragen; er bleibt auch nach dem Ende des
PR-Probelaufs (9).

## 9. Prozess-Nachzug: der PR-Probelauf ist beendet (V2)

Der offene Punkt **V2** aus Sprint 4 (§17.5, §18) ist damit geschlossen, und
`PROZESS.md` ist an **einer** Stelle nachgezogen.

**Sachstand, nicht neu bewertet:** Die DoD-Prüfung von Sprint 4 (§15.8) hat den
Probelauf gegen sein **vorab festgelegtes** Kriterium geprüft und als **nicht
bestanden** befunden — kein automatischer Testlauf auf einem PR
(`statusCheckRollup` leer für #64 und #65), und der einzige `fail`-Befund des
Sprints hing an Zeilen aus einem einzigen Commit, war also über `git show`
ebenso auffindbar. Die Vorab-Regel („trifft keines von beidem zu, endet der
Probelauf") war eine Kundenentscheidung vom Vormittag des 02.08.2026. Das
Ergebnis ist dem Kunden zweimal vorgelegt worden (§15.8, §17.5) und blieb
unwidersprochen.

**Was geändert wurde:** Der auf Sprint 4 befristete Absatz trägt jetzt sein
**Ergebnis** statt einer abgelaufenen Anweisung. Eine befristete Regel, deren
Frist verstrichen ist, ist eine tote Regel — sie wird gelesen und befolgt, weil
niemand die Fußnote sieht.

**Was gilt, ist der Stand davor:** Zweige und Worktrees nach B13, gemerged
ausschließlich vom PO, kein PR-Zwang. **Basis-Tag und Review-Kette bleiben
unverändert.** Aus dem Probelauf bleibt eine Zeile: das Räumen der Zweige
**auch auf `origin`** (Sprint-Abschluss, Punkt 8) — sie hängt am öffentlichen
Repo und nicht am Pull Request, und gilt weiter, sooft ein Story-Zweig überhaupt
auf `origin` gelangt.

**Nicht verschwiegen:** Der Probelauf ist nur zur Hälfte durchgeführt worden —
an beiden PRs stand weder ein Dev-Bericht noch ein Review-Befund (§15.8,
Beobachtung 1). Bewertet wurde er trotzdem an seinem eigenen Kriterium, und
dieses Kriterium trifft die Mechanik, die tatsächlich lief. **Eine spätere
Einführung automatischer Testläufe (CI) ist davon unberührt und wäre eine neue
Kundenentscheidung** — sie ist der einzige der beiden Kriteriumsteile, der ohne
menschliche Disziplin trägt.

## 10. Hinweise an den Product Owner

1. **Der Schnitt geht dem Kunden zur Freigabe** (Freigabemodell, Kundenentscheidung
   31.07.2026). Vorzulegen sind Sprint-Ziel (4.2) und die vier Issues (4.3), dazu
   **ausdrücklich der Kontostand**: vier von vier Issues, keine Luft (1).
2. **Sechs AK-Anpassungen** liegen als Wortlaut in 8 (K1–K6). K1 und K3 ändern die
   Prüfbarkeit, K2 und K5 die Reihenfolge der Arbeit.
3. **UI-Story-Einstufung ist PO-Entscheidung** (6) — sie muss **vor** dem Spawn
   fallen, sonst bauen die Stränge ohne Bildpflicht.
4. **`sprint-05-basis` setzen** (K7), bevor der erste Strang läuft.
5. **Milestone „Sprint 5"** anlegen und die vier Issues zuordnen; die Labels
   `sp:*` an #57 (2 → 3) nachziehen und an #66/#67 erstmals setzen (5 und 2).
6. **Der `/usr`-Takt gehört in beide Spawn-Aufträge** als Verbot mit Begründung,
   nicht als Hinweis (Risiko 5).

## 11. Was dem Kunden zur Entscheidung vorliegt

1. **Sprint-Ziel und Schnitt** — vier Stories, 11 SP: die Symbole am
   Nachfragedialog (#66) und an den Bibliotheks-Schaltflächen (#67), die Liste,
   die unter dem Zeiger stehenbleibt (#57), und Zeitstempel, die dem Farbschema
   folgen (#58).
2. **Eine Grenze am Anschlag:** vier von vier zulässigen Issues. Jeder Nachtrag
   im laufenden Sprint kommt als Grenzüberschreitung zurück auf den Tisch.
   Alternative, falls das stört: drei Issues (ohne #58, 10 SP) — dann bliebe
   allerdings eine Kopie eines bereits abgenommenen Fehlers stehen.
3. **#59 (Scrollstelle) bleibt draußen**, obwohl es zum selben Thema gehört. Als
   fünftes Issue risse es die Grenze. Der Strang, der #57 baut, kennt es und
   verbaut seine Heilung nicht.
4. **#55 (Fensterhülle, 8 SP) ist der Sprint-6-Kandidat** — die Kriterien wurden
   heute neu gefasst, der Gestaltungsauftrag läuft noch.
5. **Der PR-Probelauf ist beendet** (9), wie es die eigene Vorab-Regel des
   Kunden vorsieht. Automatische Testläufe (CI) blieben eine eigene Entscheidung.

## 12. done / next

**done:** Sprint-5-Planning moderiert und vorgelegt — sechs Schätzpaare nach der
Stufenregel konsolidiert (in keinem Fall war eine begründete Entscheidung des
Scrum Masters nötig; #57 ersetzt sein Alt-Label mit Begründung), das Sprint-Konto
ab Zeile 1 geführt und die **Story-Grenze am Anschlag** als ersten Befund
ausgewiesen; die Kollisionsfläche beider Stränge am Code vermessen (zwei
gemeinsame Dateien, kleinster Bereichsabstand 38 Zeilen) und mit drei
Festlegungen versehen; das benannte Risiko F2 **selbst gefahren statt abgewogen**
— `QT_QPA_PLATFORMTHEME=kde` an `librarytest` lässt heute 101 von 102 Tests grün
und verschiebt **keinen** Geometriewert, und der eine rote Test ist die
offscreen-Reproduktion des Kundenbefunds aus #66 (drei leere Symbolnamen);
festgestellt, dass `bringsTheHeadAlongWhenAVisibleNoteOfAnotherGroupIsClicked`
das Verhalten zusichert, das #57 beseitigt, und deshalb umzukehren ist; die
Dateimengen-Notation nach karpathy-Befund 3.1 um **Beleg- und Prüfmittelpfade**
erweitert; UI-Story-Einstufung für vier Issues vorgeschlagen, davon eine mit
begründetem Prüfmittel-Ersatz; sieben Risiken und sieben Klärungspunkte mit
Wortlautvorschlag benannt; **V2 geschlossen** — der befristete PR-Absatz in
`PROZESS.md` trägt jetzt sein Ergebnis statt einer abgelaufenen Anweisung.

**next:** (1) PO legt die UI-Story-Einstufung fest und arbeitet K1–K6 in die
Issues ein. (2) PO legt Sprint-Ziel und Schnitt dem Kunden zur Freigabe vor,
**mit dem Kontostand**. (3) Nach der Freigabe: Milestone „Sprint 5", Labels,
`sprint-05-basis`, dann Spawn von zwei Strängen nach 5.2 — **A vor B im Merge**.
(4) Freigabe-Stand in die Kontotabelle (1) eintragen. (5) Vor dem Sprint-6-Planning
zu heilen: zweite Schätzung und drei Klärungen zu #69 (2.4). (6) Retro nach
Sprint 6 mit den vier Kandidaten aus Sprint 4, 17.6 — der B13-Nachzug (5.1) ist
dort einzutragen.
