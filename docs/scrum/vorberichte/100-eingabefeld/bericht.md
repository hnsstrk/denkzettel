# Vorprüfbericht #100 — Eingabefeld aus der Theme-Grafik

**Gegenstand:** Issue #100, „Erfassungsfenster: Der Eingabebereich ist nicht als
solcher erkennbar" (Kundenbefund 05.08.2026)
**Konsolidiert vom Scrum Master am 07.08.2026, 21:23 Uhr** · Quellstand `88129ba`

| | |
|---|---|
| **Messung A** | `denkzettel-ux`, `messung-a.md`, Belege `messungen/`, `sonden/`, `pruefen.sh` |
| **Messung B** | Scrum Master, `messung-b.md`, Belege `messungen-b/`, `sonden-b/` |
| **Nachprüfung** | `messungen-b/m7-nachpruefung-a.txt` — drei Messungen aus A, die in B fehlten oder ihr widersprachen |

**Beide Messungen sind unabhängig entstanden.** Bearbeiter B hat `messung-a.md`
erst nach Abgabe der eigenen Datei und nach Aufhebung der Sperre durch den PO
gelesen.

**Stand der Kriterien:** Dieser Bericht urteilt gegen die **korrigierte** Fassung
der Akzeptanzkriterien (PO-Kommentar vom 07.08.2026, 18:55 Uhr) und gegen die
Entscheidung zur Fokusschicht (19:19 Uhr, #102). Die erste Fassung ist damit
überholt; die Messungen zu ihr bleiben in beiden Einzelberichten stehen und
werden nicht nachgezogen (B17).

**Werkzeugstand** (B17): ksvg 6.28.0, qt6-base 6.11.1, kwindowsystem 6.28.0,
plasma-desktop 6.7.4. Acht installierte Desktop-Themes, **19** Farbschemata (18
unter `/usr/share/color-schemes/`, dazu `Ant-Dark` im Benutzerverzeichnis; A
zählt 18, B 19 — die Differenz ist der Ablageort, nicht die Messung).

---

## 0. Zusammenführung — wo die Messungen auseinandergingen

Vier Stellen. Drei davon sind aufgelöst, eine ist eine Korrektur an einem
Akzeptanzkriterium.

### 0.1 Die Deckung der Hülle — **A hat recht, B hatte unrecht**

B hat die Deckung der Hülle in der Fenstermitte mit 216 von 255 unter `default`
gemessen, A mit 255. Nachgemessen (`m7-nachpruefung-a.txt`, V1): **B hat ohne
den Auswahlpfad `opaque` gemessen.** Den setzt der Bau, sobald nichts
weichzeichnet (`capturewindow.cpp:315`), und `themeHull()` im Test setzt ihn
ebenso — offscreen ist das immer der Fall. Die Zahl ohne den Pfad gehört zu
keinem Zustand, in dem die beiden Prüfsätze je laufen.

| Theme | Hülle ohne `opaque` | Hülle mit `opaque` | Feld | Prüfsatz nach dem Feld |
|---|---|---|---|---|
| `default`, `breeze-dark`, `breeze-light` | 216 | **255** | 255 | 255 gegen 255 → **grün** |
| die drei mitgelieferten Prüf-Themes | 255 | 255 | 255 | 255 gegen 255 → **grün** |
| `CachyOS-Nord-round` | 255 | 255 | 15 | 255 gegen 255 → **grün** |
| `Iridescent-round` | 51 | 51 | 15 | 63 gegen 51 → rot |
| `cachyos-emerald` ×3 | 7 | 9 | 15 | 23 gegen 9 → rot |

**Die Folge ist schwerer als der Fehler.** B hatte gemeldet, die beiden
Prüfsätze würden unter `default` rot. Sie bleiben **grün — überall, wo sie heute
laufen**: auf dieser Maschine reicht `themes::anyInstalledTheme()` gemessen
`CachyOS-Nord-round` (255), und auf dem CI-Läufer gibt es nur die
mitgelieferten Themes, `default` und `breeze-*` (alle 255). Rot würden sie
allein unter `Iridescent-round` und den drei `cachyos-emerald`-Themes, und
keines davon greift ein Lauf heute ab.

Damit wandert der Fund von „ein Test wird rot" zu „ein Test bleibt grün und
misst ab dann das Feld statt der Hülle". Das ist die Fehlerklasse, die dieses
Projekt an einem Abend viermal entlarvt hat. **Nachgemessen bis zur
Mutationsprobe:** `takesTheOpaqueVariantWithoutABlurringCompositor()` sichert
zu, dass der Bau den Auswahlpfad `opaque` setzt — es prüft
`drawn == opaqueAlpha` und `drawn != translucentAlpha`. Liegt das Feld über dem
Abgriff, ist `drawn` gemessen **255, mit und ohne den Auswahlpfad**. Die
Mutation, gegen die dieser Prüfsatz gebaut wurde, liefe danach unentdeckt durch.

**Das berührt AK 9 in seiner Begründung, nicht in seiner Sache.** AK 9 schreibt
heute: „Unter `default` werden sie rot (Feld 255 gegen Hülle 216)." Die Zahl 216
stammt aus Messung B und ist die falsche; der Satz gehört umgekehrt. AK 9 wird
davon nicht schwächer, sondern trägt danach mehr — siehe Feld 6, Frage 5.

### 0.2 Auflösung unter einem unbekannten Themenamen — **A hat gemessen, B nicht; bestätigt**

A: `widgets/lineedit` löst unter **jedem** Namen auf, weil KSvg je Bild auf
`default` zurückfällt. Nachgemessen (V2):

```
kein-solches-theme: isValid=ja  hasElementPrefix(base)=ja  Rand 6/6/6/6
default:            isValid=ja  hasElementPrefix(base)=ja  Rand 6/6/6/6
```

Bestätigt. Zwei Folgen: **AK 6b darf die Grenze nicht an der Frage festmachen,
ob ein Theme eine eigene Grafik mitbringt** — das ist durch KSvg gar nicht zu
beobachten. Die korrigierte Fassung macht sie an der **Deckung** fest und ist
damit richtig gefasst. Und **AK 8 kann nur in dem getrennten Prozess ohne
Theme-Pfad auslösen**, den `staysUsableWithoutADesktopTheme()` bereits fährt;
innerhalb eines gewöhnlichen Laufs ist der Fall nicht herstellbar (Feld 4).

### 0.3 Der Grund, über dem 1,39 : 1 gerechnet wurde — **A hat ihn, B hatte ihn nicht**

B hat gemessen, dass die Abhebung am Bildschirmhintergrund hängt (1,08 : 1 über
Schwarz gegen 1,88 : 1 über Weiß unter `default`), und die Zahl 1,39 : 1 als
„Zahl für einen ungenannten Grund" beanstandet. A nennt den Grund: **mittleres
Grau 128,128,128**. Nachgemessen (V3), über eben diesem Grund:

| Theme | Fläche : Hülle | Kante : Hülle |
|---|---|---|
| `default`, `breeze-dark` | **1,405 : 1** | 1,547 : 1 |
| `breeze-light` | 1,332 : 1 | 1,261 : 1 |
| `CachyOS-Nord-round` | 1,026 : 1 | 1,000 : 1 |
| `Iridescent-round`, `cachyos-emerald` ×3 | 1,095–1,104 : 1 | 1,000 : 1 |

Die Fläche reproduziert die 1,39 : 1 (gemessen 1,405 — der Abstand ist der
Abgriffpunkt). Die Kante weicht ab (1,55 gegen 1,33); die Kante ist 6 px breit,
und welcher ihrer Bildpunkte gelesen wird, verschiebt die Zahl. **Für die
Kriterien ist das ohne Belang**, weil die korrigierte Fassung keine Kontrastzahl
mehr zusichert. Es gehört trotzdem hierher: Eine Kontrastzahl gegen eine
teildeckende Fläche braucht Grund **und** Abgriffpunkt, sonst ist sie nicht
reproduzierbar.

Die Werte der fünf schwachen Themes (1,03–1,10 : 1) sind über diesem Grund
bestätigt.

### 0.4 Die Kollisionsfläche — **A hat sie gemessen, mit AK 3 fällt sie weg**

A hat gemessen, dass drei der vier Sprint-9-Kandidaten in dieselbe 47-Zeilen-
Funktion `applyTextColours()` schreiben: #100 (über AK 3), #84 und #97. **Mit
dem Wegfall von AK 3 fasst #100 diese Funktion nicht mehr an.** Die Kollision
zwischen #100 und #84/#97 ist damit aufgehoben; A's Feld 1 beschreibt hier einen
Stand, den die Korrektur des PO überholt hat. Für Sprint 9 (#100 + #101) war sie
ohnehin ohne Wirkung — #101 arbeitet in `src/ui/notelistdelegate.cpp` und hat
mit #100 keine gemeinsame Datei.

**Übereinstimmend gemessen** und deshalb hier nur benannt: Größenklasse `m`
(beide), kein Build-Eingriff (beide), Rand 6 px ringsum unter allen Themes
(beide), `setColorSet(View)` ohne Wirkung auf die Feldgrafik (beide, auf zwei
verschiedenen Wegen), das frische `ImageSet` als einziger wirksamer Weg beim
Theme-Wechsel (beide), der Fenstermittelpunkt im Textbereich (beide),
`documentMargin` als einziger offener Weg für den Einzug (A gemessen, B
arithmetisch bestätigt: 174 + 2 × 6 = 186).

---

## 1. Dateimenge

Am Code vermessen, Notation B13. Konsolidiert aus beiden Messungen, nach dem
Wegfall von AK 3 und dem Zugang von AK 8 und AK 9.

**Quellen und Tests**

| Datei | Zeilen | was daran hängt |
|---|---|---|
| `src/capture/capturewindow.h` | 218 | ein `KSvg::FrameSvg *m_field` (`:197–202`); ein Lesezugriff nach dem Muster von `hullDevicePixelRatio()` (`:147`), falls die Herkunft benannt statt nur gezeichnet belegt werden soll |
| `src/capture/capturewindow.cpp` | 660 | Konstanten `:59–69` (Bildname und Vorsatz), Konstruktor `:214–232` (dritter Rahmen in der Schleife), `reloadDesktopTheme()` `:318–320` (dieselbe Schleife — hier zieht AK 4), `paintEvent()` `:396–417` (Feld **nach** der Hülle, **vor** `QWidget::paintEvent()`), `resizeHull()` `:442–455` (Größe und Bildpunktverhältnis), `eventFilter()` `:355–394` (`QEvent::Resize` auf `m_text`, damit das Feld der Geometrie folgt), `adjustHeight()` `:651–660` bzw. `documentMargin` (AK 5). **`applyTextColours()` bleibt unberührt** (AK 3 entfallen) |
| `tests/capturetest.cpp` | 1206 | **drei** bestehende Prüfsätze: `paintsTheThemesOwnHullInOnePiece()` `:551–586` (umzubauen), `hullIsCompleteAtFiveAndEightLines()` `:878` und `takesTheOpaqueVariantWithoutABlurringCompositor()` `:1078` (Abgriff aus dem Feld heraus, AK 9). **Neu**: je ein Nachweis zu AK 1, 4, 5, 6a, 6b, 8 |
| `tests/captureshots.cpp` | 226 | die Bildreihe `:186–204` — die zwölf Bilder zeigen ab jetzt zwei Flächen |
| `tests/themes/…/denkzettel-test-breit/colors` | 42 | **nur der Kommentarkopf** (B17, siehe Falle F2) — keine Werte |

**Build** — nichts. Gemessen von beiden: `KF6::Svg` steht bereits in
`target_link_libraries(denkzettelcapture …)` (`src/CMakeLists.txt:66`), das Feld
ist eine zweite `KSvg::FrameSvg` derselben Bibliothek. Kein neues Ziel, kein
neuer Läufer, keine neue Abhängigkeit, keine neue Fixture.

**Belege und Prüfmittel** — `docs/scrum/reviews/sprint-09-s100-eingabefeld/` neu,
mit eigenem `pruefen.sh` nach dem Muster von `sprint-07-s83-native-huelle/`.
**Wiederverwendbar statt neu zu erfinden** (mit `git ls-files` bestätigt):
`docs/scrum/reviews/2026-08-06-lesbarkeit/sonden/feldkante.cpp` rechnet Fläche
und Kante gegen die Hülle; `docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/echtelage.cpp`
ist der fertige Weg zum Sitzungsbild (B21); die Sonden beider Vorprüfungen
liegen in `sonden/` und `sonden-b/`.

**Fachliche Quellen** — `SPEC.md` 3.1, **zwei** Spiegelstriche und eine Grenze:
`:218–235` (die Fläche „eine durchgehende — kein Kasten im Kasten" wird
abgelöst), `:266–271` (Innenabstände: der Feldrand kommt hinzu, AK 5), dazu die
Grenze aus AK 6b an der Stelle, an der die gleichartige Grenze schon steht
(`:211–216`). **Der Spiegelstrich zur Schrift `:241–252` bleibt unberührt** —
das ist der Unterschied, den der Wegfall von AK 3 macht.

**Ausdrücklich nicht** — `wireframes/` (Zeichnung 4b ist mit `36156fd`
nachgezogen und nimmt F3 mit der Korrektur zurück; ein Beleg wird geankert, nicht
nachgezogen), `src/ui/*`, `src/shell/*`, `src/store/*`,
`src/capture/textareaheight.{h,cpp}` (gemessen: die Formel nimmt `chrome` als
Parameter, der Feldrand fließt über `adjustHeight()` ein),
`tests/librarytest.cpp`, `tests/libraryshots.cpp`, `tests/editshots.cpp`,
`tests/searchshots.cpp`, `tests/readmeshots.cpp`, die **Werte** unter
`tests/themes/` (Feld 6, Frage 3), `docs/scrum/PROZESS.md`, `CLAUDE.md`,
`CHANGELOG.md`, sämtliche Belegordner fremder Sprints.

---

## 2. Gemessene Fallen

Die Zeilen für den Spawn-Auftrag. Herkunft in Klammern; Ungemessenes ist
gekennzeichnet.

**F1 — `widgets/lineedit` löst unter jedem Namen auf** (A, von B nachgemessen).
Auch unter `kein-solches-theme`: `isValid`, `hasElementPrefix("base")`, Rand
6/6/6/6. KSvg fällt je Bild auf `default` zurück. Wer daraus eine Zusicherung
„dieses Theme bringt eine eigene Grafik mit" baut, prüft nichts; wer erwartet,
ein unbekanntes Theme verweigere die Grafik, baut einen Aufbau, in dem der
Fehler nicht auftreten kann. *Beleg:* `messungen/a1-feldprobe.txt` A/B,
`messungen-b/m7-nachpruefung-a.txt` V2.

**F2 — Der Rückfall wird vom Theme umgefärbt, und daran ist AK 4 prüfbar**
(A und B unabhängig). Dieselbe `default`-Grafik zeichnet unter
`denkzettel-test-schmal` die Fläche 20,22,24 und unter `denkzettel-test-breit`
**51,34,17** — das ist `BackgroundNormal` aus `[Colors:View]` der `colors`-Datei
jenes Prüf-Themes. Der Theme-Wechsel ist damit an den mitgelieferten Themes
messbar, ohne neue Fixture.
*Zugleich der Fund außerhalb der Fläche:* Der Kommentarkopf jener Datei sagt
heute, `[Colors:View]` bestehe „für die Mutationsprobe und für nichts sonst" und
die Werte färbten „die Grafik daneben nicht". **Beides wird mit dieser Story
falsch** — die Zeilen gehören im selben Zug nachgezogen (B17), die Werte nicht.
*Beleg:* `messungen/a2-feldbild.txt` E, `messungen-b/m1-feld-je-theme.txt`.

**F3 — `setColorSet(View)` ändert an der Feldgrafik nichts** (A am gezeichneten
Bild, B an `Svg::color()`). Unter allen elf geprüften Themes liefern
`colorSet(View)` und `colorSet(Window)` bildpunktgleiche Flächen. Die Farbe
wählt der Klassenname im SVG (`ColorScheme-ViewBackground`), nicht der Farbsatz.
Die Zeile sieht aus wie das Gegenstück zu `m_hull->setColorSet(Window)` (`:223`)
und ist keines. *Beleg:* `messungen/a2-feldbild.txt` G,
`messungen-b/m1-feld-je-theme.txt` M4.

**F4 — Die Feldgrafik folgt der Skalierung nicht von selbst** (A gemessen, B
bestätigt aus #83). Vorgabe nach dem Bau ist Verhältnis 1; erst
`setDevicePixelRatio(1,6)` macht aus 560×90 die 896×144 Gerätebildpunkte,
während die Ränder logisch 6 bleiben. **Offscreen unsichtbar**, weil
`QEvent::DevicePixelRatioChange` dort nie eintrifft (`capturewindow.cpp:434`).
Ein fehlendes `setDevicePixelRatio()` am Feldrahmen färbt keinen Test rot.
*Beleg:* `messungen/a2-feldbild.txt` H.

**F5 — Ein `FrameSvg` folgt auch als dritter nur einem frischen `ImageSet`**
(B, Mechanismus aus #83). Umbenennen wirkt nicht, ein frisches Set wirkt.
`reloadDesktopTheme()` baut ohnehin ein frisches und reicht es in einer Schleife
weiter (`:318`) — **der neue Rahmen gehört in diese Schleife**, sonst fällt AK 4
lautlos aus. *Beleg:* `messungen-b/m1-feld-je-theme.txt` M5.

**F6 — Der Fenstermittelpunkt liegt im Textbereich** (beide). Am laufenden
Fenster unter drei Themes und bei beiden Größen aus SPEC 3:

```
default:                600x178  Textbereich 18,39 564x98   Mitte 300,89   im Feld: ja
default (acht Zeilen):  600x232  Textbereich 18,39 564x152  Mitte 300,116  im Feld: ja
denkzettel-test-schmal: 600x174  Textbereich 16,37 568x98   Mitte 300,87   im Feld: ja
CachyOS-Nord-round:     600x182  Textbereich 20,41 560x98   Mitte 300,91   im Feld: ja
```

**Die beiden Prüfsätze dort bleiben grün**, wo sie heute laufen (§0.1), und
messen ab dann das Feld. Das ist AK 9. *Beleg:*
`messungen-b/m5-fenstergeometrie.txt`, `messungen-b/m7-nachpruefung-a.txt` V1,
`messungen/a1-feldprobe.txt` D.

**F7 — `documentMargin` ist der einzige offene Weg für den Einzug** (A gemessen,
B arithmetisch bestätigt). `QAbstractScrollArea::setViewportMargins()` ist
`protected`; `setContentsMargins(6,6,6,6)` auf dem `QPlainTextEdit` bewirkt
**gar nichts** (Fenster, Textbereich und Sichtfeld bleiben unverändert).
`documentMargin + 6` wirkt und zieht die Höhenrechnung mit: 174 → 186,
Textbereich 98 → 110. Grund: `adjustHeight()` `:656` verrechnet `documentMargin`
bereits im `chrome`, und `heightFollowsAFontChange()` rechnet dieselbe Formel
nach — der Prüfsatz wächst von selbst mit. *Beleg:* `messungen/a2-feldbild.txt` F,
`messungen-b/m5-fenstergeometrie.txt` (`documentMargin` 4, `frameWidth` 0).

**F8 — Der Textbereich muss transparent bleiben** (A). `Base = Qt::transparent`
(`:514`) und `viewport()->setAutoFillBackground(false)` (`:232`) sind seit #83
die Bedingung dafür, dass die Theme-Grafik durchkommt. Wer stattdessen `Base`
auf die Feldfarbe setzt, malt ein **Rechteck** über die gerundeten Ecken der
Feldgrafik. Der Kommentar an `:230–232` („eine durchgehende Fläche") wird mit
dieser Story sachlich falsch, obwohl der Code darunter gebraucht wird.
*Ungemessen:* dass die Ecken dabei sichtbar abgeschnitten aussähen; gemessen ist
allein der 6-px-Rand und dass die Grafik ihre Ecken darin zeichnet.

**F9 — Der Rand ist 6 px, aber keine Ganzzahl** (beide). 6/6/6/6 unter sieben
Themes, **6/6/5,99999/6,00001** unter den vier CachyOS-Themes. Ein Prüfsatz mit
`==` auf `6` fällt daran um; dieselbe Beobachtung steht für `marginSize()`
bereits in `capturetest.cpp:114`. *Beleg:*
`messungen-b/m1-feld-je-theme.txt`, `messungen/a1-feldprobe.txt`.

**F10 — Unter Wayland wird `QT_SCALE_FACTOR` nicht gesetzt** (A). Offscreen
liefert `QT_SCALE_FACTOR=1,6` genau 1,6; in der angemeldeten Sitzung
**multipliziert** es sich mit der Sitzungsskalierung zu 2,56. Der
Sprint-8-UI-Review hat die 1,6 stattdessen aus dem laufenden Fenster
mitgeschrieben. *Beleg:* `docs/scrum/reviews/sprint-08-ui-review/bericht.md:37–40`.

**F11 — Eine Kontrastzahl braucht Grund und Abgriffpunkt** (A nennt den Grund,
B misst die Spanne). Die Zahlen 1,39 : 1 und 1,33 : 1 sind über mittlerem Grau
128,128,128 gerechnet; über Schwarz beziehungsweise Weiß ergibt dieselbe Grafik
1,08 : 1 und 1,88 : 1. Die Kante verschiebt sich zusätzlich mit dem gelesenen
Bildpunkt ihrer 6 px Breite (1,55 gegen 1,33). **Für die korrigierten Kriterien
ohne Belang** — sie sichern keine Kontrastzahl mehr zu. *Beleg:* §0.3.

**F12 — Was das Feld trägt, ist von außen nicht abzulesen** (A). Für die Hülle
gibt es `hullDevicePixelRatio()` (`:147`) als benannten Ersatz für ein Bild; für
das Feld gibt es nichts dergleichen. Ohne einen gleichartigen Lesezugriff ist
AK 1 nur mittelbar prüfbar — über die gezeichneten Bildpunkte gegen eine zweite
Ausfertigung derselben Grafik, wie `themeHull()` (`:446–465`) es für die Hülle
tut. Das genügt; der Lesezugriff ist eine Bequemlichkeit, keine Bedingung.

---

## 3. AK-Urteil: **ready — ja**

Das Urteil fällt der Scrum Master nach beiden Messungen, gegen die korrigierte
Fassung der Kriterien.

**Das „nein" der Messung B ist ausgeräumt.** Es hing allein an AK 6, aus zwei
Gründen: der Wortlaut traf die falsche Themenmenge, und die zugesicherte Größe
hatte keinen festen Messwert. Die neue Fassung trennt beides sauber — **AK 6a**
sichert die Herkunft zu und führt dasselbe Prüfmittel wie AK 1; **AK 6b** macht
die Grenze an der **Deckung** fest (15 gegen 255), und die ist eine Eigenschaft
der Grafik statt eine des Bildschirmhintergrunds. Beide Zahlen sind in beiden
Messungen unabhängig bestätigt. Auch die Berufung auf die Dateimenge trägt
nicht mehr in die Irre: Dass ein Theme eine eigene Grafik mitbringt, ist durch
KSvg gar nicht zu beobachten (F1) — die Deckung ist es.

**Die übrigen Korrekturen tragen.** AK 2 misst jetzt gegen die Theme-Grafik
statt gegen einen Zustand, den es nach der Umsetzung nicht mehr gibt. AK 5 sagt,
dass der Rand um das **Textfeld** gemeint ist. AK 8 und AK 9 heben zwei Funde,
die vorher nur in den Vorberichten standen, auf die Ebene, auf der sie geprüft
werden. Der Wegfall von AK 3 nimmt der Story die einzige Stelle, an der sie in
`applyTextColours()` geschrieben hätte.

**Die drei Zusatzsätze der Definition of Ready, einzeln geprüft:**

- *Selbstdeklarierte offene Punkte:* keine mehr. Die Gestaltungsfrage ist seit
  dem 06.08.2026 entschieden, die Textfarbenfrage am 07.08.2026 durch den
  UX-Agenten, die Fokusschicht durch PO und UX als **#102** — und #102 ist
  angelegt, bevor #100 geschlossen wird.
- *Dateinamen als Prüfmittel:* Jede in Feld 4 genannte Fundstelle existiert
  (`git ls-files`) — `feldkante.cpp`, `echtelage.cpp`,
  `docs/scrum/reviews/2026-08-06-lesbarkeit/` und
  `docs/scrum/reviews/2026-08-07-textfarbe/entscheidung.md`.
- *B21-Belegform:* AK 2 spricht über Randmaß, Eckform, Kontur und Schatten und
  nennt das Sitzungsbild ausdrücklich. AK 4 spricht über Farben aus der
  Theme-Grafik und nennt es ebenso. Erfüllt an der Stelle, an der die Kriterien
  **formuliert** werden.

**Eine Korrektur des PO trägt nicht** und ist vor dem Ziehen zu berichtigen —
sie hindert das „ja" nicht, weil sie die Begründung eines Kriteriums betrifft
und nicht seine Prüfbarkeit: Die Zahl in AK 9 („unter `default` werden sie rot,
Feld 255 gegen Hülle 216") ist aus Messung B übernommen und dort ohne den
Auswahlpfad `opaque` gemessen worden. Richtig ist das Gegenteil, und es macht
AK 9 wichtiger (§0.1, Feld 6 Frage 5).

---

## 4. Prüfmittel je Kriterium

Wer schneidet, schneidet auch Feld 4. Geschnitten hat hier der PO; die Zeilen zu
AK 6a, 6b, 8 und 9 sind neu, die übrigen aus beiden Messungen zusammengeführt.
**AK 7 gibt es nicht** — die Nummerierung springt von 6b auf 8, weil AK 3
entfallen ist und die übrigen Nummern stehen blieben.

| AK | Prüfmittel | Belegform |
|---|---|---|
| **AK 1 — Herkunft** | Die gezeichneten Bildpunkte des Feldbereichs gegen eine **zweite Ausfertigung** von `widgets/lineedit`/`base` aus einer `ImageSet` desselben Namens — das Muster von `themeHull()` (`:446–465`), eine Grafik tiefer. **Gegenprobe im selben Lauf:** unter `denkzettel-test-breit` unterscheiden sich die Bildpunkte von denen unter `schmal` (51,34,17 gegen 20,22,24), und die gezeichnete Farbe ist nicht die Palettenfarbe — sonst hielte die Zusicherung auch für ein Feld, das aus der Palette gefüllt wird | offscreen, zwei Farbschemata **und** zwei Desktop-Themes |
| **AK 2 — Hülle unberührt** | Die bestehenden Hüllen-Zusicherungen `hullHasNoStairAtTheCorner()`, `squareThemeKeepsSquareCorners()`, `hullIsCompleteAtFiveAndEightLines()` laufen unverändert weiter und messen gegen die Theme-Grafik. **Reihenfolge-Bedingung:** die dritte davon ist genau die, die AK 9 repariert — AK 2 ist erst aussagekräftig, wenn AK 9 erledigt ist. **Schatten:** `CaptureWindow::shadow()` (`:136`) als benannter Ersatz | Zusicherungen offscreen; dazu **ein Bild aus der angemeldeten Sitzung** (B21) für Hülle, Rundung und Kontur |
| **AK 4 — Theme-Wechsel** | `reloadDesktopTheme()` **hin und zurück** am stehenden Fenster, geprüft an den gezeichneten Bildpunkten des Feldes; F2 macht das an den mitgelieferten Themes messbar. Hin *und* zurück, weil eine Farbe, die sich nur einmal bewegt, auch von einer einmal gesetzten und nie geräumten erklärt würde | offscreen; dazu **ein Sitzungsbild** (B21) |
| **AK 5 — Innenabstände** | Der Textbereich wächst um genau 2 × den Feldrand (gemessen 174 → 186), und der Text beginnt um den Feldrand weiter innen; **App-Name und Fußzeile stehen unverändert** — das ist die Hälfte, die die Zweideutigkeit ausräumt und deshalb mitzuprüfen ist. **Relativ zusichern, nicht gegen 6** (F9) | offscreen, zwei Desktop-Themes mit verschiedenem Hüllenrand, zwei Fenstergrößen (DoD 1) |
| **AK 6a — Herkunft** | Dasselbe Prüfmittel wie AK 1; keine eigene Zusicherung nötig | wie AK 1 |
| **AK 6b — Grenze an der Deckung** | `qAlpha()` der `framePixmap()` je Theme, **mit dem Auswahlpfad `opaque`** — gemessen 15 unter den fünf genannten Themes gegen 255 unter `default` und Breeze. Dazu der Textnachweis, dass die Bedingung in SPEC 3.1 steht (`git grep`). **Grenze der Prüfbarkeit, ausgesprochen:** Die fünf Themes sind CachyOS-Pakete; auf dem CI-Läufer liegen nur `default` und `breeze-*`, dort deckt jede Grafik 255. Die Zusicherung braucht einen `QSKIP` nach dem Muster von `takesTheOpaqueVariantWithoutABlurringCompositor()` (`:1068`) — und die Themes werden **nach gemessener Deckung** gewählt, nicht nach Namen und nicht nach `installedThemePair()` | offscreen; Fundstelle für die SPEC-Hälfte |
| **AK 8 — Wache gegen fehlendes Theme** | `staysUsableWithoutADesktopTheme()` im **getrennten Prozess** (`DENKZETTEL_TEST_WITHOUT_DESKTOP_THEME`), erweitert um den Fenstermittelpunkt. **Grenze der Prüfbarkeit, ausgesprochen:** In einem gewöhnlichen Lauf ist der Fall nicht herstellbar — jeder Name löst auf (F1). Ein Prüfsatz, der einen erfundenen Themenamen setzt, wäre ein Aufbau, in dem der Fehler nicht auftreten kann | offscreen, eigener Prozess |
| **AK 9 — die zwei Abgriffe** | **Mutationsprobe, und nur sie trägt hier.** Der Abgriff wandert aus dem Feld, und danach wird gemessen, ob `takesTheOpaqueVariantWithoutABlurringCompositor()` rot wird, wenn man den Auswahlpfad `opaque` aus `reloadDesktopTheme()` entfernt. **Heute wird er es nicht** — mit und ohne Auswahlpfad liegt der Abgriff bei 255 (§0.1). Eine Zusicherung, die grün bleibt, wenn man ihr den Gegenstand wegnimmt, ist keine. Dazu: beide Prüfsätze sagen danach im Namen oder im Kommentar, gegen welche Fläche sie messen | offscreen; die Mutationsprobe gehört in den Übergabebericht |

### Was ein Agent nicht prüfen kann

- **Den Schatten.** Offscreen scheitert `KWindowShadow::create()` mangels
  Compositor; `QWidget::grab()` nimmt nur das Widget auf, und der Schatten liegt
  außerhalb. Auch das Sitzungsbild zeigt ihn nicht, weil `echtelage.cpp` allein
  das Fenster aufnimmt. Bleibt der benannte Ersatz (`shadow()`).
- **Den Befund selbst.** Ob der Eingabebereich *erkennbar* ist, entscheidet der
  Kunde. Messbar ist die Abhebung in Zahlen; dass eine Zahl als „erkennbar"
  durchgeht, ist ein Urteil. Der Maßstab dafür ist KRunner mit 1,41 : 1 im
  Sitzungsbild von Sprint 7.
- **Ob der Kunde das Feld auf seinem Schreibtisch sieht.** Die Abhebung hängt am
  Grund hinter der teildeckenden Hülle (F11) und damit am Bildschirmhintergrund.
  Genau deshalb sichert AK 6b die Deckung zu und keine Kontrastzahl.
- **Die Deckungsgrenze aus AK 6b auf dem CI-Läufer** (siehe oben).
- **Den Theme-Wechsel über die Einstellung des Kunden.** Ein Agent schreibt
  `plasmarc` des Kunden nicht um. Für AK 4 nicht nötig: `reloadDesktopTheme(name)`
  wechselt am stehenden Fenster ohne Einstellungsänderung.
- **Nicht** darunter fällt das Sitzungsbild: Die Sitzung ist erreichbar, ein
  gegen `denkzettelcapture` gelinkter Helfer liefert es, und `QT_SCALE_FACTOR`
  bleibt dabei **ungesetzt** (F10).

---

## 5. Größenklasse: **`size:m`**

*„Trägt einen Strang aus."* Beide Messungen kamen unabhängig auf `m`, beide
allerdings **mit** AK 3. Die Klasse ist deshalb neu bewertet worden.

**Was der Wegfall von AK 3 abzieht:** die einzige Änderung in
`applyTextColours()`, **vier** der sieben brechenden Prüfsätze
(`noteTextUsesTheWindowTextRole`, `noteTextComesFromTheThemesOwnColours`,
`textColoursFollowADesktopThemeChange`, `themeTextColoursOutlastAColourSchemeChange`),
einen der drei SPEC-Spiegelstriche und die Frage nach der Struktur
`ThemeTextColours`. Dazu die Kollision mit #84 und #97 (§0.4).

**Was AK 8 und AK 9 hinzufügen:** eine `isValid()`-Wache von einer Zeile,
eine Erweiterung eines bestehenden Prüfsatzes, und den Umbau zweier Abgriffe,
der in beiden Messungen ohnehin schon gezählt war. **Netto ist die Story
kleiner geworden.**

**Warum trotzdem nicht `size:s`:** `s` heißt „wenige Dateien, kein neuer
Prüfweg". Beides trifft nicht zu. Berührt sind zwei Produktivdateien, drei
Testdateien und zwei SPEC-Spiegelstriche; und es kommt ein **neuer Prüfweg**
hinzu — die Deckungszusicherung je Theme samt ihrer ausgesprochenen Grenze auf
dem CI-Läufer (AK 6b), die Mutationsprobe zu AK 9 und ein Bild aus der
angemeldeten Sitzung für AK 2 und AK 4.

**Warum nicht `size:l`:** Es kommt kein Mechanismus hinzu. Das Feld ist eine
zweite `KSvg::FrameSvg` auf der `ImageSet`, die seit #83 da ist — dieselbe
Bibliothek, dieselbe Schleife, ein Vorsatz tiefer. Gemessen: kein Build-Eingriff,
keine neue Abhängigkeit, kein neuer Läufer, keine neue Fixture, keine
Schemaänderung. Zum Vergleich brachte #83 als `l` KSvg, den Schatten, zwei
Effektanmeldungen und die Theme-Wache überhaupt erst ins Haus. Die Fassung mit
der Fokusschicht wäre `l` gewesen; sie ist mit #102 abgetrennt.

**Sprint-Konto:** `2 Issues · 2×m` (#100 und #101). Kein `xl`, kein `l`, also
keine der beiden Grenzen berührt — die Klassenregel hält ohne
Grenzüberschreitung.

---

## 6. Offene Fragen

**Frage 1 — AK 6 (Messung B, blockierend): erledigt.** Die Teilung in 6a und 6b
räumt beide gemessenen Einwände aus. Kein Rest.

**Frage 2 — die Lesbarkeitsfolge aus AK 3 (Messung B): gegenstandslos.** Der
gemessene Rückfall von 4,74 : 1 auf 4,22 : 1 entstand erst durch die Umstellung
der Textfarbe. Mit dem Wegfall von AK 3 entsteht er nicht. Die Messung bleibt in
`messung-b.md` stehen; sie ist zugleich der Beleg dafür, dass **der einzige Wert
unter 4,5 : 1 in der gesamten Untersuchung durch AK 3 entstanden wäre** — was
die Entscheidung des UX-Agenten von der anderen Seite bestätigt.

**Frage 3 — braucht es ein viertes Prüf-Theme mit eigener `lineedit`-Grafik?**
**Entschieden: nein.** Drei gemessene Gründe:

1. **Die Unterscheidung, die ein solches Theme verkörpern sollte, ist gar nicht
   beobachtbar.** Ob ein Theme die Grafik mitbringt, sagt KSvg nicht — jeder
   Name löst auf (F1). Ein Prüf-Theme „mit eigener Grafik" belegte nichts, was
   ein Prüf-Theme ohne sie nicht auch belegte.
2. **Die vorhandenen Fixtures genügen für AK 1 und AK 4.** Weil der Rückfall vom
   Theme umgefärbt wird (F2), zeichnen `denkzettel-test-schmal` und
   `denkzettel-test-breit` zwei sichtbar verschiedene Felder — auf jeder
   Maschine, auch ohne `libplasma`.
3. **Für AK 6b hülfe ein Prüf-Theme gerade nicht.** Die Deckungsklassen 15 und
   255 sind Eigenschaften wirklicher Plasma-Themes; eine mitgelieferte Grafik
   mit Deckung 15 prüfte unsere eigene SVG-Datei gegen unsere eigene Erwartung.
   `desktopthemes.h` hält genau diese Unterscheidung fest („Neither replaces the
   other"), und die richtige Antwort ist dort der `QSKIP`, nicht eine vierte
   Fixture.

**Zu den Wählern in `tests/desktopthemes.h`, die Messung B als „nur zufällig
treffend" gemeldet hatte: sie bleiben unverändert.** Der Befund war richtig —
`installedThemePair()` sortiert nach Hüllenrand und trifft die Belegform „eines
mit, eines ohne eigene Grafik" nur auf dieser Maschine. Er ist mit der
korrigierten AK-6b-Fassung aber gegenstandslos: Die Belegform ruht nicht mehr
auf dieser Unterscheidung, und AK 6b wählt seine Themes **nach gemessener
Deckung**, so wie `takesTheOpaqueVariantWithoutABlurringCompositor()` seine nach
gemessenem Variantenunterschied wählt (`:1053–1063`). Das ist das Muster, das
die Datei bereits trägt; ein neuer Wähler wäre eine Regel neben einer
gleichwertigen.

**Frage 4 — AK 5, wo die 6 px greifen: vom PO beantwortet.** Der Rand um das
Textfeld; App-Name und Fußzeile bleiben stehen. In Feld 4 ist beides als
Prüfgegenstand aufgenommen, damit die Hälfte „bleiben stehen" nicht ungeprüft
mitläuft.

**Frage 5 (neu, an den PO): Die Zahl in AK 9 gehört berichtigt.**
AK 9 begründet sich heute mit „unter `default` werden sie rot (Feld 255 gegen
Hülle 216)". Die 216 ist ohne den Auswahlpfad `opaque` gemessen und gehört zu
keinem Zustand, in dem die Prüfsätze laufen. Richtig ist: **Sie bleiben grün,
überall wo sie heute laufen**, und messen ab dann das Feld statt der Hülle; rot
würden sie allein unter `Iridescent-round` und den drei
`cachyos-emerald`-Themes, die kein Lauf heute abgreift. Der Fehler stammt aus
Messung B. **Kein Grund, das Kriterium zu ändern** — nur seinen
Begründungssatz, und die Mutationsprobe in Feld 4 gehört dazu, weil sie das
Einzige ist, was hier noch trägt.

**Frage 6 (neu, an den PO, klein): AK 7 fehlt in der Zählung.** Nach dem Wegfall
von AK 3 springt die Nummerierung von 6b auf 8. Das ist beabsichtigt (die
Nummern bleiben stehen, damit Issue, Zeichnung und Bericht dieselbe Sprache
sprechen), sieht aber wie eine Lücke aus. Ein Satz im Issue erspart dem nächsten
Leser die Suche.

**Melden, nicht heilen — zwei Funde außerhalb der Fläche** (beide von A,
einer von B unabhängig bestätigt):

- `tests/themes/plasma/desktoptheme/denkzettel-test-breit/colors`, Kommentarkopf:
  „`[Colors:View]` existiert für die Mutationsprobe und für nichts sonst" und
  „keiner der beiden Werte färbt die Grafik daneben" werden mit dieser Story
  falsch — gemessen färbt `[Colors:View]` die zurückgefallene
  `lineedit`-Grafik. Der Kommentar gehört nachgezogen, die Werte nicht.
- `src/capture/capturewindow.cpp:230–232`: Der Kommentarblock begründet
  `viewport()->setAutoFillBackground(false)` mit „eine durchgehende Fläche, kein
  Kasten im Kasten". Der Code darunter wird für das Feld **gebraucht**, die
  Begründung stimmt nicht mehr.

---

## Reproduktion

Die Sonden beider Bearbeiter liegen versioniert im Ordner. Messung A:
`bash docs/scrum/vorberichte/100-eingabefeld/pruefen.sh`. Messung B und die
Nachprüfung:

```
cmake -B docs/scrum/vorberichte/100-eingabefeld/build-b \
      -S docs/scrum/vorberichte/100-eingabefeld/sonden-b -DCMAKE_BUILD_TYPE=Debug
cmake --build docs/scrum/vorberichte/100-eingabefeld/build-b
cd docs/scrum/vorberichte/100-eingabefeld
QT_QPA_PLATFORM=offscreen ./build-b/nachpruefung ../../../../tests/themes   # V1–V3
QT_QPA_PLATFORM=offscreen ./build-b/feldsonde    ../../../../tests/themes
QT_QPA_PLATFORM=offscreen ./build-b/schichtsonde ../../../../tests/themes
QT_QPA_PLATFORM=offscreen ./build-b/alphasonde
QT_QPA_PLATFORM=offscreen ./build-b/paarsonde
```

Die Bauverzeichnisse sind von `.gitignore` (`build-*/`) erfasst; Quelltexte und
Ausgaben liegen im Repository. Keine Installation nach `/usr`, kein Eingriff in
`build/`.
