# Retro Sprint 3 — Technische Stellungnahme des Entwicklers

**Verfasser:** Agent `denkzettel-dev` · **Datum:** 2026-08-02 · **Stand:** `e73efff`
Input zu den vier Kundenthemen (sprint-03.md, Abschnitt 14). Beobachtungen sind
gemessen; jede trägt den Befehl, der sie zeigt.

## 1. PR-Workflow bei paralleler Agenten-Arbeit

**Beobachtung.** `gh pr list --state all --json number` → `[]`; `gh release list`
→ leer; `git tag -l | wc -l` → `0`; `ls .github` → nicht vorhanden, also keine CI.
`git log --merges --oneline | wc -l` → `9`, davon 8 mit Issue-Bezug; einer ist ein
Rückwärts-Merge (`c3e4daf Merge branch 'main' into story/8-volltextsuche`).
`git rev-list --left-right --count origin/main...main` → `0 1`: der aktuelle Stand
ist **nicht gepusht**. `gh repo view --json visibility` → `PUBLIC`.

**Wo ein PR trägt.** Er ist der fehlende *Anker*. DoD 3 lässt karpathy- und
UI-Review über „den Sprint-Diff" laufen — einen Bereich, den der PO jedes Mal von
Hand konstruiert. Ein PR ist dieser Bereich, benannt und stabil, mit
zeilengebundenen Befunden, die den Agenten überleben; heute liegen die Befunde als
Prosa in `docs/scrum/reviews/` und hängen an keiner Diff-Zeile. Er ist zugleich die
Voraussetzung dafür, dass `ctest` je von selbst liefe — dafür fehlt heute der Ort.

**Wo er nur kostet.** Eine Maschine, ein Prüfer von Amts wegen: ein PR, der
geöffnet und drei Minuten später selbst gemerged wird, ist Zeremonie. Dazu ein
Preis, den private Repos nicht kennen — **Branch-Push ist Veröffentlichung.** Heute
ist nur das kuratierte `main` öffentlich; mit PR-Pflicht wäre jeder Zwischenstand
publiziert, `c3e4daf` eingeschlossen.

**Empfehlung: ein PR je Story-Strang** — genau die vier Zweige aus Sprint 3 —,
geöffnet bei der Übergabe, mit Dev-Bericht sowie karpathy- und UI-Befund als
Kommentare; der PO merged. Prozess-, Doku- und SPEC-Commits bleiben direkt auf
`main`: sie haben keinen Review-Anker und würden das Verfahren nur füllen.

**Merge-Strategie: bei `--no-ff`-Merge-Commits bleiben.** Gegen Squash spricht
Messbares — `git log --format='%H%x09%b' | awk -F'\t' 'length($2)>0' | wc -l` →
`90` von `91` Commits haben einen Body, und `git log -1 19ca42b` trägt drei
gemessene Festlegungen (Präfix-Stern gegenstandslos, Ausweichweg für Kurzbegriffe,
Indexgröße 1,8 → 10,9 MiB bei 20 000 Notizen). Squash presst das in eine Zeile.
Rückwärts-Merges künftig vermeiden: ein Strang, der `main` braucht, rebased.

**Die acht liegengebliebenen Zweige.** Einzeln geprüft mit `git merge-base
--is-ancestor <branch> main` (Exit 0 = vollständig in `main`);
`git branch --no-merged main` ist leer. **Alle acht sind gefahrlos löschbar**:
`story/46-posteingang` (`7787339`), `story/8-volltextsuche` (`14bf65d`),
`fix/44-tray-linksklick` (`f8187b4`), `fix/54-theme-farben` (`ea2337a`) und die
vier `worktree-agent-*`-Zweige (`…a1c081dc52c4b8225`, `…a6c5cf68b79ab9593`,
`…a814d77d7b5ce3a29`, `…ae733f11b4f6ec5b9`), die alle auf `59d0d3f` aus Sprint 2
zeigen. **Ich lösche nichts** (melden, nicht heilen). Zwei Hinweise: Die
`worktree-agent-*`-Zweige sind Nebenprodukte der Worktree-Erzeugung, auf denen nie
gearbeitet wurde — das Muster wiederholt sich bei jedem parallelen Sprint. Und
`git branch -d` scheitert an den Story-Zweigen, solange die Worktrees bestehen
(`git worktree list`: alle vier ausgecheckt); erst `git worktree remove`, dann
`git branch -d`. Sie sind sauber (`git -C <wt> status --porcelain` je leer) und
belegen `du -sh .claude/worktrees` → **296 MB**, überwiegend `build/`-Bäume.

## 2. Versionierung

**Beobachtung.** `CMakeLists.txt:3` → `project(denkzettel VERSION 0.1.0 …)`.
`grep -rn "PROJECT_VERSION\|KAboutData\|setApplicationVersion" src desktop
CMakeLists.txt` → **kein Treffer** außerhalb von `build/`-Caches; `src/main.cpp:21`
ff. setzt Organisation, Anwendungsname und Desktop-Datei, aber keine Version.
`grep -in version SPEC.md` → nur Schemaversionen: die SPEC kennt weder eine
Programmversion noch einen „Über"-Dialog.

**Schluss.** Die Version ist **inert** — sie steht im Bauskript und erreicht die
Anwendung nirgends. Deshalb fiel in drei Sprints nicht auf, dass sie stillsteht:
eine Erhöhung hätte nichts verändert, was jemand hätte sehen können. Das ist die
Lücke, nicht die Zahl. Erster echter Bedarfsträger ist #41, dessen AK „makepkg baut
auf Ganymed" ein `pkgver` verlangt, das sich bewegt.

**Empfehlung — eine Quelle: `CMakeLists.txt:3`.** Der Tag `vX.Y.Z` ist das Siegel,
nicht die Quelle; das PKGBUILD spiegelt die Zahl. Gegen die Umkehrung (CMake leitet
per `git describe` ab) spricht, dass ein Quell-Tarball ohne `.git` dann keine
Version hätte und ein Fallback-Pfad nötig würde — Maschinerie für ein Projekt mit
null Releases (Karpathy 2). **Beobachtbar** wird die Zahl mit wenigen Zeilen: als
Compile-Definition an `app.setApplicationVersion()` plus `--version`-Option. Ein
**`KAboutData`-„Über"-Dialog steht nicht in der SPEC** — er wäre eine neue
Festlegung und braucht eine eigene Story mit SPEC-Eintrag (DoD 4); ich schlage ihn
vor, ich erfinde ihn nicht.

**Schema: 0.x-SemVer, MINOR je abgenommenem Sprint** (`0.4.0` mit der Abnahme von
Sprint 4), PATCH für außerplanmäßige Behebungen, `1.0.0`, wenn der Kunde erklärt,
dass das Werkzeug seinen Alltag trägt. **Auslöser ist die Kundenabnahme, nicht das
Sprint-Ende** — der Sprintschluss ist ein Ereignis des Teams, ein Release eines für
den Kunden. Gegen CalVer spricht ein Fall aus diesem Sprint: die Migration auf
Schemaversion 2 (`src/store/store.cpp:45`). Wer echten Bestand hat, muss an der
Zahl sehen, dass ein Stand seine Datenbank anfasst; ein Datum sagt das nicht. Daher
zusätzlich: **jede Schemamigration erzwingt mindestens einen MINOR-Sprung.**

## 3. Changelog

**Beobachtung — Quellenqualität, gemessen.**

| Größe | Befehl | Wert |
|---|---|---|
| Commits gesamt / ohne Merges | `git log --oneline \| wc -l` | 91 / 82 |
| Commits an `src`/`tests` | `git log --no-merges --oneline -- src tests` | 39 |
| davon mit `#NN` | dito + `grep -cE '#[0-9]+'` | **36 (92 %)** |
| Produktcommits ohne `#NN` | dito + `grep -vE` | 3 (`fa240b4`, `54b0105`, `9eece07`) |
| Conventional-Commit-Präfixe | `grep -cE '^(feat\|fix\|docs\|chore)…'` | **0** |

**Schluss.** Mit 92 % Issue-Deckung *ist* ein Changelog erzeugbar — **nur nicht aus
dem Commit-Log**: ohne Conventional-Präfixe kann kein Generator (git-cliff,
release-please) Added/Fixed/Changed ableiten, und die Betreffzeilen stehen im
Entwicklerregister („Suche: Wortteile in der Mitte finden, Tokenizer auf trigram")
— das ist keine Release-Note. Präfixe nachrüsten hieße 82 Commits umschreiben.

**Die bessere Quelle haben wir schon.** Die Labels `typ:story|bug|tech` sind exakt
die Keep-a-Changelog-Kategorien, die Milestones gruppieren sprintweise, und die
Issue-Titel stehen dem Nutzerregister näher als jede Betreffzeile. Probelauf
ausgeführt: `gh issue list --state closed --milestone "Sprint 3" --json
number,title,labels` liefert die fünf Sprint-3-Issues samt Labels — **ein Befehl**,
und die Pflege dafür läuft ohnehin.

**Empfehlung — eine Quelle, zwei Ausgaben.** `CHANGELOG.md` nach *Keep a
Changelog*, fortgeschrieben zur Kundenabnahme; die Release-Notes entstehen beim
Taggen aus dem betreffenden Abschnitt (`gh release create … --notes-file`). Die
Umkehrung scheidet aus: wer klont oder ein Paket baut, sieht die Release-Seite nie.
**Weil das Repo öffentlich ist**, drei Auflagen: kein Innenregister (keine
Story-Kürzel „S6", kein Sprint- und Agentenvokabular, keine Kundenzitate);
Issue-Nummern als Verweis sind nützlich; und die Schemamigration gehört
ausdrücklich hinein — heute existiert der Hinweis, dass ein Upgrade die Datenbank
umschreibt, nur in einem Test (#9).

## 4. Verwaltungsaufgaben für kleine Modelle

**Trennlinie: ausführen ja, entscheiden nein — und nur, wo das Ergebnis per Befehl
prüfbar und die Handlung umkehrbar ist.**

**Mechanisch genug (Haiku, notfalls Sonnet):** Zweige nach Merge aufräumen (das
Kriterium ist ein Exit-Code, `git merge-base --is-ancestor`, und der Commit liegt
in `main` — umkehrbar); Changelog-Zeilen als **Entwurf** aus den geschlossenen
Issues eines Milestones ziehen; reine Abgleichsberichte (trägt jedes geschlossene
Issue einen Commit-Verweis? existiert jedes `#NN` als Issue? stimmt der Tag mit
`CMakeLists.txt:3`? sind alle Issues eines geschlossenen Milestones geschlossen?);
Issues schließen — aber nur als **Vollzug** einer bereits protokollierten
PO-Abnahme, mit Zitat des Commits.

**Gehört nicht an ein kleines Modell.** *AK-Haken setzen* ist die
Abnahmeentscheidung selbst: Wer einen Bericht liest und daraufhin „makepkg baut auf
Ganymed" abhakt, macht aus einer Behauptung einen Nachweis — der Fehlermodus, an
dem dieses Projekt schon vier grüne Tests verlor, die nichts prüften (CLAUDE.md,
Prüfhaltung); der Haken ist zudem nicht umkehrbar, er wird zur Beweislage. Ebenso
wenig: *entscheiden, ob* geschlossen wird, und jede Behebung gefundener Mängel —
das verletzt „melden, nicht heilen" unabhängig von der Modellgröße. Der
*Versionssprung* setzt voraus zu wissen, ob eine Schemamigration enthalten ist —
Urteil. Und die *Changelog-Formulierung*: der Entwurf ist mechanisch, der Text
nicht. Er ist das, was die Öffentlichkeit liest; kleine Modelle erzeugen hier
zuverlässig plausibel klingende Fehlgriffe — die teuerste Sorte, weil sie nicht
auffällt.

**Zuschnitt.** Ein einziger neuer Agent (`denkzettel-verwalter`, Haiku), Werkzeuge
auf `Read` und `Bash(git …|gh …)` beschränkt, **ohne Schreibrecht auf `SPEC.md`,
`src/` und Issue-Rümpfe**. Standardausgabe ist ein Bericht; die umkehrbaren
Handlungen führt er nur auf ausdrücklichen Auftrag des PO aus. Kein zweiter Agent
für den Changelog-Entwurf — dieselbe Rolle, dieselbe Schranke (Karpathy 2).

## Fazit — Empfehlungsreihenfolge

Zuerst die Versionierung sichtbar machen: `CMakeLists.txt:3` als einzige Quelle,
`--version` in der Anwendung, MINOR je Kundenabnahme, Schemamigration erzwingt
einen Sprung — der einzige der vier Punkte, den #41 hart blockiert, und er kostet
wenige Zeilen. Zweitens der Changelog aus den geschlossenen Issues je Milestone:
diese Quelle ist gemessen tragfähig (92 % Issue-Deckung, Labels als Kategorien),
das Commit-Log dagegen nicht (0 Conventional-Präfixe) — eine Datei im Repo,
Release-Notes daraus erzeugt, nie umgekehrt. Drittens der PR-Workflow, begrenzt auf
einen PR je Story-Strang als Anker für karpathy- und UI-Review; sofort und
unabhängig davon können die acht restlos gemergten Zweige samt 296 MB Worktrees
weg. Viertens der Verwaltungsagent auf Haiku — der kleinste Hebel, und er wird erst
nützlich, wenn die ersten drei Punkte ihm etwas Mechanisches zu tun geben.
