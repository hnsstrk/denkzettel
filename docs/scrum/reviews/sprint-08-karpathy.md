# Karpathy-Review Sprint 8

**Prüfgegenstand:** `git diff sprint-08-basis..main` (Endstand `f8b7711`) —
138 Dateien, #85 + #61 + #76, dazu Sprint-Protokoll, UI-Review, PO-Änderungen
an SPEC/CLAUDE.md/PROZESS.md/Agentendatei. Prüfer: karpathy-reviewer
(Fable 5, frischer Kontext), 05.08.2026. Geprüft am Diff und an den Quellen
(`metadata.desktop` der Themes, Messprotokolle, Testcode), nicht an den
Behauptungen der Berichte.

**Task:** Drei Sprint-Stories liefern — Lesbarkeit unter fremden Themes aus
derselben Quelle wie die Fläche (#85), Versionsanzeige mit einer einzigen
Versionsquelle (#61), Linterschwelle auf null mit begründeten Ausnahmen (#76)
— samt Belegen nach B7.

**Gesamt-Verdict:** warn

## Befunde

| ID | Prinzip | Verdict | Ort | Befund | Status |
|----|---------|---------|-----|--------|--------|
| K1 | 4 Goal-Driven | warn | docs/scrum/reviews/sprint-08-s85-lesbarkeit/mutationsproben.sh:108–110 | M4 („Tor liest `[Colors:View]`") ist am Fehlerbild von M3 nicht unterscheidbar: beide hinterlassen dieselben fünf FAIL-Sätze bei 27/5 (p8-mutationsproben.txt), weil das Prüfgut (`tests/themes/…/denkzettel-test-breit/colors`) nur `[Colors:Window]` trägt — „falsche Gruppe" und „keine Datei" kollabieren beide auf `return {}`. Zwölf Eingriffe sind es, aber M4 belegt keinen eigenen Sachverhalt über M3 hinaus; die Gruppenwahl als Zusicherung ist so nicht gemessen | offen |
| K2 | 1 Think First | warn | docs/scrum/reviews/sprint-08-s85-lesbarkeit/bericht.md:312–336 | Übergabebericht #85 §4 führt eine an der Quelle widerlegte Begründung ohne Berichtigungsvermerk weiter: „genau dafür trägt das Theme seine `[ContrastEffect]`-Gruppe" für `cachyos-emerald-light` — dessen `metadata.desktop` trägt keine (selbst nachgemessen: die Gruppe haben nur `Iridescent-round`, `cachyos-emerald`, `cachyos-emerald-color`). SPEC 3.2 Punkt 10 und UI-Review P2 sind berichtigt; der Strangbericht liest sich weiter als geltend, obwohl das Projekt Berichtigungsvermerke in Berichten praktiziert (#76 §14) | offen |
| K3 | 3 Surgical | warn | docs/scrum/vorberichte/76-linterbefunde/pruefen.sh:45–54 · sprint-08-s76-linterbefunde/bericht.md:581–584 | Das Vorprüfskript zu #76 wurde im Strang-Commit `e0f21c6` geheilt (`xargs -a` statt ungeschütztem `$DATEIEN`, Dateizahl wird vorgelesen), der Bericht §15 beschreibt die Falle dort aber als bestehend („Wer das Skript mit zsh startet, bekommt eine plausible Zahlenfolge über nichts") und weist die Heilung nirgends aus. Vorberichte sind zudem nicht die Dateimenge des Strangs — entweder fehlt die Meldung an den PO oder die Berichtszeile; in beiden Lesarten widersprechen sich Bericht und Diff | offen |
| K4 | 4 Goal-Driven | warn | docs/scrum/reviews/sprint-08-ui-review/bericht.md:161 | UX-Befund P1 (Rangfolge-Umkehr: die gedämpfte Klasse ist unter zwei Themes lesbarer als der Notiztext, 4,64:1 gegen 2,06:1) hat als einziger der vier UX-Befunde keinen gebuchten Folge-Schritt. P2 ist in SPEC geheilt (`43e92dd`), P3 über den Cursor-Spiegelstrich in SPEC 3.1 (`3f4e605`), P4 durch die UX-Nachmessung selbst geschlossen. Die vorgeschlagene „eigene Frage an den Kunden" steht in keinem Issue und nicht im Sprint-Protokoll — vor dem Füllen von §6–8 sichtbar machen, sonst geht sie unter | offen |

## Fix-Vorschläge

- **K1** → Dem Prüfgut eine `[Colors:View]`-Gruppe mit abweichenden Werten
  geben (drei Zeilen); M4 fällt dann mit den View-Werten statt mit dem
  Schemarückfall und misst die Gruppenwahl wirklich. Alternativ M4 im Bericht
  als Doppelung von M3 ausweisen — dann sind es zwölf Eingriffe über elf
  Sachverhalte, und genau das war der Sprint-7-Zählfehler in anderer Gestalt.
- **K2** → Datierter Berichtigungsvermerk in §4 des #85-Berichts, Verweis auf
  UI-Review P2 und SPEC 3.2 Punkt 10 — dieselbe Form, die #76 §14 für den
  `--help`-Ordner vormacht.
- **K3** → Einen Satz in #76 §15: das Vorbericht-Skript ist im selben Zug auf
  `xargs -a` umgestellt worden — oder, wenn das PO-Fläche war, die Buchung
  beim PO nachholen.
- **K4** → P1 als Issue buchen oder als offene Kundenfrage ins
  Sprint-Protokoll aufnehmen, bevor Takt 2 läuft.

## Die vier Fragen des PO

1. **37 `NOLINT` — Aufräumen oder Aufweichen? Aufräumen, die Entscheidung
   trägt.** Nachgezählt: 37 Marken (29 `misc-const-correctness`,
   8 `easily-swappable-parameters`) in 12 Dateien, Verteilung deckungsgleich
   mit der Berichtstabelle. Das Argument zu Entscheidung 9 ist sprachlich
   korrekt: `DialogWatch` (tests/librarytest.cpp:64) schreibt `m_appeared`
   aus einer Timer-Lambda; auf einem `const`-definierten Objekt ist das UB
   nach [dcl.type.cv]/4, und ein grüner `-O2`-Lauf kann UB grundsätzlich
   nicht widerlegen — der Bericht sagt genau das und behauptet keinen
   Messwert, den es nicht gibt. Es wurde keine Zahl gegen ein unmessbares
   Argument getauscht, sondern eine unmessbare Zusicherung gegen eine
   messbare Zahl verteidigt; die Wette ist einseitig und richtig herum. Die
   Erweiterung 16 → 20 Stellen ist als Abweichung ausgewiesen und begründet
   (Regel statt Diff; vier gleichlautende Bestandszeilen).
2. **Mutationsproben #85 nachgezählt:** zwölf Eingriffe (M1–M5, M6, M6b, M7,
   M9, M10, M8, M11), Grundlinie 32/0 im Protokoll, alle zwölf rot, die
   FAIL-Sätze je Probe decken sich mit der Berichtstabelle. Elf treffen je
   einen eigenen Sachverhalt; M4 nicht — Befund K1.
3. **#61 — einfachste Lösung, die trägt: ja.** Die Auslagerung nach
   `src/shell/appidentity.cpp` ist testbedingt (die Registrierung muss aus
   `identitytest` im Prozess aufrufbar sein; `denkzettelshell` ist verlinkbar,
   `main.cpp` nicht). Die zwei Testbinärdateien messen Verschiedenes und
   brauchen verschiedene Umgebungen: `identitytest` in-process offscreen,
   `commandlinetest` den gebauten Dienst unter `dbus-run-session` mit
   entleertem `XDG_DATA_DIRS`. `expectRefusal()` (tests/commandlinetest.cpp:88)
   unterscheidet Zurückweisung von Absturz über den erreichbaren Bus — die
   Konstruktion ist der Kern des Prüfwerts, nicht Beiwerk. Die Ablehnung von
   `setupCommandLine()` ist begründet und durch M7 gedeckt. Kein Bloat;
   SPEC 15.1 war AK 8.
4. **Grenzen: überwiegend ehrlich gezogen.** „Zugesichert ist die Herkunft,
   nicht die Kontrastzahl" ist präzise, in SPEC 3.1 als eigener Spiegelstrich
   verankert, und die Verluste stehen mit Zahlen im Bericht (20,35 → 1,37
   unter `cachyos-emerald-light`). Der gescheiterte `-O2`-Beleg, der
   Release-Befund (#99), der `--help`-Ordner (#98) und die eigenen Fehlläufe
   der Sonden sind offen berichtet. Die zwei Ausnahmen sind K2 und K3 —
   beides Berichtslücken, keine Produktdefekte.

## Was gut ist

- **Der Produktcode selbst ist ohne Befund.** Vorrangregel an einer Stelle
  (`applyTextColours()`), beide Anlässe laufen durch; `themeTextColoursOf()`
  nimmt denselben Griff wie der Nachbar `contrastEffectOf()`;
  `appidentity.cpp` beseitigt die Doppelsetzer-Falle statt sie zu umgehen.
- **Prüfgut-Farben (255,0,153 / 0,153,255), die kein Farbschema trägt** —
  damit ist Herkunft von Wert unterscheidbar; das mitgelieferte Theme-Paar
  löst die Prüfung von der Paketlage der Maschine.
- **Selbstkorrekturen mit benanntem eigenen Fehler:** SPEC 3.2 Punkt 10
  erklärt, warum die erste Fassung falsch war; #76 §14 berichtigt den eigenen
  Prüfausdruck zum `--help`-Ordner; beide Mutationsläufe dokumentieren ihre
  wertlosen Erstdurchläufe (rote Grundlinie, fehlendes offscreen).
- **Der UI-Review misst unabhängig** (`schriftimbild.py` kennt keine Palette)
  statt Strangzahlen zu übernehmen — so wurde P2 überhaupt gefunden.
- **CI-Dreizahl mit gemessenen Lücken** (rc-blindes clazy, stiller Nulllauf)
  und die Schwelle zusätzlich in DoD 1 — die Retro-Regel „Ergebnisse sind
  Änderungen" ist eingehalten, ebenso mit Fall 10 in `denkzettel-dev.md`.
- **„Melden, nicht heilen" bei #98 und #99 eingehalten**; die PO-Änderungen
  an CLAUDE.md, README und PROZESS.md bleiben eng am Sprint-Anlass (B17).
- **Prinzip 1 und 3 sonst sauber:** Annahmen stehen mit Quelle (B17-Stände in
  jedem Bericht), K7/K8/K10 sind als Entscheidungen mit Kehrseite
  protokolliert, und ich habe im Diff keine unangefragte Nebenverbesserung
  gefunden — die eine Kandidatin ist K3.

**Hinweis außerhalb der Verdicts:** Der CI-Lauf des Endstands `f8b7711` stand
zur Prüfzeit auf `in_progress` — der B18-Nachschlag (`completed` **und**
`success`) steht noch aus und gehört dem Sprint-Abschluss.

---

## Vollzugsvermerk des Product Owners, 05.08.2026

Angehängt, nicht eingearbeitet (B17). Die Statusspalte oben beschreibt den Stand
zum Zeitpunkt des Reviews.

| ID | Stand |
|---|---|
| **K1** | **angenommen**, an Strang A zurück. Zwölf Eingriffe, elf Sachverhalte — M3 („falsche Gruppe") und M4 („keine Datei") fallen beide auf `return {}` zusammen, weil das Prüfgut nur `[Colors:Window]` trägt. Dieselbe Fehlerklasse wie K1 in Sprint 7, und deshalb kein Schönheitsfehler: AK 6 verlangt eine Probe **je tragender Zusicherung** |
| **K2** | **angenommen**, an Strang A zurück. SPEC 3.2 Punkt 10 und der UI-Review sind bereits berichtigt; der Strangbericht führt die widerlegte Begründung als einziger weiter und bekommt eine datierte Zeile |
| **K3** | **teils gegenstandslos, teils angenommen.** Dass der Strang `docs/scrum/vorberichte/76-linterbefunde/pruefen.sh` angefasst hat, war **vom PO ausdrücklich beauftragt** — dieselbe zsh-Falle, und ein Prüfskript, das unter einer anderen Shell stumm nichts tut, ist schlimmer als keines. Es war also keine Überschreitung der Fläche. **Was trägt:** Der Bericht beschreibt die Falle als bestehend und weist die Heilung nicht aus. Vom PO unten nachgetragen |
| **K4** | **bereits erledigt, für den Reviewer nicht sichtbar.** UX-Befund P1 ist am 05.08.2026 als **#97** gebucht („Unter zwei Themes ist der Platzhaltertext besser lesbar als die Notiz"), ausdrücklich als Frage an den Kunden und nicht als Mangel an #85. Die vier UI-Befunde haben damit alle einen Folge-Schritt: P1→#97, P2→SPEC 3.2 Punkt 10, P3→SPEC 3.1, P4→erledigt. *Der Reviewer prüft den Diff; eine Buchung im Backlog steht dort nicht — das ist eine Grenze des Prüfwegs, kein Fehler des Reviews* |

### Nachtrag zu K3 (PO)

`docs/scrum/vorberichte/76-linterbefunde/pruefen.sh` **ist geheilt** — der
Strang hat die Dateiliste auf `xargs -a` umgestellt und liest die Dateizahl
vor, Commit `e0f21c6`. Der Befund in §15 seines Berichts beschreibt den Stand
**vor** dieser Heilung. Er bleibt stehen, weil er die Falle beschreibt; **hier
steht, dass sie an dieser Stelle nicht mehr wirkt.**

### Zur ersten Frage des PO, weil die Antwort schärfer ist als die Frage

Gefragt war, ob die 37 `NOLINT` Aufräumen oder Aufweichen sind — und ob der PO
„eine Zahl gegen ein Argument getauscht hat, das sich nicht messen lässt". Die
Antwort des Reviewers dreht die Sache um und trifft sie damit besser:

> Ein grüner `-O2`-Lauf kann undefiniertes Verhalten prinzipiell nicht
> widerlegen. Getauscht wurde keine Zahl gegen ein unmessbares Argument,
> sondern **eine unmessbare Zusicherung gegen eine messbare Zahl verteidigt.**
