# Karpathy-Nachprüfung: Heilungen zu den Befunden vom 02.08.2026

**Auftrag:** Nachprüfung nur der geheilten Stellen aus dem Review
`2026-08-02-entscheidungen-karpathy.md` (Gesamtverdikt dort: fail — 1a, 4a),
beauftragt vom PO, nach dem Muster der UI-Nachprüfläufe.
**Prüfgegenstand:** aktueller ungecommitteter Stand auf `main` @ `5af0beb`
(`git diff` + untracked: `CLAUDE.md`, `docs/scrum/PROZESS.md`,
`.claude/agents/denkzettel-verwalter.md`, `CHANGELOG.md`).
**Prüfer:** karpathy-reviewer (Fable, fresh context). **Datum:** 2026-08-02.

**Gesamtverdikt: pass** — alle fünf geheilten Stellen tragen. Die beiden
fail-Befunde des Vorlaufs sind behoben. Vier neue Hinweise, alle durch die
Heilung entstanden, keiner mit einem Weg zu einem falschen Ergebnis.

Jede Behauptung unten ist am Stand gemessen (Fundstelle oder Befehl benannt),
nicht an der Heilungsmeldung.

---

## Stelle 1 — 1a: Kollision AK-Haken (war fail) → **pass**

Die Kollision ist weg. `PROZESS.md:234–240` entflechtet Punkt 5 im Wortlaut:
Die AK-Haken sind als Abnahmeentscheidung des PO benannt („nicht umkehrbar
und werden zur Beweislage"), das Schließen mit Kommentar und Commit-Verweis
als delegierbarer Vollzug. `denkzettel-verwalter.md:25–31` regelt den
Konfliktfall mit genau diesem Beispiel und gibt das Verfahren vor (erlaubte
Teilschritte ausführen, verbotenen melden, nicht ganz abbrechen). Das
Haken-Verbot steht jetzt dreifach: description (Z. 9–11), Konfliktregel mit
Beispiel (Z. 27–29), Verbotsliste (Z. 61) — dazu die Rollenzeile
`PROZESS.md:16`. Es gibt keinen Wortlaut mehr, nach dem der Verwalter Haken
setzen müsste oder dürfte; der verbleibende Satzanfang von Punkt 5 („Issues
mit AK-Haken … geschlossen") wird im selben Punkt aufgelöst.

**Neuer Hinweis N2 (durch die Entflechtung entstanden):** Punkt 5 erklärt
auch das **Milestone-Schließen** für delegierbar (`PROZESS.md:238–240`).
Weder die Rollenzeile (`PROZESS.md:16`: Milestone-Stände nur „erheben und
berichten") noch die Agentendatei (Z. 55–57: „Zustände erheben und
berichten") führen es als Tu-Punkt. Es ist nicht verboten und mechanisch
unkritisch, aber ungedeckt — ein Halbsatz an einer der drei Stellen schließt
die Lücke.

## Stelle 2 — 1b: Stopp-Klausel (war Hinweis) → **pass**

`denkzettel-verwalter.md:19–24`: Der Auftrag muss Fundstelle, konkrete Liste
und vorgegebene Texte nennen; „Fehlt eines davon, stoppst du diesen Punkt
und meldest es". Der Grenzfall wie gestellt — „Schließe die Issues des
Milestones X" ohne weiteres — stoppt sicher, sogar doppelt: Fundstelle fehlt,
Kommentartexte fehlen.

**Neuer Hinweis N3 (Restweg der erweiterten Klausel):** Die Klammer „(welche
Issues, welche Zweige, welcher Milestone)" (Z. 20–21) liest sich als drei
gleichwertige Formen der Gegenstandsangabe. Sind Fundstelle und Texte
beigegeben und nur die Issue-Aufzählung durch „alle Issues des Milestones X"
ersetzt, deckt der Text die mechanische Auflösung (`gh issue list
--milestone X`) — ein Stopp ist dann nicht erzwungen. Das Risiko ist klein
(Auflösung eines protokollierten Bestands; der teuerste Fehlweg, selbst
formulierte Texte, stoppt immer), aber wer die Aufzählung im Auftrag
erzwingen will, muss die Klammer je Aufgabenart zuordnen statt als
Alternativen anbieten.

## Stelle 3 — 4a: Changelog-Filter (war fail) → **pass**

Die Regel widerspricht keinem Präzedenzfall mehr. `PROZESS.md:253–258` macht
die Nutzersicht zum Maßstab und `typ:tech` zum Anhaltspunkt; die drei
benannten Präzedenzfälle stimmen mit dem jetzigen `CHANGELOG.md` und den
Labels überein (nachgemessen per `gh issue view 1|3|6|9`):

| Issue | Label | Im Changelog | Regeldeckung |
|---|---|---|---|
| #6 Autostart | `typ:tech` | ja (`CHANGELOG.md:34–35`) | „Anhaltspunkt, kein Filter" + sichtbarer Dienst |
| #1 Wayland-Spike | `typ:tech` | nein (kommt nicht vor) | rein interntechnisch |
| #9 Migrationstest | `typ:tech` | nur Schemazeile (`CHANGELOG.md:39–41`) | Schema-Pflichteintrag |
| #3 SQLite-Store | `typ:story` | ja, neu (`CHANGELOG.md:22–23`) | Nutzersicht: lokale Ablage, kein Cloud-Zwang |

Die Auslassung von #3 — im Vorlauf „nicht gedeckt" — ist durch den neuen
Eintrag geheilt, nicht durch eine Ausnahmeklausel; das ist die sauberere der
beiden Richtungen.

**Neuer Hinweis N1 (durch die Heilung entstanden): Rohliste vs. Entwurf.**
`PROZESS.md:262–264` erlaubt, dem Verwalter nur die **Rohliste** zu ziehen
(„Auswahl und Text verantwortet der PO — … die Aufnahme eines Eintrags ist
ein Urteil"). Rollenzeile (`PROZESS.md:16`) und Agentendatei (Z. 48–54)
beauftragen dagegen einen **Entwurf**: vorformulierte Einträge mit selbst
angewandtem Nutzersicht-Maßstab. Dazu die Spannung zur erweiterten
Stopp-Klausel im selben Dokument: „einen Text selbst zu formulieren wäre ein
Urteil" (Z. 23–24) — der Entwurfsauftrag verlangt genau das; nur die Klammer
„(Kommentare, Vermerke)" (Z. 22) hält die Klausel vom Entwurf fern. Kein
fail, weil kein falsches Ergebnis droht (der Verwalter fügt nichts selbst in
`CHANGELOG.md` ein, Z. 53–54; die Endauswahl bleibt beim PO) — aber ein
wortlauttreuer Haiku hat für die Entwurfsaufgabe zwei Normen. **Fix:** eine
Reichweite festlegen — entweder Punkt 9 auf „Rohliste oder Entwurf, je nach
Auftrag" erweitern oder Rollenzeile/Agentendatei auf die Rohliste ziehen —
und die Stopp-Klausel ausdrücklich auf Vollzugsaufträge begrenzen.

## Stelle 4 — 4c/3b/1c: Punkt 8, Punkt 10, Rollenzeile → **pass**

- **Punkt 8** (`PROZESS.md:247–250`): Remote-Räumung „ab dem PR-Probelauf"
  mit `git push origin --delete`, begründet über die Öffentlichkeit des
  Repos — schließt exakt die Lücke aus 4c.2. Kein Widerspruch zur
  Dev-Push-Regel (die Zweige pusht und löscht der Takt-2-Ausführende, nicht
  der Dev) und keiner zum befristeten Probelauf (endet er, entstehen keine
  origin-Zweige mehr, die Klausel läuft leer). **Neuer Hinweis N4:** Die
  Räum-Anleitung der Agentendatei (Z. 44–47) kennt den origin-Schritt nicht
  — wer nur „Was du tust" liest, räumt lokal und lässt origin stehen.
  Gedeckt über die Arbeitsliste (Z. 36 verweist auf den Sprint-Abschluss),
  aber ein Halbsatz in Z. 44–47 nähme die Lücke.
- **Punkt 10** (`PROZESS.md:274–281`): „ausgesetzt"-Führung im
  Vollzugsvermerk, Sammlung unter `[Unveröffentlicht]` (Abschnitt existiert,
  `CHANGELOG.md:10`), keine rückwirkenden Versionen, `sprint-03-abschluss`
  als Übergangsform und „kein drittes Tag-Schema" benannt — löst 3b und
  nebenbei 3c. Der `[0.1.0]`-Block kollidiert nicht mit „keine Version
  rückwirkend": Er dokumentiert die seit Sprint 1 in `CMakeLists.txt`
  stehende Zahl, und sein Siegel ist als Übergangsform erklärt.
- **Rollenzeile** (`PROZESS.md:16`): auf „erheben und berichten" gezogen,
  deckungsgleich mit Z. 55–57 der Agentendatei — 1c ist geschlossen (Rest:
  N2, siehe Stelle 1).

Untereinander und gegen den Rest der Datei sind die drei Änderungen
widerspruchsfrei; die Verweise (Sprint 3, 16.3.3 / 16.9 / 16.13) existieren
im Protokoll (per `grep` geprüft).

## Stelle 5 — tools:-Zeile → **pass**

`denkzettel-verwalter.md:13`: `Read, Glob, Grep, Bash, Write` — exakt der
Fix aus 2a. `Edit` fehlt, damit ist der bequemste Fehlweg (Dateien im Repo
ändern) technisch zu. `Write` ist für die `--body-file`-Dateien (Z. 42)
nötig und durch die Verbotsliste (Z. 63–64) begrenzt; einen der Rolle
widersprechenden Weg, den der Text nicht abfängt, öffnet es nicht. `Bash`
bleibt naturgemäß offen (`gh issue edit` könnte Haken setzen) — das fängt
der Text dreifach (Z. 9–11, 27–29, 61); dieselbe ehrliche Grenze wie im
Vorlauf, durch die Heilung nicht verschlechtert. Das neue `git push origin
--delete` aus Punkt 8 ist mit der Verbotsliste (Z. 65) vereinbar — gewollt.

---

## Neue Hinweise (alle durch die Heilung entstanden, keiner fail)

1. **N1 — Rohliste vs. Entwurf:** `PROZESS.md:262–264` gegen
   `PROZESS.md:16` und `denkzettel-verwalter.md:48–54`, plus Spannung zur
   Stopp-Klausel (Z. 22–24). Eine Reichweite festlegen.
2. **N2 — Milestone-Schließen:** in Punkt 5 delegierbar erklärt, aber in
   keiner Tu-Liste gedeckt (`PROZESS.md:238–240` gegen `:16` und
   `denkzettel-verwalter.md:55–57`).
3. **N3 — Milestone-Referenz als Listen-Ersatz:** Klammer in
   `denkzettel-verwalter.md:20–21` erzwingt keine Issue-Aufzählung, wenn
   Fundstelle und Texte vorliegen.
4. **N4 — origin-Schritt fehlt in der Räum-Anleitung:**
   `denkzettel-verwalter.md:44–47` gegen `PROZESS.md:247–248`.

## Was gut ist (beibehalten)

- **Die Entflechtung in Punkt 5 begründet die Grenze, statt sie nur zu
  ziehen** („nicht umkehrbar und werden zur Beweislage") — das macht sie
  auch für ein kleines Modell nachvollziehbar.
- **Die Konfliktregel nennt den konkreten Fall samt Verfahren** — erlaubte
  Teilschritte ausführen, melden, nicht abbrechen. Genau die Bauart, die 1a
  verlangt hat.
- **4a wurde in die richtige Richtung geheilt:** nicht die Regel an den
  Bestand angepasst, sondern der Bestand (#3-Eintrag) an die Nutzersicht —
  und die Präzedenzfälle stehen jetzt als Messpunkte in der Regel selbst.
- **Punkt 10 regelt seinen eigenen Ruhezustand vollständig** (ausgesetzt,
  Sammelort, kein Rückwirken, Übergangsform benannt) — keine offene Lesart
  mehr.
