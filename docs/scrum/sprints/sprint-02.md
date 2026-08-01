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

**Fortschreibung am Sprint-Ende (01.08.2026): keine neuen Fälle, Zählstand
weiter acht.** Der PO hat nach dem Planning in jeden Auftragstext einen
ausdrücklichen Zustellhinweis aufgenommen („Nachrichten erreichen den
Empfänger erst in dessen nächstem Zug — sende ab und beende dich normal").
Seither lieferten **alle fünf** Folge-Agenten des Sprints ihre Berichte
ordnungsgemäß ab: `dev-s4`, `dev-t2`, `dev-s5`, `ux-review-s5` und
`dev-heal-s5`. Das ist die erste ununterbrochene Serie seit Beobachtungsbeginn
und stützt die oben formulierte Zustellweg-These deutlich: Geändert wurde
nicht das Verhalten der Agenten, sondern die **Erwartung des Empfängers** an
den Zeitpunkt der Zustellung. Für die Retro nach Sprint 3 heißt das, die
Prüfung des Zustellwegs bleibt der erste Schritt — die Evidenz spricht jetzt
gegen schärfere Berichtsauflagen als Gegenmaßnahme.

**I4 — Automatisierte Prüfbarkeit UI-lastiger Stories (offen, jetzt mit
Teil-Gegenmaßnahme).** In Sprint 2 wird das Impediment akut: S5 ist genau die
Story-Klasse, die es benennt. Neu gegenüber Sprint 1 ist der UI-Review durch
`denkzettel-ux` gegen Wireframe, SPEC und HIG (PROZESS.md, DoD 3) — eine
Kontrollinstanz, die es beim Verfassen von I4 noch nicht gab. Sie ersetzt
den automatisierten Test nicht, prüft aber gegen eine schriftliche Referenz
statt gegen Augenmaß. Die manuelle Checkliste nach SPEC 16 bleibt Pflicht.

**I5 — Git-Hygiene bei parallel arbeitenden Agenten (neu, Gegenmaßnahme
wirkt).** Zwei Vorfälle am 31.07./01.08., beide vom PO gemeldet:

- Ein `git add -A` nahm den unversionierten Wireframe-Stand eines anderen
  Agenten in einen Story-Commit auf.
- Ein `git commit --amend` erwischte einen fremden, bereits gepushten Commit;
  die entstandene Historien-Divergenz musste per Rebase geheilt werden.

Wirkung: Beide Male war nicht der Inhalt falsch, sondern die Zuordnung — ein
Sprint-Diff, dessen Commits fremde Arbeit enthalten, ist für den karpathy-
Review nicht mehr sauber abgrenzbar. **Vom Scrum Master am Endstand
nachgeprüft:** Die Historie ist geheilt; `1ada199` und `0dd6251` tragen
ausschließlich die Wireframe-Datei, die vier S4-Commits ausschließlich
Quellcode und Tests (Prüfung: `git show --stat` je Commit). Der Schaden ist
also behoben und im Sprint-Diff nicht mehr sichtbar — er wird hier
festgehalten, weil er sonst aus der Evidenz verschwände.

Gegenmaßnahme seit dem zweiten Fall: verbindliche Arbeitsregeln im
Auftragstext (gezielt stagen statt `add -A`, kein `amend`). Seither kein
weiterer Fall — `dev-s5` und `dev-heal-s5` arbeiteten regelkonform.
**Für die Retro:** Diese Regeln stehen derzeit in jedem einzelnen
Auftragstext. Das ist dieselbe Bauart von Gegenmaßnahme, die bei I3 ins Leere
lief, und sie hängt daran, dass der PO sie jedes Mal erneut hinschreibt.
Vorschlag zur Prüfung in der Retro: dauerhaft in die Agent-Definition
`.claude/agents/denkzettel-dev.md`, nicht in den Auftrag.

## 7. Sprint-Review

**Geprüft am:** 2026-08-01, 00:55–01:28 (Ganymed) durch den Scrum Master.
**Prüfgrundlage:** eigener frischer Build, eigener Testlauf, drei eigene
Mutationstests, Quellcode, Commit-Inhalte, Issue-Stände, Vault-Journal und
eine Bus-Introspektion — nicht die Meldungen des Product Owners. Alle Belege
sind unten benannt und wiederholbar.

**Sprint-Diff:** `3a5e0c9..54ae35d` (19 Commits). `HEAD` steht auf `54ae35d`,
`git log origin/main..main` ist leer, der Arbeitsbaum sauber: der geprüfte
Stand ist der veröffentlichte.

**Abdeckung des karpathy-Reviews — an einem Ende geschlossen, am anderen
benannt.** Nach unten ist keine Lücke: Zwischen dem Sprint-1-Endstand
`48ffb70` und `3a5e0c9` liegen vier Commits, die **ausschließlich**
Prozess-Artefakte berühren (`docs/scrum/`, `.claude/agents/`) — kein
Quellcode. Nach oben besteht eine, und sie ist bauartbedingt: `54ae35d` ist
die **Antwort** auf den Review (seine eigene Nachricht sagt „Review-Befunde
aus dem S5-UI-Review und dem karpathy-Review"), also lief der Review, bevor
dieser Commit existierte. 175 Zeilen in `librarywindow.cpp`,
`notelistmodel.cpp` und `librarytest.cpp` hat kein karpathy-Durchgang gesehen.

Der Scrum Master hat diese Lücke deshalb **selbst geschlossen**, statt sie
stehenzulassen: Die drei Mutationstests aus 7.1 richten sich genau auf diesen
Commit und prüfen jede seiner drei Änderungen einzeln gegen einen mutierten
Produktivstand. Das ersetzt keinen vollständigen Review-Durchgang, belegt aber
das, worauf es bei einem Heilungs-Commit ankommt: dass die neuen Tests die
geheilten Fehler tatsächlich fangen. Sprint 1 hatte dieselbe Form
(Review → Heilung → „DoD 3 erfüllt") und hat sie nicht benannt; hier ist sie
benannt und belegt. Ob der Kunde einen zweiten Review-Durchgang über
`1e70e81..54ae35d` verlangt, ist seine Entscheidung — der Scrum Master hält
ihn bei diesem Umfang nicht für nötig.

### 7.1 Prüfbelege

- **Frischer Build** (`cmake -S . -B <tmp> -DCMAKE_BUILD_TYPE=Debug`,
  `cmake --build -j8`): Exit 0, **null Warnungen** in Konfiguration und Bau.
  Die Warnstufe ist real (KDECompilerSettings, siehe Sprint-01, 8.1).
- **Tests** (`ctest`): **7 von 7 grün**, 3,70 s. Davon sind **sechs
  projekteigen**, `appstreamtest` ist wie in Sprint 1 eine ECM-Zugabe ohne
  Aussage über Projektcode. Die sechs im Einzelnen — `storetest` (9
  Testfunktionen), `capturetest` (7), `librarytest` (36), `shelltest` (8),
  `firstruntest` (2) und `installtest`, das kein QTest-Binary ist, sondern ein
  CMake-Skript: Es installiert per `DESTDIR` in ein Staging-Verzeichnis und
  prüft, dass Anwendungs- und Autostart-Eintrag dort ankommen.
- **Testzahl von `librarytest` präzisiert.** QTest meldet „38 passed"; das
  sind 36 Testfunktionen plus `initTestCase`/`cleanupTestCase`. Die
  PO-Angabe „34 → 38" ist rechnerisch belegt: Der Heilungs-Commit fügt genau
  vier Testfunktionen hinzu (`git show 54ae35d -- tests/librarytest.cpp`).
- **Drei eigene Mutationstests am Heilungs-Commit**, in einem separaten
  Worktree auf `54ae35d`, jeweils gegen den grünen Ausgangsstand:
  1. `showLibrary()` auf die alte Bedingung `!isVisible()` zurückgedreht →
     `readsTheStoreAgainWhenTheOpenWindowIsShownAgain` fällt (1 von 38).
  2. `m_deletion->flush()` aus `closeEvent` entfernt → `carriesOutTheDeletion
     WhenTheWindowCloses` **und** `carriesOutTheDeletionWhenTheApplication
     Quits` fallen (2 von 38).
  3. `setMinimumWidth(MinimumListWidth)` entfernt →
     `keepsTheListWideEnoughForThePreview` fällt (1 von 38).
  Alle drei beißen. Mutation 2 ist dabei mehr als ein Testnachweis: Sie belegt
  den **Mechanismus**, auf dem die Widerlegung des karpathy-Befundes B beruht.
  Der Quit-Test wird nur dann rot, wenn `closeEvent` während `quit()`
  tatsächlich läuft — die Löschung nimmt also nachweislich den Weg über das
  Fensterschließen, nicht über einen Sonderpfad.
- **Bus-Introspektion, mit einem Befund zur Prüfmethode.**
  `busctl --user introspect org.denkzettel.Daemon /Daemon` weist auf dem
  laufenden Prozess **nur `ShowCapture`** aus, nicht `AddNote`, `ShowLibrary`
  und `Quit`. Das ist **kein Codebefund**: Der Prozess (PID 255272) läuft seit
  31.07., 21:48 — also seit vor dem S4-Commit `3fe30b9` (23:53); das Binary auf
  der Platte ist von 01.08., 01:16 und wurde nie neu gestartet. Der Quellcode
  exportiert alle vier Methoden als `Q_SCRIPTABLE` über
  `ExportScriptableSlots` (`src/shell/daemonservice.cpp:15–19`,
  `daemonservice.h:33–43`); `shelltest` deckt `AddNote` und `ShowLibrary`
  fachlich ab (4 der 8 Funktionen). **Konsequenz für die Abnahme:** Vor den
  qdbus-Prüfungen muss der Daemon neu gestartet werden, sonst prüft der Kunde
  den Sprint-1-Stand. Dieser Punkt steht in 7.4 als Vorbedingung.
  `AddNote` und `Quit` hat der Scrum Master **nicht** aufgerufen — das eine
  schriebe in die Produktivdatenbank, das andere beendete einen Prozess des
  Nutzers (Melden, nicht heilen).
- **Codeprüfung.** Konflikterkennung über
  `KGlobalAccel::globalShortcutsByKey(sequence, KGlobalAccel::Equal)`
  (`src/shell/globalshortcuts.cpp:56`) mit eigenem Modul `shortcutconflict`
  für die Auswertung; Komponentenname aus `desktopFileName() + ".desktop"`
  abgeleitet statt doppelt geschrieben (Zeile 21–24); die fehlgeschlagene
  Registrierung meldet über `KNotification` statt still zu scheitern
  (Zeile 65–74). Erststart als eigenes Modul `firstrun` mit Marker, der
  ausdrücklich nur die einmaligen Schritte schützt. In S5: `KStandardShortcut::
  undo()`/`close()` verdrahtet, Suchfeld deaktiviert mit Tooltip, alle
  sichtbaren Texte über `i18n()`, Fließtexte in Infinitivform nach SPEC 15
  („Zum Lesen links eine Notiz auswählen.").

### 7.2 DoD-Matrix

Die sechs Punkte aus PROZESS.md, je Story.

| DoD | S4 (#5) | T2 (#6) | S5 (#7) |
|---|---|---|---|
| 1 Build warnungsarm, Tests grün | erfüllt¹ | erfüllt | erfüllt |
| 2 AK erfüllt, PO-Abnahme | **offen**² | **offen**³ | **offen**⁴ |
| 3 karpathy-reviewer ohne `fail` | erfüllt⁵ | erfüllt⁵ | erfüllt⁶ |
| 4 SPEC/KONZEPT nachgezogen | entfällt⁷ | **offen**⁸ | erfüllt⁹ |
| 5 Commit + Issue geschlossen | **offen**¹⁰ | **offen**¹⁰ | **offen**¹⁰ |
| 6 Journal-Eintrag der Session | erfüllt¹¹ | erfüllt¹¹ | erfüllt¹¹ |

¹ Mit einer benannten Abweichung vom Wortlaut, wie sie S1 in Sprint 1 hatte
(Sprint-01, Fußnote ¹): Die eigentliche Registrierung
`GlobalShortcuts::registerCaptureShortcut()` hat **keinen** automatisierten
Test — sie spricht mit einem laufenden `kglobalacceld` und ist ohne Bus- und
Compositor-Fixture nicht sinnvoll unit-testbar. Automatisiert abgedeckt ist
die reine Auswertungslogik im eigens dafür herausgezogenen Modul
`shortcutconflict` (4 der 8 `shelltest`-Funktionen). Den Nachweis für den
Registrierungsweg tragen die manuellen Punkte 1–4 aus 7.4. Begründete
Abweichung, kein Mangel — aber sie ist der Grund, warum DoD 2 für S4 ohne die
Sichtprüfung nicht schließbar ist.

² Codeseitig sind alle vier AK belegt (7.1). Offen sind die manuellen Punkte
aus 7.4 — Meta+N am echten Compositor, das Kürzel im Systemeinstellungs-KCM,
der Konfliktzweig und die drei qdbus-Aufrufe. Genau der Fall, den I4 benennt.

³ Beide manuell abnehmbaren AK waren beim Planning vorab benannt (3.3) und
sind es geblieben: Installationslauf und Sitzungswechsel. `installtest` belegt,
dass die Install-Regeln existieren und beide Einträge ankommen — **nicht**,
dass eine Sitzung das Zielverzeichnis liest. Der Test sagt das über sich
selbst (`tests/installtest.cmake`, Kopfkommentar); diese Ehrlichkeit ist ein
Positivbefund, kein Mangel.

⁴ Alle acht AK sind automatisiert oder im UI-Review belegt; offen sind die
fünf Sichtprüfpunkte aus 7.4.

⁵ Sprint-Ende-Review über den Sprint-Diff, Gesamtverdikt **warn**, kein
`fail`. Damit ist DoD 3 in der Grundfassung erfüllt. Zur Abdeckungsgrenze des
Reviews und ihrer Schließung siehe den Kopf von Abschnitt 7.

⁶ S5 ist UI-Story (4.1), für sie gilt DoD 3 in der erweiterten Fassung. Der
UI-Review hatte **einen `fail`**; er ist mit `54ae35d` geheilt und vom Scrum
Master per Mutationstest nachgeprüft (7.1). Keine offenen `fail`-Befunde.

⁷ S4 setzt SPEC 2.3 und 2.4 um, ohne eine Festlegung zu ändern.

⁸ Mangel M2 (7.5). T2 hat eine harte Installationsbedingung entdeckt, die in
SPEC 15 fehlt. Der Scrum Master führt diesen Punkt bewusst als **offen** und
nicht als `entfällt`, weil Sprint 1 denselben Sachverhalt so bewertet hat: Dort
machte die fehlende Nennung zweier Build-Abhängigkeiten DoD 4 für S1 und S3
offen (Sprint-01, 8.2 und Mangel M2). Eine unvollständig gebliebene Festlegung
zweimal verschieden zu bewerten, würde die Vergleichbarkeit der Protokolle
zerstören.

⁹ SPEC 15 ist mit `9ddd64a` um **KWidgetsAddons** (KMessageWidget) und
**KWindowSystem** (KWindowConfig) ergänzt, mit `1bdcfdf` um die app-weite
Infinitivform. Abgleich des Scrum Masters: Die im Build tatsächlich
angeforderten Komponenten (`CMakeLists.txt:21–31`: Qt6 DBus/Widgets/Sql; KF6
Config, DBusAddons, GlobalAccel, I18n, Notifications, StatusNotifierItem,
WidgetsAddons, WindowSystem) sind **vollständig** in SPEC 15 genannt.
**Sprint-1-Mangel M2 wiederholt sich in dieser Form nicht** — die
Abhängigkeitsliste ist vollständig; offen ist allein die Paketierungsbedingung
aus Fußnote ⁸.

¹⁰ Mangel M1 (7.5).

¹¹ Vault-Daily `2026-08-01.md` enthält vier Einträge im Abschnitt
`## Claude Code Protokoll` (00:04 S4, 00:20 T2, 00:37 S5, 01:12 Reviews und
Heilung), die den gesamten Umsetzungszeitraum abdecken.
**Sprint-1-Mangel M3 wiederholt sich nicht.**

### 7.3 Review-Ergebnisse

Die Berichte selbst lagen dem Scrum Master nicht vor; die Zusammenfassung
stammt vom PO. Was der Scrum Master eigenständig nachgeprüft hat, ist
jeweils benannt.

**karpathy-Review über den Sprint-Diff — Gesamt `warn`, kein `fail`.**
Methodik: frischer Build, Testlauf, drei Mutationstests (Konflikterkennung,
Sieben-Tage-Grenze der Zeitstempel, Sofortausführung beim zweiten Löschen —
letztere riss Unit- und Fenstertest zugleich). Prinzipien 1–3 ohne Befund,
mit Positivbefunden: explizit gemachte Annahmen, eine Selbstkorrektur
(`7b6756c` — ein Kommentar, der zu viel behauptete), chirurgische Diffs.
Prinzip 4 `warn` mit zwei Randfällen:

- **(A) `showLibrary()` lud bei offenem Fenster nicht nach.** Trifft zu,
  geheilt, nachgeprüft (Mutation 1).
- **(B) Verdacht, das D-Bus-`Quit()` lasse die Löschfrist verfallen.**
  **Widerlegt** — siehe unten.
- Dritter Punkt, kein Befund: Die drei qdbus-Aufrufe gehören wörtlich auf die
  manuelle Checkliste. Übernommen in 7.4.

**UI-Review durch `denkzettel-ux` — ein `fail`, sonst `pass`.** Der `fail` war
deckungsgleich mit Randfall A und nannte zusätzlich den Fokusklau beim
erneuten Zeigen. Alle acht S5-Akzeptanzkriterien sonst erfüllt; SPEC 9
eingehalten — echtes `DELETE`, kein Soft-Delete-Zustand in der DB. Damit liegt
das im Review-Auftrag (4.5, Punkt 2) ausdrücklich als `fail` benannte
Kriterium **nicht** vor. Bewertungsfragen: QLocale-Punktform des Wochentags
korrekt; der Wireframe war an dieser Stelle eine Näherung (AK in #7 vom PO
korrigiert); QSplitter HIG-konform unter der Auflage einer Mindestbreite von
rund 220 px; Nur-Größe-Persistenz ist wörtlich von SPEC 15 gedeckt
(KWindowConfig, Position setzt ein Wayland-Client nicht selbst); die
Auslassungen sind sauber begründet (Bearbeiten → S8, Tag-Chips → M3, Player →
M4). Nicht blockierende Politur-Hinweise: Layout-Abstände nicht vom Stil
bezogen, fehlendes Mnemonic am Löschen-Knopf (vom Scrum Master am Code
bestätigt: `src/ui/librarywindow.cpp:238` schreibt `i18n("Löschen")` ohne
`&`), App-Icon statt eines semantischen Leerzustands-Icons. **Vormerkung für
M5:** Das Fenster hat keine Menüleiste; SPEC 8.3 verlangt später eine.
Nebenauftrag Titelzeile der Wireframe-Datei erledigt (`0dd6251`).

**Heilungslauf `54ae35d` — Befund A behoben, Befund B mit Messung widerlegt.**
Nachladen jetzt an `!isPending()` statt an `!isVisible()`; die Auswahl folgt
der Notiz-**ID** statt der Zeilennummer, mit Guard gegen −1 in
`NoteListModel::rowOf`; Fokus wandert nur beim echten Öffnen in die Liste;
der Lesebereich scrollt bei gleichem Text nicht zurück. Vier neue Tests, zwei
davon vorher rot gesehen.

Zu **Befund B**: Qt 6 hat `quit()` von „Ereignisschleife beenden" auf
„Fenster schließen, dann beenden" umgestellt, `closeEvent` läuft also auch auf
diesem Weg; der karpathy-Verdacht beruhte auf Qt-5-Semantik. Statt
Produktivcode entstand der Regressionstest
`carriesOutTheDeletionWhenTheApplicationQuits`. **Der Scrum Master hat diese
Widerlegung nicht geglaubt, sondern nachgemessen** (Mutation 2, 7.1): Der Test
hängt nachweislich am `flush()` im `closeEvent`. Die Widerlegung trägt. Die
benannte Restgrenze — bei SIGKILL oder Absturz überlebt die Notiz — ist von
SPEC 9 nicht gefordert und damit kein Mangel.

Zusätzlich im Heilungslauf: die Mindestbreite 220 px als benannte Konstante
`MinimumListWidth` mit eigenem Test, wie vom UI-Review als Auflage verlangt.

### 7.4 Offene Punkte fürs Kunden-Review

Die Meilensteine M1 **und** M2 werden mit diesem Sprint erstmals sichtprüfbar;
die Checklisten nach SPEC 16 fallen deshalb zusammen an. Reihenfolge ist
bewusst: Die Vorbedingung steht oben, weil ohne sie zwei Prüfungen den
falschen Stand messen.

**Vorbedingung (Befund des Scrum Masters, 7.1).** Der Daemon auf dem Bus läuft
seit dem 31.07., 21:48 und ist damit der **Sprint-1-Stand**. Vor allen
Prüfungen: laufende Instanz beenden und das aktuelle Binary starten. Ohne
diesen Schritt zeigt die Introspektion nur `ShowCapture`, und Meta+N erreicht
einen Prozess ohne Kürzel-Registrierung.

**M1 / S4 (#5) — fünf Punkte:**

1. Meta+N am echten Plasma/Wayland: Öffnet es das Capture-Fenster mit sofortigem
   Tastaturfokus?
2. Kürzel im Systemeinstellungs-KCM „Kurzbefehle" sichtbar — **erst nach
   systemweiter Installation**, weil kglobalacceld die Komponente über die
   Desktop-Datei auflöst.
3. Komponentenname zur Laufzeit gegenprüfen (registrierter Name gegen
   `org.denkzettel.Denkzettel.desktop`).
4. Konfliktzweig: nur reproduzierbar **nach Löschen des `FirstRunDone`-Markers**,
   weil die Warnung bewusst einmalig ist.
5. Die drei qdbus-Aufrufe über den echten Session-Bus, wörtlich:
   `AddNote` (legt eine Notiz an und liefert deren ID), `ShowLibrary` (öffnet
   das Bibliotheksfenster) und `Quit` (beendet den Dienst). Empfehlung des
   Scrum Masters: `AddNote` mit einem erkennbaren Testtext, damit die Notiz
   hinterher wieder aus dem Bestand zu nehmen ist.

**M1 / T2 (#6) — drei Punkte:**

6. Installation **zwingend mit `-DCMAKE_INSTALL_PREFIX=/usr`**. Nur dieser
   Präfix legt den Autostart-Eintrag nach `/etc/xdg/autostart`, das einzige
   Autostart-Verzeichnis eines unveränderten `XDG_CONFIG_DIRS`
   (Begründung als Kommentar in `CMakeLists.txt:37–45`, Issue #6). **Anmerkung
   des Scrum Masters:** Auf Ganymed löst schon der Vorgabewert auf `/usr` auf,
   weil KDEInstallDirs den Präfix von Qt übernimmt („Installing in the same
   prefix as Qt") — die Angabe ist also Absicherung, nicht Reparatur. Auf einer
   anderen Maschine wäre sie es sehr wohl.
7. Ab- und Anmelden: Läuft `denkzetteld` danach mit der Sitzung?
8. Plasma-Autostart-Modul sichtprüfen, den Eintrag abschalten, erneut anmelden —
   bleibt er aus?

**M2 / S5 (#7) — fünf Punkte:**

9. Tooltip am deaktivierten Suchfeld anhovern (Qt zeigt an deaktivierten
   Widgets keinen Tooltip; die Umsetzung legt ihn deshalb auf ein umgebendes
   Widget — genau das ist hier zu prüfen).
10. Undo-Meldung mit Orca gegenhören: Wird sie vorgelesen?
11. Fenstergröße und Splitterstellung über einen echten Ab- und
    Anmeldevorgang — der Test deckt nur den Prozessneustart ab.
12. Splitter auf das Minimum ziehen und beurteilen, ob die zweizeilige
    Vorschau bei 220 px ihren Zweck noch erfüllt.
13. Löschen mit Rückgängig durchspielen, **einschließlich eines zweiten
    Löschens innerhalb der Frist**: Es darf nur eine Meldung geben, die Frist
    startet neu, das erste Löschen wird sofort ausgeführt.

**Offene Frage an den PO:** Die Übergabe nennt „sechs manuelle
Checklisten-Punkte" aus dem UI-Review, führt aber fünf auf. Der Scrum Master
protokolliert die fünf belegten Punkte und meldet die Differenz, statt einen
sechsten zu erfinden. Falls der UI-Review-Bericht einen weiteren Punkt nennt,
gehört er vor der Abnahme hier hinein.

### 7.5 Mängel

**M1 — Die drei Sprint-Issues sind offen (DoD 5).** #5, #6 und #7 stehen auf
`OPEN`, der Milestone „Sprint 2" weist 0 geschlossene Vorgänge aus. Das ist
zum Prüfzeitpunkt **erwartbar**, weil die Abnahme aussteht — es bleibt
trotzdem der offene DoD-Punkt und ist nach der Sichtprüfung vom PO zu
schließen, mit Commit-Verweis in beiden Richtungen. In Sprint 1 war dies
derselbe Mangel M1; die Wiederholung ist bislang Zeitpunkt-, nicht
Sorgfaltsfrage. **Nebenbefund ohne Handlungsbedarf:** Die Milestone-API meldet
`open=2`, obwohl drei offene Issues zugeordnet sind — ein Zählerartefakt bei
GitHub, die Zuordnung selbst ist korrekt (`gh issue list --milestone
"Sprint 2"` zeigt alle drei).

**M2 — SPEC 15 kennt die Präfix-Bedingung der Installation nicht (DoD 4,
gering).** Der Sprint hat eine harte Bedingung entdeckt: Der Autostart-Eintrag
wirkt nur bei Präfix `/usr`. Sie steht als Kommentar in `CMakeLists.txt` und im
Issue #6 — SPEC 15 sagt weiterhin bloß „zunächst lokales `cmake --install`",
ohne Einschränkung. Dieselbe Liste ist später die Grundlage des PKGBUILD
(S28, M7), und dort ist eine unausgesprochene Präfix-Bedingung teuer; es ist
exakt die Bauart von Lücke, die in Sprint 1 als M2 auffiel. Ein Halbsatz
genügt. **Solange er fehlt, bleibt DoD 4 für T2 offen** (7.2, Fußnote ⁸).
PO-Aufgabe, der Scrum Master ändert SPEC.md nicht selbst.

**M3 — Ein Testkommentar behauptet das Gegenteil dessen, was sein Test belegt
(gering, aber irreführend).** `tests/librarytest.cpp` schreibt über
`carriesOutTheDeletionWhenTheApplicationQuits` wörtlich: „D-Bus Quit() (SPEC
2.3) ends the event loop **without closing the window, so no close event
carries the deletion out**." Der Mutationstest 2 des Scrum Masters zeigt das
Gegenteil: Entfernt man `flush()` aus `closeEvent`, fällt genau dieser Test —
die Löschung nimmt also sehr wohl den Weg über das Schließen. Der Kommentar
gibt damit die **widerlegte** Annahme wieder, während die Commit-Nachricht von
`54ae35d` die richtige Erklärung trägt. Wer den Test später liest, lernt
daraus das Falsche und könnte ihn beim nächsten Qt-Umstieg als gegenstandslos
streichen — obwohl er dann gerade seinen Zweck hätte. Bemerkenswert: Derselbe
Entwicklerlauf hat mit `7b6756c` von sich aus einen Kommentar korrigiert, der
zu viel behauptete; hier ist dasselbe Muster stehengeblieben. Meldung an den
PO, Behebung durch den Dev — der Scrum Master ändert keinen Quellcode.

**Nachtrag PO (01:36):** M2 und M3 sind behoben — SPEC 15 trägt die
Präfix-Bedingung jetzt als Halbsatz mit Begründung, der Testkommentar
beschreibt die Qt-6-Mechanik und den Regressions-Zweck des Tests statt der
widerlegten Annahme (Kommentar-Fix, kein Verhalten; Behebung durch den PO
statt eines eigenen Dev-Laufs — ein Kommentar rechtfertigt keinen Spawn).
DoD 4 für T2 ist damit erfüllt. Die offene Frage aus der DoD-Prüfung zum
„sechsten" Checklisten-Punkt ist geklärt: Der UI-Review führte Fenstergröße
und Splitter-Aufteilung als zwei getrennte Punkte über denselben
Sitzungswechsel; 7.4 fasst sie in einem Punkt zusammen — die Substanz ist
vollständig, keine Protokolländerung nötig.

### 7.5.2 Abnahme-Ergebnis der ersten Sichtprüfung (Nachtrag PO, 01:48)

**Die Sichtprüfung ist in ihrem ersten Durchlauf gescheitert — Sprint 2 ist
nicht abgenommen.** Drei Kundenbefunde, alle dokumentiert:

1. **Meta+N ohne Wirkung** (Issue #5, AK 1). Diagnose-Momentaufnahme des PO:
   Die Komponente existiert bei kglobalacceld überhaupt nicht
   (`allMainComponents` ohne denkzettel, Komponentenpfad `UnknownObject`) —
   die Registrierung schlägt still fehl, eine Meldung erschien nicht
   (Konfliktprüfung nur beim Erststart; `FirstRunDone` war gesetzt).
   Verdachtsmoment: Prüfung lief ohne systemweite Installation der
   Desktop-Datei. Details im Issue-Kommentar.
2. **Bibliotheks-Layout kaputt** (Issue #7): Suchfeld schwebt in der
   Fenstermitte, große Leerflächen, Liste beginnt erst in der unteren
   Fensterhälfte — Wireframe 2b verlangt Kopfzeile oben mit voller Resthöhe
   für Liste und Detail. Exakt das I4-Restrisiko: 38 Tests prüfen Logik,
   nicht Geometrie.
3. **Tray-Linksklick ohne Wirkung** (neues Issue #44, typ:bug): Konvention
   ist Linksklick = Hauptaktion, Rechtsklick = Menü; Denkzettel reagiert nur
   auf Rechtsklick. Bestand aus S1, vom Kunden in der Abnahme gefunden.

**Bestanden:** die drei qdbus-Aufrufe am echten Session-Bus (AK 3 von #5).

Nächste Schritte: Diagnose- und Heilungslauf zu den Befunden 1 und 2,
danach erneute Sichtprüfung (die übrigen elf Punkte aus 7.4 stehen noch
aus — der Kunde brach nach den ersten Befunden ab). Befund 3 ist
Backlog-Material und wird beim Sprint-3-Planning priorisiert.

### 7.6 Sprint-Ziel

**Inhaltlich erreicht, formal vorbehaltlich der Sichtprüfung.** Das Ziel
lautete: „Meta+N öffnet das Capture-Fenster aus jeder Sitzung heraus — und was
aufgeschrieben wurde, lässt sich in einem Bibliotheksfenster durchblättern und
lesen." Beide Hälften sind gebaut und automatisiert belegt, soweit sie sich
automatisiert belegen lassen; alle 10 Story Points sind inhaltlich
abgearbeitet. Die verbleibende Unsicherheit ist genau die, die I4 vorhergesagt
hat — die dreizehn Punkte aus 7.4 sind Sichtprüfung, nicht Bauarbeit.

Ein Vorbehalt zur Formulierung „aus jeder Sitzung heraus": Diese Hälfte des
Ziels hängt an T2 und ist damit **erst nach dem Ab- und Anmeldevorgang aus 7.4
belegt**. Bis dahin ist belegt, dass das Kürzel in einer laufenden Sitzung
registriert wird — nicht, dass es nach einem Neustart von allein wieder da
ist.

## 8. done / next

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

**done (Fortschreibung am Sprint-Ende, 01.08.2026):** Alle drei Stories in der
geplanten Reihenfolge umgesetzt (S4 → T2 → S5, 10 SP); karpathy-Review über
den Sprint-Diff (`warn`, kein `fail`) und UI-Review zu S5 (ein `fail`)
durchlaufen, der `fail` mit `54ae35d` geheilt; DoD-Prüfung gegen frischen
Build, Testlauf, drei eigene Mutationstests, Quellcode, Commit-Inhalte,
Issue-Stände, Vault-Journal und eine Bus-Introspektion geführt — 0 Warnungen,
7/7 Tests grün (sechs davon projekteigen), Sprint-Diff-Abdeckung des Reviews
belegt; die Widerlegung des karpathy-Befundes B eigenständig nachgemessen;
drei Mängel (M1–M3) und dreizehn Sichtprüfpunkte plus eine Vorbedingung
benannt; Impediment I3 ohne neuen Fall fortgeschrieben (fünf von fünf Agenten
lieferten ab — Stützung der Zustellweg-These), I5 „Git-Hygiene bei parallelen
Agenten" neu aufgenommen. Sprint-1-Mangel M3 (Journal) hat sich **nicht**
wiederholt; M2 nur in anderer Gestalt — die Abhängigkeitsliste ist diesmal
vollständig, es fehlt die Paketierungsbedingung.

**next:** Der Kunde führt die dreizehn Sichtprüfpunkte aus 7.4 durch — zuerst
die Vorbedingung, den veralteten Daemon zu ersetzen, sonst messen zwei
Prüfungen den Sprint-1-Stand. Danach schließt der PO #5, #6 und #7 mit
Commit-Verweis (Mangel M1), ergänzt SPEC 15 um die Präfix-Bedingung (M2) und
lässt den irreführenden Testkommentar in `tests/librarytest.cpp` durch den Dev
korrigieren (M3); die Differenz beim sechsten UI-Checklistenpunkt (7.4) ist
aufzuklären. **Beide Bedingungen aus 4.4 für S8 sind erfüllt** — vom Scrum
Master nachgeprüft: Wireframe 2a und die 1b-Variante im M2-Stand liegen mit
`1ada199` vor, und #11 führt die neugefassten Interaktions-AK samt der
ausdrücklich vermerkten Herauslösung des Vorschlags-Kriteriums nach S18a
(#29, M5). S8 ist damit in Sprint 3 ziehbar; die 2 SP sind laut Issue beim
Planning gegen die neuen AK zu bestätigen. Sprint-3-Planning danach:
technische Schranke ist S6 + T3 + S7 (7 SP), ergänzbar um S8 (2 SP) —
Priorisierung und Kundenfreigabe wie gehabt. Hinweis 5.3 ist erledigt
(#43 hängt am Milestone „Sprint 1"), und der Journal-Eintrag zur
Planungssitzung ist geschrieben (Daily 2026-07-31, 23:34 Uhr). **Ein
Journal-Punkt bleibt:** Der Eintrag von 01:12 im Daily 2026-08-01 steht auf
„DoD-Prüfung läuft" — er braucht einen Abschluss oder einen Folgeeintrag mit
dem Ergebnis dieses Reviews, sonst ist DoD 6 rückwirkend unvollständig und
wiederholt sich als Mangel in Sprint 3.

## 9. Retrospektive (01.08.2026)

**Datum:** 2026-08-01, 10:27–10:36 (Ganymed)
**Moderation:** Scrum Master
**Teilnehmer:** Scrum Master · Product Owner · Entwickler (`dev-retro-s2`) ·
UI/UX (`ux-retro-s2`). Beide Fachrollen haben unabhängig Stellung genommen,
ohne den Bericht der jeweils anderen zu kennen; die Belege liegen unter
`docs/scrum/retro/sprint-02/`.

**Anlass — Kundenanweisung, nicht Kadenz.** PROZESS.md sah die erste Retro
erst nach Sprint 3 vor. Der Kunde hat sie am 01.08.2026 vorgezogen; seine
Entscheidung überstimmt die vereinbarte Kadenz. Kritik sinngemäß im Wortlaut:
Der zweite Sprint lief nicht gut, das Kürzel funktioniert nicht, das
Bibliotheks-Layout war kaputt — „Wozu haben wir einen UI/UX-Experten, wenn so
etwas passiert?" Auftrag: die Arbeitsweise so anpassen, dass diese Probleme
nicht wiederkehren, und über Werkzeuge nachdenken (Vorgabe: Open Source, ins
Claude-Code-Ökosystem integrierbar; Kundenbeispiel semgrep).

**Prüfgrundlage des Scrum Masters.** Nicht nur die beiden Stellungnahmen,
sondern eigene Nachprüfung am Quellstand `091fcc5`: `src/ui/librarywindow.cpp`
fügt den Splitter in Zeile 162 tatsächlich ohne Stretch-Faktor ein; eine Suche
über `tests/librarytest.cpp` nach `y()`, `height()`, `geometry()` und
`sizeHint()` liefert **null Treffer** — die Testdatei enthält keine einzige
Geometrie-Zusicherung; beide Belegbilder wurden angesehen und zeigen den
Kundenbefund und seine Heilung; `pacman -Si clazy` weist 1.17.1-1.1 in
`cachyos-extra-v3` aus (nicht installiert); `.claude/settings.json` führt das
Plugin `semgrep@claude-plugins-official` als aktiviert.

### 9.1 Befund 1 — Meta+N ohne Wirkung (#5)

**Ursachenkette, lückenlos belegt.** kglobalacceld behandelt jeden
Komponentennamen mit `.desktop`-Endung als Service-Action-Komponente und
verlangt eine auflösbare Desktop-Datei; findet es weder einen KService noch
eine Datei unter `<Datenverzeichnis>/kglobalaccel/<name>`, gibt
`createServiceActionComponent()` `nullptr` zurück und legt **gar keinen**
Eintrag an (Beleg im Fremdquellcode `plasma/kglobalacceld`,
`src/globalshortcutsregistry.cpp`). Genau das erklärt den Kundenbefund
„`allMainComponents` ohne denkzettel, Komponentenpfad `UnknownObject`".
Die Desktop-Datei war nie installiert: `/usr/share/applications/` und
`/etc/xdg/autostart/` kennen keinen denkzettel-Eintrag, und **keine der fünf
Dateien** aus `build/install_manifest.txt` existiert. Zweite, unabhängige
Quelle aus dem Journal des Kundenlaufs: `Failed to register with host portal …
"Could not register app ID: App info not found for 'org.denkzettel.Denkzettel'"`.

**Warum keine Meldung kam.** `KGlobalAccel::setGlobalShortcut()` kann einen
Backend-Fehlschlag strukturell nicht anzeigen: `doRegister()` setzt den
D-Bus-Aufruf ab und **wertet dessen Ergebnis nicht aus**; `false` kommt nur bei
Müll-Tastencode oder namenloser Aktion zurück. Der Meldezweig in
`globalshortcuts.cpp:65–74` hängt also an einem Wert, der Fehlschläge nicht
kennt. Gebaut wurde eine *Konflikt*erkennung; das Akzeptanzkriterium verlangte
eine *Fehlschlag*meldung.

**Erste Korrektur der bisherigen Protokolllage.** Abschnitt 7.5.2 führt als
Teilerklärung „Konfliktprüfung nur beim Erststart; `FirstRunDone` war gesetzt".
Das trifft nicht zu. Die Erststart-Bindung in `src/main.cpp:82` ist an diesem
Befund **unbeteiligt** und keine Umsetzungslücke: SPEC 2.4 schreibt die Prüfung
wörtlich „beim Erststart und bei Kürzel-Änderung" vor, die Umsetzung folgt der
Spec. Es gab zudem nichts zu melden — dieselbe SPEC-Stelle hält fest, dass
Meta+N auf Ganymed frei ist; die Konfliktliste war korrekt leer.

**Zweite Korrektur — sie trifft den Scrum Master selbst.** Punkt 4 der
Sichtprüfliste in 7.4 („Konfliktzweig nur reproduzierbar nach Löschen des
`FirstRunDone`-Markers") hätte den Fehler **nicht** aufgedeckt: Auch mit
gelöschtem Marker wäre keine Meldung erschienen, weil kein Konflikt vorlag. Der
vom Scrum Master entworfene Prüfweg zielte am Fehler vorbei.

**Der Stachel: Das Wissen lag im Team vor.** Punkt 2 derselben Liste sagt
„erst nach systemweiter Installation, weil kglobalacceld die Komponente über die
Desktop-Datei auflöst". Diese Erkenntnis wurde nur auf die *Sichtbarkeit im
Systemeinstellungs-Modul* angewandt, nie auf die Frage, ob das Kürzel ohne
Installation überhaupt funktionieren kann. Die Antwort ist nein, und sie stand
implizit schon da. Drei Handgriffe hätten den Fehler im Entwicklerlauf
aufgedeckt, jeder einzeln: ein Blick ins Journal
(`journalctl --user -t denkzetteld -n 20`, dort stand um 23:51 und 23:58
`Couldn't start kglobalaccel … ServiceUnknown`), ein Lesebefehl gegen den
Daemon, oder eine programmatische Rückprüfung mit
`KGlobalAccel::self()->globalShortcut(...)`.

### 9.2 Befund 2 — Bibliotheks-Layout (#7)

**Ursache: eine fehlende Zahl.** `src/ui/librarywindow.cpp:162` fügt den
Splitter ohne Stretch-Faktor in das äußere `QVBoxLayout` ein. Ein horizontaler
`QSplitter` hat vertikal die Größenpolitik **Preferred**, die Kopfzeile als
nacktes `QWidget` ebenfalls — damit hat kein Posten eine Ausdehnungsrichtung,
und Qt verteilt die Überschusshöhe gleichmäßig. In der aufgeblähten Kopfzeile
zentriert Qt das `QLineEdit`, weil ein Eingabefeld feste Höhe hat: das
„schwebende Suchfeld".

Beide Fachrollen haben **unabhängig voneinander** am echten `LibraryWindow`
gemessen (offscreen, 900×600) und kommen auf dieselben Zahlen: Kopfzeile
`y=0 h=300`, Suchfeld `y=137`, Splitter `y=300 h=300` — je exakt die halbe
Fensterhöhe. Mit Stretch-Faktor 1: Kopfzeile `h=41`, Suchfeld `y=8`, Splitter
`y=41 h=559`. Die Bilder `ux-echt-ist.png` und `ux-echt-soll.png` in der
Retro-Akte zeigen beides; der Scrum Master hat sie angesehen — das Ist-Bild ist
der Kundenbefund, ohne Abstriche.

**Warum 38 Tests das nicht fingen.** Nicht wegen fehlender Infrastruktur: Die
steht vollständig (`QT_QPA_PLATFORM=offscreen` in `tests/CMakeLists.txt`, an
neunzehn Stellen echte Fenster mit `qWaitForWindowExposed`). Es fehlt die
geprüfte **Eigenschaft** — null Zusicherungen über Position oder Höhe, vom
Scrum Master per Suchlauf bestätigt. Über das Suchfeld sagt der Test nur
`QVERIFY(search->isVisible())`, und ein Widget in der Fenstermitte ist sichtbar.
Der Test war nicht falsch, er prüfte das Falsche. Der Kopfkommentar der
Testdatei (Zeilen 29–32) hatte die Grenze sogar schriftlich benannt — „the
window itself — layout … stays on the manual checklist" —, nur stand das Layout
auf keiner manuellen Liste.

### 9.3 Antwort auf die Kundenfrage zum UI/UX-Experten

Die Frage ist berechtigt, und die Antwort ist unbequem.

**Der UI-Review hat das Fenster nie gesehen.** Er lief rein statisch am Code:
geprüft wurde, ob die richtigen Bedienelemente existieren, beschriftet,
übersetzt, tastaturbedienbar und in richtiger Reihenfolge angelegt sind — nicht,
**wo sie landen**. Für die gesamte Fehlerklasse „Raumaufteilung" hatte das
Verfahren kein Prüfmittel. `denkzettel-ux` fällt darüber das Verdikt **`fail`
über den eigenen Review**, methodisch und im Einzelfall, und benennt zusätzlich
das Übersehen: Die Ursache war ohne laufendes Programm an einer Zeile erkennbar,
und der Testkopf hat schriftlich auf die Lücke hingewiesen. Dieselbe Rolle hatte
im selben Review die *waagerechte* Achse abgesichert (Mindestbreite 220 px, als
Test `keepsTheListWideEnoughForThePreview`) — es war kein Nicht-Können, sondern
ein blinder Fleck ohne Systematik.

**Der Prozess hat diese Lücke gedeckt, und daran trägt der Scrum Master
mit.** Der Review-Auftrag in 4.5 stammt von ihm; er verlangt „Abgleich gegen den
Wireframe" und nennt fünf Sichtprüfpunkte — **kein Punkt betrifft die
Raumaufteilung, und kein Satz verlangt ein Bild**. Die DoD-Matrix in 7.2 hat den
UI-Review anschließend als erfüllt gebucht, ohne zu fragen, woran er das Layout
gemessen hat. Impediment I4 hatte genau dieses Restrisiko benannt und den
UI-Review als Teil-Gegenmaßnahme geführt (Abschnitt 6) — eine Gegenmaßnahme, der
das Prüfmittel fehlte.

**Was die Rolle im selben Sprint nachweislich geleistet hat**, ohne dass es das
Obige entschuldigt: neun AK-Ergänzungen an #7, die Zurückstellung von S8 wegen
fehlender Zeichnung (das blockierte unabhängig von jeder AK-Formulierung und hat
Fehlplanung verhindert), und der eine `fail` zum Nachladen bei offenem Fenster,
deckungsgleich mit dem karpathy-Befund A. Ein Review, der Existenz und Semantik
prüft, aber nie das Bild sieht, bleibt trotzdem ein Code-Review mit
UI-Vokabular.

### 9.4 Was gut lief

- **Beide Ursachen waren binnen einer Sitzung eingegrenzt** — mit
  Fremdquellcode, Journalauszügen und je einer eigenen Messung am echten
  Fenster. Zwei unabhängige Messungen kamen auf identische Zahlen.
- **Beide Fachrollen haben ihr eigenes Versagen zuerst benannt**, statt Werkzeug
  oder Auftrag verantwortlich zu machen. Der Dev korrigiert dabei zwei
  Protokollaussagen zu seinen eigenen Ungunsten (9.1).
- **Das Herausziehen der Auswertungslogik nach `shortcutconflict`** hat sich
  bewährt: Der nicht testbare D-Bus-Teil ist von der testbaren Regel getrennt.
- **Die Logikabdeckung von S5 trug tatsächlich** — Löschfrist, Undo-Kanten,
  Zeitstempel, Nachladen waren beim Kunden inhaltlich in Ordnung.
- **Die Mutationstests des Scrum Masters** erzeugten Erkenntnis statt
  Bestätigung: Mutation 2 widerlegte die im Testkommentar behauptete Qt-Mechanik.

### 9.5 Beschlüsse

Acht Beschlüsse. Jeder ist heute umgesetzt; das geänderte Artefakt steht dabei.

**B1 — Selbst-Sichtprüfung des Entwicklers vor der Übergabe.**
Bei jeder Story mit sichtbarem oder systemweit registriertem Verhalten startet
der Entwickler den gebauten Stand, führt den Hauptweg einmal selbst aus und legt
den Nachweis in den Bericht (Terminalausgabe, Journalauszug oder Bild).
*Begründung:* Beide Befunde wären daran gescheitert — Meta+N an der leeren
Komponentenliste, das Layout am ersten Blick auf ein `grab()`. Kein Dev-Lauf des
Sprints hat je ein gerendertes Fenster erzeugt.
*Geändert:* `docs/scrum/PROZESS.md` (DoD 2) · `.claude/agents/denkzettel-dev.md`
(neuer Abschnitt „Vor der Übergabe").

**B2 — Geometrie-Zusicherungen als Testfunktionen je Ansicht.**
Jede Aussage des Wireframes über die Raumaufteilung wird als Testfunktion
festgehalten, geprüft bei zwei Fenstergrößen; offscreen genügt.
*Begründung:* Zusammenführung von Dev-V2 und UX-(b) — derselbe Beschluss aus
zwei Richtungen. Die Wirksamkeit ist gemessen, nicht behauptet: Am Sprint-Stand
wären die Zusicherungen rot gewesen (137 statt < 40; 300 statt > 450). Kein
neues Werkzeug nötig. Vom Pixelvergleich wird ausdrücklich abgeraten —
Schriftrendering und Theme machen ihn flackrig, und eine Wache, die grundlos
anschlägt, wird ignoriert.
*Geändert:* `docs/scrum/PROZESS.md` (DoD 1).

**B3 — Der UI-Review ist ohne Bild nicht geführt.**
Der Entwickler legt je UI-Story einen Screenshot pro Wireframe-Zustand bei
(Normalfall, Leerzustand, Meldungszustand). `denkzettel-ux` erzeugt
**zusätzlich eigene** Bilder aus dem Sprint-Stand und prüft sie gegen den
Wireframe; die Prüfpunkte leitet er aus dem Wireframe ab — jeder gezeichnete
Bereich erzeugt genau eine Prüffrage —, nicht aus dem Gedächtnis. Ein UI-Review
ohne eigene Bildprüfung zählt für DoD 3 nicht.
*Begründung:* 9.3. Der Weg ist erprobt und kostet rund fünf Minuten (out-of-source
bauen, Helfer gegen `denkzettelui` linken, offscreen, `grab().save()`); die
Dev-Bilder allein genügen nicht, sonst hängt die Prüfung wieder an einer fremden
Meldung.
*Geändert:* `docs/scrum/PROZESS.md` (DoD 3) · `.claude/agents/denkzettel-ux.md`
(Modus 3).

**B4 — Geprüft wird der installierte Stand.**
Vor der Sichtprüfung wird mit `-DCMAKE_INSTALL_PREFIX=/usr` installiert;
Prüfling ist die installierte Binärdatei, nicht das Build-Verzeichnis.
*Begründung:* Befund 1 lag genau in dieser Lücke. Der Sprint hat die Regel
zweimal berührt (7.4 Punkte 2 und 6), ohne sie zu ziehen. Nebeneffekt: Der
Desktop-Datei-, D-Bus- und Portalpfad wird mitgeprüft — im Build-Verzeichnis ist
er prinzipiell nicht prüfbar.
*Geändert:* `docs/scrum/PROZESS.md` (DoD 2).

**B5 — Keine stillen Fehlpfade; Registrierungen werden zurückgelesen.**
Wo eine Registrierung bei einem fremden Dienst stattfindet (KGlobalAccel,
D-Bus-Namen, Tray, Portale), fragt der Code anschließend beim Dienst nach, ob
sie angekommen ist, und meldet den Fehlschlag bei jedem Start sichtbar. Dazu die
Prozessregel: **Eine im Bericht benannte Grenze der Prüfbarkeit schließt die
Story nicht** — sie wird geschlossen oder als Impediment eskaliert.
*Begründung:* „Meldet, wenn der Rückgabewert `false` ist" war nachweislich
wirkungslos (9.1). Die Grenze stand im S4-Bericht und wurde als Fußnote abgelegt,
während die Story als AK-erfüllt gemeldet wurde — das ist der Kernfehler des
Laufs, und er ist eine Prozess-, keine Codefrage.
*Geändert:* `.claude/agents/denkzettel-dev.md` (Kodierregel) ·
`docs/scrum/PROZESS.md` (DoD 2, Satz zur Prüfgrenze).

**B6 — Git-Regeln dauerhaft in die Agentendefinition (I5).**
Gezielt stagen, nie `git add -A`, nie `git commit --amend`.
*Begründung:* Die Regeln wirkten, standen aber in jedem einzelnen Auftragstext
und hingen daran, dass der PO sie jedes Mal erneut hinschreibt — dieselbe Bauart
Gegenmaßnahme, die bei I3 ins Leere lief.
*Geändert:* `.claude/agents/denkzettel-dev.md`.

**B7 — Review- und Retro-Belege gehören ins Repo, nicht ins Scratchpad.**
UI-Review-Berichte samt geprüften Bildern unter `docs/scrum/reviews/`,
Retro-Stellungnahmen und Messbelege unter `docs/scrum/retro/sprint-NN/`.
*Begründung:* In Sprint 2 lag der UI-Review-Bericht dem Scrum Master **gar nicht
vor** (7.3, Zeile 632); er musste sich auf die Zusammenfassung des PO stützen —
eine DoD-Prüfung gegen eine Behauptung statt gegen ein Artefakt. Sitzungs-
Scratchpads sind flüchtig; was ein Protokoll behauptet, muss im Repo liegen.
*Geändert:* `docs/scrum/PROZESS.md` (Artefakte) ·
`.claude/agents/denkzettel-ux.md` (Berichtspflicht, Schreibzugriff) · vollzogen
mit dieser Retro-Akte.

**B8 — Kadenz-Regel an die Wirklichkeit angepasst.**
Die Retro-Kadenz bleibt (nach Sprint 3, danach jede dritte); ergänzt wird, dass
der Kunde jederzeit eine Retro anordnen kann.
*Begründung:* Genau das ist heute geschehen. Ohne den Halbsatz steht die nächste
außerplanmäßige Retro wieder im Widerspruch zur eigenen Arbeitsvereinbarung.
*Geändert:* `docs/scrum/PROZESS.md` (Retrospektiven).

**Nicht beschlossen:** Quellcode-Änderungen. Die Heilung der Befunde 1 und 2 ist
Dev-Arbeit nach der Retro (Ursachen eingegrenzt, siehe „next"). Eine Retro
ändert keinen Produktivcode.

**Beschlossene Folgeaufgabe an `denkzettel-ux` (Gestaltungsmodus):** In die
Festlegungstafel der Wireframes zu 2b/2c (Datei-Zeile 410 ff.) eine Zeile
„Raumaufteilung" aufnehmen, die als Prüfsatz taugt — Kopfzeile oben bündig,
Liste und Detail über die volle Resthöhe, keine Leerfläche dazwischen. Damit
bekommt B2 seine Referenz. Der Scrum Master zeichnet nicht selbst; Beauftragung
durch den PO.

### 9.6 Werkzeug-Empfehlung an den Kunden

**Vorab der unbequeme Befund: Keines der geprüften Werkzeuge hätte einen der
beiden Sprint-2-Befunde gefunden.** Kein Linter kennt einen Check zu
Stretch-Faktoren oder Größenpolitik, und keiner prüft, ob eine D-Bus-
Registrierung beim Daemon ankommt. Werkzeuge helfen hier gegen *andere*
Fehlerklassen. Die Lehre aus Sprint 2 lautet nicht „uns fehlt ein Linter",
sondern: Die Tests prüften die falsche Eigenschaft, und niemand hat das Programm
angesehen, bevor der Kunde es ansah. B1 bis B3 kosten zusammen etwa eine Stunde
und hätten beide Befunde verhindert.

Die Messlatte ist zudem hoch: `KDECompilerSettings` setzt bereits `-Wall
-Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual
-Wzero-as-null-pointer-constant -Wsuggest-override -Wlogical-op` sowie
`QT_NO_CAST_FROM_ASCII`, `QT_NO_KEYWORDS`, `QT_NO_NARROWING_CONVERSIONS_IN_CONNECT`.

**Rang 1 — clazy.** Das einzige Werkzeug im Feld, das Qt-Semantik versteht, und
das, was KDE in seiner eigenen CI fährt. Verfügbar als `clazy 1.17.1-1.1` in
`cachyos-extra-v3` (vom Scrum Master per `pacman -Si` bestätigt, **nicht
installiert**). Findet, was weder GCC noch clang-tidy sehen: sechzehn
Signal/Slot-Checks (`connect-3arg-lambda` — Lambda ohne Kontextobjekt, Absturz
nach Empfängertod; `connect-by-name`; `connect-non-signal`),
`auto-unexpected-qstringbuilder`, `range-loop-detach`, `qstring-allocations`.
Für unseren `connect()`-lastigen Code real. Einstieg `level0,level1`;
Fixit-Automatik **nicht** einschalten (das Upstream-README warnt selbst davor —
im Agentenbetrieb editiert sie hinter dem Rücken des Agenten). Bekannter blinder
Fleck: clazy kennt nur `tr()`-Checks, für `i18n()` ist es blind; diese Lücke
schließt eine ripgrep-Zeile billiger als jedes Werkzeug.

**Rang 2 — clang-tidy mit engem Checkset.** Bereits installiert
(`/usr/bin/clang-tidy`), kostet also nur Konfiguration:
`Checks: '-*, bugprone-*, performance-*, misc-const-correctness'`,
`HeaderFilterRegex` ohne `_autogen`, `SKIP_LINTING` auf `mocs_compilation.cpp`,
und als **eigenes CMake-Target**, nicht als `CMAKE_CXX_CLANG_TIDY` — sonst zahlt
jeder Build die Analyse mit. `bugprone-unused-return-value` nimmt eine eigene
Funktionsliste entgegen und erzwingt damit einen Teil von B5 per Konfiguration;
noch billiger und vom Compiler geprüft ist `[[nodiscard]]` an eigenen Wrappern.
`cppcoreguidelines-*` **nicht** — widerspricht dem Qt-Parent-Child-Modell
systematisch.

**Rang 3 — Screenshot-Helfer als Projekt-Skill unter `.claude/skills/`.** Nichts
zu installieren, sofort wirksam, und es ist genau das Werkzeug, mit dem diese
Retro ihre Belege erzeugt hat. Das Verzeichnis existiert noch nicht (`.claude/`
enthält bisher nur `agents/` und `settings.json`) und wäre anzulegen. Trägt B3
operativ.

**Rang 4 — `selenium-webdriver-at-spi` (KDE) als Prüfauftrag vor M3.** Der
offizielle KDE-Weg für Blackbox-GUI-Tests über AT-SPI, aktiv gepflegt, prüft
Bedienung und Zugänglichkeit in einem: Was der Treiber nicht findet, findet Orca
auch nicht. **Nicht in den Arch-Repos** — Beschaffung und Installation wären
eine Kundenentscheidung nach der Werkzeug-Evaluationsregel, sinnvoll erst vor M3.

**Ergänzend, ohne eigenen Rang:** ImageMagick `compare` (installiert, 7.1.2.29)
taugt als Wächter „hat sich das Bild geändert", **nicht** als Abnahmekriterium —
Theme- und Schriftwechsel erzeugen sonst Dauerfehlalarm. Nested
`kwin_wayland --virtual` ist unerprobt; es wurde bewusst kein nested Compositor
in der laufenden Sitzung des Kunden gestartet. Verworfen mit Begründung:
*include-what-you-use* (kein Qt-6-Mapping, Kernproblem seit 2015 offen),
*cppcheck* (starke Überlappung mit clang-tidy; allenfalls seltener CI-Lauf mit
`--library=qt`), *pre-commit* (greift zum falschen Zeitpunkt — ein Agent schreibt
zwanzigmal und committet einmal), *openQA* und *dogtail* (zu schwer bzw. zu alt).

**Zu semgrep — die Evidenz des Teams steht gegen das Kundenbeispiel.** Das ist
unbequem und wird deshalb ausgeschrieben statt weggelächelt.

Der Entwickler lehnt semgrep für unser C++ mit Belegen ab: Die C++-Unterstützung
der OSS-Engine ist ausdrücklich experimentell (Hersteller wörtlich: „the
languages will stay experimental"; interprozedurale Analyse nur im
Bezahlprodukt), die Regel-Registry enthält kein `cpp/`-Verzeichnis, und `#ifdef`
bricht den Parser (offenes P0 von April 2026). Der gefährliche Teil ist der
**stille Fehlermodus**: Eine Datei, die nicht vollständig geparst wird, liefert
keine Treffer und sieht aus wie eine bestandene Prüfung. Das ist exakt die
Fehlerbauart, die uns Sprint 2 gekostet hat — eine Wache, deren Schweigen kein
Erfolgsnachweis ist. Die UX-Seite hält eine Regel „`QSplitter` per `addWidget`
in ein `QBoxLayout` ohne Stretch-Faktor" für denkbar, nennt sie aber unerprobt
und niemals als Ersatz für das Bild.

Der Kunde hat während der Retro das Claude-Code-Plugin **„Semgrep Guardian"**
installiert; `.claude/settings.json` führt `semgrep@claude-plugins-official` als
aktiviert (die Datei ist unversioniert und wurde von dieser Retro nicht
angetastet). Es stellt MCP-Werkzeuge für die Semgrep-AppSec-Plattform bereit:
SAST-, Secrets- und Supply-Chain-Befunde, login-gebunden gegen einen
Cloud-Dienst.

Ehrliche Zusammenführung, was es **heute für dieses Repo** leistet:

- *SAST auf unserem Quellcode:* nach obiger Beleglage nicht belastbar. Für die
  zwei Sprint-2-Befunde hätte es nichts geliefert.
- *Supply-Chain:* keine Angriffsfläche vorhanden — das Repo enthält **kein**
  Paketmanifest (kein `package.json`, `Cargo.toml`, `go.mod`,
  `requirements.txt`) und **keine CI** (kein `.github/workflows`); Abhängigkeiten
  kommen aus Distributionspaketen. Das ändert sich mit dem PKGBUILD (S28, M7)
  und dem Tag, an dem eine CI entsteht — **dann** ist die Prüfung sinnvoll.
- *Secrets:* der plausibelste Nutzen. Das Repo ist öffentlich, und SPEC 7.1
  verlangt API-Schlüssel aus KWallet oder Umgebung. Sobald M6 die
  API-Anbindungen bringt, ist ein Secrets-Lauf vor dem Push eine billige
  Zusatzwache — dafür braucht es keine C++-Semantik, nur Mustererkennung.
- *Zu bedenken:* Der Dienst ist login-gebunden und cloud-seitig; Quellcode
  verlässt die Maschine. Bei einem öffentlichen Repo ist das weniger heikel,
  bleibt aber eine bewusste Entscheidung.

**Die Entscheidung über jede Werkzeug-Einführung bleibt beim Kunden** (Regel
`werkzeug-evaluation`). Diese Retro hat recherchiert und gelesen, **nichts
installiert und nichts ausgeführt**. Empfehlung des Scrum Masters in einem Satz:
zuerst B1–B3 wirken lassen, dann clazy `level0,level1` als einmaligen Lauf
ansehen, und das Guardian-Plugin für Secrets und Supply-Chain vormerken, sobald
Manifeste oder CI existieren.

### 9.7 Impediments

**I3 — „Agent meldet sich untätig statt Bericht zu liefern": GESCHLOSSEN.**

Neue Evidenz aus dieser Retro-Runde, vom PO beobachtet: **Beide** Retro-Agenten
meldeten sich zuerst untätig; ihre Berichte trafen jeweils erst im Folgezug ein,
nachdem der PO formlos nachgefasst hatte — **obwohl beide Auftragstexte den
Zustellhinweis trugen**. Genau das entscheidet die Frage. Ein Hinweis an den
*Absender* kann nichts bewirken, wenn der Absender nie das Problem war: Die
Untätig-Meldung überholt die bereits abgesetzte Nachricht. Das ist Mechanik des
Zustellwegs, kein Agentenverhalten.

Damit liegt die These dreifach gestützt vor: die Selbstaufzeichnung des Scrum
Masters in Fall 8 (SendMessage quittierte mit „queued for the main
conversation's next turn", der Bericht stand zusätzlich im Abschlusstext); die
ununterbrochene Serie von fünf Agenten nach Einführung des Zustellhinweises
(Abschnitt 6); und jetzt zwei Agenten, die **trotz** Hinweis erst im Folgezug
ankamen.

**Entscheidung des Scrum Masters:** I3 wird geschlossen, und zwar mit der
Gegenmaßnahme, die der PO vorschlägt — **geänderte Empfänger-Erwartung** statt
schärferer Berichtsauflagen. Eine Untätig-Meldung ohne Bericht ist **kein
Fehlverhalten**; der PO wartet einen Zug ab oder fasst formlos nach. Der Scrum
Master schließt sich dem fachlich an: Die alternative Gegenmaßnahme (Auflagen in
Aufträgen und Agentendateien) ist an acht Fällen nachweislich wirkungslos
geblieben, während die Erwartungsänderung risikolos ist.

*Grenze der Beweislage, benannt statt verschwiegen:* Der Zustellweg wurde nicht
am Werkzeug selbst gemessen, sondern aus dem Verhalten erschlossen — es ist ein
Indizienbeweis. **Wiedereröffnung**, falls ein Agent einen Bericht tatsächlich
nie liefert, auch nicht im Folgezug: Dann läge ein anderes Problem vor als das
hier geschlossene.

**I5 — Git-Hygiene bei parallel arbeitenden Agenten (offen, Gegenmaßnahme jetzt
dauerhaft verankert).** Mit B6 stehen die Regeln in
`.claude/agents/denkzettel-dev.md` statt in jedem Auftragstext. Das Impediment
bleibt **offen**, bis ein Sprint mit mehreren Dev-Agenten ohne Vorfall gelaufen
ist — die Verankerung ist die Maßnahme, nicht der Beleg. Prüfung am Sprint-3-Ende.

**I1** (Werkzeugkette unvollständig) und **I4** (automatisierte Prüfbarkeit
UI-lastiger Stories) bleiben offen. I4 hat mit B2 und B3 erstmals
Gegenmaßnahmen, die auf die Fehlerklasse zielen, an der es gescheitert ist;
geschlossen wird es erst, wenn eine UI-Story diese Kette einmal vollständig
durchlaufen hat.

### 9.8 done / next

**done:** Retro auf Kundenanweisung vorgezogen und moderiert; beide
Fachstellungnahmen unabhängig eingeholt und ihre Kernaussagen am Quellstand
`091fcc5` nachgeprüft (Layout-Zeile, null Geometrie-Zusicherungen in
`librarytest.cpp`, beide Belegbilder gesichtet, `pacman -Si clazy`,
`.claude/settings.json`); Ursachenketten beider Befunde konsolidiert,
einschließlich zweier Korrekturen an der bisherigen Protokolllage — die
Erststart-Bindung ist unbeteiligt und SPEC-konform, und der Prüfweg aus 7.4
Punkt 4 hätte den Fehler nicht aufgedeckt; die Kundenfrage zum UI/UX-Experten
direkt beantwortet (Selbstverdikt `fail` der Rolle, plus der Anteil des Scrum
Masters an Review-Auftrag und DoD-Buchung); acht Beschlüsse gefasst und **am
selben Tag umgesetzt** (`docs/scrum/PROZESS.md`,
`.claude/agents/denkzettel-dev.md`, `.claude/agents/denkzettel-ux.md`);
Werkzeug-Empfehlung mit Rangfolge und ehrlicher semgrep-Abwägung erstellt, ohne
etwas zu installieren; I3 geschlossen, I5 fortgeschrieben; Retro-Akte unter
`docs/scrum/retro/sprint-02/` angelegt (zwei Stellungnahmen, drei Belegbilder).

**next:** (1) Dev-Heilungslauf zu Befund 1 und 2 — die Ursachen sind
eingegrenzt: fehlende systemweite Installation plus fehlende Rücklese-Prüfung
(B5) für das Kürzel, `librarywindow.cpp:162` Stretch-Faktor für das Layout; die
Heilung bringt nach B2 ihre Geometrie-Zusicherungen mit. (2) Gestaltungsauftrag
an `denkzettel-ux`: Zeile „Raumaufteilung" in die Festlegungstafel zu 2b/2c.
(3) Erneute Sichtprüfung durch den Kunden — die elf offenen Punkte aus 7.4
stehen weiterhin aus, jetzt am **installierten** Stand (B4). (4) Entscheidung
des Kunden über clazy, clang-tidy, Screenshot-Skill und AT-SPI-Treiber. (5)
Danach erst Sprint-3-Planning; die Beschlüsse dieser Retro durchlaufen als
Prozess-Artefakt-Änderung den karpathy-reviewer (PROZESS.md, Retrospektiven).
Unverändert offen aus Abschnitt 8: Schließen von #5, #6, #7 nach der Abnahme
(Mangel M1) und der Abschluss des Journal-Eintrags von 01:12.
