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
