# Denkzettel — Arbeitsanweisung für Claude Code

Quick-Capture-Werkzeug für KDE Plasma (Wayland), C++/Qt6/KF6, CMake, QTest.
Dieses Projekt arbeitet mit einem **Agenten-Scrum-Team**. Wer hier arbeitet,
arbeitet nach dessen Regeln — sie stehen vollständig in
**`docs/scrum/PROZESS.md`** und sind vor jeder Arbeit am Produkt zu lesen.

## Die Regeln, die am häufigsten übergangen werden

Sie stehen hier, weil sie in diesem Projekt bereits verletzt wurden — jede
einzelne hat einen Mangel oder Kundenbefund im Rücken.

**Geprüft wird am installierten Stand** (`-DCMAKE_INSTALL_PREFIX=/usr`), nicht
im Build-Verzeichnis (DoD 2). *Bei mehreren gleichzeitig arbeitenden Agenten
gilt zusätzlich:* Es gibt nur ein `/usr`. Installieren zwei gleichzeitig, prüft
einer den Stand des anderen — deshalb taktet der PO die Installation, und **am
Sprint-Ende wird der Endstand einmal installiert und geprüft**. Ohne diesen
Schritt ist die Sprint-Abnahme unvollständig (Sprint-3-Mangel M1).

**Installieren heißt nicht laufen** (B16). Nach `cmake --install` hält ein
laufender Dienst die gelöschte alte Datei weiter; umgekehrt reicht
`KDBusService::Unique` den Start eines Debug-Builds an den laufenden Dienst
weiter. Beide Male prüft man unbemerkt den falschen Stand. Vor jeder Prüfung am
installierten Stand: Dienst beenden, neu starten, dann
`readlink /proc/$(pgrep -x denkzetteld)/exe` — ohne `(deleted)`. Wer den
**Debug**-Stand prüfen will, beendet vorher den installierten Dienst.

**Ein UI-Review ohne eigenes Bild ist nicht geführt** (DoD 3, Beschluss B3).
Tests ersetzen die Bildprüfung nicht, und Bilder ersetzen die Tests nicht:

> Bei Bewegungen ist der Weg der Prüfgegenstand, nicht das Ziel.
> Bei Zuständen ist das Bild der Prüfgegenstand, nicht die Zusicherung.

Für Bildläufe **muss `QT_QPA_PLATFORMTHEME=kde` gesetzt sein** — sonst
verfälscht eine Ersatzschrift die Größenverhältnisse.

**Ein offscreen erzeugtes Bild zeigt nicht, was der Kunde sieht** (B21). Es
belegt Geometrie, Textsatz und Farbrollen; es belegt **nicht** Hülle, Rundung,
Kontur, Schatten oder Dekoration — die zeichnen Theme und Compositor, und
offscreen fehlt beiden die Grundlage (gemessen am 04.08.2026 an `tinted()`, das
offscreen den Alphakanal verlor und unter Wayland nicht — **die Funktion ist mit
#83 gefallen, die Messung gilt**; seit dem 05.08.2026 tragen drei
Mutationsproben von #83 dieselbe Aussage: ohne `DevicePixelRatioChange`-Zweig,
ohne `nullptr`-Wache und ohne Weichzeichner-Anmeldung bleibt der Testlauf
offscreen **grün** und fällt in der Sitzung — Rückgabe 139 im zweiten Fall).
Zweierlei folgt daraus: Ein Bild, das als
Beleg dient, läuft mit `QT_SCALE_FACTOR` auf der Skalierung des Kunden — und wo
ein Akzeptanzkriterium über Theme oder Compositor etwas behauptet, gehört ein
Bild aus der angemeldeten Sitzung dazu.

**Ein unversionierter Beleg ist kein Beleg** (B7). Prüfberichte und Bilder
gehören nach `docs/scrum/reviews/`, nicht ins Scratchpad und nicht in einen
Arbeitsbaum, der mit dem Agenten verschwindet.

**Melden, nicht heilen.** Wer außerhalb seiner Fläche einen Fehler findet,
meldet ihn dem PO. Das gilt auch, wenn die Heilung eine Zeile wäre.

**Entdeckte Bedingungen ziehen die SPEC nach** (DoD 4 in der Fassung nach B9) —
nicht nur geänderte Festlegungen. Wenn beim Bauen herauskommt, dass etwas nur
unter einer Bedingung gilt, gehört die Bedingung in die SPEC.

**Ein Sprint endet nicht mit dem letzten Commit.** Der Abschluss steht als
Liste in `docs/scrum/PROZESS.md`, Abschnitt „Sprint-Abschluss", und wird
Punkt für Punkt im Sprint-Protokoll abgehakt: vor der Kundenabnahme
installieren, Belege ablegen, prüfen — nach ihr Issues schließen, Journal,
Push, Zweige und Worktrees räumen. Acht der neun Mängel aus Sprint 3 waren
Abschlussmängel.

**Jeder Zugang nach der Sprint-Freigabe wird gebucht** — Issues *und*
Größenklassen. Berührt er eine der beiden Grenzen (2–4 Stories; kein
`size:xl`, höchstens eine `size:l`, daneben nur `size:s`), legt der PO ihn
dem Kunden als Grenzüberschreitung vor. In Sprint 3 wurde bei jedem Zugang
die Punktzahl mitgezählt, die Zahl der Issues nicht — die Grenze fiel
niemandem auf.

**Flüchtige Belege sofort sichern.** Kundenbilder liegen in temporären
Ordnern; von acht Bildern der Sprint-3-Abnahme überlebte sieben Minuten nur
eines.

**Das `size:`-Label wird im selben Zug gesetzt wie der Vorprüfbericht —
vorher gar nicht** (04.08.2026). Der PO legt die Issues an, also entsteht die
Zweideutigkeit hier: Ein Label sieht gleich aus, ob ein Urteil aus zwei
unabhängigen Messungen dahintersteht oder eine beim Anlegen hingeschriebene
Zahl. Beides ist unter dem alten `sp:`-Label vorgekommen — #57 trug `sp:2`
aus der Anlage und wurde von zwei Schätzern auf 3 gehoben, #68 trug `sp:5`
aus einer Hand. In der *einzigen Quelle der Wahrheit* ist ein doppeldeutiges
Feld teurer als anderswo. Eine grobe Hausnummer gehört als Freitextsatz ins
Issue, der nicht wie ein Urteil aussieht. Damit ist der zweimal gerissene
Punkt (M2 in Sprint 5, K1 in Sprint 6) verankert; die Story-Point-Skala
selbst ist abgeschafft — Gründe, Größenklassen und die
Falle beim Nachprüfen des Altbestands (die alte Schätzung steht unter der
**Story-ID**, nicht unter der Issue-Nummer) stehen in `PROZESS.md`.

## Prüfhaltung

Dieses Projekt hat an einem einzigen Abend **vier grüne Tests entlarvt, die
nichts prüften**, und drei plausibel begründete Fehlentscheidungen kassiert.
Alle sieben fielen durch Messung, keine durch Nachdenken. Daraus:

- **Prüfe am Einzelfall, nicht an der Plausibilität.** Eine Begründung, die
  trägt und trotzdem den falschen Schluss stützt, fällt nicht auf.
- **Eine Vereinfachung ist erst geprüft, wenn sie gegen die zuletzt geheilten
  Fälle gehalten wurde.**
- **Prüfe die Voraussetzung deiner eigenen Begründung.** Wer sich auf eine
  Zeichnung beruft, liest sie vorher.
- **Eine Aussage gilt für einen Stand** (B17). Wer eine Bau- oder
  Werkzeugeigenschaft ändert, sucht im selben Zug nach den Aussagen darüber:
  ```
  git grep -n <Eigenschaft> -- . ':!docs/scrum/reviews' ':!docs/scrum/sprints' \
      ':!docs/scrum/retro' ':!docs/scrum/vorberichte' ':!src' ':!tests'
  ```
  **Der Griff schließt aus, statt aufzuzählen** — ausgenommen sind die
  Belegarchive, denn ein Beleg wird geankert und nicht nachgezogen. Alles
  andere wird durchsucht;
  beim Erweitern einer Aufzählung nach dem Namen eines **Geschwisters** statt
  nach dem neuen (`readmeshots` findet jede Liste, die `captureshots` noch nicht
  kennt). Gemessen: Der Griff hätte alle drei Fundstellen von Sprint-6-Mangel
  M4 gezeigt, bei fünf Zeilen Ausgabe. Wer den Zustand ändert und die Aussage
  stehenlässt, macht aus einer Begründung eine Falle — die Pflicht gilt dann
  weiter, nur aus einem anderen Grund.
  **`wireframes/` und `SPEC.md` stehen seit dem 05.08.2026 in der Liste, und
  zwar auf einen Fehlschlag hin.** In Sprint 7 hat der PO den Griff selbst
  gefahren, fünf Zeilen Ausgabe bekommen und ihn für vollständig gehalten. Der
  karpathy-Reviewer fand danach, dass Zeichnung 4a/4b weiterhin den vom Kunden
  **abgewählten** Nachbau zeigt — samt der Kontur, die es nicht mehr gibt — und
  dass `SPEC.md` in der Überschrift von 3.1 darauf verweist. Beides war für den
  Griff unsichtbar. **Ein Werkzeug, dessen Suchraum kleiner ist als der
  Geltungsbereich der Regel, meldet Vollständigkeit und liefert sie nicht** —
  und es meldet sie besonders überzeugend, weil es Treffer hatte.
  **Warum ausschließen und nicht aufzählen — das ist der eigentliche Lehrsatz,
  und er hat zwei Runden gebraucht.** Der Griff zählte auf und übersah
  `wireframes/`; beim Beheben trug der PO `wireframes/` und `SPEC.md` nach und
  übersah `.claude/` — **obwohl eine der drei Fundstellen desselben Vorgangs
  dort lag** und er sie nur gefunden hatte, weil er `.claude/` beim Suchen von
  Hand angehängt hatte. Der Scrum Master fand es in der DoD-Prüfung desselben
  Sprints (M5). **Wer ein zu enges Werkzeug erweitert, erweitert es um das,
  woran er sich erinnert** — die Lücke danach sieht aus wie keine. Gegen
  `git ls-files | sed 's|/.*||' | sort -u` gehalten fehlten der zweiten Fassung
  weiterhin `cmake/`, `desktop/`, `KONZEPT.md` und `CHANGELOG.md`.
  **Gemessen ist die Ausschlussform zugleich vollständiger und leiser:** vier
  einschlägige Zeilen statt sechzig, weil die Belegarchive nicht mitkommen.
- Ein Testaufbau, in dem der Fehler gar nicht auftreten *kann*, ist kein Test.
- **Ein Bildbeleg ist erst ein Beleg, wenn sein Läufer frisch gebaut ist.**
  Es gibt fünf: `editshots`, `libraryshots`, `searchshots`, `readmeshots`,
  `captureshots`. Ein veralteter Läufer schreibt plausible Bilder eines
  **alten** Standes mit frischem Zeitstempel (Vorfall Sprint 5: grüner Test
  und falsches Bild zugleich). Vor jedem Bildbeleg:
  `cmake --build build --target <läufer>`.
  Seit dem 04.08.2026 sind sie **nicht mehr** `EXCLUDE_FROM_ALL` — ein
  gewöhnlicher Build fasst sie an. Das entschärft die Falle, hebt sie aber
  nicht auf: Wer den Läufer startet, ohne vorher zu bauen, bekommt weiter das
  alte Bild. Die Regel gilt unverändert, nur ihr Grund ist ein anderer.
- **Kein Agent kann sich unter Wayland den Fokus zurückholen** (Sprint 6,
  §16.1 M-B1). `activateWindow()` tut es nicht, ein Alt-Tab lässt sich nicht
  auslösen — dazu bräuchte es ein xdg-activation-Token. Wer eine Prüfung mit
  Fensterwechsel baut, nimmt den compositor-getriebenen Weg: das obenauf
  liegende Fenster **schließen**, dann gibt der Compositor den Fokus von
  selbst zurück. Der erste Sichtlauf, der es anders versuchte, maß nichts und
  sah dabei aus wie ein Beleg. Weitere Fälle dieser Art führt
  `.claude/agents/denkzettel-dev.md`.

## Retrospektiven

**Ergebnisse sind Änderungen, keine Absichtserklärungen** — Skills, Regeln,
Agentendateien, Memory-Einträge oder Anpassungen an `PROZESS.md`. Eine
Erkenntnis, die nur im Sprint-Protokoll steht, ist nicht verankert; sie wirkt
erst, wenn sie an dem Ort steht, den die nächste Sitzung von selbst liest.
**Das ist die häufigste Lücke: die Regel existiert, aber niemand sieht sie.**

Prüfe deshalb am Ende jeder Retro ausdrücklich: Ist jeder Beschluss in einem
Artefakt gelandet — und wird dieses Artefakt automatisch geladen?

## Rollen

| Rolle | Wer |
|---|---|
| Kunde | hnsstrk — Ziele, Prioritäten, Freigaben, Abnahme |
| Product Owner | Claude (Haupt-Session) — Backlog, Story-Schnitt, AK, Kundenkontakt |
| Scrum Master | Agent `scrum-master` — DoD, Schätz-Moderation, Retro, Impediments |
| Entwickler | Agent `denkzettel-dev` |
| UI/UX | Agent `denkzettel-ux` |
| Verwalter | Agent `denkzettel-verwalter` (Haiku) — führt Abschlussarbeit aus, entscheidet nicht |

**Der PO schreibt keinen Produktivcode**, der Scrum Master ändert weder Code
noch SPEC, und Agenten arbeiten nur in ihrer zugewiesenen Dateimenge.

## Technisches

- Bauen: `cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug && cmake --build build`
- Testen: `ctest --test-dir build`
- Linter: `cmake --build build --target lint-tidy` bzw. `lint-clazy`
- Installieren (braucht das Kundenpasswort, grafischer Dialog):
  `pkexec /usr/bin/cmake --install ~/Projekte/denkzettel/build`
- Das Repository ist **öffentlich**. Was in Issues und Commits steht, ist
  veröffentlicht. Zugelassen sind Kundenzitate und Messwerte; nicht
  zugelassen sind Systemdetails und personenbezogene Angaben
  (Kundenentscheidung 02.08.2026).
- **Gepusht wird nach jedem abgeschlossenen Arbeitsblock, ohne Rückfrage**
  (Kundenentscheidung 02.08.2026). Das Sprint-Ende-Minimum regelt der
  Sprint-Abschluss in `PROZESS.md`.
  **Jeder Push auf `main` löst einen öffentlichen Bau- und
  Testlauf aus** (`.github/workflows/ci.yml`). Er schlägt bei jeder
  Compiler-Warnung, jedem roten Test **und seit #76 (05.08.2026) bei jedem
  Linterbefund** fehl — `lint-tidy` und `lint-clazy` stehen beide auf Schwelle
  null, und die Wache prüft die Dreizahl (Rückgabewert, Warnungen, Fehler)
  statt `grep`-Zeilen zu zählen. Seine Marke steht am öffentlichen Repository.
  Wer pusht, sieht nach — **am Lauf des eigenen Commits**, nicht am obersten
  der Liste (B18):
  `gh run list --commit $(git rev-parse HEAD) --json status,conclusion --jq '.[]|[.status,.conclusion]|@tsv'`
  Erst `completed` **und** `success` ist ein Nachschlag. `in_progress` heißt:
  noch einmal — nicht „nichts Rotes gesehen". Umgekehrt ist die grüne Marke
  **kein** DoD-Nachweis — der Lauf erreicht DoD 2 und DoD 3 gar nicht.
