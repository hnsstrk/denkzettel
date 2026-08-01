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
