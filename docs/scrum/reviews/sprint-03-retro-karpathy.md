# karpathy-Review — Prozessänderung der Sprint-3-Retrospektive (`5cefea0`)

**Datum:** 02.08.2026 · **Reviewer:** `karpathy-reviewer` (Fable, frischer Kontext)
**Auftrag:** sprint-03.md, 16.10, Auftrag 3 (Entwurf des Scrum Masters, beauftragt vom PO)
**Prüfstand:** `main` @ `5cefea0` · **Prüfgegenstand:** `git show 5cefea0` vollständig gelesen;
dazu sprint-03.md §12, §13, §14, §15, §16, `PROZESS.md` (ganze Datei), `CLAUDE.md`,
`docs/scrum/retro/sprint-03/stellungnahme-dev.md`, `docs/scrum/reviews/sprint-03-karpathy.md`.
**Eigene Messungen dieser Sitzung:** `git worktree list` (nur Hauptbaum), `git branch -a`
(nur `main`), `git rev-list --left-right --count origin/main...main` → `0 2`,
`git ls-files docs/scrum/retro/sprint-03/` (Stellungnahme versioniert),
`git status --porcelain` (leer), `README.md:7` (Verfahrensangabe entfernt),
Existenz und Betreff von `4746358` und `7787339` (tautologische Zusicherung: eingeführt/entfernt).

**Gesamt-Verdikt: pass in allen fünf Prüfpunkten, kein `fail`.** Sieben Hinweise, keiner blockiert.

---

## Prüfpunkt 1 — Prinzip 2 (Simplicity): Rückführbarkeit der neun Abschlusspunkte

**Verdikt: pass.** Jeder der neun Punkte (`PROZESS.md:151–191`) ist auf einen Mangel aus
13.12 oder eine in 16.2 gemessene offene Stelle rückführbar. Kein Streichkandidat.

| Punkt | Rückführung | Befund |
|---|---|---|
| 1 (Endstand installiert, Hauptweg je Story) | **M1 (schwer)**, 13.3; Ersatzpflicht aus 16.1.1 | pass |
| 2 (Bericht je Prüflauf, vor der DoD-Prüfung) | **M3, M4**, 13.4; dazu 16.1.2: erster karpathy-Lauf ganz ohne Bericht, DoD 3 gegen eine Meldung geprüft | pass |
| 3 (DoD 1–4, Doku-Abgleich inkl. README-Statuszeile) | **M8**, 13.8; Fehlbuchung von M2/M5 zum Prüfzeitpunkt (Begründung der zwei Takte, `PROZESS.md:185–188`); 16.2: Statuszeile am 02.08. erneut falsch | pass |
| 4 (Mängelliste an den PO) | 13.12 (bestehende, funktionierende Praxis) + 16.2: „Melden ohne einen Ort … endet als Ablage" — der Dev-Befund zu Zweigen/Worktrees lag seit dem Morgen vor und fand keinen Adressaten | pass, siehe Hinweis H1 |
| 5 (Issues + Milestone, DoD 5) | **M2, M5**, 13.3/13.6 | pass |
| 6 (Journal, DoD 6) | **M7**, 13.7 | pass |
| 7 (Push) | 16.2 („Push offen", gemessen `0 1`); Vorgeschichte 13.6 (33 Commits, offen seit Sprint 2) | pass |
| 8 (Zweige und Worktrees) | 16.2 (8 Zweige, 296 MB, gemessen); Dev-Stellungnahme Abschnitt 1 | pass |
| 9 (Vollzugsvermerk) | 16.2 („Vollzugsvermerk existiert nicht") | pass |

**H1 (Hinweis, Punkt 4):** Der einzige Punkt, der keinen Mangel heilt, sondern bestehende
Praxis kodifiziert (13.12 fand ohne Listenpflicht statt). Er ist trotzdem kein Streichkandidat:
Er ist der Ausgabekanal der Punkte 1–3, und die 16.2-Beobachtung (Befund ohne Adressat endet
als Ablage) ist eine gemessene offene Stelle, die genau einen benannten Empfänger verlangt.
Die Rückführung ist die schwächste der neun, aber vorhanden.

**Zur Bürokratie-Frage:** Neun Haken je Sprint gegen acht von neun Mängeln, die genau an
diesen Haken hingen, ist ein günstiges Verhältnis. Die Liste erfindet keinen einzigen neuen
Arbeitsschritt — jeder Punkt beschreibt Arbeit, die ohnehin geschuldet war und nachweislich
liegen blieb. Die Ausklammerung von Version/Tag/Changelog (`PROZESS.md:190–191`) ist
ausdrücklich als Kundenentscheidung markiert — vorbildlich gegen spekulative Erweiterung.

**H2 (Hinweis, Dopplung):** Der `CLAUDE.md`-Absatz zu B12 wiederholt die konkreten Grenzen
(„2–4 Stories, ~13 SP"). Ändert eine spätere Retro die Grenzen in `PROZESS.md`, driftet
`CLAUDE.md`. Der B11-Absatz macht es besser: Er verweist auf die Liste, statt sie zu
kopieren. Kein Fehler heute — eine Wartungsstelle für morgen.

---

## Prüfpunkt 2 — Prüfbarkeit: Befehl/Artefakt oder Ermessen?

**Verdikt: pass.** Sieben von neun Punkten sind unmittelbar gegen Befehl oder Artefakt
entscheidbar; Punkt 8 benennt sein Kriterium sogar ausdrücklich als Exit-Code
(`git merge-base --is-ancestor`, `PROZESS.md:180–182`) — das ist die richtige Antwort auf
die Fehlerbauart des Sprint-2-Befunds zu DoD 4. Zwei Punkte tragen einen Ermessensrest:

**H3 (Hinweis, Punkt 1):** „der Hauptweg jeder Story ist daran ausgeführt" — was der
Hauptweg ist und welcher Beleg die Ausführung zeigt, ist nicht benannt. DoD 2 verlangt für
die Dev-Selbstprüfung ausdrücklich „Terminalausgabe, Journalauszug oder Bild"
(`PROZESS.md:112–113`); für die Sprint-Ende-Ausführung fehlt die analoge Belegform. Der
Ermessensrest ist geerbt, nicht neu — aber die Abhak-Pflicht „mit Beleg" (`PROZESS.md:155`)
bleibt ohne Formangabe weich. Vorschlag: die drei Belegformen aus DoD 2 per Verweis
übernehmen.

**H4 (Hinweis, Punkt 2):** „Jeder Prüflauf hat einen Bericht" ist nur entscheidbar, wenn
die Menge der Prüfläufe aufzählbar ist — ein Lauf, der keine Spur hinterlässt, ist für den
Prüfer unsichtbar. Die Retro hat den praktikablen Detektor selbst vorgeführt (16.1.2,
Fund 1: Commit-Botschaft `e18630c` nennt „drei Befunde des Prinzipien-Reviews", `grep -rl
karpathy docs/scrum/reviews/` liefert keinen passenden Bericht), ihn aber nicht in den
Punkt geschrieben. Vorschlag: den Abgleich „Commit-Botschaften, die Befunde nennen, gegen
abgelegte Berichte" als Prüfweg benennen.

Die übrigen Punkte: 3 (README-Statuszeile: das konkrete Verbot der Verfahrensangabe ist
grep-bar; der Doku-Abgleich trägt denselben Ermessensrest wie B10 bisher — nicht neu),
5 (`gh issue list`/Milestone-Status), 6 (Journal gegen letzten Commit-Zeitstempel),
7 (`git rev-list --left-right --count`), 9 (Vermerk existiert oder nicht): entscheidbar.

---

## Prüfpunkt 3 — Prinzip 1 (Think): Trägt die Erklärung „kein Ort im Ablauf" (16.1.3)?

**Verdikt: pass.** Am Verlauf geprüft, nicht an der Plausibilität. Der im Auftrag benannte
Gegen-Fall — B1/B2/B3 hätten denselben fehlenden Ort und hielten trotzdem — liegt **nicht**
vor:

- **B1** hängt am Übergabebericht: Der Nachweis der Selbst-Sichtprüfung ist Pflichtteil
  des Berichts (DoD 2, `PROZESS.md:110–113`). Er fand in Sprint 3 statt (13.3: die
  Prüfungen „liefen gegen die Build-Verzeichnisse der Worktrees" — dass der Prüfling falsch
  war, ist B4, nicht B1; Beleg im Protokoll sauber getrennt, 16.1-Tabelle).
- **B2** hängt am Test selbst: DoD 1 prüft ihn je Story; die Retro hat die Zusicherungen
  mit Dateizeilen nachgezählt (16.1-Tabelle: `librarytest.cpp:1795`, `:1853`, `:1015`).
- **B3** hängt am Review-Gate in DoD 3: Es lieferte den einzigen `fail` des Sprints (13.4).

Alle drei sind an Gegenstände gekoppelt, die die DoD-Prüfung je Story ohnehin anfasst —
sie *haben* einen Ort. B4 und B7 verlangten dagegen Handlungen, deren Zeitpunkt kein
Artefakt benannte; B10 hatte einen Zeitpunkt, aber den falschen. Die Erklärung
diskriminiert also korrekt zwischen den gehaltenen und den gerissenen Beschlüssen — sie
ist keine Begründung, die alles erklärt hätte.

**H5 (Hinweis, keine Abwertung):** Für B7 ist „kein Ort" nur die halbe Ursache. Die zweite
Komponente steht in 16.1.2, Punkt 3, ausdrücklich im Protokoll: Die DoD-Prüfung hat DoD 3
passieren lassen, obwohl die Verdikte zum Prüfzeitpunkt in keinem Artefakt standen — „Der
Scrum Master hat gegen eine Meldung geprüft." Das ist ein Strenge-Problem der Prüfung,
kein Ort-Problem der Regel. Die Verdichtung auf „kein Ort" in 16.1.3 stützt aber keinen
falschen Schluss, denn der Beschluss heilt beide Komponenten in einem: B11 Takt 1 Punkt 2
erzwingt den Bericht **vor** der DoD-Prüfung — damit gibt es sowohl den Zeitpunkt als auch
das Artefakt, gegen das statt der Meldung geprüft wird.

Die Belege der Modellrevision (B15) halten der Nachmessung stand: `4746358` („Bibliothek:
Notizliste als Posteingang gliedern (#46)") und `7787339` („Tests: tautologische
Zusicherung entfernt … (#46)") existieren mit genau diesen Inhalten.

---

## Prüfpunkt 4 — Prinzip 3 (Surgical): Ist der Diff chirurgisch?

**Verdikt: pass.** Am Diff geprüft, Hunk für Hunk:

- `PROZESS.md`: Stand-Datum (Zeile 3, Pflege der geänderten Datei); Modellzuordnung — B15
  ersetzt exakt den einen Satz „Revision dieser Zuordnung ist Retro-Thema"; Belege — B14
  angehängt, Bestand unangetastet; Ablauf-Bullet — nur der Querverweis „siehe
  ‚Parallelarbeit'" eingefügt (Folge von B13, verhindert eine verweislose neue Sektion; der
  Zeilenumbruch der Folgezeile ist mechanische Konsequenz); zwei neue Bullets (B12, B13);
  neuer Abschnitt (B11). **Kein angrenzender Text umformuliert, DoD-Liste unberührt.**
- `README.md`: ausschließlich die Parenthese „(Sprint 3 in der Kundenabnahme)" entfernt —
  wortgenau der Auftrag 2.
- `CLAUDE.md`: drei Absätze, wortgleich mit dem in 16.10 Auftrag 1 vorgegebenen Wortlaut,
  an der benannten Stelle (Abschnitt „Die Regeln, die am häufigsten übergangen werden").
- `sprint-03.md`: reiner Anhang (`@@ -1192,3 +1192,622` — ein einziger Hunk, nichts
  Bestehendes verändert).
- `stellungnahme-dev.md`: Neuaufnahme des untracked Belegs (Auftrag 2, B7).

Jede geänderte Zeile lässt sich auf einen der fünf Beschlüsse oder auf Auftrag 1/2
zurückführen. Keine Orphans: Der Querverweis zeigt auf eine existierende Sektion, die
Beschluss-Nummern folgen der bestehenden Konvention (B1–B10 aus sprint-02.md 9.5).

**H6 (Hinweis, kosmetisch):** „von acht Bildern der Sprint-3-Abnahme überlebten sieben
Minuten nur eines" — Subjekt „eines" (Singular) gegen „überlebten" (Plural); richtig wäre
„überlebte". Der Fehler steht dreifach (`PROZESS.md:61`, `CLAUDE.md`, Ursprung in 16.10,
Auftrag 1) — der PO hat den vorgegebenen Wortlaut auftragsgemäß wortgleich übernommen,
der Fehler liegt in der Vorlage des Scrum Masters.

---

## Prüfpunkt 5 — Widerspruchsfreiheit: Sprint-Abschluss gegen DoD 2/5/6 und Doku-Abgleich

**Verdikt: pass.** Im Einzelnen:

- **DoD 2 ↔ Takt 1 Punkt 1:** deckungsgleich — Punkt 1 wiederholt die Pflicht aus der
  DoD-2-Präzisierung („Am Sprint-Ende wird der Endstand einmal installiert …",
  `PROZESS.md:121–124`) wörtlich sinngleich. Konsistent; die Dopplung ist eine bewusste
  Verankerung am Ablauf-Ort, kein Widerspruch.
- **DoD 5/DoD 6 ↔ Takt 2:** kein Widerspruch in der Sache — die Taktung löst genau die
  Fehlbuchung auf, die in Sprint 3 geschah (M2/M5 als Mängel geführt, obwohl vor der
  Abnahme unerfüllbar; `PROZESS.md:185–188` benennt das ausdrücklich). Siehe aber H7.
- **Doku-Abgleich (unter der DoD-Liste) ↔ Takt 1 Punkt 3:** konsistent, und die Auflösung
  ist elegant: Statt den Abgleich hinter die Abnahme zu schieben (wo er für alles andere zu
  spät käme), verbietet Punkt 3 die Verfahrensangabe in der Statuszeile — dann kann die
  Abnahme sie nicht mehr falsch machen. Der Abschlussabsatz begründet das selbst
  (`PROZESS.md:185–188`).
- **Zwei Takte als Konstruktion:** stimmig; die Reihenfolge (erst prüfen, dann abnehmen,
  dann abschließen) entspricht dem tatsächlichen Sprint-3-Verlauf inklusive seiner Fehler.

**H7 (Hinweis):** Die DoD-Liste selbst (`PROZESS.md:104–142`) trägt keinen Verweis auf die
Taktung. Wer nur die DoD-Sektion liest und „die DoD-Prüfung" über alle sechs Punkte führt,
bucht DoD 5/6 vor der Abnahme erneut als Mängel — exakt die Fehlbuchung, die B11 abstellen
soll. Beide Abschnitte stehen in derselben Datei, die der Scrum Master von Amts wegen ganz
liest; das Risiko ist deshalb klein, aber die Regel „die nächste Sitzung liest den Ort von
selbst" (CLAUDE.md, Retrospektiven) spräche für einen Ein-Zeilen-Verweis nach DoD 6 oder
unter der Liste („Prüfzeitpunkte: siehe Sprint-Abschluss, Takt 1/Takt 2"). Kein Widerspruch,
eine Lesepfad-Lücke.

*Nebenbefund außerhalb der fünf Prüfpunkte, der Vollständigkeit halber:* Takt 2 Punkt 7
(„`main` gepusht") und die offene Kundenentscheidung zur Push-Kadenz (16.9, Punkt 6)
kollidieren nicht — Punkt 7 setzt ein Minimum am Sprintschluss, die Kadenzfrage betrifft
die laufende Arbeit —, aber die Wechselwirkung ist nirgends benannt. Und: Der Commit-Titel
„Abschluss-Rückstand geräumt" überzeichnet leicht; heute gemessen stehen Punkt 7 (Push,
`0 2`) und Punkt 9 (Vollzugsvermerk) noch offen. Beides ist als Folge-Schritt benannt
(16.11 next; Auftrag 2, letzter Satz), der Push ist nach diesem Review ohnehin erst fällig
— kein Mangel, eine Präzisierung für das Protokoll.

---

## Was gut ist — beibehalten

- **Jeder Beschluss trägt seine Begründung mit Fundstelle im Regeltext selbst** (die
  *Grund:*-Absätze bei B12/B13, der Vorspann des Sprint-Abschlusses). Wer die Regel in
  einem Jahr liest, weiß, warum es sie gibt und wann sie hinfällig würde.
- **Punkt 8 ist die Blaupause für prüfbare Abschlusspunkte:** Kriterium als Exit-Code,
  Reihenfolge der Befehle benannt. H3/H4 sind daran zu messen.
- **Die Ausklammerung von Version/Tag/Changelog ist ausdrücklich als Absicht markiert**
  („damit die Auslassung als Absicht erkennbar bleibt und nicht als Lücke nachgetragen
  wird") — das schützt die Liste vor stillem Wachstum.
- **Die Abschlussprüfung 16.8 weist die eigene Halbverankerung offen aus** („Drei von fünf
  Beschlüssen sind heute nur zur Hälfte verankert") statt sie wegzuschreiben — und dieser
  Commit führt Auftrag 1 tatsächlich aus.
- **B13 beantwortet die Planning-Frage zur Agentendatei mit einem begründeten Nein**
  (der Entwickler legt keinen Worktree an, er wird hineingesetzt) — Adressatenprüfung
  statt Reflex-Verankerung an allen Orten.

## Konkrete Fix-Vorschläge (Empfehlungen, keine Pflicht — Entscheidung beim PO)

1. **H3:** In Takt 1 Punkt 1 die Belegform per Verweis auf DoD 2 benennen
   („Nachweis wie in DoD 2: Terminalausgabe, Journalauszug oder Bild"). `PROZESS.md:161–162`.
2. **H4:** In Takt 1 Punkt 2 den Prüfweg für die Vollzähligkeit benennen
   (Commit-Botschaften mit Befund-Nennung gegen abgelegte Berichte halten). `PROZESS.md:163–167`.
3. **H7:** Ein-Zeilen-Verweis unter der DoD-Liste auf die Taktung der Prüfzeitpunkte.
   `PROZESS.md:142` ff.
4. **H6:** „überlebten" → „überlebte" an drei Stellen (`PROZESS.md:61`, `CLAUDE.md`,
   sprint-03.md 16.10) — Routine-Fix, kein erneutes Review nötig.
5. **H2:** Bei der nächsten Änderung der Sprint-Grenzen daran denken, dass `CLAUDE.md`
   die Zahlen dupliziert (kein Handlungsbedarf heute).
