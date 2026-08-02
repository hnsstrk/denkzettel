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

### 2.4 Was nicht geschätzt werden konnte

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

## 12.1 Kundenfreigabe (PO-Vermerk, 02.08.2026, 16:25)

Der Kunde hat den Schnitt freigegeben — **11 SP / 4 Stories in zwei
Strängen** (A: #66+#67, B: #57+#58), Sprint-Ziel wie in Abschnitt 1.
Vorgelegt mit vollständiger Inhaltsangabe in der Frage selbst (Lehre aus
zwei Fällen heute: Erklärtext vor einem Fragedialog erreicht den Kunden
nicht zuverlässig — entscheidungsrelevanter Inhalt gehört in die Frage).
Die Startbedingung K2 (Zeichnungen 2a/2b mit Symbolen) war zur Freigabe
erfüllt (`711d899`); K1/K5/K6 in den Issues vermerkt.

**Sprint-Konto bei Freigabe: 11 von ~13 SP · 4 von 4 Stories** — die
Story-Grenze ist ausgeschöpft, jeder Zugang ist eine Grenzüberschreitung
und wird dem Kunden als solche vorgelegt.

---

# Sprint 5 — DoD-Prüfung, Takt 1 (vor der Kundenabnahme)

**Datum:** 02.08.2026, 18:11 (Ganymed) · **Prüfer:** Scrum Master (Agent
`scrum-master`, frischer Kontext)
**Prüfgegenstand:** `main` @ `7247500` — beauftragt war `43abc06`; der PO hat
während der Prüfung den UI-Review-Befund W1 nachgezogen (`7247500`, nur
`wireframes/…dc.html`). **Kein Commit seit `1adffd3` fasst `src/` oder `tests/`
an**, der Prüfgegenstand für Code und Tests ist also unverändert; der
Zuwachs wird unter DoD 3 gewürdigt.
**Installierter Stand:** `/usr/bin/denkzetteld`, Dienst PID 460335 (seit
02.08.2026 18:02:36).

**Grundlage:** `PROZESS.md`, DoD 1–6 und „Sprint-Abschluss", Takt 1
(Punkte 1–4). DoD 5 und DoD 6 sind **nicht** Gegenstand dieses Takts — sie
sind vor der Abnahme nicht erfüllbar (Sprint-3-Mängel M2/M5).

**Prüfhaltung:** Jede Zeile unten steht auf einem Befehl, den ich selbst
gefahren habe. Wo ich einen fremden Bericht nur bestätige, steht es dabei; wo
ich eine Behauptung nicht selbst nachmessen konnte, steht das ebenfalls.

## 13. Was ich selbst gemessen habe

Out-of-source gebaut, um den Arbeitsbaum des PO nicht anzufassen
(`cmake -B <scratch>/build-sm -S . -DCMAKE_BUILD_TYPE=Debug`).

| Messung | Befehl | Ergebnis |
|---|---|---|
| Bau am Endstand | `cmake --build … -j 12` | rc=0, **0 Warnungen** im ganzen Protokoll |
| Testsatz | `ctest --output-on-failure` | **7/7 passed**, 5,81 s |
| `librarytest` **ohne** Plattformthema | `QT_QPA_PLATFORM=offscreen` | **107 passed, 0 failed** |
| `librarytest` **mit** Plattformthema | `+ QT_QPA_PLATFORMTHEME=kde` | **107 passed, 0 failed** |
| `lint-tidy` | `cmake --build … --target lint-tidy` | rc=0, drei Befunde auf `librarywindow` |
| `lint-clazy` | `cmake --build … --target lint-clazy` | zwei Befunde, **keiner** auf `librarywindow` |

**Zu den Linterbefunden — alle Altbestand, keiner aus diesem Sprint.** Ich habe
sie nicht geglaubt, sondern zugeordnet (`git blame` + `git merge-base
--is-ancestor <commit> sprint-05-basis`):

| Befund | Herkunft | Einordnung |
|---|---|---|
| `librarywindow.cpp:134` `bugprone-easily-swappable-parameters` | `9ddd64af` (01.08.) | vor Sprint 5 |
| `librarywindow.h:72` `performance-enum-size` (`Selection`) | `54ae35d1` (01.08.) | vor Sprint 5 |
| `librarywindow.h:80` `performance-enum-size` (`UnsavedAnswer`) | `4740e5d4` (02.08.) | vor `sprint-05-basis` |
| `librarytest.cpp:2336`/`:2342` `range-loop-detach` | `54249e0b` (01.08.) | vor Sprint 5 |

Damit ist die Aussage von Strang A („keine neuen Linterbefunde, was steht,
ist Altbestand") **unabhängig bestätigt** — nicht nachgelesen.

## 14. DoD 1 — Bau, Tests, Geometrie-Zusicherungen

**Ergebnis: erfüllt.**

- **Warnungsarm:** 0 Warnungen (13). Keine Linterbefunde aus diesem Sprint.
- **Neue Logik hat Tests:** 102 → **107**. Fünf netto neue Zusicherungen, eine
  Umkehrung. Gezählt am Diff, nicht am Bericht:
  `leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked` (ersetzt
  `bringsTheHeadAlongWhenAVisibleNoteOfAnotherGroupIsClicked`),
  `keepsTheHeadFetchAfterAClickThatSelectedNothing`,
  `textsFollowAColourSchemeChange`, `showsTheWarningSymbolInTheGuardDialog`,
  `namesTheSymbolsOfTheDetailButtons`, `namesTheSymbolOfTheUndoAction`.
  102 + 6 − 1 = 107 — die Rechnung geht auf.
- **Beide Umgebungen grün.** Das ist der Punkt, den Planning 5.3.2 als
  Prüfmittel festgelegt hat: Am Ausgangsstand stand es 102/0 und 101/1, heute
  107/0 und 107/0. Der Unterschied ist genau die Arbeit von Strang A.

**Geometrie-Zusicherungen, bei zwei Fenstergrößen — nachgezählt, nicht
übernommen.** Drei Familien, jede mit `_data()` und **je zwei** Größen
(900×600 und 1200×800), belegt in `tests/librarytest.cpp:2165 ff.`,
`:2223 ff.`, `:3279 ff.`:

| Zusicherung | Wireframe | Größen |
|---|---|---|
| `keepsTheHeaderAtTheTopAndTheRestForTheNotes` | 2b (Kopfleiste, Restraum) | 900×600 · 1200×800 |
| `keepsTheMeasuresOfTheGroupedList` | 3a (Listenmaße) | 900×600 · 1200×800 |
| `keepsTheMeasuresOfTheEditState` | 2a Zustand B (Bearbeiten) | 900×600 · 1200×800 |

**Sprint 5 fügt keine Ansicht mit eigener Raumaufteilung hinzu**, also fällt
keine neue Geometriefamilie an. Der einzige neue gezeichnete Gegenstand ist der
Wächterdialog (2a Zustand C) — und die Zeichnung sichert dort ausdrücklich
**keine** Maße und **keine** Reihenfolge zu, sondern Rollen und Wirkung
(Annotation „Anordnung der Schaltflächen ist **nicht** durch diese Zeichnung
festgelegt"). Das ist auch sachlich richtig: Das Innenleben eines
`KMessageDialog` gehört dem Framework, nicht dieser Anwendung. Gemessen wird
deshalb, was die Zeichnung wirklich behauptet — dass ein Warnsymbol da ist
(`showsTheWarningSymbolInTheGuardDialog`, misst das Bildetikett) und dass die
Antworten ihre Symbole und ihre Bedeutung tragen.

## 15. DoD 2 — Akzeptanzkriterien am Stand, installierter Stand

**Ergebnis: erfüllt bis auf einen Punkt — der Hauptweg keiner der vier Stories
ist am installierten Stand belegt (Mangel M1).**

### 15.1 Der installierte Stand ist der Endstand — von mir nachgewiesen

Der Installationsbericht behauptet Binärgleichheit; ich habe sie nachgemessen
und um den Teil ergänzt, den er offenlässt (dass das Abbild wirklich die
Sprint-5-Arbeit trägt):

| Prüfung | Befehl | Ergebnis |
|---|---|---|
| Binärgleichheit | `md5sum /usr/bin/denkzetteld build/bin/denkzetteld` | identisch, `228bbbbd2b839ab91b2d640006baa652` |
| Aktualität | `stat` gegen `git log -1 1adffd3` | Abbild gebaut 17:39:22, letzter Quell-Commit 17:39:15 — **7 s danach** |
| Kein Nachzug seither | `git diff --name-only 43abc06..HEAD` | nur `wireframes/…dc.html` |
| Sprint-5-Arbeit im Abbild | `strings -el /usr/bin/denkzetteld` | `document-edit`, `edit-undo`, `dialog-warning` vorhanden; `KMessageDialog` und `WarningTwoActionsCancel` als Typinformation vorhanden |
| Dienst läuft daraus | `ps -p 460335` | `/usr/bin/denkzetteld`, gestartet 18:02:36 |
| Dienst antwortet | `gdbus introspect --dest org.denkzettel.Daemon` | `ShowCapture`, `AddNote`, `ShowLibrary`, `Quit` |

*Zur Vollständigkeit der Symbolprobe:* `edit-delete`, `document-save` und
`dialog-cancel` stehen **nicht** im Abbild, und das ist richtig so — sie kommen
aus `KStandardGuiItem::del()/save()/cancel()` und werden erst zur Laufzeit vom
Framework geliefert (`librarywindow.cpp:368`, `:441`, `:444`, `:995–997`). Nur
die drei wörtlich gesetzten Namen können im Abbild stehen, und sie stehen darin.

### 15.2 Was daran **nicht** geführt ist

`docs/scrum/reviews/sprint-05-installationstakt.md` belegt Installation,
Binärgleichheit und Dienstwechsel — **aber keinen einzigen Story-Hauptweg am
installierten Stand.** Alle vier stehen unter „Offen für die Kundenabnahme".
Takt 1, Punkt 1 verlangt beides: installieren *und* den Hauptweg jeder Story
daran ausführen, mit Belegform (Terminalausgabe, Journalauszug oder Bild).
→ **M1.**

Der Maßstab liegt im eigenen Haus: Der Sprint-4-Takt
(`sprint-04-installationstakt.md`, Punkte 5 und 6) hat für #60 das Menü am
installierten Dienst per `GetLayout` und die Kürzelkette gemessen; nur der
Bearbeiten-Hauptweg ging an den Kunden. Hier geht alles an den Kunden.

### 15.3 Selbstprüfung der Stränge vor der Übergabe — geführt

DoD 2 verlangt vom Entwickler den selbst gestarteten Stand *vor* der Übergabe.
Beide Stränge haben sie geführt und belegt, jeder am **gebauten** Stand (die
Installation war ihnen untersagt, Planning Risiko 5):

| Strang | Nachweis | Form |
|---|---|---|
| A (#66/#67) | Bericht §5 „Selbstprüfung am gebauten Stand (DoD 2)", `bildlauf-messwerte.txt`, fünf Bilder | Messwerte + Bild |
| B (#57/#58) | `selbststart.txt` — eigener Sitzungsbus (`dbus-run-session`), eigene XDG-Pfade, `ShowLibrary`/`ShowCapture`/`Quit` über D-Bus, Journalauszug zu PID 403740 | Terminalausgabe + Journal |

Strang B hat dabei die Sitzung des Kunden ausdrücklich nicht angefasst — die
richtige Auslegung des `/usr`-Verbots.

### 15.4 Akzeptanzkriterien je Story, am Stand nachgemessen

Stichproben von mir selbst gefahren, nicht aus den Berichten übernommen.

**#66 — Wächterdialog auf KDE-Bauart**

| AK | Befund | Beleg |
|---|---|---|
| Drei Knöpfe tragen unter echtem Plasma die Symbole (Foto) | **offen — Kundensache**, so im AK selbst festgelegt | — |
| Drei Antworten behalten ihre Bedeutung; Matrix auf Bedeutungsebene | erfüllt | UI-Review §2 (Abbrechen endet wieder im Bearbeiten-Zustand); karpathy Prinzip 4, Mutation 4 |
| Testumgebung festgelegt (K1) | erfüllt | `tests/CMakeLists.txt:52–53` setzt `QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=kde` — selbst gelesen |
| SPEC 9 nachgezogen | erfüllt | siehe 17 |

Eigene Bildprüfung an `sprint-05-ui-review/bilder/s5-edit-03-waechterdialog.png`
(ich habe das Bild angesehen, nicht seine Beschreibung gelesen): orangefarbenes
Warndreieck links, „Änderungen speichern?" als erste Zeile, Leerzeile,
Erläuterung mit dem Zeitstempel in Klammern; drei Knöpfe mit Symbol —
*Speichern* (Diskette, mit Fokusrahmen als Vorgabe), *Verwerfen* (roter
Papierkorb), *Abbrechen* (durchgestrichener Kreis).

**#67 — Symbole an den Bibliotheks-Schaltflächen**

Sechs Stellen, fünf Namen. Ich habe die Quelle jeder einzelnen gelesen
(`librarywindow.cpp:173`, `:361`, `:368`, `:441`, `:444`, `:995–997`) und das
Bild `s5-edit-01-lesen.png` selbst angesehen: „Bearbeiten" mit Stift,
„Löschen" mit rotem Papierkorb, beide mit Text daneben. Alle sechs Stellen sind
im UI-Review namentlich gegen die Tafel gehalten — Verdikt **ok**. Der
Bildnachweis am echten Plasma ist auch hier Kundensache.

**#57 — Klick auf eine sichtbare Notiz lässt das Bild springen**

| AK | Befund | Beleg |
|---|---|---|
| Klick auf eine vollständig sichtbare Zeile bewegt das Bild nicht | erfüllt | `leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked` (`:1481`); UI-Review §4.1: Rollwert bleibt 6, der 387-px-Sprung ist weg |
| AK-7-Heilung unangetastet | erfüllt | `bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready` (`:1424`) steht unverändert — selbst nachgesehen |
| Gemessen wird der Rollwert vorher/nachher, nicht der Endzustand | erfüllt | UI-Review fuhr das S8-Szenenprogramm **unverändert** (md5 geprüft) gegen `main`; `diff` gegen den Nachher-Lauf des Strangs: kein Unterschied |

**#58 — Zeitstempel und Hinweise am Schemawechsel**

| AK | Befund | Beleg |
|---|---|---|
| Nach dem Schemawechsel bei laufendem Dienst folgen die Texte | erfüllt | `textsFollowAColourSchemeChange`; UI-Review §3.2 misst `#707d8a` → `#a1a9b1` |
| Bilder vor/nach aus **einem** Lauf, hell und dunkel | erfüllt | `s5-58-a`…`-e`, dazu die Gegenbilder vom Basisstand |
| Test in der Machart von `CaptureTest::textsFollowAColourSchemeChange()` | erfüllt | gleichnamiger Test in `librarytest.cpp` |
| **Kein weiteres Vorkommen der Bauart in `src/ui/`** | erfüllt — **selbst gefahren** | `grep -rn "setPalette" src/` → **kein Treffer**; verblieben sind drei `setForegroundRole`-Aufrufe (`capturewindow.cpp:36`, `librarywindow.cpp:125`, `:353`) |

Der stärkste Beleg dieses Sprints steht im UI-Review §3.2 und ist keiner der
beauftragten: `QPalette::PlaceholderText` auf `#ff00ff` gesetzt — eine Farbe,
die in keinem Schema vorkommt. Am geheilten Stand werden alle betroffenen
Stellen magenta, am Basisstand bleiben sie grau. Das trennt „folgt der Rolle"
von „sieht zufällig richtig aus", und genau diese Trennung ist der Kern von
#58.

## 16. DoD 3 — Prüfläufe, Berichte, Vollzähligkeit

**Ergebnis: erfüllt.** Kein offener `fail` in beiden Ketten.

### 16.1 Die Berichte liegen als Datei vor, vor dieser Prüfung (Takt 1, Punkt 2)

| Lauf | Bericht | Verdikt |
|---|---|---|
| karpathy-Review (Sprint-Diff) | `docs/scrum/reviews/sprint-05-karpathy.md` | **warn**, kein `fail` |
| UI-Review (`denkzettel-ux`) | `docs/scrum/reviews/sprint-05-ui-review/bericht.md` + 20 eigene Bilder | **abnahmefähig**, drei `warn` |
| Strang A (#66/#67) | `docs/scrum/reviews/sprint-05-s-symbole/bericht.md` + Bilder, sechs Läufe | — |
| Strang B (#57/#58) | `docs/scrum/reviews/sprint-05-s-verhalten/bericht.md` + Bilder, sieben Messdateien | — |
| Installationstakt (PO) | `docs/scrum/reviews/sprint-05-installationstakt.md` | siehe M1 |

**Vollzähligkeit nach dem Prüfweg aus B11.2** (Commit-Botschaften, die Befunde
eines Prüflaufs nennen, gegen die abgelegten Berichte): `7247500`, `a44784b`,
`4cd37c2`, `fe0c83b`, `8a8c652`, `afe55e5`, `3e6f562` durchgesehen. **Jeder
genannte Befund hat seinen Bericht.** Der eine Grenzfall ist der
UX-Zwischenlauf zum Warnsymbol (`8a8c652`, „UX-Votum mit Messbefund") — er hat
keine eigene Berichtsdatei, aber sein Messbefund ist versioniert: im Strang-A-
Bericht §9 (samt der von der UX gemessenen 64×64) und, dauerhafter, als
Bedingung in SPEC 9. **Kein Mangel**, sondern der Fall, für den B11.2 gedacht
ist — er ist eingetreten und die Substanz ist im Repo gelandet.

### 16.2 Der UI-Review ist mit eigenem Bild geführt (DoD 3, B3)

Das ist der Punkt, an dem dieses Projekt schon gescheitert ist, deshalb
ausdrücklich: `denkzettel-ux` hat **nicht** die Bilder der Stränge bewertet,
sondern einen eigenen Out-of-source-Bau angelegt, die Läufer frisch gebaut
(die Lehre aus dem `EXCLUDE_FROM_ALL`-Vorfall dieses Sprints, `CLAUDE.md`),
ein **eigenes Prüfprogramm** geschrieben (`ux-review-s5.cpp`, versioniert) und
20 eigene Bilder erzeugt. Zusätzlich hat er denselben Lauf gegen den
Basisstand `a322b86` gefahren — eine Gegenprobe, die kein AK verlangt hat.
Für #57 kam der beim Planning festgelegte **Prüfmittel-Ersatz** zum Tragen
(Rollwert statt Bildvergleich, K4) und er steht als Ersatz im Bericht, nicht
als stillschweigende Auslassung.

### 16.3 Die Befunde und ihr Verbleib — alle abgearbeitet

| Befund | Art | Verbleib | von mir geprüft |
|---|---|---|---|
| karpathy 1: Wireframe-Satz „`librarytest` setzt es heute nicht" | warn | `4cd37c2` — steht jetzt „setzt es seit Sprint 5 (`tests/CMakeLists.txt`); wer den Lauf dort wieder herausnimmt, macht den Prüfsatz still unwirksam" | Wortlaut selbst gelesen |
| karpathy 2: `SPEC.md:597` KWidgetsAddons-Klammer unvollständig | warn | `fe0c83b` — Klammer nennt jetzt „KMessageDialog samt KStandardGuiItem" | `SPEC.md:596–598` selbst gelesen |
| karpathy 3: N2-Fall als Issue anlegen | warn | **#70** angelegt | `gh issue view 70` |
| UI-Review W1: Zeichnung 2a C trägt überholten Dialogtext | warn | `7247500` — Text auf den gebauten Wortlaut, dritter Nachtrag begründet die fehlende Größenstufung | Diff selbst gelesen |
| UI-Review B1: #58-Bilder zeigen den Zweigstand ohne #67-Symbole | warn | `a44784b` — Nachtrag in `sprint-05-s-verhalten/bericht.md:201–202` | Zeilen selbst gelesen |
| UI-Review B2: Klick auf angeschnittene Zeile | warn | **#71** angelegt | `gh issue view 71` |
| UI-Review H1: Tooltips mit Kürzel (Empfehlung) | keine | **#72** angelegt | `gh issue view 72` |

**Beobachtung ohne Mangelcharakter:** Die beiden Wireframe-Korrekturen
(`4cd37c2`, `7247500`) sind *nach* den jeweiligen Reviews entstanden und
deshalb selbst ungeprüft. Das ist die Natur einer Auflagenerfüllung und kein
Befund; ich halte es fest, weil der nächste Prüfer sonst annimmt, der
UI-Review habe den heutigen Zeichnungsstand gesehen. Er hat den Stand von
`5a4a83a` gesehen.

**Ebenfalls festgehalten:** Der karpathy-Lauf deckt den `CLAUDE.md`-Absatz aus
`5a4a83a` mit ab (Bericht, Prinzip 3) — die globale Regel „Regel-Änderungen
durchlaufen den Reviewer" ist damit eingehalten, nicht übersprungen.

## 17. DoD 4 — SPEC/KONZEPT nachgezogen, einschließlich entdeckter Bedingungen

**Ergebnis: erfüllt, und zwar in der strengen Fassung nach B9.** Ich habe den
SPEC-Diff Zeile für Zeile gelesen (`git diff sprint-05-basis..HEAD -- SPEC.md`,
+69 Zeilen).

**Die geänderten Festlegungen:**

- **SPEC 9, Abschnitt Wächterdialog:** Die Bauart ist jetzt entschieden und
  benannt — `KMessageDialog` vom Typ `WarningTwoActionsCancel` mit
  `KStandardGuiItem`-Symbolen, Vorgabeantwort „Speichern".
- **SPEC 9, Listenteil:** Der Kopf wird beim Grenzübertritt **per Taste**
  geholt, **nicht per Mausklick** — mit dem gemessenen Wert (387 px) und der
  Begründung im Text.
- **SPEC 15:** `KMessageDialog` samt `KStandardGuiItem` in der
  KWidgetsAddons-Klammer; Teststrategie um die Plattformthema-Bedingung
  ergänzt.

**Die entdeckten Bedingungen — der Punkt, an dem B9 gemessen wird.** Beide vom
PO benannten Bedingungen stehen in SPEC, und drei weitere dazu:

| Bedingung | Fundstelle | Vom PO benannt? |
|---|---|---|
| Die Plattformintegration beantwortet einen gebauten `QMessageBox` mit einem **eigenen Meldungsfenster samt eigenen Knopfobjekten**; ein `KMessageDialog` ist ein gewöhnlicher `QDialog` und bleibt der eigene | SPEC 9, „Bauart des Dialogs" | **ja** |
| `KMessageDialog::setIcon()` sagt zu, bei leerem Symbol eines nach Dialogtyp zu wählen — **gemessen kommt keines**; das Warnsymbol wird ausdrücklich gesetzt | SPEC 9, letzter Spiegelstrich | **ja** (Doku-Falle) |
| `KMessageDialog` kennt **keinen Zweittext**; Frage und Erläuterung in einem Text, durch Leerzeile getrennt | SPEC 9 | nein |
| Antwortrollen sind `Yes`·`No`·`Reject`; **zugesichert ist die Bedeutung**, nicht Rolle und nicht Reihenfolge | SPEC 9 | nein |
| Die Vorgabeantwort **folgt dem Fokus** — „Speichern" muss nach dem Anzeigen Fokus *und* Vorgabe erhalten, und eine Zusicherung darüber gilt erst am **sichtbaren** Dialog | SPEC 9 | nein |
| Von Hand angezeigt ist der Dialog **nicht mehr modal** durch ein späteres `exec()` | SPEC 9 | nein |
| Ohne Plattformthema löst `QIcon::fromTheme()` nichts auf und liefert ein Symbol **ohne Namen** — die Zusicherung wäre rot, ohne dass am Bau etwas fehlt | SPEC 15 | nein |

Sieben Bedingungen, alle mit Messdatum. Die dritte bis siebte sind gerade die
Sorte, die sonst als Stammeswissen im Kopf eines Agenten verschwindet. Dass
sie in SPEC stehen, ist die eigentliche Erfüllung von B9 — nicht dass eine
Festlegung geändert wurde.

## 18. Doku-Abgleich nach B10

**Ergebnis: eine Abweichung — M3.** Geprüft wurde `README.md` (22 Zeilen,
ganz gelesen) und `docs/`; `docs/` enthält ausschließlich `scrum/`, also keine
Produktdokumentation, die nachziehen müsste.

- **Statuszeile (`README.md:7`): in Ordnung.** Sie beschreibt den gelieferten
  Stand und **nicht** das Verfahren — kein „Sprint N in der Kundenabnahme".
  Genau daran ist der Abgleich in Sprint 3 gescheitert.
- **Aber sie beschreibt drei der vier gelieferten Verhaltensweisen nicht:**

| Lieferung | im README? |
|---|---|
| Wächterdialog gegen ungespeicherte Änderungen | ja (aus Sprint 4) |
| Symbole an den Bibliotheks-Schaltflächen und im Wächterdialog (#66/#67) | **nein** |
| Die Liste bleibt beim Klick stehen (#57) | **nein** |
| Zeitstempel und Hinweise folgen dem Farbschema (#58) | **nein** |

Der Maßstab ist der eigene: Für das Tray-Menü steht seit Sprint 4 der Satz
„Das Tray-Menü trägt Symbole und deutsche Beschriftungen" im README. Was dort
nennenswert war, ist hier nicht weniger nennenswert. → **M3.**

## 19. Sprint-Konto (B12) — Schlussstand

| Buchung | Issues | Story Points | Grenzen (2–4 · ~13) |
|---|---|---|---|
| Freigabe-Stand (02.08.2026, 16:25) | 4 | 11 | beide gehalten, Story-Grenze **am Anschlag** |
| Zugänge im Sprint | **0** | **0** | — |
| **Schlussstand** | **4** | **11** | **beide gehalten** |

**Selbst geprüft, nicht übernommen:** `gh issue list --milestone "Sprint 5"
--state all` liefert genau #66, #67, #57, #58. Die drei im Sprint entstandenen
Befunde sind als Issues **#70** (N2-Fall), **#71** (angeschnittene Zeile, B2)
und **#72** (Tooltips, H1) angelegt und tragen **keinen Milestone** — sie sind
im Backlog gelandet, nicht im Sprint. `gh api …/milestones` bestätigt:
`Sprint 5: open=4 closed=0`.

**Das ist der Punkt, an dem B12 seine Probe bestanden hat.** Der Sprint startete
mit ausgeschöpfter Story-Grenze; jeder Zugang wäre eine Grenzüberschreitung
gewesen. Drei Befunde, die zum Thema des Sprints gehören und die man leicht
„noch mitgenommen" hätte, sind stattdessen Backlog geworden. In Sprint 3 wäre
das nicht aufgefallen — dort endete der Sprint bei fünf Issues, ohne dass es
jemand vorgelegt hätte.

**Ein Nebenbefund zur Kontoführung, kein eigener Mangel:** Die Kontotabelle in
§1 trägt in der Zeile *Freigabe-Stand* noch den Platzhalter „einzutragen nach
der Kundenentscheidung"; der Stand steht in Prosa in §9 (Kundenfreigabe). Der
eigene next-Punkt (4) des Plannings ist damit offen geblieben. Ich trage ihn
oben nicht nach — das ist der Abschnitt eines anderen Laufs, und die Zahl ist
hier belegt. *(Ebenfalls nur zur Kenntnis: das Protokoll führt die
Abschnittsnummer 9 zweimal.)*

## 20. Changelog — Vorbereitung für Takt 2

Kein Prüfpunkt dieses Takts; die Einträge entstehen nach der Abnahme
(Sprint-Abschluss, Punkt 9). Zwei Feststellungen, die dem PO Arbeit sparen:

1. **Keine Schemaänderung in Sprint 5.** Selbst geprüft: Der Sprint-Diff fasst
   an Quellcode ausschließlich `src/ui/librarywindow.{h,cpp}` an — `src/store/`
   ist unberührt. Damit greift weder die Nennungspflicht für Schemaänderungen
   noch der erzwungene MINOR-Sprung aus Punkt 10.
2. **Alle vier Stories gehören aus Nutzersicht hinein**, auch die beiden mit
   `typ:bug`: Symbole an Schaltflächen, eine Liste, die unter dem Zeiger
   stehenbleibt, und Texte, die dem Farbschema folgen, sind samt und sonders
   sichtbar. Der Maßstab ist die Nutzersicht, nicht das Label.
3. Punkt 10 (Version und Tag) bleibt **ausgesetzt**, bis #61 umgesetzt ist —
   `CMakeLists.txt` steht bei `0.1.0`, die Einträge sammeln sich weiter unter
   `[Unveröffentlicht]`. Der Vollzugsvermerk in Takt 2 muss das so führen.

## 21. Mängelliste an den PO (Takt 1, Punkt 4)

Melden, nicht heilen. Nichts davon ist von mir behoben worden.

### M1 — Kein Story-Hauptweg am installierten Stand belegt · **Schwere: mittel**

**Regel:** Sprint-Abschluss Takt 1, Punkt 1 — „Der Endstand ist einmal nach
`/usr` installiert, **und der Hauptweg jeder Story ist daran ausgeführt**
(DoD 2). … Ohne Belegform ist ‚mit Beleg abgehakt' eine Behauptung."

**Befund:** `sprint-05-installationstakt.md` belegt Installation (rc=0),
Binärgleichheit, Dienstwechsel (PID 460335) und D-Bus-Antwort. Von den vier
Hauptwegen ist **keiner** am installierten Stand ausgeführt; alle vier stehen
unter „Offen für die Kundenabnahme".

**Warum das zählt:** Genau diese Pflicht ist als Ersatz dafür entstanden, dass
den Strängen die eigene Installation untersagt wird (Sprint-3-Mangel M1). Wird
der Ersatz nicht geleistet, ist DoD 2 für den Sprint aufgehoben statt
geschützt. Die Kundenabnahme ist **nicht** dieser Ersatz: Sie prüft, ob das
Produkt gefällt, nicht ob der ausgelieferte Stand den Hauptweg trägt.

**Abgeschwächt, aber nicht geheilt:** Ich habe nachgewiesen, dass das
installierte Abbild der Sprint-5-Stand ist (15.1) — der **Stand** ist belegt,
der **Weg** nicht.

**Vorschlag (PO-Fläche, drei Handgriffe):** Der Dienst nimmt `ShowLibrary`
an — die Methode ist vorhanden (von mir per Introspektion belegt), `spectacle`
ist installiert. Ein Aufruf, ein Bildschirmfoto der Bibliothek mit Symbolen und
eines des Wächterdialogs am laufenden `/usr`-Dienst, abgelegt in
`docs/scrum/reviews/sprint-05-installationstakt/`, schließt den Punkt für
#66/#67 und #58. Für #57 (Mausbewegung) und für die Kundenfotos bleibt der
Kundenblick — das ist eine benannte Grenze der Prüfbarkeit und schließt die
Story nach DoD 2 nicht. **Die Grenze gehört benannt, nicht stillschweigend
ausgelassen.**

### M2 — Schätzung steht nicht am Backlog · **Schwere: gering (Rückverfolgbarkeit)**

**Regel:** `PROZESS.md`, Artefakte — GitHub Issues sind „die **einzige Quelle
der Wahrheit** für Stories, Akzeptanzkriterien, **Schätzung** und Status";
Labels `sp:1|2|3|5|8`. Planning §10, Punkt 5 hat es dem PO ausdrücklich
aufgetragen.

**Befund, selbst erhoben** (`gh issue list --milestone "Sprint 5"`):

| Issue | Konsolidiert (Planning §2) | Label heute |
|---|---|---|
| #66 | 5 | **keines** |
| #67 | 2 | **keines** |
| #57 | 3 (ersetzt `sp:2` mit Begründung, §2.1) | **`sp:2`** — veraltet |
| #58 | 1 | `sp:1` ✓ |

**Warum das zählt:** Aus den Labels ergäbe sich heute ein Sprint von **3 SP**
statt 11. Wer in einem halben Jahr die Velocity aus dem Backlog zieht — der
einzigen Quelle der Wahrheit —, bekommt eine falsche Zahl. Bei #57 ist es
mehr als eine fehlende Zahl: Die Erhöhung war eine begründete Entscheidung
zweier unabhängiger Schätzer gegen das Alt-Label, und im Backlog steht weiter
das Alt-Label.

**Vorschlag:** `sp:5` an #66, `sp:2` an #67, `sp:2` → `sp:3` an #57. Drei
`gh`-Aufrufe, vor dem Schließen der Issues in Takt 2.

### M3 — README beschreibt drei der vier Lieferungen nicht · **Schwere: gering**

**Regel:** `PROZESS.md`, DoD-Anhang — „Beschreiben README und `docs/` den
gelieferten Stand? Abweichungen meldet er als Mangel"; Takt 1, Punkt 3.

**Befund:** `README.md:7` nennt den Wächterdialog (Sprint 4), aber weder die
Symbole an Dialog und Schaltflächen (#66/#67) noch die ruhige Liste (#57) noch
das Mitgehen der Texte beim Schemawechsel (#58). Für das Tray-Menü steht der
entsprechende Satz seit Sprint 4 darin.

**Vorschlag:** ein bis zwei Halbsätze in der Statuszeile, in derselben Machart
wie der Tray-Satz. Formulierung ist PO-Sache; **Zeitpunkt: vor Takt 2**, damit
der Changelog-Eintrag und die Statuszeile dasselbe sagen.

### Was ich geprüft und **nicht** beanstandet habe

Ausdrücklich, damit „geprüft, nichts gefunden" von „vergessen" unterscheidbar
bleibt (PROZESS.md verlangt das für den Doku-Abgleich; ich führe es für alle
Punkte):

- **DoD 1** — Bau warnungsarm (0 Warnungen), 107/0 in beiden Umgebungen,
  ctest 7/7, drei Geometriefamilien bei je zwei Fenstergrößen, keine
  Linterbefunde aus diesem Sprint.
- **DoD 3** — beide Prüfketten geführt, Berichte als Datei, kein offener
  `fail`, alle sieben Befunde abgearbeitet, UI-Review mit **eigenen** Bildern
  und eigenem Prüfprogramm.
- **DoD 4** — SPEC 9 und 15 nachgezogen; **sieben** entdeckte Bedingungen
  festgehalten, darunter beide vom PO benannten.
- **Sprint-Konto** — 4/4 Issues, 11/~13 SP, null Zugänge, drei neue Issues
  sauber im Backlog.
- **Dateimengen** — kein Übergriff; die eine Abweichung (Belegordnername) ist
  vom Strang selbst deklariert und geht auf den Spawn-Auftrag zurück.
- **Melden statt heilen** — von beiden Strängen eingehalten: Strang A meldete
  die SPEC-15-Klammer und fasste sie nicht an, Strang B ließ den roten
  Strang-A-Test ausdrücklich stehen.

## 22. Abnahme-Checkliste für den Kunden

Vier Handgriffe am laufenden, installierten Stand (Dienst PID 460335 aus
`/usr/bin`). Die Bilder daneben sind aus dem Prüflauf — sie zeigen, worauf zu
schauen ist, ersetzen den eigenen Blick aber nicht.

### 1 · Die Bibliothek hat Symbole (#67)

Bibliothek öffnen, eine Notiz in der Liste anklicken.

- [ ] Oben rechts stehen **„Bearbeiten"** mit einem **Stift** und **„Löschen"**
      mit einem **roten Papierkorb**.
- [ ] Jedes Symbol steht **neben** seiner Beschriftung, keines ersetzt sie.
- [ ] F2 drücken: unten rechts stehen **„Speichern"** (Diskette) und
      **„Abbrechen"** (durchgestrichener Kreis).
- [ ] Eine Notiz löschen: die Meldungszeile bietet **„Rückgängig"** mit einem
      **Pfeil nach links**.

*Vergleichsbild: `docs/scrum/reviews/sprint-05-ui-review/bilder/s5-edit-01-lesen.png`*

### 2 · Der Nachfragedialog sieht aus wie ein KDE-Dialog (#66) — **Foto-Punkt**

F2 drücken, etwas tippen, dann eine andere Notiz anklicken.

- [ ] Links im Dialog steht ein **großes orangefarbenes Warndreieck**.
- [ ] Der Text beginnt mit **„Änderungen speichern?"**, darunter nach einer
      Leerzeile die Erklärung mit dem Zeitstempel in Klammern.
- [ ] Alle **drei** Antworten tragen ein Symbol: **Speichern** (Diskette),
      **Verwerfen** (roter Papierkorb), **Abbrechen** (durchgestrichener Kreis).
- [ ] **„Speichern" ist vorausgewählt** — es trägt den Rahmen, und Enter löst es
      aus.
- [ ] Die Antworten tun, was sie sagen: *Speichern* schreibt und wechselt zur
      angeklickten Notiz, *Verwerfen* wechselt ohne zu schreiben, *Abbrechen*
      bleibt im Editor.

**Bitte ein Foto** — dieser Punkt ist der Grund für die Story: In der
Sprint-4-Abnahme trug derselbe Dialog unter echtem Plasma **keine** Symbole,
und kein Test hat das gesehen. Nur das Bild aus Ihrer Sitzung schließt ihn.
*Vergleichsbild aus dem Prüflauf:
`docs/scrum/reviews/sprint-05-ui-review/bilder/s5-edit-03-waechterdialog.png`*

### 3 · Die Liste bleibt unter dem Zeiger stehen (#57)

In der Liste weit nach unten rollen, bis Notizen mehrerer Tage sichtbar sind.
Dann eine **vollständig sichtbare** Notiz aus einer **anderen** Tagesgruppe
anklicken.

- [ ] **Die Liste bewegt sich nicht.** Die angeklickte Zeile bleibt genau da,
      wo der Zeiger sie getroffen hat.
- [ ] Der Tag steht trotzdem da: rechts im Detailbereich, mit vollem
      Zeitstempel.
- [ ] Gegenprobe mit der **Tastatur**: mit den Pfeiltasten über eine
      Tagesgrenze gehen — hier **soll** die Liste den Gruppenkopf ins Bild
      holen. Das ist Absicht: Wer zeigt, will Ruhe; wer tippt, will geführt
      werden.

*Vorher war das ein Sprung über den halben Bildschirm (387 px gemessen).*
*Bekannte Ausnahme, als Issue #71 im Backlog: Eine am unteren Rand
**angeschnittene** Zeile rückt beim Anklicken noch um eine Zeilenhöhe — das ist
Bestand, kein Rückschritt dieses Sprints.*

### 4 · Farbschema wechseln, ohne den Dienst neu zu starten (#58)

Bibliothek geöffnet lassen. In den Systemeinstellungen von einem **hellen** auf
ein **dunkles** Farbschema wechseln (oder umgekehrt).

- [ ] Die **Uhrzeiten** in der Liste und die **Vorschauzeilen** wechseln
      **sofort** mit — sie bleiben lesbar, nicht dunkelgrau auf dunkel.
- [ ] Auch im Detailbereich: der Zeitstempel oben, und im Bearbeiten-Zustand
      die Beschriftungen **„Kategorie"** und **„Tags"** sowie der Fußzeilenhinweis
      *„Esc bricht ab · Strg+Enter speichert"*.
- [ ] **Kein Neustart nötig** — das war der Kern des Befundes: Das Fenster wird
      beim Dienststart gebaut und vorgehalten, die Farben froren bis zum
      Neustart ein.
- [ ] Auch zurück auf hell prüfen.

*Vergleichsbild: `docs/scrum/reviews/sprint-05-ui-review/bilder/s5-58-b-dunkel-lesen.png`;
danebengelegt das ungeheilte Bild `s5-58-b-UNGEHEILT-dunkel-lesen.png`.*

### Was der Kunde **nicht** prüfen muss

Diese drei Punkte sind in diesem Sprint gefunden und bewusst **nicht** gebaut
worden; sie liegen als Issues im Backlog und gehören nicht in die Abnahme:
**#70** (Pfeiltaste zur ersten Gruppen-Notiz lässt den Kopf 35 px draußen —
braucht eine Produktentscheidung), **#71** (angeschnittene Zeile, siehe oben),
**#72** (Tastenkürzel als Tooltip an den Schaltflächen — nie beauftragt,
Empfehlung des UI-Reviews).

## 23. done / next

**done:** DoD-Prüfung Takt 1 für Sprint 5 geführt — Bau, `ctest` und
`librarytest` in **beiden** Umgebungen selbst gefahren (0 Warnungen, 7/7,
107/0 und 107/0), beide Linter selbst gefahren und **jeden** ihrer fünf
Befunde per `git blame` gegen `sprint-05-basis` als Altbestand nachgewiesen
statt den Bericht zu glauben; die drei Geometriefamilien und ihre je zwei
Fenstergrößen nachgezählt und begründet, warum der Wächterdialog keine vierte
verlangt; den installierten Stand über vier unabhängige Wege als den Endstand
belegt (Binärgleichheit, Bauzeit 7 s nach dem letzten Quell-Commit,
Sprint-5-Zeichenketten im Abbild, laufender Dienst mit `ShowLibrary`); die
Akzeptanzkriterien aller vier Stories am Stand geprüft, darunter #58 AK 4
selbst gefahren (`grep -rn setPalette src/` → kein Treffer) und zwei Bilder
selbst angesehen statt ihre Beschreibung zu lesen; die Vollzähligkeit der
Prüfberichte über den Commit-Prüfweg aus B11.2 geführt und den einen
Grenzfall (UX-Zwischenlauf zum Warnsymbol) als versioniert nachgewiesen; alle
sieben Review-Befunde bis zu ihrem Verbleib verfolgt; den SPEC-Diff Zeile für
Zeile gelesen und **sieben** entdeckte Bedingungen nach B9 ausgewiesen; den
Doku-Abgleich nach B10 geführt; das Sprint-Konto geschlossen (4/4 Issues,
11/~13 SP, **null Zugänge** — die drei Sprint-Funde liegen milestone-frei im
Backlog). **Drei Mängel** an den PO gemeldet, keiner geheilt. Abnahme-Checkliste
in Kundensprache mit vier Handgriffen und einem ausgewiesenen Foto-Punkt
vorgelegt.

**next:** (1) PO behebt M1 vor der Abnahme — der Aufwand ist ein D-Bus-Aufruf
und zwei Bildschirmfotos am laufenden `/usr`-Dienst; die verbleibende Grenze
(#57 und die Plasma-Fotos) gehört benannt, nicht ausgelassen. (2) M2 vor dem
Schließen der Issues: drei `sp:`-Label. (3) M3 vor Takt 2: Statuszeile im
README um die drei fehlenden Verhaltensweisen ergänzen. (4) Kundenabnahme
gegen die Checkliste in 22, mit Foto zum Wächterdialog. (5) Nach der Abnahme
Takt 2 in der Reihenfolge der Punkte 5–**12**; Punkt 10 bleibt **ausgesetzt**
(#61 offen), die Einträge sammeln sich unter `[Unveröffentlicht]`. Der
Scrum Master vermerkt den Vollzug (Punkt 11). **Punkt 12 ist neu**
(Schätzhistorie, Kundenauftrag 02.08.2026) und läuft in diesem Sprint zum
ersten Mal: Der Ausgangsbestand samt den vier Sprint-5-Zeilen liegt in §24,
der Verwalter überträgt ihn mechanisch. (6) Retro nach Sprint 6 —
Kandidaten unverändert aus Sprint 4 §17.6, dazu der B13-Nachzug aus §5.1
dieses Protokolls.

## 24. Anhang — Schätzhistorie, Ausgangsbestand (Kundenauftrag 02.08.2026)

**Diese Tabelle ist die einzige Quelle, aus der `docs/scrum/diagramme/schaetzhistorie.json`
erzeugt wird.** Jede Zeile ist an ihrer Quelle nachgelesen; die
Anlass-Kennzeichen sind mein Urteil und stehen nirgendwo sonst. Ab Sprint 5
schreibt die DoD-Prüfung je Sprint die neuen Zeilen fort (PROZESS.md,
Sprint-Abschluss Punkt 12).

**Festlegungen, ohne die die Reihe nicht eindeutig ist:**

- **Erstschätzung ist die erste *konsolidierte* Schätzung** — der Wert, mit dem
  die Story im Backlog stand, nicht die Rohzahl eines einzelnen Schätzers. Die
  Streuung zweier Schätzer am selben Tag ist eine andere Größe und gehört nicht
  in diesen Kegel (dieselbe Begründung, mit der die Sprint-5-Deckungsgleichheiten
  von #58, #59 und #55 als Datenpunkte ausscheiden).
- **Abstand** = Zahl der Sprints zwischen dem Sprint, in dem die Erstschätzung
  fiel, und dem Sprint der Umsetzung.
- **Faktor** = Endwert ÷ Erstwert. Er kann **unter 1** liegen (#5), und die
  Achse muss das darstellen können.
- **Provenienz wird mitgeführt, nicht geglättet:** „Klausur/Planning (2
  unabhängige)" ist etwas anderes als „Label bei Anlage (1 Hand)". Wer die
  Reihe später gewichten will, braucht die Unterscheidung; wer sie einebnet,
  verliert sie unwiederbringlich.

### 24.1 Reihe — Stories mit mindestens einer Gelegenheit zur Revision

Nur diese Zeilen tragen den Kegel: Zwischen Erstschätzung und Umsetzung lag ein
Ereignis, bei dem eine Revision **hätte** stattfinden können.

| Story | Issue | Erstschätzung | Quelle | Revision | End | Umsetzung | Abst. | Faktor | Anlass |
|---|---|---|---|---|---|---|---|---|---|
| S4 Globales Kürzel + D-Bus | #5 | **5** · 31.07.2026 | Schätzklausur, `sprint-01.md` §2 (A 3 · B 5, Regel höherer Wert) — 2 unabhängige | **3** · 01.08.2026, `sprint-02.md` §1.3 | 3 | Sprint 2 | 1 | **0,60** | `erkenntnis` |
| T2 Autostart und Erststart | #6 | **2** · 31.07.2026 | Schätzklausur, §4.2 (neu geschnitten) | — | 2 | Sprint 2 | 1 | 1,00 | `keine` |
| S5 Bibliotheksfenster | #7 | **5** · 31.07.2026 | Schätzklausur §2 (A 5 · B 3, Regel höherer Wert) — 2 unabhängige | — | 5 | Sprint 2 | 1 | 1,00 | `keine` |
| S6 Volltextsuche (FTS5) | #8 | **3** · 31.07.2026 | Schätzklausur §2 (A 3 · B 3, einig) — 2 unabhängige | **5** · während Sprint 3, `sprint-03.md` 13.10 | 5 | Sprint 3 | 2 | **1,67** | `erkenntnis` |
| T3 Migrationstest 1→2 | #9 | **1** · 31.07.2026 | Schätzklausur §4.3 (neu) | — | 1 | Sprint 3 | 2 | 1,00 | `keine` |
| S8 Bearbeiten-Ansicht | #11 | **2** · 31.07.2026 | Schätzklausur §2 (A 2 · B 2, Entscheidung E3) — 2 unabhängige | **5** · 02.08.2026, `sprint-04.md` §2.1 | 5 | Sprint 4 | 3 | **2,50** | `gegenstand-geändert` |
| T4 Ollama-Modelle | #12 | **1** · 31.07.2026 | Schätzklausur §4.4 (neu) | — | 1 | Sprint 4 | 3 | 1,00 | `keine` |

**Hüllkurve:** Abstand 1 → [0,60; 1,00] · 2 → [1,00; 1,67] · 3 → [1,00; 2,50].

**Nachgerechnet statt behauptet:** Im Logarithmus, auf dem die Achse steht, ist
die größte Abweichung je Abstand **0,51 · 0,51 · 0,92** — denn `ln(5/3)` und
`ln(3/5)` sind betragsgleich. Die Hüllkurve ist damit **nicht-fallend, aber
nicht streng wachsend**: Abstand 1 und 2 sind gleich weit, die Weitung tritt
erst bei Abstand 3 ein. Ein Kegel im Sinne der Lehrbuchfigur wäre erst mit mehr
Punkten je Abstand belegt. Wer hier „monoton weitend" schreibt, sagt mehr, als
die sieben Punkte hergeben.

**Zu den drei Kennzeichen, weil sie das Urteil tragen:**

- **#5 `erkenntnis`, abwärts.** B's Aufschlag stand für ein Wayland-Risiko; das
  Risiko wurde gemessen und löste sich auf, der Gegenstand blieb derselbe
  (`sprint-02.md` §1.3). **Der einzige Punkt unter 1,0** — und der Beleg dafür,
  dass Unsicherheit sich in beide Richtungen auflöst.
- **#8 `erkenntnis`.** Die Story ging mit 3 in Sprint 3 (`sprint-03.md`, Tabelle
  Zeile 31) und endete bei 5. Der Zuwachs kam aus der Arbeit am Gegenstand, nicht
  aus einer Änderung an ihm.
- **#11 `gegenstand-geändert`, nicht `erkenntnis`.** Zwischen der 2 und der 5
  liegen S5a und S6 — Flächen, mit denen die Bearbeiten-Ansicht wechselwirkt, und
  neugefasste Akzeptanzkriterien. `sprint-04.md` §2.1 wörtlich: *„Die 2 hat einen
  anderen Gegenstand geschätzt."* **Der größte Faktor der Reihe ist zugleich der,
  der am wenigsten über Schätzgenauigkeit sagt** — genau dafür existiert die
  Spalte.

### 24.2 Erfasst, aber nicht in der Kurve

| Story | Issue | Erst | End | Umsetzung | Grund der Auslassung |
|---|---|---|---|---|---|
| T1 Wayland-Fokus-Spike | #1 | 2 | 2 | Sprint 1 | Erstschätzung und Umsetzung im selben Planning |
| S1 Projektgerüst mit Tray | #2 | 3 | 3 | Sprint 1 | dito |
| S2 SQLite-Store | #3 | 3 | 3 | Sprint 1 | dito (Klausur A 5 · B 3, Entscheidung E2) |
| S3 Text-Capture-Fenster | #4 | 5 | 5 | Sprint 1 | dito |
| Capture Starthöhe 5 Zeilen | #42 | 1 | 1 | Sprint 1 | Zugang im laufenden Sprint, Label bei Anlage |
| App-Icon und Tray-Icon | #43 | 2 | 2 | Sprint 1 | dito |
| Tray-Linksklick ohne Wirkung | #44 | 1 | 1 | Sprint 3 | Zugang im laufenden Sprint (2 unabhängige, beide 1, 01.08.2026) |
| S5a Notizliste als Posteingang | #46 | 5 | 5 | Sprint 3 | Zugang im laufenden Sprint (2 unabhängige, beide 5, 01.08.2026) |
| Capture folgt Themewechsel nicht | #54 | 1 | 1 | Sprint 3 | Zugang im laufenden Sprint, Label bei Anlage |
| S33 Tray-Menüs | #60 | 5 | 5 | Sprint 4 | im selben Planning geschätzt (Dev 3 · UX 5 → 5) |
| T10 Spike spellfix1 | #62 | 3 | 3 | eigener Lauf | **Schätzregel nicht erfüllt** (nur eine Schätzung, `sprint-04.md` §2.3), kein Sprint-Milestone |

**Warum diese elf draußen bleiben — und warum sie trotzdem dastehen.** Ihr
Faktor ist 1,0, weil zwischen Erstschätzung und Umsetzung **keine Gelegenheit
zur Revision lag**, nicht weil eine Schätzung sich als richtig erwiesen hätte.
Wer sie mitzeichnet, drückt den linken Rand des Kegels künstlich zusammen und
lässt das Diagramm eine Genauigkeit behaupten, die nirgends gemessen wurde. Sie
stehen hier, damit die Auslassung sichtbar ist und niemand sie später für ein
Versehen hält (dieselbe Form, in der die Sprint-4-Protokolle ihre eigenen
Grenzen benennen).

### 24.3 Sprint 5 — die vier Zeilen dieses Sprints

Der Sprint ist umgesetzt; **kein Wert ist im Sprint revidiert worden** (§19:
null Zugänge, keine Heraufsetzung). Damit stehen Erst- und Endwerte fest, und
der Verwalter hat für Takt 2 nichts zu rekonstruieren.

| Story | Issue | Erstschätzung | Quelle | End | Abst. | Faktor | Anlass |
|---|---|---|---|---|---|---|---|
| Wächterdialog KDE-Bauart | #66 | 5 · 02.08.2026 | Sprint-5-Planning §2 (Dev 5 · UX 3) — 2 unabhängige | 5 | 0 | 1,00 | `keine` — **24.2-Fall** |
| Bibliotheks-Symbole | #67 | 2 · 02.08.2026 | Sprint-5-Planning §2 (Dev 2 · UX 1) — 2 unabhängige | 2 | 0 | 1,00 | `keine` — **24.2-Fall** |
| Klick-Sprung | #57 | **2** · 01.08.2026 | Label bei Anlage — 1 Hand | **3** · 02.08.2026, §2.1 | 2 | **1,50** | `erkenntnis` |
| Palettenrolle | #58 | 1 · 01.08.2026 | Label bei Anlage — 1 Hand | 1 | 2 | 1,00 | `keine` |

**#57 und #58 gehen in die Kurve, #66 und #67 nicht.** #57 ist `erkenntnis`,
nicht `gegenstand-geändert`: Die Akzeptanzkriterien sind unverändert; gestiegen
ist der Wert, weil die naheliegende Lösung **belegt widerlegt** wurde (§2.1).
#58 hatte zwei Sprints lang Gelegenheit zur Revision und hat sie nicht genutzt —
der Punkt ist gemessen, anders als die 1,0-Fälle in 24.2.

**Damit wächst die Reihe auf neun Punkte**, und der Abstand 2 bekommt seinen
dritten und vierten Wert. Die Hüllkurve ändert sich dadurch **nicht**
(Abstand 2 bleibt [1,00; 1,67], jetzt mit vier statt zwei Punkten) — der
Zuwachs macht die mittlere Spalte dichter, nicht breiter. Das ist der
gewöhnliche Fall und der Grund, warum die Reihe Zeit braucht: Ein Kegel
entsteht aus Belegdichte, nicht aus Einzelwerten.

> **Achtung für den mechanischen Lauf:** Die Endwerte dieser vier Zeilen
> stehen **hier**, nicht verlässlich am Backlog. Mangel **M2** (§21) hat
> gemessen, dass #66 und #67 **kein** `sp:`-Label tragen und #57 noch das
> veraltete `sp:2` — aus den Labeln ergäbe sich ein Sprint von 3 SP statt 11.
> Solange M2 offen ist, ist die Spalte „End" aus dem Protokoll zu ziehen und
> **nicht** aus dem Issue. Das ist kein Umweg um M2, sondern seine zweite
> Begründung: Die Labelpflege hat ab jetzt einen zweiten Abnehmer.

**Offen und bewusst nicht entschieden:** ob Zeilen mit Provenienz „1 Hand"
(#57, #58) gleich schwer wiegen wie die der Schätzklausur. Das ist eine Frage an
die Reihe, wenn sie länger ist — heute wäre jede Antwort darauf eine Erfindung.
Die Spalte hält sie offen.
