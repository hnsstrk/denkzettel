# Vorprüfung #85 — Messung B (Scrum Master)

**Story:** #85 „Capture: Lesbarkeit unter fremden Desktop-Themes",
sechs Akzeptanzkriterien · **Bearbeiter B**, unabhängig von Bearbeiter A
(`denkzettel-ux`); `messung-a.md` ist für diese Messung nicht geöffnet worden.

**Stand:** `717f077`, 05.08.2026, 20:10 CEST, Ganymed. Die Messungen liefen
teils gegen `402ee8b`; zwischen beiden Ständen ist `git diff --name-only -- src/
tests/` leer, der gemessene Code also derselbe.

**Prüfstand** (B17): kwin 6.7.3, kwindowsystem 6.28.0, ksvg 6.28.0,
qt6-base 6.11.1, plasma-desktop 6.7.3; acht installierte Desktop-Themes,
19 Farbschemata; Farbschema des Kunden `CachyOSNordLightly`
(`Window` 30,34,51 · `WindowText` 102,194,242 · `ForegroundInactive`
102,106,115); `plasmarc` führt nur `[Wallpapers]`, also greift der Rückfall
`default`.

**Wiederholbar:** `bash docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/pruefen-b.sh`
Es baut nicht, installiert nicht und verstellt keine Einstellung des Kunden;
gelesen werden der Quellcode, `/usr/share/plasma/desktoptheme/*/colors` und die
bereits abgelegte Compile-Datenbank unter `build/lint/`. Ausgaben in
`messungen-b/b1…b5`.

**Der Boden dieser Messung ist der Code nach #83.** Das Label `size:m` an #85
stammt aus dem Schnitt vom 04.08.2026 und ist bis hierher eine Fortschreibung;
es wird unten ohne Rücksicht darauf neu gesetzt.

---

## Feld 1 — Dateimenge

Notation nach B13, am Code vermessen (`messungen-b/b2-dateimenge.txt`).

| | **#85** |
|---|---|
| **Issue / Zweig** | #85 (`epic:M1`, `typ:story`) · `story/85-lesbarkeit-fremde-themes` |
| **Quellen und Tests** | `src/capture/capturewindow.cpp` (587 Z.) an vier Stellen: freie Funktion im Namensraum `capture` neben `contrastEffectOf()` `:114–142`, `reloadDesktopTheme()` `:260–299` (AK 5), `applyTextColours()` `:440–454` (AK 1), Ereignisfilter `:332–335`. `src/capture/capturewindow.h` (167 Z.): Deklaration neben `contrastEffectOf()` `:36–45`, ggf. ein Feld neben `m_contrast` `:160–161`.<br>`tests/capturetest.cpp` (1018 Z.): `textsFollowAColourSchemeChange()` `:330–355`, `noteTextUsesTheWindowTextRole()` `:563–586`, dazu neue Prüfsätze zu AK 1 und AK 5.<br>`tests/themes/plasma/desktoptheme/…`: **eine `colors`-Datei an einem mitgelieferten Prüf-Theme** — keines der drei hat heute eine (Feld 2, F-B2). Muster: `denkzettel-test-breit` hat mit #83 eine `[ContrastEffect]`-Gruppe bekommen.<br>ggf. `tests/desktopthemes.h` (164 Z.) für einen Namen wie `bundledColoured()`. |
| **Build** | **Nichts.** `KF6::ConfigCore` ist an `denkzettelcapture` verlinkt (`src/CMakeLists.txt:48 ff.`), `QStandardPaths` ist in Gebrauch, und `tests/CMakeLists.txt:57` reicht den **Ordner** `tests/themes` durch — eine neue Datei darin braucht keine CMake-Zeile. |
| **Belege und Prüfmittel** | `docs/scrum/reviews/sprint-NN-s85-lesbarkeit/` mit eigenem `pruefen.sh` nach dem Muster von `sprint-07-s83-native-huelle/`. **Wiederverwendbar:** `…/83-native-huelle/sonden/weichzeichnerbeleg.cpp` (Bauplan des Sitzungsbelegs, AK 2), `…/reviews/2026-08-04-abnahme-befunde/sonden/echtelage.cpp`, `…/ux-beratung/sonden/deckung.cpp` (Theme-Achse, AK 3), `…/po-themeschrift.py` sowie `messungen-b/kontraste.py` dieser Vorprüfung (AK 4), `sprint-07-s83-native-huelle/mutationsproben-sitzung.sh` (Sitzungsschiene für AK 6). Alle mit `git ls-files` geprüft, `messungen-b/b5-pruefmittel-existenz.txt`. |
| **Fachliche Quellen** | **SPEC 3.1** (`:152–205`) — der Spiegelstrich „**Notiztext** `WindowText`" ist die Festlegung, die diese Story ändert; DoD 4 zieht ihn nach. **SPEC 3.2** (`:206 ff.`) nimmt die entdeckten Bedingungen auf. |
| **Ausdrücklich nicht** | `src/ui/`, `src/shell/`, `src/store/`, `src/capture/textareaheight.*`, `subtleLabel()` `:96–108` (die gedämpfte Rolle ist **#84**; AK 4 misst sie und behebt sie nicht), `tests/librarytest.cpp`, `tests/libraryshots.cpp`, `tests/editshots.cpp`, `tests/searchshots.cpp`, `tests/readmeshots.cpp`, `wireframes/`, `CLAUDE.md`, `docs/scrum/PROZESS.md`, `.claude/agents/`, Belegordner fremder Sprints. `tests/captureshots.cpp` wird voraussichtlich nicht geändert, seine **Bilder** ändern sich aber, sobald der Bildläufer unter einem Theme mit eigener `colors`-Datei läuft. |

### 1.1 Kollisionsfläche

Gemessen gegen die beiden anderen Sprint-8-Kandidaten in Feld 7.

---

## Feld 2 — Gemessene Fallen

Elf Zeilen für den Spawn-Auftrag, jede mit Beleg. **Die drei teuren zuerst.**

**F-B1 — Die Zahlen im Issue sind deckend gerechnet, und die ungestützten
liegen anders.** Das Issue sagt es für AK 2 selbst; es gilt auch für die
Kopfzahl von AK 1. Gerechnet gegen die Fläche mit ihrem gemessenen Alpha, über
weißem beziehungsweise schwarzem Grund, davon der schlechtere Wert
(`messungen-b/b1-kontraste.txt`, Methode aus `ux-beratung/sonden/kleintext.py`):

| Theme | Alpha | heute deckend / ungestützt | nach AK 1 deckend / ungestützt |
|---|---|---|---|
| `breeze-light` | 216 | 1,75 / **1,23** | 13,35 / **9,41** |
| `breeze-dark` | 216 | 7,94 / 4,88 | 15,39 / **9,45** |
| `cachyos-emerald-color` | 7 | 10,56 / 1,87 | 9,57 / 2,07 |
| **`cachyos-emerald-light`** | **7** | 10,56 / **1,87** | 1,38 / **1,38** |

**Der eingekaufte Preis ist kleiner, als die Kundenvorlage ihn ausgewiesen
hat.** Für `cachyos-emerald-light` steht dort „10,56:1 — gut" gegen „1,38:1 —
unlesbar". Bei 2,7 % Deckung steht der Text auf dem Bildschirmhintergrund, und
dort ist der heutige Wert **1,87**, nicht 10,56. Die Verschlechterung geht von
1,87 auf 1,38 — beide unter jeder Lesbarkeitsschwelle. **Der Gewinn bleibt in
beiden Rechnungen ein Gewinn**, `breeze-light` 1,23 → 9,41.

**F-B2 — Kein Prüfmittel dieses Projekts erreicht heute den Theme-Zweig von
AK 1.** Gemessen (`b2-dateimenge.txt`, Abschnitte 5 und 6): Keines der drei
mitgelieferten Prüf-Themes bringt eine `colors`-Datei mit. Und
`themes::anyInstalledTheme()` hängt an einer Sortierung, die keine Zusicherung
ist — `QDir::entryList(…, QDir::Name)` ohne `QDir::IgnoreCase` liefert
großschreibungsempfindlich `CachyOS-Nord-round` (**ohne** `colors`), ohne
Unterscheidung `breeze-dark` (**mit** `colors`). Ein Prüfsatz auf
`anyInstalledTheme()` prüft je nach Sortierung den einen oder den anderen Zweig
und sagt nicht, welchen. **Zeile für den Auftrag:** Der Nachweis läuft gegen ein
**mitgeliefertes** Theme mit eigener `colors`-Datei; das Anlegen dieser Datei ist
Umfang der Story. Dieselbe Fehlerklasse wie F16 in #83 — grün, und nichts
geprüft.

**F-B3 — Ein grüner Prüfsatz sichert heute das Gegenteil von AK 1 zu.**
`noteTextUsesTheWindowTextRole()` (`capturetest.cpp:563–586`) setzt ein neues
Farbschema und verlangt, dass die Notiztextfarbe ihm folgt. Unter einem Theme
mit eigener `colors`-Datei darf sie das nach AK 1 nicht mehr. Der Prüfsatz
bleibt trotzdem grün, weil sein Theme keine `colors`-Datei hat (F-B2). Wer ihn
stehenlässt, hat einen laufenden Test, der die neue Regel verneint.

### Zum Mechanismus

**F-B4 — `reloadDesktopTheme()` fasst die Textfarbe nicht an.** Die Funktion
(`capturewindow.cpp:260–299`) setzt Bildsatz, Auswahlpfad, Kontrastwerte,
Ränder, Höhe, Hülle und Schatten. `applyTextColours()` wird von zwei Stellen
gerufen — dem Konstruktor `:205` und dem Ereignisfilter `:333` —, von hier
nicht. **AK 5 hängt genau an dieser fehlenden Zeile.**

**F-B5 — `applyTextColours()` hängt am Palettenwechsel** (`:332–335`,
`QEvent::PaletteChange` auf `m_text`). Wer die Themefarbe allein in
`reloadDesktopTheme()` setzt, verliert sie beim nächsten Schemawechsel, und
zwar lautlos: kein Rückgabewert, kein Ereignis, kein roter Test. Die beiden
Quellen brauchen eine Vorrangordnung an **einer** Stelle. Dies trägt den Befund
in Feld 3.

**F-B6 — Seit #83 hängt die Deckung am Auswahlpfad, und offscreen greift
`opaque`.** `sessionBlursBehindWindows()` gibt außerhalb von Wayland und X11
`false` zurück (`:151–153`), woraufhin `reloadDesktopTheme()` den Auswahlpfad
`opaque` setzt (`:283–285`): `default` deckt offscreen zu 100 % statt zu 84,7 %.
Eine Sonde nach dem Muster von `deckung.cpp` baut ihr eigenes `ImageSet` **ohne**
Auswahlpfad und misst weiter die durchscheinende Fassung. **Wer AK 3 offscreen
nachmisst, misst nicht, was das Programm dort zeichnet** — der Messweg muss
sagen, welcher Pfad gemeint ist.

**F-B7 — Das Theme lässt sich am Fenster setzen, ohne `plasmarc` anzufassen.**
`reloadDesktopTheme(const QString &name)` nimmt einen Namen entgegen
(`capturewindow.h:118–125`); der Bildläufer nutzt das bereits. AK 2 und AK 5
brauchen keinen Eingriff in die Einstellung des Kunden.

**F-B8 — Das Muster für das Selbstlesen steht in derselben Datei.**
`capture::contrastEffectOf()` (`:114–142`, 29 Zeilen) liest `metadata.desktop`
des Themes über `QStandardPaths::locate` und `KConfigGroup`. Die `colors`-Datei
liegt im selben Ordner und ist dieselbe Dateiart. Am Build ist nichts zu tun.

**F-B9 — Es gibt womöglich einen kürzeren Weg, und er ist ungemessen.**
`KSvg::Svg::color(StyleSheetColor)` ist öffentlich (`svg.h:629`),
`libKF6Svg.so` verlinkt `libKF6ColorScheme.so.6` und trägt die Zeichenkette
`/colors` (`b2-dateimenge.txt`, Abschnitt 7). Das legt nahe, dass KSvg die
`colors`-Datei des Themes selbst liest und ohne sie auf das Farbschema
zurückfällt — also genau die Regel von AK 1. **Was `m_hull->color(KSvg::Svg::Text)`
je Theme liefert, ist nicht gemessen**; ein Lauf dazu ist die erste Handlung
des Strangs und entscheidet, ob AK 1 eine Zeile kostet oder dreißig.

### Zwei Kleinigkeiten, die je einen Fehlversuch sparen

**F-B10 — Die gedämpfte Textklasse hat drei Stellen, nicht zwei.** Neben
App-Name und Fußzeile trägt sie der Platzhaltertext des Eingabefeldes
(`m_text->setPlaceholderText`, `:199`), gezeichnet aus `PlaceholderText` der
Palette von `m_text`. Die Zahl von AK 4 ändert sich dadurch nicht.

**F-B11 — Die Zeilenangabe in AK 4 ist überholt.** Das Kriterium nennt
`capturewindow.cpp:132` für `subtleLabel()`; dort steht heute eine schließende
Klammer. Die Zeile ist **105**, die Funktion beginnt bei 96
(`b5-pruefmittel-existenz.txt`).

---

## Feld 3 — AK-Urteil: **ready = nein**

Ein Befund und vier Auflagen. Alle Prüfmittel, die das Issue nennt, existieren
(`b5-pruefmittel-existenz.txt`, sechs von sechs). Selbstdeklarierte offene
Punkte führt das Issue keine.

| AK | Urteil | Kurz |
|---|---|---|
| 1 | **Auflage** | Kopfzahlen deckend gerechnet; Prüfgrundlage fehlt (F-B2) |
| 2 | **Auflage** | die Zahl gilt für einen Compositor-Stand, an dem der Kontrasteffekt unbeobachtbar ist |
| 3 | **Auflage** | der Messweg muss den Auswahlpfad nennen (F-B6) |
| 4 | **Auflage** | Zeilenangabe überholt (F-B11) |
| 5 | **ok** | trifft die fehlende Zeile genau (F-B4); Prüfmittel vorhanden |
| 6 | **ok** | zählbar formuliert, Muster aus #83 vorhanden |
| — | **nein** | **es fehlt ein Kriterium** (unten) |

### Der Befund: die Regel von AK 1 hat keinen Prüfsatz für den Schemawechsel

AK 1 sagt, woher die Textfarbe kommt. AK 5 sagt, dass sie einem **Theme**-Wechsel
folgt. Kein Kriterium sagt, was bei einem **Schema**-Wechsel geschieht, während
ein Theme mit eigener `colors`-Datei eingestellt ist — und genau dort sitzt der
gemessene Mechanismus: `applyTextColours()` hängt am Palettenwechsel (F-B5) und
schreibt `WindowText` des Schemas in die Palette des Textfeldes. Eine Umsetzung,
die AK 1 allein in `reloadDesktopTheme()` erfüllt, ist beim Abnehmen richtig und
nach dem ersten Schemawechsel falsch. Nichts meldet das: kein Rückgabewert, kein
roter Test — der einzige einschlägige Prüfsatz sichert heute das Gegenteil zu
und bleibt grün (F-B3).

Das ist dieselbe Lücke, die der Schnitt vom 04.08.2026 schon einmal gelassen
hat; damals fehlte das Kriterium zum Theme-Wechsel und wurde als #85 AK 5
nachgetragen. Die symmetrische Hälfte fehlt noch.

**Satz zum Übernehmen, als neues AK:**

> **AK 7** — Ist ein Desktop-Theme mit eigener `colors`-Datei eingestellt,
> **bleibt** die Textfarbe des Notiztextes bei einem Wechsel des Farbschemas
> unverändert; ohne eine solche Datei folgt sie ihm wie bisher. Belegt an je
> einem Prüfsatz für beide Fälle. *Grund:* `applyTextColours()` hängt am
> Palettenwechsel (`capturewindow.cpp:332–335`) und ist heute die Stelle, an
> der die Schemafarbe gesetzt wird; ohne dieses Kriterium gilt AK 1 bis zum
> ersten Schemawechsel.

**Und ein Satz zum Nachziehen an AK 1**, damit die Anpassung von
`noteTextUsesTheWindowTextRole()` nicht als Nebensache untergeht:

> Der Prüfsatz `noteTextUsesTheWindowTextRole()` wird auf die neue Regel
> gefasst; er sichert heute zu, dass der Notiztext dem Farbschema folgt.

### Die vier Auflagen

**AK 1 — die Kopfzahlen sind deckend gerechnet, und die Prüfgrundlage fehlt.**
*Erstens:* „1,75:1 → 13,35:1" rechnet gegen eine Fläche, die bei 84,7 % Deckung
so nicht steht. Ungestützt sind es **1,23:1 → 9,41:1** (F-B1). Der Gewinn bleibt;
die Zahl gehört gekennzeichnet oder ersetzt, sonst misst ein Strang gegen sie
und trifft sie nie. *Zweitens:* Der Nachweis braucht ein **mitgeliefertes**
Theme mit eigener `colors`-Datei — heute hat keines eine, und
`anyInstalledTheme()` entscheidet die Frage per Sortierung (F-B2). *Auflage:*
beides in das Kriterium, das Anlegen der Datei als Umfang benannt.

**AK 2 — die Zahl gilt für einen Compositor-Stand, und der ist besonders.** Auf
diesem Stand ist die **Wirkung** von `enableBackgroundContrast` nicht
beobachtbar (Übergabebericht #83, §4 Punkt 1). Gerade die Themes, um die es in
AK 2 geht, sind darauf gebaut, dass der Compositor ihren Grund abdunkelt. Die
gemessene Zahl beschreibt dann nicht, was das Theme vorsieht. *Auflage:* Ein
Satz, dass der Vermerk den Compositor-Stand nennt und die Unbeobachtbarkeit des
Kontrasteffekts dazu — B17-Form. Das Kriterium bleibt erfüllbar; es sagt dann
nur ehrlich, wofür seine Zahl gilt.

**AK 3 — der Messweg muss den Auswahlpfad nennen.** Seit #83 zeichnet das
Programm offscreen die deckende Fassung, eine Sonde nach dem genannten Muster
misst die durchscheinende (F-B6). *Auflage:* „gemessen für beide Auswahlpfade,
je benannt" — oder ausdrücklich für den Pfad der angemeldeten Sitzung.

**AK 4 — die Zeilenangabe ist überholt.** `capturewindow.cpp:132` zeigt auf eine
schließende Klammer; gemeint ist `:105` (F-B11). *Auflage:* Zeile berichtigen
oder streichen; der Funktionsname `subtleLabel()` trägt allein.

### Was ich als gelungen vermerke

- **AK 5** trifft die Lücke im Code genau, ohne sie zu benennen: Die Zeile, die
  fehlt, steht in `reloadDesktopTheme()` (F-B4). Ein Kriterium, das eine
  ungeschriebene Zeile beschreibt, ist selten.
- **AK 2** nennt Belegform, Prüfgrund und die Werkzeugfalle (`spectacle -a`
  taugt nicht) in einem Zug und spricht aus, dass die Folge **nicht** behoben
  wird. Das ist die Bauart, die die DoR meint.
- **AK 3** fordert eine benannte Grenze statt einer Lösung und sagt es selbst
  (DoD 4 in der Fassung nach B9).

---

## Feld 4 — Prüfmittel, und was ein Agent nicht prüfen kann

| AK | Prüfmittel | Grenze |
|---|---|---|
| **1** | QTest gegen ein **mitgeliefertes** Theme mit eigener `colors`-Datei, beide Zweige; dazu eine Mutationsprobe. Vorab ein Lauf über `KSvg::Svg::color(Text)` (F-B9) | Über installierte Themes ist der Zweig nicht sicher erreichbar (F-B2). Die genannten Kontrastzahlen sind eine Rechnung, keine Messung am Bild |
| **2** | Bildschirmaufnahme aus der angemeldeten Sitzung über einem abgelegten Prüfgrund; Bauplan `…/83-native-huelle/sonden/weichzeichnerbeleg.cpp`, Sitzungsschiene `…/sprint-07-s83-native-huelle/mutationsproben-sitzung.sh`. Theme über `reloadDesktopTheme(name)` (F-B7) | Die Zahl gilt für **diesen** Grund und **diesen** Compositor-Stand; die Wirkung des Kontrasteffekts ist unbeobachtbar. Eine Fensteraufnahme taugt nicht (steht im Kriterium, gemessen) |
| **3** | Sonde über `/usr/share/plasma/desktoptheme/`, Muster `ux-beratung/sonden/deckung.cpp` | Der Auswahlpfad gehört in den Messweg (F-B6), sonst misst die Sonde eine andere Fassung als das Programm |
| **4** | Rechnung aus den Farbdateien für beide Klassen, deckend **und** ungestützt — `messungen-b/kontraste.py` liegt vor und deckt beide Klassen über alle acht Themes | Bei 2,7 % Deckung beschreibt eine einzelne Zahl nichts; der Grund gehört dazu. Die Klasse hat drei Textstellen (F-B10) |
| **5** | QTest mit `reloadDesktopTheme(name)` über ein Theme mit und eines ohne `colors`; dazu ein Sitzungslauf | keine |
| **6** | Heilung entfernen, Rotwerden zeigen, Aufzählung im Übergabebericht | Eine Zusicherung, die offscreen nicht fallen kann, braucht die Sitzung — Muster S1–S3 aus #83 |
| **(7)** | zwei Prüfsätze, Schemawechsel unter je einem Theme mit und ohne `colors` | keine, sobald das Kriterium steht |

**Was ein Agent an dieser Story nicht prüfen kann:**

1. **Ob es lesbar genug ist.** Der Kontrastwert ist ein Maß; bei 2,7 % Deckung
   hängt der Eindruck am Bildschirmhintergrund des Kunden. Deshalb verlangt AK 2
   einen *benannten, abgelegten* Prüfgrund — die Zahl gilt für ihn und für
   nichts sonst.
2. **Ob `enableBackgroundContrast` wirkt.** `org_kde_kwin_contrast_manager`
   fehlt in der Globalenliste dieses Compositors; der Aufruf ist belegt, die
   Wirkung nicht. Das ist eine Eigenschaft des Standes.
3. **Den Bildschirmhintergrund des Kunden als Prüfgrund verwenden**, ohne seine
   Einstellung anzufassen.

---

## Feld 5 — Größenklasse: **`size:m`**

Neu gemessen auf dem Boden nach #83; das alte Label ist dabei nicht
herangezogen worden.

**Nicht `size:s`.** Die Klasse verlangt „wenige Dateien, **kein neuer
Prüfweg**". Beides trifft nicht zu: Es entsteht ein Mechanismus, den es heute
nicht gibt (die Textfarbe hat keine zweite Quelle, F-B4/F-B5), es entsteht ein
neues Prüfmittel (ein mitgeliefertes Theme mit eigener `colors`-Datei, F-B2),
und AK 2 führt einen sitzungsgebundenen Bildbeleg über einem abgelegten
Prüfgrund.

**Nicht `size:l`.** Die Klasse ist „füllt den Sprint". Gemessen dagegen: am
Build ist nichts zu tun (Feld 1, Zeile Build), der Codeanteil liegt bei rund
fünfzig Zeilen in zwei Dateien und folgt einem Muster, das zwanzig Zeilen weiter
oben in derselben Datei steht (F-B8); es gibt keine Compositor-Zusicherung, keine
offene Gestaltungsfrage und keinen zweiten Anmeldeweg. Die Sitzungsschiene für
AK 2 und AK 6 ist aus #83 vorhanden und muss nicht erfunden werden. Die beiden
Messtabellen, die AK 3 und AK 4 verlangen, liegen als Skript und Ausgabe bereits
in dieser Vorprüfung.

**Zum Vergleich, damit die Klasse nicht frei schwebt:** #83 trug vierzehn
Kriterien, drei Anmeldungen beim Fenstersystem, eine Bauentscheidung mit
Abhängigkeitswirkung und fünfzehn Probeläufe — das war die `l`. #85 trägt sechs
Kriterien, eine Farbquelle und eine Ereigniszeile. Der Abstand ist eine ganze
Klasse.

**Label:** `size:m`, zu setzen im selben Zug wie der konsolidierte Bericht.

---

## Feld 6 — Offene Fragen

### An den Product Owner

1. **Das fehlende Kriterium** (Feld 3). Ein Satz, Vorschlag liegt vor. Bis dahin
   ist #85 nicht ziehbar.
2. **Die vier Auflagen** an AK 1, 2, 3 und 4 — je ein bis zwei Sätze.
3. **Soll #85 den Farbsatz-Befund schließen?** Siehe Feld 8, Punkt 2. Das wäre
   zusätzlicher Umfang (ein Prüf-Theme, das die Farbsätze unterscheidet) und
   gehört entschieden, nicht nebenbei gebaut.
4. **Vor dem Sprint-8-Schnitt: die Dateimenge von #76 neu messen.** Ihre Zahlen
   stammen von `6acc87e`, also von vor #83. Gemessen ist der Bedarf: die
   Compile-Datenbank unter `build/lint/` ist ebenfalls von vor #83 und bricht an
   `'QDBusConnection' file not found` ab, weil `Qt6::DBus` erst mit #83 an
   `denkzettelcapture` kam (`messungen-b/b4-linterbefunde-nach-83.txt`).

### An den Kunden — zur Kenntnis, keine neue Entscheidung

Der Preis, den er am 04.08.2026 in Kauf genommen hat, ist kleiner als die
Vorlage ihn ausgewiesen hat. Dort stand für `cachyos-emerald-light` „10,56:1 —
gut" gegen „1,38:1 — unlesbar". Die 10,56 sind gegen eine gedachte volle Fläche
gerechnet; bei 2,7 % Deckung steht der Text auf dem Bildschirmhintergrund, und
dort liegt der heutige Wert bei **1,87**. Die Verschlechterung geht also von
1,87 auf 1,38, und beide Werte sind unbrauchbar. **Die Richtung der Entscheidung
ändert sich dadurch nicht** — die Vorlage hat die Einschränkung selbst genannt.
Es gehört ihm gesagt, weil die Zahl 10,56 in seiner Entscheidungsvorlage stand.

---

## Feld 7 — Der Sprint-8-Schnitt: #85 + #61 + #76

### 7.1 Die Regel hält, das Profil ist kleiner als Sprint 1

| Prüfung | Ergebnis |
|---|---|
| 2–4 Stories | drei — **hält** |
| kein `size:xl` | keins — **hält** |
| höchstens eine `size:l` | keine — **hält** |
| neben `size:l` nur `size:s` | greift nicht |

Profil: **`m m m`**. Der Auftrag nennt Sprint 1 als Vergleich; nachgemessen an
der Tabelle in `PROZESS.md` trug Sprint 1 das Profil **`s m m m`** — vier
Stories, drei davon `m`. Der vorgeschlagene Schnitt liegt also **eine `size:s`
unter** dem größten je angenommenen Sprint, nicht auf seiner Höhe. Die in
`PROZESS.md` benannte offene Flanke (vier `size:m` wären zulässig und lägen über
allem Gezogenen) wird nicht berührt.

### 7.2 Die Dateimengen je Strang

| | #85 | #61 | #76 |
|---|---|---|---|
| **Quellen** | `src/capture/capturewindow.{h,cpp}` | `src/main.cpp` | 6 Dateien in `src/`, 4 Kopfdateien mit Kern-Datentypen |
| **Tests** | `tests/capturetest.cpp`, `tests/themes/…` | ein neuer Nachweis, `tests/CMakeLists.txt` | Tests und **fünf Bildläufer** |
| **Build** | keiner | `CMakeLists.txt` der Wurzel, `src/CMakeLists.txt` | `.clang-tidy`, `.github/workflows/ci.yml` |
| **SPEC** | 3.1 (`:152–205`) | 2.3 (`:54–67`), 2.4 (`:68–123`), 15 (`:810 ff.`) | — |

### 7.3 Der kleinste gemessene Abstand ist null, und zwar zweimal

| Paar | gemeinsame Datei | kleinster Abstand |
|---|---|---|
| **#85 gegen #61** | `SPEC.md` | **29 Zeilen** (2.4 endet bei 123, 3.1 beginnt bei 152) |
| **#85 gegen #76** | `tests/capturetest.cpp` | **null — dieselben Prüfsätze** |
| **#61 gegen #76** | `src/main.cpp` | **null — dieselbe Funktion** |

Gemessen am heutigen Stand mit `clang-tidy -p build/lint`
(`messungen-b/b4-linterbefunde-nach-83.txt`, `b3-schnitt-abstand.txt`):

- `tests/capturetest.cpp` trägt fünf Befunde. Zwei davon sitzen **in den beiden
  Prüfsätzen, die #85 umschreibt**: `:349` in
  `textsFollowAColourSchemeChange()` (`:330–355`) und `:569` in
  `noteTextUsesTheWindowTextRole()` (`:563–586`). Die Variable in Zeile 569 ist
  genau die, deren Palettenzusicherung #85 anfasst.
- `src/main.cpp` trägt drei Befunde in den Zeilen 41, 59 und 61 — im
  Startabschnitt, in den #61 seine Argumentbehandlung vor der
  Einzelinstanz-Weiche einzieht.
- `src/capture/capturewindow.cpp` zeigt heute **keinen eigenen Befund** mehr.
  **Dieser eine Wert ist unsicher:** Der Lauf bricht an
  `'QDBusConnection' file not found` ab, weil die Compile-Datenbank von vor #83
  stammt; hinter dem Abbruch kann eine Prüfung ausfallen. Der Befund ist
  deshalb als Anlass zur Neumessung zu lesen, nicht als Freispruch.

### 7.4 Urteil zum Schnitt

**Ich vertrete #85 + #61 + #76 nur mit zwei Auflagen.** Ohne sie vertrete ich
ihn nicht.

**Auflage 1 — #76 läuft nicht parallel, sondern als letzter Strang** auf dem
gemergten Stand von #85 und #61. Der Grund steht in #76s eigenem
Vorprüfbericht: „*Wer dort `const`-Korrektheit und `enum`-Basistypen heilt,
heilt Code, den #83 löscht. Das ist nicht nur Mischkonflikt, das ist verworfene
Arbeit.*" Genau dieses Argument hat #76 aus Sprint 7 herausgehalten — mit der
Begründung, nach Sprint 7 sei der Code endgültig. **Für die Capture-Dateien
stimmt das nicht mehr, sobald #85 im selben Sprint liegt.** Zwei der fünf
Befunde in `capturetest.cpp` sitzen in den Prüfsätzen, die #85 umschreibt; das
ist derselbe Fall, eine Story später. Sequentiell aufgelöst verschwindet er
vollständig — es ist dieselbe Auflösung wie zwischen Sprint 7 und 8, nur
innerhalb eines Sprints.

**Auflage 2 — die Dateimenge von #76 wird vor dem Start neu gemessen** (Feld 6,
Punkt 4). „20 Dateien, 81 Befunde" stammt von `6acc87e`. #83 hat
`capturewindow.cpp` in weiten Teilen neu geschrieben und `capturetest.cpp`
erweitert; die Befundlage in genau diesen Dateien ist heute unvermessen, und die
Lint-Datenbank ist es auch. Ein Strang, der gegen die alte Zahl arbeitet, sucht
Befunde, die es nicht mehr gibt, und übersieht die, die #83 hinzugefügt hat.

**#85 und #61 laufen parallel ohne Auflage.** Sie teilen eine einzige Datei
(`SPEC.md`) in Abschnitten, die 29 Zeilen auseinanderliegen — außerhalb der
Mischbreite von Git —, und keine Quelldatei. #85 fasst den Build nicht an, #61
fasst `capturewindow.*` nicht an.

### 7.5 Reihenfolge

**Zwingend ist genau eine:** #76 nach #85 und nach #61 (7.4). Zwischen #85 und
#61 besteht keine sachliche Abhängigkeit — sie berühren einander in keiner
Quelldatei, in keinem Testziel und in keiner Zusicherung. Die aus anderen
Berichten bekannten Reihenfolgen betreffen diesen Sprint nicht: „#61 vor #73"
(#73 liegt nicht im Schnitt) und „#76 nach #83" (erledigt).

---

## Feld 8 — Die beiden Impediments aus Sprint 7

### 8.1 Die Wirkung von `enableBackgroundContrast` — **erbt #85, blockiert nicht**

**Kein Kriterium von #85 sichert Lesbarkeit unter den durchscheinenden Themes
zu.** AK 1 legt die Quelle der Textfarbe fest, AK 2 misst und vermerkt, AK 3
nennt eine Grenze, AK 4 weist Zahlen aus. Eine unbeweisbare Zusicherung entsteht
daraus nicht.

Und die Messung stützt das: Bei den drei `cachyos-emerald`-Themes (Deckung
2,7 %) ist der ungestützte Kontrast heute **1,87:1** und nach AK 1 **1,38:1**
beziehungsweise **2,07:1** — keine dieser Zahlen ist lesbar, gleich aus welcher
Quelle die Schrift kommt (`b1-kontraste.txt`). **Die Lesbarkeit hängt dort nicht
an der Textfarbe, sondern am Kontrasteffekt** — und dessen Wirkung kann diese
Story so wenig herstellen wie #83 sie messen konnte.

**Folge:** eine Auflage an AK 2 (Feld 3), sonst nichts. Das Impediment bleibt
beim PO, wo es hingehört, und wandert nicht in die Story.

### 8.2 `setColorSet(Window)` ohne fallenden Prüfsatz — **erbt #85 nicht; es kann ihn schließen**

Der Satz aus dem Übergabebericht lautet: „*Von elf Themes auf dieser Maschine
unterscheidet keines die sieben Farbsätze.*" **Gemessen gilt er für die Grafik.
Für die Farben gilt er nicht** (`b1-kontraste.txt`, Abschnitt C):

| Theme mit eigener `colors` | verschiedene `ForegroundNormal` über sieben Farbsätze |
|---|---|
| `cachyos-emerald-color` | **6 von 7** — `Window` 0,199,144 · `View` 2,189,136 · `Button` 33,255,180 · `Complementary`/`Header` 0,132,91 · `Tooltip` 11,212,135 · `Selection` 254,254,254 |
| `breeze-light` | **3** — `Window` 35,38,41 gegen `Complementary` 252,252,252 |
| `cachyos-emerald-light` | **3** — dieselbe Aufteilung |
| `breeze-dark` | 1 |

Vier installierte Themes bringen eine `colors`-Datei mit, drei davon
unterscheiden die Farbsätze in der Textfarbe. Ein Eingriff, der `Window` durch
`Complementary` ersetzt, kippt unter `breeze-light` die Schrift von dunkel auf
hell — auf weißer Fläche eine Zusicherung, die fällt.

**Voraussetzung, und sie ist ungemessen:** Das gilt nur, wenn die Textfarbe über
einen farbsatz-abhängigen Weg gelesen wird, also über
`KSvg::Svg::color(StyleSheetColor)` mit gesetztem `colorSet` (F-B9). Liest die
Umsetzung stattdessen fest die Gruppe `[Colors:Window]`, wie AK 1 es wörtlich
sagt, bleibt der Farbsatz weiter unverbunden und das Impediment offen.

**Folge:** Blockiert wird nichts. Es ist eine Gelegenheit, keine Last —
Entscheidung des PO (Feld 6, Punkt 3). Der Strang misst F-B9 ohnehin als erste
Handlung; danach ist die Frage billig zu beantworten.

---

## Belege dieser Messung

Alle unter `docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/messungen-b/`:

| Datei | Inhalt |
|---|---|
| `kontraste.py` | Messskript: beide Textklassen, acht Themes, deckend und ungestützt, dazu die Farbsätze der `colors`-Dateien |
| `b1-kontraste.txt` | dessen Ausgabe — trägt F-B1, AK 4 und Feld 8.2 |
| `b2-dateimenge.txt` | Codestellen, Prüf-Themes, Sortierfalle, Farbquelle von KSvg — trägt Feld 1 und F-B2/F-B8/F-B9 |
| `b3-schnitt-abstand.txt` | Befunde in `src/main.cpp`, Prüfsatzgrenzen, SPEC-Abschnitte — trägt Feld 7 |
| `b4-linterbefunde-nach-83.txt` | `clang-tidy` auf den drei Capture-Dateien — trägt 7.3 und Feld 6, Punkt 4 |
| `b5-pruefmittel-existenz.txt` | `git ls-files` über die sechs im Issue genannten Prüfmittel, dazu F-B11 |

Wiederholbar: `bash docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/pruefen-b.sh`
