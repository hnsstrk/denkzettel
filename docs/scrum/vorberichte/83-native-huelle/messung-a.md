# Vorprüfung #83 — Messung Bearbeiter A (`denkzettel-dev`)

**Gegenstand:** Issue #83, „Capture: Hülle als native Plasma-Überlagerung — den
Nachbau entfernen" · **Datum:** 04.08.2026, Ganymed · **Quellstand:** `main` @
`80b52ae` · **Belege:** `messungen/`, `bilder/`, Sonden in `sonden/`,
wiederholbar über `bash docs/scrum/vorberichte/83-native-huelle/pruefen.sh`

Dieser Bericht trägt die Felder **1, 2, 4 und 5** des Vorprüfberichts
(`docs/scrum/PROZESS.md`, Sprint-Mechanik). **Feld 3 (Ready-Urteil) fällt der
Scrum Master**, Feld 6 steht als „Offene Fragen" am Ende.

**Stand der Werkzeuge** (B17 — eine Aussage gilt für einen Stand):
kwin 6.7.3, kwindowsystem 6.28.0, ksvg 6.28.0, qt6-base 6.11.1, plasma-desktop
6.7.3. Sitzungsskalierung des Kunden: **1,6** (der Bildschirm meldet DPR 2,
das Fenster bekommt 1,6 — siehe F3). Beleg:
`messungen/sonde5-stand-und-globale.txt`.

---

## Feld 1 — Dateimenge (am Code vermessen, Notation nach B13 / Sprint 6 §5.2)

| | **#83** — die Hülle als native Überlagerung |
|---|---|
| **Issue** | **#83** (`epic:M1`, `typ:story`) |
| **Zweig** | `story/83-native-huelle` |
| **Quellen & Tests** | `src/capture/capturewindow.{h,cpp}` — **ganz** (471 bzw. 99 Zeilen); die Fläche verteilt sich über die Datei: Konstanten `:44–54` (`OutlineWidth`, `FrameContrast`), `mixed()`/`tinted()` `:69–94` (**beide fallen weg**), Konstruktor `:143–158` (`m_hullInner` entfällt), `reloadDesktopTheme()` `:241`, `paintEvent()` `:309–330` (**neu geschrieben**), `resizeHull()` `:338–343`, `present()` `:419–430` (Weichzeichner-Anmeldung), Kopfdatei `:93–97`. Dazu **neu**: eine Behandlung von `QEvent::DevicePixelRatioChange` (F3).<br>`tests/capturetest.cpp` — die vier Hüllen-Zusicherungen `:411–433`, `:499–549`, `:334–393`, sowie ein neuer Nachweis zu AK 2/AK 4.<br>`tests/captureshots.cpp` — die Bildreihe (der Grund hinter `shoot()` ist bei durchscheinender Hülle jetzt **Messmittel**, nicht Schmuck). |
| **Build** | **Nichts.** Gemessen: `KF6::WindowSystem` steht bereits in `target_link_libraries(denkzettelcapture …)` (`src/CMakeLists.txt`), `WindowSystem` bereits in der `find_package(KF6 …)`-Liste der Wurzel (`CMakeLists.txt:24–33`). `KWindowEffects` braucht nur einen `#include`. *Ausnahme:* Verlangt der PO einen eigenen, versionierten Läufer für den Weichzeichner-Beleg (siehe Feld 4, AK 5), kommt ein Block in `tests/CMakeLists.txt` hinzu — **und dann steigt die Größenklasse** (Feld 5). |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-NN-s83-native-huelle/` — neu anzulegen, mit eigenem `pruefen.sh` nach dem Muster von `sprint-06-s55-huelle/`. **Wiederverwendbar, nicht neu zu erfinden:** `sonden/weichzeichnerbeleg.cpp` dieser Vorprüfung ist der fertige Bauplan des AK-5-Belegs; `docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/eckhelligkeit.py` ist das Prüfmittel für den hellen Streifen; `…/pruef-theme/` ist das Theme für AK 7. |
| **Fachliche Quellen** | **SPEC 3.1** (`:152–186`) — der Absatz „Form kommt vom Theme, Farbe aus der Palette" beschreibt genau den Weg, den #83 abschafft; die Zeile zu `frameContrast` (`:170–173`) fällt ersatzlos. **SPEC 3.2** (`:188–214`) — nimmt die neuen Bedingungen auf (F3, F7). **SPEC 15** (`:720–735`) — die KF6-Liste kennt `KWindowEffects` nicht. **SPEC 16** (`:770–796`) — „was offscreen prinzipbedingt nicht zu belegen ist" bekommt den Weichzeichner dazu, und zwar mit der schärferen Aussage, dass ihn auch eine **Fensteraufnahme** nicht zeigt. Wireframe 4b: **neue Aussage nötig, aber nicht durch den Dev** — `wireframes/` bleibt UX. |
| **Ausdrücklich nicht** | `src/ui/*`, `src/shell/*`, `src/store/*`, `src/capture/textareaheight.{h,cpp}` (die Höhenformel gehört #79/#81, nicht dieser Story), `tests/librarytest.cpp`, `tests/libraryshots.cpp`, `tests/editshots.cpp`, `tests/searchshots.cpp`, `tests/readmeshots.cpp`, `wireframes/`, sowie sämtliche Belegordner fremder Sprints. |

### Kollisionsfläche gegen #55, #79 und #81 — **keine läuft parallel**

Alle drei arbeiten in **derselben Datei**, `src/capture/capturewindow.cpp`.
Gemessen an den Zeilenbereichen:

| Vorgang | Bereich in `capturewindow.cpp` | Kleinster Abstand zu #83 | Urteil |
|---|---|---|---|
| **#55** (Hülle, angenommen, Issue offen) | `paintEvent()` `:309–330`, Konstruktor, `resizeHull()` — die Stellen, an denen AK 1, 2 und 4 hängen | **null** — #83 schreibt genau diesen Code neu und **streicht ein AK von #55** | nicht parallel, und zwar begrifflich, nicht nur technisch |
| **#79** (Fenster schrumpft nicht) | `adjustHeight()` `:462–471`; die Heilung muss am **gezeigten** Fenster greifen, was auf `present()` `:419–430` zeigt | **null**, falls #79 `present()` anfasst; sonst 32 Zeilen (`:430` → `:462`) | nicht parallel — #83 fasst `present()` für die Weichzeichner-Anmeldung an |
| **#81** (Textkante) | `m_text`-Aufbau `:161–167` (`documentMargin`), Höhenformel `:467` | **3 Zeilen** (`:158` → `:161`) | nicht parallel — Git mischt mit drei Zeilen Kontext, die Hunks berühren sich |

**Woran ich das messe:** an den Zeilenbereichen der Funktionen (`grep -n
'^void CaptureWindow::'`) und an der Frage, welche Funktion die jeweilige
Heilung anfassen *muss*. Der Sprint-6-Maßstab lautete „der kleinste Abstand ist
null: beide schreiben in dieselbe 157-Zeilen-Datei" (§5.1). Die Datei ist
inzwischen 471 Zeilen lang, aber die Bereiche überlappen trotzdem — bei #55
vollständig, bei #81 innerhalb der Mischbreite von Git.

**Zusätzlich zur Textkollision:** #83, #79 und #81 ändern alle drei dieselbe
Bildreihe (`captureshots`). Zwei Stränge, die gleichzeitig zwölf Bilder
neu erzeugen, überschreiben einander die Belege.

---

## Feld 2 — Gemessene Fallen (die Zeilen für den Spawn-Auftrag)

Muster: Sprint 6 §10.6, „Punkte, die je einen Fehlversuch ersparen". Jede
Zeile mit Messbeleg.

### Zum Bildpunktverhältnis

**F1 — `KSvg::FrameSvg` folgt dem Bildschirm nicht von selbst.**
`devicePixelRatio()` ist nach dem Bau **1**, gleich welche Skalierung gilt; die
Anwendung muss den Wert setzen.
*Beleg:* `sonde1-rahmenmasse-offscreen.txt`, Abschnitt A.

**F2 — Die Reihenfolge ist gleichgültig, und ein DPR-Wechsel allein genügt.**
`setDevicePixelRatio()` vor `resizeFrame()` und danach liefern beide
960×279 bei DPR 1,6; ein späterer Wechsel des Verhältnisses wirkt **ohne**
erneutes `resizeFrame()`. Wer hier eine Reihenfolge sucht, sucht umsonst.
*Beleg:* `sonde1-…-offscreen.txt`, Abschnitte B und C.

**F3 — Das Verhältnis des Fensters steht nach `show()` noch nicht fest, und
es ändert sich ohne Größenänderung.** Unter Wayland meldet Qt zunächst **2**
und rund eine Sekunde später **1,6**; zugestellt wird das als
`QEvent::DevicePixelRatioChange`, **ohne begleitendes `QEvent::Resize`**. Wer
die Hülle nur im `resizeEvent()` nachzieht, zeichnet dauerhaft bei 2 auf einem
Fenster, das 1,6 ist.
*Beleg:* `sonde2-fensterlauf-wayland-skala-1.txt` — Ereignismitschrift
„Resize, DevicePixelRatioChange, DevicePixelRatioChange", dazu die Zeile
„Hülle bekam dabei zuletzt 2 … Fenster bei 1,6".
**Offscreen tritt der Fall nicht auf** (Mitschrift: nur „Resize") — der Fehler
ist offscreen unsichtbar, und ein Bildbeleg offscreen zeigt ihn nie.

**F4 — `QT_SCALE_FACTOR` multipliziert unter Wayland mit der
Sitzungsskalierung.** `QT_SCALE_FACTOR=1` ergibt Fenster-DPR **1,6**,
`QT_SCALE_FACTOR=1,6` ergibt **2,56**. Offscreen ergibt `1,6` genau 1,6. Wer
den Kundenstand in der Sitzung nachstellen will, setzt die Variable
**gar nicht**; wer ihn offscreen nachstellt, setzt sie auf 1,6.
*Beleg:* `sonde2-fensterlauf-wayland-skala-1.txt` gegen `-skala-1.6.txt` gegen
`sonde2-fensterlauf-offscreen-skala-1.6.txt`.

### Zur Maske und zum Weichzeichner

**F5 — `mask()` liefert logische Bildpunkte, und sie ist nicht leer.**
Das Hüllrechteck ist bei DPR 1 **und** bei 1,6 genau 600×174 — also logisch,
genau das Koordinatensystem, das `enableBlurBehind` laut Kopfdatei erwartet
(„specified in logical pixels"). Umgerechnet wird nichts. Und obwohl die native
Hülle nur zu 84,7 % deckt, enthält `mask()` 7 Rechtecke mit 104.352 von 104.400
Bildpunkten: Sie stammt aus den `mask-`Elementen, die voll decken (Alpha 255 in
der Mitte, 0 in der Ecke).
*Beleg:* `sonde1-…-offscreen.txt`, Abschnitte D und E2.

**F6 — `enableBlurBehind(nullptr, …)` stürzt unter Wayland ab.**
Rückgabe 139 (SIGSEGV); offscreen kehrt derselbe Aufruf zurück. Wer die
Anmeldung in den Konstruktor legt, wo `windowHandle()` noch `nullptr` ist,
bekommt keinen Fehlschlag, sondern einen Absturz — **und offscreen fällt er
nicht auf**.
*Beleg:* `sonde2-fensterlauf-wayland-skala-1.txt`, Abschnitt E bricht ab;
`…-offscreen-skala-1.txt`, Abschnitt E läuft durch.

**F7 — Der Weichzeichner wirkt nur, wenn er unmittelbar nach `show()`
angemeldet wird. Der Zeitpunkt entscheidet, und nur er.**
A/B über sieben Läufe mit Vollbild-Schachbrett hinter der Hülle, gemessen an
der Helligkeitsspannweite auf einer festen Prüflinie durch die Hülle:

| Lauf | Anmeldung | Spannweite | Befund |
|---|---|---|---|
| `aus` | keine | 203–242 | Schachbrett scharf |
| `an` | einmal, 1,2 s nach `show()`, mit Maskenregion | 203–242 | **kein Weichzeichner** |
| `zweimal` | zweimal, 200 ms auseinander, spät | 203–242 | **kein Weichzeichner** |
| `spaet-leer` | spät, mit **leerer** Region (= ganzes Fenster) | 203–242 | **kein Weichzeichner** |
| `frueh` | sofort nach `show()` | 222–223 | **weichgezeichnet** |
| `wiederzeigen-neu` | früh, nach `hide()`/`show()` erneut | 222–223 | weichgezeichnet |
| `wiederzeigen-alt` | früh, nach `hide()`/`show()` **nicht** erneut | 222–223 | weichgezeichnet |

*Beleg:* `messungen/sonde4-weichzeichner-*.txt`, Bilder
`bilder/weichzeichner-pruefline-*.png`. Der Lauf `spaet-leer` schließt die
Region als Ursache aus: es liegt am Zeitpunkt, nicht an der Maske.

**F8 — Die Anmeldung überlebt ein `hide()`/`show()` — anders als der
Schatten.** `wiederzeigen-alt` bleibt weichgezeichnet, ohne erneute Anmeldung.
*Grenze, ausdrücklich:* gemessen mit unmittelbarem `hide()`/`show()`, **nicht**
mit dem 50-ms-Umweg, den `showCapture()` nimmt. Wer sich darauf verlässt,
misst es an der echten Klasse nach.

**F9 — `isEffectAvailable(BlurBehind)` ist träge und taugt nur *nach* der
Anmeldung als Zurücklesen.** Vor dem ersten `enableBlurBehind()` liefert es in
der angemeldeten Plasma-Sitzung **`false`**, danach **`true`** — der Wert sagt
also nicht, was der Compositor kann, sondern ob die Wayland-Erweiterung schon
gebunden ist.
*Beleg:* `sonde2-…-wayland-skala-1.txt` (`false`, `false` nach dem Aufruf im
selben Durchlauf) gegen `sonde4-weichzeichner-frueh.txt` (`true`, nachdem früh
angemeldet wurde). **Kandidat für die Liste „Rückgabewerte, die nichts
belegen" in `.claude/agents/denkzettel-dev.md`.**

**F10 — `enableBlurBehind()` ist `void`.** Es gibt keinen Rückgabewert, den man
missdeuten könnte — und keinen, der etwas belegte.
*Beleg:* `/usr/include/KF6/KWindowSystem/kwindoweffects.h`.

### Zu den Tests und zum Bild

**F11 — Drei heutige Zusicherungen fallen, und zwar auf beiden Plattformen.**
An einem Fenster nach dem nativen Weg nachgerechnet:

- `hullIsCompleteAtFiveAndEightLines()` prüft `qAlpha(...) == 255` an fünf
  Stellen. Die native Hülle liefert **216** in der Mitte und **235** an den
  Kantenmitten. **Fällt.**
- `paintsOneSurfaceInThePaletteColours()` prüft `QCOMPARE(Pixel, palette
  Window)`. Die Farbe stimmt (32,35,38 gegen 32,35,38), das **Alpha nicht**
  (216 gegen 255) — und `QColor` vergleicht das Alpha mit. **Fällt.**
- `cornerRun() > 0` in `checkHullDiffersBetween()` und
  `hullIsCompleteAtFiveAndEightLines()` **hält** (4 bei DPR 1,6), ebenso
  `wideCorner != narrowCorner` (11 gegen 3 bei DPR 1 über die mitgelieferten
  Themes, F16).

*Beleg:* `sonde2-fensterlauf-*`, Abschnitt B, offscreen wie Wayland.

**F12 — AK 4 ist erfüllbar, aber nicht so, wie es dasteht.** Offscreen und
Wayland liefern bei **gleichem Bildpunktverhältnis** Zahl für Zahl dasselbe
Bild: beide 960×278, Mitte 32,35,38/Alpha 216, `cornerRun` 4, oberste Zeile
`0 0 0 0 2 65 125 173 195 209 235 …`. Bei **gleichem `QT_SCALE_FACTOR`** tun
sie es **nicht**, weil die Plattformen verschiedene Verhältnisse einstellen
(F4) — und solange die Hülle dem Fenster hinterherhinkt (F3), erst recht nicht
(`cornerRun` 5 gegen 4, andere Alphazeile).
*Beleg:* Abschnitt A2 in `sonde2-…-offscreen-skala-1.6.txt` gegen
`sonde2-…-wayland-skala-1.txt`.

**F16 — Die mitgelieferten Prüf-Themes decken voll, die installierten nicht.
Ein Test darüber bleibt grün, obwohl sich das Verhalten geändert hat.**
Auf dem nativen Weg gemessen:

| Theme | Herkunft | Alpha in der Mitte | Randwert der Kante |
|---|---|---|---|
| `denkzettel-test-schmal` | `tests/themes/` | **255** | 255 |
| `denkzettel-test-breit` | `tests/themes/` | **255** | 255 |
| `denkzettel-pruef-eckig` | Abnahme-Belege | **255** | 255, Anstieg 0 Spalten (eckig, wie gewollt) |
| `default` | installiert | **216** | 235 |

`hullIsCompleteAtFiveAndEightLines()` läuft über beide mitgelieferten Themes
**und** über `themes::anyInstalledTheme()`. Die Zusicherung
`qAlpha(...) == 255` fällt also **nur am installierten Theme** — auf einem
Bauplatz ohne installierte Desktop-Themes bliebe der Test grün, während die
Hülle im Betrieb durchscheint. Das ist die Bauart, gegen die `CLAUDE.md` seine
Prüfhaltung fasst; wer die Zusicherung anpasst, muss sie so anpassen, dass sie
**am mitgelieferten Theme genauso etwas prüft**.
*Beleg:* `messungen/sonde1-pruefthemes.txt`.
*Nebenbefund für AK 7:* Das eckige Prüf-Theme liefert auf dem nativen Weg
Alpha 255 ab Spalte 0 — `cornerRun() == 0`, also eine rechteckige Ecke. Das AK
ist damit als Zahl prüfbar.

**F13 — Am Build ist nichts zu tun.** `KF6::WindowSystem` ist bereits
verkabelt (siehe Feld 1). Wer danach sucht, sucht umsonst.

**F14 — „Ohne Anpassungen" umfasst bei Plasma zwei Aufrufe, nicht einen.**
`libPlasmaQuick` — die Bibliothek hinter Plasmas eigenen Überlagerungen — ruft
**`enableBlurBehind` und `enableBackgroundContrast`**. AK 5 nennt nur den
ersten.
*Beleg:* `sonde3-plasma-anmeldungen.txt` (`nm -D -u`).

**F15 — Der Compositor bietet die alten KDE-Protokolle nicht mehr an.**
`org_kde_kwin_blur_manager` und `org_kde_kwin_contrast_manager` stehen nicht in
der Globalenliste; an ihrer Stelle steht `ext_background_effect_manager_v1`.
`org_kde_kwin_shadow_manager` gibt es weiterhin — **der Schatten läuft
unverändert.** Das KF6-Plugin kennt beide Wege.
*Beleg:* `sonde5-stand-und-globale.txt`. Die Fallen F7 und F9 sind
Eigenschaften **dieses Standes** (B17).

---

## Feld 4 — Prüfmittel je Akzeptanzkriterium, und was ein Agent nicht kann

| AK | Prüfmittel | Grenze |
|---|---|---|
| **1** — ein Stück, `tinted()`/Ring/`frameContrast` entfernt | `git grep -n "tinted\|FrameContrast\|m_hullInner\|alphaMask\|OutlineWidth" -- src/` muss **leer** sein; dazu Mutationsprobe an der neuen Zeichenzeile | keine |
| **2** — Verhältnis der Grafik folgt dem des Fensters, belegt bei 1 und 1,6 | **Test offscreen** mit `QT_SCALE_FACTOR` (dort ist das Verhältnis stabil und steuerbar, F4): `framePixmap().devicePixelRatio() == devicePixelRatioF()`. **Dazu ein Sitzungslauf ohne `QT_SCALE_FACTOR`** — nur dort tritt der Nachziehfehler aus F3 überhaupt auf | Ein Agent kann das Verhältnis **in der Sitzung nicht wählen**; es kommt vom Compositor, `QT_SCALE_FACTOR` multipliziert nur (F4). Der Kundenstand 1,6 ist in der Sitzung nur *vorgefunden*, nicht *eingestellt* |
| **3** — Alphaverlauf der obersten Zeile monoton, kein Plateau | Sonde 1 misst es als vier Zahlen: Randwert, Länge des Anstiegs, Rückschritte, Wiederholungen im Anstieg. Gemessen: **235 / 6 bzw. 10 Spalten / 0 / 0** | **Der AK-Wortlaut ist zu weit.** Die gerade Oberkante *ist* ein Plateau — sie hält 235 über hunderte Spalten. Prüfbar ist allein der **Anstieg bis zum Randwert**. Ohne diese Einschränkung ist das AK von keiner Hülle erfüllbar (siehe Offene Fragen) |
| **4** — offscreen und Wayland liefern dasselbe Bild | Zwei Läufe **derselben Binärdatei**, Verhältnis gleichgestellt, Ausgabe gegen Ausgabe (`diff`) — das leistet `pruefen.sh`, nicht `ctest` | **Ein QTest läuft in einer Plattform.** Als Test zusicherbar ist nur die *Ursache*: dass das gezeichnete Bild einen Alphakanal trägt (Format 6) und dass nichts deckend gefüllt wird. Der Plattformvergleich selbst bleibt ein Skript. Das ist eine benannte Grenze, keine Auslassung |
| **5** — Weichzeichner über `enableBlurBehind` mit der Maskenregion angemeldet | **Der einzige belastbare Beleg ist eine Bildschirmaufnahme mit bekanntem Muster dahinter.** Sonde 4 zeigt, dass das geht *und datensparsam geht*: eigenes Vollbild-Schachbrett unter dem Fenster, `spectacle -f`, Vollbildpuffer sofort löschen, nur den Ausschnitt behalten. A/B gegen einen Lauf ohne Anmeldung | **`spectacle -a` (nur das Fenster) taugt nicht** — sie liefert die eigene Fläche mit Alpha 216, genau wie `QWidget::grab()` (gemessen). **`isEffectAvailable()` taugt nur nach der Anmeldung** (F9). **`enableBlurBehind()` ist `void`** (F10). Ein Bild aus der angemeldeten Sitzung ist damit **zwingend** — und zwar ein Bild des **Bildschirms**, nicht des Fensters |
| **6** — Flächenfarbe folgt dem Farbschema | Der Messweg von `native-ak2-kontrast.txt` (20 Schemata) läuft unverändert weiter; im Test relativ gegen `palette().color(QPalette::Window)`, **Alpha ausgenommen** (F11) | 3 von 20 Schemata weichen um einen Zählschritt ab — die Zusicherung muss die Toleranz nennen |
| **7** — eckiges Theme ergibt eckige Ecken | Prüf-Theme liegt vor und ist auf dem nativen Weg gemessen: `cornerRun() == 0`, Alpha 255 ab Spalte 0 (F16) | **Es liegt am falschen Ort.** `capturetest` lädt seine Themes aus `tests/themes/` (`themes::addBundledThemesToDataPath()`). Entweder wandert das Theme dorthin — dann ist es Testgut — oder das AK bleibt an ein Skript gebunden. Das ist eine Dateimengen-Entscheidung des PO |
| **8** — Mutationsprobe je tragender Zusicherung | Heilung entfernen, Rotwerden zeigen — wie in Sprint 6 | keine |

**Was ein Agent an dieser Story grundsätzlich nicht prüfen kann**, in einem
Satz je Punkt:

1. **Das Aussehen des Weichzeichners im Betrieb.** Belegbar ist, *dass* das
   Muster dahinter verwischt (Sonde 4). Ob das Ergebnis dem Kunden gefällt,
   entscheidet sein Blick.
2. **Die Skalierung des Kunden herstellen.** Sie ist vorzufinden, nicht
   einzustellen (F4).
3. **Das *Warum* der Zeitpunktbedingung (F7).** Gemessen ist das *Ob* über
   sieben Läufe; die Protokollebene habe ich nicht untersucht.

---

## Feld 5 — Größenklasse: **`size:m`**

Bedeutung laut `PROZESS.md`: *„trägt einen Strang aus"*.

**Wofür `m` und nicht `s`:** Die Story ist keine Nebenherarbeit. Sie schreibt
`paintEvent()` neu, fasst Konstruktor, `resizeHull()`, `present()` und die
Kopfdatei an, braucht **einen neuen Ereignisweg** (`DevicePixelRatioChange`,
F3), repariert **drei fallende Testzusicherungen** (F11), zieht **vier
SPEC-Stellen** nach (3.1, 3.2, 15, 16) und bringt mit dem Weichzeichner einen
**neuen Prüfweg** ins Projekt, den es vorher nicht gab. „Kein neuer Prüfweg" —
das Kennzeichen von `s` — trifft ausdrücklich nicht zu.

**Wofür nicht `l`:** Der Codeumfang **sinkt**. `tinted()`, `mixed()`, der
zweite `FrameSvg`, `FrameContrast` und `OutlineWidth` fallen weg; hinzu kommen
zwei Aufrufe und ein Ereigniszweig. #55 — die Vergleichsgröße für `l` — hat
dieselbe Datei *aufgebaut*, vier Bibliotheken verkabelt, einen Bildläufer neu
erfunden und acht Akzeptanzkriterien getragen. Der Abstand ist deutlich.

**Die Bedingung, unter der es `l` wird** (bitte beim Schnitt entscheiden):
Verlangt der PO den Weichzeichner-Beleg als **versionierten, wiederholbaren
Läufer unter `tests/`** — also einen sechsten Bildläufer neben `editshots`,
`libraryshots`, `searchshots`, `readmeshots`, `captureshots` —, dann kommen ein
CMake-Block, ein Vollbild-Aufbau und die Frage nach der Bildschirmaufnahme im
CI-Lauf hinzu. Das trägt einen Sprint. Bleibt der Beleg ein Skript unter
`docs/scrum/reviews/`, wie ihn diese Vorprüfung gebaut hat, bleibt es `m`.

**`xl` messe ich nicht.** Falls jemand dafür argumentiert, läge die Teilung an
der einzigen sauberen Naht: **(a) Zeichnung** — Hülle in einem Stück,
Verhältnis, Tests, SPEC 3.1/3.2 — und **(b) Weichzeichner** — Anmeldung,
Zeitpunktbedingung, Belegweg, SPEC 15/16. Die beiden hängen nur über die
Maskenregion zusammen, und die kommt aus derselben `FrameSvg`. Ich empfehle die
Teilung **nicht**: (b) allein ist zu klein, und (a) allein liefert eine Hülle,
die zu 84,7 % deckt und hinter der nichts verwischt — also genau den Zustand,
den der Kunde beanstanden würde.

---

## Zu den drei offenen Punkten des Issues

**Punkt 1 — Textkontrast: Kundenentscheidung, keine Messfrage mehr.**
Die Zahlen liegen vor (`native-ak2-kontrast.txt`, 20 Schemata): deckend
gerechnet schlechtestens **4,74:1**, durchscheinend auf ungünstigem Grund
**3,57:1**, fünf Schemata unter 4,5:1, beim Kunden **4,88:1**. Was ich
beitrage: Die Verschlechterung ist **eine Folge dieser Story** — heute füllt
`tinted()` deckend, die Fläche hat Alpha 255. #83 führt die Durchsichtigkeit
ein. Und: **Plasma hat darauf eine eigene Antwort**, nämlich
`enableBackgroundContrast`, das `libPlasmaQuick` neben dem Weichzeichner ruft
(F14). Es mitzurufen wäre **keine Anpassung**, sondern Teil des nativen Wegs.
Drei Wege stehen zur Wahl: (a) hinnehmen, (b) `enableBackgroundContrast`
mitanmelden wie Plasma, (c) deckend zeichnen — das ist die abgewählte Anpassung.
*Meine Grenze:* Ob (b) auf diesem Stand überhaupt wirkt, habe ich **nicht
gemessen**. `org_kde_kwin_contrast_manager` fehlt genau wie der Blur-Manager
(F15); ob `ext_background_effect_manager_v1` den Kontrast mitträgt, weiß ich
nicht.

**Punkt 2 — der helle Streifen: vorab so weit geprüft, wie es ohne Umsetzung
geht.** Der Mechanismus, den die Erklärung nennt, ist auf dem nativen Weg weg:
der Alphalauf der obersten Zeile ist bei DPR 1 und 1,6 **monoton, ohne
Rückschritt und ohne Wiederholung** (Feld 4, AK 3). Ob der Streifen dadurch
verschwindet, entscheidet ein Bild aus der Sitzung nach der Umsetzung; das
Prüfmittel steht bereit (`eckhelligkeit.py`). **Keine Kundenentscheidung
nötig.**

**Punkt 3 — `setColorSet()` ohne Wirkung: nicht neu ermittelt, und das mit
Absicht.** Die Ursache zu suchen kostet Zeit und ändert an der Story nichts:
Der Farbsatz trifft `Window` in 17 von 20 Schemata, und #83 schreibt nirgends
vor, ihn als Stellschraube zu benutzen. Mein Beitrag ist die Form: Die Zeile
gehört als **Verbot** in den Spawn-Auftrag („der Farbsatz ist keine
Stellschraube"), nicht als Beobachtung ins Issue. **Keine
Kundenentscheidung nötig.**

---

## Offene Fragen an PO oder Kunde

1. **AK 5 braucht eine Bedingung** (F7): Der Weichzeichner wirkt nur bei
   Anmeldung **unmittelbar nach `show()`**. Soll das ins AK und in SPEC 3.2?
   *Dev-Empfehlung: ja* — sonst baut ein Strang die Anmeldung an die
   naheliegende Stelle und misst einen Fehlschlag, den kein Rückgabewert
   meldet.
2. **AK 5, zweiter Teil** (F14): `enableBackgroundContrast` mitanmelden, wie
   Plasmas eigene Überlagerungen es tun? Hängt an Punkt 1 oben.
3. **AK 4 ist so, wie es dasteht, gemessen falsch** (F12). Vorschlag:
   „Offscreen und Wayland liefern **bei gleichem Bildpunktverhältnis** dasselbe
   Bild."
4. **AK 3 ist so, wie es dasteht, von keiner Hülle erfüllbar.** Vorschlag:
   „…der Alphaverlauf **bis zum Randwert** ist monoton und wiederholt keinen
   Zwischenwert."
5. **AK 7 — wo liegt das eckige Prüf-Theme künftig?** In `tests/themes/` (dann
   trägt `capturetest` das AK) oder weiter unter den Abnahme-Belegen (dann
   trägt es ein Skript). Das entscheidet die Dateimenge mit.
6. **Der Textkontrast** (Punkt 1) — Kundenentscheidung.
7. **Wireframe 4b braucht eine neue Aussage**, nicht eine umformulierte (so
   sagt es das Issue selbst). Das ist ein **UX-Auftrag** und liegt außerhalb
   der Dateimenge dieser Story. Vor der Story oder mit ihr?

---

## Was ich **nicht** klären konnte

- **Warum** die späte Anmeldung nicht wirkt (F7). Gemessen ist das *Ob* über
  sieben Läufe; die Wayland-Protokollebene habe ich nicht untersucht.
- Ob die Weichzeichner-Anmeldung auch den **50-ms-Umweg** von `showCapture()`
  übersteht (F8). Gemessen mit unmittelbarem `hide()`/`show()`.
- Ob **`enableBackgroundContrast`** auf diesem Compositor-Stand wirkt.
- Die **Ursache** der Wirkungslosigkeit von `setColorSet()` — nicht gesucht,
  Begründung oben.
- Ob **KRunner** auf diesem Stand tatsächlich weichzeichnet. Ich habe kein
  Gegenstück gemessen; die Aussage „so machen es Plasmas Überlagerungen" ruht
  bei mir allein auf den Symbolen in `libPlasmaQuick` (F14), nicht auf einem
  Bild.
- Der **Aufwand** dieser Story in Zeit. Er wird in diesem Projekt nicht
  erhoben; die Größenklasse ist an der Dateimenge und den Prüfwegen gemessen,
  nicht an einer Dauer.

---

## Befehle, mit denen ich gemessen habe

```
bash docs/scrum/vorberichte/83-native-huelle/pruefen.sh      # alle Sonden
gh issue view 83 ; gh issue view 55 ; gh issue view 79 ; gh issue view 81
grep -n '^void CaptureWindow::' src/capture/capturewindow.cpp
git grep -n "tinted\|FrameContrast\|m_hullInner\|alphaMask\|OutlineWidth" -- src tests wireframes SPEC.md
nm -D -u /usr/lib/libPlasmaQuick.so.7 | grep KWindowEffects
wayland-info | grep -E "shadow|blur|contrast|background_effect"
busctl --user call org.kde.KWin /Effects org.kde.kwin.Effects isEffectLoaded s blur
pacman -Q kwindowsystem kwin plasma-desktop qt6-base ksvg
```

**Nicht getan:** nichts committet, nichts gepusht, nichts nach `/usr`
installiert, keine Zeile unter `src/`, `tests/`, `SPEC.md` oder `wireframes/`
geändert. Der Bauplatz der Sonden liegt unter
`docs/scrum/vorberichte/83-native-huelle/build/` und ist von `.gitignore`
gedeckt; `build/` der Repositoriumswurzel wurde nicht angefasst.
