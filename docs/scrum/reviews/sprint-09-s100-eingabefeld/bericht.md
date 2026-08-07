# Übergabebericht #100 — Eingabefeld aus der Theme-Grafik

**Story:** Issue #100, „Erfassungsfenster: Der Eingabebereich ist nicht als
solcher erkennbar" (Kundenbefund 05.08.2026)
**Strang A, Sprint 9.** Zweig `story/100-eingabefeld`, Basis `366b69f`.
**Abgabe:** 07.08.2026, Bearbeiter `denkzettel-dev`.

Geurteilt wird gegen die **korrigierte** Fassung der Akzeptanzkriterien
(PO-Kommentare vom 07.08.2026, 18:55 und 19:28 Uhr) und gegen den
Vorprüfbericht `docs/scrum/vorberichte/100-eingabefeld/bericht.md`.

---

## 1. Was gebaut wurde

Das Erfassungsfenster zeichnet eine **zweite Grafik desselben Desktop-Themes**:
`widgets/lineedit` mit dem Vorsatz `base`, aus derselben `KSvg::ImageSet` wie
die Hülle seit #83. Es ist dieselbe Bauart eine Ebene tiefer — und dieselbe
Grafik, aus der KRunners Eingabefeld gezeichnet ist, das der Kunde als Maßstab
genannt hat.

Das Feld liegt auf der Geometrie des Textbereichs, wird **nach** der Hülle und
**vor** `QWidget::paintEvent()` gezeichnet, und der Notiztext rückt um den Rand
der Feldgrafik nach innen. App-Name und Fußzeile stehen unverändert.

**Fünf Stellen, an denen der Bau einer gemessenen Falle ausweicht:**

| | |
|---|---|
| F5 | Der Feldrahmen steht in der Schleife über das **frische** `ImageSet` in `reloadDesktopTheme()`. Ohne das folgte er dem Theme-Wechsel lautlos nicht. |
| F7 | Der Einzug läuft über `documentMargin`. `setViewportMargins()` ist protected, `setContentsMargins()` bewirkt auf einem `QPlainTextEdit` gemessen nichts. `documentMargin` zieht die Höhenrechnung von selbst mit — 174 → 186. |
| F4 | `resizeField()` setzt erst das Bildpunktverhältnis, dann die Größe. Offscreen träfe `DevicePixelRatioChange` nie ein; die Zusicherung dazu läuft deshalb im Prozess bei 1,6. |
| F3 | Kein `setColorSet()` am Feld. Es sähe aus wie das Gegenstück zu `m_hull->setColorSet(Window)` und ist keines: `View` und `Window` rendern bildpunktgleich, die Farbe wählt der Klassenname im SVG. |
| F9 | Kein Prüfsatz vergleicht gegen die Zahl 6. Der Rand wird aus der Grafik gelesen; vier der acht installierten Themes melden 5,99999. |

**Geänderte Dateien** — die Dateimenge aus Feld 1 des Vorberichts, ohne Zugang:

| Datei | was |
|---|---|
| `src/capture/capturewindow.h` | `m_field`, `resizeField()`, `applyFieldMargin()` |
| `src/capture/capturewindow.cpp` | Konstanten, dritter Rahmen, `paintEvent()`, `reloadDesktopTheme()`, `eventFilter()`, `event()`; drei Kommentare nachgezogen (AK 10, B17) |
| `tests/capturetest.cpp` | fünf neue Prüfsätze, drei bestehende umgebaut, vier Hilfsfunktionen |
| `tests/captureshots.cpp` | Kommentar zur Bildreihe (zwei Flächen, und was die Schema-Spalten davon zeigen) |
| `tests/themes/…/denkzettel-test-breit/colors` | **nur der Kommentarkopf** (AK 10) |
| `SPEC.md` | 3.1: zwei Flächen statt einer durchgehenden, Deckungsgrenze, Feldrand bei den Innenabständen |

Kein Eingriff in den Bau, keine neue Abhängigkeit, kein neuer Läufer, keine
neue Fixture — wie im Vorbericht gemessen.

---

## 2. Die Kriterien, einzeln

### AK 1 (F1) — Herkunft · **erfüllt**

`paintsTheThemesFieldOntoTheHull()`. Das gezeichnete Fenster wird gegen eine
**zweite Ausfertigung** beider Grafiken gehalten, zusammengesetzt wie
`paintEvent()` sie zusammensetzt — unter zwei Desktop-Themes und an drei
Punkten (Feldfläche, Feldkante, Hülle daneben), dazu der ganze Randring des
Feldes Bildpunkt für Bildpunkt.

Drei Gegenproben im selben Lauf, ohne die alle Vergleiche auch für ein Fenster
**ohne** Feld gälten (die Vorlage wäre dann auf dieselbe Weise falsch wie das
Bild): Die Feldfläche unterscheidet sich von der Hülle daneben; die Kante
unterscheidet sich von der Fläche; und unter `denkzettel-test-breit` trägt die
Fläche 51,34,17 — eine Farbe, die kein Farbschema führt, sondern die
`colors`-Datei jenes Themes.

*Mutationsprobe 1* (Feld gar nicht zeichnen): rot.

### AK 2 (F2) — die Hülle bleibt unberührt · **erfüllt, mit Sitzungsbild**

Die Zusicherungen von #83 laufen unverändert weiter und sind grün:
`hullHasNoStairAtTheCorner()`, `squareThemeKeepsSquareCorners()`,
`hullIsCompleteAtFiveAndEightLines()`, `hullFollowsTheWindowPixelRatio()`,
`hullHoldsAtTheCustomersScale()`, `bindsAShadowFromTheThemeTiles()`.

**Der Beleg, auf den es ankommt, ist das Sitzungsbild** (B21), und er ist
schärfer als eine Zusicherung: In der angemeldeten Sitzung unter `default`
misst dieser Stand

```
Eckenlauf 4   Kantenlauf 7.4.3.2.1.1.1.0.0.0   Schatten vom Compositor angenommen
```

— **Ziffer für Ziffer dieselben Werte, die #83 in derselben Sitzung gemessen
hat** (`docs/scrum/reviews/sprint-07-s83-native-huelle/messungen/`
`m5-fensterlage-sitzung.txt`). Bildpunktverhältnis 1,6, die Einstellung des
Kunden, ungesetzt vorgefunden statt eingestellt (F10).

Geändert hat sich allein die Höhe: 174 → 186 logische Bildpunkte. Das ist der
Feldrand zweimal und von AK 5 gewollt.

Belege: `messungen/m5-sitzungsbild.txt`, `bilder/sitzung/`.

### AK 4 (F4) — das Feld folgt dem Theme-Wechsel · **erfüllt, mit Sitzungsbild**

`fieldFollowsADesktopThemeChange()` wechselt am **stehenden** Fenster hin und
zurück — zurück, weil eine Farbe, die sich nur einmal bewegt, auch von einer
einmal gesetzten und nie geräumten erklärt würde. Zugesichert ist zusätzlich,
dass die angekommene Farbe die des neuen Themes ist und nicht irgendeine.

In der Sitzung dasselbe über drei Aufnahmen: `default` → `breeze-light` →
`default`, Feldfläche 20,22,24 → 255,255,255 → 20,22,24, Feldkante 76,78,81 →
198,200,201 → 76,78,81.

*Mutationsprobe 2* (Feldrahmen aus der Schleife über das frische `ImageSet`):
rot. Das ist die Falle aus #83, und sie ist gedeckt.

### AK 5 (F5) — Innenabstände · **erfüllt**

`textSitsInsideTheFieldBorder()`, unter zwei Themes und an beiden Größen aus
SPEC 3 (DoD 1). Geprüft wird gegen den Rand, den die **Grafik** meldet, nie
gegen die Zahl 6:

- `documentMargin` ≥ Feldrand, und der Cursor beginnt entsprechend weiter innen;
- die fünf beziehungsweise acht Zeilen haben ihren Platz behalten — der
  Textbereich ist um den Rand gewachsen, statt Raum für Text zu verlieren;
- **App-Name und Fußzeile stehen unverändert** am Innenrand des Layouts. Das
  ist die Hälfte, die sonst ungeprüft mitliefe.

Gemessen am Fenster: 174 → 186, Textbereich 98 → 110 — genau die Zahlen, die
der Vorbericht vorhergesagt hat.

*Mutationsprobe 3* (`applyFieldMargin()` entfällt): rot.

### AK 6a — Herkunft · **erfüllt**

Dasselbe Prüfmittel wie AK 1, keine eigene Zusicherung.

### AK 6b — die Grenze an der Deckung · **erfüllt**

`fieldCoverageIsTheThemesOwn()`. Die Themes werden **nach gemessener Deckung**
gewählt, nie nach Namen — dass ein Theme eine eigene Grafik mitbringt, ist
durch KSvg gar nicht zu beobachten (F1, im Protokoll nachgefahren: auch
`kein-solches-theme` löst auf, mit Rand 6 und der Grafik von `default`).

Gemessen (`messungen/m1-feldgrafik-je-theme.txt`), Auswahlpfad `opaque`:

| Deckungsklasse | Themes | Fläche |
|---|---|---|
| **255** | `default`, `breeze-dark`, die drei mitgelieferten | 20,22,24 bzw. 51,34,17 |
| **255** | `breeze-light` | 255,255,255 |
| **15** | `CachyOS-Nord-round`, `Iridescent-round`, `cachyos-emerald` ×3 | 0,0,0 |

Der Prüfsatz zeigt dazu, dass das Fenster in **beiden** Klassen zeichnet, was
die Grafik gibt — dort, wo sie einen Hauch gibt, einen Hauch. Das Bild
`bilder/offscreen/07-rand-breit-hell-leer.png` unter `CachyOS-Nord-round` zeigt
die Grenze, wie sie aussieht: Das Feld ist da und praktisch unsichtbar.

**Grenze der Prüfbarkeit, ausgesprochen:** Die fünf schwachen Themes sind
CachyOS-Pakete. Auf dem öffentlichen Läufer liegen nur `default` und `breeze-*`,
dort deckt jede Grafik 255 — der Prüfsatz überspringt sich dann mit genau dieser
Begründung (`QSKIP`).

**SPEC 3.1 zieht nach** (DoD 4/B9): Die Deckungsgrenze steht dort, und der
Textnachweis liegt in `messungen/m6-spec-nachweis.txt`.

### AK 8 — die Wache gegen ein fehlendes Desktop-Theme · **erfüllt, mit benannter Grenze**

Das Feld trägt dieselbe `isValid()`-Wache wie die Hülle seit #83, und
`staysUsableWithoutADesktopTheme()` prüft im getrennten Prozess zusätzlich die
Mitte des Textbereichs und die Feldkante gegen die Palettenfarbe.

**Was daran nicht prüfbar ist, und ich sage es lieber, als es grün aussehen zu
lassen:** Der Fall „Hülle löst auf, Feld nicht" ist nicht herstellbar. KSvg
fällt je Bild auf `default` zurück, also lösen entweder beide auf oder keins;
und wo keins auflöst, kehrt `paintEvent()` schon an der Hüllenwache um, bevor
das Feld an die Reihe kommt. Die Feldwache ist damit eine Zeile, deren
Ausfall auf dieser Bauart keinen Prüfsatz rot färbt. Die beiden neuen
Zusicherungen prüfen die Lage, für die AK 8 geschrieben ist — kein Feld auf
einem Fenster ohne Theme —, und sie prüfen sie an der Stelle, an der ein Feld
stünde.

### AK 9 — die zwei Abgriffe · **erfüllt, und hier trägt allein die Mutationsprobe**

Beide Abgriffe liegen jetzt in der Lücke **über** dem Textbereich, wo Hülle
steht und sonst nichts. Beide Prüfsätze sagen im Kommentar, gegen welche Fläche
sie messen.

**Der Nachweis besteht aus zwei Läufen, und der zweite ist der wichtige:**

| Probe | Eingriff | Ergebnis |
|---|---|---|
| **4** | Auswahlpfad `opaque` entfällt, **reparierter** Abgriff | **rot** — `takesTheOpaqueVariantWithoutABlurringCompositor()` `drawn == opaqueAlpha` |
| **5** | derselbe Wegfall, **alter** Abgriff (Fenstermitte) | **grün geblieben** |

Probe 5 ist der Befund des Vorberichts am laufenden Code: Hätte der Abgriff in
der Fenstermitte bleiben dürfen, wäre die Mutation, gegen die dieser Prüfsatz
gebaut wurde, unentdeckt durchgelaufen — und der Lauf hätte ausgesehen wie
jeder andere.

**Der zweite Abgriff hat mich einen Prüfsatz gekostet, und das gehört hierher.**
`hullIsCompleteAtFiveAndEightLines()` vergleicht **Deckung**. Feld und Hülle
decken unter jedem Theme, das ein Lauf dieses Projekts erreicht, beide 255 —
ein Feld über das ganze Fenster gelegt (*Probe 6*) ließ den Prüfsatz zunächst
**grün**. Die Verschiebung des Punktes war also richtig und **nicht belegbar**.
Deshalb steht dort jetzt zusätzlich ein Vergleich der **Farbe** gegen die
Hülle; damit fällt Probe 6 auf ihn, und „misst die Hülle" ist eine messbare
Aussage geworden statt einer über die Lage eines Punktes.

*Dasselbe noch einmal bei F4:* Probe 7 (Feld ohne Bildpunktverhältnis) blieb
zunächst grün — drei Abgriffe in flächigen Bereichen sehen bei 1 und bei 1,6
gleich aus. Erst der Vergleich des ganzen Randrings fängt sie. Beide Male hat
die Probe den Prüfsatz verbessert und nicht bestätigt.

### AK 10 — zwei Begründungen, die falsch werden · **nachgezogen (B17)**

- `tests/themes/…/denkzettel-test-breit/colors`: Der Kommentarkopf sagte,
  `[Colors:View]` bestehe „für die Mutationsprobe und für nichts sonst" und die
  Werte färbten „die Grafik daneben nicht". Beides ist nachgezogen: Die Gruppe
  färbt die zurückgefallene `lineedit`-Grafik, sie trägt jetzt zwei Zwecke, und
  wer ihre Werte ändert, verschiebt auch das, was die Feldprüfsätze messen. Der
  Hinweis auf `WideThemeFieldColour` steht dabei. **Die Werte sind unverändert.**
- `src/capture/capturewindow.cpp`: die beiden Stellen zur „durchgehenden
  Fläche" (`viewport()->setAutoFillBackground(false)` und die Begründung der
  Textrolle in `applyTextColours()`).

**Dritte Fundstelle, von mir gefunden und mitgezogen:** `bindWindowEffects()`
belegte die Aussage „die Region wird in logischen Bildpunkten übergeben" mit
„gemessen 600x174". Die Zahl ist jetzt 186. Was die Messung zeigen soll, ist
das Wort *logisch*, und das ist unverändert — der Beleg dafür war es nicht.

Der Ausschluss-Griff aus `CLAUDE.md` ist gefahren (vier einschlägige Zeilen);
`wireframes/` ist ausdrücklich nicht meine Fläche und mit `36156fd` bereits
nachgezogen.

---

## 3. Testnachweis

```
$ QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde ctest --test-dir build
100% tests passed out of 9

capturetest: 37 passed, 0 failed, 0 skipped
```

Vollständig in `messungen/m3-testlauf.txt`. `takesTheOpaqueVariant…` läuft dort
durch (nicht übersprungen); `fieldCoverageIsTheThemesOwn()` läuft auf dieser
Maschine durch und überspringt sich auf dem öffentlichen Läufer.

**Linter, mit Zahl der angefassten Dateien** — eine Zahl kleiner als 30 wäre
hier ein Abbruchgrund und kein Nebenbefund:

```
lint-tidy : rc=0, 30 Dateien, 0 Befunde
lint-clazy: rc=0, 30 Dateien, 0 Befunde
```

**Bau:** warnungsfrei.

**Neue Prüfsätze:** `paintsTheThemesFieldOntoTheHull`,
`fieldColoursComeFromTheThemeBeforeTheScheme`, `fieldFollowsADesktopThemeChange`,
`textSitsInsideTheFieldBorder`, `fieldCoverageIsTheThemesOwn`.
**Umgebaut:** `paintsTheThemesOwnHullInOnePiece`,
`hullIsCompleteAtFiveAndEightLines`,
`takesTheOpaqueVariantWithoutABlurringCompositor`,
`staysUsableWithoutADesktopTheme`, `hullHoldsAtTheCustomersScale`,
`noteTextUsesTheWindowTextRole` (nur Kommentar).

**Belegform je Achse:** zwei Desktop-Themes in jedem Feldprüfsatz; zwei
Farbschemata in `fieldColoursComeFromTheThemeBeforeTheScheme()`, das ein
eigenes Schema in einem eigenen Prozess mit eigenem `HOME` schreibt.

---

## 4. Zwei Dinge, die beim Bauen herauskamen (DoD 4 / B9)

**Die Feldgrafik folgt `kdeglobals`, nicht `qApp->palette()`.** Gemessen: Unter
BreezeLight zeichnet `default` die Feldfläche 255,255,255, unter BreezeDark
20,22,24 — dieselbe Grafik, dieselbe Binärdatei. Eine im laufenden Prozess
gesetzte Palette bewegt sie **nicht**. Zwei Folgen:

1. Die zweite Farbschema-Achse ist nur in einem eigenen Prozess zu prüfen; der
   Prüfsatz dazu schreibt sich sein Schema selbst und bekommt ein eigenes
   `HOME`, damit es nicht in `~/.qttest` liegenbleibt und spätere Läufe färbt.
2. Die Spalten „hell" und „dunkel" der Bildreihe `captureshots` zeigen vom Feld
   **denselben** Farbwert. Das gilt seit #83 schon für die Hülle; das Feld
   schließt sich an. Der Kommentar der Bildreihe sagt es jetzt.

**Zwei lebende `ImageSet` desselben Themenamens teilen ihre Auswahlpfade** —
der bekannte Fall, hier zum zweiten Mal aufgetreten: In der Sitzung liefern die
Zeilen „Fassung durchscheinend" und „Fassung deckend" dieselben Zahlen, weil
das lebende Set des Fensters die Auswahl vorgibt. Für den Vergleich
„Gezeichnet == Fassung" ist das ohne Folgen (beide Seiten tragen die Fassung,
die das Fenster fährt); als Vergleich **zweier Fassungen** ist die Zeile in der
Sitzung nicht zu lesen. Offscreen ist sie es, und dort steht sie auch.

---

## 5. Was ich nicht prüfen konnte

- **Den Schatten selbst.** Belegt ist, dass der Compositor ihn annimmt
  (`window.shadow()` in der Sitzung) — im Bild ist er nicht, weil `grab()` nur
  das Widget zeichnet.
- **Den Befund des Kunden.** Ob der Eingabebereich *erkennbar* ist, entscheidet
  er. Messbar ist, dass sich das Feld abhebt: in der Sitzung unter `default`
  Feldfläche 20,22,24 gegen Hülle 32,35,38 — und die Bilder liegen bei.
- **Die Deckungsgrenze auf dem öffentlichen Läufer** (siehe AK 6b).
- **AK 8 in seiner Mutationsform** (siehe dort).

---

## 6. Melden, nicht heilen

Nichts außerhalb meiner Fläche angefasst. Zwei Beobachtungen für den PO:

- **`installedThemePair()` wählt für die Bildreihe `breeze-dark` und
  `CachyOS-Nord-round`** — also gerade ein Theme aus jeder Deckungsklasse. Das
  ist ein Glücksfall dieser Maschine und keine Regel: Der Wähler sortiert nach
  **Hüllenrand**. Die Bildreihe zeigt die Grenze aus AK 6b deshalb heute und
  anderswo womöglich nicht. Der Vorbericht hat den Wähler ausdrücklich
  unangetastet gelassen (Feld 6, Frage 3); ich lasse ihn ebenso.
- **Zeichnung 4b, Zeile „Innenabstand"** sagt weiterhin „12/10/8 zuzüglich des
  Randes, den das Theme vorgibt" ohne den Feldrand — die Prüfsätze F5 daneben
  nennen ihn. `wireframes/` ist nicht meine Fläche, und ein Beleg wird geankert
  statt nachgezogen; ich melde es, damit die Entscheidung beim PO liegt.

---

## 7. Nachtrag 07.08.2026 — der öffentliche Lauf war rot, und warum

Lauf `31216657864` auf `70902a4`: `capturetest` **28 passed, 6 failed,
3 skipped**. Eine Wurzel, und sie ist meine.

**Die Ursache.** Auf dem Läufer liegt `widgets/lineedit` nirgends. `ksvg` hängt
nicht von `libplasma` ab, ein Bauwirt mit nur den KF6-Teilen dieses Projekts hat
also keine Plasma-Grafiken — und die mitgelieferten Prüf-Themes bringen allein
`dialogs/background` mit. Damit fehlt auch das, worauf KSvg zurückfiele. Meine
Falle F1 („jeder Themename löst auf") gilt nur, **solange die
`default`-Grafiken installiert sind**; diesen Halbsatz hatte ich nicht
mitgedacht.

**Der peinlichste der sechs war `fieldCoverageIsTheThemesOwn()`.** Er hat die
Lage vollständig erkannt und im Fehlertext ausgesprochen — und ist trotzdem
gefallen, weil das `QVERIFY2` **vor** dem `QSKIP` stand. Ich hatte den Skip
gemeldet; im Prüfsatz stand er an der falschen Stelle. Eine Vorbedingung wird
zuerst gefragt, sonst ist sie keine.

**Nachgestellt statt geraten.** `XDG_DATA_DIRS=/nonexistent` liefert auf dieser
Maschine **28 passed, 6 failed, 3 skipped** — dieselben Zahlen, dieselben sechs
Prüfsätze, dieselben drei Übersprungenen wie auf dem Läufer. Das ist zugleich
die Antwort auf die Frage, ob dort noch etwas anderes im Spiel ist: nein.

**Die Reparatur.** Ein Wächter, `whyNoFieldGraphic(theme, kriterium)`, gefragt
in `paintsTheThemesFieldOntoTheHull()`,
`fieldColoursComeFromTheThemeBeforeTheScheme()`,
`fieldFollowsADesktopThemeChange()`, `textSitsInsideTheFieldBorder()` und —
in seiner Deckungsform — in `fieldCoverageIsTheThemesOwn()`.

Er hängt an der **gemessenen Ursache**: löst `widgets/lineedit` auf, gibt es den
Vorsatz `base`, meldet die Grafik einen Rand? Das ist dieselbe Frage, die
`paintEvent()` stellt, bevor es zeichnet. **Nicht** an `CI=true` und nicht an
einer Variablen von uns — eine solche Bedingung legte den Prüfsatz überall dort
still, wo jemand die Variable setzt, und ein stillgelegter Prüfsatz liest sich
wie ein grüner.

Jeder Skip-Text nennt beide Hälften: welche Voraussetzung fehlt (mit den
gemessenen Werten `isValid`, Vorsatz, Rand) und welches Kriterium damit
ungeprüft bleibt — dazu den Satz, dass die Lage „kein Theme" selbst nicht
ungeprüft ist, sondern von `staysUsableWithoutADesktopTheme()` getragen wird.

**Der Gegenversuch, beide Richtungen, in `mutationsproben.sh`:**

| Probe | Lage | Ergebnis |
|---|---|---|
| **8** | ohne Grafik, unmutiert | **29 passed, 0 failed, 8 skipped** — jeder Skip mit Grund |
| **9** | ohne Grafik, mit Mutation 1 | grün, 8 übersprungen — die Kehrseite, ausgesprochen: wo nichts zu messen ist, fängt der Läufer diese Mutation nicht |
| **1** | **mit** Grafik, dieselbe Mutation 1 | rot, sechs Prüfsätze fallen |

Und die Zahl, die gegen einen zu breiten Skip zeugt: **auf Ganymed 37 passed,
0 failed, 0 skipped** — der Wächter greift hier bei keinem einzigen Prüfsatz.
Die Proben 1 bis 7 fallen unverändert wie zuvor.

**Zu den beiden Kindprozessen.** Ihre Rückgabe allein sagt nichts, ihr Text
sagt es: Beide Eltern reichen `child.readAll()` in die Fehlermeldung, weshalb
im Läuferprotokoll auch „Compared values are not the same" aus dem Kind stand.
Die Lücke war der **Übersprung**, nicht der Fehlschlag — ein Kind, das alles
überspringt, kehrt mit 0 zurück und der Elternteil meldet grün. Deshalb fragt
`fieldColoursComeFromTheThemeBeforeTheScheme()` die Vorbedingung jetzt im
Elternteil, **bevor** ein Kind startet. Bei `hullHoldsAtTheCustomersScale()`
bleibt das Feld in der Liste: Sein Gegenstand ist die Hülle bei 1,6, und die
Feldzusicherung läuft im selben Lauf auch auf oberster Ebene — die fehlende
Voraussetzung steht dort und nicht nur in der Ausgabe eines Kindes, die
niemand liest, wenn es gelingt.

**Beleg:** `messungen/m7-ohne-plasma-grafik.txt` (der ganze Lauf mit allen
Skip-Texten), `messungen/m2-mutationsproben.txt` Proben 8 und 9.

---

## Reproduktion

```
bash docs/scrum/reviews/sprint-09-s100-eingabefeld/pruefen.sh
```

Fährt M1 bis M6: Feldgrafik je Theme, die sieben Mutationsproben, den
Testlauf, die Bildreihe offscreen auf der Skalierung des Kunden (1,6), das
Sitzungsbild und den Textnachweis in `SPEC.md`. Der Sitzungslauf wird
übersprungen, wenn keine Wayland-Sitzung da ist **oder die Sitzung gesperrt
ist** — bei gesperrter Sitzung fotografiert man den Rollladen und bekommt
Rückgabe 0.

Eigener Bauplatz unter `build/` neben der Datei, von `.gitignore` gedeckt.
Keine Installation nach `/usr`, kein Eingriff in `build/` der
Repositoriumswurzel.
