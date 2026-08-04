# UI-Review Sprint 6 — Fensterhülle (#55) und Feldhöhe (#56)

**Modus:** UI-Review (DoD 3) · **Datum:** 2026-08-04, Ganymed ·
**Prüfstand:** `main` @ `2ef495f` (Merge Strang A) ·
**Prüfer:** `denkzettel-ux`

**Ergebnis in einem Satz:** Beide Stories erfüllen ihre Akzeptanzkriterien; die
Zeichnung 4b ist Maß für Maß getroffen. **Kein `fail`.** Vier Befunde mit
`warn` — einer davon betrifft die Belegkette selbst (vier der zwölf
Hüllenbilder zeigen einen Rollbalken, den es im Betrieb nicht gibt), einer eine
Zusage der Zeichnung, die im zweiten Öffnen bricht.

---

## 1. Die Bilder sind meine, nicht die des Strangs

Der Bildläufer wurde **vor** dem Lauf frisch gebaut, in einem **eigenen**
Bauplatz — `build/` der Repositoriumswurzel gehört während des Sprints anderen
Agenten:

```
cmake -B <bauplatz> -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build <bauplatz> --target captureshots capturetest
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde \
    <bauplatz>/bin/captureshots docs/scrum/reviews/sprint-06-ux-review/bilder
```

Alles zusammen — Bau, Bilder, Sonden, Messungen — wiederholt

```
bash docs/scrum/reviews/sprint-06-ux-review/pruefen.sh
```

**Bilder 01–14** kommen aus `captureshots`, **16, 17 und 20** aus eigenen
Sonden (`sonden/wieder.cpp`, `sonden/setzen.cpp`), **18 und 19** sind Lupen aus
meinen eigenen Bildern (`sonden/ecken.py`, `sonden/lupe4.py`) — Wireframe 4a
zeichnet die linke obere Ecke 4-fach, hier 8- und 12-fach. **Die 15 fehlt
absichtlich:** Bild 15 des Strangs ist eine Aufnahme aus der laufenden
Plasma-Sitzung; die Nummer bleibt frei, damit die beiden Reihen nicht
verwechselt werden.

Die Bildreihe zeigt `breeze-dark` (Rand 4) als schmales und
`CachyOS-Nord-round` (Rand 8) als breites Theme — `bilder/themes.txt`, vom
Läufer selbst geschrieben. **Die Namen sagen die Wahrheit:** Das schmale Theme
liefert Rand 4 und Eckform 6, das breite Rand 8 und Eckform 7 (`masse.txt`,
`bildauswertung.txt`).

**Meine 14 Läuferbilder sind byteweise identisch mit denen des Strangs.** Das
ist kein Ersatz für eigene Bilder, aber es sagt etwas: Der Läufer ist
reproduzierbar, und die Bilder des Strangs zeigen den Stand, den sie zu zeigen
behaupten.

> **Anker (Nachtrag 04.08.2026):** Dieser Satz gilt für den Prüfstand
> `2ef495f`. Seit `c0c623d` weichen die beiden Reihen ab — **beabsichtigt, nicht
> veraltet.** Siehe 7.

Eigene Messungen am gebauten Fenster (nicht nur am Bild) liegen als
`masse.txt`, `ecke.txt`, `schriftwechsel.txt`, `rollbalken.txt`,
`wiederoeffnen.txt` und `bildauswertung.txt` daneben; die Sonden dazu in
`sonden/`.

**Pflichtläufe in meinem Bauplatz:** `capturetest` 21/21 ohne Plattformthema
und 21/21 mit `QT_QPA_PLATFORMTHEME=kde`.

---

## 2. Prüffragen aus der Zeichnung

Jeder gezeichnete Bereich von **4a** und **4b** erzeugt genau eine Prüffrage,
die Raumaufteilung eingeschlossen. Die Fragen stammen aus der Zeichnung, nicht
aus dem Gedächtnis und nicht aus dem Bericht des Strangs.

| Nr. | Prüfsatz (Fundstelle in der Zeichnung) | Urteil | Fundstelle des Belegs |
|---|---|---|---|
| P1 | 4a links: Das Fenster trägt unter dem schmalen Theme dessen Randmaß | **pass** | `masse.txt`: Layoutrand 16/14/16/12 = 12/10/12/8 + 4 |
| P2 | 4a Lupe links: flacher Bogen beim schmalen Theme | **pass** | Bild 18/19, `bildauswertung.txt`: Eckanlauf 6 px, an allen vier Ecken gleich |
| P3 | 4a rechts: dasselbe Fenster unter dem breiten Theme trägt dessen Randmaß | **pass** | `masse.txt`: 20/18/20/16 = 12/10/12/8 + 8; Fensterhöhe 182 gegen 174 |
| P4 | 4a Lupe rechts: runder Bogen, **nicht aus dem Randmaß abgeleitet** | **pass** | Bild 18/19: Eckanlauf 7 gegen 6, während der Rand 8 gegen 4 steht — Eckform folgt dem Rand also gerade nicht |
| P5 | 4a: Aufbau App-Name oben, Textbereich, Fußzeile mittig unten | **pass** | Bilder 01/07/16; `masse.txt` |
| P6 | 4a: Die Hülle wird gerendert, nicht gezeichnet (`dialogs/background`) | **pass** | `capturewindow.cpp:58`, `:302–316`; keine Rundungszahl im Code (P20) |
| P7 | 4a: **Form vom Theme, Farbe aus der Palette** | **pass** | Bild 01: Theme `breeze-dark` unter hellem Schema, Fläche exakt `#eff0f1` = `Window` der Palette (`bildauswertung.txt`) |
| P8 | 4b links: die zweite Fläche des Textbereichs ist weg | **pass** | `masse.txt`: Feld-`Base` = transparent (Alpha 0), Viewport-Autofill aus; `capturewindow.cpp:160,361` |
| P9 | 4b rechts: eine durchgehende Fläche, am Bild gemessen | **pass** | `bildauswertung.txt`: in derselben Zeile links des Feldes, im Feld und rechts davon je (239,240,241) |
| P10 | 4b Farbrollen: Fläche `Window` | **pass** | wie P7 |
| P11 | 4b Farbrollen: Notiztext `WindowText`, nicht die Feldrolle | **pass** | `masse.txt`: Feld-`Text` = `#232629` = `WindowText` des Fensters |
| P12 | 4b Farbrollen: App-Name und Fußzeile `PlaceholderText`, bei jedem Zeichnen neu aufgelöst | **pass** | `masse.txt`: Vordergrundrolle 20 = `PlaceholderText` an beiden Beschriftungen; `capturewindow.cpp:125` setzt die **Rolle**, keine Farbe |
| P13 | 4b Farbrollen: Kontur = Mischung `Window`/`WindowText` im Verhältnis `frameContrast`, **die einzige Linie im Fenster** | **warn** → **U1** | Kanten exakt getroffen (198,200,201 hell / 76,78,81 dunkel), **an den Rundungen fehlt sie** |
| P14 | 4b: Auswahl, Cursor, Rollbalken kommen aus Palette und Widget-Stil | **pass** | Cursor in Bild 16; senkrechter Rollbalken in 03/06/09/12 |
| P15 | 4b Maße: Fensterbreite 600 px | **pass** | `masse.txt`, alle Bilder: 600 unter beiden Themes und in allen Zuständen |
| P16 | 4b Maße: Innenabstand 12/10/8 **zuzüglich** des Theme-Randes | **pass** | `masse.txt` (siehe P1/P3) |
| P17 | 4b Maße: Fußzeile mittig, unterster Baustein; **Abstand über der Fußzeile größer als unter dem App-Namen** | **pass** | `masse.txt`: 12 gegen 8, unter **beiden** Themes und in allen Höhen; Ausrichtung 132 = `AlignCenter` |
| P18 | 4b Maße: Notiztext in der allgemeinen Systemschrift, Kleintexte in der kleinsten lesbaren | **pass** | `masse.txt`: 10 pt gegen 8 pt; in Bild 14 bleiben die Kleintexte klein, während der Notiztext auf 24 pt wächst |
| P19 | 4b Maße: **5 Zeilen beim Öffnen**, mitwachsend bis 8, danach Rollbalken; Höhe folgt auch der Schrift (#56) | **warn** → **U2** | Wachstum, Deckel und Rollbalken sind richtig; das Öffnen ist ab dem zweiten Mal nicht mehr fünfzeilig |
| P20 | 4b: **Rundung und Rand sind keine Zahl** | **pass** | Keine Radius- oder Randkonstante im Code; die einzigen Zahlen sind Breite, Innenabstände, Abstände, `OutlineWidth` und `frameContrast` — alle aus 4b |
| P21 | 4b Schluss: **keine Trennlinie über der Fußzeile** | **pass** | Bilder 01/07/16; keine waagerechte Linie zwischen Feld und Fußzeile |
| P22 | Raumaufteilung: schmales Namensband, dominierender Schreibraum, schmale Fußzeile | **pass** | `masse.txt`: 15 / 98 / 15 px bei 174 px Fensterhöhe — der Schreibraum trägt 56 % |
| P23 | 4b: linke Textkante von App-Name, Notiztext und Inhalt bündig | **warn** → **U3** | Notiztext sitzt 4 px weiter rechts als der App-Name |
| P24 | #55 AK 4: Hülle bei 5 wie bei 8 Zeilen vollständig und unverzerrt | **pass** | `bildauswertung.txt`: gleicher Eckanlauf und deckende Kantenmitten bei 174 wie bei 228 px, unter beiden Themes und Schemata |
| P25 | #56: Zwei deutlich verschiedene Schriftgrößen zeigen je fünf Zeilen | **pass** | Bilder 13/14; `masse.txt`, `schriftwechsel.txt`: 9 pt → 93 px, 24 pt → 223 px, beide exakt 5 Zeilen |
| P26 | Belegtauglichkeit: Zeigen die zwölf Hüllenbilder den Zustand, den sie behaupten? | **warn** → **U4** | Bilder 02/05/08/11 zeigen einen waagerechten Rollbalken, den es nur einen Wimpernschlag lang gibt |

**Der Schatten ist nicht mein Prüfgegenstand** (Auftrag, Planning §6.2). Ich habe
ihn weder geprüft noch bewertet.

---

## 3. Die vier Befunde

### U1 · `warn` — Die Kontur läuft nicht um die Rundung, und an ihrem Ende steht eine Kerbe

**Was 4b zusagt:** Die Kontur ist „die **einzige Linie** im Fenster", eine
Mischung aus `Window` und `WindowText` im Verhältnis `frameContrast`. Die Lupe
in 4a zeichnet sie als Bogen, der **um die Ecke herumläuft** (`border-radius`
auf allen vier Lupenkanten).

**Was gebaut ist** (`bildauswertung.txt`, `ecke.txt`, Bilder 18 und 19):

* Auf allen vier **geraden Kanten** sitzt genau ein Pixel in genau der
  zugesagten Farbe — hell (198, 200, 201), dunkel (76, 78, 81). Das ist die
  Mischung mit 0,20 auf den Punkt.
* Auf den **Eckbögen** kommt diese Farbe **nirgends** vor. Die Pixel springen
  vom Hintergrund direkt auf die Fläche. Beim schmalen Theme beginnt die Kontur
  erst 6 px nach der Ecke, beim breiten erst 10 px — das ist der ganze Bogen.
* Am Übergang von Bogen zu Kante steht je Ecke ein **vollständig
  durchsichtiges Pixel innerhalb der Silhouette** (`ecke.txt`: Alpha 0 bei
  (1|6) und (6|1) unter `breeze-dark`, bei (1|7) und (7|1) unter
  `CachyOS-Nord-round`). Es ist **kein eingeschlossenes Loch** — geprüft und
  widerlegt (`sonden/loecher.py`: 0 Löcher mit vier deckenden Nachbarn) —,
  sondern eine Kerbe, die zum Bogen hin offen ist.

**Warum das im hellen Schema mehr weh tut als im dunklen:** Im dunklen Schema
liegt die Fläche weit vom Hintergrund entfernt, die Ecke zeichnet sich durch
den Alphaverlauf selbst ab (Bild 19, Felder 3 und 4). Im hellen Schema liegen
Fläche und Untergrund nah beieinander — dort **endet der Umriss des Fensters an
der Ecke sichtbar im Nichts** (Bild 19, Felder 1 und 2).

**Mögliche Ursache, mit ihrem Beleg und ihrer Grenze:** Die Hülle entsteht als
Ring — äußere Form in Konturfarbe, darüber dieselbe Form 2 px kleiner in
Flächenfarbe, um (1,1) versetzt (`capturewindow.cpp:314–315, 328–329`). Ein
`FrameSvg` **skaliert seine Eckstücke aber nicht**: Die Alphamaske des inneren
Rahmens trägt an der Ecke exakt dieselben Werte wie die des äußeren
(`ecke.txt`, Abschnitt „Alphamaske"). Damit verschluckt die innere Form am
Bogen genau das, was der Ring dort sein sollte. *Grenze dieser Erklärung:* Die
Alphamasken meiner Sonde und das, was das Fenster tatsächlich malt, liegen an
der Diagonalen um ein Pixel auseinander; die letzte Pixelrechnung habe ich
nicht zu Ende geführt. Der **Befund** steht am Bild, die **Ursache** ist ein
begründeter Vorschlag.

**Korrekturvorschlag (Entscheidung des Entwicklers):** Den Ring nicht aus zwei
verschieden großen Rahmen bilden, sondern aus **einer** Maske und ihrer
Erosion — dieselbe Alphamaske viermal um je ein Pixel versetzt mit
`CompositionMode_DestinationIn` in ein Zwischenbild, das Ergebnis als
Innenform. Dann folgt die Linie der Rundung überall gleich breit, und die Kerbe
kann nicht entstehen.

**Warum nicht `fail`:** Kein Akzeptanzkriterium von #55 verlangt die
geschlossene Kontur; verlangt sind Rundung, Kontur und Schatten aus dem Theme,
und die sind da. Der Befund ist 1 px breit. Er gehört als Issue gebucht, nicht
als Sprintblockade.

---

### U2 · `warn` — „5 Zeilen beim Öffnen" gilt nur beim ersten Mal

**Was 4b zusagt:** „5 Zeilen beim Öffnen, mitwachsend bis 8, danach
Rollbalken." SPEC 3 sagt dasselbe, mit dem Kundenbefund der Sprint-1-Abnahme im
Rücken.

**Was gebaut ist** (`wiederoeffnen.txt`, Bilder 16 und 17 — der Weg, den der
Nutzer geht: öffnen, acht Zeilen tippen, **Esc**, Kürzel erneut):

```
1. erstes Öffnen              : Fenster 174 px      (5 Zeilen)
2. acht Zeilen getippt        : Fenster 228 px      (8 Zeilen)
3. nach Esc (versteckt)       : Fenster 228 px, Text leer
4. zweites Öffnen, Feld leer  : Fenster 228 px      -> 8 Zeilen Leere
```

Bild 17 zeigt es: ein leeres Fenster in Achtzeilenhöhe. Ab der ersten längeren
Notiz öffnet sich das Fenster für den Rest der Sitzung in der Höhe der längsten
je getippten Notiz.

**Und es trifft #56 unmittelbar** (`schriftwechsel.txt`), was in der Meldung des
Strangs so noch nicht steht:

```
Schriftfolge 9pt -> 24pt -> 9pt
  verdeckt: 169 -> 299 -> 169     (das Fenster folgt)
  gezeigt : 174 -> 299 -> 299     (das Feld folgt, das Fenster nicht)
```

Das **Textfeld** ist in beiden Fällen richtig — 5 Zeilen, immer. Damit ist
AK 1 von #56 wörtlich erfüllt, es spricht vom Textfeld. Das **Fenster** aber
bleibt nach einer Schriftverkleinerung am gezeigten Fenster auf der alten Höhe
stehen: 223 px Feld in einem 299 px hohen Fenster.

**Ursache** (vom Strang bereits als B1 gemeldet, hier bestätigt und um den
Schriftfall erweitert): Das Layout setzt am gezeigten Fenster eine Mindesthöhe,
die das `resize()` in `adjustHeight()` deckelt; danach löst nichts ein erneutes
Anpassen aus. Der Befund ist **vorbestehend** — der Strang hat ihn gegen den
Ausgangsstand `0a229d2` gemessen —, aber er widerspricht einer **gezeichneten**
Zusage und ist am Kundenblick sichtbar.

**Korrekturvorschlag:** eigenes Issue, mit beiden Gesichtern in einem: das
Wiederöffnen **und** die Schriftverkleinerung am stehenden Fenster. Der grüne
Test `windowFollowsTheTextHeight()` gehört dabei mitgenommen — er ist nur grün,
weil er das Fenster nie zeigt (B1 des Strangs).

**Für die Abnahme-Checkliste:** *„Eine lange Notiz tippen, mit Esc verwerfen,
Kürzel erneut drücken — steht das Fenster wieder auf fünf Zeilen?"*

**Warum nicht `fail`:** vorbestehend, außerhalb der Akzeptanzkriterien beider
Stories, und keine der beiden hat ihn verschlimmert.

---

### U3 · `warn` — Der Notiztext steht 4 px weiter rechts als der App-Name

**Was 4b zeigt:** In der Spalte „Neu — eine Fläche" sitzen App-Name und
Notiztext an derselben linken Kante. Die Spalte „Heute" gibt dem Textbereich
ausdrücklich eine eigene Polsterung — das ist der Zustand, den die Zeichnung
ablöst.

**Was gebaut ist** (`masse.txt`, Bilder 02/13/14): Beschriftung und Textfeld
beginnen beide bei x = 16 (schmales Theme). Das Textfeld setzt seinen Text aber
erst hinter seinem eigenen Dokumentrand von 4 px, also bei x = 20. Am Bild
gemessen: Tinte des App-Namens ab x = 17, Tinte des Notiztexts ab x = 21 —
unter dem breiten Theme 21 gegen 25, derselbe Versatz.

Sichtbar wird es dort, wo die Schrift groß ist: In Bild 14 (24 pt) springt der
Notiztext gegenüber „Denkzettel" erkennbar ein. Nach den KDE HIG ist eine
gemeinsame linke Kante der Inhaltsspalte der Regelfall; hier hat das Fenster
zwei Kanten, die um 4 px auseinanderliegen.

**Korrekturvorschlag:** `document()->setDocumentMargin(0)` am Textfeld. Die
Höhenrechnung liest den Dokumentrand bereits aus (`capturewindow.cpp:455`) und
bleibt damit von selbst richtig; der senkrechte Anteil des Dokumentrandes fällt
dann ebenfalls weg, was den Abstand unter dem App-Namen auf die gezeichneten
8 px zurückführt.

**Warum nicht `fail`:** 4 px, kein Akzeptanzkriterium spricht die Textkante an.

---

### U4 · `warn` — Vier der zwölf Hüllenbilder zeigen einen Rollbalken, den es im Betrieb nicht gibt

**Was auffällt:** In den Bildern **02, 05, 08 und 11** (Zustand „getippt", beide
Themes, beide Schemata) liegt am unteren Rand des Textfelds ein **waagerechter
Rollbalken** über die volle Breite, mit einer Linie darüber. In der Zeichnung
kommt er nicht vor, und das Feld bricht Zeilen um — zu rollen gibt es nichts.

**Gemessen** (`rollbalken.txt`, Sonde `sonden/rollbalken2.cpp` — genau der Weg
des Bildläufers):

```
Zustand 1 (getippt)
   direkt nach show() : waagerecht sichtbar=1  max=0  Sichtfenster 77
   150 ms nach show() : waagerecht sichtbar=0  max=0  Sichtfenster 98
```

Der Balken lebt zwischen `show()` und dem ersten Durchlauf der
Ereignisschleife; sein Rollbereich ist leer (`max=0`). Der Läufer greift das
Bild **unmittelbar** nach `show()` und hält damit einen Zustand fest, den es im
Betrieb nicht gibt. Mein Gegenbild **20** zeigt dieselbe Szene nach einem
Durchlauf der Ereignisschleife — ohne Balken.

**Warum das zählt:** Nach DoD 3 ist bei Zuständen das Bild der Prüfgegenstand.
Ein Bild, das ein Element zeigt, das die Gestaltung nicht vorsieht, kostet
entweder eine Nachfrage in der Abnahme oder — schlimmer — es wird geglaubt.
Vier von zwölf Hüllenbildern sind betroffen, in beiden Belegreihen (meine und
die des Strangs sind byteweise gleich).

**Korrekturvorschlag:** Im Bildläufer zwischen `show()` und `grab()` die
Ereignisschleife einmal laufen lassen (`QTest::qWait(…)`), dann bilden die
Bilder den Zustand ab, den der Nutzer sieht. Der Läufer gehört dem Entwickler —
ich habe ihn nicht angefasst.

**Kein Produktbefund:** Ob der Balken im Betrieb je ein Bild lang aufblitzt, ist
damit **nicht** gesagt und wäre am laufenden Compositor zu klären. Was belegt
ist: In den Belegbildern steht er, und im ruhenden Fenster steht er nicht.

> **Anker (Nachtrag 04.08.2026):** Dieser Befund samt der Aussage „byteweise
> gleich" gilt für den Prüfstand `2ef495f`. Behoben mit `c0c623d`; die Reihe des
> Strangs zeigt den Balken nicht mehr, meine weiterhin — sie ist der Beleg zum
> geprüften Stand. Siehe 7.

---

## 4. Was ich nicht prüfen konnte

1. **Den Schatten** — ausdrücklich nicht mein Prüfgegenstand (Auftrag,
   Planning §6.2). Weder Objekt noch Aussehen bewertet.
2. **Den installierten Stand.** Geprüft ist der gebaute Sprint-Stand in einem
   eigenen Bauplatz. Es gibt nur ein `/usr`, und die Installation taktet der PO
   (DoD 2). Ob die Hülle nach der Installation dieselbe ist, sagt dieser
   Bericht nicht.
3. **Den Theme-Wechsel über die Systemeinstellungen.** Meine Bilder wechseln
   das Theme über `reloadDesktopTheme(name)`, wie der Läufer. Der Weg
   „Systemeinstellungen → `plasmarc` → laufendes Fenster" hätte das
   Desktop-Theme des Kunden verstellt; der Strang hat ihn in Teilen gemessen
   (Grenze 4 seines Berichts), und mein Review schließt diese Lücke **nicht**.
4. **Auswahlfarbe und Textcursor im Bild.** Die Zeichnung zeigt keinen
   Auswahlzustand, der Läufer erzeugt keinen. Der Cursor ist in Bild 16
   sichtbar, mehr nicht.
5. **Verhalten bei hoher Bildschirmauflösung (Skalierung > 1).** Offscreen mit
   Faktor 1 gemessen. Die Hülle wird aus Pixelmasken gebaut; ob die Konturbreite
   bei Skalierung 1,5 oder 2 hält, ist offen — und angesichts von U1 der Punkt,
   an dem ich zuerst nachsähe.
6. **#59** — keine UI-Story, vom PO eingestuft (Sprint 6 §15.2); nicht Teil
   dieses Reviews.

---

## 5. Was der PO daraus machen muss

1. **Kein `fail`, die DoD ist aus UI-Sicht nicht blockiert.** #55 und #56
   treffen ihre Akzeptanzkriterien und die Zeichnung.
2. **Zwei Issues** aus U1 (Kontur an der Rundung) und U3 (Textkante) — beide
   klein, beide belegt, beide am Kundenblick sichtbar.
3. **U2 ist der Befund mit dem größten Gewicht** und der einzige, den der Kunde
   in der Abnahme von selbst finden wird. Er ist vorbestehend; die Empfehlung
   des Strangs, ihn als eigenes Issue zu buchen, verstärke ich um den
   Schriftfall — er liegt näher an #56, als die Meldung bisher sagt.
4. **U4 betrifft die Belegkette, nicht das Produkt.** Er ist billig zu heilen
   und sollte geheilt werden, bevor die Bilder in die Abnahme gehen.
5. **Zwei Sätze für die Abnahme-Checkliste**, zusätzlich zu denen des Strangs:
   *„Eine lange Notiz tippen, mit Esc verwerfen, Kürzel erneut drücken — steht
   das Fenster wieder auf fünf Zeilen?"* und *„Auf hellem Hintergrund: laufen
   die Ecken des Fensters sauber um, oder endet die Linie vor der Rundung?"*

> **Anker (Nachtrag 04.08.2026):** Dieser Abschnitt ist eine Empfehlung an den
> PO zum Prüfstand `2ef495f`. Was er daraus gemacht hat — drei Issues, eine
> Heilung, zwei Sätze in der Abnahme-Checkliste — steht in 7.

---

## 6. Dateien dieses Reviews

```
docs/scrum/reviews/sprint-06-ux-review/
├── bericht.md              diese Datei
├── pruefen.sh              wiederholt alles: Bau, Bilder, Sonden, Messungen
├── bilder/                 01–14 (Läufer), 16/17/20 (Sonden), 18/19 (Lupen), themes.txt
├── masse.txt               Maße aus 4b am gebauten Fenster, beide Themes
├── wiederoeffnen.txt       U2, erster Teil
├── schriftwechsel.txt      U2, zweiter Teil
├── rollbalken.txt          U4
├── ecke.txt                U1: ARGB der Fensterecke und die beiden Alphamasken
├── bildauswertung.txt      Kontur, Eckprofile, Tintenkanten, Vollständigkeit
└── sonden/                 die Prüfprogramme und Auswerteskripte
```

Ein unversionierter Beleg ist kein Beleg (B7) — deshalb liegen die Sonden hier
und nicht im Scratchpad.

---

## 7. Belegnachtrag 04.08.2026 — was seit dem Prüfstand geschehen ist

**Der Bericht oben bleibt unverändert.** Er beschreibt den Stand `2ef495f`, und
das soll er weiter tun: Ein Beleg, den man passend macht, ist keiner. Was hier
folgt, ist der Anker — nicht die Glättung.

**Die beiden Bildreihen sind seit `c0c623d` nicht mehr gleich.** Der Strang hat
U4 in `shoot()` selbst geheilt — `show()` und der Durchlauf der Ereignisschleife
stehen jetzt beide dort, statt an den vierzehn Aufrufstellen — und seine Reihe
neu erzeugt. **Nachgemessen am 04.08.2026:** Alle vierzehn Bilder weichen jetzt
ab, nicht nur die vier aus U4.

```
für jedes Bild:  cmp  bilder/<name>  ../sprint-06-s55-huelle/bilder/<name>
    -> 14 von 14 abweichend
```

Dass **alle** abweichen und nicht nur die vier, ist erwartbar: Ein Bild nach dem
ersten Ereignisdurchlauf zeigt auch den Textcursor und den eingeschwungenen
Rollbalkengriff. Der Strang zählt die drei Unterschiede in seinem Bericht aus.

**Meine Reihe bleibt, wie sie ist.** Sie ist der Beleg zum geprüften Stand, und
ein Bild, das man dem geheilten Stand nachzieht, belegt nichts mehr. Wer die
beiden Reihen nebeneinanderlegt, sieht den Unterschied, den U4 benannt hat —
das ist ein Gewinn, kein Mangel.

**Stand der vier Befunde** (vom PO gebucht, nicht von mir):

| Befund | Stand |
|---|---|
| **U1** Kontur an der Rundung | Issue **#80** |
| **U2** Öffnungshöhe und Schriftverkleinerung | in Issue **#79** aufgenommen |
| **U3** Textkante des Notiztexts | Issue **#81** |
| **U4** Rollbalken in den Belegbildern | **behoben**, `c0c623d` — damit ist auch die Empfehlung 5.4 erledigt |

**Nicht nachgezogen wird:** kein Urteil, kein Befundtext, keine Prüffrage. Was
im Präsens steht, steht im Präsens des Prüfstands `2ef495f` — so, wie es der
Kopf dieses Berichts ansagt.
