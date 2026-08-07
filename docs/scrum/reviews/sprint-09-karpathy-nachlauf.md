# Karpathy-Review Sprint 9 — Nachlauf

**Prüfgegenstand:** `70902a4..HEAD` (HEAD `58bd93f`), 16 Commits, 151 Dateien,
3717 Zeilen hinzugefügt, 136 entfernt — der Teil des Sprints, der **nach** dem
ersten Karpathy-Review entstanden ist und den dieser deshalb nicht gesehen hat.
Darin: die L9-Korrektur an den Trennlinien (der einzige neue Produktivcode), ein
zweiter ctest-Lauf unter der Skalierung des Kunden, die Heilungen von K1 bis K9
sowie SPEC 9 und Zeichnung 3a.
**Prüfer:** `karpathy-reviewer` (Fable 5, frischer Kontext), 08.08.2026.
**Maßstab:** die vier Arbeitsprinzipien, dazu `CLAUDE.md` und
`docs/scrum/PROZESS.md`.

**Eigene Messungen dieses Laufs** (eigener Bauplatz im Scratchpad, weder `build/`
noch `/usr` angefasst):

- vollständiger Neubau, **0 Compilerwarnungen**; `ctest`: **10 von 10 grün**,
  `librarytestskaliert` darunter.
- `uxsonde` gegen diesen Stand gebaut und in **acht** Lagen gefahren:
  Skalierung 1 · 1,25 · 1,4 · 1,5 · 1,75 · 2 sowie 1,6 mit und ohne Überfahren.
- eine eigenständige Rastersonde, die `hairline()` mit und ohne
  Oberkanten-Ausrichtung gegen den alten `QRect` stellt (15 Zeilenlagen).
- das Abnahmebild der angemeldeten Sitzung nachgemessen.

---

## Karpathy-Review

**Task:** Den Rest des Sprint-9-Diffs prüfen — die L9-Korrektur als einzigen
ungeprüften Produktivcode, die Heilungen der ersten neun Befunde auf
Überkorrektur, und die Frage, ob eine Heilung eine neue unausgesprochene
Voraussetzung eingeführt hat.

**Gesamt-Verdict:** fail

### Befunde

| ID | Prinzip | Verdict | Ort | Befund | Status |
|----|---------|---------|-----|--------|--------|
| N1 | 1 Think Before | fail | `src/ui/notelistdelegate.cpp:123`, `SPEC.md:739`, `wireframes/…dc.html:567` | Die Zusicherung „**die Oberkante auf der Gerätebildpunktgrenze**" hält nicht, und der Term, der sie herstellen soll, trägt nichts. Gemessen an fünfzehn Zeilenlagen bei 1,6: `hairline()` **mit** und **ohne** `std::round(top·ratio)/ratio` liefert überall dieselben 2 Gerätebildpunktzeilen, der alte `QRect` dagegen 1 oder 2. Es trägt allein die **ganzzahlige Höhe**. Die Oberkante liegt in der Bibliothek gar nicht auf dem Raster: Das Sichtfeld der Liste beginnt bei logisch 48, also **76,8** Gerätebildpunkten — die Ausrichtung geschieht gegen den Malerursprung, und der liegt selbst zwischen zwei Bildpunkten. Mutationsprobe 7 kann es nicht bemerken: Sie tauscht die **ganze** Rückgabezeile und ändert damit auch die Höhe. | offen |
| N2 | 1 Think Before | warn | `src/ui/notelistdelegate.cpp:121` gegen `wireframes/…dc.html:567` | Die Zeichnung sagt „eine ganze, **aufgerundete** Zahl von Gerätebildpunktzeilen", der Code rundet **kaufmännisch** (`std::round`). Unterhalb 1,5 fällt das auseinander, und die Linie wird dünner als ein logischer Punkt — gemessen: **1,25 → 1 Bildpunkt (0,80 logische Punkte)**, **1,4 → 1 (0,71)**; ab 1,5 → 2. 125 % ist Plasmas nächste Stufe über 100 %, der Fall ist nicht theoretisch. Die Begründung in `SPEC.md:744–747` („im schwächsten Schema 1,24 : 1 … dünner machen ist genau das, was hier nicht passieren darf") gilt damit nur ab 1,5. Die **Einheitlichkeit** bleibt in allen gemessenen Lagen erhalten — der Befund betrifft die Stärke, nicht den Gleichklang. | offen |
| N3 | 4 Goal-Driven | warn | `tests/capturetest.cpp:1633–1653`, belegt in `…/s100-eingabefeld/messungen/m7-ohne-plasma-grafik.txt:41–43` | Die Trennung von Hülle und Feld hat die Lücke, wegen der sie stattfand, im Geschwister stehenlassen. `runAtTheCustomersScale()` prüft nur den Rückgabewert des Kindes, und `hullHoldsAtTheCustomersScale()` fragt **keine** Vorbedingung, bevor es eines startet. In M7 ist genau das eingetreten: `hullHasNoStairAtTheCorner()` übersprang sich, und der Elternteil meldete **PASS**. Der Bericht schreibt „ein Kind, das alles überspringt, kehrt mit 0 zurück" — das gilt auch, wenn es die Hälfte überspringt. | offen |
| N4 | 4 Goal-Driven | warn | `…/s100-eingabefeld/mutationsproben.sh:57–63`, `:164–165` gegen `…/s101-listentrenner/mutationsprobe.sh:175–181` | K3 ist nur in **einem** der beiden Mutationsskripte geheilt. Das #101-Skript beendet sich bei Abbruch mit Rückgabewert 1; das #100-Skript — das, in dem der Abbruch dieser Runde tatsächlich passiert ist — bekam die Prüfsummen-Wache, aber **keinen Rückgabewert**: Es schreibt „ABGEBROCHEN", `return`t aus der Funktion und endet mit `echo "Fertig."` und 0. Sein Aufrufer `pruefen.sh:59` läuft unter `set -e` und kann es deshalb nicht bemerken. Damit steht die Meldung wieder nur im Protokoll — der Wortlaut des Befundes K3. | offen |
| N5 | 4 Goal-Driven | warn | `…/s100-eingabefeld/mutationsproben.sh:89–119` gegen `:42–48` | `ohne_grafik()` hat **gar keine** Eingriffs-Wache, und dort wird sie dringender gebraucht als in `lauf()`: Probe 9 erwartet ausdrücklich **grün**, also sieht ein `sed`, das nicht mehr trifft, exakt aus wie ein geglückter Eingriff. In `m2-mutationsproben.txt:67–92` sind die Ergebnisse von Probe 8 (Eingriff `true`) und Probe 9 bis auf die Laufzeit **identisch** — 29 passed, 0 failed, 9 skipped, dieselben neun Namen. Der Beleg für „die Kehrseite" hängt allein daran, dass der Eingriff angekommen ist, und niemand prüft das. Das ist die Falle, gegen die zwei Zeilen weiter oben die Prüfsumme steht. | offen |
| N6 | 4 Goal-Driven | warn | `…/s101-listentrenner/messungen/mutationsprobe.txt:2` gegen `:30`; `…/messungen/b7-strichstaerke.txt:2` | Beide Messungen nennen den Stand **`46bb5b5`**. Dort gibt es `hairline()` nicht — `git show 46bb5b5:src/ui/notelistdelegate.cpp` zeigt die beiden alten `QRect`-Füllungen. Probe 7 mutiert aber eine Zeile aus `hairline()`, und B7 misst die berichtigte Stärke. Ursache: `mutationsprobe.sh:60` und `pruefen.sh:35` nehmen den Stand aus `git rev-parse HEAD`, während die Korrektur noch unbeschrieben im Arbeitsbaum lag. Wer die Belege am genannten Stand nachfahren will, kann es nicht, und der Widerspruch liest sich schlimmer, als er ist. | offen |
| N7 | 4 Goal-Driven | warn | `.github/workflows/ci.yml` (Schritt „Tests"), `…/s100-eingabefeld/messungen/m7-ohne-plasma-grafik.txt:54` | Der öffentliche Lauf ist jetzt dauerhaft grün über **neun übersprungenen** Prüfsätzen, sechs davon die Feldzusicherungen von #100 — und **nichts in seiner Ausgabe sagt das**. `ctest --output-on-failure` zählt ein `QSKIP` als bestanden und meldet „100% tests passed out of 10". Vor der Heilung war der Läufer rot, also laut; jetzt ist er still. Der Bericht führt es sauber (§7 und Proben 8/9) — die Marke am öffentlichen Repositorium führt es nicht, und gelesen wird die Marke. | offen |

### Stand der neun Befunde des ersten Durchgangs

Nachgeprüft, nicht übernommen. Kein Befund verschwindet stillschweigend.

| ID | Status | woran geprüft |
|----|--------|---------------|
| K1 | behoben | Lauf `31221410562` auf HEAD: `completed` **und** `success` (selbst abgefragt). Bericht §3 trägt den datierten Vermerk, die falsche Fassung bleibt lesbar. Der zweite Teil — die Kopplung an `hullHoldsAtTheCustomersScale()` — ist mit `fieldHoldsAtTheCustomersScale()` aufgelöst; `m2-mutationsproben.txt` zeigt Proben 1, 2, 3, 6 und 7 jetzt auf dem Feldsatz. Reststück: **N3**. |
| K2 | behoben | `whyNoFieldGraphic()` hängt an der gemessenen Ursache, nicht an einer Umgebungsvariablen; in `fieldCoverageIsTheThemesOwn()` steht der Skip jetzt **vor** dem `QVERIFY2`. M7: 29 passed, 0 failed, 9 skipped statt 28/6/3. Auf Ganymed greift der Wächter bei keinem einzigen Prüfsatz — kein zu breiter Skip. |
| K3 | behoben für #101 | `mutationsprobe.sh:175–181` beendet mit 1; Probe 5 läuft und ist rot. Für #100 nur halb: **N4**, **N5**. |
| K4 | behoben | `bericht.md:221–224` durchgestrichen, `:260` sagt jetzt, dass 10c und 10d die Notizliste zeigen, samt der wirklichen Ursache. |
| K5 | behoben (Vermerk statt Bild) | `README.md:49–55` sagt, welchen Stand die Bilder zeigen und was seither dazugekommen ist, mit Verweis auf #96 als Grund. PO-Entscheidung, nachvollziehbar begründet. |
| K6 | behoben | `kcolorscheme` in `ci.yml:78`, mit Begründungszeile in `:66–74`, dazu die gemessene Herkunft der transitiven Auflösung. |
| K7 | behoben | `initTestCase()` liest den alten Wert **nach** `QStandardPaths::setTestModeEnabled(true)` (`librarytest.cpp:514`, `:535`) — gelesen wird also der Sandkasten, zurückgeschrieben derselbe. `cleanupTestCase()` löscht den Schlüssel, wenn er vorher fehlte. Läuft auch im Einzelfunktionslauf. |
| K8 | behoben | `pruefen.sh:99–107` legt M3a **und** M3b ab, Prüfsatz für Prüfsatz. |
| K9 | behoben | `besideTheField()` nimmt die Mitte zwischen ausgelegtem Namensschild und Textbereich; `paintsTheThemesOwnHullInOnePiece()` sichert zusätzlich zu, dass beide Abgriffe zwischen ihren Nachbarn liegen. Beim Nachziehen ist die Signaturänderung durch Mutationsprobe 5 gelaufen und wurde gefunden — das Skript trägt die Lehre jetzt als Prüfsumme. |

### Fix-Vorschläge

- **N1** → Zweierlei, und das erste ist das eilige: Den Satz in `SPEC.md:739`
  und in P6 der Zeichnung auf das setzen, was trägt — *eine ganze Zahl von
  Gerätebildpunktzeilen*, ohne die Aussage über die Oberkante. Danach
  entscheiden, ob `std::round(top · ratio) / ratio` bleibt: Es kostet nichts und
  schadet nichts, aber es ist ein Term ohne Wirkung an einer Stelle, an der
  gerade sehr genau begründet wurde, warum jeder Term dort steht. Bleibt er,
  gehört ein Satz dazu, der sagt, dass er die Zusicherung **nicht** trägt.
  Zusätzlich: Mutationsprobe 7 in **zwei** Eingriffe teilen — einmal nur die
  Höhe zurück auf `1`, einmal nur die Oberkanten-Ausrichtung heraus. Der zweite
  bliebe grün, und genau das wäre der Beleg, den dieser Befund vorwegnimmt.
- **N2** → In der Zeichnung „aufgerundete" durch „gerundete" ersetzen, oder —
  falls die Zeichnung recht behalten soll — `std::round` durch `std::ceil`.
  Beides ist eine Entscheidung des PO mit dem Kunden, weil sie sein Bild bei
  125 % betrifft. Die Zahlen dafür stehen oben. Was nicht bleiben kann, ist der
  Widerspruch: Der Code liefert bei 1,25 genau das, was die Begründung daneben
  als das bezeichnet, was nicht passieren darf.
- **N3** → `hullHoldsAtTheCustomersScale()` seine Vorbedingung im Elternteil
  fragen lassen, wie `fieldHoldsAtTheCustomersScale()` es tut — oder
  `runAtTheCustomersScale()` die Ausgabe des Kindes auf `SKIP` durchsehen und
  den Übersprung an den Elternteil weiterreichen. Der zweite Weg deckt beide
  Aufrufer auf einmal.
- **N4** → In `mutationsproben.sh` einen Abbruchzähler führen und am Ende mit
  `exit 1` schließen, wie es das Schwesterskript tut. Vier Zeilen.
- **N5** → Die Prüfsummen-Wache aus `lauf()` in `ohne_grafik()` übernehmen. Sie
  steht dort schon geschrieben; sie muss nur auch für die zweite Funktion
  gelten.
- **N6** → In den Kopf beider Skripte einen Vermerk aufnehmen, wenn der
  Arbeitsbaum verändert ist:
  `git diff --quiet || echo " (Arbeitsbaum verändert gegenüber diesem Stand)"`.
  Danach die beiden abgelegten Messungen entweder am festgeschriebenen Stand
  nachfahren oder ihren Kopf von Hand berichtigen — mit dem Vermerk, warum.
- **N7** → Eine Zeile in den Testschritt, die die Zeile `Totals:` von
  `capturetest` ausgibt (oder die Zahl der Übersprungenen nennt). Wer schärfer
  will: gegen eine im Arbeitsablauf hinterlegte Erwartungszahl prüfen, damit ein
  **neuer** Übersprung auffällt. Die Entscheidung gehört dem PO; die
  Mindestforderung ist, dass die Zahl überhaupt sichtbar wird.

### Was gut ist

- **Die L9-Korrektur trägt, und sie trägt in der Sitzung des Kunden.** Das
  Abnahmebild `sprint-09-abnahme/bilder/hauptweg-bibliothek-zuschnitt.png`
  nachgemessen: fünf Eintragslinien (352 Bildpunkte breit) bei y = 274, 389,
  675, 791, 906 und eine Gruppenlinie (390 breit) bei y = 506 — **jede einzelne
  genau zwei Gerätebildpunkte**, Farbe 66,68,70 durchgehend. Eine einzige
  Stärke, im Bild aus der angemeldeten Sitzung, auf der Skalierung des Kunden.
  Das steht in keinem Bericht und sollte dort stehen; die Abnahmetabelle prüft
  Lage und Länge, nicht die Stärke, um die es bei L9 ging.
- **Über acht Skalierungen einheitlich.** Selbst gemessen bei 1 · 1,25 · 1,4 ·
  1,5 · 1,75 · 2 und bei 1,6 mit und ohne Überfahren: In **jeder** Lage tragen
  Gruppen- und Eintragslinien dieselbe Stärke. Die Kundenfestlegung — die
  Rangfolge kommt aus der Länge, nicht aus der Stärke — hält überall. Der
  Einwand aus N2 betrifft nur, **wie stark** sie unter 1,5 ist.
- **Die eigene Fehleranalyse zum Prüfsatz ist der wertvollste Teil dieses
  Diffs.** Zwei verworfene Anläufe stehen im Bericht (§8.1) und im Prüfsatz
  selbst: der **nachgebaute** Maßstab, der bei 1,25 den Fehler zeigte und bei
  1,6 nicht — also gerade dort bestand, wo der Fehler saß —, und die zu kurze
  Szene mit vier Linien in drei Lagen. Die Rechnung dahinter stimmt: 72 logische
  Punkte Zeilenhöhe verschieben die Rasterphase bei 1,6 um ein Fünftel, acht
  Notizen laufen alle fünf durch. Nachgerechnet und an der Szene bestätigt.
- **Die Vorbedingung ist an die Messung gehängt und nicht an die Umgebung.**
  `whyNoFieldGraphic()` fragt dasselbe, was `paintEvent()` fragt, bevor es
  zeichnet; jeder Skip-Text nennt beide Hälften — welche Voraussetzung fehlt
  (mit `isValid`, Vorsatz und Rand als Zahlen) und welches Kriterium dadurch
  ungeprüft bleibt. Ein `CI=true` hätte den Prüfsatz überall stillgelegt, wo
  jemand die Variable setzt. Auf Ganymed greift der Wächter bei keinem einzigen
  Prüfsatz — der Skip ist nicht zu breit.
- **Die Kehrseite des Überspringens ist ausgesprochen statt verschwiegen.**
  Probe 9 hält fest, dass der Läufer die Mutation dort nicht fängt, wo nichts zu
  messen ist. Das ist die verlangte Prüfhaltung; sie braucht nur noch ihre Wache
  (N5).
- **Die seitlichen Kanten sind ausdrücklich ungeregelt gelassen — mit Zahl.**
  0,12 Punkte Versatz gegen 0,5 bis 2,4 Punkte Schwankung im Seitenrand der
  Glyphen daneben, in `SPEC.md:748–750` und in der Zeichnung. Gemessen, nicht
  geheilt, und der Grund steht dabei. Genau die Grenzziehung, die
  „melden, nicht heilen" verlangt.
- **Prinzip 3 hält.** 41 Zeilen Produktivcode auf 455, jede auf L9
  zurückführbar; keine Nebenbei-Verbesserung in `notelistdelegate.cpp`, keine
  angefasste Nachbarsektion in SPEC oder Zeichnung. Die Änderungen an
  `capturetest.cpp` sind Signaturfolgen von K9 und nichts sonst. Der Neubau
  läuft warnungsfrei, `ctest` ist 10 von 10 grün.
- **Prinzip 2 kein Befund** — bis auf den inerten Term aus N1. Keine
  Abstraktion für einen Fall, keine unbestellte Option; die zweite
  ctest-Anmeldung ist begründet und auf **eine** Prüffunktion beschränkt.

---

**Offene `fail`-Befunde: ja — einer (N1).**
