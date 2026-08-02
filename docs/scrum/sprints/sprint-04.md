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

## 13. Kundenfreigabe (PO-Vermerk, 02.08.2026, 12:40)

Der Kunde hat im Anschluss an das vierte Design-Interview entschieden:
**Weg A** — Sprint 4 startet mit den drei Strängen **#11 (5) · #60 (5) ·
#12 (1) = 11 SP / 3 Stories**; der Spike #62 läuft als eigener Lauf nach
dem Sprint (Vermerk an #62). Sprint-Ziel wie in Abschnitt 5 vorgeschlagen.
Die PO-Aufgaben aus Abschnitt 10 waren zum Zeitpunkt der Freigabe
erledigt: #11 nachgeschärft (K1–K3, K7), Labels angeglichen (#11 sp:5,
#60 sp:5, #62 sp:3), K4-Vorprüfung positiv (beide Spec-Defaults auf
ollama.com erreichbar, HTTP 200), Wireframe 5a liegt vor (K6 erfüllt).

**Sprint-Konto bei Freigabe: 11 von ~13 SP · 3 von 4 Stories.**

## 14. Scope-Entscheidung während des Sprints (PO-Vermerk, 02.08.2026, 13:09)

Strang B hat die SNI-Messung als ersten Schritt geführt — **Negativbefund**:
Ein selbst gezeichnetes Menü ist unter Plasma/Wayland unsichtbar (Abbau nach
2 ms) oder landet in der Bildschirmmitte; die Kette bis zum Handler trägt,
erst das Anzeigen scheitert (Belege im Zweig `story/60-traymenues`,
Commit `cacc4d2`). Die Rückfallregel aus #60 griff wie vorgesehen: Stopp,
Meldung, Kundenentscheidung.

Dem Kunden lagen beide Wege samt Einordnung vor (die Referenz-Applets sind
Bausteine der Leiste und öffnen links ihren Inhalt, kein zweites Menü):
**Direktstart** (Linksklick öffnet das Erfassungsfenster; PO-Empfehlung) und
**ein Menü mit abgesetztem „Beenden"**. Der Kunde wählte das eine Menü —
die #44-Entscheidung gilt fort (A1), A2/A3 entfallen, der ausgegraute
„Einstellungen"-Eintrag entfällt bis #16. AK von #60 umgestellt, Strang B
baut mit dem Rückfallstand weiter, Wireframe 5a wird angepasst.

Sprint-Konto unverändert: 11 von ~13 SP · 3 von 4 Stories (Umfang von #60
verengt sich, wird nicht nachgebucht — die Messung war eingepreist).

## 15. DoD-Prüfung Sprint 4 — Takt 1 (Scrum Master, 02.08.2026, 14:55)

**Prüfgegenstand:** `main` @ `d1d6ac0` (beide PRs gemergt: #64 → `81f1605`,
#65 → `c6d6ba6`) und der installierte Stand unter `/usr`.
**Prüfweise:** Jeder Haken unten steht auf einer eigenen Messung, nicht auf
einem Bericht. Die Berichte sind die Behauptung, der Befehl ist der Beleg
(Sprint 3, 12.1). Wo ich einen Bericht nur wiedergebe, steht es dabei.

### 15.1 Takt-1-Liste (B11) Punkt für Punkt

| # | Punkt | Beleg | Stand |
|---|---|---|---|
| 1 | Endstand einmal nach `/usr` installiert | `md5sum build/bin/denkzetteld /usr/bin/denkzetteld` → beide `7fbd905e8705992a699bcd88fb8f7a09`; `diff` der Desktop-Datei `/usr` gegen Repo → leer; laufender Dienst PID 254981, `readlink /proc/254981/exe` → `/usr/bin/denkzetteld` | **erfüllt** |
| 1a | Hauptweg #60 daran ausgeführt | `com.canonical.dbusmenu.GetLayout` am laufenden Dienst (`:1.1320`, Objekt `/MenuBar`), selbst abgefragt: acht Einträge in der Reihenfolge von Wireframe 5a, alle Beschriftungen deutsch, `icon-name` an jedem Eintrag, zwei Trenner, „Beenden“/`application-exit` als letzter, `shortcut [['Super','N']]` am ersten. Kürzelkette: `Component.allShortcutInfos` an `/component/org_denkzettel_Denkzettel_desktop` → `show-capture`, Anzeigename „Notiz erfassen“, Tasten `268435534` (= `Qt::MetaModifier` 268435456 + `Qt::Key_N` 78), **genau ein** Eintrag; alle Komponenten des Sitzungsdienstes auf `268435534` durchgesehen — nur diese eine | **erfüllt bis auf den Klick** (O1) |
| 1b | Hauptweg #12 daran ausgeführt | `ollama list` → `qwen3:8b` (5,2 GB) und `bge-m3:latest` (1,2 GB) vorhanden; `/api/embed` mit `bge-m3` → Vektor der Länge 1024; `/api/chat` mit `qwen3:8b` selbst gemessen → `message.content` „Paris“, `done_reason: stop` | **erfüllt** (Belegmangel M1) |
| 1c | Hauptweg #11 daran ausgeführt | **kein Nachweis in einer der drei zugelassenen Formen.** Als Grenze im Bericht benannt (`sprint-04-installationstakt.md`, „Grenzen dieses Takts“) und in die Kundenabnahme gelegt. Tragfähig, weil das installierte Abbild bytegleich mit dem gebauten ist (Zeile 1) — was der installierte Lauf für #11 hinzufügt, ist allein die echte Plasma-Sitzung, und genau dort sitzt die einzige offene Frage (Befund 5) | **offener Punkt O2**, keine Mängelbuchung |
| 2 | Jeder Prüflauf hat einen Bericht als Datei, vor der DoD-Prüfung | Vollzähligkeit über den Prüfweg aus B11 geführt: Jede Commit-Botschaft zwischen `sprint-04-basis` und `main`, die Befunde eines Prüflaufs nennt, hat ihren Bericht — `cacc4d2`/`54840bd` → `sprint-04-s33-traymenues/` (messung.md, README.md, sni-messung.txt, menue-getlayout.txt, kuerzel-nachpruefung.txt), `433e87e`/`95bcab6`/`6848c44` → `sprint-04-s8-ui-review/bericht.md`, `1e75067`/`2513630` → `sprint-04-karpathy.md`, `5fdee07`/`a78920a` → `sprint-04-s8-bearbeiten/LIESMICH.md`, `d1d6ac0` → `sprint-04-installationstakt.md`. `646804a` und `88d6e7f` nennen zwar Befunde, sind aber Selbstkorrekturen des Entwicklers, kein Prüflauf. Alle Dateien lagen vor 14:55 im Repo | **erfüllt** |
| 3 | DoD 1–4 je Story, Doku-Abgleich nach B10 | 15.2 bis 15.6 | **geführt** |
| 4 | Mängelliste an den PO | 15.8 | **geführt** |

### 15.2 DoD 1 — Build und Tests

- **Warnungsarm:** Vollständiger Neubau in einem leeren Verzeichnis
  (`cmake -B <scratch> -S . -DCMAKE_BUILD_TYPE=Debug`, danach `cmake --build`),
  32 Übersetzungseinheiten, Rückgabewert 0, **null Zeilen `warning:`**. Der
  Neubau war nötig: Im vorhandenen `build/` wurde nichts übersetzt, dort hätte
  jede Warnung gefehlt.
- **Tests grün:** `ctest --test-dir build` → **7 von 7** bestanden (5,36 s),
  darunter `librarytest` und `shelltest`.
- **Geometrie-Zusicherungen für den neuen Ansichts-Zustand, bei zwei
  Fenstergrößen** — am Testcode nachgezählt, nicht dem Bericht geglaubt:
  `keepsTheMeasuresOfTheEditState` (`tests/librarytest.cpp:2930–3056`) ist
  datengetrieben über **900×600 und 1200×800** und trägt **zwölf**
  Zusicherungen je Größe: Knopfbreite gleich natürlicher Breite für
  „Bearbeiten“ und „Löschen“ (Befund 7), gleiche y-Koordinate des Textstapels
  in beiden Zuständen (Befund 1), die drei Reihenfolgen Kopf → Textfeld →
  Merkmalszeile → Fußzeile, Fußzeile am unteren Rand (≤ 12 px Rest), Textfeld
  ≥ halbe Bereichshöhe, Merkmalszeile und Fußzeile einzeilig, die
  Höhendifferenz zwischen Lese- und Bearbeiten-Zustand exakt gleich
  Merkmalszeile + Fußzeile + 2 × Abstand, Splitterbreite 300. Einzellauf
  bestätigt: beide Datenzeilen `PASS`.
- **Wächterdialog:** `asksBeforeUnsavedChangesAreLost` ist die volle 3 × 3
  Matrix (Auswahlwechsel · Fensterschließen · Esc × Speichern · Verwerfen ·
  Abbrechen), alle neun `PASS`. Der vierte Auslöseweg der SPEC — der
  Abbrechen-Knopf — ist **kein Loch**: Der Knopf löst dieselbe `QAction` aus
  wie Esc (`librarywindow.cpp:378–379` gegen `:228–230`), und dass er sie
  auslöst, prüft `leavesTheEditorWithoutAskingWhenNothingWasChanged`
  (`librarytest.cpp:2534`).
- #60 hat keine Ansicht mit Raumaufteilung; Geometrie-Zusicherungen entfallen
  dort zu Recht.

### 15.3 DoD 2 — Akzeptanzkriterien je Story

**#11 (S8), elf AK:** Alle elf haben Test, Messwert oder Bild. Stichproben am
Code statt an der Liste: AK 1 → `opensTheEditorWithTheButton`,
`opensTheEditorWithF2`, `leavesTheWordSelectionToTheDoubleClick`; AK 4 →
`marksTheSavedNoteForANewEmbedding`, `findsTheSavedTextInTheSearchIndex`,
`keepsCategoryTagsAndStateWhileSaving`; AK 7 → die 3 × 3-Matrix; AK 11 (K2) →
`keepsTheSavedNoteInTheResultListUntilTheSearchChanges` plus Bilder 11–13.
Der Hauptweg ist am **gebauten** Stand belegt (`sprint-04-s8-bearbeiten/`),
am installierten nicht (O2).

**#60 (S33), sechs AK:** AK 1 (SNI-Messung als erster Schritt) abgehakt und
belegt. AK 2, 3 und 5 am installierten Dienst selbst nachgemessen (15.1, 1a);
zusätzlich Strukturtest im `shelltest`
(`announcesItselfAsAMenuAndKeepsTheMenuToShow`,
`showsTheEntriesOfTheWireframeWithTheirIcons`, `keepsQuitApartInTheLastGroup`,
`hintsTheShortcutWithoutBindingItASecondTime`). AK 4 (Eindeutschung an fünf
Fundstellen) nachgezählt: `globalshortcuts.cpp:60`, `trayicon.cpp:75`,
Desktop-Aktion `Name=Notiz erfassen`, die drei Meldungstexte in
`shortcutregistration.cpp:49/57/62`, SPEC 10 — alle deutsch; `show-capture`
als Objektname (`globalshortcuts.cpp:70`) und `Keywords=…Capture;`
unverändert, wie festgelegt; „Capture öffnen“ kommt in `src/` und `desktop/`
nicht mehr vor. AK 6 (SPEC 10) siehe 15.5. **Offen:** die zwei Panel-Fotos
(O1).

**#12 (T4), zwei AK:** Modelle gezogen und vorhanden. Der Endpunkt-Beleg
weicht vom AK ab — siehe **M1**.

### 15.4 DoD 3 — Reviews

- **karpathy-Sprint-Review:** `docs/scrum/reviews/sprint-04-karpathy.md`,
  Erstlauf `fail` (ein Befund), Nachprüfung nach der Heilung **`warn`, kein
  `fail`**. Die Heilung des `fail`-Befundes selbst nachgelesen:
  `tests/librarytest.cpp:2963–2972` trägt die gemessene Fassung (der Stapel
  fordert die Breite an, tragend ist das waagerechte `QSizePolicy::Maximum`)
  und benennt die widerlegte addStretch-Lesart ausdrücklich als widerlegt —
  deckungsgleich mit `librarywindow.cpp:336–339` und der Botschaft von
  `60cae75`. **DoD 3 erfüllt.**
- **UI-Review S8 (#11), zweistufig:** `sprint-04-s8-ui-review/bericht.md` mit
  eigenen Bildern der Prüferin (`uxshots.cpp`, `uxshots-nachpruefung.cpp`,
  `iconprobe.cpp` daneben versioniert, B7). Sechs Befunde im Erstlauf, davon
  vier geheilt und nachgeprüft, Befund 6 liegt als **#63** im Backlog,
  Befund 5 bleibt `warn` mit benannter Ursache. Ein siebter Befund entstand in
  der Nachprüfung und ist geheilt. **Kein offener `fail` — DoD 3 erfüllt**,
  mit der Einschränkung **M3**.
- **#60 ist per PO-Festlegung keine UI-Story im DoD-3-Sinn.** Die drei
  Ersatz-Prüfmittel sind erbracht: (a) Strukturtest — vier Zusicherungen im
  `shelltest`; (b) `GetLayout` — von mir am laufenden Dienst selbst abgefragt;
  (c) zwei Panel-Fotos — **offen** (O1). Zwei von drei erbracht, das dritte
  ist der Kundenklick.

### 15.5 DoD 4 — SPEC gegen die Lieferung

- **SPEC 9** (`SPEC.md:405–422`) trägt die drei Bedingungen des
  Bearbeiten-Zustands; die letzten beiden sind als **bei der Umsetzung
  entdeckt** gekennzeichnet (Fassung nach B9). Gegen die Lieferung gelesen:
  Wächterdialog über alle vier Auswege, Stehenbleiben in der Trefferliste,
  abgeschaltetes Suchfeld — alle drei im Bau vorhanden. **erfüllt.**
- **SPEC 10** (`:435–494`) ist auf ein Menü umgestellt, Icon-Tabelle und
  Wortlaut stimmen mit dem überein, was der installierte Dienst über
  `GetLayout` ausliefert (Eintrag für Eintrag verglichen). Die **erste
  entdeckte Bedingung (SNI-Grenze)** steht dort ausdrücklich als solche
  (`:470–484`), samt Belegverweis und der Feststellung, dass von A1–A3 nur A1
  bleibt. Eine zweite entdeckte Bedingung (Symbolnamen erreichen Plasma nur
  bei auflösbarem Symbol-Thema, `QT_QPA_PLATFORMTHEME=kde` für Testläufe)
  steht ebenfalls dort und deckt sich mit `tests/CMakeLists.txt:75–76`.
  **erfüllt.**
- **Die zweite im Sprint entdeckte Bedingung des S8-Strangs — der
  Plattform-Ersatzdialog — steht in keiner bindenden Quelle.** Siehe **M2**.

### 15.6 Doku-Abgleich (B10)

- **README-Statuszeile:** beschreibt den gelieferten Stand (Bearbeiten mit
  Wächterdialog, Tray-Menü mit Symbolen und deutschen Beschriftungen,
  „Beenden“ abgesetzt) und **keinen Verfahrensstand** — kein „in der
  Kundenabnahme“. Regelkonform nach B11 Takt 1, Punkt 3. **ok.**
- **Übriges README:** Linter-Abschnitt und Namensabschnitt unberührt und
  weiter zutreffend. **ok.**
- **`docs/`** enthält nur Prozessunterlagen, keine Produktbeschreibung; kein
  Abgleichsbedarf über 15.5 hinaus. **ok.**
- **Hinweis, kein Mangel:** SPEC §2/§3 sprechen weiter vom „Capture-Fenster“
  (Modul- und Architektursprache), die Meldungstexte sagen jetzt
  „Erfassungsfenster“. Das liegt außerhalb des Umbenennungsumfangs von #60
  (nur sichtbare Texte; Ids ausdrücklich nicht) und ist korrekt nicht
  angefasst worden — der PO sollte die Wortfamilien-Divergenz kennen
  (karpathy 3.1, „gemeldet, nicht geheilt“).

### 15.7 Sprint-Konto — Schlussstand (B12)

| Buchung | Issues | Story Points | Grenzen (2–4 · ~13) |
|---|---|---|---|
| Kandidatenfeld | 4 | 14 | SP-Grenze gerissen |
| Vorschlag des Scrum Masters (4.3) | 3 | 11 | beide gehalten |
| **Freigabe-Stand (13)** | **3** | **11** | beide gehalten |
| Scope-Entscheidung #60 (14) | 3 | 11 | Verengung, keine Buchung |
| **Schlussstand** | **3 von 4** | **11 von ~13** | **beide gehalten** |

Nach der Freigabe ist **kein Zugang** gebucht worden; der Milestone „Sprint 4“
trägt genau die drei freigegebenen Issues (#11, #60, #12), keines mehr. Damit
ist die Bauart des Sprint-3-Befunds (fünf Issues, nie als Grenzüberschreitung
vorgelegt) in diesem Sprint nicht wiedergekehrt. Das Konto hat sich in
Sprint 4 zweimal bewährt: einmal beim Fund der 14 SP im Kandidatenfeld, einmal
hier als Nachweis, dass die Verengung von #60 keine stille Nachbuchung war.

### 15.8 PR-Probelauf — Bewertung gegen das vorab festgelegte Kriterium

Das Kriterium steht in `PROZESS.md` und lautet: *mindestens ein Befund hängt
an einer Diff-Zeile, der ohne PR nicht auffindbar gewesen wäre,* **oder** *ein
automatischer Testlauf ist auf einem PR gelaufen.* Beide Hälften selbst
geprüft:

- **Automatische Testläufe:** `gh pr list --json statusCheckRollup` liefert für
  #64 und #65 je eine **leere Liste**. Kein Lauf. **Nicht erfüllt.**
- **Befund an einer Diff-Zeile:** Der einzige `fail` des Sprints hängt an
  `tests/librarytest.cpp:2964–2967`. Diese Zeilen stammen aus **einem einzigen
  Commit** (`60cae75`, `git show --stat`: vier Dateien, 29 Zeilen, alle
  hinzugefügt), und der Widerspruch wird erst im Nebeneinander von Diff-Zeile,
  Commit-Botschaft und `librarywindow.cpp:336–339` sichtbar — alle drei zeigt
  `git show 60cae75`. Der Prüfer hat das selbst so festgehalten. **Nicht
  erfüllt.**

**Ergebnis: nicht bestanden.** Nach der vorab getroffenen Festlegung endet der
Probelauf damit, und der Basis-Tag bleibt allein.

Zwei Beobachtungen, die zur Ehrlichkeit der Bewertung gehören:

1. **Der Probelauf ist nur zur Hälfte durchgeführt worden.** Die Festlegung
   verlangte „Dev-Bericht und Review-Befunde als Kommentare“ am PR. Gemessen:
   #64 und #65 tragen **je einen** Kommentar, und der ist eine automatische
   Abschaltmeldung eines fremden Dienstes. Weder Dev-Bericht noch
   Review-Befunde stehen an den PRs. Getragen hat also nur die Mechanik
   (Zweig, PR, `--no-ff`), nicht das Verfahren. Wer den Probelauf für
   gescheitert erklärt, sollte wissen, dass die Hälfte, die den Nutzen bringen
   sollte, nicht gelaufen ist.
2. **Die vorhergesagte Kollision blieb aus** (8.5): Beide Stränge kamen ohne
   `main` aus, kein Rebase, kein Force-Push, keine „outdated“-Kommentare. Das
   Risiko war richtig eingeschätzt und richtig eingehegt.

Die Entscheidung über Fortführung, Wiederholung mit vollständigem Verfahren
oder Ende liegt beim Kunden; die Regel sieht ohne Gegenentscheidung das Ende
vor.

### 15.9 Mängelliste (melden, nicht heilen)

**M1 — #12: Der vom AK und von SPEC 7.1 benannte Endpunkt `/api/chat` ist
nicht belegt. Schwere: gering.**
Das AK verlangt „`/api/chat` und `/api/embed` je einmal von Hand belegt“;
SPEC 7.1 (`SPEC.md:257`) bindet dieselben beiden. Der Vollzugskommentar an #12
belegt `/api/embed` und **`/api/generate`**. Selbst nachgemessen: `/api/chat`
mit `qwen3:8b` antwortet richtig (`message.content` „Paris“, `done_reason:
stop`) — der Mangel ist ein Belegmangel, kein Funktionsmangel. Er ist trotzdem
mehr als eine Formalie: Die Antwort von `/api/chat` trägt den Denkteil in
einem **eigenen Feld** `message.thinking`, während `content` sauber bleibt.
Genau daran hängt der Hinweis für S9 im selben Kommentar („leere Antwort bei
knappem `num_predict`“) — an `/api/generate` gemessen, gilt er für den
Endpunkt, den die SPEC gar nicht vorsieht.
*Empfehlung:* Beleg für `/api/chat` in den Issue-Kommentar nachtragen und den
S9-Hinweis auf den Endpunkt der SPEC beziehen. Zuständig: PO.

**M2 — DoD 4: Die entdeckte Bedingung „Plattform-Ersatzdialog“ steht in keiner
bindenden Quelle. Schwere: mittel.**
Gemessen im UI-Review (`bericht.md`, Stelle 5, mit `iconprobe.cpp` isoliert):
Unter `QT_QPA_PLATFORMTHEME=kde` zeigt Qt statt des gebauten `QMessageBox`
einen Ersatzdialog der KDE-Plattformintegration **mit eigenen
Knopfobjekten**; nachträglich gesetzte Symbole, Vorgabe- und Escape-Knopf
wirken dort nicht. Das ist genau die Bauart, für die B9 gefasst ist: eine
Bedingung der Umgebung, unter der eine Festlegung nicht so gilt, wie der Code
sie ausdrückt. Sie steht heute in vier Prüfberichten und einem Testkommentar
(`tests/editshots.cpp:195`) — **nicht in SPEC 9 und nicht in SPEC 16**. Die
Folge ist bereits im Repo sichtbar und nicht dokumentiert: `librarytest`
bekommt in `tests/CMakeLists.txt:31` nur `QT_QPA_PLATFORM=offscreen` (der
`shelltest` bekommt in `:75–76` beides), also misst
`namesTheThreeAnswersOfTheGuardDialog` einen Dialog, den kein Nutzer einer
KDE-Sitzung zu sehen bekommt. Der Test ist richtig und grün — er misst nur
etwas anderes, als er zu messen scheint. `CLAUDE.md`: „Entdeckte Bedingungen
ziehen die SPEC nach.“
*Empfehlung:* Bedingung in SPEC 9 (bei den Bedingungen des
Bearbeiten-Zustands) und ihre Testfolge in SPEC 16 aufnehmen — entweder jetzt
oder zusammen mit der Entscheidung zu Befund 5 (O3). Zuständig: PO.

**M3 — DoD 3: Die eigenen Bilder der Prüferin zeigen nicht den gelieferten
Stand. Schwere: gering.**
Nachgerechnet, nicht vermutet: `sprint-04-s8-ui-review/n01-lesen.png` hat die
Prüfsumme `28798c927d3e6311df47210afbec40bf` — **bytegleich mit dem
Entwicklerbild `01-lesen.png` in der Fassung vor `60cae75`** und verschieden
vom heutigen (`a53ec30c…`). Die letzte sichtbare Änderung der Kopfzeile
(`60cae75`, 14:17, Heilung von Befund 7) liegt **nach** der Nachprüfung der
Prüferin (`95bcab6`, 14:12) und nach ihren Bildern. Der Entwickler hat seine
Bilder 01 und 04 mit dem Commit erneuert; die Prüferin hat den geheilten
Zustand nie in einem eigenen Bild gesehen. DoD 3 ist formal erfüllt (kein
offener `fail`), aber B3 sagt: *bei Zuständen ist das Bild der Prüfgegenstand,
nicht die Zusicherung* — und die Kopfzeile ist ein Zustand. Dass Befund 7
selbst in einem Bild (`n08`) gefunden wurde und nicht in einem Test, ist das
Argument gegen die Abkürzung.
*Zu bedenken:* Die Prüferin hatte für diese Stelle ausdrücklich eine
Geometrie-Zusicherung verlangt, und die ist geliefert und läuft bei zwei
Fenstergrößen (15.2). Der Kunde sieht die Kopfzeile in der Abnahme ohnehin —
mit Punkt 3 der Checkliste (15.10) ist M3 dort schließbar.
*Empfehlung:* entweder ein kurzer Bildlauf der Prüferin auf dem Endstand oder
ein PO-Vermerk, dass die Zusicherung hier genügt und die Kundensicht den Rest
tut. Zuständig: PO.

Keine weiteren Mängel. Insbesondere ohne Befund geblieben: Build-Warnungen,
Testschärfe, Sprint-Konto, README-Statuszeile, SPEC 9 und SPEC 10,
Berichts-Vollzähligkeit.

### 15.10 Offene Punkte und Abnahme-Checkliste für den Kunden

**Offene Punkte (nicht als Mangel geführt, weil planmäßig in die Abnahme
gelegt):**

- **O1 — #60: die zwei Panel-Fotos.** Ein Agent kann das Panel nicht bedienen;
  die Unmöglichkeit ist gemessen und im Strang-README belegt.
- **O2 — #11: der Bearbeiten-Hauptweg am installierten Stand.**
- **O3 — #11 Befund 5: Trägt der Ersatzdialog unter echtem Plasma Symbole?**
  Wenn ja, ist die Sache erledigt und `setIcon()` nur wirkungslos, nicht
  falsch. Wenn nein, führt der Weg über `KMessageBox`/`KMessageDialog`. Das
  ist eine Messung am laufenden Stand, keine Geschmacksfrage.

---

**Abnahme-Checkliste für den Kunden** — fünf Handgriffe, alle am installierten
Stand, keiner braucht ein Werkzeug:

1. **Linksklick aufs Tray-Symbol, dann Rechtsklick.** Beide Male soll dieselbe
   Liste aufgehen: *Notiz erfassen* (mit dem Hinweis Meta+N) · *Sprachnotiz
   aufnehmen* (grau) — Trennstrich — *Bibliothek öffnen* · *Jetzt analysieren*
   (grau) · *Vorschläge* (grau) — Trennstrich — *Beenden*. Alles auf Deutsch,
   jeder Eintrag mit einem Symbol. Bitte **von beiden Menüs ein Foto**, und
   sagen Sie uns dabei, ob in Ihren KDE-Einstellungen „Symbole in Menüs
   anzeigen“ an oder aus ist. *(schließt O1)*
2. **Meta+N drücken.** Das Erfassungsfenster soll kommen — das Kürzel wurde im
   Sprint umgehängt, deshalb steht es hier. *(prüft AK 4 von #60)*
3. **Bibliothek öffnen, eine Notiz anklicken, F2 drücken, etwas ändern, auf
   „Speichern“ klicken.** Zwei Dinge zum Hinsehen: Springt der Notiztext beim
   Umschalten zwischen Lesen und Bearbeiten *nicht* — steht die erste Zeile
   also still? Und sind die beiden Knöpfe „Bearbeiten“ und „Löschen“ oben
   rechts **so breit wie ihre Beschriftung** und nicht künstlich in die Breite
   gezogen, auch wenn Sie das Fenster groß ziehen? *(schließt O2 und M3)*
4. **Nochmals F2, etwas tippen, dann eine andere Notiz anklicken.** Es muss
   eine Nachfrage kommen: *Speichern · Verwerfen · Abbrechen*. Bitte schauen
   Sie, ob diese drei Knöpfe **Symbole tragen** (Diskette, Papierkorb-artig,
   Kreuz) oder ob sie nur beschriftet sind — davon hängt ab, ob wir hier noch
   eine Zeile ändern. *(schließt O3)*
5. **„Abbrechen“ in dieser Nachfrage drücken.** Sie sollen im Bearbeiten-Feld
   bleiben, mit Ihrer Änderung. Nichts darf ungefragt gespeichert oder
   verworfen werden.

Was Sie **nicht** prüfen müssen: die KI-Modelle. Sie liegen auf dem Rechner
und antworten; sichtbar wird davon erst im nächsten Sprint etwas.

### 15.11 done / next

**done:** Takt 1 des Sprint-Abschlusses (B11) vollständig geführt, jeder Punkt
mit eigener Messung statt mit einem übernommenen Bericht — Neubau ohne
Warnung (32 Übersetzungseinheiten, null `warning:`), 7 von 7 Tests, zwölf
Geometrie-Zusicherungen bei zwei Fenstergrößen am Testcode nachgezählt, das
installierte Abbild als bytegleich nachgewiesen, das ausgelieferte Tray-Menü
per `GetLayout` und die Kürzelkette per D-Bus am laufenden Dienst selbst
abgefragt (und alle Komponenten der Sitzung auf eine zweite Meta+N-Bindung
durchgesehen), `/api/chat` und `/api/embed` selbst gemessen; Berichts-
Vollzähligkeit über den Prüfweg aus B11 geführt; DoD 1–4 je Story und der
Doku-Abgleich nach B10 abgeschlossen; **drei Mängel** (M1 Belegmangel an #12,
M2 nicht nachgezogene Bedingung „Plattform-Ersatzdialog“, M3 Prüferinnenbilder
vor dem gelieferten Stand) und **drei offene Punkte** für die Kundenabnahme
benannt; das Sprint-Konto mit 3 von 4 Issues und 11 von ~13 SP geschlossen,
ohne Zugang nach der Freigabe; der **PR-Probelauf gegen sein vorab
festgelegtes Kriterium als nicht bestanden bewertet**, mit dem Vermerk, dass
er nur zur Hälfte durchgeführt wurde.

**next:** (1) M1–M3 an den PO — melden, nicht heilen. (2) Kundenabnahme nach
der Checkliste in 15.10; O1–O3 schließen sich dort. (3) Danach Takt 2 (B11,
Punkte 5–11): AK-Haken durch den PO, Issues und Milestone schließen, Journal,
Push, Zweige und Worktrees **auch auf `origin`** entfernen, Changelog
fortschreiben; Punkt 10 (Version und Tag) bleibt **ausgesetzt**, solange #61
offen ist — der Vollzugsvermerk führt ihn als solchen. (4) Kundenentscheidung
zum PR-Probelauf: Ende nach der Regel, oder Wiederholung mit vollständigem
Verfahren; je nach Ausgang zieht `PROZESS.md` nach. (5) **Für die Retro nach
Sprint 6 vorgemerkt:** die zu enge B13-Dateimengen-Notation — beide Stränge
mussten regelwidrig außerhalb ihrer Dateimenge schreiben, um ihren Beleg- und
Prüfmittelpflichten nachzukommen (`docs/scrum/reviews/<story>/`, Bildläufer,
CMake-Verdrahtung); ohne Ergänzung der Vorlage ist künftig jede Story mit
Bildpflicht formal im Verstoß (karpathy 3.1, Vorschlag 4).

## 16. Kundenabnahme (PO-Vermerk, 02.08.2026, 15:04)

Der Kunde hat die fünf Handgriffe der Abnahme-Checkliste (§15.10) am
installierten Stand ausgeführt und abgenommen: **„Sieht gut aus."**
Nachgereicht werden auf PO-Bitte: die zwei Panel-Fotos (O1, zugleich
Foto-AK von #60 und Schließung von M3) und die Antwort auf O3
(Symbole der drei Wächterdialog-Knöpfe — entscheidet die
`KMessageBox`-Frage aus S8-Befund 5). M1 und M2 waren zur Abnahme
bereits geschlossen (`d46265c`, #12-Kommentar).

Damit beginnt Takt 2; erster Einsatz des Verwalters (Kundenentscheidung
vom Vormittag): Issues #11 und #12 schließen als Vollzug dieser Abnahme,
Zweige und Worktrees räumen, Changelog-Rohliste. #60 schließt, sobald
die Fotos vorliegen.
