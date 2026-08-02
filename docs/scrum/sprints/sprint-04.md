# Sprint 4 — Planning-Protokoll

**Datum:** 2026-08-02, 12:24 (Ganymed) · **Moderation:** Scrum Master (Agent
`scrum-master`)
**Teilnehmer:** Scrum Master · Product Owner · Schätzer Dev (unabhängige
Schätzung zu #11, #60, #62, #12 am 02.08.2026) · UI/UX (Agent
`denkzettel-ux`, Planning-Beratung zu #60).
**Status des Sprint-Vorschlags:** vorgelegt, Freigabe durch den Kunden steht aus.

**Grundlagen:** `docs/scrum/PROZESS.md` (Stand nach den Beschlüssen B11–B15 und
den sechs Kundenentscheidungen vom 02.08.2026), `docs/scrum/sprints/sprint-03.md`
(12, 13, 15, 16), `SPEC.md` (9, 10), `KONZEPT.md` (viertes Design-Interview),
`recherche/2026-08-02-fuzzy-suche.md`, GitHub-Issues #11, #12, #57, #59, #60, #62.
**Quellstand der Prüfungen:** `main` @ `a484d49`.

Alle Aussagen über Konflikte, Abhängigkeiten und Risiken sind am Quellcode
geführt, nicht an den Story-Texten; Zeilennummern sind am Stand `a484d49`
nachgezählt. **Der Dev-Schätztext lag als flüchtige Sitzungsdatei vor** — was
das Planning trägt, ist hier übernommen (B7, B14).

## 1. Sprint-Konto (B12) — erste Anwendung

Die Buchführung beginnt mit der ersten Zeile, nicht mit dem ersten Zugang. Wer
13 im Blick hat, sieht die 13 einhalten und übersieht die 5 (Sprint 3, 12.7).

| Buchung | Issues | Story Points | Grenzen (2–4 · ~13) |
|---|---|---|---|
| Kandidatenfeld | 4 | **14** | Story-Grenze berührt, **SP-Grenze gerissen** |
| Vorschlag des Scrum Masters (4.3) | 3 | **11** | beide gehalten |
| *Freigabe-Stand* | *einzutragen nach der Kundenentscheidung* | | |

**Der erste Blick des Kontos ist zugleich sein erster Befund:** Mit
regelkonform konsolidierten Schätzungen tragen die vier Kandidaten **14 SP** —
mehr, als die Sprint-Mechanik zulässt. Das ist keine Formalie, sondern die
Zahl, an der der Schnitt sich entscheidet (4.2). Der PO hatte im Auftrag mit
12 SP gerechnet; die Differenz entsteht ausschließlich an #60 (2.2).

## 2. Schätzstand und Konsolidierung

| Issue | Story | Dev | Zweitschätzung | **Konsolidiert** | Regel |
|---|---|---|---|---|---|
| #11 | S8 Bearbeiten-Ansicht | 5 | Alt-Label `sp:2` | **5** | >1 Stufe → begründete Entscheidung (2.1) |
| #60 | S33 Tray-Menüs | 3 | UX 5 | **5** | ≤1 Stufe → höherer Wert (2.2) |
| #62 | T10 Spike spellfix1 | 3 | **keine** | **(3)** | **Schätzregel nicht erfüllt** (2.3) |
| #12 | T4 Ollama-Modelle | 1 | Alt-Label `sp:1` | **1** | deckungsgleich |

**Keine 13er-Story im Feld** — nichts ist teilungsbedürftig.

### 2.1 #11 — die 2 ist keine Zweitschätzung, sondern eine andere Story

*Beobachtung:* Das Label `sp:2` stammt aus dem Sprint-1-Planning; der
Issue-Text sagt selbst „beim Sprint-3-Planning gegen die neugefassten AK zu
bestätigen". Zwischen der 2 und heute liegen die Gliederung (S5a) und die
Suche (S6) — beides Flächen, mit denen die Bearbeiten-Ansicht wechselwirkt.

*Schlussfolgerung:* Die 2 hat einen anderen Gegenstand geschätzt und ist
**keine unabhängige Zweitmeinung zur heutigen Story**. Formal liegt für #11
damit nur eine Schätzung gegen die geltenden AK vor. Die Konsolidierung folgt
deshalb nicht dem Vergleich zweier Zahlen, sondern der Prüfung der einen:

- Am Code belegt: `buildDetail()` (`librarywindow.cpp:243–267`) trägt heute
  Zeitstempel, Löschen-Schaltfläche und ein **schreibgeschütztes**
  `QTextBrowser` (`:257`). Der Bearbeiten-Zustand des Wireframes 2a fehlt
  vollständig; der Wächterdialog hat drei Auslösepfade.
- Der teuerste Pfad ist der Auswahlwechsel: `currentChanged` läuft heute
  ungebremst in `showNote()` (`:195`, `:438`) — dort muss künftig ein Dialog
  dazwischen.
- Die Store-Seite ist fast geschenkt: `Store::updateNote` existiert
  (`store.cpp:342–360`), die FTS-Triggerfalle ist geschlossen, `storetest`
  deckt den Änderungsfall ab.
- Kalibrierung: S5a und S6 waren je 5 und sind vergleichbar groß.

**Entscheidung: 5 SP.** Die alte 2 wird nicht „übernommen", sie wird ersetzt.

### 2.2 #60 — 5, obwohl die Stopp-Regel das Risiko deckelt

*Beobachtung:* Dev 3, UX 5 — eine Stufe. Der Dev begründet die 3 damit, dass
die Unbekannte (Wayland-Popup-Positionierung) durch die Stopp-Regel gedeckelt
ist; UX begründet die 5 mit der Wayland-Unbekannten **plus** dem Kürzel-Pfad
und nennt eine Bedingung: „mit vorab geklärter SNI-Frage: 3".

*Schlussfolgerung:* Die Regel ist eindeutig — **bei Abweichung um eine Stufe
gilt der höhere Wert**. Sie existiert genau für diesen Fall: Zwei plausible
Begründungen, und die optimistische ist die bequemere. Sprint 3 hat vorgeführt,
was das kostet: #8 wurde mitten im Sprint von 3 auf 5 heraufgesetzt
(Sprint 3, 13.10). **Konsolidiert: 5 SP.**

Die Bedingung von UX ist damit nicht vom Tisch, sie wird zur **Option für den
Schnitt** (4.2 C): Wird die SNI-Frage vor der Freigabe am Panel gemessen, trägt
#60 die 3. Die Arbeit verschwindet dadurch nicht — sie wandert vor die Freigabe.

### 2.3 #62 — die Schätzregel ist nicht erfüllt

`PROZESS.md`, Sprint-Mechanik: **zwei unabhängige Schätzer je Story.** Für #62
liegt eine Schätzung vor (Dev 3). Das ist kein Vorwurf an den Dev, sondern ein
Zustand, der vor dem Ziehen zu heilen ist — ausgewiesen statt stillschweigend
übergangen. Die 3 steht deshalb in Klammern.

## 3. Konfliktanalyse am Code

### 3.1 Was welche Story anfasst

| Story | Dateien | Beleg |
|---|---|---|
| #11 S8 | `src/ui/librarywindow.*` (`buildDetail` `:243`, `showNote` `:438`, `closeEvent` `:324`, `currentChanged` `:195`), ggf. neue Datei im `denkzettelui`-Block (`src/CMakeLists.txt:29–36`), `tests/librarytest.cpp`, ggf. SPEC 9 | AK-Liste #11; `Store::updateNote` existiert bereits (`store.cpp:342`) |
| #60 S33 | `src/shell/trayicon.cpp` (79 Zeilen), `globalshortcuts.cpp`, `shortcutregistration.cpp`, `desktop/org.denkzettel.Denkzettel.desktop`, `tests/shelltest.cpp`, **SPEC 10** (`SPEC.md:417`) | Fundstellenliste in #60 |
| #62 T10 | Build-Verkabelung im `denkzettelstore`-Block (`src/CMakeLists.txt:1–3`), Prototyp, Lint-Ausschluss; **kein Produktivcode in `main`** | Abgrenzung in #62 |
| #12 T4 | **keine Repo-Datei** — `ollama pull`, zwei API-Belege als Issue-Kommentar | AK-Liste #12 |

### 3.2 Zwei disjunkte Codeflächen, eine gemeinsame Datei

- **#11 und #60 berühren einander nicht.** `src/ui/*` gegen `src/shell/*`,
  `librarytest` gegen `shelltest` — kein gemeinsamer Übersetzungs- oder
  Testbereich. Das ist der sauberste Schnitt seit Sprint 2.
- **`SPEC.md` ist die gemeinsame Fläche**: #11 zieht gegebenenfalls SPEC 9 nach
  (Bearbeiten-Ansicht, `SPEC.md:398–405`), #60 verbindlich SPEC 10 (`:417 ff.`).
  Verschiedene Abschnitte, für Git ein gewöhnlicher Merge — aber beide Stränge
  schreiben in dieselbe Datei, und das gehört benannt, bevor es auffällt.
- **`src/CMakeLists.txt`**: #11 gegebenenfalls im `denkzettelui`-Block, #62 im
  `denkzettelstore`-Block. Getrennte Blöcke, wie in Sprint 3 (2.3).
- **#12 hat keine Fläche** und kann neben allem laufen.

### 3.3 Die eigentliche Wechselwirkung von #11 sitzt nicht in einer fremden Datei

Sie sitzt in **`showNote()`** — und zwar gegen die eigene Story: Nach dem
Speichern baut die Liste neu auf; damit ist `previous` entwertet, und
`crossesAGroupBoundary` (`:463`) wird **immer wahr**. Der Kopf wird geholt, die
Liste springt. Das ist exakt die Fläche der offenen Fehler **#57**
(„Klick auf eine sichtbare Notiz anderer Gruppe lässt das Bild springen") und
**#59** („Fensteraktivierung ohne Tageswechsel verliert die Scrollstelle").

**Folge für das Planning:** Wer #11 baut, arbeitet in der Fläche zweier offener
Fehler. Sie mitzuziehen ist eine Priorisierungsfrage und damit PO-Sache — der
Scrum Master meldet nur, dass die Gelegenheit dort liegt und dass ein Dev, der
sie nicht kennt, die Symptome für seine eigenen halten wird.

## 4. Sprint-4-Vorschlag

### 4.1 Sprint-Ziel

> **Notizen lassen sich in der Bibliothek ändern, und das Tray-Menü trennt
> Arbeit von Verwaltung — deutsch beschriftet und mit Symbolen; im Hintergrund
> stehen die KI-Modelle für den nächsten Meilenstein bereit.**

Nachprüfbar in drei Handgriffen ohne Werkzeug: Eine Notiz auswählen, F2
drücken, den Text ändern, speichern — die Änderung steht da und ist auffindbar.
Links aufs Tray-Symbol klicken — deutsche Einträge mit Symbolen, **kein**
„Beenden". Rechts klicken — „Beenden". Dazu `ollama list` mit den beiden
Modellen.

### 4.2 Die Grenzfrage — sie entscheidet den Schnitt, nicht der Geschmack

Alle vier Kandidaten ergeben **14 SP bei 4 Issues**. Die Story-Grenze wäre
berührt (4 ist der zulässige Höchstwert), die **SP-Grenze gerissen**. Nach B12
wird das dem Kunden **als Grenzüberschreitung** vorgelegt und nicht als
Story-Frage verkleidet. Drei Wege:

| Weg | Umfang | Was er kostet |
|---|---|---|
| **A — #62 in einen eigenen Lauf** *(Empfehlung)* | 3 Issues, **11 SP** | Der Fuzzy-Vortest beginnt später |
| **B — SNI-Frage vor der Freigabe messen** | 4 Issues, **12 SP** | Eine Kundensitzung am Panel **vor** dem Sprint, samt kleinem Prüfprogramm |
| **C — 14 SP freigeben** | 4 Issues, **14 SP** | Grenzüberschreitung; keine Luft für Review-Auflagen |

**Empfehlung: Weg A.** Vier Gründe, in dieser Reihenfolge:

1. **#62 ist nach unserer eigenen Regel nicht ziehbar** — es fehlt die zweite
   Schätzung (2.3).
2. **Es zahlt nicht auf das Sprint-Ziel ein.** Der Spike endet mit einem
   Bericht, nicht mit einem Feature (Abgrenzung in #62); die Umsetzungsstory
   wird danach erst geschnitten. Dieselbe Lage wie #50 in Sprint 3 (3.3) — dort
   war die Trennung richtig.
3. **Er braucht den getakteten `/usr`-Abschnitt** (Nachweis am installierten
   Stand) und konkurriert damit mit den beiden anderen Strängen um das eine
   `/usr` — die Bauart von Sprint-3-Mangel M1.
4. **Die freien 2 SP sind kein Leerlauf.** #11 ist die erste UI-Story mit
   Dialogfluss; in Sprint 2 erzeugte allein der UI-Review einer Story drei
   Auflagen (`54249e0`), in Sprint 3 einen `fail` und einen `warn`.

Weg B ist ehrlich gerechnet die zweitbeste Wahl: Er beseitigt eine Unbekannte
durch Messung, genau wie die FTS5-Vorabmessung vor Sprint 3 (dort 6.5). Er ist
nur teurer, als er aussieht — die Messung braucht ein kleines Prüfprogramm mit
`KStatusNotifierItem` **und** die laufende Plasma-Sitzung des Kunden.

Weg C ist die Wahl des Kunden, nicht des Teams.

### 4.3 Der Schnitt (Weg A)

| Strang | ID | Issue | Story | SP |
|---|---|---|---|---|
| A | S8 | #11 | Bearbeiten-Ansicht in der Bibliothek | 5 |
| B | S33 | #60 | Tray-Menüs trennen, eindeutschen, bebildern | 5 |
| C | T4 | #12 | Ollama-Modelle bereitstellen | 1 |
| | | | **Summe** | **11** |

**Empfohlene Zahl der Dev-Agenten: zwei.** Sie folgt aus den Flächen, nicht aus
der Erlaubnis: Es gibt genau zwei disjunkte Dateimengen (`src/ui/*` +
`librarytest`; `src/shell/*` + `shelltest` + `desktop/`). **#12 braucht keinen
Worktree und keinen Agenten mit Schreibrecht am Repo** — sein Ergebnis ist ein
Issue-Kommentar; es läuft als kleiner eigener Lauf neben den beiden Strängen.

**Worktree- und Zweigzuordnung** (B13, in die Spawn-Aufträge zu übernehmen):

| Strang | Zweig | Dateimenge |
|---|---|---|
| A | `story/11-bearbeiten` | `src/ui/*`, `tests/librarytest.cpp`, `src/CMakeLists.txt` (nur `denkzettelui`-Block), SPEC 9 |
| B | `story/60-traymenues` | `src/shell/*`, `desktop/*.desktop`, `tests/shelltest.cpp`, SPEC 10 |
| C | — | keine Repo-Datei |

## 5. UX-Planning-Beratung zu #60 und die Entscheidungen des PO

Verdikt der Beratung: **`warn`**, sechs Befunde. Der PO hat alle sechs
entschieden und in die Akzeptanzkriterien geschrieben (`gh issue view 60`).
Protokolliert, damit die Entscheidungen nicht nur im Issue leben:

| # | Befund der UX-Beratung | Entscheidung des PO |
|---|---|---|
| 1 | SNI-Trennung ist unter Wayland eine Unbekannte | **Rückfallregel:** Messung am echten Panel ist der erste Schritt; trägt die Positionierung nicht, **Stopp und Kundenentscheidung** — kein Nachbohren (Loop-Regel „needs a human") |
| 2 | Umbenennungsumfang unklar | **Fünf Fundstellen** benannt: QAction, Desktop-`Name=`, drei Meldungstexte in `shortcutregistration.cpp`, SPEC-Wortlaut. Aktions-Id `show-capture` und `Keywords=` bleiben; danach Meta+N vollständig neu prüfen (B5) |
| 3 | DoD 3 greift nicht — plasmashell zeichnet das Menü, `QWidget::grab()` fasst es nicht | **Prüfmittel-Ersatz wie bei #44:** Strukturtest (Beschriftungen, `QIcon::name()`, Trenner, „Beenden" nicht im Linksklick-Menü) + `com.canonical.dbusmenu.GetLayout` am laufenden Dienst + zwei Panel-Fotos, mit dem Zustand der Einstellung „Symbole in Menüs anzeigen" im Bericht |
| 4 | Kürzel-Hinweis im Menü | Meta+N wird angezeigt, **darf die KGlobalAccel-Registrierung nicht doppeln — zu messen, nicht anzunehmen** |
| 5 | „Denkzettel einrichten …" fehlt | Kommt **erst mit #16**; kein ausgegrauter Platzhalter (HIG) |
| 6 | Wireframe 1e zeigt nur ein Menü | **Gestaltungsauftrag läuft**: 1e wird vor der Umsetzung korrigiert (Doppelmenü + Überholt-Vermerk) → Startbedingung für Strang B (6, K6) |

**Ausdrücklich nicht wiedereröffnet: die von UX benannte „Variante C"**
(Linksklick öffnet direkt das Erfassungsfenster). Das ist die Kundenentscheidung
aus #44, am 02.08.2026 bestätigt („Der Linksklick passt", Sprint 3, 15.1). Sie
steht hier, damit sie in einem späteren Review nicht als Befund
wiederauftaucht — dasselbe Muster wie der Vermerk „bei HIG-Reviews kein Befund"
in SPEC 10.

## 6. Klärungspunkte vor dem Ziehen — mit Vorschlag

Die AK-Anpassung macht der PO; der Scrum Master legt den Wortlaut vor.

**K1 — #11, AK „Player während des Bearbeitens nicht bedienbar" ist in M2 nicht
prüfbar.** Der Player ist S16 (#26, M4); in M2 gibt es keinen. Ein AK, das
niemand prüfen kann, macht die Story unabschließbar.
*Vorschlag — herauslösen, nicht umformulieren.* Das Muster steht schon in #11
selbst (das `proposals`-AK ist nach S18a gewandert):

> Aus dem AK streichen: „…der Player ist während des Bearbeitens nicht
> bedienbar." Stattdessen als Kommentar an **#26**: „Aus S8 herausgelöst
> (Sprint-4-Planning): Während des Bearbeitens eines Transkripts ist der Player
> nicht bedienbar. In M2 nicht prüfbar, weil es dort keinen Player gibt."

**K2 — #11, was geschieht mit einer Notiz, die aus der laufenden Suche fällt?**
Wird der Text so geändert, dass er nicht mehr auf den Suchbegriff passt, ist
unbestimmt, ob die Notiz stehen bleibt oder verschwindet. Kein AK sagt es.
*Vorschlag: stehen lassen bis zur nächsten Suchänderung.*

> Ergänzendes AK: „Fällt die gespeicherte Notiz aus der laufenden Suche, bleibt
> sie bis zur nächsten Änderung des Suchbegriffs sichtbar und ausgewählt; die
> Liste springt dabei nicht."

*Begründung:* Eine Notiz, die unter der Hand verschwindet, während man sie
gerade bearbeitet hat, ist derselbe Eindruck, den der Kunde an #57 als
„springt" bemängelt hat. Die Gegenposition (sofort verschwinden lassen, weil
die Trefferliste sonst lügt) ist vertretbar — deshalb **eine kurze UX-Beratung
vor der Festlegung**, entschieden wird vom PO.

**K3 — #11, Kategorie und Tags sind im Pflichtbild leer.** In M2 befüllt sie
niemand (das macht der Analyse-Lauf in M3), das AK verlangt sie aber sichtbar.
*Vorschlag:* Prüfmittel-Vermerk ins Issue, analog zur Prüfmittel-Festlegung bei
#60: „Für die Bilder des UI-Reviews wird die Datenbank von Hand mit Kategorie
und zwei Tags bestückt; der Vermerk gehört in den Bericht."
*Nebenbefund des Devs, melden statt heilen:* Wireframe 2a/2b zeigt Tag-Chips
auch im **Lesezustand** — S5 hat sie nie geliefert. Das ist weder Teil von #11
noch ein Mangel dieser Story; es gehört als eigener Vorgang aufgenommen oder am
Wireframe als offen vermerkt. **Entscheidung des PO.**

**K4 — #12, die Kippbedingung.** Gemessen: ollama 0.32.5 läuft, API antwortet,
1,2 TB frei; `qwen3:8b` und `bge-m3` sind **nicht** installiert, `ollama pull`
braucht kein Root. SPEC 7.1 nennt genau diese beiden als Vorgabe. Ist eine
davon nicht mehr ziehbar, ist die Modellwahl eine SPEC- und Kundenfrage
(DoD 4) — und dann ist #12 keine 1 SP mehr.
*Vorschlag:* AK-Zusatz „Ist ein Spec-Default nicht ziehbar, stoppt der Lauf und
legt dem PO die Modellwahl vor (SPEC 7.1, DoD 4)" — und der Zug wird **vor der
Freigabe** einmal angestoßen. Er kostet nur Bandbreite und nimmt dem Sprint
seine einzige verbliebene Unbekannte.

**K5 — #62, zweite Schätzung fehlt.** Vor dem Ziehen zu heilen: entweder eine
unabhängige Zweitschätzung einholen oder den Spike ausdrücklich als
**Zeitbudget-Spike** führen (Budget statt Punkte — dann steht das Budget im
Issue). So oder so nicht in diesem Sprint (4.2).

**K6 — #60, Startbedingung.** Wireframe 1e ist vor Beginn der Umsetzung
korrigiert (Gestaltungsauftrag läuft). Beginnt der Strang vorher, prüft der
UI-Vergleich gegen eine überholte Zeichnung — der Fehler, den Sprint 2 bei S8
als `fail` gebucht hat.

**K7 — #57 und #59 liegen in der Fläche von #11** (3.3). Weder gezogen noch
geschätzt. *Vorschlag:* keine stille Mitnahme. Entweder der PO zieht sie
bewusst (dann ins Sprint-Konto buchen und dem Kunden vorlegen — die
Story-Grenze wäre erreicht) oder er schreibt in den Auftrag von Strang A, dass
beide Fehler **bekannt und nicht Teil der Story** sind. Ohne diesen Satz hält
der Dev die Symptome für seine eigenen und heilt sie außerhalb seiner Fläche.

## 7. Nachweise, die Agenten nicht führen können

Vorab benannt statt im Review entdeckt (Muster Sprint 3, 5).

1. **#60 — die SNI-Messung und die zwei Panel-Fotos.** Ein Agent kann das
   exportierte Menü über `GetLayout` zurücklesen; ob das Popup unter Wayland an
   der richtigen Stelle erscheint, sieht nur der Kunde am Panel. **Erster
   Schritt der Story, und er braucht eine Kundensitzung.**
2. **#11 — ob sich das Bearbeiten richtig anfühlt.** Cursorposition,
   Dialogfluss, Rückkehr in den Lesezustand sind offscreen prüfbar; der
   Gesamteindruck ist es nicht. Gehört auf die M2-Checkliste und in die
   Kundensichtprüfung.
3. **#12 — nichts.** Der Zug ist agentenfähig; nur die Ladezeit ist real.

## 8. Risiken, die diesen Schnitt kippen

**8.1 — Der Neuaufbau nach dem Speichern lässt die Liste springen** (3.3).
Das schwerste Risiko, und es ist ein Fehler mit Ansage: `previous` ist nach dem
Neuaufbau entwertet, `crossesAGroupBoundary` immer wahr. **Vorkehrung:** Eine
Zusicherung, die den Rollwert **vor und nach** dem Speichern vergleicht — nicht
zwei Endbilder (Sprint 3, 12.1: *bei Bewegungen ist der Weg der Prüfgegenstand*).
Gehört als Testauflage in den Auftrag von Strang A.

**8.2 — Der Wächterdialog hat drei Auslösepfade** (Auswahlwechsel, Fenster
schließen, Esc). Der Auswahlwechsel ist der teure: `currentChanged` läuft heute
ungebremst in `showNote()`. **Vorkehrung:** Die Bauart-Entscheidung (Dialog vor
dem Modellwechsel oder Rücksetzen der Auswahl danach) fällt **am Anfang** der
Story und steht mit Begründung im Bericht — dieselbe Vorkehrung, die bei der
Zeilensemantik in Sprint 3 getragen hat (dort 6.1).

**8.3 — #60 kann planmäßig scheitern.** Trägt die Wayland-Positionierung nicht,
endet die Story mit einem Stopp und einer Kundenfrage. **Das ist kein Mangel,
sondern der vorgesehene Weg** (DoD 2 in der Fassung nach B5) — es ist aber ein
halbes Sprint-Ziel. Der Kunde sollte das vor der Freigabe wissen.

**8.4 — `SPEC.md` als gemeinsame Fläche zweier Stränge** (3.2). Verschiedene
Abschnitte; der Merge macht der PO. Kein neues Risiko, nur eines, das in
Sprint 3 nicht bestand.

**8.5 — Der PR-Probelauf trifft auf die Rebase-Regel.** `PROZESS.md` sagt „ein
Strang, der `main` braucht, rebased" — nach einem Push erzwingt das einen
Force-Push und setzt PR-Kommentare auf „outdated" (karpathy-Befund 4c.1 vom
02.08.2026). **Vorkehrung für diesen Sprint:** Die beiden Stränge brauchen
`main` in diesem Schnitt nicht (disjunkte Flächen). Braucht einer ihn doch,
entscheidet der PO im Einzelfall; **kein Force-Push durch einen Dev**. Die
Erfahrung gehört in die Bewertung des Probelaufs.

## 9. Die neuen Instrumente, erstmals im Einsatz

- **Sprint-Konto (B12):** Abschnitt 1, ab der ersten Zeile geführt. Jeder Zugang
  nach der Freigabe wird dort gebucht — mit **beiden** Zahlen.
- **Basis-Tag (Kundenentscheidung 02.08.2026):** Nach der Freigabe setzt der PO
  `sprint-04-basis` auf den Ausgangsstand. Der Prüf-Diff des Sprint-Endes ist
  dann `git diff sprint-04-basis..main` — nicht mehr eine von Hand gesuchte
  Commit-Kennung.
- **PR-Probelauf, befristet auf diesen Sprint:** Je Story-Strang ein PR, vom PO
  geöffnet und gemerged, `--no-ff`. **Strang C erzeugt keinen PR** — er hat
  keinen Code; das Abbruchkriterium wird über die PRs bewertet, die es gibt.
  Kriterium (steht vorab in `PROZESS.md`): mindestens ein Befund hängt an einer
  Diff-Zeile, der ohne PR nicht auffindbar gewesen wäre, **oder** ein
  automatischer Testlauf ist auf einem PR gelaufen. Sonst endet das Verfahren.
- **Sprint-Abschluss (B11):** Die elf Punkte in zwei Takten gelten; Punkt 10
  (Version und Tag) wird als **ausgesetzt** geführt, solange #61 offen ist.

## 10. Hinweise an den Product Owner

**Vor dem Ziehen zu erledigen:**

1. **#11:** K1 (Player-AK herauslösen, Wortlaut in 6), K2 (AK zum Suchfall nach
   kurzer UX-Beratung), K3 (Prüfmittel-Vermerk).
2. **#12:** K4 (Stopp-AK) und den Zug vor der Freigabe einmal anstoßen.
3. **#62:** K5 (zweite Schätzung oder Zeitbudget) — unabhängig davon, wann es
   gezogen wird.
4. **#60:** K6 (Wireframe 1e als Startbedingung bestätigen).
5. **Labels angleichen:** #11 trägt `sp:2`, konsolidiert sind 5; #60 und #62
   tragen **kein** `sp:`-Label. Die Issues sind die einzige Quelle der Wahrheit
   — der Widerspruch von Sprint 3 (M6) darf sich nicht wiederholen.
6. **Milestone „Sprint 4" anlegen** und die freigegebenen Issues zuordnen.
7. **K7 entscheiden:** #57 und #59 ziehen oder im Auftrag von Strang A
   ausdrücklich als bekannt und ausgenommen benennen.
8. **Worktrees und Zweige** aus 4.3 in die Spawn-Aufträge übernehmen.

## 11. Was dem Kunden zur Entscheidung vorliegt

1. **Freigabe des Sprints.** Zwei Dinge, die Sie sehen: Notizen lassen sich in
   der Bibliothek ändern (F2, tippen, speichern — mit Nachfrage, wenn Sie
   ungespeichert weiterklicken). Und das Tray-Menü wird zu zwei Menüs: links
   die Arbeitswege auf Deutsch und mit Symbolen, ohne „Beenden"; rechts die
   Verwaltung. Dazu ein drittes, unsichtbares: Die KI-Modelle werden auf den
   Rechner geladen — der erste Schritt zur automatischen Ordnung, die Sie im
   Interview zuerst haben wollten.
2. **Eine Grenze ist im Spiel.** Alle vier Themen zusammen ergeben 14 Punkte;
   vereinbart sind höchstens etwa 13 und höchstens vier Arbeitspakete.
   **Empfehlung: den Fuzzy-Vortest (#62) getrennt laufen lassen** — er liefert
   einen Bericht, keine sichtbare Änderung, und er ist noch nicht zu Ende
   geschätzt. Dann bleiben 11 Punkte und Luft für die Nacharbeit, die eine
   Bearbeiten-Ansicht erfahrungsgemäß erzeugt. Wollen Sie alle vier, ist das
   Ihre Entscheidung — dann bitte ausdrücklich, denn es ist eine
   Grenzüberschreitung.
3. **Ein Teil des Ziels kann planmäßig scheitern.** Ob sich unter Wayland zwei
   getrennte Tray-Menüs zuverlässig öffnen lassen, weiß niemand vorher. Wir
   messen es als **erstes** und hören auf, wenn es nicht trägt — dann legen wir
   Ihnen einen Ersatz vor (etwa „Beenden" abgetrennt in einer letzten Gruppe).
   Das ist gewollt und kostet dann nur den Messschritt.
4. **Zwei Dinge müssen Sie selbst tun.** Die Messung am echten Panel und die
   beiden Fotos der Menüs — ein Agent kann das Panel nicht bedienen. Und die
   Sichtprüfung, ob sich das Bearbeiten richtig anfühlt.
5. **Nicht in diesem Sprint:** die Suchoperatoren (#10) und die
   Einstellungen-Seite (#16). Beide sind vorbereitet.

## 12. done / next

**done:** Sprint-4-Schnitt vorgeschlagen (3 Issues, 11 SP, zwei Codestränge und
ein Lauf ohne Repo-Datei) und am Quellstand `a484d49` begründet; vier
Schätzungen konsolidiert — #11 auf 5 mit der Feststellung, dass die alte 2 eine
andere Story geschätzt hat, #60 regelkonform auf 5 statt auf die bequemere 3,
#62 als **nicht schätzregelkonform** ausgewiesen; das **Sprint-Konto (B12)
erstmals geführt** und dabei sofort eine Grenzüberschreitung gefunden (14 SP im
Kandidatenfeld), die dem Kunden als solche vorliegt; die Konfliktflächen am Code
geprüft (zwei disjunkte Codeflächen, `SPEC.md` als gemeinsame Datei) und die
eigentliche Wechselwirkung von #11 in `showNote()` lokalisiert — der Fläche der
offenen Fehler #57/#59; UX-Beratung zu #60 samt der sechs PO-Entscheidungen
protokolliert und die Nicht-Wiedereröffnung von „Variante C" festgehalten;
sieben Klärungspunkte mit Wortlautvorschlägen vorgelegt; fünf Risiken benannt,
darunter das Springen der Liste nach dem Speichern und die Kollision des
PR-Probelaufs mit der Rebase-Regel.

**next:** (1) Kundenentscheidung über Freigabe und über den Umgang mit der
Grenze (11.2). (2) PO-Aufgaben vor dem Ziehen (10), darunter der vorgezogene
`ollama pull` und die Label-Angleichung. (3) Nach der Freigabe: Basis-Tag
`sprint-04-basis` setzen, Milestone anlegen, Worktrees und Zweige nach 4.3
aufsetzen, PR je Strang. (4) Während des Sprints: Sprint-Konto bei jedem Zugang
fortschreiben. (5) Am Sprint-Ende: Sprint-Abschluss nach B11 in zwei Takten,
Punkt 10 ausgesetzt bis #61; Bewertung des PR-Probelaufs gegen sein vorab
festgelegtes Kriterium.

## 13. Kundenfreigabe (PO-Vermerk, 02.08.2026, 12:42)

Der Kunde hat im Anschluss an das vierte Design-Interview entschieden:
**Weg A** — Sprint 4 startet mit den drei Strängen **#11 (5) · #60 (5) ·
#12 (1) = 11 SP / 3 Stories**; der Spike #62 läuft als eigener Lauf nach
dem Sprint (Vermerk an #62). Sprint-Ziel wie in Abschnitt 5 vorgeschlagen.
Die PO-Aufgaben aus Abschnitt 10 waren zum Zeitpunkt der Freigabe
erledigt: #11 nachgeschärft (K1–K3, K7), Labels angeglichen (#11 sp:5,
#60 sp:5, #62 sp:3), K4-Vorprüfung positiv (beide Spec-Defaults auf
ollama.com erreichbar, HTTP 200), Wireframe 5a liegt vor (K6 erfüllt).

**Sprint-Konto bei Freigabe: 11 von ~13 SP · 3 von 4 Stories.**
