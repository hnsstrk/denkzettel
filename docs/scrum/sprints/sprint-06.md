# Sprint 6 — Planning-Protokoll

**Datum:** 2026-08-02, 19:40 (Ganymed) · **Moderation:** Scrum Master (Agent
`scrum-master`, frischer Kontext)
**Teilnehmer:** Scrum Master · Product Owner. Schätzer Dev und UI/UX sind zum
Zeitpunkt dieses Protokolls **nicht** befragt worden — das ist der Kern von
Abschnitt 2 und der Grund, warum der Schnitt unter Vorbehalt steht.
**Status des Sprint-Vorschlags:** vorgelegt, Freigabe durch den Kunden steht aus.

**Grundlagen:** `CLAUDE.md`, `docs/scrum/PROZESS.md` (Stand 02.08.2026,
einschließlich Sprint-Abschluss Punkt 12 und der Schätzhistorie-Pflicht in der
DoD-Prüfung), `docs/scrum/sprints/sprint-05.md` (§1 Kontomuster, §13–23
DoD-Prüfung, §24 Schätzhistorie), `SPEC.md` (3, 15, 16),
`docs/scrum/reviews/2026-08-01-capture-theme-treue.md` samt Messdateien,
Wireframes Turn 4 (4a Hülle, 4b Fläche), GitHub-Issues #55, #56, #59, #61,
#68, #69, #70, #71, #72.
**Quellstand aller Prüfungen:** `main` @ `e5e8d78`.

**Kundenrichtung für diesen Sprint:** Kern ist **#55** — der Kunde hat den
Befund am 02.08.2026 per eigener Denkzettel-Notiz erneut angemahnt („Das
Fenster erscheint, aber die Ecken sind noch nicht rund").

Alle Aussagen über Machbarkeit, Prüfmittel und Risiken sind an einer eigenen
Messung oder am Quellcode geführt. Wo ich eine fremde Angabe nur wiedergebe,
steht es dabei.

---

## 1. Sprint-Konto (B12) — ab Zeile 1

| Buchung | Issues | Story Points | Grenzen (2–4 · ~13) |
|---|---|---|---|
| Kandidatenfeld (#55, #56, #68, #59, #70, #71, #72, #61) | 8 | **nicht summierbar** — nur #55 (8) und #59 (2) sind regelkonform geschätzt | Story-Grenze doppelt gerissen |
| Nach Ausschluss des nicht Ziehbaren (#70 AK offen, #71/#72/#61 ungeschätzt) | 4 | 8 + 1 + 2 + *#68 offen* | Story-Grenze **gehalten, am Anschlag** |
| **Vorschlag des Scrum Masters** (2, falls #68 ≤ 3: 3) | **2–3** | **9**, mit #68 **12** | beide gehalten |
| *Freigabe-Stand* | *einzutragen nach der Kundenentscheidung* | | |

**Der erste Befund des Kontos ist, dass es sich nicht führen lässt.** Von acht
Kandidaten trägt genau **einer** eine regelkonforme Schätzung, die auch am
Backlog steht (#59, `sp:2`). Der Kern des Sprints (#55) ist deckungsgleich mit
8 SP geschätzt, aber **ohne `sp:`-Label** — das ist derselbe Mangel, den ich in
Sprint 5 als **M2** gemeldet habe, an der nächsten Story wiederholt. Seit
Abschluss-Punkt 12 gilt: *„Endwert ist das `sp:`-Label des Issues"* (Sprint 5,
§24). Ein fehlendes Label ist damit nicht mehr nur eine Frage der
Rückverfolgbarkeit, sondern eine **Lücke in der Datenreihe des Schätzkegels**.

**Die Rechnung, was neben die 8 passt** — beide Grenzen zugleich, nicht nur die
Punkte (das war die Blindstelle, gegen die B12 gefasst ist):

| Kombination | Issues | SP | Urteil |
|---|---|---|---|
| #55 allein | 1 | 8 | **Story-Grenze unterschritten** (Minimum 2) — kein zulässiger Sprint |
| #55 + #56 | 2 | 9 | hält; 2 Stories und 4 SP Luft |
| #55 + #59 | 2 | 10 | hält; Thema gerissen (Bibliothek statt Erfassungsfenster) |
| **#55 + #56 + #68**, falls #68 ≤ 3 | **3** | **≤ 12** | **hält; 1 Story und ≥ 1 SP Luft** |
| #55 + #56 + #68, falls #68 = 5 | 3 | 14 | **SP-Grenze gerissen** |
| #55 + #56 + #59 | 3 | 11 | hält; Thema gerissen |
| #55 + #56 + #68(3) + #59 | 4 | 14 | **SP-Grenze gerissen** |
| #55 + #56 + #59 + #61(2) | 4 | 13 | beide **am Anschlag**, keine Luft |

**Merksatz aus Sprint 5, hier wieder anwendbar:** Ein Sprint, dessen Konto beim
Start ausgeschöpft ist, verträgt keine Review-Auflage. Sprint 2 erzeugte allein
aus einem UI-Review drei Auflagen. #55 ist die größte Story, die dieses Projekt
je gezogen hat, und trägt drei ungemessene Punkte (7) — genau hier gehört die
Luft hin, nicht ins Konto einer vierten Story.

---

## 2. Schätzlage — was ziehbar ist und was nicht

Die Sprint-Mechanik verlangt **zwei unabhängige Schätzer je Story**. Ein Label
aus der Anlagezeit erfüllt das nicht: In Sprint 5 trug #57 `sp:2` von der
Anlage und wurde von zwei unabhängigen Schätzern auf **3** gehoben — *„Die
beiden Schätzer ersetzen das Label, sie übernehmen es nicht"* (Sprint 5, §2.1).
Dieselbe Lage liegt bei #56 vor.

| Issue | Vorliegende Schätzung | Quelle | Schätzregel erfüllt? | `sp:`-Label heute | Ziehbar? |
|---|---|---|---|---|---|
| **#55** | Dev 8 · UX 8 | Sprint-5-Planning §2, 02.08.2026 | **ja** — 2 unabhängige, deckungsgleich | **keines** ✗ | **ja**, Label fehlt (K1) |
| **#59** | Dev 2 · UX 2 · Label 2 | Sprint-5-Planning §2, 02.08.2026 | **ja** — dreifach deckungsgleich | `sp:2` ✓ | **ja** |
| **#56** | 1 | Label bei Anlage 01.08.2026 — **1 Hand** | **nein** | `sp:1` | **nein**, bis zwei Schätzungen vorliegen |
| **#68** | — | — | **nein** | keines | **nein** |
| **#71** | — | — | **nein** | keines | nein |
| **#72** | — | — | **nein** | keines | nein |
| **#61** | — | — | **nein** | keines | nein |
| **#70** | — | — | **nein** | keines | **nein, doppelt** — die AK sind offen (Produktentscheidung) |
| **#69** | — | drei Klärungen offen | **nein** | keines | **nein** — ausdrücklich nicht für Sprint 6 |

### 2.1 Was ich vom PO brauche, bevor der Schnitt steht

**Zwingend — ohne diese beiden Schätzungen ist gar kein zulässiger Sprint
schneidbar** (#55 allein unterschreitet die Story-Untergrenze):

1. **#56** — zwei unabhängige Schätzungen. Erwartungswert klein: Die Heilung
   ist zeichengenau bekannt (`adjustHeight()` zusätzlich bei
   `QEvent::FontChange`), die Stolperstelle steht im AK, und die Prüfbarkeit
   ohne Plasma ist im AK selbst gelöst. Das ist die Lage von #58 in Sprint 5,
   *„die seltene Lage, in der eine 1 wirklich eine 1 ist"* — aber das Urteil
   fällen die Schätzer, nicht ich.
2. **#68** — zwei unabhängige Schätzungen. **Der Wert entscheidet den
   Schnitt:** ≤ 3 heißt drei Stories und ≤ 12 SP, 5 heißt 14 SP und damit
   draußen. Hinweis für die Schätzer, damit sie nicht am falschen Gegenstand
   schätzen: #68 ist **nicht** capture-only — sein AK nennt „Erfassungsfenster
   **und Bibliothek**", also `src/capture/` *und* `src/ui/`.

**Bedingt — nur falls der Kunde das Konto voller haben will** (siehe 4.4):

3. **#61** — zwei unabhängige Schätzungen. Argument für die Aufnahme: Solange
   #61 offen ist, ist **Abschluss-Punkt 10 ausgesetzt**; jeder abgenommene
   Sprint erzeugt keine Version, und die Changelog-Einträge sammeln sich unter
   `[Unveröffentlicht]`. Das ist eine Schuld, die mit jedem Sprint wächst.
   Argument dagegen: Es liegt im Epic M7 und außerhalb der Sprint-Klammer.

**Nicht angefragt und warum:** #71 und #72 sind Bibliotheksbefunde aus dem
Sprint-5-UI-Review. Sie sind ungeschätzt, aber ihre Schätzung anzufordern hätte
nur Sinn, wenn sie in den Schnitt könnten — beide reißen die Klammer, und die
Punkte trügen sie ohnehin nicht (1). Sie bleiben im Backlog; Priorisierung ist
PO-Sache.

### 2.2 Keine 13er-Story, aber die erste 8er

**#55 ist mit 8 SP die größte Story des Projekts.** Sie ist nicht
teilungsbedürftig im Sinne der Regel (die greift bei 13), aber sie ist der
erste Fall, in dem eine einzige Story **zwei Drittel des Sprint-Budgets** trägt.
Das gehört ausgesprochen, weil es die Bauart des Sprints bestimmt: Dieser
Sprint hat einen Gegenstand und Beiwerk, nicht vier gleichrangige Stories.

---

## 3. Vier eigene Messungen — statt abgewogener Risiken

Die Kundenrichtung nennt drei ungemessene Punkte an #55. Ich habe sie nicht
abgewogen, sondern so weit gefahren, wie es ohne Produktivcode geht. Prüfmittel:
eine Wegwerf-Sonde im Sitzungs-Scratchpad (`KSvg::FrameSvg` + `KWindowShadow`,
gegen `KF6Svg` und `KF6WindowSystem` gelinkt) und ein Bau von `capturetest`
out-of-source, damit der Arbeitsbaum des PO unberührt bleibt.

### 3.1 Messung 1 — die Eckstück-Aussage aus der AK-Richtigstellung ist wahr, ihr Beleg fehlte

Die Richtigstellung an #55 vom 02.08.2026 nimmt eine Messaussage zurück
(„sieben von acht Themes ohne Eckstück", Quelle `achse3-themerundung.txt`) und
setzt an ihre Stelle: **alle acht Themes haben Eckstücke und Schattenkacheln**,
belegt durch `achse3-huellen.txt`.

**Das habe ich nachgesehen, und der Beleg trägt nur zur Hälfte.**
`achse3-huellen.txt` (selbst gelesen, 21 Zeilen) führt drei Spalten: *Ränder*,
*Schattenteile*, *Füllfarbe*. **Eine Spalte für Eckstücke hat die Datei nicht.**
Sie widerlegt die Schatten-Hälfte der alten Messung eindeutig — die
Eckstück-Hälfte widerlegt sie nicht, sie schweigt dazu. Eine versionierte
Messung vom 02.08.2026, die Eckstücke misst, existiert im Repo nicht
(`grep -rl "Eckstück\|Eckstueck\|topleft" docs/` findet nur den Theme-Bericht
und die überholte Frühmessung).

**Also selbst gemessen, auf zwei unabhängigen Wegen:**

*Weg 1 — direkt an den Theme-Dateien* (`zcat …/dialogs/background.svgz`,
Element-IDs gezählt):

| Theme | `topleft` | `shadow-topleft` |
|---|---|---|
| default | ja | ja |
| cachyos-emerald · -color · -light | ja (je) | ja (je) |
| CachyOS-Nord-round | ja | ja |
| Iridescent-round | ja | ja |
| breeze-light · breeze-dark | **keine eigene `dialogs/background`** — erben von `default` | dito |

*Weg 2 — durch `KSvg::FrameSvg`, also so, wie der Code sie sehen wird* (3.2).

**Ergebnis: Die Richtigstellung ist sachlich richtig.** Alle acht Themes tragen
Eckstücke und Schattenkacheln; die beiden Breeze-Varianten über die Vererbung
auf `default`. **Der Beleg dafür lag aber nicht im Repo** — und ein
unversionierter Beleg ist kein Beleg (B7). Er liegt jetzt hier. Das ist kein
Mangel an der Story, sondern der §24.4-Fall in der Form, in der er beim
nächsten Mal wieder auftreten wird: Eine Berichtigung, die weiter reicht als
ihre neue Quelle.

### 3.2 Messung 2 — KSvg löst das Desktop-Theme offscreen vollständig auf

Das ist die **tragende Annahme des ganzen Bildnachweises** aus AK 8: Rundung
und Kontur sollen offscreen im Bild belegt werden. Wenn `KSvg` ohne
Plasma-Sitzung nichts fände, fiele dieser Nachweis — und mit ihm die
Prüfbarkeit der Story.

Sonde unter `QT_QPA_PLATFORM=offscreen`, `ImageSet` auf `plasma/desktoptheme`
gesetzt, `FrameSvg` auf `dialogs/background`, Rahmen auf 600×200 gelegt:

| Theme | `isValid()` | Rand links | Rand oben | `hasElement("topleft")` | `hasElementPrefix("shadow")` | Maske leer? |
|---|---|---|---|---|---|---|
| default | **1** | 4 | 4 | **1** | **1** | **nein** |
| breeze-dark | **1** | 4 | 4 | **1** | **1** | **nein** |
| CachyOS-Nord-round | **1** | 8 | **7,99998** | **1** | **1** | **nein** |
| Iridescent-round | **1** | 8 | **7,99998** | **1** | **1** | **nein** |

Mit und ohne `QT_QPA_PLATFORMTHEME=kde` **identisch** — die Theme-Auflösung
hängt an den XDG-Datenpfaden, nicht an der Plattformintegration.

**Drei Folgerungen für das Planning:**

1. **AK 8 ist erfüllbar.** Die Randmaße 4/4 gegen 8/8 aus `achse3-huellen.txt`
   sind offscreen reproduzierbar, die Maske ist nicht leer, die Eckstücke sind
   da. Der Prüfsatz aus 4b (*„Bei zwei Desktop-Themes mit unterschiedlichem
   Rand unterscheiden sich Randmaß und Eckform entsprechend"*) ist offscreen
   messbar **und** bildbar.
2. **`marginSize()` liefert Fließkomma, und es ist nicht glatt.** `7,99998`
   statt 8. Eine Zusicherung `QCOMPARE(margin, 8)` fällt. Das gehört in den
   Spawn-Auftrag, sonst kostet es den Dev einen Anlauf gegen einen roten Test,
   der nichts über die Sache sagt — die Bauart, gegen die `CLAUDE.md` seine
   Prüfhaltung fasst.
3. **`KF6::Svg` ist auf der Maschine vorhanden** — `ksvg 6.28.0-1.1`,
   `/usr/lib/cmake/KF6Svg`. Die neue Abhängigkeit ist kein Beschaffungsrisiko,
   nur ein Verkabelungsrisiko (7.3).

### 3.3 Messung 3 — `KWindowShadow::create()` liefert offscreen tatsächlich falsch

Dieselbe Sonde, echtes `QWindow` mit `create()`, eine 8×8-Kachel als
`setTopTile()`:

```
KWindowShadow::create() = 0      (offscreen, mit und ohne Plattformthema)
```

**Die Aussage der UX-Beratung ist unabhängig bestätigt.** Die getrennten
Belegformen in AK 8 sind damit keine Vorsicht, sondern Messung — und die
Konsequenz für DoD 3 steht in Abschnitt 6.2.

### 3.4 Messung 4 — das Plattformthema an `capturetest` kostet heute nichts

`tests/CMakeLists.txt:34` setzt für `capturetest` heute **nur**
`QT_QPA_PLATFORM=offscreen`, ohne `QT_QPA_PLATFORMTHEME=kde`. AK 8 verlangt das
Thema für die Bildläufe, und `CLAUDE.md` verlangt es für jeden Bildlauf
überhaupt („sonst verfälscht eine Ersatzschrift die Größenverhältnisse"). Die
Frage ist dieselbe wie F2 in Sprint 5: Verschiebt das Thema bestehende
Geometriewerte?

Gebaut out-of-source aus `e5e8d78`, beide Läufe von mir gefahren:

| Lauf | Ergebnis |
|---|---|
| `QT_QPA_PLATFORM=offscreen` (heutiger Stand) | **10 passed, 0 failed** |
| `QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde` | **10 passed, 0 failed** |

**Kein Test wird rot** — anders als in Sprint 5, wo genau ein Test fiel und
dieser der Kundenbefund war. Der Grund ist am Testbestand ablesbar: Die
Höhenzusicherungen von `capturetest` messen die reine Funktion
`capture::textAreaHeight(Zeilen, Zeilenabstand, Chrom)` mit **eingesetzten**
Werten (`tests/capturetest.cpp:81–101`), und `windowFollowsTheTextHeight`
vergleicht **relativ** (größer/gleich), nicht absolut. Beides ist gegen einen
Schriftwechsel unempfindlich.

**Folge:** Die Zeile kann gesetzt werden, wann immer der Strang sie braucht;
sie ist heute wirkungslos und wird es erst mit den neuen Zusicherungen nicht
mehr sein. Das verschiebt das Risiko — wie in Sprint 5 — auf die Zusicherungen,
die der Sprint **erst schreibt** (7.5).

---

## 4. Sprint-6-Vorschlag

### 4.1 Sprint-Ziel

> **Das Erfassungsfenster sieht aus wie ein KDE-Fenster: Es trägt Rundung,
> Kontur und Schatten seines Desktop-Themes — und es behält seine fünf Zeilen,
> wenn der Nutzer die Systemschrift ändert.**

Nachprüfbar in drei Handgriffen ohne Werkzeug:

1. Kürzel drücken — **die Ecken des Fensters sind rund**, und es liegt ein
   Schatten darunter wie unter einem Plasma-Popup (#55). Das ist der Satz, den
   der Kunde zweimal angemahnt hat.
2. Desktop-Theme wechseln, **ohne** den Dienst neu zu starten — die Hülle
   wechselt mit: schmaler flacher Bogen bei Breeze, breiter runder bei
   CachyOS-Nord-round (#55, AK 7).
3. Systemschrift vergrößern — **das Feld bleibt fünf Zeilen hoch**, statt auf
   drei zu fallen (#56).

Falls #68 hinzukommt, tritt ein vierter Handgriff daneben: Systemschrift
ändern und die Fenster folgen **ohne Neustart** — dann trägt das Ziel den
Zusatz *„und beides folgt dem laufenden Betrieb"*.

**Warum diese Klammer und keine weitere.** Das Ziel deckt genau die Stories, die
darunter fallen. Ein Bibliotheksbefund wie #59 oder #71 wäre unter dieser
Klammer nicht nachprüfbar — er würde sie zu *„Denkzettel verhält sich wie eine
KDE-Anwendung"* verwässern, also zum Sprint-5-Ziel zurück. Der Sprint-5-Maßstab
gilt hier gegen die eigene Versuchung, das Konto zu füllen: *„der Sprint hätte
kein Thema mehr, sondern ein Vorhaben"* (Sprint 5, §4.1).

### 4.2 Der Schnitt

| Strang | Issue | Story | SP |
|---|---|---|---|
| A | #55 | Capture-Fensterhülle — Rundung, Kontur, Schatten aus dem Theme | **8** |
| A | #56 | Feldhöhe folgt einer Schriftänderung nicht | *1, zu bestätigen* |
| A | #68 | Schrift folgt zur Laufzeit nicht (KConfigWatcher) | ***offen — nur bei ≤ 3*** |
| | | **Summe ohne #68** | **9** |
| | | **Summe mit #68 (bei 3)** | **12** |

**Ein Strang, nicht zwei — und das ist ein Befund, kein Versäumnis.** Dazu 5.

**Reihenfolge innerhalb des Strangs: #56 → #55 → #68.** Nicht Ermessen, sondern
aus den Issues abgeleitet:

- **#56 vor #68**, weil #56 es selbst verlangt: *„Sobald jemand das nachrüstet
  [#68], tritt dieser Fehler sofort zutage. Er gehört deshalb vorher behoben,
  nicht nachher: Sonst baut man eine Verbesserung und liefert die
  Verschlechterung mit."* Das beantwortet zugleich die offene Frage in #68 AK 2
  (*„löst #56 mit oder baut darauf auf — im Planning entscheiden"*) → **K3**.
- **#56 vor #55**, weil #56 die kleinste und am genauesten bekannte Änderung
  ist und dieselbe Datei anfasst. Wer zuerst die 1 baut, hat einen grünen Stand,
  gegen den er die 8 misst.

### 4.3 Was draußen bleibt und warum

- **#59 (2 SP, geschätzt und ziehbar)** — das einzige Issue des Feldes, das
  heute sofort ziehbar wäre. Es bleibt draußen, weil es die Klammer reißt
  (Bibliothek statt Erfassungsfenster). Es ist zugleich der naheliegendste
  Zugang, falls der Kunde drei Stories will und #68 zu groß ausfällt (4.4).
- **#70** — die AK sind offen, es braucht eine Produktentscheidung. Nicht
  ziehbar, unabhängig von jeder Schätzung. **Wenn der Kunde ohnehin am Tisch
  sitzt, ist die Freigabe der billigste Moment, diese Entscheidung
  mitzunehmen** — sie kostet einen Satz und macht #70 für Sprint 7 schätzbar
  (K5).
- **#71, #72** — ungeschätzt und außerhalb der Klammer.
- **#69 (spellfix1)** — ausdrücklich nicht für Sprint 6. Die drei Klärungen aus
  Sprint 5, §2.4 sind unverändert offen; der eigene next-Punkt (5) des
  Sprint-5-Plannings, sie vor diesem Planning zu heilen, ist **nicht erledigt**.
  Festgehalten, nicht beanstandet — die Priorität liegt beim PO.

### 4.4 Der Ersatzweg, falls #68 nicht hineinpasst

Fällt die Schätzung für #68 auf **5**, ist der Schnitt **#55 + #56 = 9 SP /
2 Stories**. Das hält beide Grenzen und lässt bewusst Luft. Zwei Lesarten, und
ich empfehle die erste:

1. **Bei 2 Stories und 9 SP bleiben.** Die Luft ist nicht Leerlauf, sondern
   Vorsorge: #55 ist die erste 8er des Projekts, hat drei ungemessene Punkte,
   und ihr Schatten-Nachweis hängt an einem Kundenfoto (6.2). Sprint 2 erzeugte
   aus einem einzigen UI-Review drei Auflagen.
2. **#59 als dritte Story ziehen** (11 SP / 3 Stories). Kostet die Klammer;
   dafür ist es das einzige heute ziehbare Issue und läge in einer **vollständig
   disjunkten Fläche** (`src/ui/librarywindow.cpp`), wäre also der einzige
   Kandidat für einen echten zweiten Strang. **Gegenargument aus dem eigenen
   Prozess:** *„Sprints sind Arbeitspakete, keine Zeiträume"* — Parallelität ist
   in diesem Projekt kein Wert an sich, also trägt das Argument
   „läuft nebenher" hier nicht.

Ein dritter Weg, **#61**, ist von anderer Art: Er füllt das Konto nicht besser
als #59, löst aber eine Prozessschuld (Abschluss-Punkt 10 ausgesetzt). Er
braucht zuerst zwei Schätzungen. **Empfehlung: nicht in Sprint 6** — ein Sprint
mit einer 8er-Risikostory ist der falsche Ort für eine Story aus einem fremden
Epic. Als **Sprint-7-Kandidat vormerken**, damit die Aussetzung nicht dauerhaft
wird.

---

## 5. Strang-Zuschnitt und Dateimengen (B13, Notation nach karpathy 3.1)

### 5.1 Warum dieser Sprint nicht parallelisiert

Sprint 4 hatte disjunkte Flächen (`src/ui/` gegen `src/shell/`). Sprint 5 hatte
zwei gemeinsame Dateien, aber **38 Zeilen kleinsten Bereichsabstand** in Dateien
von über tausend Zeilen — Git mischt mit drei Zeilen Kontext, ein Textkonflikt
war ausgeschlossen.

**Hier ist die Lage anders, und zwar messbar:**

| Datei | Länge | #55 fasst an | #56 fasst an | #68 fasst an |
|---|---|---|---|---|
| `src/capture/capturewindow.cpp` | **157 Zeilen** | Konstruktor `:42–74` (Ränder `:58`, Abstände `:59`), neu `paintEvent`/`resizeEvent`, `present()` `:111–116` | Konstruktor `:67–70` (Signalbindung), `adjustHeight()` `:148–157`, neu `changeEvent` | dieselben Stellen wie #56, dazu Signalbindung an `KConfigWatcher` |
| `src/capture/capturewindow.h` | 38 Zeilen | neue Member, neue überschriebene Methoden | neue überschriebene Methode | dito |

**Der kleinste Abstand ist null: Alle drei schreiben in denselben Konstruktor
(`:42–74`) und in dieselbe Kopfdatei von 38 Zeilen.** Ein Textkonflikt ist bei
paralleler Arbeit nicht unwahrscheinlich, sondern der Normalfall.

**Empfohlene Zahl der Dev-Agenten: eins.** Sie folgt aus den Bereichen, nicht
aus der Erlaubnis — dieselbe Herleitung wie in Sprint 5, mit umgekehrtem
Ergebnis.

### 5.2 Dateimenge Strang A

| | **Strang A** — die einzige Fläche dieses Sprints |
|---|---|
| **Issues** | #55, #56 (+#68 nach Schätzung) |
| **Zweig** | `story/55-fensterhuelle` |
| **Quellen & Tests** | `src/capture/capturewindow.{h,cpp}` — **ganz**, der Strang ist allein darin; `src/capture/textareaheight.{h,cpp}` **nur**, falls #56 die Höhenformel berührt (sie sollte es nicht: der Fehler sitzt im Auslöser, nicht in der Formel); `tests/capturetest.cpp` |
| **Build** | `src/CMakeLists.txt` — **nur** der `denkzettelcapture`-Block (`:41–52`, `target_link_libraries` bei `:48–52`): `KF6::Svg` und `KF6::WindowSystem` ergänzen; `CMakeLists.txt` (Wurzel) — **nur** die `find_package(KF6 …)`-Komponentenliste (`:24–33`): `Svg` aufnehmen; `tests/CMakeLists.txt` — **nur** der `capturetest`-Block (`:24–34`, Plattformthema nach 3.4) und ein **neuer** `captureshots`-Block nach dem Muster von `libraryshots` (`:65–72`) |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-06-s55-huelle/` — Bericht, eigene Bilder, Messprotokolle; **neu anzulegen: `tests/captureshots.cpp`** (es gibt heute keinen Bildläufer für das Erfassungsfenster — `searchshots`, `libraryshots` und `editshots` hängen alle an `denkzettelui`) |
| **Fachliche Quellen** | **SPEC 3** (`:110–131`) — kennt die Hülle heute nicht (AK 9); **SPEC 15** (`:591–615`) — die KF6-Liste nennt `KSvg` nicht; **SPEC 16** (`:616–640`) — der Bedingungssatz zum Plattformthema führt heute nur `shelltest` und `librarytest`; Wireframes 4a/4b als Referenz, **nicht** zu ändern |
| **Ausdrücklich nicht** | `src/ui/*` (außer #68, siehe unten), `src/shell/*`, `src/store/*`, `tests/librarytest.cpp`, `tests/libraryshots.cpp`, `tests/editshots.cpp`, `wireframes/` |

**Zwei Zeilen, die leicht übersehen werden und beide zur Pflicht gehören:**

- **Der Bildläufer `tests/captureshots.cpp` existiert nicht.** AK 8 verlangt
  Bilder je Zustand (leer · getippt · 8 Zeilen mit Scrollbalken) × 2 Farbschemata
  × 2 Desktop-Themes = **12 Bilder**. Ohne Läufer entstehen sie nicht. Er steht
  deshalb **in** der Dateimenge — das ist genau der karpathy-3.1-Befund aus
  Sprint 4, angewandt statt wiederholt.
- **#68 sprengt die Fläche.** Sein AK nennt Erfassungsfenster **und**
  Bibliothek. Wird es gezogen, kommt `src/ui/librarywindow.cpp` und
  `tests/librarytest.cpp` in die Dateimenge des Strangs. Da nur ein Strang
  läuft, ist das kein Konfliktrisiko — aber es ist der Grund, warum #68s
  Schätzung nicht am Capture-Fenster allein bemessen werden darf (2.1).

### 5.3 Zwei Festlegungen

1. **`QT_QPA_PLATFORMTHEME=kde` an `capturetest` setzt Strang A**, wann immer
   er es braucht. Es ist heute wirkungslos (3.4, 10/10 in beiden Umgebungen),
   und der Strang ist der einzige im Sprint. **Prüfmittel vor jeder Übergabe,
   beide Läufe grün:**
   ```
   QT_QPA_PLATFORM=offscreen                          ./build/bin/capturetest
   QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde ./build/bin/capturetest
   ```
2. **Keine Zahl im Code, auch nicht im Test.** AK 1 verlangt Rundung und Rand
   aus dem Theme, nicht aus einer Konstanten; 4b sagt es für den Prüfsatz
   ausdrücklich (*„Eine Zeichnung, die einen Radius festschreibt, wäre falsch"*).
   Die Zusicherung lautet **relativ** — Randmaß und Eckform unter Theme X
   unterscheiden sich von denen unter Theme Y —, nicht absolut. Meine Messung
   liefert den Grund gleich mit: `marginSize()` gibt `7,99998`, nicht `8`
   (3.2, Folgerung 2).

---

## 6. UI-Story-Einstufung (Vorschlag an den PO)

Welche Stories UI-Stories im Sinne von DoD 3 sind, legt der PO beim Planning
fest (Kundenentscheidung 31.07.2026).

| Issue | UI-Story? | Prüfmittel nach DoD 3 | Grund |
|---|---|---|---|
| **#55** | **ja, mit geteilter Bildpflicht** | *Rundung, Kontur, Fläche, Farbrollen, Maße:* eigene Bilder von `denkzettel-ux` aus dem Sprint-Stand, offscreen, 3 Zustände × 2 Farbschemata × 2 Desktop-Themes. *Schatten:* **kein Bild möglich** — benannter Ersatz, siehe 6.2 | Reiner **Zustand**: „Bei Zuständen ist das Bild der Prüfgegenstand, nicht die Zusicherung" (B3). Es ist die bildlastigste Story, die das Projekt bisher hatte |
| **#56** | **ja, mit Bildpflicht** | Zwei Bilder aus **einem** Lauf: kleine und große Systemschrift, je das ganze Fenster | Die Feldhöhe ist ein Zustand, und der Kunde hat die 3 Zeilen bei der Sprint-1-Abnahme am Bild zurückgewiesen — dasselbe Bild schließt sie wieder aus |
| **#68** *(falls gezogen)* | **ja, mit Bildpflicht** | Bilder vor/nach der Schriftänderung aus **einem** Lauf, Erfassungsfenster **und** Bibliothek | Dieselbe Bauart wie #58 in Sprint 5: geprüft wird, dass ein **stehendes** Fenster der Änderung folgt |

### 6.2 Die Konsequenz für DoD 3 — ausdrücklich, weil sie sonst später als Auslassung gelesen wird

DoD 3 sagt: *„Der UI-Review ist ohne Bild nicht geführt."* Bei #55 ist ein
Teil des Prüfgegenstands **prinzipbedingt nicht bildbar** — und das ist
gemessen, nicht behauptet (3.3: `KWindowShadow::create()` = 0 offscreen; und
`QWidget::grab()` zeichnet nur das Widget, der Schatten liegt außerhalb).

**Vorschlag zur Handhabung, in der Form, die dieses Projekt sich bei #44, #60
und #57 selbst gegeben hat:**

1. **DoD 3 gilt für #55 mit voller Bildpflicht** — für Rundung, Kontur,
   Fläche, Farbrollen und Maße. Diese fünf sind offscreen bildbar, und meine
   Messung 3.2 belegt, dass sie es sind. Hier gibt es **keinen** Nachlass.
2. **Für den Schatten tritt ein benannter Prüfmittel-Ersatz an die Stelle des
   Bildes**, zweiteilig:
   - *im Sprint:* die Zusicherung „Schatten angelegt" — dass der Code die
     Kacheln aus dem Theme zieht und `KWindowShadow` bindet, nachweisbar am
     Rückgabewert und an der Kachelquelle, **nicht** am Aussehen;
   - *in der Abnahme:* ein Bild aus der Plasma-Sitzung von PO oder Kunde.
     Das ist derselbe Foto-Punkt wie #66 in Sprint 5 — und der hat dort
     funktioniert.
3. **Der Ersatz gehört in den Review-Auftrag und in den Bericht**, nicht ins
   Stillschweigen. Sonst wird „geprüft" später als „ohne Bild geprüft" gelesen.

**Die unangenehme Stelle, die daraus folgt und benannt gehört:** Der Punkt
„der Schatten wird nach **jedem** Remap neu gebunden" (7.2) ist **weder
offscreen prüfbar noch durch einen Unit-Test gedeckt noch am Standbild
sichtbar** — er zeigt sich erst, wenn man das Fenster zweimal öffnet. Er hängt
damit vollständig am Kundenblick. **Das ist eine benannte Grenze der
Prüfbarkeit** (DoD 2: eine benannte Grenze schließt die Story nicht) — und sie
gehört wörtlich in die Abnahme-Checkliste: *„Fenster schließen, Kürzel erneut
drücken — liegt der Schatten beim zweiten Mal noch darunter?"*

---

## 7. Risiken, die diesen Schnitt kippen

1. **Alphakanal und eigenes `paintEvent`.** `WA_TranslucentBackground` ändert,
   wie das ganze Fenster gezeichnet wird — nicht nur den Rand. Ungemessen
   bleibt, ob der `QPlainTextEdit` darauf ohne eigenen Hintergrund korrekt
   zeichnet und ob die Scrollleiste des 8-Zeilen-Zustands sichtbar bleibt.
   Das ist der erste der drei Punkte, den ich **nicht** vorab messen konnte:
   er verlangt Produktivcode. **Er gehört als Erstes gebaut**, weil an ihm
   alles andere hängt.
2. **`KWindowShadow` und die Remap-Mechanik.** SPEC 3 hält fest, dass vor jedem
   Zeigen `hide()`/`show()` läuft und die Wayland-Surface dabei verschwindet
   (`capturewindow.cpp:76–89`, `RemapDelayMs = 50`). Ein Schatten, der einmal
   im Konstruktor gebunden wird, ist nach dem ersten Verstecken weg. **Die
   Bindung gehört in `present()` (`:111–116`), nach `show()`.** *Zur Genauigkeit:*
   die technische Notiz an #55 sagt, `showCapture()` zerstöre „die QWindow" —
   zerstört wird die **Surface**; das QWindow-Objekt überlebt. Für die Folgerung
   ist das gleich, für die Fehlersuche nicht. **Vom Dev am laufenden Plasma zu
   messen, offscreen geht es nicht** (3.3).
3. **Die neue Abhängigkeit `KF6::Svg`.** Sie ist vorhanden (3.2), aber an
   **drei** Stellen zu verkabeln: Wurzel-`CMakeLists.txt` (Komponentenliste),
   `src/CMakeLists.txt` (`denkzettelcapture` linkt heute nur `Qt6::Widgets` und
   `KF6::I18n` — **auch `KF6::WindowSystem` fehlt** und wird für
   `KWindowShadow` gebraucht), und **SPEC 15**. Der SPEC-Nachzug ist keine
   Kür: „fehlende Build-Abhängigkeit" ist der wörtliche Beispielfall von
   **DoD 4/B9**.
4. **Der Randfall ohne Plasma-Sitzung.** AK 9 verlangt: kein Absturz, Fenster
   bleibt nutzbar, wenn `dialogs/background` fehlt. Meine Messung sagt dazu
   **nichts** — auf dieser Maschine ist das Theme immer da. Der Fall lässt sich
   herbeiführen (`XDG_DATA_DIRS` beschneiden) und **gehört zugesichert**, sonst
   ist AK 9 eine Absichtserklärung.
5. **Neue Zusicherungen unter dem Plattformthema.** Für den *heutigen* Bestand
   ist die Frage gemessen und erledigt (3.4). Für die Zusicherungen, die der
   Sprint **erst schreibt**, gilt sie weiter — insbesondere für die Maßangaben
   aus 4b (12 px seitlich, 10 oben, 8 unten, 12 über der Fußzeile gegen 8 unter
   dem App-Namen), die von der Schrift abhängen. **Regel wie in Sprint 5: Wer
   die rote Zusicherung geschrieben hat, heilt sie.** Bei einem Strang ist das
   trivial — bis ein *bestehender* fremder Test rot wird; dann ist es ein
   Impediment an den PO, keine Eigenreparatur (melden, nicht heilen).
6. **Der `/usr`-Takt.** Alle Stories brauchen den installierten Stand: #55 für
   Hülle und Schatten am echten Plasma, #56 für die Schriftumstellung, #68 für
   den laufenden Betrieb. **Der Strang installiert nicht selbst** (DoD 2,
   Präzisierung nach Sprint-3-M1); der PO taktet, und der Endstand wird am
   Sprint-Ende einmal installiert und **der Hauptweg jeder Story daran
   ausgeführt**. *Das war der Mangel M1 aus Sprint 5* — dort belegte der
   Installationstakt den Stand, aber keinen einzigen Hauptweg. Bei #55 ist die
   Wiederholung besonders teuer: Der Schatten ist der einzige Nachweis, der
   **ausschließlich** dort zu führen ist.
7. **Eine Story trägt zwei Drittel des Sprints.** Fällt #55 in die
   Loop-Disziplin (zweimal derselbe Fehlschlag ohne neue Evidenz → Stopp und
   Impediment), fällt der Sprint, nicht ein Viertel davon. Das ist der Preis
   dieses Schnitts, und er ist bewusst gezahlt: Der Kunde hat den Befund
   zweimal angemahnt.
8. **Bilderzahl.** AK 8 ergibt 3 Zustände × 2 Schemata × 2 Themes = **12
   Bilder** vom Strang, dazu die eigenen Bilder des UI-Reviews (Sprint 5: 20).
   Der Läufer muss das Desktop-Theme **im Lauf** wechseln können — meine Sonde
   zeigt, dass das geht (`ImageSet::setImageSetName()` je Durchgang, 3.2), aber
   es ist eine Bauanforderung an `captureshots.cpp` und keine Selbstverständlichkeit.

---

## 8. Klärungspunkte vor dem Ziehen — mit Vorschlag

Die AK- und Label-Anpassungen macht der PO; der Scrum Master legt den Wortlaut
vor.

**K1 — `sp:8` an #55, und die Label-Disziplin als Dauerpflicht.**
#55 trägt kein `sp:`-Label, obwohl die Schätzung seit dem 02.08.2026 im
Issue-Text steht. Das ist **M2 aus Sprint 5**, an der nächsten Story wiederholt.
*Vorschlag:* `sp:8` an #55 setzen, **vor** der Freigabe. Und: Seit
Abschluss-Punkt 12 ist das Label die Quelle des **Endwerts** der Schätzhistorie
(Sprint 5, §24) — ein fehlendes Label erzeugt jetzt eine Lücke in der
Datenreihe, nicht nur eine unsaubere Rückverfolgung. Wird ein Issue geschätzt,
wird im selben Zug das Label gesetzt.

**K2 — Schätzungen für #56 und #68 einholen (blockierend).**
Ohne beide ist **kein zulässiger Sprint schneidbar** (1). Der PO holt sie nach
dem üblichen Muster: zwei unabhängige Schätzer, kein Blick auf den Alt-Wert.
*Vorschlag zur Reihenfolge:* zuerst #68 — sein Wert entscheidet, ob der Sprint
drei oder zwei Stories hat.

**K3 — #68 AK 2: die offene Frage ist entscheidbar, und zwar jetzt.**
Das AK sagt selbst: *„löst #56 mit oder baut darauf auf — im Planning
entscheiden."*
*Vorschlag:* **#68 baut auf #56 auf, #56 wird zuerst gebaut.** Begründung steht
in #56 selbst: Ohne die Höhenkorrektur liefert #68 die Verschlechterung mit.
Und #56 ist ohne #68 prüfbar — sein AK 2 legt den Weg fest (Schrift des Widgets
direkt setzen). Das AK von #68 entsprechend fassen: *„setzt #56 voraus"*.

**K4 — #55 AK 8: den Schatten-Ersatz in den Review-Auftrag schreiben.**
*Vorschlag:* Wie in 6.2 festlegen — volle Bildpflicht für die fünf bildbaren
Sätze, benannter Ersatz für den Schatten, und die Remap-Grenze wörtlich in die
Abnahme-Checkliste. Ohne diese Festlegung prüft `denkzettel-ux` gegen ein Bild,
in dem der Schatten gar nicht auftreten **kann** — *„Ein Testaufbau, in dem der
Fehler gar nicht auftreten kann, ist kein Test"* (`CLAUDE.md`).

**K5 — #70: die Produktentscheidung bei der Freigabe mitnehmen.**
#70 ist nicht ziehbar, weil die AK offen sind, und wird es ohne Kundenwort auch
nicht. Die Freigabe ist der billigste Moment.
*Vorschlagstext für die Frage an den Kunden:* „Wer mit der Pfeiltaste zur
ersten Notiz eines Tages geht — soll die Liste den Tageskopf mit ins Bild
holen (führt, bewegt aber das Bild), oder stehenbleiben (ruhig, aber der Tag
ist nur rechts im Detailbereich zu sehen)?" **Kein Scope für Sprint 6** — nur
die Antwort, damit #70 für Sprint 7 schätzbar wird.

**K6 — die Eckstück-Messung als Beleg sichern.**
Der Beleg, auf den die AK-1-Richtigstellung verweist, deckt ihre
Eckstück-Aussage nicht (3.1). Meine Messung deckt sie und steht in diesem
Protokoll.
*Vorschlag:* Wenn der Strang für seine Prüfläufe ohnehin misst, legt er die
Ausgabe als `docs/scrum/reviews/sprint-06-s55-huelle/theme-eckstuecke.txt` ab.
Damit hängt die Aussage nicht mehr an einem Planning-Absatz. *Kein Mangel*,
eine Sicherung.

**K7 — Basis-Tag.**
Vor dem Strang-Spawn setzt der PO `sprint-06-basis` auf den Ausgangsstand
(`PROZESS.md`, Sprint-Mechanik). Er trägt den Prüf-Diff des Sprint-Endes.

**K8 — Milestone „Sprint 6"** anlegen und die gezogenen Issues zuordnen.

---

## 9. Schätzhistorie — die Spalten der gezogenen Stories (Abschluss-Punkt 12)

**Neu ab diesem Sprint:** Die Tabelle wird **beim Planning** angelegt, nicht
erst in der DoD-Prüfung. Grund: Sie ist die Quelle für Abschluss-Punkt 12, und
wer sie erst am Ende schreibt, rekonstruiert die Erstschätzung aus dem
Gedächtnis. Die Werte unten sind der Stand **vor** der Umsetzung; die
DoD-Prüfung prüft sie nach und trägt Revisionen und das endgültige
Anlass-Kennzeichen ein.

**Festlegungen aus Sprint 5, §24, hier angewandt:** Erstschätzung ist die erste
konsolidierte Schätzung, Provenienz wird mitgeführt und nicht geglättet,
**Endwert ist das `sp:`-Label**, Abstand ist die Zahl der Sprints zwischen
Erstschätzung und Umsetzung, Faktor ist Endwert ÷ Erstwert. Abstand und Faktor
stehen **ausgerechnet** in der Tabelle — der Verwalter überträgt, er rechnet
nicht.

| Story | Issue | Erstschätzung | Quelle | Revision | End | Umsetzung | Abst. | Faktor | Anlass |
|---|---|---|---|---|---|---|---|---|---|
| Capture-Fensterhülle | **#55** | **8** · 02.08.2026 | Sprint-5-Planning §2 (Dev 8 · UX 8, deckungsgleich) — **2 unabhängige** | *keine bisher* | 8 | Sprint 6 | **1** | **1,00** | `keine` |
| Feldhöhe nach Schriftänderung | **#56** | **1** · 01.08.2026 | Label bei Anlage — **1 Hand** | *ausstehend (K2)* | *offen* | Sprint 6 | **3** | *offen* | *offen* |
| Schrift folgt zur Laufzeit | **#68** | *ausstehend (K2)* | Sprint-6-Planning — 2 unabhängige | — | *offen* | Sprint 6 | **0** | 1,00 | `keine` — **24.2-Fall** |

**Drei Anmerkungen, die das Urteil tragen und die der Verwalter später nicht
selbst fällen darf:**

- **#55 geht in die Kurve, und das ist neu.** Seine Erstschätzung fiel im
  **Sprint-5**-Planning, die Umsetzung liegt in Sprint 6 — zwischen beiden liegt
  genau ein Ereignis, bei dem eine Revision hätte stattfinden können: dieses
  Planning hier. Damit ist der §24.1-Maßstab erfüllt. **Abstand 1 bekommt seinen
  vierten Punkt**, und zum ersten Mal einen aus einer 8er-Story.
- **#56 hat Abstand 3.** Angelegt am 01.08.2026, also während Sprint 3;
  umgesetzt in Sprint 6. Die Zählweise ist die von Sprint 5 (#57 und #58, beide
  01.08. angelegt, Umsetzung Sprint 5, dort Abstand 2). **Das wäre der bislang
  weiteste Abstand der Reihe** — bisher reicht sie bis 3 (#11, #12), und beide
  Punkte dort stammen aus der Schätzklausur. Ein Punkt mit Provenienz „1 Hand"
  bei Abstand 3 ist deshalb inhaltlich interessant, wie auch immer er ausfällt.
- **#68 ist ein 24.2-Fall.** Erstschätzung und Umsetzung fallen in dasselbe
  Planning; sein Faktor ist 1,00 von Konstruktion wegen und keine Messung. Er
  wird **erfasst, aber nicht gezeichnet** — damit die Auslassung sichtbar bleibt.

**Was die DoD-Prüfung am Sprint-Ende nachzutragen hat:** die tatsächlichen
Endwerte aus den `sp:`-Labeln, jede Revision im Sprint mit Datum und Fundstelle,
und das endgültige Anlass-Kennzeichen je Zeile. Fällt #68 aus dem Schnitt,
entfällt seine Zeile ersatzlos — eine nicht umgesetzte Story hat keinen
Umsetzungssprint.

---

## 10. Hinweise an den Product Owner

1. **Zwei Schätzungen sind blockierend** (K2): #56 und #68. Ohne sie ist der
   Sprint nicht schneidbar, weil #55 allein die Story-Untergrenze
   unterschreitet.
2. **`sp:8` an #55 vor der Freigabe** (K1) — und ab jetzt Label im selben Zug
   wie die Schätzung, weil die Schätzhistorie daran hängt.
3. **Der Schnitt geht dem Kunden zur Freigabe** (Freigabemodell,
   Kundenentscheidung 31.07.2026). Vorzulegen sind Sprint-Ziel (4.1), die
   gezogenen Issues (4.2) und **ausdrücklich der Kontostand** samt der
   Feststellung, dass die Luft Vorsorge für eine 8er-Risikostory ist und kein
   Leerlauf.
4. **UI-Story-Einstufung ist PO-Entscheidung** (6) — sie muss **vor** dem Spawn
   fallen, besonders die geteilte Belegform bei #55 (6.2).
5. **Ein Dev-Agent, nicht zwei** (5.1), und der `/usr`-Takt gehört als Verbot
   mit Begründung in den Spawn-Auftrag, nicht als Hinweis (Risiko 6).
6. **Vier Punkte für den Spawn-Auftrag, die aus meinen Messungen stammen** und
   dem Strang je einen Fehlversuch ersparen: `marginSize()` liefert `7,99998`
   statt `8` (3.2); `KWindowShadow::create()` ist offscreen immer `false`, das
   ist kein Fehler des Codes (3.3); `KSvg` findet das Theme offscreen
   vollständig, der Bildnachweis trägt (3.2); die Schattenbindung gehört in
   `present()` nach `show()`, nicht in den Konstruktor (Risiko 2).
7. **#70 bei der Freigabe mitentscheiden lassen** (K5) — ein Satz vom Kunden,
   und das Issue wird für Sprint 7 schätzbar.
8. **#61 für Sprint 7 vormerken** (4.4) — Abschluss-Punkt 10 bleibt bis dahin
   ausgesetzt, und die Aussetzung sollte nicht zum Dauerzustand werden.
9. **Retro nach Sprint 6 ist fällig** (Kadenz: Sprint 3, 6, 9, …). Kandidaten
   liegen vor: die vier aus Sprint 4 §17.6 und der B13-Nachzug der erweiterten
   Dateimengen-Notation aus Sprint 5 §5.1. **Dazu neu aus diesem Planning:** die
   Frage, ob die Label-Pflicht nach der Schätzung in `PROZESS.md` gehört — sie
   ist jetzt zweimal in Folge gerissen worden (M2 in Sprint 5, K1 hier).

---

## 11. Was dem Kunden zur Entscheidung vorliegt

1. **Sprint-Ziel und Schnitt** — das Erfassungsfenster bekommt die Hülle seines
   Desktop-Themes (#55) und behält seine fünf Zeilen bei größerer Systemschrift
   (#56); je nach Schätzung kommt hinzu, dass Schriftänderungen im laufenden
   Betrieb ankommen (#68).
2. **Der Sprint hat einen Gegenstand, nicht vier.** #55 trägt allein 8 der 9
   bis 12 Punkte. Das ist Absicht — der Befund ist zweimal angemahnt worden —,
   und es heißt: Fällt diese Story, fällt der Sprint.
3. **Bewusst nicht gefüllt.** Zwei bis drei von vier zulässigen Stories, 9 bis
   12 von ~13 Punkten. Die Luft ist Vorsorge für Review-Auflagen an der größten
   Story, die dieses Projekt bisher hatte. Wer das Konto voller will: #59
   (ruhige Liste bei Fensteraktivierung) wäre der Zugang — er kostet die
   Klammer des Sprint-Ziels.
4. **Ein Nachweis kann nur vom Kunden kommen.** Der **Schatten** ist am
   Prüfrechner grundsätzlich nicht abzubilden — gemessen, nicht vermutet.
   In der Abnahme steht deshalb wieder ein **Foto-Punkt**, wie beim
   Wächterdialog in Sprint 5, und dazu die Frage: Liegt der Schatten auch beim
   **zweiten** Öffnen des Fensters noch darunter?
5. **Eine kleine Entscheidung nebenbei** (K5): Soll die Liste beim Wandern mit
   der Pfeiltaste den Tageskopf ins Bild holen oder stehenbleiben? Kein Scope
   für diesen Sprint — die Antwort macht #70 für den nächsten schätzbar.

---

## 12. done / next

**done:** Sprint-6-Planning moderiert und vorgelegt — die Schätzlage aller neun
Kandidaten am Backlog erhoben statt aus den Issue-Texten übernommen und
festgestellt, dass **nur zwei** Issues die Schätzregel erfüllen und **nur eines**
davon sein Label trägt; das Sprint-Konto ab Zeile 1 geführt und **acht
Kombinationen gegen beide Grenzen durchgerechnet**, statt nur die Punkte zu
addieren; **vier eigene Messungen gefahren statt Risiken abzuwägen** — die
Eckstück-Aussage der AK-1-Richtigstellung auf zwei unabhängigen Wegen bestätigt
**und ihren fehlenden Beleg benannt**, `KSvg` als offscreen voll auflösend
nachgewiesen (damit trägt der Bildnachweis aus AK 8), `KWindowShadow::create()`
offscreen als `false` bestätigt (damit ist die geteilte Belegform Messung statt
Vorsicht), und `capturetest` in beiden Umgebungen gefahren (10/10 und 10/10 —
das Plattformthema kostet heute nichts); die Kollisionsfläche am Code vermessen
und daraus **einen** Strang abgeleitet statt zwei (kleinster Bereichsabstand:
null, dieselbe 157-Zeilen-Datei und dieselbe 38-Zeilen-Kopfdatei); die
Dateimenge um den **nicht existierenden Bildläufer** `tests/captureshots.cpp`
und die drei Verkabelungsstellen von `KF6::Svg` ergänzt; die Konsequenz der
nicht bildbaren Schatten für DoD 3 ausformuliert samt der Grenze, die
ausschließlich am Kundenblick hängt; acht Risiken und acht Klärungspunkte mit
Wortlautvorschlag benannt; die **Schätzhistorie erstmals beim Planning** angelegt
(Abschluss-Punkt 12) — mit der Feststellung, dass #55 in die Kurve geht und #56
den bislang weitesten Abstand der Reihe trägt.

**next:** (1) PO holt die zwei blockierenden Schätzungen (#56, #68) — der Wert
von #68 entscheidet zwischen zwei und drei Stories. (2) PO setzt `sp:8` an #55.
(3) PO legt die UI-Story-Einstufung fest, besonders die geteilte Belegform bei
#55 (6.2), und arbeitet K3, K4 und K6 in die Issues ein. (4) PO legt Sprint-Ziel
und Schnitt dem Kunden zur Freigabe vor, **mit dem Kontostand** und der Frage
aus K5 (#70). (5) Nach der Freigabe: Milestone „Sprint 6", `sprint-06-basis`,
dann Spawn **eines** Strangs nach 5.2, in der Reihenfolge #56 → #55 → #68.
(6) Freigabe-Stand in die Kontotabelle (1) eintragen — dieser Punkt ist in
Sprint 5 offen geblieben (dort §19). (7) Vor dem Sprint-7-Planning zu heilen:
die drei Klärungen und die zweite Schätzung zu #69, unverändert offen seit
Sprint 5 §2.4. (8) **Retro nach diesem Sprint** (Kadenz) — Kandidaten in 10,
Punkt 9.
