# Denkzettel — Scrum-Arbeitsvereinbarung

Stand: 2026-08-02. Diese Vereinbarung regelt, wie das Agenten-Team an
Denkzettel arbeitet. Änderungen an ihr entstehen in Retrospektiven.

## Rollen

| Rolle | Besetzung | Verantwortung |
|---|---|---|
| **Kunde** | hnsstrk | Ziele, Prioritätswünsche, Sprint-Freigaben, Abnahme des Produkts |
| **Product Owner** | Claude (Haupt-Session) | Backlog-Inhalt und -Priorisierung, Story-Schnitt, Akzeptanzkriterien, Abnahme der Stories, Kundenkontakt |
| **Scrum Master** | Agent `scrum-master` | Prozesshüter: moderiert Schätzung, schlägt Sprint-Schnitt vor, wacht über Definition of Done, moderiert Retrospektiven, pflegt Prozess-Doku, meldet Impediments |
| **Entwickler** | Agenten `denkzettel-dev` (je Sprint gespawnt) | Umsetzung der Stories nach Spec, Tests, technische Entscheidungen im Story-Rahmen |
| **UI/UX** | Agent `denkzettel-ux` | Planning-Beratung zu UI-Stories, Gestaltung von Wireframes und Mockups (`wireframes/`), UI-Review gegen Wireframes, SPEC und KDE HIG (Teil der DoD für UI-Stories) |
| **QA / Review** | Agent `karpathy-reviewer` | Prüfung gegen die vier Karpathy-Prinzipien am Sprint-Ende und bei Prozess-Artefakt-Änderungen (Teil der DoD) |

Der Scrum Master arbeitet mit dem karpathy-reviewer zusammen: Er definiert,
wann ein Review fällig ist (DoD, Retro-Ergebnisse), formuliert den
Review-Auftrag und nimmt die Befunde in die Sprint-/Retro-Protokolle auf.
Für UI-Reviews durch `denkzettel-ux` gilt dasselbe Muster: Der Scrum Master
formuliert den Auftrag und protokolliert die Befunde; den Aufruf führt der
PO aus.

### Modellzuordnung (Kundenentscheidung 31.07.2026)

- `scrum-master`, `denkzettel-dev` und `denkzettel-ux` laufen auf **Opus 5**
  (`model: opus` im Agent-Frontmatter) — für klar gescopte Stories,
  Prozessarbeit und Facharbeit gegen dokumentierte Referenzen ausreichend,
  bei halben Kosten gegenüber Fable.
- **Ausnahme**: Spikes und Risiko-Stories stuft der PO je Spawn per
  `model`-Parameter auf Fable hoch.
- `karpathy-reviewer` bleibt auf **Fable** (Session-Modell) — das
  Sicherheitsnetz wird nicht abgestuft.
- **Revision in der Sprint-3-Retro (02.08.2026): bestätigt** (Belege im
  Sprint-3-Protokoll, 16.4). Der eine Fund des Sprints, den eine Opus-Rolle
  geschrieben und keine andere Opus-Rolle gefunden hat — die tautologische
  Zusicherung in `librarytest.cpp`, eingeführt mit `4746358`, entfernt erst
  mit `7787339` —, kam vom `karpathy-reviewer` auf Fable. Das ist der Beleg
  dafür, das Sicherheitsnetz nicht abzustufen. Nächste Revision: Retro nach
  Sprint 6.

## Artefakte und Werkzeuge

- **Product Backlog**: GitHub Issues im Repo `hnsstrk/denkzettel` — die
  **einzige Quelle der Wahrheit** für Stories, Akzeptanzkriterien, Schätzung
  und Status. Kein Backlog-Spiegel im Repo (Kundenentscheidung 31.07.2026).
  Labels: `epic:M1`…`epic:M7`, `sp:1|2|3|5|8`, `typ:story|bug|tech`.
- **Priorisierung**: Epic-Reihenfolge M1→M7 als Grundlinie; die
  Feinreihenfolge legt der PO beim Sprint-Planning über die
  Milestone-Zuordnung fest.
- **Sprint**: GitHub Milestone `Sprint N` mit den gezogenen Issues.
- **Protokolle**: `docs/scrum/sprints/sprint-NN.md` (Planning, Review,
  ggf. Retro) — je Sprint eine Datei, angelegt beim Planning.
- **Belege**: UI-Review-Berichte samt geprüften Bildern unter
  `docs/scrum/reviews/`, Retro-Stellungnahmen und Messbelege unter
  `docs/scrum/retro/sprint-NN/`. Sitzungs-Scratchpads sind flüchtig; was ein
  Protokoll behauptet, liegt im Repo (Retro Sprint 2, B7).
  **Flüchtige Belege werden beim Eintreffen gesichert, nicht am Ende des
  Arbeitsschritts** (Retro Sprint 3, B14): Kundenbilder liegen in temporären
  Ordnern, die weggeräumt werden, während man noch schreibt — von acht
  Bildern der Sprint-3-Abnahme überlebte sieben Minuten nur eines
  (Sprint 3, 15.3).
- **Fachliche Quellen**: `SPEC.md` (bindend), `KONZEPT.md` (Historie der
  Entscheidungen), `wireframes/` (UI-Referenz; Spiegel im
  Claude-Design-Projekt „Denkzettel" auf claude.ai/design, Sync durch den
  PO über das DesignSync-Werkzeug seiner Claude-Code-Session —
  Kundenentscheidung 31.07.2026).

## Sprint-Mechanik

- **Sprint-Umfang**: 2–4 Stories, zusammen max. ~13 Story Points, mit einem
  benennbaren Sprint-Ziel. Sprints sind Arbeitspakete, keine Zeiträume.
- **Freigabemodell (Kundenentscheidung 31.07.2026)**: Jeder Sprint startet
  erst nach Freigabe durch den Kunden (Sprint-Ziel + gezogene Stories werden
  vorgelegt).
- **Ablauf**: Planning (SM schlägt Schnitt vor; bei UI-Stories berät
  `denkzettel-ux`; PO bestätigt, Kunde gibt frei) → Umsetzung (Dev-Agenten;
  bei paralleler Arbeit Worktree-Isolation, siehe „Parallelarbeit") → Review
  (PO nimmt gegen Akzeptanzkriterien ab; karpathy-reviewer und bei UI-Stories
  `denkzettel-ux` gemäß DoD) → Protokoll → ggf. Retro.
- **Schätzung**: Story Points, Fibonacci (1, 2, 3, 5, 8, 13). Zwei
  unabhängige Schätzer je Story; weichen sie um mehr als eine Stufe ab,
  konsolidiert der Scrum Master mit Begründung. 13er-Stories werden vor dem
  Ziehen geteilt.
- **Sprint-Konto (Retro Sprint 3, B12)**: Das Sprint-Protokoll führt **beide**
  Grenzen laufend mit — Zahl der Issues *und* Story Points, je mit Ausgangs-
  und neuem Stand. Jeder Zugang nach der Freigabe wird dort gebucht; berührt
  er eine der beiden Grenzen, legt der PO ihn dem Kunden **als
  Grenzüberschreitung** vor, nicht nur als Story. *Grund:* In Sprint 3 wurde
  bei jedem Zugang die Punktzahl mitgezählt, die Zahl der Issues nicht — der
  Sprint endete bei fünf Issues, und dass damit eine Grenze fiel, ist dem
  Kunden nie vorgelegt worden (Sprint 3, 12.7 und 13.10). Wer 13 im Blick
  hat, sieht die 13 einhalten und übersieht die 5.
- **Parallelarbeit (Retro Sprint 3, B13)**: je Strang ein eigener Worktree und
  ein eigener Zweig (`story/NN-…`, `fix/NN-…`); gemeinsamer Ausgangsstand ist
  gepushtes `main`; gemerged wird ausschließlich durch den PO; ein Strang, der
  `main` braucht, rebased, statt rückwärts zu mergen. Die Installation nach
  `/usr` ist ein exklusiver Abschnitt, den der PO taktet (DoD 2). *Grund:* In
  Sprint 3 hat dieses Verfahren getragen — vier Stränge, 33 Commits, acht
  Merges, kein Vorfall (13.9) —, stand aber nur in den Spawn-Aufträgen und
  hing daran, dass der PO es jedes Mal erneut hinschreibt. Genau diese Bauart
  war der Anlass für B6.

## Definition of Done (je Story)

1. Code kompiliert warnungsarm; neue Logik hat Unit-/Integrationstests, alle
   Tests grün (auf Ganymed). Jede Aussage des Wireframes über die
   Raumaufteilung einer Ansicht ist als Geometrie-Zusicherung im Test
   festgehalten, geprüft bei zwei Fenstergrößen (offscreen genügt).
2. Akzeptanzkriterien des Issues erfüllt und vom PO abgenommen. Vor der
   Übergabe hat der Entwickler den gebauten Stand selbst gestartet, den
   Hauptweg der Story einmal ausgeführt und den Nachweis in den Bericht
   gelegt (Terminalausgabe, Journalauszug oder Bild). Geprüft wird der
   installierte Stand (`-DCMAKE_INSTALL_PREFIX=/usr`), nicht das
   Build-Verzeichnis. Eine im Bericht benannte Grenze der Prüfbarkeit
   schließt die Story nicht — sie wird geschlossen oder als Impediment
   eskaliert.
   **Bei mehreren gleichzeitig arbeitenden Agenten** (Präzisierung nach
   Sprint-3-Mangel M1): Es gibt nur ein `/usr`. Installieren zwei annähernd
   gleichzeitig, prüft einer den Stand des anderen — deshalb untersagt der PO
   den Strängen die eigenmächtige Installation und taktet sie. **Dann trägt er
   die Pflicht: Am Sprint-Ende wird der Endstand einmal installiert und der
   Hauptweg jeder Story daran ausgeführt.** Wer die Regel für die Stränge
   aussetzt, ohne diesen Ersatz zu schaffen, hebt DoD 2 auf, statt sie zu
   schützen — genau das ist in Sprint 3 geschehen.
3. karpathy-reviewer-Durchgang ohne offene `fail`-Befunde
   (Sprint-Ende-Review über den Sprint-Diff genügt, Einzel-Review bei
   riskanten Stories). UI-Stories zusätzlich: UI-Review durch
   `denkzettel-ux` ohne offene `fail`-Befunde — welche Stories UI-Stories
   sind, legt der PO beim Planning fest (Kundenentscheidung 31.07.2026).
   **Der UI-Review ist ohne Bild nicht geführt**: Der Entwickler legt je
   UI-Story einen Screenshot pro Wireframe-Zustand bei (Normalfall,
   Leerzustand, Meldungszustand), `denkzettel-ux` erzeugt zusätzlich eigene
   Bilder aus dem Sprint-Stand und prüft sie gegen den Wireframe. Ein
   UI-Review ohne eigene Bildprüfung zählt für diesen Punkt nicht.
4. SPEC.md/KONZEPT.md nachgezogen, falls die Umsetzung eine Festlegung
   ändert **oder eine Bedingung entdeckt, ohne die eine Festlegung nicht
   gilt** (Bauart: fehlende Build-Abhängigkeit, Installations-Präfix —
   Retro Sprint 2, B9); mit Begründung im Issue.
5. Commit(s) auf `main` bzw. Feature-Branch gemäß Kundenvorgabe; Issue
   geschlossen mit Verweis auf Commit.
6. Journal-Eintrag der Session gemäß globaler Protokollpflicht.

Zur Sprint-Ende-Prüfung des Scrum Masters gehört der **Doku-Abgleich**:
Beschreiben README und `docs/` den gelieferten Stand? Abweichungen meldet er
als Mangel; die Korrektur ist Sache von PO oder Dev (melden, nicht heilen).
Das Ergebnis steht in der DoD-Prüfung des Sprint-Protokolls, auch bei
Befundfreiheit — sonst ist „geprüft, nichts gefunden" nicht von „vergessen"
zu unterscheiden.

**Prüfzeitpunkte: siehe „Sprint-Abschluss".** DoD 1–4 und der Doku-Abgleich
gehören in Takt 1 (vor der Kundenabnahme), DoD 5 und DoD 6 in Takt 2 (nach
ihr) — vor der Abnahme sind sie nicht erfüllbar. Wer alle sechs Punkte in einem
Zug prüft, bucht sie erneut als Mängel (Sprint-3-Mängel M2 und M5).

## Sprint-Abschluss (Retro Sprint 3, B11)

Acht der neun Mängel aus Sprint 3 waren Abschlussmängel (Sprint 3, 14.5).
Diese Liste ist die Antwort darauf. Der Scrum Master übernimmt sie am
Sprint-Ende in das Sprint-Protokoll und hakt jeden Punkt **mit Beleg** ab:
Eine Regel, die in keiner laufenden Liste steht, ist keine Regel
(Sprint 3, 12.7).

**Takt 1 — vor der Kundenabnahme.** Ausführung PO/Dev, Prüfung Scrum Master:

1. Der **Endstand ist einmal nach `/usr` installiert**, und der Hauptweg jeder
   Story ist daran ausgeführt (DoD 2). Hauptweg ist der Weg, den das
   Akzeptanzkriterium beschreibt; der Nachweis hat dieselbe Form wie in DoD 2 —
   **Terminalausgabe, Journalauszug oder Bild**. Ohne Belegform ist „mit Beleg
   abgehakt" eine Behauptung.
2. **Jeder Prüflauf hat einen Bericht als Datei** unter
   `docs/scrum/reviews/` — UI-Review *und* karpathy-Review, Zwischenläufe
   eingeschlossen —, und er liegt vor, **bevor die DoD-Prüfung läuft**. Sonst
   prüft der Scrum Master DoD 3 gegen eine Meldung statt gegen ein Artefakt;
   genau dagegen ist B7 gefasst. Kürzungen werden als solche gekennzeichnet.
   *Prüfweg für die Vollzähligkeit:* Commit-Botschaften, die Befunde eines
   Prüflaufs nennen, gegen die abgelegten Berichte halten — so ist der erste
   karpathy-Lauf des Sprints 3 aufgefallen (`e18630c` nannte drei Befunde, ein
   Bericht dazu lag nicht im Repo; Sprint 3, 16.1.2). **Grenze des Prüfwegs:** Ein Lauf, der weder Bericht noch Commit
   hinterlässt, bleibt unsichtbar — dagegen hilft nur, dass der PO den Aufruf
   beauftragt und mitzählt.
3. **DoD 1–4 je Story** geprüft, **Doku-Abgleich** nach B10 einschließlich der
   Statuszeile des README. Sie beschreibt den **gelieferten Stand**, nicht den
   Stand des Verfahrens: „Sprint N in der Kundenabnahme" wird durch Takt 2
   falsch und gehört gar nicht erst hinein.
4. **Mängelliste an den PO** — melden, nicht heilen.

**Takt 2 — nach der Kundenabnahme.** Ausführung PO:

5. Issues mit AK-Haken, Abnahmekommentar und Commit-Verweis geschlossen,
   Milestone geschlossen (DoD 5).
6. Journal bis zum letzten Commit nachgeführt (DoD 6).
7. `main` gepusht.
8. Story-Zweige und Worktrees entfernt. Das Kriterium ist ein Exit-Code, kein
   Ermessen: `git merge-base --is-ancestor <zweig> main` → 0 heißt löschbar.
   Erst `git worktree remove`, dann `git branch -d`.
9. Der Scrum Master vermerkt den **Vollzug von Takt 2** im Sprint-Protokoll.

Zwei Takte, weil DoD 5 und DoD 6 vor der Abnahme gar nicht erfüllbar sind: In
Sprint 3 wurden sie zum Prüfzeitpunkt trotzdem als Mängel geführt (M2, M5),
und der Doku-Abgleich lief vor der Abnahme — deshalb ging er an der
Statuszeile vorbei, die erst durch die Abnahme falsch wurde.

Version, Tag und Changelog stehen bewusst **nicht** in dieser Liste. Sie sind
Kundenentscheidung (Sprint 3, 16.9) und werden erst nach ihr aufgenommen.

## Retrospektiven

- **Kadenz**: nach Sprint 3 die erste Retro, danach jede dritte
  (Sprint 6, 9, 12, …). Der Kunde kann jederzeit eine Retro anordnen; seine
  Anweisung überstimmt die Kadenz (erstmals am 01.08.2026 nach Sprint 2).
- **Moderation**: Scrum Master; Input: Sprint-Protokolle, Impediment-Liste,
  Befunde des karpathy-reviewers, Kunden-Feedback.
- **Ergebnisse sind Änderungen, keine Absichtserklärungen**: neue oder
  geänderte Skills, Regeln (`~/.claude/rules/` bzw. `.claude/`), Agenten
  (`.claude/agents/`), Memory-Einträge oder Prozess-Anpassungen an dieser
  Datei. Jede Änderung wird im Retro-Protokoll
  (`docs/scrum/sprints/sprint-NN.md`, Abschnitt Retro) mit Begründung
  dokumentiert; Skill-/Regel-/Agenten-Änderungen durchlaufen den
  karpathy-reviewer (globale Regel).
- **Abschlussprüfung jeder Retro — zwei Fragen, beide schriftlich beantwortet**
  (Anweisung des Kunden vom Projektbeginn, präzisiert am 02.08.2026):
  1. *Ist jeder Beschluss in einem Artefakt gelandet?* Eine Erkenntnis, die nur
     im Sprint-Protokoll steht, ist dokumentiert, aber nicht verankert.
  2. *Wird dieses Artefakt automatisch gelesen?* Eine Regel in einer Datei, die
     keine Sitzung von selbst aufschlägt, wirkt nicht. **Das war die längste
     Zeit die eigentliche Lücke dieses Projekts:** Alle zehn Beschlüsse der
     Sprint-2-Retro waren korrekt in `PROZESS.md` und den Agentendateien
     verankert — nur las `PROZESS.md` niemand von selbst. Deshalb existiert
     seit dem 02.08.2026 eine `CLAUDE.md` im Projektstamm; sie ist der Ort, den
     jede Sitzung ohne Zutun liest, und trägt die Regeln, die am häufigsten
     übergangen wurden.

## Loop-Disziplin (gemäß ~/.claude/rules/loop-conventions.md)

- **goal met**: Sprint-Ziel erreicht und DoD erfüllt → Sprint endet.
- **budget spent**: Umfangsgrenze (~13 SP) und je Story max. 2 Anläufe.
- **stalled**: gleicher Fehler zweimal ohne neue Evidenz → Stopp, Impediment
  an PO/Kunden — nicht weiterprobieren.
- **needs a human**: Scope-Änderungen, Zielkonflikte, Kosten — Kundensache.
- **Progress-Log**: jedes Sprint-Protokoll endet mit done/next.
