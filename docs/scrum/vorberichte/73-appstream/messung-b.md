# Vorprüfung #73 — Messung B (Scrum Master)

Gemessen am 2026-08-04, 20:30–20:38 CEST, Stand `6acc87e`. Unabhängig von
Bearbeiter A; dessen Messung war zum Zeitpunkt dieser Arbeit nicht gelesen.

## Feld 1 — Dateimenge (so weit ohne A gemessen)

| | **#73** |
|---|---|
| **Quellen und Tests** | **Neu:** `org.denkzettel.Denkzettel.metainfo.xml` (Ablageort offen — `desktop/` liegt nahe, weil die `.desktop`-Datei dort liegt).<br>**Neu:** `tests/metainfotest.cmake` nach dem Muster von `tests/installtest.cmake` (56 Z.). |
| **Build** | `CMakeLists.txt` (Wurzel) — eine `install(FILES … DESTINATION ${KDE_INSTALL_METAINFODIR})`-Zeile im Block bei `:44–60`.<br>`tests/CMakeLists.txt` — eine `add_test`-Registrierung nach dem Muster `:177–183`. |
| **Belege und Prüfmittel** | `docs/scrum/reviews/sprint-NN-s73-metainfo/` mit der Mutationsprobe (AK 5). |
| **Fachliche Quellen** | `docs/scrum/PROZESS.md`, Abschnitt „Sprint-Abschluss" — **siehe Feld 6, das ist keine Dev-Fläche.** |
| **Ausdrücklich nicht** | `src/` (kein Produktivcode), `.github/workflows/ci.yml` (siehe unten), `CHANGELOG.md`. |

**Gemessen: `.github/workflows/ci.yml` muss nicht angefasst werden.** `appstream`
steht bereits in der Paketliste (`ci.yml:70`, letzte Zeile des `pacman`-Aufrufs),
und `appstreamcli` ist auf Ganymed vorhanden (1.1.5). AK 6 erfüllt sich damit
allein dadurch, dass der neue `ctest`-Fall existiert — es ist kein eigener
CI-Schritt nötig. Das ist die billigere Hälfte der Story, als das Issue
vermuten lässt.

## Feld 2 — Fallen (Teilmessung)

- **`appstreamtest` bleibt daneben stehen und bleibt grün.** Das Manifest führt
  fünf Zeilen, keine mit `.metainfo.xml` (`build/install_manifest.txt`, heute
  nachgemessen — unverändert gegenüber dem Issue-Befund). Wer nach der Umsetzung
  „7/7 grün" sieht, hat weiterhin keinen Beleg; der Beleg ist der **neue** Test.
- **Der Ablageort der Datei ist nicht festgelegt.** Das AK nennt nur das
  Installationsziel. `git ls-files` zeigt `desktop/org.denkzettel.Denkzettel.desktop`
  — daneben ist der einzige Ort, der keine neue Ordnerentscheidung erzwingt.

## Feld 3 — AK-Urteil: **ready = nein**

Fünf der sechs Kriterien sind einzeln prüfbar und haben ein Prüfmittel, das
`git ls-files` bzw. `which` zeigt (`tests/installtest.cmake` als Muster ✓,
`appstreamcli` 1.1.5 ✓, `docs/scrum/reviews/` ✓). Die drei DoR-Zusätze vom
04.08.2026 greifen sonst nicht: keine selbstdeklarierten offenen Punkte (beide
Vorlagen sind am 04.08. vom Kunden entschieden), keine Aussage über Hülle,
Rundung, Kontur, Schatten oder Dekoration (B21 nicht einschlägig).

**AK 3 trägt nicht — drei Gründe, jeder für sich hinreichend:**

1. **„`releases` ist aus `CHANGELOG.md` gespeist"** benennt kein Prüfmittel und
   ist zweideutig: einmal von Hand übertragen oder bei jedem Bau erzeugt? Das
   sind verschiedene Stories. Prüfbar wäre etwa: *„zu jeder Version in
   `CHANGELOG.md` steht ein `<release version=… date=…>`; der neue Test
   vergleicht beide Listen."*
2. **„die Fortschreibungsregel steht im Sprint-Abschluss"** verlangt eine
   Änderung an `docs/scrum/PROZESS.md`. Diese Datei gehört dem Scrum Master
   (Rollen-Tabelle), nicht dem Dev-Agenten — das Kriterium ist von dem, der es
   erfüllen soll, nicht erfüllbar.
3. **„und schreibt die Aussetzung von Punkt 10 bis #61 mit"** ist ein Satz, der
   in dem Augenblick falsch wird, in dem #61 geliefert ist. Wer ihn heute
   schreibt, legt eine B17-Falle: eine Aussage im Präsens ohne genannten
   Prüfstand.

**Behebung (PO):** AK 3 in zwei Teile trennen — den prüfbaren Teil über die
`releases`-Liste behält die Story, der Prozesstext geht als eigene Zeile an den
Scrum Master und nennt den Stand, für den er gilt.

## Feld 5 — Größenklasse: **`size:m`**

**Nicht `size:s`, und der Grund steht in der Klassendefinition selbst:**
„`size:s` = wenige Dateien, **kein neuer Prüfweg**." AK 4 verlangt ausdrücklich
einen **projekteigenen Validierungstest** — ein neuer `ctest`-Fall, eine neue
`.cmake`-Datei, eine neue Registrierung. Damit ist `s` per Definition
ausgeschlossen, unabhängig davon, wie billig die XML ist.

Dazu kommt AK 5: Die Mutationsprobe ist ein zweiter Lauf mit absichtlich
beschädigter Datei samt versioniertem Beleg — das Muster ist im Projekt
vorhanden, kostet aber einen eigenen Arbeitsgang.

Nicht `size:l`: Kein Produktivcode, keine SPEC-Änderung an einer Festlegung,
keine Bildprüfung, kein Compositor. Die Fläche ist eng und liegt vollständig
außerhalb von `src/`.

## Feld 6 — Offene Fragen

**An den PO:**

1. **Wer schreibt die Fortschreibungsregel in `PROZESS.md`?** Nach der
   Rollen-Tabelle der Scrum Master. Solange AK 3 sie dem Umsetzenden zuschreibt,
   ist die Story nicht ready (siehe Feld 3).
2. **`releases` erzeugt oder abgetippt?** Entscheidung nötig, bevor AK 3 prüfbar
   formuliert werden kann.
3. **Der Abschnitt „## Schätzung — 3 SP" steht noch im Issue.** Das Verfahren
   ist am 04.08.2026 beendet; der Abschnitt nennt „zwei unabhängige Schätzer"
   und sieht damit genau so aus wie das Prüfergebnis, das die Regel vom
   04.08.2026 von einer Hausnummer trennen will. Bauart der Bereinigung steht in
   `PROZESS.md`, Artefakte („Label weg, Zahl bleibt lesbar"). Melden, nicht
   heilen.

**Kundenentscheidung:** keine offen. Beide Vorlagen (Sprache, Screenshots) sind
am 04.08.2026 entschieden und stehen im Issue.
