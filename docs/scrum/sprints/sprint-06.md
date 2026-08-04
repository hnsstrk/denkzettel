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
`bugprone-unused-return-value`.

*Einordnung des PO, nach Ansicht der Stelle:* **Die Warnung ist gegenstandslos,
und der Code sagt das selbst.** `globalshortcuts.cpp:89–93` verwirft den
Rückgabewert von `KGlobalAccel::setGlobalShortcut()` mit Begründung: Er *kann*
kein Scheitern des Dienstes zeigen, weil `doRegister()` seinen D-Bus-Aufruf
absetzt und die Antwort fallen lässt. Die Stelle prüft stattdessen zurück, was
der Dienst tatsächlich hält (`:99–104`) — genau der Weg, den SPEC 2.4 nach
Retro-Beschluss B5 vorschreibt, nachdem der Kunde beide stillen Fehler am
01.08.2026 je einmal getroffen hat.

Wer diese Warnung „behebt", indem er den Rückgabewert auswertet, ersetzt eine
belastbare Rückfrage durch eine Zusicherung, die nichts weiß. **Nicht anfassen.**
Die übrigen Befunde sind Bestand.

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

---

## 17. DoD-Prüfung Sprint 6

### 17.1 Was der Sprint-Diff außer den drei Stories enthält (K4)

`git diff sprint-06-basis..main` umfasst **73 Dateien, 6025 Zeilen**. Die drei
Stories erklären davon nicht alles. Der karpathy-Review hat das als Befund K4
gemeldet — nicht weil etwas fehlte, sondern weil mein Review-Auftrag nur die
Stories nannte und die Deckung damit unausgesprochen blieb. Ein Diff, dessen
Umfang niemand erklärt, wird später entweder als Lücke gelesen oder als stille
Behauptung.

Der Beifang stammt aus der PO-Arbeit desselben Tages, vor und neben dem Sprint:

| Teil | Beleg |
|---|---|
| Klangfreiheit der Testläufe (`tests/testsilence.cpp`, `tests/CMakeLists.txt`, SPEC 9) | `2026-08-04-testklaenge.md`, `2026-08-04-klangfrei.md`, `2026-08-04-testklaenge-nachtrag.md` |
| BM25 geprüft und verworfen (SPEC 6) | `2026-08-04-bm25/` |
| CI eingerichtet und erweitert (`.github/workflows/ci.yml`) | `2026-08-04-ci/` |
| KI-Transparenzhinweis (`README.md`) | Kundenentscheidung 04.08.2026 |
| Schätzkegel-Rückbau, Bildläufer nicht mehr `EXCLUDE_FROM_ALL`, sp:-Label-Regel | `sprint-06-kegel-rueckbau.md`, `2026-08-04-v1-bildlaeufer.md`, `2026-08-04-v2-v4-label.md` |
| `sprint-05-installationstakt.md`, Verwalter-Berichte | Sprint-5-Nachlauf |

**Der Reviewer hat die codetragenden Teile davon mitgeprüft — ohne Befund.**
Das steht hier, damit die Prüfung nicht später als übersprungen gilt.

*Für den nächsten Sprint:* Der Review-Auftrag nennt den Diff, nicht die Stories.
Wer die Stories aufzählt, beschreibt seine Absicht; der Diff beschreibt, was
tatsächlich zur Prüfung ansteht. Die beiden fallen auseinander, sobald an einem
Tag auch außerhalb des Sprints gearbeitet wird — und das ist der Normalfall.

---

## 18. Abnahme-Checkliste für den Kunden

Was hier steht, kann kein Test dieses Projekts prüfen. Jeder Punkt hat einen
benannten Grund, warum er am Auge hängt — keiner ist Bequemlichkeit.

**Voraussetzung:** Der Endstand ist nach `/usr` installiert (DoD 2). Danach den
Dienst einmal neu starten, sonst läuft der alte weiter.

### #55 — die Fensterhülle

**1. Der erste Eindruck.** `Meta+N` drücken. Sieht das Fenster jetzt aus, als
gehörte es zu KDE? Das ist der Befund vom 01.08.2026 im Wortlaut: *„Sieht nicht
aus als ob es zu KDE gehören würde … Farben, Schriften, abgerundete Ecken"* —
und die Frage, die diese Story beantworten sollte.

**2. Der Schatten.** Liegt einer unter dem Fenster?

*Warum das am Auge hängt:* Gemessen ist, dass der Compositor die Kacheln
**annimmt** (`create() == wahr`, Kacheln 32×16 und 16×16). Nicht gemessen ist,
ob er gut aussieht. Genau diese Lücke hat den Strang einen echten Fehler
gekostet — sein erster Bau übergab acht Mal dasselbe Bild statt acht Kacheln,
und `create()` meldete auch dafür wahr.

**3. Der Schatten beim zweiten Mal.** Fenster schließen, `Meta+N` erneut
drücken — liegt der Schatten noch darunter?

*Warum:* `showCapture()` zerstört die Fensterfläche bei jedem Anzeigen, der
Schatten muss also jedes Mal neu gebunden werden. Am Standbild ist das nicht zu
sehen, und ein Unit-Test erreicht es nicht. Der Strang hat es am laufenden
Compositor gemessen und beide Male „ja" bekommen; dies ist die Gegenprobe mit
dem Auge.

**4. Der Theme-Wechsel im Betrieb.** Desktop-Theme in den Systemeinstellungen
umstellen, **ohne** den Dienst neu zu starten — wechselt die Hülle mit?

*Warum:* Belegt sind die drei Teilstücke einzeln — die Hülle folgt einem neuen
Namen am stehenden Fenster, sie liest ihn aus `plasmarc`, und eine Änderung
dieser Datei erreicht die Wache. **Der Zusammenbau ist nicht end-to-end
geprüft.** Das ist die Fehlerklasse aus #54, eine Ebene höher.

### #59 — die ruhige Liste

**5. Alt-Tab mit der Hand.** Bibliothek öffnen, eine Notiz weit unten
auswählen, ganz nach oben rollen, in ein anderes Fenster wechseln und
zurückkommen. Bleibt die Liste stehen?

*Warum:* Der Strang hat den Weg über das Schließen des davorliegenden Fensters
gemessen — beides läuft über dasselbe Compositor-Ereignis, aber **den
Tastendruck kann ein Agent unter Wayland nicht auslösen**: Ein Prozess kann
sich den Fokus nicht selbst zuteilen. Vorher waren es 459 px Rücksprung bei
552 px Sichthöhe.

### Zwei Punkte aus dem UI-Review

**6. Das zweite Öffnen.** Eine lange Notiz tippen, mit `Esc` verwerfen, Kürzel
erneut drücken — steht das Fenster wieder auf fünf Zeilen?

*Warum:* Es steht nicht. Gemessen: 228 px leeres Fenster statt 174. Ab der
ersten längeren Notiz öffnet sich das Fenster für den Rest der Sitzung in der
Höhe der längsten je getippten Notiz. Der Befund ist **vorbestehend** und als
[#79](https://github.com/hnsstrk/denkzettel/issues/79) gebucht — er steht hier,
weil das UI-Review ihn für den einzigen Befund dieses Sprints hält, **den der
Kunde in der Abnahme von selbst finden wird.** Besser, er steht auf der Liste,
als dass er als Überraschung kommt.

**7. Die Ecken auf hellem Hintergrund.** Laufen die Ecken des Fensters sauber
um, oder endet die Linie vor der Rundung?

*Warum:* Sie endet. Auf allen vier geraden Kanten sitzt die Konturfarbe exakt;
auf den Eckbögen kommt sie **nirgends** vor — beim schmalen Theme beginnt die
Kontur erst 6 px nach der Ecke, beim breiten erst 10. Im dunklen Schema
kaschiert der Alphaverlauf das, im hellen endet der Umriss sichtbar im Nichts.
Gebucht als [#80](https://github.com/hnsstrk/denkzettel/issues/80). Der Befund
ist 1 px breit und blockiert die Abnahme nicht — aber Du sollst wissen, worauf
Du schaust.

### Was ausdrücklich **nicht** auf dieser Liste steht

**#56 ist am Kundenblick nicht prüfbar.** Plasma reicht Qt-Widgets-Anwendungen
Schriftänderungen nicht nach (Befund B6 vom 01.08.2026, kein Fehler unseres
Codes — eine nackte Qt-Anwendung verhält sich genauso). Wer die Systemschrift
umstellt, sieht am Erfassungsfenster nichts, weder vorher noch nachher. Der
Nachweis ist Test und Bild, und das ist keine Auslassung, sondern die Lage.

Die Story wurde trotzdem gebaut, und zwar **vor** #68: Sobald jemand die
Schriftzustellung nachrüstet, tritt der Fehler sofort zutage. Wer es
andersherum macht, baut eine Verbesserung und liefert die Verschlechterung mit.

---

## 19. DoD-Prüfung Takt 1 (Scrum Master)

**Datum:** 2026-08-04, 15:53 (Ganymed) · **Prüfer:** Scrum Master, frischer
Kontext · **Prüfstand:** `main` @ `96cf51f` · **Prüfmittel:** eigener Bauplatz
außerhalb des Repositoriums (`cmake -B <sandkasten> -S . -DCMAKE_BUILD_TYPE=Debug`),
damit `build/` und `build-install/` unberührt bleiben.

**Ergebnis in einem Satz:** DoD 1, 3 und 4 tragen und sind nachgemessen; **fünf
Mängel** gehen an den PO, davon zwei mit Gewicht — die letzte öffentliche Marke
von `main` ist **rot** (M1), und für #55 und #56 fehlt die dokumentierte
PO-Abnahme (M3). Punkt 1 des Abschlusses ist **offen**, nicht mangelhaft.

### 19.1 Abschluss-Punkt 1 — Installation nach `/usr`: **offen**

Gebucht als offen, nicht als Mangel: Der PO hat ihn beauftragt, er hängt am
Kundenpasswort.

| Gegenstand | Befund |
|---|---|
| installierter Stand | `/usr/bin/denkzetteld` trägt **02.08.2026 17:39** — der Stand aus Sprint 5 |
| Bau für die Installation | `build-install/CMakeCache.txt`: `CMAKE_INSTALL_PREFIX=/usr`, `CMAKE_BUILD_TYPE=Debug`; `bin/denkzetteld` gebaut **15:28** |
| deckt der Bau den Endstand? | **ja** — die letzte Änderung unter `src/` ist `2ef495f` (15:11); `c0c623d` und `96cf51f` fassen nur `tests/captureshots.cpp`, Belege und README an |

**Was mit Punkt 1 offen bleibt:** die Hauptwege der drei Stories am
installierten Stand (DoD 2, Ersatzpflicht nach Sprint-3-Mangel M1) und damit
der einzige Nachweis, der ausschließlich dort zu führen ist — der Schatten
(§6.2). **#56 hat dort keinen Hauptweg**, das ist die benannte Grenze aus
§4.1.1 und im Takt zu notieren, nicht stillschweigend auszulassen.

### 19.2 Abschluss-Punkt 2 — Prüfsummen der Bildbelege: **kein Fund**

```
bash docs/scrum/bildbelege-pruefen.sh docs/scrum/reviews/sprint-06-s55-huelle   -> RC 0 (15 Bilder)
bash docs/scrum/bildbelege-pruefen.sh docs/scrum/reviews/sprint-06-ux-review    -> RC 0 (19 Bilder)
```

Kein Urteil zu fällen: Das Skript hat nichts gefunden. **Erster Lauf der
Stop-Bedingung — 1 von 3.**

**Die Bilder sind darüber hinaus nachgefahren, nicht nur gezählt.** Der Läufer
frisch gebaut, dann gegen die versionierte Reihe gehalten:

```
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde <sandkasten>/bin/captureshots <ausgabe>
-> 14 von 14 Bildern byteweise gleich mit sprint-06-s55-huelle/bilder/
   ("Desktop-Themes dieser Bildreihe (installiert): schmal=breeze-dark  breit=CachyOS-Nord-round")
```

Damit ist die Regel aus `CLAUDE.md` („Ein Bildbeleg ist erst ein Beleg, wenn
sein Läufer frisch gebaut ist") **am Ergebnis** geprüft und nicht an der Zusage
der Berichte. Der Vorfall aus Sprint 5 — grüner Test und falsches Bild zugleich
— ist hier ausgeschlossen.

### 19.3 Abschluss-Punkt 2 — Vollzähligkeit der Prüfberichte: **vollzählig**

Prüfweg nach PROZESS: Commit-Botschaften, die Befunde eines Prüflaufs nennen,
gegen die abgelegten Berichte.

| Prüflauf | Commit | Bericht als Datei |
|---|---|---|
| karpathy, Erstlauf (1 fail, 3 warn) | `ee47602` | `sprint-06-karpathy.md`, Abschnitt „Befunde" |
| karpathy, **Nachprüfung** (Verdict ok) | `c85ffbe` | **dieselbe Datei**, Abschnitt „Wiedervorlage 04.08.2026" — beide Commits fassen sie an |
| UI-Review (kein fail, 4 warn) | `63645a8` | `sprint-06-ux-review/bericht.md` samt 19 Bildern und `pruefen.sh` |
| Übergabe Strang A / Strang B | `38db754` / `4ac9e83` | `sprint-06-s55-huelle/bericht.md` · `sprint-06-s59-scrollstelle/bericht.md` |
| U4-Behebung | `c0c623d` | Nachtrag §12 im Strang-A-Bericht |
| PO-Arbeit desselben Tages (karpathy-Tagesreview, Bildbelege, BM25, CI, Klangfreiheit, Kegel-Rückbau, `sp:`-Label) | `dfdb5be`…`0a229d2` | je eigene Datei unter `docs/scrum/reviews/` — alle vorhanden |

**Kein Commit dieses Sprint-Diffs nennt einen Prüflauf ohne abgelegten
Bericht.** Der Sprint-3-Fall (`e18630c`) wiederholt sich nicht. Die
Prozessänderungen dieses Sprints (`PROZESS.md`, `denkzettel-verwalter.md`) sind
durch `2026-08-04-karpathy.md` gedeckt — Verdict `warn`, kein `fail`.

### 19.4 Die beiden Bildreihen — geprüft, kein Mangel, aber eine Stelle altert

Der Strang hat seine zwölf Hüllenbilder nach U4 neu erzeugt (`c0c623d`); die
Reihe des UI-Reviews dokumentiert den geprüften Stand und bleibt.

- **Der Strang-Bericht sagt es**, §12.3: *„Die Bilder des UI-Reviews … sie
  dokumentieren den geprüften Stand. Dass meine Reihe seit dieser Korrektur
  davon abweicht, ist richtig so."* Dazu §12.2 mit ausgezählten
  Pixel-Unterschieden.
- **Der UX-Bericht sagt es nicht**, sondern trägt in §1 weiterhin: *„Meine 14
  Läuferbilder sind byteweise identisch mit denen des Strangs."* Der Satz war
  an seinem Prüfstand (`2ef495f`, im Kopf genannt) wahr und ist heute falsch.

**Kein Mangel** — ein Bericht ist Beweislage seines Standes, und ein
nachträglich geglätteter Beleg wäre keiner mehr (`346a4c0`). **Aber der Satz
steht im Präsens und ohne Anker**, und aufgelöst wird der Widerspruch nur in
der *anderen* Datei. *Vorschlag an den PO (nicht geheilt):* eine angehängte
Zeile im UX-Bericht, die auf `c0c623d` verweist — dieselbe Bauart wie die
Belegnachträge vom Vormittag.

### 19.5 DoD 1–4 je Story

Alle Läufe im eigenen Sandkasten, an `96cf51f`.

| | **#56** (1 SP) | **#55** (8 SP) | **#59** (2 SP) |
|---|---|---|---|
| **DoD 1** Bau + Tests | ✓ | ✓ | ✓ |
| **DoD 2** AK erfüllt · **PO-Abnahme** | Belege ✓ · **fehlt (M3)** | Belege ✓ · **fehlt (M3)** | ✓ §16.2, AK-Tabelle gegen Belege gelesen |
| **DoD 3** karpathy · UI-Review | ok · kein fail (P25) | ok · kein fail (P1–P26) | ok · keine UI-Story (§15.2) |
| **DoD 4** SPEC nachgezogen | SPEC 3 | SPEC 3.1, **3.2 (5 Bedingungen)**, 15, 16 | SPEC 9 (1 Bedingung) |

**DoD 1, gemessen statt geglaubt:**

```
Neubau                                                0 Compiler-Warnungen
ctest                                                 7/7 (appstreamtest ist die Leerstelle aus §13.5)
capturetest  QT_QPA_PLATFORM=offscreen                21 passed, 0 failed, 0 skipped
capturetest  … + QT_QPA_PLATFORMTHEME=kde             21 passed, 0 failed, 0 skipped
librarytest  … + QT_QPA_PLATFORMTHEME=kde            108 passed, 0 failed, 0 skipped
```

Der zweite Halbsatz von DoD 1 — Raumaufteilung bei **zwei Fenstergrößen** — ist
durch `hullIsCompleteAtFiveAndEightLines()` (`capturetest.cpp:498`, 174 gegen
228 px) gedeckt, geprüft unter beiden Themes und beiden Schemata (UX P24).

**DoD 4 in der B9-Fassung — die Behauptung trägt**, am Diff `sprint-06-basis..main`
von SPEC.md gelesen, nicht am Bericht:

- **SPEC 3.2 führt genau fünf entdeckte Bedingungen**: KSvg liest `plasmarc`
  nicht selbst · ein `FrameSvg` folgt nur einem **frischen** `ImageSet` ·
  Zustellung über `KDirWatch`, nicht `KConfigWatcher` · ohne Theme keine Hülle,
  aber ein brauchbares Fenster (**und ein unbekannter Name erzeugt diesen
  Zustand nicht**) · der Schatten wird nach jedem Neuzeigen neu gebunden.
- **SPEC 9 führt die eine Bedingung zu #59**: neu gruppiert wird nur bei
  **anderem Kalendertag**, mit dem gemessenen Wert (459 px) und der Begründung,
  warum der Kalendertag als Bedingung genügt.
- Dazu SPEC 3 (Höhe folgt der Schrift, #56), SPEC 15 (`KSvg`, `KWindowShadow`,
  `KCoreAddons` — der wörtliche B9-Beispielfall) und **drei** neue Absätze in
  SPEC 16 (Grenzen der Offscreen-Prüfbarkeit, Zustände mit eigenem Prozess,
  keine Zusicherung an einem Maschinennamen).

**Was an DoD 4 fehlt, siehe M5:** die Begründung **im Issue**.

### 19.6 Schätzhistorie — die Bedingung an #56 hat gehalten

| Story | Erstschätzung | Provenienz | Revision im Sprint | Label heute |
|---|---|---|---|---|
| #55 | 8 | Sprint-5-Planning, Dev 8 · UX 8 — 2 unabhängige | keine | `sp:8` |
| #56 | 1 | Anlage (1 Hand), **bestätigt** 02.08. durch Zweitschätzung | keine | `sp:1` |
| #59 | 2 | Sprint-5-Planning, Dev 2 · UX 2 | keine | `sp:2` |

**Die Bedingung aus §2.3 ist am Commit geprüft, nicht am Bericht:**
`tests/captureshots.cpp` ist mit **`38db754` (#55)** angelegt; der #56-Commit
`48d73e5` fasst ausschließlich `src/capture/capturewindow.cpp` (+11) und
`tests/capturetest.cpp` (+37) an. **#56 hat den Läufer nicht bezahlt** — die 1
bleibt 1, der Prüfsatz aus §9 („kippt die Zuordnung, wird #56 zur 2") ist nicht
ausgelöst.

**#59 hat keine Zeile in §9.** Die Tabelle entstand beim Planning, bevor #59 mit
der Freigabe hinzukam; seit dem Kegel-Rückbau (§13.4) ist die Historie kein
Pflichtteil mehr. **Kein Mangel**, festgehalten zur Vollständigkeit.

**Sprint-Konto geprüft:** Milestone „Sprint 6" trägt genau #55, #56, #59 — **3
Issues, 11 SP**, beide Grenzen gehalten. Die drei im Sprint angelegten Issues
**#79, #80, #81** hängen an **keinem** Milestone; es gab **keinen Zugang** nach
der Freigabe.

### 19.7 Doku-Abgleich (B10)

Der Nachzug des PO in `96cf51f` ist nachgeprüft und trägt: `README.md:56–57`
nennt `Svg`, `README.md:61–62` `ksvg` **und** `kcoreaddons`, dazu der Satz zum
fehlenden Desktop-Theme (`:65–67`). **An derselben Datei bleiben zwei Stellen
stehen — M4 und M2.** Die Statuszeile nach B10 existiert in dieser README nicht
als eigene Zeile; ihre Funktion nimmt der Absatz „Auf der Liste stehen noch"
wahr, und genau dort sitzt M2.

---

### 19.8 Mängelliste an den PO (melden, nicht heilen)

**M1 · schwer — Die letzte öffentliche Marke von `main` ist rot, und kein
Artefakt dieses Sprints nennt sie.**

`gh run list`: Lauf **30912635307** auf `2ef495f` („Merge Strang A") —
**failure**. Grund laut Lauf: *„16 clazy-Befunde, erlaubt sind 3 (Altbestand vom
04.08.2026)."* Alle Läufe davor grün.

Am Prüfstand nachgemessen (`cmake --build <sandkasten> --target lint-clazy`):
**13 verschiedene Befunde**, die CI zählt 16, weil `desktopthemes.h` über
`capturetest.cpp` und `captureshots.cpp` zweimal in den Lint geht. Davon sind
**drei** der Altbestand, den die Schwelle 3 abbildet (`librarytest.cpp:2387,2393`,
`shelltest.cpp:361`) — die übrigen **zehn stammen aus diesem Sprint**:

```
src/capture/capturewindow.cpp:57,58,59,60   non-pod-global-static
src/capture/capturewindow.cpp:192           connect-non-signal
tests/desktopthemes.h:91,112,135            range-loop-detach
tests/capturetest.cpp:512,573               range-loop-detach
```

**Warum das mehr ist als eine Linterzeile:** `1006d33` in **diesem** Sprint hat
denselben Fall behandelt und benannt — *„Bau grün, ctest grün,
Compiler-Warnungen null … der Linter im automatischen Lauf war der Einzige, der
es sah."* Beim zweiten Mal ist er stehengeblieben. `CLAUDE.md` verlangt: *„Wer
pusht, sieht nach: `gh run list --limit 1`."* Der Nachschlag ist unterblieben
oder folgenlos geblieben; weder Sprint-Protokoll noch Übergabebericht erwähnt
den roten Lauf.

*Zur Genauigkeit:* **DoD 1 ist davon nicht gerissen** — sie verlangt einen
warnungsarmen Bau, und Compiler-Warnungen sind null. Gerissen ist die
Kundenentscheidung vom 04.08.2026 über die automatischen Testläufe, deren
Schwelle „sinkt die Zahl, wird die Schwelle nachgezogen" lautet — hier ist sie
gestiegen.

**M2 · mittel — `README.md:47–49` führt „runde Fensterecken" weiter unter dem,
was noch aussteht.** Der Satz *„Auf der Liste stehen noch: … Export nach
Obsidian und Taskwarrior, runde Fensterecken"* beschreibt einen Stand, den `main`
seit `2ef495f` nicht mehr hat; die Funktionsliste (`:34–43`) nennt die Hülle
gar nicht. Das ist die B10-Bauart an einer anderen Stelle: Die README beschreibt
den **gelieferten** Stand. Spätestens mit der Kundenabnahme ist der Satz
öffentlich falsch.

**M3 · schwer — Für #55 und #56 ist die PO-Abnahme nirgends dokumentiert.**
DoD 2 verlangt *„Akzeptanzkriterien des Issues erfüllt **und vom PO
abgenommen**"*. Für **#59** steht sie in §16.2, mit AK-Tabelle und dem
ausdrücklichen Satz, dass der PO die Belege gelesen hat und nicht den Bericht.
Für #55 und #56 findet sich nichts: §17 enthält nur 17.1, und die letzten
Issue-Kommentare stammen vom 01. und 02.08.2026, also aus der Zeit vor der
Umsetzung. Die Berichte der Stränge sind Selbstauskunft und ersetzen die
Abnahme nicht — sonst nimmt die Story sich selbst ab.

**M4 · mittel — Drei Dateien beschreiben die Bildläufer falsch oder
unvollständig; die wichtigste davon liest jede Sitzung von selbst.**

| Ort | Aussage | Befund |
|---|---|---|
| `CLAUDE.md:84–85` | „Die Bildläufer (`editshots`, `libraryshots`, `searchshots`, `readmeshots`) sind `EXCLUDE_FROM_ALL` — ein gewöhnlicher Build fasst sie nicht an" | **zweifach überholt**: `tests/CMakeLists.txt:93` sagt *„They were EXCLUDE_FROM_ALL once"*, und **`captureshots` fehlt in der Liste** |
| `README.md:110` | „Die vier Bildläufer …" | EXCLUDE-Aussage richtig, **`captureshots` fehlt** |
| `.github/workflows/ci.yml:11–13` | „die Bildläufer … sind EXCLUDE_FROM_ALL und werden hier nicht gebaut" | **falsch** — sie werden gebaut |

Die `CLAUDE.md`-Stelle wiegt am schwersten, weil sie dort eine **Prüfpflicht
begründet** („Vor jedem Bildbeleg: `cmake --build build --target <läufer>`").
Wer die Begründung als überholt erkennt, legt die Pflicht mit ab — und die
Pflicht gilt weiter, nur aus einem anderen Grund.

**M5 · leicht — DoD 4 verlangt die Begründung *im Issue*; an keiner der drei
Stories steht sie.** Der SPEC-Nachzug selbst trägt (19.5). An #55 und #56 enden
die Kommentare am 02.08.2026, **#59 hat überhaupt keinen Kommentar**.
*Empfehlung:* mit dem Abnahmekommentar in Takt 2 erledigen — dann ist es ein
Satz mehr und keine Nacharbeit.

### 19.9 Zwei Feststellungen ohne Mangelcharakter

1. **Acht Commits sind nicht gepusht** (`ee47602`…`96cf51f`; `origin/main` steht
   auf `2ef495f`). Der Push ist **Takt 2, Punkt 7** — kein Takt-1-Mangel. Er
   gehört trotzdem hierher, weil er an M1 hängt: Der öffentliche Stand ist
   heute der **rote**, und ein Push ohne vorherige Behebung erzeugt eine zweite
   rote Marke. **Reihenfolge: erst M1, dann pushen.**
2. **Das done/next des Sprint-Endes fehlt.** §12 ist das done/next des
   Plannings. Der Pflichtteil verlangt eines je Protokoll; es entsteht sinnvoll
   erst mit Takt 2.

### 19.10 Was ich ausdrücklich **nicht** geprüft habe

- **Den Sprint-Diff auf seine Deckung** — in §17.1 aufgeschlüsselt und vom
  karpathy-Reviewer bestätigt; auf Weisung des PO nicht erneut erhoben.
- **DoD 5 und DoD 6** — Takt 2, vor der Abnahme nicht erfüllbar. Sie hier zu
  buchen wäre die Wiederholung der Sprint-3-Mängel M2 und M5.
- **Den installierten Stand** — Punkt 1 ist offen (19.1).
- **Die vier `warn`-Befunde des UI-Reviews und die drei des Tagesreviews.** Kein
  `fail`, damit ist DoD 3 erfüllt; U1/U2/U3 sind als #80, #79, #81 gebucht, U4
  ist behoben.

---

## 20. Abnahme #55 und #56 durch den PO (M3)

Nachgeholt auf Mangel M3 der DoD-Prüfung. Für #59 stand die Abnahme seit dem
Merge in §16.2; für die beiden Stories aus Strang A gab es nur die
Selbstauskunft des Strangs — und die ist keine Abnahme. Der Scrum Master hat
recht: Ein Bericht sagt, was der Erbauer gemessen hat, nicht was der PO
angenommen hat.

Geprüft habe ich die Belege, nicht die Berichtssätze.

### 20.1 #56 — Feldhöhe folgt einer Schriftänderung

| AK | Beleg, den ich gelesen habe |
|---|---|
| 1 — Höhe entspricht nach einer Schriftänderung wieder SPEC 3 | `heightFollowsAFontChange()` prüft ruhend `Höhe − Chrom == 5 × Zeilenabstand` **und** nach acht Zeilen `== 8 × Zeilenabstand`; vor der Heilung rot mit *Actual 85, Expected 215* |
| 2 — prüfbar ohne Plasma-Schriftumstellung, Test setzt die Schrift direkt | Derselbe Test setzt `text->setFont(...)` — Weg C der Schätzmessung. Die Heilung sitzt im `eventFilter`, nachgeprüft am Code (`capturewindow.cpp`), nicht am Bericht |
| 3 — mindestens zwei deutlich verschiedene Schriftgrößen | 9 pt und 24 pt im selben Test; Bilder 13/14 |

**Angenommen.** Die Zusicherung ist relativ geführt — gemessen wird gegen den
Zeilenabstand der jeweiligen Schrift, nicht gegen eine Pixelzahl. Ein absoluter
Vergleich hätte die Schriftauswahl der Maschine mitgeprüft.

**Was die Annahme nicht deckt:** #56 ist am Kundenblick nicht prüfbar (§18).
Das ist keine Auslassung, sondern die Lage — und der Grund, warum die Story
**vor** #68 gebaut wurde.

### 20.2 #55 — Capture-Fensterhülle

| AK | Beleg, den ich gelesen habe |
|---|---|
| 1 — Rundung, Kontur, Schatten aus dem Theme; Randmaß **und** Eckform unterscheiden sich | `hullFollowsTheDesktopTheme()`; `theme-eckstuecke.txt` über alle acht Themes. Nach der Nachbesserung `wideCorner != narrowCorner` statt `>` — die Eckform folgt dem Rand nicht, und die alte Fassung leitete sie daraus ab |
| 2 — durchgehende Fläche, Notiztext auf `WindowText` | `paintsOneSurfaceInThePaletteColours()`, `noteTextUsesTheWindowTextRole()`; unabhängig am Bild bestätigt (UI-Review P9: dieselbe Farbe links des Feldes, im Feld und rechts davon) |
| 3 — Innenabstände zuzüglich Theme-Rand; Fußzeile luftiger als der App-Name | UI-Review P1/P3/P16/P17: 16/14/16/12 bei Rand 4, 20/18/20/16 bei Rand 8; 12 gegen 8 unter **beiden** Themes |
| 4 — Hülle bei 5 wie bei 8 Zeilen vollständig | `hullIsCompleteAtFiveAndEightLines()`, nach Mutationsprobe um `QVERIFY(cornerRun > 0)` ergänzt — ohne diese Zeile blieb der Test grün, obwohl gar keine Hülle gezeichnet wurde |
| 5 — Theme-Wechsel bei laufendem Dienst | `hullFollowsTheDesktopTheme()` am stehenden Fenster, `readsTheDesktopThemeFromPlasmarc()`; Zustellung über `KDirWatch` gemessen. **Der Zusammenbau ist nicht end-to-end geprüft** — benannte Grenze, Punkt 4 der Abnahme-Checkliste |
| 6 — keine Titelleiste, kein Dekorationsrahmen | `wearsNoDecoration()`, Mutationsprobe nachgefahren; Bild 15 aus der laufenden Sitzung |
| 7 — Belegformen getrennt | Zwölf Bilder aus frisch gebautem Läufer, nach U4 neu erzeugt. Schatten: `create() == wahr` am laufenden Compositor, Kacheln 32×16 und 16×16, auch nach dem zweiten Zeigen |
| 8 — `KF6::Svg` verkabelt, SPEC nachgezogen, Randfall ohne Absturz | Vier Bibliotheken statt einer, aus Messung begründet; SPEC 3/3.1/3.2/15/16, vom Scrum Master am SPEC-Diff gegengelesen; `staysUsableWithoutADesktopTheme()` in eigenem Prozess |

**Angenommen**, mit drei benannten Grenzen der Prüfbarkeit: das **Aussehen**
des Schattens, der **Zusammenbau** des Theme-Wechsels und die **Skalierung > 1**
(vom UI-Review nicht geprüft und dort als erster Nachsehpunkt zu #80 benannt).
Alle drei stehen in der Abnahme-Checkliste oder als Issue — keine ist
stillschweigend geblieben. Eine benannte Grenze schließt die Story nicht
(DoD 2).

**Nicht Teil der Abnahme, sondern gebucht:** #79 (Fenster schrumpft nicht
zurück, vorbestehend), #80 (Kontur an der Rundung), #81 (Textkante). Keiner
der drei berührt ein Akzeptanzkriterium dieser Story.

---

## 21. Vollzug der DoD-Mängel (PO)

Die Mängelliste des Scrum Masters steht in §19.8. Hier der Vollzug, je mit dem
Beleg, an dem ich ihn geprüft habe — nicht mit der Meldung, die ihn ankündigte.

| Mangel | Vollzug | Beleg |
|---|---|---|
| **M1** — öffentliche Marke rot, 16 clazy-Zeilen bei Schwelle 3 | **behoben** | Zehn Befunde geheilt (`bb1dcd2`), von mir selbst nachgezählt: **3 Zeilen, 3 Stellen** — `librarytest.cpp:2387`, `:2393`, `shelltest.cpp:361`, genau der Altbestand. CI-Lauf **30917700346 = success** |
| **M2** — README führt „runde Fensterecken" unter dem Ausstehenden | **behoben** | `c3759f0`: aus der Liste entfernt, dafür die Hülle in den Funktionen |
| **M3** — PO-Abnahme für #55/#56 nirgends dokumentiert | **behoben** | §20, AK-Tabellen in der Form von §16.2, mit drei benannten Grenzen der Prüfbarkeit |
| **M4** — drei Dateien beschreiben die Bildläufer falsch | **behoben** | `c3759f0`: `CLAUDE.md`, `README.md`, `ci.yml`; `captureshots` ergänzt, `EXCLUDE_FROM_ALL` richtiggestellt |
| **M5** — DoD-4-Begründung fehlt im Issue | **Takt 2** | Der Abnahmekommentar trägt sie; vor der Kundenabnahme nicht schreibbar |

### 21.1 Was an M1 und M4 hängenbleibt

Beide sind **meine** Fehler, und sie haben dieselbe Bauart.

Bei **M1** habe ich nach dem Merge `gh run list` gelesen und den grünen Lauf
davor für meinen gehalten. `CLAUDE.md` sagt: *„Wer pusht, sieht nach."* Ich habe
nachgesehen und trotzdem das Falsche gesehen — die Regel nennt die Handlung,
nicht ihren Gegenstand. **Vorschlag für die Retro:** den Satz um den Gegenstand
ergänzen — nachgesehen wird der Lauf **des eigenen Commits**, nicht der oberste
der Liste.

Bei **M4** habe ich am Vormittag die Bildläufer von `EXCLUDE_FROM_ALL` befreit
und die Regel darüber stehen lassen, die sich darauf beruft. An genau diesem
Satz hängt die Prüfpflicht für Bildbelege — die Regel, die nach dem
Sprint-5-Vorfall geschrieben wurde. Sie war einen halben Tag lang falsch
begründet.

**Der gemeinsame Nenner:** Beide Male habe ich einen Zustand geändert und die
Aussage über ihn nicht. Das ist dieselbe Klasse wie die überholten Präsens-Sätze
im UI-Bericht — nur dort hat sie jemand gefunden, bevor sie schadete.

---

## 22. Abschluss-Punkt 1 — Installation und Prüfung am installierten Stand

**Vollzogen am 04.08.2026, 16:19.** Der Endstand ist einmal nach `/usr`
installiert (`pkexec cmake --install build-install`, Exit 0, `denkzetteld`
ersetzt).

### 22.1 Ein Befund beim Installieren, und er ist der teuerste des Sprints gewesen

Nach der Installation lief der Dienst **weiter mit der alten Binärdatei**:

```
PID 4569, gestartet 07:45:41
/proc/4569/exe -> /usr/bin/denkzetteld (deleted)
```

Die Datei war ersetzt, der Prozess hielt den gelöschten Inode. **Eine Abnahme
zu diesem Zeitpunkt hätte den Stand vom 02.08.2026 geprüft** — und zwar ohne
jedes Anzeichen: Das Fenster wäre erschienen, das Kürzel hätte funktioniert,
nur die Hülle wäre nicht dagewesen, und niemand hätte gewusst warum.

**Das ist Sprint-3-Mangel M1 in seiner ursprünglichen Form** — dort prüfte ein
Strang den Stand eines anderen, hier hätte der Kunde den Stand von vorgestern
geprüft. Strang B hat denselben Mechanismus von der anderen Seite gemeldet
(M-B2: `KDBusService::Unique` reicht den Start des Debug-Builds an den
laufenden Dienst weiter).

**Geheilt:** Dienst beendet und neu gestartet. Neuer Stand:

```
PID 514464, gestartet 16:19:37
/proc/514464/exe -> /usr/bin/denkzetteld     (ohne "deleted")
```

**Für die Retro:** Der Sprint-Abschluss sagt „der Endstand ist einmal nach
`/usr` installiert". Installieren genügt nicht — der laufende Dienst muss
danach der installierte sein. Vorschlag: Punkt 1 um die Rückfrage ergänzen,
und zwar in der belegbaren Form (`/proc/<pid>/exe` ohne `(deleted)`), nicht als
„Dienst neu gestartet".

### 22.2 Was am installierten Stand geprüft ist

| Prüfung | Ergebnis |
|---|---|
| Dienst läuft aus der installierten, nicht gelöschten Binärdatei | `/proc/514464/exe -> /usr/bin/denkzetteld` |
| Globales Kürzel registriert, beim Dienst **zurückgefragt** | Komponente `org_denkzettel_Denkzettel_desktop`, Kürzel `show-capture`, Anzeigename „Notiz erfassen" |
| Tastenkombination | `268435534` = `0x1000004E` = `Qt::META \| Qt::Key_N` = **Meta+N** |
| Desktop-Datei, über die KGlobalAccel auflöst | `org.denkzettel.Denkzettel.desktop` |

Das ist die Rückfrage aus SPEC 2.4 in der Fassung nach Retro-Beschluss B5 —
nicht die Zusicherung der Anwendung, sich registriert zu haben, sondern die
Antwort des Dienstes darauf, was er hält.

### 22.3 Was am installierten Stand **nicht** geprüft ist

**Der sichtbare Hauptweg jeder Story.** Ein Agent kann unter Wayland weder ein
globales Kürzel auslösen noch sich den Fokus zuteilen (Meldung M-B1 aus Strang
B, an diesem Sprint zum zweiten Mal aufgetreten). Damit ist der Weg „Meta+N
drücken, tippen, Strg+Enter" von hier aus nicht ausführbar.

**Das ist keine Auslassung, sondern die Zuständigkeit:** Genau dafür steht die
Abnahme-Checkliste in §18. Ihre sieben Punkte sind der Hauptweg, aufgeteilt in
das, was ein Auge sehen muss.

**Sprint 6 steht damit an der Kundenabnahme.**

---

## 23. Retrospektive Sprint 6

**Datum:** 2026-08-04, 16:28 (Ganymed) · **Moderation:** Scrum Master ·
**Prüfstand:** `main` @ `977e804`, CI-Lauf 30918437478 **completed/success**
(selbst abgefragt, nicht dem Auftrag entnommen — der nannte `2c69944`; seither
liegt ein Commit darüber, und sein Lauf war zur Beauftragung noch
`in_progress`. Das ist beiläufig und trotzdem der Gegenstand von B18).

**Zeitpunkt:** Takt 1 vollständig, Kundenabnahme offen. Diese Retro betrifft das
Verfahren, nicht das Produkt.

**Fortlaufend ab B16** (die Sprint-3-Retro endete bei B15).

### 23.1 Der Zusammenhang — er trägt, aber anders als vermutet

Die Vermutung des PO: Die Befunde M1 (falscher CI-Lauf gelesen), M4 (Regel über
die Bildläufer nach der Änderung stehengelassen) und die überholten
Präsens-Sätze im UI-Bericht sind **dieselbe Klasse**. Sie trägt: In allen drei
Fällen fällt eine **Aussage** von dem **Stand** ab, für den sie gilt.

Sie hat aber zwei Richtungen, und die Heilung ist je eine andere:

| | Fall | Was auseinanderfällt | Was hilft |
|---|---|---|---|
| **Schreibseite** | M4, UI-Bericht | Der Stand ändert sich, die Aussage bleibt stehen | Beim Ändern nach den Aussagen **suchen**; überholte Belege **ankern** |
| **Leseseite** | M1 | Die gelesene Aussage gehört zu einem **anderen** Stand als dem gemeinten | Den Gegenstand **benennen** (Commit-Kennung statt Listenkopf) |

**Eine gemeinsame Prüfung wäre der falsche Schluss** — und zwar aus einem Grund,
der am Material steht: **Die Prüfung gibt es bereits, und sie hat 3 von 3
gefunden.** Alle drei Fälle stammen aus derselben Quelle, dem Doku-Abgleich und
der DoD-Prüfung des Sprint-Endes (§19.4, §19.8 M1, §19.8 M4), geführt von einer
Rolle in frischem Kontext. **Keiner** ist von dem gefunden worden, der den
Zustand geändert hat.

Damit ist die Lücke nicht die fehlende Prüfung, sondern ihr **Zeitpunkt**: Sie
greift am Sprint-Ende, einen halben Tag nachdem `CLAUDE.md` eine Prüfpflicht
falsch begründete. Was fehlt, ist ein **Griff für den Verursacher im Moment der
Änderung** — und der muss so billig sein, dass er nicht zur Disziplinfrage wird.
Er ist es: siehe B17, ein `git grep`, gemessen fünf Zeilen Ausgabe.

**Eine Berichtigung zum Material:** Die Anker im UI-Bericht hat der Reviewer
**nicht** von selbst gesetzt. Gefunden hat den Befund der Scrum Master um 15:53
(§19.4, mit Vorschlag an den PO); die Anker stehen seit `98d9455` (16:02) — und
dort hat der Reviewer dann **drei** statt der einen vorgeschlagenen Zeile
gesetzt und zusätzlich #82 gebucht. Das ändert nichts am Wert des Vorgehens,
aber es ändert die Bilanz: **Dreimal derselbe Fund, dreimal derselbe Finder.**

### 23.2 Beschlüsse

Jeder Beschluss ist eine Änderung an einem benannten Artefakt. Der Einbau ist
Sache des PO — der Scrum Master ändert `CLAUDE.md`, `PROZESS.md` und die
globalen Regeln nicht selbst.

---

**B16 — Installieren heißt nicht laufen: welcher Stand läuft, wird belegt.**

*Anlass:* §22.1 — nach `cmake --install` hielt der Dienst die **gelöschte** alte
Binärdatei weiter (`/proc/4569/exe -> /usr/bin/denkzetteld (deleted)`). Eine
Abnahme zu diesem Zeitpunkt hätte den Stand vom 02.08.2026 geprüft, ohne jedes
Anzeichen. M-B2 aus Strang B ist derselbe Mechanismus von der anderen Seite:
`KDBusService::Unique` reicht den Start des Debug-Builds an den laufenden Dienst
weiter. Beide Male prüft man unbemerkt den falschen Stand — das ist
Sprint-3-Mangel M1 in seiner ursprünglichen Form.

*Warum eine Regel und nicht ein Vermerk:* Von allen sieben Befunden ist dies der
einzige, der eine **Kundenabnahme still entwertet** hätte. B13 taktet die
Installation gegen die Kollision zweier Stränge — gegen den laufenden Dienst
schützt sie nicht.

*Geändert 1:* `docs/scrum/PROZESS.md`, Sprint-Abschluss, Takt 1, **Punkt 1** —
angehängt:

> Installieren genügt nicht: Ein laufender Dienst hält nach `cmake --install`
> die **gelöschte** alte Binärdatei weiter und zeigt das an nichts. Vor der
> Prüfung wird deshalb belegt, dass der laufende Prozess der installierte ist —
> `readlink /proc/$(pgrep -x denkzetteld)/exe` muss auf `/usr/bin/denkzetteld`
> zeigen und **darf nicht** auf `(deleted)` enden. Ohne diesen Beleg prüft die
> Abnahme den Stand des vorigen Sprints (Sprint 6, §22.1).

*Geändert 2:* `CLAUDE.md`, Absatz „Geprüft wird am installierten Stand" —
angehängt:

> **Installieren heißt nicht laufen.** Nach `cmake --install` hält ein laufender
> Dienst die gelöschte alte Datei weiter; umgekehrt reicht `KDBusService::Unique`
> den Start eines Debug-Builds an den laufenden Dienst weiter. Beide Male prüft
> man unbemerkt den falschen Stand. Vor jeder Prüfung am installierten Stand:
> Dienst beenden, neu starten, dann `readlink /proc/$(pgrep -x denkzetteld)/exe`
> — ohne `(deleted)`. Wer den **Debug**-Stand prüfen will, beendet vorher den
> installierten Dienst.

*Automatisch geladen?* `CLAUDE.md`: **ja**, von jeder Sitzung im Projekt.
`PROZESS.md`, Sprint-Abschluss: **ja im Wirkmoment** — die Liste wird in Takt 1
Punkt für Punkt abgearbeitet, das ist kein zufälliges Lesen.
*Geprüft:* Der Befehl ist am 04.08.2026, 16:28 gelaufen und liefert
`/usr/bin/denkzetteld`, Rückgabe 0.

---

**B17 — Eine Aussage gilt für einen Stand: beim Ändern suchen, beim Überholen
ankern.**

*Anlass:* M4 (drei Dateien beschrieben die Bildläufer falsch, die schwerste in
`CLAUDE.md`, wo eine Prüfpflicht daran hängt) und die überholten Präsens-Sätze
des UI-Berichts. Schreibseite der Klasse aus §23.1.

*Der Griff ist gemessen, nicht ausgedacht.* Am Stand vor der Heilung:

```
git grep -n EXCLUDE_FROM_ALL c3759f0^ -- CLAUDE.md README.md docs/ .github/
    -> 3 Treffer: ci.yml:12, CLAUDE.md:85, tests/CMakeLists.txt:93
git grep -n readmeshots      c3759f0^ -- CLAUDE.md README.md docs/ .github/
    -> 5 Treffer, darunter alle drei Fundstellen von M4
```

Der zweite Griff ist der wichtigere und der unauffälligere: Wer eine
**Aufzählung erweitert** (`captureshots` als fünfter Läufer), findet die
veralteten Listen nicht über den neuen Namen — den kennen sie ja gerade nicht —,
sondern über den Namen eines **Geschwisters**.

*Geändert 1:* `CLAUDE.md`, Abschnitt „Prüfhaltung" — neuer Punkt:

> - **Eine Aussage gilt für einen Stand.** Wer eine Bau- oder
>   Werkzeugeigenschaft ändert, sucht im selben Zug nach den Aussagen darüber:
>   `git grep -n <Eigenschaft> -- CLAUDE.md README.md docs/ .github/`; beim
>   Erweitern einer Aufzählung nach dem Namen eines **Geschwisters** statt nach
>   dem neuen (`readmeshots` findet jede Liste, die `captureshots` noch nicht
>   kennt). Gemessen: Der Griff hätte alle drei Fundstellen von Sprint-6-Mangel
>   M4 gezeigt, bei fünf Zeilen Ausgabe. Wer den Zustand ändert und die Aussage
>   stehenlässt, macht aus einer Begründung eine Falle — die Pflicht gilt dann
>   weiter, nur aus einem anderen Grund.

*Geändert 2:* `docs/scrum/PROZESS.md`, Abschnitt „Definition of Done", Absatz
zum Doku-Abgleich — Umfang erweitert:

> Zur Sprint-Ende-Prüfung des Scrum Masters gehört der **Doku-Abgleich**:
> Beschreiben README, `docs/`, **`CLAUDE.md` und die Kommentarköpfe von
> `.github/workflows/ci.yml` und den `CMakeLists.txt`** den gelieferten Stand?

> *Grund für den Zusatz:* In Sprint 6 standen alle drei falschen Aussagen über
> die Bildläufer genau dort, und die schwerste in `CLAUDE.md` — außerhalb des
> bis dahin genannten Umfangs (M4). Gefunden hat der Scrum Master sie trotzdem;
> die Regel soll ihm das nicht als Kür überlassen.

*Geändert 3:* `docs/scrum/PROZESS.md`, Abschnitt „Artefakte", bei **Belege** —
angehängt:

> **Ein überholter Beleg wird geankert, nicht geglättet.** Der Berichtstext
> bleibt, wie er war; angehängt wird eine datierte Zeile, die den Prüfstand
> nennt und den Commit, seit dem der Satz nicht mehr gilt (`346a4c0`, `98d9455`
> sind die Bauart). Ein Bericht ist Beweislage seines Standes — wer ihn
> nachzieht, zerstört genau das, wofür B7 ihn ins Repo gestellt hat. **Sätze im
> Präsens ohne genannten Prüfstand sind die Stelle, an der das auffällt**
> (Sprint 6, §19.4).

*Automatisch geladen?* `CLAUDE.md`: **ja**. `PROZESS.md`: **ja für den Scrum
Master** (Agentendatei), der beide Stellen als Prüfer anwendet — und er ist bei
beiden der Adressat. Der Griff aus Änderung 1 adressiert **jeden**, deshalb
steht er in `CLAUDE.md` und nicht in `PROZESS.md`.

---

**B18 — Der Nachschlag gilt dem Lauf des eigenen Commits.**

*Anlass:* M1 (§21.1). `CLAUDE.md` sagt „Wer pusht, sieht nach." Der PO hat
nachgesehen und den grünen Lauf **davor** für seinen gehalten. **Die Regel nennt
die Handlung, nicht ihren Gegenstand** — und `--limit 1` liefert den obersten
Lauf, nicht den eigenen. Leseseite der Klasse aus §23.1.

*Geändert:* `CLAUDE.md`, Abschnitt „Technisches", letzter Absatz — der Satz
„Wer pusht, sieht nach: `gh run list --limit 1`." wird ersetzt durch:

> Wer pusht, sieht nach — **am Lauf des eigenen Commits**, nicht am obersten der
> Liste:
> `gh run list --commit $(git rev-parse HEAD) --json status,conclusion --jq '.[]|[.status,.conclusion]|@tsv'`
> Erst `completed` **und** `success` ist ein Nachschlag. `in_progress` heißt:
> noch einmal — nicht „nichts Rotes gesehen".

*Automatisch geladen?* **Ja**, `CLAUDE.md`.
*Geprüft:* Der Befehl ist am 04.08.2026 gegen `2c69944` und gegen `HEAD`
gelaufen; die volle Kennung ist nötig (`git rev-parse`), die Kurzform liefert
leer. Der `in_progress`-Fall ist **in dieser Retro eingetreten** (§23, Kopf) —
er ist kein gedachter.

---

**B19 — Der Review-Auftrag benennt den Diff, nicht die Stories.**

*Anlass:* K4. Der Auftrag nannte drei Stories, der Diff umfasste **73 Dateien,
6025 Zeilen**. Der Reviewer hat die codetragenden Teile mitgeprüft und den
Auftrag zugleich als Befund gemeldet — ohne diese Selbstauskunft wäre der
Beifang später entweder als ungeprüft gelesen worden oder stillschweigend als
geprüft.

*Geändert:* `docs/scrum/PROZESS.md`, **DoD 3**, nach dem ersten Satz eingefügt:

> **Der Review-Auftrag benennt den Diff, nicht die Stories**: Bereich
> (`sprint-NN-basis..main`), Zahl der Dateien und die Teile, die *nicht* aus den
> Stories stammen. Wer die Stories aufzählt, beschreibt seine Absicht; der Diff
> beschreibt, was zur Prüfung ansteht. Beide fallen auseinander, sobald an einem
> Tag auch außerhalb des Sprints gearbeitet wird — und das ist der Normalfall
> (Sprint 6, K4).

*Automatisch geladen?* **Ja für den Scrum Master**, der den Auftrag laut
Rollentabelle formuliert. **Verbleibendes Risiko, offen benannt:** In Sprint 6
hat den Auftrag der **PO** geschrieben (§17.1: „mein Review-Auftrag"). Für ihn
wirkt DoD 3 nur, weil er sie am Sprint-Ende als Liste abarbeitet. Ich schlage
**keine** zusätzliche `CLAUDE.md`-Zeile vor: Die Datei trägt die Regeln, die
*wiederholt* übergangen wurden, und K4 ist der erste Fall. Tritt er ein zweites
Mal ein, gehört er dorthin.

---

**B20 — Modellzuordnung revidiert (fällig aus B15), bestätigt.**

B15 hat als nächsten Revisionstermin „Retro nach Sprint 6" gesetzt. Hier ist er.

*Beleg aus Sprint 6:* **K1** — der Übergabebericht von Strang A behauptete,
„jede tragende Zusicherung" sei gegen eine Mutation des Produktivcodes gehalten
worden; gedeckt waren **8 von 11**. Geschrieben von einer Opus-Rolle, gefunden
vom `karpathy-reviewer` auf **Fable**, und zwar durch **Nachzählen**, nicht
durch Nachdenken — in dem Sprint, dessen Review-Auftrag ausdrücklich nach der
Vollständigkeit dieser Liste fragte.

*Ehrliche Einschränkung, damit der Beleg nicht stärker aussieht als er ist:* Der
Reviewer war der **erste fremde Leser** des Berichts. Anders als in Sprint 3
(wo eine tautologische Zusicherung mehrere Opus-Durchgänge überlebte) ist damit
nicht belegt, dass eine Opus-Rolle sie übersehen **hätte**. Der Sprint-3-Beleg
bleibt der tragende; dieser stützt ihn.

*Verwalter auf Haiku:* zwei Aufträge in Sprint 6 (Kegel-Rückbau, `sp:`-Label),
beide mit abgelegter Datei, beide vom PO nachgemessen. Die drei Bedingungen der
Delegation halten.

*Geändert:* `docs/scrum/PROZESS.md`, Modellzuordnung — der letzte Aufzählpunkt
(„Revision in der Sprint-3-Retro … Nächste Revision: Retro nach Sprint 6")
bekommt einen zweiten Absatz:

> **Revision in der Sprint-6-Retro (04.08.2026): bestätigt.** Der eine Befund
> des Sprints, den eine Opus-Rolle geschrieben und der Reviewer auf Fable durch
> Nachzählen gefunden hat, ist K1 (`sprint-06-karpathy.md`): „jede tragende
> Zusicherung gegen eine Mutation gehalten" deckte 8 von 11. Kein Anlass, das
> Sicherheitsnetz abzustufen. Der `denkzettel-verwalter` auf Haiku hat zwei
> Aufträge mit abgelegter Datei und nachgemessenem Ergebnis ausgeführt. **Nächste
> Revision: Retro nach Sprint 9.**

*Automatisch geladen?* **Ja für den Scrum Master und den PO beim Spawn** —
dieselbe Lage wie bei B15, dort mit „Ja" beantwortet und seither gehalten.

### 23.3 Gegenrechnung — was zu keinem Beschluss führt, und warum

Prinzip 2 gilt auch für Prozess. Dieses Projekt hat 15 Beschlüsse, elf
Abschlusspunkte, sechs DoD-Punkte und eine `CLAUDE.md` mit acht Merksätzen. Jede
Regel, die nicht trägt, verdünnt die, die tragen.

| Befund | Beschluss | Warum nicht |
|---|---|---|
| **M-B2** — Debug-Build startet nicht neben dem Dienst | **nein**, als Wissen in B16 aufgenommen | Derselbe Mechanismus wie §22.1; **zwei Regeln für einen Mechanismus sind eine zu viel** |
| **M-B2**, offene Frage: Prüfläufer als CMake-Ziel? | **nein — an den PO** | Eine Werkzeug-Entscheidung mit Bauzeit-Kosten gehört in eine Story, nicht in eine Retro. Bisher **ein** Anwendungsfall (`probe59.cpp`); ein zweiter macht sie zur Story |
| **M-B3** — clazy-Bestandswarnungen | **nein** | Der auffälligste Befund ist gegenstandslos, am Code nachgewiesen (§16.1). Eine Regel „Bestand abarbeiten" würde genau die Stelle anfassen, die B5 schützt |
| **UI-Bericht, Präsens ohne Anker** | **kein eigener** — dritte Änderung in B17 | Es ist die Schreibseite derselben Klasse. Ein eigener Beschluss daneben wäre die dritte Fassung desselben Satzes |
| **`sp:`-Label** (§1: „gehört in die Retro") | **nein — bereits erledigt** | Am 04.08.2026 als Kundenentscheidung in `CLAUDE.md` **und** `PROZESS.md` verankert, samt Altbestand und der Story-ID-Falle. Nachgeprüft, steht dort. Ein Retro-Beschluss wäre eine Wiederholung |
| **Linterlauf in den Dev-Auftrag** (aus `1006d33`: „die fünf Nachweise enthielten keinen Linterlauf") | **nein** | Genau das ist die Hälfte, die an Disziplin hängt — und die CI ist ihre Mechanisierung. Beide Male hat sie gegriffen. Die Regel nachzuziehen hieße, dieselbe Prüfung zweimal zu bezahlen |

### 23.4 Nachprüfung: haben die früheren Beschlüsse in Sprint 6 gewirkt?

Am Material geprüft, nicht an der Erinnerung.

| Beschluss | Wirkung in Sprint 6 | Beleg |
|---|---|---|
| **B3** — UI-Review ohne Bild nicht geführt | **getragen, und mehr als das:** Der Satz „Bei Bewegungen ist der Weg der Prüfgegenstand" war die **Entscheidungsregel**, mit der der PO #59 als Nicht-UI-Story einstufte | §15.2; UI-Review mit 19 eigenen Bildern und eigenen Sonden |
| **B5** — Registrierungen zurücklesen | **getragen** | §22.2: nicht die Zusicherung der Anwendung, sondern die Antwort des Dienstes. Zugleich das Argument, mit dem M-B3 abgewehrt wurde |
| **B7** — unversionierter Beleg ist kein Beleg | **getragen** | `probe59.cpp` im Belegordner, UX-Sonden versioniert, §19.3 vollzählig — kein Commit nennt einen Prüflauf ohne abgelegten Bericht |
| **B9** — DoD 4 erfasst entdeckte Bedingungen | **getragen, stärkster Fall bisher** | SPEC 3.2 mit **fünf** entdeckten Bedingungen, SPEC 9 mit einer, SPEC 15 der wörtliche B9-Beispielfall (§19.5, am SPEC-Diff gelesen) |
| **B10** — Doku-Abgleich | **getragen, Umfang zu eng** | Fand M2 **und** M4 (§19.7) — M4 aber in `CLAUDE.md`, das die Regel gar nicht nennt. Deshalb B17, Änderung 2 |
| **B11** — zwei Takte | **getragen** | §19.10 hat DoD 5/6 **ausdrücklich nicht** gebucht. Genau die Sprint-3-Mängel M2/M5, die nicht wiederkehrten |
| **B12** — Sprint-Konto, beide Grenzen | **getragen** | §1 bucht den Zugang #59 bei der Freigabe mit beiden Zahlen (3 von 2–4 · 11 von ~13); §19.6 misst am Milestone nach: kein Zugang danach |
| **B13** — Parallelarbeit in der Prozessdatei | **getragen** | Zwei Stränge, Dateimengen aus gemessenem Abstand abgeleitet (§15.1), Installation den Strängen untersagt (§15.3), Merges nur durch den PO. **Grenze:** Die Installations-Taktung schützt gegen zwei Stränge, nicht gegen den laufenden Dienst → B16 |
| **B14** — flüchtige Belege sofort sichern | **nicht prüfbar** | In Takt 1 sind keine Kundenbilder angefallen. Prüfbar erst in der Abnahme |
| **B15** — Modellzuordnung | **fällig und erledigt** | → B20 |

**Die vier Punkte der Prozessökonomie (§13), zu deren Prüfung die Retro
beauftragt war:**

- **P1 — automatische Testläufe: Wache, nicht Zierde.** Zwei rote Läufe in
  Sprint 6, beide echte Rückschritte, beide sonst unsichtbar: `d30f5d0` (vierter
  clazy-Befund aus `testsilence.cpp`, geheilt `1006d33`) und `2ef495f`
  (16 clazy-Befunde bei Schwelle 3, geheilt `bb1dcd2`). Bau grün,
  `ctest` grün, Compiler-Warnungen null — beide Male war der Lauf der Einzige,
  der es sah. **Kein Rückbau.** *Genau benannt:* Sein bisheriger Wert liegt
  vollständig im **Linter**; das ist die Prüfung, die in den Aufträgen fehlte.
- **P2 — Pflicht-/Kürteil: noch nicht beurteilbar.** `sprint-06.md` hat vor
  dieser Retro **1825 Zeilen** gegen ein Mittel von **1614** aus den Sprints 3–5
  (2137 · 1043 · 1663; die im Nachtrag genannten „rund 1.300" sind zu niedrig).
  Die Regel entstand jedoch **mitten im Sprint** (§13.2), die Zeilen 1–966
  stammen von davor. **Messpunkt: Sprint 7**, der erste vollständig unter der
  Regel geführte. **B7 ist nicht verletzt** — in der Differenz steckt kein
  Beleg. Die vier Pflichtteile sind vorhanden bis auf das done/next des
  Sprint-Endes, das in Takt 2 entsteht (§19.9).
- **P3 — Verwalter-Bericht als Existenzprüfung: offen.** Sprint 6 hatte zwei
  Verwalter-Aufträge, beide mit abgelegter Datei
  (`sprint-06-verwalter-kegel.md`, `-splabel.md`). Der beauftragte **dritte
  Lauf** ist der Takt-2-Auftrag dieses Sprints und steht noch aus.
  *Klarstellung, die ich vorschlage, aber nicht als Beschluss führe:* Punkt 11
  nennt **einen** festen Dateinamen je Sprint; die Praxis führt **eine Datei je
  Auftrag**. Solange nur Takt 2 gemeint ist, kollidiert nichts — bei zwei
  Takt-2-Aufträgen überschriebe der zweite den ersten. Ein Wort im Auftrag
  genügt.
- **P4 — Schätzkegel: zurückgebaut, nachgemessen.** `docs/scrum/diagramme/` ist
  fort, die Abschlussliste endet bei 11, der datierte Vermerk in der
  Sprint-Mechanik steht. Kein Rückstand.

### 23.5 Abschlussprüfung — die zwei Fragen, je Beschluss

| Beschluss | (1) In welchem Artefakt? | (2) Wird es automatisch geladen? |
|---|---|---|
| **B16** | `CLAUDE.md` (installierter Stand) · `PROZESS.md` Takt 1 Punkt 1 | **Ja** · **Ja im Wirkmoment** — die Abschlussliste wird Punkt für Punkt abgearbeitet |
| **B17** | `CLAUDE.md` Prüfhaltung · `PROZESS.md` Doku-Abgleich · `PROZESS.md` Belege | **Ja** · **Ja** (Adressat ist bei beiden `PROZESS.md`-Stellen der Scrum Master, der die Datei als Grundlage liest) |
| **B18** | `CLAUDE.md` Technisches | **Ja** |
| **B19** | `PROZESS.md` DoD 3 | **Ja für den Scrum Master**; für den PO nur als abgearbeitete Liste am Sprint-Ende. **Verbleibendes Risiko benannt**, bewusst keine `CLAUDE.md`-Zeile beim ersten Vorkommen |
| **B20** | `PROZESS.md` Modellzuordnung | **Ja** |

**Bilanz, ohne Beschönigung:** Vier der fünf Beschlüsse landen ganz oder
teilweise in `CLAUDE.md` — der Datei, die jede Sitzung ohne Zutun liest. Das ist
die Antwort auf die Lücke, die die Sprint-2-Retro zehnmal und die Sprint-3-Retro
dreimal hatte. **B19 ist die eine Ausnahme, und sie ist eine Entscheidung, keine
Auslassung:** `CLAUDE.md` trägt die Regeln, die *wiederholt* gerissen sind. Wer
dort jeden Erstfall einträgt, macht aus der Datei, die jeder liest, eine, die
keiner mehr liest.

### 23.6 done / next (Retro)

**done:** Sieben Befunde ausgewertet · fünf Beschlüsse mit Wortlaut und
Zielartefakt · sechs Befunde ausdrücklich **ohne** Beschluss, je mit Grund · zehn
frühere Beschlüsse und vier Prozessökonomie-Punkte am Material nachgeprüft · die
fällige Modellrevision aus B15 erledigt.

**next:** (1) PO baut B16–B20 ein — vier Stellen in `CLAUDE.md`, fünf in
`PROZESS.md`; danach karpathy-Review über den Prozess-Diff (globale Regel für
Regel-/Agentenänderungen). (2) Standdatum in `PROZESS.md` mitführen. (3) Sprint 7:
Zeilenzahl des Protokolls messen (P2) und den Takt-2-Verwalter-Bericht auf
Existenz prüfen (P3, dritter Lauf). (4) Offen an den PO, nicht beschlossen:
Prüfläufer als CMake-Ziel (M-B2).

---

### 23.7 Nachtrag: der Prüfweg der Bildbelege (B21)

*Nachgereicht am 04.08.2026 auf Vorlage des PO. Der Befund stammt aus der
Kundenabnahme und lag bei der Retro noch nicht vor; er steht als offene
DoD-Frage in §25.3. **Zur Nummer:** 23.5 und 23.6 waren vergeben, deshalb 23.7
statt der vorgeschlagenen 23.5.*

#### 23.7.1 Zuerst ausgezählt

Der PO hat verlangt, den Umfang zu **zählen** statt zu vermuten. Gemessen am
Stand `main` @ `1de4c73`; alle fünf Läufer vorher frisch gebaut.

**Achse A — der fehlende Alphakanal.** `tinted()` hat genau **zwei**
Aufrufstellen, beide in `src/capture/capturewindow.cpp:326–327`. Der Weg ist nur
erreichbar, wo ein `CaptureWindow` gezeichnet wird — also in den Läufern, die
`denkzettelcapture` linken.

| Läufer | linkt | Hülle im Bild | Achse A |
|---|---|---|---|
| `captureshots` | `denkzettelcapture`, `KF6::Svg` | ja | **betroffen** |
| `readmeshots` | `denkzettelcapture`, `denkzettelui` | ja | **betroffen** |
| `libraryshots` | `denkzettelui` | nein | nicht betroffen |
| `editshots` | `denkzettelui` | nein | nicht betroffen |
| `searchshots` | `denkzettelui` | nein | nicht betroffen |

**Zwei von fünf** — und die zweite ist nicht erschlossen, sondern gemessen. Ein
frischer `readmeshots`-Lauf zeigt am linken oberen Bogen dasselbe Loch wie
`captureshots`:

```
cmake --build build --target readmeshots
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=2 \
    LANG=de_DE.UTF-8 build/bin/readmeshots <Zielordner>
```

Ecke oben links, 20 × 20 Bildpunkte. `K` = Konturfarbe `4c4e51` ± 12,
`F` = Flächenfarbe `202326` ± 12, `.` = Alpha < 8:

```
............KKKKKKKK      Die Kontur läuft über die Gerade oben (K)
............KKKKKKKK      und über die Gerade links (K, ab Zeile 12).
..............FFFFFF      Auf dem Bogen dazwischen: kein einziges K —
..............FFFFFF      die Fläche (F) stößt unmittelbar an das
......FFFFFFFFFFFFFF      Durchsichtige (.), die Kontur fehlt.
......FFFFFFFFFFFFFF
....FFFFFFFFFFFFFFFF
....FFFFFFFFFFFFFFFF
....FFFFFFFFFFFFFFFF
....FFFFFFFFFFFFFFFF
....FFFFFFFFFFFFFFFF
....FFFFFFFFFFFFFFFF
KK..FFFFFFFFFFFFFFFF
KK..FFFFFFFFFFFFFFFF
KKFFFFFFFFFFFFFFFFFF
KKFFFFFFFFFFFFFFFFFF
KKFFFFFFFFFFFFFFFFFF
KKFFFFFFFFFFFFFFFFFF
KKFFFFFFFFFFFFFFFFFF
KKFFFFFFFFFFFFFFFFFF
```

Die Ursache ist dieselbe wie bei `captureshots` und liegt in
`b1-huellenring-offscreen.txt` vermessen vor: `tinted()` liefert offscreen
Format 4 **ohne** Alphakanal, unter Wayland Format 6 **mit**.

**Achse B — die Skalierung.** Kein Läufer fährt bei 1,6. Vier fahren bei 1,
`readmeshots` bei 2 (`QT_SCALE_FACTOR=2`, für die Lesbarkeit im README). Ob
darin etwas verborgen liegt, ist nachgemessen: `libraryshots`, `editshots` und
`searchshots` je bei 1 und bei 1,6 gefahren, dann die sechs stärksten senkrechten
Kanten des Helligkeitsprofils verglichen (Kante bei 1,6 geteilt durch 1,6):

| Läufer / Bild | Kanten bei 1 | Kanten bei 1,6 ÷ 1,6 |
|---|---|---|
| `libraryshots` 01 | 278 · 279 · 285 · 293 · 299 · 300 | 278,1 · 279,4 · 285,6 · 293,1 · 299,4 · 300,6 |
| `editshots` 02 | 12 · 299 · 300 · 312 · 686 · 887 | 12,5 · 299,4 · 300,6 · 312,5 · 686,2 · 887,5 |
| `searchshots` 2 | 12 · 34 · 35 · 49 · 299 · 300 | 12,5 · 13,8 · 34,4 · 49,4 · 299,4 · 300,6 |

Alle drei Fenster wachsen exakt (900 × 600 → 1440 × 960), und jede Kante fällt
auf ihre Stelle bei 1 zurück, Abweichung ≤ 1,3 logische Pixel. **Das Bild bei
1,6 ist bei diesen dreien das Bild bei 1, vergrößert.** Beim Capture-Fenster
nicht: dort ist die Ecke bei 1,6 eine **andere** Zeichnung, weil `alphaMask()`
bei DPR 1 bleibt (600 × 150) und hochskaliert wird
(`b1-eckenraster-skala-*.txt`) — im Bild `04-rand-schmal-dunkel-leer.png`
laufen die Zwischenwerte bei 1,6 über verdoppelte Zeilen
(`767777 767777`, `cccbc8 cccbc8`), bei 1 über einzelne.

**Ergebnis der Zählung.** Der Befund ist **nicht allgemein**. Er sitzt dort, wo
das Bild von etwas abhängt, das **nicht unser Code zeichnet** — der vom
Desktop-Theme gerenderten Hülle. Wo unser Code in unsere Palette zeichnet, trägt
offscreen bei Verhältnis 1.

#### 23.7.2 Wo der blinde Fleck geschrieben steht

Drei Messungen treffen die Ursache genauer als „der Läufer fährt offscreen".

1. **Das Akzeptanzkriterium hat die Belegform selbst zugeordnet — eine
   Eigenschaft zu weit.** AK 7 von #55, wörtlich aus
   `docs/scrum/reviews/sprint-06-s55-huelle/bericht.md`, Zeile 70: „Rundung und
   Kontur offscreen im Bild …; Schatten als Bild am laufenden Plasma **oder**
   benannte Zusicherung." Die Teilung zwischen offscreen und laufender Sitzung
   war also **da** — sie verlief nur an der falschen Stelle. Der Läufer hat
   getan, was das AK verlangte.
2. **Der Entwickler hat die laufende Sitzung bereits benutzt — einmal.**
   `15-schatten-am-plasma.png` misst 992 × 310; der Inhalt darin ist 960 × 278,
   und das ist 600 × 174 bei Verhältnis 1,6. **Eines von 34 Belegbildern des
   Sprints** (15 aus der Dev-Übergabe, 19 aus dem UX-Review) ist unter den
   Bedingungen des Kunden entstanden — genau das zu der einen Eigenschaft, die
   das AK der laufenden Sitzung zugeordnet hatte. Das Mittel existiert, ist
   bezahlbar und war in diesem Sprint bereits im Einsatz.
3. **Die Anweisung, offscreen zu fahren, steht in beiden Agentendateien:**
   `.claude/agents/denkzettel-dev.md:58` („Für Fenster genügt
   `QT_QPA_PLATFORM=offscreen` plus `QWidget::grab().save()`") und
   `.claude/agents/denkzettel-ux.md:47` (dasselbe, „verbindlich nach DoD 3").
   **Beide ohne ein Wort zur Skalierung.**

Punkt 3 korrigiert die Vorlage des PO in einem Punkt. Die beiden Bildreihen sind
**als Dateien** verschieden — stichprobenweise geprüft, drei Paare, keines
bytegleich —, aber die **Vorschrift** war dieselbe, und sie stand in der Datei,
die jeder der beiden Agenten garantiert liest. Zwei Agenten mit derselben
schriftlichen Anweisung sind nicht zwei Beobachter; sie sind ein Beobachter,
zweimal ausgeführt. Damit ist die zweite Retro-Frage hier vorweg beantwortet:
Der Ort, der garantiert gelesen wird, ist in diesem Fall **nicht** `CLAUDE.md`,
sondern die Agentendatei — dort steht der Satz, der den Befund erzeugt hat.

#### 23.7.3 Der Beschluss

---

**B21 — Ein offscreen erzeugtes Bild belegt nicht, was Theme und Compositor
zeichnen; und jeder Bildbeleg entsteht bei der Skalierung des Kunden.**

*Anlass:* §25.3 und die Zählung in 23.7.1. Zwölf Belegbilder zu AK 1 und AK 7
von #55 zeigen eine Ecke ohne Kontur; der Kunde sieht sie. Zwei von vier
Abnahmebefunden bestehen bei Verhältnis 1 gar nicht.

*Warum überhaupt ein Beschluss, wo Prinzip 2 gegen den einundzwanzigsten
spricht:* Weil die Voraussetzung von B3 gerissen ist. B3 sagt „bei Zuständen ist
das Bild der Prüfgegenstand, nicht die Zusicherung" — das setzt voraus, dass das
Bild den Zustand des Kunden zeigt. Genau das hat nicht gehalten, und **beide**
Bildreihen des Sprints haben es nicht bemerkt. Ein Vorbehalt allein reicht dabei
nicht: AK 1 von #55 behauptet etwas über die Kontur auf dem Bogen, also über
genau das, was offscreen nicht entsteht. Ohne Auslöser hätte unter denselben
zwölf Bildern künftig „offscreen, Vorbehalt bekannt" gestanden, und der Sprint
wäre gleich verlaufen.

*Warum trotzdem keine allgemeine Pflicht zum Sitzungsbild:* Die Zählung zeigt,
dass drei der fünf Läufer davon **nichts** gewinnen — bei ihnen ist das Bild bei
1,6 das Bild bei 1, vergrößert, und ihre Pixel zeichnet unser Code. Eine Pflicht,
die in drei von fünf Fällen nachweislich nichts findet, wird umgangen. Der
Beschluss zerlegt den Befund deshalb in die Hälfte, die **umsonst** zu schließen
ist (Skalierung: eine Umgebungsvariable), und die Hälfte, die **teuer** ist
(Plattform: eine angemeldete Sitzung, nicht in der CI wiederholbar) — und
verpflichtet nur die erste allgemein.

*Geändert 1:* `docs/scrum/PROZESS.md`, DoD 3, hinter „Ein UI-Review ohne eigene
Bildprüfung zählt für diesen Punkt nicht." — angehängt:

> **Was ein offscreen erzeugtes Bild belegt, und was nicht.** Es belegt
> Geometrie, Textsatz und Farbrollen — alles, was unser Code in unsere Palette
> zeichnet. Es belegt **nicht**, was Desktop-Theme oder Compositor beisteuern:
> Hülle, Rundung, Kontur, Schatten, Dekoration, Durchsichtigkeit. *Gemessen:*
> `tinted()` füllt eine QPixmap deckend, woraufhin Qt offscreen ein Format ohne
> Alphakanal wählt und die Kontur auf dem Eckbogen verschwindet; derselbe
> Binärcode unter Wayland liefert ein Format mit Alphakanal und den
> vollständigen Bogen
> (`docs/scrum/reviews/2026-08-04-abnahme-befunde/messungen/b1-huellenring-offscreen.txt`
> gegen `-live.txt`).
> **Macht ein Akzeptanzkriterium eine Aussage über eine dieser Größen, gehört
> ein Bild aus der angemeldeten Sitzung zum Beleg**; ohne es ist DoD 3 für diese
> Story nicht geführt. Die Bildläufer bleiben offscreen — das Sitzungsbild tritt
> daneben, nicht an ihre Stelle. Der Prüfweg dafür liegt fertig vor
> (`docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/echtelage.cpp`); er
> nimmt allein das Fenster auf, nicht den Bildschirm.
> **Und jedes Bild, das als Beleg dient, entsteht bei der Skalierung, die der
> Kunde fährt** (`QT_SCALE_FACTOR`). Die Zahl steht im Prüfbericht, nicht in
> dieser Regel — am 04.08.2026 gemessen 1,6 —, sonst altert sie hier still.
> README-Bilder sind hiervon ausgenommen: Sie belegen nichts, sie sollen lesbar
> sein.
> **Wer die Belegform in ein Akzeptanzkriterium schreibt, entscheidet damit
> darüber.** AK 7 von #55 hat „Rundung und Kontur" der offscreen-Seite
> zugeordnet und nur den Schatten der laufenden Sitzung — die Teilung war da,
> eine Eigenschaft zu weit. Der Läufer hat getan, was dort stand.

*Geändert 2:* `CLAUDE.md`, Block „Ein UI-Review ohne eigenes Bild ist nicht
geführt", hinter dem Satz zu `QT_QPA_PLATFORMTHEME=kde` — angehängt:

> **Ein offscreen erzeugtes Bild zeigt nicht, was der Kunde sieht.** Es belegt
> Geometrie, Textsatz und Farbrollen; es belegt **nicht** Hülle, Rundung,
> Kontur, Schatten oder Dekoration — die zeichnen Theme und Compositor, und
> offscreen fehlt beiden die Grundlage (gemessen: `tinted()` verliert offscreen
> den Alphakanal, unter Wayland nicht). Zweierlei folgt daraus: Ein Bild, das
> als Beleg dient, läuft mit `QT_SCALE_FACTOR` auf der Skalierung des Kunden —
> und wo ein Akzeptanzkriterium über Theme oder Compositor etwas behauptet,
> gehört ein Bild aus der angemeldeten Sitzung dazu.

*Geändert 3:* `.claude/agents/denkzettel-dev.md`, Abschnitt „Vor der Übergabe —
Selbst-Sichtprüfung", Satz „Für Fenster genügt `QT_QPA_PLATFORM=offscreen` plus
`QWidget::grab().save()`" — ersetzt durch:

> Für Fenster `QT_QPA_PLATFORM=offscreen` plus `QWidget::grab().save()`,
> **dazu `QT_SCALE_FACTOR` auf der Skalierung des Kunden** — ein Bild bei
> Verhältnis 1 belegt seinen Zustand nicht (DoD 3). **Behauptet ein
> Akzeptanzkriterium etwas über Hülle, Rundung, Kontur, Schatten, Dekoration
> oder Durchsichtigkeit, kommt ein Bild aus der angemeldeten Sitzung dazu** —
> offscreen zeichnet weder Theme noch Compositor vollständig.

*Geändert 4:* `.claude/agents/denkzettel-ux.md`, Punkt 3 „UI-Review", derselbe
Zusatz hinter `QT_QPA_PLATFORM=offscreen`, `QWidget::grab().save()`:

> …, **`QT_SCALE_FACTOR` auf der Skalierung des Kunden**, und bei Aussagen über
> Hülle, Rundung, Kontur, Schatten oder Dekoration zusätzlich ein Bild aus der
> angemeldeten Sitzung (DoD 3).

*Automatisch geladen?* `CLAUDE.md`: **ja**, von jeder Sitzung im Projekt.
`PROZESS.md` DoD 3: **ja für den Scrum Master** (Prüfer) und für den
Entwickler, dessen Agentendatei `PROZESS.md` **ganz** verlangt. Die
Agentendateien: **ja, und hier ist das der Punkt** — die Anweisung, die den
Befund erzeugt hat, steht genau dort; eine Regel anderswo hätte gegen sie
verloren. Drei der vier Änderungen sitzen damit an Orten, die ohne Zutun gelesen
werden.

*Geprüft am Wortlaut:* Die Zusätze zu Geändert 3 und 4 sind gegen die heutigen
Läufer gehalten — `QT_SCALE_FACTOR=1.6` ist an `libraryshots`, `editshots`,
`searchshots` und `captureshots` gelaufen und liefert Bilder; die Regel
verlangt nichts, was nicht heute schon geht.

---

#### 23.7.4 Was ausdrücklich **nicht** beschlossen wird

- **Keine allgemeine Pflicht zum Sitzungsbild.** Sie ist teuer, in der CI nicht
  wiederholbar, und die Zählung zeigt drei von fünf Läufern, bei denen sie
  nachweislich nichts fände. Der Vorbehalt des PO trägt — er bekommt nur einen
  Auslöser statt keinen.
- **Keine Prüfung der Bildläufer gegen ein Sitzungsbild** (etwa als
  automatischer Vergleich). Das wäre ein zweiter Prüfweg für dasselbe und
  scheitert an derselben Nichtwiederholbarkeit.
- **Keine Änderung an `readmeshots`' Skalierung.** Seine 2 dient der Lesbarkeit
  im README, nicht der Beweisführung; er ist kein DoD-3-Beleg. Der Befund an
  seinen Bildern ist ein Doku-Mangel, kein Prozessmangel — siehe 23.7.5.
- **Keine Regel zur Testseite.** `capturetest` misst die Hülle über
  `cornerRun()` und die Alphawerte der Kantenmitten — beides Größen, die
  offscreen korrekt entstehen. Es behauptet nichts über die Konturfarbe auf dem
  Bogen und ist daher nicht betroffen. (Dass niemand sie zusichert, ist eine
  Frage an die Story, nicht an den Prozess.)

#### 23.7.5 Zwei Meldungen an den PO (melden, nicht heilen)

**N1 — Die beiden README-Bilder zeigen einen Stand vor #55.**
`docs/bilder/erfassungsfenster.png` stammt aus `cead9f9`, und `cead9f9` liegt
**vor** `38db754` (#55, Hülle). Gemessen: Die Datei im Repo ist 1200 × 324, ihre
linke obere Ecke ist auf 14 × 14 Bildpunkten durchgehend `202326` bei Alpha 255
— rechteckig, deckend, ohne Hülle. Ein Lauf des heutigen `readmeshots` liefert
1200 × 348 mit gerundeter, durchsichtiger Ecke. Das öffentliche README zeigt
damit ein Fenster, das `main` nicht mehr zeichnet. Ob es neu erzeugt wird, hängt
an der Entscheidung über #55 und ist Sache des PO, nicht meine.

**N2 — Der Prüfweg aus 23.7.1 ist nicht als Skript versioniert.**
Die Zählung in 23.7.1 ist wiederholbar (alle Befehle stehen oben), aber die
Auswertung der Eckkarte und der Kantenprofile lief über Wegwerf-Skripte im
Sitzungsordner. Nach B7 ist das kein Beleg für die Zukunft, wohl aber für
heute — die Zahlen stehen hier im versionierten Protokoll. Falls der Auslöser
aus B21 öfter greift, wäre eine kleine Sonde neben
`docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/` der Ort. **Kein Mangel,
ein Hinweis** — und ausdrücklich kein Auftrag an mich.

---

## 24. Kundenabnahme Sprint 6 — #55 nicht angenommen

**Geführt am 04.08.2026 am installierten Stand** (`/usr/bin/denkzetteld`,
Stand `977e804`, Dienst 16:19 gestartet). Bilder versioniert unter
`docs/scrum/reviews/2026-08-04-abnahme-befunde/kundenbilder/`.

### 24.1 Ergebnis je Story

| Story | Ergebnis |
|---|---|
| **#59** — ruhige Liste | **offen.** Der Kunde hat `Tab` geprüft (Fokuswechsel innerhalb des Fensters, Bildlaufleiste stand still) statt `Alt+Tab` (Fensterwechsel). Der geprüfte Fall ist damit nicht der Fall der Story; Nachprüfung erbeten |
| **#56** — Feldhöhe folgt der Schrift | **offen.** Am Kundenblick nicht prüfbar (§18), keine Rückmeldung nötig; wird mit #55 zusammen abgenommen |
| **#55** — Fensterhülle | **nicht angenommen** |

### 24.2 Die vier Befunde des Kunden

**K-A1 — Die Ecke ist eine Treppe, kein Bogen.** *„Sie ist nicht wirklich rund.
Der weiße Hintergrund ragt über den Rahmen hinaus. […] Bei Denkzettel sieht
man, dass das Fenster nachgebaut ist und kein natives Anwendungsfenster."* Am
Vergleichsbild bestätigt: drei bis vier sichtbare Stufen gegen einen glatten
kantengeglätteten Bogen.

**K-A2 — Der Schatten passt nicht.** *„Ja, ein Schatten ist da, aber auch hier
habe ich die Vermutung, dass dieser nicht aus dem KDE-Theme kommt."* Nachgereicht:
*„Der Schatten passt auch nicht zu dem Schatten, den andere Fenster werfen."*
Damit Sichtbefund am Vergleich, nicht Vermutung.

**K-A3 — Die Schriftfarben des Themes werden nicht übernommen.** Zwei Bilder,
eines unter seinem Theme, eines unter einem Win11-Dark-Theme (*„scheint es zu
passen"*).

**K-A4 — Die Grundsatzfrage**, und sie trägt die anderen drei:

> *„Die Frage ist doch, wie kann es sein, dass das völlig anders aussieht als
> alle anderen Fenster. Das sollte KDE nativ sein."*

### 24.3 Die Antwort darauf — und warum sie eine Entscheidung nach sich zog

**Native Fenster zeichnen ihre Hülle nicht selbst.** Rundung, Kontur und
Schatten kommen von KWins Fensterdekoration
(`org.kde.kdecoration3/org.kde.breeze.so`); die Anwendung füllt nur ihre
Innenfläche. Deshalb sehen alle gleich aus — es zeichnet sie alle derselbe.

**Denkzettel ist ausdrücklich rahmenlos** (`capturewindow.cpp:139`,
`Qt::Window | Qt::FramelessWindowHint`, SPEC 3 seit Sprint 1). Die Folge davon
hat bis heute niemand ausgesprochen: **Rahmenlos heißt, dass KWin nichts
zeichnet.** Was zu sehen ist, hat sich das Fenster selbst gemalt — und zwar aus
`dialogs/background` des Desktop-Themes, der Grafikfamilie der Plasma-
**Überlagerungen** (KRunner, Aufklapper, Sperr-OSD), nicht der der
Anwendungsfenster.

**Daraus folgt: Das Fenster sähe auch bei fehlerfreier Umsetzung anders aus als
Dolphin — es sähe aus wie KRunner.** Der Befund des Kunden zerfällt damit in
einen Fehler (die Treppe) und eine **Kategorienfrage, die ihm nie vorgelegt
worden war**: Zu welcher Familie soll das Erfassungsfenster gehören?

### 24.4 Kundenentscheidung 04.08.2026

> **„Dann eine native Plasma-Überlagerung ohne Anpassungen"**

**Tragend ist „ohne Anpassungen": kein Nachbau.** Die Hülle wird von
`KSvg::FrameSvg` **in einem Stück** gezeichnet, wie Plasmas eigene
Überlagerungen es tun — kein selbst zusammengesetzter Ring aus zwei Rahmen,
keine selbst gezogene Kontur, keine eigene Rundung. Was die Grafik des Themes
hergibt, ist das Bild.

**Kein Widerspruch zur Farbzusage, sondern ihr Mechanismus:** `KSvg::Svg` trägt
eine `colorSet`-Eigenschaft (`/usr/include/KF6/KSvg/ksvg/svg.h:86`); die Grafik
wird vom Farbschema eingefärbt. Das ist der native Weg, auf dem Plasma seine
Überlagerungen an das Schema anpasst — **und ein Kandidat für die Ursache von
K-A3**: Wer die Fläche selbst mit der Palette malt, umgeht genau diesen
Mechanismus.

**Was dadurch wackelt und dem Kunden vorgelegt wird, sobald die Messung liegt:**
AK 2 von #55 (Fläche `Window`, Notiztext `WindowText`, begründet mit einer
Kontrastmessung 4,74:1 über 18 Schemata) und die Kontur-Zusage
(`frameContrast` 0,20 — unsere eigene Konstruktion, die das Theme unter dem
nativen Weg selbst zeichnet).

**Untersuchung läuft** (`docs/scrum/reviews/2026-08-04-abnahme-befunde/`). Der
Umbau wird als Story geschnitten, wenn die Messung liegt — nicht vorher.

---

## 25. Sprint-Bilanz

**Der Sprint endet mit einer abgelehnten Story.** Das ist ein Ergebnis, kein
Unfall, und es gehört so gebucht.

| Story | SP | Ausgang |
|---|---|---|
| **#59** — ruhige Liste bei Fensteraktivierung | 2 | **abgenommen und geschlossen.** Kundenprüfung am installierten Stand: *„Die Auswahl ist noch oben."* Kein Rücksprung; vorher 459 px bei 552 px Sichthöhe |
| **#56** — Feldhöhe folgt der Schrift | 1 | **abgenommen und geschlossen.** Am Kundenblick nicht prüfbar (benannte Grenze), Nachweis Test und Bild |
| **#55** — Fensterhülle | 8 | **nicht angenommen.** Bleibt offen, geht in den Backlog, wird mit #83 zusammen abgenommen |

**Geliefert: 2 von 3 Stories, 3 von 11 Punkten.**

**Der Milestone ist geschlossen — mit #55 darin.** Das ist Absicht. Die Story
wurde in diesen Sprint gezogen und ist in ihm gescheitert; sie aus dem
Milestone zu nehmen, hätte die Bilanz auf „2 von 2" geschönt. Der Milestone
liest sich dauerhaft als **1 offen, 2 geschlossen**, und das ist die Wahrheit
über diesen Sprint. Weitergebaut wird an #55 zusammen mit #83, in einem
Sprint, den der Kunde noch freigeben muss.

### 25.1 Warum die Ablehnung der wertvollste Teil dieses Sprints ist

#55 war die größte Story, die dieses Projekt gezogen hat. Sie ist durch beide
Reviews gegangen — karpathy mit Verdict `ok`, UI-Review mit 22 von 26 `pass`
und keinem `fail` —, durch eine DoD-Prüfung mit fünf Mängeln, durch eine
CI mit null Warnungen. **Und sie war trotzdem falsch.**

Nicht handwerklich falsch. Die Farbzusagen halten, unter dem Schema des Kunden
nachgemessen; die Maße treffen die Zeichnung; die Rundung kommt nachweislich
aus dem Theme und nicht aus dem Code. **Falsch war die Kategorie**, und die hat
niemand je entschieden: Das Fenster ist rahmenlos (SPEC 3, Sprint 1), also
dekoriert KWin es nicht, also malt es sich die Hülle selbst — und sähe damit
auch fehlerfrei aus wie eine Plasma-Überlagerung, nicht wie ein
Anwendungsfenster.

**Das hat kein Prüfmittel dieses Projekts gefunden, und keines hätte es
gefunden.** Reviews prüfen gegen Akzeptanzkriterien und Zeichnungen; die
Zeichnung zeigte eine Hülle, die AK verlangten sie aus dem Theme, beides war
erfüllt. Gefehlt hat die Frage, zu welcher Familie das Fenster gehört — und die
stellt nur, wer das Fenster **neben andere Fenster stellt und hinsieht**.

Das ist die Rechtfertigung der Kundenabnahme in einem Satz, und sie ist an
diesem Sprint teuer bezahlt worden.

### 25.2 Zwei Befunde, die der PO zurückgenommen hat

**#80 war kein Produktfehler.** Ich habe dem Kunden auf seiner
Abnahme-Checkliste geschrieben, die Konturlinie ende vor der Rundung. Sie tut
es nicht — er sieht sie. Was sie nicht zeigt, sind **unsere eigenen
Bildbelege**: Offscreen wählt Qt ein Format ohne Alphakanal und löscht die
Linie. Der Befund stammte aus einem Bild, das schlechter ist als das Programm.

**#82 hatte die falsche Ursache.** Der Verdacht lautete, `OutlineWidth` werde
als logisches Pixel geführt und zweimal als Pixelmaß benutzt. Tatsächlich war
es die **Auflösung der Maske**. Beide Issues sind mit Messung geschlossen.

**Der gemeinsame Nenner mit M1 und M4 (§21.1):** Auch hier fiel eine Aussage
von ihrem Gegenstand ab — nur diesmal in die andere Richtung. Nicht der Zustand
änderte sich unter der Aussage, sondern die Aussage stammte aus einem
**anderen Zustand** als dem, über den sie sprach: aus dem Bild statt aus dem
Programm.

### 25.3 Die Bedingung, die alles ordnete und nirgends stand

**Der Kunde fährt Skalierung 1,6.** 600 × 174 logische Pixel sind bei ihm
960 × 278 tatsächliche. **Zwei der vier Abnahmebefunde bestehen bei Verhältnis
1 gar nicht** — deshalb hat sie in drei Sprints niemand gesehen.

Damit sagt die Bildreihe zu AK 1 und AK 7 von #55 nichts über das, was der
Kunde vor sich hat: Sie entsteht bei Verhältnis 1 und offscreen, und beides
weicht vom Auslieferungszustand ab. **Das ist eine DoD-Frage für den Scrum
Master, keine Story** — und der schwerere der beiden Prüfweg-Befunde dieses
Sprints.

---

## 26. Vollzug B21 und die Antwort auf N1 (PO)

**B21 ist in allen vier Artefakten eingebaut** — `CLAUDE.md`,
`docs/scrum/PROZESS.md` (DoD 3), `.claude/agents/denkzettel-dev.md` und
`.claude/agents/denkzettel-ux.md`.

Die beiden Agentendateien sind der wirksame Ort, und das ist der Fund, der den
Beschluss trägt: Der Satz „Für Fenster genügt `QT_QPA_PLATFORM=offscreen`"
stand in **beiden**, ohne ein Wort zur Skalierung. **Zwei Agenten mit derselben
schriftlichen Anweisung sind nicht zwei Beobachter, sondern ein Beobachter,
zweimal ausgeführt.** Meine Vorlage hatte von „zwei unabhängigen Bildreihen"
gesprochen — das war als Dateiaussage richtig und als Prüfaussage falsch.

**Wo der Scrum Master mir widersprochen hat, hatte er recht.** Ich hatte einen
bloßen *Vorbehalt* angeboten statt einer Pflicht. Sein Gegenargument: AK 1 von
#55 behauptet etwas über die Kontur auf dem Bogen — also über genau das, was
offscreen nicht entsteht. Unter denselben zwölf Bildern hätte künftig
„offscreen, Vorbehalt bekannt" gestanden, und der Sprint wäre gleich verlaufen.

**Was er ausdrücklich nicht beschlossen hat**, und ich halte das für den
besseren Teil: keine allgemeine Sitzungsbild-Pflicht. Drei der fünf Läufer
gewinnen davon nachweislich nichts — bei ihnen fällt bei 1,6 jede Kante nach
Division auf ihre Stelle bei 1 zurück (≤ 1,3 logische Pixel). Eine Pflicht, die
in drei von fünf Fällen nichts findet, wird umgangen.

### 26.1 N1 — die README-Bilder zeigen einen Stand vor #55

**Nachgemessen, nicht übernommen:** `docs/bilder/erfassungsfenster.png` ist
1200 × 324, die linke obere Ecke durchgehend `202326` bei Alpha 255 —
rechteckig, deckend, ohne Hülle. Herkunft `cead9f9`, das **vor** `38db754`
liegt. Das öffentliche README zeigt ein Fenster, das `main` nicht mehr
zeichnet.

**Entscheidung des PO: nicht jetzt nachziehen.** Zwei Gründe, beide gemessen:

1. **`readmeshots` ist einer der zwei betroffenen Läufer** (23.7.1). Ein
   frischer Lauf erzeugt heute ein Bild mit der Lücke auf dem Eckbogen — ich
   ersetzte ein veraltetes Bild durch ein fehlerhaftes.
2. **#55 ist abgelehnt und wird mit #83 neu gebaut.** Das Aussehen ändert sich
   ohnehin, und mit ihm die Durchsichtigkeit.

**Gebunden an #83:** Die README-Bilder werden mit dessen Abnahme erneuert, aus
einem Läufer, dessen Alphakanal-Fehler dann behoben ist. Bis dahin steht der
Befund hier — sichtbar, nicht geheilt.

*Was daran nach B10 unbefriedigend bleibt und benannt gehört:* Die README
beschreibt bis dahin nicht den gelieferten Stand. Das ist ein bewusst
getragener Mangel, kein übersehener.
