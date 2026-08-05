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

*(Takt 1, vor der Abnahme — wird beim Sprint-Ende gefüllt)*

## 7. Mängelliste

*(wird beim Sprint-Ende gefüllt)*

## 8. done/next

*(wird beim Sprint-Ende gefüllt)*
