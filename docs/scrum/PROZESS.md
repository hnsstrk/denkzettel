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
| **Verwaltung** | Agent `denkzettel-verwalter` (Kundenentscheidung 02.08.2026) | Vollzug bereits protokollierter Entscheidungen: abgenommene Issues und den Milestone schließen, Zweige und Worktrees räumen (auf `origin` nur auf Auftrag), Changelog-**Entwurf** aus den Milestone-Issues; dazu Abgleichsstände **erheben und berichten**. **Führt aus, entscheidet nicht** — keine AK-Haken, keine Mängelbehebung, kein Versionssprung, keine Tags, keine Änderung an Code, SPEC, Prozess oder `CHANGELOG.md`. Fehlt die Fundstelle der Entscheidung, oder verlangt seine Arbeitsliste einen verbotenen Schritt, führt er den erlaubten Teil aus und meldet den Rest |

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
- `denkzettel-verwalter` läuft auf **Haiku** (Kundenentscheidung 02.08.2026).
  Tragfähig, weil seine Arbeit die drei Bedingungen erfüllt, unter denen
  Modellgröße gleichgültig wird: Das Ergebnis ist per Befehl prüfbar, die
  Handlung ist umkehrbar, und die Entscheidung dahinter hat ein anderer
  getroffen. Wo eine dieser Bedingungen fällt, ist er nicht zuständig — nicht
  wegen des Modells, sondern wegen der Rolle.
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
- **Changelog**: `CHANGELOG.md` nach *Keep a Changelog*, aus Nutzersicht
  geschrieben (Kundenentscheidung 02.08.2026). Quelle sind die geschlossenen
  Issues des Sprint-Milestones; Pflege siehe „Sprint-Abschluss", Takt 2.
- **Öffentlichkeit des Repositories** (Kundenentscheidung 02.08.2026): Das
  Repo ist öffentlich, jeder Issue-Kommentar und jede Commit-Botschaft also
  eine Veröffentlichung. Wörtliche Kundenzitate und Messwerte dürfen hinein;
  **Systemdetails und personenbezogene Angaben nicht**. Gilt gleichermaßen für
  Issues, Commits, Protokolle, Belege und den Changelog.
  **Was „Systemdetail" heißt, ist entschieden** (Kunde, viertes
  Design-Interview 02.08.2026, auf die Frage aus Sprint 3, 16.13):
  - **Tabu:** Zugangsdaten und Schlüssel; Interna des Heimnetzes (Hostnamen
    und Adressen anderer Rechner, Freigaben); Pfade außerhalb des Projekts.
  - **Erlaubt:** der Name der Entwicklungsmaschine und Projektpfade in
    Messbelegen — ohne sie ist kein Beleg zu führen, und die Belegpflicht ist
    die tragende Regel dieses Projekts.
  - **Altbestand** wird umformuliert, nicht gelöscht.
  Damit ist der karpathy-Befund 4e vom 02.08.2026 geschlossen: Der
  Rechnername „Ganymed" in DoD 1 und die `stat`-Belege an Systempfaden sind
  regelkonform, Home-Pfade außerhalb des Projekts sind es nicht.
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
- **Basis-Tag (Kundenentscheidung 02.08.2026)**: Beim Planning setzt der PO
  auf den Ausgangsstand den Tag `sprint-NN-basis`. Er macht den Prüf-Diff des
  Sprint-Endes stabil (`git diff sprint-NN-basis..main` statt einer von Hand
  gesuchten Commit-Kennung) — die Reviews nach DoD 3 laufen über „den
  Sprint-Diff", und der wurde bisher jedes Mal neu konstruiert.
- **PR-Probelauf: durchgeführt in Sprint 4, nicht bestanden, beendet.** Das
  Verfahren (je Story-Strang ein Pull Request, Dev-Bericht und Review-Befunde
  als Kommentare, Merge mit `--no-ff` durch den PO) lief unter einem **vorab
  festgelegten** Abbruchkriterium: Am Sprint-Ende hängt mindestens ein Befund
  an einer Diff-Zeile, der ohne PR nicht auffindbar gewesen wäre, **oder** ein
  automatischer Testlauf ist auf einem PR gelaufen — sonst wird das Verfahren
  eingestellt (Kundenentscheidung 02.08.2026). **Beide Hälften sind gemessen
  nicht eingetreten** (Sprint 4, §15.8): `statusCheckRollup` war für #64 und
  #65 leer, und der einzige `fail`-Befund des Sprints hing an Zeilen aus einem
  einzigen Commit, war über `git show` also ebenso auffindbar. Der Probelauf
  ist damit **beendet**.
  **Es gilt der Stand davor:** Zweige und Worktrees nach „Parallelarbeit",
  gemerged ausschließlich vom PO, **kein PR-Zwang**. Basis-Tag und
  Review-Kette bleiben unverändert. Aus dem Probelauf bleibt eine Zeile: das
  Räumen der Zweige **auch auf `origin`** (Sprint-Abschluss, Punkt 8) — sie
  hängt am öffentlichen Repo und nicht am Pull Request und gilt weiter, sooft
  ein Story-Zweig überhaupt auf `origin` gelangt.
  *Zur Ehrlichkeit des Ergebnisses:* Der Probelauf ist nur zur Hälfte
  durchgeführt worden — an beiden PRs stand weder ein Dev-Bericht noch ein
  Review-Befund; getragen hat allein die Mechanik. Eine spätere Einführung
  automatischer Testläufe (CI) ist davon unberührt und wäre eine **neue
  Kundenentscheidung**; sie ist der einzige der beiden Kriteriumsteile, der
  ohne menschliche Disziplin trägt.
- **Push-Kadenz (Kundenentscheidung 02.08.2026)**: Der PO pusht nach jedem
  abgeschlossenen Arbeitsblock ohne Rückfrage. `main` ist ohnehin
  veröffentlicht — die Sichtbarkeit ändert sich dadurch nicht, das Liegenbleiben
  schon: Der Push war in drei Sprints zweimal der letzte offene Punkt.

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

Ebenfalls zur Sprint-Ende-Prüfung gehört die **Schätzhistorie** (Takt 2,
Punkt 12): Der Scrum Master legt in der DoD-Prüfung die Tabelle der Stories
dieses Sprints vor — Issue · Erstschätzung (Wert, Datum, Quelle, Zahl der
Schätzer) · Revisionen · Endwert · Umsetzungssprint · **Anlass-Kennzeichen**
(`gegenstand-geändert` | `erkenntnis` | `keine`) — und prüft, ob das
committete Diagramm aus der aktuellen Datenreihe erzeugt ist. Das Kennzeichen
ist das Urteil, das der Verwalter später mechanisch überträgt und nicht
selbst fällen darf: Eine Schätzung, die stieg, **weil sich der Gegenstand
änderte**, ist keine Schätzabweichung, und ein Punkt, der beides vermengt,
zeichnet einen Kegel, der nichts misst. Auch dieses Ergebnis steht im
Protokoll, wenn nichts zu beanstanden war.

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
   Milestone geschlossen (DoD 5). **Arbeitsteilung, weil dieser Punkt zugleich
   Arbeitsliste des `denkzettel-verwalter` ist:** Die **AK-Haken setzt der PO**
   — sie sind seine Abnahmeentscheidung, nicht umkehrbar und werden zur
   Beweislage. Das **Schließen der abgenommenen Issues** mit Kommentar und
   Commit-Verweis sowie das Schließen des Milestones ist Vollzug einer bereits
   getroffenen Entscheidung und delegierbar. **Gegenstand der Delegation ist
   die Abnahme, nicht der Milestone:** Der Auftrag zählt die Issues auf, die
   der PO abgenommen hat. „Alle Issues des Milestones" ist kein Ersatz — ein
   Issue kann am Milestone hängen, ohne abgenommen zu sein, und ein
   Abnahmekommentar an einer nicht abgenommenen Story ist ein falscher Eintrag
   in der Beweislage (karpathy-Nachprüfung, N3).
6. Journal bis zum letzten Commit nachgeführt (DoD 6).
7. `main` gepusht.
8. Story-Zweige und Worktrees entfernt. Das Kriterium ist ein Exit-Code, kein
   Ermessen: `git merge-base --is-ancestor <zweig> main` → 0 heißt löschbar
   (gleichwertig: der Zweig steht in `git branch --merged main` und nicht in
   `git branch --no-merged main`). Erst `git worktree remove`, dann
   `git branch -d` — niemals `-D`. **Ab dem PR-Probelauf auch auf `origin`:**
   `git push origin --delete <zweig>`. Sonst bleiben genau die
   Zwischenstände dauerhaft öffentlich stehen, die der Basis-Tag vermeiden
   sollte — das Repo ist öffentlich.
9. **Changelog fortgeschrieben** (Kundenentscheidung 02.08.2026): ein Abschnitt
   je Version in `CHANGELOG.md`, gezogen aus den geschlossenen Issues des
   Milestones. **Maßstab ist die Nutzersicht, nicht das Label.** `typ:tech` ist
   ein Anhaltspunkt, kein Filter — am ersten Changelog gemessen: **#6**
   (Autostart) trägt `typ:tech` und steht zu Recht drin, weil ein Dienst, der
   sich selbst startet, sichtbar ist; **#1** (Wayland-Spike) trägt dasselbe
   Label und bleibt zu Recht draußen; **#9** (Migrationstest) erscheint nur
   dort, wo er die Schemazeile belegt. **Jede Änderung am Datenbank-Schema
   wird genannt**, auch wenn kein Issue sie im Titel trägt — sie steckt als
   Nebenwirkung in einer Story und ist das, was einen Nutzer am meisten
   angeht. Die
   Repo-Grenzen (Artefakte) gelten. Der `denkzettel-verwalter` liefert je nach
   Auftrag die **Rohliste** oder einen **vorformulierten Entwurf**; in beiden
   Fällen **verantwortet der PO Auswahl und Text**, und der Verwalter trägt
   nichts selbst in `CHANGELOG.md` ein. Das Ziehen ist mechanisch, die
   Aufnahme eines Eintrags ist ein Urteil — deshalb taugen die Labels hier
   nicht als Filter (Sprint 3, 16.3.3).
10. **Version erhöht und getaggt** (Kundenentscheidung 02.08.2026): 0.x-SemVer,
    **MINOR bei jeder Kundenabnahme**, PATCH für außerplanmäßige Behebungen,
    `1.0.0` erst auf Erklärung des Kunden; **jede Schemamigration erzwingt
    mindestens einen MINOR-Sprung**. Einzige Quelle ist `CMakeLists.txt`, der
    Tag `vX.Y.Z` ist das Siegel, nicht die Quelle. Auslöser ist die Abnahme,
    nicht das Sprint-Ende: Der Sprintschluss ist ein Ereignis des Teams, ein
    Release eines für den Kunden — ein Sprint ohne Abnahme erzeugt keine
    Version. **Wirksam ab #61**; bis dahin erreicht die Zahl die Anwendung
    nicht und wäre eine Behauptung ohne Sichtbarkeit. **Bis #61 umgesetzt ist,
    führt der Vollzugsvermerk (Punkt 11) diesen Punkt als *ausgesetzt*, die
    Einträge sammeln sich unter `[Unveröffentlicht]`, und Abnahmen davor
    bekommen keine Version rückwirkend.** Ab #61 siegelt der `vX.Y.Z`-Tag den
    Abschluss; ein zusätzlicher Abschluss-Tag entfällt dann. Der einmalige
    `sprint-03-abschluss` ist eine Übergangsform aus der Zeit vor dieser Regel
    (Sprint 3, 16.13) und kein drittes Tag-Schema.
11. Der Scrum Master vermerkt den **Vollzug von Takt 2** im Sprint-Protokoll.
12. **Schätzhistorie fortgeschrieben** (Kundenauftrag 02.08.2026): Die
    Datenreihe `docs/scrum/diagramme/schaetzhistorie.json` trägt die Stories
    des abgeschlossenen Sprints, und das Diagramm ist daraus neu erzeugt.
    **Arbeitsteilung:** Die Zeile je Story steht mit ihrem
    **Anlass-Kennzeichen** bereits in der DoD-Prüfung des Sprint-Protokolls
    (Takt 1) — dort ist sie ein **Urteil**. Der `denkzettel-verwalter`
    überträgt sie **mechanisch**, lässt den Generator laufen und **meldet den
    Diff**; er trägt keine eigene Zeile ein und ändert weder Wert noch
    Kennzeichen. Geht der Diff über die neuen Zeilen hinaus, ist das ein
    Befund an den PO, keine Selbstheilung. Der PO committet.
    *Was das Diagramm zeigt und was nicht:* Es misst den **Revisionsfaktor**
    (Endwert ÷ Erstwert) über dem **Abstand in Sprints** zwischen
    Erstschätzung und Umsetzung — **nicht** den Abstand zum tatsächlichen
    Aufwand; dieser wird im Projekt nicht erhoben. Der Satz, der das sagt,
    steht unter dem Bild und ist Bedingung, nicht Zierde: Eine Story, die
    niemand neu geschätzt hat, steht bei 1,0, auch wenn sie teurer war.
    Deshalb bleiben Stories, deren Erstschätzung und Umsetzung in **dasselbe**
    Planning fallen, aus der Kurve heraus — ihr Faktor ist 1,0 von
    Konstruktion wegen und keine Messung. Sie werden trotzdem erfasst, damit
    die Auslassung sichtbar bleibt.

Zwei Takte, weil DoD 5 und DoD 6 vor der Abnahme gar nicht erfüllbar sind: In
Sprint 3 wurden sie zum Prüfzeitpunkt trotzdem als Mängel geführt (M2, M5),
und der Doku-Abgleich lief vor der Abnahme — deshalb ging er an der
Statuszeile vorbei, die erst durch die Abnahme falsch wurde.

Version, Tag und Changelog standen bis zum 02.08.2026 bewusst **nicht** in
dieser Liste, weil sie Kundenentscheidung waren. Der Kunde hat sie am
02.08.2026 wie vorgeschlagen entschieden (Sprint 3, 16.9); seitdem sind sie
die Punkte 9 und 10.

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
