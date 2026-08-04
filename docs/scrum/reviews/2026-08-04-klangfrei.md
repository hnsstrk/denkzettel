# Umsetzung W1: Testläufe ohne Systemklang

Datum: 04.08.2026 · Umsetzung der Kundenentscheidung vom 04.08.2026 zu
`docs/scrum/reviews/2026-08-04-testklaenge.md` (Weg W1, zentral).
Nicht committet — der PO committet.

## 1. Was geändert wurde

| Datei | Änderung |
|---|---|
| `tests/testsilence.cpp` | **neu** — setzt `CANBERRA_DRIVER=null` vor `main()` |
| `tests/CMakeLists.txt` | Objektbibliothek `testsilence` + `link_libraries()` am Dateianfang |
| `SPEC.md`, Abschnitt 9 | Nachtrag zur entdeckten Bedingung (DoD 4/B9) |

**Kein Produktivcode berührt**, keine Bildläufer-`main()` geändert, keine
`ENVIRONMENT`-Zeile eines Tests angefasst. `git status` zeigt außer diesen
Dateien und dem Berichtsordner nichts von diesem Strang.

## 2. Der Entwurf

> **Nachtrag 04.08.2026:** Die Bauart innerhalb der Datei ist inzwischen eine
> andere — das statische Objekt hat die CI rot gemacht. Der Entwurf im Großen
> (Objektbibliothek, `link_libraries()` am Dateikopf) gilt unverändert.
> Siehe Abschnitt 6.

Eine Objektbibliothek `testsilence` mit einem statischen Objekt, dessen
Konstruktor `qputenv("CANBERRA_DRIVER", "null")` ruft. Der Konstruktor läuft
**vor `main()`**; libcanberra liest die Variable erst beim Öffnen seines
Kontexts, also beim ersten Klangversuch — lange danach. Eingehängt wird sie
mit **einer** Zeile am Kopf von `tests/CMakeLists.txt`:

```cmake
add_library(testsilence OBJECT testsilence.cpp)
target_link_libraries(testsilence PRIVATE Qt6::Core)
link_libraries(testsilence)
```

`link_libraries()` gilt für jedes Ziel, das dieses Verzeichnis **danach**
anlegt. Damit erbt jeder heutige und jeder künftige Test *und* jeder
Bildläufer die Stille, ohne dass sein Autor davon wissen muss.

**Eine Objektbibliothek, keine statische Bibliothek:** Aus einem Archiv zieht
der Binder nur Objekte, deren Symbole jemand braucht — und niemand ruft hier
etwas auf. Objektbibliotheken werden vollständig einkopiert. Nachgeprüft, nicht
angenommen: in allen neun Zielen steckt die Objektdatei, und das Symbol steht
im gelinkten Programm (§3, Nachweis 0).

**Die Sollbruchstelle sichtbar statt kommentiert.** Die Regel „nichts oberhalb
dieser Zeile" ist die einzige, die ein künftiger Autor verletzen kann — und sie
verletzt sich nur, wenn jemand ein Ziel *über* den Dateianfang schreibt. Der
natürliche Ort für einen fünften Läufer ist das Dateiende; von dort erbt er.
Der Kommentar an der Zeile sagt genau das.

### Verworfen: die zwei Hälften (ctest-`ENVIRONMENT_MODIFICATION` + `qputenv` je Läufer)

Der Vorschlag aus der Diagnose, den ich geprüft und verworfen habe:
`ENVIRONMENT_MODIFICATION` als Sammelzeile am Dateiende über
`get_property(… DIRECTORY PROPERTY TESTS)`, dazu eine Zeile in jeder
Bildläufer-`main()`.

Gegen ihn spricht genau der Punkt, den der Auftrag betont:

- **Er erbt nur halb.** Die CMake-Hälfte erbt (Sammelzeile am Ende), die
  C++-Hälfte nicht: Ein fünfter Bildläufer bekommt seine `qputenv`-Zeile nur,
  wenn sein Autor daran denkt. Genau dieser Fall ist in diesem Projekt schon
  eingetreten — der vierte Läufer entstand zwei Tage nach dem
  `EXCLUDE_FROM_ALL`-Vorfall nach demselben Muster, mit der Ermahnung im
  Kontext.
- **Zwei Mechanismen für eine Sache.** Zwei Stellen, an denen es schiefgehen
  kann, zwei Stellen zu pflegen. Der gewählte Entwurf deckt beide Wege mit
  einem.
- **Die Sammelzeile am Dateiende ist zerbrechlicher als eine am Dateianfang.**
  Ein neuer Test wird ans Ende einer Datei geschrieben — also mit einiger
  Wahrscheinlichkeit *unter* die Sammelzeile, wo sie ihn nicht mehr erfasst.
  Beim `link_libraries()` am Anfang zeigt die natürliche Schreibrichtung in die
  richtige Richtung.

Vorteil des verworfenen Wegs, der Vollständigkeit halber: Er lässt die
Testprogramme unberührt — die Stille stünde ausschließlich im Buildsystem.
Der gewählte Weg legt dafür ein Objekt in jedes Testprogramm, das eine
Umgebungsvariable setzt. Das ist der Preis; er ist eine Datei groß und wirkt
nur unter `tests/`.

## 3. Nachweise

Alle Messungen mit **nicht gesetztem** `CANBERRA_DRIVER` in der Umgebung —
`env | grep -i canberra` war leer. Sonst hätte die Messung den Einbau nicht
geprüft, sondern die Shell.

### 0 — Der Einbau greift überhaupt

```
$ grep -c "testsilence.dir/testsilence.cpp.o" build/tests/CMakeFiles/<ziel>.dir/link.txt
1     # für alle neun Ziele: storetest capturetest librarytest shelltest
      # firstruntest searchshots libraryshots editshots readmeshots
$ nm -C build/bin/librarytest | grep silentAudio
000000000009f161 b (anonymous namespace)::silentAudio
```

### 1 — `ctest` bleibt vollständig grün

```
$ ctest --test-dir build
100% tests passed out of 7
Total Test time (real) =   5.76 sec
```

### 2 — Die ganze Suite erzeugt 0 Audio-Streams

Gemessen wie vom PO: `pactl subscribe` lief die gesamte Messung über in den
Hintergrund, gezählt wurden Ereignisse auf `sink-input`.

| Schritt | Zeilen im Log | `sink-input`-Ereignisse |
|---|---|---|
| Start des Abonnenten | 0 | 0 |
| Positivkontrolle 1 (`paplay --volume=0`) | 16 | **6** |
| `editshots` (frisch gebaut) | 16 | 6 → **0 neu** |
| `ctest --test-dir build` (7/7) | 16 | 6 → **0 neu** |
| Positivkontrolle 2 (`paplay --volume=0`) | 29 | 12 → **+6** |

Vollständiges Log: `2026-08-04-klangfrei/pactl-messung.log`. Es hat 32 Zeilen,
nicht 29: Nach der letzten Zählung liefen noch drei Zeilen des ausklingenden
Kontroll-Streams (`change on sink/source`) und die Abschlusszeile des
Abonnenten ein. Auf `sink-input` bleibt es bei 12.

### 3 — Positivkontrolle der eigenen Messung

```
$ paplay --volume=0 /usr/share/sounds/freedesktop/stereo/dialog-warning.oga
```

```
Event 'new' on client #940
Event 'new' on sink-input #941
...
Event 'remove' on sink-input #941
```

Die Kontrolle wurde **vor und nach** den beiden Nullmessungen gefahren. Die
zweite ist die wichtigere: Sie belegt, dass der Abonnent auch am Ende noch
gelaufen wäre — „0 Streams" ist damit von „Abonnent tot" unterschieden.
Lautlos, wie beauftragt (`--volume=0`).

### 4 — Ein Bildläufer erzeugt 0 Streams

```
$ cmake --build build --target editshots     # frisch gebaut, CLAUDE.md
$ QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde \
      ./build/bin/editshots <zielverzeichnis>
```

Null neue Ereignisse (Tabelle oben). Dass dabei wirklich ein Wächterdialog
angezeigt wurde — sonst wäre die Null nichts wert —, zeigt das Bild, das der
Lauf geschrieben hat: `2026-08-04-klangfrei/editshots-waechterdialog.png`.
Dialog, Warnsymbol, drei Antworten, Vorgabe auf „Speichern": unverändert. Nur
still.

### 5 — Der Bau bleibt warnungsfrei

Vollständiger Neubau in einem leeren Verzeichnis, gezählt wie die CI zählt
(`grep -c 'warning:'`, deshalb `LC_ALL=C`):

```
$ LC_ALL=C cmake -B <neubau> -S . -DCMAKE_BUILD_TYPE=Debug
$ LC_ALL=C cmake --build <neubau> -j 12 > bau.log
$ grep -c 'warning:' bau.log
0
```

Die Warnungsschwelle der CI steht auf null (`.github/workflows/ci.yml`,
Schritt „Warnungen zählen", `-ne 0 → exit 1`) — unverändert und eingehalten.

## 4. Was der Entwurf nicht abdeckt

- **Ein Test außerhalb von `tests/`** würde die Stille nicht erben.
  `link_libraries()` wirkt im Verzeichnis und seinen Unterverzeichnissen; heute
  gibt es weder das eine noch das andere.
- **Ein anderer Klangweg als libcanberra** bliebe hörbar. Das ist so gewollt
  („melden, nicht heilen"): Fängt künftig etwas anderes an zu tönen, fällt es
  auf, statt still unterdrückt zu werden.
- **Der Klang beim echten Nutzer bleibt** — genau das war die
  Kundenentscheidung. Nachgetragen in SPEC 9, damit ihn niemand später für ein
  Versehen hält.

## 5. Offen für den PO

- **Committen und pushen** — beides bewusst unterlassen.
- **Der zweite Punkt der Diagnose bleibt offen:** Das Klangmuster, das der
  Kunde um 10:04:25 `libraryshots` zuschrieb, erklärt der gefundene Mechanismus
  nicht (`libraryshots` zeigt keinen Dialog). Der Einbau macht ihn ohnehin
  still, aber die Frage, ob es einen zweiten Auslöser gibt, ist damit nicht
  beantwortet, sondern nur überdeckt. Eine Wiederholungsmessung je Läufer
  wäre die saubere Klärung — sie stand nicht in meinem Auftrag.

---

# Nachtrag vom 04.08.2026: der erste rote CI-Lauf

Auftrag des PO nach Commit `d30f5d0`. Geändert wurde allein
`tests/testsilence.cpp`; `tests/CMakeLists.txt` blieb unberührt, weil der
Entwurf im Großen trägt. Nicht committet.

## 6. Der Befund und seine Behebung

Die CI meldete an `d30f5d0`:

```
tests/testsilence.cpp:41:1: warning: non-POD static
  (SilentAudio (anonymous namespace)::silentAudio) [-Wclazy-non-pod-global-static]
```

Damit standen 4 clazy-Befunde gegen die Schwelle 3, und der Lauf brach ab.
Selbst nachgemessen, bevor ich etwas anfasste:

```
$ cmake --build build --target lint-clazy | grep -c 'warning:'
4
```

**Behoben, ohne die Wirkung zu verlieren.** Gebraucht wird nur *eine
Auswertung vor `main()`*, nicht ein Objekt, das den ganzen Lauf über steht:

```cpp
int silenceAudio()
{
    qputenv("CANBERRA_DRIVER", "null");
    return 0;
}

const int silenceApplied = silenceAudio();
```

Ein `int` ist POD — kein Konstruktor, keine unklare Initialisierungsreihenfolge,
kein Destruktorlauf beim Beenden, und damit auch nichts, was clazy zu Recht
anmerkt. Die dynamische Initialisierung des Namensraum-Objekts läuft dennoch
vor `main()`; sie hat eine Nebenwirkung und darf deshalb nicht wegoptimiert
werden. Nachgemessen statt geglaubt: Der Initialisierer steht im gelinkten
Programm.

```
$ nm -C build/bin/librarytest | grep -i silence
... t _GLOBAL__sub_I_testsilence.cpp
... t (anonymous namespace)::silenceAudio()
... b (anonymous namespace)::silenceApplied
```

Ein `NOLINT` habe ich nicht vorgelegt: Der Befund ist nicht falsch, und die
POD-Fassung ist die einfachere Sache — sie schafft ab, worüber der Linter
klagt, statt es zuzudecken. Der Kommentarkopf der Datei ist mitgezogen
(„der **Aufruf** unten läuft vor `main()`" statt „das statische Objekt"), und
die neue Stelle trägt in zwei Zeilen den Grund, damit niemand sie in ein
Objekt zurückverwandelt.

## 7. Nachweise des Nachtrags

Wieder alle Messungen mit **nicht gesetztem** `CANBERRA_DRIVER`
(`env | grep -ci canberra` → `0`).

### clazy: 3 Befunde, Schwelle gehalten

```
$ cmake --build build --target lint-clazy | grep 'warning:'
tests/librarytest.cpp:2336:5: c++11 range-loop might detach Qt container (QList) [-Wclazy-range-loop-detach]
tests/librarytest.cpp:2342:5: c++11 range-loop might detach Qt container (QList) [-Wclazy-range-loop-detach]
tests/shelltest.cpp:361:24: Don't call QList::first() on temporary [-Wclazy-detaching-temporary]
```

Genau die drei Altbefunde vom 04.08.2026, kein vierter.

### clang-tidy: kein neuer Befund — und eine Abweichung, die ich melde

```
$ cmake --build build --target lint-tidy > tidy.log
$ grep -c 'warning:' tidy.log            # so zählt die CI bei clazy
130
$ grep 'warning:' tidy.log | sort -u | wc -l
73
$ grep 'warning:' tidy.log | grep -c testsilence
0
```

Keine Zeile nennt `testsilence.cpp`. Weil „keine Zeile nennt sie" ein
Umkehrschluss ist, habe ich den Vergleich gefahren, statt ihn zu erschließen —
derselbe Lauf ohne die Datei:

```
$ run-clang-tidy -p build/lint -quiet "^<repo>/src/.*\.cpp$" \
      "^<repo>/tests/(?!testsilence|spellfixspike).*\.cpp$"
Running clang-tidy ... for 25 files out of 42 in compilation database
roh: 130   eindeutig: 73
```

**Identisch mit und ohne die Datei.** Der Beitrag dieser Änderung ist
gemessen null.

Zwei Dinge dazu, die nicht meine Fläche sind und deshalb als Meldung stehen
bleiben, nicht als Änderung:

1. **Der bekannte Stand ist 72, gemessen habe ich 73** (eindeutige Befunde).
   Die Abweichung besteht mit und ohne meine Datei, kommt also nicht von ihr;
   woher sie kommt, habe ich nicht ermittelt — das lag außerhalb des Auftrags.
2. **Die Zählart entscheidet über die Zahl:** roh 130, eindeutig 73. Die
   Differenz sind Befunde in Kopfdateien, die je einschließender
   Übersetzungseinheit erneut ausgegeben werden. Käme clang-tidy je als Tor in
   die CI, müsste die Zählart mit der Schwelle zusammen festgelegt werden —
   sonst misst die Schranke etwas anderes als die Notiz, gegen die sie gesetzt
   wurde. (Bei clazy stellt sich die Frage nicht: dort sind roh und eindeutig
   beide 3.)

Ein Nebenbefund aus meinem ersten Vergleichslauf, weil er dieselbe Falle
zeigt: Ein Regex `src/.*\.cpp$` trifft auch `build/src/…/mocs_compilation.cpp`
und förderte 21 zusätzliche Befunde in **generiertem** Code zutage. Das Ziel
`lint-tidy` übergibt absolute Pfade und ist davon nicht betroffen; wer den
Linter von Hand ruft, muss den Ausdruck verankern.

### `ctest` weiterhin 7/7

```
$ ctest --test-dir build
100% tests passed out of 7
Total Test time (real) =   5.55 sec
```

### Die Stille wirkt weiter: 0 Audio-Streams

| Schritt | `sink-input`-Ereignisse |
|---|---|
| Positivkontrolle 1 (`paplay --volume=0`) | **6** |
| `ctest` (7/7) | 6 → **0 neu** |
| `editshots`, frisch gebaut | 6 → **0 neu** |
| Positivkontrolle 2 (`paplay --volume=0`) | 12 → **+6** |

Log: `2026-08-04-klangfrei/pactl-messung-nachtrag.log`. Der Läufer hat dabei
wieder seine fünf Bilder geschrieben, den Wächterdialog eingeschlossen.

### Bau warnungsfrei

Vollständiger Neubau in einem leeren Verzeichnis, gezählt wie die CI zählt:

```
$ LC_ALL=C cmake --build <neubau> -j 12 > bau.log
$ grep -c 'warning:' bau.log
0
```

## 8. Was der Vorgang zeigt

**Der erste rote CI-Lauf dieses Projekts hat einen Befund gefunden, den fünf
vorgegebene Prüfungen und zwei eigene Nachmessungen durchgelassen haben.**

Die Prüfungen waren nicht nachlässig gefahren — sie waren auf die falsche
Frage gerichtet. Sie fragten „wirkt die Änderung?" (Streams, Symbol, Bild)
und „bricht sie etwas?" (`ctest`, Compiler-Warnungen). Keine fragte „hält sie
die Regeln ein, auf die dieses Projekt sich festgelegt hat?" — und genau das
prüfen die Linter. Ein Compiler-Warnungslauf ist dafür kein Ersatz: Der Bau
war in beiden Fassungen bei null.

Wirksam wurde das Tor überdies erst am selben Tag: Ohne den clazy-Schritt in
`ci.yml` (Commit `4df6c63`) hätte der Befund den Weg in `main` genommen und
wäre bei der nächsten Heilungs-Story als vierter „Altbefund" mitgezählt
worden.

Für die Retro nach Sprint 6 gehört beides zusammen: Das Tor hat gehalten —
und die Prüfliste eines Auftrags ist selbst ein Prüfgegenstand. Wo eine
Regelprüfung existiert, gehört sie in die Nachweise, sonst prüft der Auftrag
nur die halbe Definition of Done.
