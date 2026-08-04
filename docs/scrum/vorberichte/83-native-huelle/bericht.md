# Vorprüfbericht #83 — konsolidiert

**Story:** #83 „Capture: Hülle als native Plasma-Überlagerung — den Nachbau
entfernen" · **Konsolidiert:** 04.08.2026, 19:54 CEST, Ganymed · **Stand:**
`4385b31` · **Gegenstand:** die **sechzehn** Akzeptanzkriterien der Fassung vom
04.08.2026, nach den drei Kundenfestlegungen

**Bearbeiter A:** `denkzettel-dev` → `messung-a.md` (Felder 1, 2, 4, 5)
**Bearbeiter B:** Scrum Master → `messung-b.md` (Felder 1–6)
**Planning-Beratung:** `denkzettel-ux` → `ux-beratung.md` samt `ux-beratung/`
**Zusammenführung und Ready-Urteil:** Scrum Master (dieser Bericht)

Beide Messungen sind unabhängig entstanden; A hat B nicht gelesen, B hat A erst
zu dieser Konsolidierung gelesen, die UX-Beratung hat beide nicht geöffnet
(`ux-beratung.md` §0.1). Der Ertrag dieser Unabhängigkeit steht in §0 — er ist
der Grund, warum dieser Bericht länger ist als eine Glättung.

**Eigene Nachmessung dieser Konsolidierung:** `sm-textrolle.cpp` →
`sm-textrolle.txt` (welche Farbe `QPalette::PlaceholderText` annimmt, Anlass
AK 10).

---

## 0. Was erst die Zusammenführung gezeigt hat

Drei Befunde entstehen **nur** aus dem Nebeneinander der Messungen. Keiner
steht in einer der drei Quellen; jeder ist an abgelegten Dateien nachprüfbar.

### 0.1 AK 4 und AK 7 widersprechen einander — gemessen von A, formuliert nach UX

AK 7 macht den `opaque`-Auswahlpfad davon abhängig, dass
`KWindowEffects::isEffectAvailable(BlurBehind)` **falsch** ist. A hat genau
diesen Rückgabewert als untauglich vermessen (F9): In der **angemeldeten
Plasma-Sitzung des Kunden** liefert er vor der ersten Anmeldung `false`
(`messungen/sonde2-fensterlauf-wayland-skala-1.txt:50`, `sonde4-weichzeichner-an.txt:5`)
und erst nach einer frühen Anmeldung `true`
(`sonde4-weichzeichner-frueh.txt:6`). **AK 7 wörtlich ausgeführt schaltet das
Fenster beim Start in der Sitzung des Kunden auf `opaque`** — 100 % Deckung,
also das Gegenteil der Kundenfestlegung 1. Die UX-Beratung hat die Bedingung
vorgeschlagen (B-9), ohne F9 zu kennen; A hat F9 gemessen, als AK 7 noch nicht
existierte. Sichtbar wird der Widerspruch erst hier.

### 0.2 AK 4 ist von der vorgeschriebenen Lösung nicht erfüllbar — und der Beleg liegt im Repo

Die zweite Hälfte von AK 4 verlangt, dass die Anfangsspalte der Hülle „über die
ersten zehn Zeilen monoton mit höchstens einem Bildpunkt Schritt je Zeile"
läuft, **bei Verhältnis 1 und 1,6**. Der native Weg tut das bei 1,6 nicht:

| Beleg (alle versioniert) | Theme | DPR | Kantenlauf | Stufen ≥ 2 |
|---|---|---|---|---|
| `…/abnahme-befunde/messungen/native-huelle-breeze.txt` | `default` | 1 | 3·2·1·0·0·0 | **0** |
| dieselbe Datei | `default` | **1,6** | 7·**4**·3·2·1·1·1·0 | **1** (Sprung 3) |
| `native-huelle-nord.txt` | `CachyOS-Nord-round` | 1,6 | 7·**4**·3·2·1·1·1·0 | **1** |
| `native-huelle-nord-wayland.txt` | dito, Wayland | 1,6 | 7·**4**·3·2·1·1·1·0 | **1** |

Zum Vergleich der Kundenbefund selbst (`b1-echtelage.txt`, DPR 1,6):
6·6·3·2·2·2·0·0·0 — **zwei** Stufen. Der native Weg halbiert sie auf **eine**;
er beseitigt sie bei 1,6 nicht.

**Damit ist auch die Überschrift des Issues zu stark.** Der Satz „**Die Treppe
verschwindet.** … `framePixmap()` läuft `3·2·1·0·0·0` mit **null**" zitiert die
**DPR-1**-Zeile derselben Datei. Bei der Skalierung des Kunden steht in
derselben Datei eine Stufe. Das ist kein Fehler der Story — es ist ein Satz, der
für einen Stand gilt und ohne diesen Stand aufgeschrieben wurde (B17). Ein
Strang, der AK 4 wörtlich nimmt, baut gegen ein Ziel, das die vorgeschriebene
Lösung nachweislich verfehlt, und meldet am Ende einen Fehlschlag, den diese
Vorprüfung schon kennt.

### 0.3 A und B widersprechen sich in der Neubindung des Weichzeichners — und beide haben recht

B und die UX-Beratung schließen aus SPEC 3.2 Punkt 5, die Anmeldung müsse nach
jedem Neuzeigen erneuert werden (B Falle 1, UX B-6). **A hat es gemessen und das
Gegenteil gefunden** (F8): Der Lauf `wiederzeigen-alt` bleibt nach
`hide()`/`show()` **ohne** erneute Anmeldung weichgezeichnet
(`messungen/sonde4-weichzeichner-wiederzeigen-alt.txt`). A nennt die Grenze
selbst: gemessen mit unmittelbarem `hide()`/`show()`, nicht über den
50-ms-Umweg, den `showCapture()` nimmt (`capturewindow.cpp:32`, `:263`).

**Auflösung, und sie ist glücklich:** `present()` ruft `show()` und **unmittelbar
danach** `bindShadow()` (`capturewindow.cpp:419–430`). Die Stelle, an der die
Neubindung nach dem Muster des Schattens hingehört, ist damit genau die Stelle,
an der A den Weichzeichner **wirken** sah (F7, Lauf `frueh`). Die 50 ms liegen
**vor** `present()`, nicht zwischen `show()` und der Anmeldung. Die Forderung von
AK 5 ist also erfüllbar und kostet nichts; nach F8 ist sie womöglich überflüssig,
schadet aber nicht. **Sie bleibt richtig — nicht aus dem Grund, aus dem B und UX
sie erhoben haben.**

---

## Feld 1 — Dateimenge

Notation nach B13. A und B haben unabhängig dieselbe Menge vermessen und
kommen zur selben Kollisionsaussage; A hat die Zeilenbereiche feiner
aufgelöst, B die Prüfmittelseite. Zusammengeführt:

| | **#83** |
|---|---|
| **Issue / Zweig** | #83 (`epic:M1`, `typ:story`) · `story/83-native-huelle` |
| **Quellen und Tests** | `src/capture/capturewindow.{h,cpp}` — **ganz** (471 bzw. 99 Z.). Die Fläche verteilt sich: Konstanten `:44–54` (`OutlineWidth`, `FrameContrast`), `mixed()`/`tinted()` `:69–94` (**fallen weg**), `subtleLabel()` `:122–135` (AK 9), Konstruktor `:143–158` (`m_hullInner` entfällt), Textpalette `:366–376` (AK 9), `reloadDesktopTheme()` `:223–252`, `paintEvent()` `:309–330` (**neu**), `resizeHull()` `:338–343`, `bindShadow()` `:387 ff.`, `present()` `:419–430`. **Neu:** Behandlung von `QEvent::DevicePixelRatioChange` (F3).<br>`tests/capturetest.cpp` (669 Z.) — die Hüllen-Zusicherungen `:334–393`, `:411–433`, `:499–549` und neue Nachweise zu AK 3/4/5.<br>`tests/captureshots.cpp` (225 Z.).<br>`tests/themes/` — Aufnahme des rechteckigen Prüf-Themes (AK 11). |
| **Build** | **Nicht mehr „nichts".** `KF6::WindowSystem` ist bereits verlinkt (`src/CMakeLists.txt`), `KWindowEffects` braucht nur einen `#include` — insoweit stimmen A (F13) und B überein. **Hinzugekommen ist die offene Bauentscheidung zu AK 6:** die `[ContrastEffect]`-Werte führt `Plasma::Theme` aus `libPlasma`, einer Abhängigkeit, die das Projekt heute nicht hat (UX B-13). Entweder kommt sie hinzu — dann ändern sich `CMakeLists.txt` der Wurzel **und** `src/CMakeLists.txt` — oder die vier Schlüssel werden selbst gelesen. **Dazu kommt AK 9**, das die `colors`-Datei des Themes liest: ein zweiter Selbstleseweg in dieselbe Metadatei. |
| **Belege und Prüfmittel** | `docs/scrum/reviews/sprint-NN-s83-native-huelle/` mit eigenem `pruefen.sh` nach dem Muster von `sprint-06-s55-huelle/`. **Wiederverwendbar, nicht neu zu erfinden:** `sonden/weichzeichnerbeleg.cpp` (Bauplan des AK-5-Belegs), `…/abnahme-befunde/sonden/echtelage.cpp` (Sitzungsbild, nimmt allein das Fenster auf), `…/sonden/eckhelligkeit.py` (AK 15), `…/pruef-theme/` (AK 11), `ux-beratung/sonden/deckung.cpp` (Theme-Achse), `po-themeschrift.py` (AK 9). |
| **Fachliche Quellen** | **SPEC 3.1** (`:152–186`) — der tragende Absatz wird umgedreht, die `frameContrast`-Zeile (`:170–173`) fällt ersatzlos. **SPEC 3.2** (`:188–214`) — nimmt die entdeckten Bedingungen auf (F3, F7, Neubindung, `opaque`-Pfad). **SPEC 15** (`:720–735`) — kennt `KWindowEffects` nicht. **SPEC 16** (`:770–796`) — „offscreen prinzipbedingt nicht belegbar" bekommt den Weichzeichner dazu, mit der schärferen Aussage, dass ihn auch eine **Fensteraufnahme** nicht zeigt. |
| **Ausdrücklich nicht** | `src/ui/`, `src/shell/`, `src/store/`, `src/capture/textareaheight.{h,cpp}` (gehört #79/#81), `tests/librarytest.cpp`, `tests/libraryshots.cpp`, `tests/editshots.cpp`, `tests/searchshots.cpp`, `tests/readmeshots.cpp`, `wireframes/` (Zeichnung 4a/4b ist ein eigener UX-Schritt), Belegordner fremder Sprints — sowie `CLAUDE.md`, `docs/scrum/PROZESS.md` und `.claude/agents/denkzettel-dev.md` (die drei B17-Fundstellen, §6.3). |

### 1.1 Kollisionsfläche — der kleinste Abstand ist null

A und B haben sie unabhängig vermessen und kommen zum selben Urteil: **keiner
der drei anderen offenen Capture-Vorgänge läuft parallel zu #83.**

| Vorgang | kleinster Abstand in `capturewindow.cpp` | Urteil |
|---|---|---|
| **#55** (geschlossen) | null — #83 schreibt denselben Code neu | entfällt, Issue ist zu |
| **#79** (Fenster schrumpft nicht) | null, sobald #79 `present()` anfasst; sonst 32 Zeilen | **nicht parallel** |
| **#81** (Textkante) | **3 Zeilen** (`:158` → `:161`) — innerhalb der Mischbreite von Git | **nicht parallel** |

Dazu zwei Gründe außerhalb des Textes, die B beigesteuert hat und die A auf der
Bildseite bestätigt: **#83 und #81 kollidieren an der Höhenformel**
(`adjustHeight()` liest den Dokumentrand `:467`, #81 setzt ihn auf 0, #83
verschiebt über die Theme-Ränder dieselbe Höhe — danach weiß niemand mehr,
welche Änderung die Starthöhe bewegt hat; das ist der Wert, den die
Sprint-1-Abnahme zurückgewiesen hat, #42). Und **#83, #79 und #81 ändern
dieselbe Bildreihe** (`captureshots`): zwei Stränge, die gleichzeitig zwölf
Bilder neu erzeugen, überschreiben einander die Belege, und
`bildbelege-pruefen.sh` schlägt genau dort an.

**Empfehlung an den Schnitt:** #83 nicht mit #81 in einem Sprint; #79 nur bei
getrennten Belegordnern.

---

## Feld 2 — Gemessene Fallen

Dies ist das Feld, aus dem der Spawn-Auftrag geschrieben wird. Sechzehn Fallen
von A (F1–F16), vier von B, dazu die drei aus §0. Jede mit Beleg. **Die drei
teuren stehen zuerst.**

### Die drei teuren

**T1 — Der Weichzeichner wirkt nur, wenn er unmittelbar nach `show()`
angemeldet wird. Der Zeitpunkt entscheidet, und nur er.** (A, F7 — sieben A/B-Läufe
mit Vollbild-Schachbrett, gemessen an der Helligkeitsspannweite auf einer festen
Prüflinie.)

| Lauf | Anmeldung | Spannweite | Befund |
|---|---|---|---|
| `aus` | keine | 203–242 | Schachbrett scharf |
| `an` | einmal, 1,2 s nach `show()`, mit Maskenregion | 203–242 | **kein Weichzeichner** |
| `zweimal` | zweimal, 200 ms auseinander, spät | 203–242 | **kein Weichzeichner** |
| `spaet-leer` | spät, **leere** Region (= ganzes Fenster) | 203–242 | **kein Weichzeichner** |
| `frueh` | sofort nach `show()` | 222–223 | **weichgezeichnet** |
| `wiederzeigen-neu` | früh, nach `hide()`/`show()` erneut | 222–223 | weichgezeichnet |
| `wiederzeigen-alt` | früh, nach `hide()`/`show()` **nicht** erneut | 222–223 | weichgezeichnet |

*Beleg:* `messungen/sonde4-weichzeichner-*.txt`, Bilder
`bilder/weichzeichner-pruefline-*.png`. Der Lauf `spaet-leer` schließt die
Region als Ursache aus — es liegt am Zeitpunkt, nicht an der Maske.
**Zeile für den Auftrag:** Die Anmeldung gehört in `present()` unmittelbar hinter
`show()`, neben `bindShadow()` (§0.3). Kein Rückgabewert meldet den Fehlschlag;
wer sie anderswohin legt, merkt es nur am Bild.

**T2 — Das Bildpunktverhältnis des Fensters steht nach `show()` noch nicht fest
und ändert sich ohne Größenänderung.** (A, F3.) Unter Wayland meldet Qt zunächst
**2** und rund eine Sekunde später **1,6**; zugestellt wird das als
`QEvent::DevicePixelRatioChange`, **ohne begleitendes `QEvent::Resize`**. Wer die
Hülle nur im `resizeEvent()` nachzieht, zeichnet dauerhaft bei 2 auf einem
Fenster, das 1,6 ist. *Beleg:*
`messungen/sonde2-fensterlauf-wayland-skala-1.txt` — Ereignismitschrift „Resize,
DevicePixelRatioChange, DevicePixelRatioChange", dazu „Hülle bekam dabei zuletzt
2 … Fenster bei 1,6". **Offscreen tritt der Fall nicht auf** (Mitschrift: nur
„Resize") — der Fehler ist offscreen unsichtbar, und **ein Bildbeleg offscreen
zeigt ihn nie.**

**T3 — `enableBlurBehind(nullptr, …)` stürzt unter Wayland ab.** (A, F6.)
Rückgabe 139 (SIGSEGV); offscreen kehrt derselbe Aufruf zurück. Wer die
Anmeldung in den Konstruktor legt, wo `windowHandle()` noch `nullptr` ist,
bekommt keinen Fehlschlag, sondern einen Absturz — **und offscreen fällt er
nicht auf**. *Beleg:* `sonde2-fensterlauf-wayland-skala-1.txt`, Abschnitt E
bricht ab; `…-offscreen-skala-1.txt`, Abschnitt E läuft durch.

### Zu Maß und Verhältnis

**F1 — `KSvg::FrameSvg` folgt dem Bildschirm nicht von selbst.**
`devicePixelRatio()` ist nach dem Bau **1**, gleich welche Skalierung gilt; die
Anwendung muss den Wert setzen. *Beleg:*
`sonde1-rahmenmasse-offscreen.txt`, Abschnitt A.

**F2 — Die Reihenfolge ist gleichgültig, und ein DPR-Wechsel allein genügt.**
`setDevicePixelRatio()` vor und nach `resizeFrame()` liefern beide 960×279 bei
DPR 1,6; ein späterer Wechsel wirkt **ohne** erneutes `resizeFrame()`. Wer hier
eine Reihenfolge sucht, sucht umsonst. *Beleg:* ebd., Abschnitte B und C.

**F4 — `QT_SCALE_FACTOR` multipliziert unter Wayland mit der
Sitzungsskalierung.** `QT_SCALE_FACTOR=1` ergibt Fenster-DPR **1,6**,
`QT_SCALE_FACTOR=1,6` ergibt **2,56**; offscreen ergibt 1,6 genau 1,6. Wer den
Kundenstand in der Sitzung nachstellen will, setzt die Variable **gar nicht**;
wer ihn offscreen nachstellt, setzt sie auf 1,6. *Beleg:*
`sonde2-fensterlauf-wayland-skala-1.txt` gegen `-skala-1.6.txt` gegen
`sonde2-fensterlauf-offscreen-skala-1.6.txt`.

**F5 — `mask()` liefert logische Bildpunkte, und sie ist nicht leer.** Das
Hüllrechteck ist bei DPR 1 **und** 1,6 genau 600×174 — das Koordinatensystem,
das `enableBlurBehind` laut Kopfdatei erwartet („specified in logical pixels").
Umgerechnet wird nichts. Und obwohl die native Hülle nur zu 84,7 % deckt,
enthält `mask()` 7 Rechtecke mit 104.352 von 104.400 Bildpunkten — sie stammt
aus den `mask-`Elementen, die voll decken. *Beleg:* ebd., Abschnitte D und E2.

### Zu den Rückgabewerten, die nichts belegen

**F9 — `isEffectAvailable(BlurBehind)` ist träge und taugt nur *nach* der
Anmeldung als Zurücklesen.** Vor dem ersten `enableBlurBehind()` liefert es in
der angemeldeten Plasma-Sitzung **`false`**, danach **`true`**. Der Wert sagt
nicht, was der Compositor kann, sondern ob die Wayland-Erweiterung schon
gebunden ist. *Beleg:* `sonde2-…-wayland-skala-1.txt:50` und `:55` gegen
`sonde4-weichzeichner-frueh.txt:6`. **Diese Falle trägt §0.1** — und sie ist ein
Kandidat für die Liste „Rückgabewerte, die nichts belegen" in
`.claude/agents/denkzettel-dev.md`.

**F10 — `enableBlurBehind()` ist `void`.** Es gibt keinen Rückgabewert, den man
missdeuten könnte — und keinen, der etwas belegte. *Beleg:*
`/usr/include/KF6/KWindowSystem/kwindoweffects.h`.

**F15 — Der Compositor bietet die alten KDE-Protokolle nicht mehr an.**
`org_kde_kwin_blur_manager` und `org_kde_kwin_contrast_manager` stehen nicht in
der Globalenliste; an ihrer Stelle steht `ext_background_effect_manager_v1`.
`org_kde_kwin_shadow_manager` gibt es weiterhin — **der Schatten läuft
unverändert.** *Beleg:* `sonde5-stand-und-globale.txt`. **F7 und F9 sind
Eigenschaften dieses Standes** (B17): kwin 6.7.3, kwindowsystem 6.28.0,
ksvg 6.28.0, qt6-base 6.11.1, plasma-desktop 6.7.3.

### Zu den Tests

**F11 / B-2 — Vier Zusicherungsfamilien fallen.** A und B haben das unabhängig
gemessen und kommen auf dieselben Zahlen (A: `sonde2-fensterlauf-*`, Abschnitt B;
B: `messung-b-testfolgen.txt`, Theme `default`, Schema des Kunden):

| Zusicherung in `tests/capturetest.cpp` | heute | nativ |
|---|---|---|
| `qAlpha(Randmitte oben)`, 5× in `hullIsCompleteAtFiveAndEightLines` | 255 | **235** |
| `qAlpha(Fenstermitte)` | 255 | **216** |
| `QCOMPARE(Pixel, Window)` in `paintsOneSurfaceInThePaletteColours` | `#ff1e2233` = `#ff1e2233` | **`#d81e2233` ≠ `#ff1e2233`** (Farbe stimmt, Alpha nicht — `QColor` vergleicht es mit) |
| `cornerRun()` und seine drei Nutzer | 6 | **2** |

Das ist Umfang der Story, kein Fehler des nativen Weges: „geschlossen" bedeutet
unter einer Überlagerung nicht mehr „undurchsichtig". **`cornerRun() > 0` und
`wideCorner != narrowCorner` halten** (A, F11) — die Herkunftsaussage bleibt
also zusicherbar.

**F16 / B-3 — Ein theme-abhängiger Test trügt hier, und zwar in die grüne
Richtung.** Beide Bearbeiter haben es unabhängig gefunden. A hat die Prüf-Themes
vermessen (`messungen/sonde1-pruefthemes.txt`):

| Theme | Herkunft | Alpha Mitte | Randwert |
|---|---|---|---|
| `denkzettel-test-schmal` | `tests/themes/` | **255** | 255 |
| `denkzettel-test-breit` | `tests/themes/` | **255** | 255 |
| `denkzettel-pruef-eckig` | Abnahme-Belege | **255** | 255, Anstieg 0 Spalten |
| `default` | installiert | **216** | 235 |

`hullIsCompleteAtFiveAndEightLines()` läuft über beide mitgelieferten Themes
**und** über `themes::anyInstalledTheme()`. Die Zusicherung `qAlpha == 255`
fällt also **nur am installierten Theme** — auf einem Bauplatz ohne installierte
Desktop-Themes bliebe der Test grün, während die Hülle im Betrieb durchscheint.
B hat dieselbe Falle von der anderen Seite gemessen: unter
`CachyOS-Nord-round` halten **alle vier** Zusicherungen unverändert, weil dieses
Theme mit Alpha 255 zeichnet. **Wer die neuen Zusicherungen nur gegen ein Theme
prüft, kann beide Ergebnisse bekommen und beide für richtig halten.** Wer die
Zusicherung anpasst, muss sie so anpassen, dass sie **am mitgelieferten Theme
genauso etwas prüft**.

### Zur Plattformgleichheit

**F12 / B-12 — „dasselbe Bild" gilt für die Hülle, nicht für das Fenster.**
A: Offscreen und Wayland liefern bei **gleichem Bildpunktverhältnis** Zahl für
Zahl dasselbe (beide 960×278, Mitte 32,35,38/Alpha 216, `cornerRun` 4, oberste
Zeile `0 0 0 0 2 65 125 173 195 209 235 …`); bei gleichem `QT_SCALE_FACTOR`
**nicht**, weil die Plattformen verschiedene Verhältnisse einstellen (F4).
UX: die **Hülle allein** ist byteweise gleich über vier Themes, **dieselbe Hülle
mit Text darauf verschieden** über alle vier — 1.587 von 154.440 Bildpunkten,
sämtlich in den Zeilen 49–190 und Spalten 47–416, also im Textbereich; Ursache
ist die Schriftrasterung über Fontconfig, nicht KSvg
(`ux-beratung/messungen/m6-plattformvergleich.txt`). **Zeile für den Auftrag:**
Bildvergleiche ganzer Fenster über Plattformgrenzen hinweg sind kein Prüfmittel.

### Zum Theme als zweiter Achse

**B-4 — Die Flächenfarbe folgt dem Farbschema nur unter `default`.** B hat die
Achse gedreht, die die Messgrundlage des Issues festhält (ein Schema, acht
Themes statt zwanzig Schemata, ein Theme;
`messung-b-themefarbe.txt`, offscreen, `QT_QPA_PLATFORMTHEME=kde`, Schema
`CachyOSNordLightly`, `Window 30,34,51`, `WindowText 102,194,242`):

| Desktop-Theme | Fläche | Alpha | = `Window`? | Text auf Fläche |
|---|---|---|---|---|
| `default` | 30,34,51 | 216 | **gleich** | 7,93:1 |
| `CachyOS-Nord-round` | 30,34,51 | 255 | **gleich** | 7,93:1 |
| `breeze-dark` | 32,35,38 | 216 | Abweichung 13 | 7,92:1 |
| **`breeze-light`** | **240,240,241** | 216 | **Abweichung 210** | **1,74:1** |
| `Iridescent-round` | 0,0,0 | **51** | Abweichung 51 | Fläche fast durchsichtig |
| die drei `cachyos-emerald` | 0,0,0 | **7** | Abweichung 51 | Fläche praktisch nicht vorhanden |

UX hat dieselbe Achse mit vier Schemata gekreuzt und kommt zum gleichen Bild
(`ux-beratung/messungen/m1-deckung-je-theme.txt`): `default` folgt in allen vier
Schemata, `breeze-dark`/`breeze-light`/`CachyOS-Nord-round` je nur unter „ihrem"
Schema, die vier durchsichtigen **nie**. **Die Zusicherung des Issues hält, weil
beim Kunden Theme und Schema zufällig zusammenpassen:** `plasmarc` trägt keinen
`[Theme] name`-Eintrag, das Fenster läuft auf dem KSvg-Rückfall `default`, und
das ist das eine Theme von achten, das der Palette folgt.

**B-5 — `native-farben.txt` gibt es nicht.** Die alte AK-6-Fassung nannte diese
Datei als Messweg; `git grep native-farben` liefert über das ganze Repository
keinen Treffer. Beide Bearbeiter haben es unabhängig gefunden (B Falle 4,
UX B-4). **In der neuen Fassung ist es behoben** — AK 8 nennt
`native-ak2-kontrast.txt`, und die Datei ist versioniert. Der Befund bleibt hier
stehen, weil er die Verfahrensänderung in §7.3 trägt.

### Zum nativen Vertrag

**F14 / B-13 — „Ohne Anpassungen" umfasst bei Plasma drei Anmeldungen, nicht
eine.** A und UX haben unabhängig denselben Binärcode befragt
(`sonde3-plasma-anmeldungen.txt`, `ux-beratung/messungen/m4-effektanmeldungen.txt`,
je `nm -D -u` auf `libPlasmaQuick`): `KWindowShadow` (hat Denkzettel),
`enableBlurBehind`, `enableBackgroundContrast`. `KSvg` selbst meldet nichts an.
UX hat die Gegenprobe an den Themes geführt: genau die Themes, deren Grafik fast
nichts füllt, fordern einen `[ContrastEffect]` in ihrer eigenen Beschreibung
(`Iridescent-round` 20 % Deckung mit `intensity=0.45`, die drei
`cachyos-emerald` 2,7 % mit `intensity=0.40`;
`ux-beratung/messungen/m3-effektangaben.txt`). **Diese Themes sind nicht kaputt
— sie sind darauf gebaut, dass der Compositor den Grund hinter ihnen
abdunkelt.**

**UX B-9 — Der `opaque`-Auswahlpfad ist der native Weg für den Fall ohne
Compositor**, und er trägt nur für die Breeze-Familie
(`m3-effektangaben.txt`): `default`/`breeze-dark`/`breeze-light` 84,7 % →
**100 %**; die drei `cachyos-emerald` 2,7 % → 3,5 %; `CachyOS-Nord-round` und
`Iridescent-round` bringen **keine** Auswahlpfade mit. **Randbefund, der leicht
übersehen wird:** Die Schattenkacheln kommen aus dem `shadow`-Prefix derselben
Grafik — wird ein Auswahlpfad gesetzt, **wechselt der Schatten mit**.

**F13 — Am Build war nichts zu tun.** Das galt für die alte Fassung. Mit AK 6
und AK 9 gilt es nicht mehr (Feld 1, Zeile „Build").

### Bestandsbefund, melden statt heilen

**UX §6.6 — Die Polsterung des Schattens wird in `bindShadow()` gesetzt und bei
`resizeHull()` nicht nachgezogen**, obwohl das Fenster seine Höhe im Betrieb
ändert. Das ist heutiger Stand, kein Zugang durch #83. **Gehört dem PO, nicht
dem Strang.**

---

## Feld 3 — AK-Urteil: **ready = nein**

Sechzehn Kriterien, einzeln gegen die Definition of Ready geprüft. Maßstab:
prüfbar formuliert · Prüfmittel benannt **oder** Grenze ausgesprochen ·
B21-Belegform bedacht · kein Widerspruch zu SPEC, Beschluss oder Messung.

**Vorab, und es ist der wichtigste Satz dieses Feldes:** Die Umarbeitung ist ein
großer Fortschritt. Von den sechs Befunden aus `messung-b.md` und den vierzehn
der UX-Beratung sind **achtzehn korrekt eingearbeitet**. Was unten steht, sind
**vier** verbliebene Befunde und **fünf** Auflagen — keine Wiederholung der
alten Liste.

| AK | Urteil | Kurz |
|---|---|---|
| 1 | **Auflage** | `git grep` ohne Geltungsbereich ist dauerhaft unerfüllbar |
| 2 | **ok** | SPEC-Nachzug als eigenes Kriterium — Befund aus `messung-b.md` sauber übernommen |
| 3 | **Auflage** | die Belegform trifft den gemessenen Fehler nicht (T2) |
| 4 | **nein** | zweite Hälfte von der vorgeschriebenen Lösung nicht erfüllbar (§0.2); Zielwert der ersten Hälfte falsch benannt |
| 5 | **Auflage** | geordneter Fehlschlag für die Größenänderung fehlt |
| 6 | **ok** | Bedingung, Belegform und beide Fälle benannt |
| 7 | **nein** | die Auslösebedingung ist der Wert, den A als untauglich vermessen hat (§0.1) |
| 8 | **Auflage** | Messwegangaben ohne Pfadwurzel |
| 9 | **nein** | steht gegen Kundenfestlegung 2, gemessen; Zahl für vier Themes gegen eine Fiktion gerechnet |
| 10 | **ok** | Zahlen bestätigt (`sm-textrolle.txt`); Rollenname ist eine Anmerkung, kein Mangel |
| 11 | **ok** | Überführung und Sitzungsbild benannt |
| 12 | **ok** | beide Achsen, zwei Themes mit verschiedener Deckung |
| 13 | **ok** | **das beste Kriterium der Liste** — Grenze der Prüfbarkeit ausgesprochen |
| 14 | **Auflage** | verlangt ein Bild, aber kein Urteil; Vergleichsfenster nicht benannt |
| 15 | **ok** | geordneter Fehlschlag — Musterfall |
| 16 | **ok** | Aufzählpflicht macht den Satz zählbar |

**Vier `nein`, fünf Auflagen, sieben `ok`. Ready = nein.**

### Die vier Befunde

**AK 4 — nicht erfüllbar wie formuliert.** Zwei Mängel, der erste ist der
schwere.

*Erstens*, die zweite Hälfte: „höchstens ein Bildpunkt Schritt je Zeile … bei
Verhältnis 1 und 1,6". Der native Weg springt bei 1,6 in Zeile 1 um **drei**
Spalten — gemessen über zwei Themes und beide Plattformen, drei versionierte
Dateien (§0.2). Es ist auch keine Formulierungsfrage: Ein Kreisbogen vom Radius
r läuft in der obersten Zeile mit √(2r−1) Bildpunkten je Zeile, nahe dem
Scheitel also stets um mehr als einen. **Die Forderung ist geometrisch nur von
einer Ecke zu erfüllen, die gar kein Bogen ist.** Was den Kundenbefund
tatsächlich trennt, ist die **Zahl der Stufen** (zwei gegen eine) und ihre
Herkunft (Maske bei DPR 1 hochskaliert gegen Rahmenelemente beim Verhältnis des
Fensters) — nicht ein Schritt von höchstens eins.
*Vorschlag:* „…und der Kantenlauf über die ersten zehn Zeilen ist monoton
fallend und enthält bei Verhältnis 1 **keine** Stufe von zwei oder mehr Spalten,
bei 1,6 **höchstens eine** (gemessener Ausgangsstand: zwei, `b1-echtelage.txt`).
Messweg: der Kantenlauf aus `native-huelle-*.txt`."
**Zusatz, den ich für nötig halte:** Auch die Richtung stimmt nicht. Das
Kriterium sagt „wächst … monoton"; die Anfangsspalte **fällt** von oben nach
unten (7·4·3·2·1·1·1·0). Wer es wörtlich prüft, weist den richtigen Zustand
zurück. Der Fehler stammt aus dem Korrekturvorschlag der UX-Beratung (B-1) und
ist beim Umschreiben mitgewandert.

*Zweitens*, die erste Hälfte: „streng steigend **bis zur Flächendeckung**". Der
Anstieg endet gemessen bei **235**, die Flächendeckung ist **216** — der
Randwert liegt *über* der Fläche (`native-huelle-breeze.txt`, Abschnitt D). „Bis
zur Flächendeckung" wird nie erreicht. A hatte den richtigen Begriff („Anstieg
bis zum **Randwert**", F/AK 3); beim Umschreiben ist er verlorengegangen.

**AK 7 — die Auslösebedingung ist gemessen untauglich.** §0.1. In der Sitzung
des Kunden liefert `isEffectAvailable(BlurBehind)` vor der ersten Anmeldung
`false`; das Kriterium schaltet dort also auf `opaque` und hebt genau die
Durchsichtigkeit auf, die Festlegung 1 verlangt.
*Vorschlag:* die Bedingung **nicht** an diesen Rückgabewert hängen, sondern an
die Umgebung — kein Wayland-Compositor mit Weichzeichner-Erweiterung
(`ext_background_effect_manager_v1` bzw. `org_kde_kwin_blur_manager` in der
Globalenliste, F15), oder schlicht: keine Plasma-Sitzung. Wird der Rückgabewert
beibehalten, muss das Kriterium sagen, **wann** er gelesen wird — nämlich erst
nach der ersten Anmeldung —, und dann ist es eine Aussage über unsere eigene
Bindung, nicht über den Compositor.

**AK 9 — steht gegen Kundenfestlegung 2, und das ist gemessen.**
Festlegung 2 lautet „unter **jedem** Desktop-Theme lesbar". Festlegung 3
(Schrift aus derselben Quelle wie die Fläche) erfüllt das für zwei Themes und
zerstört es für ein drittes — die Messung des PO sagt es selbst
(`po-themeschrift.txt`):

| Theme mit eigener `colors` | Schemaschrift | Themeschrift |
|---|---|---|
| `breeze-light` | 1,75:1 | **13,35:1** |
| `breeze-dark` | 7,94:1 | **15,39:1** |
| `cachyos-emerald-color` | 10,56:1 | 9,57:1 |
| `cachyos-emerald-light` | **10,56:1** | **1,38:1** |

AK 9 verbucht das als „**benannte Grenze**: das wird vermerkt, nicht behoben".
Eine benannte Grenze ist das richtige Mittel, wenn etwas nicht erreichbar ist —
hier aber wird ein Zustand, der die Kundenforderung **erfüllt** (10,56:1),
absichtlich in einen verwandelt, der sie verletzt (1,38:1). **Das ist keine
Grenze, das ist ein Zielkonflikt zwischen zwei Festlegungen desselben Kunden,
und er gehört ihm vorgelegt** (Feld 6, Vorlage 1). Kein Kriterium der Liste
sichert Festlegung 2 überhaupt zu; AK 8, 9 und 10 verlangen sämtlich nur
*messen und vermerken*.

*Zweiter Mangel an AK 9, kleiner, aber er entwertet die Zahl:* Die
Kontrastwerte sind laut Kopf der Messung „Flächenfarbe **deckend** gerechnet".
Für `breeze-light` (Alpha 216) ist das eine brauchbare Näherung. Für die drei
`cachyos-emerald` (Alpha **7** von 255, `messung-b-themefarbe.txt`) wird gegen
eine Fläche gerechnet, die es nicht gibt — dort steht der Text in Wahrheit auf
dem Desktop-Hintergrund. **Die 1,38:1 sind gegen eine Fiktion gerechnet.** Das
macht den Befund nicht kleiner, aber die Zahl darf nicht als gemessener
Kundenkontrast in ein Kriterium.

### Die fünf Auflagen

**AK 1 — `git grep` braucht seinen Geltungsbereich.** Repoweit finden die
Bezeichner heute **37 Dateien**: SPEC, `wireframes/`, `CLAUDE.md`,
`PROZESS.md`, `.claude/agents/denkzettel-dev.md`, sämtliche Belegordner und
diese Vorprüfung. Nach B17 werden Belege geankert, nicht geglättet — repoweit
ist das Kriterium also **dauerhaft** unerfüllbar. Auf `-- src/` eingegrenzt
findet es genau eine Datei, `capturewindow.cpp`, und wird zu dem, was gemeint
ist. *Auflage:* `git grep -n "tinted\|FrameContrast\|m_hullInner\|OutlineWidth" -- src/`
muss leer sein.

**AK 3 — die Belegform trifft den gemessenen Fehler nicht.** Verlangt sind Test
und „je ein Bild" bei 1 und 1,6. Beides ist offscreen zu haben, und **offscreen
tritt T2 nicht auf** (die Ereignismitschrift zeigt dort nur „Resize"). Ein
Kriterium, dessen Beleg den einzigen bekannten Fehlermechanismus systematisch
nicht sehen kann, ist erfüllbar, ohne etwas zu zeigen. *Auflage:* mindestens der
1,6-Beleg kommt aus der angemeldeten Sitzung, ohne `QT_SCALE_FACTOR` (F4).

**AK 5 — der Größenänderung fehlt der geordnete Fehlschlag.** Die Neubindung
beim Neuzeigen ist erfüllbar und billig (§0.3). Ob eine **späte** Anmeldung die
Region eines bereits laufenden Weichzeichners noch ändert, ist **nicht
gemessen**, und T1 gibt konkreten Anlass zum Zweifel: alle vier späten Läufe A's
blieben wirkungslos. *Auflage:* denselben Satzbau wie AK 15 aufnehmen — „lässt
sich die Region nach einer Größenänderung nicht nachführen, wird das gemessen
und mit dem Befund vorgelegt; die Story bleibt davon abnehmbar." Sonst hängt ein
ganzes Kriterium an einer unvermessenen Annahme.

**AK 8 — die Messwege brauchen ihre Pfadwurzel.** `messungen/native-ak2-kontrast.txt`
ist zweideutig: Es gibt `docs/scrum/reviews/2026-08-04-abnahme-befunde/messungen/`
**und** `docs/scrum/vorberichte/83-native-huelle/messungen/`. Beide Dateien
existieren und sind versioniert (geprüft mit `git ls-files`), aber der Name
allein löst nicht auf. *Auflage:* Pfade ab Repositoriumswurzel.

**AK 14 — verlangt ein Bild, aber kein Urteil.** Der Vorschlag der UX-Beratung
trug einen zweiten Satz: „Beide tragen dieselbe Eckform, dasselbe Randmaß und
dieselbe Deckung." Er ist beim Umschreiben entfallen. *Ich verteidige den
Entfall zur Hälfte:* „dieselbe Deckung" wäre falsch gewesen — KRunner zeichnet
unter `default`, unser Fenster unter dem eingestellten Theme, und die Deckung
reicht über die Themes von 2,7 % bis 100 % (B-4). Aber ohne **jedes** Urteil
sagt AK 14 nur noch „es liegt ein Bild vor" und ist durch Ablage erfüllt.
*Auflage:* zwei Zusätze — welche Überlagerung (KRunner, damit der Beleg
wiederholbar ist), und dass die **Beurteilung des Bildes Sache der
Kundenabnahme** ist. Dann ist das Kriterium ehrlich: Es beschafft die Grundlage
für ein Urteil, das kein Agent fällen kann (Feld 4, Grenze 3).

### Was ich ausdrücklich als gelungen vermerke

- **AK 2** macht den SPEC-Nachzug zum eigenen Kriterium. Das war mein Befund zu
  AK 1 alt, und er ist besser umgesetzt, als ich ihn vorgeschlagen hatte —
  benannt sind die Absätze *und* was aus ihnen wird.
- **AK 13** spricht die Grenze der Prüfbarkeit aus („ein datierter Doppellauf
  mit abgelegten Ausgaben, kein laufender Test") und nennt die Falle mit Zahl
  (1.587 Bildpunkte, Fontconfig). Das ist die Bauart, die die DoR meint.
- **AK 15** ordnet den Fehlschlag: Widerlegt die Messung die Erklärung, geht der
  Befund als eigenes Issue zurück und blockiert die Abnahme nicht. Musterfall.
- **AK 16** macht aus dem Satz, der in Sprint 6 gerissen ist, einen zählbaren.
- **AK 10** ist inhaltlich richtig, und ich habe es nachgemessen, weil der
  Rollenname mich stutzig gemacht hat: Das Kriterium nennt `ForegroundInactive`,
  der Code setzt `QPalette::PlaceholderText` (`capturewindow.cpp:132`, in
  `subtleLabel()` — App-Name **und** Fußzeile). **Gemessen ist es dieselbe
  Farbe:** `PlaceholderText` nimmt in allen drei geprüften Schemata genau
  `ForegroundInactive` der Gruppe `[Colors:Window]` an (`sm-textrolle.txt`). Die
  Zahlen der UX-Beratung stehen also. *Anmerkung ohne Mangelcharakter:* Der
  Strang sucht im Code nach `ForegroundInactive` und findet nichts — ein
  Klammerzusatz „(im Code: `QPalette::PlaceholderText`)" spart ihm einen
  Fehlversuch.

### Was das Issue selbst noch offen führt

Die neue Fassung hat die Rubrik „Offene Punkte, die vor dem Ziehen entschieden
gehören" **abgeschafft** — die drei Punkte sind in Kriterien und
Kundenfestlegungen aufgegangen. Das ist richtig und behebt meinen
Verfahrensbefund 2 in der Sache. Geblieben ist eine **offene
Bauentscheidung** (`Plasma::Theme` als Abhängigkeit oder die vier Schlüssel
selbst lesen), die das Issue ausdrücklich dem Strang überträgt und die in den
Übergabebericht gehört. **Das ist zulässig** und kein Ready-Hindernis: Es ist
eine Wie-Frage innerhalb der Story mit benanntem Ergebnisort, keine
selbstdeklarierte Unfertigkeit. Sie schlägt aber auf Feld 1 (Build) und auf die
Größenklasse durch.

---

## Feld 4 — Prüfmittel, und was ein Agent nicht prüfen kann

| AK | Prüfmittel | Grenze |
|---|---|---|
| **1** | `git grep -- src/`; dazu Mutationsprobe an der neuen Zeichenzeile | keine, sobald der Geltungsbereich steht |
| **2** | Lesen von SPEC 3.1/3.2 gegen den gelieferten Code; DoD 4 trägt es ohnehin | keine |
| **3** | Test offscreen mit `QT_SCALE_FACTOR` (dort ist das Verhältnis stabil und steuerbar, F4): `framePixmap().devicePixelRatio() == devicePixelRatioF()`; **dazu ein Sitzungslauf ohne `QT_SCALE_FACTOR`** | Ein Agent kann das Verhältnis **in der Sitzung nicht wählen** — es kommt vom Compositor (F4). 1,6 ist vorzufinden, nicht einzustellen. Offscreen tritt T2 nicht auf |
| **4** | Kantenlauf aus `native-huelle-*.txt`, vier Zahlen je Verhältnis: Randwert, Länge des Anstiegs, Rückschritte, Stufen ≥ 2 Spalten | Der Zielwert muss aus der Messung kommen, nicht aus dem Wunsch (§0.2) |
| **5** | Bildschirmaufnahme mit bekanntem Muster dahinter; Bauplan liegt fertig vor (`sonden/weichzeichnerbeleg.cpp`): eigenes Vollbild-Schachbrett unter dem Fenster, `spectacle -f`, Vollbildpuffer sofort löschen, nur den Ausschnitt behalten; A/B gegen einen Lauf ohne Anmeldung | **`spectacle -a` (nur das Fenster) taugt nicht** — liefert die eigene Fläche mit Alpha 216, genau wie `QWidget::grab()`. **`isEffectAvailable()` taugt nur nach der Anmeldung** (F9). **`enableBlurBehind()` ist `void`** (F10). Ein Bild des **Bildschirms** ist zwingend |
| **6** | Lesen der Theme-Metadatei (`[ContrastEffect]`); Nachweis über zwei Themes, eines mit und eines ohne Gruppe | Ob `enableBackgroundContrast` auf diesem Compositor-Stand **wirkt**, ist **nicht gemessen** — `org_kde_kwin_contrast_manager` fehlt wie der Blur-Manager (F15). Der Aufruf ist belegbar, die Wirkung nur im Bild |
| **7** | Deckungsmessung mit und ohne Auswahlpfad, Muster `ux-beratung/sonden/deckung.cpp` | die Auslösebedingung ist offen (§0.1) |
| **8** | Sondenlauf über `/usr/share/color-schemes/` (Muster `pruefen.sh`) **und** über `/usr/share/plasma/desktoptheme/` (Muster `deckung.cpp`) | 3 von 19 Schemata weichen um einen Zählschritt ab — die Toleranz steht im Kriterium, gut so |
| **9** | `po-themeschrift.py` auf allen acht Themes | Für Themes unter Alpha 51 ist die Zahl gegen eine gedachte Fläche gerechnet und nicht der Kontrast, den der Nutzer sieht |
| **10** | `ux-beratung/messungen/m2-kleintexte.txt`; Rollenauflösung belegt in `sm-textrolle.txt` | keine — behoben wird sie nicht, das steht im Kriterium |
| **11** | QTest mit überführtem Prüf-Theme; auf dem nativen Weg vorab gemessen: `cornerRun() == 0`, Alpha 255 ab Spalte 0 (F16), Kantenlauf null Stufen bei 1 **und** 1,6 | Das Theme liegt heute im **Belegordner**, nicht unter `tests/themes/`, wo `themes::addBundledThemesToDataPath()` sucht — die Überführung ist Umfang und steht im Kriterium |
| **12** | Theme-Wechsel im laufenden Dienst, Muster aus #55 (KDirWatch); je ein Bild | keine |
| **13** | Zwei Läufe **derselben Binärdatei**, Verhältnis gleichgestellt, Ausgabe gegen Ausgabe (`diff`) — das leistet ein `pruefen.sh`, nicht `ctest` | **Ein QTest läuft in einer Plattform.** Als Test zusicherbar ist nur die *Ursache*: dass das gezeichnete Bild einen Alphakanal trägt und nichts deckend gefüllt wird. Der Plattformvergleich bleibt ein Skript — im Kriterium ausgesprochen |
| **14** | `sonden/echtelage.cpp` — nimmt allein das Fenster auf; für AK 14 zusätzlich eine Aufnahme mit zwei Fenstern | **Das Urteil kann kein Agent fällen** (unten, Grenze 3) |
| **15** | `…/abnahme-befunde/sonden/eckhelligkeit.py` auf einer neuen Aufnahme derselben Stelle | keine — der Fehlschlag ist geordnet |
| **16** | Heilung entfernen, Rotwerden zeigen; Aufzählung im Übergabebericht | keine |

**Was ein Agent an dieser Story grundsätzlich nicht prüfen kann:**

1. **Die Skalierung des Kunden herstellen.** Sie ist vorzufinden, nicht
   einzustellen; `QT_SCALE_FACTOR` multipliziert unter Wayland (F4).
2. **Die Wirkung der beiden Anmeldungen ohne Bild.** Beide Funktionen sind
   `void`, es gibt keine Rückfrage, und der eine verfügbare Rückgabewert lügt
   vor der Anmeldung (F9, F10).
3. **Ob es „nativ aussieht".** Das ist der Befund, an dem #55 gescheitert ist,
   und `sprint-06.md` §25.1 sagt es klar: *„Das hat kein Prüfmittel dieses
   Projekts gefunden, und keines hätte es gefunden."* Die Frage stellt nur, wer
   das Fenster neben andere Fenster stellt und hinsieht. **Diese Story kann mit
   allen sechzehn Kriterien grün sein und trotzdem abgelehnt werden.** AK 14
   beschafft die Grundlage dafür; das Urteil bleibt beim Kunden, und das gehört
   in die Abnahmeplanung.
4. **Das *Warum* der Zeitpunktbedingung (T1).** Gemessen ist das *Ob* über sieben
   Läufe; die Wayland-Protokollebene hat niemand untersucht.

---

## Feld 5 — Größenklasse: **`size:xl`** — die Story ist nicht ziehbar

**A misst `size:m`, B hat `size:l` gemessen.** Nach der Ein-Stufen-Regel gälte
`l`. Beide Messungen sind jedoch an der **alten** Fassung mit acht Kriterien
entstanden; die drei Kundenfestlegungen und die Umarbeitung auf sechzehn
Kriterien sind danach gekommen. Ich messe deshalb neu und begründe, warum die
neue Klasse über beiden liegt.

### Warum A und B auseinandergingen — und wer worin recht hatte

A misst am **Codeumfang**, und dort hat er recht: Der Zeichenweg schrumpft.
`tinted()` (12 Z.), `mixed()` (6 Z.), `FrameContrast`, `OutlineWidth`,
`m_hullInner` und der zweite `resizeFrame()` fallen weg; hinzu kamen in der
alten Fassung zwei Aufrufe und ein Ereigniszweig. B misst am **Prüfweg**, und
dort hat B recht: #55 durfte offscreen belegen, #83 nicht mehr — B21 ist seit
demselben Tag in Kraft, und drei der acht alten Kriterien sprachen über
B21-Größen. **Der Gegensatz ist keiner: Die Story ist im Code kleiner und im
Nachweis größer als #55.** A hat die Bedingung, unter der er auf `l` ginge,
selbst benannt (ein versionierter Weichzeichner-Läufer unter `tests/`), und B
hat den Konsolidierungsfall vorweggenommen. Beide Messungen tragen; keine wird
verworfen.

### Was seit beiden Messungen hinzugekommen ist

| Zugang | Wirkung |
|---|---|
| `enableBackgroundContrast` (AK 6) | zweite Anmeldung **plus** eine Bauentscheidung mit Abhängigkeitswirkung (`libPlasma` oder Selbstlesen) |
| `opaque`-Auswahlpfad (AK 7) | dritter nativer Weg, **mit Nebenwirkung auf die Schattenkacheln** (UX B-9) |
| Schriftquelle aus der `colors`-Datei (AK 9) | **ein Mechanismus, den es heute nicht gibt** — und er muss beim Theme-Wechsel nachziehen, also einen zweiten Wächterpfad bedienen |
| Theme-Achse in AK 8, beide Textklassen in AK 10 | zweite Messmatrix (8 Themes) neben der bestehenden (19 Schemata) |
| Kriterienzahl | **8 → 16**, davon **fünf** mit Sitzungsbild (AK 4, 5, 11, 14, 15) |

Zum Vergleich: **#55 trug acht Kriterien und war die `size:l` des Sprints 6** —
und durfte offscreen belegen. #83 trägt doppelt so viele bei höherem
Nachweisaufwand je Kriterium, bringt zwei Anmeldungen und einen dritten nativen
Weg, und meine `l`-Messung ruhte ausdrücklich auf „**Build bleibt
unangetastet**". Das gilt nicht mehr.

### Zur Teilungsnaht — sie trägt, aber nicht umsonst

Ich hatte `xl` ausgeschlossen, weil ich keine Naht sah, die einen zeigbaren
Zwischenstand hinterlässt. Diese Begründung galt einer **anderen** Naht: „erst
zeichnen, dann Weichzeichner" (dieselbe, die A geprüft und ebenfalls abgelehnt
hat). Sie hinterlässt ein Fenster, das zu 84,7 % deckt und hinter dem nichts
verwischt — den Zustand, den der Kunde beanstanden würde. **Die vorgelegte Naht
ist eine andere, und sie trägt:**

- **Teil (a) enthält den ganzen nativen Vertrag**, also auch beide Anmeldungen.
  Der Zwischenstand ist damit kein halber Weichzeichner, sondern eine
  vollständige native Überlagerung.
- **Teil (b) berührt den Stand des Kunden gemessen nicht.** `default` bringt
  **keine** eigene `colors`-Datei mit (`po-themeschrift.txt`, Spalte „colors?"),
  und AK 9 sagt für diesen Fall „dann bleibt es beim Farbschema". Nach Teil (a)
  ist die Maschine des Kunden also **bereits im Endzustand**. Das ist der
  seltene Fall, in dem ein Zwischenstand zeigbar ist, weil er beim Kunden gar
  kein Zwischenstand ist.

**Was die Teilung kostet, und das gehört dem Kunden gesagt:** Teil (a) allein
**verschlechtert** die Lesbarkeit unter fremden Themes. Heute füllt `tinted()`
mit der Schemafarbe; danach zeichnet das Theme, und unter `breeze-light` steht
das Blau des Kundenschemas auf einer weißen Themefläche — **1,74:1** (B-4).
Teil (b) heilt das für zwei Themes und verschlechtert ein drittes (AK 9,
Feld 3). **Teil (a) erfüllt Kundenfestlegung 1, nicht Festlegung 2.** Wer nur
(a) liefert und dabei „ohne Anpassungen ist umgesetzt" sagt, sagt die Hälfte.

### Die Teile, mit Namen

**#83a — „Capture: der native Vertrag"** (voraussichtlich `size:l`)
Zeichnung in einem Stück, Bildpunktverhältnis samt `DevicePixelRatioChange`,
alle drei Anmeldungen (Weichzeichner, Kontrasteffekt, Auswahlpfad `opaque`)
einschließlich Neubindung bei Neuzeigen, Größenänderung und Theme-Wechsel;
Tests neu gefasst, Prüf-Theme überführt, SPEC 3.1/3.2/15/16 nachgezogen.
→ **AK 1–7, 11–16** sowie die **erste** Hälfte von AK 8.

**#83b — „Capture: Lesbarkeit unter fremden Desktop-Themes"**
(voraussichtlich `size:m`)
Schriftquelle aus der `colors`-Datei des Themes, die Theme-Achse der
Farbzusicherung, beide Textklassen mit ihren Zahlen und benannten Grenzen.
→ **AK 9, AK 10** und die **zweite** Hälfte von AK 8.

*Zur zweiten Hälfte von AK 8 (Messung über die Themes) sage ich dazu, dass sie
auch in (a) bleiben könnte* — sie ist eine Messung ohne Codeanteil. Ich lege sie
zu (b), weil sie dort dieselbe Achse misst wie AK 9 und weil sie in (a) eine
Verschlechterung dokumentieren würde, für die (a) keine Antwort hat. Das ist
eine Ermessensentscheidung, keine gemessene; der PO kann sie anders schneiden,
ohne dass etwas bricht.

*Reihenfolge:* (b) setzt (a) sachlich voraus — seine Messgrundlage („die Fläche
gehört dem Theme") entsteht erst mit (a). Zwei Sprints, nicht zwei Stränge.

**Folge für den Schnitt:** Solange #83 ungeteilt ist, trägt es `size:xl` und ist
**nicht ziehbar** (`PROZESS.md`, Größenklassen). Nach dem Schnitt gilt für (a)
die Regel „neben `size:l` steht nur `size:s`"; #81 fällt ohnehin aus (§1.1),
#79 nur bei getrennten Belegordnern.

**Label gesetzt:** `size:xl`, im selben Zug wie dieser Bericht (04.08.2026,
19:59 CEST) — vorher trug #83 keins.

---

## Feld 6 — Offene Fragen

### An den Kunden — Vorlage 1: Zwei Deiner drei Festlegungen widersprechen einander

Wortlautvorschlag, wie er vorzulegen wäre:

> **Worum es geht.** Du hast zwei Dinge festgelegt, die sich bei einem Theme
> gegenseitig ausschließen. Wir haben es gemessen, bevor wir bauen.
>
> - **Festlegung 2:** „unter jedem Desktop-Theme lesbar."
> - **Festlegung 3:** „die Schrift kommt aus derselben Quelle wie die Fläche —
>   bringt das Theme eine eigene Farbdatei mit, gilt sie."
>
> Vier Deiner acht Themes bringen eine eigene Farbdatei mit. Was Festlegung 3
> dort bewirkt:
>
> | Theme | heute (Schemaschrift) | mit Themeschrift |
> |---|---|---|
> | Breeze hell | 1,75:1 — unlesbar | **13,35:1** |
> | Breeze dunkel | 7,94:1 | **15,39:1** |
> | cachyos-emerald-color | 10,56:1 | 9,57:1 |
> | **cachyos-emerald-light** | **10,56:1 — gut** | **1,38:1 — unlesbar** |
>
> Bei den ersten beiden ist Festlegung 3 die Rettung. Beim letzten macht sie
> aus einem guten Wert einen unbrauchbaren — dort verletzt Festlegung 3 also
> Festlegung 2.
>
> *Eine Einschränkung zur Ehrlichkeit der Zahlen:* Die drei
> `cachyos-emerald`-Themes zeichnen fast keine Fläche (Deckung 3 %). Die Werte
> oben sind gegen eine gedachte, volle Fläche gerechnet; in Wirklichkeit steht
> der Text dort auf Deinem Bildschirmhintergrund, und keine Schriftfarbe
> repariert das. Der Widerspruch bleibt trotzdem echt — er verschiebt sich nur
> von „unlesbar" zu „nicht zugesichert".
>
> **Drei Wege:**
>
> 1. **Festlegung 3 gilt, Festlegung 2 wird zu „gemessen und benannt".** Wir
>    nehmen die Themeschrift überall, halten fest, unter welchen Themes die
>    Lesbarkeit dann nicht zugesichert ist, und heilen nichts. Am nächsten an
>    „ohne Anpassungen".
> 2. **Festlegung 3 gilt mit Rückfall.** Themeschrift, außer sie ist schlechter
>    als die Schemaschrift — dann bleibt die Schemaschrift. Kein Theme wird
>    schlechter als heute. **Das ist eine Anpassung**, eine, die nur im
>    Schadensfall greift.
> 3. **Festlegung 2 gilt vorrangig.** Wir wählen je Theme die besser lesbare
>    der beiden Quellen. Das ist dieselbe Anpassung wie 2, nur ohne die
>    Vorzugsrichtung.
>
> **Zur Reihenfolge, falls Du Weg 1 oder 2 wählst:** Die Story ist zu groß für
> einen Sprint. Wir schlagen vor, sie zu teilen — erst der native Vertrag
> (Zeichnung und alle drei Anmeldungen; auf **Deiner** Einstellung ist danach
> alles fertig), dann die Lesbarkeit unter fremden Themes. **Wichtig dabei:**
> Nach dem ersten Teil sind sechs Deiner acht Themes schlechter lesbar als
> heute; erst der zweite Teil bringt sie zurück. Auf Deiner eigenen Einstellung
> ändert sich nichts.

*Anmerkung an den PO zur Vorlage:* Weg 2 und Weg 3 widersprechen dem Wortlaut
„ohne Anpassungen" unterschiedlich stark. Ich lege sie vor, statt sie
vorzusortieren — die Ablehnung von #55 ist daran entstanden, dass eine
Kategorienfrage nie gestellt wurde (§25.1). Zweimal ist einmal zu oft.

### An den PO

1. **#83 schneiden** (Feld 5). Bis dahin ist die Story nicht ziehbar.
2. **AK 4 und AK 7 neu fassen** — beide sind gegen die eigene Messgrundlage
   nicht erfüllbar (§0.1, §0.2). Das ist Formulierungsarbeit von je drei Sätzen,
   aber sie muss vor dem Ziehen geschehen, sonst baut ein Strang gegen ein
   falsches Ziel.
3. **AK 9** hängt an der Kundenvorlage oben.
4. **Fünf Auflagen** an AK 1, 3, 5, 8, 14 — je ein bis zwei Sätze (Feld 3).
5. **Die drei B17-Fundstellen** (`CLAUDE.md:40`, `PROZESS.md:427`,
   `.claude/agents/denkzettel-dev.md:81`) ankern, **sobald `tinted()` fällt** —
   nicht vorher, sonst ankern wir einen Stand, den es noch gibt. Der Satz
   „gemessen: `tinted()` verliert offscreen den Alphakanal" steht an allen drei
   Stellen im **Präsens**; in `CLAUDE.md` ohne genannten Prüfstand, und das ist
   der kritische Fall (Sprint 6, §19.4). B21 selbst bleibt richtig — nur seine
   Begründung wird historisch. **Gehört dem PO, nicht dem Strang.**
6. **`CLAUDE.md` nachzuziehen, sobald die Story läuft** (melden, nicht heilen):
   die Liste der fünf Bildläufer und die Aussage zu `tinted()` — siehe Punkt 5.
   Vorher nichts.
7. **Bestandsbefund**: Die Schattenpolsterung wird in `bindShadow()` gesetzt und
   bei `resizeHull()` nicht nachgezogen (UX §6.6). Eigenes Issue oder bewusst
   liegen lassen — nicht in #83 hineinziehen.
8. **Die Abnahme dieser Story braucht den Vergleich neben anderen Fenstern.**
   AK 14 beschafft das Bild; das Urteil fällt kein Kriterium und kein Review.
   Gehört in die Abnahme-Checkliste, zusammen mit dem Punkt „**Schatten ist
   ausdrücklich nicht enthalten**", damit er nicht als neuer Befund wiederkehrt
   (UX B-3).

### Zum Schließen von #55 — nachgeprüft, kein Einwand

Ich hatte empfohlen, #55 zu schließen, aber **nicht** als „not planned", weil
das nach Wegräumen aussieht. Der PO hat „not planned" gewählt und im Kommentar
ausgesprochen, dass es **diese Fassung** der Story meint, welche vier Kriterien
geliefert sind und was im Produkt bleibt. **Damit ist mein Einwand gegenstandslos:**
Er galt der Wirkung, und der Kommentar hebt sie auf. „Completed" wäre die
falschere Wahl gewesen — der Kunde hat die Story nicht angenommen. Die Tabelle
je Kriterium im Schließkommentar deckt sich mit meiner Durchsicht. **Kein
Handlungsbedarf.**

---

## 7. Verfahrensbefunde — was am neuen Vorprüfbericht beim ersten Lauf geklemmt hat

Dies war der erste Lauf des Verfahrens (`PROZESS.md`, Sprint-Mechanik, in Kraft
seit `2ad4eef`). Vier Änderungen sind mit diesem Bericht in `PROZESS.md`
eingetragen, die fünfte habe ich nach Ermessen **nicht** vorgenommen.

**7.1 Ablage: ein Ordner je Story.** Die Fassung sagte „je Story eine Datei" und
ist beim ersten Anfassen gerissen — **beide anderen Bearbeiter haben die
Abweichung unabhängig gemeldet** (`messung-b.md` §1, `ux-beratung.md` §0.2). Zwei
unabhängige Bearbeiter können nicht in dieselbe Datei schreiben, ohne
voneinander zu erfahren, und die Messausgaben brauchen ohnehin einen Ordner
(B7). **Geändert:** `docs/scrum/vorberichte/NN-<kurzname>/` mit `messung-a.md`,
`messung-b.md`, `bericht.md` und den Messausgaben.

**7.2 Selbstdeklarierte offene Punkte.** Ein Issue, das selbst sagt, es sei
unfertig, ist nicht ready — unabhängig von seinen Kriterien. Bei #83 alt stimmte
das Ergebnis nur zufällig, weil auch die Kriterien nicht trugen; **die Regel
hätte es nicht gefunden, wenn sie getragen hätten.** **Geändert:** ein Satz in
der DoR.

**7.3 Ein Dateiname ist erst ein Prüfmittel, wenn `git ls-files` ihn zeigt.**
`native-farben.txt` hat die DoR wörtlich erfüllt und existiert nicht (B-5).
Dieselbe Bauart wie die Existenzprüfung des Verwalter-Berichts
(Sprint-Abschluss, Punkt 11): Es fehlte nicht die Regel, es fehlte ihre
Prüfbarkeit. **Geändert:** ein Satz in der DoR. *Anwendungsfall aus diesem
Bericht:* Alle in der neuen Fassung genannten Prüfmittel habe ich mit
`git ls-files` geprüft — **sie existieren alle**; geblieben ist die fehlende
Pfadwurzel (AK 8, Auflage).

**7.4 B21 muss die Stelle erreichen, an der Kriterien formuliert werden.** In
der alten Fassung sprachen drei von acht Kriterien über B21-Größen, keines nannte
ein Sitzungsbild — das Issue entstand am selben Tag wie der Beschluss. In der
neuen Fassung ist es an fünf Stellen ausdrücklich benannt; die Regel hat
gewirkt, weil ein Mensch sie angewandt hat, nicht weil sie am richtigen Ort
stand. **Geändert:** ein Satz in der DoR.

**7.5 Feldreihenfolge — keine Änderung, und das ist eine Entscheidung.** Der
Bericht führt das Ready-Urteil als Feld 3 und die Prüfmittel als Feld 4, obwohl
sich am Prüfmittel entscheidet, ob ein Kriterium prüfbar ist; gearbeitet habe
ich beide Male in der Reihenfolge 4 vor 3. **Ich nummeriere trotzdem nicht um.**
Eine Umnummerierung entwertet jede bestehende Fundstelle — „Feld 3 fällt der
Scrum Master" steht dreimal in `PROZESS.md`, in beiden Messungen, in der
UX-Beratung und in diesem Bericht —, und der Gewinn wäre eine hübschere
Reihenfolge. Stattdessen ein halber Satz am Feld selbst. *Simplicity First: die
Nummer beschreibt den Bericht, nicht den Arbeitsgang.*

**Und eine Beobachtung, die für das Verfahren spricht.** Die drei tragenden
Befunde dieses Berichts (§0) sind **keinem** der drei Bearbeiter allein
zugänglich gewesen: §0.1 braucht A's Messung **und** UX' Vorschlag, §0.2 braucht
den Blick auf die Skalierung des Kunden statt auf die Überschrift, §0.3 braucht
A's Gegenmessung zu einer Annahme, die B und UX **beide** für selbstverständlich
hielten. Zwei unabhängige Bearbeiter plus Konsolidierung ist kein Ritual — an
diesem Fall hat jede der drei Stufen etwas beigetragen, das die anderen nicht
hatten. **Der teuerste Einzelbefund (§0.2) stammt allerdings aus keiner der drei
Messungen, sondern aus dem Nachlesen der Belegdatei, die das Issue selbst
zitiert.** Auch das ist ein Ergebnis: Wer eine Messung zitiert, liest sie ganz.

---

## Belege dieses Berichts

Alle versioniert (B7), sämtlich mit `git ls-files` geprüft:

| Ort | Inhalt |
|---|---|
| `messung-a.md`, `messungen/sonde1…sonde5*`, `bilder/`, `sonden/`, `pruefen.sh` | Bearbeiter A |
| `messung-b.md`, `messung-b-themefarbe.txt`, `messung-b-testfolgen.txt`, `themefarbe.cpp`, `testfolgen.cpp` | Bearbeiter B |
| `ux-beratung.md`, `ux-beratung/messungen/m1…m6`, `ux-beratung/bilder/`, `ux-beratung/sonden/` | Planning-Beratung |
| `po-themeschrift.py`, `po-themeschrift.txt` | Messung des PO zur Schriftquelle |
| `sm-textrolle.cpp`, `sm-textrolle.txt` | Nachmessung dieser Konsolidierung (AK 10) |
| `docs/scrum/reviews/2026-08-04-abnahme-befunde/messungen/native-huelle-*.txt`, `b1-echtelage.txt` | Grundlage von §0.2 |

Wiederholbar:
`bash docs/scrum/vorberichte/83-native-huelle/pruefen.sh` und
`bash docs/scrum/vorberichte/83-native-huelle/ux-beratung/pruefen.sh`.

---

## Nachtrag: Vollzug durch den Product Owner, 04.08.2026

Dieser Abschnitt hält fest, was aus den Befunden dieses Berichts geworden ist.
Der Bericht selbst bleibt unverändert — er hat richtig protokolliert, was zum
Zeitpunkt der Messung galt.

### Die Kundenentscheidung zum Festlegungskonflikt (Feld 6, Vorlage 1)

Vorgelegt am 04.08.2026 mit den drei Wegen. **Der Kunde hat den ausnahmslosen
Weg gewählt:** Bringt das Desktop-Theme eine eigene `colors`-Datei mit, gilt
ihre Textfarbe — immer. Damit ist der Konflikt entschieden, nicht aufgelöst:
`cachyos-emerald-light` fällt von 10,56:1 auf 1,38:1, und das ist in Kauf
genommen. Der Einwand dieses Berichts, die Zahl sei gegen eine Fiktion
gerechnet, ist als eigenes Kriterium aufgenommen (#85 AK 2: am laufenden Stand
nachmessen, nicht deckend rechnen).

### Der Schnitt

`size:xl` vollzogen entlang der im Bericht geprüften Naht:

| | Titel | Klasse | Kriterien |
|---|---|---|---|
| **#83** | Hülle als native Plasma-Überlagerung — der native Vertrag | `size:l` | 14 |
| **#85** | Lesbarkeit unter fremden Desktop-Themes | `size:m` | 5 |

#83 behält die Nummer, weil sie in #55, #84 und in den Protokollen referenziert
ist. Die Kehrseite, die dieser Bericht benannt hat, steht ausdrücklich in beiden
Issues: **Nach #83 allein sind sechs von acht Themes schlechter als heute.**

### Die vier Befunde

- **AK 4** neu gefasst: Richtung *fallend* statt wachsend, Anstieg bis zum
  **Randwert** statt bis zur Flächendeckung, und statt „höchstens ein Bildpunkt
  je Zeile" die Zahl der Stufen (bei Verhältnis 1 keine von zwei oder mehr
  Spalten, bei 1,6 höchstens eine). Der geometrische Einwand trägt: Ein
  Kreisbogen kann die alte Fassung nie erfüllen.
- **AK 7** neu gefasst: Die Bedingung hängt nicht mehr an
  `isEffectAvailable(BlurBehind)`, sondern an der Globalenliste des Compositors.
  Der Widerspruch zu AK 5 ist damit fort.
- **AK 9 alt** ist mit der Kundenentscheidung nach #85 gewandert und trägt dort
  die Folge als eigenes Kriterium statt als „benannte Grenze".

### Die fünf Auflagen

Alle eingearbeitet: `-- src/` an AK 1 · der 1,6-Beleg aus der angemeldeten
Sitzung ohne `QT_SCALE_FACTOR` an AK 3 · der geordnete Fehlschlag für die
Region an AK 5 · Pfade ab Repositoriumswurzel an AK 8 · KRunner benannt und die
Beurteilung ausdrücklich der Kundenabnahme zugewiesen an AK 12. Dazu der
Klammerzusatz `QPalette::PlaceholderText` in #85 AK 4, der einem Strang die
Suche nach einem Bezeichner erspart, den es im Code nicht gibt.

### Offen

**Das Ready-Urteil auf die geschnittene Fassung steht aus.** Es fällt der Scrum
Master, nicht der PO — die Befunde sind abgearbeitet, die Bestätigung fehlt.
Ebenfalls offen: der Bestandsbefund zur Schattenpolsterung (`bindShadow()` ohne
Nachzug bei `resizeHull()`), noch nicht gebucht.
