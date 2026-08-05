# Übergabebericht #85 — Capture: Lesbarkeit unter fremden Desktop-Themes

**Strang A des Sprints 8** · Zweig `story/85-lesbarkeit`, Ausgangsstand
`sprint-08-basis` · **Stand dieses Berichts:** 05.08.2026, 21:10 CEST, Ganymed ·
Nichts nach `/usr` installiert, nichts gepusht, nichts gemerged.

**Prüfstand** (B17 — eine Aussage gilt für einen Stand): kwin 6.7.3-1.1,
ksvg 6.28.0-1.1, qt6-base 6.11.1-1.1, libplasma 6.7.3-1.1, plasma-desktop
6.7.3-1.1; Sitzung Wayland, Bildschirm 2400×1350 logisch bei Verhältnis 1,6,
Aufnahmen 3840×2160. Farbschema der Sitzung `[Colors:Window]` 32,35,38 ·
252,252,252 · 161,169,177 (Breeze Dark). `plasmarc` nennt kein Desktop-Theme,
der Rückfall `default` gilt. Vollständig in `messungen/p7-pruefstand.txt`.

**Wiederholbar:** `bash docs/scrum/reviews/sprint-08-s85-lesbarkeit/pruefen.sh`
und `…/mutationsproben.sh`.

---

## 1. Was gebaut wurde

**Die Schrift kommt aus derselben Quelle wie die Fläche**, für **beide**
Textklassen. Bringt das Desktop-Theme eine eigene `colors`-Datei mit, gelten
deren Farben; bringt es keine mit, gilt das Farbschema wie bisher.

Drei Stücke, rund fünfzig Zeilen:

1. **`capture::themeTextColoursOf()`** liest `[Colors:Window]` aus der
   `colors`-Datei des Themes — derselbe Griff, mit dem `contrastEffectOf()`
   zwanzig Zeilen weiter oben die Nachbardatei liest. Am Bau ändert sich
   nichts.
2. **`applyTextColours()`** trägt die Vorrangregel, und zwar allein. Der
   Notiztext kommt über `KSvg::Svg::color(Text)` bei gesetztem
   `colorSet(Window)`, die gedämpfte Klasse aus der gelesenen Datei — KSvg hat
   für `ForegroundInactive` kein Gegenstück.
3. **Zwei Anlässe laufen durch dieselbe Funktion:** der Theme-Wechsel aus
   `reloadDesktopTheme()` und der Schemawechsel aus dem Ereignisfilter.

**Warum der Notiztext nicht ebenfalls aus der gelesenen Datei kommt:** Er soll,
wo das Theme nichts mitbringt, der **Palette** folgen und nicht dem, was KSvg
für das Farbschema hält. Die beiden gehen im Testmodus auseinander (Vorprüfung
F3), und ein Rückfall auf KSvg hätte den Prüfsatz an einen Zustand gehängt, den
es auf keiner Maschine gibt. Wo das Theme eine Datei mitbringt, sagen beide Wege
dasselbe — das sichert `noteTextComesFromTheThemesOwnColours()` ausdrücklich zu,
damit sie nicht unbemerkt auseinanderlaufen.

### Geänderte Dateien

| Datei | Was |
|---|---|
| `src/capture/capturewindow.h` | `ThemeTextColours`, `themeTextColoursOf()`, `m_themeText`, `m_subtleLabels` |
| `src/capture/capturewindow.cpp` | die drei Stücke oben; `applyTextColours()` im Konstruktor entfällt, weil `reloadDesktopTheme()` es ruft |
| `tests/capturetest.cpp` | fünf neue Prüfsätze; zwei bestehende nennen jetzt ihr Theme selbst |
| `tests/themes/…/denkzettel-test-breit/colors` | **neu** — das Prüfgut, ohne das AK 1 an der Paketlage der Maschine hinge |
| `SPEC.md` | 3.1 (die Regel, drei statt zwei Stellen der gedämpften Klasse, Herkunft statt Kontrastzahl) und 3.2 (Punkte 10–12) |

**Nicht angefasst:** `src/main.cpp`, alle `CMakeLists.txt`, `src/ui/`,
`src/shell/`, `src/store/`, `textareaheight.*`, die Bildläufer der anderen
Ansichten, `wireframes/`, `CLAUDE.md`, `PROZESS.md`, Belegordner fremder
Sprints. Am Bau ist nichts zu tun gewesen; `KF6::Svg` und `KF6::ConfigCore`
waren verlinkt, und `tests/CMakeLists.txt` reicht den Theme-**Ordner** durch,
also brauchte die neue Datei keine Zeile.

---

## 2. Die Kriterien einzeln

### AK 1 — beide Textklassen aus derselben Quelle wie die Fläche

**Prüfmittel:** `readsTheTextColoursOfTheDesktopTheme()`,
`noteTextComesFromTheThemesOwnColours()`,
`subtleTextsComeFromTheThemesOwnColours()` gegen das mitgelieferte Paar (breit
**mit**, schmal **ohne** `colors`-Datei); dazu die Messreihe
`messungen/p2-schriftquelle-je-theme.txt` über alle acht installierten Themes
und `messungen/p3-fenster-je-theme.txt` am gebauten Fenster.

**Ergebnis: erfüllt.** Am Fenster gemessen, je Theme (P3, Auswahlpfad `opaque`,
weil offscreen nichts weichzeichnet):

| Theme | `colors`? | Notiztext | gedämpfte Klasse |
|---|---|---|---|
| `default` | nein | 252,252,252 *(Schema)* | 161,169,177 *(Schema)* |
| `breeze-dark` | ja | 252,252,252 | 161,169,177 |
| `breeze-light` | ja | **35,38,41** | **112,125,138** |
| `CachyOS-Nord-round` | nein | 252,252,252 *(Schema)* | 161,169,177 *(Schema)* |
| `Iridescent-round` | nein | 252,252,252 *(Schema)* | 161,169,177 *(Schema)* |
| `cachyos-emerald` | nein | 252,252,252 *(Schema)* | 161,169,177 *(Schema)* |
| `cachyos-emerald-color` | ja | **0,199,144** | **102,106,115** |
| `cachyos-emerald-light` | ja | **35,38,41** | **112,125,138** |

`breeze-dark` bringt eine eigene Datei mit, deren Werte **zufällig** denen des
eingestellten Schemas gleichen. Unter einem anderen Schema stehen sie fest (P4)
— genau der Fall, den eine Messung unter nur einem Schema nicht von „folgt dem
Schema" unterscheiden könnte.

**Zugesichert ist die Herkunft, nicht die Kontrastzahl.** Im Testmodus sieht
`capturetest` `default` mit hellem Grund unter der dunklen Schrift des Kunden —
1,11 : 1, ein Zustand, den es nirgends gibt. Die Prüfsätze vergleichen deshalb
gegen die Farbe der `colors`-Datei beziehungsweise gegen die Palettenfarbe. Die
beiden Werte des Prüfguts (255,0,153 und 0,153,255) trägt kein Farbschema; ein
Prüfsatz gegen 35,38,41 hätte Theme- und Schemafarbe nicht unterscheiden können.

**Mutationsproben:** M1 (Notiztext zurück auf die Schemafarbe), M2 (gedämpfte
Klasse zurück), M3 (das Tor sagt immer „nichts mitgebracht"), M4 (das Tor liest
`[Colors:View]` statt `[Colors:Window]`), M8 (dem Prüf-Theme wird die
`colors`-Datei genommen), M9 (Platzhaltertext), M10 (App-Name und Fußzeile).
**Alle sieben rot.**

### AK 2 — nachgemessen am laufenden Stand in der angemeldeten Sitzung

**Prüfmittel:** `sonden/sitzungsbeleg.cpp` — Grund als **gewöhnliches** Fenster
in Bildschirmgröße, drei Aufnahmen mit `spectacle -f`, Fenster über den
Unterschied gefunden, Farben **am Fenster gelesen**. Protokoll
`messungen/p5-sitzung-benannter-grund.txt`, Bilder in `bilder/`.

**Ergebnis: erfüllt.** Sitzung entsperrt (`LockedHint=no`, im Protokoll),
Auswahlpfad **durchscheinend** (die Sitzung weichzeichnet), je Zahl der Grund
benannt:

| Theme | Grund | Fläche im Fenster | Notiztext | gedämpft | *vorher* Notiztext |
|---|---|---|---|---|---|
| `cachyos-emerald-light` | weiß | 248,248,248 | **14,32 : 1** | 3,96 : 1 | 1,04 : 1 |
| `cachyos-emerald-light` | schwarz | 1,1,1 | **1,37 : 1** | 4,96 : 1 | 20,35 : 1 |
| `cachyos-emerald-color` | weiß | 248,248,248 | **2,07 : 1** | 5,10 : 1 | 1,04 : 1 |
| `breeze-light` | weiß | 242,242,243 | **13,59 : 1** | 3,76 : 1 | 1,09 : 1 |
| `default` | weiß | 66,69,71 | 9,42 : 1 | 4,06 : 1 | 9,42 : 1 *(unverändert)* |
| `CachyOS-Nord-round` | weiß | 30,34,51 | 15,37 : 1 | 6,63 : 1 | 15,37 : 1 *(unverändert)* |

**Zwei Sätze, die der Kunde lesen muss** (§4 führt sie aus): Über hellem Grund
gewinnt die Story deutlich. Über **dunklem** Grund verliert sie unter
`cachyos-emerald-light` — von 20,35 : 1 auf 1,37 : 1.

**Zwei Fehlversuche auf dem Weg, beide gemessen und beide behoben:**

1. **Der erste Lauf maß den blanken Grund und nicht das Fenster.** Das Fenster
   über den Gesamtunterschied zweier Aufnahmen zu suchen liefert ein Rechteck
   von 2210×1274 statt 960×291 — es reicht bis in die **Fensterleiste**, weil
   dort ein Eintrag dazukommt. Der Messpunkt lag danach im weißen Grund, und die
   Zahl (15,21 : 1) sah aus wie ein Befund über die Hülle. Die Sonde zerlegt die
   Unterschiedsmaske jetzt in zusammenhängende Teile und erkennt das Fenster an
   seiner **von außen bekannten** Größe (Fenstergröße × Bildpunktverhältnis).
   Die berichtigte Zahl ist 14,32 : 1 — und trifft damit die unabhängig erhobene
   Zahl der Vorprüfung (M1, `cachyos-emerald-light`, Themeschrift über weißem
   Grund) auf die zweite Stelle.
2. **Die untere Größenschranke war zu streng.** Bei genau der Fenstergröße
   meldete die Sonde über schwarzem Grund „das Fenster hebt sich nirgends ab" —
   ein Befund über die Schranke, der aussieht wie einer über das Theme. Über
   Schwarz hebt sich der äußere Hüllenrand nicht ab, das Teilstück fällt auf
   941×272. Die Schranke liegt jetzt bei drei Vierteln, und der Fehlschlagfall
   nennt das größte gefundene Teilstück, statt nur „nichts" zu sagen.

**Mutationsprobe:** Für einen Sitzungsbeleg gibt es keine; er ist eine Messung
und keine Zusicherung im Code. Was an seine Stelle tritt, ist die
**Selbstprüfung des Aufbaus** in der Sonde selbst: Sie bricht mit Rückgabe 3 ab,
wenn die erste Aufnahme den eigenen Grund nicht zeigt (gesperrter Bildschirm),
und sie nennt das Verhältnis von gefundenem Teilstück zu Fenstergröße in jeder
Ausgabe.

### AK 3 — alle installierten Desktop-Themes, mit `colors`-Datei und Schemafolge

**Prüfmittel:** `messungen/p2-schriftquelle-je-theme.txt` (acht Themes, je
eigener Prozess, **beide** Auswahlpfade) und
`messungen/p4-folgt-dem-schema.txt` (dieselben acht unter **drei** Schemata —
unter einem misst man Übereinstimmung, nicht Folgen).

**Ergebnis: erfüllt.** Vier Themes bringen eine `colors`-Datei mit
(`breeze-dark`, `breeze-light`, `cachyos-emerald-color`,
`cachyos-emerald-light`), vier nicht. Über drei Schemata folgen die vier ohne
Datei dem Schema ausnahmslos, die vier mit Datei stehen ausnahmslos fest.

**Eine Berichtigung zur Vorprüfung, damit sie sich nicht als Halbwahrheit
fortschreibt:** Der Befund „`CachyOS-Nord-round` folgt dem Farbschema nicht"
gilt für seine **Fläche** — die zeichnet unter allen drei Schemata 30,34,51. Sein
**Text** folgt dem Schema sehr wohl (35,38,41 / 252,252,252 / 102,194,242, P4),
denn es bringt keine `colors`-Datei mit. Fläche und Schrift sind hier zwei
Fragen, und die Story beantwortet die zweite.

Die Deckung je Theme hängt am Auswahlpfad, und beide stehen in P2 — `default`
216/255 durchscheinend gegen 255/255 unter `opaque`, die drei Emerald-Themes
drehen dabei von 0,0,0 bei 7/255 nach 227,227,255 bei 9/255.

**Mutationsprobe:** keine — reine Messung, kein Codeanteil.

### AK 4 — die gedämpfte Klasse gemessen, ihre Grenze benannt, nicht behoben

**Prüfmittel:** dieselben Läufe; die Grenze steht in P3 und P5.

**Ergebnis: erfüllt, und die Grenze steht.** Unter `breeze-light` erreicht
**keine** der beiden Quellen 4,5 : 1: aus dem Theme 3,69 : 1 (deckend, P3)
beziehungsweise 3,76 : 1 (in der Sitzung über weißem Grund, P5), aus dem Schema
2,09 : 1. Die Story hat die Zahl von 2,09 auf 3,7 gehoben und den Mindestwert
nicht erreicht. **Das ist #84 und nicht diese Story.**

Die gedämpfte Klasse hat **drei** Stellen, nicht zwei: App-Name, Fußzeile und
der Platzhaltertext des leeren Eingabefeldes. Alle drei sind auf dem Bild
`bilder/fenster-breeze-light-weiss-leer.png` zu sehen und tragen dieselbe Farbe.

**Mutationsproben:** M2, M9, M10 — alle rot.

### AK 5 — nach einem Theme-Wechsel im laufenden Betrieb stimmt die Farbe

**Prüfmittel:** `textColoursFollowADesktopThemeChange()` — an **einem**
stehenden Fenster, hin und zurück (eine Farbe, die nur einmal wanderte, wäre
auch durch eine erklärt, die einmal gesetzt und nie geräumt wurde). Dazu P3: die
acht Zeilen entstehen alle an demselben Fenster nach je einem Theme-Wechsel.

**Ergebnis: erfüllt.**

**Mutationsprobe:** M5 (der Theme-Wechsel schreibt die Farben nicht mehr nach) —
rot.

### AK 6 — Mutationsprobe je tragender Zusicherung

**Prüfmittel:** `mutationsproben.sh`, ausführbar, arbeitet auf einer Kopie unter
`/tmp` und hält je Probe **den Eingriff im Wortlaut**, den Lauf und das erwartete
Ergebnis. Protokoll `messungen/p8-mutationsproben.txt`.

**Ergebnis: erfüllt. Grundlinie 32 grün / 0 rot, zwölf Proben, zwölfmal rot,
keine „grün geblieben".**

| Probe | Eingriff | Ergebnis |
|---|---|---|
| M1 | Notiztext nimmt wieder die Schemafarbe | rot (3) |
| M2 | gedämpfte Klasse nimmt wieder die Schemafarbe | rot (3) |
| M3 | das Tor sagt immer „dieses Theme bringt nichts mit" | rot (5) |
| M4 | das Tor liest `[Colors:View]` statt `[Colors:Window]` | rot (5) — *seit 06.08.2026, siehe unten* |
| M5 | der Theme-Wechsel schreibt die Farben nicht mehr nach | rot (4) |
| M6 | die Themefarbe überlebt den Palettenwechsel nicht | rot (4) |
| M6b | die Vorrangregel wandert nach `reloadDesktopTheme()` | rot (3) |
| M7 | der Palettenwechsel erreicht die Textfarben nicht mehr | rot (2) |
| M8 | dem breiten Prüf-Theme wird die `colors`-Datei genommen | rot (5) |
| M9 | der Platzhaltertext bekommt die Themefarbe nicht | rot (2) |
| M10 | App-Name und Fußzeile bekommen die Themefarbe nicht | rot (3) |
| M11 | `noteTextUsesTheWindowTextRole()` nennt sein Theme nicht selbst | rot (1) |

**Berichtigung vom 06.08.2026 (karpathy-Befund K1): M4 hat bis dahin nichts
Eigenes geprüft.** Das mitgelieferte Prüf-Theme trug nur `[Colors:Window]`, also
endete „die falsche Gruppe gelesen" auf demselben `return {}` wie M3 „keine
Datei gefunden" — **dieselben fünf Fehlschläge mit denselben Ist-Werten.** Es
waren zwölf Eingriffe, aber **elf Sachverhalte**; die Tabelle oben war ehrlich,
die Zusammenfassung zählte doppelt. Das ist dieselbe Fehlerklasse, die in
Sprint 7 an #83 gefunden wurde (fünfzehn behauptete Proben, zwölf Sachverhalte),
und sie trifft AK 6 im Kern: Eine Zusicherung war ungeprüft.

**Behoben durch drei Zeilen im Prüfgut**, nicht durch eine Umformulierung: Das
Theme trägt jetzt eine `[Colors:View]`-Gruppe mit **abweichenden** Werten
(0,204,0 und 204,102,0). Die falsche Gruppe liefert damit eine Farbe statt
nichts, und die beiden Fehlerbilder trennen sich — nachgemessen:

| | M3 „keine Datei" | M4 „falsche Gruppe" |
|---|---|---|
| `readsTheTextColoursOfTheDesktopTheme` | Ist `#ff000000` *(ungültig)* | Ist `#ff00cc00` *(View-Farbe)* |
| `noteTextComesFromTheThemesOwnColours` | Notiztext fällt auf `#fffcfcfc` *(Schemafarbe)* | Notiztext **bleibt** `#ffff0099`, und der Satz fällt an der Zeile, die **beide Lesewege gegeneinander hält** |
| gedämpfte Klasse | `#ffa1a9b1` *(Schemafarbe)* | `#ffcc6600` *(View-Farbe)* |

Damit belegt M4 genau die Zusicherung, die vorher unbelegt war: dass der
KConfig-Weg und der KSvg-Weg dieselbe Farbe nennen. **Zwölf Eingriffe, zwölf
Sachverhalte.** Die Gesamtzahl der Fehlschläge bleibt bei beiden Proben 5 — wer
Mutationsproben an der Zahl der roten Sätze unterscheidet, unterscheidet sie
auch jetzt nicht. Unterscheidbar sind sie an der **gefallenen Zeile** und am
**Ist-Wert**, und danach ist zu prüfen.

**Zwei weitere Dinge, die dieser Lauf über sich selbst gelernt hat, und beide
gehören in den Bericht, weil sie beim ersten Mal falsch waren:**

1. **Der erste Durchlauf hatte eine rote Grundlinie und war damit wertlos.** Die
   Kopie entsteht aus `git ls-files`, und die neue `colors`-Datei war noch nicht
   getrackt — die Kopie stand also dauerhaft im Zustand von M8, fünf Prüfsätze
   rot, und jedes „rot nach dem Eingriff" war bedeutungslos. Das Skript gibt
   die Grundlinie jetzt aus, und sie steht im Protokoll.
2. **Der erste Durchlauf lief ohne `QT_QPA_PLATFORM=offscreen`** und damit in
   der angemeldeten Sitzung, wo `sessionBlursBehindWindows()` `true` sagt: vier
   Prüfsätze der Hülle fielen unabhängig von jeder Mutation. Das Skript setzt
   jetzt dieselbe Umgebung, die `ctest` dem Prüfsatz gibt.

**Und eine Grenze der Mutationsprobe selbst, gemessen statt vermutet:** Ein
Eingriff, der **allein** den AK-7-Satz fallen lässt, ist auf diesem Fenster
nicht zu bauen. `QWidget::setPalette()` stellt seinem Widget das
`QEvent::PaletteChange` unmittelbar zu — noch innerhalb des Aufrufs —, also
läuft jede Mutation am Ereignisfilter auch bei dem `setPalette()` ein, das
`applyTextColours()` selbst auslöst, und trifft den Theme-Wechsel mit. Zwei
Anläufe (M6 und M6b) fallen deshalb breiter als vorhergesagt. Was AK 7 trotzdem
trägt, steht unten.

### AK 7 — nach einem Schemawechsel bleibt die Textfarbe die des Themes

**Prüfmittel:** `themeTextColoursOutlastAColourSchemeChange()` — unter dem
breiten Theme das Schema wechseln, beide Klassen prüfen; **und im selben Lauf
der Gegenfall**, weil die Zusicherung sonst auch für ein Fenster gälte, das
Palettenwechsel überhaupt ignoriert.

**Ergebnis: erfüllt.** Der Mechanismus liegt an einer Stelle: Theme-Wechsel und
Schemawechsel laufen beide durch `applyTextColours()`, und `m_themeText` wird
vom Schemawechsel nicht berührt.

**Mutationsproben:** M6 und M6b, beide rot. Der Satz ist der einzige, der
**nach** einem Schemawechsel unter einem Theme mit `colors`-Datei vergleicht; in
beiden Proben fällt er an derselben Zeile mit `#ff232629` — der Schemafarbe, die
er selbst gesetzt hat, und einem Wert, den kein anderer Prüfsatz erzeugt.

---

## 3. Selbst-Sichtprüfung (DoD 2/3)

Am **gebauten** Stand, in der **angemeldeten** Sitzung, bei der Skalierung des
Kunden (Bildpunktverhältnis 1,6, in jeder Ausgabe von P5 mitgeschrieben).
Zwölf Bilder in `bilder/`, je Theme der Normalfall und der Leerzustand.

- `fenster-breeze-light-weiss.png` — dunkle Schrift auf heller Hülle. Vor dieser
  Story stand hier die fast weiße Schemaschrift auf derselben Hülle: 1,09 : 1.
- `fenster-breeze-light-weiss-leer.png` — der Leerzustand. Platzhaltertext,
  App-Name und Fußzeile tragen dieselbe gedämpfte Themefarbe; das ist die
  dritte Stelle, die die Vorprüfung als F-B10 gemeldet hat.
- `fenster-cachyos-emerald-color-weiss.png` — die Eigenfarbe des Themes
  (0,199,144). Dünn, aber sichtbar; vorher stand hier Weiß auf fast Weiß.
- `fenster-cachyos-emerald-light-schwarz.png` — der Fall, der schlechter
  geworden ist (§4).

Ein Meldungszustand existiert im Erfassungsfenster nicht.

**Journal:** `journalctl --user -t denkzetteld` führt aus dem Zeitraum nur die
Einträge des **anderen** Sprint-Strangs (nicht systemweit installiert, kein
kglobalaccel). Diese Story meldet nichts bei einem fremden Dienst an, also gibt
es hier nichts nachzulesen — der Sitzungsbeleg fährt das echte `CaptureWindow`
und nicht den Dienst, weil ein zweiter Dienst dem Kunden seinen laufenden
weggerissen hätte.

**Linter:** `lint-tidy` und `lint-clazy` laufen durch; clazy meldet **3**
Befunde, das ist die CI-Schwelle und der unveränderte Altbestand vom
04.08.2026. Der Bau selbst ist warnungsfrei.

---

## 4. Was diese Story nicht leistet — und der eine Punkt, der dem PO gehört

**Die gedämpfte Klasse wird nicht lesbar.** Unter `breeze-light` bleibt sie unter
4,5 : 1, aus beiden Quellen. Das ist AK 4 und es ist so benannt; die Behebung
ist **#84**.

**Unter den drei `cachyos-emerald`-Themes wird keine Lesbarkeit zugesichert**,
und der Grund ist geerbt: Der Kontrasteffekt, auf den diese Themes gebaut sind,
**existiert auf diesem Compositor-Stand nicht.** KWin 6.7.3 führt unter 54
geladenen Effekten keinen mit „contrast" im Namen; `isEffectLoaded("blur")` ist
`true`, `isEffectLoaded("backgroundcontrast")` ist `false`
(`messungen/p6-kontrasteffekt.txt`). Bei 3,5 % Deckung steht der Text dort auf
dem Bildschirmhintergrund und auf nichts sonst.

*Zum Halbsatz „auf die diese Themes gebaut sind" siehe die Berichtigung vom
06.08.2026 weiter unten: `cachyos-emerald-light` fordert den Effekt gar nicht
an. Die Zusicherungslücke bleibt für alle drei, ihr Grund ist nicht bei allen
dreien derselbe.*

**Und daraus folgt der Punkt, der dem PO gehört, weil er über die Story
hinausgeht:**

> **Unter `cachyos-emerald-light` ist die Lesbarkeit über einem dunklen Grund
> von 20,35 : 1 auf 1,37 : 1 gefallen.** Über hellem Grund ist sie im selben Zug
> von 1,04 : 1 auf 14,32 : 1 gestiegen. Beides ist dieselbe Entscheidung: Die
> Schrift kommt aus derselben Hand wie die Fläche, und dieses Theme reicht eine
> **dunkle** Schrift zu einer Hülle, die zu 3,5 % deckt. Die Rechnung geht auf,
> **wenn** der Compositor den Grund hinter der Hülle abdunkelt — genau dafür
> trägt das Theme seine `[ContrastEffect]`-Gruppe. Ohne diesen Effekt gibt es
> keine Schriftfarbe, die beide Richtungen trägt.

Das ist kein Fehler dieser Umsetzung und keiner der Kundenentscheidung, sondern
die Folge eines fehlenden Compositor-Effekts. **Ich melde es, statt es zu
heilen** — eine Ausnahme für dieses Theme wäre genau die „Anpassung", die der
Kunde am 04.08.2026 abgewählt hat. Was damit zu tun ist, entscheidet der PO;
sachlich hängt es an demselben Impediment wie #83.

**Berichtigung vom 06.08.2026 (karpathy-Befund K2). Der letzte Satz des Kastens
oben ist widerlegt: `cachyos-emerald-light` trägt keine
`[ContrastEffect]`-Gruppe.** Der Absatz bleibt stehen, weil ein überholter Beleg
geankert und nicht geglättet wird (B17); was gilt, steht hier.

An den Dateien nachgemessen (`/usr/share/plasma/desktoptheme/*/metadata.desktop`,
06.08.2026):

| Theme | `metadata.desktop` | `[ContrastEffect]` |
|---|---|---|
| `cachyos-emerald` | ja | **ja** |
| `cachyos-emerald-color` | ja | **ja** |
| `cachyos-emerald-light` | ja | **nein** |
| `Iridescent-round` | ja | **ja** |
| `CachyOS-Nord-round` | ja | nein |
| `default`, `breeze-dark`, `breeze-light` | keine | — |

**Was das für die Reichweite des Impediments ändert, und es ändert sie:** Der
Absturz auf 1,37 : 1 unter `cachyos-emerald-light` hängt **nicht** am
Compositor. Dieses Theme fordert keinen Kontrasteffekt an, also bekäme es auch
auf einem KWin **mit** geladenem Effekt keinen — der Fall bliebe, wie er ist. Es
reicht eine dunkle Schrift zu einer Hülle, die zu 3,5 % deckt, und verlässt sich
auf nichts, was den Grund darunter abdunkelte. Das ist eine Eigenschaft des
Themes, keine des Fenstersystems.

Das Impediment unten bleibt davon unberührt und gilt für die drei Themes, die
den Effekt tatsächlich anfordern (`cachyos-emerald`, `cachyos-emerald-color`,
`Iridescent-round`). Auch nach seiner Behebung wäre `cachyos-emerald-light` über
dunklem Grund unlesbar — wer diesen Fall lösen will, braucht eine eigene
Entscheidung dazu und nicht die Rückkehr des Effekts. Ich melde ihn; ihn zu
heilen wäre wieder die abgewählte Anpassung.

SPEC 3.2 Punkt 10 und der UI-Review sind bereits berichtigt; dieser Bericht war
die letzte Stelle, die die widerlegte Begründung führte.

### Impediment (unverändert offen, jetzt schärfer belegt)

**Die Wirkung von `enableBackgroundContrast` ist auf diesem Stand nicht zu
beobachten, weil der Effekt nicht geladen ist.** Sprint 7 hatte „nicht
beobachtbar" gemeldet; gemessen ist er **nicht vorhanden**. Solange das so ist,
kann kein Strang eine Zusicherung über die Lesbarkeit unter den drei
Emerald-Themes prüfen. Als SPEC 3.2, Punkt 10 festgeschrieben.

### Was ich außerhalb meiner Fläche gesehen und nicht angefasst habe

Nichts Neues. Der Bestandsbefund zur Schattenpolsterung (in `bindShadow()`
gesetzt, bei `resizeHull()` nicht nachgezogen) steht seit dem 05.08.2026 als
**#86** und ist damit gebucht.

---

## 5. Belege dieses Ordners

| Datei | Was |
|---|---|
| `pruefen.sh` | fährt P1–P7 in einem Zug |
| `mutationsproben.sh` | zwölf Proben, je mit Eingriff im Wortlaut; arbeitet auf einer Kopie unter `/tmp` |
| `sonden/sitzungsbeleg.cpp` | AK 2: Aufnahme über benanntem Grund, Farben **am Fenster** gelesen, Selbstprüfung des Aufbaus |
| `messungen/p1-pruefsaetze.txt` | `ctest` vollständig, dazu die sieben Prüfsätze dieser Story einzeln |
| `messungen/p2-schriftquelle-je-theme.txt` | acht Themes plus das mitgelieferte Paar, beide Auswahlpfade |
| `messungen/p3-fenster-je-theme.txt` | beide Textklassen am gebauten Fenster, je Theme |
| `messungen/p4-folgt-dem-schema.txt` | dieselben acht unter drei Farbschemata |
| `messungen/p5-sitzung-benannter-grund.txt` | sechs Sitzungsbelege, je Zahl mit Grund und Auswahlpfad |
| `messungen/p6-kontrasteffekt.txt` | KWin kennt keinen Kontrasteffekt |
| `messungen/p7-pruefstand.txt` | der Stand, für den diese Zahlen gelten (B17) |
| `messungen/p8-mutationsproben.txt` | Grundlinie und zwölf Proben |
| `bilder/*.png` | zwölf Bilder aus der angemeldeten Sitzung, je Theme Normalfall und Leerzustand |

Die vier Messsonden der Vorprüfung
(`docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/sonden/`) werden **von
dort** gebaut und nicht hierher kopiert: Zwei Fassungen derselben Sonde gehen
auseinander, und dann misst man mit der falschen.

**Was ich hinterlasse:** die Bauplätze `build/` und `build-vorsonden/` in diesem
Ordner sowie `build/` in der Wurzel des Arbeitsbaums — alle drei von
`.gitignore` erfasst, alle drei von `pruefen.sh` bei Bedarf neu angelegt.
Nichts nach `/usr` installiert, keine Einstellung des Kunden geschrieben.
