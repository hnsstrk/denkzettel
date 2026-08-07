# Karpathy-Review Sprint 9

**Prüfgegenstand:** `sprint-09-basis..main` (Basis `366b69f`, HEAD `70902a4`),
9 Commits, 77 Dateien, 3191 Zeilen hinzugefügt, 70 entfernt.
**Stories:** #100 (Textfeld aus `widgets/lineedit`) und #101 (Haarlinien in der
Notizliste).
**Prüfer:** `karpathy-reviewer` (Fable 5, frischer Kontext), 07.08.2026.
**Maßstab:** die vier Arbeitsprinzipien, dazu `CLAUDE.md` und
`docs/scrum/PROZESS.md`.

---

## Karpathy-Review

**Task:** Sprint-9-Diff mit #100 und #101 gegen die vier Arbeitsprinzipien
prüfen — Rückführbarkeit jeder geänderten Zeile auf ein Akzeptanzkriterium,
Verhältnis von Beleg zu Produktivcode, Tragfähigkeit der behaupteten Messwerte,
und die Frage, ob Prüfsätze still ihre Grundlage verloren haben.

**Gesamt-Verdict:** fail

### Befunde

| ID | Prinzip | Verdict | Ort | Befund | Status |
|----|---------|---------|-----|--------|--------|
| K1 | 4 Goal-Driven | fail | `docs/scrum/reviews/sprint-09-s100-eingabefeld/bericht.md:232–241` | Der öffentliche Lauf zu `70902a4` ist completed/**failure** (capturetest 28 passed, 6 failed, 3 skipped); Basis `366b69f` war grün. Der Bericht meldet „100% tests passed" und sagt für den Läufer nur ein `QSKIP` voraus. Mit gefallen ist `hullHoldsAtTheCustomersScale()` — ein Bestandssatz, der erst rot wurde, weil #100 `paintsTheThemesFieldOntoTheHull` in dessen Kindprozess gehängt hat (`tests/capturetest.cpp:1480`). | offen |
| K2 | 4 Goal-Driven | fail | `tests/capturetest.cpp:1023` (dazu `:803`, `:945`) | Der vom PO verlangte `QSKIP` sitzt auf der falschen Hälfte: er deckt „kein Theme zeichnet einen Hauch" (`:1027`), während „kein Theme deckt 255" eine Zeile davor hart abbricht — und genau das ist die Lage auf dem Läufer. `QVERIFY(ring > 0)` und `QVERIFY2(border > 0, …)` tragen dieselbe unausgesprochene Voraussetzung „die Grafik existiert". | offen |
| K3 | 4 Goal-Driven | fail | `docs/scrum/reviews/sprint-09-s101-listentrenner/bericht.md:85` gegen `…/messungen/mutationsprobe.txt` | Der Bericht führt Mutationsprobe 5 als „rot"; die Messdatei sagt „**ABBRUCH: Eingriff ließ sich nicht anbringen**". `mutationsprobe.sh:96` sucht `const qreal share = KColorScheme::frameContrast();`, im Code steht seit Fund 4.1 `const auto share = static_cast<float>(…)` (`src/ui/notelistdelegate.cpp:85`). AK 4 ist damit die einzige Zusicherung von #101 ohne Mutationsnachweis — in dem Abschnitt, den der Bericht „Der Nachweis, der diesen Bericht trägt" nennt. | offen |
| K5 | 3 Surgical | fail | `docs/bilder/erfassungsfenster.png`, `docs/bilder/bibliothek.png` (Stand `cead9f9`, 04.08.2026); `README.md:6`, `:47` | Die README-Bilder des **öffentlichen** Repositoriums zeigen das Produkt vor beiden Kundenbefunden: Fenster ohne Feld, Liste ohne Trennlinien. Der Läufer `readmeshots` erzeugt genau diese zwei Dateien. Kein Bericht meldet es unter „Melden, nicht heilen" — der Ausschluss-Griff durchsucht keine PNG. | offen |
| K4 | 1 Think Before | warn | `docs/scrum/reviews/sprint-09-s101-listentrenner/bericht.md:160` | „**Keine der drei** zeigt die Notizliste" — `bilder/skalierung-1/10c-schema-dunkel-bearbeiten.png` zeigt sie samt beider Gruppenköpfe und der vollen Linie über „Gestern" (nachgesehen). Der Schluss mag tragen, seine Begründung nicht; der Befund geht so an den PO. | offen |
| K6 | 3 Surgical | warn | `.github/workflows/ci.yml:66–70` | `KF6ColorScheme 6.20` ist neue Pflichtabhängigkeit (`CMakeLists.txt:43`), steht aber nicht in der Paketliste — der Bau gelingt dort nur transitiv. Die Datei nennt zu jedem Paket seinen Grund; ein späterer Bruch läse sich als Upstream-Sprung. | offen |
| K7 | 3 Surgical | warn | `tests/librarytest.cpp:3177–3181` | `frameContrast=0.45` wird in die Sandkasten-`kdeglobals` geschrieben und nie zurückgenommen; der Wert überlebt in `~/.qttest/config/` und ist Grundlage jedes späteren Laufs. Heute fällt nichts darüber, aber es ist genau der hinterlassene Zustand, den der Schwesterprüfsatz aus #100 mit eigenem `HOME` ausdrücklich vermeidet (`tests/capturetest.cpp:439–441`). | offen |
| K8 | 4 Goal-Driven | warn | `docs/scrum/reviews/sprint-09-s100-eingabefeld/bericht.md:236–239` | „capturetest: 37 passed, 0 failed, 0 skipped … Vollständig in `messungen/m3-testlauf.txt`" — m3 enthält nur die ctest-Zusammenfassung, keine Einzelausgabe. #101 führt die Entsprechung (`b3-pruefsaetze.txt`) korrekt. | offen |
| K9 | 4 Goal-Driven | warn | `tests/capturetest.cpp:134–137` gegen `src/capture/capturewindow.cpp:48` | Der neue Abgriff `besideTheField()` liegt bei `text->y() - 4` in einem Zwischenraum von **8 px** (`SpacingBelowAppName`) — 4 px Puffer nach oben wie nach unten. Kein Prüfsatz hält diese 8 fest: `footerHasMoreAirThanTheApplicationName()` (`:1285–1291`) sichert nur `belowTheName > 0`. Schrumpft der Abstand, misst der reparierte Abgriff still den App-Namen — dieselbe Bauart wie die Falle, die AK 9 gerade behoben hat, eine Ebene weiter außen. | offen |

### Fix-Vorschläge

- **K1** → Nach K2 erneut pushen und
  `gh run list --commit $(git rev-parse HEAD) --json status,conclusion`
  bis `completed` **und** `success` nachfahren (B18); im selben Zug den
  Berichtssatz „überspringt sich auf dem öffentlichen Läufer" auf das
  Gemessene setzen.
- **K2** → Die drei Voraussetzungszeilen (`:803`, `:945`, `:1023`) auf einen
  gemeinsamen `QSKIP` „`widgets/lineedit` löst auf diesem Läufer nicht auf"
  umstellen — oder, falls es ein Paket gibt, das die Grafik mitbringt, dieses
  in `ci.yml` nachziehen und die Prüfsätze scharf lassen. Der zweite Weg ist
  der bessere: sonst prüft der öffentliche Lauf die Story gar nicht.
- **K3** → Muster in `mutationsprobe.sh:96` auf `KColorScheme::frameContrast()`
  allein kürzen (die „genau einmal"-Wache trägt weiter), Probe nachfahren,
  Berichtszeile 85 auf das Ergebnis setzen. Fällt sie grün aus, ist AK 4
  ungeprüft und der Prüfsatz gehört überarbeitet.
- **K5** → `readmeshots` fahren, beide Bilder neu ablegen, Bildunterschriften
  gegen das Bild lesen. PO-Entscheidung, aber vor der Kundenabnahme: der Kunde
  hat genau diese zwei Ansichten beanstandet.
- **K4** → Satz auf das Gemessene zurückführen (die Liste **ist** in 10c/10d im
  Bild; die Abweichung liegt nicht dort, sondern beim blinkenden Textzeichen —
  oder die Ursache offen lassen).
- **K6** → `kcolorscheme` mit Begründungszeile in die `pacman`-Liste aufnehmen.
- **K7** → Alten Wert merken und im `cleanup()` zurückschreiben, oder den
  Prüfsatz wie den #100-Schwestersatz in einen Kindprozess mit eigenem `HOME`
  legen.
- **K8** → Prüfsatz-Einzelausgabe in `m3-testlauf.txt` nachziehen (nach K1/K2
  ohnehin fällig, weil die Zahl sich ändert).
- **K9** → Eine Zeile in `footerHasMoreAirThanTheApplicationName()`, die
  `belowTheName` gegen `SpacingBelowAppName` festnagelt — oder
  `besideTheField()` seinen Puffer selbst prüfen lassen, nach dem Muster der
  Wache, die `takesTheOpaqueVariantWithoutABlurringCompositor()` bei
  `:1602–1607` bereits trägt.

### Was gut ist

- **Die Reparatur der beiden Abgriffe aus AK 9 trägt** — nachgeprüft, nicht
  angenommen. `hullIsCompleteAtFiveAndEightLines()` bekommt sein Layout über
  `shot()` (`tests/capturetest.cpp:472–483`), bevor `besideTheField()`
  ausgewertet wird; `takesTheOpaqueVariantWithoutABlurringCompositor()` hat
  dafür eigens `show()` bekommen und sichert seinen Abgriff zusätzlich mit
  `sample.y() < text->y()` ab. Der zusätzliche **Farb**vergleich gegen die
  Hülle ist die entscheidende Zutat: die Deckung allein konnte Feld und Hülle
  nicht unterscheiden, und Mutationsprobe 6 belegt das. Einzige Einschränkung
  ist K9.
- **Auf der #101-Seite hat keine Zusicherung still ihre Grundlage verloren.**
  Vor dieser Story gab es in `tests/librarytest.cpp` keinen einzigen
  Bildpunktabgriff (geprüft über alle `grab()`-, `pixelColor`- und
  `pixel(`-Stellen); die Geometriesätze und die fünf #70-Sätze arbeiten über
  `visualRect()` und sind von einer Linie in einem bestehenden Innenabstand
  unberührt. `sizeHint()` ist nicht angefasst.
- **Prinzip 3 hält im Produktivcode.** Jede geänderte Zeile in `src/` ließ sich
  auf ein Akzeptanzkriterium zurückführen: die dritte `FrameSvg` und
  `applyFieldMargin()` auf AK 1/4/5, `repaintTheRowAbove()` auf AK 3c, die
  beiden `fillRect()` auf AK 1/2/3, `find_package(KF6ColorScheme 6.20)` als
  eigener Aufruf auf die Kundenentscheidung. Die Werte in
  `tests/themes/…/denkzettel-test-breit/colors` sind unverändert, nur der
  Kommentarkopf ist nachgezogen (AK 10). Zwei parallele Stränge, kein
  Kreuztreffer, keine Nebenbei-Verbesserung.
- **Prinzip 4 in der Sache, dort wo gemessen wurde.** Stichprobe bestanden: der
  Sitzungsbeleg von #100 trägt Ziffer für Ziffer die Werte von #83
  (`docs/scrum/reviews/sprint-07-s83-native-huelle/messungen/m5-fensterlage-sitzung.txt:11`).
  Mutationsprobe 5 von #100 dokumentiert einen **grün gebliebenen** Lauf als
  Befund; Probe 6 und 7 haben je einen Prüfsatz verbessert statt bestätigt.
- **AK 3c ist mit dem richtigen Prüfmittel belegt** — der `PaintCounter` erbt
  vom Produktions-Delegate, und der Bericht sagt ausdrücklich, dass ein
  Bildbeleg hier nichts zeigen kann. Der Fund „`QColor::fromRgbF` vergleicht
  sich nie gleich mit einem Bildpunkt" samt seiner schlimmeren Hälfte (der
  Rollenvergleich wäre immer grün gewesen) ist die verlangte Prüfhaltung.
- **Prinzip 2 kein Befund.** 64 der 77 Dateien sind Belege, überwiegend PNGs;
  die 1,6er-Reihe legt bewusst 2 von 15 Szenen ab, mit Begründung im Skript.
  Der Produktivteil ist knapp — keine Abstraktion für einen Fall, keine
  unbestellte Option.

---

**Offene `fail`-Befunde: ja — vier (K1, K2, K3, K5).** DoD 3 ist damit nicht
erreicht.
