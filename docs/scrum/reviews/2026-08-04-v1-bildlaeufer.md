# V1 umgesetzt: Bildläufer im Normalbau, `lint-clazy` wieder brauchbar

**Datum:** 04.08.2026, 10:06 (Ganymed) · **Umsetzer:** Entwickler (Agent
`denkzettel-dev`) · **Auftraggeber:** Product Owner, nach Kundenentscheidung vom
04.08.2026 zu V1 aus `docs/scrum/reviews/2026-08-04-workflow-optimierungen.md`.

**Prüfgegenstand:** `main` @ `9d222d2` zuzüglich der beiden unten genannten
Änderungen. **Geändert wurden zwei Dateien** — `CMakeLists.txt` und
`tests/CMakeLists.txt` — plus dieser Bericht. Nichts sonst; insbesondere kein
`src/`, kein Test, keine README.

**Methode.** Jede Zahl steht auf einem Befehl, den ich selbst gefahren habe.
Gebaut wurde in Bauordnern **außerhalb** des Projektbaums (es arbeiten weitere
Agenten parallel, `build/` bleibt unangetastet). Der Vorher-Stand ist kein
Gedächtnisprotokoll, sondern ein frischer Export: `git archive HEAD | tar -x -C
<vorher-baum>`, gemessen vor den Änderungen und danach noch einmal am
unveränderten Export. Der Arbeitsbaum trug während der Messung fremde
Änderungen an `CLAUDE.md`, `docs/scrum/PROZESS.md` und
`.claude/agents/denkzettel-dev.md` — keine davon geht in einen Compilerlauf ein.

Beobachtung und Schluss sind getrennt.

---

## 1 · Was geändert wurde

### 1.1 `tests/CMakeLists.txt` — `EXCLUDE_FROM_ALL` gestrichen (V1)

Bei allen **vier** Bildläufern: `searchshots`, `libraryshots`, `editshots`,
`readmeshots`. Der erklärende Kommentar über der Gruppe ist um den Grund
ergänzt (englisch, wie die übrigen Kommentare dieser Datei).

**Was ausdrücklich geblieben ist:** kein `add_test()`. Die im Quelltext
festgehaltene Begründung (*„a broken screenshot writer must not colour the suite
red"*) trägt weiterhin und wird von dieser Änderung nicht berührt. Nachweis
unten, Punkt 2.2.

### 1.2 `CMakeLists.txt` — `tests/spellfixspike.cpp` folgt seinem Ziel (N1)

Die Linterliste entsteht aus `file(GLOB … tests/*.cpp)`. Neu wird genau eine
Datei wieder entfernt, solange `DENKZETTEL_SPIKE_SPELLFIX` aus ist:

```cmake
if(NOT DENKZETTEL_SPIKE_SPELLFIX)
    list(REMOVE_ITEM DENKZETTEL_LINT_TEST_SOURCES
         ${CMAKE_CURRENT_SOURCE_DIR}/tests/spellfixspike.cpp)
endif()
```

Die Bedingung ist dieselbe, unter der `tests/CMakeLists.txt:13` das Ziel baut —
und damit dieselbe, unter der `spellfixspike.moc` überhaupt entsteht. Der
Schalter ist eine `option()` in `src/CMakeLists.txt:15` und wird durch
`add_subdirectory(src)` gesetzt, bevor die Linterliste gebaut wird.

---

## 2 · Die geforderten Nachweise

### 2.1 Bauzeit vorher und nachher

Verfahren je Lauf: `rm -rf <bau>` · `cmake -B <bau> -S <quelle>
-DCMAKE_BUILD_TYPE=Debug` · `cmake --build <bau> -j N`, Zeit um den Bauschritt
gelegt.

| Lauf | vorher | nachher | Zuschlag |
|---|---|---|---|
| Vollbau aus leerem Baum, `-j 12` | 9,67 · 9,62 · 9,73 s | 9,75 · 9,83 · 9,82 s | **+0,15 s** (Median) |
| Vollbau aus leerem Baum, `-j 4` | 13,34 · 13,36 s | 16,87 · 16,55 s | **+3,4 s** (+25 %) |
| Bau ohne Änderung danach | 0,17 s | 0,17 s | **0 s** |

**Die im Ausgangsbericht genannten 7 s bestätige ich der Größe nach, aber sie
beantworten eine andere Frage.** Als *eigener, nachgelagerter Schritt* gemessen
— `cmake --build <bau> --target editshots libraryshots searchshots readmeshots
-j 12` auf einem bereits fertigen Bau — kosten die Läufer **10,1 s** (vier
statt drei Ziele). Genau diesen Betrag zahlt heute, wer die Ermahnung aus
`CLAUDE.md` befolgt.

**Schluss.** Im Vollbau fällt dieser Betrag fast vollständig weg, weil die vier
Übersetzungen bei `-j 12` in ohnehin freie Plätze rutschen. **Die Ersparnis ist
also kein Nulltarif, sondern geliehene Parallelität** — der `-j 4`-Lauf zeigt,
was der Zuschlag kostet, wenn keine freien Plätze da sind. Für die CI (dort
`-j $(nproc)` auf einem kleineren Läufer) ist die untere Zeile die
einschlägige, nicht die obere. In beiden Fällen bleibt der Zuschlag unter der
Stop-Bedingung des Vorschlags (*„spürbar länger als 7 s"*).

### 2.2 `ctest` bleibt 7/7

```
ctest --test-dir <bau>
100% tests passed out of 7
Total Test time (real) =   5.75 sec
```

Kein Bildläufer ist in die Suite geraten; die Zahl der Tests ist unverändert.

### 2.3 Ein gewöhnlicher Bau erzeugt die vier Läufer

`cmake --build <bau> -j 24` ohne jedes `--target`, danach `<bau>/bin`:

```
capturetest  denkzetteld  editshots  firstruntest  libraryshots
librarytest  readmeshots  searchshots  shelltest  storetest
```

Vorher standen dort sechs Dateien, die vier `*shots` fehlten. Im Baulog stehen
`Built target searchshots`, `libraryshots`, `readmeshots`, `editshots`.

**Zusätzlich geprüft, weil ein gebautes Programm noch kein laufendes ist:** Alle
vier ausgeführt mit `QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde
<bau>/bin/<läufer> <zielordner>` — `rc=0` bei allen vieren, geschrieben wurden 6
(`searchshots`), 13 (`libraryshots`), 5 (`editshots`) und 2 (`readmeshots`)
Bilder. Die Bilder selbst sind Wegwerfware dieser Prüfung und liegen nicht im
Repository; Gegenstand war die Lauffähigkeit, nicht der Bildinhalt.

### 2.4 `lint-clazy` liefert jetzt einen brauchbaren Rückgabewert

| | vorher | nachher |
|---|---|---|
| Rückgabewert | **2** | **0** |
| Laufzeit | 49 s | 47 s |
| harter Fehler | `tests/spellfixspike.cpp:509:10: fatal error: 'spellfixspike.moc' file not found` | keiner |
| `no such include directory` (Autogen) | 4 Verzeichnisse, 5 Meldungen | **0** |
| echte Befunde | 3 | **3** (dieselben) |

Die drei Befunde sind unverändert und liegen außerhalb meiner Fläche:

```
tests/librarytest.cpp:2336:5  range-loop might detach Qt container (QList)
tests/librarytest.cpp:2342:5  range-loop might detach Qt container (QList)
tests/shelltest.cpp:361:24    Don't call QList::first() on temporary
```

**Antwort auf die gestellte Frage:** Er wird **0**. `clazy-standalone` bewertet
diese drei als Warnungen, nicht als Fehler; der Rückgabewert 2 kam allein vom
fehlenden `.moc`. Als Tor taugt der Rückgabewert damit ab sofort — er sagt jetzt
„die Analyse ist durchgelaufen" statt „die Analyse ist abgestürzt".

**Beide Wirkungen getrennt gemessen**, damit die Zuordnung nicht auf Plausibilität
ruht. Kontrolllauf auf dem **unveränderten** Vorher-Baum, in dem ich die vier
Läufer von Hand gebaut habe:

| Zustand | `no such include` | fatal error | rc |
|---|---|---|---|
| vorher, Läufer ungebaut | 4 | ja | 2 |
| vorher, Läufer von Hand gebaut (Kontrolle) | **0** | ja | **2** |
| nachher (V1 + N1) | 0 | nein | **0** |

Die Autogen-Meldungen hängen also nachweislich am Bauen der Läufer (V1), der
Rückgabewert am `.moc` (N1). Keine der beiden Änderungen allein hätte gereicht.

**Gegenprobe zur Bedingung:** `cmake -B <bau> -S . -DDENKZETTEL_SPIKE_SPELLFIX=ON`
nimmt `tests/spellfixspike.cpp` wieder in die Linterliste auf (nachgesehen in der
erzeugten `build.make`-Regel des Ziels `lint-clazy`); bei `OFF` steht sie dort
nicht. Die Datei ist nicht dauerhaft ausgeschlossen, sondern an ihr Ziel gebunden.

### 2.5 `lint-tidy`: 73 eindeutige Befunde, vorher wie nachher

| | vorher | nachher |
|---|---|---|
| Rückgabewert | 0 | 0 |
| Laufzeit | 6 s | 6 s |
| Rohzeilen `warning:` | 130 | 130 |
| eindeutig (`Datei:Zeile:Spalte` + Prüfung) | **73** | **73** |
| analysierte Dateien | 25 von 40 in der Compile-Datenbank | 25 von 40 |

Aufteilung (unverändert): 64× `performance-enum-size`, 55×
`misc-const-correctness`, 8× `bugprone-easily-swappable-parameters`, je 1×
`performance-no-automatic-move`, `bugprone-unused-return-value`,
`bugprone-implicit-widening-of-multiplication-result`.

**Die 72 des Ausgangsberichts sind bestätigt, nicht widerlegt.** Der Unterschied
ist eine einzige Zeile und hat einen Namen: Der Bericht maß an `5294094`, seither
ist `tests/readmeshots.cpp` dazugekommen (`git diff --stat 5294094..HEAD -- src
tests` zeigt genau diese Datei und `tests/CMakeLists.txt`). Sie trägt
`readmeshots.cpp:52` — `bugprone-easily-swappable-parameters`. 72 + 1 = 73.

### 2.6 Der Bau bleibt warnungsfrei

Die Warnungsschwelle der CI nachgestellt, am geänderten Stand:

```
set -o pipefail
cmake --build <bau> -j 24 2>&1 | tee bau.log
grep -c 'warning:' bau.log   →   0
```

**Kein Bildläufer erzeugt eine Compiler-Warnung.** Damit halten sie die
Nullschwelle, die seit heute früh jeden Lauf durchfallen lässt. Es gab hier
nichts zu melden und nichts zu ändern.

---

## 3 · Zwei Berichtigungen am Ausgangsbericht

Gemessen, nicht vermutet — beide betreffen A3.

**3.1 „Beide Linter melden … `no such include directory`" trifft nur auf einen
zu.** `lint-clazy` meldet es (vier Verzeichnisse), `lint-tidy` **nie** — weder
vorher noch nachher, weder als Warnung noch als Fehler. Der Grund ist messbar:
`run-clang-tidy` gleicht die übergebenen Pfade gegen die Compile-Datenbank ab
und arbeitet nur deren Treffer ab (*„for 25 files out of 40"*).
`clazy-standalone` nimmt die Dateiliste wörtlich.

**3.2 Dieselbe Bauart trifft `lint-tidy` auch bei `spellfixspike` nicht** — und
zwar aus demselben Grund. Ohne den Spike-Schalter existiert das Ziel nicht, also
steht die Datei **nicht in der Compile-Datenbank** (`grep -c spellfixspike
<bau>/compile_commands.json` → 0), und `run-clang-tidy` lässt sie stillschweigend
aus. Deshalb bleibt die Zahl 25 von 40 vor und nach meiner Änderung gleich: Der
`REMOVE_ITEM` nimmt `lint-tidy` nichts weg, was es je angesehen hätte.

**Schluss.** Die Frage aus dem Auftrag (*„prüfe, ob dieselbe Bauart auch
`lint-tidy` trifft"*) ist mit **nein** beantwortet, und die Antwort ist nicht
Glück: `lint-tidy` hat eine Filterstufe, die `lint-clazy` fehlt. Wer künftig eine
Datei aus der Linterliste nimmt, ändert damit **nur** das Verhalten von
`lint-clazy`.

---

## 4 · Zum Melden an den PO (melden, nicht heilen)

Alles außerhalb meiner Dateimenge, von mir nicht angefasst:

- **M1 · Die README stimmt nicht mehr.** Abschnitt „Bauen und Testen" verlangt,
  den Bildläufer ausdrücklich zu bauen. Nach V1 tut das jeder gewöhnliche Bau.
- **M2 · Der Kopfkommentar von `.github/workflows/ci.yml` stimmt nicht mehr.**
  Er hält fest, die Bildläufer seien `EXCLUDE_FROM_ALL` und *„werden hier nicht
  gebaut"* — ab sofort werden sie es. Zwei Folgen: Die CI-Bauzeit steigt um den
  in 2.1 für `-j 4` gemessenen Betrag, und die Warnungsschwelle deckt jetzt vier
  Dateien mehr ab (gemessen warnungsfrei, siehe 2.6). Beides ist erwünscht; der
  Kommentar behauptet nur das Gegenteil.
- **M3 · Die Ermahnung in `CLAUDE.md`** (*„Vor jedem Bildbeleg: `cmake --build
  build --target <läufer>`"*) ist gegenstandslos geworden. Die Stop-Bedingung des
  Vorschlags sieht ihre Streichung nach zwei Sprints ohne Bildvorfall vor — die
  Entscheidung liegt nicht bei mir.
- **M4 · Die drei `lint-clazy`-Befunde** (2.4) liegen in `tests/librarytest.cpp`
  und `tests/shelltest.cpp` und sind seit heute erstmals als Befunde *sichtbar*,
  weil sie vorher hinter einem abgestürzten Lauf standen. Sie sind unberührt.
- **M5 · V3 hat noch eine Vorarbeit.** Die erste (`lint-clazy` als Tor
  brauchbar) ist mit diesem Bericht erledigt. Die zweite — die 73
  `lint-tidy`-Befunde heilen oder als Grundlinie einfrieren — steht unverändert
  offen und ist keine Dev-Entscheidung.

---

## done / next

**done:** V1 an allen vier Läufern umgesetzt, N1 an der Wurzel geheilt; sechs
Nachweise geführt, jeder mit seinem Befehl; die Wirkung von V1 und N1 durch
einen Kontrolllauf getrennt; die Bedingung des `REMOVE_ITEM` durch eine
Gegenprobe mit eingeschaltetem Schalter bestätigt; zwei Aussagen des
Ausgangsberichts berichtigt; fünf Meldungen an den PO.

**next:** PO — M1 bis M3 sind Textstellen, die heute falsch sind; M5 ist eine
Entscheidung. Nicht committet, nicht gepusht.
