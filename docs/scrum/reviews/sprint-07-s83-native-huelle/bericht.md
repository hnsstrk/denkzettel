# Übergabebericht #83 — Capture: Hülle als native Plasma-Überlagerung

**Story:** #83, „der native Vertrag" (`size:l`, 14 Akzeptanzkriterien) ·
**Strang A, Sprint 7** · **Zweig:** `story/83-native-huelle`, Ausgangsstand
`sprint-07-basis` · **Stand dieses Berichts:** 05.08.2026, Ganymed ·
**Grundlage:** `docs/scrum/vorberichte/83-native-huelle/bericht.md`

**Wiederholbar:**
`bash docs/scrum/reviews/sprint-07-s83-native-huelle/pruefen.sh` ·
`bash …/mutationsproben.sh` · `bash …/mutationsproben-sitzung.sh`
Keines der drei fasst `build/` der Repositoriumswurzel an, keines installiert
nach `/usr`, keines liest oder verstellt eine Einstellung des Kunden.

**Prüfstand** (eine Aussage gilt für einen Stand, B17): kwin 6.7.3,
kwindowsystem 6.28.0, ksvg 6.28.0, qt6-base 6.11.1, plasma-desktop 6.7.3;
Sitzung Wayland, Bildschirm 2400×1350 logisch, Fenster-Bildpunktverhältnis
**1,6**; Desktop-Theme `default` (der Rückfall, `plasmarc` nennt keins),
Farbschema des Kunden (dunkel, `Window` 30,34,51).

---

## 1. Was umgesetzt ist

Der Nachbau der Hülle ist entfernt. An seine Stelle tritt die Grafik des
Desktop-Themes, in **einem** Stück gezeichnet (`FrameSvg::framePixmap()`) beim
Bildpunktverhältnis des Fensters — und daneben die beiden Anmeldungen, die
`libPlasmaQuick` für Plasmas eigene Überlagerungen macht.

| Datei | Was |
|---|---|
| `src/capture/capturewindow.h` | `capture::ContrastEffect` und die zwei freien Funktionen `contrastEffectOf()` / `sessionBlursBehindWindows()`; `hullDevicePixelRatio()` als benannte Ersatzform; `m_hullInner` entfällt |
| `src/capture/capturewindow.cpp` | `tinted()`, `mixed()`, `FrameContrast`, `OutlineWidth` entfallen · `paintEvent()` zeichnet in einem Stück · `resizeHull()` setzt das Verhältnis · neuer `event()`-Zweig für `DevicePixelRatioChange` · `bindWindowEffects()` · Auswahlpfad `opaque` in `reloadDesktopTheme()` |
| `src/CMakeLists.txt` | `Qt6::DBus` an `denkzettelcapture` (Begründung §3) |
| `tests/capturetest.cpp` | vier Zusicherungen neu gefasst, sechs Prüfsätze neu |
| `tests/desktopthemes.h` | `bundledSquare()` — sonst zählt das überführte Prüf-Theme als installiertes (§5, Fund 1) |
| `tests/themes/…/denkzettel-pruef-eckig/` | aus dem Belegordner der Sprint-6-Abnahme überführt (AK 9) |
| `tests/themes/…/denkzettel-test-breit/metadata.desktop` | neu: eine `[ContrastEffect]`-Gruppe, damit AK 6 nicht an installierten Themes hängt |
| `SPEC.md` | 3.1 umgedreht, 3.2 um vier Bedingungen erweitert, 15 und 16 nachgezogen (AK 2) |

**Nicht angefasst:** `src/ui/`, `src/shell/`, `src/store/`,
`src/capture/textareaheight.*`, `tests/librarytest.cpp`, `tests/libraryshots.cpp`,
`tests/editshots.cpp`, `tests/searchshots.cpp`, `tests/readmeshots.cpp`,
`wireframes/`, `CLAUDE.md`, `docs/scrum/PROZESS.md`,
`.claude/agents/denkzettel-dev.md`. `tests/captureshots.cpp` läuft unverändert
weiter und brauchte keine Änderung.

**Zwei Dateien außerhalb der Fläche sind angefasst worden, und zwar genau
soweit, wie die Überführung des Prüf-Themes sie kaputtgemacht hat:**
`docs/scrum/reviews/2026-08-04-abnahme-befunde/pruefen.sh` und
`docs/scrum/vorberichte/83-native-huelle/pruefen.sh` zeigten mit
`XDG_DATA_DIRS` auf den alten Ort; beide tragen jetzt `tests/themes`. Dazu vier
Zeilen in der `LIESMICH.md` des Abnahmeordners, die den neuen Ort nennen — die
Aussage stehenzulassen hätte aus einer Begründung eine Falle gemacht (B17).

## 2. Die Akzeptanzkriterien, einzeln

| AK | Prüfmittel | Ergebnis |
|---|---|---|
| **1** Zeichnung in einem Stück | `git grep -n "tinted\|FrameContrast\|m_hullInner\|OutlineWidth" -- src/` | **leer**. Mutationsprobe 1 und 4 rot |
| **2** SPEC nachgezogen | Lesen gegen den Code | 3.1 umgedreht, 3.2 Punkte 6–9 neu, 15 und 16 ergänzt |
| **3** Verhältnis folgt dem Fenster | `hullFollowsTheWindowPixelRatio` + Kindlauf bei 1,6; Sitzung `m5` | Sitzung **ohne `QT_SCALE_FACTOR`**: Fenster 1,6 / Hülle 1,6 — gleich. Mutationsprobe 2 rot, Sitzungsprobe S1 zeigt 2 gegen 1,6 |
| **4** kein Stufenlauf | `hullHasNoStairAtTheCorner`; `m1`, `m5` | `default` bei 1: `3·2·1·0`, **0** Stufen · bei 1,6: `7·4·3·2·1·1·1·0`, **1** Stufe, fallend. Anstieg streng steigend bis zum Randwert 235 (`2·65·125·173·195·209·235`). Sitzungsbild bei 1,6 liegt vor |
| **5** Weichzeichner angemeldet | A/B am Bildschirm, `m6` | Spannweite im Innenstreifen **39 ohne / 6 mit** Anmeldung. Nach `hide()`/`showCapture()`: 6 — die Anmeldung überlebt. **Größenänderung: die Region folgt** (Hülle im Bild 967×278 → 967×375, Spannweite bleibt 6). Der geordnete Fehlschlag ist **nicht** eingetreten |
| **6** Kontrasteffekt aus der Theme-Gruppe | `readsTheContrastEffectOfTheDesktopTheme`; `m8` | `default` ohne Gruppe → nicht angemeldet; `Iridescent-round`, `cachyos-emerald`, `cachyos-emerald-color` mit Gruppe → angemeldet. Mutationsprobe 6 rot. **Grenze:** dass er *wirkt*, ist unvermessen (§4) |
| **7** `opaque` ohne Weichzeichner-Sitzung | `takesTheOpaqueVariantWithoutABlurringCompositor`; `m1` | `default` 84,7 % → **100 %**. Mutationsproben 7 und 8 rot. Ermittlungsweg §3 |
| **8** Flächenfarbe folgt dem Schema | `m7`, ein Prozess je Schema | **0 von 19** Farbschemata über der Toleranz von einem Zählschritt |
| **9** eckiges Theme, eckige Ecken | `squareThemeKeepsSquareCorners`; `m5` | `cornerRun` 0, Kantenlauf `0·0·0…`, Alpha ab Spalte 0 bei 255 — **in der Sitzung bei 1,6**. Mutationsprobe 5 rot |
| **10** Theme-Wechsel im Betrieb | `m5`, zwei Themes | `default` → `CachyOS-Nord-round`: Innenrand 16 → 20, Region 600×174 → 600×182, Schattenkachel 32×16 → 20×24, Deckung 216 → 255 |
| **11** Hülle plattformgleich | `m1`/`m2`/`m3`, Prüfsummen je Zeile | **Kein Unterschied** über 11 Themes × 2 Auswahlpfade × 2 Verhältnisse |
| **12** Bild neben KRunner | `m11`, `bilder/sitzung/fenster-neben-krunner.png` | liegt vor, Sitzung, 1,6. **Beurteilung: Kundenabnahme** |
| **13** heller Streifen am Bogen | `m10`, `bilder/sitzung/ecke-am-bildschirm.png` | Grund 128, hellster Bildpunkt am Bogen **128** — kein Bildpunkt heller als der Grund. Der Schnitt läuft 128→127→…→116→100→79→63→49 durch. **Erfüllt**, die bisherige Erklärung bleibt stehen |
| **14** Mutationsprobe je Zusicherung | `m12`, `m14` | **fünfzehn Proben, einzeln aufgezählt in §6.** Vierzehn belegt, eine als Grenze benannt |

## 3. Die beiden Bauentscheidungen, die die Story dem Strang überlassen hat

**Die Kontrastwerte werden selbst gelesen, `libPlasma` kommt nicht hinzu.**
`Plasma::Theme` liest die Gruppe `[ContrastEffect]` aus der `metadata.desktop`
des Themes — einer gewöhnlichen KConfig-Datei. Für vier Zahlen zöge die
Abhängigkeit QtQuick in ein Widgets-Programm. `capture::contrastEffectOf()`
liest dieselbe Datei und dieselbe Gruppe mit `KConfigGroup`; `KF6::ConfigCore`
ist längst verlinkt. **Am Build hat sich dafür nichts geändert.**

**Die Bedingung von AK 7 wird bei KWin selbst erfragt, über D-Bus.** Das
Kriterium nennt die Globalenliste des Compositors und lässt den Weg frei,
solange er **vor** der ersten Anmeldung ein zutreffendes Ergebnis liefert.

- `isEffectAvailable(BlurBehind)` scheidet aus: In der Sitzung des Kunden
  liefert er vor der ersten Anmeldung `false` (Vorprüfung, F9).
- Die Globalenliste unmittelbar zu lesen, hieße `wayland-client` zu verlinken —
  eine neue harte Abhängigkeit, die auch in `.github/workflows/ci.yml`
  nachzuziehen wäre. Das liegt außerhalb der Fläche dieser Story.
- Gewählt: `org.kde.kwin.Effects.isEffectLoaded("blur")`. **Qt6::DBus ist
  keine neue Abhängigkeit des Projekts** — SPEC 15 führt sie, `denkzettelshell`
  verlinkt sie, der CI-Container hat sie. Neu ist allein, dass
  `denkzettelcapture` sie ebenfalls verlinkt.
- **Gleichwertigkeit, gemessen:** KWin bietet
  `ext_background_effect_manager_v1` genau dann an, wenn der Effekt geladen ist.
  Auf diesem Stand sagen beide Wege dasselbe — die Globalenliste der Vorprüfung
  (`sonde5-stand-und-globale.txt`) zeigt die Erweiterung, die D-Bus-Antwort
  lautet `true`, und der Lauf `m5` bestätigt es aus dem laufenden Fenster
  heraus („Weichzeichnende Sitzung: ja").
- Vorgeschaltet ist eine Frage ohne Rückfrage: Ist die Plattform weder Wayland
  noch X11, wird gar nicht erst gefragt. Das hält den Testlauf **deterministisch** —
  sonst hinge sein Ergebnis daran, ob zufällig eine Plasma-Sitzung
  danebensteht. Der Bus wird mit 1 s Zeitgrenze gefragt: Ein Fenstersystem,
  das nicht antwortet, zeichnet auch nicht weich.

## 4. Grenzen der Prüfbarkeit — benannt, nicht weggelassen

1. **Ob `enableBackgroundContrast` wirkt, ist unvermessen.**
   `org_kde_kwin_contrast_manager` steht nicht mehr in der Globalenliste dieses
   Compositors. Der **Aufruf** ist belegt (Mutationsprobe 6, `m8`), die
   **Wirkung** nicht. In SPEC 16 aufgenommen. *Das ist eine Eigenschaft des
   Compositor-Standes, keine des Codes* — und es ist keine Grenze, die dieser
   Strang schließen kann: Es gibt auf dieser Maschine keinen Weg, den Effekt zu
   beobachten. **Als Impediment an den PO: §7, Punkt 1.**
2. **Der Farbsatz der Hülle ist richtig gesetzt und nicht messbar.**
   `m_hull->setColorSet(KSvg::Svg::Window)` ist das, was Plasma für einen
   Dialoggrund tut. Die Mutationsprobe 9 bleibt grün — **und die Messung sagt,
   warum:** Von elf Themes auf dieser Maschine unterscheidet **keines** die
   sieben Farbsätze (`m1`, letzter Abschnitt). Es ist keine ungeprüfte
   Behauptung, sondern eine, an der auf diesem Bestand nichts hängt.
   **Als Impediment an den PO: §7, Punkt 2.**
3. **Das Urteil „sieht das nativ aus" fällt kein Prüfmittel.** AK 12 beschafft
   das Bild; die Beurteilung gehört in die Abnahme. Diese Story kann mit allen
   vierzehn Kriterien grün sein und trotzdem abgelehnt werden.
4. **Der Plattformvergleich ist ein datierter Doppellauf, kein laufender
   Test.** Ein QTest läuft in einer Plattform, der CI-Lauf hat keinen
   Compositor. `m3` ist die Ausgabe eines Laufs vom 05.08.2026.

## 5. Zwei Funde, die die Arbeit selbst hervorgebracht hat

**Fund 1 — Die Überführung des Prüf-Themes hat drei Zusicherungen umgeworfen,
und zwar in die stille Richtung.** `themes::installedThemes()` kannte die
mitgelieferten Themes beim Namen und schloss zwei davon aus; das dritte, gerade
überführte, zählte es als installiert. Nach Namen sortiert steht
`denkzettel-pruef-eckig` vorn, also reichte `anyInstalledTheme()` ein
**eckiges** Theme an drei Zusicherungen weiter, die ein rundendes brauchen.
Gefangen hat es der Testlauf (`narrowCorner > 0` fiel), nicht das Nachdenken.
Behoben mit `themes::bundledSquare()`.

**Fund 2 — Zwei lebende `KSvg::ImageSet` desselben Themenamens teilen ihre
Auswahlpfade.** Der Prüfsatz zu AK 7 sollte die durchscheinende Fassung neben
die deckende stellen; solange das Erfassungsfenster seine eigene `ImageSet` mit
`opaque` am Leben hält, meldet eine zweite Instanz **ebenfalls `opaque`** und
zeichnet danach — ohne dass ihr jemand einen Auswahlpfad gegeben hätte.
Gemessen in vier Schritten (`m13`): allein 216, lebende mit `opaque` 255, zweite
daneben 255, nach ihrem Ende wieder 216. **Der Prüfsatz suchte danach sein
Theme, während das Fenster ein mitgeliefertes trägt.**

*Warum das hierhergehört:* Beide Funde sind Fälle, in denen ein Lauf grün
gewesen wäre und nichts geprüft hätte. Fund 2 ist ein Kandidat für die Liste
„Rückgabewerte und Läufe, die nichts belegen" in
`.claude/agents/denkzettel-dev.md` — **eingetragen wird er dort nicht von
diesem Strang**, die Datei liegt außerhalb der Fläche (§7, Punkt 4).

## 6. Die Mutationsproben, gezählt (AK 14)

Fünfzehn tragende Zusicherungen, je eine Probe. Zwölf offscreen
(`mutationsproben.sh` → `m12`), drei in der angemeldeten Sitzung
(`mutationsproben-sitzung.sh` → `m14`), weil ihr Fehler offscreen gar nicht
auftreten kann.

| # | Zusicherung | Eingriff | Ergebnis |
|---|---|---|---|
| 1 | Die Hülle ist die Grafik des Themes | Fläche mit der Palettenfarbe füllen | **rot** — 4 Prüfsätze |
| 2 | Die Hülle trägt das Verhältnis des Fensters | `setDevicePixelRatio()` entfernen | **rot** — 2 Prüfsätze |
| 3 | Die Hülle wird auf die Fenstergröße gebracht | `resizeFrame()` entfernen | **rot** — 3 Prüfsätze |
| 4 | Gezeichnet wird `framePixmap()`, nicht `alphaMask()` | Aufruf tauschen | **rot** — 2 Prüfsätze |
| 5 | Die Eckform kommt aus dem Theme | eigener Radius von 12 | **rot** — 7 Prüfsätze |
| 6 | Die Kontrastwerte kommen aus der Theme-Gruppe | Gruppenname verstellen | **rot** |
| 7 | Ohne Weichzeichner-Sitzung gilt `opaque` | `setSelectors()` entfernen | **rot** |
| 8 | Ohne Compositor wird nicht gefragt | Plattformwache entfernen | **rot** |
| 9 | Der Farbsatz ist der eines Dialoggrundes | `setColorSet()` entfernen | **grün** — benannte Grenze, §4 Punkt 2 |
| 10 | Verhältnis wird bei `DevicePixelRatioChange` nachgezogen | Zweig abschalten | offscreen **grün** → Sitzung S1 |
| 11 | Die Anmeldungen bekommen kein `nullptr`-Fenster | Wache entfernen | offscreen **grün** → Sitzung S2 |
| 12 | Der Weichzeichner wird angemeldet | Aufruf entfernen | offscreen **grün** → Sitzung S3 |
| S1 | dieselbe wie 10, in der Sitzung | Zweig abschalten | **Hülle 2, Fenster 1,6 — „VERSCHIEDEN"** |
| S2 | dieselbe wie 11, in der Sitzung | Wache entfernen | **Rückgabe 139, Signal 11 (SIGSEGV)** |
| S3 | dieselbe wie 12, in der Sitzung | Aufruf entfernen | **Spannweite 39 statt 6** — der Grund bleibt scharf |

**Vierzehn von fünfzehn belegt, eine benannt.** Der erste Lauf von `m12` hatte
Probe 7 grün — daran ist der Prüfsatz zu AK 7 aufgefallen und nachgebessert
worden (§5, Fund 2). Die Zahlen oben stammen aus dem Lauf **danach**.

## 7. Was offen ist und dem Product Owner gehört

1. **Impediment:** Die Wirkung von `enableBackgroundContrast` ist auf diesem
   Compositor-Stand nicht zu beobachten (§4 Punkt 1). Der Strang kann die
   Grenze nicht schließen. Entweder wird sie als Eigenschaft des Standes
   hingenommen — sie steht in SPEC 16 — oder es braucht eine eigene
   Untersuchung, wie `ext_background_effect_manager_v1` den Kontrast trägt.
2. **Impediment, klein:** Für `setColorSet(Window)` gibt es auf diesem
   Themenbestand keinen Prüfsatz, der fallen könnte (§4 Punkt 2). Schließbar
   nur mit einem mitgelieferten Prüf-Theme, das die Farbsätze unterscheidet —
   das wäre neuer Umfang und gehört entschieden, nicht nebenbei gebaut.
3. **Melden, nicht heilen:** Der Bestandsbefund der Vorprüfung ist unverändert
   offen — die Schattenpolsterung wird in `bindShadow()` gesetzt und bei
   `resizeHull()` nicht nachgezogen, obwohl das Fenster seine Höhe im Betrieb
   ändert (UX §6.6). Er ist beim Bauen erneut sichtbar geworden und weiterhin
   nicht gebucht.
4. **Melden, nicht heilen:** Die drei B17-Fundstellen zu `tinted()`
   (`CLAUDE.md`, `PROZESS.md`, `.claude/agents/denkzettel-dev.md`) sind jetzt
   fällig — `tinted()` ist gefallen. Dazu gehört der neue Fall aus §5, Fund 2
   in die Liste der Läufe, die nichts belegen. Alle drei Dateien liegen
   außerhalb der Fläche dieses Strangs.
5. **Für die Abnahme-Checkliste:** Der **Schatten ist ausdrücklich nicht
   enthalten** (Issue-Text), und das Bild neben KRunner will beurteilt werden.
6. **Zur Erinnerung an die Kehrseite:** Nach #83 allein sind sechs von acht
   Themes schlechter lesbar als zuvor. Auf der Einstellung des Kunden ändert
   sich nichts (Rückfall `default`); #85 heilt den Rest.
7. **Ein Detail an der Messgrundlage der Vorprüfung**, ohne Folgen für die
   Story: `native-huelle-nord.txt` und `native-huelle-nord-wayland.txt` tragen
   im Kopf `Theme: default` — sie sind unter dem Farbschema von Nord gelaufen,
   nicht unter dessen Desktop-Theme. Die Tabelle in §0.2 des Vorprüfberichts
   liest sie als zwei Themes; gemessen ist ein Theme, dreimal. Die Zahlen für
   `default` stimmen und sind hier bestätigt; `CachyOS-Nord-round` läuft
   tatsächlich `8·6·4·3·2·2·1·1·0·0` bei 1 und `14·11·9·8·6·5·4·4·3·2` bei 1,6
   (`m1`). **Das ist der Grund, warum der Prüfsatz zu AK 4 gegen `default`
   läuft und nicht gegen ein beliebiges Theme:** Die Stufenzahlen von AK 4 sind
   die dieses einen Themes.

## 8. Selbst-Sichtprüfung (DoD 2 und 3)

Am **gebauten** Stand, nicht am installierten — den taktet der PO.

- `cmake --build build` — 0 Warnungen.
- `ctest --test-dir build` — **7 von 7 grün** (`m9`), darunter `capturetest`
  mit 27 Prüfsätzen.
- `cmake --build build --target lint-clazy` — **3 Befunde**, unverändert der
  Altbestand vom 04.08.2026 (2× range-loop-detach, 1× detaching-temporary).
  Zwei eigene Befunde („non-POD static") sind vor dem Commit beseitigt worden.
- Hauptweg der Story selbst ausgeführt: Erfassungsfenster in der angemeldeten
  Sitzung geöffnet, getippt, gewachsen, Theme gewechselt, neu geöffnet —
  Protokoll `m5`, Bilder unter `bilder/sitzung/`.
- Bilder je Zustand: Ruhe, acht Zeilen (im Weichzeichner-Lauf `groesse`),
  eckiges Theme, beide Themes des Wechsels, Ecke am Bildschirm, Fenster neben
  KRunner. Alle aus der Sitzung bei **1,6**, ohne `QT_SCALE_FACTOR`.
- Journal des Dienstes: nicht einschlägig — diese Story startet keinen Dienst
  und meldet nichts bei einem fremden Dienst **an, das zurückzulesen wäre**;
  die beiden Anmeldungen sind `void` und stehen in §4.

**Nicht geprüft, weil es dem PO gehört:** der installierte Stand unter `/usr`.

---

## Berichtigung 05.08.2026 (PO, nach dem karpathy-Review, Befund K1)

**Der Text oberhalb bleibt unverändert** (B17). Berichtigt wird die
**Kopfzahl** von §6 — die Tabelle darunter ist richtig und ehrlich, der Satz
darüber zählt falsch.

**Es sind zwölf Zusicherungen in fünfzehn Läufen, nicht fünfzehn
Zusicherungen.** Die Zeilen 10, 11 und 12 der Tabelle sind offscreen grün und
verweisen ausdrücklich auf S1, S2 und S3, die **dieselben** Sachverhalte in der
angemeldeten Sitzung prüfen. Wer sie als eigene Zusicherungen zählt, zählt drei
doppelt.

Richtig lautet die Bilanz:

| | |
|---|---|
| tragende Zusicherungen | **12** |
| abgelegte Probeläufe | **15** (12 offscreen, 3 in der Sitzung) |
| durch eine rote Probe belegt | **11** |
| als Grenze benannt, mit Messung warum | **1** (Nr. 9, `setColorSet`) |

**AK 14 ist damit erfüllt** — es verlangt eine Probe je tragender Zusicherung
und deren Aufzählung im Bericht, und beides steht da. Falsch war allein die
Zusammenfassung.

*Warum das eine Berichtigung wert ist und keine Fußnote:* In Sprint 6 ist genau
diese Fehlerklasse einmal teuer geworden — „jede tragende Zusicherung gegen eine
Mutation gehalten" deckte acht von elf, und gefunden hat es das **Nachzählen**
eines fremden Lesers. Dass der Reviewer diesmal wieder nachgezählt hat und
wieder fündig wurde, ist der Beleg dafür, dass die Prüfung an dieser Stelle
gebraucht wird.
