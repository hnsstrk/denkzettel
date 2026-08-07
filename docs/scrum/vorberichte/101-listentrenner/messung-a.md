# Vorprüfung #101 — Messung A (UX)

**Story:** #101 „Bibliothek: Notizen und Gruppen heben sich optisch nicht
voneinander ab" (`epic:M2`, `typ:story`, heute ohne `size:`-Label), sieben
Akzeptanzkriterien aus dem PO-Kommentar vom 07.08.2026 ·
**Bearbeiter A**, Rolle UI/UX, Modus **Vorprüfung (Planning-Beratung)** ·
**Stand dieses Berichts:** 07.08.2026, 20:27 CEST, Ganymed ·
**Boden:** `main` bei `88129ba`.

**Prüfstand** (eine Aussage gilt für einen Stand, B17): qt6-base 6.11.1-1.1,
kcolorscheme 6.28.0-1.1, kconfig 6.28.0-1.1, kwidgetsaddons 6.28.0-1.1.
Alle Läufe offscreen mit `QT_QPA_PLATFORMTHEME=kde`; der Skalierungslauf
zusätzlich mit `QT_SCALE_FACTOR=1.6`, der Einstellung des Kunden.

**Wiederholbar:** `bash docs/scrum/vorberichte/101-listentrenner/pruefen.sh`
Der Lauf baut ausschließlich unter `docs/scrum/vorberichte/101-listentrenner/build/`,
fasst `build/` im Projektwurzelverzeichnis nicht an (dort arbeiten die anderen
Stränge), installiert nichts nach `/usr` und ändert keine Einstellung des
Kunden — die Sonden laufen in einem eigenen XDG-Sandkasten.

**Ich habe `messung-b.md` und die Dateien `messungen/mb*.txt` nicht gelesen.**
Sie sind während meines Laufs im Ordner aufgetaucht; die Unabhängigkeit der
beiden Messungen ist der Zweck des Verfahrens.

**Belege dieses Berichts** (alle unter `messungen/`):

| Kürzel | Datei | Inhalt |
|---|---|---|
| M1 | `m1-kanten.txt` | Was der Delegate sieht · Zeilenkanten · wer beim Auswahlwechsel neu gezeichnet wird · Breite mit und ohne Rollbalken |
| M2 | `m2-kanten-skaliert.txt` | dieselbe Sonde unter `QT_SCALE_FACTOR=1.6` |
| M3 | `m3-farbe.txt` | Linienfarbe und Kontrast über alle 18 installierten Schemata |
| M4 | `m4-kontrastwert.txt` | Gruppe, Schlüssel und Herkunft des Wertes `frameContrast` |

Sonden: `sonden/kanten.cpp`, `sonden/farbe.cpp`, `sonden/kontrastwert.cpp`.
Sie linken gegen die im eigenen Bauplatz gebaute `libdenkzettelui.a` und
ändern keinen Produktivcode.

---

## Feld 1 — Dateimenge

Notation nach B13, am Code vermessen (Stand `88129ba`).

| | **#101** |
|---|---|
| **Issue / Zweig** | #101 (`epic:M2`, `typ:story`) · `story/101-listentrenner` |
| **Quellen und Tests** | `src/ui/notelistdelegate.cpp` (168 Z.) — **die Hauptarbeit, an drei Stellen**: die Konstanten `:12–28` (**kein neues Maß nötig**, beide Linien liegen in `VerticalPadding = 9` `:13` und `HeadTopPadding = 14` `:27`), der Kopfzweig in `paint()` `:109–123` (Gruppenlinie in der obersten Bildpunktzeile, **vor** dem `return` `:122`), der Notizzweig `:125–151` (Eintragslinie in der letzten Bildpunktzeile, **nach** `style->drawControl(...)` `:126` — der Aufruf füllt das ganze Rechteck und überdeckte sie sonst). Eine Zeichenhilfe nach dem Muster von `drawLine()` `:62–77`.<br>`src/ui/notelistdelegate.h` (51 Z.) — Deklaration der Hilfe neben `drawLine()` `:45–50`.<br>`src/ui/librarywindow.cpp` (1143 Z.) — **eine Stelle**: `showNote(index, previous)` `:732 ff.`, der bereits an `currentChanged` hängt (`:316`). Dort gehört das Neuzeichnen der oberen Nachbarn hin (Falle F2). Der Listenaufbau `:202–207` bleibt unberührt.<br>`tests/librarytest.cpp` (3943 Z.) — neue Prüffunktionen für P1–P3 (**neuer Prüfweg, siehe Feld 4**), `keepsTheMeasuresOfTheGroupedList()` `:2648` als Ort für AK 5, `groupsTheSearchResultsLikeTheLibrary()` (Deklaration `:214`) als Ort für AK 6.<br>`tests/libraryshots.cpp` (521 Z.) — das Bild des Normalfalls für AK 7.<br>`tests/searchshots.cpp` (123 Z.) — die Szene `2-trefferliste-mit-gruppen.png` `:108` als Bild zu AK 6. |
| **Build** | **Nur wenn `KColorScheme::frameContrast()` benutzt wird**: `KF6::ColorScheme` an `denkzettelui` (`src/CMakeLists.txt:81–91`, heute ConfigCore `:84`, ConfigGui `:87`, I18n `:88`, WidgetsAddons `:89`, WindowSystem `:90`) und `ColorScheme` in die Komponentenliste (`CMakeLists.txt:24–34`). **Dazu die Versionsgrenze:** `frameContrast()` ist `\since 6.20` (`/usr/include/KF6/KColorScheme/kcolorscheme.h:420`), `KF_MIN_VERSION` steht auf `6.0.0` (`CMakeLists.txt:6`). Der zweite Weg — den Wert selbst mit `KConfigGroup` lesen — kostet **gar nichts am Build** (`KF6::ConfigCore` ist verlinkt), verlangt aber, dass unser Code Gruppe und Schlüssel kennt (M4). |
| **Belege und Prüfmittel** | `docs/scrum/reviews/sprint-NN-s101-listentrenner/` mit eigenem `pruefen.sh` nach dem Muster von `sprint-07-s83-native-huelle/`. **Fertig wiederverwendbar in diesem Ordner:** `sonden/kanten.cpp` (Zeilenkanten, Nachbarauswahl, Neuzeichnen, Breite, Skalierung), `sonden/farbe.cpp` (Linienfarbe über 18 Schemata), `sonden/kontrastwert.cpp` (Herkunft von `frameContrast`). Die Bildpunkt-Technik liegt fertig in `tests/capturetest.cpp:382–445` (`shot()`, `cornerRun()`, `edgeWalk()`). Bildläufer **vor** jedem Bildbeleg bauen: `cmake --build <bau> --target libraryshots searchshots`. |
| **Fachliche Quellen** | **Zeichnung 3a** in `wireframes/Denkzettel Wireframes.dc.html` — am 06.08.2026 nachgezogen (`36156fd`), Maße und Prüfsätze P1–P5 stehen dort. **Wird von dieser Story nicht mehr angefasst.**<br>**SPEC 9** (`:674 ff.`) — beschreibt die Gliederung der Liste und sagt über Trennung heute **nichts**; ob die Linienregel dorthin gehört, ist eine PO-Frage (Feld 6).<br>**SPEC 15** (`:933–958`) — die Abhängigkeitsliste. Wird `KF6::ColorScheme` verlinkt, ist das eine entdeckte Bedingung nach DoD 4/B9 und gehört dort hinein, samt der Versionsgrenze 6.20. |
| **Ausdrücklich nicht** | `src/capture/`, `src/shell/`, `src/store/`, `src/ui/notelistmodel.cpp/.h` (**gemessen: keine Modelländerung nötig** — die Art der Nachbarzeile liefert `GroupHeaderRole` schon heute, F1), `src/ui/elidedlines.*`, `src/ui/pendingdeletion.*`, `src/ui/timestampformat.*`, `tests/capturetest.cpp`, `tests/captureshots.cpp`, `tests/editshots.cpp`, `tests/readmeshots.cpp`, `wireframes/`, `CLAUDE.md`, `docs/scrum/PROZESS.md`, `.claude/agents/*`. |

### 1.1 Kollisionsfläche

**Gegen die drei anderen Sprint-9-Kandidaten null.** #100, #97 und #84 arbeiten
sämtlich in `src/capture/capturewindow.cpp` und `tests/capturetest.cpp`; #101
fasst weder das eine noch das andere an. Gemeinsam berührt werden nur drei
Dateien, und dort nicht dieselben Zeilen:

| Datei | #101 | die drei Capture-Stories |
|---|---|---|
| `CMakeLists.txt` (Wurzel) | Komponentenliste `:24–34`, ggf. `KF_MIN_VERSION` `:6` | keine gemessene Berührung |
| `src/CMakeLists.txt` | `denkzettelui` `:81–91` | `denkzettelcapture` `:52–68` |
| `SPEC.md` | Abschnitt 15, ggf. 9 | Abschnitte 3.1 / 3.2 |

Der kleinste Abstand ist damit **eine Zeilengruppe in derselben Datei**, nicht
dieselbe Zeile. Wer #101 neben einer Capture-Story zieht, braucht die
Worktree-Trennung nach B13 und getrennte Belegordner — mehr nicht.

---

## Feld 2 — Gemessene Fallen

Diese Zeilen gehören in den Spawn-Auftrag des Entwicklers. Was ungemessen ist,
steht als ungemessen da.

### F1 — Die Auswahl der Nachbarzeile ist erreichbar, und sie kostet wenig (M1, §1)

Die Frage, die der PO ausdrücklich vorgelegt hat. Gemessen am **gebauten**
`LibraryWindow` mit dem echten `NoteListModel`:

```
option.widget gesetzt               : ja (QListView)
cast auf QAbstractItemView          : ja
selectionModel() erreichbar         : ja
Auswahl der Nachbarzeile beantwortet: ja
QObject-Elter ist die Ansicht       : ja
```

`option.widget` trägt die Ansicht, der `qobject_cast` auf
`const QAbstractItemView *` gelingt, und
`view->selectionModel()->isSelected(index.sibling(row + 1, 0))` antwortet.
**Preis: ein Cast, eine `nullptr`-Wache, kein neues Feld, keine Änderung am
Modell und keine an `LibraryWindow`.** Der Delegate liest schon heute Rollen
aus dem Modell (`GroupHeaderRole`, `notelistdelegate.cpp:50`), die Art der
Nachbarzeile ist also ohne jede neue Kopplung zu haben; hinzu kommt allein die
Auswahl.

Als zweiter, unabhängiger Weg steht die Ansicht bereits als `QObject`-Elter
zur Verfügung (`librarywindow.cpp:203`: `new NoteListDelegate(m_list)`).
`option.widget` ist der dokumentierte Weg und der, den ich empfehle.

**Mein Urteil: verhältnismäßig.** Der im Issue benannte Rückfall — die Linie
unter der Auswahl stehenlassen und AK 3 kürzen — ist nicht nötig. Die teure
Zeile liegt woanders, nämlich hier:

### F2 — Die Ansicht zeichnet den oberen Nachbarn beim Auswahlwechsel NICHT neu (M1, §3)

Das ist der Fund dieser Vorprüfung. Die Ansicht zeichnet beim Wechsel der
Auswahl die Strecke zwischen alter und neuer Auswahl neu — der obere Nachbar
liegt außerhalb davon und bleibt stehen:

```
Sprung 4 → 5: neu gezeichnet [4, 5]
   oberer Nachbar der neuen Auswahl (Zeile 4, innerhalb der Strecke): gezeichnet
   oberer Nachbar der alten Auswahl (Zeile 3, außerhalb der Strecke): NICHT gezeichnet
Sprung 5 → 4: neu gezeichnet [4, 5]
   oberer Nachbar der neuen Auswahl (Zeile 3, außerhalb der Strecke): NICHT gezeichnet
   oberer Nachbar der alten Auswahl (Zeile 4, innerhalb der Strecke): gezeichnet
```

Zeile 3, 4 und 5 sind in der Messreihe drei Notizen derselben Gruppe. Beide
Richtungen sind betroffen, und beide erzeugen genau den falschen Zustand:

- **abwärts (4 → 5):** Zeile 3 verliert ihre Auswahl-Nachbarschaft, ihre Linie
  müsste **wiederkommen** — sie bleibt weg.
- **aufwärts (5 → 4):** Zeile 3 bekommt einen ausgewählten Nachbarn, ihre Linie
  müsste **verschwinden** — sie bleibt stehen.

Ein Bau, der AK 3 wörtlich umsetzt und sonst nichts tut, ist damit **beim
Tastendurchlauf falsch**, obwohl er in jedem Standbild richtig aussieht. Das
Heilmittel ist eine Zeile in `showNote()` (`librarywindow.cpp:732`), die die
oberen Nachbarn von `index` und `previous` zum Neuzeichnen anmeldet; der Slot
bekommt beide Indizes bereits.

**Gegenprobe zur Belegform, ebenfalls gemessen:**

```
nach dem Sprung 4 → 5 waren 2 Zeilen neu gezeichnet,
nach einem zusätzlichen window.grab() sind es 8 von 8.
→ Ein gegriffenes Standbild VERDECKT die stehengebliebene Linie.
```

`window.grab()` zeichnet die ganze Liste neu. **Ein Bildbeleg kann diesen
Fehler nicht zeigen** — er ist ausschließlich am Neuzeichnen nachzuweisen.
Das ist der Prüfweg, den Feld 4 dafür benennt.

### F3 — `frameContrast` steht nicht im Farbschema (M3, M4)

AK 4 und Zeichnung 3a sagen „`frameContrast` des Schemas". Gemessen stimmt das
nicht:

- `KColorScheme::frameContrast(config)` liest **Gruppe `[KDE]`, Schlüssel
  `frameContrast`**. Gemessen an einer Datei, die in `[General]` **0,45** und
  in `[KDE]` **0,55** trägt: Rückgabe **0,5500**.
- **Keine** der 18 installierten Schemadateien trägt den Schlüssel — weder in
  `[General]` noch in `[KDE]`.
- Ohne übergebene Konfiguration liest die Funktion die **Anwendungs­konfiguration**;
  im leeren Sandkasten liefert sie die Voreinstellung **0,2000**, nach einem
  Eintrag `[KDE] frameContrast=0,40` liefert sie **0,4000**.

Praktisch heißt das: Der Wert ist heute unter jedem Schema **0,20**, und die
Kontrastzahlen der Vorlage vom 06.08.2026 (**1,24 : 1** bis **1,93 : 1**,
in M3 nachgemessen als 1,24 : 1 bis 1,92 : 1 gegen die Palettenrolle `Base`)
bleiben gültig. Was nicht gilt, ist die Begründung: Die Zahl kommt aus der
Anwendungskonfiguration, nicht aus der `.colors`-Datei. Die Messung vom
06.08.2026 hat `[General]` gelesen und ist auf denselben Wert gekommen, weil
beide Wege in dieselbe Voreinstellung fallen — ein Zusammentreffen, kein Beleg.

**Zwei Folgen.** Erstens gehört AK 4 im Wortlaut korrigiert (Feld 6).
Zweitens steuert ein Test den Wert **nicht** über `setPalette()`, sondern nur
über einen Eintrag in der Konfiguration seines eigenen Sandkastens.

### F4 — „Volle Breite der Liste" ist nicht die Breite des Listen-Widgets (M1, §4)

```
Fenster 900×600:            Liste 300 · Viewport 300 · Zeile 300 · Rollbalken aus
Fenster 900×220:            Liste 300 · Viewport 279 · Zeile 279 · Rollbalken sichtbar
```

Sobald ein senkrechter Rollbalken steht, sind Listenbreite (300) und
Zeilenbreite (279) verschiedene Zahlen. AK 2 verlangt die Gruppenlinie „bis
x = Breite−1"; gemeint sein kann nur die Breite des **Zeilenrechtecks**. Ein
Bildpunkt-Prüfsatz, der stattdessen im gegriffenen Fensterbild an der rechten
Kante der Liste misst, liest bei gefüllter Liste den Rollbalken.

### F5 — Die Zeilenrechtecke stoßen lückenlos aneinander (M1, §2)

`spacing = 0`, erste Zeile bei y = 0, und zwischen je zwei Zeilen null
Bildpunkte Lücke — Zeile 1 endet bei 98, Zeile 2 (ein Kopf) beginnt bei 99.
Zweierlei folgt daraus:

- „Letzte Bildpunktzeile der oberen Notiz" (AK 1) und „oberste Bildpunktzeile
  des Kopfes" (AK 2) sind **benachbarte** Bildpunktzeilen. Ein Bau, der die
  Eintragslinie unter *jede* Notiz zeichnet, erzeugt an jeder Gruppengrenze
  eine Doppellinie von 2 Bildpunkten. Genau das fängt P3.
- Die Höhen sind gemessen: erster Kopf 27, jeder weitere 35, jede Notiz 72 —
  die Zahlen, gegen die AK 5 unverändert bleiben muss.

### F6 — Bildpunkt-Prüfsätze gehören auf Skalierung 1, nicht auf 1,6 (M2)

Unter `QT_SCALE_FACTOR=1.6` bleiben alle Kanten in logischen Punkten gleich
(Kopf 27/35, Notiz 72, y-Werte identisch), das gegriffene Bild misst aber
1440 × 960 Bildpunkte bei `devicePixelRatio` **1,60**. Eine Linie von einem
logischen Punkt belegt dort **1,6 Bildpunktzeilen**. P1 bis P3 zählen
Bildpunktzeilen und laufen deshalb auf 1; AK 7 ist ein **Ansichtsbeleg** bei
1,6 und kein Bildpunktbeleg. Beides gehört getrennt.

### F7 — Der Kopf malt keinen Grund, die Notiz schon (am Code gemessen)

Der Kopfzweig kehrt vor `style->drawControl(...)` zurück
(`notelistdelegate.cpp:109–123`), die Gruppenlinie steht also auf blankem
Listengrund. Die Eintragslinie dagegen liegt **innerhalb** des Rechtecks, das
`drawControl(QStyle::CE_ItemViewItem, ...)` `:126` füllt — sie gehört nach
diesem Aufruf gezeichnet, sonst überdeckt der Stil sie.

### F8 — Die Trefferliste der Suche kostet keinen Code (am Code gemessen)

Die zweite Frage des PO. `git grep ProxyModel -- src/ tests/` ist **leer**: es
gibt keinen Proxy. Die Suche geht über `m_store->search(...)` in
`NoteListModel::setNotes(...)` (`librarywindow.cpp:641`), also durch dasselbe
Modell, dieselbe `buildRows()`-Gruppierung (`notelistmodel.cpp:13–33`) und
denselben Delegate. **AK 6 fällt mit an** — die Linienregeln greifen in der
gefilterten Liste an derselben Stelle. Zu tun bleiben ein Test (Anker:
`groupsTheSearchResultsLikeTheLibrary()`) und ein Bild (Anker: Szene
`2-trefferliste-mit-gruppen.png` in `searchshots.cpp:108`) — kein eigener
Aufwand am Produktivcode.

### F9 — Die Bildläufer sind vor jedem Bildbeleg zu bauen

Keine eigene Messung, sondern die Projektregel (CLAUDE.md, Prüfhaltung): Ein
veralteter Läufer schreibt ein plausibles Bild eines alten Standes mit
frischem Zeitstempel. Betroffen sind hier `libraryshots` und `searchshots`.

---

## Feld 3 — AK-Urteil: Vorschlag **ready = nein**, mit zwei kleinen Korrekturen

Das verbindliche Urteil fällt der Scrum Master. Meine Einschätzung:

**Sechs der sieben Kriterien tragen.** AK 1, AK 2, AK 5, AK 6 und AK 7 sind
vollständig, einzeln prüfbar und haben in Feld 4 ein Prüfmittel. Die Story ist
in der Sache entschieden, die Zeichnung ist nachgezogen, es gibt keine offenen
Gestaltungspunkte. Zwei Stellen halten sie trotzdem auf, und beide sind in
Minuten zu beheben:

1. **AK 4 nennt eine falsche Quelle** (F3). „Die Linienfarbe entsteht als
   Mischung … im Verhältnis `frameContrast` des Schemas" ist gemessen unrichtig:
   Der Wert steht in der Anwendungskonfiguration, Gruppe `[KDE]`, Voreinstellung
   0,20; keine der 18 Schemadateien trägt ihn. Wer das Kriterium wörtlich prüft,
   sucht an der falschen Stelle. **Ein Dateiname ist erst dann ein Prüfmittel,
   wenn `git ls-files` ihn zeigt** — dieselbe Bauart gilt für einen
   Konfigurationsschlüssel.
2. **AK 3 sagt nichts über den Zustand nach einem Auswahlwechsel** (F2). Es
   beschreibt einen Standzustand, und der gemessene Fehler tritt genau dort auf,
   wo es schweigt: Nach einem Tastenschritt bleibt beim oberen Nachbarn eine
   Linie stehen beziehungsweise weg. Das Kriterium braucht einen Satz dazu,
   sonst ist der einzige echte Baufehler dieser Story **abnahmefähig**.

Ein dritter Punkt ist **kein** Hindernis, sondern nur eine Klarstellung wert:
AK 3 sagt „an beiden Kanten der ausgewählten Zeile" und lässt offen, ob damit
auch die Gruppenlinie über dem *nächsten* Kopf entfällt, wenn die letzte Notiz
der Gruppe darüber ausgewählt ist. **Die Zeichnung beantwortet es.** In 3a
trägt der Kopf „Gestern" seine Linie (`border-top:1px solid #999`), obwohl die
Notiz unmittelbar darüber die ausgewählte ist (`background:#eef4fc`). Die
Ausnahme betrifft also allein die Eintragslinie. Ein halber Satz im Kriterium
erspart dem Entwickler diesen Weg.

---

## Feld 4 — Prüfmittel, und was ein Agent nicht prüfen kann

| AK | Prüfmittel | Anmerkung |
|---|---|---|
| **AK 1** (P1) | Neuer Bildpunkt-Prüfsatz in `librarytest.cpp`: Fenster greifen, in der letzten Bildpunktzeile des Zeilenrechtecks einer Notiz die Farbe an x = 0…11, x = 12…Breite−13 und x = Breite−12…Breite−1 lesen. Technik fertig in `capturetest.cpp:382–445`. Offscreen, zwei Fenstergrößen, dpr **1** (F6). | **Neuer Prüfweg**: `librarytest.cpp` zählt heute keine Bildpunkte. Die Breite ist die des **Zeilenrechtecks** (F4). |
| **AK 2** (P2) | Derselbe Prüfsatz auf der obersten Bildpunktzeile jedes Kopfes ab dem zweiten, einschließlich x = 0 und x = Breite−1; dazu der erste Kopf als Gegenprobe. | wie AK 1 |
| **AK 3** (P3), Standteil | Drei Gegenproben im selben Prüfsatz: unter der letzten Notiz einer Gruppe, unter jedem Kopf, an beiden Kanten der ausgewählten Zeile. Szene mit **drei** Gruppen, die mittlere mit drei Notizen (die Sonde `kanten.cpp` baut sie fertig auf). | Der einzige Satz, den ein „Linie an jede Zeilenkante"-Bau reißt. |
| **AK 3**, Wechselteil (F2) | **Kein Bildbeleg möglich** — `window.grab()` zeichnet alles neu und verdeckt den Fehler (gemessen). Nachweis nur am Neuzeichnen: entweder ein `QPaintEvent`-Filter auf `list->viewport()`, der die angemeldete Region gegen die Nachbarrechtecke hält, oder ein zählender Delegate im Test nach dem Muster von `sonden/kanten.cpp`. | **Zweiter neuer Prüfweg.** Fehlt er, ist der einzige echte Baufehler dieser Story unsichtbar. |
| **AK 4** (P4) | Farbe der Linie aus dem gegriffenen Bild lesen und gegen die gerechnete Mischung halten; zusätzlich gegen **alle** Palettenrollen prüfen, damit „keine Palettenrolle" belegt ist. Zwei Schemata über `KColorScheme::createApplicationPalette()`, `frameContrast` über einen Eintrag in der Konfiguration des Sandkastens (F3) — **nicht** über `setPalette()`. | Sonde `farbe.cpp` liefert die Sollwerte je Schema. |
| **AK 5** (P5) | Die bestehenden Geometrie-Prüfsätze bleiben unverändert grün, allen voran `keepsTheMeasuresOfTheGroupedList()` (`librarytest.cpp:2648`, Daten `:2637`) und die #70-Sätze `bringsTheHeadAlongWhenTheSelectionReachesTheFirstNoteOfItsGroup()` und die übrigen `bringsTheHead…`-Funktionen. Sollzahlen aus M1: Kopf 27 / 35, Notiz 72. | Kein neuer Prüfweg. |
| **AK 6** | Bestehender Test `groupsTheSearchResultsLikeTheLibrary()` um die Linien-Gegenproben erweitern (kein Code am Produkt, F8); Bild aus `searchshots`, Szene 2. | |
| **AK 7** | `libraryshots` mit `QT_SCALE_FACTOR=1.6`, `QT_QPA_PLATFORMTHEME=kde`, Läufer vorher gebaut; Ablage unter `docs/scrum/reviews/`. | **Ansichtsbeleg, kein Bildpunktbeleg** (F6). B21 verlangt hier kein Sitzungsbild: kein Kriterium spricht über Hülle, Rundung, Kontur, Schatten oder Dekoration. |

**Was ein Agent nicht prüfen kann:**

- **Ob der Kunde die Linie sieht.** 1,24 : 1 im schlechtesten Schema ist
  messbar und trägt keinen Barrierefreiheitsanspruch. Das bleibt ein Urteil des
  Kunden an seinem Bildschirm; die Messung kann es vorbereiten, nicht ersetzen.
- **Ob die Gruppenlinie die Trennung stärkt oder schwächt** (Punkt 4 der fünf
  Schwächen im Issue). Aus einem 14-px-Loch wird ein Kasten mit Oberkante; ob
  das besser gliedert, entscheidet das Auge. Der UI-Review kann es zeigen,
  nicht beweisen.
- **Ein Fensterwechsel unter Wayland** kommt in dieser Story nicht vor; die
  bekannte Grenze (kein Agent holt sich den Fokus zurück) ist hier ohne Belang.

---

## Feld 5 — Größenklasse: **`size:m`**

Begründet gegen die Tabelle in `PROZESS.md`:

- **Gegen `size:s`** („wenige Dateien, kein neuer Prüfweg"): Die Dateien sind
  wenige — im Kern eine. Aber die Story bringt **zwei** neue Prüfwege mit, die
  es in `librarytest.cpp` bis heute nicht gibt: das Zählen von Bildpunkten in
  der Bibliotheksliste und den Nachweis über das Neuzeichnen (F2, gemessen als
  nicht bildbelegbar). Beides muss erst gebaut werden.
- **Gegen `size:l`** („füllt den Sprint"): Der Eingriff am Produktivcode ist
  klein und liegt in einer 168-Zeilen-Datei plus einer Zeile in `showNote()`.
  Kein Maß ändert sich (F5), kein Modell, kein Proxy, und AK 6 fällt ohne
  eigenen Code mit an (F8). Die Auswahl-Kopplung, die im Issue als teuerste
  Zeile geführt wird, ist gemessen billig (F1).
- **Der Rest ist Prüfarbeit**, und die trägt genau einen Strang aus.

Die Build-Abhängigkeit `KF6::ColorScheme` samt Versionsgrenze verschiebt die
Klasse nicht; sie ist eine Zeile in zwei Dateien und ein Absatz in SPEC 15.

---

## Feld 6 — Offene Fragen

### An den PO

1. **AK 4 im Wortlaut korrigieren** (F3). Vorschlag: „… im Verhältnis
   `frameContrast`, wie `KColorScheme::frameContrast()` es liefert — der Wert
   steht in der Anwendungskonfiguration (Gruppe `[KDE]`), nicht in der
   Schemadatei; Voreinstellung 0,20, und keines der 18 installierten Schemata
   überschreibt sie. Die genannten Kontraste 1,93 : 1 und 1,24 : 1 entstehen
   aus den verschiedenen Grund- und Textfarben der beiden Schemata bei
   gleichem `frameContrast`." Dieselbe Klarstellung gehört in Zeichnung 3a —
   dort steht derselbe Satz. **Das ist eine Änderung an einer fachlichen
   Quelle und keine, die ich ohne Auftrag vornehme** (melden, nicht heilen).
2. **AK 3 um den Auswahlwechsel ergänzen** (F2). Vorschlag als eigener Satz:
   „Auch nach einem Wechsel der Auswahl — mit Pfeiltaste oder Maus, in beiden
   Richtungen — trifft dieses Kriterium zu; insbesondere trägt die Zeile über
   der zuvor ausgewählten wieder ihre Linie." Dazu die Belegform: **nicht am
   Bild, sondern am Neuzeichnen** (gemessen).
3. **AK 3 klarstellen**, dass die Ausnahme nur die Eintragslinie betrifft und
   die Gruppenlinie über dem nächsten Kopf stehenbleibt, auch wenn die letzte
   Notiz der Gruppe darüber ausgewählt ist. Die Zeichnung sagt es bereits; das
   Kriterium sollte es auch sagen.
4. **Welchen Weg zu `frameContrast` soll der Entwickler nehmen?** Zwei stehen
   zur Wahl, beide gemessen:
   - `KColorScheme::frameContrast()` — kurz, fremdgepflegt, aber
     `KF6::ColorScheme` als neue Abhängigkeit und `\since 6.20` gegen ein
     `KF_MIN_VERSION` von 6.0.0. Entweder steigt die Mindestversion für alle
     Komponenten, oder es braucht ein eigenes `find_package(KF6ColorScheme 6.20)`.
   - `KConfigGroup(KSharedConfig::openConfig(), "KDE").readEntry("frameContrast", 0.20)`
     — kein neuer Build-Eintrag (ConfigCore ist verlinkt), aber unser Code trägt
     dann Gruppe, Schlüssel und Voreinstellung einer fremden Bibliothek.
     **Meine Empfehlung: der erste Weg**, weil die zweite Fassung genau die Art
     Wissen dupliziert, die still veraltet. Die Entscheidung über eine
     Abhängigkeit und eine Mindestversion gehört aber nicht mir.
5. **Gehört die Linienregel in SPEC 9?** Der Abschnitt beschreibt die
   Gliederung der Liste und schweigt heute über die Trennung; die Festlegung
   steht allein in Zeichnung 3a. SPEC 15 ist ohnehin fällig, sobald
   `KF6::ColorScheme` dazukommt (DoD 4/B9).

### Was ich außerhalb meiner Fläche gefunden habe

Der Griff nach anderen Aussagen über die gemessene Eigenschaft, in der Form aus
CLAUDE.md (`git grep -n frameContrast -- . ':!docs/scrum/reviews' ':!docs/scrum/sprints'
':!docs/scrum/retro' ':!docs/scrum/vorberichte' ':!src' ':!tests'`), liefert vier
Zeilen: **`SPEC.md:278`** und drei Stellen in `wireframes/Denkzettel Wireframes.dc.html`
(`:553`, `:565`, `:813`).

- `SPEC.md:278` und `wireframes …:813` gehören zu Abschnitt 3.1 und
  Zeichnung 4b, also zum **Erfassungsfenster**, nicht zu dieser Story. Beide
  nennen `frameContrast` ohne Herkunftsangabe und tragen dieselben Zahlen
  (1,24 : 1 bis 1,93 : 1 beziehungsweise 1,91 : 1). Sie werden durch M4 nicht
  falsch — die Zahlen stimmen —, aber wer die Herkunft in 3a klarstellt, sollte
  denselben Blick auf diese beiden werfen. **Meldung, keine Heilung**: Ich habe
  dort nichts geändert.
- `wireframes …:553` und `:565` sind die beiden Stellen in Zeichnung 3a, die
  Punkt 1 oben betrifft.

### An den Kunden — nur über den PO

Keine. Die Gestaltungsfrage ist am 06.08.2026 entschieden; alles Offene ist
Formulierung und Bauweg.
