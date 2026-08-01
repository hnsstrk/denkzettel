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
