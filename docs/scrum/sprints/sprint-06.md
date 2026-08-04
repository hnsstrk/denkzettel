# Sprint 6 — Planning-Protokoll

**Datum:** 2026-08-02, 19:40 (Ganymed), **nachgezogen 20:03** ·
**Moderation:** Scrum Master (Agent `scrum-master`, frischer Kontext)
**Teilnehmer:** Scrum Master · Product Owner · Schätzer Dev.
**Status des Sprint-Vorschlags:** **festgeschrieben**, Freigabe durch den Kunden
steht aus.

**Zwei Durchgänge, und der Unterschied ist protokolliert.** Der erste Durchgang
(19:40) hat den Schnitt **unter Vorbehalt** vorgelegt: Von acht Kandidaten
erfüllten nur zwei die Schätzregel, und ob der Sprint zwei oder drei Stories
trägt, hing an der ausstehenden Schätzung für #68. Der zweite Durchgang (20:03)
schreibt ihn fest, nachdem die beiden blockierenden Schätzungen vorlagen (2.1).
**Was der Vorbehalt vorab benannt hatte, ist eingetreten:** #68 kam mit **5 SP**
und ist damit nach der eigenen Bedingung dieses Plannings draußen — der Schnitt
lautet **#55 + #56, 9 SP, zwei Stories**. Die Bedingung stand **vor** der Zahl
im Protokoll; sie ist nicht nachträglich um das Ergebnis herumgelegt worden.

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
| Kandidatenfeld (#55, #56, #68, #59, #70, #71, #72, #61) | 8 | **nicht summierbar** — nur #55 (8) und #59 (2) waren regelkonform geschätzt | Story-Grenze doppelt gerissen |
| Nach Ausschluss des nicht Ziehbaren (#70 AK offen, #71/#72/#61 ungeschätzt) | 4 | 8 + 1 + 2 + *#68 offen* | Story-Grenze **gehalten, am Anschlag** |
| Vorschlag des Scrum Masters, 19:40 (2, falls #68 ≤ 3: 3) | 2–3 | 9, mit #68 12 | beide gehalten |
| **Festgeschriebener Schnitt, 20:03** (#68 mit 5 SP draußen) | **2** | **9** | **beide gehalten** — 2 Stories und 4 SP Luft |
| **Freigabe durch den Kunden, 04.08.2026** — **#59 zugezogen** | **3** | **11** | **beide gehalten** — 1 Story und 2 SP Luft |

**Zur Freigabe (04.08.2026):** Der Kunde hat den festgeschriebenen Schnitt um
**#59** (ruhige Liste bei Fensteraktivierung, `sp:2`) erweitert. Der Zugang ist
hier gebucht (B12); **beide Grenzen halten** — 3 von 2–4 Stories, 11 von ~13
Punkten. Er ist damit **keine Grenzüberschreitung** und musste dem Kunden nicht
gesondert vorgelegt werden; er *ist* die Kundenentscheidung.

**Zwei Folgen, die das Planning für genau diesen Fall benannt hat** (11, Punkt 3):
Das Sprint-Ziel verliert seine Klammer — #59 gehört zur Bibliothek, nicht zur
Fensterhülle —, und Sprint 7 verliert einen seiner vier Kandidaten. Beides war
vorher aufgeschrieben und ist mit der Freigabe angenommen worden.

**Die Luft schrumpft von 4 auf 2 Punkte.** Sie war als Vorsorge für
Review-Auflagen an #55 gedacht, der größten Story dieses Projekts. Reißt der
Sprint, ist dies die Stelle, an der es begann — festgehalten vor dem Ereignis,
nicht danach.

**Der erste Befund des Kontos war, dass es sich nicht führen ließ** — und er ist
inzwischen zur Hälfte geheilt. Von acht Kandidaten trug am 19:40 genau **einer**
eine regelkonforme Schätzung, die auch am Backlog stand (#59, `sp:2`). Der Kern
des Sprints (#55) war deckungsgleich mit 8 SP geschätzt, aber **ohne
`sp:`-Label** — derselbe Mangel, den ich in Sprint 5 als **M2** gemeldet habe,
an der nächsten Story wiederholt. Seit Abschluss-Punkt 12 gilt: *„Endwert ist
das `sp:`-Label des Issues"* (Sprint 5, §24). Ein fehlendes Label ist damit
nicht mehr nur eine Frage der Rückverfolgbarkeit, sondern eine **Lücke in der
Datenreihe des Schätzkegels**.

**Stand 20:03, von mir am Backlog nachgemessen** (`gh issue view`, nicht aus der
Meldung übernommen): **#55 `sp:8` · #56 `sp:1` · #68 `sp:5`** — alle drei
gesetzt. K1 und K2 sind damit erledigt (8). **Der Befund bleibt trotzdem
stehen**, weil er zweimal in Folge eingetreten ist und die Heilung beide Male
eine Nachforderung war, keine Regel: Sprint 5 M2, Sprint 6 K1. Er gehört in die
Retro (10, Punkt 9).

**Die Rechnung, was neben die 8 passt** — beide Grenzen zugleich, nicht nur die
Punkte (das war die Blindstelle, gegen die B12 gefasst ist):

| Kombination | Issues | SP | Urteil |
|---|---|---|---|
| #55 allein | 1 | 8 | **Story-Grenze unterschritten** (Minimum 2) — kein zulässiger Sprint |
| **#55 + #56** | **2** | **9** | hält; 2 Stories und 4 SP Luft — **← eingetreten** |
| #55 + #59 | 2 | 10 | hält; Thema gerissen (Bibliothek statt Erfassungsfenster) |
| #55 + #56 + #68, falls #68 ≤ 3 | 3 | ≤ 12 | hätte gehalten; 1 Story und ≥ 1 SP Luft |
| **#55 + #56 + #68, bei #68 = 5** | 3 | **14** | **SP-Grenze gerissen — der gemessene Fall** |
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

**Stand 20:03.** Die beiden blockierenden Schätzungen liegen vor; die Zeilen sind
nachgezogen und die Label von mir am Backlog nachgemessen.

| Issue | Vorliegende Schätzung | Quelle | Schätzregel erfüllt? | `sp:`-Label | Ziehbar? |
|---|---|---|---|---|---|
| **#55** | Dev 8 · UX 8 | Sprint-5-Planning §2, 02.08.2026 | **ja** — 2 unabhängige, deckungsgleich | `sp:8` ✓ *(20:03 gesetzt)* | **ja — gezogen** |
| **#56** | Label 1 · **Dev 1** | Anlage 01.08.2026 (1 Hand) + Zweitschätzung 02.08.2026 | **ja** *(seit 20:03)* — deckungsgleich | `sp:1` ✓ | **ja — gezogen**, unter einer Bedingung (2.3) |
| **#68** | **Dev 5** | Zweitschätzung 02.08.2026, am Code gemessen | *einseitig, aber entscheidungsreif* (2.4) | `sp:5` ✓ *(20:03 gesetzt)* | **nein** — 14 SP rissen die Punktgrenze |
| **#59** | Dev 2 · UX 2 · Label 2 | Sprint-5-Planning §2, 02.08.2026 | **ja** — dreifach deckungsgleich | `sp:2` ✓ | ja, aber außerhalb der Klammer (4.3) |
| **#71** | — | — | **nein** | keines | nein |
| **#72** | — | — | **nein** | keines | nein |
| **#61** | — | — | **nein** | keines | nein |
| **#70** | — | — | **nein** | keines | **nein, doppelt** — die AK sind offen (Produktentscheidung) |
| **#69** | — | drei Klärungen offen | **nein** | keines | **nein** — ausdrücklich nicht für Sprint 6 |

### 2.1 Die beiden blockierenden Schätzungen — angefordert 19:40, eingetroffen 20:03

**K2 ist erledigt.** Angefordert waren zwei Schätzungen, ohne die kein
zulässiger Sprint schneidbar war (#55 allein unterschreitet die
Story-Untergrenze). Beide liegen als Issue-Kommentar am Backlog — der einzigen
Quelle der Wahrheit —, beide mit Begründung am Code:

| Issue | Angefordert | Eingetroffen | Wirkung auf den Schnitt |
|---|---|---|---|
| **#68** | zuerst, weil sein Wert die Story-Zahl entscheidet | **5 SP** | **draußen** — 14 SP rissen die Punktgrenze |
| **#56** | Erwartungswert klein, Urteil bei den Schätzern | **1 SP bestätigt** | **drin**, unter der Bedingung aus 2.3 |

**Die empfohlene Reihenfolge hat getragen.** #68 zuerst zu schätzen war der
Vorschlag von 19:40, und genau dieser Wert hat den Sprint von drei auf zwei
Stories festgelegt. Wäre #56 zuerst gekommen, wäre nichts entschieden gewesen.

**Die Messbelege des Devs liegen versioniert unter
`docs/scrum/reviews/sprint-06-schaetzung/`** — vier Sonden mit Quelltext,
Ausgabe und einem Skript `pruefen.sh`, das sie wiederholbar macht; committet mit
`573901e`, gemerged mit `5982327`. Ich gebe die Zahlen hier nicht doppelt
wieder; die tragenden Sätze stehen in den Issue-Kommentaren an #56 und #68.

**Von mir geprüft, nicht geglaubt** (`git ls-files`): Die vier `*.txt`, die vier
`*.cpp`, `LIESMICH.md`, `CMakeLists.txt` und `pruefen.sh` sind im Repo; nur der
Bauplatz `build/` ist ausgeschlossen. **Damit ist B7 erfüllt** — und zwar in der
stärkeren Form, die dieses Projekt sonst nur bei Prüfläufen kennt: Der Beleg ist
nicht nur abgelegt, er ist **nachfahrbar**. *Zur Redlichkeit:* Als ich diesen
Absatz um 20:03 zuerst schrieb, lag der Ordner noch nicht im Repo, und ich habe
ihn als offenen Prüfpunkt (K9) hingeschrieben statt als erledigt zu behaupten.
Er ist es inzwischen; K9 ist damit geschlossen, nicht weggefallen.

### 2.2 Keine 13er-Story, aber die erste 8er

**#55 ist mit 8 SP die größte Story des Projekts.** Sie ist nicht
teilungsbedürftig im Sinne der Regel (die greift bei 13), aber sie ist der
erste Fall, in dem eine einzige Story **zwei Drittel des Sprint-Budgets** trägt
— nach dem Ausscheiden von #68 sind es sogar **acht von neun Punkten**. Das
gehört ausgesprochen, weil es die Bauart des Sprints bestimmt: Dieser Sprint
hat einen Gegenstand und ein kleines Beiwerk, nicht zwei gleichrangige Stories.

### 2.3 Die Bedingung, unter der die 1 an #56 hält

Der Zweitschätzer bindet seine 1 an eine Festlegung dieses Plannings: **Der
Bildläufer `tests/captureshots.cpp` bleibt in der Dateimenge von #55 gebucht**
(5.2). Würde die Bildpflicht aus DoD 3 auf #56 gebucht, zahlte #56 den Läufer
und wäre eine **2** — der Sprint stünde dann bei 10 SP.

**Die Bedingung ist erfüllt und bleibt es**, aus einem Grund, der unabhängig von
der Schätzung trägt: #55 braucht den Läufer für **zwölf** Bilder (3 Zustände ×
2 Farbschemata × 2 Desktop-Themes, AK 8), #56 für **zwei** (zwei Schriftgrößen).
Wer den Läufer der Story anlastet, die zwei Bilder braucht, statt der, die zwölf
braucht, bucht ihn falsch.

**Prüfsatz für die DoD-Prüfung:** Kippt diese Zuordnung im Sprint, ist #56
nachzuschätzen und die Änderung im Sprint-Konto zu buchen. Eine Bedingung, an
der eine Schätzung hängt, gehört mitgeprüft — sonst ist sie eine Fußnote, die
niemand liest.

### 2.4 #68 trägt `sp:5` aus **einer** Hand — das gehört Sprint 7 gesagt

Die 5 stammt vom Dev, am Code gemessen und ausführlich begründet. Sie ist
**entscheidungsreif genug, um #68 auszuschließen**: Der Ausschluss braucht nur
die Aussage „größer als 3", und die trägt eine Hand allemal — zumal sie in die
teure Richtung zeigt und ihre Begründung den Gegenstand ausweitet (drei
Fundstellen in zwei Bibliotheken, eine fehlende Verkabelung, eine offene
Entwurfsentscheidung zu den inline erzeugten Labels).

**Sie ist aber keine regelkonforme Schätzung.** Die Sprint-Mechanik verlangt
zwei unabhängige Schätzer, und das Label `sp:5` steht seit 20:03 am Issue.
**Damit droht in Sprint 7 genau der Fehler, den Sprint 5 an #57 gemacht und
korrigiert hat:** Ein Label wird für eine Schätzung gehalten, weil es am Backlog
steht. Es ist keine — die Provenienz ist *„1 Hand (Dev, am Code gemessen,
02.08.2026)"*, nicht *„Planning, 2 unabhängige"*, und Sprint 5 §24 verlangt, die
Unterscheidung **mitzuführen und nicht einzuebnen**. Vor dem Ziehen von #68 ist
eine zweite Schätzung einzuholen; sie kann die 5 bestätigen oder ersetzen
(next, Punkt 7).

**Nicht angefragt und warum:** #71 und #72 sind Bibliotheksbefunde aus dem
Sprint-5-UI-Review. Ihre Schätzung anzufordern hätte nur Sinn, wenn sie in den
Schnitt könnten — beide reißen die Klammer, und die Punkte trügen sie ohnehin
nicht (1). **#61** ist mit dem Wegfall von #68 ebenfalls nicht mehr angefragt
(4.4). Alle drei bleiben im Backlog; Priorisierung ist PO-Sache.

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
> Kontur und Schatten seines Desktop-Themes.**

Nachprüfbar in zwei Handgriffen ohne Werkzeug:

1. Kürzel drücken — **die Ecken des Fensters sind rund**, und es liegt ein
   Schatten darunter wie unter einem Plasma-Popup (#55). Das ist der Satz, den
   der Kunde zweimal angemahnt hat.
2. Desktop-Theme wechseln, **ohne** den Dienst neu zu starten — die Hülle
   wechselt mit: schmaler flacher Bogen bei Breeze, breiter runder bei
   CachyOS-Nord-round (#55, AK 7).
### 4.1.1 Warum #56 **nicht** im Sprint-Ziel steht — Korrektur am eigenen Entwurf

Der erste Durchgang (19:40) trug im Ziel den Halbsatz *„und es behält seine fünf
Zeilen, wenn der Nutzer die Systemschrift ändert"* und dazu einen dritten
Handgriff *„Systemschrift vergrößern — das Feld bleibt fünf Zeilen hoch"*.
**Beides ist falsch, und der Wegfall von #68 macht es sichtbar.**

**Der Kunde kann #56 nicht nachprüfen.** Der Grund steht in #56 selbst: *„Der
Fehler ist zurzeit nicht sichtbar, weil Plasma Schriftänderungen an
Qt-Widgets-Anwendungen ohnehin nicht nachreicht (B6)."* Wer nach Sprint 6 die
Systemschrift ändert, sieht am stehenden Fenster **gar nichts** — weder den
Fehler noch die Heilung. Ein frisch gestarteter Dienst hätte die richtige Höhe
schon heute. Genau diese Zustellung nachzurüsten ist **#68**, und #68 ist
draußen.

**Ein Sprint-Ziel, das eine Nachprüfbarkeit behauptet, die es nicht gibt, ist
schlechter als ein schmales Ziel.** Es schickt den Kunden in einen Handgriff,
der nichts zeigen kann — *„Ein Testaufbau, in dem der Fehler gar nicht auftreten
kann, ist kein Test"* (`CLAUDE.md`); für eine Abnahme-Checkliste gilt das
genauso. Im ersten Durchgang stand der Halbsatz nur deshalb da, weil #68
danebenstand und die Lücke zudeckte.

**Was #56 stattdessen ist: eine vorgezogene Heilung, und sie gehört genau
hierher.** Das Issue begründet es selbst — *„Heute verdeckt, morgen akut. Sobald
jemand das nachrüstet, tritt dieser Fehler sofort zutage. Er gehört deshalb
vorher behoben, nicht nachher: Sonst baut man eine Verbesserung und liefert die
Verschlechterung mit."* Da #68 als Kern von Sprint 7 empfohlen ist (4.3), ist
Sprint 6 der letzte Sprint, in dem diese Reihenfolge kostenlos zu haben ist.

**Wie #56 belegt wird, wenn nicht am Kundenblick:** durch Test und Bild — sein
AK 2 legt den Weg selbst fest (die Schrift des Widgets direkt setzen und die
Höhe gegen den Zeilenabstand messen), AK 3 verlangt zwei deutlich verschiedene
Schriftgrößen. **Das ist eine benannte Grenze der Prüfbarkeit** (DoD 2: eine
benannte Grenze schließt die Story nicht) — und sie gehört in die
Abnahme-Vorlage, damit der Kunde nicht nach etwas sucht, das er nicht finden
kann.

**Warum diese Klammer und keine weitere.** Das Ziel deckt genau die Stories, die
darunter fallen. Ein Bibliotheksbefund wie #59 oder #71 wäre unter dieser
Klammer nicht nachprüfbar — er würde sie zu *„Denkzettel verhält sich wie eine
KDE-Anwendung"* verwässern, also zum Sprint-5-Ziel zurück. Der Sprint-5-Maßstab
gilt hier gegen die eigene Versuchung, das Konto zu füllen: *„der Sprint hätte
kein Thema mehr, sondern ein Vorhaben"* (Sprint 5, §4.1).

### 4.2 Der Schnitt — festgeschrieben 20:03

| Strang | Issue | Story | SP |
|---|---|---|---|
| A | **#55** | Capture-Fensterhülle — Rundung, Kontur, Schatten aus dem Theme | **8** |
| A | **#56** | Feldhöhe folgt einer Schriftänderung nicht | **1** |
| | | **Summe** | **9** |

**Zwei Stories, 9 von ~13 SP — beide Grenzen gehalten, mit Luft.** #68 ist mit
5 SP ausgeschieden (2.1); der Sprint wäre mit ihm bei 14 SP gelandet.

**Ein Strang, nicht zwei — und das ist ein Befund, kein Versäumnis.** Dazu 5.

**Reihenfolge innerhalb des Strangs: #56 → #55.** Nicht Ermessen, sondern aus
den Issues abgeleitet:

- **#56 zuerst**, weil es die kleinste und am genauesten bekannte Änderung ist
  und dieselbe Datei anfasst. Wer zuerst die 1 baut, hat einen grünen Stand,
  gegen den er die 8 misst. Der Zweitschätzer nennt dieselbe Reihenfolge in
  seiner Bedingung (2.3).
- **#56 überhaupt in diesem Sprint**, weil es #68 vorausgeht: *„Sobald jemand
  das nachrüstet, tritt dieser Fehler sofort zutage. Er gehört deshalb vorher
  behoben, nicht nachher."* Mit #68 als Sprint-7-Kern (4.3) ist das jetzt der
  richtige Zeitpunkt — die Reihenfolge ist hier noch kostenlos, in Sprint 7
  nicht mehr.

### 4.3 Was draußen bleibt und warum

- **#68 (5 SP)** — die Schätzung hat es ausgeschlossen, und zwar nach der
  Bedingung, die vor der Zahl im Protokoll stand. **Der Dev empfiehlt es als
  Kern eines Bibliotheks-Sprints 7 neben #59, #71 und #72**, und die Empfehlung
  trägt: Seine Begründung (Belege in `docs/scrum/reviews/sprint-06-schaetzung/`,
  Kommentar an #68) zeigt, dass der Gegenstand zum größeren Teil **nicht** im
  Erfassungsfenster liegt, sondern in `src/ui/` — Delegate, Gruppenüberschriften
  und zwölf Labels mit gesetzter Schrift. **Damit ist #68 keine
  Capture-Story, die man an #55 anhängt, sondern die Klammer eines eigenen
  Sprints.** Das ist ein Fund der Schätzung, den das Planning von 19:40 nicht
  hatte: Dort stand nur die Vermutung, #68 sei „nicht capture-only".
- **#59 (2 SP, geschätzt und ziehbar)** — das einzige weitere Issue des Feldes,
  das sofort ziehbar wäre. Es bleibt draußen, weil es die Klammer reißt
  (Bibliothek statt Erfassungsfenster) — und es ist nach der #68-Schätzung
  ohnehin besser in Sprint 7 aufgehoben, wo es mit #68, #71 und #72 dieselbe
  Fläche teilt.
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

### 4.4 Die vier freien Punkte — bewusst nicht gefüllt

Der Sprint steht bei **9 von ~13 SP und 2 von 4 Stories**. Der eingetretene Fall
ist der, den ich um 19:40 für ihn empfohlen habe; die Begründung ist unverändert
und wird durch die #68-Schätzung eher stärker:

**Die Luft ist Vorsorge, kein Leerlauf.** #55 ist die erste 8er des Projekts,
trägt **acht von neun Punkten** des Sprints, hat drei ungemessene Punkte (7),
und ihr Schatten-Nachweis hängt an einem Kundenfoto (6.2). Sprint 2 erzeugte aus
einem einzigen UI-Review drei Auflagen, Sprint 3 einen `fail` und einen `warn`.
Ein Sprint, dessen Konto beim Start ausgeschöpft ist, verträgt keine davon.

**Die beiden Wege, das Konto trotzdem zu füllen — und warum ich abrate:**

1. **#59 als dritte Story** (11 SP / 3 Stories). Es kostet die Klammer des
   Sprint-Ziels, und seit der #68-Schätzung kostet es mehr: #59 gehört
   fachlich zu dem Bibliothekspaket, das der Dev für Sprint 7 vorschlägt
   (#68, #59, #71, #72). Es jetzt einzeln herauszubrechen, zerlegt genau das
   Paket, das nächsten Sprint die Klammer bilden soll.
   **Gegenargument, das nicht trägt:** #59 läge in einer vollständig disjunkten
   Fläche und wäre der einzige Kandidat für einen zweiten Strang — aber
   *„Sprints sind Arbeitspakete, keine Zeiträume"* (PROZESS.md). Parallelität
   ist in diesem Projekt kein Wert an sich.
2. **#61** ist von anderer Art: Es füllt das Konto nicht besser als #59, löst
   aber eine Prozessschuld (Abschluss-Punkt 10 ausgesetzt, jeder abgenommene
   Sprint erzeugt keine Version, die Changelog-Einträge sammeln sich unter
   `[Unveröffentlicht]`). Es bräuchte zuerst zwei Schätzungen — die habe ich
   mit dem Wegfall von #68 nicht mehr angefordert.
   **Empfehlung: nicht in Sprint 6** — ein Sprint mit einer 8er-Risikostory ist
   der falsche Ort für eine Story aus einem fremden Epic. Als
   **Sprint-7-Kandidat vormerken**, damit die Aussetzung nicht zum Dauerzustand
   wird.

**Sprint-7-Bild, das sich aus diesem Planning ergibt** (Vorschlag, keine
Festlegung — Priorisierung ist PO-Sache): ein Bibliothekssprint mit **#68** als
Kern, dazu **#59**, **#71**, **#72** und, falls die Punkte reichen, **#61**.
Vier der fünf brauchen vorher Schätzungen (2.4).

---

## 5. Strang-Zuschnitt und Dateimengen (B13, Notation nach karpathy 3.1)

### 5.1 Warum dieser Sprint nicht parallelisiert

Sprint 4 hatte disjunkte Flächen (`src/ui/` gegen `src/shell/`). Sprint 5 hatte
zwei gemeinsame Dateien, aber **38 Zeilen kleinsten Bereichsabstand** in Dateien
von über tausend Zeilen — Git mischt mit drei Zeilen Kontext, ein Textkonflikt
war ausgeschlossen.

**Hier ist die Lage anders, und zwar messbar:**

| Datei | Länge | #55 fasst an | #56 fasst an |
|---|---|---|---|
| `src/capture/capturewindow.cpp` | **157 Zeilen** | Konstruktor `:42–74` (Ränder `:58`, Abstände `:59`), neu `paintEvent`/`resizeEvent`, `present()` `:111–116` | `eventFilter()` `:91–109` (**nicht** `changeEvent`, siehe 5.3.3), `adjustHeight()` `:148–157` |
| `src/capture/capturewindow.h` | 38 Zeilen | neue Member, neue überschriebene Methoden | ggf. neuer Slot |

**Der kleinste Abstand ist null: Beide schreiben in dieselbe 157-Zeilen-Datei
und dieselbe Kopfdatei von 38 Zeilen.** Ein Textkonflikt ist bei paralleler
Arbeit nicht unwahrscheinlich, sondern der Normalfall. Mit #68 wäre es noch
enger gewesen — es hätte dieselben Stellen wie #56 angefasst.

**Empfohlene Zahl der Dev-Agenten: eins.** Sie folgt aus den Bereichen, nicht
aus der Erlaubnis — dieselbe Herleitung wie in Sprint 5, mit umgekehrtem
Ergebnis. **Der Wegfall von #68 ändert daran nichts**: Zwei Stories in einer
157-Zeilen-Datei parallelisieren so wenig wie drei.

### 5.2 Dateimenge Strang A

| | **Strang A** — die einzige Fläche dieses Sprints |
|---|---|
| **Issues** | **#55, #56** (9 SP) |
| **Zweig** | `story/55-fensterhuelle` |
| **Quellen & Tests** | `src/capture/capturewindow.{h,cpp}` — **ganz**, der Strang ist allein darin; `src/capture/textareaheight.{h,cpp}` **nur**, falls #56 die Höhenformel berührt (sie sollte es nicht: der Fehler sitzt im Auslöser, nicht in der Formel); `tests/capturetest.cpp` |
| **Build** | `src/CMakeLists.txt` — **nur** der `denkzettelcapture`-Block (`:41–52`, `target_link_libraries` bei `:48–52`): `KF6::Svg` und `KF6::WindowSystem` ergänzen; `CMakeLists.txt` (Wurzel) — **nur** die `find_package(KF6 …)`-Komponentenliste (`:24–33`): `Svg` aufnehmen; `tests/CMakeLists.txt` — **nur** der `capturetest`-Block (`:24–34`, Plattformthema nach 3.4) und ein **neuer** `captureshots`-Block nach dem Muster von `libraryshots` (`:65–72`) |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-06-s55-huelle/` — Bericht, eigene Bilder, Messprotokolle; **neu anzulegen: `tests/captureshots.cpp`** (es gibt heute keinen Bildläufer für das Erfassungsfenster — `searchshots`, `libraryshots` und `editshots` hängen alle an `denkzettelui`) |
| **Fachliche Quellen** | **SPEC 3** (`:110–131`) — kennt die Hülle heute nicht (AK 9); **SPEC 15** (`:591–615`) — die KF6-Liste nennt `KSvg` nicht; **SPEC 16** (`:616–640`) — der Bedingungssatz zum Plattformthema führt heute nur `shelltest` und `librarytest`; Wireframes 4a/4b als Referenz, **nicht** zu ändern |
| **Ausdrücklich nicht** | **`src/ui/*`** — mit dem Wegfall von #68 ist die Fläche jetzt vollständig auf `src/capture/` begrenzt; ferner `src/shell/*`, `src/store/*`, `tests/librarytest.cpp`, `tests/libraryshots.cpp`, `tests/editshots.cpp`, `wireframes/` |

**Zwei Zeilen, die leicht übersehen werden und beide zur Pflicht gehören:**

- **Der Bildläufer `tests/captureshots.cpp` existiert nicht.** AK 8 verlangt
  Bilder je Zustand (leer · getippt · 8 Zeilen mit Scrollbalken) × 2 Farbschemata
  × 2 Desktop-Themes = **12 Bilder**. Ohne Läufer entstehen sie nicht. Er steht
  deshalb **in** der Dateimenge — das ist genau der karpathy-3.1-Befund aus
  Sprint 4, angewandt statt wiederholt.
- **Der Läufer ist auf #55 gebucht, nicht auf #56**, und daran hängt eine
  Schätzung: Der Zweitschätzer bindet seine 1 an genau diese Zuordnung (2.3).
  #56 nutzt den Läufer für seine zwei Bilder mit, zahlt ihn aber nicht.

**Was der Wegfall von #68 an der Fläche ändert:** Die Dateimenge ist jetzt
**vollständig auf `src/capture/` begrenzt** — kein Zugriff auf
`src/ui/librarywindow.cpp`, keinen Delegate, keine Labels der Bibliothek. Das
ist der saubere Zuschnitt, den das Planning von 19:40 nur unter der Bedingung
„#68 ≤ 3" nicht bekommen hätte.

### 5.3 Drei Festlegungen

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
3. **Die #56-Heilung gehört in den `eventFilter` auf `m_text`, nicht in ein
   `changeEvent` am Fenster.** Das ist ein Messbefund des Zweitschätzers
   (Kommentar an #56, Belege unter `docs/scrum/reviews/sprint-06-schaetzung/`):
   `QEvent::FontChange` erreicht den `eventFilter` auf **allen drei** Wegen, ein
   `changeEvent` am Fenster verpasst **Weg C** — und Weg C (`field.setFont`) ist
   genau der, den **AK 2 dem Test vorschreibt**.
   **Warum das als Festlegung dasteht und nicht als Hinweis:** Wer die Heilung
   ins `changeEvent` legt und den Test nach AK 2 schreibt, bekommt einen roten
   Test, der nichts über die Sache sagt — die Bauart, gegen die `CLAUDE.md`
   seine Prüfhaltung fasst („Ein Testaufbau, in dem der Fehler gar nicht
   auftreten *kann*, ist kein Test" — hier in der Spiegelform: ein Aufbau, in
   dem die Heilung gar nicht wirken *kann*). Der naheliegende Weg ist hier der
   falsche, und das fällt ohne Messung nicht auf.
   `capturewindow.cpp` hat den Filter bereits (`:52`, `:91–109`) — die Heilung
   hängt sich an einen bestehenden Weg, sie baut keinen neuen.

---

## 6. UI-Story-Einstufung (Vorschlag an den PO)

Welche Stories UI-Stories im Sinne von DoD 3 sind, legt der PO beim Planning
fest (Kundenentscheidung 31.07.2026).

### 6.1 Beide Stories sind UI-Stories

| Issue | UI-Story? | Prüfmittel nach DoD 3 | Grund |
|---|---|---|---|
| **#55** | **ja, mit geteilter Bildpflicht** | *Rundung, Kontur, Fläche, Farbrollen, Maße:* eigene Bilder von `denkzettel-ux` aus dem Sprint-Stand, offscreen, 3 Zustände × 2 Farbschemata × 2 Desktop-Themes. *Schatten:* **kein Bild möglich** — benannter Ersatz, siehe 6.2 | Reiner **Zustand**: „Bei Zuständen ist das Bild der Prüfgegenstand, nicht die Zusicherung" (B3). Es ist die bildlastigste Story, die das Projekt bisher hatte |
| **#56** | **ja, mit Bildpflicht — auf #55s Läufer** | Zwei Bilder aus **einem** Lauf: kleine und große Systemschrift, je das ganze Fenster. **Der Läufer wird von #55 gestellt** (2.3, 5.2) | Die Feldhöhe ist ein Zustand, und der Kunde hat die 3 Zeilen bei der Sprint-1-Abnahme am Bild zurückgewiesen — dasselbe Bild schließt sie wieder aus. **Und es ist der einzige Nachweis, den es hier gibt:** Am Kundenblick ist #56 nicht prüfbar (4.1.1) |

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
6. **Der `/usr`-Takt.** #55 braucht den installierten Stand für Hülle und
   Schatten am echten Plasma. **Der Strang installiert nicht selbst** (DoD 2,
   Präzisierung nach Sprint-3-M1); der PO taktet, und der Endstand wird am
   Sprint-Ende einmal installiert und **der Hauptweg jeder Story daran
   ausgeführt**. *Das war der Mangel M1 aus Sprint 5* — dort belegte der
   Installationstakt den Stand, aber keinen einzigen Hauptweg. Bei #55 ist die
   Wiederholung besonders teuer: Der Schatten ist der einzige Nachweis, der
   **ausschließlich** dort zu führen ist.
   **#56 hat hier keinen Hauptweg am installierten Stand** — das ist die Folge
   von 4.1.1 und gehört im Installationstakt **als benannte Grenze notiert**,
   nicht stillschweigend ausgelassen. Sein Nachweis ist Test und Bild.
7. **Eine Story trägt acht von neun Punkten.** Fällt #55 in die Loop-Disziplin
   (zweimal derselbe Fehlschlag ohne neue Evidenz → Stopp und Impediment),
   fällt der Sprint — was bliebe, wäre eine 1. Das ist der Preis dieses
   Schnitts, und er ist bewusst gezahlt: Der Kunde hat den Befund zweimal
   angemahnt. **Mit dem Wegfall von #68 hat sich das Verhältnis verschärft**
   (vorher 8 von 12 möglichen); die vier freien Punkte (4.4) sind die einzige
   Reserve, die dieser Sprint hat.
8. **Bilderzahl.** AK 8 ergibt 3 Zustände × 2 Schemata × 2 Themes = **12
   Bilder** vom Strang, dazu die eigenen Bilder des UI-Reviews (Sprint 5: 20).
   Der Läufer muss das Desktop-Theme **im Lauf** wechseln können — meine Sonde
   zeigt, dass das geht (`ImageSet::setImageSetName()` je Durchgang, 3.2), aber
   es ist eine Bauanforderung an `captureshots.cpp` und keine Selbstverständlichkeit.

---

## 8. Klärungspunkte vor dem Ziehen — mit Vorschlag

Die AK- und Label-Anpassungen macht der PO; der Scrum Master legt den Wortlaut
vor.

**K1 — `sp:8` an #55, und die Label-Disziplin als Dauerpflicht. ✅ ERLEDIGT 20:03.**
#55 trug kein `sp:`-Label, obwohl die Schätzung seit dem 02.08.2026 im
Issue-Text stand. Das war **M2 aus Sprint 5**, an der nächsten Story wiederholt.
*Vollzug, von mir am Backlog nachgemessen:* **#55 `sp:8` · #68 `sp:5`** gesetzt,
#56 trug `sp:1` bereits.
**Der Vorschlag zur Dauerpflicht steht weiter offen und geht in die Retro:** Seit
Abschluss-Punkt 12 ist das Label die Quelle des **Endwerts** der Schätzhistorie
(Sprint 5, §24) — ein fehlendes Label erzeugt jetzt eine Lücke in der
Datenreihe, nicht nur eine unsaubere Rückverfolgung. **Wird ein Issue geschätzt,
wird im selben Zug das Label gesetzt.** Dieser Satz steht in keiner Regeldatei;
zweimal in Folge hat ihn eine Nachforderung ersetzt (10, Punkt 9).

**K2 — Schätzungen für #56 und #68 einholen (blockierend). ✅ ERLEDIGT 20:03.**
Beide liegen vor: **#68 = 5** (damit draußen), **#56 = 1 bestätigt** (damit
drin, unter der Bedingung 2.3). Der Vollzug samt Wirkung steht in 2.1. Die
empfohlene Reihenfolge — #68 zuerst — hat den Schnitt entschieden.

**K3 — #68 AK 2: die offene Frage ist entscheidbar, und zwar jetzt.
Gilt weiter, jetzt für Sprint 7.**
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

**K8 — Milestone „Sprint 6"** anlegen und **#55 und #56** zuordnen — nicht #68.

**K9 — den Schätz-Belegordner prüfen. *(neu 20:03)* ✅ ERLEDIGT.**
Als ich den Nachtrag begann, lag `docs/scrum/reviews/sprint-06-schaetzung/`
noch nicht im Repo, während Protokoll und Issue-Kommentare bereits darauf
verwiesen — genau die Bauart, die ich in 3.1 an der AK-1-Richtigstellung
beanstandet habe. Ich habe sie deshalb als offenen Punkt hingeschrieben statt
sie zu übergehen.
*Vollzug, von mir mit `git ls-files` geprüft:* Elf Dateien versioniert
(`573901e`, gemerged `5982327`), darunter alle vier Messausgaben und ein
Skript, das die Messungen wiederholbar macht. **Kein offener Rest.**

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
| Capture-Fensterhülle | **#55** | **8** · 02.08.2026 | Sprint-5-Planning §2 (Dev 8 · UX 8, deckungsgleich) — **2 unabhängige** | *keine* | **8** | Sprint 6 | **1** | **1,00** | `keine` |
| Feldhöhe nach Schriftänderung | **#56** | **1** · 01.08.2026 | Label bei Anlage — **1 Hand** | **bestätigt 1** · 02.08.2026, Zweitschätzung Dev (2.1) | **1** | Sprint 6 | **3** | **1,00** | `keine` |

**Drei Anmerkungen, die das Urteil tragen und die der Verwalter später nicht
selbst fällen darf:**

- **#55 geht in die Kurve, und das ist neu.** Seine Erstschätzung fiel im
  **Sprint-5**-Planning, die Umsetzung liegt in Sprint 6 — zwischen beiden liegt
  genau ein Ereignis, bei dem eine Revision hätte stattfinden können: dieses
  Planning hier. Damit ist der §24.1-Maßstab erfüllt. **Abstand 1 bekommt seinen
  vierten Punkt**, und zum ersten Mal einen aus einer 8er-Story.
- **#56 hat Abstand 3 und `keine` als Anlass.** Angelegt am 01.08.2026, also
  während Sprint 3; umgesetzt in Sprint 6. Die Zählweise ist die von Sprint 5
  (#57 und #58, beide 01.08. angelegt, Umsetzung Sprint 5, dort Abstand 2).
  **Das ist der bislang weiteste Abstand der Reihe zusammen mit #11 und #12** —
  und der erste dort, der **nicht** aus der Schätzklausur stammt.
  **Der Punkt ist gemessen, nicht konstruiert:** #56 hatte drei Sprints lang
  Gelegenheit zur Revision, und die Zweitschätzung vom 02.08.2026 hat die 1
  ausdrücklich **bestätigt** statt sie zu übernehmen — dieselbe Form, in der
  Sprint 5 die 1 an #58 als echten Datenpunkt gewertet hat (dort §24.3).
  **Das ist der inhaltlich wertvollste Punkt dieses Sprints für die Reihe:** Ein
  Faktor 1,00 bei Abstand 3, der auf einer Prüfung beruht und nicht auf einer
  Auslassung.
- **#68 hat keine Zeile.** Es ist nicht umgesetzt, und eine nicht umgesetzte
  Story hat keinen Umsetzungssprint. **Seine `sp:5` gehört ausdrücklich nicht in
  die Reihe** — auch nicht als 24.2-Fall, denn 24.2 erfasst umgesetzte Stories
  ohne Revisionsgelegenheit, nicht ungezogene. Die Zeile entsteht in dem
  Planning, das #68 zieht; ihre **Erstschätzung ist dann die 5 vom 02.08.2026
  mit Provenienz „1 Hand"** (2.4), nicht der Wert, den Sprint 7 konsolidiert.
  *Der Satz steht hier, weil genau er später verlorengeht.*

**Was die DoD-Prüfung am Sprint-Ende nachzutragen hat:** die tatsächlichen
Endwerte aus den `sp:`-Labeln, jede Revision im Sprint mit Datum und Fundstelle,
und die Bestätigung des Anlass-Kennzeichens je Zeile. **Ein besonderer
Prüfpunkt:** Kippt die Läufer-Zuordnung aus 2.3, wird #56 zur 2 — dann ändert
sich sein Endwert, sein Faktor **und** sein Anlass-Kennzeichen (auf
`gegenstand-geändert`, weil dann der Umfang der Story gewachsen wäre, nicht die
Einsicht in sie).

---

## 10. Hinweise an den Product Owner

1. **K1 und K2 sind erledigt** — beide Schätzungen liegen vor, alle drei
   `sp:`-Label sind gesetzt und von mir nachgemessen (1, 2.1). **Offen bleibt
   der Grundsatz dahinter:** Label im selben Zug wie die Schätzung (Punkt 9).
2. **K9 ist ebenfalls erledigt** — die Schätzbelege liegen versioniert und
   **wiederholbar** unter `docs/scrum/reviews/sprint-06-schaetzung/`
   (`pruefen.sh`), von mir per `git ls-files` geprüft.
3. **Der Schnitt geht dem Kunden zur Freigabe** (Freigabemodell,
   Kundenentscheidung 31.07.2026). Vorzulegen sind Sprint-Ziel (4.1), die
   **zwei** Issues (4.2) und **ausdrücklich der Kontostand** samt der
   Feststellung, dass die Luft Vorsorge für eine 8er-Risikostory ist und kein
   Leerlauf.
4. **UI-Story-Einstufung ist PO-Entscheidung** (6) — sie muss **vor** dem Spawn
   fallen, besonders die geteilte Belegform bei #55 (6.2).
5. **Ein Dev-Agent, nicht zwei** (5.1), und der `/usr`-Takt gehört als Verbot
   mit Begründung in den Spawn-Auftrag, nicht als Hinweis (Risiko 6).
6. **Sechs Punkte für den Spawn-Auftrag, die je einen Fehlversuch ersparen** —
   vier aus meinen Messungen, zwei aus denen des Zweitschätzers:
   - `KSvg` findet das Theme offscreen vollständig, der Bildnachweis trägt (3.2);
   - `marginSize()` liefert bei 8-px-Themes **`7,99998`** statt `8` — eine
     absolute Zusicherung fällt (3.2, 5.3.2);
   - `KWindowShadow::create()` ist offscreen **immer** `false`; das ist kein
     Fehler des Codes (3.3);
   - die Schattenbindung gehört in `present()` **nach** `show()`, nicht in den
     Konstruktor (Risiko 2);
   - **die #56-Heilung gehört in den `eventFilter`, nicht in ein `changeEvent`**
     — ein `changeEvent` verpasst genau den Weg, den AK 2 dem Test vorschreibt
     (5.3.3, Dev-Messung);
   - #56 ist **am Kundenblick nicht prüfbar**; sein Nachweis ist Test und Bild
     (4.1.1).
7. **#70 bei der Freigabe mitentscheiden lassen** (K5) — ein Satz vom Kunden,
   und das Issue wird für Sprint 7 schätzbar.
8. **Sprint 7 zeichnet sich ab** (4.4): ein Bibliothekssprint mit **#68** als
   Kern (Dev-Empfehlung, am Code begründet), dazu **#59**, **#71**, **#72** und
   ggf. **#61** — letzteres, damit Abschluss-Punkt 10 nicht dauerhaft ausgesetzt
   bleibt. **Vier der fünf brauchen vorher Schätzungen; #68s `sp:5` stammt aus
   einer Hand und ist keine** (2.4).
9. **Retro nach Sprint 6 ist fällig** (Kadenz: Sprint 3, 6, 9, …). Kandidaten
   liegen vor: die vier aus Sprint 4 §17.6 und der B13-Nachzug der erweiterten
   Dateimengen-Notation aus Sprint 5 §5.1. **Dazu zwei neue aus diesem
   Planning:**
   - **Label im selben Zug wie die Schätzung.** Zweimal in Folge gerissen (M2 in
     Sprint 5, K1 hier), beide Male durch Nachforderung geheilt statt durch eine
     Regel. Der Satz steht in keiner Datei, die eine Sitzung von selbst liest.
   - **Ein Sprint-Ziel behauptet keine Nachprüfbarkeit, die es nicht gibt.**
     Mein eigener Entwurf von 19:40 hat es getan (4.1.1); aufgefallen ist es
     erst, als #68 wegfiel und die Stütze verschwand.

---

## 11. Was dem Kunden zur Entscheidung vorliegt

1. **Sprint-Ziel und Schnitt — zwei Stories, 9 Punkte.** Das Erfassungsfenster
   bekommt die Hülle seines Desktop-Themes: **runde Ecken**, Kontur und Schatten
   (#55). Dazu wird ein Fehler vorgezogen geheilt, der heute noch nicht sichtbar
   ist (#56).
2. **Der Sprint hat einen Gegenstand, nicht mehrere.** #55 trägt **acht der
   neun Punkte**. Das ist Absicht — der Befund ist zweimal angemahnt worden —,
   und es heißt auch: Fällt diese Story, fällt der Sprint.
3. **Bewusst nicht gefüllt.** Zwei von vier zulässigen Stories, 9 von ~13
   Punkten. Die Luft ist Vorsorge für Review-Auflagen an der größten Story, die
   dieses Projekt bisher hatte. Wer das Konto voller will: #59 (ruhige Liste bei
   Fensteraktivierung) wäre der Zugang — er kostet die Klammer des Sprint-Ziels
   und nimmt Sprint 7 eine seiner vier Stories weg.
4. **#56 werden Sie nicht sehen können — und das ist kein Versäumnis.** Die
   Feldhöhe fällt nach einer Schriftänderung von fünf auf drei Zeilen; genau die
   drei Zeilen, die Sie bei der Sprint-1-Abnahme zurückgewiesen haben. Sichtbar
   wird der Fehler erst, wenn Denkzettel Schriftänderungen im laufenden Betrieb
   überhaupt annimmt — **das ist #68 und kommt in Sprint 7.** Wir heilen ihn
   **vorher**, damit die Verbesserung dann nicht die Verschlechterung mitbringt.
   Belegt wird er durch Test und Bild, nicht durch Ihren Blick.
5. **Ein Nachweis kann nur von Ihnen kommen.** Der **Schatten** ist am
   Prüfrechner grundsätzlich nicht abzubilden — gemessen, nicht vermutet. In der
   Abnahme steht deshalb wieder ein **Foto-Punkt**, wie beim Wächterdialog in
   Sprint 5, und dazu eine zweite Frage: Liegt der Schatten auch beim
   **zweiten** Öffnen des Fensters noch darunter? (Das Fenster wird bei jedem
   Zeigen neu aufgebaut — der Schatten kann dabei verlorengehen, und kein Test
   dieses Projekts würde es bemerken.)
6. **Eine kleine Entscheidung nebenbei** (K5): Soll die Liste beim Wandern mit
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
(Abschluss-Punkt 12).

**Zweiter Durchgang (20:03), nach Eintreffen der beiden Schätzungen:** den
**Schnitt festgeschrieben** — #68 mit 5 SP ausgeschieden nach der Bedingung, die
**vor** der Zahl im Protokoll stand, #56 mit 1 SP bestätigt, Ergebnis **#55 +
#56 = 9 SP / 2 Stories**; alle drei `sp:`-Label am Backlog nachgemessen statt
die Meldung zu glauben (K1, K2 erledigt); **einen Fehler im eigenen Entwurf
korrigiert** — das Sprint-Ziel von 19:40 behauptete für #56 eine
Nachprüfbarkeit, die es nicht gibt, und der Wegfall von #68 hat die Stütze
weggezogen (4.1.1); die Bedingung des Zweitschätzers (Bildläufer auf #55
gebucht) als **Prüfsatz für die DoD-Prüfung** verankert samt der Folge für das
Anlass-Kennzeichen (2.3, 9); festgehalten, dass #68s `sp:5` **aus einer Hand**
stammt und in Sprint 7 nicht für eine Schätzung gehalten werden darf (2.4); die
beiden Dev-Messbefunde als **Festlegung** in den Strang-Zuschnitt genommen
(5.3.3, 10.6); den zunächst fehlenden Belegordner als **K9** offen
hingeschrieben statt ihn als erledigt zu behaupten — und nach seinem Eintreffen
per `git ls-files` geprüft und geschlossen; das Sprint-7-Bild aus der
Dev-Empfehlung abgeleitet (4.4).

**next:** (1) *(erledigt während des Nachtrags: K9 — die Schätzbelege sind
versioniert und wiederholbar, `573901e`.)* (2) PO legt die UI-Story-Einstufung
fest, besonders die geteilte Belegform bei #55 (6.2), und arbeitet K4 und K6 in
die Issues ein; **K3 wandert nach Sprint 7** (#68 AK 2 auf „setzt #56 voraus"
fassen). (3) PO legt Sprint-Ziel und Schnitt dem Kunden zur Freigabe vor, **mit
dem Kontostand**, mit der Erklärung zu #56 (11, Punkt 4) und mit der Frage aus
K5 (#70). (4) Nach der Freigabe: Milestone „Sprint 6" mit **#55 und #56**,
`sprint-06-basis`, dann Spawn **eines** Strangs nach 5.2, in der Reihenfolge
**#56 → #55**. (5) Freigabe-Stand in die Kontotabelle (1) eintragen — dieser
Punkt ist in Sprint 5 offen geblieben (dort §19). (6) Vor dem
Sprint-7-Planning zu heilen: zweite Schätzungen für **#68, #59 ist gedeckt,
#71, #72, #61**, dazu die drei Klärungen und die zweite Schätzung zu **#69**,
unverändert offen seit Sprint 5 §2.4. (7) **Retro nach diesem Sprint**
(Kadenz) — Kandidaten in 10, Punkt 9.

---

## 13. Nachtrag: Prozessökonomie — vier Punkte für die Retro (PO, 04.08.2026)

**Anlass.** Der Kunde hat gefragt, was die Arbeit dieses Teams der
Entwicklung bringt und ob sie zu behalten, zu optimieren oder aufzugeben sei.
Die Antwort ist am Bestand geführt worden, nicht am Eindruck; die Messung steht
unten. **Alle vier Punkte sind am 04.08.2026 entschieden** — drei vom PO
umgesetzt, der vierte (Schätzkegel) vom Kunden entschieden und am selben Tag
zurückgebaut (13.4). Sie ergänzen die Kandidaten aus 10, Punkt 9 und ersetzen
sie nicht.

**Die Messung, die den Anlass trägt** (04.08.2026, selbst gezählt):

| Gegenstand | Zeilen |
|---|---|
| Produktivcode (`src/`) | 3.894 |
| Tests (`tests/`) | 6.158 |
| Prozessdokumentation (`docs/scrum/`) | 13.659 |
| Belegbilder (`docs/scrum/**/*.png`) | 258 Stück |

Die Prozessdokumentation ist das Dreieinhalbfache des Produkts. Das ist
vertretbar, **wenn** die Prüfarbeit selbst der Gegenstand ist — es ist nicht
vertretbar als Nebenkosten eines Erfassungswerkzeugs. Diese Frage ist eine
Kundenfrage und steht als Punkt 4 unten.

### 13.1 Umgesetzt — automatische Testläufe (P1)

`.github/workflows/ci.yml`, verankert in `PROZESS.md`, Sprint-Mechanik. Er war
die Hälfte des PR-Abbruchkriteriums, die **keine Disziplin verlangt**, und lag
seit dem 02.08.2026 als offene Kundenentscheidung herum (Sprint 4, §15.8).

*Was die Retro daran zu prüfen hat:* Hat der Lauf in Sprint 6 **etwas gefunden**,
das ohne ihn durchgegangen wäre? Findet er zwei Sprints lang nichts, ist zu
entscheiden, ob er Wache oder Zierde ist — „Schweigen ist kein Erfolgsnachweis"
gilt auch für ihn. Gegenprobe zur Wachsamkeit ist am 04.08.2026 geführt
(Mutationsprobe, siehe Bericht des Tages).

### 13.2 Umgesetzt — Pflichtteil und Kürteil der Protokolle (P2)

`PROZESS.md`, Artefakte. Vier Abschnitte sind Pflicht, der Rest ist Kür.
*Was die Retro daran zu prüfen hat:* Zeilenzahl des Sprint-6-Protokolls gegen
den Durchschnitt der Sprints 3–5 (rund 1.300 Zeilen). **Kein Beleg darf in der
Differenz stecken** — sinkt die Zahl, weil Bilder oder Messwerte fehlen, ist
die Kürzung fehlgeschlagen und B7 verletzt.

### 13.3 Umgesetzt — Verwalter-Bericht als Existenzprüfung (P3)

`PROZESS.md`, Sprint-Abschluss, Punkt 11. Der Befund V3 aus Sprint 5 hatte es
selbst benannt: *„Es fehlt nicht die Regel, es fehlt ihre Wirkung."* Eine dritte
Ermahnung wäre die dritte gewesen; stattdessen hängt der Bericht jetzt an einer
Dateiprüfung mit Exit-Code, wie das Zweig-Räumen an `git merge-base`.
*Was die Retro daran zu prüfen hat:* Liegt nach dem dritten Lauf eine Datei vor?
Wenn nein, ist nicht die Regel zu schärfen, sondern die Rolle zu streichen.

### 13.4 Entschieden — der Schätzkegel wird entfernt (P4, Kundenentscheidung)

Der Kegel kostet 840 Zeilen Werkzeug und Datenreihe, einen eigenen
Abschluss-Punkt (12), einen DoD-Prüfsatz und eine Übertragungsregel für den
Verwalter. Was er misst, steht in `PROZESS.md` selbst:

> Er misst den **Revisionsfaktor** (Endwert ÷ Erstwert), **nicht** den Abstand
> zum tatsächlichen Aufwand; dieser wird im Projekt nicht erhoben.

Damit zeigt er, **wie oft neu geschätzt wurde**, nicht **ob richtig geschätzt
wurde**. Eine Story, die niemand angefasst hat, steht bei 1,0 — auch wenn sie
das Doppelte gekostet hat. Er kann per Konstruktion nie belegen, dass eine
Schätzung falsch war.

**Drei Wege lagen dem Kunden vor**, weil der Kegel sein Auftrag vom 02.08.2026
ist:

1. **Aufwandserhebung nachrüsten** — dann misst er Schätzgüte. Kostet je Story
   eine gemessene Zahl, die es heute nicht gibt.
2. **Streichen** — spart 840 Zeilen und drei Prozessregeln.
3. **Lassen, aber als das benennen, was er ist** — ein Bild der Revisionsunruhe,
   nicht der Schätzgüte. Das war der bisherige Stand, und er war der teuerste
   der drei.

*Empfehlung des PO:* Weg 1, falls das Projekt Schätzgüte wirklich wissen will —
sonst Weg 2. Der Mittelweg trägt die vollen Kosten für die halbe Aussage.

**Entscheidung des Kunden vom 04.08.2026: Weg 2 — entfernen.** Zurückgebaut
wurde am selben Tag:

- **`docs/scrum/diagramme/`** vollständig gelöscht (`LIESMICH.md`, `kegel.py`,
  `kegel.svg`, `schaetzhistorie.json`).
- **`PROZESS.md`**: Sprint-Abschluss **Punkt 12** ersatzlos gestrichen — die
  Liste endet bei 11; der Schätzhistorie-Absatz der DoD-Sektion samt der
  Begründung für Abstand- und Faktor-Spalte entfernt; in den Pflichtteilen der
  Protokolle steht die DoD-Prüfung nun ohne den Zusatz „samt Schätzhistorie".
  An die Stelle trat ein datierter Vermerk in der Sprint-Mechanik, der
  festhält, **dass** es den Kegel gab und **warum** er wegfiel — sonst schlägt
  ihn in sechs Monaten jemand ohne die Begründung erneut vor.
- **`.claude/agents/denkzettel-verwalter.md`**: der Auftragspunkt
  „Schätzhistorie fortschreiben und Diagramm erzeugen" entfernt.

**Die Sprint-Protokolle bleiben unangetastet** — §9 dieses Protokolls,
`sprint-05.md` §24 und alle weiteren Kegel-Stellen sind historische Berichte
über das, was damals galt. Wer sie nachträglich glättet, zerstört die
Beweislage, auf der dieses Projekt seine Prüfungen führt.

### 13.5 Befund am Rande, gefunden beim Einrichten der CI

`appstreamtest` läuft seit Sprint 1 in jedem `ctest` mit und zählt in jedes
„7/7 grün" hinein — auch in die Belege unter
`docs/scrum/reviews/sprint-05-s-symbole/gruen-ctest.txt` und
`spike-62-spellfix1/ctest.txt`. Das Projekt hat **keine** `.metainfo.xml` und
keine `.appdata.xml`; das ECM-Skript
(`/usr/share/ECM/kde-modules/appstreamtest.cmake`) sammelt dann eine leere
Dateiliste, überspringt die Validierung und endet grün. **Der Test kann per
Konstruktion nicht rot werden** — dieselbe Klasse wie die tautologische
Zusicherung aus Sprint 3, nur nicht selbst geschrieben, sondern von ECM
geerbt.

*Nicht geheilt, sondern gemeldet:* Eine `metainfo.xml` zu verfassen ist eine
Produktentscheidung (Beschreibung, Kategorien, Bilder, Release-Historie am
Changelog) und gehört in eine Story, nicht in einen Nebenweg. Als **#73**
angelegt. Bis dahin gilt: **„7/7" heißt in diesem Projekt sechs Tests und eine
Leerstelle.**

---

## 14. Backlog-Grooming (PO, 04.08.2026)

**Anlass:** Kundenauftrag nach der BM25-Prüfung — Reihenfolge in diesem Zug neu
zu bewerten. Stand: 49 offene Issues, 34 mit Schätzung.

### 14.1 Was sich geändert hat

| Issue | Von | Nach | Grund |
|---|---|---|---|
| #74 Seite „Kürzel" | M7 | **M2** | Die Kürzel existieren heute; `SPEC.md:77` verspricht Konflikterkennung *„bei Kürzel-Änderung"* und hat ohne diese Seite **keinen Auslöser** |
| #75 Seite „Export" | M7 | **M5** | Gehört zu Obsidian-Export (#32) und Volllauf-Schutz (#34); in M7 stünde die Seite vor der Funktion, die sie einstellt |
| #61 Versionsanzeige | M7, ruhend | **Kandidat Sprint 7/8** | Hält Abschluss-Punkt 10 ausgesetzt — Sprint 4 und 5 sind ohne Version abgenommen, Sprint 6 wird der dritte |

**Die Regel dahinter, erstmals ausgesprochen:** *Einstellungsseiten reisen mit
ihrer Funktion.* Das Projekt hat sie schon zweimal angewandt, ohne sie zu
benennen — die Seite „Sprachnotizen" hängt in M4 an #27, „KI-Provider" und
„Analyse" in M3 an #16. Die beiden heute angelegten Seiten waren die erste
Abweichung.

### 14.2 Neu angelegt

- **#77** Trefferliste zeigt, warum ein Treffer einer ist (Ergebnis der
  BM25-Prüfung, aus zwei Richtungen unabhängig empfohlen)
- **#78** Trefferliste hat keine Obergrenze — `store.cpp:448` ohne `LIMIT`,
  weder SPEC noch Zeichnung nennen eine. **Der erste Schritt ist eine Messung,
  kein Umbau**; möglicherweise endet die Story mit einem Satz in der SPEC.

### 14.3 Sprint-7-Bild

**Kern #68** (Schrift folgt zur Laufzeit nicht, 5 SP). #56 aus Sprint 6 ist
genau seine Voraussetzung — die vorgezogene Heilung des Fehlers, den #68
sichtbar machen wird. Aufgefüllt wird aus **#71, #72, #77**; **#70** braucht
zuvor eine Kundenentscheidung (K5, seit dem Sprint-6-Planning offen).

**Alle vier Auffüll-Kandidaten sind ungeschätzt.** Vor dem Planning brauchen
sie je zwei unabhängige Schätzungen — sonst wiederholt sich der Sprint-6-Fall,
in dem von acht Kandidaten nur zwei regelkonform geschätzt waren.

### 14.4 Was ich geprüft und **nicht** geändert habe

- **Der Ideenspeicher** (#48 Zwischenablage-Zettel, #49 Wiedervorlage) bleibt
  geparkt. Beide sind Kundenideen ohne Priorisierung; sie zu bewegen wäre eine
  Entscheidung, die mir nicht zusteht.
- **#63** (Tag-Chips im Lesezustand fehlen) bleibt in M3. Die Chips zeigen
  KI-Tags — vor M3 gäbe es nichts anzuzeigen; die Einordnung ist richtig.
- **Die Epic-Reihenfolge M1→M7** bleibt Grundlinie. Sie ist heute nicht
  angetastet worden; verschoben wurden drei Issues **innerhalb** dieser Linie.

---

## 15. Sprint-Start (PO, 04.08.2026)

Zwei Festlegungen, die das Planning nicht treffen konnte: **#59 kam erst mit
der Freigabe in den Sprint** (§1), der Strang-Zuschnitt in §5 ist für #55 und
#56 geschrieben.

### 15.1 Zweiter Strang für #59 — dieselbe Prüfung, umgekehrtes Ergebnis

§5.1 hat die Zahl der Dev-Agenten aus dem Abstand der Flächen abgeleitet, nicht
aus der Erlaubnis. Dieselbe Rechnung für #59:

| Datei | Länge | Strang A (#55, #56) | Strang B (#59) |
|---|---|---|---|
| `src/capture/capturewindow.{h,cpp}` | 157 + 38 | ganz | — |
| `src/ui/librarywindow.cpp` | 1064 | — | `:311–322`, `:472`, `:604` |
| `tests/capturetest.cpp` | 206 | ganz | — |
| `tests/librarytest.cpp` | 3440 | — | ein neuer Test |
| `tests/CMakeLists.txt` | 156 | `capturetest`-Block, neuer `captureshots`-Block | — |
| `src/CMakeLists.txt`, `CMakeLists.txt` | | `denkzettelcapture`, `find_package` | — |

**Die Schnittmenge ist leer.** #59 braucht keine der drei `CMakeLists.txt`: Der
Test kommt in ein bestehendes Ziel, es entsteht kein neues. Damit liegt der
Fall so, wie Sprint 4 lag (`src/ui/` gegen `src/shell/`) — und nicht wie der
Fall in §5.1, wo #55 und #56 in dieselbe 157-Zeilen-Datei schreiben.

**Zwei Stränge, drei Stories.** #55 und #56 bleiben bei einem Agenten, weil ihr
Abstand null ist; #59 bekommt einen eigenen, weil ihr Abstand vollständig ist.

| Strang | Issues | SP | Zweig | Worktree |
|---|---|---|---|---|
| **A** | #56, dann #55 | 9 | `story/55-fensterhuelle` | `../denkzettel-s6a` |
| **B** | #59 | 2 | `fix/59-scrollstelle` | `../denkzettel-s6b` |

Ausgangsstand beider Zweige ist gepushtes `main` (`0a229d2`), nicht der
Basis-Tag: B13 schreibt den gepushten Stand vor, und zwischen `sprint-06-basis`
und heute liegen CI-Ergänzung und README-Arbeit. Der Prüf-Diff des Sprint-Endes
läuft weiter über `sprint-06-basis..main` und enthält beides — das ist richtig
so, es ist in diesem Sprint geschehen.

**Reihenfolge in Strang A: #56 vor #55.** #56 hängt sich an den bestehenden
`eventFilter` (§5.3.3), #55 baut den Konstruktor um. Andersherum stünde die
kleine Heilung im Schatten der großen und wäre im Diff nicht mehr zu trennen.

### 15.2 #59 ist keine UI-Story im Sinne von DoD 3

§6 stuft #55 und #56 ein; für #59 fehlt die Einstufung, und sie ist dem PO
vorbehalten (Kundenentscheidung 31.07.2026).

**Einstufung: keine Bildpflicht.** Prüfgegenstand ist eine Bewegung, und B3
sagt dazu: *„Bei Bewegungen ist der Weg der Prüfgegenstand, nicht das Ziel."*
AK 1 des Issues verlangt aus demselben Grund die Messung **am Rollwert, nicht
am Endbild**. Ein Standbild könnte hier nur zeigen, wo die Liste steht — die
Frage ist aber, ob sie sich bewegt hat.

Das ist die Gegenrichtung zu #55, wo dieselbe Regel volle Bildpflicht erzeugt:
Dort ist der Prüfgegenstand ein Zustand.

Der karpathy-Review über den Sprint-Diff (DoD 3, erster Halbsatz) gilt für #59
unverändert.

### 15.3 Was die Stränge nicht dürfen

Beiden ausdrücklich untersagt: Installation nach `/usr` (DoD 2 taktet der PO,
es gibt nur ein `/usr` und zwei Stränge), Merge nach `main`, Push, und jede
Änderung außerhalb der eigenen Dateimenge. Strang B ist zusätzlich auf die
Nachbarschaft zu #57 und #70 hingewiesen worden, einschließlich des in Sprint 3
geprüften und **vom Reviewer selbst zurückgenommenen** Schwellwert-Wegs — er
ist der naheliegende Griff an dieser Stelle und der falsche.

---

## 16. Meldungen aus den Strängen (melden, nicht heilen)

Was die Stränge außerhalb ihrer Fläche gefunden haben. Der PO entscheidet, was
davon wohin gehört; keiner der Punkte ist vom Finder geändert worden.

### 16.1 Aus Strang B (#59)

**M-B1 — `activateWindow()` holt unter Wayland den Fokus nicht zurück.**
Ein Prozess kann sich den Fokus nicht selbst zuteilen; dazu bräuchte er ein
xdg-activation-Token. Der erste Sichtlauf des Strangs meldete deshalb
„zurückgekommen: nein" und maß nichts — ein Lauf, der ausgesehen hätte wie ein
Beleg. Was funktioniert, ist der compositor-getriebene Weg: das obenauf
liegende Fenster schließen, dann gibt der Compositor den Fokus von sich aus
zurück, denselben Weg wie ein Alt-Tab.

*Einordnung des PO:* Das ist eine Bedingung für **jede** künftige Sicht- oder
Bildprüfung mit Fensterwechsel, nicht nur für #59. Es gehört damit in die
Prüfhaltung von `CLAUDE.md` — die Datei, die jede Sitzung von selbst liest.
**Retro-Kandidat**, Vorschlag: als Zeile bei den Bildbeleg-Regeln aufnehmen.
Bis dahin steht es hier und im Bericht des Strangs.

**M-B2 — der Debug-Build ist als Anwendung nicht startbar, solange der
installierte Daemon läuft.** `KDBusService::Unique` gibt den Start an die
laufende Instanz weiter. Wer den Debug-Stand prüfen will, prüft dann den
installierten — die Falle aus Sprint-3-Mangel M1, hier auf einem zweiten Weg.
Der Strang ist ihr ausgewichen, indem er einen eigenständigen Prüfläufer gegen
`libdenkzettelui.a` und `libdenkzettelstore.a` gebaut hat
(`probe59.cpp`, versioniert im Belegordner) — mit eigener temporärer Datenbank
und eigenem `XDG_CONFIG_HOME`, damit die Notizen und die Fenstergeometrie des
Kunden unberührt bleiben.

*Einordnung des PO:* Der Ausweg trägt, ist aber von Hand zu übersetzen — die
Befehlszeile steht im Bericht. **Retro-Kandidat**, offene Frage: Soll das
Projekt einen solchen Prüfläufer als CMake-Ziel führen, wie es die Bildläufer
sind? Dann wäre er wiederholbar statt rekonstruierbar.

**M-B3 — clang-tidy-Bestandswarnungen** in `note.h`, `timestampformat.h`,
`notelistmodel.h`, `trayicon.cpp`, `globalshortcuts.cpp`, `notelistdelegate.cpp`
und `librarywindow.{h,cpp}`. Nicht von dieser Story verursacht, nicht
angefasst. Der auffälligste: `globalshortcuts.cpp:93` —
`bugprone-unused-return-value`. *Einordnung des PO:* `globalshortcuts.cpp:93`
wird geprüft; die Stelle registriert globale Kürzel, und ein verworfener
Rückgabewert ist dort die Bauart, die #73 und die Kürzel-Rückmeldung
betrifft. Die übrigen sind Bestand.

### 16.2 Abnahme von #59 durch den PO

**Angenommen**, vorbehaltlich der Sprint-Ende-Prüfungen (DoD 2 Installation,
DoD 3 karpathy-Review über den Sprint-Diff).

Geprüft habe ich nicht den Bericht, sondern die Belege:

| AK | Beleg, den ich gelesen habe |
|---|---|
| 1 — Rollwert unverändert | `QCOMPARE(list->verticalScrollBar()->value(), rolledTo)` in `librarytest.cpp:1881`; ohne die Heilung rot mit *Actual 7 / Expected 0* (`befund-vor-der-heilung.txt`) |
| 2 — Tageswechsel gruppiert weiter | `regroupsWhenTheWindowIsActivated` unverändert und im **roten** Lauf grün — er misst also weiter seinen eigenen Fall |
| 3 — Auswahl nicht in Zeile 0 | `QVERIFY(selected.row() > 0)` **und** `QVERIFY2(!viewport()->rect().intersects(visualRect(selected)), …)` — der zweite Satz schließt den Aufbau aus, in dem der Fehler gar nicht auftreten kann |

Der dritte Punkt ist der, an dem diese Story hätte scheitern können: Der alte
Test war nicht falsch, er stand nur an einer Stelle, an der nichts zu sehen
war. Der neue sichert seine eigene Voraussetzung zu.

Gemerged als `4d3978c`.
