# Vorprüfbericht #83 — Bearbeiter B (Scrum Master)

**Story:** #83 „Capture: Hülle als native Plasma-Überlagerung — den Nachbau
entfernen" · **Gemessen:** 04.08.2026, 18:57–19:10 CEST · **Stand:** `2ad4eef`
· **Messgrundlage des Issues:** `docs/scrum/reviews/2026-08-04-abnahme-befunde/`

**Unabhängigkeit:** Diese Messung ist ohne Kenntnis der Arbeit von Bearbeiter A
entstanden; `messung-a.md` wurde in diesem Lauf weder gesucht noch gelesen. Die
Konsolidierung ist ein eigener Auftrag.

**Eigene Messungen dieses Berichts** (Sonden und Ausgaben liegen daneben, B7):

| Datei | Was sie misst |
|---|---|
| `themefarbe.cpp` → `messung-b-themefarbe.txt` | Flächenfarbe des nativen Weges über **alle acht installierten Desktop-Themes**, unter zwei Farbschemata |
| `testfolgen.cpp` → `messung-b-testfolgen.txt` | Welche bestehenden Zusicherungen aus `tests/capturetest.cpp` der native Weg umwirft |

---

## Feld 1 — Dateimenge

Notation nach B13, fünf Zeilen. Am Code vermessen; der Schwerpunkt dieses
Feldes liegt bei Bearbeiter A, die Kollisionsfläche ist meiner.

- **Quellen und Tests:** `src/capture/capturewindow.cpp` (471 Z.),
  `src/capture/capturewindow.h` (99 Z.), `tests/capturetest.cpp` (669 Z.),
  `tests/captureshots.cpp` (225 Z.), `tests/themes/` (Aufnahme eines
  rechteckigen Prüf-Themes, siehe AK 7)
- **Build:** **keine Änderung nötig.** `KF6::WindowSystem` ist bereits
  verlinkt (`src/CMakeLists.txt:60`, für `KWindowShadow`); `KWindowEffects`
  liegt im selben Modul. Der einzige neue Aufruf der Story braucht keine neue
  Abhängigkeit
- **Belege und Prüfmittel:** `docs/scrum/reviews/sprint-NN-s83-*/` — offscreen-
  Bildreihe **und** Sitzungsbild bei `QT_SCALE_FACTOR=1.6`; das
  Sitzungswerkzeug liegt fertig vor
  (`docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/echtelage.cpp`)
- **Fachliche Quellen:** `SPEC.md` 3.1 und 3.2 (substantieller Nachzug, siehe
  AK 1), `wireframes/Denkzettel Wireframes.dc.html` (Zeichnung 4b braucht eine
  neue Aussage — das Issue sagt es selbst)
- **Ausdrücklich nicht:** `src/ui/`, `src/shell/`, `src/store/`; und **nicht**
  `CLAUDE.md`, `docs/scrum/PROZESS.md`, `.claude/agents/denkzettel-dev.md` —
  siehe die B17-Fundstellen unten, sie gehören dem PO (melden, nicht heilen)

### 1.1 Kollisionsfläche zu #55, #79 und #81 — der kleinste Abstand ist null

Alle drei offenen Capture-Vorgänge schreiben in **dieselben zwei Dateien** wie
#83:

| Vorgang | Stelle in `capturewindow.cpp` | Stelle in `capturetest.cpp` |
|---|---|---|
| **#83** | `paintEvent()` (309–330), `tinted()` (83–94), `FrameContrast` (54), `OutlineWidth` (45), `resizeHull()` (338–343), `m_hullInner` | `hullIsCompleteAtFiveAndEightLines`, `paintsOneSurfaceInThePaletteColours`, `cornerRun()` und seine drei Nutzer |
| **#79** | `adjustHeight()` (462–471) | `windowFollowsTheTextHeight` (170–191) |
| **#81** | Textfeld-Aufbau (161–168), Dokumentrand | `footerHasMoreAirThanTheApplicationName`, Höhenzusicherungen |
| **#55** | — (geliefert, abgelehnt) | — |

**Zwei Beobachtungen, die den Schnitt betreffen:**

1. **#83 und #81 kollidieren an der Höhenformel.** #81 will
   `document()->setDocumentMargin(0)`; die Formel in `adjustHeight()` liest den
   Dokumentrand aus (`capturewindow.cpp:467`) — und #83 verschiebt über die
   Theme-Ränder dieselbe Höhe. Beide gleichzeitig, und niemand weiß mehr,
   welche Änderung die Starthöhe bewegt hat. Das ist genau der Wert, den die
   Sprint-1-Abnahme zurückgewiesen hat (#42).
2. **#83 und #79 kollidieren nicht am Code, aber am Bild.** #79 verlangt den
   Nachweis am **gezeigten** Fenster; #83 verlangt Bilder bei zwei
   Bildpunktverhältnissen. Beide erzeugen Bildbelege desselben Fensters in
   verschiedenen Zuständen — und der Prüfsummenlauf (`bildbelege-pruefen.sh`)
   schlägt genau dort an, wo zwei verschiedene Zustände denselben Beleg tragen.

**Empfehlung an den Schnitt:** #83 nicht im selben Sprint mit #81. #79 ist
verträglich, wenn die Belegordner getrennt bleiben.

---

## Feld 2 — Gemessene Fallen

Vier Zeilen für den Spawn-Auftrag, je mit Beleg. Der Schwerpunkt dieses Feldes
liegt bei Bearbeiter A; dies sind die, die aus meiner Prüfung fallen.

1. **Der Weichzeichner muss nach jedem Neuzeigen neu angemeldet werden — wie
   der Schatten.** SPEC 3.2, Punkt 5: Vor jedem Zeigen wird das Fenster neu
   gemappt, die Wayland-Surface verschwindet dabei, und eine im Konstruktor
   gebundene Zusage ist nach dem ersten Verstecken weg. `enableBlurBehind`
   hängt am `QWindow` genauso wie `KWindowShadow`. Der Code sagt es über den
   Schatten wörtlich: *„This is the line no test of this project would notice
   missing — it shows only on the second opening"* (`capturewindow.cpp:428`).
   **Wer den Blur einmal im Konstruktor anmeldet, baut denselben Fehler ein,
   und kein Test dieses Projekts fängt ihn.**
2. **Drei Zusicherungen in `capturetest.cpp` werden durch den nativen Weg rot**
   (gemessen, `messung-b-testfolgen.txt`, Theme `default`, Schema des Kunden):

   | Zusicherung | heute | nativ |
   |---|---|---|
   | `qAlpha(Randmitte oben)` — 5× in `hullIsCompleteAtFiveAndEightLines` | 255 | **235** |
   | `qAlpha(Fenstermitte)` | 255 | **216** |
   | `QCOMPARE(Pixel, Window)` in `paintsOneSurfaceInThePaletteColours` | `#ff1e2233` = `#ff1e2233` | **`#d81e2233` ≠ `#ff1e2233`** |
   | `cornerRun()` (Transparenzlauf der obersten Zeile) | 6 | **2** |

   Das ist Umfang der Story, kein Fehler des nativen Weges: „geschlossen"
   bedeutet unter einer Überlagerung nicht mehr „undurchsichtig".
3. **Ein theme-abhängiger Test trügt hier.** Dieselbe Messung unter
   `CachyOS-Nord-round`: alle vier Zusicherungen halten unverändert, weil
   dieses Theme mit Alpha 255 zeichnet. Wer die neuen Zusicherungen nur gegen
   ein Theme prüft, kann beide Ergebnisse bekommen und beide für richtig
   halten.
4. **`native-farben.txt` gibt es nicht.** AK 6 nennt diese Datei als Messweg;
   `git grep native-farben` über das ganze Repository liefert **keinen
   Treffer**. Die Messung, die gemeint ist, liegt unter
   `docs/scrum/reviews/2026-08-04-abnahme-befunde/messungen/native-ak2-kontrast.txt`.

### 2.1 B17-Fundstellen — drei Aussagen über einen Stand, den diese Story aufhebt

`git grep -n "frameContrast\|FrameContrast\|alphaMask\|tinted"` über die
Anweisungsdateien:

- `CLAUDE.md:40` — *„gemessen: `tinted()` verliert offscreen den Alphakanal,
  unter Wayland nicht"*, im **Präsens**, als Begründung der geltenden Regel B21
- `docs/scrum/PROZESS.md:427` — dieselbe Messung, in der DoD-3-Fassung nach B21
- `.claude/agents/denkzettel-dev.md:81` — als Falle Nr. 4 für den bauenden
  Agenten

Fällt `tinted()`, sind alle drei Sätze **Belege eines vergangenen Standes**.
B21 selbst bleibt richtig — nur seine Begründung wird historisch. Nach B17 wird
ein überholter Beleg **geankert, nicht geglättet**; `CLAUDE.md` ist dabei der
kritische Fall, weil dort kein Prüfstand genannt ist (Sprint 6, §19.4). **Das
ist Sache des PO, nicht des Stranges** — deshalb steht es in Feld 1 unter
„ausdrücklich nicht". Ich melde es hier, damit es nicht am Sprint-Ende als
Doku-Abgleichsmangel wieder auftaucht.

---

## Feld 3 — AK-Urteil: **ready = nein**

Acht Akzeptanzkriterien, einzeln geprüft. Maßstab: prüfbar formuliert ·
Prüfmittel benannt oder Grenze ausgesprochen · B21-Pflicht bedacht · kein
Widerspruch zu SPEC oder geltendem Beschluss.

### AK 1 — Hülle in einem Stück; `tinted()`, Ring und `frameContrast` entfernt

**Prüfbar: ja.** Abwesenheit dreier benannter Bezeichner ist per `git grep`
entscheidbar; „in einem Stück" ist an der Zahl der Zeichenaufrufe ablesbar.

**Aber es fehlt der SPEC-Nachzug als Kriterium.** SPEC 3.1 schreibt heute
ausdrücklich vor, was AK 1 entfernt:

> „Gezeichnet wird deshalb die *Alphamaske* des Themes, gefüllt mit
> Palettenfarben" · „**Kontur** eine Mischung aus `Window` und `WindowText` im
> Verhältnis `frameContrast`. In einer Widgets-Anwendung ist das der konstante
> Faktor **0,20**"

Das ist die größte SPEC-Änderung seit Sprint 1. DoD 4 verlangt den Nachzug
ohnehin — aber #55 hatte ihn als eigenes AK („SPEC 3 und SPEC 15
nachgezogen"), und #83 hat ihn nicht. **Urteil: prüfbar, unvollständig.**

### AK 2 — Bildpunktverhältnis folgt dem Fenster, belegt bei 1 und 1,6

**Prüfbar: ja.** `devicePixelRatio()` der erzeugten Pixmap gegen das Fenster;
der Messweg liegt in `sonden/eckenraster.cpp` vor.

**Offen bleibt die Belegform.** „Belegt bei 1 und 1,6" sagt nicht, ob Test oder
Bild. 1,6 ist der Kundenstand (Sprint 6, §25.3), und ein Bild bei dieser
Skalierung ist nach DoD 3 ohnehin Pflicht. **Urteil: ok mit Auflage** —
Belegform benennen.

### AK 3 — kein Stufenlauf: Alphaverlauf der obersten Zeile monoton, kein Plateau

**Nicht prüfbar wie formuliert — es scheitert am eigenen Referenzbeleg.** Das
Issue führt als Sollzustand die Reihe `0·0·0·0·2·65·125·173·195` an. Die vier
führenden Nullen **sind ein Plateau**. Ein Prüfer, der „kein Plateau" wörtlich
nimmt, weist den Zustand zurück, den die Story herstellen will.

Was gemeint ist, ist offenbar: *ab dem ersten Bildpunkt mit Alpha > 0 streng
steigend, bis die Flächendeckung erreicht ist.* Das ist prüfbar, aber es steht
nicht da.

**Zweitens: B21 greift.** „Stufenlauf an der Ecke" ist eine Aussage über die
**Rundung** — nach DoD 3 in der Fassung nach B21 gehört dazu ein Bild aus der
angemeldeten Sitzung. Das AK nennt keins. Es könnte sich auf AK 4 stützen
(offscreen == Wayland) — aber dann hängt AK 3 an AK 4, und AK 4 ist selbst
nicht abgesichert (unten). **Urteil: nein.**

### AK 4 — „Offscreen und Wayland liefern dasselbe Bild"

**Nicht ready, und es ist der schwerste der Formulierungsbefunde.**

- **Kein automatisches Prüfmittel möglich.** Der Vergleich verlangt zwei Läufe
  in zwei Umgebungen. Der CI-Lauf hat keinen Compositor — `PROZESS.md` sagt das
  ausdrücklich —, und ein QTest auf Ganymed läuft offscreen. Es bleibt ein
  Handlauf (`pruefen.sh` ist die Bauart). Das ist eine **Grenze der
  Prüfbarkeit**, und die DoR verlangt, dass sie ausgesprochen wird. Sie ist es
  nicht.
- **„Dasselbe Bild" ist zu weit gefasst.** Die Messung des Issues belegt
  Gleichheit für **einen Kantenlauf**, nicht für ein ganzes Bild. Eine Zusage
  auf Byte-Gleichheit über zwei Plattformen ist eine harte Zusage ohne
  Toleranz — und ohne benannten Vergleichsumfang (welches Fenster, welcher
  Zustand, welche Skalierung) nicht entscheidbar.
- **„Das ist neu und ausdrücklich zuzusichern"** sagt nicht, in welcher Form
  zugesichert wird. Eine Zusicherung ohne Träger ist eine Absichtserklärung.

### AK 5 — Weichzeichner über `enableBlurBehind` mit der Maskenregion angemeldet

**Nicht ready.** Gemessen am Header
(`/usr/include/KF6/KWindowSystem/kwindoweffects.h:78`):

```
KWINDOWSYSTEM_EXPORT void enableBlurBehind(QWindow *window, bool enable = true, const QRegion &region = QRegion());
```

**Rückgabetyp `void`, keine Rückfrage-Funktion.** Prüfbar ist damit der
*Aufruf* (Codeprüfung), nicht die *Wirkung*. Die Wirkung ist Durchsichtigkeit —
eine der sechs Größen, für die B21 ein Bild aus der angemeldeten Sitzung
verlangt. Das AK nennt keins.

Zweitens fehlt die Bedingung aus Falle 1: **Anmeldung nach jedem Neuzeigen**,
nicht einmal im Konstruktor. Ohne diese Zeile ist das AK am ersten Öffnen
erfüllt und am zweiten nicht — und kein Test dieses Projekts sieht den
Unterschied.

*Beiläufig verfügbar, falls ein Testanker gesucht wird:*
`KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind)` existiert
(ebd., Zeile 55). Das prüft die Verfügbarkeit des Effekts, nicht unsere
Anmeldung — es taugt als Vorbedingung, nicht als Nachweis.

### AK 6 — „Die Flächenfarbe folgt weiterhin dem Farbschema"

**Nicht ready. Drei Gründe, der dritte ist eine Sachfrage und gehört dem
Kunden.**

**(a) Das benannte Prüfmittel existiert nicht.** `native-farben.txt` liefert
über das ganze Repository keinen Treffer. Gemeint ist offenbar
`messungen/native-ak2-kontrast.txt`. Nach B7 ist ein Beleg, den man nicht
findet, kein Beleg.

**(b) Es fehlt die Toleranz.** Die eigene Messgrundlage zeigt bei **3 von 20**
Schemata eine Abweichung um einen Zählschritt (BreezeClassic, BreezeLight,
KritaBright: `239,240,241` → gezeichnet `240,240,241`). „Folgt" ohne Toleranz
ist gegen die eigene Messung nicht erfüllbar.

**(c) Die Zusicherung gilt nur unter dem Desktop-Theme, unter dem sie gemessen
wurde.** Das ist meine eigene Messung, und sie ist der gewichtigste Befund
dieses Berichts.

Die Messgrundlage des Issues dreht **eine** Achse: 20 Farbschemata unter
**einem** Theme (`default`). Ich habe die andere Achse gemessen — ein Schema,
alle acht installierten Themes (`messung-b-themefarbe.txt`, offscreen,
`QT_QPA_PLATFORMTHEME=kde`, Schema des Kunden `CachyOSNordLightly`,
`Window 30,34,51`, `WindowText 102,194,242`):

| Desktop-Theme | Fläche gezeichnet | Alpha | = `Window`? | Text auf Fläche |
|---|---|---|---|---|
| `default` | `30,34,51` | 216 | **gleich** | 7,93:1 |
| `CachyOS-Nord-round` | `30,34,51` | 255 | **gleich** | 7,93:1 |
| `breeze-dark` | `32,35,38` | 216 | Abweichung 13 | 7,92:1 |
| **`breeze-light`** | **`240,240,241`** | 216 | **Abweichung 210** | **1,74:1** |
| `Iridescent-round` | `0,0,0` | **51** | Abweichung 51 | (Fläche fast durchsichtig) |
| `cachyos-emerald` | `0,0,0` | **7** | Abweichung 51 | (Fläche praktisch nicht vorhanden) |
| `cachyos-emerald-color` | `0,0,0` | **7** | Abweichung 51 | dito |
| `cachyos-emerald-light` | `0,0,0` | **7** | Abweichung 51 | dito |

Die Gegenprobe mit hellem Schema (`BreezeLight`) kehrt es um: `breeze-dark`
und `CachyOS-Nord-round` liefern dort **1,03:1** und **1,04:1**.

**Das ist derselbe Befund, aus dem SPEC 3.1 am 01.08.2026 die Palettenfüllung
abgeleitet hat** — dort steht er wörtlich:

> „Von den acht auf der Kundenmaschine installierten Themes richtet nur
> `default` seine Füllfarbe am Farbschema aus; ein Fenster in Theme-Farben
> stünde bei sieben von acht dunkel auf dunkel (Messung 01.08.2026)."

#83 entfernt genau diese Füllung und sichert trotzdem zu, die Farbe folge dem
Schema. **Die Zusicherung hält, weil beim Kunden Theme und Schema zufällig
zusammenpassen** — `plasmarc` trägt keinen `[Theme] name`-Eintrag, das Fenster
läuft also auf dem KSvg-Rückfall `default`, und das ist das eine Theme von
achten, das der Palette folgt.

Die Ursache ist mitgemessen und liegt nicht im Code: `breeze-light`,
`breeze-dark`, `cachyos-emerald-color` und `cachyos-emerald-light` bringen eine
eigene `colors`-Datei mit und setzen ihre Farbe damit gegen die Anwendung
durch; `CachyOS-Nord-round` hat eine fest eingefärbte Grafik; die
Emerald-Themes zeichnen fast nichts und verlassen sich auf den Weichzeichner.
`default` bringt nichts davon mit — deshalb folgt es der Palette.

**Warum das keine Nörgelei an einer Formulierung ist:** „Ohne Anpassungen"
heißt, dass die Lesbarkeit der Notiz ab dieser Story von der Kombination
Desktop-Theme × Farbschema abhängt, die der Nutzer einstellt. Bei sechs von
acht installierten Themes ist der Notiztext unter dem Schema des Kunden
entweder unlesbar (1,74:1) oder steht auf dem Desktop-Hintergrund
(Alpha 7–51). Das ist eine **Kundenfrage**, keine Umsetzungsfrage — siehe
Feld 6, Vorlage 1.

*Grenze meiner Messung, damit sie nicht stärker aussieht, als sie ist:* Sie ist
offscreen gelaufen und misst nur die Fläche, nicht das fertige Fenster mit
Weichzeichner. Das Issue belegt für den nativen Weg
Plattformunabhängigkeit — unter dieser Voraussetzung trägt die Zahl; sie ist
sie nicht selbst.

### AK 7 — Ein Theme mit rechteckigen Eckstücken erzeugt rechteckige Ecken

**Prüfbar: ja, und es ist das sauberste Kriterium der Liste** — es sichert die
Herkunft der Form zu, ohne eine Zahl festzuschreiben (dieselbe Bauart wie in
`checkHullDiffersBetween`).

**Zwei Auflagen:**

- „Prüf-Theme liegt vor" stimmt — aber es liegt in einem **Belegordner**
  (`docs/scrum/reviews/2026-08-04-abnahme-befunde/pruef-theme/plasma/desktoptheme/denkzettel-pruef-eckig/`),
  nicht unter `tests/themes/`, wo `themes::addBundledThemesToDataPath()` die
  Testthemes sucht. Es muss überführt werden; das ist Umfang und steht nicht im
  AK.
- „Rechteckige Ecken" ist eine Aussage über die Rundung → B21, Sitzungsbild.
  Der Weg dafür liegt vor (`b1-echtelage-eckiges-theme.txt` ist genau dieser
  Fall), er ist nur nicht im AK benannt.

### AK 8 — Mutationsprobe je tragender Zusicherung

**Prüfbar erst mit Liste.** Genau dieser Satz ist in Sprint 6 gerissen: „jede
tragende Zusicherung gegen eine Mutation gehalten" deckte 8 von 11, und
gefunden hat es der Reviewer durch **Nachzählen** (K1,
`docs/scrum/reviews/sprint-06-karpathy.md`). Wer denselben Satz ohne Zählbarkeit
wieder aufschreibt, bekommt denselben Befund.

**Vorschlag:** „…je tragender Zusicherung; der Übergabebericht zählt sie auf
und nennt je Zusicherung das Ergebnis der Probe." Dann ist es zählbar statt
beteuert.

### Gesamturteil

**ready = nein.**

| AK | Urteil |
|---|---|
| 1 | prüfbar, aber SPEC-Nachzug fehlt als Kriterium |
| 2 | ok mit Auflage (Belegform benennen) |
| 3 | **nein** — widerspricht dem eigenen Referenzbeleg; B21-Bild fehlt |
| 4 | **nein** — kein Prüfmittel, keine Toleranz, Grenze nicht ausgesprochen |
| 5 | **nein** — Wirkung nicht prüfbar, B21-Bild fehlt, Remap-Bedingung fehlt |
| 6 | **nein** — Prüfmittel existiert nicht, keine Toleranz, offene Sachfrage |
| 7 | ok mit Auflage (Prüf-Theme überführen, Belegform benennen) |
| 8 | ok mit Auflage (Zusicherungen aufzählen) |

Hinzu kommt: Das Issue führt selbst drei Punkte als „vor dem Ziehen zu
entscheiden". Solange sie offen sind, ist die Story nach der Definition of
Ready ohnehin nicht ziehbar — **das Issue widerspricht sich hier nicht, es
sagt es nur an einer Stelle, an der die DoR nicht hinsieht.**

**Behoben wird das vom PO, nicht von mir** (melden, nicht heilen). Vier der
sechs Punkte sind Formulierungsarbeit von je zwei Sätzen; einer (AK 6) ist eine
Kundenvorlage.

---

## Die drei offenen Punkte des Issues — Einordnung

### Punkt 1 — der Textkontrast: **Kundenfrage**

Und sie ist größer, als das Issue sie stellt. Das Issue fragt nach der
**Durchsichtigkeit** (fünf von zwanzig Schemata unter 4,5:1, beim Kunden
4,88:1). Meine Messung zu AK 6 fügt die **zweite Achse** hinzu: das
Desktop-Theme. Beide Achsen haben dieselbe Wurzel — mit „ohne Anpassungen" gibt
die Anwendung die Kontrolle über den Grund ab, auf dem ihr Text steht.

Beides gehört dem Kunden in **einer** Vorlage, nicht in zweien: Es ist eine
Frage, zweimal gemessen.

**Wortlautvorschlag (Feld 6, Vorlage 1)** — siehe unten.

### Punkt 2 — der helle Streifen an der Ecke: **Messung, und zwar eine, die erst nachher möglich ist**

Das Issue sagt es selbst: *„mit dieser Story kostenlos prüfbar"*. Eine Messung,
die erst nach der Umsetzung laufen kann, ist **kein Punkt, der vor dem Ziehen
zu entscheiden ist** — sie blockiert nichts, sie gehört in die Abnahme.

**Empfehlung an den PO:** aus der Liste der offenen Punkte streichen und als
neuntes AK aufnehmen, etwa: *„Der helle Streifen an der Ecke (Kundenbefund
K-A1) ist am Sitzungsbild bei 1,6 kleiner oder verschwunden; bleibt er, liegt
eine Erklärung im Übergabebericht."* So bleibt er verbindlich, ohne die Story
zu sperren.

### Punkt 3 — `setColorSet()` ohne Wirkung: **ausstehende Messung, und sie wiegt schwerer als das Issue sie führt**

Das Issue schreibt: *„Praktisch trifft es sich gut."* Das ist zu ruhig. Nach
`sprint-06.md` §24.4 ist `colorSet` **der native Weg, auf dem Plasma seine
Überlagerungen ans Farbschema anpasst** — und AK 6 sichert genau diese
Anpassung zu. Eine Zusicherung, deren Mechanismus nachweislich nicht wirkt und
deren Ursache unermittelt ist, ruht auf einem Zufall.

**Meine Messung liefert einen Teil der Ursache mit** (`messung-b-themefarbe.txt`
plus `ls` über `/usr/share/plasma/desktoptheme/`): Farbsätze sind eine
Eigenschaft der **Theme-Datei**, nicht des Aufrufs. `default` bringt keine
`colors`-Datei mit — es gibt dort keine Farbsätze, zwischen denen `setColorSet`
wählen könnte, und KSvg fällt auf die Anwendungspalette zurück. Themes **mit**
`colors` (`breeze-light`, `breeze-dark`, `cachyos-emerald-color`,
`cachyos-emerald-light`) setzen ihre eigene Farbe durch.

Das ist ein Hinweis, keine abgeschlossene Ursachenklärung — ich habe den
KSvg-Quelltext nicht gelesen. **PO-Entscheidung:** ob vor dem Ziehen
ausgemessen wird. Ich empfehle ja, weil AK 6 daran hängt.

---

## Feld 4 — Prüfmittel, und was der Agent nicht prüfen kann

**Was trägt:**

| Zusicherung | Prüfmittel |
|---|---|
| Ring, `tinted()`, `frameContrast` entfernt | `git grep`, Codeprüfung |
| Bildpunktverhältnis folgt (1 und 1,6) | QTest mit `QT_SCALE_FACTOR`; Muster `sonden/eckenraster.cpp` |
| Alphaverlauf der Ecke | QTest auf der Pixelreihe; Muster `cornerRun()` |
| Flächenfarbe je Schema | Sondenlauf über `/usr/share/color-schemes/`, Muster `pruefen.sh` Z. 130–137 |
| Rechteckiges Theme → rechteckige Ecken | QTest mit überführtem Prüf-Theme |
| Hülle bei 5 und 8 Zeilen | QTest, bestehende Bauart (neu zu fassende Schwellen) |
| Erscheinung bei 1,6 in der Sitzung | `sonden/echtelage.cpp` — nimmt allein das Fenster auf |

**Was ein Agent nicht prüfen kann — ausdrücklich, wie die DoR es verlangt:**

1. **Die Wirkung des Weichzeichners.** `enableBlurBehind` ist `void`; es gibt
   keine Abfrage, ob KWin die Region angenommen hat. Nachweisbar ist der Aufruf
   und — im Sitzungsbild — dass der Grund hinter dem Fenster weich ist. Ein
   automatischer Test kann das nicht.
2. **„Offscreen == Wayland" als laufende Zusicherung.** Der Vergleich braucht
   zwei Umgebungen; die CI hat keinen Compositor. Als Test nicht haltbar, nur
   als datierter Doppellauf mit abgelegten Ausgaben.
3. **Ob es „nativ aussieht".** Das ist der Befund, an dem #55 gescheitert ist,
   und `sprint-06.md` §25.1 sagt es klar: *„Das hat kein Prüfmittel dieses
   Projekts gefunden, und keines hätte es gefunden."* Es stellt nur, wer das
   Fenster neben andere Fenster stellt und hinsieht. **Diese Story kann mit
   allen acht AK grün sein und trotzdem abgelehnt werden.** Das ist keine
   Schwäche der Kriterien — es ist der Grund, warum die Kundenabnahme
   existiert, und es gehört dem PO in der Abnahmeplanung gesagt.

---

## Feld 5 — Größenklasse: **`size:l`**

Gemessen am Code, nicht an der Beschreibung des Issues.

**Was für eine kleinere Klasse spricht** — und es ist nicht wenig:

- Der Zeichenweg **schrumpft**: `tinted()` (12 Z.), `mixed()` (6 Z.),
  `FrameContrast`, `OutlineWidth`, `m_hullInner` und der zweite `resizeFrame()`
  fallen weg; `paintEvent()` verliert zwei Zeilen und bekommt eine
- **Keine neue Abhängigkeit.** `KF6::WindowSystem` ist verlinkt
  (`src/CMakeLists.txt:60`); `KWindowEffects` liegt darin. Der Build bleibt
  unangetastet
- Die Infrastruktur steht: KSvg verkabelt, `plasmarc`-Wache läuft,
  Theme-Wechsel geprüft, Schatten gebunden. **#83 baut nichts davon neu**

**Was `size:l` trägt:**

1. **Drei Zusicherungsfamilien fallen und müssen neu gefasst werden**
   (gemessen, `messung-b-testfolgen.txt`): die fünf `Alpha == 255` in
   `hullIsCompleteAtFiveAndEightLines`, die beiden `QCOMPARE` gegen `Window` in
   `paintsOneSurfaceInThePaletteColours`, und `cornerRun()` mit seinen **drei**
   Nutzern. Das ist nicht Anpassen von Zahlen: „geschlossen" und „die Fläche
   ist die Palettenfarbe" sind unter einer Überlagerung **andere Aussagen** und
   müssen erst gefunden werden.
2. **Zwei Belegformen statt einer.** Offscreen-Bildreihe **und** Sitzungsbild
   bei `QT_SCALE_FACTOR=1.6` — AK 3, 5 und 7 sprechen alle über B21-Größen. Das
   Sitzungsbild ist in diesem Projekt noch nie Teil einer Story gewesen; es ist
   seit heute Pflicht (B21) und hier zum ersten Mal fällig.
3. **Ein zusätzliches Prüfmittel muss ins Testverzeichnis überführt werden**
   (rechteckiges Theme, AK 7) — mit allem, was `tests/desktopthemes.h` daran
   hängt.
4. **Substantieller SPEC-Nachzug.** SPEC 3.1 wird in ihrem tragenden Absatz
   umgedreht („Form kommt vom Theme, Farbe aus der Palette" gilt nicht mehr),
   3.2 bekommt mindestens eine Bedingung dazu (Blur-Neubindung nach Remap).
5. **AK 8 verlangt Mutationsproben je tragender Zusicherung** — bei etwa einem
   Dutzend Zusicherungen ist das ein eigener Arbeitsgang, und Sprint 6 zeigt,
   dass er unterschätzt wird.
6. **Die Kollisionsfläche ist maximal.** Dieselbe 471-Zeilen-Datei tragen drei
   weitere offene Vorgänge; der kleinste Abstand ist null.

**Einordnung gegen #55** (wäre `size:l` gewesen): #83 ist im Code kleiner,
im **Prüfweg** größer — #55 durfte offscreen belegen, #83 nicht mehr.

**Nicht `xl`.** Die Story hat einen Gegenstand und eine Naht, an der sie ganz
ist: die Hülle des Erfassungsfensters. Eine Teilung („erst zeichnen, dann
Weichzeichner") würde einen Zwischenstand erzeugen, in dem das Fenster zu
84,7 % deckt und niemand den Grund dahinter weichzeichnet — also einen Stand,
den man dem Kunden nicht zeigen kann. Das ist der Fall, in dem Teilen schadet.

**Konsolidierungshinweis:** Kommt Bearbeiter A auf `size:m`, gilt nach der
Ein-Stufen-Regel die höhere Klasse, also `l`. Kommt er auf `xl`, entscheide ich
begründet — dann wäre die Trennlinie zu benennen, und ich sehe keine, die einen
zeigbaren Zwischenstand hinterlässt.

**Folge für den Schnitt:** Neben `size:l` steht nur `size:s`. #81 fällt ohnehin
aus (Kollision, 1.1); #79 kommt nur infrage, wenn seine eigene Vorprüfung `s`
ergibt.

**Kein Label gesetzt** — es entsteht mit dem konsolidierten Bericht.

---

## Feld 6 — Offene Fragen

### An den Kunden — Vorlage 1: Wie weit reicht „ohne Anpassungen"?

Wortlautvorschlag, wie er vorzulegen wäre. Er trägt beide Achsen, weil es
dieselbe Frage ist:

> **Worum es geht.** Deine Entscheidung „eine native Plasma-Überlagerung ohne
> Anpassungen" heißt: Das Fenster zeichnet, was das Desktop-Theme hergibt — und
> nichts darüber hinaus. Zwei Messungen zeigen, was das für die Lesbarkeit der
> Notiz bedeutet.
>
> **Erstens: die Überlagerung deckt nicht ganz.** Sie deckt zu 84,7 %; was
> darunter liegt, scheint durch. Der Weichzeichner mildert das, aber ein weißes
> Fenster dahinter genügt für den ungünstigen Fall. Über 20 Farbschemata
> gerechnet: fünf fallen unter den Lesbarkeitswert 4,5:1 — der schlechteste bei
> 3,57:1. **Dein eigenes Schema liegt mit 4,88:1 knapp darüber.**
>
> **Zweitens: die Fläche gehört jetzt dem Desktop-Theme, nicht mehr dem
> Farbschema.** Von den acht auf deiner Maschine installierten Themes passt die
> Fläche nur bei zweien zu deinem Farbschema — bei deinem eingestellten
> (`default`) und bei `CachyOS-Nord-round`. Stellst du `breeze-light` ein,
> zeichnet das Theme eine helle Fläche unter deine helle Schrift: **1,74:1**,
> praktisch unlesbar. Die drei Emerald-Themes zeichnen fast gar keine Fläche —
> dort steht die Notiz auf deinem Desktop-Hintergrund.
>
> **Bisher war das anders.** Seit Sprint 6 füllt Denkzettel die Form des Themes
> mit **deiner Schemafarbe** — genau deshalb, weil wir am 01.08. gemessen
> hatten, dass sieben von acht Themes das nicht selbst tun. Diese Füllung ist
> der „Nachbau", den du abgewählt hast. Wir nehmen sie heraus, wenn du es so
> willst — aber dann geht die Lesbarkeit an die Kombination über, die du
> einstellst.
>
> **Drei Wege:**
>
> 1. **Ganz ohne Anpassung.** Das Theme bestimmt Fläche und Deckung. Bei deiner
>    heutigen Einstellung sieht es richtig aus. Wechselst du das Desktop-Theme,
>    kann die Notiz unlesbar werden — dann ist das Theme das Problem, nicht die
>    Anwendung. **Am nächsten an deiner Entscheidung.**
> 2. **Ohne Anpassung, aber mit Notbremse.** Gezeichnet wird die Theme-Grafik.
>    Nur wenn der Kontrast zwischen Fläche und Schrift unter 4,5:1 fällt, legt
>    Denkzettel die Schemafarbe darunter. Sichtbar würde das bei sechs deiner
>    acht Themes, bei deinem heutigen nie. **Das ist eine Anpassung** — eine,
>    die nur im Schadensfall greift.
> 3. **Auch die Schrift aus dem Theme.** Der eigentlich konsequente Weg: Plasmas
>    Überlagerungen nehmen Fläche *und* Schrift aus derselben Quelle. Denkzettel
>    nimmt heute die Fläche vom Theme und die Schrift vom Farbschema — daher
>    kommt der Bruch. Ungemessen; wir wissen nicht, ob alle acht Themes
>    brauchbare Schriftfarben mitbringen. Wäre eine eigene Messung vorab.
>
> **Ohne deine Antwort schätzen wir die Story nicht zu Ende** — die Antwort
> entscheidet, ob ein Kriterium hinzukommt.

*Anmerkung an den PO zur Vorlage:* Weg 2 und Weg 3 widersprechen dem Wortlaut
„ohne Anpassungen" unterschiedlich stark. Ich lege sie vor, statt sie
vorzusortieren — die Ablehnung von #55 ist daran entstanden, dass eine
Kategorienfrage nie gestellt wurde (§25.1). Zweimal ist einmal zu oft.

### An den PO

1. **AK 3, 4, 5, 6 nachschärfen** — Einzelheiten je AK oben. Der Aufwand ist
   klein, außer bei AK 6 (hängt an der Kundenantwort).
2. **`native-farben.txt` richtigstellen** auf
   `messungen/native-ak2-kontrast.txt`, oder die Datei unter dem genannten
   Namen ablegen.
3. **Die drei B17-Fundstellen** (`CLAUDE.md:40`, `PROZESS.md:427`,
   `denkzettel-dev.md:81`) ankern, sobald `tinted()` fällt — nicht durch den
   Strang. Meine Empfehlung: nicht vorher, sonst ankern wir einen Stand, den es
   noch gibt.
4. **Punkt 2 der offenen Punkte** in ein AK überführen, damit er das Ziehen
   nicht sperrt.
5. **Die Abnahme dieser Story braucht den Vergleich neben anderen Fenstern.**
   Kein AK und kein Review erreicht das (Feld 4, Punkt 3). Es gehört in die
   Abnahme-Checkliste, nicht in die Kriterien.

---

## Die Prozessfrage: Wie stehen #55 und #83 zueinander?

**Sie ist meine, und ich beantworte sie ausdrücklich gegen die heutige Buchung
im Sprint-Protokoll.**

### Trägt „gegenstandslos, nicht unerfüllt" für die ganze Story?

**Nein.** Der Satz trägt für das Kontur-Kriterium, und die Begründung des
Issues ist dort sauber (Kante und Fläche der Theme-Grafik haben dieselbe Farbe
und unterscheiden sich nur in der Deckung — `frameContrast` hat darin keine
Entsprechung). Für die acht Kriterien von #55 insgesamt trägt er nicht. Ich
habe sie einzeln durchgesehen:

| AK von #55 | Verhältnis zu #83 |
|---|---|
| 1 — Rundung, Kontur, Schatten aus dem Theme | **teils gegenstandslos**: Kontur fällt, Rundung und Schatten bleiben zugesichert |
| 2 — durchgehende Fläche, `WindowText`, 4,74:1 | **nicht gegenstandslos, sondern inhaltlich anders**: die Fläche ist jetzt die des Themes, und 4,74:1 gilt nur deckend gerechnet |
| 3 — Innenabstände zuzüglich Theme-Rand | **unberührt und geliefert** |
| 4 — Hülle bei 5 und 8 Zeilen vollständig | **umzudeuten**: „vollständig" heißt nicht mehr Alpha 255 (gemessen: 216) |
| 5 — Theme-Wechsel bei laufendem Dienst | **unberührt und geliefert** |
| 6 — keine Titelleiste, SPEC 3 gilt | **unberührt und geliefert** |
| 7 — Belegformen getrennt | **überholt, aber nicht durch #83** — durch B21. `PROZESS.md` sagt es wörtlich: AK 7 hat „Rundung und Kontur" der offscreen-Seite zugeordnet, „eine Eigenschaft zu weit" |
| 8 — KF6::Svg verkabelt, SPEC nachgezogen | **geliefert** |

Vier Kriterien sind geliefert und von #83 unberührt, eines ist durch einen
Beschluss überholt, eines teilweise gegenstandslos, **zwei werden inhaltlich
andere Kriterien**. Die Formel „gegenstandslos, nicht unerfüllt" deckt davon
eines.

### Meine Empfehlung: **#55 schließen, #83 ersetzt es — nicht gemeinsam abnehmen**

Heute steht im Protokoll: *„Bleibt offen, geht in den Backlog, wird mit #83
zusammen abgenommen"* (§25). Das hat drei Kosten:

1. **Es prüft gegen einen überholten Text.** Wer #55 mit #83 zusammen abnimmt,
   hält die Umsetzung gegen acht Kriterien, von denen zwei etwas anderes
   verlangen als das, was gebaut wird, und eines gegen einen Beschluss steht.
   Genau diese Bauart hat den teuersten Fehler dieses Sprints erzeugt: eine
   Aussage, die von ihrem Gegenstand abgefallen ist (§25.2).
2. **Es hält ein doppeldeutiges Feld in der einzigen Quelle der Wahrheit
   offen.** Ein offenes Issue sieht gleich aus, ob es auf Arbeit wartet oder auf
   eine Abnahme, die anderswo stattfindet. Dasselbe Argument hat am 04.08. das
   `sp:`-Label entschieden.
3. **Es bringt nichts ein.** Die Bilanz „2 von 3 Stories, 3 von 11 Punkten"
   steht im Protokoll und im geschlossenen Milestone. Sie hängt nicht daran,
   dass das Issue offen bleibt — der Milestone liest sich dauerhaft als
   1 offen, 2 geschlossen, und das bliebe er auch mit geschlossenem #55, weil
   der Ausgang im Protokoll steht.

**Was ich vorschlage:**

- **#55 schließen** mit einem Kommentar, der die Abnahme je Kriterium festhält:
  AK 3, 5, 6, 8 abgenommen; AK 1 teilweise; AK 2, 4 durch #83 ersetzt; AK 7
  durch B21 überholt. **Nicht** als „not planned" — die Story hat geliefert,
  was das Projekt heute noch trägt (KSvg-Anbindung, Theme-Wache,
  Schattenbindung, SPEC 3.1/3.2).
- **#83 trägt die neuen Kriterien** und den Verweis auf #55 als Vorgeschichte.
- **Die Kundenabnahme findet einmal statt**, an #83.

**Was dagegen spricht, und ich sage es dazu:** Der Kunde hat #55 abgelehnt. Ein
abgelehntes Issue zu schließen, kann wie Wegräumen aussehen. Deshalb gehört der
Schließkommentar dem Kunden vorgelegt — **das Schließen selbst ist eine
Abnahmeentscheidung des PO, nicht meine.** Ich melde, ich entscheide nicht.

**Die schwächere Alternative, falls der PO beim gemeinsamen Weg bleibt:** Dann
muss die AK-Liste von #55 **jetzt** auf den Stand gebracht werden (streichen,
umschreiben, den B21-Vermerk an AK 7). Sonst prüft die nächste Abnahme gegen
einen Text, von dem alle wissen, dass er nicht mehr gilt. Das ist Arbeit an
einer Story, deren Text ohnehin durch #83 ersetzt wird — deshalb halte ich sie
für die schlechtere Wahl, nicht für unzulässig.

---

## Wo das neue Verfahren beim ersten Anfassen geklemmt hat

Dies ist der erste Lauf des Vorprüfberichts (`PROZESS.md`, Sprint-Mechanik,
heute in Kraft mit `2ad4eef`). Fünf Beobachtungen, die in die nächste Retro
gehören.

**1. Der Ablageort im Auftrag widerspricht der Prozess-Doku.** `PROZESS.md`
sagt: *„`docs/scrum/vorberichte/NN-<kurzname>.md` … Je Story eine Datei."* Der
Auftrag verlangt `83-native-huelle/messung-b.md` — einen **Ordner** mit einer
Datei je Bearbeiter. Der Auftrag ist praktisch im Recht: Zwei unabhängige
Bearbeiter können nicht in dieselbe Datei schreiben, ohne voneinander zu
erfahren, und die Messausgaben brauchen ohnehin einen Ordner (B7). **Die
Prozess-Doku ist an dieser Stelle unfertig geschrieben worden** — sie hat den
Bericht als Ergebnis gedacht und das Verfahren dahin nicht mitgeschrieben. Ich
bin dem Auftrag gefolgt. **Nachzuziehen:** ein Ordner je Story mit
`messung-a.md`, `messung-b.md`, `bericht.md` (konsolidiert) und den
Messausgaben. Das betrifft `PROZESS.md`, Abschnitt Artefakte.

**2. Die DoR hat keinen Ort für „das Issue erklärt sich selbst für unfertig".**
#83 führt drei Punkte „vor dem Ziehen zu entscheiden". Damit ist die Story
nicht ziehbar, bevor ein Bearbeiter ein Kriterium gelesen hat. Die DoR prüft
aber nur die Kriterien. Das Ergebnis stimmt zufällig, weil die Kriterien
ebenfalls nicht tragen — **die Regel hätte es nicht gefunden, wenn sie
getragen hätten.** Vorschlag: „Ein Issue mit selbstdeklarierten offenen Punkten
ist nicht ready, unabhängig von den Kriterien."

**3. „Prüfmittel benannt" prüft nicht, ob das Prüfmittel existiert.** AK 6
nennt `native-farben.txt`; es gibt sie nicht. Nach dem Wortlaut der DoR ist ein
Prüfmittel benannt — die Bedingung ist erfüllt. **Ein Dateiname ist erst dann
ein Prüfmittel, wenn `git ls-files` ihn zeigt** (dieselbe Bauart wie die
Existenzprüfung des Verwalter-Berichts, Abschluss-Punkt 11).

**4. B21 ist bei den Kriterien noch nicht angekommen.** Drei der acht
Kriterien (3, 5, 7) sprechen über B21-Größen, keines nennt ein Sitzungsbild.
Das Issue wurde am selben Tag geschrieben wie der Beschluss. Das ist kein
Vorwurf an den PO — es zeigt, dass die Regel die Stelle noch nicht erreicht
hat, an der sie greifen muss: **die Formulierung von Kriterien**. Vorschlag:
eine Zeile in die DoR — „Behauptet ein Kriterium etwas über Hülle, Rundung,
Kontur, Schatten, Dekoration oder Durchsichtigkeit, nennt es das Sitzungsbild
als Belegform."

**5. Die Reihenfolge der Felder arbeitet gegen die Messung.** Der Bericht führt
das Ready-Urteil als Feld 3 und die Prüfmittel als Feld 4 — aber ob ein
Kriterium prüfbar ist, entscheidet sich am Prüfmittel. Ich habe Feld 4 zuerst
gemessen und Feld 3 danach geschrieben. Das ist keine Fehlfunktion, nur eine
Nummerierung, die die Arbeit nicht abbildet; erwähnenswert für den nächsten,
der die Felder der Reihe nach abarbeiten will.

**Und eine Beobachtung, die für das Verfahren spricht:** Die unabhängige
Messung hat den Befund gebracht, der zählt. AK 6 sieht mit der Messgrundlage
des Issues erfüllt aus — 20 Schemata, 17 exakt. Erst das Drehen der Achse zeigt,
dass die Zusicherung an einem Theme hängt, das der Kunde nicht eingestellt hat,
sondern das als Rückfall greift. **Ein zweiter Bearbeiter, der die Belege des
ersten liest, hätte dieselbe Achse gemessen.** Das ist das Argument für die
Unabhängigkeit, an einem Fall.
