# Vorprüfung #85 — Messung A (UX)

**Story:** #85 „Capture: Lesbarkeit unter fremden Desktop-Themes" (heute
`size:m`, fortgeschrieben aus dem Schnitt vom 04.08.2026, fünf Kriterien) ·
**Bearbeiter A**, Rolle UI/UX, Modus **Planning-Beratung / Vorprüfung** ·
**Stand dieses Berichts:** 05.08.2026, 20:15 CEST, Ganymed ·
**Boden:** `main` bei `767f2d9`, also der Code **nach** #83. *(Der Zweig ist
während des Laufs auf `989c901` weitergegangen; `git diff 767f2d9..989c901 --
src/ tests/ CMakeLists.txt` ist **leer** — am Code hat sich nichts geändert, die
Messungen gelten unverändert.)*

**Prüfstand** (eine Aussage gilt für einen Stand, B17): kwin 6.7.3-1.1,
ksvg 6.28.0-1.1, qt6-base 6.11.1, libplasma 6.7.3-1.1; Sitzung Wayland,
Bildschirm 2400×1350 logisch, Aufnahmen 3840×2160. **Farbschema der Sitzung
zur Messzeit:** `[Colors:Window]` Background 32,35,38 · ForegroundNormal
252,252,252 · ForegroundInactive 161,169,177 (das ist Breeze Dark, siehe §0.1).
`plasmarc` nennt weiterhin **kein** Desktop-Theme, der Rückfall `default` gilt.

**Wiederholbar:** `bash docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/pruefen.sh`
Der Lauf fasst `build/` der Repositoriumswurzel nicht an, installiert nichts
nach `/usr` und schreibt keine Einstellung des Kunden: seine `kdeglobals` wird
gelesen und in ein eigenes `XDG_CONFIG_HOME` kopiert, seine `plasmarc` gar nicht
gebraucht — die Sonden bekommen den Themenamen übergeben
(`CaptureWindow::reloadDesktopTheme(name)` ist ein öffentlicher Slot).

**Ich habe `messung-b.md` und `messungen-b/` nicht gelesen** — beide sind
während meines Laufs im Ordner aufgetaucht; die Unabhängigkeit der beiden
Messungen ist der Zweck des Verfahrens.

---

## 0. Was die Messung zuerst gezeigt hat

Drei Befunde verschieben die Grundlage der Story, bevor über Kriterien geredet
werden kann. Alle drei sind heute gemessen.

### 0.1 Das Farbschema des Kunden ist ein anderes als das, gegen das #85 rechnet

Jede Kontrastzahl im Issue ist gegen `WindowText` **102,194,242** auf einem
Fenstergrund **30,34,51** gerechnet — das Schema `CachyOSNordLightly`. Die
`kdeglobals` der Sitzung trägt heute **252,252,252** auf **32,35,38** und
`ForegroundInactive` **161,169,177**; `LookAndFeelPackage` steht auf
`org.kde.breezedark.desktop`. Datei und `plasmarc` tragen beide den Zeitstempel
**05.08.2026 17:41** — der Wechsel liegt also nach dem Übergabebericht von
Strang A, der noch „Window 30,34,51" ausweist. *Warum es passiert ist, habe ich
nicht untersucht; es gehört dem PO.*

Was daraus folgt, ist keine Kleinigkeit:

| Zahl im Issue | dort | heute gemessen | Beleg |
|---|---|---|---|
| AK 1, `breeze-light` Schemaschrift | 1,75:1 | **1,11:1** | M1, M4 |
| AK 1, `breeze-light` Themeschrift | 13,35:1 | **13,35:1** | M1 |
| AK 1, `breeze-dark` Schemaschrift | 7,94:1 | **15,39:1** | M1 |
| AK 1, `breeze-dark` Themeschrift | 15,39:1 | **15,39:1** | M1 |
| AK 1, `cachyos-emerald-color` | 10,56 → 9,57 | **20,47 → 9,57** | M1 |
| AK 4, Kleintext deckend | 2,91:1 | **6,64:1** | M4 (`default`) |
| AK 4, Kleintext durchscheinend | 1,79:1 | **7,04:1** über dunklem, **4,06:1** über hellem Grund | M1 |

**Die Themeschrift-Spalte ist stabil, die Schemaschrift-Spalte nicht** — die
eine hängt am Theme, die andere am Schema. Ein Kriterium, das eine Zahl aus der
zweiten Spalte nennt, altert mit jeder Schemawahl des Kunden. Das ist kein
Argument gegen Zahlen im Kriterium, sondern eines dafür, die **Quelle** der Zahl
mitzuschreiben.

### 0.2 Die Quelle, die der Kunde beschrieben hat, gibt es fertig — in einer Bibliothek, die längst verlinkt ist

`KSvg::Svg::color(KSvg::Svg::Text)` liefert bei gesetztem `colorSet(Window)`
**genau** die Regel der Kundenentscheidung: die `ForegroundNormal`-Farbe aus der
`colors`-Datei des Themes, wenn es eine mitbringt, sonst die Schemafarbe.
Gemessen über acht Themes und **drei** Farbschemata (M1, M7):

| Theme | `colors`? | `color(Text)` unter Breeze Dark | unter Breeze Light | unter Nord |
|---|---|---|---|---|
| `default` | nein | 252,252,252 | 35,38,41 | 102,194,242 |
| `CachyOS-Nord-round` | nein | 252,252,252 | 35,38,41 | 102,194,242 |
| `Iridescent-round` | nein | 252,252,252 | 35,38,41 | 102,194,242 |
| `cachyos-emerald` | nein | 252,252,252 | 35,38,41 | 102,194,242 |
| `breeze-dark` | ja | 252,252,252 | **252,252,252** | **252,252,252** |
| `breeze-light` | ja | **35,38,41** | 35,38,41 | **35,38,41** |
| `cachyos-emerald-color` | ja | **0,199,144** | **0,199,144** | **0,199,144** |
| `cachyos-emerald-light` | ja | **35,38,41** | 35,38,41 | **35,38,41** |

Die vier Themes ohne eigene Datei folgen dem Schema, die vier mit eigener Datei
stehen fest. `KF6::Svg` ist an `denkzettelcapture` bereits verlinkt
(`src/CMakeLists.txt:66`), und `m_hull` ist ein `KSvg::FrameSvg` — der Aufruf
kostet keine Zeile Bau und keinen zweiten Leseweg.

**Eine Lücke hat dieser Weg, und sie trifft AK 4.** Die Aufzählung
`KSvg::Svg::StyleSheetColor` (`/usr/include/KF6/KSvg/ksvg/svg.h:176–231`) kennt
je Farbsatz `Text`, `Background`, `Highlight`, `HighlightedText` und drei
Signalfarben — **kein Gegenstück zu `ForegroundInactive`**. Für die gedämpfte
Textklasse (App-Name und Fußzeile) gibt KSvg nichts her; die Farbe steht in
derselben `colors`-Datei und ist mit `KConfigGroup` zu lesen, so wie
`contrastEffectOf()` heute die `[ContrastEffect]`-Gruppe liest.

### 0.3 Seit #83 hängt die Deckung auch am Auswahlpfad — die Tabellen des Issues kennen nur eine der beiden Lagen

`reloadDesktopTheme()` setzt den Auswahlpfad `opaque`, wenn die Sitzung nicht
weichzeichnet (`capturewindow.cpp:283–285`). Damit gibt es je Theme **zwei**
Flächen, und die Zahlen des Issues beschreiben nur die durchscheinende:

| Theme | durchscheinend (Sitzung des Kunden) | `opaque` (kein Weichzeichner, auch: jeder Testlauf) |
|---|---|---|
| `default` | 32,35,38 · 84,7 % | 32,35,38 · **100 %** |
| `breeze-dark` | 32,35,38 · 84,7 % | 32,35,38 · **100 %** |
| `breeze-light` | 240,240,241 · 84,7 % | 239,240,241 · **100 %** |
| `CachyOS-Nord-round` | 30,34,51 · 100 % | unverändert |
| `Iridescent-round` | 0,0,0 · 20 % | unverändert |
| die drei `cachyos-emerald` | 0,0,0 · 2,7 % | **227,227,255 · 3,5 %** |

Die letzte Zeile ist die unangenehme: Der `opaque`-Pfad dreht bei den
Emerald-Themes die **Richtung** der Fläche von schwarz nach hell. Ein Prüfsatz,
der offscreen läuft, misst diese Zeile — der Kunde sieht die andere.

---

## Feld 1 — Dateimenge

Notation nach B13, am Code vermessen (Stand `767f2d9`).

| | **#85** |
|---|---|
| **Issue / Zweig** | #85 (`epic:M1`, `typ:story`) · `story/85-lesbarkeit-fremde-themes` |
| **Quellen und Tests** | `src/capture/capturewindow.cpp` (587 Z.) — **fünf Stellen, alle klein**: `subtleLabel()` `:96–108` (die zweite Textklasse, heute nur eine Rolle), Konstruktor `:205` und `:212/:217` (Aufruf und die beiden Labels), `reloadDesktopTheme()` `:260–299` (**hier gehört die neue Farbe hinein**, neben `m_contrast = …` `:290`), `eventFilter()` `:332–335` (der Palettenzweig — er fasst heute **nur** `m_text` an), `applyTextColours()` `:440–454`. Für den Schemawechsel bei gesetzter Themefarbe kommt entweder ein Zweig in `event()` `:384–399` dazu oder ein `changeEvent()`.<br>`src/capture/capturewindow.h` (167 Z.) — `applyTextColours()` `:141`; ein Feld für die Themefarben neben `m_contrast` `:161`.<br>`tests/capturetest.cpp` (1018 Z.) — `textsFollowAColourSchemeChange()` `:330–360` und `noteTextUsesTheWindowTextRole()` `:563–585` sprechen über **genau die Rollen**, die #85 umlenkt, und sind mitzuziehen; dazu neue Prüfsätze für AK 1 und AK 5.<br>`tests/desktopthemes.h` (164 Z.) — ein Helfer „Theme mit eigener `colors`-Datei", falls der Nachweis nicht an installierten Themes hängen soll (Falle F6).<br>`tests/themes/plasma/desktoptheme/…` — **keines der drei mitgelieferten Prüf-Themes bringt eine `colors`-Datei mit** (gemessen: nur `metadata.json`, bei `denkzettel-test-breit` zusätzlich `metadata.desktop`). Eine solche Datei anzulegen ist derselbe Griff, mit dem #83 die `[ContrastEffect]`-Gruppe mitgeliefert hat. |
| **Build** | **nichts.** Beide gangbaren Wege sind bereits verlinkt: `KF6::Svg` (`src/CMakeLists.txt:66`) für `KSvg::Svg::color()`, `KF6::ConfigCore` (`:63`) für das unmittelbare Lesen der `colors`-Datei. Nur ein dritter Weg über `KColorScheme` bräuchte `KF6::ColorScheme` — den habe ich **nicht** gemessen und schlage ihn nicht vor. |
| **Belege und Prüfmittel** | `docs/scrum/reviews/sprint-NN-s85-lesbarkeit/` mit eigenem `pruefen.sh` nach dem Muster von `sprint-07-s83-native-huelle/`. **Wiederverwendbar, in diesem Ordner fertig gebaut:** `sonden/themetext.cpp` (Schriftquelle und Kontraste je Theme, beide Auswahlpfade, beide Textklassen), `sonden/fenstertext.cpp` (dasselbe am gebauten `CaptureWindow`), `sonden/nebenlauf.cpp` (die Bildsatz-Falle und der Theme-Wechsel), `sonden/sitzungsgrund.cpp` (AK 2: Aufnahme über benanntem Grund, **mit Selbstprüfung des Aufbaus**). Dazu aus #83: `sonden/weichzeichnerbeleg.cpp`, `…/abnahme-befunde/sonden/echtelage.cpp`. |
| **Fachliche Quellen** | **SPEC 3.1** (`:152–205`) — der Absatz sagt heute „Form und Farbe kommen vom Theme" und nennt für den Text weiter `WindowText`/`PlaceholderText` als Rollen des **Schemas**. Genau dieser Satz wird umgedreht und braucht die Bedingung „sofern das Theme keine eigene `colors`-Datei mitbringt". Die Zeile „Issue #85 behandelt das" (`:176–178`) wird eingelöst.<br>**SPEC 3.2** (`:206–274`) — die Bedingungen des nativen Wegs; hier kommt die neue theme-abhängige Größe dazu.<br>**SPEC 16** — Grenzen der Prüfbarkeit: der gesperrte Bildschirm (Falle F1) und der fehlende Kontrasteffekt (F9) gehören dorthin. |
| **Ausdrücklich nicht** | `src/ui/`, `src/shell/`, `src/store/`, `src/capture/textareaheight.*`, `tests/librarytest.cpp`, `tests/libraryshots.cpp`, `tests/editshots.cpp`, `tests/searchshots.cpp`, `tests/readmeshots.cpp`, `wireframes/`, `CLAUDE.md`, `docs/scrum/PROZESS.md`, `.claude/agents/*`. **Und die Wahl der Rolle für die Kleintexte** — das ist #84, nicht diese Story. |

### 1.1 Kollisionsfläche

`capturewindow.cpp` ist wieder die eine Datei, an der sich alles trifft. Offene
Vorgänge mit Abstand null: **#79** (Fenster schrumpft nicht, `present()` und die
Höhenformel) und **#81** (Textkante, Konstruktor `:158 ff.`). #85 schreibt in
den Konstruktor (`:205`, `:212`, `:217`), in `reloadDesktopTheme()` und in den
Ereigniszweig — **#81 liegt drei Zeilen daneben**, innerhalb der Mischbreite von
Git. Empfehlung unverändert wie bei #83: **nicht mit #81 im selben Sprint**,
#79 nur bei getrennten Belegordnern. `tests/captureshots.cpp` fasst #85 nicht an,
solange die Bildreihe nicht um Theme-Bilder erweitert wird; wird sie erweitert,
kollidiert sie mit jedem anderen Capture-Strang.

---

## Feld 2 — Gemessene Fallen

Neun, jede mit Beleg. Die drei teuren stehen zuerst.

### Die drei teuren

**F1 — Ein gesperrter Bildschirm liefert ein schwarzes Bild, und der Fehlschlag
sieht aus wie ein Messergebnis.** AK 2 verlangt die Zahl aus einer Aufnahme der
angemeldeten Sitzung. Bei gesperrtem Bildschirm gibt `spectacle -f -b -n` ein
durchweg schwarzes Bild zurück — mit Rückgabe 0, in voller Auflösung
(3840×2160), ohne jede Fehlermeldung. Die Fenster werden weiter abgebildet, die
Sonde läuft durch, und der Bericht lautet dann: *„Das Fenster hebt sich vom
Grund nirgends ab."* Das ist ein Satz, der wie ein Befund klingt und den
Rollladen misst. *Beleg:* `messungen/m5-sitzung-benannter-grund.txt` — zwei
Aufnahmen, größter Unterschied **0 Zählschritte**, Bildmitte 0,0,0 bei weißem
Grundfenster; `loginctl … LockedHint=yes`. **Zeile für den Auftrag:** Jede
Sitzungssonde prüft zuerst, ob die Aufnahme den eigenen Grund zeigt, und bricht
sonst ab. `sonden/sitzungsgrund.cpp` tut das jetzt (Abbruch mit Rückgabe 3).

**F2 — Ein Vollbildfenster als Prüfgrund liegt über dem Erfassungsfenster.** Der
Bauplan aus #83 zeigt das Schachbrett mit `showFullScreen()`. Bei diesem
Compositor liegt ein Vollbildfenster in einer höheren Ebene als ein
gewöhnliches; das Erfassungsfenster verschwindet darunter, und die Aufnahme
zeigt nur den Grund. *Beleg:* derselbe Lauf, erster Versuch. **Zeile für den
Auftrag:** Der Grund wird als gewöhnliches Fenster in Bildschirmgröße gezeigt
(`resize(screen->size())`, kein `showFullScreen()`); er verdeckt den Schreibtisch
genauso.

**F3 — Im Testmodus sagen Palette und KSvg Verschiedenes über dasselbe
Farbschema.** `capturetest.cpp:157` ruft `QStandardPaths::setTestModeEnabled(true)`.
Danach findet KSvg die `kdeglobals` nicht mehr und rechnet mit der
Qt-Ersatzpalette, während die Anwendungspalette über das KDE-Plattformthema
weiterhin die Farben des Kunden trägt. Gemessen (M6, `default`):

| | ohne Testmodus | mit Testmodus |
|---|---|---|
| Anwendungspalette `WindowText` | 252,252,252 | 252,252,252 |
| `KSvg::Svg::color(Text)` | 252,252,252 | **35,38,41** |
| gezeichnete Fläche | 32,35,38 | **240,240,241** |

Im Testlauf steht der helle Grund von `default` also unter der **dunklen**
Schrift des Kunden — 1,11:1, ein Zustand, den es auf keiner Maschine gibt.
**Zeile für den Auftrag:** Ein Prüfsatz zu AK 1 darf keine Kontrastzahl
zusichern; er sichert die **Herkunft** der Farbe zu (Themefarbe gegen
Palettenfarbe), und die Zahlen entstehen im Messlauf außerhalb von `ctest`.

### Zu KSvg und den Farben

**F4 — Zwei lebende `KSvg::ImageSet` desselben Themenamens teilen ihre
Auswahlpfade, ihre Farben aber nicht.** Der Fund aus Sprint 7 gilt für die
Fläche und **nur** für den gleichen Namen. Gemessen (M3), während das Fenster
`breeze-light` mit `opaque` trägt: ein zweiter Bildsatz desselben Namens meldet
Deckung **255 statt 216**, ein Bildsatz eines **anderen** Themes bleibt
unverändert, und `color(Text)` ist in **allen vier** Fällen richtig. Dieselbe
Messung mit `cachyos-emerald-color` neben `cachyos-emerald-light`: Fläche
verschoben (7 → 9), Farben unberührt (0,199,144 gegen 35,38,41). **Zeile für den
Auftrag:** Wer Flächen vergleicht, gibt jedem Theme einen eigenen Prozess oder
lässt den Bildsatz vorher sterben; wer **Farben** vergleicht, braucht das nicht.

**F5 — Ein Reihenlauf ist unbedenklich, solange kein Bildsatz überlebt.** Acht
Themes nacheinander in einem Prozess liefern Zahl für Zahl dasselbe wie acht
eigene Prozesse (M2, alle 32 Messzeilen Zeichen für Zeichen gleich). Die Falle hängt an
**gleichzeitigem** Leben, nicht an der Reihenfolge — das erspart der Story eine
Prozessschleife für die Messmatrix.

**F6 — Kein mitgeliefertes Prüf-Theme bringt eine `colors`-Datei mit.** Damit
hängt jeder Nachweis zu AK 1 heute an installierten Themes. Der CI-Lauf
installiert `libplasma` (`.github/workflows/ci.yml:70`), und `breeze-light`
samt seiner `colors`-Datei stammt genau daraus (`pacman -Qo`), also liefe ein
Prüfsatz gegen `breeze-light` dort durch. Auf einer Maschine ohne `libplasma`
liefe er nicht. **Zeile für den Auftrag:** Entweder eine `colors`-Datei zu einem
mitgelieferten Theme (der Griff von #83 AK 6) oder ein ausgesprochenes
`QSKIP` — schweigend an installierten Themes zu hängen ist der Fall, der in
Sprint 7 grün blieb und nichts prüfte.

**F7 — `default` folgt dem Schema, `CachyOS-Nord-round` folgt ihm nicht.** Die
Tabelle des Issues führt beide als „folgt dem Schema". Gemessen über drei
Schemata (M1, M7) zeichnet `CachyOS-Nord-round` **immer** 30,34,51 — unter Nord
stimmte das zufällig mit der Schemafarbe überein, und die Messung von damals
lief unter Nord. `default` dagegen wechselt mit: 32,35,38 / 240,240,241 /
30,34,51. **Zeile für den Auftrag:** Eine Aussage „folgt dem Schema" braucht
**zwei** Schemata; unter einem misst man Übereinstimmung, nicht Folgen.

### Zum Fenster und zum Wechsel

**F8 — `KSvg::Svg::color()` zieht beim Theme-Wechsel nach.** Ein `FrameSvg`, dem
nacheinander frische Bildsätze gegeben werden — genau das, was
`reloadDesktopTheme()` tut —, meldet nach jedem Wechsel die Farbe des neuen
Themes, auch beim Zurückwechseln (M3, Abschnitt B, zweimal hin und her).
**Zeile für den Auftrag:** AK 5 braucht keinen zweiten Wächterpfad; die Farbe
liegt an derselben Stelle bereit, an der `m_contrast` schon gesetzt wird
(`capturewindow.cpp:290`).

**F9 — Die Kontrast-Anmeldung hat auf diesem Stand keinen Empfänger.** Strang A
hat gemeldet, dass die **Wirkung** unvermessen sei. Schärfer gemessen (M8):
KWin kennt in seiner Liste von 54 Effekten **keinen einzigen** mit „contrast" im
Namen; `isEffectLoaded("backgroundcontrast")` und `isEffectLoaded("contrast")`
antworten `false`, `isEffectLoaded("blur")` antwortet `true`, und in der
Globalenliste steht `ext_background_effect_manager_v1`. Das heißt für #85: Was
die drei Emerald-Themes bei 2,7 % Deckung lesbar machen soll, **existiert auf
dieser Maschine nicht**. Unter ihnen steht der Text auf dem Bildschirmhintergrund
und auf nichts sonst.

---

## Feld 3 — AK-Urteil: Vorschlag **ready = nein**

*Das Urteil fällt der Scrum Master; dies ist mein Vorschlag mit Begründung.*
Die Kriterien sind gut geschnitten und einzeln prüfbar — sechs Punkte stehen
dagegen, fünf davon sind Zahlen- oder Formulierungsfragen und in Minuten
behoben. Der sechste ist eine Entscheidung.

| AK | Urteil | Begründung |
|---|---|---|
| **1** | **nachbessern** | Der Mechanismus ist prüfbar und gemessen erreichbar (§0.2). Die genannte Belegzahl „1,75:1 → 13,35:1" gilt für ein Schema, das der Kunde nicht mehr fährt; heute lautet sie **1,11:1 → 13,35:1**. Dazu fehlt die Angabe, **welche** Textklasse gemeint ist (siehe Punkt 6 unten) |
| **2** | **nachbessern** | Belegform, Prüfgrund und Werkzeugfalle stehen da — das ist gut. Ungenannt ist die Voraussetzung, an der die Messung heute gescheitert ist: **der Bildschirm muss entsperrt sein** (F1), und der Fehlschlag ist stumm. Ebenfalls ungenannt: der Grund darf **kein Vollbildfenster** sein (F2) |
| **3** | **nachbessern** | Zwei Sachfehler in der Grundlage: `CachyOS-Nord-round` folgt dem Schema nicht (F7), und die Deckung hat seit #83 **zwei** Werte je Theme — durchscheinend und `opaque` (§0.3). Ohne diese Achse ist „84,7 %" mal richtig, mal falsch |
| **4** | **nachbessern** | Prüfbar, und die Grenze ist richtig ausgesprochen. Die genannte Ausgangslage (2,91:1 / 1,79:1) ist die des alten Schemas; heute **6,64:1** deckend unter `default`. Der Satz „über 19 Schemata liegen 16 unter 4,5:1" bleibt gültig — er hängt nicht an der Wahl des Kunden |
| **5** | **ready** | Sauber formuliert, Prüfweg vorhanden, und gemessen erfüllbar (F8). Das einzige Kriterium der Story, an dem ich nichts auszusetzen habe |
| **6** | **ready** | Mutationsprobe je tragender Zusicherung, Aufzählung im Bericht — die Fassung, die #83 nach der Berichtigung trägt |

**Der sechste Punkt, und er ist der einzige, der eine Entscheidung braucht:**
Die Kundenentscheidung lautet „**die Schrift** kommt aus derselben Quelle wie
die Fläche". Das Fenster hat **zwei** Schriften. AK 1 spricht nur vom Notiztext;
AK 4 verlangt für die gedämpfte Klasse eine Zahl und eine benannte Grenze,
sagt aber nicht, aus welcher Quelle ihre Farbe kommt. Damit ist offen, ob nach
dieser Story App-Name und Fußzeile weiterhin aus dem Schema stammen, während der
Notiztext aus dem Theme kommt — zwei Schriften aus zwei Quellen auf einer
Fläche. Gemessen wäre der Unterschied (M1, durchscheinend, Kleintext):

| Theme | Kleintext aus dem Schema | Kleintext aus dem Theme |
|---|---|---|
| `breeze-light` | 2,09:1 | **3,70:1** |
| `breeze-dark` | 6,64:1 | 6,64:1 (dieselbe Farbe) |
| `cachyos-emerald-color` | 8,83:1 über dunklem, 2,24:1 über hellem Grund | 3,87:1 / 5,10:1 |
| `cachyos-emerald-light` | 8,83:1 / 2,24:1 | 4,99:1 / 3,96:1 |

Keine der beiden Quellen gewinnt überall, und **keine erreicht 4,5:1 unter
`breeze-light`** — das bleibt #84. Die Frage hier ist allein, ob die Regel
„dieselbe Quelle wie die Fläche" für beide Klassen gilt. **Vorlage an den
Kunden gehört dem PO.**

---

## Feld 4 — Prüfmittel, und was ein Agent nicht prüfen kann

| AK | Prüfmittel | Grenze |
|---|---|---|
| **1** | QTest gegen ein Theme **mit** und eines **ohne** eigene `colors`-Datei: die Farbe des Notiztextes gleicht der Themefarbe im ersten und der Palettenfarbe im zweiten Fall. Dazu die Zahlenreihe aus `sonden/themetext.cpp` (acht Themes, beide Auswahlpfade) als Messlauf im Belegordner | **Kontrastzahlen sind im Testlauf nicht zusicherbar** (F3): dort weichen Palette und KSvg-Grundlage auseinander. Und ohne `libplasma` gibt es kein installiertes Theme mit `colors` (F6) — ohne mitgeliefertes Prüf-Theme hängt der Nachweis daran |
| **2** | `sonden/sitzungsgrund.cpp` — gewöhnliches Grundfenster in Bildschirmgröße, zwei Aufnahmen (mit und ohne Erfassungsfenster), Fenster über den Unterschied gefunden, nur der Ausschnitt abgelegt, Selbstprüfung des Aufbaus | **Der Bildschirm muss entsperrt sein, und das kann ein Agent nicht herstellen** (F1). Die Messung ist damit an einen Zeitpunkt gebunden, den ein Mensch setzt. Zusätzlich: bei 2,7 % Deckung ist der Kontrast **kein Wert, sondern ein Bereich** — gerechnet über weißem gegen schwarzem Grund liegt die Themeschrift zwischen **14,32:1** und **1,38:1** (M1). Eine Zahl ohne Grund beschreibt nichts, und das steht im Kriterium schon richtig |
| **3** | `sonden/themetext.cpp` über acht Themes × **zwei** Schemata × zwei Auswahlpfade (M1, M7); reine Messung, kein Codeanteil | „Folgt dem Schema" ist unter **einem** Schema nicht messbar (F7). Der Themebestand ist der dieser Maschine — auf einer anderen sind es andere acht |
| **4** | `sonden/fenstertext.cpp` liest beide Textklassen am gebauten Fenster und rechnet die Kontraste; die Aussage über 19 Schemata steht in `ux-beratung/messungen/m2-kleintexte.txt` von #83 | Behoben wird hier nichts (steht im Kriterium). Die Zahl je Theme hängt am Farbschema des Kunden und ist damit ein Wert **seines Standes**, nicht des Codes |
| **5** | QTest: Theme mit `colors` → Theme ohne → zurück, je die Farbe des Notiztextes gelesen (`reloadDesktopTheme(name)` ist öffentlich). Der Weg ist gemessen gangbar (F8) | keine. Der Fehlschlag wäre auffällig: ohne die Zeile bleibt die Farbe stehen |
| **6** | Heilung entfernen, Rotwerden zeigen, Aufzählung im Übergabebericht | keine — vorausgesetzt, jede Probe läuft in der Lage, in der ihr Fehler **auftreten kann**. Für den Auswahlpfad heißt das offscreen, für ein Sitzungsbild in der Sitzung |

**Was ein Agent an dieser Story grundsätzlich nicht prüfen kann:**

1. **Einen entsperrten Bildschirm herstellen** (F1). Gemessen am 05.08.2026:
   die Sitzung war während der gesamten Vorprüfung gesperrt, und jede Aufnahme
   war schwarz. Der AK-2-Beleg braucht ein Zeitfenster, das der Kunde öffnet.
2. **Die Wirkung des Kontrasteffekts zeigen** (F9) — er ist auf diesem Stand
   nicht abgeschaltet, sondern **nicht vorhanden**. Diese Grenze erbt #85 von
   #83 und kann sie nicht schließen; sie ist der Grund, warum die drei
   Emerald-Themes von keiner Schriftwahl gerettet werden.
3. **Die Skalierung des Kunden wählen** (aus #83, F4 dort): sie ist vorzufinden,
   `QT_SCALE_FACTOR` multipliziert unter Wayland.
4. **Beurteilen, ob es gut aussieht.** Für zwei Schriften aus zwei Quellen auf
   einer Fläche gibt es keine Zahl, die das entscheidet.

---

## Feld 5 — Größenklasse: **`size:m`**

Das fortgeschriebene Label bestätige ich, jetzt gemessen statt vermutet.

**Warum nicht `size:s`:**

- Ein **Mechanismus, den es heute nicht gibt** — die Textfarbe hatte bisher
  keine zweite Quelle, und `applyTextColours()` kennt nur die Palette.
- **Zwei Textklassen an zwei verschiedenen Stellen**: der Notiztext über eine
  gesetzte Farbe (`:448`), die Kleintexte über eine Rolle (`:105`). Die zweite
  hat heute **gar keinen** Aktualisierungspfad, weil eine Rolle keinen braucht;
  eine gesetzte Farbe braucht ihn.
- **Ein neuer Prüfweg mit menschlicher Voraussetzung** (AK 2, F1).
- **Ein neues Prüf-Gut**: ohne `colors`-Datei bei einem mitgelieferten Theme
  hängt AK 1 an der Paketlage der Maschine (F6).
- Vier Kriterien tragen Zahlen, die vor dem Bauen neu zu erheben sind (§0.1).

**Warum nicht `size:l`:**

- **Am Bau ändert sich nichts** — beide Wege sind verlinkt (§0.2). Das war bei
  #83 der Punkt, an dem die Wertung kippte.
- **Der Code liegt in einer Datei**, in fünf kleinen Stellen, und die größte
  davon ist ein Aufruf neben einem vorhandenen (`:290`).
- **Die Messwerkzeuge stehen fertig** — vier Sonden in diesem Ordner, gebaut und
  gelaufen; die Messmatrix ist einmal komplett erhoben.
- **Kein offener Compositor-Punkt gehört dieser Story**: F9 ist eine geerbte,
  benannte Grenze und keine Aufgabe.

*Die Ein-Stufen-Regel greift, falls Bearbeiter B auf `l` kommt; dann gälte `l`.*

---

## Feld 6 — Offene Fragen

### An den PO

1. **Gilt „dieselbe Quelle wie die Fläche" für beide Textklassen?** Feld 3,
   Punkt 6. Ohne Antwort baut der Strang entweder zu wenig oder mehr als
   entschieden. Zahlen für die Vorlage stehen dort.
2. **Die Zahlen in #85 und #84 ruhen auf einem Farbschema, das die Sitzung
   nicht mehr trägt** (§0.1). Betroffen sind #85 AK 1 und AK 4 sowie die
   Ausgangslage von #84 (2,91:1 → heute 6,64:1). Melden, nicht heilen: Ich habe
   an keinem Issue etwas geändert.
3. **Zwei Sachfehler in der Tabelle von #85** (F7 und §0.3): `CachyOS-Nord-round`
   folgt dem Schema nicht, und die Deckung hat seit #83 zwei Werte je Theme.
   Beide stehen im Issue-Text, nicht nur im Kriterium.
4. **Bestandsbefund, unverändert offen und weiterhin nicht gebucht:** die
   Schattenpolsterung wird in `bindShadow()` gesetzt und bei `resizeHull()`
   nicht nachgezogen (UX §6.6 zu #83, im Übergabebericht von Strang A als Punkt
   3 erneut gemeldet). Ich melde ihn ein drittes Mal, weil er beim Vermessen
   derselben Datei wieder sichtbar wurde.
5. **Zwei Fallen gehören in die Sammlung, die eine Sitzung von selbst liest**
   (`.claude/agents/denkzettel-dev.md`, Liste „Läufe, die nichts belegen"): der
   schwarze Schirm bei gesperrter Sitzung (F1) und das Vollbildfenster über dem
   Prüfgegenstand (F2). Beide Male ist der Lauf grün und die Aussage falsch.
   Die Datei liegt außerhalb meiner Fläche.

### An den Kunden — nur über den PO

**Vorlage 1 (kurz):** *„Dein Erfassungsfenster hat zwei Schriften: den Notiztext
und die beiden kleinen, gedämpften Zeilen (App-Name, Fußzeile). Deine
Entscheidung ‚die Schrift kommt aus derselben Quelle wie die Fläche' haben wir
für den Notiztext eingeplant. Soll sie auch für die kleinen Zeilen gelten? Unter
Breeze hell würden sie dadurch von 2,09:1 auf 3,70:1 steigen — besser, aber
weiterhin unter dem Mindestwert 4,5:1, an dem eigenes Issue #84 arbeitet."*

### Zum Anker in den Belegen von #83

Der Übergabebericht von Strang A hält fest, dass `native-huelle-nord.txt` unter
dem **Farbschema** von Nord lief und nicht unter dessen Desktop-Theme. Meine
Messung F7 ist der zweite Fall derselben Verwechslung, diesmal in der Tabelle
von #85 selbst. Beide Male entsteht sie dadurch, dass ein Theme und ein Schema
denselben Namen tragen.

---

## Belege dieses Berichts

Alle unter `docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/`:

| Datei | Was |
|---|---|
| `messungen/m1-schriftquelle-je-theme.txt` | acht Themes, je eigener Prozess: `colors`-Datei, `KSvg::Svg::color()`, beide Auswahlpfade, beide Textklassen, Kontraste deckend und über hellem/dunklem Grund |
| `messungen/m2-reihenlauf-gegen-einzellauf.txt` | Reihenlauf gegen Einzelläufe — 32 Messzeilen, kein Unterschied (F5) |
| `messungen/m3-nebenlauf-und-wechsel.txt` | die Bildsatz-Falle (F4) und der Theme-Wechsel der Farbe (F8) |
| `messungen/m4-fenster-heute.txt` | die Ausgangslage am gebauten `CaptureWindow` über acht Themes |
| `messungen/m5-sitzung-benannter-grund.txt` | der Sitzungslauf, der an der gesperrten Sitzung gescheitert ist (F1, F2) |
| `messungen/m6-testmodus.txt` | Palette gegen KSvg-Grundlage im Testmodus (F3) |
| `messungen/m7-folgt-dem-schema.txt` | zwei Schemata über acht Themes (F7) |
| `messungen/m8-kontrasteffekt.txt` | KWin kennt keinen Kontrasteffekt (F9) |
| `sonden/*.cpp`, `sonden/CMakeLists.txt` | die vier Sonden, wiederverwendbar für die Umsetzung |
| `pruefen.sh` | fährt M1–M8 in einem Zug |
| *(kein `bilder/`)* | Der einzige vorgesehene Bildbeleg — AK 2 über benanntem Grund — ist an F1 gescheitert, und ein leerer Ordner lässt sich nicht versionieren. `pruefen.sh` legt ihn an |

**Was ich hinterlasse:** den Bauplatz `build-vor85` in der Repositoriumswurzel
(unversioniert; `.gitignore` erfasst ihn seit dem Zusatz `build-*/`) und `…/85-lesbarkeit-fremde-themes/build/`
(von `.gitignore` erfasst). `pruefen.sh` legt beide bei Bedarf neu an; der PO
kann sie löschen.

**Was ich nicht angefasst habe:** Produktivcode, Tests, SPEC, Issues, Labels,
und keine Einstellung des Kunden. Der einzige Schreibzugriff außerhalb dieses
Ordners war der Bauplatz.
