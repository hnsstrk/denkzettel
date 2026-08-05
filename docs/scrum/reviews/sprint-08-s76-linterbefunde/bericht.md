# Prüfbericht #76 — Linterbefunde heilen

**Sprint 8, Strang C** · Zweig `tech/76-linterbefunde` · Ausgangsstand `1795c87`
· Ganymed, 05.08.2026 · Belege in `messungen/`, wiederholbar über
`bash docs/scrum/reviews/sprint-08-s76-linterbefunde/pruefen.sh`

**Werkzeugstand** (B17 — eine Aussage gilt für einen Stand): clang 22.1.8,
clazy 1.17.1, cmake 4.4.2, qt6-base 6.11.1 (selbst abgefragt, nicht aus der
Vorprüfung übernommen — dort stand cmake 4.1.2).

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
wurde — siehe §8.

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
| 1 | **30 von 49** | rc=0 | **0** | **90** | **29** |
| 2 | **30 von 49** | rc=0 | **0** | **86** | **25** |
| 3 | **30 von 49** | rc=0 | 0 | 86 | 25 |
| 4 | **30 von 49** | rc=0 | 0 | 86 | 25 |

**Die Spalte „geprüfte Dateien" steht hier auf Auflage des PO, und sie ist die
wichtigste der Tabelle.** Sie kommt aus der Zeile
`Running clang-tidy … for N files out of M`, die `run-clang-tidy` je Lauf
schreibt. Ist `N` kleiner als die Zahl der Dateien, die geheilt werden sollten,
ist das ein Abbruchgrund und kein Nebenbefund — warum, steht in §15.

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
| `src/shell/globalshortcuts.cpp:53` | `const QString application` → `QString application`, dazu `NOLINTNEXTLINE(misc-const-correctness)` mit Begründung | PO-Entscheidung 3. Die Variable wird zurückgegeben; das `const` verhinderte die Rückgabe per Verschiebung und kostete eine Kopie je Aufruf. Der Wert ändert sich nirgends — zwischen Zuweisung und `return` steht nur ein `isEmpty()`. **Eine Kopie weniger, sonst nichts.** Die Regel dahinter steht als Kommentar in `.clang-tidy`, damit die nächste Funktion dieser Bauart ihr folgt |

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

### 3.5 `const` **nicht gesetzt**, wo die Änderung über eine Qt-Verbindung kommt — 3 Stellen

PO-Entscheidung 9, ausführlich in §5. Alle drei stehen in `src/main.cpp` und
tragen `NOLINTNEXTLINE(misc-const-correctness)` mit Verweis auf Regel 2 in
`.clang-tidy`.

| Stelle | Änderung | Warum das Verhalten dasselbe bleibt |
|---|---|---|
| `src/main.cpp:50` | `KDBusService service` bleibt ohne `const` | **Das ist der Stand von `main` — die Änderung ist, dass er es bleibt.** Der Maschinenfix wollte `const` setzen; `activateRequested` wird auf diesem Objekt ausgelöst (`:63`), also verändert es sich. Kein `const` gesetzt heißt: keine Änderung am Verhalten, und die Marke hält den Prüfer davon ab, sie erneut vorzuschlagen |
| `src/main.cpp:69` | `LibraryWindow library` bleibt ohne `const` | dasselbe. `:74` und `:78` verbinden `&LibraryWindow::showLibrary` darauf, und der Slot ruft `reload()`, `show()`, `raise()`, `setFocus()`. **Am gebauten Stand nachgewiesen** (§7): `ShowLibrary()` über D-Bus öffnet das Fenster |
| `src/main.cpp:72` | `TrayIcon tray` bleibt ohne `const` | dasselbe. Das Objekt sendet `captureRequested` und `libraryRequested` und hält ein `KStatusNotifierItem`, das der Panel-Wirt bedient |

**Die Zeilen sind gegenüber `main` unverändert** — was hinzukommt, ist je eine
Kommentarzeile darüber. Ein Verhalten kann sich daran nicht ändern.

### 3.6 `NOLINT`-Blöcke ohne Codeänderung — 2 Stellen

`src/shell/shortcutregistration.cpp:33` (`desktopFileDeclaresAction`) und
`src/ui/librarywindow.cpp:153` (`placeholderPage`) bekommen je einen
Begründungsblock und eine `NOLINTNEXTLINE`. **Es ändert sich keine Codezeile**,
nur Kommentar. Siehe §4.

## 4. Jedes `NOLINT` mit Begründung, gezählt (AK 4)

**37 Marken in vier Klassen**, gezählt mit `git grep -E '^\s*// NOLINT' -- src tests`.

| Klasse | Prüfung | Zahl | Fundorte | Begründung |
|---|---|---|---|---|
| **1** (PO-Entscheidung 1) | `misc-const-correctness` | **8** | `tests/librarytest.cpp`, alle auf einer `QFETCH`-Zeile | `QFETCH` ist ein Makro und deklariert die Variable selbst (`Type name = *static_cast<Type *>(…)`). Ein `const` ist dort nur unterzubringen, indem man `QFETCH` aufgibt — die QTest-Redensart für datengetriebene Tests. **Eine** gemeinsame Begründung steht im Kopf der Datei, jede Marke verweist darauf |
| **2** (PO-Entscheidung 3) | `misc-const-correctness` | **1** | `src/shell/globalshortcuts.cpp` | An einer zurückgegebenen lokalen Variablen gewinnt `performance-no-automatic-move`. Siehe §3.3 |
| **3** (PO-Entscheidung 4) | `bugprone-easily-swappable-parameters` | **8** | `src/shell/shortcutregistration.cpp`, `src/ui/librarywindow.cpp`, `tests/editshots.cpp` (2), `tests/libraryshots.cpp`, `tests/librarytest.cpp`, `tests/readmeshots.cpp`, `tests/searchshots.cpp` | Heilen hieße Signaturen ändern oder eigene Typen einführen — Entwurf, nicht Aufräumen. Der sichtbare Fall (`placeholderPage`) bekommt stattdessen eine Testzusicherung als **#88** |
| **4** (PO-Entscheidung 9) | `misc-const-correctness` | **20** | `src/main.cpp` (3), `tests/librarytest.cpp` (6), `tests/shelltest.cpp` (5), `tests/captureshots.cpp`, `tests/editshots.cpp`, `tests/identitytest.cpp`, `tests/libraryshots.cpp`, `tests/readmeshots.cpp`, `tests/searchshots.cpp` (je 1) | Die Änderung kommt über eine Qt-Verbindung, die der Prüfer nicht sieht. Siehe §5. Die gemeinsame Begründung steht als Regel 2 in `.clang-tidy`, jede Marke nennt sie |
| | | **37** | verteilt auf 12 Dateien | |

Nach Prüfung: **29 `misc-const-correctness`** (8 + 1 + 20) und
**8 `bugprone-easily-swappable-parameters`**.

**Zwei Fallen beim Zählen und beim Setzen, beide gemessen.**

**Eine Zählung, die den Fließtext der eigenen Begründung mitzählt, meldet 18
statt 17.** Im Kopf von `librarytest.cpp` steht die gemeinsame Begründung der
ersten Klasse, und darin kommt das Wort `NOLINTNEXTLINE` vor. Ein
`git grep -o 'NOLINT'` zählt es mit. Die Zählung sieht deshalb nur auf Zeilen,
die mit der Marke **beginnen** (`^\s*// NOLINT`). Dieselbe Familie wie
`grep -h` ohne Dateinamen und wie „für 0 von 49 Dateien" (§15).

**Die ersten acht Marken der dritten Klasse haben nichts unterdrückt.** Sie
standen als **erste** Zeile eines fünfzeiligen Kommentarblocks — und
`NOLINTNEXTLINE` gilt für die unmittelbar folgende Zeile, die dort eine weitere
Kommentarzeile war. Der Linterlauf zeigte es sofort; ein Lauf, der nur den
Rückgabewert gelesen hätte, nicht. Seither steht die Marke als **letzte** Zeile
des Blocks.

## 5. Der vierte Fall — PO-Entscheidung 9, entschieden und umgesetzt

`misc-const-correctness` schlägt `const` auf **Objekten** vor, die über eine
Qt-Verbindung verändert werden. Der Prüfer sieht diese Änderung nicht:
`QObject::connect`, `QSignalSpy` und `QTest::qExec` nehmen ihre Ziele als
`const QObject *` und `const_cast`en innen, und eine im Konstruktor gefangene
Lambda behält ein nicht-`const`es `this`, wie das Objekt auch deklariert sei.
Ein `const` definiertes Objekt während seiner Lebensdauer zu verändern, ist
nach [dcl.type.cv]/4 undefiniert, und der Übersetzer darf den Lesezugriff auf
den Anfangswert zurückfalten.

Der Fall ist als vierter an den PO gegangen, wie AK 4 es verlangt, und **am
05.08.2026 als Entscheidung 9 entschieden**:

> Auf einem Objekt, das auf diesem Weg verändert wird, wird `const` nicht
> gesetzt; die Stelle trägt `NOLINT(misc-const-correctness)` mit gemeinsamer
> Begründung, und die Regel steht als Kommentar in `.clang-tidy` neben der aus
> Entscheidung 3.

**Umgesetzt an 20 Stellen** (3 in `src/`, 17 in `tests/`). Die drei Fälle, an
denen das keine Sprachjuristerei ist:

1. **`tests/librarytest.cpp:3684` und `:3719` — `DialogWatch watch;`.** Der
   Konstruktor hängt eine Lambda an einen Timer, die `m_appeared = true;`
   schreibt; `:3692` und `:3733` prüfen `QVERIFY2(!watch.appeared(), …)`.
   **Faltet der Übersetzer `m_appeared` auf seinen Anfangswert, sind beide
   Zusicherungen bedingungslos grün.**
2. **`src/main.cpp:69` — `LibraryWindow library(&store);`**, und `:74`/`:78`
   verbinden `&LibraryWindow::showLibrary` darauf. Der Slot ruft `reload()`,
   `show()`, `raise()` und `setFocus()`. Ein benannter verändernder Aufruf auf
   einem `const` definierten Objekt, am Typsystem vorbeigeführt.
3. **`tests/captureshots.cpp`, `tests/identitytest.cpp` u. a. —
   `QApplication app(argc, argv);`** vor `QTest::qExec()`. Ein
   Anwendungsobjekt, das eine Ereignisschleife fährt, verändert sich.

**Zeiger bleiben unberührt** — `const QPushButton *edit` qualifiziert nur
unseren Zeiger, nicht das Objekt, und wir lesen nur darüber (§3.1).

### Zwei Abweichungen von dem, was der PO wörtlich aufgetragen hat

**Es sind 20 Stellen und nicht 16, und damit 37 `NOLINT` und nicht 33.** Die 16
waren meine eigene Liste, und die stammte aus dem Maschinendiff. Eine
vollständige Erhebung findet **vier weitere**, die schon vor dieser Story
`const` waren: `const QApplication app(argc, argv);` in `tests/editshots.cpp`,
`tests/libraryshots.cpp`, `tests/readmeshots.cpp` und `tests/searchshots.cpp`.
Der PO hat eine davon selbst genannt (`editshots.cpp:219`), was zeigt, dass die
**Regel** gemeint war und nicht der Diff. Sie stehen alle in der Dateimenge
dieser Story. Sie stehenzulassen hieße, `const QApplication app` in
`libraryshots.cpp` neben ein mit `NOLINT` versehenes in `captureshots.cpp` zu
setzen — dieselbe Zeile, zwei Urteile. **Wer die vier lieber draußen hätte,
sagt es; es sind vier Zeilen zurück.**

**Die Regel steht in `.clang-tidy` etwas weiter gefasst als „QObject-Abkömmling".**
`DialogWatch` — der stärkste Fall überhaupt — **ist kein QObject**: eine
gewöhnliche Klasse mit einem `QTimer`-Feld. Der Mechanismus ist derselbe (die
Änderung kommt über eine Qt-Verbindung), die Vererbung ist es nicht. Die Regel
spricht deshalb von einem Objekt, das **auf diesem Weg** verändert wird,
„gleich ob es von QObject abstammt".

### Die Lehre des Falls

**Der Vorprüfbericht hat diesen Fix an `const LibraryWindow library;` gemessen
und festgestellt, dass er kompiliert** (Falle F2). Das war richtig gemessen und
beantwortete die falsche Frage — kompilierbar heißt hier nicht gefahrlos, und
zwar aus demselben Grund, aus dem der Prüfer die Änderung nicht sieht: Der
Empfängerparameter von `connect` ist `const`-qualifiziert.

**Ich habe versucht, das hart zu messen, und es ist mir nicht gelungen.** Ein
Release-Bau (`-O2`) ist der Ort, an dem ein Übersetzer die Konstantheit
ausnutzen dürfte; die beiden `DialogWatch`-Tests laufen dort **grün**. Das
entkräftet das Argument nicht — undefiniertes Verhalten, das heute nicht
zubeißt, bleibt undefiniertes Verhalten, und der Übersetzer darf seine Meinung
mit jeder Version ändern —, aber es ist ehrlich zu sagen, was der Kern von §5
ist: **ein Argument aus der Sprachnorm und dem gelesenen Änderungsweg, kein
Messwert.** Ein Lauf, der das Gegenteil zu belegen schiene, wäre hier ebenfalls
keiner: Ein grüner Test beweist bei undefiniertem Verhalten nur, dass dieser
Übersetzer an diesem Tag so entschieden hat.

**Und die Wette ist einseitig.** Verlieren wir sie, verlieren wir zwei
Zusicherungen **still**. Gewinnen wir sie, sparen wir zwanzig Kommentarzeilen.

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

## 7. Die Selbst-Sichtprüfung am gebauten Stand (DoD 2)

**Vom PO freigegeben, weil dafür der installierte Dienst zu beenden war.** Ohne
das reicht die Einzelinstanz-Weiche von `KDBusService` den Start weiter, und
man prüft den installierten Stand statt des gebauten (B16).

**Warum sie bei einer Aufräumstory nicht entfällt:** Diese Story fasst
`src/main.cpp` an (Lebensdauer und `const` der drei Dienstobjekte) und
`src/shell/globalshortcuts.cpp` (Registrierung und Rücklesen des Kürzels). Das
sind die beiden Stellen, an denen „nur Aufräumen" aufhört, eines zu sein.

Sitzung vorher abgefragt und nicht angenommen: `Type=wayland`, `Active=yes`,
**`LockedHint=no`** — bei gesperrter Sitzung liefert `spectacle` ein schwarzes
Bild mit Rückgabe 0, und der Lauf hätte den Rollladen fotografiert.

| Schritt | Beleg |
|---|---|
| vorher | PID 4029, `exe=/usr/bin/denkzetteld` |
| gebauten Stand gestartet | PID 502543, `exe=…/denkzettel-76/build/bin/denkzetteld` |
| Meldungen | **stderr leer** — kein „Kürzel nicht einsatzbereit", kein „Exporting org.denkzettel.Daemon failed". `registerCaptureShortcut()` hat also `Reached` erreicht |
| Kürzel **beim Dienst zurückgelesen** | `a(ai) 1 4 268435534 0 0 0` — eine Sequenz, `268435534 = 0x1000004E = Qt::META \| Qt::Key_N`, also **Meta+N** |
| D-Bus | `org.denkzettel.Daemon` auf dem Bus, `/Daemon` mit `AddNote`, `Quit`, `ShowCapture`, `ShowLibrary` |
| **Hauptweg** | `ShowLibrary()` über D-Bus, `rc=0`, Dienst lief unverändert weiter |
| nachher | PID 505843, `exe=/usr/bin/denkzetteld`, **kein `(deleted)`**, Kürzel erneut als Meta+N zurückgelesen |

**Das Rücklesen des Kürzels ist genau der Beleg, den der Rückgabewert von
`setGlobalShortcut()` nicht liefern kann** — der Grund für die Streichung in
`.clang-tidy` (§9). Und **`ShowLibrary()` ist genau der Weg aus Entscheidung 9**:
`DaemonService::libraryRequested` hängt in `src/main.cpp:78` an
`&LibraryWindow::showLibrary`, und `library` ist das Objekt, an dem der
Maschinenfix ein `const` setzen wollte.

Vom Sitzungsbild des aktiven Fensters abgelesen: Die Bibliothek steht offen,
mit Kopfzeile „Denkzettel — Bibliothek", Volltextsuchfeld, gruppierter Liste
(„Gestern", „Letzte Woche") mit Zeitstempeln und dem Leerzustand des
Lesebereichs („Keine Notiz ausgewählt / Zum Lesen links eine Notiz auswählen.").

**Das Bild liegt nicht im Repositorium.** Es zeigt die echten Notizen des
Kunden, und das Repositorium ist öffentlich (Kundenentscheidung 02.08.2026).
Belegt ist hier die Beobachtung, nicht das Bild. Belege:
`messungen/08-sichtpruefung.txt`.

## 8. Die CI-Wache prüft die Dreizahl (AK 6)

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

## 9. Die Schwelle steht nicht mehr allein in einer YAML-Datei (AK 7)

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

## 10. `.clang-tidy` — was sich geändert hat und warum (AK 5)

Alles drei steht als Kommentar in der Datei selbst — das ist die Bedingung aus
AK 5: Eine Prüfung oder eine Option zu ändern, verschiebt die Messlatte, und
ohne den Vermerk misst ein späterer Nachweis „0 Befunde" etwas anderes als
heute.

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

**Ergänzt: zwei Regeln zu `misc-const-correctness`**, als Kommentar über der
`Checks`-Zeile, damit die nächste Funktion dieser Bauart ihnen folgt. Sie stehen
dort, weil der Prüfer über die Sprache recht hat und über diese Codebasis
nicht:

- **Regel 1** (PO-Entscheidung 3): An einer zurückgegebenen lokalen Variablen
  gewinnt `performance-no-automatic-move`.
- **Regel 2** (PO-Entscheidung 9): Auf einem Objekt, das über eine
  Qt-Verbindung verändert wird, wird `const` nicht gesetzt — gleich ob es von
  QObject abstammt. Zeiger sind nicht betroffen.

Die zwanzig Marken der vierten Klasse nennen Regel 2 im Text, statt die
Begründung zwanzigmal zu wiederholen.

**Die Prüfliste selbst ist unverändert.** Keine Prüfung wurde abgewählt —
insbesondere nicht `misc-const-correctness` für `tests/`, was 52 Befunde
mitgenommen hätte, die diese Story gerade sinnvoll heilt.

## 11. Was diese Story nicht angefasst hat

- **`tests/spellfixspike.cpp`** (PO-Entscheidung 5). Die drei Befunde darin
  sind gemessen und in §6 benannt.
- **Signaturen und neue Typen** gegen `bugprone-easily-swappable-parameters`
  (PO-Entscheidung 4). Der sichtbare Fall ist **#88**.
- **Kein Verhalten.** Diese Story ändert Schlüsselwörter, Basistypen und
  Kommentare. Wo eine Änderung mehr wäre als das, steht sie einzeln in §3.

## 12. Was hier nicht prüfbar war

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
4. **Dass ein `const` auf einem QObject wirklich zubeißt** (§5), ist nicht
   messbar — ein Übersetzer *darf* die Konstantheit ausnutzen, er *muss* nicht.
   Der Versuch mit `-O2` ist in §5 protokolliert, samt seinem negativen
   Ausgang.

## 13. Der Bildvergleich — 27 von 42 Bildern bytegleich, die übrigen 15 erklärt

**Die Story behauptet nichts über das Aussehen, aber sie fasst den Zeichenweg
an**: `NoteListDelegate::paint()` bekommt einen `const QStyle *`, und fünf der
sieben Enums mit neuem Basistyp stecken in der Bibliotheksansicht
(`NoteGroup`, `Role`, `Selection`, `UnsavedAnswer`, `Note::Type`). Ein grünes
`ctest` deckt das nur zum Teil. Deshalb: alle fünf Bildläufer gegen eine frisch
gebaute Debug-Kopie von `main` gehalten, beide mit
`QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=2` — die
Skalierung, mit der auch die README-Bilder entstehen.

| Läufer | Bilder | bytegleich |
|---|---|---|
| `editshots` | 5 | **5** |
| `searchshots` | 6 | **6** |
| `readmeshots` | 2 | **2** |
| `captureshots` | 14 | **14** |
| `libraryshots` | 15 | 12 |
| | **42** | **27 direkt** |

**Die drei übrigen sind nicht meine Änderung.** Fünf Läufe **desselben**
Binärcodes von `main` liefern für genau diese drei Bilder je **zwei**
Fassungen; die anderen zwölf sind stabil. Jedes der 15 Bilder meines Zweiges
liegt in der Menge der Fassungen, die `main` selbst erzeugt.

Damit ist der Zeichenweg belegt, soweit ein Bild ihn belegen kann — und die
Grenze aus B21 bleibt: offscreen zeichnen weder Theme noch Compositor
vollständig. Für diese Story genügt es, weil kein Akzeptanzkriterium über
Hülle, Rundung, Kontur, Schatten oder Durchsichtigkeit etwas behauptet.

Beleg: `messungen/07-bildvergleich.txt`.

## 14. Zwei Befunde außerhalb meiner Fläche — gemeldet, nicht geheilt

**`ctest` ist im Release-Bau rot, und zwar auf `main`, schon vor #76.**
Gemessen, weil ich §5 hart belegen wollte:

| Bau | Stand | `ctest` |
|---|---|---|
| `Debug` | `main` und dieser Zweig | 9/9 grün |
| `Release` | **`main`** | **8/9** — `librarytest` fällt |
| `Release` | dieser Zweig | **8/9** — dieselben zwei Fälle |
| `RelWithDebInfo` | `main` | **8/9** — dieselben zwei Fälle |

Es sind zweimal dieselben Fälle, in zwei Läufen wiederholbar:

- `LibraryTest::showsCategoryAndTagsAsPlainDisplayWhileEditing()` — die
  Kategorie „Software-Ideen" steht nicht in der Anzeige
  (`tests/librarytest.cpp:2942`).
- `LibraryTest::keepsCategoryTagsAndStateWhileSaving()` — `saved->category` ist
  leer statt „Software-Ideen" (`tests/librarytest.cpp:3060`).

**Diese Story hat das nicht verursacht** — der Kontrolllauf auf `main` zeigt
dasselbe Bild. Aufgefallen ist es nur, weil hier zum ersten Mal jemand mit
`-O2` gebaut hat; DoD 1 und der CI-Lauf bauen beide `Debug`. Nicht meine
Fläche, keine Zeile angefasst.

**`libraryshots` arbeitet nicht deterministisch, und das README behauptet das
Gegenteil.** README.md sagt: *„Der Läufer arbeitet deterministisch: Zwei Läufe
hintereinander liefern bytegleiche Dateien."* Gemessen an fünf Läufen desselben
Binärcodes auf `main`: **3 von 15 Bildern kommen in zwei Fassungen.**

Die Ursache ist eng lokalisiert — in allen drei Fällen **70 Bildpunkte** auf
einer Fläche von zwei Bildpunkten Breite und 35 Höhe:

| Bild | Bereich |
|---|---|
| `02-leerzustand.png` | x 32–33, y 30–64 (im Suchfeld) |
| `10c-schema-dunkel-bearbeiten.png` | x 1061–1062, y 212–246 (im Editor) |
| `10d-schema-hell-bearbeiten.png` | dieselbe Stelle |

Das ist ein **blinkender Textcursor**, aufgenommen in verschiedenen Phasen des
Blinkens. **Warum das über eine Schönheitsfrage hinausgeht:** Auf
Bildvergleichen ruht DoD 3. Wer zwei Stände nebeneinanderhält, sieht bis zu
drei Unterschiede, die keine sind — und gewöhnt sich daran, Unterschiede
wegzuerklären. Das ist die Bauart, an der dieses Projekt seine wertlosen
Belege gefunden hat. Nicht meine Fläche, keine Zeile angefasst.

**Ein Ordner `--help` lag in der Wurzel des Repositoriums** mit acht PNG aus
einem Bildlauf (`ecke-*.png`, `fenster-*.png`). Alle drei Bildläufer nehmen
jedes Argument als Zielverzeichnis; der PO hat den Ordner entfernt und die
Ursache als **#98** gebucht.

**Berichtigung, weil sie die Bewertung verändert hätte:** Ich hatte gemeldet,
der Ordner sei **nicht versioniert**. Er war es. Mein Prüfausdruck war falsch —
`git ls-files | grep -- '^--help$'` sucht nach einer *Datei* dieses Namens, und
`git ls-files` führt Verzeichnisse gar nicht auf; richtig gewesen wäre
`grep -- '^--help/'`, was die acht PNG sofort zeigt. Ein leeres Suchergebnis
sah aus wie ein Befund und war ein Fehler in der Frage. Der Ordner war seit dem
05.08.2026 gepusht und damit veröffentlicht.

## 15. Ein Fund, der in die Fallenliste gehört

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
