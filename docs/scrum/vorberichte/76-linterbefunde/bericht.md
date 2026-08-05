# Vorprüfbericht #76 — Linterbefunde heilen

**Konsolidiert vom PO am 05.08.2026** aus `messung-a.md` (Bearbeiter A,
`denkzettel-dev`) und `messung-b.md` (Bearbeiter B, Scrum Master), beide gegen
Stand `6acc87e`, unabhängig voneinander.

**Ergebnis: `size:m`, ready — als einzige der fünf Vorprüfungen vom 04.08.2026
ohne Nacharbeit am Issue.**

---

## 1. Beide Messungen stimmen im Urteil überein

`size:m` von beiden, und beide begründen es gleich: **nicht `s`**, weil 20
Dateien Befunde tragen und AK 6 einen neuen Prüfweg in `ci.yml` aufnimmt;
**nicht `l`**, weil die Arbeit mechanisch ist und keinen Entwurfsanteil hat —
keine offene Gestaltungsfrage, keine Bildprüfung, kein Compositor.

B fasst die Ehrlichkeit der Einstufung in einem Satz, der übernommen gehört:
*„`m` heißt hier ‚trägt einen Strang aus', und das trifft es — aber der Strang
berührt jede andere Fläche des Repositories. Die Klasse beschreibt den Aufwand,
nicht die Verträglichkeit."*

## 2. Der Fund, der die Größenordnung der Story korrigiert

**A hat nachgezählt statt nachgelesen und dabei eine Division gefunden:**
`performance-enum-size` sind **7 Enums, nicht 59 Stellen**. Jede Kopfdatei wird
von jeder Übersetzungseinheit erneut gemeldet — **Faktor 9,4**.

Das trifft nicht nur das Issue, sondern auch die zweite Messung: **B führt in
seiner Dateitabelle 36 Befunde für `src/store/note.h`** und schließt daraus,
`performance-enum-size` in der zentralen Datentyp-Datei sei „eine Entscheidung,
keine Heilung … dieselbe Frage wie beim `bugprone-unused-return-value`, nur
36-mal". Nach der Entdoppelung ist das eine deutlich kleinere Frage. **Der PO
hat sie deshalb im Sinne des Kunden entschieden** (heilen statt einfrieren) und
nicht als Grundsatzfrage vorgelegt.

**Das ist der Wert der zwei Bearbeiter an diesem Fall:** B hat die Fläche
kartiert und die richtige Frage gestellt; A hat gemessen, dass die Frage
kleiner ist, als sie aussieht. Keiner der beiden allein hätte beides geliefert.

## 3. Vier weitere Funde, die je einen Fehlversuch ersparen

**Eine Runde `-fix` genügt nicht** (A). Gemessen: 140 → 91 → 88 Rohzeilen,
danach stabil. Das `const` auf einer Variablen legt das `const` auf der
nächsten erst frei. **Wer einmal `-fix` laufen lässt und den Rest zählt, zählt
falsch.** Der maschinelle Teil trägt 52 der 81 Befunde, 57 geänderte Zeilen in
8 Dateien, `ctest` danach 7/7 grün, null Compilerwarnungen.

**Acht Befunde kann kein Werkzeug heilen** (A). Sie stehen alle auf einer
`QFETCH`-Zeile; das Makro deklariert die Variable selbst. Ein `const` ist dort
nur unterzubringen, indem man `QFETCH` aufgibt — die QTest-Redensart für
datengetriebene Tests.

**Zwei Prüfungen der Liste ziehen an derselben Variablen in entgegengesetzte
Richtungen** (A). `misc-const-correctness` fordert `const` auf lokalen
Variablen; `performance-no-automatic-move` meldet an derselben Stelle, das
`const` verhindere die Rückgabe per Verschiebung. **Die Story kann nicht beiden
folgen** und braucht dafür eine Regel, nicht einen Griff.

**Hinter einem der Befunde steckt gar kein Fehler — der Widerspruch liegt in
`.clang-tidy`, und er ist datiert** (A). Die Prüfregel
`^::KGlobalAccel::setGlobalShortcut$` kam mit `9e8353b` (01.08.2026, 11:18),
das Rücklesen, das sie überflüssig macht, mit `7130f81` — **acht Minuten
später, auf einem anderen Strang.** Die Regel wurde geschrieben, bevor der
bessere Nachweis im Code stand, und fordert seither einen Beleg, den es nicht
gibt. Positiv gemessen (`sonden/rueckgabeprobe.cpp`): Das Rücklesen ist
**strikt stärker** als der Rückgabewert — es findet alles, was `false` gemeldet
hätte, und zusätzlich die Fehlschläge, die `false` gar nicht melden kann.

**Und eine Vermutung wurde widerlegt:** A hatte erwartet, dass
`const LibraryWindow library;` an `QObject::connect` bricht. Die Probe sagt in
beiden Fassungen `rc=0` — der Empfängerparameter ist `const`-qualifiziert.

## 4. Die sechs Felder

**Feld 1 — Dateimenge.** **20 Dateien tragen Befunde**, gemessen: 6 in `src/`,
4 Kopfdateien mit Kern-Datentypen, 5 Bildläufer, der Rest Tests. Dazu
`.github/workflows/ci.yml` und `.clang-tidy`. **Das ist praktisch das ganze
Repository** — die Dateimenge ist hier keine Eingrenzung, sondern eine Warnung.
**Ausdrücklich nicht:** `tests/spellfixspike.cpp` (PO-Entscheidung 5).

**Feld 2 — gemessene Fallen:** die vier aus §3 plus der Zählfehler aus §2, dazu
`clazy-standalone` liefert `rc=0` auch mit Befunden (der Rückgabewert taugt
nicht als Nachweis) und die gewanderten Zeilennummern der clazy-Befunde (wer
nach Zeilennummer sucht, sucht falsch).

**Feld 3 — AK-Urteil: ready = ja** (B). Alle Kriterien einzeln prüfbar, jedes
Prüfmittel existiert und ist von `git ls-files` gedeckt, B21 nicht einschlägig.
B vermerkt ausdrücklich, was hier gelungen ist: *„AK 4 verlangt den Nachweis je
Änderung in `src/`, nicht nur einen grünen `ctest`. Das ist genau die Bauart, an
der dieses Projekt seine vier wertlosen Tests gefunden hat."*
Der Abschnitt „Ein Befund, der eine Entscheidung braucht" sieht wie ein
selbstdeklarierter offener Punkt aus, ist aber keiner: Dort stand nicht „vor
dem Ziehen zu entscheiden", sondern der Entwickler legt **innerhalb** der Story
vor — und ein Kriterium fängt beide Wege auf.

**Feld 4 — Prüfmittel.** `lint-tidy` und `lint-clazy` sind als Ziele definiert
und laufen; `ci.yml` liegt im Repo; `ctest` läuft. **Was ein Agent nicht kann:**
messen, ob `setGlobalShortcut()` bei **erreichbarem** Daemon jemals `false`
liefert — das legte eine echte Registrierung in der Sitzung des Kunden an, und
die Sonde vermeidet das bewusst.

**Feld 5 — Größenklasse: `size:m`**, beide Bearbeiter unabhängig.

**Feld 6 — acht offene Fragen, alle vom PO am 05.08.2026 entschieden.** Die
Tabelle steht im Issue; die Kurzfassung: „auf 0" gilt wörtlich (mit acht
`NOLINTNEXTLINE` und einer gemeinsamen Begründung) · die
`KGlobalAccel`-Regel wird **gestrichen**, nicht mit `NOLINT` übergangen · für
zurückgegebene lokale Variablen gewinnt `performance-no-automatic-move`, als
**Regel** im Kommentar von `.clang-tidy` · `easily-swappable-parameters`
bekommt `NOLINT`, der sichtbare Fall ein eigenes Issue (**#88**) ·
`spellfixspike.cpp` bleibt draußen, die Schalterstellung kommt ins Kriterium ·
**#76 nach #83**, nicht daneben · **DoD 1 nennt künftig die Linterschwelle**
(entdeckte Bedingung, DoD 4/B9) · die CI-Wache zählt die **Dreizahl** statt
`grep`-Zeilen.

## 5. Die Kollisionsaussage — sie hat den Sprint-7-Schnitt bestimmt

**#76 kollidiert mit allem, was offen ist**, und der schwerste Fall ist #83:

| Gegen | Fläche | Urteil |
|---|---|---|
| **#83** | `capturewindow.cpp`, `capturetest.cpp`, `captureshots.cpp` — **drei der vier Kerndateien von #83** | **nicht parallel** |
| **#71 / #72 / #70** | `librarywindow.{h,cpp}`, `librarytest.cpp`, `libraryshots.cpp` | **nicht parallel** |
| **#61** | `src/main.cpp`, `CMakeLists.txt` | Naht, auflösbar |
| **#73** | keine gemeinsame Datei | frei |

*„Wer dort `const`-Korrektheit und `enum`-Basistypen heilt, heilt Code, den #83
löscht. Das ist nicht nur Mischkonflikt, das ist verworfene Arbeit."* (B)

**Vollzogen:** #83 und die drei Bibliotheksstories liegen in Sprint 7, #76 in
Sprint 8. Die Kollision löst sich damit von selbst — nach Sprint 7 ist der Code,
den #76 anfasst, der endgültige.
