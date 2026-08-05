# Sprint 8

**Sprint-Ziel:** Was das Erfassungsfenster nativ macht, wird auch unter fremden
Themes lesbar — und die Anwendung sagt endlich, welche Version sie ist.

**Basis-Tag:** `sprint-08-basis` = `9f2063a` · **Milestone:** Sprint 8 ·
**Planning:** 05.08.2026

---

## 1. Freigabe und Rollenlage

Wie in Sprint 7: Der Kunde hat Freigabe und Abnahme beider Sprints am
05.08.2026 vorab an den PO übertragen und sieht das Gesamtergebnis nach
Sprint 8. Die Definition of Done bleibt vollständig in Kraft; die Übertragung
betrifft die Rolle des Abnehmenden, nicht den Prüfumfang. Begründung und
Wortlaut stehen in `sprint-07.md` §1.

**Eine Folge, die hier zum Tragen kommt:** Weil der Kunde **beide** Sprints in
einem Zug abnimmt, wird auch **einmal** installiert — am Ende von Sprint 8, für
den Endstand beider Sprints. Das ist die Form, die der Sprint-Abschluss für
parallel arbeitende Stränge ohnehin vorsieht („Am Sprint-Ende wird der Endstand
einmal installiert und der Hauptweg jeder Story daran ausgeführt"). Bis dahin
bleibt DoD 2 für **alle sieben Stories** beider Sprints offen; in Sprint 7 ist
das als Mangel **M1** gebucht und nicht stillschweigend abgehakt.

## 2. Der Zuschnitt

**Drei Issues, drei Stränge — zwei parallel, einer nachgelagert.**

| | **Strang A** | **Strang B** | **Strang C** |
|---|---|---|---|
| **Issue** | #85 Lesbarkeit unter fremden Themes (`m`) | #61 Versionsanzeige (`m`) | #76 Linterbefunde heilen (`m`) |
| **Zweig / Worktree** | `story/85-lesbarkeit` · `../denkzettel-85` | `story/61-versionsanzeige` · `../denkzettel-61` | folgt |
| **Quellen und Tests** | `src/capture/capturewindow.{h,cpp}`, `tests/capturetest.cpp`, `tests/themes/` | `src/main.cpp`, `CMakeLists.txt`, `src/CMakeLists.txt`, `tests/` | 19 Dateien quer durch das Repositorium |
| **SPEC** | 3.1, 3.2 | Ende von 15, dazu 2.3 und 2.4 | — |
| **Start** | sofort | sofort | **nach dem Merge von A und B** |

**Klassenprofil `m m m`.** Die Regel hält: kein `xl`, keine `l`, höchstens vier
Stories. Zum Vergleich mit der Sprint-Historie: Sprint 1 trug `s m m m` — dieser
Zuschnitt liegt **eine `size:s` darunter**, nicht auf gleicher Höhe.

### Warum #76 nicht parallel läuft

**Der kleinste gemessene Abstand ist null, und zwar zweimal.** `tests/capturetest.cpp`
trägt Linterbefunde, zwei davon **in den beiden Prüfsätzen, die #85 umschreibt**;
`src/main.cpp` trägt drei in genau dem Startabschnitt, in den #61 seine
Argumentbehandlung einzieht.

Das ist dasselbe Argument, mit dem #76 aus Sprint 7 herausgehalten wurde — *„wer
dort `const`-Korrektheit heilt, heilt Code, den der Nachbarstrang gerade
umschreibt"*. Es galt gegen #83 und gilt unverändert gegen #85 und #61.

**Auflage des Scrum Masters, vor dem Start erfüllt:** Die Dateimenge von #76 war
neu zu messen — „20 Dateien, 81 Befunde" stammte von `6acc87e`, also von **vor**
#83. Gemessen am Stand `9f2063a`:

| | vor #83 (`6acc87e`) | heute (`9f2063a`) |
|---|---|---|
| Rohzeilen | 140 | **140** |
| eindeutige Befunde | 81 | **81** |
| Dateien mit Befunden | 20 | **19** |
| `performance-enum-size` | 66 roh / 7 eindeutig | **66 / 7** |
| `misc-const-correctness` | 60 | **63** |
| `bugprone-narrowing-conversions` | 3 | **0** |

Die drei `bugprone-narrowing-conversions` kamen mit #55 und sind mit #83
gefallen. Sonst hat sich nichts bewegt — die Zahlen des Issues tragen weiter.

### Warum #85 und #61 nebeneinander laufen dürfen

**Sie teilen keine einzige Datei in Code oder Tests.** Gemeinsam ist allein
`SPEC.md`, und dort liegen ihre Abschnitte **29 Zeilen** auseinander (2.4 endet
bei 123, 3.1 beginnt bei 152). Git mischt mit drei Zeilen Kontext; ein
Textkonflikt ist ausgeschlossen. #61 schreibt zudem ausdrücklich **ans Ende von
Abschnitt 15**, nicht in die KF6-Aufzählung — das hält die Naht zusätzlich offen.

### UI-Stories im Sinne von DoD 3

| Issue | UI-Story? | Begründung |
|---|---|---|
| **#85** | **ja** | Lesbarkeit unter fremder Grafik; AK 2 verlangt einen Sitzungsbeleg |
| **#61** | **nein** | Kommandozeile und Registrierung; kein sichtbares Element. Der Über-Dialog ist als **#87** herausgeschnitten |
| **#76** | **nein** | Aufräumarbeit ohne sichtbare Wirkung. **Wird eine sichtbar, ist das ein Fehler der Story** — AK 3 verlangt je Änderung in `src/` den Nachweis, warum das Verhalten dasselbe bleibt |

## 3. Sprint-Konto (B12)

| Zeitpunkt | Stand | Anlass |
|---|---|---|
| Freigabe 05.08.2026 | **3 Issues · 3×m** | Zuschnitt oben |

Beide Grenzen mit Luft gehalten: drei von höchstens vier Issues, kein `xl`,
keine `l`. **Ein Zugang wäre keine Grenzüberschreitung** — anders als in
Sprint 7, der bei vier Issues startete.

## 4. PO-Entscheidungen an Kundenstelle

Fortlaufend, damit der Kunde sie nach Sprint 8 in einem Zug prüfen kann. Die
Entscheidungen K1 bis K6 stehen in `sprint-07.md` §6.

| Nr. | Gegenstand | Entscheidung | Begründung in Kürze |
|---|---|---|---|
| **K7** | **#85 — gilt „dieselbe Quelle wie die Fläche" auch für die gedämpften Kleintexte?** | **Ja, für beide Textklassen** | Die Messung entscheidet es **nicht** — keine der beiden Quellen gewinnt überall (`breeze-light` 2,09 gegen 3,70; `breeze-dark` identisch; die Emerald-Themes drehen je nach Grund). Also entscheidet die Konsistenz, und die Kundenentscheidung sagt „immer". Zwei Schriften aus zwei Quellen auf einer Fläche wären derselbe Bruch, den die Story heilt, nur eine Ebene tiefer. **Ausdrücklich nicht behauptet:** dass die gedämpfte Klasse damit lesbar wird — unter `breeze-light` erreicht keine Quelle 4,5 : 1. Das bleibt #84 |
| **K8** | **Wann wird installiert?** | **Einmal, am Ende von Sprint 8**, für den Endstand beider Sprints | Der Kunde nimmt beide Sprints in einem Zug ab. Eine Installation je Sprint hätte den ersten Stand geprüft und den zweiten überschrieben; der Sprint-Abschluss sieht für parallele Arbeit ohnehin **einen** Endstand vor. **Der Preis, ausdrücklich:** DoD 2 bleibt bis dahin für alle sieben Stories offen und ist in Sprint 7 als Mangel M1 gebucht |
| **K9** | **#76 — die acht Entscheidungen zu den Linterbefunden** | siehe Issue-Text | „auf 0" gilt wörtlich, mit acht `NOLINTNEXTLINE` und **einer** gemeinsamen Begründung · die `KGlobalAccel`-Prüfregel wird **gestrichen**, nicht mit `NOLINT` übergangen (dahinter steckt kein Fehler: Die Regel entstand acht Minuten **vor** dem Rückleseweg, der sie überflüssig macht) · für zurückgegebene lokale Variablen gewinnt `performance-no-automatic-move`, als **Regel** im Kommentar von `.clang-tidy` · `easily-swappable-parameters` bekommt `NOLINT`, der sichtbare Fall ein eigenes Issue (**#88**) · `spellfixspike.cpp` bleibt draußen, die Schalterstellung kommt ins Kriterium · **DoD 1 nennt künftig die Linterschwelle** (entdeckte Bedingung, DoD 4/B9) · die CI-Wache zählt die **Dreizahl** statt `grep`-Zeilen |

## 5. Was aus Sprint 7 mitgenommen wird

| Sache | Stand |
|---|---|
| **Installation nach `/usr`** | offen, siehe K8 — der eine Punkt, der einen Menschen braucht |
| **README-Bilder** | bleiben auf dem Stand vor #55. Sie hingen an #83, dann irrtümlich an #85; die richtige Bindung ist **#96** (der Läufer gibt KSvg kein Farbschema) |
| **Zehn neue Issues** #86–#95, dazu #96 | im Backlog geblieben, kein Zugang in einen laufenden Sprint |
| **Zwei Prüfmittel-Fallen** (gesperrte Sitzung, Vollbild-Prüfgrund) | als Punkte 8 und 9 in `denkzettel-dev.md` verankert |

## 5a. Lieferung

Alle drei Stränge zusammengeführt, jeder mit `--no-ff`. **Bau warnungsfrei,
`ctest` 9 von 9** (zwei neue Testbinärdateien aus #61), **`lint-tidy` und
`lint-clazy` je null** — am gemergten Stand vom PO selbst nachgemessen.

| Strang | Commits | Was |
|---|---|---|
| **A** (#85) | 2 | Beide Textklassen nehmen ihre Farbe aus derselben Quelle wie die Fläche. `KSvg::Svg::color()` mit gesetztem `colorSet` für den Notiztext, der KConfig-Weg für die gedämpfte Klasse — für `ForegroundInactive` hat KSvg kein Gegenstück |
| **B** (#61) | 4 | `denkzetteld --version` antwortet **`denkzettel 0.1.0`**. Die Nummer steht weiter allein in `project(… VERSION …)` und wird durchgereicht; `KAboutData` trägt sie |
| **C** (#76) | 7 | 88 Befunde geheilt, **37 `NOLINT` in vier Klassen** begründet stehengelassen, beide Linter auf null. `ci.yml` prüft die Dreizahl, DoD 1 nennt die Schwelle |

### Was #61 besser gemacht hat als verlangt

Die Story sollte die Namensfalle **umgehen**. Sie hat sie **beseitigt**:
`KAboutData::setApplicationData()` schreibt Organisationsdomäne und
Desktop-Dateinamen mit, und seine Vorgaben (`kde.org`, `org.kde.denkzettel`)
hätten den Busnamen aus SPEC 2.3 und den Kürzel-Komponentennamen aus SPEC 2.4
gebrochen — **ohne dass ein Rückgabewert es meldet**. Statt daneben
gegenzusteuern, setzt die Registrierung beide Werte am `KAboutData`-Objekt, und
`main.cpp` setzt sie nicht mehr doppelt. **Vorher entschieden zwei Setzer die
Frage nach Zeilenreihenfolge; jetzt steht der Wert an einer Stelle.**

### Der vierte `NOLINT`-Fall — die Entscheidung, an der der Sprint hing

AK 4 von #76 verlangt: *„ein vierter Fall geht als Frage an den PO, nicht als
stiller `NOLINT`."* Der Fall trat ein, und er war der schwerste des Sprints.

`misc-const-correctness` schlägt `const` auf Objekten vor, die über eine
Qt-Verbindung verändert werden — der Prüfer sieht die Änderung nicht, weil
`connect`, `QSignalSpy` und `QTest::qExec` ihr Ziel als `const QObject *` nehmen
und intern `const_cast`en. Der Maschinenfix hatte es an zwanzig Stellen getan.

**Der Fall, der es entschieden hat:**

```cpp
DialogWatch() {
    QObject::connect(&m_timer, &QTimer::timeout, &m_timer, [this] {
        …  m_appeared = true;          // schreibt in ein const-Objekt
    });
}
…
const DialogWatch watch;
QVERIFY2(!watch.appeared(), "…");     // darf der Compiler auf wahr falten
```

**PO-Entscheidung K10: `const` zurücknehmen, `NOLINT` mit gemeinsamer
Begründung, Regel als Kommentar in `.clang-tidy`.** Die Alternative, die auf
null Befunde käme, hieße einen Test stehen zu lassen, dessen Zusicherung ein
Optimierer aushebeln darf — **ein grüner Test, der nichts prüft, um eine Zahl
schöner zu machen.**

**Zur Ehrlichkeit der Entscheidungsgrundlage:** Der Strang hat versucht, den
Fall mit einem `-O2`-Lauf **hart zu belegen, und ist gescheitert** — die
betroffenen Tests laufen im Release grün. Er hat das mit negativem Ausgang in
den Bericht geschrieben und als eigene Grenze geführt. Die Entscheidung ruht
damit auf der Sprachnorm und dem gelesenen Änderungsweg, **nicht auf einem
Messwert**. Das steht hier, weil es sonst stärker aussähe, als es ist.

*Zwei Berichtigungen des PO durch den Strang, beide angenommen:* Es sind
**zwanzig** Stellen, nicht sechzehn — die Zahl des PO stammte aus einem
Maschinendiff, nicht aus einer Erhebung, und vier gleichlautende Zeilen standen
schon vorher da. Und die Regel in `.clang-tidy` ist weiter gefasst als
„QObject-Abkömmling", **weil `DialogWatch` keiner ist** — die vom PO diktierte
Fassung hätte ihren eigenen Kronzeugen nicht gedeckt.

### Der wertvollste Fund des Sprints stand in keinem Kriterium

**Im Release-Bau gehen Kategorien verloren.** Zwei Prüfsätze fallen mit `-O2`
(`ctest` 8/9), im Debug-Bau nicht; die Anzeige zeigt „Kategorie —",
`saved->category` ist leer. **Das besteht auf `main` vor diesem Sprint** und ist
vom PO unabhängig nachgemessen.

Gefunden hat es ein Strang, der aus einem ganz anderen Grund einen Release-Bau
brauchte. **In acht Sprints hat nie jemand mit `-O2` gebaut** — DoD 1, der
öffentliche Lauf und der installierte Stand bauen alle `Debug`. Gebucht als
**#99**; den Kunden trifft es heute nicht, jedes Paket aber schon.

### Prüfsummen der Bildbelege

Drei Gruppen im UI-Belegordner, alle einzeln beurteilt, **kein Mangel**:
`breeze-dark` und `default` erzeugen dasselbe Bild, weil die Themefarbe dort
gemessen der Schemafarbe **gleicht**; und die Bytegleichheit von
`wechsel-2` und `wechsel-3` ist **der Nachweis von AK 7** — nach einem
Schemawechsel bleibt die Themefarbe —, nicht sein Verstoß.

## 6. DoD-Prüfung

**Takt 1, vor der Abnahme.** Geführt vom Scrum Master am 05.08.2026 gegen den
Stand `849a6a1` auf `main`. **Gemessen, nicht übernommen:** Bau, Testlauf, beide
Linterläufe, die Prüfsummen der Bildbelege, der installierte Stand, der
Doku-Abgleich und die Heilung des UI-Befunds P2 sind eigene Läufe. Die Berichte
der Stränge und der Vollzugsvermerk des PO dienen als Anspruch, gegen den
gemessen wurde, nicht als Beleg.

### 6.1 Eigene Messungen

| Gegenstand | Lauf | Ergebnis |
|---|---|---|
| Bau warnungsfrei | frischer Bauplatz außerhalb des Repositoriums, `cmake -DCMAKE_BUILD_TYPE=Debug` + `--build -j12` | **0 Warnungen** in 139 Zeilen Bauausgabe (`grep -ci warning` = 0). Nicht in `build/` gemessen — der gehört den Strängen |
| Tests auf Ganymed | `ctest` auf demselben Bauplatz | **9 von 9** bestanden, 6,87 s. Darunter die beiden neuen aus #61 (`identitytest`, `commandlinetest`) |
| `lint-tidy` | `cmake --build … --target lint-tidy` | **rc=0, 0 Warnungen, 0 Fehler** — und die Zeile, ohne die eine Null nichts sagt: `for 30 files out of 49` |
| `lint-clazy` | dito | **rc=0, 0 Warnungen, 0 Fehler** |
| `NOLINT`-Zählung (#76 AK 4) | `git grep -cE '^\s*// NOLINT' -- src tests` | **37 Marken in 12 Dateien**, davon **29** `misc-const-correctness` und **8** `bugprone-easily-swappable-parameters`. Deckungsgleich mit Bericht und karpathy-Nachzählung |
| Eine Versionsquelle (#61 AK 3) | `git grep -F 0.1.0 -- src tests` | **0 Treffer**. Einzige Fundstelle `CMakeLists.txt:3` |
| Versionsanzeige (#61 AK 1, 6, 7) | gebauter Stand des eigenen Bauplatzes | `denkzettel 0.1.0`, Rückgabe 0 — auch mit `env -u DBUS_SESSION_BUS_ADDRESS`. `--kennt-keiner` und `--desktopfile` je „Unbekannte Option", zurückgewiesen |
| Automatischer Lauf (B18) | `gh run list --commit 849a6a1` | **`completed success`** — am Lauf des eigenen Commits, nicht am obersten der Liste. Zur Prüfzeit des karpathy-Reviews stand er noch auf `in_progress`; das ist inzwischen nachgeschlagen |
| **Installierter Stand** | `readlink /proc/$(pgrep -x denkzetteld)/exe`, `stat` | `/usr/bin/denkzetteld`, **kein `(deleted)`** — und die Datei trägt den **04.08.2026, 16:05** bei 7.913.024 Bytes gegen den gebauten Stand vom 05.08.2026, 22:18 bei 8.355.456 Bytes. **Der laufende Dienst ist der Stand von Sprint 6** (M1) |
| UI-Befund P2 gegen die Quelle | `[ContrastEffect]`-Gruppe in `/usr/share/plasma/desktoptheme/*/metadata.desktop` | **Die Heilung trifft.** Genau `cachyos-emerald`, `cachyos-emerald-color` und `Iridescent-round` tragen die Gruppe; `cachyos-emerald-light` trägt keine; `default`, `breeze-dark` und `breeze-light` haben gar keine `metadata.desktop`. SPEC 3.2 Punkt 10 nennt jetzt diese drei und führt `cachyos-emerald-light` als eigenen Fall |

### 6.2 DoD 1–4 je Story

| Story | DoD 1 Bau/Tests/Linter | DoD 2 AK + installierter Stand | DoD 3 Reviews | DoD 4 SPEC |
|---|---|---|---|---|
| **#85** | **erfüllt** | **nicht geführt** — AK 1–7 am gebauten Stand belegt, Hauptweg am **installierten** Stand nicht ausgeführt (M1) | **erfüllt** — der eine `fail` des UI-Reviews (P2) ist in SPEC 3.2 Punkt 10 geheilt, und die Heilung ist oben **gegen die Themedateien selbst** nachgeprüft, nicht gegen die Meldung; der zweite `fail` (Punkt 7, Zeichnung) liegt in der Fläche des UI/UX und ist dort als V1 geheilt. karpathy ohne `fail` | **erfüllt** — 3.1 (Herkunftsregel, dritte Stelle der gedämpften Klasse, Textcursor, „Herkunft statt Kontrastzahl") und 3.2 Punkte 10–12 |
| **#61** | **erfüllt** | **nicht geführt, und schwerer als bei den anderen** — AK 4 und AK 5 verlangen den Nachweis wörtlich „am **laufenden, installierten** Stand, nicht im Testlauf". Beide sind offen (M1, M2) | **erfüllt** — keine UI-Story (Planning §2); karpathy ohne `fail` | **erfüllt** — SPEC 15.1 neu am Ende von 15, 2.3 und 2.4 je um zwei entdeckte Bedingungen ergänzt |
| **#76** | **erfüllt, und die Schwelle selbst ist Teil davon** — DoD 1 nennt seit dieser Story die Dreizahl; meine Läufe treffen sie | **nicht geführt** — dieselbe Ursache (M1). Die Selbst-Sichtprüfung am gebauten Stand (Bericht §7) ist geführt und vom PO getaktet | **erfüllt** — keine UI-Story; karpathy ohne `fail`, K3 vom PO teils widerlegt und im Übrigen nachgetragen | **erfüllt, geprüft statt unterstellt** — die Streichung von `KGlobalAccel::setGlobalShortcut` aus `.clang-tidy` widerspricht keiner SPEC-Festlegung: **SPEC 2.4 trägt die Bedingung bereits seit dem 01.08.2026** („kann einen Fehlschlag des Dienstes nicht melden … deshalb fragt Denkzettel nach jeder Registrierung beim Dienst nach"). Die Linterschwelle ist eine Prozess-Festlegung und steht in DoD 1 (AK 7). Kein SPEC-Nachzug nötig |

**Zu DoD 2, damit die Zeile nicht milder gelesen wird als sie ist:** Die
Akzeptanzkriterien sind je Story einzeln belegt, und die Belege tragen. DoD 2
verlangt den Hauptweg aber **am installierten Stand**, und Takt 1 Punkt 1
verlangt ihn für **jede** Story. Beides ist nicht geschehen — für die drei
Stories dieses Sprints so wenig wie für die vier aus Sprint 7. Bei #61 kommt
hinzu, dass zwei Akzeptanzkriterien den installierten Stand **selbst** verlangen;
dort ist nicht nur der Prüfschritt offen, sondern das Kriterium unerfüllt.

### 6.3 Doku-Abgleich (B10 in der Fassung nach B17)

Geführt mit dem Griff aus `CLAUDE.md` in seiner **Ausschlussform** (seit
05.08.2026), einmal für die Linterschwelle und einmal für die Versionsanzeige:

```
git grep -n -i "clang-tidy\|clazy\|Linterbefund\|Linterschwelle\|lint-tidy\|lint-clazy" \
    -- . ':!docs/scrum/reviews' ':!docs/scrum/sprints' ':!docs/scrum/retro' \
       ':!docs/scrum/vorberichte' ':!src' ':!tests'
```

**Der Griff hat sich bewährt:** Er findet `.clang-tidy`, `ci.yml`, `CLAUDE.md`,
`README.md`, beide `CMakeLists.txt`, `PROZESS.md` und
`.claude/agents/denkzettel-dev.md` in einem Zug — also genau die Stellen, an
denen die Aufzählungsform in Sprint 6 und Sprint 7 je eine übersehen hat.
**Das Ergebnis steht hier auch dort, wo nichts zu melden war.**

| Ort | Befund |
|---|---|
| `.github/workflows/ci.yml`, Kopf und beide Linterschritte | **in Ordnung.** Der Kopf beschreibt die drei nicht erreichten DoD-Punkte weiterhin richtig; der clazy-Schritt steht auf 0, der `clang-tidy`-Schritt ist neu, beide prüfen die Dreizahl und begründen sie im Kommentar. `clang` steht in der Paketliste — ohne es fände CMake `run-clang-tidy` nicht |
| `CMakeLists.txt`, `src/`, `tests/` | **in Ordnung.** Die neuen Zeilen aus #61 (`DENKZETTEL_VERSION` als Übersetzungsdefinition) und die `XDG_DATA_DIRS`-Zeile für `commandlinetest` tragen je ihre Begründung im Kommentar |
| `.clang-tidy` | **in Ordnung und mustergültig.** Beide neuen Regeln stehen als Kommentar über der `Checks`-Zeile, die Streichung ist an ihrer Fundstelle begründet und datiert — das ist genau die Bedingung aus #76 AK 5 |
| `CLAUDE.md:191, :204` | **in Ordnung.** Die Zeile über den automatischen Lauf nennt jetzt „**jedem Linterbefund**" und die Schwelle null für beide Linter |
| `CLAUDE.md:106–141`, B17-Griff | **in Ordnung.** Der Griff schließt aus statt aufzuzählen; der Sprint-7-Mangel M5 ist damit behoben, und die Herleitung hält beide Fehlschläge fest |
| `README.md:161–167` | **in Ordnung.** „jedem Baufehler, jeder Compiler-Warnung, jedem roten Test und **jedem Linterbefund**", dazu die Null seit dem 05.08.2026 |
| `README.md:140–143` | **Befund M7** — „stehen auf **null Befunden**" nennt die Schalterstellung nicht. Das Projekt hat in **diesem** Sprint entschieden, dass eine Null ohne sie keine Aussage ist (#76 AK 1, Bericht §6: mit `-DDENKZETTEL_SPIKE_SPELLFIX=ON` sind es 3). DoD 1 nennt sie, die öffentliche Datei nicht |
| `README.md:81–96`, „Bedienung" | **Befund M6** — `denkzetteld --version` und `--help` sind mit #61 hinzugekommen und stehen nirgends in der README. Der Abschnitt führt Tastenkürzel und zwei D-Bus-Aufrufe; die Kommandozeile fehlt |
| `README.md:126–127`, Determinismus | **in Ordnung — und der gegenteilige Befund des #76-Berichts trifft nicht** (M8). Der Satz steht unmittelbar hinter dem `readmeshots`-Befehl und sagt „**Der** Läufer"; er gilt `readmeshots`, und den hat #76 selbst mit **2 von 2 bytegleich** gemessen. Die gemessene Unstetigkeit liegt bei `libraryshots`, über den die README nichts behauptet |
| `README.md:49–52`, Statuszeile | **existiert in dieser README nicht** — dieselbe Feststellung wie in Sprint 6 §19.7 und Sprint 7 §7.3. Der Absatz „Auf der Liste stehen noch" nennt keinen Sprint. **Durch die Abnahme wird daran nichts falsch** |
| `README.md:56–65`, Abhängigkeiten | **in Ordnung, geprüft statt unterstellt.** #61 zieht nichts Neues nach — `KCoreAddons` (für `KAboutData`) und `KDBusAddons` stehen bereits in der Liste; #85 liest die `colors`-Datei mit `KConfigCore` und zeichnet mit `KSvg`, beide ebenfalls schon dort. Die Liste wächst nicht. Das ist die Stelle, an der Sprint-6-Mangel M4 saß |
| **README-Bilder** | **Befund M11, Mangel bleibt offen.** Eigene Nachmessung an `docs/bilder/erfassungsfenster.png`: 1200×324, Farbmodus **RGB ohne Alphakanal**, Fläche (20, 22, 24) deckend — ein Fenster ohne Theme-Hülle, der Stand vor #55. Die Bindung liegt seit Sprint 7 bei **#96** und damit außerhalb dieses Sprints |
| `docs/scrum/PROZESS.md`, DoD 1 und „Automatische Testläufe" | **in Ordnung.** Beide Stellen nennen die Null, die Dreizahl und ihre zwei gemessenen Gründe |
| `docs/scrum/PROZESS.md`, Abschluss-Punkt 10 | **Befund M9** — der Punkt ist „**bis #61 umgesetzt ist**" ausgesetzt. #61 ist geliefert; der Satz ist damit nicht falsch, aber niemand kann ihm ansehen, dass die Bedingung umgeschlagen ist. Dasselbe für `CHANGELOG.md:8` und `:13` („sichtbar in der Anwendung mit #61", „die Versionsnummer folgt mit #61"). Der Strang B hat es gemeldet (Bericht §7); vollzogen ist es nicht |
| `.claude/agents/denkzettel-dev.md` | **in Ordnung.** Fall 10 der Liste „Rückgabewerte und Läufe, die nichts belegen" trägt den zsh-Fund aus #76 §15. Der Vorschlag aus dem #61-Bericht §6 (unerreichbarer Sitzungsbus) ist **nicht** aufgenommen — der Strang hat ihn ausdrücklich dem PO überlassen und die Datei nicht angefasst; das ist richtig gearbeitet, der Eintrag steht aber aus (siehe next) |

### 6.4 Prüfsummen der Bildbelege

Eigener Lauf über die Belegordner dieses Sprints: **Rückgabe 1** (UI-Review,
drei Gruppen), **Rückgabe 0** (#85). Gleiche Zahl wie beim PO. Die drei Gruppen
habe ich nicht übernommen, sondern gegen `messungen/ux-m3-wechsel.txt` und die
Prüfsummen selbst gelesen:

| Gruppe | Urteil | Grund |
|---|---|---|
| `fenster-breeze-dark-weiss-leer` = `fenster-default-weiss-leer` | **kein Mangel** | Derselbe Zustand. `breeze-dark` bringt eine `colors`-Datei mit, deren Werte denen des eingestellten Schemas gleichen (252,252,252 / 161,169,177) — die Gleichheit ist gemessen und in P4 des Strangs über drei Schemata von „folgt dem Schema" unterschieden |
| `fenster-breeze-dark-weiss` = `fenster-default-weiss` = **`wechsel-1-default`** | **kein Mangel — und die Gruppe hat drei Mitglieder, nicht zwei** | Der PO nennt in §5a nur die ersten beiden. Das dritte Bild stammt aus einer **anderen Sonde** (`wechselbeleg` statt `sitzungsbeleg`) und zeigt denselben Zustand. Byteidentität über zwei getrennte Prüfmittel ist hier kein Duplikat, sondern der stärkere Befund |
| `wechsel-2` = `wechsel-3` | **kein Mangel, das Urteil des PO hält — und es ist nachprüfbar** | Zwischen beiden liegt der Schemawechsel auf Magenta; `ux-m3` schreibt „Schema sagt: 255,0,255", das Fenster zeigt weiter 35,38,41 / 112,125,138. Das ist AK 7. **Gegenprobe, damit die Byteidentität nicht alles erklärt:** `wechsel-1` und `wechsel-4` tragen dieselben Farbwerte und sind **nicht** bytegleich — zwischen ihnen liegt `wechsel-5` mit gesetzter Auswahl. Die Wache unterscheidet also noch |

**Stop-Bedingung nach PROZESS.md:** Dieser Lauf hat neue Gruppen gefunden, die
Prüfung bleibt Vollprüfung.

### 6.5 Vollzähligkeit der Prüfberichte (Takt 1, Punkt 2)

**Fünf Prüfläufe, fünf Berichte als Datei**, alle versioniert und alle vor
dieser Prüfung abgelegt: `sprint-08-s85-lesbarkeit/bericht.md`,
`sprint-08-s61-versionsanzeige/bericht.md`,
`sprint-08-s76-linterbefunde/bericht.md`, `sprint-08-karpathy.md`,
`sprint-08-ui-review/bericht.md`.

Gegenprobe nach dem vorgeschriebenen Weg — alle 23 Commit-Botschaften des
Sprint-Diffs nach Nennungen von Prüfläufen durchsucht und gegen die abgelegten
Berichte gehalten: karpathy-Lauf, UI-Review, die drei Übergabeberichte, beide
Bildvergleiche und die Wachenprobe haben je ihren Bericht oder ihre Messausgabe
im Repo. **Ein Lauf ohne Bericht ist nicht aufgefallen.**
`git status --untracked-files=all` über `docs/`, `wireframes/` und `SPEC.md` ist
leer — der Sprint-7-Mangel M8 (unversionierte Berichtszeile) wiederholt sich
nicht.

Eine Beobachtung am karpathy-Bericht selbst: Sein Prüfgegenstand ist
`f8b7711` mit **138 Dateien**; der Sprint-Diff umfasst heute **139 Dateien** bis
`849a6a1`. Der Unterschied ist der Bericht selbst und der Commit, der seine
Befunde K1 und K2 heilt — und dieser Commit ändert ein **Prüfgut**
(`tests/themes/…/denkzettel-test-breit/colors`). **Befund M5.** Er ist die
Statusspalte nachgeführt, mit angehängtem Vollzugsvermerk statt durch
Überschreiben — der Sprint-7-Mangel M7 wiederholt sich also nicht.

### 6.6 Sprint-Konto (B12)

| Zeitpunkt | Stand | Anlass |
|---|---|---|
| Freigabe 05.08.2026 | 3 Issues · 3×m | Zuschnitt §2 |
| Sprint-Ende 05.08.2026 | **3 Issues · 3×m** | **kein Zugang** |

Nachgemessen am Milestone: `Sprint 8` trägt genau **#85, #61, #76**, alle drei
mit `size:m`. Die vier im Sprint entstandenen Issues **#96, #97, #98, #99**
tragen **keinen Milestone** und sind offen im Backlog. Beide Grenzen sind
gehalten, und anders als in Sprint 7 mit Luft: drei von höchstens vier Issues.

### 6.7 Der Stand der Sprint-7-Mängel

Nachgemessen, nicht aus dem Protokoll übernommen.

| Sprint-7 | Stand heute | Beleg |
|---|---|---|
| **M1** Installation | **offen** — als M1 fortgeführt | `stat /usr/bin/denkzetteld` = 04.08.2026, 16:05 |
| **M2** zwei `fail` im UI-Review (W1, W2) | **behoben und nachgeprüft** | `9f2063a` „Zeichnung 4a/4b geheilt und nachgeprueft"; die vier Nachprüfbilder `ux-nachpruefung-w1…w4` liegen im Sprint-7-Belegordner, und der Sprint-8-UI-Review hat W1–W4 erneut gegen den neuen Stand gehalten |
| **M3** README-Bilder | **offen**, jetzt an #96 gebunden — als M11 fortgeführt | eigene Bildmessung, §6.3 |
| **M4** README „geschätzte Stories" | **behoben** | `git grep "geschätzt" README.md` ist leer; :153–155 beschreibt jetzt Vorprüfbericht und Größenklasse |
| **M5** B17-Griff ohne `.claude/` | **behoben, und besser als gefordert** | `CLAUDE.md:109` schließt aus statt aufzuzählen |
| **M6** SPEC nennt „K5" statt K4 | **behoben** | `SPEC.md:325` nennt „karpathy-Befund K4 zu Sprint 7" |
| **M7** karpathy-Bericht nicht nachgeführt | **behoben** | `sprint-07-karpathy.md:120–135`, angehängter Vollzugsvermerk mit allen sieben Befunden |
| **M8** unversionierte Zeile im UI-Bericht | **behoben** | P5 steht in der versionierten Fassung, nichts untracked |
| **M9** `build-vor85/` im Repository | **behoben** | `git ls-files` führt keine Datei darunter mehr; `.gitignore:21` deckt jetzt `build-*/` |
| **M10** sieben Commits ungepusht | **behoben** | `git rev-list --count origin/main..main` = 0 |

**Eine Berichtigung am Sprint-7-Protokoll selbst, gefunden beim Nachmessen von
M3.** §7.3 dort schreibt: „`docs/bilder/erfassungsfenster.png` ist 600×178
Bildpunkte, die Fläche (239, 240, 241) deckend, und außer dem Eckpunkt (0,0) ist
keine Zeile durchscheinend". Die versionierte Datei war zum Sprint-7-Prüfstand
`268a7c5` bereits **1200×324 im Farbmodus RGB** — ohne Alphakanal, den man
prüfen könnte, und mit einer **dunklen** Fläche (20, 22, 24). Gemessen wurde
dort offenkundig das im §6a-Nachtrag verworfene **neu erzeugte** Bild, nicht das
committete. **Der Schluss stimmt trotzdem** — die committete Datei zeigt ein
Fenster ohne Hülle und damit den Stand vor #55 —, seine Zahlen gehören aber
berichtigt. **Befund M10.**

---

## 7. Mängelliste

Melden, nicht heilen. Die Behebung liegt beim PO.

| # | Mangel | DoD/Regel | Gewicht |
|---|---|---|---|
| **M1** | **Der Endstand ist nicht nach `/usr` installiert, und der Hauptweg keiner der sieben Stories beider Sprints ist am installierten Stand ausgeführt.** Gemessen: `/usr/bin/denkzetteld` trägt den 04.08.2026, 16:05 bei 7.913.024 Bytes gegen den gebauten Stand vom 05.08.2026, 22:18 bei 8.355.456 Bytes. `readlink` zeigt `/usr/bin/denkzetteld` ohne `(deleted)` — der laufende Prozess **ist** der installierte, nur ist der installierte der von **Sprint 6**. Die Lage ist eine protokollierte PO-Entscheidung (K8), kein Versäumnis; **erledigt ist der Punkt dadurch nicht** | **DoD 2**, Takt 1 Punkt 1 | **schwer** |
| **M2** | **Bei #61 sind zwei Akzeptanzkriterien unerfüllt, nicht bloß ungeprüft.** AK 4 und AK 5 verlangen den Nachweis wörtlich „nachgewiesen am **laufenden, installierten** Stand, nicht im Testlauf" — die Story ist genau um diese Bedingung herum geschnitten worden, weil kein Rückgabewert den Bruch meldet. Der Strang führt beide selbst als offen. Das unterscheidet #61 von den übrigen sechs Stories, bei denen allein der Prüfschritt aussteht | DoD 2 | **schwer** |
| **M3** | **Die Befehlsliste für die Installation deckt nur eine der sieben Stories.** `sprint-08-s61-versionsanzeige/bericht.md` §4 ist sorgfältig gearbeitet und am laufenden Dienst nachgeprüft — sie führt aber Busname, Anwendungs-Id, Kürzel, Journal und `--version` und damit den Hauptweg von **#61**. Nicht abgedeckt: **#71, #70, #72** (Bibliothek — kein `ShowLibrary`, kein Tastaturweg, kein Tooltip), **#85** (Erfassungsfenster unter einem Theme mit eigener `colors`-Datei) und **#83** (Hülle, Rundung, Kontur, Schatten — nach B21 ein Bild aus der angemeldeten Sitzung). Für **#76** trägt sie mittelbar, weil Kürzel und `ShowLibrary` genau die zwei Stellen berühren, an denen „nur Aufräumen" aufhört, eines zu sein. Takt 1 Punkt 1 verlangt den Hauptweg **jeder** Story mit Belegform | Takt 1 Punkt 1 | **schwer** |
| **M4** | **Sechs Datumsangaben „06.08.2026" in versionierten Belegen, geschrieben am 05.08.2026 um 22:35.** Fundstellen: `sprint-08-s85-lesbarkeit/bericht.md` (fünfmal, darunter beide Berichtigungsüberschriften zu K1 und K2) und `mutationsproben.sh:108`. Commit `849a6a1` trägt `2026-08-05 22:35:03`. B17 lebt davon, dass eine angehängte Zeile den **Stand** richtig datiert; eine Berichtigung, die sich einen Tag vordatiert, verkehrt die Reihenfolge zweier Belege, sobald jemand sie nebeneinanderlegt. Nachzuziehen ist das Datum, nicht der Text | B17, reale Zeitstempel | mittel |
| **M5** | **Der karpathy-Review deckt den letzten Commit des Sprints nicht.** Sein Prüfgegenstand ist `f8b7711` mit 138 Dateien; der Sprint-Diff reicht bis `849a6a1` mit 139. Der Unterschied ist nicht nur der Bericht selbst: `849a6a1` heilt K1, indem es **ein Prüfgut ändert** (`tests/themes/…/denkzettel-test-breit/colors` bekommt eine `[Colors:View]`-Gruppe). Dass daran nichts hängt, ist gemessen — `ctest` bleibt 9/9, und der Strang hat die Hülle auf Byteidentität geprüft —, aber der Prüfsatz „karpathy über den Sprint-Diff" trägt für diesen Commit nicht. Entweder ein kurzer Nachlauf oder eine datierte Zeile am Bericht, die den Unterschied benennt | DoD 3, Takt 1 Punkt 2 | leicht |
| **M6** | **Die README nennt die Kommandozeile nicht.** `denkzetteld --version` und `--help` sind mit #61 hinzugekommen und stehen weder unter „Bedienung" noch sonstwo. Der Abschnitt führt Tastenkürzel und zwei D-Bus-Aufrufe — die neue, für einen Nutzer sichtbarste Zeile dieses Sprints fehlt | B10 | leicht |
| **M7** | **`README.md:140` nennt die Linter-Null ohne Schalterstellung.** Dieser Sprint hat selbst entschieden, dass „0" ohne `-DDENKZETTEL_SPIKE_SPELLFIX=OFF` keine Aussage ist (#76 AK 1; mit `ON` sind es gemessen 3). DoD 1 nennt die Bedingung, die öffentliche Datei nicht — dieselbe Lücke, die AK 7 gerade für `ci.yml` geschlossen hat, eine Datei weiter | B10, DoD 4/B9 | leicht |
| **M8** | **`sprint-08-s76-linterbefunde/bericht.md` §14 wirft der README einen Widerspruch vor, den sie nicht enthält.** Der Satz „Der Läufer arbeitet deterministisch" steht unmittelbar hinter dem `readmeshots`-Befehl und gilt `readmeshots` — den der Bericht selbst mit **2 von 2 bytegleich** misst. Die gemessene Unstetigkeit liegt bei `libraryshots`, über den die README nichts behauptet. **Warum das mehr ist als eine Formulierung:** Der nächste, der den Befund abarbeitet, ändert einen Satz, der stimmt, und die Aussage wird dabei schwächer statt richtiger | B7 | leicht |
| **M9** | **Der Aussetzungsgrund für Abschluss-Punkt 10 ist entfallen, und niemand hat es vermerkt.** `PROZESS.md` setzt Version und Tag aus „bis #61 umgesetzt ist"; #61 ist geliefert. Dasselbe an `CHANGELOG.md:8` und `:13` („sichtbar in der Anwendung mit #61", „die Versionsnummer folgt mit #61") — beide Sätze werden mit der Abnahme falsch. Strang B hat es gemeldet (Bericht §7) und richtig nicht selbst angefasst | B10 | leicht |
| **M10** | **`sprint-07.md` §7.3 belegt den README-Bild-Mangel mit den Zahlen eines anderen Bildes.** Dort steht „600×178 …, die Fläche (239, 240, 241) deckend, und außer dem Eckpunkt (0,0) ist keine Zeile durchscheinend"; die versionierte Datei war schon am Prüfstand `268a7c5` **1200×324 im Farbmodus RGB** — ohne Alphakanal — mit der Fläche (20, 22, 24). Gemessen wurde das im §6a-Nachtrag verworfene **neu erzeugte** Bild. Der Schluss bleibt richtig, sein Beleg ist ein anderer als angegeben — dieselbe Bauart wie die Berichtigung, die der Sprint-7-Scrum-Master seinerseits am PO vorgenommen hat | B7 | leicht |
| **M11** | **Die README beschreibt den gelieferten Stand weiter nicht** (Sprint-7-Mangel M3, fortgeführt). Eigene Messung: `docs/bilder/erfassungsfenster.png` ist RGB ohne Alphakanal, Fläche deckend — ein Fenster ohne Theme-Hülle, Stand vor #55. Die Ursache ist seit Sprint 7 gemessen und als **#96** gebucht; die Heilung liegt außerhalb dieses Sprints. **Bewusst getragen, nicht übersehen** | B10 | mittel |
| **M12** | **#91 beschreibt weniger, als gemessen ist.** Das Issue führt „Szene 10c/10d"; #76 hat über fünf Läufe desselben Binärcodes **drei** unstete Bilder gefunden — dazu `02-leerzustand.png`. Ein Issue, das enger gefasst ist als die Messung, schließt sich zu früh | B7 | leicht |

**Was aus M1, M2 und M3 für die Abnahme folgt, ausdrücklich:** Beide Sprints
sind **nicht abnahmefähig**, solange Takt 1 Punkt 1 offen ist — er ist der erste
Punkt der Liste, und DoD 2 steht für alle sieben Stories aus. Eine Abnahme jetzt
wäre die Abnahme des **gebauten** Standes; genau diese Verwechslung ist der
Sprint-3-Mangel M1, um dessentwillen die Regel in `CLAUDE.md` steht. **Die
Entscheidung K8 ändert daran nichts** — sie verlegt die Installation ans Ende
von Sprint 8, sie erlässt sie nicht. Was jetzt fehlt, ist zweierlei: die
Installation selbst (ein Schritt, der das Kundenpasswort braucht) und **eine
Befehlsliste, die alle sieben Hauptwege führt**. Die vorliegende deckt einen.
Bei #61 kommt hinzu, dass zwei Kriterien ohne diesen Schritt gar nicht erfüllbar
sind — sie sind offen, nicht ungeprüft. Was nicht geht, ist die Punkte still
abzuhaken.

### Empfehlung zu #99 — ja, es ist eine Prozessfolge zu ziehen

Der Befund liegt außerhalb dieses Sprints und ist unabhängig nachgemessen; er
ist als #99 gebucht, und ihn zu heilen ist nicht Sprintarbeit. Zur Frage, ob das
Verfahren daraus etwas lernen muss, urteile ich: **ja, und zwar an einer sehr
kleinen Stelle.**

**Der tragende Satz steht in diesem Sprint schon geschrieben, eine Ebene
tiefer.** #76 hat entschieden, dass „`lint-tidy` meldet 0" ohne genannte
Schalterstellung **keine Aussage** ist — und die Bedingung dafür in DoD 1
verankert. „`ctest` meldet 9 von 9" ohne genannten **Bautyp** ist derselbe Satz.
Er war acht Sprints lang unbemerkt richtig, weil alle denselben Bautyp fuhren.

Nachgemessen, damit die Empfehlung nicht auf einer Vermutung ruht:

| Weg | Bautyp | Flaggen |
|---|---|---|
| DoD 1, `CLAUDE.md`, `ci.yml` | ausdrücklich `Debug` | — |
| README-Rezept `cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr` **im Git-Baum** | ECM setzt **`Debug`** | — |
| dasselbe Rezept aus einem **Tarball ohne `.git`** | **leer** | `CMAKE_CXX_FLAGS` leer — weder `-O` noch `NDEBUG` |
| `Release` / `RelWithDebInfo` | — | `-O3 -DNDEBUG` bzw. `-O2 -g -DNDEBUG` |

**Das schärft den Befund und entschärft ihn zugleich:** Kein Weg, den dieses
Projekt heute geht oder in der README beschreibt, erreicht `-O2` — auch der
Tarball-Bau nicht. Getroffen wird, wer ein Paket baut, denn Paketrezepte setzen
den Bautyp selbst. „Den Kunden trifft es heute nicht, jedes Paket aber schon"
stimmt also, und der Grund ist der Bautyp, nicht das Betriebssystem.

**Zwei Änderungen, beide sofort umsetzbar, beide beim PO:**

1. **`ci.yml` bekommt einen zweiten Bau- und Testlauf mit `Release`.** Das ist
   die Stelle, an der es nichts kostet: Der Lauf besteht bereits, die Zeile ist
   eine Matrix-Erweiterung, und **sie verlangt von niemandem Disziplin** — genau
   das Argument, an dem der PR-Probelauf gescheitert und der automatische Lauf
   eingeführt worden ist. Ohne sie fällt #99 beim nächsten Mal wieder erst
   jemandem auf, der aus einem ganz anderen Grund `-O2` baut.
2. **DoD 1 nennt den Bautyp**, so wie sie seit #76 die Linterschwelle nennt.
   Ein Satz genügt: Die Zusicherung „alle Tests grün" gilt für den Bautyp, in dem
   gemessen wurde, und das ist `Debug`.

**Was ich ausdrücklich *nicht* empfehle:** DoD 1 um einen zweiten Prüflauf von
Hand zu erweitern. Das wäre eine Pflicht, die je Story Zeit kostet und die ein
Container umsonst erledigt — und dieses Projekt hat zweimal gemessen, dass eine
Pflicht ohne Wirkungsprüfung folgenlos bleibt.

**Eine Grenze der Empfehlung, damit sie nicht mehr behauptet, als gemessen ist:**
Ob der Ausfall an der Optimierung oder an `NDEBUG` hängt, ist offen — beide
gefallenen Bautypen tragen beides. Ein `Debug`-Bau mit `-DNDEBUG` trennte die
zwei Ursachen in einem Lauf; das gehört zu #99 und nicht hierher. **Die
Empfehlung ändert sich durch die Antwort nicht:** Ein Release-Lauf im Container
fängt beide Fälle.

---

## 8. done/next

**done**

- **Drei Stories gebaut und belegt**, drei Stränge, alle mit `--no-ff`
  zusammengeführt. Eigenständig nachgemessen: Bau **warnungsfrei** (frischer
  Bauplatz außerhalb des Repositoriums), `ctest` **9 von 9**, `lint-tidy`
  **rc=0 / 0 / 0** über 30 von 49 Dateien, `lint-clazy` **rc=0 / 0 / 0**.
- **DoD 1, DoD 3 und DoD 4 sind für alle drei Stories erfüllt.** DoD 2 ist für
  alle drei offen, für #61 zusätzlich als unerfülltes Kriterium.
- **Der eine `fail` des UI-Reviews ist geheilt und die Heilung nachgeprüft** —
  nicht gegen die Meldung, sondern gegen die `metadata.desktop`-Dateien der acht
  Themes: die `[ContrastEffect]`-Gruppe tragen genau `cachyos-emerald`,
  `cachyos-emerald-color` und `Iridescent-round`.
- **Fünf Prüfläufe, fünf abgelegte Berichte**, keiner ohne Datei, nichts
  unversioniert.
- **Prüfsummen der Bildbelege gelaufen und einzeln beurteilt** — drei Gruppen,
  kein Mangel; eine Gruppe hat ein Mitglied mehr, als das Protokoll nennt, und
  die Gegenprobe `wechsel-1` ≠ `wechsel-4` zeigt, dass die Wache noch trennt.
- **Sprint-Konto gehalten:** drei Issues, `3×m`, **kein Zugang** nach der
  Freigabe. Die vier neuen Issues #96–#99 sind ohne Milestone im Backlog.
- **Neun der zehn Sprint-7-Mängel sind behoben**, jeder einzeln nachgemessen.
  Offen bleibt M1 (Installation) und, an #96 gebunden, der README-Bild-Mangel.
- **Der wertvollste Fund des Sprints stand in keinem Kriterium** und ist als
  #99 gebucht: Im Release-Bau fallen zwei Prüfsätze, im Debug nicht.

**next — in dieser Reihenfolge**

1. **M3 vor M1: die Befehlsliste auf sieben Hauptwege erweitern.** Die
   vorliegende deckt #61. Es fehlen die Bibliothekswege (#71, #70, #72), das
   Erfassungsfenster unter einem Theme mit eigener `colors`-Datei (#85) und ein
   Sitzungsbild für die Hülle (#83, B21). Die Reihenfolge ist nicht kosmetisch:
   Solange der Kunde am Rechner ist, läuft das Zeitfenster für die Installation,
   und eine Liste, die erst dort entsteht, kostet es.
2. **M1: installieren**, sobald der Kunde am Rechner ist — vorher `denkzetteld`
   beenden und neu starten, den `readlink`-Beleg ablegen (B16), dann die Liste
   aus Punkt 1 abarbeiten und je Story die Belegform ablegen (Terminalausgabe,
   Journalauszug oder Bild). **M2 erledigt sich in demselben Zug**, wenn die
   Punkte 3 bis 5 der bestehenden Liste laufen.
3. **M4** — die sechs Datumsangaben auf den 05.08.2026 setzen, Text unverändert.
   Vor der Abnahme, damit die Beweislage nicht mit einem falschen Datum
   eingefroren wird.
4. **M5, M8, M9, M10, M12** — Nachträge an karpathy-Bericht, #76-Bericht,
   `PROZESS.md`, `CHANGELOG.md`, `sprint-07.md` und #91; je datiert und
   anhängend, nicht überschreibend.
5. **M6, M7** — die beiden README-Zeilen. **M11** bleibt an #96 und gehört nicht
   in diesen Sprint.
6. **Die Empfehlung zu #99 entscheiden** (Release-Lauf in `ci.yml`, Bautyp in
   DoD 1). Beides ist Prozessarbeit und braucht keine Story.
7. **Den Fall aus dem #61-Bericht §6 in `denkzettel-dev.md` aufnehmen** — „ein
   Prüfsatz gegen einen unerreichbaren Sitzungsbus kann Zurückweisung nicht von
   Absturz unterscheiden". Der Strang hat ihn ausdrücklich dem PO überlassen und
   die Agentendatei nicht angefasst; ohne den Eintrag ist der Fund nicht
   verankert.
8. **Dann erst die Abnahme** beider Sprints durch den PO (Rollenlage §1), danach
   Takt 2 für **beide** Milestones: AK-Haken, Issues und Milestones schließen,
   Journal, Push, Zweige und Worktrees räumen — es stehen **fünf** Zweige und
   fünf Worktrees offen (`story/83-native-huelle`, `story/71-ruhige-liste`,
   `story/85-lesbarkeit`, `story/61-versionsanzeige`, `tech/76-linterbefunde`),
   dazu der Zweig `sicherung-vor-bereinigung` —, Changelog.
   **Punkt 10 ist mit dieser Abnahme nicht mehr ausgesetzt** (M9): #61 ist
   geliefert, die Nummer erreicht die Anwendung, und die Abnahme löst den
   MINOR-Sprung und den Tag aus. Eine Abnahme für beide Sprints heißt **eine**
   Version.

---

## 9. Vollzug von Takt 2 — 05.08.2026

| Punkt | Stand |
|---|---|
| **5** Issues mit AK-Haken, Abnahmekommentar und Commit-Verweis geschlossen | **7 von 7** — 50 Haken gesetzt, **einer bewusst offen** (#61 AK 5: die Registrierung des Kürzels ist belegt, seine Wirkung nicht — kein Prozess löst unter Wayland einen Tastendruck aus) |
| **5** Milestone geschlossen | Sprint 7 und Sprint 8 |
| **6** Journal nachgeführt | Eintrag 22:59 im Daily |
| **7** `main` gepusht | ja |
| **8** Story-Zweige und Worktrees entfernt | fünf Worktrees, sechs Zweige; auf `origin` steht nur `main`. *`story/83-native-huelle` musste mit `-D` fallen: Die Historienbereinigung vom selben Tag hat seine Commits umgeschrieben, der Zweig zeigte auf die alten Kennungen. Die Arbeit steckt mit elf Commits in `main` — nachgeprüft, nicht angenommen* |
| **9** Changelog fortgeschrieben | Abschnitt **0.2.0**, aus Nutzersicht, mit dem ausdrücklichen Vermerk „keine Änderung am Datenbank-Schema" |
| **10** Version erhöht und getaggt | **0.2.0**, `v0.2.0` — **die erste Version dieses Projekts.** Die Aussetzung dieses Punktes endet mit #61 |

**Die Aussetzung von Punkt 10 ist beendet.** Sie stand seit dem 02.08.2026:
*„bis #61 umgesetzt ist … erreicht die Zahl die Anwendung nicht und wäre eine
Behauptung ohne Sichtbarkeit."* `denkzetteld --version` antwortet seit heute.
Die Abnahmen davor bekommen weiterhin keine Version rückwirkend.

## 10. done/next — Gesamtergebnis für den Kunden

**done**

- **Sieben Stories** über zwei Sprints, alle abgenommen. Bau warnungsfrei,
  `ctest` 9/9, beide Linter auf null.
- **Der Kundenbefund aus Sprint 6 ist geheilt.** Die Fläche des
  Erfassungsfensters ist mit KRunner und der Plasma-Benachrichtigung
  **byteidentisch**; Kante und Schattenverlauf ebenso.
- **v0.2.0** — die erste Fassung mit einer Nummer.
- **Vierzehn neue Issues** aus der Arbeit selbst (#86–#99). Der Sprint hat
  mehr Befunde hervorgebracht als er Stories hatte, und keiner davon ist in
  einen laufenden Sprint gezogen worden.

**next — was der Kunde entscheiden sollte**

1. **#99 zuerst.** Im Release-Bau gehen **Kategorien verloren**. Trifft ihn
   heute nicht (sein Dienst ist ein Debug-Bau), trifft jedes Paket.
2. **#97** und **#89** sind Blickurteile — zwei Bilder nebeneinander, in
   Sekunden beantwortet.
3. **Vier Hauptwege** stehen seinem Blick offen (`HAUPTWEGE.md`).
