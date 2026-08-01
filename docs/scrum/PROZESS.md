# Denkzettel — Scrum-Arbeitsvereinbarung

Stand: 2026-07-31. Diese Vereinbarung regelt, wie das Agenten-Team an
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
- Revision dieser Zuordnung ist Retro-Thema.

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
  bei paralleler Arbeit Worktree-Isolation) → Review (PO nimmt gegen
  Akzeptanzkriterien ab; karpathy-reviewer und bei UI-Stories
  `denkzettel-ux` gemäß DoD) → Protokoll → ggf. Retro.
- **Schätzung**: Story Points, Fibonacci (1, 2, 3, 5, 8, 13). Zwei
  unabhängige Schätzer je Story; weichen sie um mehr als eine Stufe ab,
  konsolidiert der Scrum Master mit Begründung. 13er-Stories werden vor dem
  Ziehen geteilt.

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
   ändert (mit Begründung im Issue).
5. Commit(s) auf `main` bzw. Feature-Branch gemäß Kundenvorgabe; Issue
   geschlossen mit Verweis auf Commit.
6. Journal-Eintrag der Session gemäß globaler Protokollpflicht.

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

## Loop-Disziplin (gemäß ~/.claude/rules/loop-conventions.md)

- **goal met**: Sprint-Ziel erreicht und DoD erfüllt → Sprint endet.
- **budget spent**: Umfangsgrenze (~13 SP) und je Story max. 2 Anläufe.
- **stalled**: gleicher Fehler zweimal ohne neue Evidenz → Stopp, Impediment
  an PO/Kunden — nicht weiterprobieren.
- **needs a human**: Scope-Änderungen, Zielkonflikte, Kosten — Kundensache.
- **Progress-Log**: jedes Sprint-Protokoll endet mit done/next.
