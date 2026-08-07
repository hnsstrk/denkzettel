# Messung B zu Issue #100 — Eingabefeld aus der Theme-Grafik

**Bearbeiter B:** Scrum Master · **Datum:** 07.08.2026, 20:14–20:30 Uhr
**Grundlage:** Issue #100 samt allen vier Kommentaren, Zeichnung 4b (Prüfsätze
F1–F5, Stand `36156fd`), `SPEC.md` 3.1/3.2, `CLAUDE.md`, `docs/scrum/PROZESS.md`,
Vorprüfbericht zu #83.

**Unabhängigkeit.** `docs/scrum/vorberichte/100-eingabefeld/messung-a.md` ist
nicht gelesen worden; zum Zeitpunkt dieser Messung lag im Ordner nichts von
Bearbeiter A. Alle Zahlen unten stammen aus eigenen Läufen, deren Quelltext
unter `sonden-b/` und deren Ausgaben unter `messungen-b/` liegen.

**Was gebaut wurde und wo.** Vier eigene Sonden (`sonden-b/`, gebaut nach
`build-b/`) und einmal das Projekt selbst nach
`docs/scrum/vorberichte/100-eingabefeld/build-projekt/` — nicht nach `build/`,
in dem gerade drei weitere Stränge arbeiten. Keine Installation nach `/usr`.
Der Ausgangslauf `ctest -R capture` ist in diesem eigenen Verzeichnis **grün**
(`1/1 Passed`), bevor irgendetwas gemessen wurde; ohne diesen Ausgangspunkt
wäre „bricht" unten von „war schon rot" nicht zu unterscheiden.

---

## 1. Dateimenge

Am Code vermessen, Notation B13.

**Quellen und Tests**

| Datei | Zeilen | was daran hängt |
|---|---|---|
| `src/capture/capturewindow.h` | 218 | dritte `KSvg::FrameSvg` als Feld; `ThemeTextColours` wird durch AK 3 fraglich (siehe Falle F7) |
| `src/capture/capturewindow.cpp` | 660 | Konstruktor, `reloadDesktopTheme()`, `paintEvent()`, `resizeHull()`, `applyTextColours()`, `adjustHeight()` |
| `tests/capturetest.cpp` | 1206 | **sieben** bestehende Prüfsätze brechen oder verlieren ihren Gegenstand (Falle F1); fünf neue für F1–F5 |
| `tests/captureshots.cpp` | 226 | Bildreihe für die Belegform „zwei Themes, zwei Schemata" |
| `tests/desktopthemes.h` | 164 | Auswahl eines Themes **mit** und eines **ohne** eigene `lineedit`-Grafik; die vorhandenen Wähler sortieren nach Hüllenrand, nicht nach dieser Eigenschaft (Falle F5) |

**Build** — keine Änderung erforderlich. Kein neues Ziel, keine neue Quelldatei,
keine neue Abhängigkeit: `KF6::Svg` ist seit #55 verlinkt, und
`DENKZETTEL_TEST_THEMES` steht in `tests/CMakeLists.txt:58` und `:127` bereits.
Nur falls ein weiteres mitgeliefertes Prüf-Theme entsteht (siehe Feld 6, Frage 3),
kommen Dateien unter `tests/themes/` hinzu — auch dann ohne CMake-Änderung, weil
der Pfad und nicht die Dateiliste übergeben wird.

**Belege und Prüfmittel** — neuer Ordner `docs/scrum/reviews/sprint-NN-s100-eingabefeld/`
mit `bericht.md`, `pruefen.sh` und `bilder/`; darin **ein Bild aus der
angemeldeten Sitzung** für AK 2 und AK 4 (B21), erzeugt über den fertig
vorliegenden Weg `docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/echtelage.cpp`
(mit `git ls-files` bestätigt vorhanden).

**Fachliche Quellen** — `SPEC.md`, Abschnitt 3.1: **drei** Spiegelstriche, nicht
einer. Der Spiegelstrich *Fläche* („eine durchgehende — kein Kasten im Kasten"),
der Spiegelstrich *Die Schrift kommt aus derselben Quelle wie die Fläche* (er
schreibt heute ausdrücklich `WindowText` fest, „**nicht** die Rolle für
Eingabefelder") und der Spiegelstrich *Innenabstände*. Dazu die Grenze aus AK 6.

**Ausdrücklich nicht**

- `wireframes/Denkzettel Wireframes.dc.html` — mit `36156fd` bereits nachgezogen.
  Wer hier nacharbeitet, überschreibt einen datierten Vermerk (B17).
- `src/ui/`, `src/shell/`, `src/store/`, `src/main.cpp` und alle übrigen Tests
  (`librarytest`, `shelltest`, `storetest`, `editshots`, `libraryshots`,
  `searchshots`, `readmeshots`).
- `docs/scrum/PROZESS.md`, `CLAUDE.md`, `CHANGELOG.md`, `README.md`.
- `docs/scrum/reviews/2026-08-06-lesbarkeit/` und alle älteren Belegordner —
  ein Beleg wird geankert, nicht nachgezogen.

---

## 2. Gemessene Fallen

Je Falle: was passiert, woran es gemessen ist, und was der Strang deshalb tun
muss. Ungemessenes ist als solches gekennzeichnet.

### F1 — Sieben bestehende Prüfsätze brechen, zwei davon aus einem Grund, der mit der Sache nichts zu tun hat

Gemessen an `messungen-b/m5-fenstergeometrie.txt` und `messungen-b/m6-deckung.txt`.

| Prüfsatz | warum er bricht |
|---|---|
| `paintsTheThemesOwnHullInOnePiece()` | tastet einen Punkt **im** Textbereich gegen einen **daneben** ab und verlangt Gleichheit. Das ist die alte Festlegung in Testform. |
| `noteTextUsesTheWindowTextRole()` | F3 setzt den Prüfsatz ab. Der Test setzt `WindowText` auf `#fcfcfc` und `Text` auf `#112233` und erwartet `#fcfcfc`; unter AK 3 kommt `#112233`. |
| `noteTextComesFromTheThemesOwnColours()` | erwartet `[Colors:Window] ForegroundNormal` des breiten Prüf-Themes (`#ff0099`); unter AK 3 gilt `[Colors:View] ForegroundNormal` (`#00cc00`). |
| `textColoursFollowADesktopThemeChange()` | dieselbe Erwartung |
| `themeTextColoursOutlastAColourSchemeChange()` | dieselbe Erwartung, dazu setzt der Test nur `WindowText`, nicht `Text` |
| `hullIsCompleteAtFiveAndEightLines()` | tastet `QPoint(Breite/2, Höhe/2)` ab |
| `takesTheOpaqueVariantWithoutABlurringCompositor()` | tastet denselben Punkt ab |

Die letzten beiden brechen aus einem Grund, den kein Akzeptanzkriterium
benennt: **Der Fenstermittelpunkt liegt im Textbereich.** Gemessen am
laufenden Fenster unter drei Themes und bei beiden Größen aus SPEC 3:

```
default:                Fenster 600x178  Textbereich 18,39 564x98   Mitte 300,89   im Feld: JA
default (acht Zeilen):  Fenster 600x232  Textbereich 18,39 564x152  Mitte 300,116  im Feld: JA
denkzettel-test-schmal: Fenster 600x174  Textbereich 16,37 568x98   Mitte 300,87   im Feld: JA
CachyOS-Nord-round:     Fenster 600x182  Textbereich 20,41 560x98   Mitte 300,91   im Feld: JA
```

Und die Deckung, an der beide Prüfsätze hängen (`m6-deckung.txt`):

```
default:            Huelle 216   Feld 255
breeze-dark:        Huelle 216   Feld 255
CachyOS-Nord-round: Huelle 255   Feld 15
cachyos-emerald:    Huelle 7     Feld 15
```

**Das Gefährliche daran ist nicht das Rot, sondern das Grün.** Unter
`default` und `breeze-dark` wird die Stelle 255 statt 216 tragen und der
Prüfsatz meldet sich. Unter `CachyOS-Nord-round` bleibt er **grün** — dort ist
die Hülle bereits deckend, das Feld ändert die Zahl nicht, und der Satz misst
danach das Feld statt der Hülle, ohne dass jemand es merkt. Welchen Fall ein
Lauf trifft, entscheidet `themes::anyInstalledTheme()`, und das ist auf dieser
Maschine gemessen `CachyOS-Nord-round` (`messungen-b/m4-themepaar.txt`).
**Beide Abtastpunkte gehören aus dem Feld heraus**, sonst entsteht dieselbe
Sorte Prüfsatz, die dieses Projekt an einem Abend viermal entlarvt hat.

### F2 — Ein `FrameSvg` folgt auch als dritter nur einem frischen `ImageSet`

Die Falle aus #83 wiederholt sich unverändert; sie ist an der neuen Grafik
nachgemessen (`messungen-b/m1-feld-je-theme.txt`, Abschnitt M5):

```
default: #141618  nach Umbenennen: #141618  nach frischem Set: #000000
Umbenennen wirkt: NEIN   frisches Set wirkt: ja
```

**Der Bau ist darauf schon eingerichtet**: `reloadDesktopTheme()` baut ohnehin
ein frisches Set und reicht es in einer Schleife an alle Rahmen weiter
(`capturewindow.cpp:318`). Der neue Rahmen gehört in **diese Schleife**. AK 4
ist damit fast umsonst zu haben — und fällt genauso lautlos aus, wenn der
Rahmen daneben angelegt wird.

### F3 — Die Abhebung des Feldes ist keine Eigenschaft des Codes

Gemessen (`messungen-b/m2-feld-auf-huelle.txt`): Feld über Hülle über Grund,
je einmal über schwarzem und über weißem Grund.

```
default | schwarz | Huelle #1b1e20 | Flaeche #141618 | Kante #4c4e51 | 1,082 : 1 | 2,008 : 1
default | weiss   | Huelle #424547 | Flaeche #141618 | Kante #4c4e51 | 1,878 : 1 | 1,157 : 1
```

Die Hülle deckt unter `default` zu 216/255, lässt also den Grund durch. Damit
hängt die Abhebung am **Bildschirmhintergrund des Kunden** und schwankt für die
Fläche zwischen 1,08 : 1 und 1,88 : 1. Die im Issue genannten 1,39 : 1 und
1,33 : 1 liegen in dieser Spanne und widersprechen ihr nicht; sie sind eine
Zahl für einen Grund, den keiner der beiden Berichte benennt. Für die
Zusicherung folgt daraus: **prüfbar ist die Herkunft (F1/AK 1), nicht die
Zahl.** Genau das sagt die Zeichnung an ihrer Grenzstelle auch; AK 6 sagt es
nicht (siehe Feld 3).

### F4 — AK 6 sichert die Abhebung unter Themes zu, die sie nicht liefern

Wortlaut AK 6: „Zugesichert ist die Abhebung unter dem Rückfall `default` und
unter den Themes, die die Grafik zeichnen."

Gemessen zeichnen **alle elf** geprüften Themes die Grafik — die fünf schwachen
eingeschlossen (`m1-feld-je-theme.txt`, Spalte *base gueltig*: elfmal `ja`).
Was sie unterscheidet, ist die Deckung: 255 unter `default` und den
Breeze-Themes gegen **15** unter `CachyOS-Nord-round`, `Iridescent-round` und
den drei `cachyos-emerald`-Themes. Der Wortlaut trifft damit die fünf Themes
mit, unter denen er gerade nicht gilt.

Geschichtet gemessen liegen diese fünf bei 1,000–1,143 : 1 für die Fläche und
**1,000 : 1 für die Kante** — die Kante ist dort gar nicht da. Die Angabe
1,03–1,10 : 1 aus dem Issue ist eine Stufe daneben und im Ergebnis dieselbe
Aussage.

### F5 — Die vorhandene Theme-Wahl trifft die Belegform nur zufällig

Die Belegform von F1 verlangt ein Theme **mit** und eines **ohne** eigene
`lineedit`-Grafik. Die vorhandenen Wähler in `tests/desktopthemes.h` sortieren
nach **Hüllenrand**. Gemessen (`m4-themepaar.txt`):

```
installedThemePair(): schmal=breeze-dark  breit=CachyOS-Nord-round
anyInstalledTheme():  CachyOS-Nord-round
```

Auf dieser Maschine passt das: `breeze-dark` bringt keine eigene Grafik,
`CachyOS-Nord-round` bringt eine. Auf dem CI-Läufer gibt es kein Paar mit
verschiedenem Rand — `libplasma` liefert `default`, `breeze-dark`,
`breeze-light`, und alle drei tragen 4 px —, also fällt der Lauf auf die
mitgelieferten Themes zurück, und **beide** bringen keine eigene
`lineedit`-Grafik. Dieselbe Bauart wie die in `desktopthemes.h` bereits
festgehaltene Falle mit `bundledSquare()`. Ein Wähler nach der richtigen
Eigenschaft gehört dazu; auf dem CI-Läufer stehen `default` (eigene Grafik,
`.svgz`) und `breeze-light` (keine) beide bereit, die Belegform ist dort also
erfüllbar.

### F6 — Ohne Desktop-Theme gibt es auch kein Feld, und das steht in keinem Kriterium

`staysUsableWithoutADesktopTheme()` prüft in einem eigenen Prozess ohne
Theme-Pfad, dass der Fenstermittelpunkt die Palettenfarbe trägt. Löst
`widgets/lineedit` nicht auf und zeichnet der Bau trotzdem, ist der Prüfsatz
rot; wacht der Bau wie bei der Hülle (`paintEvent`, `capturewindow.cpp:400`),
bleibt er grün. **Der Bau braucht dieselbe `isValid()`-Wache für das Feld.**
Kein Akzeptanzkriterium verlangt sie. *Nicht gemessen*, weil dafür Code zu
ändern wäre; die Aussage folgt aus dem Prüfsatz und aus der bestehenden Wache
für die Hülle.

### F7 — Der Kreuzvergleich der beiden Farbwege bricht mit AK 3

`noteTextComesFromTheThemesOwnColours()` hält absichtlich zwei Wege
gegeneinander: die gemalte Farbe (`KSvg::Svg::color(Text)`) gegen die selbst
gelesene (`themeTextColoursOf().normal` aus `[Colors:Window]`). Unter AK 3
malt der eine Weg aus `[Colors:View]`, der andere liest weiter
`[Colors:Window]` — die Klammer geht auf. Entweder liest
`themeTextColoursOf()` künftig beide Gruppen, oder der Kreuzvergleich fällt weg.
**Die Struktur `ThemeTextColours` und ihre Kopfkommentare hängen daran**
(`capturewindow.h:60–80`).

Dazu ein Nebenbefund am Prüfmuster: Die Gruppe `[Colors:View]` im
mitgelieferten breiten Prüf-Theme ist laut ihrem eigenen Kommentar „für die
Mutationsprobe und für sonst nichts" angelegt. Gemessen färbt sie **schon
heute die Feldfläche**: unter diesem Theme rendert das Feld `#332211`, und das
ist Zeichen für Zeichen `[Colors:View] BackgroundNormal = 51,34,17`
(`m1-feld-je-theme.txt`). Wer die Werte für die Mutationsprobe ändert,
verschiebt ab dieser Story stillschweigend auch das, was die Feldprüfsätze
messen. Der Kommentar gehört mit.

### F8 — `KSvg::Svg::color(ViewText)` hängt nicht am Farbsatz, die Farbe wechselt trotzdem

Gemessen (`m1-feld-je-theme.txt`, Abschnitt M4): `Window/ViewText` und
`View/ViewText` liefern durchweg denselben Wert; `Window/Text` und `View/Text`
tun es **nicht**, sobald das Theme eine eigene `colors`-Datei mitbringt
(`denkzettel-test-breit`: `#ff0099` gegen `#00cc00`; `cachyos-emerald-color`:
`#00c790` gegen `#02bd88`). Für den Bau heißt das: `color(ViewText)` genügt,
ein zusätzliches `setColorSet(View)` ist nicht nötig — und ein irrtümlich
gesetztes `colorSet` an der falschen Stelle würde die Hüllenfarbe mit
verschieben (AK 2).

Für die **Grafik** ist kein `colorSet` nötig: Das Feld hat in allen Läufen
ohne gesetzten Farbsatz die View-Farben des Themes gezogen.

### F9 — Der Rand des Vorsatzes `base` ist 6 px, aber nicht als Ganzzahl

Gemessen über alle elf Themes (`m1-feld-je-theme.txt`, Spalte *Rand*): 6/6/6/6
unter acht, **6/6/5.99999/6.00001** unter den vier CachyOS-Themes. Die Angabe
aus AK 5 stimmt. Ein Prüfsatz mit `==` auf `6` fällt bei vier Themes um; das
Projekt hält dieselbe Beobachtung schon für `marginSize()` fest („hands out
7,99998 where a drawing would have written 8", `capturetest.cpp:114`).

### F10 — Wo die 6 px landen, entscheidet, ob ein Prüfsatz mitwächst oder rot wird

Gemessen (`m5-fenstergeometrie.txt`): `documentMargin` = 4, `frameWidth` = 0.
`adjustHeight()` rechnet `chrome = 2·documentMargin + 2·frameWidth`, und
`heightFollowsAFontChange()` rechnet dieselbe Formel nach. Geht der
6-px-Einzug über `documentMargin`, wächst der Prüfsatz von selbst mit; geht er
über einen anderen Weg, wird er rot — oder, schlimmer, das Feld beschneidet den
Text, ohne dass die Höhe es meldet.

### F11 — Das Bildpunktverhältnis muss auch am neuen Rahmen ankommen

`resizeHull()` setzt es an der Hülle von Hand, weil ein `FrameSvg` dem
Bildschirm nicht folgt (`capturewindow.cpp:449`, gemessen in #83). Für den
dritten Rahmen gilt dasselbe. Offscreen fällt ein Fehlen nicht auf: Das
Ereignis `DevicePixelRatioChange` kommt dort nie an — festgehalten im Code an
`capturewindow.cpp:434` und in `CLAUDE.md`.

### F12 — AK 3 und AK 6 greifen ineinander, und die Rechnung geht dabei einmal unter 4,5 : 1

Über alle 19 auf dieser Maschine installierten Farbschemata gerechnet
(`messungen-b/m3-schriftgrund-18-schemata.txt`; das Projekt spricht von 18,
`Ant-Dark` liegt zusätzlich im Benutzerverzeichnis):

| Fall | schlechtester Wert |
|---|---|
| heute: `WindowText` auf der Fensterfläche | **4,74 : 1** (KritaNeutral) |
| AK 3, Feld sichtbar: `Text` auf der Feldfläche | **5,70 : 1** (IridescentLightly3) |
| AK 3, Feld unsichtbar: `Text` auf der Fensterfläche | **4,22 : 1** (KritaNeutral) |

Die ersten beiden Zahlen reproduzieren die im Projekt umlaufenden 4,74 und 4,22
Ziffer für Ziffer, die Rechnung ist also an bekannten Werten geeicht.

Der dritte Fall ist der, den AK 6 als Grenze benennt, ohne ihn zu Ende zu
rechnen: Unter `CachyOS-Nord-round` — Hülle deckend, Feld bei 15/255 unsichtbar,
keine eigene `colors`-Datei, also Schemaweg — steht der Notiztext künftig mit
der Ansichtsfarbe auf der Fensterfläche. Unter KritaNeutral sind das **4,22 : 1
statt heute 4,74 : 1**, und damit erstmals unter dem Mindestwert 4,5 : 1.
Dieselbe Lage gilt für `Iridescent-round` und `cachyos-emerald`.

**Das ist keine Fehlkonstruktion, sondern eine Folge, die niemand
aufgeschrieben hat.** Sie gehört in die Grenze aus AK 6 und in SPEC 3.1 — dort
steht heute die Zahl 4,74 : 1 als Begründung für `WindowText`, und diese
Begründung fällt mit AK 3.

### F13 — Der Zwischenzustand ohne AK 3 ist unbedenklich

Gerechnet über dieselben 19 Schemata: `WindowText` auf der **Feldfläche** ergibt
schlechtestens **5,70 : 1**, kein Schema fällt unter 4,5 : 1. Würde AK 3 zu
einem eigenen Issue, entstünde also kein Zwischenstand, der schlechter läge als
heute. Das ist die Zahl, an der die Frage aus Feld 6 hängt — sie fällt gegen
die naheliegende Vermutung aus.

---

## 3. AK-Urteil aus dieser Messung: **ready — nein**

Das verbindliche Urteil fällt nach beiden Messungen; hier steht das Urteil aus
Messung B. Es hängt an **einem** Kriterium.

**Vier Kriterien tragen.** AK 1, AK 2, AK 4 und AK 5 sichern je etwas
Prüfbares zu, benennen die Quelle statt einer Farbe und haben in Feld 4 ein
Prüfmittel. AK 2 und AK 4 sprechen über Hülle, Kontur und Schatten und nennen
das Sitzungsbild ausdrücklich als Belegform — der dritte Satz der Definition of
Ready ist damit erfüllt, und zwar an der Stelle, an der die Kriterien
**formuliert** werden.

**AK 3 trägt mit einer Ergänzung.** Das Kriterium selbst ist prüfbar (die
Herkunft der Farbe ist messbar, siehe Feld 4). Was fehlt, ist die Folge aus
F12: Unter drei Themes fällt der Notiztext auf die Fensterfläche zurück und der
schlechteste Fall sinkt von 4,74 : 1 auf 4,22 : 1. Das ist eine Änderung an der
Lesbarkeit, und der Kunde hat für Sprint 9 „Lesbarkeit geht vor" festgelegt. Sie
gehört benannt, bevor gezogen wird — als Satz im Kriterium, nicht als Fund im
Review.

**AK 6 trägt nicht.** Zwei gemessene Gründe:

1. *Der Wortlaut trifft die falsche Menge.* „Die Themes, die die Grafik
   zeichnen" sind gemessen **alle elf**, die fünf schwachen eingeschlossen
   (F4). Ein Kriterium, das die Abhebung unter genau den Themes zusichert, unter
   denen es sie im selben Satz für unmöglich erklärt, ist nicht prüfbar — es
   ist nicht falsch, es ist nicht entscheidbar.
2. *Die zugesicherte Größe hat keinen Messwert.* Die Abhebung schwankt unter
   `default` je nach Grund zwischen 1,08 : 1 und 1,88 : 1 (F3), weil die Hülle
   zu 216/255 deckt. Ein Kriterium, das eine Abhebung zusichert, braucht ein
   Prüfmittel für die Abhebung; es gibt keines, das nicht zugleich den
   Bildschirmhintergrund festschriebe.

Der zweite Halbsatz von AK 6 — die Bedingung gehört in SPEC 3.1 — ist
dagegen sauber prüfbar und soll bleiben.

**Was das „nein" heilt** (Sache des PO, nicht meine): AK 6 auf das umschreiben,
was die Zeichnung an ihrer Grenzstelle selbst sagt — zugesichert ist die
**Herkunft**, benannt ist die Grenze, und die Grenze wird an der **Deckung der
Grafik** festgemacht (255 gegen 15, gemessen) statt an einer Kontrastzahl. Dann
ist AK 6 mit demselben Prüfmittel wie AK 1 zu führen. Dazu die Folge aus F12
in AK 3 oder AK 6 aufnehmen.

Zwei weitere Punkte der Definition of Ready, geprüft und **nicht** beanstandet:

- *Selbstdeklarierte offene Punkte:* Das Issue führt einen — „Ob AK 3 in dieser
  Story bleibt oder ein eigenes Issue wird". Er ist der Vorprüfung ausdrücklich
  vorgelegt und wird in Feld 6 beantwortet; damit ist er geschlossen, sobald der
  PO die Antwort ins Issue übernimmt. Er trägt das „nein" oben **nicht**.
- *Dateinamen als Prüfmittel:* Die im Issue und in der Zeichnung genannten
  Belegorte sind mit `git ls-files` bestätigt —
  `docs/scrum/reviews/2026-08-06-lesbarkeit/` (24 Dateien) und
  `docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/echtelage.cpp`.

---

## 4. Prüfmittel

Je Kriterium: womit der Nachweis geführt wird. Die Belegform aus dem Issue —
offscreen unter mindestens zwei Farbschemata und zwei Desktop-Themes für
AK 1/AK 3/AK 5, Sitzungsbild für AK 2/AK 4 — ist übernommen und im Einzelnen
geschärft.

**AK 1 (Herkunft).** Prüfsatz in `capturetest.cpp`: die vom Fenster gezeichnete
Feldfläche gegen eine **zweite Darstellung derselben Grafik** aus einer eigenen
`KSvg::ImageSet` desselben Themes, Bildpunkt für Bildpunkt — dieselbe Bauart wie
`paintsTheThemesOwnHullInOnePiece()` sie heute für die Hülle führt. Kein
Vergleich gegen eine Farbe. Zwei Themes, gewählt nach eigener
`lineedit`-Grafik (F5), nicht nach Hüllenrand. *Gegenprobe im selben Lauf:* Die
gezeichnete Farbe darf nicht die Palettenfarbe sein, sonst hielte der Prüfsatz
auch für ein Feld, das aus der Palette gefüllt wird.

**AK 2 (Hülle unberührt).** Zwei Teile, weil zwei Belegformen nötig sind.
*Offscreen:* Randmaß und Eckform über `cornerRun()` und `faultOfEdgeWalk()` vor
und nach Einführung des Feldes — die Werkzeuge stehen in `capturetest.cpp:75–96`.
*Sitzungsbild (B21):* Kontur und Schatten belegt kein offscreen erzeugtes Bild;
dafür der Lauf aus `echtelage.cpp`, mit `QT_SCALE_FACTOR` auf der Einstellung
des Kunden. **Der Schatten ist auch dort nur mittelbar zu belegen** — er liegt
außerhalb des Widgets, `QWidget::grab()` sieht ihn nie; die benannte Zusicherung
ist `CaptureWindow::shadow()` samt Kachelvergleich, wie in
`bindsAShadowFromTheThemeTiles()`.

**AK 3 (Text auf dem Feld).** Zwei Wege, beide vorhanden:
*Theme mit eigener `colors`-Datei* — die Notiztextfarbe gegen
`[Colors:View] ForegroundNormal` des mitgelieferten breiten Prüf-Themes
(`#00cc00`, gemessen). *Theme ohne* — die Notiztextfarbe gegen `QPalette::Text`,
und im selben Lauf die Gegenprobe, dass sie **nicht** `QPalette::WindowText` ist;
das vorhandene Muster in `noteTextUsesTheWindowTextRole()` setzt beide Rollen
verschieden und ist genau dafür gebaut. Der Kreuzvergleich der beiden Farbwege
(F7) wird entweder mitgezogen oder ausdrücklich gestrichen.

**AK 4 (Theme-Wechsel).** Offscreen an **einem stehenden Fenster**, hin und
zurück — die Bauart von `textColoursFollowADesktopThemeChange()`. „Hin und
zurück" ist nicht Sorgfalt, sondern die einzige Form, die eine einmal gesetzte
und nie geräumte Farbe von einer folgenden unterscheidet. Dazu das Sitzungsbild
nach B21, weil das Kriterium über Farben aus der Theme-Grafik spricht.

**AK 5 (Innenabstände).** Am gelegten Fenster gemessen, nicht an den
Abstandswerten: die Textposition innerhalb des Feldrahmens gegen den Rand, den
`getMargins()` des Vorsatzes `base` meldet — **relativ und mit
`qFuzzyCompare`**, wegen F9. Bei zwei Fenstergrößen (fünf und acht Zeilen), wie
DoD 1 es für Aussagen über Raumaufteilung verlangt.

**AK 6 (Grenze).** In der vorliegenden Fassung: **kein Prüfmittel** — das ist
der Grund für das „nein" in Feld 3. In der in Feld 3 vorgeschlagenen Fassung:
die Deckung der Grafik je Theme, `qAlpha()` der `framePixmap()` (gemessen 255
gegen 15), plus ein Textabgleich, dass die Bedingung in SPEC 3.1 steht.

**Was ein Agent nicht prüfen kann**

1. **Ob der Kunde das Feld sieht.** Die Abhebung hängt am Grund hinter der
   teildeckenden Hülle (F3) und damit am Bildschirmhintergrund. Kein Lauf und
   kein Bild aus einem Testaufbau entscheidet das; ein Bild aus der Sitzung des
   Kunden zeigt einen Fall, nicht die Zusicherung.
2. **Schatten und Kontur offscreen.** Unverändert seit #55/#83; hier nur
   insoweit betroffen, als AK 2 sie unberührt behauptet.
3. **Fokuswechsel unter Wayland.** Kein Agent holt sich den Fokus zurück
   (Sprint 6, M-B1). Für diese Story ohne Belang — keines der sechs Kriterien
   verlangt einen Fensterwechsel.
4. **Der Lauf des CI-Servers deckt AK 1 nur zur Hälfte ab**, solange die
   Theme-Wahl nach Hüllenrand geht (F5). Das ist behebbar und deshalb hier
   genannt, nicht als Grenze, sondern als Arbeit.

---

## 5. Größenklasse: **`size:m`**

**Wofür `m` und nicht `s`:** Die Story fasst fünf Stellen in
`capturewindow.cpp` an (Konstruktor, `reloadDesktopTheme`, `paintEvent`,
`resizeHull`, `applyTextColours`, dazu `adjustHeight`), schreibt **sieben**
bestehende Prüfsätze um und legt **fünf** neue an, verlangt einen Prüflauf in
der angemeldeten Sitzung und zieht drei Spiegelstriche in SPEC 3.1 nach. Das
ist kein Vorbeigang.

**Wofür `m` und nicht `l`:** Der Mechanismus steht seit #83 vollständig — eine
`ImageSet`, eine Schleife über die Rahmen, ein `paintEvent`, ein
Bildpunktverhältnis. Der neue Rahmen ist der dritte an derselben Kette und
gemessen ohne eigene Entscheidungen zu haben: kein `colorSet` nötig (F8), Rand
einheitlich 6 px (F9), Theme-Wechsel über den vorhandenen Weg (F2). Es entsteht
keine neue Abhängigkeit, kein neues Build-Ziel, kein neuer Prüfweg. Zum
Vergleich trug #83 drei Anmeldungen beim Fenstersystem, neun Kriterien, ein
neues Prüf-Theme und die Erstbegehung dieser Grafikquelle; dort war `l`
begründet, hier ist es das nicht.

**Was die Klasse kippen würde:** ein zusätzliches mitgeliefertes Prüf-Theme mit
eigener `lineedit`-Grafik (Feld 6, Frage 3). Das bliebe `m`. Nach oben kippt
die Story nur, wenn der Kunde die Grenze aus AK 6 nicht hinnehmen will und eine
Lesbarkeitszusicherung unter den fünf schwachen Themes verlangt — das wäre eine
andere Story und nach #83 nicht ohne Aufgabe der Entscheidung „Form und Farbe
kommen vom Theme" zu haben.

---

## 6. Offene Fragen

**Frage 1 (an den PO, blockierend): AK 6 in der vorliegenden Fassung.**
Siehe Feld 3. Solange das Kriterium eine Abhebung zusichert, hat es kein
Prüfmittel und die Story ist nicht ziehbar. Vorschlag steht in Feld 3; die
Entscheidung liegt beim PO.

**Frage 2 (an den PO): Die Folge aus F12 gehört in ein Kriterium.**
Unter `CachyOS-Nord-round`, `Iridescent-round` und `cachyos-emerald` sinkt der
schlechteste Fall des Notiztextes von 4,74 : 1 auf 4,22 : 1 und damit unter den
Mindestwert. Das ist gemessen, es ist eine Folge der Kundenentscheidung vom
06.08.2026, und es steht heute nirgends. Zwei Wege: als benannte Grenze in AK 6
mitführen, oder dem Kunden vorlegen — er hat für Sprint 9 „Lesbarkeit geht vor"
festgelegt, und diese Zahl geht in die Gegenrichtung. **Ich halte das Vorlegen
für richtig**, weil die Zahl eine Kundenfestlegung berührt und nicht nur eine
Prüfform.

**Frage 3 (an den PO, entscheidbar ohne Kunden): ein mitgeliefertes Prüf-Theme
mit eigener `lineedit`-Grafik?**
Auf dieser Maschine und auf dem CI-Läufer sind beide Hälften der Belegform aus
installierten Themes zu haben (`default` mit eigener Grafik, `breeze-light`
ohne). Ein mitgeliefertes Theme wäre die Absicherung gegen einen Bauplatz ohne
`libplasma`. Die Testsuite führt diese Doppelung heute bewusst
(`desktopthemes.h`: „Neither replaces the other"). **Ich empfehle, darauf zu
verzichten** — der CI-Lauf installiert `libplasma` ausdrücklich zu diesem
Zweck, und ein viertes Prüf-Theme kostet mehr Pflege, als es hier belegt.

**Frage 4 (die dem Bericht ausdrücklich vorgelegte): AK 3 in dieser Story oder
eigenes Issue?**
**Antwort: in dieser Story.** Zwei Gründe, einer davon gegen meine erste
Vermutung:

- Der Sicherheitsgrund trägt **nicht**. Ich hatte erwartet, ein Zwischenstand
  ohne AK 3 sei schlecht lesbar; gemessen liegt `WindowText` auf der Feldfläche
  schlechtestens bei 5,70 : 1, kein Schema unter 4,5 : 1 (F13). Ein eigenes
  Issue erzeugte also **keinen** schlechten Zwischenstand.
- Der Arbeitsgrund trägt. AK 3 und die Feldfläche fassen dieselbe Funktion an
  (`applyTextColours()`), dieselbe Struktur (`ThemeTextColours`, F7) und
  **dieselben fünf Prüfsätze**, die ohnehin schon durch das Feld brechen
  (F1). Getrennt würden diese fünf zweimal umgeschrieben, und SPEC 3.1 zweimal
  nachgezogen. Die Größenklasse bleibt mit AK 3 bei `m`.

**Frage 5 (an den PO, klein, aber vor dem Ziehen zu klären): Wo greifen die
6 px aus AK 5?**
Der Wortlaut („die 12/10/8 px gelten zuzüglich des Randes … genau wie sie schon
zuzüglich des Hüllenrandes gelten") vergleicht mit dem **Layout-Rand des
gesamten Inhalts**, der Halbsatz „Der Text rückt nach innen" und die Zeichnung
meinen den Rand **um das Textfeld**. Nur die zweite Lesart passt zur
Zeichnung — nach der ersten rückten App-Name und Fußzeile mit ein, was das Bild
nicht zeigt. Ein Satz im Kriterium erspart dem Strang die Rückfrage; F10 sagt,
woran die Wahl im Bau hängt.

---

## Reproduktion

```
cmake -B docs/scrum/vorberichte/100-eingabefeld/build-b \
      -S docs/scrum/vorberichte/100-eingabefeld/sonden-b -DCMAKE_BUILD_TYPE=Debug
cmake --build docs/scrum/vorberichte/100-eingabefeld/build-b
cd docs/scrum/vorberichte/100-eingabefeld
QT_QPA_PLATFORM=offscreen ./build-b/feldsonde   $PWD/../../../../tests/themes   # M1, M4, M5
QT_QPA_PLATFORM=offscreen ./build-b/schichtsonde $PWD/../../../../tests/themes  # M2
QT_QPA_PLATFORM=offscreen ./build-b/alphasonde                                  # M6
QT_QPA_PLATFORM=offscreen ./build-b/paarsonde                                   # M4 Themepaar
```

Die Geometriesonde (`m5-fenstergeometrie.txt`) braucht das Fenster selbst und
ist gegen einen eigenen Projektbau gelinkt; der Aufruf steht als Kommentar in
`sonden-b/geometriesonde.cpp`. Die Kontrastrechnung über die Farbschemata
(`m3-schriftgrund-18-schemata.txt`) ist ein Python-Lauf über
`/usr/share/color-schemes/*.colors` und `~/.local/share/color-schemes/*.colors`.

Die Bauverzeichnisse sind von `.gitignore` (`build-*/`) erfasst und liegen
bewusst nicht im Repository; Quelltexte und Ausgaben liegen darin.
