# Sprint 3 — Planning-Protokoll

**Datum:** 2026-08-01, 21:40 (Ganymed)
**Moderation:** Scrum Master (Agent `scrum-master`)
**Teilnehmer:** Scrum Master · Product Owner (Claude Haupt-Session) ·
Schätzer Dev und Schätzer UI/UX (unabhängige Schätzklausur zu #46, #44, #50,
#47 am 01.08.2026) · UI/UX (Agent `denkzettel-ux`, Grooming-Beratung zu #46
und Wireframes 3a/3b).
**Status des Sprint-Vorschlags:** vorgelegt, Freigabe durch den Kunden steht aus.

Grundlagen: `docs/scrum/PROZESS.md` (Stand nach den Beschlüssen B1–B10 der
Sprint-2-Retro), `docs/scrum/sprints/sprint-02.md` (Abschnitte 2, 4.4, 5.4, 9),
`SPEC.md` (Stand `1a308d3`), `wireframes/Denkzettel Wireframes.dc.html`
(Ansichten 2b, 2c, 3a, 3b), GitHub Issues #8, #9, #10, #11, #44, #46, #47, #50.
Quellstand der Prüfungen: `main` @ `daf2000`.

Alle Aussagen dieses Protokolls über Konflikte, Abhängigkeiten und Risiken sind
am Quellcode geführt, nicht an den Story-Texten. Wo eine Zeilennummer steht, ist
sie am Stand `daf2000` nachgezählt.

## 1. Schätzstand

Die Schätzung liegt vor; sie wurde im Grooming am 01.08.2026 von zwei
unabhängigen Schätzern erhoben, die Labels sind gesetzt. Der Scrum Master
konsolidiert hier nichts neu, sondern hält den Stand fest und benennt die eine
Zahl, die kippen kann.

| Issue | Story | SP | Zustand |
|---|---|---|---|
| #46 | S5a Posteingangs-Gliederung | 5 | ziehbar (Wireframes 3a/3b abgenommen) |
| #8 | S6 Volltextsuche (FTS5) | 3 | ziehbar; bringt Schemaversion 2 |
| #9 | T3 Migrationstest 1→2 | 1 | zieht mit #8 |
| #10 | S7 Suchoperator-Parser | 3 | setzt #8 voraus |
| #11 | S8 Bearbeiten-Ansicht | 2 | ziehbar, beide Bedingungen aus Sprint-02 4.4 erfüllt |
| #44 | Tray-Linksklick öffnet Menü | 1 | ziehbar |
| #50 | T8 Spike Fenstertitel (Wayland) | 3 | ziehbar, reiner Spike |
| #47 | S29 Kontext-Stempel | 5 | **blockiert** (Spike offen, kein Einstellungsdialog) |

**Keine 13er-Story im Feld** — nichts ist teilungsbedürftig.

**Die 5 von #46 hält nur mit einer AK-Präzisierung.** Beide Schätzer haben
unabhängig denselben Punkt benannt, und er ist am Code belegt: Der Zeitstempel
wird heute an zwei Stellen aus der Systemuhr gerechnet —
`src/ui/notelistmodel.cpp:81` und `src/ui/librarywindow.cpp:347`, jeweils
`QDateTime::currentDateTime()` als Argument von `library::relativeTimestamp`.
Die Gruppenzuordnung braucht denselben Bezugszeitpunkt. Liest das Modell ihn
selbst aus der Uhr, sind die geforderten Grenzfälle („an einem Montag steht der
Sonntag unter Gestern") nur über Uhrmanipulation prüfbar, und das erste
Akzeptanzkriterium von #46 verlangt ausdrücklich das Gegenteil. Der Dev merkt
das erst, wenn die halbe Story steht — dann kostet der Umbau die Stufe auf 8.

**Empfehlung: ja, vor dem Ziehen ins AK heben.** Vorschlag für den Wortlaut,
als eigener Punkt in #46:

> - [ ] Der Bezugszeitpunkt der Gruppierung und des Eintrags-Zeitstempels wird
>   von außen gesetzt (Fenster → Modell), nicht im Modell aus der Uhr gelesen;
>   `QDateTime::currentDateTime()` steht nach der Story an keiner Stelle in
>   `src/ui/notelistmodel.cpp`. Das Fenster setzt ihn bei jedem Neuaufbau der
>   Liste und beim Aktivieren des Fensters.

Das kostet eine Zeile im Issue und ist die billigste verfügbare Absicherung
gegen die einzige Abrutschstelle des Sprints.

**Ein Buchführungsfehler nebenbei:** #46, #44 und #50 tragen die Labels `sp:5`,
`sp:1`, `sp:3`, im Issue-Text steht aber weiterhin „Schätzung: offen — Team
schätzt im Sprint-3-Planning". Da die Issues die einzige Quelle der Wahrheit
sind, widersprechen sie sich derzeit selbst. Angleichen durch den PO (7.1).

## 2. Konfliktanalyse am Code

Der Auftrag lautete, die Konfliktzonen selbst am Code zu prüfen statt die
Warnung der Schätzer zu übernehmen. Ergebnis: **Die Warnung trifft im Kern zu,
zeigt aber auf die falschen Dateien.** Das verändert den Schnitt.

### 2.1 Was welche Story anfasst

| Story | Dateien | Beleg |
|---|---|---|
| #46 S5a | `ui/notelistmodel.*`, `ui/notelistdelegate.*`, `ui/timestampformat.*`, `ui/librarywindow.cpp` (Liste, Auswahl, Löschen/Undo), `tests/librarytest.cpp`, `src/CMakeLists.txt:25–32` | AK-Liste #46; `timestampformat.cpp:11` `WeekdayFormLimit = 7` entfällt |
| #8 S6, Datenweg | `store/store.cpp` (Migrationsliste `:19–47`, `migrate()` `:179–230`, Abfrage `notes()` `:327–344`), `store/store.h`, `tests/storetest.cpp` | FTS5-Tabelle + Trigger sind Schemaversion 2 |
| #8 S6, Ansicht | `ui/librarywindow.cpp` — `buildHeader()` `:220–223` (`search->setEnabled(false)`), `reload()` `:317`, `m_listPages` `:121–127` (Leerzustand „Keine Treffer"), `tests/librarytest.cpp:697` | Suchfeld verdrahten, Trefferliste einspeisen |
| #9 T3 | `tests/storetest.cpp` | reiner Store-Test |
| #44 | `shell/trayicon.cpp:21–44`, `tests/shelltest.cpp` | `setIsMenu(true)` neben `setContextMenu()` |
| #50 T8 | kein Produktivcode | Ergebnis ist ein Kommentar an #47 |

### 2.2 Der Konflikt sitzt nicht, wo die Warnung ihn vermutet

Gewarnt wurde vor `notelistmodel.cpp`, `notelistdelegate.cpp` und
`librarywindow.cpp` als gemeinsamer Fläche von #46 und #8. Nachgeprüft:

- **`notelistmodel.cpp` und `notelistdelegate.cpp` fasst #8 gar nicht an.** Die
  Trefferliste kommt über dieselbe Schnittstelle in die Ansicht wie die
  Gesamtliste — `librarywindow.cpp:317` ruft `m_model->setNotes(...)`, und ob
  das Argument `m_store->notes()` oder eine Suchabfrage ist, sieht das Modell
  nicht. Genau das fordert auch das letzte Akzeptanzkriterium von #46: „Die
  Gliederung sitzt in der Listenansicht (Modell und Delegate), nicht im
  Datenweg." Ist sie dort, erbt die Suche sie ohne eine Zeile eigener Arbeit.
- **`librarywindow.cpp` ist die echte gemeinsame Fläche**, und zwar an zwei
  Stellen: `reload()` (`:310–325`) — #46 ändert dort die Auswahlwiederherstellung
  und setzt den Bezugszeitpunkt, #8 ersetzt die Datenquelle — sowie die
  Leerseiten-Verwaltung `m_listPages` (`:121–127`, `updatePages()` `:327–339`),
  in die #8 eine zusätzliche Seite „Keine Treffer" einhängt, während #46 die
  Auswahllogik darüber umbaut.
- **`tests/librarytest.cpp` ist die zweite gemeinsame Fläche**, und die
  unangenehmere: 983 Zeilen, eine einzige Datei für Zeitstempel, Elision,
  Modell, Löschverzögerung und Fenster. #46 schreibt darin
  `usesTheWeekdayFormWithinTheLastSevenDays` (`:174`) und
  `usesTheAbsoluteDateBeyondSevenDays` (`:185`) um, dazu die Auswahl- und
  Geometriefunktionen; #8 muss `showsTheSearchFieldWithoutItsFunction` (`:697`)
  ersetzen — der Test heißt wörtlich „zeigt das Suchfeld ohne seine Funktion".

**Daraus folgt ein feinerer Schnitt als „#46 vor #8".** Der Datenweg von #8
(Migration, FTS5, Trigger, Suchabfrage) und #9 berühren keine einzige Datei von
#46 und können vom ersten Tag an parallel laufen. Nur der Ansichts-Teil von #8
muss hinter #46 warten. Die Story #8 wird dabei **nicht geteilt** — sie behält
ihre Akzeptanzkriterien und wird als eine Story abgenommen; geteilt wird nur die
Arbeitsreihenfolge innerhalb des Sprints.

### 2.3 Weitere geprüfte Flächen

- **`src/CMakeLists.txt`**: #46 ergänzt gegebenenfalls den Block `denkzettelui`
  (`:25–32`), ein späteres S7 den Block `denkzettelstore` (`:1–3`). Getrennte
  Blöcke, für Git ein gewöhnlicher Merge.
- **#44 ist vollständig disjunkt** — `shell/trayicon.cpp` und `tests/shelltest.cpp`
  werden von keiner anderen Kandidatenstory berührt.
- **#11 S8 würde kollidieren.** Die Bearbeiten-Ansicht fasst `buildDetail()`
  (`librarywindow.cpp:241–265`), `showNote()` (`:341–356`) und `closeEvent()`
  (`:296–308`) an — `showNote()` und `closeEvent()` ändert #46 ebenfalls. Dazu
  käme ein zweiter UI-Review mit eigener Bildprüfung im selben Sprint. Siehe 3.4.

## 3. Sprint-3-Vorschlag

**Sprint-Ziel:** Die Bibliothek liest sich wie ein Posteingang — Notizen stehen
unter Heute · Gestern · Diese Woche · Letzte Woche · Älter mit Betreff und
Vorschau —, das Suchfeld findet Notizen im Volltext samt Umlaut-Toleranz
(„bucher" findet „Bücher"), und ein Linksklick auf das Tray-Icon öffnet das Menü.

Nachprüfbar in drei Handgriffen ohne Werkzeug: Fenster öffnen und die Gruppen
sehen; „bucher" tippen und eine Notiz mit „Bücher" finden; links aufs Tray-Icon
klicken und das Menü sehen.

| Strang | Reihenfolge | ID | Issue | Story | SP |
|---|---|---|---|---|---|
| A | 1 | S5a | #46 | Notizliste als Posteingang gliedern | 5 |
| B | 1 | S6 | #8 | Volltextsuche (FTS5) — Datenweg | (3) |
| B | 2 | T3 | #9 | Migrationstest Schemaversion 1→2 | 1 |
| B | 3 | S6 | #8 | Volltextsuche — Suchfeld und Trefferliste | 3 gesamt |
| C | 1 | — | #44 | Tray-Linksklick öffnet das Menü | 1 |
| | | | | **Summe** | **10** |

Vier Issues, 10 Story Points — innerhalb beider Grenzen aus PROZESS.md
(2–4 Stories, max. ~13 SP) und genau auf der Kapazität von Sprint 2 (10 SP;
Sprint 1: 13 SP).

### 3.1 Die drei Stränge und ihre eine Synchronisationsstelle

- **Strang A — #46 (5 SP).** Beginnt sofort. Nach dem Merge ist die Fläche
  `librarywindow.cpp` / `librarytest.cpp` für Strang B frei.
- **Strang B — #8 + #9 (4 SP).** Beginnt sofort mit dem Datenweg: Migration auf
  Schemaversion 2, FTS5-Tabelle `notes_fts(content)` mit Triggern, Tokenizer
  `unicode61 remove_diacritics 2`, Suchabfrage im Store; danach #9 als Test der
  Migration. **Die dritte Etappe — Suchfeld verdrahten, Trefferliste einspeisen,
  Leerzustand „Keine Treffer" — beginnt erst nach dem Merge von Strang A.** Das
  ist die einzige Wartestelle im Sprint.
- **Strang C — #44 (1 SP).** Beginnt sofort, ist unabhängig von allem, endet
  früh. Der Nachweis am Panel wandert danach zum Kunden (Abschnitt 5).

**Empfohlene Zahl der Dev-Agenten: drei.** Sie folgt aus den Abhängigkeiten,
nicht aus der Erlaubnis: Es gibt genau drei disjunkte Dateimengen
(`ui/*` + `librarytest`; `store/*` + `storetest`; `shell/trayicon` + `shelltest`).
Ein vierter Agent hätte in diesem Schnitt keine konfliktfreie Fläche mehr — er
müsste entweder in einer bereits belegten Datei arbeiten oder #50 ziehen, das
nicht auf das Sprint-Ziel einzahlt (3.3). Vier sind erlaubt, drei sind begründbar.

Kommt Strang C früh zum Ende, kann sein Agent die Wartezeit von Strang B nicht
verkürzen — die Wartestelle hängt an Strang A, nicht an fehlenden Händen.

### 3.2 Warum dieser Kern und nicht der aus Sprint-02, 5.4

Das Sprint-2-Protokoll (5.4) hatte S6 + T3 + S7 (7 SP), gegebenenfalls plus S8,
als natürlichen Sprint-3-Kern benannt. Diese Empfehlung ist durch #46 überholt:
Die Gliederung ist ein Kundenwunsch aus der Sprint-2-Abnahme, sie hat eine
abgenommene Zeichnung (Wireframes 3a/3b), und sie steht seit `1a308d3` in SPEC 9.
**Vor allem aber ist ihre Reihenfolge gegenüber S6 nicht frei**: Wird die
Trefferliste gegen die flache Liste gebaut, ist sie danach ein zweites Mal
anzufassen — die Doppelarbeit steckt in keiner der beiden Schätzungen.

S7 (#10) bleibt draußen, weil es strikt hinter dem vollständigen S6 liegt (AK 2:
„Suche in der Bibliothek nutzt den Parser") und dieses erst nach der Wartestelle
fertig wird. Ein Strang aus 5 → 4 → 3 SP hintereinander wäre der ganze Sprint an
einer Kette.

### 3.3 Was bewusst draußen bleibt

- **#50 (T8, Spike Fenstertitel, 3 SP).** Er zahlt auf das Sprint-Ziel nicht ein
  — sein Ergebnis entblockt #47 in M5. Zweitens braucht er die laufende
  Plasma-Sitzung des Kunden (KWin-Skripte laden, Fokuswechsel provozieren) und
  stört damit genau die Sitzung, in der parallel drei Devs bauen und installieren.
  Er läuft besser allein. Drittens stuft der PO Spikes nach PROZESS.md je Spawn
  auf Fable hoch — das ist eine Kostenfrage, die der Kunde entscheidet, nicht der
  Schnitt. **Empfehlung: eigener Lauf nach diesem Sprint oder Sprint 4.**
  Aufnahme in Sprint 3 wäre nur als fünftes Issue möglich (13 SP) und bräche die
  Grenze „2–4 Stories" — Kundenentscheidung, siehe 9.
- **#10 (S7, 3 SP)** — hinter der Wartestelle, siehe 3.2.
- **#11 (S8, 2 SP).** Beide Bedingungen aus Sprint-02 4.4 sind erfüllt: Wireframe
  2a existiert, die AK sind neu gefasst und AK 2 ist nach S18a herausgelöst. Die
  Story ist ziehbar — aber sie kollidiert mit #46 in `showNote()` und
  `closeEvent()` (2.3), hängt zusätzlich an der FTS-Aktualität aus #8 und wäre
  die **zweite** UI-Story mit eigener Bildprüfung im selben Sprint-Ende. Sie ist
  der erste Kandidat für Sprint 4, dann konfliktfrei.
- **#47 (S29)** bleibt blockiert (Spike offen, kein Einstellungsdialog).

### 3.4 Warum nicht auf 13 SP auffüllen

Die freien 3 SP sind kein Leerlauf, sondern der Prüfaufwand, den Sprint 2 gelehrt
hat: der UI-Review von #46 mit **eigener** Bildprüfung durch `denkzettel-ux`
(DoD 3 / B3) über sechs Zustände — Normalfall 3a, die vier Sonderfälle aus 3b,
der Leerzustand 2c; die Geometrie-Zusicherungen nach B2 bei zwei Fenstergrößen;
die Selbst-Sichtprüfungen der drei Devs am **installierten** Stand (B4), die
nacheinander laufen müssen (6.4); und der Umbau von `tests/librarytest.cpp` mit
983 Zeilen, der in keiner der vier Zahlen steckt. In Sprint 2 hat allein der
UI-Review drei Auflagen erzeugt (`54249e0`).

## 4. UI-Story-Einstufung und Review-Aufträge

Die Einstufung trifft der PO (DoD 3, Kundenentscheidung 31.07.2026). Der Scrum
Master legt die Sachlage vor und formuliert die Aufträge; den Aufruf führt der PO.

### 4.1 #46 ist eine UI-Story — unstrittig

Sie hat eine abgenommene Zeichnung (3a/3b), elf Akzeptanzkriterien, davon eines
rein geometrisch. Volle Kette nach DoD 3 und B3.

**Empfehlung zur Taktung: den UI-Review von #46 vorziehen**, unmittelbar nach
dem Merge von Strang A statt am Sprint-Ende. Grund: Die dritte Etappe von
Strang B setzt auf der gelieferten Liste auf. Eine Auflage, die erst am
Sprint-Ende kommt, träfe dann beide Stories und beide Devs.

### 4.2 #8 hat einen sichtbaren Anteil ohne Zeichnung — Entscheidung nötig

Ein Suchlauf über `wireframes/Denkzettel Wireframes.dc.html` findet **keinen**
gezeichneten Zustand „Keine Treffer" (0 Treffer). Ansicht 2c kennt zwei
Leerzustände — leere Bibliothek und keine Auswahl —, die Suche ohne Treffer ist
ein dritter. #46 hält das ausdrücklich fest: „S6 bekommt zusätzlich ein eigenes
AK für den Leerzustand ‚Keine Treffer' — den erbt es nicht, weil 2c ihn nicht
kennt." **In #8 steht dieses AK bis heute nicht.**

Das ist die Lage, an der S8 in Sprint 2 das Verdikt `fail` bekam: sichtbarer
Anteil ohne Referenz. Zwei gangbare Wege, beide klein:

1. **#8 als Nicht-UI-Story einstufen** und den Leerzustand textlich im AK
   festlegen. Das ist vertretbar, weil die Bauart bereits existiert —
   `placeholderPage(titel, hinweis, withIcon)` in `librarywindow.cpp:72–98`, im
   `QStackedWidget` `m_listPages`. Es entsteht kein neues Gestaltungsproblem,
   sondern eine dritte Seite derselben Machart. Vorschlag für den AK-Text:
   *„Eine Suche ohne Treffer zeigt in der Listenspalte den Leerzustand ‚Keine
   Treffer' mit dem Hinweis, den Suchbegriff zu ändern — dieselbe Machart wie
   die Leerzustände aus 2c, ohne Icon; der Detailbereich bleibt leer. Ein
   geleertes Suchfeld stellt die vollständige Liste wieder her."*
2. **Zeichnen lassen** — kleiner Ergänzungsauftrag an `denkzettel-ux` vor dem
   Sprintstart. Kostet eine Runde vor dem Ziehen.

**Empfehlung: Weg 1.** Der Zustand besteht aus zwei Textzeilen in einer
vorhandenen Konstruktion; eine eigene Zeichnung dafür wäre Aufwand ohne
Erkenntnis. Die Entscheidung gehört trotzdem ausdrücklich getroffen und im Issue
festgehalten, sonst steht sie am Sprint-Ende als Befund im Review.

### 4.3 #44 ist keine UI-Story im Sinne der DoD

Es ändert kein Layout und keine Ansicht, sondern eine Eigenschaft des
Tray-Objekts. Ein UI-Review nach B3 mit Bildprüfung gegen einen Wireframe hat
hier nichts zu prüfen. Die Sichtprüfung am Panel bleibt (Abschnitt 5).
Zusätzlich hält #44 ausdrücklich fest, dass die Abweichung vom KDE-Standard eine
dokumentierte Kundenentscheidung ist und **bei künftigen HIG-Reviews kein Befund**.

### 4.4 Review-Auftrag #46 (Entwurf des Scrum Masters, Aufruf durch den PO)

Fällig nach dem Merge von Strang A.

> **Was geändert wurde:** Die Notizliste der Bibliothek ist wie ein Posteingang
> gegliedert (Issue #46, S5a) — Gruppen „Heute · Gestern · Diese Woche · Letzte
> Woche · Älter" mit nicht auswählbaren Köpfen, erste Zeile als Betreff,
> Folgetext als gedämpfte Vorschau, Zeitstempelform je Gruppe. Die
> Sieben-Tage-Regel ist durch die Kalenderwoche ersetzt; `WeekdayFormLimit`
> entfällt. Fenster, Kopfzeile, Meldungszeile und Detailbereich sind unverändert.
>
> **Worauf zu schauen ist:**
> 1. Abgleich gegen Wireframe **3a** (Normalfall samt Maßskizze) und **3b**
>    (vier Sonderfälle: eine Gruppe mit einem Eintrag, lange erste Zeile,
>    Einzeiler, Scrollen) sowie die Festlegungstafel zu 3a/3b. Jeder gezeichnete
>    Bereich erzeugt genau eine Prüffrage (B3).
> 2. Eigene Bilder aus dem Sprint-Stand für sechs Zustände: 3a-Normalfall, die
>    vier Sonderfälle aus 3b, der Leerzustand aus 2c. Ein Review ohne eigene
>    Bildprüfung zählt für DoD 3 nicht.
> 3. Die Raumaufteilung aus 2b/2c gilt unverändert weiter — Kopfzeile unter
>    40 px, Liste über 450 px bei 900×600, Listenbreite 300 px. Prüfen, ob die
>    Geometrie-Zusicherungen nach B2 diese Sätze tatsächlich abbilden und nicht
>    nur danebenstehen.
> 4. Die bewussten Auslassungen: kein „In Gruppen anzeigen"-Umschalter, keine
>    einklappbaren Köpfe, kein Zähler am Kopf. Gebaut zu haben, was ausdrücklich
>    nicht gebaut werden sollte, ist ein Befund.
> 5. Tastaturwege: Pfeiltasten laufen lückenlos über Gruppengrenzen, Entf und F2
>    greifen nie einen Kopf, beim Sprung über eine Grenze ist der neue Kopf im
>    Bild. KDE HIG: Abstände, Farbrollen (`PlaceholderText`, `HighlightedText`),
>    kein Fettdruck im Eintrag.
> 6. Was die manuelle Checkliste für M2 aufnehmen sollte.
>
> **Ergebnisformat:** Befunde als `pass` / `fail` / `Hinweis`, jeder mit Bezug
> auf Wireframe, SPEC-Abschnitt oder HIG-Regel. Bericht und geprüfte Bilder nach
> `docs/scrum/reviews/` (B7). Offene `fail`-Befunde blockieren DoD 3.

### 4.5 Review-Auftrag karpathy-reviewer am Sprint-Ende (Entwurf)

> **Was geändert wurde:** Drei Stories über den Sprint-Diff — Posteingangs-
> Gliederung der Notizliste (#46), Volltextsuche mit FTS5 und der ersten realen
> Schema-Migration auf Version 2 samt Migrationstest (#8, #9), Tray-Linksklick
> (#44).
>
> **Worauf zu schauen ist:**
> 1. **Migration und Trigger** (`store/store.cpp`): Enthält Schemaversion 2 nur,
>    was M2 braucht? Entscheidung E2 lautet „das Schema wächst je Meilenstein";
>    in Sprint 1 war genau die Gegenrichtung ein Befund (drei `notes`-Spalten zu
>    früh). Halten die Trigger den Index bei Anlegen, Bearbeiten und Löschen
>    synchron — auch auf dem Löschweg mit Transaktion (`removeNote`, `:346–397`)?
> 2. **Simplicity First an der Gruppierung**: Ist die Bauart die einfachste, die
>    die AK trägt, oder eine Abstraktion auf Vorrat?
> 3. **Chirurgische Änderung**: Der Sprint fasst `librarywindow.cpp` und
>    `librarytest.cpp` aus zwei Strängen an. Trägt jede geänderte Zeile zu ihrer
>    Story bei, oder ist beim Merge Fremdes mitgekommen?
> 4. **Testaussage**: Prüfen die neuen Zusicherungen die Eigenschaft, um die es
>    geht? Der Kernbefund von Sprint 2 war ein grüner Test an der falschen
>    Eigenschaft.

## 5. Nachweise, die Agenten nicht führen können

Vorab benannt statt im Review entdeckt — das Muster aus Sprint-02, 3.3.

1. **#44, Linksklick am echten Panel.** In #44 dokumentiert („Am echten
   Plasma-Panel sichtgeprüft, Bild liegt bei"). Ein Agent kann das Setzen der
   Eigenschaft zurücklesen — `org.kde.StatusNotifierItem.ItemIsMenu` über D-Bus,
   das ist der Nachweis nach B5 — aber nicht klicken. **Der Klick ist Kundensache.**
   Der Dev liefert den Rücklese-Beleg, der Kunde den Klick.
2. **#46, Gesamteindruck der Gliederung.** Geometrie, Auswahlwege, Scrollen und
   Elision sind offscreen prüfbar; ob die Gliederung sich *liest* wie ein
   Posteingang, ist es nicht. Diese Prüfung gehört auf die M2-Checkliste und in
   die Kundensichtprüfung — sie ist der Grund, warum die Story überhaupt existiert.
3. **#8, Umlaut-Toleranz am realen Bestand.** Der Testnachweis läuft gegen
   Fixtures. Ob „bucher" in der Bibliothek des Kunden findet, was er sucht, sieht
   nur er. Die technische Voraussetzung ist geprüft (6.5).

Kein weiterer Nachweis in diesem Schnitt liegt außerhalb dessen, was Agenten
führen können.

## 6. Risiken, die diesen Schnitt kippen

**6.1 — Die Zeilensemantik des Modells (`ui/notelistmodel.cpp`, `librarywindow.cpp`).**
Das schwerste Risiko und die eigentliche Stelle, an der aus 5 SP acht werden.
Heute gilt „Modellzeile == Notiz", und die Löschwege rechnen damit:
`librarywindow.cpp:365–375` merkt sich `m_deletedRow` und ruft
`m_model->takeRow(row)`, `:381–384` fügt beim Undo an derselben Zeile wieder ein,
`notelistmodel.cpp:20–63` ist durchgehend zeilenindiziert. Werden Gruppenköpfe zu
Modellzeilen — die naheliegende Bauart —, gilt diese Gleichung nicht mehr, und
Löschen, Undo und Auswahlfortschaltung brechen gleichzeitig. **Vorkehrung:** Die
Bauart-Entscheidung (Köpfe als Modellzeilen / Proxy-Modell / vom Delegate
gezeichnete Trenner) fällt am Anfang der Story und steht mit Begründung im
Bericht, nicht am Ende. Die bestehenden Tests `movesTheSelectionToTheFollowingNote`
(`librarytest.cpp:487`), `movesTheSelectionBackwardsAfterTheLastNote` (`:506`) und
`keepsTheNoteWhenTheDeletionIsUndone` (`:363`) müssen weiter gelten.

**6.2 — AK-Lücke: Löschen und Undo an der Gruppengrenze.** #46 verlangt, die
Gruppierung werde „bei jedem Neuaufbau der Liste (Öffnen, Nachladen, Anlegen,
Löschen, Bearbeiten)" neu bestimmt. **Beim Löschen baut der Code die Liste aber
gar nicht neu** — `deleteCurrentNote()` nimmt eine Zeile heraus (`:369`), das
Undo setzt sie zurück (`:383`). Damit ist unbestimmt, was geschieht, wenn die
letzte Notiz einer Gruppe gelöscht wird: Der Kopf müsste verschwinden („Leere
Gruppe erzeugt keinen Kopf") und beim Undo zurückkommen. Kein AK sagt das.
**Vorkehrung:** Ein AK ergänzen (Vorschlag in 7.1) oder den Fall wenigstens in
den Umsetzungsauftrag schreiben. Ohne das entscheidet ihn der Dev still, und der
UI-Review findet ihn als Befund.

**6.3 — Die Wartestelle in Strang B wird zum Stau.** Zieht sich #46, steht die
dritte Etappe von #8 am Sprint-Ende unter Druck — und mit ihr das halbe
Sprint-Ziel. **Vorkehrung:** Der PO setzt für Strang A einen Meldepunkt („Modell
und Delegate stehen, Tests grün") und zieht die dritte Etappe von B nicht erst
nach dem vollständigen UI-Review, sondern nach dem Merge des Codes; Auflagen aus
dem Review trägt dann derselbe Dev nach.

**6.4 — Es gibt nur ein `/usr`, aber drei Devs.** DoD 2 in der Fassung nach B4
verlangt die Prüfung am installierten Stand (`-DCMAKE_INSTALL_PREFIX=/usr`).
Installieren zwei Devs annähernd gleichzeitig, prüft einer den Stand des anderen
— **exakt die Fehlerbauart von Sprint-2-Befund 1**: der falsche Prüfling. Das
Risiko ist neu und entsteht erst durch die Parallelarbeit. **Vorkehrung:** Die
Installation nach `/usr` und die anschließende Selbst-Sichtprüfung sind ein
exklusiver Abschnitt; der Dev meldet sie an, der PO taktet sie nacheinander.
Alternative wäre ein eigenes Präfix je Dev — die prüft dann aber gerade den
System-Desktop- und D-Bus-Pfad nicht, um dessentwillen B4 gefasst wurde.

**6.5 — FTS5 im Qt-Treiber: geprüft, kein Risiko.** Das Kernrisiko von #8 wäre
ein SQLite ohne FTS5. Am System nachgemessen: Das Qt-Plugin ist gegen die
System-Bibliothek gelinkt (`ldd /usr/lib/qt6/plugins/sqldrivers/libqsqlite.so`
→ `/usr/lib/libsqlite3.so.0`), und dieselbe Bibliothek legt eine
`fts5`-Virtualtabelle mit `tokenize="unicode61 remove_diacritics 2"` an und
findet damit „Bücher lesen" auf die Anfrage `bucher`. Der Kern des ersten
Akzeptanzkriteriums von #8 ist damit vor dem Sprint belegt.

**6.6 — Git-Hygiene bei drei Agenten.** Siehe Abschnitt 8 (I5).

## 7. Hinweise an den Product Owner

**7.1 — Vor dem Ziehen zu erledigen.**

1. **#46**: das AK zum durchgereichten Bezugszeitpunkt aufnehmen (Wortlaut in
   Abschnitt 1). Ohne diesen Punkt trägt die 5 nicht.
2. **#46**: das Verhalten von Löschen und Undo an der Gruppengrenze festlegen
   (6.2). Vorschlag: *„Wird die letzte Notiz einer Gruppe gelöscht, verschwindet
   deren Kopf; das Undo bringt Notiz und Kopf an derselben Stelle zurück. Die
   Auswahl folgt dabei den Regeln aus 2c (nachfolgende Notiz, sonst
   vorhergehende) und landet nie auf einem Kopf."*
3. **#8**: Einstufung als UI- oder Nicht-UI-Story treffen und den Leerzustand
   „Keine Treffer" als AK aufnehmen (4.2, Wortlaut dort).
4. **#46, #44, #50**: die Zeile „Schätzung: offen — Team schätzt im
   Sprint-3-Planning" durch den beschlossenen Wert ersetzen; die Labels stehen
   bereits (Abschnitt 1).
5. **Milestone „Sprint 3" anlegen** und #46, #8, #9, #44 zuordnen. Ein Sprint ist
   nach PROZESS.md ein Milestone mit den gezogenen Issues; „Sprint 1" und
   „Sprint 2" sind geschlossen, ein dritter existiert noch nicht.
6. **Worktree und Branch je Strang** in die Spawn-Aufträge schreiben — die
   Agentendatei kennt die Regel nicht (Abschnitt 8).

**7.2 — Zwei Nachträge zu #7 (geschlossen).** Das Akzeptanzkriterium 2 von #7
trägt weiterhin die Sieben-Tage-Fassung des Zeitstempels, die #46 ersetzt. SPEC 9
ist mit `1a308d3` bereits auf die Kalenderwoche umgestellt, der Wireframe trägt
die Überholt-Vermerke — nur das geschlossene Issue nicht. Da Issues die einzige
Quelle der Wahrheit sind, empfiehlt sich ein kurzer Kommentar an #7 („AK 2
überholt durch #46"), keine nachträgliche Änderung des AK-Textes.

**7.3 — Der Push steht aus.** `main` ist drei Commits vor `origin/main`
(`daf2000`, `1a308d3`, `ed511d6`). Pushen entscheidet der PO; vor drei parallel
arbeitenden Worktrees ist ein gemeinsamer, gepushter Ausgangsstand die billigere
Reihenfolge.

**7.4 — Erledigt, nur zur Buchführung:** Der Hinweis 5.3 aus Sprint 2 ist
abgearbeitet — #43 hängt jetzt am Milestone „Sprint 1". Ebenso der offene Punkt
(2) aus der Retro-Fortschreibung: Die Festlegungstafel der Wireframes trägt seit
`1a308d3` die Zeile „Raumaufteilung" mit Prüfsätzen; B2 hat damit seine
gezeichnete Referenz.

## 8. Impediment-Liste (Fortschreibung)

**I5 — Git-Hygiene bei parallel arbeitenden Agenten (offen, jetzt erstmals
prüfbar).** Das Impediment steht seit Sprint 2 offen „bis ein Sprint mit mehreren
Dev-Agenten ohne Vorfall gelaufen ist"; die Prüfung war für das Sprint-3-Ende
angesetzt. **Mit der Kundenerlaubnis für parallele Devs wird Sprint 3 zur ersten
echten Prüfgelegenheit** — in Sprint 1 und 2 lief nie mehr als ein Dev
gleichzeitig, die Gegenmaßnahme B6 ist also verankert, aber unerprobt.

Damit „ohne Vorfall" am Sprint-Ende feststellbar ist und nicht Ermessenssache
bleibt, legt der Scrum Master das Prüfkriterium **vorher** fest. Ein Vorfall
liegt vor, wenn eine dieser vier Prüfungen am Sprint-Diff anschlägt:

1. ein Commit trägt Dateien, die nicht zur Story seines Autors gehören
   (`git log --stat` gegen die Dateizuordnung aus 2.1);
2. ein Commit ist ein `--amend` auf einen fremden oder bereits gepushten Commit
   (Reflog);
3. Änderungen eines Strangs sind beim Merge verlorengegangen oder überschrieben
   worden;
4. `.claude/settings.json` oder andere unversionierte Arbeitsdateien sind in
   einen Story-Commit geraten (`git add -A`-Signatur).

**Empfohlene Vorkehrung — und der Punkt, an dem die vorhandene Regel nicht
reicht:** B6 verankert „gezielt stagen, nie `git add -A`, nie `--amend`" in
`.claude/agents/denkzettel-dev.md` (Zeilen 32–35). Diese Regeln verhindern, dass
ein Agent fremde Änderungen **mitkommittiert** — sie verhindern nicht, dass drei
Agenten im selben Arbeitsverzeichnis dieselbe Datei gleichzeitig bearbeiten und
einander überschreiben. Dagegen hilft nur Isolation. PROZESS.md nennt sie
(„bei paralleler Arbeit Worktree-Isolation"), **die Agentendatei nicht** — der
Dev-Agent erfährt sie also nur, wenn sie im Auftrag steht. Genau diese Bauart
(Regel lebt im Auftragstext statt in der Definition) war der Anlass für B6.

Empfehlung für Sprint 3, umzusetzen im Spawn-Auftrag:

- je Strang ein eigener Worktree und ein eigener Branch, etwa
  `git worktree add ../denkzettel-s5a story/46`, dazu `story/8`, `story/44`;
- gemeinsamer Ausgangsstand: gepushtes `main` (7.3);
- Merge nach `main` ausschließlich durch den PO, in der Reihenfolge
  A (#46) → C (#44) → B (#8, #9); B wird erst nach A gemergt, weil seine dritte
  Etappe auf A aufsetzt;
- ein Dev pusht nie (steht bereits in der Agentendatei).

Ob die Worktree-Regel dauerhaft in die Agentendatei gehört, ist ein Kandidat für
die Retro am Sprint-3-Ende — nicht für dieses Planning.

**I1** (Werkzeugkette unvollständig, betrifft M3/M4 — in diesem Sprint keine
Story berührt) und **I4** (automatisierte Prüfbarkeit UI-lastiger Stories)
bleiben offen. **I4 kann in diesem Sprint geschlossen werden**: #46 ist die erste
Story, die die Kette aus B1 (Selbst-Sichtprüfung), B2 (Geometrie-Zusicherungen)
und B3 (UI-Review mit eigener Bildprüfung) vollständig durchläuft. Läuft sie
durch, ist die Bedingung aus Sprint-02, 9.7 erfüllt.

**Retro-Kadenz:** Nach Sprint 3 ist die reguläre erste Retro fällig (PROZESS.md,
Retrospektiven). Die Retro am 01.08.2026 war eine vom Kunden angeordnete
Vorziehung und ersetzt sie nicht.

## 9. Was dem Kunden zur Entscheidung vorliegt

1. **Freigabe des Sprints.** Drei Dinge: Die Notizliste wird nach Tagen
   gegliedert wie ein Posteingang, mit der ersten Zeile als Betreff. Das
   Suchfeld, das bisher nur da war, funktioniert — auch wenn Umlaute fehlen
   („bucher" findet „Bücher"). Der Linksklick aufs Tray-Symbol öffnet das Menü.
   Zusammen 10 Punkte, so viel wie im letzten Sprint.
2. **Drei Entwickler statt vier.** Mehr Hände bringen hier nichts: Es gibt nur
   drei Arbeitsbereiche, die sich nicht überschneiden. Ein vierter müsste in
   einer Datei arbeiten, an der schon jemand sitzt — und genau daraus entstehen
   die Fehler, gegen die wir in der letzten Retro Regeln gefasst haben.
3. **Eine offene Frage: der Wayland-Spike (#50).** Er klärt, ob Denkzettel
   überhaupt mitschreiben kann, in welchem Programm ein Gedanke entstand — die
   Vorbedingung für den Kontext-Stempel. Er gehört fachlich nicht zu diesem
   Sprint und braucht Ihre laufende Sitzung, in der gleichzeitig drei
   Entwickler bauen. **Empfehlung: eigener Lauf danach.** Wollen Sie ihn
   dennoch in diesem Sprint, wären es fünf Arbeitspakete statt der vereinbarten
   höchstens vier — das ist Ihre Entscheidung, nicht unsere.
4. **Zwei Dinge müssen Sie selbst prüfen.** Den Linksklick aufs Tray-Symbol kann
   kein Agent ausführen. Und ob sich die neue Liste tatsächlich liest wie ein
   Posteingang, sehen nur Sie — dafür ist sie gebaut.
5. **Nicht in diesem Sprint:** das Bearbeiten von Notizen (#11) und die
   Suchoperatoren wie `tag:` oder `nach:` (#10). Beide sind vorbereitet und
   liegen als Erstes im nächsten Sprint; sie würden hier mit der neuen Liste
   kollidieren.

## 10. done / next

**done:** Sprint-3-Schnitt vorgeschlagen (4 Issues, 10 SP, drei Stränge) und am
Quellstand `daf2000` begründet; die Konfliktwarnung der Schätzer nachgeprüft und
präzisiert — sie trifft `librarywindow.cpp` und `librarytest.cpp`, nicht Modell
und Delegate, wodurch der Datenweg von #8 parallel zu #46 laufen kann;
Sprint-Ziel in einer von Hand nachprüfbaren Form formuliert; sechs Risiken
belegt, darunter zwei neue (Zeilensemantik des Modells bei Gruppenköpfen; die
Kollision dreier Devs am einen `/usr`-Präfix); FTS5 samt Umlaut-Toleranz vorab am
System belegt; UI-Einstufung für #8 zur Entscheidung vorgelegt, weil der
Leerzustand „Keine Treffer" keine gezeichnete Referenz hat; Review-Aufträge für
`denkzettel-ux` und `karpathy-reviewer` entworfen; I5 mit einem vorab
festgelegten Prüfkriterium versehen und die Lücke zwischen B6 und
Worktree-Isolation benannt.

**next:** (1) Kundenentscheidung über Freigabe, Entwicklerzahl und #50.
(2) PO-Aufgaben vor dem Ziehen: zwei AK-Ergänzungen an #46, AK und Einstufung an
#8, Schätzzeilen angleichen, Milestone „Sprint 3" anlegen, Worktree-Vorgabe in
die Spawn-Aufträge (7.1). (3) Push des Ausgangsstands vor dem Sprintstart (7.3).
(4) Nach dem Merge von Strang A: UI-Review #46 durch `denkzettel-ux`, Bericht und
Bilder nach `docs/scrum/reviews/` (B7). (5) Am Sprint-Ende: DoD-Prüfung
einschließlich Doku-Abgleich (B10), I5-Prüfung nach dem Kriterium aus Abschnitt 8,
I4-Schließung prüfen — und die reguläre Retrospektive nach Sprint 3.

## 11. Scope-Änderung während des Sprints (01.08.2026, 23:00)

**Was passiert ist.** Nach der Abgabe von Etappe 1 und 2 durch Strang B hat der
Kunde eine neue Anforderung gestellt: „Die Suche in Denkzettel muss eine fuzzy
Suche haben." Der PO hat den Begriff im Interview zerlegt, weil er vier
verschiedene Verhaltensweisen mit sehr unterschiedlichem Aufwand meinen kann.
Der Kunde wollte drei davon: Wortteile in der Mitte, Schreibvarianten,
Tippfehler — ausdrücklich **nicht** die sinnverwandte Suche (M5, S17).

**Entscheidung und Begründung.** Nur eine der drei ist in den laufenden Sprint
gewandert, und das aus einem Grund, der zeitlich gebunden war:

- **Wortteile in der Mitte → in #8 aufgenommen.** Der Tokenizer steht in der
  Migration, die Strang B gebaut, aber noch nicht abgeliefert hatte. Ein
  Wechsel auf `trigram` kostet dort wenige Zeilen; dieselbe Änderung nach dem
  Merge hätte eine zweite Migration samt eigenem Test verlangt. Der PO hat vor
  der Vorlage am System gemessen: `trigram` findet „grafieren" in
  „fotografieren", behält mit `remove_diacritics` die Umlaut-Toleranz und
  erzeugt bei 2000 Notizen **keinen größeren** Index.
- **Schreibvarianten → S30 (#51)**, bereits vor dieser Entscheidung angelegt.
- **Tippfehler → T9 (#52)**, Klärungsauftrag. Der einzige der drei Wünsche
  ohne Weg mit Bordmitteln; er berührt über eine mögliche SQLite-Erweiterung
  auch die Paketierung (S28, #41) und wird deshalb erst geklärt, dann
  geschnitten.

**Der Preis, benannt statt verschwiegen.** Der `trigram`-Tokenizer findet
prinzipbedingt nichts unter drei Zeichen: „KI" oder „PO" liefern künftig keine
Treffer. Das ist als Akzeptanzkriterium an Strang B gegeben — er entscheidet
den Umgang damit, begründet ihn und zieht SPEC 6 nach. **Eine Suche, die bei
kurzen Begriffen wortlos leer bleibt, wäre ein Fehler mit Ansage.**

**Offene Frage an den Sprint:** Ob die Erweiterung die 3 SP von #8 sprengt, hat
Strang B zu melden — mit Zahl und früh. Die Punktzahl des Sprints bleibt bis
dahin bei 10; eine Korrektur gehört in die DoD-Prüfung am Sprint-Ende.

**Für die Retro vorgemerkt.** Dies ist die erste Scope-Änderung nach einer
erteilten Sprint-Freigabe. Sie war fachlich richtig — der Zeitpunkt war die
billigste Gelegenheit, und Warten hätte doppelte Arbeit erzeugt. Sie zeigt
aber, dass der Prozess für diesen Fall keine Regel kennt: Wer entscheidet, was
eine Freigabe noch trägt, und ab welcher Größe ist es ein neuer Sprint? Bisher
hat der PO im Einzelfall abgewogen und den Kunden entscheiden lassen.

## 12. Beobachtungen für die Retrospektive (fortlaufend während des Sprints)

Gesammelt, während sie frisch waren, statt am Sprint-Ende rekonstruiert.

### 12.1 Vier Sätze zur Prüfbarkeit, aus konkreten Fehlern gewonnen

> **Bei Bewegungen ist der Weg der Prüfgegenstand, nicht das Ziel.**
> **Bei Zuständen ist das Bild der Prüfgegenstand, nicht die Zusicherung.**
> **Eine Vereinfachung ist erst geprüft, wenn sie gegen die zuletzt geheilten Fälle gehalten wurde.**
> **Eine Begründung, die nicht trägt, fällt auf; eine, die trägt und trotzdem den falschen Schluss stützt, nicht.**

Der erste Satz stammt aus drei Scroll-Fehlern desselben Bauart-Typs: Das Bild
*nach* dem Tastendruck war jedes Mal richtig, falsch war die Bewegung dorthin.
Der Entwickler ist zweimal in dieselbe Falle gelaufen und hat sie beim zweiten
Mal selbst gefunden — durch Messung des Rollwerts statt Vergleich der
Endbilder. Der UI-Review fand den dritten Fall mit derselben Technik.

Der zweite Satz ist seine Umkehrung und stammt aus Sprint 2: Das kaputte
Bibliotheks-Layout war in keinem Test zu sehen, nur im Bild. Zusammen sind
beide die Begründung dafür, warum die Bildprüfung aus Beschluss B3 **neben**
den Tests steht und nicht durch sie zu ersetzen ist.

Der dritte und vierte Satz stammen aus zwei Beinahe-Fehlern des PO (12.2).

### 12.2 Zwei Fehlentscheidungen des PO, beide aus schlüssigen Begründungen

**Erster Fall — AK 7.** Der Entwickler meldete eine Auslegungsfrage statt sie
still zu entscheiden, und begründete seine Lesart mit Wireframe 3b Fall 4. Der
PO stimmte zu. Der UI-Review widerlegte beides: Der dort ohne Überschrift
gezeichnete Eintrag ist **nicht die Auswahl**. Eine Berufung auf die Zeichnung,
die die Zeichnung nicht gelesen hatte — und eine Zustimmung, die den Grenzfall
nicht gesucht hat, der die Lesart kippt.

**Zweiter Fall — die Zusatzbedingung.** Nach dem Fund des Rad-Sprungs empfahl
der Reviewer, nur noch vorzuscrollen, wenn die Auswahl nicht ohnehin sichtbar
ist. Der PO entschied so. Der Reviewer maß nach und nahm die eigene Empfehlung
**zurück**: Im gerade geheilten Fall war die Zielzeile bereits vollständig
sichtbar, der Kopf lag 35 px darüber draußen — die neue Regel hätte ihn
draußen gelassen und damit den Befund von einer Stunde zuvor wiederhergestellt.
Die Korrektur erreichte den Entwickler, bevor er sie einbaute.

**Das Muster ist beide Male dasselbe und liegt beim PO:** Zustimmung zu einer
plausiblen Begründung, ohne den Fall zu suchen, der sie kippt. Beide Male war
die Begründung sachlich richtig und der Schluss daraus falsch. Gerettet hat
beide Male dieselbe Methode — nachmessen statt nachdenken.

**Gegenmaßnahme, die schon greift:** Der Prüfsatz „Springt die Auswahl über
eine Gruppengrenze auf eine schon vollständig sichtbare Zeile, muss der Kopf
danach im Bild sein" ist in die Testauflage gewandert. Ohne ihn hätte jede
spätere Vereinfachung wieder wie eine Verbesserung ausgesehen.

### 12.3 Der Sprint ist um 30 % gewachsen

Von 10 auf 13 SP, damit an der oberen Grenze aus PROZESS.md. Drei Zugänge nach
der Kundenfreigabe, jeder einzeln entschieden und begründet: die Wortteil-Suche
(zeitgebunden — der Tokenizer stand in der noch nicht abgelieferten Migration),
der Capture-Farbfehler #54 (eine Zeile, trifft den Kundenwunsch wörtlich) und
die Aufwertung von #8 (3 → 5 SP nach gemeldetem Mehraufwand). **Jede für sich
begründet, in der Summe ein Drittel mehr Sprint.** Für die Retro: Der Prozess
kennt keine Regel dafür, was eine erteilte Freigabe noch trägt.

### 12.4 Bildläufe brauchen das Plattformthema

Ohne `QT_QPA_PLATFORMTHEME=kde` liefert `SmallestReadableFont` eine Ersatzschrift,
die **größer** ausfällt als die Eintragsschrift — die beabsichtigte Rangfolge
kehrt sich um. Wer so ein Bild beurteilt, beurteilt eine Schrift, die im Betrieb
nie erscheint. Gehört in die Bildlauf-Regel von B3. Der Entwickler hat die
Warnung inzwischen in den Kopf seines Bildprogramms geschrieben.

### 12.5 I5 (Git-Hygiene bei parallelen Agenten) — erster Prüflauf

Vier Agenten in getrennten Worktrees, 18 Commits, **keine Kollision**. Die
formale Bewertung nach dem Kriterium aus Abschnitt 8 gehört in die DoD-Prüfung.
Bemerkenswert ist, was die Parallelarbeit **neu** erzeugt hat: die Kollision am
einen `/usr`-Präfix (im Planning vorhergesehen, vom PO in den Spawn-Aufträgen
zunächst vergessen und nachträglich getaktet) und der gleichzeitige Zugriff
zweier UX-Agenten auf die Wireframe-Datei (ebenfalls getaktet). **Beide Risiken
entstehen erst durch Parallelität und wären bei einem einzelnen Dev nie
aufgefallen.**

### 12.6 Das Repository ist öffentlich — und niemand hat es benannt

`gh repo view` weist `hnsstrk/denkzettel` als **PUBLIC** aus. Jeder
Issue-Kommentar ist damit eine Veröffentlichung unter dem Namen des
Eigentümers. Am 01.08.2026 sind über dreißig Kommentare entstanden — Grooming,
Entscheidungen, Befunde, Prüfberichte —, ohne dass PO oder Agenten die
Sichtbarkeit je geprüft hätten.

**Aufgefallen ist es einem Prüf-Agenten**, nicht dem PO: Er holte sich vor
einem Issue-Kommentar die Freigabe über das Berechtigungssystem, statt sie aus
der fachlichen Beauftragung abzuleiten, und benannte den Grund.

**Kein Schaden entstanden.** Die Inhalte sind Projektarbeit; die einzigen
Systemangaben sind installierte KDE-Themes und Farbschemata. Aber die Prüfung
hat gefehlt, und sie hätte vor dem ersten Kommentar stattfinden müssen, nicht
nach dem einunddreißigsten.

**Für die Retro:** Der Prozess kennt keine Regel dafür, was in ein öffentliches
Repository geschrieben wird. Zu klären ist mindestens: Gehören wörtliche
Kundenzitate hinein? Messwerte vom Rechner des Kunden? Interne Fehleranalysen
des PO? Alles drei ist heute geschehen — vertretbar, aber unentschieden. Und
die Frage stellt sich schärfer, sobald ein Fremder das Repository liest, was
bei einem öffentlichen jederzeit möglich ist.

### 12.7 Die Story-Grenze hat niemand als Grenze wiedererkannt

Nachtrag zu 12.3, aus der DoD-Prüfung. Das Planning hat den Fall wörtlich
vorweggenommen (3.3): *„Aufnahme in Sprint 3 wäre nur als fünftes Issue möglich
(13 SP) und bräche die Grenze ‚2–4 Stories' — Kundenentscheidung, siehe 9."*
Die Schranke stand also da, aufgestellt für #50. Passiert hat sie #54 — und in
diesem Moment hat sie niemand wiedererkannt. Der Kunde hat #54 freigegeben; dass
er damit zugleich über die Story-Grenze entschied, ist ihm nicht vorgelegt
worden.

Bemerkenswert ist nicht der Bruch, sondern **warum er unbemerkt blieb**: Die
Punktzahl wurde bei jedem Zugang mitgezählt, die Zahl der Issues nicht. Wer 13
im Blick hat, sieht die 13 einhalten und übersieht die 5. Eine Grenze, die nur
im Planning-Text steht und in keiner laufenden Zählung, ist keine Grenze.

### 12.8 Die Worktrees lagen im Repository, ungeschützt

`.claude/worktrees/` liegt innerhalb des Arbeitsbaums und steht **nicht** in
`.gitignore` (dort nur `build/`, `compile_commands.json`, `.cache/`). Ein
einziges `git add -A` hätte vier vollständige Arbeitsbäume eingecheckt. Kein
Vorfall — die Regel B6 („gezielt stagen, nie `git add -A`") hat gehalten. Aber
sie war das Einzige, was zwischen dem Repository und diesem Fehler stand, und
sie ist eine Verhaltensregel, keine Sperre. Retro-Vorschlag: die Zeile in
`.gitignore` nachtragen; Verhaltensregeln durch Mechanik zu stützen ist billiger
als sie zu schärfen.

## 13. DoD-Prüfung am Sprint-Ende (Scrum Master, 01.08.2026, 23:59)

**Prüfstand:** `main` @ `47a1774`, alle Stränge gemergt. Geprüft wurde gegen
Code, Binärstand, Git-Historie und GitHub, nicht gegen Meldungen. Jede Zahl
unten ist in dieser Sitzung selbst gemessen.

**Gesamturteil: Der Sprint ist inhaltlich fertig und formal unfertig.** Die
Arbeit trägt — Build, Tests, Spezifikation und Prüfläufe sind in einem Zustand,
der über das hinausgeht, was die DoD verlangt. Offen sind **acht Mängel**, davon
einer schwer (M1). Sieben der acht sind Buchführung und in Minuten zu beheben;
M1 verlangt eine Handlung.

### 13.1 Die sechs Punkte

| # | DoD-Punkt | #46 | #8 | #9 | #44 | #54 |
|---|---|---|---|---|---|---|
| 1 | Build warnungsarm, Tests grün, Geometrie bei zwei Größen | erfüllt | erfüllt | erfüllt | erfüllt | erfüllt |
| 2 | AK erfüllt, PO-Abnahme, Selbstprüfung am **installierten** Stand | **M1, M2** | **M1, M2** | **M1, M2** | erfüllt | **M1, M2** |
| 3 | karpathy ohne `fail`; UI-Story zusätzlich UX ohne `fail` | erfüllt, **M3** | erfüllt | erfüllt | erfüllt | erfüllt |
| 4 | SPEC/KONZEPT nachgezogen | erfüllt | erfüllt | erfüllt | erfüllt | erfüllt |
| 5 | Commits; Issue geschlossen mit Commit-Verweis | **M5** | **M5, M6** | **M5** | **M5** | **M5** |
| 6 | Journal-Eintrag der Session | **M7** | **M7** | **M7** | **M7** | **M7** |

Doku-Abgleich nach B10: **M8** (README). SPEC, KONZEPT und Wireframes: kein
Befund — ausdrücklich festgehalten, damit „geprüft, nichts gefunden" von
„vergessen" unterscheidbar bleibt.

### 13.2 DoD 1 — erfüllt, mit eigener Messung

Ein **frisches, leeres** Build-Verzeichnis konfiguriert und vollständig gebaut
(`CMAKE_BUILD_TYPE=Debug`, 12 Jobs): Rückgabewert 0, **null Warnungen** über
alle elf Ziele. Das bestehende `build/` war aktuell und hätte nichts gezeigt —
ein Neubau war nötig, um die Aussage überhaupt führen zu können.

`ctest` mit `QT_QPA_PLATFORM=offscreen`: **7/7 grün** in 4,33 s.

Geometrie-Zusicherungen nach B2, offscreen bei zwei Fenstergrößen —
nachgezählt in `tests/librarytest.cpp`:
`keepsTheHeaderAtTheTopAndTheRestForTheNotes_data` und
`keepsTheMeasuresOfTheGroupedList_data` fahren je 900×600 und 1200×800;
`bringsTheHeadOfTheNewGroupIntoView_data` fährt 900×600 und „so flach wie
möglich" (900×150) — mit einer Begründung im Code, warum gerade die flachste
Größe die härtere Prüfung ist. Die Datei ist von 983 auf **2040 Zeilen** und 67
Testfunktionen gewachsen; der im Planning (3.4) benannte Umbau ist geleistet.

### 13.3 DoD 2 — M1: der Prüfling war der falsche

**M1 (schwer).** `/usr/bin/denkzetteld` datiert auf **01.08., 22:00** und
entspricht dem Merge von #44 (`2c1654c`, 22:00). Gemessen an den Zeichenketten
des Binärstands:

| Merkmal | installiert (`/usr`) | frischer HEAD-Build |
|---|---|---|
| „Keine Treffer" (#8) | **fehlt** | vorhanden |
| „trigram" (#8) | **fehlt** | vorhanden |
| „Diese Woche" / „Letzte Woche" (#46) | **fehlt** | vorhanden |
| „Älter" (#46) | **fehlt** | vorhanden |

Damit steht fest: **Von den fünf Issues ist allein #44 am installierten Stand
geprüft worden.** Für #46, #8, #9 und #54 kann keine Prüfung am installierten
Stand stattgefunden haben — der installierte Stand kennt ihren Code nicht. Die
Selbst-Sichtprüfungen liefen gegen die Build-Verzeichnisse der Worktrees.

Das ist genau Risiko **6.4** dieses Planning („Es gibt nur ein `/usr`, aber drei
Devs") und die Fehlerbauart von **Sprint-2-Befund 1**: der falsche Prüfling.
Vorhergesehen, benannt, mit einer Vorkehrung versehen — und trotzdem
eingetreten. Die Vorkehrung war eine Taktung durch den PO; sie wurde laut 12.5
zunächst vergessen und dann nur für die Reihenfolge nachgeholt, nicht für die
Frage, ob am Ende der aktuelle Stand installiert ist.

*Einordnung, damit der Mangel die richtige Größe behält:* Die betroffenen
Prüfungen sind fachlich nicht wertlos. Für offscreen-Bildläufe und
Geometriemessungen ist der Installationspfad ohne Belang; B4 wurde für den
Desktop- und D-Bus-Pfad gefasst (Autostart, Kürzel), den #46 und #8 nicht
berühren. **Der Mangel ist trotzdem einer**, weil DoD 2 den Satz ohne Vorbehalt
führt und die Gegenprobe fehlt: Ob die Bibliothek unter `/usr` startet und ihre
Datenbank migriert, ist an keinem Punkt dieses Sprints gezeigt worden — und die
Migration auf Schemaversion 2 ist die erste, die auf einen **Bestand** trifft.

**Empfehlung an den PO:** HEAD nach `/usr` installieren und den Hauptweg
einmal ausführen — Bibliothek öffnen, Gliederung sehen, „bucher" tippen. Das ist
zugleich der Stand, den der Kunde für seine Sichtprüfung braucht.

**M2.** Die Akzeptanzkriterien sind in **allen fünf Issues unangehakt**; #9 und
#54 haben null Kommentare. Die Abnahme des PO existiert damit in der einzigen
Quelle der Wahrheit nicht. Erfüllt ist dagegen der Nachweis nach B5 für #44: Der
Rücklese-Beleg zu `ItemIsMenu` über D-Bus liegt als Issue-Kommentar vor.

### 13.4 DoD 3 — erfüllt, aber der Beleg der Abnahme liegt außerhalb des Repos

**karpathy-Review** über `59d0d3f..HEAD`: Gesamt `warn`, kein `fail` — die
Bedingung ist erfüllt. Drei Befunde sind mit `e18630c` und `7787339` geheilt,
der vierte ist als **#59** im Backlog (angelegt 23:44). Ein `warn` blockiert
DoD 3 nicht.

**UI-Review #46** durch `denkzettel-ux`: ein `fail` an AK 7, geheilt (`7bc24ae`),
nachgeprüft, danach ein neuer `warn`, ebenfalls geheilt (`c1b43a9`), Abschlusslauf
**alle elf Szenen `ok`**. Die Zugabe zu #8 (Trefferliste, Leerzustand): kein
`fail`, kein `warn`. Vier Prüfakten mit eigenen Bildern des Reviewers — DoD 3 in
der Fassung nach B3 ist damit geführt.

**M3.** Der Beleg für die Abnahme liegt **unversioniert** im Arbeitsbaum: die 60
Zeilen „Abschlusslauf nach der Heilung des `warn`" in
`docs/scrum/reviews/sprint-03-s5a-ak7-nachpruefung.md` sind nicht committet, der
zugehörige Bilderordner `nach-der-warn-heilung/` ist untracked. **Genau der
Satz, auf dem die Erfüllung von DoD 3 ruht — „alle elf Szenen `ok`" —, steht
nicht im Repository.** Das ist der Fall, den B7 verhindern soll, und der PO hat
ihn um 23:30 selbst benannt („sie lagen unversioniert und wären mit den
Worktrees verschwunden"); für den letzten Lauf ist er noch offen.

**M4 (leicht).** Kein karpathy-Bericht als Datei unter `docs/scrum/reviews/`. In
Sprint 2 wurden die Befunde wenigstens im Sprint-Protokoll ausgeschrieben (7.3);
für Sprint 3 fehlt bis zu diesem Abschnitt beides. Die Befundlage ist hier
festgehalten, der Bericht selbst bleibt ein fehlender Beleg.

### 13.5 DoD 4 — erfüllt, und über das Geforderte hinaus

Der stärkste Punkt dieses Sprints. Nachgelesen, nicht gemeldet bekommen:

- **SPEC 6** trägt den Tokenizer-Wechsel, die zurückgenommene Präfix-Festlegung
  (mit dem Messgrund: `"foto"` und `"foto"*` sind beim trigram identisch), die
  Indexkosten (Faktor 6 bei 20 000 Notizen, 1,8 → 10,9 MiB), die ß-Grenze und
  den `LIKE`-Weg für Begriffe unter drei Zeichen samt gemessener Kosten (3 ms)
  **und dessen eigener Grenzen**.
- **SPEC 5.1** hält die Bedingung fest, ohne die der Index still verwahrlost:
  Die Trigger müssen FTS5 den **alten** Text mitgeben. Das ist ein
  Musterbeispiel für DoD 4 in der Fassung nach **B9** — eine entdeckte
  Bedingung, ohne die eine Festlegung nicht gilt, mit dem Zusatz, dass weder ein
  Fehler noch `integrity-check` sie anzeigen würde.
- **SPEC 9** trägt die Gliederung samt Locale-Regel, **SPEC 10** den Linksklick
  mit dem ausdrücklichen Vermerk „bei HIG- oder UI-Reviews kein Befund".
- **Wireframes** nachgezogen: 3a/3b überarbeitet, 4a/4b neu, der Zustand „Keine
  Treffer" gezeichnet. Damit ist die im Planning (4.2) benannte Lücke — ein
  sichtbarer Anteil ohne Referenz, das Verdikt-Muster von S8 in Sprint 2 —
  nachträglich geschlossen, obwohl der PO den Weg „textlich im AK" gewählt hatte.
- **KONZEPT.md**: keine Änderung nötig, es ist Entscheidungshistorie.

### 13.6 DoD 5 — M5: kein einziges Issue ist geschlossen

**M5.** Milestone „Sprint 3": **fünf offen, null geschlossen**. Kein Issue trägt
einen Commit-Verweis. DoD 5 verlangt beides. Der Punkt ist zum Prüfzeitpunkt
schlicht noch nicht abgearbeitet — der Reihenfolge nach gehört er hinter die
Kundenabnahme, aber er ist Teil der DoD und wird deshalb als offen geführt.

**M6.** #8 widerspricht sich selbst: Der Kopf trägt weiterhin „**Schätzung:**
3 SP" und „Tokenizer `unicode61 remove_diacritics 2`", während das Label `sp:5`
steht und `trigram` umgesetzt ist. Dasselbe Muster hat dieses Planning in
Abschnitt 1 schon einmal gerügt (Labels gegen Issue-Text). Da Issues die einzige
Quelle der Wahrheit sind, ist ein Widerspruch dort teurer als anderswo.

Die Commits selbst sind in Ordnung: 33 seit `59d0d3f`, sprechende Betreffzeilen,
Issue-Verweis durchgehend.

*Hinweis, kein DoD-Punkt:* `main` steht **33 Commits vor `origin/main`**. Der
Push ist seit Sprint 2 offen (7.3) und war dort für den Sprintstart empfohlen.

### 13.7 DoD 6 — M7: der Sprint ist bis 23:32 protokolliert

Vault-Pfad dynamisch aufgelöst; `Journal/Daily/2026/2026-08-01.md` trägt
`## Claude Code Protokoll` als letzten Abschnitt, chronologisch sortiert, mit
den Pflichtfeldern je Eintrag. Der Sprint ist in drei Einträgen erfasst (22:17,
22:51, 23:32), inhaltlich dicht und selbstkritisch.

**M7.** Die Arbeit **nach 23:32** fehlt: fünf Commits zwischen 23:33 und 23:50 —
der Wayland-Nachweis zu #54, der Merge von #54, der Retro-Punkt zur Sichtbarkeit
des Repositories, die Heilung der drei karpathy-Befunde und ihr Merge. Der letzte
Eintrag steht auf „In Arbeit" und trägt zusätzlich eine zweite Statuszeile
„Abgeschlossen"; beides gehört bereinigt.

### 13.8 Doku-Abgleich nach B10 — M8: der README behauptet den Stand von gestern

**M8.** `README.md:7` lautet: *„Status: In Entwicklung — Daemon mit Tray,
Capture-Fenster, globales Kürzel, Autostart und Bibliotheksfenster sind gebaut
(Sprint 2 in der Kundenabnahme)."* Eine Suche über den ganzen README nach
`suche|volltext|posteingang|gliederung|gruppe` liefert **null Treffer**. Die
Volltextsuche — das halbe Sprint-Ziel — kommt an keiner Stelle vor, die
Posteingangs-Gliederung ebenso wenig.

Das ist die erste Anwendung von B10, und sie schlägt sofort an. Bemerkenswert
ist die Richtung des Fehlers: Die **innere** Dokumentation (SPEC, Wireframes) ist
vorbildlich nachgezogen, die **äußere** — die einzige, die ein Fremder liest —
steht auf dem Stand von gestern. Das Repository ist öffentlich (12.6).

Kein Befund bei SPEC, KONZEPT und Wireframes (13.5). `docs/` enthält
ausschließlich Prozessdokumente; eine Nutzerdokumentation, die abweichen könnte,
existiert nicht.

### 13.9 Impediment I5 — geschlossen

Geprüft nach dem **vorab in Abschnitt 8 festgelegten** Kriterium, damit „ohne
Vorfall" nicht Ermessenssache ist. Alle vier Prüfungen schlagen **nicht** an:

1. **Fremde Dateien im Story-Commit — nein.** Jeder der elf Produktivcode-Commits
   trägt genau die Dateimenge seiner Story aus 2.1: #46 in `src/ui/*` +
   `librarytest`, #8/#9 in `src/store/*` + `storetest`, #44 in `src/shell/*` +
   `shelltest`, #54 in `src/capture/*` + `capturetest`. *Ein Grenzfall:*
   `7787339` trägt die Betreffzeile „(#46)", ändert aber auch
   `tests/searchshots.cpp` und `tests/storetest.cpp`. Das ist die
   Sprint-Ende-Heilung **nach** allen Merges, keine Parallelarbeit — kein
   Vorfall, aber die Betreffzeile nennt nur eine der berührten Stories.
2. **`--amend` auf fremde oder gepushte Commits — nein.** Im Haupt-Reflog
   stammen alle `amend`-, `rebase`- und `reset`-Einträge vom 31.07. und vom
   01.08. um 00:04, also aus Sprint 1 und 2; Sprint 3 begann um 22:00. Die vier
   Worktree-Reflogs (`.git/worktrees/*/logs/HEAD`) enthalten je genau einen
   Eintrag `reset: moving to HEAD` — ein Nulleffekt ohne Änderungsverlust.
3. **Beim Merge verlorene Änderungen — nein.** Alle acht Merges haben einen
   **leeren** kombinierten Diff: keine Konfliktauflösung von Hand, also keine
   Stelle, an der etwas hätte verlorengehen können. `git log main..<branch>` ist
   für alle vier Story-Branches leer — nichts ist liegengeblieben.
4. **`git add -A`-Signatur — nein.** Im gesamten Sprint-Diff kein Treffer auf
   `.claude/`, `settings.json`, Build-Artefakte oder Cache-Dateien.

**Entscheidung des Scrum Masters: I5 wird geschlossen.** Vier Agenten, vier
Worktrees, 33 Commits, acht Merges, keine Kollision. Die Bedingung aus Sprint-02,
9.7 („bis ein Sprint mit mehreren Dev-Agenten ohne Vorfall gelaufen ist") ist
erfüllt, und zwar nach einem Maßstab, der vor dem Sprint feststand.

*Grenze der Aussage, benannt statt verschwiegen:* Geschlossen ist das
Impediment „Git-Hygiene". Die Parallelarbeit hat zwei **andere** Risiken erzeugt,
die es ohne sie nicht gäbe — die Kollision am einen `/usr`-Präfix, die in M1
tatsächlich eingetreten ist, und der gleichzeitige Zugriff zweier UX-Agenten auf
die Wireframe-Datei. Beide sind keine Git-Fragen und gehören in die Retro
(12.5), nicht in I5. Dazu die ungeschützte Lage der Worktrees im Repo (12.8).

**Impediment I4 — ebenfalls geschlossen.** #46 ist die erste UI-Story, die die
Kette aus B2 (Geometrie-Zusicherungen) und B3 (UI-Review mit eigener
Bildprüfung) vollständig durchlaufen hat, und sie hat sie nicht nur durchlaufen,
sondern belastbar gemacht: Der `fail` an AK 7 wurde durch die eigene Bildprüfung
des Reviewers gefunden, nicht durch Tests. Damit ist die Bedingung aus
Sprint-02, 9.7 erfüllt. **I1** (Werkzeugkette, betrifft M3/M4) bleibt offen; in
diesem Sprint war keine Story berührt.

### 13.10 Sprint-Umfang — die Punktzahl hält, die Story-Zahl nicht

Klar beantwortet, weil danach gefragt wurde:

- **Die SP-Grenze ist gehalten.** 5 + 5 + 1 + 1 + 1 = **13 SP**, PROZESS.md sagt
  „max. ~13". Die Labels decken sich mit dieser Zählung. **Kein Mangel.**
- **Die Story-Grenze ist gerissen.** PROZESS.md sagt „2–4 Stories", der Sprint
  hat **fünf** Issues. **Mangel gegen die Sprint-Mechanik** — formal, nicht
  fachlich.

Fachlich war jeder der drei Zugänge einzeln begründet, und der zeitgebundene
(Wortteil-Suche im noch nicht abgelieferten Tokenizer) war die billigste
verfügbare Reihenfolge; ihn zu verschieben hätte eine zweite Migration samt Test
gekostet. Auch die Aufwertung von #8 auf 5 SP ist keine Ausweitung, sondern eine
Korrektur der Schätzung an gemessenem Mehraufwand — sie gehört ausdrücklich
gemeldet und nicht versteckt.

Was fehlt, ist die Regel, an der auffällt, dass die Summe eine Grenze passiert,
die niemand für sich beansprucht. Siehe 12.7: Die Punktzahl wurde bei jedem
Zugang mitgezählt, die Zahl der Issues nicht. **Retro-Thema, nicht Sprint-Thema.**

### 13.11 Sprint-Ziel — zwei Teile erreicht, der dritte wartet auf den Kunden

> „Die Bibliothek liest sich wie ein Posteingang, das Suchfeld findet Notizen im
> Volltext samt Umlaut-Toleranz, und ein Linksklick auf das Tray-Icon öffnet das
> Menü."

1. **Posteingang — erreicht.** Am Bild geprüft, nicht am Bericht
   (`docs/scrum/reviews/s5a-posteingang/01-normalfall.png`): Die Gruppen stehen
   in der festgelegten Reihenfolge, jeder Eintrag trägt Betreff und gedämpfte
   Vorschau, der Zeitstempel folgt seiner Gruppe („14:32" unter Heute, „Di., 28.
   Juli" unter Diese Woche, „10.07.2026" unter Älter). Ob sie sich *liest* wie
   ein Posteingang, bleibt Kundensache — im Planning (5.2) vorab so benannt.
2. **Volltextsuche samt Umlaut-Toleranz — erreicht und erweitert.** Die
   Zusicherungen stehen und laufen grün:
   `searchFindsWordsSpelledWithoutUmlauts` („bucher" → „Bücher"),
   `searchMatchesAnyPartOfAWord` („grafieren" → „fotografieren"),
   `searchFindsTermsShorterThanThreeCharacters` („KI", auch klein geschrieben),
   `keepsSearchIndexInSync`. Der Migrationstest #9 prüft eine echte Bestands-DB
   von Schemaversion 1 auf 2 und sucht danach „bucher" im migrierten Bestand.
3. **Tray-Linksklick — technisch belegt, Klick offen.** Der Rücklese-Beleg zu
   `ItemIsMenu` liegt an #44, der Test greift. Der Klick am Panel ist im Planning
   (5.1) vorab als Kundensache benannt. Nach DoD 2 in der Fassung nach B5 —
   *„Eine im Bericht benannte Grenze der Prüfbarkeit schließt die Story nicht"* —
   ist das **kein Mangel**, sondern der vorgesehene Weg.

**Urteil: Das Sprint-Ziel ist erreicht, soweit es ohne den Kunden feststellbar
ist.** Beide von Agenten prüfbaren Teile sind belegt; der dritte ist vorbereitet
und liegt beim Kunden — zusammen mit der Frage, ob sich die Liste liest wie ein
Posteingang, für die die Story gebaut wurde.

### 13.12 Mängelliste zur Entscheidung durch den PO

Melden, nicht heilen — nichts davon ist vom Scrum Master angefasst worden.

| # | Mangel | Schwere | Betrifft |
|---|---|---|---|
| **M1** | Installierter Stand (`/usr`, 22:00) trägt nur #44; #46, #8, #9, #54 sind nie am installierten Stand geprüft | **schwer** | DoD 2 |
| **M2** | AK-Haken und PO-Abnahme fehlen in allen fünf Issues; #9 und #54 ohne jeden Kommentar | mittel | DoD 2 |
| **M3** | Der Beleg der DoD-3-Erfüllung (Abschlusslauf, alle elf Szenen `ok`) liegt unversioniert im Arbeitsbaum | mittel | DoD 3 / B7 |
| **M4** | Kein karpathy-Bericht als Beleg im Repo | leicht | DoD 3 / B7 |
| **M5** | Kein Sprint-Issue geschlossen, kein Commit-Verweis (5 offen, 0 geschlossen) | mittel | DoD 5 |
| **M6** | #8 widerspricht sich: Kopf „3 SP" und `unicode61` gegen Label `sp:5` und umgesetztes `trigram` | leicht | DoD 5 |
| **M7** | Journal endet bei 23:32; fünf Commits danach unprotokolliert, letzter Eintrag mit zwei Statuszeilen | leicht | DoD 6 |
| **M8** | README behauptet den Sprint-2-Stand; Suche und Gliederung kommen nicht vor | mittel | B10 |
| **M9** | Fünf Issues statt „2–4 Stories" (13.10) | formal | Sprint-Mechanik |

**Empfohlene Reihenfolge:** M3 sofort (der Beleg kann verlorengehen, alles
andere kann warten) → M1 (der Kunde braucht den installierten Stand ohnehin für
seine Sichtprüfung) → M8, M6 → nach der Kundenabnahme M2 und M5 → M7 zum
Sitzungsende. M4 und M9 sind Retro-Stoff.

### 13.13 done / next

**done:** DoD-Prüfung über fünf Issues gegen den tatsächlichen Stand geführt —
frischer Build (0 Warnungen) und `ctest` (7/7) selbst gemessen statt übernommen,
der installierte Stand am Binärinhalt gegen HEAD gehalten, Geometrie-Zusicherungen
und Suchtests am Testcode nachgezählt, die Gliederung am Bild geprüft, SPEC und
Wireframes gegen die Lieferung gelesen, Issue-Stand und Milestone über `gh`
abgefragt, Journal im Vault über den dynamisch aufgelösten Pfad geprüft. **Acht
Mängel** benannt, einer davon schwer (M1: der falsche Prüfling, vorhergesehen als
Risiko 6.4 und trotzdem eingetreten), dazu ein formaler (M9). **I5 nach dem vorab
festgelegten Vier-Punkte-Kriterium geprüft und geschlossen**, ebenso **I4**;
Sprint-Ziel in zwei von drei Teilen als erreicht belegt, der dritte liegt
vorbereitet beim Kunden. Zwei Retro-Beobachtungen ergänzt (12.7 Story-Grenze,
12.8 Worktrees ohne `.gitignore`-Schutz).

**next:** (1) PO entscheidet über die neun Mängel in der empfohlenen Reihenfolge;
M3 ist eilig, weil ein unversionierter Beleg mit dem Arbeitsbaum verschwindet.
(2) Kundenabnahme am **installierten** Stand: Tray-Linksklick, Gliederung,
Suchprobe. (3) Danach Issues schließen, Milestone schließen, Push (33 Commits).
(4) **Reguläre Retrospektive nach Sprint 3** — die Kadenz aus PROZESS.md; die
Retro vom 01.08. war eine vom Kunden angeordnete Vorziehung und ersetzt sie
nicht. Vorbereitetes Material: Abschnitt 12 mit acht Beobachtungen, die neun
Mängel dieser Prüfung, die beiden geschlossenen Impedimente.

## 14. Retro-Agenda des Kunden (02.08.2026, 08:56)

Der Kunde hat vier Themen für die Retrospektive gesetzt. Der PO hat den Ist-Stand
dazu erhoben, damit die Sitzung an Zahlen und nicht an Eindrücken ansetzt.

### 14.1 Git wird kaum genutzt

> „Alles landet im Main Branch, keine Pull Requests usw."

**Gemessener Stand (02.08.2026, 08:52):**

| Größe | Wert |
|---|---|
| Pull Requests, jemals | **0** |
| Remote-Branches | nur `origin/main` |
| Lokale Branches | **9** (4 Story-/Fix-Zweige, 4 Worktree-Zweige, `main`) |
| Tags | **0** |
| Releases | **0** |

Der Befund des Kunden trifft zu — mit einer Einschränkung, die zu seinen Gunsten
zählt: In Sprint 3 hat jeder Strang auf einem **eigenen** Branch im eigenen
Worktree gearbeitet (`story/46-posteingang`, `story/8-volltextsuche`,
`fix/44-tray-linksklick`, `fix/54-theme-farben`). Die Zweige existieren also;
was fehlt, ist alles, was danach kommt: kein Push der Zweige, kein PR, kein
Review am Diff, kein Aufräumen. Gemerged wurde lokal durch den PO.

Zu klären: Bringt ein PR-Verfahren hier Nutzen oder nur Zeremonie — bei einem
Einzelentwickler-Repo, in dem der PO ohnehin jeden Diff sieht? Gegenargument:
Der karpathy- und der UI-Review haben heute keinen festen Anknüpfungspunkt im
Git-Verlauf; ein PR wäre genau dieser Ort. Und die acht Leichen im lokalen
Branch-Bestand sind ein Ergebnis des fehlenden Abschlusses.

### 14.2 Versionen und Release Notes

> „Ab einem bestimmten Punkt müssen wir uns um Versionen und Release Notes
> Gedanken machen."

`CMakeLists.txt:3` trägt seit dem ersten Commit `VERSION 0.1.0`; drei Sprints
haben daran nichts geändert. Es gibt keinen Tag, kein Release, keine Stelle, an
der ein Außenstehender sähe, was in einem Stand steckt. Solange nur der Kunde
installiert, kostet das nichts — mit #41 (S28: PKGBUILD + Installations-Doku)
wird es zur Voraussetzung: Ein PKGBUILD braucht eine Versionsnummer, die sich
bewegt.

Zu klären: Wann beginnt die Zählung, nach welchem Schema, und was löst eine
Erhöhung aus (Sprint-Ende? Milestone? Kundenabnahme?).

### 14.3 Changelog

> „Was ist eigentlich mit einem Changelog?"

Existiert nicht. Der Stoff dafür ist vollständig vorhanden — er liegt nur an
drei Orten, die kein Nutzer liest: Sprint-Protokolle, Issues, Commit-Verlauf.
Der Aufwand ist deshalb Umschichtung, nicht Neuerhebung.

Zu klären: Ein `CHANGELOG.md` nach *Keep a Changelog*, gepflegt zum Sprint-Ende
— oder aus Tags erzeugte GitHub-Release-Notes? Beides doppelt zu führen wäre
gegen Karpathy 2.

### 14.4 Modellzuordnung — Sonnet und Haiku für Verwaltungsarbeit

> „Es können auch Agenten mit Sonnet oder Haiku Modellen für einfache
> Verwaltungsaufgaben eingesetzt werden."

Der geltende Stand steht in PROZESS.md, „Modellzuordnung" (Kundenentscheidung
31.07.2026): `scrum-master`, `denkzettel-dev`, `denkzettel-ux` auf Opus 5;
`karpathy-reviewer` auf Fable; Spikes und Risiko-Stories stuft der PO je Spawn
hoch. Dort steht ausdrücklich: „Revision dieser Zuordnung ist Retro-Thema."

Der Vorschlag des Kunden zielt auf eine Lücke: Es gibt heute **keine** Rolle für
Verwaltungsarbeit. AK-Haken setzen, Issues schließen, Milestones pflegen,
Branches aufräumen, Changelog-Zeilen aus Commits ziehen — das macht heute der PO
auf Opus, oder es bleibt liegen (M2 und M5 sind genau solche Fälle: seit dem
Sprint-Ende offen, weil niemand zuständig war).

Zu klären: neuer Agent oder Ausweitung des Scrum Masters — und welche
Zuständigkeit, ohne die Regel „melden, nicht heilen" zu verletzen. Ein Agent,
der Issues schließt, trifft keine Entscheidung; einer, der Mängel behebt, schon.
Die Grenze gehört benannt.

### 14.5 Was diese vier Themen gemeinsam haben

Alle vier betreffen **das, was nach der Arbeit kommt** — Abschluss, Verpackung,
Nachweis nach außen. Das Team ist gut darin, etwas zu bauen und zu prüfen; die
neun Mängel aus 13.12 sind bis auf M1 sämtlich Abschlussmängel. Der Kunde legt
den Finger auf dieselbe Stelle, an der die DoD-Prüfung am meisten fand. Das
sollte die Retro nicht als vier Einzelthemen behandeln.

## 15. Kundenabnahme (02.08.2026, 09:00–09:06)

Der Kunde hat am installierten Stand geprüft — der dritte Teil des
Sprint-Ziels (13.11) ist damit erbracht.

### 15.1 Tray — abgenommen, vier Befunde am Menü

> „Der Linksklick passt."

**#44 ist abgenommen.** Am Menü selbst vier Befunde: „Beenden" gehört nicht
in die Linksklick-Liste; den Einträgen fehlen Icons; „Capture öffnen" ist
Englisch; der Rechtsklick soll ein eigenes Layout haben (Idee des Kunden:
Einstellungen und Beenden). Als Referenz zeigte er vier Plasma-Applets
(Benachrichtigungen, Kontakt, Wetterbericht, Energieverwaltung) mit
Rechtsklick-Menüs: Icons je Eintrag, Muster „… einrichten". Die Befunde
ändern die in #44 dokumentierte Entscheidung „dasselbe Menü für beide
Klicks" — sie laufen als **#60** weiter, nicht als Mangel an #44.

### 15.2 Suche — vier Proben des Kunden

| Eingabe | Ergebnis | Einordnung |
|---|---|---|
| `prufen` | **Treffer** („Ganz dringend die DoD prüfen.") | beauftragter Umfang: u→ü über `remove_diacritics` |
| `prüfen` | **Treffer** | exakt |
| `pruefen` | keine Treffer | **#51** — Schreibvarianten (ue-Digraph); war im Backlog |
| `prüfem` | keine Treffer | **#52** — Tippfehler-Toleranz (m statt n); war im Backlog |

Kundenurteil: „Die Suche klappt noch nicht richtig." Die beiden Fehlfälle
liegen außerhalb des in Sprint 3 beauftragten Umfangs und waren als #51/#52
erfasst — das mindert den Befund nicht. Der Kunde hat die **Recherche
beauftragt**: „Das Thema Fuzzy Search soll noch einmal recherchiert werden.
Da gibt es doch bestimmt schon was Fertiges." #52 ist damit vom Klärungs-
zum Rechercheauftrag aufgewertet (Vorgehen nach der Werkzeug-Regel: lesen,
urteilen, fragen — keine Installation vor der Entscheidung); #51 ist um den
ue/ü-Fall erweitert.

### 15.3 Belegsicherung — nur ein Bild überlebte

Die acht Kundenbilder lagen in flüchtigen Spectacle-Ordnern unter `/tmp`.
Beim Sichern (09:11) existierte nur noch das letzte —
Energieverwaltungs-Beispiel, jetzt unter
`reviews/sprint-03-kundenabnahme/vorbild-tray-energieverwaltung.png`. Die
übrigen sieben sind hier textlich festgehalten; die vier Suchproben sind am
installierten Stand jederzeit reproduzierbar. **Lehre zu B7:** Flüchtige
Kundenbelege sofort beim Eintreffen sichern, nicht am Ende des
Arbeitsschritts — sieben Minuten waren zu langsam.

### 15.4 Abschluss

Fünf Issues mit AK-Haken, Abnahmekommentar und Commit-Verweisen geschlossen,
Milestone „Sprint 3" geschlossen — **M2 und M5 damit behoben**. Sprint-Ziel
in allen drei Teilen erreicht. Neu aus der Abnahme: #60 (Tray-Menüs),
Rechercheauftrag Fuzzy-Suche (#52), Erweiterung #51.

## 16. Retrospektive (02.08.2026)

**Datum:** 2026-08-02, 09:39–09:48 (Ganymed) · **Moderation:** Scrum Master
**Anlass:** die **reguläre** Retro nach Sprint 3 gemäß Kadenz (PROZESS.md,
Retrospektiven). Die Retro vom 01.08.2026 war eine vom Kunden angeordnete
Vorziehung nach Sprint 2 und ersetzt diese nicht.
**Prüfstand:** `main` @ `e73efff`. **Eingaben:** Sprint-3-Protokoll (8, 11, 12,
13, 14, 15), `docs/scrum/sprints/sprint-02.md` 9.5 (B1–B10),
`docs/scrum/retro/sprint-03/stellungnahme-dev.md`,
`docs/scrum/reviews/sprint-03-karpathy.md`, `CLAUDE.md`, `PROZESS.md`,
GitHub-Issues, Git-Historie, installierter Stand, Vault-Journal.

**Belegregel dieser Sitzung:** Jede Aussage über den Sprint trägt ihre
Fundstelle. Wo „gemessen" steht, ist der Befehl in dieser Sitzung selbst
gelaufen; übernommene Werte sind als übernommen gekennzeichnet.

### 16.1 Wirksamkeitsprüfung der Sprint-2-Beschlüsse B1–B10

Geführt gegen den **Verlauf** von Sprint 3, nicht gegen die Prozess-Doku. Das
ist die Nagelprobe der neuen zweistufigen Abschlussprüfung: verankert *und*
gelesen?

| # | Beschluss | Wirkung in Sprint 3 | Beleg |
|---|---|---|---|
| B1 | Selbst-Sichtprüfung vor der Übergabe | gewirkt — am falschen Prüfling | Sie haben stattgefunden (13.3: „liefen gegen die Build-Verzeichnisse der Worktrees"); D-Bus-Rücklese-Beleg zu `ItemIsMenu` als Kommentar an #44; `ea2337a` „Nachweis #54: vierter Lauf nativ unter Wayland". Dass sie den Build statt `/usr` prüften, ist B4, nicht B1 |
| B2 | Geometrie-Zusicherungen, zwei Fenstergrößen | gewirkt | Heute nachgezählt: `keepsTheHeaderAtTheTopAndTheRestForTheNotes_data` (`librarytest.cpp:1795`) und `keepsTheMeasuresOfTheGroupedList_data` (`:1853`) je 900×600/1200×800, `bringsTheHeadOfTheNewGroupIntoView_data` (`:1015`) 900×600 und 900×150 |
| B3 | UI-Review ist ohne eigenes Bild nicht geführt | gewirkt, entscheidend | Der einzige `fail` des Sprints (AK 7) kam aus der eigenen Bildprüfung des Reviewers, nicht aus Tests (13.4, 13.9) |
| B4 | Geprüft wird der installierte Stand | **gerissen** | M1 (13.3) |
| B5 | Registrierungen zurücklesen; benannte Prüfgrenze schließt die Story nicht | gewirkt | Die Grenze „Klick am Panel" wurde vorab benannt (5.1), als Kundensache geführt und am 02.08. 09:00 erbracht (15.1) — keine Fußnote |
| B6 | Git-Regeln in der Agentendatei | gewirkt | I5 nach vorab festgelegtem Vier-Punkte-Kriterium geschlossen (13.9); heute nachgemessen: alle vier Story-Zweige vollständig in `main`, `git branch --no-merged main` leer |
| B7 | Belege gehören ins Repo | **viermal gerissen** — zweimal als Mangel gemeldet, zweimal in dieser Retro neu gefunden | M3, M4 (13.4) und 16.1.2 unten |
| B8 | Kadenz plus jederzeitige Kundenanordnung | trägt | Diese Sitzung nach Kadenz, die vom 01.08. auf Anordnung — ohne den Halbsatz stünden beide im Widerspruch zueinander |
| B9 | DoD 4 erfasst entdeckte Bedingungen | gewirkt, stärkster Punkt | SPEC 5.1 (Trigger müssen FTS5 den **alten** Text mitgeben — weder Fehler noch `integrity-check` zeigen es an) und SPEC 6 (13.5) |
| B10 | Doku-Abgleich am Sprint-Ende | gewirkt, schlug sofort an — **und ist heute wieder offen** | M8 (13.8); erneut: `README.md:7` sagt „(Sprint 3 in der Kundenabnahme)", der Kunde hat um 09:06 abgenommen (15) |

#### 16.1.1 B4 — die Regel wurde ausgesetzt, der Ersatz stand nirgends

M1 ist nicht durch Nachlässigkeit entstanden. Der PO **musste** die Regel für
die Stränge aussetzen — es gibt ein `/usr` und vier Stränge, das Planning hat
es als Risiko 6.4 vorhergesehen. Was fehlte, war der Ersatz: *am Sprint-Ende
den Endstand einmal installieren.* Er stand in keinem Artefakt, also geschah er
nicht, und vier von fünf Issues wurden am falschen Prüfling geprüft.

**Heilung gemessen, nicht übernommen:** `stat /usr/bin/denkzetteld` →
`ctime = 2026-08-02 00:23:58`, also 24 Minuten nach der DoD-Prüfung. Der
installierte Stand trägt heute alle Merkmale, deren Fehlen den Mangel belegt
hatte (`grep -a` auf die Binärdatei: „Keine Treffer", „trigram", „Diese
Woche", „Letzte Woche", „Älter" — je vorhanden). Die Regel selbst trägt
den Ersatz seit dem 02.08. in `PROZESS.md` (DoD 2) und `CLAUDE.md`.

#### 16.1.2 B7 — viermal zu kurz, zweimal davon unbemerkt

M3 (unversionierter Abschlusslauf) ist mit `47af87a` um 00:04 geheilt, M4
(fehlender karpathy-Bericht) mit `0b1a3a1` um 00:05. Drei Befunde kommen hinzu,
ein vierter entstand während dieser Sitzung:

1. **Der erste karpathy-Lauf hat überhaupt keinen Bericht.** `e18630c` (01.08.,
   22:33) heilt „drei Befunde des Prinzipien-Reviews" an #46; ein Bericht dazu
   existiert nirgends — `grep -rl karpathy docs/scrum/reviews/` liefert genau
   eine Datei, und die deckt den Sprint-Ende-Lauf ab. Die drei Befunde leben in
   einer Commit-Nachricht.
2. **Der abgelegte Bericht ist eine Fremdfassung.** Er sagt es selbst
   (`sprint-03-karpathy.md:6`): „Abgelegt vom PO … gekürzt um Wiederholungen."
   Ehrlich gekennzeichnet — aber der Beleg hat die Hand dessen passiert, dessen
   Arbeit er beurteilt. Das ist die milde Form des Sprint-2-Musters, gegen das
   B7 gefasst wurde.
3. **Und die eigene Rolle:** Die DoD-Prüfung (23:59) hat DoD 3 als erfüllt
   gebucht, obwohl der Bericht erst um 00:05 entstand. Sie hat die Lücke als M4
   gemeldet und den Punkt trotzdem passieren lassen. Nachprüfbar war die
   Aussage über Commits (`e18630c`, `7787339`) und über #59 — aber die Verdikte
   `warn`/kein `fail` standen zum Prüfzeitpunkt in keinem Artefakt. **Der Scrum
   Master hat gegen eine Meldung geprüft.**

**Nachtrag, in dieser Sitzung um 09:48 gemessen — B7 reißt gerade jetzt zum
vierten Mal.** `git status --porcelain docs/scrum/retro/` meldet
`?? docs/scrum/retro/sprint-03/stellungnahme-dev.md`. Die technische
Stellungnahme, auf der Abschnitt 16.3 vollständig ruht, ist **nicht
committet**; die Sprint-2-Akte daneben ist es (neun Dateien unter
`docs/scrum/retro/sprint-02/`, `git ls-files`). Ein Beleg, der eine Retro trägt
und mit der Sitzung verschwinden kann, ist genau der Fall, für den B7 gefasst
wurde — und er tritt in der Retro über B7 auf. **Melden, nicht heilen:** Der
Scrum Master committet nichts, der Punkt steht in Auftrag 2 (16.10).

#### 16.1.3 Der gemeinsame Nenner der beiden gerissenen Beschlüsse

B1, B2, B3, B5, B6 und B9 hängen je an einem **Gegenstand** — einem Test, einem
Bild, einer Zeile Code, einer SPEC-Zeile. Sie wurden mit ihm zusammen erledigt
und konnten gar nicht vergessen werden.

B4 und B7 verlangen eine **Handlung zu einem Zeitpunkt, den kein Artefakt
benennt**: „installieren, bevor geprüft wird", „ablegen, bevor es weg ist".
B10 hat einen Zeitpunkt — das Sprint-Ende —, aber den falschen: Er liegt vor
der Abnahme, und die Statuszeile wird erst durch die Abnahme falsch.

**Schluss: Nicht die Regeln haben gefehlt, sondern ihr Ort im Ablauf.** Daraus
folgt B11.

### 16.2 Das Muster hinter den Mängeln: der Sprint hat kein Ende

14.5 hat es benannt: Acht der neun Mängel sind Abschlussmängel. Die Probe
darauf ist der Ist-Zustand **heute**, knapp zehn Stunden nach dem letzten
Produktivcode-Commit (`47a1774`, 01.08. 23:50) — alle Zeilen in dieser Sitzung
gemessen:

| Abschlussarbeit | Zustand 02.08., 09:39 | Beleg |
|---|---|---|
| Endstand installiert | erledigt, 02.08. 00:23:58 | `stat -c %z /usr/bin/denkzetteld` |
| Prüfbelege versioniert | **teilweise offen** | `47af87a`, `0b1a3a1` erledigt; erster karpathy-Lauf ohne Bericht, `docs/scrum/retro/sprint-03/` untracked (16.1.2) |
| Issues + Milestone geschlossen | erledigt | `gh issue list --milestone "Sprint 3"` → 5/5 `CLOSED` |
| Journal | erledigt | Einträge 00:05, 00:34, 09:14 im Vault |
| README-Statuszeile | **offen** | `README.md:7`: „(Sprint 3 in der Kundenabnahme)" |
| Push | **offen** | `git rev-list --left-right --count origin/main...main` → `0 1` |
| Story-Zweige geräumt | **offen** | 8 lokale Zweige, alle vollständig in `main` (`git merge-base --is-ancestor` je Exit 0) |
| Worktrees geräumt | **offen** | `git worktree list` → 4 ausgecheckt; `du -sh .claude/worktrees` → **296 MB** |
| Vollzugsvermerk | existiert nicht | — |

**Der unbequeme Teil daran:** Der Entwickler hat die acht Zweige und die 296 MB
in seiner Stellungnahme (Abschnitt 1) einzeln geprüft, als gefahrlos löschbar
belegt und ausdrücklich **nichts** gelöscht — „melden, nicht heilen", exakt
regelkonform. Der Befund liegt seit heute früh vor und hat trotzdem keinen
Adressaten gefunden. **Melden ohne einen Ort, an dem das Gemeldete abgearbeitet
wird, endet als Ablage.** Das ist dieselbe Lücke wie bei B4 und B7, nur eine
Stufe später.

**Wo der Abschluss stehen muss, damit er gelesen wird.** Die Liste selbst
gehört in `PROZESS.md` — dort steht der Ablauf, und der Scrum Master liest die
Datei von Amts wegen. Der **Auslöser** gehört in `CLAUDE.md`: Sie ist die
einzige Datei, die jede Sitzung ohne Zutun aufschlägt, und der Ausführende von
Takt 2 ist der PO, nicht der Scrum Master. Und damit die Liste nicht wieder nur
Text ist, wird sie am Sprint-Ende ins Protokoll übernommen und mit Beleg
abgehakt — 12.7 gilt für Abschlusspunkte so gut wie für Story-Grenzen: *Eine
Regel, die in keiner laufenden Liste steht, ist keine Regel.*

### 16.3 Die vier Kundenthemen

Grundlage ist die technische Stellungnahme des Entwicklers
(`docs/scrum/retro/sprint-03/stellungnahme-dev.md`, alle Zahlen dort mit
Befehl). Sie ist in der Sache gut belegt; wo der Scrum Master widerspricht oder
schärft, steht es dabei. **Nichts davon ist entschieden** — alle vier Punkte
sind Kundenentscheidungen und stehen als solche in 16.9.

#### 16.3.1 PR-Verfahren

**Empfehlung des Devs:** ein PR je Story-Strang, geöffnet bei der Übergabe,
Dev-Bericht und Review-Befunde als Kommentare, Merge durch den PO;
Prozess- und Doku-Commits weiter direkt auf `main`; `--no-ff` statt Squash
(gemessen: 90 von 91 Commits haben einen Body); Rückwärts-Merges künftig durch
Rebase ersetzen.

**Zustimmung, wo es trägt.** Das Anker-Argument ist echt und heute nachprüfbar:
DoD 3 lässt die Reviews über „den Sprint-Diff" laufen, und dieser Bereich wird
jedes Mal von Hand konstruiert (`59d0d3f..HEAD`). Die Befunde hängen an keiner
Diff-Zeile. Auch die Merge-Strategie teilt der Scrum Master: Squash presste
Festlegungen wie die in `19ca42b` (drei gemessene Werte im Commit-Body) in eine
Zeile — dieses Projekt schreibt seine Begründungen in die Commits, das ist eine
Quelle, keine Zeremonie.

**Widerspruch: Der Dev vergleicht den PR mit „nichts" statt mit der billigeren
Alternative.** Für den Zweck „stabiler Diff-Bereich am Sprint-Ende" genügt ein
**Sprint-Basis-Tag** (`sprint-3-base` auf dem Ausgangsstand). Er kostet einen
Befehl, veröffentlicht keinen Zwischenstand und macht `git diff
sprint-N-base..main` reproduzierbar. Was ein PR **zusätzlich** böte, ist ein
Ort für zeilengebundene Kommentare und für einen automatischen Testlauf. Beides
hat heute keinen Nutzer: Die Reviewer sind Agenten, die ihre Befunde als
Bericht abliefern, und eine CI existiert nicht (`ls .github` → nicht
vorhanden). **Ein PR ohne CI und ohne Zeilenkommentare ist genau die Zeremonie,
vor der der Dev selbst warnt.**

**Zweiter Punkt, der dem Kunden gehört:** Branch-Push ist bei einem öffentlichen
Repo Veröffentlichung. Heute ist nur das kuratierte `main` sichtbar; mit
PR-Pflicht wäre jeder Zwischenstand publiziert. Das ist keine technische, das
ist seine Entscheidung.

**Empfehlung des Scrum Masters:** Sprint-Basis-Tag sofort (kostenlos, unabhängig
von allem anderen); PR-Verfahren als **Probelauf über Sprint 4** mit einem
**vorab festgelegten Kriterium** — dieselbe Methode, mit der I5 belastbar
geschlossen wurde. Kriterium: Am Ende von Sprint 4 hängt mindestens ein Befund
an einer Diff-Zeile, der ohne PR nicht auffindbar gewesen wäre, **oder** der PR
hat einen automatischen Testlauf getragen. Sonst wird das Verfahren wieder
eingestellt und der Tag bleibt.

#### 16.3.2 Versionierung

**Empfehlung des Devs:** `CMakeLists.txt:3` als einzige Quelle, Tag als Siegel,
0.x-SemVer, MINOR je **Kundenabnahme** (nicht je Sprint-Ende), PATCH für
außerplanmäßige Behebungen, 1.0.0 auf Erklärung des Kunden, jede Schemamigration
erzwingt mindestens einen MINOR-Sprung; sichtbar über
`app.setApplicationVersion()` und `--version`; ein `KAboutData`-Dialog steht
nicht in der SPEC und bräuchte eine eigene Story.

**Zustimmung in allen Punkten.** Der Kernbefund ist gemessen und erklärt drei
Sprints Stillstand: Die Version ist **inert** — `grep -rn
"PROJECT_VERSION\|KAboutData\|setApplicationVersion" src desktop CMakeLists.txt`
findet nichts. Eine Erhöhung hätte nichts verändert, was jemand hätte sehen
können. Besonders gut begründet ist die Kopplung an die **Abnahme**: Ein
Sprintschluss ist ein Ereignis des Teams, ein Release eines für den Kunden — und
ein Sprint ohne Abnahme erzeugt dann eben keine Version. Ebenso die
Migrationsregel: Wer echten Bestand hat, muss an der Zahl sehen, dass ein Stand
seine Datenbank anfasst.

**Eine Ergänzung aus Prozesssicht:** Die Umsetzung ist Produktivcode
(`--version`, Compile-Definition) und **braucht eine eigene Story mit
SPEC-Eintrag** — die SPEC kennt heute weder Programmversion noch „Über"-Dialog
(`grep -in version SPEC.md` → nur Schemaversionen). Ohne Story liefe die
Änderung an DoD 4 vorbei; genau davor warnt der Dev selbst („ich schlage ihn
vor, ich erfinde ihn nicht"). #41 (PKGBUILD) hängt daran.

#### 16.3.3 Changelog

**Empfehlung des Devs:** `CHANGELOG.md` nach *Keep a Changelog*, gespeist aus
den geschlossenen Issues je Milestone (`gh issue list --state closed
--milestone …` — ein Befehl), Release-Notes daraus beim Taggen, nie umgekehrt;
drei Auflagen, weil das Repo öffentlich ist (kein Innenregister, keine
Story-Kürzel, keine Kundenzitate).

**Zustimmung zur Quelle und zur Richtung.** Der Nachweis ist stark: 92 %
Issue-Deckung bei Produktivcode-Commits, aber **null** Conventional-Präfixe —
also ist das Commit-Log als Generator-Quelle tot, die Issues sind es nicht. Und
„Datei im Repo, Release-Seite daraus" ist richtig herum: Wer klont oder ein
Paket baut, sieht die Release-Seite nie.

**Widerspruch, belegt: Die Labels sind *nicht* „exakt die
Keep-a-Changelog-Kategorien".** Gemessen an Sprint 3 (`gh issue list
--milestone "Sprint 3" --json labels`): `typ:story` ×2, `typ:bug` ×2,
`typ:tech` ×1. Die ersten beiden bilden Added/Fixed ab — `typ:tech` bildet
nichts ab: #9 ist ein Migrationstest, für einen Nutzer unsichtbar. Und
umgekehrt fehlt der Eintrag, der einem Nutzer von allen am meisten sagt — *beim
ersten Start wird die Datenbank umgeschrieben* —, in **jedem** Issue-Titel; die
Migration steckt als Nebenwirkung in #8. Der Dev fordert diesen Eintrag selbst,
leitet ihn aber aus einer Quelle her, die ihn nicht enthält.

**Daraus folgt eine Verschärfung, keine Ablehnung:** Der Entwurf ist mechanisch,
der Changelog ist es nicht. Er braucht (a) eine Filterregel — `typ:tech`
erscheint nur, wenn es nach außen wirkt —, und (b) einen **Pflichtpunkt
„Datenbank"**: Ändert sich die Schemaversion, steht ein Hinweis im Changelog,
unabhängig davon, welche Issues geschlossen wurden. Ohne diese beiden Zusätze
erzeugt der Vorschlag ein Register geschlossener Tickets, keine Release-Note.

#### 16.3.4 Verwaltungsrolle auf kleinem Modell

**Empfehlung des Devs:** ein einziger Agent `denkzettel-verwalter` (Haiku),
Werkzeuge auf `Read` und `Bash(git …|gh …)` beschränkt, kein Schreibrecht auf
`SPEC.md`, `src/` und Issue-Rümpfe; Standardausgabe ist ein Bericht, umkehrbare
Handlungen nur auf ausdrücklichen Auftrag. Trennlinie: **ausführen ja,
entscheiden nein** — und nur, wo das Ergebnis per Befehl prüfbar und die
Handlung umkehrbar ist. Ausdrücklich **nicht** an ein kleines Modell:
AK-Haken setzen (das ist die Abnahmeentscheidung selbst), über das Schließen
entscheiden, Mängel beheben, Versionssprünge bestimmen, Changelog-Texte
formulieren.

**Vollständige Zustimmung zur Trennlinie.** Sie ist der Kern und sie ist richtig
begründet: Der Haken ist nicht umkehrbar, er wird zur Beweislage — und dieses
Projekt hat an einem Abend vier grüne Tests verloren, die nichts prüften.

**Eine Einschränkung und eine Reihenfolge.** Die Aufgaben, um die es geht — Issues
schließen, Zweige räumen, Abgleichsberichte — sind zusammen vielleicht zehn
Minuten Arbeit je Sprint. Karpathy 2 spräche dagegen, dafür einen Agenten zu
bauen. **Das Argument, das trotzdem trägt, ist nicht Aufwand, sondern
Zuständigkeit:** Diese zehn Minuten haben nicht stattgefunden — M2 und M5
standen offen, weil niemand zuständig war, und acht Zweige samt 296 MB stehen
es bis jetzt (16.2). Ein Agent gibt der Arbeit einen Inhaber.

Daraus folgt die Reihenfolge, in der der Scrum Master vom Dev abweicht: Der Dev
setzt den Verwalter hinter die drei Git-Themen. **Die eigentliche Vorbedingung
ist die Abschlussliste (B11)** — ohne sie hat der Verwalter keine Liste, die er
abarbeiten könnte, und mit ihr ist er schon nützlich, bevor PR, Version und
Changelog entschieden sind.

### 16.4 Revision der Modellzuordnung

In `PROZESS.md` ausdrücklich als Retro-Thema vorgemerkt. Geprüft am
Sprint-Verlauf, nicht am Eindruck.

**Opus 5 für `denkzettel-dev`, `denkzettel-ux`, `scrum-master`: bewährt.**

- Der Sprint lieferte 33 Commits über vier Stränge, Neubau mit **null
  Warnungen**, `ctest` 7/7, `tests/librarytest.cpp` von 983 auf 2040 Zeilen und
  67 Testfunktionen (13.2).
- `denkzettel-ux` hat den einzigen `fail` des Sprints aus **eigenen Bildern**
  gefunden, nicht aus Tests, und nach der Heilung einen zusätzlichen `warn`
  (13.4).
- `denkzettel-dev` hat in der zweiten Runde denselben Fehlertyp **selbst**
  gefunden, und zwar durch Messung des Rollwerts statt durch Vergleich von
  Endbildern (12.1).
- Der `scrum-master` hat in der DoD-Prüfung neun Mängel benannt, darunter den
  schweren M1, den niemand gemeldet hatte — gefunden durch Vergleich des
  Binärinhalts von `/usr` gegen einen frischen Build (13.3).

**Der Fall, der gegen eine Abstufung des Sicherheitsnetzes spricht.** Die
tautologische Zusicherung in `tests/librarytest.cpp` wurde von einer Opus-Rolle
geschrieben (`4746358`, #46), lief in deren eigener Suite grün — sie *konnte*
nicht rot werden — und erreichte `main`. Gefunden hat sie der
`karpathy-reviewer` auf **Fable**; entfernt wurde sie erst mit `7787339`
(23:49). Ein Test, der nicht fehlschlagen kann, ist die teuerste Fehlerart
dieses Projekts. **Der `karpathy-reviewer` bleibt auf Fable.**

**Kein Modellbefund ist M1.** Der falsche Prüfling entstand daraus, dass eine
Regel ohne Ersatz ausgesetzt wurde (16.1.1) — das ist eine Prozessfrage und
wäre auf jedem Modell passiert.

**Zum Kundenvorschlag „Sonnet/Haiku für Verwaltung":** fachlich richtig
platziert, siehe 16.3.4. Er füllt eine Lücke, keine Ersetzung: Es gibt heute
**keine** Rolle für Verwaltungsarbeit, und die Rollentabelle in `PROZESS.md`
bekommt erst nach der Kundenentscheidung eine Zeile dafür.

*Ergebnis (in `PROZESS.md`, Modellzuordnung, eingetragen): Zuordnung bestätigt,
nächste Revision Retro nach Sprint 6.*

### 16.5 Retro-Stoff aus der DoD-Prüfung

**M4 — kein karpathy-Bericht als versionierter Beleg.** Behandelt in 16.1.2; er
ist kein Einzelfall, sondern hat einen unentdeckten Bruder (der erste Lauf zu
#46 ganz ohne Bericht) und einen Konstruktionsfehler (Ablage durch den
Geprüften). Antwort: B11, Takt 1, Punkt 2 — jeder Prüflauf hinterlässt eine
Datei, **bevor** die DoD-Prüfung läuft.

**M9 — fünf Issues gegen „2–4 Stories": Regel anpassen oder Verhalten?**
Entschieden: **Verhalten, nicht Regel.** Begründung am Fall: Das Planning hat
die Grenze korrekt angewandt, als es sie kommen sah — 3.3 legt #50 wörtlich als
Kundenentscheidung vor, weil es das fünfte Issue wäre. Die Grenze hat also
funktioniert, solange jemand sie prüfte. Gerissen ist sie bei #54, und zwar
nicht, weil sie zu eng war, sondern weil bei jedem Zugang die Punktzahl
mitgezählt wurde und die Zahl der Issues nicht (12.7). Eine Erweiterung auf
„2–5" nähme die einzige Schwelle weg, die einen Zugang überhaupt zur
Kundenfrage macht — und der Kunde hat #54 ja freigegeben; ihm wurde nur nicht
gesagt, dass er damit zugleich über eine Grenze entschied. Antwort: B12
(Sprint-Konto mit beiden Zahlen und Vorlagepflicht bei Grenzberührung).

**12.3 — 30 % Sprintwachstum nach der Freigabe.** Drei Zugänge, jeder einzeln
begründet und einzeln vom Kunden freigegeben. Der Scrum Master sieht hier
**keine** zusätzliche Regel als nötig an: Die Entscheidung lag jedes Mal beim
Kunden, wo sie hingehört. Was fehlte, ist ausschließlich die Information, dass
die Summe eine Schwelle passiert — genau das leistet B12. Eine Regel „ab X %
ist es ein neuer Sprint" wäre eine Erfindung ohne Fall.

**15.3 — flüchtige Kundenbelege.** Von acht Bildern der Abnahme überlebten
sieben Minuten nur eines. B7 kennt nur die Belege, die Agenten selbst erzeugen.
**Wohin die Regel gehört:** in `PROZESS.md` (Artefakte, als Ergänzung zu B7 —
dort steht die Belegordnung) und als Auslöser in `CLAUDE.md`, weil der
Empfänger von Kundenbildern immer der PO ist und keine andere Datei ihn von
selbst erreicht. Beschluss B14.

**12.6 — das Repository ist öffentlich.** Der Prozess kennt bis heute keine
Regel dafür, was hineingeschrieben wird; wörtliche Kundenzitate, Messwerte vom
Rechner des Kunden und interne Fehleranalysen stehen bereits darin — auch in
diesem Protokoll. Das ist **keine Prozessfrage**, sondern die Entscheidung des
Eigentümers über seinen eigenen Namen. Sie steht als Punkt 5 in 16.9; der
Scrum Master beschließt dazu nichts.

### 16.6 Was gut lief — mit Beleg

- **Zwei Impedimente sind nach vorab festgelegten Kriterien geschlossen
  worden.** I5 gegen ein Vier-Punkte-Kriterium, das **vor** dem Sprint im
  Planning stand (8) und damit nicht nachträglich passend gemacht werden
  konnte; I4 gegen die Bedingung aus Sprint-02, 9.7 (13.9). Das ist der
  Unterschied zwischen „wir hatten kein Problem" und „wir haben nach einem
  Maßstab gesucht, der vorher feststand".
- **Vier Stränge, vier Worktrees, 33 Commits, acht Merges, keine Kollision.**
  Heute nachgemessen: alle vier Story-Zweige vollständig in `main`,
  `git branch --no-merged main` leer, kein `git add -A`-Fund im Sprint-Diff
  (13.9).
- **Die Prüfhaltung hat drei Fehlentscheidungen gekippt, alle drei durch
  Messung.** Zwei beim PO (12.2: eine Berufung auf einen Wireframe, der das
  Gegenteil zeigt; eine Vereinfachung, die den eine Stunde zuvor geheilten
  Fall wiederhergestellt hätte) und eine beim Reviewer, der seine **eigene**
  Empfehlung nach dem Nachmessen zurücknahm. Keine der drei wäre durch
  Nachdenken aufgefallen — jede Begründung trug.
- **DoD 4 über das Geforderte hinaus** (13.5): SPEC 5.1 hält eine Bedingung
  fest, deren Verletzung weder einen Fehler noch einen `integrity-check`
  auslöst. Das ist der Fall, für den B9 gefasst wurde, und er ist von selbst
  gefunden worden.
- **B10 hat bei seiner ersten Anwendung sofort angeschlagen** (M8). Eine
  Prüfung, die beim ersten Lauf etwas findet, war fällig.
- **Der karpathy-Reviewer hat gezielt nach weiteren wertlosen Tests gesucht**
  und zusätzlich gegengeprüft, dass `capturetest::textsFollowAColourSchemeChange`
  gegen den alten Code rot würde — eine Mutationsprobe statt einer Zusicherung
  („keine weiteren gefunden" ist erst dann eine Aussage).

### 16.7 Beschlüsse

Fortlaufend ab **B11** (die Sprint-2-Retro endete bei B10). Jeder Beschluss ist
eine Änderung an einem benannten Artefakt, keine Absichtserklärung.

**B11 — Verbindlicher Sprint-Abschluss in zwei Takten.**
Neuer Abschnitt „Sprint-Abschluss" in `PROZESS.md` mit neun Punkten: Takt 1 vor
der Kundenabnahme (Endstand installiert, jeder Prüflauf als Bericht abgelegt
*vor* der DoD-Prüfung, DoD 1–4 und Doku-Abgleich, Mängelliste an den PO),
Takt 2 nach der Abnahme (Issues und Milestone, Journal, Push, Zweige und
Worktrees geräumt, Vollzugsvermerk durch den Scrum Master). Der Scrum Master
übernimmt die Liste am Sprint-Ende ins Protokoll und hakt sie mit Beleg ab.
*Begründung:* Acht der neun Mängel sind Abschlussmängel (14.5); die beiden
gerissenen Sprint-2-Beschlüsse verlangen Handlungen ohne Ort im Ablauf
(16.1.3); Takt 2 steht knapp zehn Stunden nach Sprintende in fünf Punkten offen
(16.2). Zwei Takte, weil DoD 5 und DoD 6 vor der Abnahme nicht erfüllbar sind
und der Doku-Abgleich vor ihr am Ziel vorbeigeht.
*Geändert:* `docs/scrum/PROZESS.md` (neuer Abschnitt „Sprint-Abschluss").
*Offen — Auftrag an den PO:* Auslöser-Absatz in `CLAUDE.md` (16.10, Auftrag 1).

**B12 — Sprint-Konto: beide Grenzen werden laufend geführt.**
`PROZESS.md`, Sprint-Mechanik: Das Sprint-Protokoll führt Issue-Zahl *und*
Story Points mit Ausgangs- und neuem Stand; jeder Zugang nach der Freigabe wird
gebucht; berührt er eine Grenze, legt der PO ihn dem Kunden **als
Grenzüberschreitung** vor.
*Begründung:* M9 und 12.7 — die Punktzahl wurde bei jedem Zugang mitgezählt,
die Zahl der Issues nicht; das Planning hatte die Schranke für #50 wörtlich
aufgestellt, und bei #54 hat sie niemand wiedererkannt. Die Regel bleibt
unverändert, das Verhalten wird mechanisiert (16.5).
*Geändert:* `docs/scrum/PROZESS.md` (Sprint-Mechanik).
*Offen — Auftrag an den PO:* zwei Zeilen in `CLAUDE.md` (16.10, Auftrag 1).

**B13 — Die Arbeitsweise der Parallelstränge steht in der Prozessdatei, nicht
im Auftragstext.**
`PROZESS.md`, Sprint-Mechanik: je Strang ein Worktree und ein Zweig
(`story/NN-…`, `fix/NN-…`), Ausgangsstand ist gepushtes `main`, Merge nur durch
den PO, ein Strang der `main` braucht rebased statt rückwärts zu mergen, die
Installation nach `/usr` ist ein exklusiver, vom PO getakteter Abschnitt.
*Begründung:* Das Verfahren hat getragen (I5 geschlossen), lebte aber
ausschließlich in den Spawn-Aufträgen — dieselbe Bauart, gegen die B6 gefasst
wurde. Der Rückwärts-Merge `c3e4daf` ist der eine Beleg dafür, dass eine nicht
aufgeschriebene Konvention driftet.
*Ausdrücklich nicht geändert:* `.claude/agents/denkzettel-dev.md`. Die im
Planning (8) gestellte Frage, ob die Worktree-Regel in die Agentendatei gehört,
wird mit **nein** beantwortet: Der Entwickler legt keinen Worktree an, er wird
in einen hineingesetzt — die vier Zweige `worktree-agent-*` zeigen es, sie
stehen sämtlich unberührt auf `59d0d3f`. Die Regel adressiert den PO.
*Geändert:* `docs/scrum/PROZESS.md` (Sprint-Mechanik).

**B14 — Flüchtige Belege werden beim Eintreffen gesichert.**
Ergänzung zu B7 in `PROZESS.md` (Artefakte): Kundenbilder und andere Belege aus
temporären Ordnern werden sofort ins Repo gesichert, nicht am Ende des
Arbeitsschritts.
*Begründung:* 15.3 — sieben von acht Bildern der Abnahme waren nach sieben
Minuten weg; B7 kennt nur die Belege, die Agenten selbst erzeugen.
*Geändert:* `docs/scrum/PROZESS.md` (Artefakte).
*Offen — Auftrag an den PO:* eine Zeile in `CLAUDE.md` (16.10, Auftrag 1), weil
der Empfänger von Kundenbelegen immer der PO ist.

**B15 — Modellzuordnung geprüft und bestätigt, Revisionsvermerk
fortgeschrieben.**
`PROZESS.md`, Modellzuordnung: Der Satz „Revision dieser Zuordnung ist
Retro-Thema" wird durch das Ergebnis dieser Prüfung ersetzt, mit dem Beleg für
das Fable-Sicherheitsnetz und dem nächsten Revisionstermin (Retro nach
Sprint 6).
*Begründung:* 16.4. Ein offener Vermerk ohne Ergebnis ist nach drei Sprints
kein Vermerk mehr, sondern ein Rückstand.
*Geändert:* `docs/scrum/PROZESS.md` (Modellzuordnung).

**Nicht beschlossen — und warum nicht.** PR-Verfahren, Versionsschema,
Changelog und der Verwaltungsagent sind **nicht** beschlossen und stehen in
keiner Prozessdatei. Alle vier berühren Kosten, Veröffentlichung oder die
Rollenverteilung und gehören dem Kunden (16.9). Der Abschnitt „Sprint-Abschluss"
sagt das ausdrücklich, damit die Auslassung als Absicht erkennbar bleibt und
nicht als Lücke nachgetragen wird.

### 16.8 Abschlussprüfung — zwei Fragen je Beschluss, schriftlich

| Beschluss | (1) In welchem Artefakt gelandet? | (2) Wird es automatisch gelesen? |
|---|---|---|
| B11 | `PROZESS.md`, Abschnitt „Sprint-Abschluss" | **Teilweise.** Der Scrum Master liest `PROZESS.md` als verbindliche Grundlage (Agentendatei). Der Ausführende von Takt 2 ist der PO — für ihn wirkt sie erst mit dem `CLAUDE.md`-Absatz aus Auftrag 1. **Bis dahin unvollständig verankert.** |
| B12 | `PROZESS.md`, Sprint-Mechanik | **Teilweise**, aus demselben Grund: Der Scrum Master liest sie beim Planning, gebucht wird vom PO. Vollständig mit Auftrag 1. |
| B13 | `PROZESS.md`, Sprint-Mechanik | **Ja.** Adressaten sind Scrum Master (Sprint-Schnitt) und PO (Spawn-Aufträge); der Scrum Master liest die Datei ohnehin, und `CLAUDE.md` verweist auf sie als vor jeder Arbeit zu lesende Vereinbarung. |
| B14 | `PROZESS.md`, Artefakte | **Teilweise.** Der Adressat ist der PO im laufenden Kundengespräch — das ist der Moment, in dem niemand eine Prozessdatei aufschlägt. Wirkt erst mit Auftrag 1. |
| B15 | `PROZESS.md`, Modellzuordnung | **Ja.** Er richtet sich an Scrum Master und PO beim Spawn; beide lesen die Datei bzw. den Verweis aus `CLAUDE.md`. |

**Ehrliche Bilanz dieser Prüfung: Drei von fünf Beschlüssen sind heute nur zur
Hälfte verankert.** Sie stehen im richtigen Artefakt, aber ihr Adressat ist der
PO, und der einzige Ort, den eine PO-Sitzung ohne Zutun liest, ist `CLAUDE.md` —
eine Datei außerhalb der Dateimenge des Scrum Masters. **Genau das ist die
Lücke, die die Sprint-2-Retro zehnmal hatte** (alle Beschlüsse korrekt in
`PROZESS.md`, nur las die niemand von selbst). Sie ist deshalb hier nicht
weggeschrieben, sondern als Auftrag 1 mit fertigem Wortlaut übergeben — und die
nächste Retro prüft nach, ob er ausgeführt wurde.

### 16.9 Was dem Kunden zur Entscheidung vorliegt

1. **Pull Requests.** Vorschlag: ein PR je Story-Strang, ab Sprint 4, als
   Probelauf über **einen** Sprint mit vorab festgelegtem Kriterium (am Ende
   hängt entweder ein Befund an einer Diff-Zeile, der sonst nicht auffindbar
   gewesen wäre, oder ein automatischer Testlauf lief darauf — sonst wird das
   Verfahren eingestellt). Unabhängig davon und sofort: ein Sprint-Basis-Tag,
   der den Prüf-Diff stabil macht, ohne Zwischenstände zu veröffentlichen.
   **Zu wissen:** Bei einem öffentlichen Repo ist jeder gepushte Zweig eine
   Veröffentlichung; heute ist nur `main` sichtbar. *Empfehlung: Tag ja,
   PR als befristeter Probelauf.*
2. **Versionierung.** Vorschlag: `CMakeLists.txt` bleibt die einzige Quelle,
   der Tag `vX.Y.Z` ist das Siegel; 0.x-SemVer; MINOR bei **jeder
   Kundenabnahme**, PATCH für außerplanmäßige Behebungen, 1.0.0, wenn Sie
   sagen, dass das Werkzeug Ihren Alltag trägt; jede Schemamigration erzwingt
   mindestens einen MINOR-Sprung. Die Zahl wird über `--version` sichtbar —
   das ist Produktivcode und braucht eine eigene kleine Story mit SPEC-Eintrag.
   *Empfehlung: annehmen; #41 (PKGBUILD) hängt daran.*
3. **Changelog.** Vorschlag: `CHANGELOG.md` im Repo nach *Keep a Changelog*,
   gespeist aus den geschlossenen Issues des Milestones, Release-Notes daraus
   erzeugt — nicht umgekehrt. Zwei Zusätze des Scrum Masters: rein technische
   Einträge bleiben draußen, und **jede Schemaänderung bekommt einen
   Pflichteintrag** (dass ein Update die Datenbank umschreibt, steht heute nur
   in einem Test). *Empfehlung: annehmen.*
4. **Verwaltungsagent auf kleinem Modell.** Vorschlag: ein Agent
   `denkzettel-verwalter` (Haiku), der ausführt und nicht entscheidet — Zweige
   räumen, Abgleichsberichte, Changelog-Entwürfe, Issues **als Vollzug** einer
   protokollierten Abnahme schließen; ausdrücklich nicht: AK-Haken setzen,
   Mängel beheben, Versionssprünge bestimmen, Changelog-Texte formulieren.
   *Empfehlung: annehmen, aber erst nach B11* — vorher gibt es keine Liste,
   die er abarbeiten könnte. Er bekommt dann auch eine Zeile in der
   Rollentabelle.
5. **Was in das öffentliche Repository geschrieben werden darf.** Heute stehen
   dort wörtliche Kundenzitate, Messwerte von Ihrem Rechner und interne
   Fehleranalysen — vertretbar, aber nie entschieden (12.6). *Empfehlung:
   Zitate und Messwerte weiter zulassen, aber keine Systemdetails und keine
   personenbezogenen Angaben; wenn Ihnen das zu weit geht, sagen Sie es, dann
   wird umformuliert statt gelöscht.*
6. **Push-Kadenz.** Offen seit dem 02.08.: Soll der PO vor jedem Push Bescheid
   sagen, oder nach jedem abgeschlossenen Arbeitsblock pushen? *Empfehlung:
   nach jedem Arbeitsblock ohne Rückfrage* — der Push war in drei Sprints
   zweimal der letzte offene Punkt (33 Commits am Sprint-3-Ende, aktuell einer),
   und die Sichtbarkeit ändert sich dadurch nicht, weil `main` ohnehin
   veröffentlicht wird.

### 16.10 Umsetzungsaufträge an den Product Owner

Der Scrum Master ändert weder Code noch SPEC noch Agentendateien noch
`CLAUDE.md` noch Memory. Was dort hingehört, steht hier mit fertigem Wortlaut.

**Auftrag 1 (verankert B11, B12, B14) — drei Absätze in `CLAUDE.md`,
Abschnitt „Die Regeln, die am häufigsten übergangen werden":**

> **Ein Sprint endet nicht mit dem letzten Commit.** Der Abschluss steht als
> Liste in `docs/scrum/PROZESS.md`, Abschnitt „Sprint-Abschluss", und wird
> Punkt für Punkt im Sprint-Protokoll abgehakt: vor der Kundenabnahme
> installieren, Belege ablegen, prüfen — nach ihr Issues schließen, Journal,
> Push, Zweige und Worktrees räumen. Acht der neun Mängel aus Sprint 3 waren
> Abschlussmängel.
>
> **Jeder Zugang nach der Sprint-Freigabe wird gebucht** — Issues *und* Punkte.
> Berührt er eine der beiden Grenzen (2–4 Stories, ~13 SP), legt der PO ihn dem
> Kunden als Grenzüberschreitung vor. In Sprint 3 wurde bei jedem Zugang die
> Punktzahl mitgezählt, die Zahl der Issues nicht — die Grenze fiel niemandem
> auf.
>
> **Flüchtige Belege sofort sichern.** Kundenbilder liegen in temporären
> Ordnern; von acht Bildern der Sprint-3-Abnahme überlebten sieben Minuten nur
> eines.

**Auftrag 2 — offene Punkte aus Takt 2 des Sprint-3-Abschlusses** (16.2), zu
erledigen bevor Sprint 4 startet: die Verfahrensangabe „(Sprint 3 in der
Kundenabnahme)" aus `README.md:7` entfernen — die Statuszeile beschreibt den
gelieferten Stand, nicht den Stand der Abnahme (`PROZESS.md`,
Sprint-Abschluss, Takt 1 Punkt 3); **`docs/scrum/retro/sprint-03/` committen —
die Stellungnahme des Entwicklers ist untracked (16.1.2, Nachtrag)**; `main`
pushen (dann zwei Commits); die vier Worktrees entfernen und die acht
vollständig gemergten Zweige löschen (`git worktree remove`, dann
`git branch -d`; 296 MB). Danach meldet der PO den Vollzug, und der Scrum
Master trägt ihn nach.

**Auftrag 3 — karpathy-Review der Prozessänderung.** Fällig nach der globalen
Regel (Regel-/Prozess-Artefakt geändert) und nach `PROZESS.md`,
Retrospektiven. Entwurf des Auftrags:

> **Was geändert wurde:** `docs/scrum/PROZESS.md` — neuer Abschnitt
> „Sprint-Abschluss" (neun Punkte in zwei Takten), zwei neue Punkte in der
> Sprint-Mechanik (Sprint-Konto, Parallelarbeit), eine Ergänzung bei den
> Belegen (flüchtige Belege), das Ergebnis der Modellrevision. Dazu Abschnitt
> 16 in `docs/scrum/sprints/sprint-03.md` (Retro-Protokoll).
>
> **Worauf zu schauen ist:**
> 1. **Prinzip 2:** Erzeugt die Abschlussliste Bürokratie, die niemand füllt?
>    Jeder ihrer neun Punkte muss auf einen Mangel aus 13.12 oder auf eine
>    heute gemessene offene Stelle (16.2) zurückführbar sein. Punkte ohne
>    solchen Beleg gehören gestrichen.
> 2. **Prüfbarkeit:** Ist jeder Punkt gegen einen Befehl oder ein Artefakt
>    entscheidbar, oder hängt er am Ermessen? Der Sprint-2-Befund zu DoD 4
>    („was zählt als Bedingung?") ist die Fehlerbauart, um die es geht.
> 3. **Prinzip 1:** Die Retro behauptet, B4 und B7 seien gerissen, weil sie
>    „keinen Ort im Ablauf" haben (16.1.3). Trägt diese Erklärung, oder ist sie
>    eine plausible Begründung, die den falschen Schluss stützt? Der
>    Gegen-Fall wäre: B1/B2/B3 haben denselben fehlenden Ort und hielten
>    trotzdem.
> 4. **Prinzip 3:** Sind die `PROZESS.md`-Änderungen chirurgisch, oder ist
>    angrenzender Text mitverändert worden?
> 5. **Widerspruchsfreiheit:** Kollidiert der neue Abschnitt mit DoD 2, DoD 5,
>    DoD 6 oder dem Doku-Abgleich unter der DoD-Liste — insbesondere die
>    Aufteilung in zwei Takte?
>
> **Ergebnisformat:** `pass`/`fail`/`Hinweis` je Befund mit Fundstelle;
> Bericht als Datei unter `docs/scrum/reviews/` (B7, und neu: B11 Takt 1
> Punkt 2), ungekürzt.

### 16.11 done / next

**done:** Reguläre Retro nach Sprint 3 moderiert. Die zehn Beschlüsse der
Sprint-2-Retro gegen den **Verlauf** von Sprint 3 geprüft statt gegen die Doku
— acht haben nachweislich gewirkt, B4 ist gerissen (M1) und B7 dreifach
(M3, M4 und der bis heute unbemerkte erste karpathy-Lauf ohne Bericht);
Gemeinsamkeit der gerissenen belegt: Sie verlangen Handlungen ohne Ort im
Ablauf. Der Ist-Zustand des Abschlusses knapp zehn Stunden nach Sprintende
nachgemessen (fünf Punkte offen, darunter 8 Zweige, 296 MB Worktrees und —
während der Retro über B7 gefunden — die untrackte Stellungnahme, auf der
Abschnitt 16.3 ruht). Die
vier Kundenthemen mit der Dev-Stellungnahme durchgearbeitet, ihr in zwei
Punkten belegt widersprochen (die Labels sind **nicht** deckungsgleich mit den
Keep-a-Changelog-Kategorien; der PR wird gegen „nichts" statt gegen den
billigeren Sprint-Basis-Tag verglichen) und je einen entscheidbaren Vorschlag
samt Empfehlung formuliert. Modellzuordnung revidiert und bestätigt, mit dem
einen Fund als Beleg, den nur das Fable-Sicherheitsnetz gefunden hat. Fünf
Beschlüsse **B11–B15** gefasst und noch in dieser Sitzung in `PROZESS.md`
umgesetzt; die Abschlussprüfung offen ausgewiesen, dass drei davon bis zur
Ausführung von Auftrag 1 nur halb verankert sind.

**next:** (1) PO führt Auftrag 1 aus (drei Absätze in `CLAUDE.md`) — ohne ihn
wirken B11, B12 und B14 nur für den Scrum Master. (2) PO erledigt Auftrag 2
(README, Push, Zweige und Worktrees) und meldet den Vollzug; der Scrum Master
trägt ihn nach. (3) PO ruft den karpathy-Review nach Auftrag 3 auf; Bericht
ungekürzt unter `docs/scrum/reviews/`. (4) Kunde entscheidet die sechs Punkte
aus 16.9; erst danach kommen Version, Tag und Changelog in den Abschnitt
„Sprint-Abschluss". (5) Sprint-4-Planning: Das Sprint-Konto nach B12 wird von
der ersten Zeile an geführt; erste Kandidaten sind #11 (S8) und #10 (S7) aus
3.3, dazu die Aufträge aus der Abnahme (#60, #51, #52). (6) Nächste reguläre
Retro nach Sprint 6; sie prüft als Erstes nach, ob Auftrag 1 ausgeführt wurde.
