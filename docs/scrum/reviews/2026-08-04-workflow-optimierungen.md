# Workflow-Optimierungen — Ermittlung vom 04.08.2026

**Datum:** 04.08.2026, 08:57 (Ganymed) · **Ermittler:** Scrum Master (Agent
`scrum-master`) · **Auftraggeber:** Product Owner, im Kundenauftrag vom
04.08.2026 (*„weitere Optimierungen für unseren Workflow ermitteln und
berichten"*).

**Prüfgegenstand:** `main` @ `5294094`. **Nichts geändert** außer dieser Datei —
der Auftrag ist eine Ermittlung, die Entscheidung liegt beim Kunden.

**Methode.** Jede Zahl unten steht auf einem Befehl, den ich selbst gefahren
habe, in einem eigenen Bau außerhalb des Projektbaums (der Arbeitsbaum wurde
nicht angefasst, es arbeiten weitere Agenten parallel). Wo ich einen fremden
Bericht nur zitiere, steht es dabei. Beobachtung und Schluss sind getrennt.

**Ausgeklammert, weil heute schon entschieden:** automatische Testläufe (CI),
Existenzprüfung des Verwalter-Berichts, Pflicht-/Kürteil der Protokolle,
Rückbau des Schätzkegels, Wiederaufnahme des PR-Verfahrens.

---

## Teil A — Beobachtungen (was gemessen ist)

### A1 · Der Linterlauf der Sprint-5-DoD-Prüfung war unvollständig

**Beobachtung.** Das Sprint-5-Protokoll (§13) führt als eigene Messung:
`lint-tidy` → *„rc=0, drei Befunde auf `librarywindow`"*, und schließt daraus
in §14: *„Keine Linterbefunde aus diesem Sprint."* Prüfgegenstand war `7247500`.

Ich habe `lint-tidy` heute an `5294094` gefahren. Zwischen beiden Ständen sind
`src/`, `tests/`, `.clang-tidy`, `CMakeLists.txt` und `cmake/` **byteidentisch**
(`git diff 7247500..HEAD --` auf diese Pfade ist leer). Die Werkzeuge sind
dieselben: clang-tidy LLVM 22.1.8, seit dem 09.07.2026 nicht aktualisiert.

| | Sprint-5-Protokoll | mein Lauf heute |
|---|---|---|
| Befunde `lint-tidy` | 3 | **72** (eindeutig nach Datei:Zeile:Prüfung) |
| davon auf `librarywindow` | 3 | 3 — **dieselben drei** |
| in Dateien, die Sprint 5 geändert hat | 0 | u. a. `tests/librarytest.cpp:3304, :3322, :3323` |
| in Dateien, die seit Sprint 5 unverändert sind | — | 11 (`src/main.cpp`, `src/shell/`, `src/store/`) |

`tests/librarytest.cpp` hatte bei `sprint-05-basis` 3091 Zeilen und bei
`7247500` bereits 3440 — die Befunde jenseits Zeile 3091 sind Arbeit aus
Sprint 5 und standen zum Prüfzeitpunkt da.

**Schluss.** Ein Prüfschritt, dessen Ergebnis in einer DoD-Aussage endet, hat
einen Bruchteil des Codes gesehen. Die Ursache lässt sich nachträglich nicht
mehr feststellen; sie muss auch nicht — dass ein von Hand angestoßener Lauf
still unvollständig sein kann und trotzdem `rc=0` meldet, ist der Punkt.

### A2 · Beide Linter — Laufzeit und heutiger Stand

Gemessen an `5294094`, warmer Bau, `-j 12`:

| Lauf | Dauer | Rückgabewert | Ergebnis |
|---|---|---|---|
| `lint-tidy` | **5 s** (zweimal gemessen) | 0 | 72 eindeutige Befunde: 59× `performance-enum-size`, 55× `misc-const-correctness`, 7× `bugprone-easily-swappable-parameters`, je 1× `performance-no-automatic-move`, `bugprone-unused-return-value`, `bugprone-implicit-widening-of-multiplication-result` (Rohzeilen, vor Entdopplung 130) |
| `lint-clazy` | **47 s** | **2** | 3 Befunde (2× `range-loop-detach`, 1× `detaching-temporary`) **plus ein harter Fehler** |
| Vergleich: `ctest` | 6 s | 0 | 7/7 |
| Vergleich: Bau | 0 Warnungen | 0 | — |

Der harte Fehler ist reproduzierbar und hat nichts mit dem Code zu tun:
`tests/spellfixspike.cpp` bindet `spellfixspike.moc` ein, und ohne
`-DDENKZETTEL_SPIKE_SPELLFIX=ON` wird die Datei nie erzeugt. **`lint-clazy`
endet in der Vorgabekonfiguration deshalb immer mit `rc=2`** — als Tor heute
unbrauchbar.

### A3 · `EXCLUDE_FROM_ALL` beschädigt auch die Linter, und das Gegenmittel kostet 7 Sekunden

**Beobachtung.** Beide Linter melden für `editshots`, `libraryshots` und
`searchshots` `no such include directory: …/<ziel>_autogen/include`. Der Grund
ist derselbe wie beim Bildvorfall aus Sprint 5
(`docs/scrum/reviews/sprint-05-s-symbole/bericht.md`, §9.3): Die drei Ziele sind
`EXCLUDE_FROM_ALL`, werden von einem gewöhnlichen Bau nicht angefasst, und ihr
Autogen-Verzeichnis entsteht nie.

Gemessen: `cmake --build … --target editshots libraryshots searchshots -j 12`
kostet **7 s**. Ein Bau ohne Änderung danach: 0 s.

Die Begründung im Quelltext (`tests/CMakeLists.txt:55–57`) trägt das
`EXCLUDE_FROM_ALL` nicht: Sie erklärt, warum es **kein `add_test()`** gibt (*„a
broken screenshot writer must not colour the suite red"*) — das bliebe beim
Bauen unberührt.

### A4 · Bildbelege: 23 Dateinamen, 6 Aufnahmen

**Beobachtung.** Über alle 257 Bilder unter `docs/scrum/` gerechnet, gibt es
**23 Gruppen prüfsummengleicher Dateien innerhalb ein und desselben
Belegordners** (verschiedene Namen, identische Bytes). Auswahl:

| Ordner | Dateien | Aufnahmen |
|---|---|---|
| `2026-08-01-capture-theme/` (`achse3-huelle-*`) | 23 | **6** |
| `sprint-04-s8-ui-review/` | `n04-waechter-abbrechen.png` = `n05-waechter-auswahlwechsel.png`; `n06-nach-verwerfen.png` = `n07-nach-speichern.png`; `n01-lesen.png` = `n03-lesen-nach-abbrechen.png`; `03-bearbeiten-frisch.png` = `16-sprung-bearbeiten.png`; `01-lesen.png` = `17-sprung-lesen.png` | je 1 |
| `sprint-05-installationstakt/` | `bibliothek-installiert.png` = `schema-dunkel-installiert.png` | 1 |

**Der letzte Fall ist der Beweis für den Wert der Prüfung**: Genau ihn hat ein
Mensch in Sprint 5 von Hand gefunden und als V2 protokolliert (*„Zwei Belege
sind erst zwei, wenn sie zwei Aufnahmen sind"*). Eine Prüfsumme findet ihn in
einer Sekunde — und 22 weitere dazu.

**Schluss, vorsichtig.** Nicht jede Gruppe ist ein Mangel: Ein Zustand, der nach
einer Rückkehr wieder derselbe ist, darf zweimal gleich aussehen (`s54-theme-treue`,
`start-dunkel` = `zurueck-dunkel`). Ein Befund ist es, wo zwei **verschiedene**
Zustände denselben Beleg tragen — `n04`/`n05` (Abbrechen gegen Auswahlwechsel)
ist so ein Fall, die sieben `cachyos-emerald`-Hüllen sind ein zweiter. Der
Merksatz aus Sprint 5 steht bislang nur als Prosa in einem Bericht.

### A5 · `sp:`-Label: 24 von 36 ohne jede Protokollspur

**Beobachtung.** 36 der 47 offenen Issues tragen ein `sp:`-Label. Für **24**
davon findet sich in **keinem** Sprint-Protokoll auch nur eine Erwähnung der
Issue-Nummer — eine Schätzrunde mit zwei Schätzern hat also nachweislich nicht
stattgefunden, denn die findet im Planning statt und wird protokolliert.

Das Label kann beides bedeuten und sieht in beiden Fällen gleich aus. Es hat
dreimal Arbeit gekostet:

- Sprint 5 §2.1: #57 trug `sp:2` aus der Anlage; zwei unabhängige Schätzer hoben
  auf 3. *„Die beiden Schätzer ersetzen das Label, sie übernehmen es nicht."*
- Sprint 6 §2.4: #68 trägt `sp:5` aus **einer** Hand. *„Ein Label wird für eine
  Schätzung gehalten, weil es am Backlog steht."*
- Sprint 6 §2: Von acht Kandidaten waren **zwei** regelkonform geschätzt; das
  Planning musste eine eigene Tabellenspalte „Schätzregel erfüllt?" führen, um
  Label von Schätzung zu trennen.

Dazu zweimal in Folge fehlende Label (M2 in Sprint 5, K1 in Sprint 6), beide
Male durch Nachforderung geheilt statt durch eine Regel.

### A6 · Die Dev-Agentendatei verweist auf einen Ausschnitt von `PROZESS.md`

**Beobachtung.** `.claude/agents/denkzettel-dev.md:19` nennt als verbindliche
Grundlage *„`docs/scrum/PROZESS.md`, **Abschnitt Definition of Done**"*. Zwei
Regeln, die den Dev unmittelbar binden, stehen außerhalb dieses Abschnitts:

- **Parallelarbeit (B13)** — eigener Worktree, eigener Zweig, Rebase statt
  Rückwärts-Merge. `PROZESS.md` hält ausdrücklich fest, dass diese Regel *„nur
  in den Spawn-Aufträgen stand und daran hing, dass der PO sie jedes Mal erneut
  hinschreibt"* — und hebt sie deshalb nach `PROZESS.md`. Der Dev wird auf diese
  Stelle nicht verwiesen.
- **Das Installationsverbot für Stränge** (DoD 2, Präzisierung nach
  Sprint-3-Mangel M1). Es steht im DoD-Abschnitt, aber die Agentendatei
  erwähnt es mit keinem Wort — und `CLAUDE.md` sagt dem Dev im ersten Satz ihres
  Regelteils: *„Geprüft wird am installierten Stand."*

**Folge, messbar in den Protokollen:** Sprint 5 §10.6 und Sprint 6 §10.5
verlangen beide vom PO, *„der `/usr`-Takt gehört als Verbot mit Begründung in
den Spawn-Auftrag, nicht als Hinweis"*. Zwei Sprints in Folge dieselbe
Nachforderung an denselben Ort, an dem sie nicht bleibt.

### A7 · Die billigere Hälfte von DoD 2 ist längst gebaut — ich hätte sie beinahe ein zweites Mal vorgeschlagen

**Beobachtung.** Ein Installationslauf in ein Ablageverzeichnis (`DESTDIR`)
kostet gemessen **unter 1 s**, braucht kein Passwort und kein `/usr`, und
liefert die vollständige Dateiliste (`/usr/bin/denkzetteld`,
`/etc/xdg/autostart/…`, `/usr/share/applications/…`, zwei Symbole). Die
Gegenprobe mit `-DCMAKE_INSTALL_PREFIX=/usr/local` zeigt das Ziel als
`${CMAKE_INSTALL_PREFIX}/etc/xdg/autostart` — **die in `CMakeLists.txt:44–47`
festgehaltene Bedingung gilt heute unverändert** (Sprint 2 §723, DoD 4 in der
Fassung nach B9).

**Genau das tut `installtest` bereits** (`tests/installtest.cmake`,
`tests/CMakeLists.txt:122`): Installation nach `DESTDIR`, Prüfung des
Anwendungs- **und** des Autostart-Eintrags, dazu die Gruppe
`[Desktop Action show-capture]` samt `Exec`-Zeile — der Kundenbefund vom
01.08.2026. Der Test hängt an `ctest`, läuft also seit heute **in jedem
CI-Lauf mit**. Mein Lauf: `installtest` ist Test 7 von 7 und grün.

**Schluss, gegen meinen eigenen ersten Entwurf.** Ich hatte hierfür einen
Vorschlag geschrieben und ihn beim Gegenlesen gestrichen: Er hätte einen
bestehenden Test ein zweites Mal gebaut. Das ist dieselbe Fehlerart, gegen die
`CLAUDE.md` die Prüfhaltung setzt — eine Maßnahme klang plausibel, und die
Plausibilität ersetzte den Blick in den Testsatz. Die Beobachtung bleibt
trotzdem verwertbar: **Die Dateilage ist abgedeckt, der teure `/usr`-Schritt
trägt nur noch den laufenden Dienst und den Hauptweg** — und genau so, nicht
breiter, sollte man ihn im Sprint-Abschluss beschreiben.

**Eine echte, aber unbelegte Restlücke** steht in Teil E, Punkt 9.

---

## Teil B — Vorschläge

Jeder mit Beleg, Kosten, Stop-Bedingung und Ablageort. „Automatisch gelesen"
heißt: eine Sitzung oder ein Werkzeug schlägt die Datei ohne Zutun auf.

### V1 · `EXCLUDE_FROM_ALL` von den Bildläufern nehmen

**Beleg.** `docs/scrum/reviews/sprint-05-s-symbole/bericht.md` §9.3: grüner Test
und falsches Bild zugleich, weil der 22 Minuten alte Läufer gegen die alte
Bibliothek lief. Dazu A3: Die Linter analysieren dieselben Dateien mit
fehlendem Include-Verzeichnis.

**Kosten.** Vier Zeilen in `tests/CMakeLists.txt` (`:58`, `:67`, `:76` und
`:86` — `readmeshots` ist heute als **vierter** Läufer dazugekommen und trägt
dieselbe Falle). Laufend: **7 s** je Vollbau für die ersten drei, 0 s bei
unverändertem Stand (gemessen). Kein `add_test()` — die Läufer bleiben aus
`ctest` heraus, die im Quelltext genannte Begründung bleibt gewahrt.

**Nebenbemerkung zur Dringlichkeit:** Dass innerhalb von zwei Tagen ein
vierter Läufer nach demselben Muster entstanden ist, ohne dass jemand die
Falle nachgezogen hätte, ist der Beleg dafür, dass die Ermahnung in
`CLAUDE.md` das Muster nicht bremst.

**Stop-Bedingung.** Wenn der Bau dadurch spürbar länger dauert als die heute
gemessenen 7 s Zuschlag — dann zurücknehmen. Wirksamkeit erkennt man daran,
dass die Ermahnung in `CLAUDE.md` („Vor jedem Bildbeleg: `cmake --build build
--target <läufer>`") entbehrlich wird; sie kann nach zwei Sprints ohne
Bildvorfall gestrichen werden.

**Ort.** `tests/CMakeLists.txt` — **wird vom Bau garantiert gelesen**. Das ist
der stärkste denkbare Ort: Er ersetzt eine Regel, die ein Mensch befolgen muss,
durch eine, die eine Maschine nicht umgehen kann.

### V2 · Bildbelege am Sprint-Ende auf gleiche Prüfsummen prüfen

**Beleg.** Sprint 5, `sprint-05-installationstakt.md` (Nachtrag V2): Ein Mensch
fand von Hand **einen** Fall; die Prüfsumme findet ihn und **22 weitere** (A4).
`docs/scrum/reviews/sprint-04-s8-ui-review/` trägt allein fünf davon.

**Kosten.** Ein Befehl im Sprint-Abschluss, Takt 1, Laufzeit unter einer
Sekunde bei 257 Bildern. Die Bewertung der Treffer bleibt Kopfarbeit — 23
Gruppen einmalig durchsehen, danach nur noch die neuen je Sprint.

**Abgrenzung, damit die Wache nicht zur Gewohnheit wird.** Nur Duplikate
**innerhalb eines Belegordners** sind ein Befund. Dass dasselbe Bild in der
Dev-Übergabe und im UX-Bericht liegt, ist normal — ordnerübergreifend gäbe es
44 Gruppen, und eine Wache, die immer anschlägt, ist keine.

**Stop-Bedingung.** Findet die Prüfung in drei aufeinanderfolgenden Sprints
keine neue Gruppe, wird sie zur Stichprobe herabgestuft.

**Ort.** `docs/scrum/PROZESS.md`, „Sprint-Abschluss", Takt 1 Punkt 2 (dort steht
bereits die Belegpflicht). Gelesen wird sie automatisch vom Scrum Master, dessen
Agentendatei `PROZESS.md` als *„deine Verfassung"* führt.

### V3 · Beide Linter in die CI — aber erst nach zwei Vorarbeiten

**Beleg.** A1: Eine DoD-Aussage („keine Linterbefunde aus diesem Sprint") ruht
auf einer Messung, die am selben Code heute 72 statt 3 Befunde liefert. A2: Der
Lauf kostet zusammen 52 s — weniger als ein Zehntel dessen, was der
CI-Lauf ohnehin für Paketinstallation und Bau braucht.

**Kosten.** Etwa zehn Zeilen in `.github/workflows/ci.yml` und 52 s Laufzeit je
Lauf. **Dazu zwei Vorarbeiten, die nicht umsonst sind** — und deshalb steht
dieser Vorschlag nicht auf Platz 1:

1. `lint-clazy` gibt heute immer `rc=2` (A2). Solange das so ist, taugt der
   Rückgabewert nicht als Tor. Behebbar mit einer Zeile, die
   `tests/spellfixspike.cpp` aus der Linterliste nimmt, solange der Spike-Schalter
   aus ist — Dev-Arbeit, nicht meine.
2. Die 72 `lint-tidy`-Befunde müssen entweder geheilt oder als **Grundlinie
   eingefroren** werden (Tor: „keine *neuen* Befunde"). Eine Schwelle unterhalb
   des gemessenen Standes wäre kein Tor, sondern ein Dauerrot — dieselbe
   Überlegung, aus der die Warnungsschwelle heute auf null steht, hier mit
   umgekehrtem Vorzeichen.

**Stop-Bedingung.** Findet das Tor in drei Sprints keinen neuen Befund, genügt
ein Lauf je Sprint-Ende statt je Push. Schlägt es dagegen bei jedem Lauf an,
ist die Grundlinie falsch gesetzt und gehört korrigiert, nicht ignoriert.

**Ort.** `.github/workflows/ci.yml` — läuft ohne Zutun. Der Satz „am Sprint-Ende
wird gelintet" in einem Protokoll wäre genau die Bauart, die A1 belegt hat.

### V4 · `denkzettel-dev.md`: auf ganz `PROZESS.md` verweisen, `/usr`-Verbot aufnehmen

**Beleg.** A6: zwei Regeln außerhalb des DoD-Abschnitts, auf den allein
verwiesen wird; die Nachforderung „`/usr`-Takt als Verbot in den Spawn-Auftrag"
steht in Sprint 5 §10.6 **und** Sprint 6 §10.5. `PROZESS.md` hält bei B13
selbst fest, dass eine Regel, die nur im Spawn-Auftrag steht, an der Disziplin
des PO hängt.

**Kosten.** Vier bis fünf Zeilen in `.claude/agents/denkzettel-dev.md`. Laufend
null. Spart je Spawn drei bis vier Zeilen Auftragstext.

**Stop-Bedingung.** Wirksam, wenn zwei aufeinanderfolgende Plannings diese
Punkte **nicht** mehr als Hinweis an den PO führen müssen. Steht die
Nachforderung im dritten Sprint wieder da, wirkt die Datei nicht und die
Ursache liegt anderswo.

**Ort.** `.claude/agents/denkzettel-dev.md` — **wird beim Spawn garantiert
gelesen**. `PROZESS.md` allein genügt nicht: Der Dev bekommt sie nur über diesen
Verweis.

*(Ein fünfter Vorschlag — Prüfung der Dateilage einer Installation in der CI —
stand hier und ist gestrichen: `installtest` leistet das seit Sprint 5 und
läuft in der CI mit. Siehe A7 und Teil E, Punkt 9.)*

---

## Teil C — Rangfolge nach Nutzen je Kosten

| Rang | Vorschlag | Kosten | Nutzen |
|---|---|---|---|
| 1 | **V1** Bildläufer mitbauen | 4 Zeilen, 7 s je Bau | Schließt eine belegte Fehlerart mechanisch; ersetzt eine Ermahnung durch eine Unmöglichkeit |
| 2 | **V2** Prüfsummen der Bildbelege | 1 Befehl, < 1 s | Findet heute 23 Gruppen, darunter den einen, den ein Mensch von Hand fand |
| 3 | **V4** Dev-Agentendatei | 5 Zeilen | Beendet eine Nachforderung, die zweimal in Folge nötig war |
| 4 | **V3** Linter in die CI | 10 Zeilen + 52 s + zwei Vorarbeiten | Der schwerste Befund dieses Berichts (A1) — aber der einzige Vorschlag mit echter Vorarbeit |

**Die drei, die ich zuerst umsetzen würde: V1, V2, V4.** Alle drei kosten
zusammen weniger als eine Stunde, keiner braucht eine Vorarbeit, und jeder
landet an einem Ort, den eine Sitzung oder ein Werkzeug von selbst liest.

**V3 trägt den wichtigsten Befund, nicht das beste Verhältnis.** Wenn der Kunde
nur eine Sache entscheiden will, sollte er wissen: A1 ist der Fund, der die
Prüfhaltung dieses Projekts am unmittelbarsten betrifft.

---

## Teil D — Womit das Team aufhören sollte

**Aufhören, `sp:`-Label bei der Anlage eines Issues zu setzen.**

Das Label ist heute zweideutig: Es trägt einmal eine moderierte Schätzung aus
zwei unabhängigen Händen und einmal eine Zahl, die jemand beim Anlegen
hingeschrieben hat — und beide sehen gleich aus. 24 der 36 Label an offenen
Issues haben nachweislich nie eine Schätzrunde gesehen (A5). Der Backlog ist
die *einzige Quelle der Wahrheit*; ein Feld darin, das zwei Dinge bedeuten kann,
ist an dieser Stelle teurer als anderswo.

Die Kosten des Aufhörens sind null — es ist eine Handlung, die unterbleibt. Der
Verlust ist eine grobe Hausnummer für ungezogene Stories; den kann ein
Freitextsatz im Issue tragen, der nicht wie eine Schätzung aussieht.

**Ort.** `CLAUDE.md` (der PO legt die Issues an und liest sie automatisch) und
die Label-Zeile in `docs/scrum/PROZESS.md`, Abschnitt Artefakte. Der Satz, der
fehlt, lautet sinngemäß: *Das `sp:`-Label wird im selben Zug gesetzt wie die
zweite unabhängige Schätzung — vorher gar nicht.* Damit wäre auch der zweimal
gerissene Punkt (M2 in Sprint 5, K1 in Sprint 6) an einem Ort verankert statt
zum dritten Mal nachgefordert.

---

## Teil E — Geprüft und für unnötig befunden

Damit „nicht vorgeschlagen" von „nicht angesehen" zu unterscheiden ist.

1. **Testabdeckung messen (gcov/lcov).** Verworfen. Die vier entlarvten Tests,
   die nichts prüften (`CLAUDE.md`, Prüfhaltung), waren **tautologisch**, nicht
   **fehlend** — eine Abdeckungsmessung hätte sie als abgedeckt gezählt und die
   falsche Sicherheit verstärkt. Kein belegter Vorfall, den sie gefunden hätte.
2. **Bildvergleich gegen Referenzbilder (Pixel-Diff).** Verworfen für jetzt. Der
   Sprint-5-Vorfall war ein **veralteter Läufer**, kein Pixeldrift; V1 schließt
   ihn mechanisch. Ein Pixel-Diff bräuchte gepflegte Referenzen — 257 Bilder
   liegen im Repo, keines ist als Referenz deklariert, und die Pflege wäre
   teurer als die Wache. V2 holt aus demselben Bestand den Nutzen ohne die Pflege.
3. **Commit-Hooks gegen `git add -A` und `--amend`.** Verworfen. Die Regel steht
   in der Dev-Agentendatei und hat gehalten: Sprint 3 mit vier Strängen und 33
   Commits ohne Vorfall (`PROZESS.md`, B13). In 179 Commits kein belegter
   Verstoß. Eine Wache ohne Fund.
4. **CI auch auf Story-Zweigen.** Verworfen. Der Dev darf nicht pushen
   (Agentendatei), und Story-Zweige erreichen `origin` im Normalfall nicht — auf
   `origin` steht heute nur `main`. Ein Auslöser auf `**` liefe leer.
5. **PR-Verfahren wiederaufnehmen.** Nicht vorgeschlagen, und die CI ändert das
   Argument nicht zugunsten der PRs, sondern dagegen: Sie läuft auch beim Push
   auf `main`. Damit liefert ein PR genau die Hälfte des Abbruchkriteriums, die
   ohnehin schon erfüllt ist.
6. **Velocity, Burndown, Story-Point-Kennzahlen.** Verworfen. Dieselbe Lücke wie
   beim Schätzkegel: Ohne erhobenen Ist-Aufwand misst man die eigene Schätzung.
   `PROZESS.md` verlangt für dieses Werkzeug ausdrücklich zuerst die gemessene
   Aufwandszahl je Story — sie liegt nicht vor.
7. **Issue-Vorlagen mit Schätzfeld.** Verworfen zugunsten von Teil D: Eine
   Vorlage, die zum Ausfüllen einlädt, verschärft das Problem, statt es zu lösen.
8. **Ordnergröße der Belege (87 MB, 257 Bilder).** Angesehen, nichts
   vorgeschlagen. B7 steht dagegen, und kein Vorfall hängt an der Größe.
9. **Dateilage der Installation in der CI prüfen.** Vorgeschlagen und wieder
   gestrichen: `installtest` tut es seit Sprint 5 und läuft in der CI mit (A7).
   **Eine Restlücke bleibt und reicht für keinen Vorschlag:** `installtest`
   prüft die Selbstkonsistenz (*liegt die Datei dort, wo CMake sie hinschreibt?*),
   nicht das Präfix. Ein Bau auf `/usr/local` bliebe grün und legte den
   Autostart-Eintrag dorthin, wo keine Sitzung liest. Der Vorgabepräfix des
   Projekts ist heute gemessen bereits `/usr` (ohne dass jemand ihn setzt), und
   ein Vorfall dazu existiert nicht — deshalb Beobachtung, nicht Maßnahme.

---

## Teil F — Vermutungen (kein Beleg, deshalb getrennt)

Drei Gedanken, die ich für plausibel halte und die **an keinem Vorfall hängen**.
Sie gehören nicht in eine Entscheidung, sondern in eine Retro, falls sie einmal
einen Beleg bekommen.

1. Ein Abschluss-Tag `sprint-NN-ende` als Gegenstück zum Basis-Tag. Plausibel
   für spätere Vergleiche — bisher hat ihn niemand vermisst.
2. Ein maschineller Zähler für Prüfläufe gegen die in `PROZESS.md` benannte
   Grenze (*„Ein Lauf, der weder Bericht noch Commit hinterlässt, bleibt
   unsichtbar"*). Seit Sprint 3 kein Fall.
3. Ein `NOLINT` mit Begründung an `src/shell/globalshortcuts.cpp:93`, wo
   `.clang-tidy` den Rückgabewert einfordert und der Code ihn kommentiert
   verwirft. Ob das ein Widerspruch oder eine saubere Ausnahme ist, entscheidet
   nicht der Scrum Master.

---

## Teil G — Nebenbefunde zum Melden (melden, nicht heilen)

Außerhalb meiner Fläche gefunden, an den PO gemeldet, von mir nicht angefasst:

- **N1** `lint-clazy` endet in der Vorgabekonfiguration immer mit `rc=2`
  (`tests/spellfixspike.cpp` ohne den Spike-Schalter, A2). Betrifft V3.
- **N2** Beide Linter analysieren `editshots`, `libraryshots` und `searchshots`
  mit fehlendem Autogen-Include-Verzeichnis (A3). V1 heilt das mit. Der heute
  hinzugekommene vierte Läufer `readmeshots` (`tests/CMakeLists.txt:86`) trägt
  dieselbe Bauart.
- **N3** `bugprone-unused-return-value` schlägt an
  `src/shell/globalshortcuts.cpp:93` an; die `.clang-tidy`-Liste fordert dort den
  Rückgabewert ein, der Code verwirft ihn mit ausführlicher Begründung. Beides
  ist bewusst so entstanden und widerspricht sich (siehe F3).
- **N4** In `docs/scrum/reviews/2026-08-01-capture-theme/` stehen 23
  `achse3-huelle-*`-Dateinamen für 6 Aufnahmen (A4). Die Aussage des Berichts
  („die Hüllen unterscheiden sich sichtbar") wird davon nicht widerlegt — die
  4-px- und 8-px-Gruppen sind wirklich verschieden —, aber die Zahl der Dateien
  überzeichnet die Zahl der Messungen.

---

## done / next

**done:** Sieben Flächen gemessen (Linter, Bildläufer, DoD-2-Installation,
Schätzung, Agentenaufträge, Belegablage, Prüfaufwand); **vier** Vorschläge mit
Beleg, Kosten, Stop-Bedingung und Ablageort; ein Aufhör-Vorschlag; **neun**
geprüfte und verworfene Maßnahmen — darunter ein eigener, beim Gegenlesen
gestrichener Vorschlag (A7); drei Vermutungen getrennt gestellt; vier
Nebenbefunde gemeldet.

**next:** Kundenentscheidung. Nichts wird umgesetzt, bevor sie vorliegt — der
Auftrag war eine Ermittlung.
