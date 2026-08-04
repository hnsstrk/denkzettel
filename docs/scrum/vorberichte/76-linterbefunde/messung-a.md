# Vorprüfung #76 — Messung Bearbeiter A (`denkzettel-dev`)

**Gegenstand:** Issue #76, „T: Linterbefunde heilen — lint-tidy auf 0, clazy auf 0"
· **Datum:** 04.08.2026, Ganymed · **Quellstand:** `main` @ `6acc87e`
· **Belege:** `messungen/`, Sonden in `sonden/`, wiederholbar über
`bash docs/scrum/vorberichte/76-linterbefunde/pruefen.sh`

Dieser Bericht trägt die Felder **1, 2, 4 und 5**. **Feld 3 (Ready-Urteil) fällt
der Scrum Master**, Feld 6 steht als „Offene Fragen" am Ende.

**Stand der Werkzeuge** (B17 — eine Aussage gilt für einen Stand): clang 22.1.8,
clazy 1.17.1, cmake 4.1.2, qt6-base 6.11.1. Bauplatz dieser Vorprüfung:
`docs/scrum/vorberichte/76-linterbefunde/build/` (von `.gitignore` gedeckt).
`build/` der Repositoriumswurzel wurde nicht angefasst.

**Ich habe keinen einzigen Befund geheilt.** Die Heilungsproben liefen auf
Wegwerfkopien unter `$TMPDIR`, die aus `git archive HEAD` entstanden; der
Arbeitsbaum ist unverändert.

---

## Ist-Stand, gemessen (`messungen/01`–`04`)

| Lauf | rc | `grep -c 'warning:'` | eindeutige Befunde | `error:` |
|---|---|---|---|---|
| Bau (`cmake --build`) | 0 | **0** | — | 0 |
| `lint-clazy` | **0** | **3** | 3 | 0 |
| `lint-tidy` | **0** | **140** | **81** | 0 |

**Die 81 clang-tidy-Befunde verteilen sich auf 7 Prüfklassen und 20 Dateien:**

| Prüfung | eindeutig | Rohzeilen | mechanisch? |
|---|---|---|---|
| `misc-const-correctness` | 60 | 60 | 52 maschinell, 8 gar nicht (F4) |
| `bugprone-easily-swappable-parameters` | 8 | 8 | **nein — Signaturentscheidung** |
| `performance-enum-size` | **7** | **66** | ja, 7 Kopfzeilen |
| `bugprone-narrowing-conversions` | 3 | 3 | ja — **fallen mit #83 weg** (F7) |
| `performance-no-automatic-move` | 1 | 1 | ja, aber **gegenläufig** (F5) |
| `bugprone-unused-return-value` | 1 | 1 | **nein — PO-Entscheidung** (F6) |
| `bugprone-implicit-widening-of-multiplication-result` | 1 | 1 | **nein — Maschinenfix ist falsch** (F3) |

**Die drei clazy-Befunde** (`messungen/02-clazy.txt`), beide Fundorte
gegenüber dem Issue um 51 Zeilen verschoben:

- `tests/librarytest.cpp:2387` und `:2393` — `range-loop might detach Qt container (QList)`
  (`message->findChildren<QLabel *>()` bzw. `<QToolButton *>()` direkt im
  `for`-Kopf)
- `tests/shelltest.cpp:361` — `Don't call QList::first() on temporary`
  (`icon.item()->contextMenu()->actions().first()`)

---

## Feld 1 — Dateimenge (am Code vermessen, Notation nach B13)

| | **#76** — Linterbefunde heilen |
|---|---|
| **Issue** | **#76** (`epic:M7`, `typ:tech`) |
| **Zweig** | `story/76-linterbefunde` |
| **Quellen & Tests** | **20 von 45 versionierten Quelldateien**, gemessen an den Fundorten (`messungen/04-tidy-eindeutig.txt`).<br>**`src/` — 12 Dateien, 21 Befunde:** `main.cpp` (3), `capture/capturewindow.cpp` (3, Zeilen 71–73), `shell/globalshortcuts.cpp` (2), `shell/shortcutregistration.cpp` (2) und `.h` (1), `shell/trayicon.cpp` (2), `store/note.h` (2), `ui/librarywindow.cpp` (1) und `.h` (2), `ui/notelistdelegate.cpp` (1), `ui/notelistmodel.h` (1), `ui/timestampformat.h` (1).<br>**`tests/` — 8 Dateien, 60 Befunde:** `librarytest.cpp` (**42**), `shelltest.cpp` (7), `capturetest.cpp` (5), `editshots.cpp` (2), `captureshots.cpp` (1), `libraryshots.cpp` (1), `readmeshots.cpp` (1), `searchshots.cpp` (1). **Alle fünf Bildläufer sind darunter** — der Glob in `CMakeLists.txt:72` nimmt `tests/*.cpp` ohne Ausnahme. |
| **Build** | `.clang-tidy` — **falls** der PO den Eintrag `^::KGlobalAccel::setGlobalShortcut$` streicht (F6). Sonst nichts: die Linterziele bleiben, wie sie sind. |
| **Belege & Prüfmittel** | `.github/workflows/ci.yml` — zwei Eingriffe aus AK 3: clazy-Schwelle 3 → 0 **und** ein neuer `lint-tidy`-Schritt. Der Schritt existiert heute nicht; er ist ein **neuer Prüfweg** (Feld 5).<br>`docs/scrum/reviews/sprint-NN-t76-linterbefunde/` — neu anzulegen, mit `pruefen.sh` nach dem Muster dieses Ordners. |
| **Fachliche Quellen** | **Keine.** `git grep -n -i "clang-tidy\|clazy\|lint" -- SPEC.md docs/scrum/PROZESS.md` ist **leer**. Weder die SPEC noch die DoD kennen die Linter; DoD 1 verlangt „kompiliert warnungsarm" und sagt über Linter nichts. Die Sprint-5-Aussage „keine Linterbefunde aus diesem Sprint" hatte also gar keine Regel hinter sich. **Entdeckte Bedingung nach DoD 4 (Fassung B9): wenn diese Story ein Tor auf null setzt, gehört das Tor in die DoD** — sonst steht die Schwelle allein in einer YAML-Datei. |
| **Ausdrücklich nicht** | `src/store/store.cpp`, `src/store/library.cpp`, `src/ui/*` außer den vier oben, `tests/storetest.cpp`, `tests/firstruntest.cpp`, `tests/appstreamtest.cpp`, `tests/installtest.cpp`, `tests/desktopthemes.h`, `tests/testsilence.cpp`, `tests/spellfixspike.cpp` (siehe F9), `wireframes/`, `SPEC.md`, sämtliche Belegordner fremder Sprints. **Und ausdrücklich: kein Verhalten.** Diese Story ändert Schlüsselwörter und Signaturen, keine Logik. |

### Kollisionsfläche — **#76 kollidiert mit fast allem, was offen ist**

Das ist bei dieser Story das entscheidende Feld, und die Antwort ist unbequem.
Gemessen an den Fundorten gegen die Dateimengen der offenen Kandidaten:

| Vorgang | Größe | gemeinsame Dateien mit #76 | Befunde darin |
|---|---|---|---|
| **#83** (native Hülle, **ready**) | `size:l` | `src/capture/capturewindow.cpp`, `tests/capturetest.cpp`, `tests/captureshots.cpp` — **3 von 4** Dateien der #83-Menge | **9** |
| **#85** (Lesbarkeit unter fremden Themes) | `size:m` | dieselbe Capture-Menge | 9 |
| **#84**, **#81**, **#79** (Capture) | offen | `src/capture/capturewindow.cpp`, `tests/capturetest.cpp` | 8 |
| **#70**, **#71**, **#72** (Bibliothek) | offen | `src/ui/librarywindow.{h,cpp}`, `tests/librarytest.cpp` | **45** |
| **#74** (Einstellungsseite Kürzel) | offen | `src/shell/globalshortcuts.cpp`, `src/shell/shortcutregistration.{h,cpp}` | 5 |

**Der schärfste Punkt gegen #83:** Die drei `bugprone-narrowing-conversions`
liegen in `capturewindow.cpp:71–73` — das ist der Rumpf von `mixed()`, und der
Vorprüfbericht zu #83 sagt in Feld 1 ausdrücklich, dass `mixed()` und
`tinted()` **beide wegfallen**. Läuft #83 zuerst, heilen sich diese drei
Befunde von selbst. Läuft #76 zuerst, ändert ein Strang drei Zeilen, die ein
anderer löscht — und beide schreiben in dieselben Hunks von
`tests/capturetest.cpp` (5 Befunde bei `:297`, `:419`, `:441`, `:479`, `:480`;
#83 schreibt `:334–393`, `:411–433`, `:499–549` neu).

**Woran ich das messe:** an `cut -d: -f1` über die Fundortliste, gehalten gegen
die Dateimengen aus `docs/scrum/vorberichte/83-native-huelle/messung-a.md`
Feld 1 und den Issue-Texten von #70/#71/#72/#74. Eine handhabbare Menge ist das
**nicht** — es ist die halbe Codebasis, verteilt über jeden offenen Strang.

**Handhabbar wird sie nur durch die Reihenfolge, nicht durch den Zuschnitt:**
#76 läuft **allein** oder **zuletzt**. Das ist keine Vorsicht, sondern
Arithmetik — 20 von 45 Dateien lassen keinen Platz daneben.

---

## Feld 2 — Gemessene Fallen (die Zeilen für den Spawn-Auftrag)

### F1 — Der Ausgangsstand im Issue ist überholt, und seine auffälligste Zahl ist eine Zeilenzählung

Am Stand `009c471`, den das Issue nennt, nachgemessen
(`messungen/16-altstand-009c471.txt`) gegen HEAD:

| | `009c471` (Issue) | `6acc87e` (heute) |
|---|---|---|
| Rohzeilen | 130 | **140** |
| eindeutige Befunde | **73** (Issue sagt 72) | **81** |
| `performance-enum-size` | **64 roh / 7 eindeutig** (Issue sagt 59) | 66 roh / **7 eindeutig** |
| `misc-const-correctness` | 55 | 60 |
| `bugprone-narrowing-conversions` | 0 | 3 (mit #55 hinzugekommen) |

Die Tabelle im Issue mischt **Rohzeilen** mit einer **entdoppelten** Summe, und
zwei ihrer Zeilen reproduzieren nicht (59 gegen 64, 7 gegen 8). Praktisch
bedeutend ist nur eine davon: **`performance-enum-size` sind 7 Enums, nicht 59
Stellen.** Jede Kopfdatei wird von jeder Übersetzungseinheit erneut gemeldet —
Faktor 9,4. Wer die Story aus der Issue-Tabelle schätzt, schätzt diese Klasse
achtfach zu groß. Der Größenhinweis am Ende des Issues („114 der 121 Rohtreffer
sind Stilregeln") ruht auf derselben Zahl.

### F2 — Die maschinelle Heilung trägt 52 der 81 Befunde, konvergiert nach zwei Runden und bleibt grün

Gemessen auf einer Wegwerfkopie (`messungen/07`–`10`), `run-clang-tidy -fix`:

| Runde | Bau | Compilerwarnungen | Rohzeilen danach | `misc-const-correctness` |
|---|---|---|---|---|
| Ausgang | rc=0 | 0 | 140 | 60 |
| 1 | rc=0 | **0** | 91 | 11 |
| 2 | rc=0 | **0** | 88 | 8 |
| 3 | rc=0 | 0 | 88 | 8 |
| 4 | rc=0 | 0 | 88 | 8 |

`ctest` danach: **7/7 grün.** Umfang des Eingriffs: **57 geänderte Zeilen in 8
Dateien** (`src/main.cpp` 3, `src/shell/shortcutregistration.cpp` 1,
`src/shell/trayicon.cpp` 2, `src/ui/notelistdelegate.cpp` 1,
`tests/librarytest.cpp` 37, `tests/capturetest.cpp` 5, `tests/shelltest.cpp` 7,
`tests/captureshots.cpp` 1).

**Zwei Dinge daran sind tragend.** Erstens: **eine Runde genügt nicht** — das
`const` auf einer Variablen legt das `const` auf der nächsten erst frei
(60 → 11 → 8). Wer einmal `-fix` laufen lässt und den Rest zählt, zählt
falsch. Zweitens: **`const LibraryWindow library;` kompiliert.** Ich hatte
erwartet, dass `QObject::connect` mit einem `const`-Empfänger bricht; die Probe
`sonden/constprobe.cpp` sagt in beiden Fassungen rc=0
(`messungen/06-constprobe.txt`). Der Empfängerparameter von `connect` ist
`const`-qualifiziert. Die Vermutung war falsch, gemessen.

### F3 — Der Maschinenfix für `bugprone-implicit-widening-of-multiplication-result` ist wirkungslos und erzeugt einen neuen Befund

`tests/librarytest.cpp:443` wird zu:

```
-            library::noteGroup(monday.addSecs(-3600 * hoursBack), monday, german());
+            library::noteGroup(monday.addSecs(static_cast<qint64>(-3600 * hoursBack)), monday, german());
```

Die Umwandlung steht **außerhalb** der Multiplikation — die rechnet weiter in
`int`. Prompt meldet derselbe Linter an derselben Stelle
`bugprone-misplaced-widening-cast`: *„either cast from 'int' to 'qint64' is
ineffective, or there is loss of precision before the conversion"*. Der
richtige Griff ist `-3600LL` oder eine Umwandlung eines Operanden.

**Was daraus für den Spawn-Auftrag folgt:** `run-clang-tidy -fix` ist ein
Werkzeug, dessen Ergebnis Zeile für Zeile gelesen werden muss. Ein grüner Bau
und ein grünes `ctest` haben diesen falschen Fix **nicht** gefunden — er ist
sachlich harmlos (`hoursBack < 336`, kein Überlauf) und trotzdem falsch. Nur
der zweite Linterlauf hat ihn gemeldet.

### F4 — Acht Befunde kann kein Werkzeug heilen: sie stehen in `QFETCH`

Die acht `misc-const-correctness`, die nach der Konvergenz übrig bleiben, liegen
alle in `tests/librarytest.cpp` und alle auf einer `QFETCH`-Zeile
(`:1311`, `:2229`, `:2288`, `:3062`, `:3063`, `:3154`, `:3155`, `:3343`).
`QFETCH` ist ein Makro und deklariert die Variable selbst
(`/usr/include/qt6/QtTest/qtestcase.h:305`):

```
#define QFETCH(Type, name)\
    Type name = *static_cast<Type *>(QTest::qData(#name, ::qMetaTypeId<...>()))
```

`QFETCH(QSize, windowSize)` erzeugt `QSize windowSize = …`. Ein `const` ist dort
nur unterzubringen, indem man das Makro aufgibt — `QFETCH` ist die
QTest-Redensart für datengetriebene Tests. **Bleibt: `NOLINTNEXTLINE` mit
Begründung, achtmal.** Das ist der Grund, warum diese acht als einzige
`misc-const-correctness`-Befunde gar keinen Maschinenfix tragen.

### F5 — Zwei Prüfungen der Liste ziehen an derselben Variablen in entgegengesetzte Richtungen

`src/shell/globalshortcuts.cpp:50`:

```
const QString application = QStandardPaths::locate(...);
if (!application.isEmpty()) {
    return application;
}
```

`misc-const-correctness` fordert überall `const` auf lokalen Variablen;
`performance-no-automatic-move` meldet an genau dieser Stelle *„constness of
'application' prevents automatic move"* — das `const` verhindert die Rückgabe
per Verschiebung. Beide Prüfungen stehen in `.clang-tidy`
(`bugprone-*,performance-*,misc-const-correctness`). Für zurückgegebene lokale
Variablen kann die Story nicht beiden folgen; sie braucht dafür eine Regel
(eine Zeile in der Prüfkonfiguration oder ein `NOLINT`), nicht einen Griff.

### F6 — Hinter `globalshortcuts.cpp:93` steckt **kein** Fehler; der Widerspruch liegt in `.clang-tidy`

Gemessen mit `sonden/rueckgabeprobe.cpp` gegen den laufenden Dienst
(`messungen/15-rueckgabeprobe.txt`):

```
A  Aktion ohne objectName -> setGlobalShortcut() = false
B  Rücklesen einer nie registrierten Aktion -> 0 Sequenz(en)
C  Rücklesen von org.denkzettel.Denkzettel.desktop/show-capture -> 1 Sequenz(en) [Meta+N]
```

Das sagt dreierlei. **A:** `false` kommt aus einer örtlichen Zurückweisung
(namenlose Aktion) — nicht aus dem Dienst. **B:** Genau dieser Fall zeigt sich
beim Rücklesen als leere Antwort. **C:** Die echte Registrierung liest sich als
eine Sequenz zurück. Das Rücklesen in `registerCaptureShortcut()` (`:98–102`)
ist damit **strikt stärker** als der Rückgabewert: Es findet alles, was `false`
gemeldet hätte, und zusätzlich die Fehlschläge, die `false` gar nicht melden
kann (Falle 1 der Liste in `.claude/agents/denkzettel-dev.md`).

**Der Widerspruch ist datiert.** `git log -S` zeigt: der Eintrag
`^::KGlobalAccel::setGlobalShortcut$` kam mit `9e8353b` (01.08.2026, 11:18,
T5/#45), das Rücklesen mit `7130f81` (01.08.2026, 11:26, #5) — **acht Minuten
später, auf einem anderen Strang.** Die Prüfregel wurde geschrieben, bevor B5
im Code stand; sie fordert seither einen Beleg, den es nicht gibt.

**Dev-Vorlage an den PO** (entscheiden, nicht heilen): **den Eintrag aus
`.clang-tidy` streichen**, nicht `NOLINT` setzen. Begründung: Ein `NOLINT`
lässt die Regel stehen, die etwas messbar Falsches behauptet, und jede spätere
Registrierung bei KGlobalAccel liefe wieder dagegen. Die drei übrigen
Projekteinträge (`QDBusConnection::registerObject`, `registerService`,
`DaemonService::registerOnSessionBus`) bleiben — sie schlagen heute nirgends an,
also greifen sie korrekt.

**Meine Grenze:** Ich habe nicht gemessen, ob `setGlobalShortcut()` bei
**erreichbarem** Daemon jemals `false` liefert. Das würde eine echte
Registrierung in der Sitzung des Kunden anlegen; die Sonde vermeidet das
bewusst (namenlose Aktion, Komponente nie benutzt).

### F7 — Drei Befunde gehören #83, nicht #76

`src/capture/capturewindow.cpp:71–73` sind die drei Zeilen von `mixed()`.
`docs/scrum/vorberichte/83-native-huelle/messung-a.md`, Feld 1: *„`mixed()`/
`tinted()` `:69–94` (**beide fallen weg**)"*. Wer #76 vor #83 zieht, heilt drei
Zeilen, die #83 löscht.

### F8 — `lint-clazy` endet nicht mehr auf `rc=2` — geprüft, und zwar in beiden Konfigurationen

Der Auftrag nennt dies als Prüfpunkt. Gemessen:

| Konfiguration | `lint-clazy` rc | Befunde |
|---|---|---|
| Standard (`DENKZETTEL_SPIKE_SPELLFIX=OFF`) | **0** | 3 |
| `-DDENKZETTEL_SPIKE_SPELLFIX=ON` | **0** | 3 |

Die Heilung in `CMakeLists.txt:79` (`list(REMOVE_ITEM …)`) trägt, und mit
eingeschaltetem Spike wird `spellfixspike.moc` tatsächlich erzeugt, sodass auch
dieser Weg sauber ist. **Der Punkt ist erledigt.**

### F9 — „Auf 0" gilt nur für die Standardkonfiguration

Mit `-DDENKZETTEL_SPIKE_SPELLFIX=ON` (`messungen/14-spike-an.txt`):
`lint-tidy` meldet **145 Rohzeilen / 84 eindeutige** statt 140/81 — drei
zusätzliche Befunde in `tests/spellfixspike.cpp` (1 `easily-swappable`,
2 `const-correctness`). `lint-clazy` bleibt bei 3.

Das ist eine B17-Aussage: Ein Bericht „lint-tidy liefert 0" gilt für den Stand
**und die Schalterstellung**, unter der er entstand. Ob die Spike-Datei in die
Dateimenge gehört, entscheidet der PO (Offene Frage 5).

---

## Feld 4 — Prüfmittel, und was ein Agent nicht prüfen kann

### Ist „auf 0" überhaupt messbar? — Ja, aber nicht mit einer Zahl allein

**Der gemessene Gegenfall** (`messungen/13-stiller-nulllauf.txt`). Auf einem
frisch gebauten Baum werden zwei AUTOMOC-Dateien gelöscht:

| | `lint-clazy` | `lint-tidy` |
|---|---|---|
| **A — frisch gebaut** | rc=0, **3** warnings, 0 errors | rc=0, 140 warnings, 0 errors |
| **B — `librarytest.moc` und `shelltest.moc` fehlen** | rc=2, **0** warnings, **2** errors | rc=2, **140** warnings, 2 errors |

**`lint-clazy` meldet in Fall B genau null Befunde — und hat die beiden
Dateien, in denen alle drei Befunde liegen, überhaupt nicht analysiert.** clazy
bricht bei `fatal error: 'librarytest.moc' file not found` die Datei ab; die
Befunde dahinter erreicht es nie. Das ist die Bauart des Sprint-5-Vorfalls,
diesmal reproduzierbar hergestellt.

Erschwerend: **ein gewöhnlicher `cmake --build` holt die gelöschten
`.moc`-Dateien nicht zurück** (gemessen: „0 von 2"; AUTOMOC läuft nur bei
geänderter Quelle). Der kaputte Zustand ist stabil.

Zweiter Gegenfall (`messungen/11-unvollstaendiger-lauf.txt`): Auf einem nur
**konfigurierten**, nicht gebauten Baum meldet `lint-clazy` **27 warnings** —
27 von 27 sind `-Wmissing-include-dirs`, **kein einziger** ist ein
clazy-Befund. Der Zähler `grep -c 'warning:'` der CI zählt also
Compiler-Diagnosen mit.

**Daraus die Prüfvorschrift für AK 1 und AK 2** — drei Zahlen, nicht eine:

```
cmake --build build -j "$(nproc)"                       # muss rc=0 liefern
cmake --build build --target lint-clazy 2>&1 | tee clazy.log
#   rc == 0  UND  grep -c 'warning:' == 0  UND  grep -c 'error:' == 0
cmake --build build --target lint-tidy  2>&1 | tee tidy.log
#   rc == 0  UND  grep -c 'warning:' == 0  UND  grep -c 'error:' == 0
```

**`rc` allein ist kein Befundurteil** — gemessen rc=0 bei 140 clang-tidy- und
3 clazy-Befunden. **Aber `rc` ist ein Vollständigkeitssignal** — gemessen rc=2,
sobald eine Datei nicht analysiert werden konnte. Beide Zahlen zusammen
schließen den falschen Nullbefund; keine von beiden tut es allein.

### Je Akzeptanzkriterium

| AK | Prüfmittel | Grenze |
|---|---|---|
| **1** — `lint-tidy` liefert 0 | Dreizahl oben. Roh- und Entdopplungszählung fallen bei 0 zusammen, die Unterscheidung aus F1 ist für das *Ziel* also harmlos — nur für die *Größe* nicht | Gilt für die Standardkonfiguration (F9) und für diesen clang-Stand (B17). Ein clang-Sprung bringt neue Prüfungen mit; das Tor auf 0 färbt den CI-Lauf dann rot, ohne dass jemand etwas geändert hat — genau die beabsichtigte Frühwarnung des Container-Laufs |
| **2** — `lint-clazy` liefert 0 | dieselbe Dreizahl | dieselbe |
| **3** — Schwelle in `ci.yml` auf 0, `lint-tidy` aufgenommen | `git diff` an `.github/workflows/ci.yml`; danach **der Lauf des eigenen Commits** (`gh run list --commit $(git rev-parse HEAD)`), `completed` **und** `success` (B18) | **Der Container hat kein Plasma-Theme und keinen Compositor.** Ob die Zahlen dort dieselben sind wie auf Ganymed, weiß man erst nach dem ersten Lauf. Ein Tor auf 0, das im Container eine andere Zahl sieht, ist Dauerrot — **das ist der einzige Weg dieser Story, der vor dem Push nicht prüfbar ist** |
| **4** — kein Verhalten geändert, `ctest` grün, jede `src/`-Änderung begründet | `ctest --test-dir build` (7 Tests). Dazu **je Datei ein Satz** — betroffen sind bis zu 12 `src/`-Dateien | **`ctest` ist hier ein Netz mit großen Maschen.** Gemessen: der falsche Maschinenfix aus F3 lief durch grünen Bau *und* grünes `ctest`. Die Begründung je Datei ist Lesearbeit, kein Messwert — **ein Agent kann sie schreiben, aber nicht belegen** |
| **5** — bewusst stehengelassene Befunde tragen `NOLINT` **mit** Begründung | `git grep -n NOLINT -- src tests` gegen die Restliste aus Feld 2; jede Fundstelle muss eine Zeile Begründung darüber haben | Dass eine Begründung **stichhaltig** ist, misst kein Werkzeug. Prüfbar ist nur, dass sie dasteht |

### Was ein Agent an dieser Story grundsätzlich nicht prüfen kann

1. **Dass eine `const`-Änderung das Verhalten nicht berührt.** Belegbar ist,
   dass es kompiliert und dass die sieben Tests grün bleiben. Das ist AK 4 in
   seiner prüfbaren Hälfte; die andere Hälfte ist ein Argument.
2. **Dass ein `NOLINT` berechtigt ist.** Prüfbar ist nur seine Existenz.
3. **Wie sich das Tor im CI-Container verhält**, bevor dort ein Lauf stattfand
   (DoD-Grenze, siehe AK 3).
4. **Ob `setGlobalShortcut()` bei erreichbarem Daemon je `false` liefert** —
   nicht gemessen, weil die Messung eine Registrierung in der Kundensitzung
   hinterließe (F6).

---

## Feld 5 — Größenklasse: **`size:m`**

Bedeutung laut `PROZESS.md`: *„trägt einen Strang aus"*.

**Wofür nicht `s`.** Die Klasse `s` verlangt *„wenige Dateien, kein neuer
Prüfweg"*. Beides trifft nicht zu: **20 von 45** versionierten Quelldateien
tragen Befunde, und AK 3 verlangt einen **`lint-tidy`-Schritt in
`ci.yml`, den es heute nicht gibt** — ein neuer Prüfweg im Wortsinn der
Tabelle. Dazu kommen drei Stellen, an denen nicht gegriffen, sondern
entschieden wird (F4 acht `NOLINT`, F5 die gegenläufigen Prüfungen, F6 der
Eintrag in `.clang-tidy`), und acht Signaturbefunde, von denen einer eine
dokumentierte Kopfdatei-API betrifft (`desktopFileDeclaresAction`,
`shortcutregistration.h`).

**Wofür nicht `l`.** Der Kern ist gemessen klein: **52 der 81 Befunde heilt die
Maschine in zwei Runden, in 57 Zeilen, bei grünem Bau ohne eine einzige
Compilerwarnung und grünem `ctest`** (F2). Es entsteht kein Untersystem, es
ändert sich keine SPEC-Aussage, es fällt kein Bildbeleg an, DoD 3 greift nicht,
und es gibt keinen Sitzungslauf. Verglichen mit #83 — neue Zeichenweise, neuer
Ereignisweg, drei fallende Zusicherungen, vier SPEC-Stellen — ist der Abstand
deutlich.

**Was die Klasse nicht ausdrückt und deshalb hier stehen muss:** Die eigentliche
Last dieser Story ist nicht ihre Größe, sondern ihre **Fläche**. `size:m` sagt
„trägt einen Strang aus" und trifft den Aufwand; es sagt nichts darüber, dass
dieser Strang in 20 Dateien schreibt, die fünf andere offene Vorgänge ebenfalls
brauchen. Das steht in Feld 1 und gehört in den Schnitt.

### Die Antwort auf die Sprintfrage — nüchtern

Neben `size:l` (#83) steht nach `PROZESS.md` **nur `size:s`**. **#76 ist `m`,
also passt sie nicht neben #83.** Und selbst wenn die Regel es zuließe, wäre es
falsch: #76 berührt 3 der 4 Dateien von #83, und drei ihrer Befunde löscht #83
ohnehin (F7).

**Zwei Wege, wenn der Kunde die Heilung in Sprint 7 will** — beide braucht er
nicht zu erraten, sie sind vermessen:

- **(a) #76 auf `size:s` schneiden: nur clazy.** AK 2 + die Schwelle 3 → 0 in
  `ci.yml`. Umfang: **3 Befunde, 2 Dateien** (`tests/librarytest.cpp`,
  `tests/shelltest.cpp`), kein neuer CI-Schritt, keine `.clang-tidy`-Frage,
  keine `NOLINT`-Entscheidung. Das ist ein echtes `s` und läuft neben #83 —
  bis auf die Überschneidung `tests/librarytest.cpp`, die #83 nicht anfasst.
  Der clang-tidy-Teil bleibt als eigenes Issue stehen.
- **(b) #76 ganz, aber ohne #83.** Dann füllt sie den Sprint mit ein bis zwei
  `size:s` daneben — und sie sollte trotzdem **nach** #83 laufen, sonst arbeitet
  sie an gelöschtem Code.

**`xl` messe ich nicht.** Wenn geteilt wird, dann an der Naht aus (a): clazy
gegen clang-tidy. Sie ist sauber, weil es zwei Werkzeuge, zwei
Akzeptanzkriterien und zwei disjunkte Befundmengen sind.

---

## Feld 6 — Offene Fragen an PO oder Kunde

1. **Soll „auf 0" wörtlich gelten?** Gemessen sind **acht** Befunde, die kein
   Werkzeug heilen kann, weil sie in `QFETCH` stehen (F4). Wörtlich „0" heißt
   dann: acht `NOLINTNEXTLINE` mit Begründung in `tests/librarytest.cpp`.
   *Dev-Empfehlung:* wörtlich ja, aber mit **einer** gemeinsamen Begründung —
   und die Alternative ausdrücklich verwerfen, `misc-const-correctness` für
   `tests/` abzuschalten: Damit fielen auch die 52 Befunde weg, die die Story
   gerade sinnvoll geheilt hat.
2. **`globalshortcuts.cpp:93` — `NOLINT` oder Streichung aus `.clang-tidy`?**
   Dev-Vorlage: **streichen** (F6, mit Chronologie und Sondenbeleg). Kein
   Fehler dahinter; das Rücklesen ist strikt stärker als der Rückgabewert.
3. **Wie soll die Story mit den gegenläufigen Prüfungen umgehen** (F5)?
   `misc-const-correctness` gegen `performance-no-automatic-move` an
   zurückgegebenen lokalen Variablen. Eine Regel, nicht ein Griff — und sie
   gilt für jede künftige Funktion dieser Bauart.
4. **`bugprone-easily-swappable-parameters` (8 Stück): heilen oder
   stehenlassen?** Sechs sind Testhelfer, zwei sind Produktivcode —
   `librarywindow.cpp:134` `placeholderPage(title, hint, …)` und
   `shortcutregistration.cpp:28` `desktopFileDeclaresAction(pfad, aktion)`.
   Beim ersten wäre ein Vertauschen für den Kunden **sichtbar** (Titel und Hinweis
   im Leerzustand). Heilen heißt hier: Signaturen ändern oder eigene Typen
   einführen — das ist **keine Aufräumarbeit mehr**, sondern Entwurf.
   *Dev-Empfehlung:* `NOLINT` mit Begründung, und den sichtbaren Fall bei
   `placeholderPage` stattdessen durch eine Testzusicherung absichern — das
   aber als eigenes Issue.
5. **Gehört `tests/spellfixspike.cpp` in die Dateimenge?** Mit
   `-DDENKZETTEL_SPIKE_SPELLFIX=ON` kommen 3 Befunde dazu (F9). Die Datei folgt
   ihrem eigenen Ziel und ist im Standardbau aus der Linterliste genommen.
   *Dev-Empfehlung:* nein — und die Bedingung ins AK schreiben, damit „0" eine
   Schalterstellung nennt.
6. **Reihenfolge gegen #83.** Drei Befunde in `capturewindow.cpp` löscht #83
   von selbst (F7); die Dateimengen überlappen in 3 von 4 Dateien. *Dev-Vorlage:*
   #76 nach #83, nicht daneben.
7. **Gehört das Tor in die DoD?** SPEC und `PROZESS.md` erwähnen die Linter
   **nirgends**; DoD 1 spricht nur von „warnungsarm". Die Schwelle stünde nach
   dieser Story allein in `ci.yml`. Nach DoD 4 in der Fassung nach B9 ist das
   eine entdeckte Bedingung, die nachzuziehen ist.
8. **Soll die CI-Wache statt Zeilen Befunde zählen?** `grep -c 'warning:'`
   zählt gemessen auch Compiler-Diagnosen (27 `-Wmissing-include-dirs`) und
   zählt Kopfdateibefunde mehrfach (Faktor 9,4 bei `enum-size`). Bei der
   Schwelle **0** ist das folgenlos; sobald jemand wieder eine Schwelle über
   null setzt, ist es eine Falle. *Dev-Empfehlung:* die Prüfung auf die Dreizahl
   umstellen (rc, warnings, errors) — dann trägt sie auch über 0 hinaus.

---

## Was ich **nicht** klären konnte

- Ob `KGlobalAccel::setGlobalShortcut()` bei erreichbarem Daemon je `false`
  liefert. Die Messung hinterließe eine Registrierung in der Sitzung des Kunden.
- Wie sich die Zahlen im **CI-Container** verhalten. Dort steht ein anderer
  Paketstand und kein Plasma; ich habe den Container nicht gestartet.
- Ob die 52 maschinell geheilten Stellen **alle** verhaltensneutral sind. Belegt
  sind: 0 Compilerwarnungen, 7/7 Tests grün, Konvergenz nach zwei Runden. Das
  ist die prüfbare Hälfte von AK 4.
- Warum die Zahlen des Issues (72 eindeutig, `enum-size` 59) um eins bzw. um
  fünf von meiner Nachmessung am **selben Commit** abweichen (73 bzw. 64). Der
  Werkzeugstand hat sich seit dem 04.08. vormittags nicht geändert, soweit ich
  sehe; ich habe die Abweichung nicht weiterverfolgt, weil sie an der Aussage
  nichts ändert.
- Den **Aufwand in Zeit**. Wird in diesem Projekt nicht erhoben.

---

## Befehle, mit denen ich gemessen habe

```
bash docs/scrum/vorberichte/76-linterbefunde/pruefen.sh
cmake -B docs/scrum/vorberichte/76-linterbefunde/build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build docs/scrum/vorberichte/76-linterbefunde/build --target lint-tidy
cmake --build docs/scrum/vorberichte/76-linterbefunde/build --target lint-clazy
grep 'warning:' <log> | sort -u | grep -o '\[[a-z0-9-]*\]$' | sort | uniq -c
run-clang-tidy -p <bau>/lint -quiet -fix <dateien>        # nur auf Wegwerfkopien
git archive HEAD | tar -x -C <wegwerfkopie>
git log -S 'KGlobalAccel::setGlobalShortcut$' -- .clang-tidy
git grep -n -i 'clang-tidy\|clazy\|lint' -- SPEC.md docs/scrum/PROZESS.md
busctl --user call org.kde.kglobalaccel /kglobalaccel org.kde.KGlobalAccel allComponents
pacman -Q clang clazy cmake qt6-base
```

**Nicht getan:** nichts committet, nichts gepusht, nichts nach `/usr`
installiert, keine Zeile unter `src/`, `tests/`, `SPEC.md` oder `wireframes/`
geändert, **keinen Linterbefund geheilt**.
