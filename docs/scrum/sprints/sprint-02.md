# Sprint 2 — Planning-Protokoll

**Datum:** 2026-07-31, 23:22 (Ganymed)
**Moderation:** Scrum Master (Agent `scrum-master`)
**Teilnehmer:** Scrum Master · Product Owner (Claude Haupt-Session) ·
UI/UX (Agent `denkzettel-ux`, Planning-Beratung zu S5 und S8, Abschnitt 4).
Keine Schätzklausur in dieser Runde — alle betroffenen Stories wurden beim
Sprint-1-Planning von beiden Schätzern unabhängig bewertet; die eine
Neuschätzung (S4) war dort ausdrücklich vorbehalten (Entscheidung E7).
**Status des Sprint-Vorschlags:** freigegeben durch den Kunden am
31.07.2026 · Issues #5 (S4), #6 (T2), #7 (S5) im Milestone „Sprint 2";
AK-Ergänzungen aus Abschnitt 4 in #5 und #7 eingearbeitet, Vorbehalt an S5
damit aufgelöst

Grundlagen: `docs/scrum/PROZESS.md`, `docs/scrum/sprints/sprint-01.md`
(Abschnitte 3, 4, 7–9), `SPEC.md` (Stand 31.07.2026), GitHub Issues
#5–#11 im Repo `hnsstrk/denkzettel`, Stand `main` @ `2dc32e7` (der Quellcode
ist seit `48ffb70` unverändert; `2dc32e7` betrifft nur Prozess-Artefakte).

## 1. Neuschätzung S4 (Issue #5): 5 SP → **3 SP**

Entscheidung des Scrum Masters, wie in Sprint-01-Protokoll 8.3.3 vorgelegt.
Die Neuschätzung war beim Sprint-1-Planning unter Entscheidung E7 vorbehalten
worden; sie wird hiermit **bestätigt**.

### 1.1 Was aus dem Umfang herausfällt

Belegt am Quellstand, nicht an Meldungen:

- **Token- und Zeitstempel-Weg entfällt ersatzlos.** Der T1-Befund
  (Issue #1, SPEC 3) hat den XDG-Activation-Token-Weg widerlegt: KGlobalAccel
  liefert kein Token, der Zeitstempel ist immer 0. Getragen hat das
  Neu-Mappen des Fensters — und das ist mit S3 bereits gebaut, geprüft und
  vom Kunden abgenommen (Sprint-01, 8.7). S4 erbt eine funktionierende
  Fokus-Mechanik, statt eine zu suchen.
- **`ShowCapture()` existiert bereits** (`src/shell/daemonservice.cpp:17`,
  Interface `org.denkzettel.Daemon` per `Q_CLASSINFO` in
  `src/shell/daemonservice.h:15`). Am laufenden Prozess belegt
  (Sprint-01, 8.1: `busctl --user list`, Introspektion von `/Daemon`).
- **Akzeptanzkriterium „Zweitstart ruft ShowCapture statt neuer Instanz"
  ist bereits erfüllt** — die `activateRequested`-Verbindung in
  `src/main.cpp:45` kam aus dem karpathy-Review von Sprint 1
  (Sprint-01, 8.6, Befund 2) und wurde vom Kunden bei der Abnahme
  sichtgeprüft (8.7).

### 1.2 Was hinzukommt

Das neue Akzeptanzkriterium **Konflikterkennung** (SPEC 2.4, Nachtrag in
Issue #5): Registrierungen können unsichtbar fehlschlagen — im Spike war
Meta+F10 von KWin belegt, `invokeShortcut` funktionierte trotzdem, der echte
Tastendruck kam nie an. Der Aufwand ist klein und belegbar: KF6 stellt
`KGlobalAccel::globalShortcutsByKey(seq, MatchType)` und
`isGlobalShortcutAvailable(seq, component)` bereit
(`/usr/include/KF6/KGlobalAccel/kglobalaccel.h`, Zeilen 163 und 175, Paket
`kglobalaccel 6.28.0-1.1`). Es ist ein Abfrageaufruf plus sichtbare Meldung,
keine Eigenkonstruktion.

**Restumfang von S4:** KGlobalAccel-Aktion für `Meta+N` samt Komponentenname
und Desktop-Datei, damit das Kürzel in den Plasma-Systemeinstellungen
erscheint; die D-Bus-Methoden `AddNote(text) → id` und `Quit()`;
Konflikterkennung mit sichtbarer Meldung.

### 1.3 Begründung des Werts

Wegfall des Token-Wegs und zwei bereits gebaute Bestandteile gegen ein neues,
API-gestütztes Kriterium — netto eine Fibonacci-Stufe weniger. **3 SP** ist
kein neu erfundener Wert, sondern **die ursprüngliche Schätzung von
Schätzer A** (Sprint-01, Abschnitt 2: A 3, B 5). B's 5 kam aus dem
Wayland-Aufschlag, den E7 als Querschnittsrisiko benannt hatte; dieses Risiko
ist gemessen und aufgelöst.

**Ehrlichkeitsvorbehalt zu E7.** Die dort formulierte Bedingung lautete
wörtlich „Trägt der xdg-activation-Weg, sind S3 und S4 neu zu schätzen".
Diese Bedingung ist **falsifiziert** — xdg-activation trägt nicht. Ihr Zweck
ist gleichwohl erfüllt: Der Aufschlag stand für die Unsicherheit der
Fokusübernahme, und die ist beseitigt, weil ein anderer Weg gefunden, gebaut
und abgenommen wurde. S3 wurde zu 5 SP geliefert; dessen Neuschätzung ist
gegenstandslos.

### 1.4 Scope-Präzisierung für die Umsetzung

SPEC 2.4 nennt **zwei** Kürzel (`Meta+N` → `ShowCapture()`,
`Meta+Umschalt+N` → `ShowRecorder()`). Die Akzeptanzkriterien von Issue #5
nennen nur `Meta+N`, und `ShowRecorder()` existiert erst mit M4 (S13b).
**S4 registriert ausschließlich `Meta+N`.** Ohne diesen Satz liest ein
Entwickler aus der SPEC einen Scope heraus, den die Story nicht bezahlt.

**Vorschlag an den PO:** Label `sp:5` → `sp:3` an Issue #5. Die Änderung an
GitHub führt der PO aus.

## 2. Abhängigkeitslage in Meilenstein 2

Der PO hat ausdrücklich nach den Abhängigkeiten von S6 und S7 zur Bibliothek
gefragt. Geprüft an den Issue-Beschreibungen und SPEC 6/9:

| Story | hängt an | Beleg |
|---|---|---|
| S5 (#7) | – | Einstiegspunkt in M2; das Fenster trägt alles Weitere |
| S6 (#8) | **S5** | Scope: „FTS5-Tabelle mit Triggern **und Suchfeld in der Bibliothek**"; ohne Bibliotheksfenster hat das Suchfeld keinen Ort |
| T3 (#9) | **S6** | testet die Migration 1→2, die S6 erst anlegt (Planning 4.3: „wird zusammen mit S6 gezogen") |
| S7 (#10) | **S5 + S6** | AK 2: „Suche in der Bibliothek nutzt den Parser" — braucht Suchfeld und FTS-Abfrage |
| S8 (#11) | **S6**, teilweise **M5** | AK 1 verlangt „FTS aktuell" (existiert erst mit S6); AK 2 verlangt das Verwerfen eines offenen Vorschlags — die `proposals`-Tabelle kommt nach Entscheidung E2 erst mit S18a (M5) |

Die Kette lautet damit **S5 → S6 + T3 → S7**, mit S8 hinter S6 und einem
Rest, der bis M5 nicht abnehmbar ist (siehe 5.2). S8 ist inzwischen
zusätzlich aus gestalterischen Gründen zurückgestellt (4.4).

## 3. Sprint-2-Vorschlag

**Sprint-Ziel:** Meta+N öffnet das Capture-Fenster aus jeder Sitzung heraus —
und was aufgeschrieben wurde, lässt sich in einem Bibliotheksfenster
durchblättern und lesen.

| Reihenfolge | ID | Issue | Story | SP |
|---|---|---|---|---|
| 1 | S4 | #5 | Globales Kürzel + D-Bus-Schnittstelle | 3 |
| 2 | T2 | #6 | Autostart und Erststart | 2 |
| 3 | S5 | #7 | Bibliotheksfenster mit Liste und Detail | 5 |
| | | | **Summe** | **10** |

**S5 wurde unter Vorbehalt vorgeschlagen** — die Akzeptanzkriterien in
Issue #7 waren vor Sprint-Start um die Punkte aus der UX-Beratung zu ergänzen
(Abschnitt 4), sonst wäre die Story nach dem Verdikt von `denkzettel-ux`
nicht planungsreif gewesen. **Der Vorbehalt ist erfüllt:** Der PO hat die
Ergänzungen vor der Freigabe in #5 und #7 eingearbeitet (siehe Kopfzeile).
Die Schätzung von **5 SP bleibt unverändert** — der Wegfall der
Sidebar (PO-Entscheidung, 4.3) und die hinzukommende Kopfzeile samt
`ShowLibrary()` heben sich näherungsweise auf; die übrigen Ergänzungen
präzisieren vorhandenen Umfang, statt neuen zu schaffen.

### 3.1 Begründung

- **S4 und T2 schließen Meilenstein 1 ab.** Nach SPEC 17 umfasst M1 Daemon,
  Tray, KGlobalAccel, Text-Capture und SQLite-Store; die ersten vier Teile
  stehen, das Kürzel fehlt. Mit T2 startet der Dienst zudem mit der Sitzung —
  ohne laufenden Dienst gibt es kein Kürzel, weshalb beide zusammengehören.
  Am Sprint-Ende ist die M1-Checkliste aus SPEC 16 fällig.
- **Reihenfolge S4 vor T2.** Beide Stories arbeiten an denselben zwei
  Flächen. Erstens der Startpfad: T2s Akzeptanzkriterium „Erststart legt
  Datenverzeichnis, DB und Default-Konfiguration an" ist derselbe Pfad, an
  den SPEC 2.4 die Konflikterkennung hängt („Beim Erststart und bei
  Kürzel-Änderung"). Die Kopplung läuft für dieses eine Kriterium rückwärts
  zur Reihenfolge — S4 legt den Erststart-Haken provisorisch an, T2
  formalisiert ihn. Das gehört in den Umsetzungsauftrag, sonst entdeckt der
  Entwickler es mitten in der Story. Zweitens die Installationsfläche
  (nächster Punkt).
- **Die Installationsfläche wird in diesem Sprint zum ersten Mal benutzt.**
  Im Repo existiert **keine einzige `.desktop`-Datei**, und die Install-Regeln
  umfassen nur das Binary (`src/CMakeLists.txt:43`) und die Icons
  (`CMakeLists.txt:30`). S4 braucht eine Anwendungs-Desktop-Datei, damit sein
  Akzeptanzkriterium „Kürzel erscheint in den Plasma-Systemeinstellungen
  (Komponentenname/Desktop-Datei passen zusammen)" überhaupt greifen kann;
  T2 braucht zusätzlich einen XDG-Autostart-Eintrag samt Install-Regel. Auch
  das spricht für S4 zuerst.
- **Der dritte Platz war nicht frei wählbar.** Nach S4 und T2 ist in
  `epic:M1` kein offenes Issue mehr übrig (#42 und #43 sind geschlossen), und
  von den M2-Kandidaten ist **S5 der einzige nicht blockierte**: S6 braucht
  das Bibliotheksfenster, T3 braucht S6, S7 braucht beide, S8 braucht S6.
  Der Schnitt ist damit weniger gewählt als erzwungen — jede andere
  Zusammenstellung würde eine Story ziehen, deren Akzeptanzkriterien im
  Sprint gar nicht prüfbar wären.
- **Warum nicht auf 13 SP auffüllen.** Die beiden denkbaren Auffüllungen
  brechen jeweils eine Regel aus PROZESS.md: S6 + T3 zusätzlich ergäbe
  **14 SP und fünf Issues** (Grenzen: ~13 SP, 2–4 Stories; Tech-Stories
  zählen mit, wie T1 in Sprint 1); S6 allein ergäbe zwar 13 SP, träte aber
  die Migration ohne ihren Test los und widerspräche Planning 4.3.
- **Die freien 3 SP sind nicht Leerlauf.** Auf den Sprint fallen vier
  Prüfaufwände ohne Story Points: die M1-Checkliste nach SPEC 16; die erste
  manuelle Sichtprüfung eines Fensters mit Liste, Detail und Undo (I4); der
  erstmals durchlaufende UI-Review durch `denkzettel-ux` — ein Weg, den das
  Team noch nie gegangen ist; und die Abnahme von T2, die nur über einen
  echten Installationslauf und einen Sitzungswechsel zu führen ist (3.3).

### 3.2 Was bewusst draußen bleibt

S6 (#8, 3 SP) und T3 (#9, 1 SP) sind zusammen der natürliche Kern von
Sprint 3, sobald S5 steht; S7 (#10) folgt darauf. **S8 (#11) ist durch
PO-Entscheidung zurückgestellt** — Begründung und Bedingung für das
Wiederaufgreifen stehen in 4.4.

### 3.3 T2 ist nur manuell abnehmbar — vorab benannt statt hinterher entdeckt

Dieselbe Prüfung, die S8 in 5.2 auffällig gemacht hat, auf T2 angewandt:
Zwei seiner drei Akzeptanzkriterien lassen sich **nicht** in der Testsuite
belegen. AK 1 verlangt eine `.desktop`-Datei, die „von der Installation
gelegt" wird, AK 3 die Abschaltbarkeit im Plasma-Autostart-Modul. Beides
setzt einen echten `cmake --install`-Lauf und einen Ab- und Anmeldevorgang
voraus. Ein Paket gibt es erst mit S28 (M7), und der Installationspfad wurde
bisher nie ausgeführt — Sprint-01, 8.1 hält fest, dass `appstreamtest` gegen
ein nicht vorhandenes Installationsmanifest läuft.

**Konsequenz:** Auf die manuelle Checkliste für M1 gehören drei Punkte —
Installation in ein Präfix, Ab- und Anmelden, Sichtprüfung im
Autostart-Modul samt Abschalten und erneutem Anmelden. Ohne diese Prüfung
bliebe DoD 2 für T2 am Sprint-Ende offen, genau wie es S3 in Sprint 1
ergangen ist. Der Umfang von T2 (2 SP) ändert sich dadurch nicht; der
Prüfaufwand ist Sache der Abnahme, nicht der Umsetzung.

## 4. UI-Story-Einstufung und Review-Auftrag S5

### 4.1 Einstufung (PO-Entscheidung)

Welche Stories UI-Stories im Sinne der DoD sind, legt der **PO** beim
Planning fest (PROZESS.md, DoD 3, Kundenentscheidung 31.07.2026). Der Scrum
Master **empfiehlt**, S5 (#7) als UI-Story einzustufen: Sie baut ein
vollständiges Fenster gegen einen vorhandenen Wireframe und gegen SPEC 9.
S4 und T2 haben keine eigene Fensterfläche; für sie ist kein UI-Review
vorgesehen.

**Entscheidung des PO:** Er folgt der Empfehlung. **S5 (#7) ist UI-Story im
Sinne der DoD**, S4 (#5) und T2 (#6) sind es nicht. Damit gilt für S5
DoD-Punkt 3 in der erweiterten Fassung: zusätzlich zum karpathy-Review ein
UI-Review durch `denkzettel-ux` ohne offene `fail`-Befunde (Auftrag in 4.5).

### 4.2 Planning-Beratung `denkzettel-ux` zu S5 (#7): Verdikt **warn**

Der Bericht lag dem Scrum Master beim ersten Entwurf dieses Protokolls nicht
vor und wurde vom PO nachgereicht. Ergebnis: **planungsreif erst nach
AK-Ergänzung im Issue** — ein neues Artefakt ist nicht nötig, der Wireframe
trägt. Neun Ergänzungen an den Akzeptanzkriterien von #7:

1. **Leerzustände als AK** — HIG-konforme Platzhalter für die leere
   Bibliothek und für das Detail ohne Auswahl.
2. **Undo-Kanten festlegen** — Fensterschließen innerhalb der Frist,
   mehrere Löschungen hintereinander, Auswahl und Fokus nach dem Löschen.
3. **Undo-Darstellung**: bleibendes `KMessageWidget` statt
   5-Sekunden-Toast, weil die Löschung endgültig ist (HIG-Empfehlung für
   destruktive Aktionen).
4. **Tastaturwege als AK** — Entf, `KStandardShortcut::undo`,
   Pfeiltasten-Navigation in der Liste.
5. **„erste Zeilen" beziffern** — zwei Zeilen, elidiert.
6. **Relatives Zeitstempelformat** samt Umschaltpunkt als eigenes AK.
7. **Sidebar-Entscheidung** einholen (siehe 4.3).
8. **Zuordnung der Suchfeld-Kopfzeile** klären (siehe 4.3).
9. **AK 3 erweitern** um `ShowLibrary()` (D-Bus, SPEC 2.3) und das
   Verhalten beim Wiederöffnen.

### 4.3 PO-Entscheidungen aus der Beratung

- **Zu (7) — die Sidebar entfällt in M2** und kommt erst mit den Kategorien
  in M3 (S12). Eine Navigationsspalte mit einem einzigen Eintrag ist Fläche
  ohne Funktion (HIG `displaying_content`). Wirkung auf S5: Scope-Reduktion;
  Issue #7 nennt die Sidebar bisher im Scope und ist entsprechend zu
  korrigieren.
- **Zu (8) — S5 legt die Kopfzeile samt noch funktionslosem Suchfeld an**;
  S6 und S7 rüsten nur die Funktion nach. Damit gibt es keinen Layoutsprung
  mitten in M2.
- **Zu (3) — Empfehlung `KMessageWidget` übernommen.** Das berührt SPEC 9,
  die von einem „5-Sekunden-Undo" spricht, ohne die Darstellung
  festzulegen — die Frist bleibt, die Darstellung wird bestimmt. Kein
  Spec-Nachzug nötig; falls die Umsetzung die Frist antastet, greift DoD 4.

### 4.4 Planning-Beratung zu S8 (#11): Verdikt **fail** — zurückgestellt

Drei Befunde:

- **(a) Keine gestalterische Vorlage.** Wireframe 1b vermerkt in Zeile 98
  ausdrücklich „Nur-Lesen"; die Gegenentscheidung aus KONZEPT.md Zeile 124
  und SPEC 9 (Lese- **und** Bearbeiten-Ansicht) wurde nie in den Wireframe
  nachgezeichnet. Damit ist DoD-Punkt 3 für S8 nicht erfüllbar — ein
  UI-Review braucht eine Referenz, gegen die er prüfen kann. **Vom Scrum
  Master an den Quellen nachgeprüft:** Der Wireframe trägt in Zeile 98
  wörtlich „Nur-Lesen · Bearbeiten bewusst weggelassen"; KONZEPT.md Zeile 124
  führt die Gegenentscheidung als „Abweichung vom Wireframe" — die
  Abweichung ist also schriftlich festgehalten, nur eben nie gezeichnet
  worden. Der Befund trifft zu.
- **(b) AK 2 ist M5-abhängig** — deckungsgleich mit dem Befund des Scrum
  Masters aus 5.2, unabhängig erhoben.
- **(c) Keine Interaktions-Akzeptanzkriterien** — Einstieg in den
  Bearbeiten-Modus, Speichern und Abbrechen analog zum Capture-Fenster,
  Umgang mit ungespeicherten Änderungen beim Auswahlwechsel.

**PO-Entscheidung: S8 wird für Sprint 2 nicht gezogen** und bleibt
zurückgestellt. Der Schnitt aus Abschnitt 3 ist davon nicht betroffen — S8
war aus Abhängigkeitsgründen ohnehin nicht vorgeschlagen (Abschnitt 2). Zwei
unabhängige Wege haben hier zum selben Ergebnis geführt.

**Bedingungen für das Wiederaufgreifen** — beide vor dem nächsten Planning,
in dem S8 auftaucht:

1. **Gestaltungsauftrag an `denkzettel-ux`**: Bearbeiten-Screen plus eine
   1b-Variante im M2-Stand (also ohne Sidebar, mit Kopfzeile — Entscheidungen
   aus 4.3). Damit bekommt der spätere UI-Review die Referenz, die ihm heute
   fehlt, und DoD-Punkt 3 wird erfüllbar.
2. **Neufassung der Akzeptanzkriterien** in #11: Interaktions-AK nach Befund
   (c) ergänzen und **AK 2 herauslösen** — das Verwerfen offener Vorschläge
   wandert als eigener Punkt an M5, wo die `proposals`-Tabelle entsteht
   (S18a). Ohne diese Herauslösung bleibt S8 auch mit fertigem Wireframe
   unabnehmbar.

**Ziel-Fenster: Sprint 3.** Beide Bedingungen sind klein und hängen an
niemandem außerhalb des Teams. Damit bleibt der Hinweis des Scrum Masters aus
5.2 gewahrt — die Bearbeiten-Ansicht ist Kernfunktion von M2 und wartet nicht
auf M5, sondern nur auf eine Zeichnung und eine AK-Runde.

### 4.5 Review-Auftrag S5 (Entwurf des Scrum Masters, Aufruf durch den PO)

Fällig am Sprint-Ende, Stand nach den Entscheidungen aus 4.3.

> **Was geändert wurde:** Erstes Bibliotheksfenster (Issue #7, S5) —
> Kopfzeile mit noch funktionslosem Suchfeld, chronologische Notizliste mit
> Zeitstempel und zwei elidierten Zeilen, Detail-Leseansicht, Löschen mit
> 5-Sekunden-Undo über ein `KMessageWidget`, Tray-Menüeintrag „Bibliothek"
> und D-Bus-Methode `ShowLibrary()` verdrahtet. **Ohne Sidebar** —
> PO-Entscheidung, die kommt mit M3.
>
> **Worauf zu schauen ist:**
> 1. Abgleich gegen den Wireframe der Bibliothek
>    (`wireframes/Denkzettel Wireframes.dc.html`, Ansicht 1b) — Abweichungen
>    benennen und einordnen: bewusste Entscheidung oder Versehen? Der
>    fehlende Sidebar-Bereich ist eine bekannte, gewollte Abweichung.
> 2. Abgleich gegen SPEC 9: Liste, Detail lesen, Löschen mit Undo als rein
>    client-seitige Verzögerung (**kein** Soft-Delete-Zustand in der DB —
>    ausdrückliche Spec-Festlegung, ihre Verletzung ist ein `fail`).
> 3. Ob die neun Punkte aus der Planning-Beratung (4.2) in der Umsetzung
>    tatsächlich angekommen sind — insbesondere Leerzustände, Undo-Kanten
>    und Tastaturwege, weil genau die erfahrungsgemäß am Sprint-Ende
>    gekürzt werden.
> 4. KDE Human Interface Guidelines: Abstände, Beschriftungen,
>    Tastaturbedienbarkeit, Verhalten und Platzierung der Undo-Meldung.
> 5. Was die manuelle Checkliste nach SPEC 16 für M2 aufnehmen sollte —
>    Impediment I4 sagt, dass genau diese Story sich automatisierten Tests
>    entzieht.
>
> **Ergebnisformat:** Befunde als `pass` / `fail` / `Hinweis`, jeder Befund
> mit Bezug auf Wireframe, SPEC-Abschnitt oder HIG-Regel. Offene
> `fail`-Befunde blockieren DoD-Punkt 3.

## 5. Hinweise an den Product Owner

**5.1 — GitHub-Aktionen zum Sprint-Start (vom PO erledigt, siehe Kopfzeile).**
Ein Sprint ist nach PROZESS.md ein **Milestone mit den gezogenen Issues**,
nicht nur eine Liste im
Protokoll. Nach Kundenfreigabe fällig war: Milestone „Sprint 2" anlegen; #5, #6
und #7 zuordnen; an #5 das Label `sp:5` durch `sp:3` ersetzen und die
Neuschätzung samt der Scope-Präzisierung aus 1.4 als Kommentar hinterlegen;
das neue Akzeptanzkriterium Konflikterkennung aus dem Nachtragskommentar in
die AK-Liste von #5 übernehmen, damit es beim Abhaken sichtbar ist. **An #7
zusätzlich**: die neun AK-Ergänzungen aus 4.2 einarbeiten und den Scope um
die Sidebar bereinigen (4.3) — ohne das ist S5 nach dem UX-Verdikt nicht
planungsreif, und der Vorbehalt aus Abschnitt 3 bleibt stehen.

**5.2 — S8 (#11): Befund des Scrum Masters, überholt durch PO-Entscheidung.**
Der Befund lautete: Das zweite Akzeptanzkriterium („Steckt die Notiz in einem
offenen Vorschlag, wird dieser verworfen") setzt die `proposals`-Tabelle
voraus, die nach Entscheidung E2 erst mit S18a in M5 entsteht — die Story ist
in M2 nie vollständig abnehmbar. Empfehlung war die bedingte AK-Formulierung
statt einer Verschiebung.

**Diese Empfehlung ist überholt, und zwar aus einem Grund, den der Scrum
Master nicht gesehen hat.** Sie setzte voraus, dass AK 2 das einzige Hindernis
ist — dann hätte eine Umformulierung genügt, um die Story ziehbar zu machen.
`denkzettel-ux` hat AK 2 unabhängig ebenso befundet, aber zusätzlich die
fehlende gestalterische Vorlage gefunden (4.4a), und die blockiert S8
**unabhängig von jeder AK-Formulierung**: Ohne Wireframe für den
Bearbeiten-Screen ist DoD-Punkt 3 nicht erfüllbar, egal wie AK 2 lautet. Die
Entscheidung des PO — Zurückstellung plus Gestaltungsauftrag — ersetzt die
Empfehlung des Scrum Masters. Die AK-Klärung bleibt darin als zweite
Bedingung erhalten (4.4).

**5.3 — Milestone-Zuordnung von #43 ist inkonsistent.** Issue #43 (App-Icon
und Tray-Icon, `epic:M1`, 2 SP) ist geschlossen, hängt aber an **keinem**
Milestone, während das ebenfalls nachträglich entstandene #42 in „Sprint 1"
geführt wird. Wenn nach Sprint 2 behauptet wird, M1 sei abgeschlossen, sollte
die Buchführung das tragen. Zuordnung zu „Sprint 1" nachholen oder die
Auslassung begründen.

**5.4 — Abhängigkeitskette für die Feinreihenfolge.** Aus Abschnitt 2:
S5 → S6 + T3 → S7. S6 und T3 gehören in denselben Sprint (Planning 4.3);
S7 setzt beide voraus. Das legt Sprint 3 auf S6 + T3 + S7 (7 SP) nahe,
gegebenenfalls ergänzt um S8 (2 SP), sobald dessen beide Bedingungen aus 4.4
erfüllt sind — zusammen 9 SP und vier Issues, also innerhalb der Grenzen.
Priorisierung ist PO-Sache, dies ist nur die technische Schranke.

## 6. Impediment-Liste (Fortschreibung)

**I1 — Werkzeugkette auf Ganymed unvollständig (offen, unverändert).**
Betrifft M3 und M4; in Sprint 2 ist keine Story davon berührt.

**I2 — Wayland-Fokusübernahme ungeklärt: GESCHLOSSEN.** Erledigt durch T1.
Beleg: XDG-Activation-Token widerlegt (kein Token, Zeitstempel 0),
Neu-Mappen trägt; in SPEC 3 festgeschrieben, in S3 gebaut und vom Kunden am
31.07.2026 sichtgeprüft (Sprint-01, 8.7). Der Aufschlag, den dieses
Impediment in S4 verursacht hatte, ist mit Abschnitt 1 aufgelöst.

**I3 — Arbeitsmuster „Agent meldet sich untätig statt Bericht zu liefern"
(offen, jetzt acht Fälle).** Zwei neue Fälle in diesem Planning, beide vom
PO gemeldet:

- **Fall 7 — `denkzettel-ux`** hat sich nach der Planning-Beratung untätig
  gemeldet; der Bericht kam erst auf Nachfrage des PO. Bemerkenswert ist,
  dass es ein **frisch angelegter Agent** ist: Das Muster hängt nicht an
  einzelnen Agentendefinitionen, sondern tritt bei jedem neuen Agenten
  erneut auf. Das schließt „Berichtspflicht in die betroffene Agentendatei
  schreiben" als Einzelfallkorrektur aus.
- **Fall 8 — der Scrum Master selbst**, nach dem Schreiben dieses Protokolls
  um 21:27. Beobachtung des PO: Der beauftragte Bericht war nicht da, der PO
  musste nachfassen; der Bericht traf erst **nach** dessen
  Fertigstellungs-Nachricht ein. Aufzeichnung des Scrum Masters: Der Bericht
  wurde per SendMessage an `main` abgesetzt, der Aufruf war erfolgreich und
  quittierte mit „Message queued for the main conversation's next turn";
  derselbe Bericht stand zusätzlich im abschließenden Text des Laufs. Der
  Fall wird **gezählt** — dass der Protokollführer seinen eigenen Fall
  auslässt, würde die Evidenz entwerten. Strittig ist nicht, **ob** der PO
  vor einer Lücke stand, sondern **woran** sie lag.

**Warum die Unterscheidung für die Retro zählt.** Die beiden Beobachtungen zu
Fall 8 widersprechen sich nicht, wenn die Zustellung erst im nächsten Zug des
Empfängers erfolgt: Ein zugestellter, aber noch nicht ausgelieferter Bericht
ist vom Empfänger aus **nicht von Schweigen zu unterscheiden**. Sollte das
zutreffen, wäre ein Teil der acht Fälle kein Verhaltensproblem der Agenten,
sondern ein Zustellungsmechanismus — und die naheliegende Gegenmaßnahme
(schärfere Berichtsauflagen in Aufträgen und Agentendateien) würde ins Leere
zielen. Die Retro nach Sprint 3 sollte deshalb **zuerst den Zustellweg
prüfen** und erst danach über Auflagen entscheiden. Der Scrum Master hat
diese Prüfung nicht selbst vorgenommen — sie greift in die Werkzeugebene ein
und gehört damit vor den Kunden, nicht in ein Sprint-Protokoll (Melden, nicht
heilen). Zählstand bleibt bis zur Klärung bei **acht**.

**I4 — Automatisierte Prüfbarkeit UI-lastiger Stories (offen, jetzt mit
Teil-Gegenmaßnahme).** In Sprint 2 wird das Impediment akut: S5 ist genau die
Story-Klasse, die es benennt. Neu gegenüber Sprint 1 ist der UI-Review durch
`denkzettel-ux` gegen Wireframe, SPEC und HIG (PROZESS.md, DoD 3) — eine
Kontrollinstanz, die es beim Verfassen von I4 noch nicht gab. Sie ersetzt
den automatisierten Test nicht, prüft aber gegen eine schriftliche Referenz
statt gegen Augenmaß. Die manuelle Checkliste nach SPEC 16 bleibt Pflicht.

## 7. done / next

**done:** Neuschätzung S4 entschieden und belegt (5 → 3 SP, Wert von
Schätzer A; Belege: T1-Befund, `src/shell/daemonservice.cpp:17`,
`src/main.cpp:45`, KGlobalAccel-API im installierten KF6); Abhängigkeitslage
in M2 an den Issue-Beschreibungen geprüft (S5 → S6 + T3 → S7, S8 zusätzlich
M5-abhängig); Sprint-2-Schnitt über 10 SP vorgeschlagen; Prüfbarkeit von T2
geklärt (nur manuell abnehmbar, 3.3); Planning-Beratung `denkzettel-ux`
eingearbeitet — S5 mit neun AK-Ergänzungen und drei PO-Entscheidungen
gezogen (Vorbehalt inzwischen erfüllt), S8 auf PO-Entscheidung
zurückgestellt; UI-Review-Auftrag für S5 entworfen; S5 vom PO als UI-Story
eingestuft (4.1); vier Hinweise an den PO; Impediment I2 geschlossen, I3 auf
acht Fälle fortgeschrieben (Fall 8 der Scrum Master selbst; die Ursache ist
offen, mit Prüfauftrag an die Retro), I4 fortgeschrieben. Der Kunde hat den
Schnitt am 31.07.2026 freigegeben; Milestone und AK-Stände in #5 und #7 hat
der PO nachgezogen.

**next:** Umsetzung in der Reihenfolge S4 → T2 → S5 (Begründung 3.1). Am
Sprint-Ende fällig: die manuelle M1-Checkliste nach SPEC 16 einschließlich
der drei T2-Punkte aus 3.3 (Installation in ein Präfix, Ab- und Anmelden,
Sichtprüfung im Autostart-Modul), der karpathy-Review über den Sprint-Diff
und der UI-Review zu S5 nach dem Auftrag in 4.5. Für S8 sind die beiden
Bedingungen aus 4.4 zu erfüllen — Gestaltungsauftrag an `denkzettel-ux` und
AK-Neufassung mit Herauslösung von AK 2 nach M5 —, damit die Story in
Sprint 3 ziehbar wird. Hinweis 5.3 ist erledigt (#43 hängt jetzt am
Milestone „Sprint 1", per API nachgezogen), und der Journal-Eintrag zur
Planungssitzung ist geschrieben (Daily 2026-07-31, 23:34 Uhr) — der
Sprint-1-Mangel M3 wiederholt sich nicht.
