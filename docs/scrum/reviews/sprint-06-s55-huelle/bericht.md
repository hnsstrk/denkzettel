# Strang A, Sprint 6 — Übergabebericht

**Datum:** 2026-08-04, Ganymed · **Zweig:** `story/55-fensterhuelle` ·
**Ausgangsstand:** `main` @ `0a229d2` · **Stories:** #56 (1 SP), #55 (8 SP),
in dieser Reihenfolge gebaut.

**Kurz:** Beide Stories sind umgesetzt, alle Akzeptanzkriterien sind belegt,
`ctest` ist grün (7/7) und `capturetest` in beiden geforderten Umgebungen
(20/20). Zwei der drei Grenzen der Prüfbarkeit, die das Planning benannt hat,
sind **geschlossen** statt nur benannt — der Schatten ist am laufenden
Compositor gemessen, einschließlich des Remap-Falls. Was offen bleibt, steht in
Abschnitt 6; **zwei Fehler in meiner eigenen Arbeit**, die durch Messung
aufgefallen sind und beide grün aussahen, stehen in Abschnitt 5.

---

## 1. Was gebaut wurde

| Commit | Story | Inhalt |
|---|---|---|
| `48d73e5` | **#56** | `adjustHeight()` läuft zusätzlich bei `QEvent::FontChange`, im bestehenden `eventFilter` auf `m_text`. |
| *(dieser Commit)* | **#55** | Hülle aus dem Desktop-Theme: `KSvg::FrameSvg` über `dialogs/background`, eigenes `paintEvent`, Theme-Rand in den Innenabständen, Notiztext auf `WindowText`, `KWindowShadow` nach jedem Zeigen, Wache auf `plasmarc`, Randfall ohne Theme. Dazu der neue Bildläufer `tests/captureshots.cpp` und die Verkabelung von `KF6::Svg`, `KF6::WindowSystem`, `KF6::ConfigCore` und `KF6::CoreAddons`. |

**Berührte Dateien** — alle innerhalb der zugewiesenen Menge:
`src/capture/capturewindow.{h,cpp}` · `tests/capturetest.cpp` ·
**neu** `tests/captureshots.cpp`, `tests/desktopthemes.h`, `tests/themes/` ·
`tests/CMakeLists.txt` (nur der `capturetest`-Block und ein neuer
`captureshots`-Block) · `src/CMakeLists.txt` (nur der
`denkzettelcapture`-Block) · `CMakeLists.txt` (nur die
`find_package(KF6 …)`-Liste) · `SPEC.md` (3, 15, 16) ·
`docs/scrum/reviews/sprint-06-s55-huelle/`.
`src/capture/textareaheight.{h,cpp}` ist **nicht** angefasst worden — die
Vermutung des Plannings hat gehalten: Der Fehler von #56 saß im Auslöser, nicht
in der Formel.

**Die drei Festlegungen aus dem Planning sind eingehalten:** Die #56-Heilung
sitzt im `eventFilter` (nicht in einem `changeEvent`); im Code steht **keine**
Zahl für Rundung oder Rand, und keine Zusicherung vergleicht gegen eine — alle
Aussagen zur Hülle sind relativ geführt; beide `capturetest`-Läufe sind grün.

---

## 2. Akzeptanzkriterien #56 — je mit Beleg

| AK | Beleg |
|---|---|
| **1** — Höhe entspricht nach einer Schriftänderung wieder SPEC 3 (fünf ruhend, bis acht mitwachsend) | Test `heightFollowsAFontChange()`: prüft ruhend `Höhe − Chrom == 5 × Zeilenabstand` **und** nach acht Zeilen `== 8 × Zeilenabstand`. Vor der Heilung rot mit *Actual 85, Expected 215*. |
| **2** — Prüfbar ohne Plasma-Schriftumstellung; der Test setzt die Schrift des Widgets direkt und misst gegen den Zeilenabstand | Derselbe Test setzt `text->setFont(...)` — Weg C der Schätzmessung. Genau deshalb sitzt die Heilung im `eventFilter`: Ein `changeEvent` am Fenster sieht diesen Weg nicht. |
| **3** — Gilt bei mindestens zwei deutlich verschiedenen Schriftgrößen | Derselbe Test läuft über 9 pt und 24 pt. Bilder `13-schrift-9pt-fuenf-zeilen.png` und `14-schrift-24pt-fuenf-zeilen.png`. |

**Die Zusicherung ist relativ, nicht absolut.** Gemessen wird gegen den
Zeilenabstand der jeweils geltenden Schrift, nicht gegen eine Pixelzahl — sonst
prüfte der Test die Schriftauswahl der Maschine mit.

---

## 3. Akzeptanzkriterien #55 — je mit Beleg

Nummerierung nach der AK-Liste des Issues (A-Nummern der UX-Beratung in
Klammern).

| AK | Beleg |
|---|---|
| **1 (A4)** — Rundung, Kontur, Schatten aus dem Desktop-Theme, keine festen Werte; Prüfsatz: bei zwei Themes mit unterschiedlichem Rand unterscheiden sich **Randmaß und Eckform** | Test `hullFollowsTheDesktopTheme()`: alle vier Ränder unter dem 8-px-Theme größer als unter dem 4-px-Theme, **und** die Eckform am Bild gemessen (durchsichtiger Anlauf in der obersten Zeile) unterscheidet sich. Beide Aussagen **relativ**, keine Zahl im Test. Messung 4 (`theme-eckstuecke.txt`) über alle acht Themes: Eckform 6 gegen 7, Rand 4/4/4/4 gegen 8/7,99998/8,00002/8,00011. Bilder 01–12. |
| **2 (A1)** — durchgehende Fläche nach 4b; Notiztext mit `WindowText`, nicht der Feldrolle | Tests `paintsOneSurfaceInThePaletteColours()` (Farbe hinter dem Textfeld = Farbe daneben = `Window`) und `noteTextUsesTheWindowTextRole()` (Textrolle folgt `WindowText`, auch nach einem Schemawechsel — kein eingefrorener Wert, #54). Bilder 01–12: kein Kasten im Kasten. |
| **3 (A5)** — Innenabstände **zuzüglich** Theme-Rand; Abstand über der Fußzeile größer als unter dem App-Namen | Erste Hälfte: `hullFollowsTheDesktopTheme()` misst die Innenabstände des Layouts, die sich mit dem Theme ändern. Zweite Hälfte: `footerHasMoreAirThanTheApplicationName()`, gemessen an den **ausgelegten Widgets** und bei **beiden** Fenstergrößen (DoD 1). |
| **4 (A6)** — Hülle bei 5 wie bei 8 Zeilen vollständig und unverzerrt | `hullIsCompleteAtFiveAndEightLines()`: gleiche Eckform bei beiden Größen, jede Kantenmitte und die Fensermitte deckend, unter **beiden** Desktop-Themes. Bilder 03/06/09/12 zeigen den Achtzeilenfall samt Scrollbalken. |
| **5 (A7)** — nach einem Theme-Wechsel **bei laufendem Dienst** folgt die Hülle | `hullFollowsTheDesktopTheme()` wechselt das Theme an **einem stehenden Fenster**; `readsTheDesktopThemeFromPlasmarc()` prüft denselben Weg über die Datei. Dass eine Änderung dieser Datei zugestellt wird, ist in Messung 2 gemessen. |
| **6** — keine Titelleiste, kein Dekorationsrahmen | `wearsNoDecoration()`. Bild 15 aus der laufenden Sitzung zeigt es am echten Compositor. |
| **7 (A8)** — Belegformen getrennt: Rundung und Kontur offscreen im Bild (3 Zustände × 2 Schemata × 2 Themes, mit `QT_QPA_PLATFORMTHEME=kde`); Schatten als Bild am laufenden Plasma **oder** benannte Zusicherung | **Zwölf Bilder** 01–12 aus `tests/captureshots.cpp`, Läufer vor jedem Lauf frisch gebaut. **Schatten: mehr als die Zusicherung** — `bindsAShadowFromTheThemeTiles()` belegt Objekt und **Kachelquelle**, und Messung 5 belegt am laufenden Plasma `KWindowShadow::create() == wahr`. Siehe 4. |
| **8 (A9)** — `KF6::Svg` verkabelt; SPEC 3 und 15 nachgezogen; Randfall ohne `dialogs/background` ohne Absturz | Verkabelung in `CMakeLists.txt` und `src/CMakeLists.txt` (vier Bibliotheken, nicht eine — siehe 5.1). SPEC-Nachzug in Abschnitt 7. Randfall: `staysUsableWithoutADesktopTheme()`, **in einem eigenen Prozess** mit beschnittenem `XDG_DATA_DIRS` — im laufenden Prozess ist der Zustand nicht herstellbar (5.3). |

### 3.1 Terminalausgabe der beiden Pflichtläufe

```
$ QT_QPA_PLATFORM=offscreen ./build/bin/capturetest
Totals: 20 passed, 0 failed, 0 skipped, 0 blacklisted, 133ms

$ QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde ./build/bin/capturetest
Totals: 20 passed, 0 failed, 0 skipped, 0 blacklisted, 144ms

$ ctest --test-dir build
100% tests passed out of 7
```

Der Bau ist warnungsfrei (`cmake --build build` ohne `warning`), was seit dem
04.08.2026 auch die CI verlangt.

### 3.2 Alle elf neuen Zusicherungen sind gegen Mutation geprüft

Dieses Projekt hat an einem Abend vier grüne Tests entlarvt, die nichts
prüften. Ich habe deshalb jede neue Zusicherung gegen eine Mutation des
Produktivcodes gehalten — sie muss rot werden, wenn das Verhalten verschwindet.

**Die Tabelle unten deckte zunächst nur acht der elf**, und der Satz darüber
behauptete trotzdem Vollständigkeit. Der karpathy-Review hat das als **K1**
beanstandet; die drei fehlenden Mutationen stehen nachgefahren in **§11**, und
die Überschrift nennt seither die Zahl, die die Tabellen tragen. *Der Befund
war berechtigt und ist die unangenehmste Stelle dieses Berichts:* Er ist genau
die Bauart — eine Behauptung breiter als ihr Beleg —, gegen die dieses Projekt
seine Evidenzpflicht geschrieben hat.

| Mutation | rot wird |
|---|---|
| Theme-Rand nicht auf die Innenabstände rechnen | `hullFollowsTheDesktopTheme`, `readsTheDesktopThemeFromPlasmarc` |
| Hülle gar nicht zeichnen (immer die Ersatzfläche) | `hullFollowsTheDesktopTheme`, `hullIsCompleteAtFiveAndEightLines` |
| Notiztext auf der Feldrolle lassen | `noteTextUsesTheWindowTextRole` |
| Schatten nicht binden | `bindsAShadowFromTheThemeTiles` |
| Fußzeilenabstand = Abstand des App-Namens | `footerHasMoreAirThanTheApplicationName` |
| Kacheln über `pixmap(id)` statt `image(elementSize, id)` | `bindsAShadowFromTheThemeTiles` |
| `FontChange` im `eventFilter` entfernen | `heightFollowsAFontChange` |

**Vollständig wird die Liste erst mit den drei Mutationen in §11.1** — bis
dahin fehlten `paintsOneSurfaceInThePaletteColours` (AK 2, tragend),
`wearsNoDecoration` (AK 6) und `staysUsableWithoutADesktopTheme` (AK 8).

**Die zweite Zeile hat beim ersten Durchgang nur *einen* Test gerissen.**
`hullIsCompleteAtFiveAndEightLines` blieb grün, obwohl gar keine Hülle mehr
gezeichnet wurde: Ohne Hülle ist jede Eckform null, und „bei beiden Größen
gleich" gilt dann auch. Der Test prüfte Vollständigkeit, aber nicht Existenz.
Eine Zeile ergänzt (`QVERIFY(cornerRun(atFive) > 0)`), Mutation wiederholt,
jetzt rot. **Die letzte Zeile ist die Mutation, die einen echten Fehler in
meinem Code gefunden hat** — siehe 5.2.

---

## 4. Der Schatten: die benannte Grenze ist geschlossen, nicht nur benannt

Das Planning (K4, §6.2) sah für den Schatten einen **benannten Ersatz** statt
eines Bildes vor, weil `KWindowShadow::create()` offscreen falsch liefert und
`grab()` den Schatten nie zeigt. Beides trifft zu und ist unabhängig bestätigt.
Der Auftrag verlangte, das ausdrücklich in den Bericht zu schreiben — hier steht
mehr, weil sich mehr messen ließ.

**Was jetzt belegt ist** (`schatten-am-compositor.txt`, Messung 5, am laufenden
Plasma, Plattform `wayland`):

```
A) Nach dem ERSTEN Zeigen          B) Nach dem ZWEITEN Zeigen
   Schattenobjekt vorhanden : ja      Schattenobjekt vorhanden : ja
   vom Compositor angenommen: ja      vom Compositor angenommen: ja
   Kachel oben              : 32x16   Kachel oben              : 32x16
   Kachel oben links        : 16x16   Kachel oben links        : 16x16
```

Damit sind **beide** Punkte erledigt, die der Auftrag wörtlich als Grenze
aufführte:

1. **„Schatten angelegt"** ist keine Zusicherung mehr, sondern ein
   Rückgabewert — `create() == wahr` am echten Compositor. Die Kachelquelle ist
   zusätzlich im Test festgehalten (`bindsAShadowFromTheThemeTiles`).
2. **„Der Schatten wird nach jedem Remap neu gebunden"** ist Abschnitt B
   derselben Messung: Das Programm versteckt das Fenster und zeigt es erneut —
   `showCapture()` zerstört dabei die Wayland-Surface (SPEC 3) — und liest
   danach erneut ab. Der Schatten liegt beim zweiten Mal wieder darunter.

**Was das *nicht* sagt, und das gehört dazu:** `create() == wahr` heißt, der
Compositor hat die Kacheln **angenommen**. Es heißt nicht, dass der Schatten
gut aussieht. Genau diese Lücke hat mich einen echten Fehler gekostet — der
erste Bau übergab acht Mal das gesamte Schattenbild statt acht Kacheln, und
`create()` meldete auch dafür wahr (5.2). Der **Augenschein** bleibt deshalb
Sache der Abnahme.

**Warum kein Bild des Schattens im Repository liegt:** `spectacle -a` schneidet
auf das Fensterrechteck, und der Schatten liegt außerhalb davon. Ein Bild, das
ihn zeigte, bräuchte einen Bereichsausschnitt und nähme den Desktop des Kunden
mit — dieses Repository ist öffentlich (Kundenentscheidung 02.08.2026). Bild 15
belegt deshalb Hülle, Rundung und Farben am **echten** Compositor; für den
Schatten selbst bleibt der Foto-Punkt der Abnahme.

**Satz für die Abnahme-Checkliste**, wie beauftragt:

> **Fenster schließen, Kürzel erneut drücken — liegt der Schatten beim zweiten
> Mal noch darunter?**

Er bleibt sinnvoll, obwohl der Rückgabewert ihn stützt: Der Wert sagt
„angenommen", das Auge sagt „sichtbar und richtig geformt".

---

## 5. Was schiefging — drei Befunde in meiner eigenen Arbeit

Alle drei sahen richtig aus. Alle drei fielen durch Messung, keiner durch
Nachdenken.

### 5.1 Die Hülle brauchte vier neue Abhängigkeiten, nicht eine

Das Planning nannte `KF6::Svg` und `KF6::WindowSystem`. Dazu kamen
`KF6::ConfigCore` und `KF6::CoreAddons`, und beide folgen aus einer Messung:
**KSvg findet das Desktop-Theme nicht selbst.** Auf `plasma/desktoptheme`
gezeigt bleibt ein `ImageSet` auf `default` — auch wenn in `plasmarc` etwas
anderes steht (Messung 1, entschieden mit einem eigenen `XDG_CONFIG_HOME`).
Der Name muss also gelesen (ConfigCore) und seine Änderung bewacht werden
(CoreAddons). Warum `KDirWatch` und nicht `KConfigWatcher`, steht in Messung 2:
Letzterer meldet **nur**, wenn der Schreiber `KConfig::Notify` benutzt hat —
die Zusicherung von AK 5 hinge damit an der Disziplin eines fremden Programms.

### 5.2 Der Schatten bestand aus acht Mal demselben Bild — und alles war grün

Der erste Bau holte die Kacheln über `KSvg::Svg::pixmap(elementID)`. Der Aufruf
nimmt eine Element-Kennung entgegen und **ignoriert sie**: Zurück kommt jedes
Mal das ganze Bild (Messung 6). Aufgefallen ist es an einer Nebensache — in
Messung 5 waren „Kachel oben" und „Kachel oben links" beide 157×64 groß.

Warum nichts anschlug:

- `KWindowShadow::create()` nahm die falschen Kacheln an und meldete **wahr**;
- kein Bild dieses Projekts zeigt den Schatten, also war nichts zu sehen;
- **die Zusicherung im Test verglich gegen denselben falschen Aufruf** und war
  deshalb grün.

Heilung: `image(elementSize(id), id)`. Die Zusicherung hält seither zusätzlich
fest, dass die Eckkachel **nicht** so groß ist wie die obere — die Aussage, an
der ein Rückfall scheitert (Mutation geprüft). Der Vorgang ist als Messung 6
versioniert, weil er die Bauart genau trifft, gegen die `CLAUDE.md` seine
Prüfhaltung fasst.

### 5.3 Zwei Prüfaufbauten, in denen der Fehler nicht auftreten konnte

- **Der Randfall ohne Desktop-Theme** (AK 8) lässt sich **nicht** durch einen
  erfundenen Theme-Namen herstellen: KSvg fällt dabei auf `default` zurück und
  zeichnet die Hülle wie immer. Mein erster Test tat genau das und war grün,
  ohne irgendetwas zu prüfen. Der Zustand braucht ein beschnittenes
  `XDG_DATA_DIRS`, also einen eigenen Prozess; `capturetest` startet sich dafür
  selbst neu.
- **Der Theme-Wechsel** wirkte zunächst gar nicht: Der Bildläufer schrieb unter
  zwei verschiedenen Desktop-Themes **byteweise identische** Bilder. Grund:
  Ein `FrameSvg` folgt seinem `ImageSet` nicht, wenn das Set umbenannt wird.
  Pfad neu setzen wirkt nicht, dasselbe Set erneut zuweisen wirkt nicht — nur
  ein **frisches** Set (Messung 3). Ohne den Bildbeleg wäre das nicht
  aufgefallen: Der Code hätte eine Hülle gezeichnet, die richtig aussieht und
  zum falschen Theme gehört.

---

## 6. Grenzen der Prüfbarkeit — was ich **nicht** belegen konnte

Ausdrücklich, weil ein Bericht, der nur Erfolge nennt, in diesem Projekt schon
zweimal aufgeflogen ist.

1. **Kein Bild des Schattens.** Begründung in Abschnitt 4. Der Rückgabewert ist
   gemessen, das Aussehen nicht. **Bleibt Foto-Punkt der Abnahme.**
2. **Kein Nachweis am installierten Stand.** Untersagt (ein `/usr`, zweiter
   Strang läuft) — der PO taktet die Installation. Geprüft ist der gebaute
   Stand.
3. **Der Dienst wurde nicht gestartet.** Der installierte `denkzetteld` des
   Kunden läuft (PID 4569); ein zweiter hätte um den D-Bus-Namen und die
   Kürzelregistrierung konkurriert. Statt seiner habe ich das **Fenster selbst**
   auf der laufenden Sitzung gefahren (Messung 5, Bild 15) — der Hauptweg
   beider Stories führt durch das Fenster, nicht durch den Dienst. Das Journal
   trägt deshalb keinen Eintrag zu diesem Lauf.
4. **Der letzte Meter des Theme-Wechsels ist nicht end-to-end geprüft.** Belegt
   ist: die Hülle folgt einem neuen Namen am stehenden Fenster (Test), sie liest
   ihn aus `plasmarc` (Test), und eine Änderung dieser Datei erreicht eine
   `KDirWatch`-Wache (Messung 2). **Nicht** geprüft ist der Zusammenbau — dass
   ein Wechsel über die Systemeinstellungen im laufenden Fenster ankommt. Dafür
   müsste das Desktop-Theme des Kunden umgestellt werden. **Vorschlag für die
   Abnahme-Checkliste:** *„Desktop-Theme umstellen, ohne den Dienst neu zu
   starten — wechselt die Hülle mit?"*
5. **#56 ist am Kundenblick nicht prüfbar** (Planning 4.1.1). Plasma reicht
   Qt-Widgets-Anwendungen Schriftänderungen nicht nach; der Nachweis ist Test
   und Bild. Das ist keine Auslassung, sondern die Lage.

---

## 7. SPEC-Nachzug (DoD 4)

Nachgezogen wurde nicht nur, was sich geändert hat, sondern auch, was sich beim
Bauen als **Bedingung** herausgestellt hat (DoD 4 in der Fassung nach B9).

- **SPEC 3** — neuer Satz zur Höhenrechnung bei Schriftwechsel (#56).
- **SPEC 3.1 (neu)** — die Hülle: Farbrollen, Maße, „Form vom Theme, Farbe aus
  der Palette", `frameContrast` 0,20, und die ausdrückliche Feststellung, dass
  **Rundung und Rand keine Zahlen dieser Spezifikation sind**.
- **SPEC 3.2 (neu)** — fünf **entdeckte Bedingungen**, ohne die die Festlegung
  nicht gilt: KSvg liest `plasmarc` nicht · ein `FrameSvg` folgt nur einem
  frischen `ImageSet` · Zustellung über `KDirWatch`, nicht `KConfigWatcher` ·
  ohne Theme keine Hülle, aber ein brauchbares Fenster (und ein unbekannter
  Name erzeugt diesen Zustand *nicht*) · der Schatten wird nach jedem Neuzeigen
  neu gebunden. Vier davon widersprechen dem, was der Aufbau nahelegt.
- **SPEC 15** — `KSvg`, `KWindowShadow` und `KCoreAddons`/`KDirWatch` in die
  Abhängigkeitsliste. Der wörtliche Beispielfall von DoD 4/B9.
- **SPEC 16** — `capturetest` in den Bedingungssatz zum Plattformthema
  aufgenommen; dazu zwei neue Absätze: was offscreen prinzipbedingt nicht zu
  belegen ist (Schatten, mit den beiden Ersatzformen), und dass Zustände, die
  im Prüfprozess nicht herstellbar sind, einen eigenen Prozess brauchen.

---

## 8. Außerhalb meiner Fläche aufgefallen — gemeldet, nicht geheilt

**B1 · Ein gezeigtes Fenster schrumpft nicht zurück, und ein grüner Test
verdeckt es.**

Wird das Erfassungsfenster **gezeigt** und dann von acht Zeilen auf leer
geräumt, bleibt es auf der Achtzeilenhöhe stehen. Ursache: Das Layout setzt beim
Auslegen eine Mindesthöhe des Fensters; das `resize()` in `adjustHeight()` wird
davon gedeckelt, und danach löst nichts ein erneutes Anpassen aus.

*Gemessen, nicht vermutet, und ausdrücklich gegen den Ausgangsstand geprüft:*

```
Basisstand 0a229d2 (vor #55/#56)   verdeckt: ruhend=162 acht=216 nach clear=162  -> schrumpft
                                   gezeigt : ruhend=162 acht=216 nach clear=216  -> SCHRUMPFT NICHT
Stand dieses Strangs               verdeckt: ruhend=174 acht=228 nach clear=174  -> schrumpft
                                   gezeigt : ruhend=174 acht=228 nach clear=228  -> SCHRUMPFT NICHT
```

**Der Befund ist vorbestehend** — er stammt nicht aus diesem Strang. Die
unangenehme Hälfte ist die zweite: `windowFollowsTheTextHeight()` sichert
`QCOMPARE(m_window->height(), resting)` nach dem Leeren zu und ist **grün, weil
der Test das Fenster nie zeigt**. Unter einem gezeigten Fenster fiele diese
Zusicherung. Das ist ein fünfter Fall für die Sammlung „grüner Test, der nichts
prüft" — und er liegt in meiner Dateimenge, aber außerhalb der
Akzeptanzkriterien beider Stories. Ich habe ihn deshalb **nicht** geheilt.
*Empfehlung: eigenes Issue.*

**B2 · Ein Fremdbibliotheks-Absturz, den der Produktweg nicht reproduziert.**

Eine Wegwerf-Sonde, die `KSvg::ImageSet`-Objekte auf dem Stapel anlegt und
sofort wieder zerstört, bricht bei fehlenden Theme-Dateien reproduzierbar mit
Heap-Korruption ab (`malloc(): unaligned tcache chunk`, SIGABRT im Konstruktor
von `KSvg::ImageSet`, fünf von fünf Läufen). **Der Produktweg reproduziert es
nicht** — `captureshots` baut unter denselben Bedingungen vierzehn Fenster und
endet mit Exit 0, und die AK-8-Zusicherung läuft grün. Ich führe es auf, weil
ein Absturz in einer Bibliothek, die wir neu aufnehmen, nicht unerwähnt bleiben
sollte; als Fehler unseres Codes ist er nicht belegt.

**B3 · Journalmeldung des installierten Dienstes** (vorbestehend, nicht aus
diesem Strang): `Failed to register with host portal QDBusError(…, "Connection
already associated with an application ID")` beim Start von `denkzetteld`.
Fällt beim Journal-Blick auf; berührt weder #55 noch #56.

---

## 9. Was der PO wissen muss

1. **Zwei Grenzen der Prüfbarkeit sind geschlossen** (Schatten, Remap), eine
   dritte bleibt (Aussehen des Schattens) und zwei Sätze gehören in die
   Abnahme-Checkliste (Abschnitte 4 und 6, Punkt 4).
2. **K6 ist erledigt**: `theme-eckstuecke.txt` liegt versioniert und
   wiederholbar vor. Die Eckstück-Aussage der AK-1-Richtigstellung hängt nicht
   mehr an einem Planning-Absatz — alle acht Themes tragen Eckstücke und
   Schattenkacheln.
3. **Die Bedingung aus Planning 2.3 hält**: Der Bildläufer ist auf #55 gebucht;
   #56 benutzt ihn für zwei Bilder mit und zahlt ihn nicht. #56 bleibt bei 1 SP.
4. **Zwei zusätzliche Bibliotheken** (`KF6::ConfigCore`, `KF6::CoreAddons`)
   gegenüber der Planning-Annahme, aus einer Messung begründet (5.1). Beide sind
   auf der Maschine vorhanden; kein Beschaffungsrisiko.
5. **B1 gehört als Issue angelegt** — vorbestehend, mit einem grünen Test
   darüber.

---

## 10. Nachtrag 04.08.2026 — der blockierende PO-Befund ist behoben

**Der Befund war richtig.** `tests/capturetest.cpp` nannte zwei Desktop-Themes
beim Namen, und `CachyOS-Nord-round` kommt aus `cachyos-nord-kde-theme-git`.
Damit hing die tragende Zusicherung von #55 an der Distribution dieser
Maschine.

### 10.1 Nachgemessen — und der Befund ist größer als gemeldet

| Theme | Paket | Rand | Eckform |
|---|---|---|---|
| `default` | `kdeplasma-addons`, `libplasma` | 4 | 6 |
| `breeze-dark`, `breeze-light` | `libplasma` | 4 | 6 |
| `CachyOS-Nord-round` | `cachyos-nord-kde-theme-git` | 8 | 7 |
| `Iridescent-round` | `cachyos-iridescent-kde` | 8 | 7 |
| `cachyos-emerald{,-color,-light}` | `cachyos-emerald-kde-theme-git` | 8 | 7 |

Der gemeldete Teil trifft zu: **Der gesamte offizielle KDE-Bestand trägt 4 px**,
jedes breitere Theme stammt aus einem CachyOS-Paket. Ein anderes Namenspaar
hätte das nicht gelöst.

**Dazu ein Teil, der in der Meldung fehlte und den Zuschnitt ändert:**
`ksvg` hängt **nicht** an `libplasma` (`pactree -d3 ksvg` findet es nicht).
Ein Bauplatz, der nur die KF6-Teile dieses Projekts installiert, hat deshalb
**gar kein Desktop-Theme** — auch kein `default`. Dort wäre nicht nur der
Vergleich schmal/breit gefallen, sondern **jede** Zusicherung, die eine Hülle
braucht: `narrowCorner > 0`, die Flächenprüfung, die Schattenkacheln. Die
CI-Paketliste allein hätte den Befund also nur halb geheilt.

**Ein Nebenbefund, der eine Zusicherung korrigiert hat:** Die Eckform folgt dem
Randmaß **nicht** — `default` paart 4 px Rand mit Eckform 6, `CachyOS-Nord-round`
8 px mit 7. Die alte Zusicherung lautete `wideCorner > narrowCorner` und leitete
damit genau das ab, was Zeichnung 4b ausdrücklich verbietet („Nicht aus dem
Randmaß ableiten"). Sie lautet jetzt **`wideCorner != narrowCorner`**. Das ist
keine Abschwächung, sondern die Aussage, die das AK trifft.

### 10.2 Gewählter Weg: A **und** B, nicht A oder B

Beide Vorschläge des PO, in der Aufteilung, die er selbst als Bedingung genannt
hat („nicht als Ersatz für den Lauf gegen echte Themes, sondern neben ihm"):

- **`tests/themes/`** — zwei eigene Prüf-Themes (Rand 4 gegen 12, Eckform 3
  gegen 11, eigene Schattenkacheln), eingehängt über `XDG_DATA_DIRS`. Sie
  tragen die Zusicherung auf **jeder** Maschine, auch auf einer ohne jedes
  Plasma-Theme. Was sie **nicht** belegen: dass der Code ein echtes
  Plasma-Theme liest — nur, dass er ein SVG von uns liest.
- **`themes::installedThemePair()`** — sucht zur Laufzeit zwei **installierte**
  Themes mit verschiedenem Rand, **gemessen statt benannt**. Das ist der Lauf,
  der das Echte belegt, und er ist der, den es nicht überall gibt.

Der neue Test `hullFollowsAnInstalledDesktopTheme()` trägt den zweiten Teil und
überspringt mit benanntem Grund, wenn kein Paar da ist. `hullFollowsTheDesktopTheme()`
trägt den ersten und läuft immer. Die Vollständigkeits- und
Schattenzusicherungen laufen über die mitgelieferten Themes **und**
zusätzlich über ein installiertes, wo eines liegt.

*Das Einhängen per `qputenv` mitten im Prozess ist gemessen, nicht angenommen:*
QStandardPaths liest `XDG_DATA_DIRS` bei jedem Zugriff neu, ein Kindprozess ist
dafür nicht nötig. Der Lauf „ohne Desktop-Theme" (AK 8) hängt sie
folgerichtig **nicht** ein — dort ist der leere Datenpfad der Prüfgegenstand.

### 10.3 Die beiden Bedingungen des PO — nachgewiesen

**Bedingung 1: Die Zusicherung bleibt relativ.** Es steht keine Zahl im Test,
auch keine Schwelle. Verglichen wird Theme gegen Theme: Ränder größer,
Eckformen verschieden. Die Namen der Prüf-Themes stehen im Test, ihre Maße
nicht.

**Bedingung 2: Die Mutationsprobe greift weiter, und der Lauf gegen echte
Themes läuft auf Ganymed** — er skippt nicht. Sieben Mutationen, alle auf
Ganymed gefahren:

| Mutation | rot | Skips |
|---|---|---|
| Theme-Rand nicht auf die Innenabstände rechnen | `hullFollowsTheDesktopTheme`, **`hullFollowsAnInstalledDesktopTheme`**, `readsTheDesktopThemeFromPlasmarc` | **0** |
| Hülle gar nicht zeichnen | `hullFollowsTheDesktopTheme`, **`hullFollowsAnInstalledDesktopTheme`**, `hullIsCompleteAtFiveAndEightLines` | **0** |
| Notiztext auf der Feldrolle | `noteTextUsesTheWindowTextRole` | 0 |
| Schatten nicht binden | `bindsAShadowFromTheThemeTiles` | 0 |
| Fußzeilenabstand = Abstand des App-Namens | `footerHasMoreAirThanTheApplicationName` | 0 |
| Kacheln über `pixmap(id)` | `bindsAShadowFromTheThemeTiles` | 0 |
| `FontChange`-Zweig entfernt (#56) | `heightFollowsAFontChange` | 0 |

Die beiden ersten Zeilen sind der geforderte Nachweis: Der Test gegen
installierte Themes **wird rot**, also läuft er.

### 10.4 Gegenprobe in zwei fremden Umgebungen

Simuliert über `XDG_DATA_DIRS`, ohne etwas zu installieren:

```
A) nur offizielle KDE-Themes (default, breeze-dark, breeze-light)
   SKIP : hullFollowsAnInstalledDesktopTheme() — Kein Paar installierter
          Desktop-Themes mit verschiedenem Rand gefunden …
   Totals: 20 passed, 0 failed, 1 skipped

B) gar kein Desktop-Theme (Bauplatz mit nur den KF6-Teilen)
   SKIP : hullFollowsAnInstalledDesktopTheme() — dieselbe Meldung
   Totals: 20 passed, 0 failed, 1 skipped
```

Auf Ganymed: **21 passed, 0 failed, 0 skipped**, in beiden Pflichtumgebungen.

### 10.5 Die Bilder

Der Läufer sucht das Paar jetzt ebenfalls zur Laufzeit und nimmt **bevorzugt
installierte** Themes — die Bilder sollen Plasmas eigene Hüllen zeigen, nicht
unser SVG. Auf Ganymed wählt er `breeze-dark` und `CachyOS-Nord-round`; die 14
Bilder sind deshalb **byteweise unverändert**. Wo kein Paar liegt, weicht er auf
die Prüf-Themes aus und sagt es laut. **Welche Themes eine Reihe zeigt, steht
jetzt in `bilder/themes.txt`** — am Bild ist es nicht abzulesen, und ohne diese
Datei wäre die Herkunft der Bilder ab jetzt maschinenabhängig und unvermerkt.

### 10.6 Was der PO noch wissen muss

**Die CI-Paketliste allein reicht nicht** (10.1): Ohne Desktop-Theme fiele mehr
als der Vergleich. Mit den mitgelieferten Prüf-Themes ist die Suite auch ohne
jedes Theme-Paket grün — das Paket in der CI macht aus dem einen Skip einen
Lauf und ist damit ein Gewinn, aber keine Voraussetzung mehr. **Ob es die Mühe
wert ist, ein CachyOS-Paket in einen Arch-Container zu holen, ist deine
Entscheidung; nötig ist es nicht.**

`SPEC.md` 16 ist um die Bedingung ergänzt (DoD 4/B9): *Keine Zusicherung hängt
an einem Namen, den nur diese Maschine kennt.*

---

## 11. Nachtrag 04.08.2026 — die karpathy-Befunde K1 bis K3

Alle drei sind behoben. K1 durch **Messung**, nicht durch Einschränkung des
Satzes: Die drei fehlenden Mutationen sind gefahren, alle drei greifen.

### 11.1 K1 (`fail`) — die drei fehlenden Mutationen

| Mutation | rot wird | Skips |
|---|---|---|
| **beide** Hälften der einen Fläche entfernt — `viewport()->setAutoFillBackground(false)` **und** `Base = transparent` | `paintsOneSurfaceInThePaletteColours`, dazu `staysUsableWithoutADesktopTheme` | 0 |
| Ersatzfläche ohne Theme entfernt (`painter.fillRect(rect(), surface)`) | `staysUsableWithoutADesktopTheme` | 0 |
| `Qt::FramelessWindowHint` aus dem Konstruktor entfernt | `wearsNoDecoration` | 0 |

**Damit tragen alle elf neuen Zusicherungen eine Mutationszeile** — sieben aus
§3.2/§10.3, drei hier, und `hullFollowsAnInstalledDesktopTheme` aus §10.3.

**Zu `wearsNoDecoration`, mit der Einordnung, die der Reviewer vorgeschlagen
hat:** Es ist eine **Bestandszusicherung, nicht tragend**. Die beiden Flags
stehen seit Sprint 1 im Konstruktor und sind von dieser Story nicht angefasst
worden; der Test hält AK 6 fest — *die Hülle ersetzt die Titelleiste nicht,
sie ist, was ein Fenster ohne Titelleiste statt nichts trägt* — und schützt sie
gegen eine spätere Regression. Ich habe die Mutation trotzdem gefahren statt
sie nur zu kennzeichnen: Ein Beleg ist billiger als eine Einordnung, der man
glauben muss.

### 11.2 K2 (`warn`) — gefahren, und der fünfte Fall ist es **nicht**

Der Reviewer hatte den Verdacht ausdrücklich als *unbelegt* gekennzeichnet. Er
ist jetzt belegt, in drei Läufen:

| Lauf | Ergebnis |
|---|---|
| nur `viewport()->setAutoFillBackground(false)` entfernt | **grün** — 21 passed |
| nur `Base = Qt::transparent` entfernt | **grün** — 21 passed |
| **beide** entfernt | **rot** — `paintsOneSurfaceInThePaletteColours` fällt |

**K2 ist damit erledigt, und die Redundanz ist begründet.** Die beiden Hälften
sprechen zwei verschiedene Zeichner an: `setAutoFillBackground(false)` hält das
Sichtfenster-Widget davon ab, seine Hintergrundrolle zu füllen;
`Base = transparent` hält `QPlainTextEdit::paintEvent` davon ab, mit dem
Base-Pinsel zu füllen. Dass hier **jede allein genügt**, ist an dieser
Plattform und diesem Widget-Stil gemessen — nicht allgemein.

**Die unangenehme Hälfte, die zum Befund gehört und nicht verschwiegen wird:**
Keine der beiden Zeilen ist **einzeln** testgedeckt. Wer eine davon streicht,
bekommt eine grüne Suite. Ich habe trotzdem **keine entfernt**: Die Messung
deckt eine Plattform, und beide Zeilen führen auf AK 2 zurück. Wer sie später
zusammenstreichen will, hat mit dieser Tabelle die Messung, die dafür fehlt.

### 11.3 K3 (`warn`) — Sprachbruch behoben, und mehr davon gefunden

Die drei gemeldeten Kommentarblöcke sind **übersetzt**, nicht weggeworfen
(`capturetest.cpp` zweimal, `captureshots.cpp` einmal).

**Beim Nachsehen fielen weitere Ersatzschreibungen auf, die der Review nicht
nennt** — in Dateien, die er offenbar nicht als Textquellen gelesen hat: die
Kopfkommentare beider Prüf-Themes (`Pruef-Theme`, `gewoehnlichem`, `faende`,
`ueberhaupt`, `haengt`) und die `Name`/`Description`-Felder ihrer
`metadata.json`. Sie stammen aus derselben Nachlässigkeit — die Dateien sind
über ein Skript entstanden, und dabei sind die Umlaute ASCII geworden.

**Behoben, indem alles unter `tests/` auf Englisch gezogen wurde**, Prüf-Themes
eingeschlossen. Das ist die Regel, die dort ohnehin gilt (*„Bezeichner und
Code-Kommentare englisch"*), und sie nimmt der Digraphen-Frage die Grundlage,
statt sie einzeln zu beantworten. Die `KPlugin.Id` beider Themes ist
**unverändert** — sie steht im Test.

*Nicht angefasst:* Die `pruefen`-Vorkommen in `tests/spellfixspike.cpp` sind
kein Verstoß, sondern der Prüfgegenstand des Spikes — dort wird gerade
zugesichert, dass jemand, der `pruefen` tippt, `prüfen` findet.

### 11.4 Stand nach der Nachbesserung

```
Ganymed, beide Pflichtumgebungen        21 passed, 0 failed, 0 skipped
nur offizielle KDE-Themes (simuliert)   20 passed, 0 failed, 1 skipped
ctest                                   7/7
```

Bau warnungsfrei. **K4 ist nicht meine Sache** (PO).

---

## 12. Nachtrag 04.08.2026 — U4 aus dem UI-Review

**Der Befund war richtig, und ich habe ihn selbst nachgesehen**, bevor ich ihn
geheilt habe: In Bild 02 lag über der Fußzeile ein waagerechter Rollbalken über
die volle Breite, mit einer Linie darüber. Zeichnung 4b kennt ihn nicht, und das
Feld bricht Zeilen um — zu rollen gibt es nichts.

### 12.1 Was war und was ist

Der Läufer griff das Bild **unmittelbar** nach `show()`. Der Balken lebt genau
zwischen `show()` und dem ersten Durchlauf der Ereignisschleife; der Läufer hielt
damit einen Zustand fest, den das ruhende Fenster nicht hat.

Geheilt in `shoot()` selbst, nicht an den Aufrufstellen: `show()` **und** der
Durchlauf der Ereignisschleife stehen jetzt beide dort. Ein Bild, das später
ergänzt wird, kann die zweite Hälfte damit nicht mehr vergessen — genau das war
der Fehler, denn `capturetest::shot()` machte es seit dem ersten Tag richtig und
der Läufer nicht.

**Es ist kein Produktbefund.** Ob der Balken im laufenden Betrieb je ein Bild
lang aufblitzt, ist damit **nicht** gesagt; belegt ist nur, dass er in den
Belegbildern stand und im ruhenden Fenster nicht steht.

### 12.2 Neu erzeugt wurden alle vierzehn Bilder, nicht vier — mit drei Unterschieden

Der Läufer ist frisch gebaut und über `pruefen.sh` gefahren. Dass sich **alle**
Bilder geändert haben, gehört erklärt; ich habe die Unterschiede gegen den alten
Stand ausgezählt statt sie zu überfliegen:

| Bild | geänderte Pixel | was sich unterscheidet |
|---|---|---|
| 01 (leer) | 18 | nur der Textcursor |
| 02 (getippt) | 5.078 | Rollbalken weg, dazu der Textcursor |
| 03 (acht Zeilen) | 694 | Textcursor und der Griff des senkrechten Balkens |
| 13 (Schrift 9 pt) | 16 | nur der Textcursor |

1. **Der waagerechte Rollbalken ist fort** — 02, 05, 08, 11. Der Befund selbst.
2. **Der Textcursor wird jetzt gezeichnet.** Das Feld hat den Tastaturfokus
   (`setFocusProxy`), und ein eingeschwungenes Fenster zeichnet seinen Cursor.
   Zeichnung 4a zeichnet ihn ebenfalls — die Bilder sind damit **näher** an der
   Vorlage als vorher, nicht weiter weg.
3. **Der Griff des senkrechten Balkens** ist in den Achtzeilenbildern von Grau
   `(203, 205, 205)` auf ein helles Blau `(177, 217, 239)` gewechselt.

**Zu Punkt 3, weil er wie eine Verschlechterung aussieht und keine ist:** Ich
habe geprüft, ob es eine noch laufende Animation ist — bei 400 ms zusätzlicher
Wartezeit steht dieselbe Farbe. Es ist der **eingeschwungene** Zustand: Vorher
war der Balken noch nicht durchgestylt, und das alte Grau war derselbe
Zwischenzustand, aus dem auch der waagerechte Balken stammte. Die Farbe kommt
aus der **synthetischen Prüf-Palette** des Läufers, die sieben Rollen setzt;
Breeze leitet den Griff daraus ab. Der senkrechte Balken selbst ist richtig —
er gehört zum Zustand „acht Zeilen mit Scrollbalken" und steht weiter da.

**Die Palette habe ich nicht erweitert.** Das wäre eine Änderung ohne AK: 4b
nimmt Auswahl, Cursor und Scrollbalken ausdrücklich aus („unverändert aus der
Palette und dem Widget-Stil"), das UI-Review hat die Farbrollenfragen bereits
mit `pass` beantwortet, und es würde erneut alle vierzehn Bilder verändern. Wer
die echten Farben sehen will, hat Bild **15** aus der laufenden Sitzung.

### 12.3 Was ich nicht angefasst habe

- **Die Bilder des UI-Reviews** (`sprint-06-ux-review/`) — sie dokumentieren den
  geprüften Stand. Dass meine Reihe seit dieser Korrektur davon abweicht, ist
  richtig so: Bild 20 des Reviews zeigt die Szene bereits ohne Balken.
- **U1** (Kontur läuft nicht um die Rundung), **U2** (Fenster bleibt ab dem
  zweiten Öffnen zu groß), **U3** (Notiztext 4 px weiter rechts als der
  App-Name) — außerhalb der Akzeptanzkriterien beider Stories, vom PO als Issues
  gebucht. **Nicht geheilt.**

**Zu U2, als Schärfung meiner eigenen Meldung B1** (§8): Das Review hat
gemessen, dass eine Schriftverkleinerung am **gezeigten** Fenster das Feld
richtig auf fünf Zeilen schrumpfen lässt, während das Fenster auf 299 px stehen
bleibt. AK 1 von #56 ist damit wörtlich erfüllt — es spricht vom **Textfeld** —,
aber der Fall liegt näher an #56, als meine Meldung sagte. B1 und U2 sind
dieselbe Ursache: Die Mindesthöhe, die das Layout beim Auslegen setzt, deckelt
das `resize()`, und danach löst nichts ein erneutes Anpassen aus.

### 12.4 Stand

```
Ganymed, beide Pflichtumgebungen   21 passed, 0 failed, 0 skipped
ctest                              7/7
Bilder                             14 neu erzeugt aus frisch gebautem Läufer,
                                   Bild 15 (laufende Sitzung) unberührt
```

---

## 13. Nachtrag 04.08.2026 — M1: die zehn clazy-Befunde dieses Sprints

**Selbst nachgezählt, nicht übernommen** — beide Zahlen des PO stimmen, und der
Unterschied zwischen ihnen hat eine Ursache, die man kennen muss:

```
vorher:   16 Zeilen in der Zählweise der CI   (Schwelle 3)
          13 verschiedene Stellen, davon 10 aus diesem Sprint
nachher:   3 Zeilen  —  genau die drei Altbefunde, genau die Schwelle
```

**Warum 16 und nicht 13:** Die CI zählt `grep -c 'warning:'`, also **Zeilen**,
nicht Stellen. `tests/desktopthemes.h` ist ein Header und wird über **zwei**
Übersetzungseinheiten verarbeitet (`capturetest.cpp` und `captureshots.cpp`);
seine drei Befunde zählen doppelt. 13 + 3 = 16. Wer künftig die Schwelle
nachzieht, sollte das wissen: **Ein Befund in einem Header kostet so viele
Zähler, wie er Übersetzungseinheiten hat.**

### 13.1 Die drei Befundklassen

**`non-pod-global-static`, `capturewindow.cpp:57–60`** — vier `const QString`
im namenlosen Namensraum. Sie werden in unbestimmter Reihenfolge relativ zu
anderen Übersetzungseinheiten gebaut und beim Beenden wieder abgeräumt.
Geheilt als `constexpr QLatin1StringView`: derselbe Name, dieselbe Stelle, aber
ein literaler Typ ohne Konstruktorlauf. Eine Anpassung war nötig —
`KConfigGroup::readEntry` leitet seinen Typ aus dem Vorgabewert ab und kann
keinen View lesen, dort steht jetzt `QString(DefaultDesktopTheme)`.

**`range-loop-detach`, fünf Stellen** — drei in `desktopthemes.h`, zwei in
`capturetest.cpp`. Die drei im Header liefen über **Zwischenwerte**
(`QDir::entryList(...)`, `installedThemes()`); dort hilft `std::as_const`
nicht, sondern eine benannte `const`-Variable. Die beiden im Test laufen über
einen veränderlichen `QStringList` und sind mit `std::as_const` geheilt — dem
Weg, den clazy selbst vorschlägt.

**`connect-non-signal`, `capturewindow.cpp:192`** — dem Vorschlag des PO
gefolgt, und ich sehe **keinen Grund**, die Schleife zu behalten: Zwei
ausgeschriebene `connect`-Aufrufe mit einer benannten Lambda-Variablen sind
kürzer als Schleife plus Unterdrückung, doppeln nichts, und clazy versteht sie.
Der Befund war inhaltlich falsch — beide sind Signale mit gleicher Signatur —,
aber ein Ausnahmekommentar wäre teurer als die Auflösung.

### 13.2 Belege

```
Neubau in einem eigenen Bauplatz, wie ihn die CI fährt:
   Compiler-Warnungen   0   (Schwelle 0)
   clazy-Befunde        3   (Schwelle 3)  — librarytest 2×, shelltest 1×
   ctest                7/7

capturetest, beide Pflichtumgebungen:
   QT_QPA_PLATFORM=offscreen                            21 passed, 0 failed, 0 skipped
   QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde   21 passed, 0 failed, 0 skipped

Bildläufer neu gebaut und gefahren: die 15 Bilder sind byteweise unverändert.
```

**Die Schwelle bleibt bei 3** und ist damit wieder das, was sie sein soll — der
gemessene Altbestand, nicht eine Schranke darüber. Sie zu heben wäre der
Fehler, den `ci.yml` in seinem eigenen Kommentar benennt.

*Zur Redlichkeit:* Der Bau war die ganze Zeit warnungsfrei, DoD 1 war nicht
gerissen. Gerissen war die CI-Kundenentscheidung vom 04.08.2026 — und zwar von
meinem Code. Dass die rote Marke erst nach dem Merge auffiel, ändert nichts
daran, wo die zehn Befunde herkommen.
