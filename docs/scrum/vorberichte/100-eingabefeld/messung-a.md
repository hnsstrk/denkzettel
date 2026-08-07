# Vorprüfung #100 — Messung Bearbeiter A (`denkzettel-ux`)

**Gegenstand:** Issue #100, „Erfassungsfenster: Der Eingabebereich ist nicht als
solcher erkennbar" · **Modus:** Vorprüfung (Messung A, am Code) ·
**Datum:** 07.08.2026, Ganymed · **Quellstand:** `main` @ `88129ba` ·
**Belege:** `messungen/a1-feldprobe.txt`, `messungen/a2-feldbild.txt`, Sonden in
`sonden/`, wiederholbar über
`bash docs/scrum/vorberichte/100-eingabefeld/pruefen.sh`

Dieser Bericht trägt die Felder **1, 2, 4, 5 und 6**; **Feld 3 (Ready-Urteil)
fällt der Scrum Master** — meine Einschätzung dazu steht als solche
gekennzeichnet.

**Stand der Werkzeuge** (B17 — eine Aussage gilt für einen Stand):
ksvg 6.28.0, qt6-base 6.11.1, kwindowsystem 6.28.0, plasma-desktop 6.7.4.
Sitzung: Wayland, Skalierung des Fensters **1,6**. Acht installierte
Desktop-Themes, achtzehn Farbschemata.

---

## Feld 1 — Dateimenge (am Code vermessen, Notation nach B13 / Sprint 6 §5.2)

| | **#100** — das Textfeld aus der Theme-Grafik |
|---|---|
| **Issue** | **#100** (`epic:M1`, `typ:story`) |
| **Zweig** | `story/100-eingabefeld` |
| **Quellen und Tests** | `src/capture/capturewindow.{h,cpp}` — **ganz** (660 bzw. 218 Zeilen), die Fläche verteilt sich über die Datei: Konstanten `:59–69` (ein Name `widgets/lineedit` und ein Vorsatz `base` kommen hinzu), Konstruktor `:214–232` (die Schleife über `{m_hull, m_shadowTiles}` bekommt einen dritten Rahmen; `m_text->viewport()->setAutoFillBackground(false)` `:232` **bleibt**, samt neuer Begründung), `reloadDesktopTheme()` `:318–320` (dieselbe Schleife, hier zieht F4), `paintEvent()` `:396–417` (das Feld wird **nach** der Hülle und **vor** `QWidget::paintEvent()` gezeichnet), `resizeHull()` `:442–455` (Bildpunktverhältnis und Größe des Feldes), `eventFilter()` `:355–394` (der Filter auf `m_text` liegt schon — ein `QEvent::Resize` genügt, damit das Feld der Textbereichsgeometrie folgt), `applyTextColours()` `:500–505` (**AK 3**, zwei Ausdrücke), `adjustHeight()` `:651–660` (unverändert, siehe F7), Kopfdatei `:197–202` (ein `KSvg::FrameSvg *m_field`) und ein Lesezugriff nach dem Muster von `hullDevicePixelRatio()` `:147` (F9).<br>`tests/capturetest.cpp` — **zwei** bestehende Zusicherungen werden ungültig und **vier** wandern auf die neue Farbquelle: `paintsTheThemesOwnHullInOnePiece()` `:551–586` (fällt, F5), `hullIsCompleteAtFiveAndEightLines()` `:878` (Abgriff in der Fenstermitte, F6), `noteTextUsesTheWindowTextRole()` `:588–618`, `noteTextComesFromTheThemesOwnColours()` `:650`, `textColoursFollowADesktopThemeChange()` `:726`, `themeTextColoursOutlastAColourSchemeChange()` `:757`, dazu die Konstante `:146`. **Neu**: je ein Nachweis zu F1–F5.<br>`tests/captureshots.cpp` — die Bildreihe `:186–204`; die zwölf Bilder zeigen ab jetzt zwei Flächen. |
| **Build** | **Nichts.** Gemessen: `KF6::Svg` steht bereits in `target_link_libraries(denkzettelcapture …)` (`src/CMakeLists.txt:66`), das Feld ist eine zweite `KSvg::FrameSvg` derselben Bibliothek. Kein neues Ziel, kein neuer Läufer, keine neue Abhängigkeit. |
| **Belege und Prüfmittel** | `docs/scrum/reviews/sprint-NN-s100-eingabefeld/` — neu anzulegen, mit eigenem `pruefen.sh` nach dem Muster von `sprint-07-s83-native-huelle/`. **Wiederverwendbar, nicht neu zu erfinden:** `docs/scrum/reviews/2026-08-06-lesbarkeit/sonden/feldkante.cpp` rechnet Fläche und Kante gegen die Hülle bereits fertig; `docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/echtelage.cpp` ist der fertige Weg zum Sitzungsbild (B21); die beiden Sonden dieser Vorprüfung liegen in `sonden/`. |
| **Fachliche Quellen** | **SPEC 3.1** — drei Spiegelstriche sind nachzuziehen (DoD 4/B9): `:218–235` („**Fläche** … eine durchgehende — kein Kasten im Kasten", steht seit dem 05.08.2026 unter Kundenbefund und wird jetzt abgelöst), `:241–252` (Notiztext `WindowText` → Ansichtsrolle, **AK 3**), `:266–271` (Innenabstände „zuzüglich des Randes, den das Theme beansprucht" — der Feldrand kommt hinzu, **AK 5**), sowie die Grenze aus **AK 6** an der Stelle, an der die gleichartige Grenze schon steht (`:211–216`, dazu `:262–265`). **Zeichnung 4b ist bereits nachgezogen** (`36156fd`) — `wireframes/` bleibt UX und ist **nicht** Dev-Fläche. |
| **Ausdrücklich nicht** | `src/ui/*`, `src/shell/*`, `src/store/*`, `src/capture/textareaheight.{h,cpp}` (gemessen: die Formel nimmt `chrome` als Parameter, der Feldrand fließt über `adjustHeight()` ein — die Datei wird nicht angefasst), `tests/librarytest.cpp`, `tests/libraryshots.cpp`, `tests/editshots.cpp`, `tests/searchshots.cpp`, `tests/readmeshots.cpp`, `tests/themes/` (gemessen: die vorhandenen Prüf-Themes genügen, F2), `wireframes/`, `CHANGELOG.md`, sowie sämtliche Belegordner fremder Sprints. |

### Kollisionsfläche — drei der vier Sprint-9-Kandidaten schreiben in dieselbe Funktion

| Vorgang | Bereich in `capturewindow.cpp` | Kleinster Abstand zu #100 | Urteil |
|---|---|---|---|
| **#84** (App-Name gehört auf `WindowText`) | `applyTextColours()` `:503–505` und `:520–526`, dazu `subtleLabel()` `:98–110` | **null bis zwei Zeilen** — AK 3 schreibt `:500–502`, #84 schreibt die Zeilen unmittelbar darunter | **nicht parallel.** Git mischt beides in denselben Hunk |
| **#97** (Platzhalter lesbarer als die Notiz) | dieselbe Funktion, dieselbe Gegenüberstellung `noteColour` gegen `subtleColour` | **null** — und darüber hinaus: AK 3 **verschiebt den Grund**, gegen den #97 gemessen wurde | **nicht parallel**, und #97 ist nach #100 neu zu messen |
| **#101** (Bibliothek: Trennung) | `src/ui/notelistdelegate.cpp`, `tests/librarytest.cpp` | keine gemeinsame Datei | **parallel möglich** |

**Woran ich das messe:** an den Zeilenbereichen der Funktionen
(`grep -n '^void CaptureWindow::'`) und daran, welche Funktion die jeweilige
Heilung anfassen *muss*. Der Sprint-6-Maßstab lautete „der kleinste Abstand ist
null: beide schreiben in dieselbe Datei" (§5.1). Hier schreiben drei Stories in
dieselbe **Funktion** von 47 Zeilen.

---

## Feld 2 — Gemessene Fallen (die Zeilen für den Spawn-Auftrag)

Muster: Sprint 6 §10.6, „Punkte, die je einen Fehlversuch ersparen". Jede Zeile
mit Messbeleg; Ungemessenes ist als solches gekennzeichnet.

### Zur Auflösung der Grafik

**F1 — `widgets/lineedit` löst unter *jedem* Namen auf, auch unter einem, auf
den nichts hört.** Alle drei mitgelieferten Prüf-Themes bringen die Grafik
**nicht** mit, und trotzdem meldet die `FrameSvg` `isValid() == true`,
`hasElementPrefix("base") == true` und Ränder von 6 px. Dasselbe unter dem
erfundenen Namen `kein-solches-theme`. KSvg fällt je Bild auf das Theme
`default` zurück. **Wer daraus eine Zusicherung „das Theme bringt eine eigene
Grafik mit" baut, prüft nichts** — und wer umgekehrt erwartet, dass ein
unbekanntes Theme die Grafik verweigert, baut einen Testaufbau, in dem der
Fehler nicht auftreten kann.
*Beleg:* `messungen/a1-feldprobe.txt`, Abschnitte A/B.

**F2 — Der Rückfall wird trotzdem vom Theme umgefärbt, und genau daran ist F4
prüfbar.** Dieselbe `default`-Grafik zeichnet unter `denkzettel-test-schmal`
die Fläche 20,22,24 und unter `denkzettel-test-breit` die Fläche **51,34,17** —
das ist `BackgroundNormal` aus der Gruppe `[Colors:View]` der `colors`-Datei
jenes Prüf-Themes. Der Theme-Wechsel ist damit **an den mitgelieferten Themes
messbar**, ohne dass eine neue Fixture-Grafik gebaut werden müsste.
*Beleg:* `messungen/a2-feldbild.txt`, Abschnitt E.
*Anmerkung an den Dev:* Der Kommentarkopf von
`tests/themes/plasma/desktoptheme/denkzettel-test-breit/colors` sagt heute,
`[Colors:View]` existiere „für die Mutationsprobe und für nichts sonst" und die
Werte färbten „die Grafik daneben nicht". Beides wird mit dieser Story falsch —
die Zeile gehört im selben Zug nachgezogen (B17).

**F3 — `setColorSet(View)` ändert an der Feldgrafik nichts.** Unter allen acht
installierten Themes **und** den drei Prüf-Themes liefern `colorSet(View)` und
`colorSet(Window)` bildpunktgleiche Flächen — auch unter
`denkzettel-test-breit`, dessen beide Gruppen ausdrücklich verschiedene Werte
tragen. Die Farbe wählt der Klassenname im SVG (`ColorScheme-ViewBackground`),
nicht der Farbsatz. **Die Zeile sieht aus wie das Gegenstück zu
`m_hull->setColorSet(Window)` `:223` und ist keines**; wer sie schreibt,
schreibt sie ohne gemessene Wirkung und schuldet dafür eine Begründung.
*Beleg:* `messungen/a2-feldbild.txt`, Abschnitt G.

**F4 — Die Feldgrafik folgt der Skalierung nicht von selbst.** Vorgabe nach dem
Bau ist Verhältnis 1; erst `setDevicePixelRatio(1,6)` macht aus 560×90 die
896×144 Gerätebildpunkte, während die Ränder logisch 6 bleiben. Das ist
dieselbe Falle wie beim Hüllenrahmen (#83, F1) — und sie ist **offscreen
unsichtbar**, weil `QEvent::DevicePixelRatioChange` dort nie eintrifft
(`capturewindow.cpp:434`). Ein fehlendes `setDevicePixelRatio()` auf dem
Feldrahmen färbt keinen Test rot.
*Beleg:* `messungen/a2-feldbild.txt`, Abschnitt H; Mechanismus: #83.

### Zu den Zusicherungen, die dabei kippen

**F5 — `paintsTheThemesOwnHullInOnePiece()` (`capturetest.cpp:551–586`) fällt
mit dieser Story, und zwar planmäßig.** Sie vergleicht den Bildpunkt *hinter*
dem Text (`:572`) mit dem *neben* dem Text und verlangt Gleichheit — das ist
der Satz „eine durchgehende Fläche", den der Kunde aufgehoben hat. Sie ist
nicht zu löschen, sondern umzubauen: neben dem Text weiter die Hülle, hinter
dem Text das Feld über der Hülle.

**F6 — `hullIsCompleteAtFiveAndEightLines()` greift in der Fenstermitte ab
(`:878`), und die Mitte liegt im Textbereich.** Gemessen am gebauten Fenster:
600×174, Textbereich x=16…584, y=37…135 — der Abgriff (300, 87) liegt mittendrin.
Die Zusicherung vergleicht nur die **Deckung**, und die trägt hier
zufällig: Prüf-Themes, `default`, `breeze-*` und `CachyOS-Nord-round` decken in
der Mitte alle a255, ein deckendes Feld darüber ergibt wieder a255. Sie kippt,
sobald `themes::anyInstalledTheme()` ein durchscheinendes Theme liefert — unter
`cachyos-emerald-color` deckt die Hülle a9 und das Feld a15, zusammen also
weder a9 noch a15. **Auf dieser Maschine grün, auf einer anderen rot**, und der
Unterschied ist die Sortierreihenfolge eines Verzeichnisses.
*Beleg:* `messungen/a1-feldprobe.txt` Abschnitt D, `a2-feldbild.txt` Abschnitt I.

**F7 — Der einzige offene Weg, den Text nach innen zu rücken, ist
`documentMargin`.** `QAbstractScrollArea::setViewportMargins()` ist
`protected` und von außen nicht erreichbar. `setContentsMargins(6,6,6,6)` auf
dem `QPlainTextEdit` bewirkt **gar nichts** — gemessen bleiben Fenster (174),
Textbereich (98) und Sichtfeld (568×98 bei 0,0) unverändert.
`documentMargin + 6` wirkt und **zieht die Höhenrechnung von selbst mit**:
Fenster 174 → 186, Textbereich 98 → 110, also genau die 2 × 6 px des Feldrandes.
Grund: `adjustHeight()` `:656` verrechnet `documentMargin` bereits im `chrome`.
*Beleg:* `messungen/a2-feldbild.txt`, Abschnitt F.

**F8 — Der Textbereich muss transparent bleiben.** `Base = Qt::transparent`
(`:514`) und `viewport()->setAutoFillBackground(false)` (`:232`) sind seit #83
die Bedingung dafür, dass die Theme-Grafik durchkommt. Wer stattdessen `Base`
auf die Feldfarbe setzt, malt ein **Rechteck** über die gerundeten Ecken der
Feldgrafik — und der Kommentar an `:230–232` („eine durchgehende Fläche") wird
mit dieser Story sachlich falsch, obwohl der Code darunter richtig bleibt.
*Ungemessen* (kein Bild dazu erzeugt): dass die Ecken dabei sichtbar
abgeschnitten aussähen. Gemessen ist allein, dass die Feldgrafik einen Rand von
6 px beansprucht und ihre Ecken innerhalb dieses Randes zeichnet.

**F9 — Was das Feld trägt, ist von außen nicht abzulesen.** Für die Hülle gibt
es `hullDevicePixelRatio()` `:147` als benannten Ersatz für ein Bild; für das
Feld gibt es nichts dergleichen. Ohne einen gleichartigen Lesezugriff ist
F1 (Herkunft) nur mittelbar prüfbar — über die gezeichneten Bildpunkte gegen
eine zweite Ausfertigung derselben Grafik, so wie `themeHull()`
(`capturetest.cpp:446–465`) es für die Hülle tut.

### Zur Belegform

**F10 — Unter Wayland wird `QT_SCALE_FACTOR` *nicht* gesetzt.** Offscreen
liefert `QT_SCALE_FACTOR=1,6` genau 1,6; in der angemeldeten Sitzung
**multipliziert** es sich mit der Sitzungsskalierung zu 2,56. Der
Sprint-8-UI-Review hat das ausdrücklich so gehandhabt und die 1,6 aus dem
laufenden Fenster mitgeschrieben.
*Beleg:* `docs/scrum/reviews/sprint-08-ui-review/bericht.md:37–40`,
`capturetest.cpp:961–964`.

**F11 — Ein Kontrastwert gegen die Hülle braucht einen benannten Grund.** Beide
Grafiken können durchscheinen; die Zahlen 1,39 : 1 und 1,33 : 1 sind über
mittlerem Grau 128,128,128 gerechnet. Ohne genannten Grund ist die Zahl nicht
reproduzierbar.
*Beleg:* `docs/scrum/reviews/2026-08-06-lesbarkeit/messungen/m3-feldkante.txt`,
Kopf.

---

## Feld 4 — Prüfmittel je Akzeptanzkriterium

| AK | Prüfmittel | Belegform |
|---|---|---|
| **AK 1 (F1) — Herkunft** | Zusicherung in `capturetest`: die gezeichneten Bildpunkte des Feldbereichs gegen eine **zweite Ausfertigung** von `widgets/lineedit`/`base` aus einer `ImageSet` desselben Namens — das Muster von `themeHull()` (`:446–465`), eine Grafik tiefer. Gegenprobe im selben Lauf: unter `denkzettel-test-breit` unterscheiden sich die Bildpunkte von denen unter `schmal` (51,34,17 gegen 20,22,24), sonst hielte die Zusicherung auch für ein Feld, das gar nicht aus dem Theme kommt | offscreen, zwei Farbschemata **und** zwei Desktop-Themes |
| **AK 2 (F2) — Hülle unberührt** | Zweiteilig, und das ist keine Bequemlichkeit. **Randmaß, Eckform, Kontur:** die bestehenden Zusicherungen `hullIsCompleteAtFiveAndEightLines()`, `hullHasNoStairAtTheCorner()`, `squareThemeKeepsSquareCorners()` laufen unverändert weiter und messen genau diese Größen gegen die Theme-Grafik — sie sind der Nachweis, dass ein Fehlgriff beim Vorsatz die Hülle nicht mitzieht (F6 beachten). **Schatten:** `CaptureWindow::shadow()` `:136` als benannter Ersatz; ein Bild kann ihn grundsätzlich nicht zeigen | Zusicherungen offscreen; dazu **ein Bild aus der angemeldeten Sitzung** (B21) für Hülle, Rundung und Kontur |
| **AK 3 (F3) — Text auf dem Feld** | `capturetest`: unter `denkzettel-test-breit` trägt `QPalette::Text` des Textbereichs **0,204,0** (die Ansichtsrolle) statt 255,0,153 (die Fensterrolle) — gemessen, dass beide Werte verschieden sind und aus verschiedenen Gruppen derselben Datei stammen; unter `denkzettel-test-schmal` folgt er dem Farbschema. Die vier bestehenden Farbzusicherungen wandern mit | offscreen, zwei Farbschemata und zwei Desktop-Themes |
| **AK 4 (F4) — Theme-Wechsel** | `capturetest`: `reloadDesktopTheme()` hin und zurück am **stehenden** Fenster, geprüft an den gezeichneten Bildpunkten des Feldes (F2 macht das messbar). Hin **und** zurück, weil eine Farbe, die sich nur einmal bewegt, auch von einer einmal gesetzten und nie gelöschten erklärt würde | offscreen; dazu **ein Bild aus der angemeldeten Sitzung** (B21), weil das Kriterium über Hülle und Dekoration mitspricht |
| **AK 5 (F5) — Innenabstände** | `capturetest`: der Textbereich wächst um genau 2 × den Feldrand (gemessen 174 → 186 bei 6 px), und der Text beginnt um den Feldrand weiter innen. **Relativ zusichern, nicht gegen 6**: die Ränder derselben Grafik kommen unter fünf der acht Themes als `5,99999` rechts und `6,00001` unten heraus — eine Zusicherung auf die ganze Zahl bräche daran | offscreen, zwei Desktop-Themes mit verschiedenem Hüllenrand |
| **AK 6 — die Grenze** | Kein Testlauf, sondern ein **Textnachweis**: der Satz steht in SPEC 3.1 und im Übergabebericht. Prüfbar ist seine Existenz (`git grep`), nicht seine Wahrheit — die ist am 06.08.2026 gemessen (`m3-feldkante.txt`, 1,03–1,10 : 1 unter fünf Themes) | Fundstelle |

### Was ein Agent nicht prüfen kann

- **Den Schatten.** Offscreen scheitert `KWindowShadow::create()` mangels
  Compositor, und `QWidget::grab()` nimmt nur das Widget auf — der Schatten
  liegt außerhalb. Auch das Sitzungsbild zeigt ihn nicht, weil der fertige
  Prüfweg (`echtelage.cpp`) allein das Fenster aufnimmt. Bleibt der benannte
  Ersatz.
- **Den Befund selbst.** Ob der Eingabebereich *erkennbar* ist, entscheidet der
  Kunde. Messbar ist die Abhebung in Zahlen; dass eine Zahl von 1,39 : 1 als
  „erkennbar" durchgeht, ist ein Urteil und keine Messung — der Maßstab dafür
  ist KRunner mit 1,41 : 1 im Sitzungsbild von Sprint 7.
- **Den Theme-Wechsel über die Einstellung des Kunden.** Ein Agent darf
  `plasmarc` des Kunden nicht umschreiben. Für AK 4 ist das auch nicht nötig:
  `reloadDesktopTheme(name)` wechselt am stehenden Fenster ohne jede
  Einstellungsänderung — genau dafür trägt die Methode ihren Namensparameter
  (`capturewindow.h:160`).
- **Nicht** darunter fällt das Sitzungsbild selbst: Die Sitzung ist erreichbar
  (`WAYLAND_DISPLAY` gesetzt), ein Helfer gegen `denkzettelcapture` gelinkt
  liefert es, und `QT_SCALE_FACTOR` bleibt dabei ungesetzt (F10).

---

## Feld 5 — Größenklasse: **`size:m`**

*„Trägt einen Strang aus."*

**Wofür `m` und nicht `s`:** Die Story fasst zwei Produktivdateien und drei
Testdateien an, macht **zwei bestehende Zusicherungen ungültig** (F5, F6),
verschiebt **vier weitere** auf eine neue Farbquelle (AK 3) und braucht fünf
neue Nachweise. Dazu vier Stellen in SPEC 3.1. Das läuft nicht nebenher.

**Wofür `m` und nicht `l`:** Es kommt kein Mechanismus hinzu. Das Feld ist eine
zweite `KSvg::FrameSvg` auf der `ImageSet`, die seit #83 schon da ist — dieselbe
Bibliothek, dieselbe Schleife, ein Vorsatz tiefer. Gemessen: **kein**
Build-Eingriff, **keine** neue Abhängigkeit, **kein** neuer Läufer, **keine**
neue Fixture (F2), **keine** Schemaänderung. Zum Vergleich: #83 war `l` und
brachte KSvg, den Schatten, zwei Effektanmeldungen und die Theme-Wache
überhaupt erst ins Haus.

**AK 3 hebt die Klasse nicht.** Gemessen kostet es zwei Ausdrücke im
Produktivcode (`:500–502`), eine Konstante und vier Testfunktionen, die ohnehin
in derselben Datei liegen — siehe Feld 6.

---

## Feld 6 — Offene Fragen an Kunde oder PO

### 6.1 Die Frage, die der PO ausdrücklich vorgelegt hat: bleibt AK 3 in dieser Story?

**Antwort: ja, AK 3 bleibt.** Drei gemessene Gründe, in dieser Reihenfolge:

1. **Die Kollision ist null.** AK 3 schreibt `applyTextColours()` `:500–502`;
   ein eigenes Issue schriebe dieselbe 47-Zeilen-Funktion und dieselben vier
   Testfunktionen (`:588`, `:650`, `:726`, `:757`). Zwei Stränge darauf sind
   nach B13 nicht parallelisierbar — der Zugewinn eines Schnitts wäre also
   nicht Nebenläufigkeit, sondern nur eine zweite Runde am selben Ort.
2. **Ohne AK 3 steht eine grüne Zusicherung gegen die Zeichnung.**
   `noteTextUsesTheWindowTextRole()` `:588–618` sichert die Fensterrolle zu.
   Zeichnung 4b hat diesen Prüfsatz am 06.08.2026 durch F3 **ersetzt** und
   ausdrücklich vermerkt, ein Prüfsatz auf der Fensterrolle würde „die neue
   Festlegung rot melden, obwohl sie richtig gebaut ist". Liefert die Story das
   Feld ohne AK 3, bleibt dieser Test grün und behauptet eine Regel, die die
   Zeichnung zurückgezogen hat. Das ist genau die Lage, gegen die B17 gefasst
   ist.
3. **Die Kosten sind gemessen klein.** `KSvg::Svg::color(ViewText)` liegt am
   Hüllenrahmen bereits an — für die Ansichtsfarbe braucht es **kein**
   Feldobjekt; es sind zwei Ausdrücke (`KSvg::Svg::Text` → `ViewText`,
   `QPalette::WindowText` → `QPalette::Text`). Die Prüfgrundlage steht
   ebenfalls schon: `denkzettel-test-breit` trägt in `[Colors:View]`
   ausdrücklich **andere** Werte (0,204,0 gegen 255,0,153).

*Gegenrede, die ich nicht unterschlage:* AK 3 ist auf der Einstellung des
Kunden **wirkungslos** — unter `default`, `breeze-dark` und `breeze-light`
sind Fenster- und Ansichtstextfarbe bildpunktgleich (252,252,252 bzw. 35,38,41).
Sichtbar wird der Unterschied allein unter `cachyos-emerald-color`
(0,199,144 gegen 2,189,136). Wer AK 3 nach Wirkung schneidet, kann es streichen;
wer nach Herkunft schneidet — und das tut dieses Projekt seit #85 —, muss es
mitnehmen.

### 6.2 AK 6 widerspricht der Zeichnung, die es zitiert

AK 6 sagt: „**Zugesichert ist die Abhebung** unter dem Rückfall `default` und
unter den Themes, die die Grafik zeichnen." Zeichnung 4b sagt an derselben
Stelle das Gegenteil: „Die Sichtbarkeit des Feldes **ist nicht zugesichert** …
Deshalb wird **nur die Herkunft** zugesichert, nicht die Zahl (F1)."

Dazu kommt eine gemessene Zweideutigkeit im Halbsatz „die Themes, die die
Grafik zeichnen": **alle acht** installierten Themes zeichnen sie — fünf davon
als Hauch mit Deckung 15 von 255. Wörtlich gelesen sichert AK 6 also eine
Abhebung zu, die unter fünf Themes gemessen nicht eintritt; sinngemäß gelesen
(„die sie sichtbar zeichnen") ist der Satz zirkulär.

**Vorschlag an den PO:** AK 6 auf die Fassung der Zeichnung ziehen — zugesichert
ist die **Herkunft**; die Grenze wird **benannt**, samt der Beobachtung, dass es
unter `default` (der Einstellung des Kunden) trägt. Dann sagen Issue, Zeichnung
und SPEC dasselbe, und das Kriterium ist mit einer Fundstelle prüfbar statt mit
einer Zahl, die für fünf Themes falsch wäre.

### 6.3 AK 2 misst gegen einen Zustand, den es nach der Umsetzung nicht mehr gibt

„Randmaß, Eckform, Kontur und Schatten des Fensters sind **dieselben wie ohne
das Feld**" — dieser Vergleichszustand verschwindet mit der Story. Prüfbar ist
der Satz trotzdem, aber gegen einen anderen Maßstab: gegen die **Theme-Grafik**,
so wie die drei bestehenden Hüllen-Zusicherungen es tun. **Vorschlag:** die
Formulierung darauf ziehen („dieselben wie die der Theme-Grafik"), sonst hängt
das Kriterium an einem Gedächtnisbild.

### 6.4 Meine Einschätzung zum Ready-Urteil (Feld 3 fällt der Scrum Master)

**Ready ja** — mit den zwei Formulierungskorrekturen aus 6.2 und 6.3, die
Kriterien und keine Gestaltungsfragen betreffen. Alle sechs Kriterien haben ein
Prüfmittel (Feld 4), die Belegform ist je Kriterium benannt, AK 2 und AK 4
nennen das Sitzungsbild ausdrücklich (die dritte DoR-Zusatzregel vom
04.08.2026), das Issue führt keine selbstdeklarierten offenen Punkte mehr, und
jeder in Feld 4 genannte Dateiname existiert (`git ls-files`). Die
Gestaltungsfrage ist mit der Kundenentscheidung vom 06.08.2026 geschlossen und
die Zeichnung nachgezogen.

### 6.5 Melden, nicht heilen — zwei Funde außerhalb meiner Fläche

- **`tests/themes/.../denkzettel-test-breit/colors`**, Kommentarkopf: Die
  Aussage „`[Colors:View]` existiert für die Mutationsprobe und für nichts
  sonst" und die Aussage „keiner der beiden Werte färbt die Grafik daneben"
  werden mit dieser Story falsch — gemessen färbt `[Colors:View]` die
  zurückgefallene `lineedit`-Grafik (F2). Gehört im selben Zug nachgezogen.
- **`src/capture/capturewindow.cpp:230–232` und `:496–499`**: zwei
  Kommentarblöcke begründen den Bau mit „eine durchgehende Fläche, kein Kasten
  im Kasten" und mit den Zahlen 4,74 : 1 / 4,22 : 1 gegen die Fensterfläche.
  Der Code darunter bleibt richtig (die Zeilen `:232` und `:514` werden für das
  Feld **gebraucht**), die Begründung stimmt nicht mehr.

### 6.6 Ein Hinweis zum Sprint-Zuschnitt, keine Priorisierung

Von den vier Kandidaten, die der Kunde für Sprint 9 genannt hat, schreiben
**drei** in `applyTextColours()`: #100 (AK 3), #84 und #97. Nur #101 hat keine
gemeinsame Datei. Ob das eine Frage des Zuschnitts oder der Reihenfolge ist,
entscheidet der PO; gemessen ist, dass #100, #84 und #97 nicht gleichzeitig
laufen können und dass #97 nach #100 **neu zu messen** ist, weil AK 3 den
Grund unter dem Notiztext verschiebt.
