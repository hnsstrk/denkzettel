# Denkzettel — Scrum-Arbeitsvereinbarung

Stand: 2026-08-04. Diese Vereinbarung regelt, wie das Agenten-Team an
Denkzettel arbeitet. Änderungen an ihr entstehen in Retrospektiven **oder aus
einer Kundenentscheidung** — die Änderungen vom 04.08.2026 (automatische
Testläufe, Verwalter-Bericht als Existenzprüfung, Pflicht-/Kürteil der
Protokolle, Rückbau des Schätzkegels, Prüfsummen der Bildbelege, Label erst mit
der zweiten Prüfung, **Ende der Story-Point-Schätzung zugunsten des
Vorprüfberichts**) sind auf dem zweiten Weg entstanden. Die **Ablage der
Vorprüfung als Ordner** und die **drei Zusätze zur Definition of Ready** kamen
am selben Tag aus dem ersten Lauf des neuen Verfahrens hinzu.

**Das Standdatum wird bei jeder Änderung mitgeführt.** Ein Datum, das niemand
fortschreibt, ist nach der ersten Änderung **still falsch** — die Datei sieht
richtig aus und datiert sich selbst zurück. Diese Fehlerklasse hat das Projekt
am eigenen `stand`-Feld beschrieben, bevor sie hier zuschlug.

## Rollen

| Rolle | Besetzung | Verantwortung |
|---|---|---|
| **Kunde** | hnsstrk | Ziele, Prioritätswünsche, Sprint-Freigaben, Abnahme des Produkts |
| **Product Owner** | Claude (Haupt-Session) | Backlog-Inhalt und -Priorisierung, Story-Schnitt, Akzeptanzkriterien, Abnahme der Stories, Kundenkontakt |
| **Scrum Master** | Agent `scrum-master` | Prozesshüter: zweiter Bearbeiter der Vorprüfung und Träger des Ready-Urteils, schlägt Sprint-Schnitt vor, wacht über Definition of Done, moderiert Retrospektiven, pflegt Prozess-Doku, meldet Impediments |
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

  **Revision in der Sprint-6-Retro (04.08.2026): bestätigt** (B20). Der eine
  Befund des Sprints, den eine Opus-Rolle geschrieben und der Reviewer auf
  Fable durch **Nachzählen** gefunden hat, ist K1
  (`docs/scrum/reviews/sprint-06-karpathy.md`): „jede tragende Zusicherung
  gegen eine Mutation gehalten" deckte 8 von 11. *Einschränkung, damit der
  Beleg nicht stärker aussieht, als er ist:* Der Reviewer war der erste fremde
  Leser des Berichts — dass eine Opus-Rolle den Fund übersehen **hätte**, ist
  damit nicht belegt; der Sprint-3-Beleg bleibt der tragende. Kein Anlass, das
  Sicherheitsnetz abzustufen. Der `denkzettel-verwalter` auf Haiku hat zwei
  Aufträge mit abgelegter Datei und nachgemessenem Ergebnis ausgeführt.
  **Nächste Revision: Retro nach Sprint 9.**

## Artefakte und Werkzeuge

- **Product Backlog**: GitHub Issues im Repo `hnsstrk/denkzettel` — die
  **einzige Quelle der Wahrheit** für Stories, Akzeptanzkriterien, Größenklasse
  und Status. Kein Backlog-Spiegel im Repo (Kundenentscheidung 31.07.2026).
  Labels: `epic:M1`…`epic:M7`, `size:s|m|l|xl`, `typ:story|bug|tech`.
  **Das Größenklassen-Label wird im selben Zug gesetzt wie der Vorprüfbericht —
  vorher gar nicht** (04.08.2026; bis dahin galt derselbe Satz für das
  `sp:`-Label und die zweite Schätzung). *Grund:* Ein Label sieht gleich aus, ob
  ein geprüftes Urteil dahintersteht oder eine beim Anlegen hingeschriebene
  Zahl — und beides ist vorgekommen: #57 trug `sp:2` aus der Anlage, zwei
  unabhängige Schätzer hoben auf 3 (Sprint 5 §2.1); #68 trug `sp:5` aus einer
  Hand (Sprint 6 §2.4). Das Sprint-6-Planning brauchte deshalb eine eigene
  Spalte „Schätzregel erfüllt?", um Label von Schätzung zu trennen. In der
  *einzigen Quelle der Wahrheit* ist ein doppeldeutiges Feld teurer als
  anderswo. Eine grobe Hausnummer darf bleiben, aber als **Freitextsatz im
  Issue, der nicht wie ein Prüfergebnis aussieht**. Damit bleibt der zweimal
  gerissene Punkt verankert statt ein drittes Mal nachgefordert (fehlendes
  Label: M2 in Sprint 5, K1 in Sprint 6) — er wandert mit dem Verfahren mit,
  statt mit ihm zu verschwinden.
  **`sp:`-Altbestand — Feststellung, kein Auftrag** (gemessen 04.08.2026,
  `docs/scrum/reviews/2026-08-04-splabel-nachmessung.txt`): Einen Berg
  unbelegter Label gab es nicht. Die Schätzklausur in `sprint-01.md` deckte den
  Bestand ab — 41 Positionen, davon **34 mit zwei Schätzern**. Die übrigen
  **sieben** (`T1`–`T7`) ruhten auf einer konsolidierten Zahl **aus einer
  Hand**, in §4 desselben Protokolls je einzeln begründet: kein Vorwurf, aber
  auch keine Zweitschätzung. Von ihnen waren am 04.08.2026 noch **drei offen und
  trugen ein Label** (#17, #19, #25); sie wurden am selben Tag auf
  Kundenentscheidung entlabelt, der Wert blieb als Freitextsatz im Issue stehen
  — ausdrücklich als *Aufwandshinweis, keine Schätzung*, mit Verweis auf die
  Klausurzeile. **Das ist die Bauart, in der ein Label ohne Prüfgrundlage
  behandelt wird: Label weg, Zahl bleibt lesbar, ohne sich als Prüfergebnis
  auszugeben.**
  **Wer in den Protokollen der Frühzeit nachschlägt, sucht nicht nach der
  Issue-Nummer:** Die Klausurtabelle führt **Story-IDs** (`S22`, `T5`), der
  Backlog führt **Nummern**; `sprint-01.md` enthält keine einzige Issue-Nummer,
  und allein der Issue-Titel verbindet beides. Eine Suche nach der Nummer meldet
  deshalb zwei Drittel der Positionen als unbelegt — sie kann nicht finden,
  wonach sie sucht, und das Ergebnis sieht trotzdem aus wie ein Befund. Genau
  daran sind am 04.08.2026 zwei Berichte zum selben Bestand entgegengesetzt
  ausgefallen.
- **Priorisierung**: Epic-Reihenfolge M1→M7 als Grundlinie; die
  Feinreihenfolge legt der PO beim Sprint-Planning über die
  Milestone-Zuordnung fest.
- **Sprint**: GitHub Milestone `Sprint N` mit den gezogenen Issues.
- **Vorprüfberichte**: `docs/scrum/vorberichte/NN-<kurzname>/`, NN ist die
  **Issue-Nummer**; der Kurzname folgt dem Zweignamen der Story
  (`story/83-native-huelle` → `83-native-huelle/`), damit Bericht und Zweig
  zusammenfinden. **Ein Ordner je Story** mit `messung-a.md`, `messung-b.md`,
  dem konsolidierten `bericht.md` und den Messausgaben; der Bericht liegt vor,
  bevor die Story gezogen werden darf. Felder und Verfahren siehe
  Sprint-Mechanik.
  *Warum ein Ordner und nicht eine Datei* (04.08.2026, erster Lauf des
  Verfahrens): Zwei unabhängige Bearbeiter können nicht in dieselbe Datei
  schreiben, ohne voneinander zu erfahren — und die Unabhängigkeit ist der
  Zweck. Die Messausgaben brauchen ohnehin einen Ordner (B7). Die erste Fassung
  sagte „je Story eine Datei" und ist beim ersten Anfassen gerissen; **beide
  anderen Bearbeiter haben die Abweichung unabhängig gemeldet**
  (`docs/scrum/vorberichte/83-native-huelle/bericht.md` §7.1).
  *Warum ein eigener Ordner neben `reviews/`:* Der Sprint-Abschluss zählt dort
  in Takt 1, Punkt 2 die **Berichte gelaufener Prüfungen** und hält sie gegen
  die Commit-Botschaften. Ein Bericht, zu dem es keinen Lauf gibt, verwässert
  genau diese Zählung. *Warum dieser Ordnername:* `vorpruefung` wäre ein
  ue-Ersatz für „Vorprüfung" und damit ein Verstoß gegen die
  Encoding-Regel; „Vorbericht" trifft die Sache und braucht keinen Umlaut.
- **Protokolle**: `docs/scrum/sprints/sprint-NN.md` (Planning, Review,
  ggf. Retro) — je Sprint eine Datei, angelegt beim Planning.
  **Pflichtteil und Kürteil (04.08.2026).** Pflicht sind vier Abschnitte:
  **Sprint-Konto** (B12), **DoD-Prüfung**, **Mängelliste**
  und **done/next**; in Retro-Protokollen zusätzlich die **Beschlüsse** mit
  ihrem Artefakt. Alles andere — Herleitungen, Risikolisten, nacherzählte
  Messwege — ist Kür: erlaubt, nicht geschuldet, und nicht dreimal.
  *Grund:* Am 04.08.2026 gemessen stehen **13.659 Zeilen** Prozessdokumentation
  gegen **3.894 Zeilen** Produktivcode. Der Wert steckt nachweislich in den vier
  Pflichtabschnitten — jeder belegte Fund dieses Projekts ist in einem von ihnen
  festgehalten. **Gekürzt wird die Prosa, nie der Beleg:** Eine Herleitung
  wegzulassen kostet Lesbarkeit, einen Beleg wegzulassen kostet die
  Nachprüfbarkeit, und B7 steht dagegen.
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
  **Ein überholter Beleg wird geankert, nicht geglättet** (B17): Der
  Berichtstext bleibt, wie er war; angehängt wird eine datierte Zeile, die den
  Prüfstand nennt und den Commit, seit dem der Satz nicht mehr gilt (`346a4c0`,
  `98d9455` sind die Bauart). Ein Bericht ist Beweislage seines Standes — wer
  ihn nachzieht, zerstört genau das, wofür B7 ihn ins Repo gestellt hat.
  **Sätze im Präsens ohne genannten Prüfstand sind die Stelle, an der das
  auffällt** (Sprint 6, §19.4).
- **Fachliche Quellen**: `SPEC.md` (bindend), `KONZEPT.md` (Historie der
  Entscheidungen), `wireframes/` (UI-Referenz; Spiegel im
  Claude-Design-Projekt „Denkzettel" auf claude.ai/design, Sync durch den
  PO über das DesignSync-Werkzeug seiner Claude-Code-Session —
  Kundenentscheidung 31.07.2026).

## Sprint-Mechanik

- **Sprint-Umfang**: 2–4 Stories mit einem benennbaren Sprint-Ziel. Dazu die
  Klassenregel: **kein `size:xl`** (die Story wird vor dem Ziehen geteilt),
  **höchstens eine `size:l`**, und **neben einer `size:l` steht nur `size:s`**.
  Sprints sind Arbeitspakete, keine Zeiträume.
- **Freigabemodell (Kundenentscheidung 31.07.2026)**: Jeder Sprint startet
  erst nach Freigabe durch den Kunden (Sprint-Ziel + gezogene Stories werden
  vorgelegt).
- **Ablauf**: Vorprüfung je Kandidat (zwei Bearbeiter, Bericht, Ready-Urteil)
  → Planning (SM schlägt Schnitt vor; bei UI-Stories berät
  `denkzettel-ux`; PO bestätigt, Kunde gibt frei) → Umsetzung (Dev-Agenten;
  bei paralleler Arbeit Worktree-Isolation, siehe „Parallelarbeit") → Review
  (PO nimmt gegen Akzeptanzkriterien ab; karpathy-reviewer und bei UI-Stories
  `denkzettel-ux` gemäß DoD) → Protokoll → ggf. Retro.
- **Vorprüfbericht (Kundenentscheidung 04.08.2026, an die Stelle der
  Schätzung)**: Bevor eine Story gezogen werden darf, liegt zu ihr ein
  Vorprüfbericht unter `docs/scrum/vorberichte/` (Artefakte). Er trägt sechs
  Felder:
  1. **Dateimenge** — was ein Strang für diese Story anfassen darf, **am Code
     vermessen**, in der Notation von B13. Muster: Sprint 6 §5.2 mit den fünf
     Zeilen *Quellen und Tests · Build · Belege und Prüfmittel · fachliche
     Quellen · **ausdrücklich nicht***.
  2. **Gemessene Fallen** — die Zeilen, die in den Spawn-Auftrag wandern, je
     mit Messbeleg. Muster: die sechs Punkte in Sprint 6 §10.6, „die je einen
     Fehlversuch ersparen".
  3. **AK-Urteil: ready ja/nein** — siehe „Definition of Ready" unten.
     *In der Arbeit nach Feld 4 zu füllen:* Ob ein Kriterium prüfbar ist,
     entscheidet sich am Prüfmittel. Die Nummer beschreibt den Bericht, nicht
     den Arbeitsgang.
  4. **Prüfmittel** — womit der Nachweis geführt wird, und **was der Agent
     nicht prüfen kann**. Muster: Unter Wayland kann ein Agent kein Alt-Tab
     auslösen, weil ein Prozess sich den Fokus nicht selbst zuteilt
     (Sprint 6 §16.1, M-B1).
     **Wer eine Story schneidet, schneidet auch Feld 4.** Ein Kriterium, das
     erst beim Schnitt entsteht, hat keine Zeile im Prüfmittelfeld des Berichts
     und ist damit ungeprüft ziehbar. *Gemessen am Schnitt von #83
     (04.08.2026):* Von 19 Kriterien beider Teile war genau eines neu, und
     genau dieses eine war ohne Prüfmittel, ohne benannten Prüfgrund und ohne
     B21-Belegform.
  5. **Größenklasse** — `size:s|m|l|xl`, siehe unten.
  6. **Offene Fragen** an Kunde oder PO.

  **Zwei unabhängige Bearbeiter je Story.** Diese Anforderung stammt aus der
  Schätzregel und bleibt; sie gilt jetzt dem Bericht. **Bearbeiter A** ist ein
  `denkzettel-dev` — bei UI-Stories `denkzettel-ux` — und misst am Code.
  **Bearbeiter B** ist der Scrum Master und misst unabhängig, ohne das Ergebnis
  von A zu kennen. Weichen die Größenklassen um eine Stufe ab, gilt die höhere; bei
  größerer Abweichung entscheidet der Scrum Master begründet, und die Begründung
  steht im Bericht. **Das Ready-Urteil (Feld 3) fällt der Scrum Master** nach
  beiden Messungen; ein „nein" behebt der PO (melden, nicht heilen).
- **Definition of Ready (04.08.2026)**: Eine Story ist ziehbar, wenn ihr
  Vorprüfbericht vorliegt und Feld 3 **ready ja** trägt — die
  Akzeptanzkriterien sind vollständig und einzeln prüfbar, und zu jedem ist in
  Feld 4 ein Prüfmittel benannt oder die Grenze der Prüfbarkeit ausgesprochen.
  Dazu drei Sätze aus dem ersten Lauf des Verfahrens (04.08.2026, Belege in
  `docs/scrum/vorberichte/83-native-huelle/bericht.md` §7):
  - **Ein Issue mit selbstdeklarierten offenen Punkten ist nicht ready**,
    unabhängig von seinen Kriterien. #83 führte drei Punkte „vor dem Ziehen zu
    entscheiden"; das Urteil stimmte nur zufällig, weil auch die Kriterien nicht
    trugen — die Regel hätte es nicht gefunden, wenn sie getragen hätten.
  - **Ein Dateiname ist erst dann ein Prüfmittel, wenn `git ls-files` ihn
    zeigt.** `native-farben.txt` erfüllte die DoR wörtlich und existiert nicht.
    Dieselbe Bauart wie die Existenzprüfung des Verwalter-Berichts
    (Sprint-Abschluss, Punkt 11): Es fehlt nicht die Regel, es fehlt ihre
    Prüfbarkeit.
  - **Behauptet ein Kriterium etwas über Hülle, Rundung, Kontur, Schatten,
    Dekoration oder Durchsichtigkeit, nennt es das Sitzungsbild als Belegform**
    (B21, DoD 3). In der ersten Fassung von #83 sprachen drei von acht
    Kriterien über diese Größen, keines nannte ein Bild — das Issue entstand am
    selben Tag wie der Beschluss. B21 muss die Stelle erreichen, an der
    Kriterien **formuliert** werden, nicht erst die, an der sie geprüft werden.
  *Warum das ausdrücklich hier steht:* Dieses Gate hat gewirkt, ohne benannt zu
  sein — getragen hat es die alte Schätzregel. Im Sprint-6-Planning hat sie
  vier von acht Kandidaten aussortiert (#70 mit offenen AK; #71, #72, #61
  ungeschätzt), in Sprint 4 zusätzlich #62 (§2.3). Mit dem Ende der Schätzung
  wäre es ersatzlos weggefallen. Es ist der Teil des alten Verfahrens, dessen
  Wirkung sich an Fällen zeigen lässt.
- **Größenklassen (04.08.2026, an die Stelle der Fibonacci-Punkte)**: vier
  Klassen, gesetzt als Label am Issue.

  | Label | Bedeutung |
  |---|---|
  | `size:s` | läuft nebenher — wenige Dateien, kein neuer Prüfweg |
  | `size:m` | trägt einen Strang aus |
  | `size:l` | füllt den Sprint — daneben passt nur `size:s` |
  | `size:xl` | muss geteilt werden — nicht ziehbar |

  **Rückgeprüft gegen die Sprints 1–6** (04.08.2026, Zahlen aus den
  Protokollen; die SP-Spalte dient nur der Rückprüfung und wird nicht
  fortgeführt). Jeder tatsächlich gezogene Sprint bleibt unter der neuen Regel
  zulässig:

  | Sprint | gezogen (alt: SP) | Klassenprofil | Regel |
  |---|---|---|---|
  | 1 | 2 · 3 · 3 · 5 = 13 | s m m m | hält |
  | 2 | 3 · 2 · 5 = 10 | m s m | hält |
  | 3 (bei Freigabe) | 5 · 3 · 1 · 1 = 10 | m m s s | hält |
  | 4 | 5 · 5 · 1 = 11 | m m s | hält |
  | 5 | 5 · 2 · 3 · 1 = 11 | m s m s | hält |
  | 6 | 8 · 1 · 2 = 11 | l s s | hält — neben der `l` nur `s` |

  **Drei Klassen genügen nicht, und das ist gemessen.** Zählt eine 5er-Story
  als „füllt den Sprint", wird Sprint 4 (5 · 5 · 1) unzulässig — ein Sprint, der
  gezogen, geliefert und abgenommen wurde. Zählt sie als „läuft nebenher",
  umfasst die untere Klasse alles von 1 bis 5, und die einzige Zurückweisung, die je an
  der Größe allein hing, lässt sich nicht mehr beschreiben: #55 + #56 + #68
  (8 · 1 · 5) fiel in Sprint 6 heraus; mit `s` und `l` allein hielte diese
  Kombination. Die vierte Klasse trennt beide Fälle, und die Zeile „neben einer
  `size:l` steht nur `size:s`" wirft die Sprint-6-Kombination heraus, wie
  gemessen.
  **Was die Klassen nicht können, ausdrücklich:** Das Kandidatenfeld von
  Sprint 4 (5 · 5 · 3 · 1) wurde mit 14 Punkten zurückgewiesen, Sprint 1 mit 13
  angenommen — beide tragen dasselbe Profil `m m m s`. Keine grobe Einteilung
  trennt 13 von 14; das könnte allein die Zahl, und die fällt aus den unten
  genannten Gründen weg. Diese Last trägt jetzt Feld 1 des Vorprüfberichts: Der
  Sprint-6-Zuschnitt ist am Code vermessen worden — „Der kleinste Abstand ist
  null: Beide schreiben in dieselbe 157-Zeilen-Datei" (§5.1) — und nicht aus
  Punkten abgeleitet. **Offene Flanke, benannt statt zugedeckt:** Vier `size:m`
  sind nach dieser Regel zulässig und lägen über allem, was je gezogen wurde
  (der größte Sprint war 13 Punkte). Der Fall ist in sechs Sprints nicht
  eingetreten; tritt er ein, gehört er dem Kunden vorgelegt, und erst dann ist
  eine engere Regel begründet statt geraten.
- **Story Points: eingeführt in Sprint 1, beendet am 04.08.2026.** Das Verfahren
  (Fibonacci 1–13, zwei unabhängige Schätzer je Story, Konsolidierung durch den
  Scrum Master, Umfangsgrenze ~13 SP) ist auf Kundenentscheidung eingestellt.
  **Vier gemessene Gründe:**
  1. **Die Zahl erreichte den bauenden Agenten nie.** In
     `.claude/agents/denkzettel-dev.md` kommt keine Punktzahl vor; einziger
     SP-Treffer über alle sechs Agentendateien war der Sprint-Zuschnitt in
     `scrum-master.md`.
  2. **Die Kontextgrenze eines Strangs leistet die Dateimenge.** Der
     Sprint-6-Zuschnitt wurde am Code vermessen (§5.1), nicht aus Punkten
     abgeleitet.
  3. **In sechs Sprints hat die Punktgrenze genau einmal allein den Ausschlag
     gegeben** (Sprint 6: #68 draußen, weil 14 > 13; die Story-Grenze hätte drei
     Stories durchgelassen). Die ~13 stammt aus Sprint 1 und ist nie gegen eine
     gemessene Kapazität geprüft worden. In Sprint 3 hat sie geschadet: Alle
     sahen auf die Punkte, und dass die Story-Grenze bei fünf Issues fiel,
     bemerkte niemand (B12).
  4. **Der tatsächliche Aufwand wird in diesem Projekt nicht erhoben** —
     derselbe Befund, an dem am selben Tag der Schätzkegel gescheitert ist.
  **Wer Story Points erneut vorschlägt, braucht zuerst die gemessene
  Aufwandszahl je Story.** Ohne sie kehrt dieselbe Lücke wieder: eine Skala, die
  niemand gegen die Wirklichkeit halten kann.
- **Schätzkegel: eingeführt am 02.08.2026, entfernt am 04.08.2026.** Ein
  Diagramm samt Datenreihe (`docs/scrum/diagramme/`) trug den
  **Revisionsfaktor** (Endwert ÷ Erstwert) über dem Sprint-Abstand zwischen
  Erstschätzung und Umsetzung auf; daran hingen ein eigener Abschluss-Punkt,
  ein DoD-Prüfsatz und eine Übertragungsregel für den Verwalter. Er maß damit,
  **wie oft neu geschätzt wurde**, nicht **ob richtig geschätzt wurde** — der
  tatsächliche Aufwand wird in diesem Projekt nicht erhoben, und eine nie
  revidierte Story steht bei 1,0 auch dann, wenn sie das Doppelte kostete.
  Dem Kunden lagen drei Wege vor (Aufwandserhebung nachrüsten · entfernen ·
  unverändert lassen); er hat am **04.08.2026** das Entfernen gewählt. **Wer
  dieses Werkzeug erneut vorschlägt, braucht zuerst die gemessene
  Aufwandszahl je Story** — ohne sie kehrt dieselbe Lücke wieder.
- **Sprint-Konto (Retro Sprint 3, B12; Buchungsgröße angepasst 04.08.2026)**:
  Das Sprint-Protokoll führt **beide** Grenzen laufend mit — Zahl der Issues
  *und* Klassenverteilung, je mit Ausgangs- und neuem Stand. Bauart einer
  Buchungszeile: `3 Issues · 1×l, 2×s`. Jeder Zugang nach der Freigabe wird dort
  gebucht; berührt er eine der beiden Grenzen, legt der PO ihn dem Kunden **als
  Grenzüberschreitung** vor, nicht nur als Story. *Grund:* In Sprint 3 wurde
  bei jedem Zugang die Punktzahl mitgezählt, die Zahl der Issues nicht — der
  Sprint endete bei fünf Issues, und dass damit eine Grenze fiel, ist dem
  Kunden nie vorgelegt worden (Sprint 3, 12.7 und 13.10). Wer 13 im Blick
  hatte, sah die 13 einhalten und übersah die 5. **Es bleiben zwei Grenzen,
  auch ohne Punkte** — B12 ist gegen die Blindstelle gefasst, dass eine Grenze
  die andere verdeckt, und die Blindstelle hängt nicht an der Maßeinheit.
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
  ohne menschliche Disziplin trägt. **Diese Entscheidung ist am 04.08.2026
  gefallen — siehe „Automatische Testläufe" unten.**
- **Automatische Testläufe (Kundenentscheidung 04.08.2026)**: `.github/workflows/ci.yml`
  baut und testet bei jedem Push auf `main`, bei jedem Pull Request und auf
  Abruf. Er läuft in einem Arch-Container, also im selben rollenden
  Paketstrom wie die Entwicklungsmaschine, und schlägt fehl bei **Baufehler,
  bei jeder Compiler-Warnung, bei jedem roten Test und bei jedem
  Linterbefund**. Die Warnungsschwelle steht auf null, weil sie am 04.08.2026
  an einem Neubau gemessen null war — eine Schranke oberhalb des gemessenen
  Standes wäre keine.
  **Die beiden Linterschwellen stehen seit #76 (05.08.2026) ebenfalls auf
  null**, `lint-clazy` heruntergezogen von 3 und `lint-tidy` als neuer
  Schritt. Beide Wachen prüfen die **Dreizahl** — Rückgabewert, Zahl der
  Warnungen, Zahl der Fehler —, weil keine der Zahlen allein trägt: Der
  Rückgabewert meldet Befunde nicht (clazy liefert rc=0 auch mit Befunden),
  und eine Zählung meldet den stillen Nulllauf nicht (eine abgebrochene
  Übersetzungseinheit liefert null Befunde über Dateien, die nie analysiert
  wurden). Die Schwelle steht zusätzlich in DoD 1, damit sie nicht wieder
  allein in einer YAML-Datei lebt.
  *Warum das der Punkt ist, an dem der PR-Probelauf scheiterte:* Von dessen
  Abbruchkriterium war dies die Hälfte, die **keine Disziplin** verlangt. Die
  andere Hälfte — Berichte und Befunde am PR — ist an genau der Disziplin
  gerissen.
  **Was der Lauf nicht ersetzt, ausdrücklich:** DoD 1 verlangt die Tests **auf
  Ganymed**, und dabei bleibt es — der Container ist ein zweites Netz, keine
  Verlegung des Prüfstands. DoD 2 (installierter Stand unter `/usr`) und DoD 3
  (Bildprüfung) erreicht er gar nicht: Er hat keinen Compositor, kein `/usr`
  dieses Projekts und baut die Bildläufer nicht. Wer die grüne Marke für eine
  erfüllte DoD hält, hat drei ihrer sechs Punkte übersprungen.
- **Push-Kadenz (Kundenentscheidung 02.08.2026)**: Der PO pusht nach jedem
  abgeschlossenen Arbeitsblock ohne Rückfrage. `main` ist ohnehin
  veröffentlicht — die Sichtbarkeit ändert sich dadurch nicht, das Liegenbleiben
  schon: Der Push war in drei Sprints zweimal der letzte offene Punkt.

## Definition of Done (je Story)

1. Code kompiliert warnungsarm; neue Logik hat Unit-/Integrationstests, alle
   Tests grün (auf Ganymed). Jede Aussage des Wireframes über die
   Raumaufteilung einer Ansicht ist als Geometrie-Zusicherung im Test
   festgehalten, geprüft bei zwei Fenstergrößen (offscreen genügt).
   **Die Linterschwelle ist null** (seit #76, 05.08.2026): `lint-tidy` und
   `lint-clazy` liefern in der Standardkonfiguration
   (`-DDENKZETTEL_SPIKE_SPELLFIX=OFF`) je **rc=0, null Warnungen, null
   Fehler**. Sie stand vorher allein in `ci.yml`, und „warnungsarm" hier sagte
   über die Linter nichts — eine entdeckte Bedingung nach DoD 4/B9.
   **Drei Zahlen und nicht eine, beide Lücken gemessen:** `rc` allein ist kein
   Befundurteil (`clazy-standalone` liefert rc=0 auch mit Befunden), eine Zahl
   allein ist kein Vollständigkeitsurteil (fehlt eine `.moc`-Datei, bricht die
   Übersetzungseinheit ab und der Zähler meldet **null** Befunde über Dateien,
   die nie analysiert wurden). **Ein Befund darf nur mit `NOLINT` und
   danebenstehender Begründung stehenbleiben**; ein Fall, den die bestehenden
   Begründungen nicht decken, geht an den PO und nicht in ein stilles `NOLINT`.
   Ein Sprung von clang oder clazy bringt neue Prüfungen mit und färbt den Lauf
   rot, ohne dass jemand etwas geändert hat — das ist die beabsichtigte
   Frühwarnung und kein Grund, die Schwelle zu heben.
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
   riskanten Stories).
   **Der Review-Auftrag benennt den Diff, nicht die Stories** (B19): Bereich
   (`sprint-NN-basis..main`), Zahl der Dateien und die Teile, die *nicht* aus
   den Stories stammen. Wer die Stories aufzählt, beschreibt seine Absicht; der
   Diff beschreibt, was zur Prüfung ansteht. Beide fallen auseinander, sobald
   an einem Tag auch außerhalb des Sprints gearbeitet wird — und das ist der
   Normalfall (Sprint 6, K4).
   UI-Stories zusätzlich: UI-Review durch
   `denkzettel-ux` ohne offene `fail`-Befunde — welche Stories UI-Stories
   sind, legt der PO beim Planning fest (Kundenentscheidung 31.07.2026).
   **Der UI-Review ist ohne Bild nicht geführt**: Der Entwickler legt je
   UI-Story einen Screenshot pro Wireframe-Zustand bei (Normalfall,
   Leerzustand, Meldungszustand), `denkzettel-ux` erzeugt zusätzlich eigene
   Bilder aus dem Sprint-Stand und prüft sie gegen den Wireframe. Ein
   UI-Review ohne eigene Bildprüfung zählt für diesen Punkt nicht.
   **Was ein offscreen erzeugtes Bild belegt, und was nicht** (B21): Es belegt
   Geometrie, Textsatz und Farbrollen — alles, was unser Code in unsere Palette
   zeichnet. Es belegt **nicht**, was Desktop-Theme oder Compositor beisteuern:
   Hülle, Rundung, Kontur, Schatten, Dekoration, Durchsichtigkeit. *Gemessen:*
   `tinted()` füllt eine QPixmap deckend, woraufhin Qt offscreen ein Format ohne
   Alphakanal wählt und die Kontur auf dem Eckbogen verschwindet; derselbe
   Binärcode unter Wayland liefert ein Format mit Alphakanal und den
   vollständigen Bogen
   (`docs/scrum/reviews/2026-08-04-abnahme-befunde/messungen/b1-huellenring-offscreen.txt`
   gegen `-live.txt`).
   **Stand dieses Belegs: `tinted()` ist mit #83 gefallen (05.08.2026).** Die
   Messung bleibt gültig — sie hat einen Mechanismus gezeigt, nicht eine
   Funktion —, und sie hat seither drei jüngere Geschwister aus demselben
   Sprint: Die Mutationsproben 10, 11 und 12 von #83 bleiben **offscreen grün**
   und fallen erst in der angemeldeten Sitzung (`docs/scrum/reviews/sprint-07-s83-native-huelle/bericht.md`
   §6). Der zweite Fall endet dort mit Rückgabe 139, Signal 11 — offscreen
   kehrt derselbe Aufruf zurück. **Wer den Beleg für überholt hält, weil die
   Funktion fort ist, verwechselt das Beispiel mit der Aussage.**
   **Macht ein Akzeptanzkriterium eine Aussage über eine dieser Größen, gehört
   ein Bild aus der angemeldeten Sitzung zum Beleg**; ohne es ist DoD 3 für
   diese Story nicht geführt. Die Bildläufer bleiben offscreen — das
   Sitzungsbild tritt daneben, nicht an ihre Stelle. Der Prüfweg dafür liegt
   fertig vor (`docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/echtelage.cpp`);
   er nimmt allein das Fenster auf, nicht den Bildschirm.
   **Und jedes Bild, das als Beleg dient, entsteht bei der Skalierung, die der
   Kunde fährt** (`QT_SCALE_FACTOR`). Die Zahl steht im Prüfbericht, nicht in
   dieser Regel — am 04.08.2026 gemessen 1,6 —, sonst altert sie hier still.
   README-Bilder sind hiervon ausgenommen: Sie belegen nichts, sie sollen
   lesbar sein.
   **Wer die Belegform in ein Akzeptanzkriterium schreibt, entscheidet damit
   darüber.** AK 7 von #55 hat „Rundung und Kontur" der offscreen-Seite
   zugeordnet und nur den Schatten der laufenden Sitzung — die Teilung war da,
   eine Eigenschaft zu weit. Der Läufer hat getan, was dort stand.
4. SPEC.md/KONZEPT.md nachgezogen, falls die Umsetzung eine Festlegung
   ändert **oder eine Bedingung entdeckt, ohne die eine Festlegung nicht
   gilt** (Bauart: fehlende Build-Abhängigkeit, Installations-Präfix —
   Retro Sprint 2, B9); mit Begründung im Issue.
5. Commit(s) auf `main` bzw. Feature-Branch gemäß Kundenvorgabe; Issue
   geschlossen mit Verweis auf Commit.
6. Journal-Eintrag der Session gemäß globaler Protokollpflicht.

Zur Sprint-Ende-Prüfung des Scrum Masters gehört der **Doku-Abgleich**:
Beschreiben README, `docs/`, **`CLAUDE.md` und die Kommentarköpfe von
`.github/workflows/ci.yml` und den `CMakeLists.txt`** den gelieferten Stand?
*Grund für den Zusatz (B17):* In Sprint 6 standen alle drei falschen Aussagen
über die Bildläufer genau dort, und die schwerste in `CLAUDE.md` — außerhalb
des bis dahin genannten Umfangs (M4). Gefunden hat der Scrum Master sie
trotzdem; die Regel soll ihm das nicht als Kür überlassen. Abweichungen meldet er
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
   **Installieren genügt nicht** (B16): Ein laufender Dienst hält nach
   `cmake --install` die **gelöschte** alte Binärdatei weiter und zeigt das an
   nichts. Vor der Prüfung wird deshalb belegt, dass der laufende Prozess der
   installierte ist — `readlink /proc/$(pgrep -x denkzetteld)/exe` muss auf
   `/usr/bin/denkzetteld` zeigen und **darf nicht** auf `(deleted)` enden. Ohne
   diesen Beleg prüft die Abnahme den Stand des vorigen Sprints
   (Sprint 6, §22.1).
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
   **Prüfsummen der Bildbelege (04.08.2026).**
   `bash docs/scrum/bildbelege-pruefen.sh <Belegordner dieses Sprints>` meldet
   Dateien mit verschiedenen Namen und identischen Bytes (Rückgabe 1 = Fund,
   0 = kein Fund). **Verglichen wird nur innerhalb eines Ordners** — dass
   dasselbe Bild in der Dev-Übergabe und im UX-Bericht liegt, ist normal;
   ordnerübergreifend wären es 44 statt 23 Gruppen, und eine Wache, die immer
   anschlägt, ist keine. **Das Skript findet, der Mensch urteilt:** Ein Zustand,
   der nach einer Rückkehr wieder derselbe ist, darf zweimal gleich aussehen;
   ein Mangel ist es, wo zwei **verschiedene** Zustände denselben Beleg tragen.
   Wer die Rückgabe 1 ungeprüft als Mangel bucht, macht aus der Wache eine
   Abhakübung. *Beleg:* Den Fall `bibliothek-installiert.png` =
   `schema-dunkel-installiert.png` fand in Sprint 5 ein Mensch von Hand; der
   erste Lauf fand ihn und 22 weitere
   (`docs/scrum/reviews/2026-08-04-bildbelege-lauf.txt`).
   **Stop-Bedingung:** Findet die Prüfung in drei aufeinanderfolgenden Sprints
   keine neue Gruppe, wird sie zur Stichprobe herabgestuft.
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
    **Die Aussetzung ist am 05.08.2026 beendet: #61 ist geliefert**, und
    `denkzetteld --version` antwortet mit der Nummer aus `CMakeLists.txt`. Ab
    der nächsten Kundenabnahme wird erhöht und getaggt; der Vollzugsvermerk
    führt Punkt 10 nicht mehr als *ausgesetzt*. Die Abnahmen davor bekommen
    weiterhin **keine** Version rückwirkend — die Regel wirkt ab #61, nicht
    davor.
11. Der Scrum Master vermerkt den **Vollzug von Takt 2** im Sprint-Protokoll.
    **Jeder Verwalter-Auftrag endet mit einer Datei (04.08.2026).** Der PO
    schreibt den Ablageort in den Auftrag — `docs/scrum/reviews/sprint-NN-verwalter.md` —,
    und der Scrum Master prüft ihn mit `git ls-files`. **Fehlt die Datei, gilt
    der Auftrag als nicht ausgeführt**, gleichgültig ob die Arbeit getan ist.
    *Grund:* Die Berichtspflicht steht seit `a484d49` in
    `.claude/agents/denkzettel-verwalter.md` — dem bestmöglichen Ort, denn eine
    Agentendatei wird garantiert gelesen — und ist trotzdem **zweimal**
    folgenlos geblieben (Sprint 4 §17.6, Sprint 5 §V3). Es fehlt nicht die
    Regel, es fehlt ihre Wirkung; sie wird deshalb an eine Existenzprüfung
    gebunden statt ein drittes Mal ermahnt. Beide Male war die Arbeit richtig
    und vom PO nachgemessen — der Schaden entsteht an dem Tag, an dem das
    niemand tut, weil es zweimal gutgegangen ist.

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
- **budget spent**: Umfangsgrenze (2–4 Stories und die Klassenregel) und je
  Story max. 2 Anläufe.
- **stalled**: gleicher Fehler zweimal ohne neue Evidenz → Stopp, Impediment
  an PO/Kunden — nicht weiterprobieren.
- **needs a human**: Scope-Änderungen, Zielkonflikte, Kosten — Kundensache.
- **Progress-Log**: jedes Sprint-Protokoll endet mit done/next.
