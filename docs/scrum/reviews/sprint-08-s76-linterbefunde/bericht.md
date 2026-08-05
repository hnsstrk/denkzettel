# Prüfbericht #76 — Linterbefunde heilen

**Sprint 8, Strang C** · Zweig `tech/76-linterbefunde` · Ausgangsstand `1795c87`
· Ganymed, 05.08.2026 · Belege in `messungen/`, wiederholbar über
`bash docs/scrum/reviews/sprint-08-s76-linterbefunde/pruefen.sh`

**Werkzeugstand** (B17 — eine Aussage gilt für einen Stand): clang 22.1.8,
clazy 1.17.1, cmake 4.1.2, qt6-base 6.11.1.

**Schalterstellung, für die „0" gilt:** `-DDENKZETTEL_SPIKE_SPELLFIX=OFF`, die
Standardkonfiguration. Ohne diesen Zusatz ist „0" keine Aussage; die Gegenprobe
mit eingeschaltetem Schalter steht in §6.

---

## 1. Ergebnis

| Lauf | rc | Warnungen | Fehler |
|---|---|---|---|
| Bau (`cmake --build build`) | 0 | **0** | 0 |
| `lint-tidy` | **0** | **0** | **0** |
| `lint-clazy` | **0** | **0** | **0** |
| `ctest` | 0 | **9/9 grün** | |

`messungen/03-endstand.txt`. Die Zeile
`Running clang-tidy … for 30 files out of 49 in compilation database` steht im
Beleg, weil eine Null ohne sie nichts darüber sagt, ob überhaupt etwas geprüft
wurde — siehe §7.

**Ausgangsstand, selbst nachgemessen** statt aus dem Issue übernommen:

| | Issue (`009c471`) | Vorprüfung (`6acc87e`) | PO (`9f2063a`) | **hier (`1795c87`)** |
|---|---|---|---|---|
| Rohzeilen | 130 | 140 | 140 | **149** |
| eindeutige Befunde | 73 | 81 | 81 | **88** |
| Dateien mit Befunden | — | 20 | 19 | **20** |
| `misc-const-correctness` | 55 | 60 | 63 | **70** |
| `performance-enum-size` | 64 roh / 7 eindeutig | 66 / 7 | 66 / 7 | **66 / 7** |
| `bugprone-narrowing-conversions` | 0 | 3 | 0 | **0** |
| clazy | 3 | 3 | 3 | **3** |

Die drei `narrowing-conversions` sind mit #83 verschwunden, wie die Vorprüfung
in F7 vorhergesagt hatte: Sie lagen im Rumpf von `mixed()`, den #83 gelöscht
hat. Der Zuwachs von 81 auf 88 kommt aus #61 und #85. Belege:
`messungen/01-ausgangsstand-tidy-eindeutig.txt`, `02-ausgangsstand-clazy.txt`.

## 2. Die Zahlenfolge der `-fix`-Runden (AK 8)

Vier Runden `run-clang-tidy -fix` über 30 Dateien (`src/**/*.cpp`,
`tests/*.cpp` ohne `spellfixspike.cpp`), jede gefolgt von einem vollständigen
Bau:

| Runde | geprüfte Dateien | Bau | Compilerwarnungen | Rohzeilen | eindeutig |
|---|---|---|---|---|---|
| Ausgang | — | rc=0 | 0 | 149 | 88 |
| nach der `.clang-tidy`-Streichung | — | rc=0 | 0 | **148** | **87** |
| 1 | 30 von 49 | rc=0 | **0** | **90** | **29** |
| 2 | 30 von 49 | rc=0 | **0** | **86** | **25** |
| 3 | 30 von 49 | rc=0 | 0 | 86 | 25 |
| 4 | 30 von 49 | rc=0 | 0 | 86 | 25 |

**Eine Runde genügt nicht**, wie die Vorprüfung gemessen hatte: Das `const` auf
einer Variablen legt das `const` auf der nächsten erst frei. Runde 2 findet
noch vier Rohzeilen, die Runde 1 nicht sehen konnte. Ab Runde 3 ist die Zahl
stabil.

**Der von der Vorprüfung angekündigte Fehlfix ist eingetreten** (F3). Die
Maschine schrieb in `tests/librarytest.cpp`:

```
-    library::noteGroup(monday.addSecs(-3600 * hoursBack), …)
+    library::noteGroup(monday.addSecs(static_cast<qint64>(-3600 * hoursBack)), …)
```

Die Umwandlung steht **außerhalb** der Multiplikation — die rechnet weiter in
`int` —, und prompt meldete derselbe Linter an derselben Stelle
`bugprone-misplaced-widening-cast`. Geheilt von Hand mit `-3600LL * hoursBack`,
was einen Operanden verbreitert und die Rechnung damit wirklich in
`long long` verschiebt. **Grüner Bau und grünes `ctest` haben diesen Fehlfix
nicht gefunden** — nur der zweite Linterlauf.

**Der Maschinenstil wurde nachgezogen.** `run-clang-tidy` schreibt Ost-const
(`KDBusService const service`, dazu `QAction  const*libraryAction` mit doppeltem
Leerzeichen); das Projekt schreibt durchgehend West-const (`const QString
application`, `const bool selected`). 64 Stellen wurden auf die Schreibweise des
Projekts umgestellt, ohne die Bedeutung anzufassen.

## 3. Jede Änderung in `src/`, einzeln belegt (AK 3)

**Ein grüner `ctest` genügt hier nicht — er war schon vorher grün.** Deshalb
steht zu jeder Änderung, was sie tut und warum das Verhalten dasselbe bleibt.

### 3.1 `const` auf Zeigern, deren Ziel wir nur lesen — 3 Stellen

| Stelle | Änderung | Warum das Verhalten dasselbe bleibt |
|---|---|---|
| `src/shell/trayicon.cpp:90` | `QAction *libraryAction` → `const QAction *` | Der Zeiger wird nur an `connect()` gereicht, dessen Sender-Parameter `const QObject *` ist. **Nur unser Zeiger** wird qualifiziert, nicht das Objekt — das gehört dem Menü und bleibt veränderbar. Ein verändernder Aufruf über diesen Zeiger würde nicht übersetzen; es gibt keinen |
| `src/shell/trayicon.cpp:102` | `QAction *quitAction` → `const QAction *` | dasselbe |
| `src/ui/notelistdelegate.cpp:125` | `QStyle *style` → `const QStyle *` | Einziger Aufruf ist `drawControl()`, und der ist `const`. Der Stil gehört dem Widget bzw. der Anwendung; wir zeichnen nur mit ihm |

### 3.2 `const` auf einem Wert, der nur gelesen wird — 1 Stelle

| Stelle | Änderung | Warum das Verhalten dasselbe bleibt |
|---|---|---|
| `src/shell/shortcutregistration.cpp:41` | `KDesktopFile file` → `const KDesktopFile` | `KDesktopFile` ist **kein** QObject, hat keine Signalanbindung und keinen anderen Weg, sich zu verändern. Beide Aufrufe darauf — `readActions()` und `hasActionGroup()` — sind `const`. Damit ist der Fall anders gelagert als die in §5 |

### 3.3 `const` **entfernt**, wo zwei Prüfungen gegeneinander ziehen — 1 Stelle

| Stelle | Änderung | Warum das Verhalten dasselbe bleibt |
|---|---|---|
| `src/shell/globalshortcuts.cpp:48` | `const QString application` → `QString application`, dazu `NOLINTNEXTLINE(misc-const-correctness)` mit Begründung | PO-Entscheidung 3. Die Variable wird zurückgegeben; das `const` verhinderte die Rückgabe per Verschiebung und kostete eine Kopie je Aufruf. Der Wert ändert sich nirgends — zwischen Zuweisung und `return` steht nur ein `isEmpty()`. **Eine Kopie weniger, sonst nichts.** Die Regel dahinter steht als Kommentar in `.clang-tidy`, damit die nächste Funktion dieser Bauart ihr folgt |

### 3.4 Enum-Basistypen — 7 Enums in 5 Dateien

| Stelle | Änderung | Warum das Verhalten dasselbe bleibt |
|---|---|---|
| `src/store/note.h:16` `Note::Type` | `: std::uint8_t` | 2 Werte |
| `src/store/note.h:21` `Note::State` | `: std::uint8_t` | 3 Werte |
| `src/shell/shortcutregistration.h:10` `ShortcutRegistration` | `: std::uint8_t` | 4 Werte |
| `src/ui/timestampformat.h:20` `library::NoteGroup` | `: std::uint8_t` | 5 Werte |
| `src/ui/librarywindow.h:73` `Selection` | `: std::uint8_t` | 2 Werte |
| `src/ui/librarywindow.h:81` `UnsavedAnswer` | `: std::uint8_t` | 3 Werte |
| `src/ui/notelistmodel.h:36` `Role` | `: std::uint16_t` | `Qt::UserRole` = 256 passt nicht in ein Byte |

**Die gemeinsame Begründung, am Code geprüft und nicht angenommen:** Ein
Basistyp ändert das Verhalten dort, wo der Typ nach außen sichtbar wird — über
`Q_ENUM`/`QMetaType` (ein anderer Metatyp), über `QVariant` (ein anderer
gespeicherter Typ), über D-Bus (eine andere Signatur) oder als Ganzzahl in der
Datenbank. **Keiner dieser Wege liegt hier vor**, gemessen:

- `git grep 'Q_ENUM\|qRegisterMetaType\|Q_DECLARE_METATYPE' -- src tests` ist
  **leer**. Keines der sieben Enums ist beim Metatypsystem angemeldet.
- Sechs der sieben sind `enum class`, also ohne stillschweigende Umwandlung.
  Sie treten nur in `switch`, Vergleichen und `std::optional` auf.
- `Note::Type` und `Note::State` gehen **als Text** in die Datenbank
  (`typeToText`/`typeFromText`, `stateToText`/`stateFromText` in
  `src/store/store.cpp:84–115`), nicht als Zahl. Der Basistyp erreicht die
  Datei gar nicht.
- `NoteListModel::Role` ist als einziges ein unbenanntes `enum` und wird als
  `int` weitergereicht (`index.data(NoteListModel::TimestampRole)`,
  13 Fundstellen). `std::uint16_t` wird zu `int` heraufgestuft; die Werte 256
  und 257 passen mit weitem Abstand. Der Kommentar an der Stelle sagt, warum
  hier nicht ein Byte steht.

### 3.5 `NOLINT`-Blöcke ohne Codeänderung — 2 Stellen

`src/shell/shortcutregistration.cpp:33` (`desktopFileDeclaresAction`) und
`src/ui/librarywindow.cpp:153` (`placeholderPage`) bekommen je einen
Begründungsblock und eine `NOLINTNEXTLINE`. **Es ändert sich keine Codezeile**,
nur Kommentar. Siehe §4.

## 4. Jedes `NOLINT` mit Begründung, gezählt (AK 4)

**17 Marken**, gezählt mit `git grep -E '^\s*// NOLINT' -- src tests`. Die
Zählung sieht ausdrücklich nur auf Zeilen, die mit der Marke **beginnen** — im
Kopf von `librarytest.cpp` steht das Wort auch im Fließtext einer Begründung,
und eine naive Zählung meldete 18.

| Fall | Prüfung | Zahl | Fundorte | Begründung |
|---|---|---|---|---|
| **1** (PO-Entscheidung 1) | `misc-const-correctness` | **8** | `tests/librarytest.cpp`, alle auf einer `QFETCH`-Zeile | `QFETCH` ist ein Makro und deklariert die Variable selbst (`Type name = *static_cast<Type *>(…)`). Ein `const` ist dort nur unterzubringen, indem man `QFETCH` aufgibt — die QTest-Redensart für datengetriebene Tests. **Eine** gemeinsame Begründung steht im Kopf der Datei, jede Marke verweist darauf |
| **2** (PO-Entscheidung 3) | `misc-const-correctness` | **1** | `src/shell/globalshortcuts.cpp:52` | An einer zurückgegebenen lokalen Variablen gewinnt `performance-no-automatic-move`. Siehe §3.3 |
| **3** (PO-Entscheidung 4) | `bugprone-easily-swappable-parameters` | **8** | `src/shell/shortcutregistration.cpp`, `src/ui/librarywindow.cpp`, `tests/editshots.cpp` (2), `tests/libraryshots.cpp`, `tests/librarytest.cpp`, `tests/readmeshots.cpp`, `tests/searchshots.cpp` | Heilen hieße Signaturen ändern oder eigene Typen einführen — Entwurf, nicht Aufräumen. Der sichtbare Fall (`placeholderPage`) bekommt stattdessen eine Testzusicherung als **#88** |
| | | **17** | | |

**Ein vierter Fall ist aufgetreten und ging als Frage an den PO, nicht in ein
stilles `NOLINT`.** Er steht in §5.

**Eine Falle beim Setzen, gemessen:** Die ersten acht `NOLINTNEXTLINE` des
dritten Falls standen als **erste** Zeile eines fünfzeiligen Kommentarblocks.
Sie haben nichts unterdrückt — `NOLINTNEXTLINE` gilt für die unmittelbar
folgende Zeile, und das war eine weitere Kommentarzeile. Der Linterlauf zeigte
es sofort; ein Lauf, der nur den Rückgabewert gelesen hätte, nicht. Seither
steht die Marke als **letzte** Zeile des Blocks.

## 5. Der vierte Fall — offen beim PO

`misc-const-correctness` schlägt `const` auf **Objekten** vor, die über das
Meta-Object-System verändert werden. Der Prüfer sieht diese Änderung nicht:
`QObject::connect`, `QSignalSpy` und `QTest::qExec` nehmen ihre Ziele als
`const QObject *` und `const_cast`en innen. Der Maschinenfix hat das an
**16 Stellen** getan (3 in `src/`, 13 in `tests/`). Ein `const` definiertes
Objekt während seiner Lebensdauer zu verändern, ist nach [dcl.type.cv]/4
undefiniert.

Die drei Fälle, an denen das keine Sprachjuristerei ist:

1. **`tests/librarytest.cpp` — `const DialogWatch watch;` (2×).** Der
   Konstruktor hängt eine Lambda an einen Timer, die `m_appeared = true;`
   schreibt; der Test prüft danach `QVERIFY2(!watch.appeared(), …)`. **Faltet
   der Übersetzer `m_appeared` auf seinen Anfangswert, ist der Test grün, ohne
   etwas gemessen zu haben.**
2. **`src/main.cpp:67` — `const LibraryWindow library(&store);`**, und
   `:71`/`:75` verbinden `&LibraryWindow::showLibrary` darauf. Der Slot ruft
   `reload()`, `show()`, `raise()` und `setFocus()`.
3. **`tests/captureshots.cpp:151`, `tests/identitytest.cpp:72` —
   `const QApplication app(argc, argv);`** vor `QTest::qExec()`.

**Vorgeschlagene Regel** (dieselbe Bauart wie PO-Entscheidung 3): *Auf einem
QObject-Abkömmling, der als Wert deklariert ist, wird `const` nicht gesetzt;
die Stelle trägt `NOLINT(misc-const-correctness)` mit gemeinsamer Begründung.*
**Zeiger bleiben unberührt** — `const QPushButton *edit` qualifiziert nur
unseren Zeiger, und wir lesen nur darüber (§3.1).

*Die Vorprüfung hat diesen Fix in F2 auf Verträglichkeit geprüft und
festgestellt, dass `const LibraryWindow library;` **kompiliert**. Das stimmt und
ist genau der Punkt: Der Empfängerparameter von `connect` ist
`const`-qualifiziert, deshalb übersetzt es — und deshalb sieht der Prüfer die
Änderung nicht.*

## 6. Die Gegenprobe — eine Null, die etwas behauptet

**Zwei Proben, weil eine Null ohne sie keine Aussage ist.**

**Gegen-Schalterstellung.** Mit `-DDENKZETTEL_SPIKE_SPELLFIX=ON` meldet
`lint-tidy` **3** Befunde, alle drei in `tests/spellfixspike.cpp`
(1 `easily-swappable-parameters`, 2 `misc-const-correctness`); `lint-clazy`
bleibt bei 0 (`messungen/06-spike-an-tidy.txt`). Das deckt sich mit F9 der
Vorprüfung und zeigt, dass „0" hier eine Schalterstellung meint und keinen
Zustand der Welt. Die Datei folgt ihrem eigenen Ziel (PO-Entscheidung 5).

**Schlagen die Wachen überhaupt an?** Zwei geheilte Muster wurden in
`tests/shelltest.cpp` wieder eingesetzt — ein `first()` auf einem Temporär und
eine lokale Variable ohne `const` (`messungen/05-gegenprobe.txt`):

| | Endstand | mit eingebautem Fehler |
|---|---|---|
| `lint-clazy` | 0 Warnungen | **1** — `Don't call QList::first() on temporary` |
| `lint-tidy` | 0 Warnungen | **1** — `variable … can be declared 'const'` |

Beide werden rot. Danach zurückgesetzt; `pruefen.sh` führt die Probe auf einer
Wegwerfkopie.

## 7. Die CI-Wache prüft die Dreizahl (AK 6)

`.github/workflows/ci.yml`: die clazy-Schwelle geht von 3 auf 0, und ein
`clang-tidy`-Schritt kommt hinzu, den es bisher nicht gab. Beide prüfen
**Rückgabewert, Zahl der Warnungen und Zahl der Fehler**. `clang` steht jetzt
in der Paketliste und im Werkzeugstand — ohne es fände CMake `run-clang-tidy`
nicht.

**Warum drei Zahlen und nicht eine.** Beide Lücken sind gemessen, und die
Wachenlogik wurde gegen echte Logs gehalten (`messungen/04-wachenprobe.txt`):

| Lage | rc | Warnungen | Wache |
|---|---|---|---|
| sauberer Lauf | 0 | 0 | **grün** |
| Befunde, aber `rc=0` | 0 | 149 | **rot** — *das verpasst eine reine `rc`-Prüfung* |
| stiller Nulllauf | 2 | 0 | **rot** — *das verpasst eine reine Zählung* |

Die zweite Zeile ist der Normalfall von `clazy-standalone`: Es liefert `rc=0`
auch mit Befunden. Die dritte ist der Fall aus Feld 4 der Vorprüfung — fehlt
eine `.moc`-Datei, bricht clazy die Übersetzungseinheit mit `fatal error` ab
und meldet **null** Befunde über Dateien, die es nie analysiert hat.

Bei Schwelle 0 fällt die Rohzeilenzählung mit der Befundzählung zusammen. **Wer
die Schwelle je wieder über null setzt, muss vorher entdoppeln** — bei
`performance-enum-size` war der Faktor gemessen 9,4 (7 Enums, 66 Rohzeilen),
und `grep -c 'warning:'` zählt zusätzlich Compiler-Diagnosen mit. Der Hinweis
steht im Kommentar beider Schritte.

## 8. Die Schwelle steht nicht mehr allein in einer YAML-Datei (AK 7)

**DoD 1 in `docs/scrum/PROZESS.md` nennt sie jetzt**: `lint-tidy` und
`lint-clazy` liefern in der Standardkonfiguration je rc=0, null Warnungen, null
Fehler; drei Zahlen, weil keine allein trägt; ein Befund bleibt nur mit
`NOLINT` und danebenstehender Begründung stehen; ein Fall, den die bestehenden
Begründungen nicht decken, geht an den PO. Entdeckte Bedingung nach DoD 4/B9 —
bisher stand dort nur „warnungsarm", und das sagte über die Linter nichts.

**Nach B17 mitgezogen**, weil die Änderung diese Aufzählungen unvollständig
macht — gesucht mit
`git grep -n -i "clang-tidy\|clazy\|lint-tidy\|lint-clazy\|Linter"`:

- `README.md:161` — „schlägt bei jedem Baufehler, jeder Compiler-Warnung und
  jedem roten Test fehl" → **und jedem Linterbefund**.
- `README.md:140` — der Linterabschnitt nennt jetzt die Null und die
  `NOLINT`-Regel.
- `docs/scrum/PROZESS.md`, „Automatische Testläufe" — dieselbe Aufzählung,
  dazu die Begründung für die Dreizahl.

**`CLAUDE.md:202` blieb stehen**: „Er schlägt bei jeder Warnung und jedem roten
Test fehl" wird durch diese Änderung nicht falsch, und die Datei ist die
Arbeitsanweisung des ganzen Projekts — das ist eine PO-Entscheidung.

## 9. `.clang-tidy` — was sich geändert hat und warum (AK 5)

Beides steht als Kommentar in der Datei selbst.

**Gestrichen: `^::KGlobalAccel::setGlobalShortcut$`** aus
`bugprone-unused-return-value.CheckedFunctions` (PO-Entscheidung 2). Die Regel
forderte einen Beleg, den es nicht gibt: Der Rückgabewert kann keinen
Backend-Fehlschlag zeigen, weil `doRegister()` seinen D-Bus-Aufruf abschickt
und die Antwort fallenlässt. Das Rücklesen in `registerCaptureShortcut()` ist
**strikt stärker** — es findet alles, was `false` gemeldet hätte, und zusätzlich
die Fehlschläge, die `false` gar nicht melden kann. Ein `NOLINT` hätte eine
Regel stehenlassen, die etwas messbar Falsches behauptet, und jede spätere
Registrierung liefe wieder dagegen. **Die drei übrigen Projekteinträge
bleiben** — neben keinem von ihnen steht ein stärkerer Nachweis.

**Ergänzt: die Regel für zwei gegenläufige Prüfungen** (PO-Entscheidung 3), als
Kommentar über der `Checks`-Zeile, damit die nächste Funktion dieser Bauart ihr
folgt.

**Die Prüfliste selbst ist unverändert.** Keine Prüfung wurde abgewählt —
insbesondere nicht `misc-const-correctness` für `tests/`, was 52 Befunde
mitgenommen hätte, die diese Story gerade sinnvoll heilt.

## 10. Was diese Story nicht angefasst hat

- **`tests/spellfixspike.cpp`** (PO-Entscheidung 5). Die drei Befunde darin
  sind gemessen und in §6 benannt.
- **Signaturen und neue Typen** gegen `bugprone-easily-swappable-parameters`
  (PO-Entscheidung 4). Der sichtbare Fall ist **#88**.
- **Kein Verhalten.** Diese Story ändert Schlüsselwörter, Basistypen und
  Kommentare. Wo eine Änderung mehr wäre als das, steht sie einzeln in §3.

## 11. Was hier nicht prüfbar war

1. **Dass eine `const`-Änderung das Verhalten nicht berührt**, ist im
   allgemeinen kein Messwert. Belegbar ist, dass es übersetzt und dass die
   neun Tests grün bleiben; die andere Hälfte ist das Argument in §3, je
   Änderung geführt.
2. **Wie sich das Tor im CI-Container verhält**, weiß man erst nach dem ersten
   Lauf dort. Der Container hat kein Plasma-Theme und keinen Compositor; ob die
   Zahlen dieselben sind wie auf Ganymed, ist der einzige Weg dieser Story, der
   vor dem Push nicht prüfbar ist. Nach dem Push gilt B18: **der Lauf des
   eigenen Commits**, `completed` **und** `success`.
3. **Ob `setGlobalShortcut()` bei erreichbarem Daemon je `false` liefert**,
   bleibt ungemessen — die Messung legte eine echte Registrierung in der
   Sitzung des Kunden an. Die Streichung in §9 ruht nicht darauf: Sie ruht
   darauf, dass das Rücklesen **jeden** Fall abdeckt, den der Rückgabewert
   abdeckt.

## 12. Ein Fund, der in die Fallenliste gehört

**Meine ersten drei `-fix`-Runden haben null Dateien geprüft und dabei wie eine
saubere Konvergenz ausgesehen.** Gemeldet wurden `Bau rc=0`, `Baufehler=0`,
`Compilerwarnungen=0` und dreimal dieselbe Zahl — stabil, wie AK 8 es verlangt.

Ursache: Der Shell, in dem die Befehle liefen, war **zsh**, nicht bash. Dort
zerlegt eine ungeschützte Parameterexpansion keine Wörter.
`run-clang-tidy … $DATEIEN` bekam **ein** Argument mit Zeilenumbrüchen darin,
baute daraus einen regulären Ausdruck, der auf nichts passt, und schrieb
`Applying fixes ...`. Der einzige Hinweis stand in einer Zeile, die niemand
liest:

```
Running clang-tidy in 24 threads for 0 files out of 49 in compilation database ...
```

Dieselbe Bauart trifft `docs/scrum/vorberichte/76-linterbefunde/pruefen.sh`
Zeile 45–47 — dort mit `#!/usr/bin/env bash`, im bash-Lauf also unschädlich.
Wer das Skript mit `zsh pruefen.sh` startet, bekommt eine plausible Zahlenfolge
über nichts.

**Die Regel dahinter:** Ein Werkzeug, das eine Dateimenge entgegennimmt, meldet
irgendwo, **wie viele** Dateien es angefasst hat. Diese Zahl gehört gelesen,
bevor die Befundzahl gelesen wird. `pruefen.sh` dieses Ordners liest sie vor
und schlägt Alarm, wenn die Zeile fehlt.
