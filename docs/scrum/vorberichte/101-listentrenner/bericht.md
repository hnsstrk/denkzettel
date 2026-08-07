# Vorprüfbericht #101 — Trenner in der Bibliotheksliste

**Story:** #101 „Bibliothek: Notizen und Gruppen heben sich optisch nicht
voneinander ab" (`epic:M2`, `typ:story`) ·
**Bearbeiter A:** `denkzettel-ux`, `messung-a.md` ·
**Bearbeiter B:** `scrum-master`, `messung-b.md` ·
**Konsolidiert:** 07.08.2026, 21:04 CEST, Ganymed · **Boden:** `main` bei `88129ba`.

**Maßgeblich sind die korrigierten Akzeptanzkriterien** aus dem PO-Kommentar
vom 07.08.2026 („Korrektur der Akzeptanzkriterien, nach der Vorprüfung"):
AK 1, AK 2, **AK 3a/3b/3c**, AK 4, AK 5, AK 6, AK 7 und **AK 10**. Die
Erstfassung von AK 3 und AK 4 ist damit abgelöst; beide Messungen sind gegen
die Erstfassung gelaufen, und beide hatten daran ihr „nein" gehängt.

**AK 8 und AK 9 gibt es nicht.** Die Nummer springt von 7 auf 10; wer sie
sucht, sucht vergeblich.

**Verfahren:** Beide Bearbeiter haben unabhängig gemessen und einander nicht
gelesen, jeder mit eigenen Sonden und eigenem Bauplatz. Nach Aufhebung der
Sperre hat B die Befunde aus A nachgeprüft, die er selbst nicht gemessen hatte
(Abschnitt 0); dabei ist eine Angabe aus B gefallen.

**Belege:** `messungen/m1`–`m4` (A), `messungen/mb1`–`mb6` (B).
Wiederholbar: `bash pruefen.sh` (A) und `bash bauen-b.sh` (B). Beide bauen
ausschließlich in eigenen Verzeichnissen, fassen `build/` im
Projektwurzelverzeichnis nicht an und installieren nichts nach `/usr`.

---

## 0. Wo die beiden Messungen auseinandergingen

Drei Stellen, alle aufgelöst. Aufgelöst heißt hier: nachgemessen, nicht
verglichen.

### 0.1 Die Breite: 279 gegen 286 — beide richtig, verschiedene Aufbauten

A misst am gebauten `LibraryWindow` ein Zeilenrechteck von **279** px bei einer
Liste von 300, B an einer nackten `QListView` **286** von 300.

Nachgemessen mit **einer** Sonde unter beiden Bedingungen
(`mb5`/`mb6`, Prüffrage 3):

| Lauf | Stil | Rollbalken | Ansicht | Sichtfeld | Zeilenrechteck |
|---|---|---|---|---|---|
| mit `QT_QPA_PLATFORMTHEME=kde` | `breeze` | 21 px | 300 | 279 | **279** |
| ohne | `fusion` | 14 px | 300 | 286 | **286** |

Der Unterschied ist die Rollbalkenbreite des Stils. **Maßgeblich ist 279**,
weil dieses Projekt jeden Bildlauf unter `QT_QPA_PLATFORMTHEME=kde` fährt
(`CLAUDE.md`, Prüfhaltung). Keine der beiden Zahlen gehört in ein Kriterium —
der PO hat AK 2 bereits auf die Größe statt auf eine Zahl gestellt, und das
ist die richtige Fassung.

### 0.2 Die Kopplung ans `selectionModel`: kein Widerspruch

Beide Messungen kommen zum selben Ergebnis, unabhängig und mit verschiedenen
Aufbauten: `option.widget` trägt die Ansicht in jedem Malvorgang, der
`qobject_cast` gelingt, `selectionModel()->isSelected()` antwortet. A misst am
gebauten `LibraryWindow` mit dem echten Modell, B an einem Nachbau. **Preis:
ein Cast und eine `nullptr`-Wache.**

Der Anschein einer Abweichung stammt aus B, Feld 5: Dort steht, ein Streichen
der Auswahl-Ausnahme ließe die Kopplung entfallen. Das ist eine Aussage über
den Umfang, keine über den Preis. Die Frage des Issues — „ist die Kopplung
unverhältnismäßig?" — beantworten beide mit **nein**.

### 0.3 `frameContrast`: A hat recht, B hatte unrecht

B schrieb, der Wert stehe in `[General]` der Anwendungskonfiguration. **Das ist
falsch.** Nachgemessen (`mb5`, Prüffrage 1):

```
ohne Konfiguration (leerer Sandkasten)      : 0.2000
Datei mit [General]=0.45 und [KDE]=0.55     : 0.5500
Datei nur mit [General]=0.45                : 0.2000
```

`KColorScheme::frameContrast()` liest **`[KDE]`**. In der Konfiguration des
Kunden steht der Schlüssel ebenfalls dort (`awk`-Griff über die
Gruppenüberschriften: `Gruppe [KDE] → frameContrast=0.2`). Der Griff in B hatte
die dazwischenliegende Gruppenüberschrift nicht angezeigt. Der Fehler ist in
`messung-b.md` als datierter Nachtrag verankert, der Text dort steht
unverändert (B17).

**Was aus B trotzdem trägt**, weil es am Mechanismus hängt und nicht an der
Gruppe: der Testmodus, die Voreinstellung 0,20 und die Sekunden-Trägheit —
alle drei auf dem entschiedenen Weg (`KColorScheme::frameContrast()`) noch
einmal nachgemessen (`mb5`, Prüffrage 2).

### 0.4 Vier Befunde aus A, die B nicht gemessen hatte — alle bestätigt

| Befund aus A | Nachprüfung durch B |
|---|---|
| F5: `spacing() == 0`, die Zeilenrechtecke stoßen lückenlos aneinander | bestätigt (`mb5`, Prüffrage 4): Lücke 0 zwischen je zwei Zeilen |
| F5: Höhen erster Kopf 27, weiterer Kopf 35, Notiz 72 | bestätigt (`mb5`, Prüffrage 5), unabhängig nachgerechnet aus den Schriftmaßen unter `QT_QPA_PLATFORMTHEME=kde`: Kopfschrift 15 px, Fließtext 18 px → 6+15+6 = 27, 14+15+6 = 35, 18+15+3+2×18 = 72 |
| F8: kein Proxy-Modell, die Suche läuft durch `setNotes()` | bestätigt: `git grep ProxyModel -- src/ tests/` leer, `librarywindow.cpp:641` in `reload()` |
| F2, Heilort: `showNote()` bekommt `previous` bereits | bestätigt: `librarywindow.h:127`, `librarywindow.cpp:732`, angebunden bei `:316` |

### 0.5 Der eine Befund, den beide unabhängig gemacht haben

Die Ansicht zeichnet beim Auswahlwechsel nur die Strecke zwischen alter und
neuer Auswahl neu; der obere Nachbar bleibt stehen. A misst am gebauten
Fenster zwei Wechsel, B am Nachbau vier — **sechs Wechsel, sechsmal derselbe
Befund**. Beide messen zusätzlich, dass `grab()` alles neu zeichnet und ein
Bildbeleg den Fehler deshalb nicht zeigen kann. Dieser Befund hat AK 3
verändert.

---

## 1. Feld 1 — Dateimenge

Notation nach B13, am Code vermessen (Stand `88129ba`). Grundlage ist die
Tabelle aus Messung A; ergänzt um den Build-Teil, den die Kundenentscheidung
zu `KF6::ColorScheme` verbindlich gemacht hat, und um AK 10.

| | **#101 — ein Strang** |
|---|---|
| **Issue / Zweig** | #101 (`epic:M2`, `typ:story`, `size:m`) · `story/101-listentrenner` |
| **Quellen und Tests** | `src/ui/notelistdelegate.cpp` (168 Z.) — **die Hauptarbeit, drei Stellen**: Konstanten `:12–28` (**kein neues Maß**, beide Linien liegen in `VerticalPadding = 9` und `HeadTopPadding = 14`), Kopfzweig `:109–123` (Gruppenlinie in der obersten Bildpunktzeile, **vor** dem `return`), Notizzweig `:125–151` (Eintragslinie in der letzten Bildpunktzeile, **nach** `style->drawControl(...)` `:126`, sonst überdeckt der Stil sie). Eine Zeichenhilfe nach dem Muster von `drawLine()` `:62–77`.<br>`src/ui/notelistdelegate.h` (51 Z.) — Deklaration neben `drawLine()` `:45–50`.<br>`src/ui/librarywindow.cpp` (1143 Z.) — **eine Stelle**: `showNote(index, previous)` `:732 ff.`; dort das Neuzeichnen der oberen Nachbarn (AK 3c, Falle 2.2). Listenaufbau `:202–207` unberührt.<br>`tests/librarytest.cpp` (3943 Z.) — neue Prüffunktionen für AK 1, AK 2, AK 3a/3b (Bildpunkte) und AK 3c (Malzähler); `keepsTheMeasuresOfTheGroupedList()` `:2648` als Ort für AK 5, `groupsTheSearchResultsLikeTheLibrary()` als Ort für AK 6.<br>`tests/libraryshots.cpp` (521 Z.) — das Bild des Normalfalls für AK 7. |
| **Build** | **Gültige Fassung (Kundenentscheidung 07.08.2026, 21:17 — siehe Nachtrag 8.1):** `KF6::ColorScheme` an `denkzettelui` (`src/CMakeLists.txt:81–91`); dazu **ein eigener `find_package`-Aufruf für `ColorScheme` mit der Mindestversion 6.20**. **`KF_MIN_VERSION` `:6` bleibt bei `6.0.0`**, und die Komponentenliste `:24–34` bleibt unberührt — der Bauplan trägt eine zweite Versionszahl, und die Untergrenze steigt allein dort, wo die Funktion sitzt.<br>*Überholte Fassung, wie sie hier bis 21:35 stand:* „`ColorScheme` in die Komponentenliste (`CMakeLists.txt:24–34`), `KF_MIN_VERSION` von `6.0.0` auf `6.20` (`CMakeLists.txt:6`)." Sie beruhte auf dem Stand vor dem Nachtrag zu AK 4. **Falle 2.6 ist der Grund, aus dem anders entschieden wurde**, und bleibt gültig. |
| **Belege und Prüfmittel** | `docs/scrum/reviews/sprint-NN-s101-listentrenner/` mit eigenem `pruefen.sh` nach dem Muster von `sprint-07-s83-native-huelle/`. **Fertig wiederverwendbar:** `sonden/kanten.cpp`, `sonden/farbe.cpp`, `sonden/kontrastwert.cpp` (A) sowie `sonden/nachbarmalung.cpp` und `sonden/nachpruefung.cpp` (B). Die Bildpunkt-Technik liegt fertig in `tests/capturetest.cpp:382–445`. **Bildläufer vor jedem Bildbeleg bauen** (F9): `cmake --build <bau> --target libraryshots searchshots`. Für AK 6 genügt ein Lauf — die Szene `2-trefferliste-mit-gruppen.png` steht bereits (`tests/searchshots.cpp:108`) und braucht keine Codeänderung. |
| **Fachliche Quellen** | **SPEC 9** (`:674 ff.`) — trägt die Linienregel nach AK 10; schweigt heute über die Trennung.<br>**SPEC 15** (`:933–958`) — nimmt `KColorScheme` auf **und zum ersten Mal eine Versionsgrenze**: Die Abhängigkeitsliste nennt heute überhaupt keine Mindestversion (gemessen: kein Treffer für `KF_MIN`, `Mindestversion`, `6.0.0` in `SPEC.md`). Entdeckte Bedingung nach DoD 4/B9.<br>**Zeichnung 3a** — Maße und Prüfsätze P1–P5, am 06.08.2026 nachgezogen (`36156fd`). **Vom Strang nicht anzufassen**, aber vor dem Spawn vom PO zu berichtigen (Feld 6.1). |
| **Ausdrücklich nicht** | `src/capture/`, `src/shell/`, `src/store/`, `src/ui/notelistmodel.{h,cpp}` (**gemessen: keine Modelländerung** — die Art der Nachbarzeile liefert `GroupHeaderRole` schon heute), `src/ui/elidedlines.*`, `src/ui/pendingdeletion.*`, `src/ui/timestampformat.*`, `tests/capturetest.cpp`, `tests/captureshots.cpp`, `tests/editshots.cpp`, `tests/readmeshots.cpp`, `tests/searchshots.cpp` (**wird gelaufen, nicht geändert**), `wireframes/`, `CLAUDE.md`, `docs/scrum/PROZESS.md`, `.claude/agents/*`. |

### 1.1 Kollisionsfläche gegen #100

Der Kunde hat Sprint 9 als **#100 + #101** freigegeben. Gemessen (A, 1.1)
arbeitet #100 in `src/capture/capturewindow.cpp` und `tests/capturetest.cpp` —
Dateien, die #101 nicht anfasst. Gemeinsam berührt werden drei:

| Datei | #101 | #100 |
|---|---|---|
| `CMakeLists.txt` (Wurzel) | **ein neuer `find_package`-Aufruf** für `ColorScheme`; `:6` und `:24–34` unberührt (8.1) | keine Berührung — **beide Bearbeiter von #100 messen „kein Build-Eingriff"** (`docs/scrum/vorberichte/100-eingabefeld/bericht.md:127`, `:151`) |
| `src/CMakeLists.txt` | `denkzettelui` `:81–91` | `denkzettelcapture` `:52–68` — `KF6::Svg` steht dort bereits (`:66`), also ebenfalls keine Änderung |
| `SPEC.md` | Abschnitte 9 und 15 | Abschnitte 3.1 / 3.2 |

Kleinster Abstand: **eine Zeilengruppe in derselben Datei**, nicht dieselbe
Zeile. Worktree-Trennung nach B13 und getrennte Belegordner genügen.

~~**Eine Ausnahme, die der PO takten muss:** Berührt #100 die
`find_package`-Zeilen doch, liegen beide Stränge in `CMakeLists.txt:6–34`.
Dann rebased der zweite Strang, statt rückwärts zu mergen.~~
**Entfallen (07.08.2026, 21:35, siehe 8.2).** Sie hing an zwei Bedingungen,
und beide sind fort: `CMakeLists.txt:6` bleibt unberührt, und der
konsolidierte Vorprüfbericht zu #100 hält für beide Bearbeiter fest, dass die
Story **keinen Build-Eingriff** braucht. Der Bauplan gehört damit in diesem
Sprint allein #101; getaktet werden muss nichts.

---

## 2. Feld 2 — Gemessene Fallen

Diese Zeilen gehören in den Spawn-Auftrag. Ungemessenes steht als ungemessen da.

**2.1 Die Kopplung ans `selectionModel` ist billig** (A F1, B 2.1, doppelt
gemessen). `option.widget` trägt den `QListView` in jedem Malvorgang, kein
einziges Mal `nullptr`; der `qobject_cast` auf `const QAbstractItemView *`
gelingt; `selectionModel()->isSelected(index.sibling(row + 1, 0))` antwortet.
Preis: ein Cast, eine `nullptr`-Wache. Die Art der Nachbarzeile liefert
`GroupHeaderRole` ohne jede neue Kopplung.

**2.2 Die Ansicht zeichnet den oberen Nachbarn nicht neu** (A F2, B 2.2, sechs
Wechsel). Die Malregion ist die Strecke zwischen alter und neuer Auswahl; die
Zeile darüber liegt außerhalb. Aufwärts bleibt eine Linie stehen, die
verschwinden müsste, abwärts fehlt eine, die zurückkommen müsste. Heilort:
`showNote()` (`librarywindow.cpp:732`) meldet die oberen Nachbarn von `index`
und `previous` zum Neuzeichnen an. **`previous` kann ungültig sein** (erster
Aufbau, Auswahl nach einer Löschung) — die Wache dagegen gehört dazu.

**2.3 Ein Bildbeleg kann 2.2 nicht zeigen** (A F2, B 2.3). Nach einem Wechsel
waren 2 von 8 beziehungsweise 5 von 12 Zeilen neu gezeichnet; das unmittelbar
folgende `grab()` zeichnete **alle**. `grab()` ist der Weg aller fünf
Bildläufer. Deshalb steht in AK 3c das Neuzeichnen als Belegform, und deshalb
ist ein Bild dort ausdrücklich kein Nachweis.

**2.4 `frameContrast` kommt aus `[KDE]` der Anwendungskonfiguration** (A F3,
von B nachgemessen). Kein installiertes Schema trägt den Schlüssel; überall
gilt die Voreinstellung 0,20. `setPalette()` erreicht den Wert nicht. Ein Test
steuert ihn über die Konfiguration seines eigenen Sandkastens.

**2.5 Zwei Werte im selben Lauf brauchen eine Sekunde Abstand** (B, `mb2` und
`mb5` Prüffrage 2). Vier beziehungsweise drei Werte hintereinander
geschrieben und mit `reparseConfiguration()` gelesen: **nur der erste kam an.**
Mit 1100 ms Pause kamen alle an; ebenso über eine eigene Konfiguration auf den
Pfad. Die Änderungserkennung von KConfig arbeitet in Sekunden. **AK 4 verlangt
zwei Schemata in einem Lauf** — ein Test ohne diese Vorsichtsmaßnahme rechnet
Erwartung und Messung aus demselben hängengebliebenen Wert und ist grün und
wertlos. Gegenmittel: den gelesenen Wert vor der Messung selbst zusichern.

**2.6 `KF_MIN_VERSION` speist zwei `find_package`-Aufrufe** (B, am Code
gemessen). `CMakeLists.txt:6` geht sowohl in `find_package(ECM …)` `:8` als
auch in `find_package(KF6 …)` `:24`. Ein Sprung auf 6.20 an dieser Stelle höbe
die Untergrenze für ECM und alle zehn Komponenten. Auf Ganymed wäre das
unkritisch (ECM 6.28, kcolorscheme 6.28), im öffentlichen Prüflauf ebenfalls,
weil er aus dem rollenden Arch-Strom installiert — für ein Paket in einer
Distribution mit älterem KF6 nicht.
**Diese Falle hat den Bauplan geändert** (07.08.2026, 21:17): Der PO hat sie
dem Kunden als Berichtigung seiner eigenen Vorlage vorgelegt, und der Kunde hat
daraufhin den engeren Weg gewählt — eigener `find_package`-Aufruf für
`ColorScheme` mit 6.20, `KF_MIN_VERSION` bleibt bei 6.0.0. Die Falle steht
deshalb weiter hier: Sie ist der Grund der Entscheidung, und wer sie streicht,
nimmt dem Bauplan seine Begründung.

**2.7 Die Zeilenrechtecke stoßen lückenlos aneinander** (A F5, von B
bestätigt). `spacing() == 0`, Lücke 0. „Letzte Bildpunktzeile der oberen
Notiz" und „oberste Bildpunktzeile des Kopfes" sind **benachbarte** Zeilen: Ein
Bau nach dem Muster „Linie unter jeder Notiz" erzeugt an jeder Gruppengrenze
eine Doppellinie von zwei Bildpunkten. AK 3a fängt das ab.

**2.8 Der Kopf malt keinen Grund, die Notiz schon** (A F7, am Code). Der
Kopfzweig kehrt vor `style->drawControl(...)` zurück — die Gruppenlinie steht
auf blankem Listengrund. Die Eintragslinie liegt **innerhalb** des Rechtecks,
das `drawControl` füllt, und gehört nach diesem Aufruf gezeichnet.

**2.9 Die Breite ist die des Zeilenrechtecks** (A F4, B 2.6, Auflösung in 0.1).
Mit Rollbalken unter `QT_QPA_PLATFORMTHEME=kde`: Ansicht 300, Zeile 279. Ein
Prüfsatz, der im gegriffenen Fensterbild an der rechten Kante der Liste misst,
liest bei gefüllter Liste den Rollbalken.

**2.10 Bildpunkt-Prüfsätze laufen auf Skalierung 1** (A F6, B 2.7). Unter 1,6
bleiben alle Kanten in logischen Punkten gleich, das Bild misst aber
`devicePixelRatio` 1,60: Eine Linie von einem logischen Punkt belegt zwei
Gerätebildpunktzeilen, und die eingerückte Kante liegt bei 19 statt bei 12.
AK 7 ist ein Ansichtsbeleg und trägt keine Bildpunktzusicherung.

**2.11 Die Bildläufer sind vor jedem Bildbeleg zu bauen** (A F9,
Projektregel). Betroffen: `libraryshots`, `searchshots`.

**Was ungemessen ist:** ob die Gruppenlinie die Trennung stärkt oder schwächt
(Auge, UI-Review); ob der Kunde die Linie bei 1,24 : 1 sieht (Abnahme); der
gebaute Stand mit Linien — es gibt ihn noch nicht.

---

## 3. Feld 3 — AK-Urteil: **ready ja**

Das Urteil ergeht gegen die **korrigierte** Fassung der Kriterien und wird vom
Scrum Master gefällt.

**Beide Messungen urteilten „nein", und beide hingen es an denselben zwei
Formulierungen: AK 3 beschrieb einen Zustand, obwohl die Sache ein Übergang
ist, und AK 4 nannte eine Quelle, die es nicht gibt.** Der PO hat beide
korrigiert. Geprüft, ob die Korrekturen tragen:

| Kriterium | Trägt? |
|---|---|
| **AK 3a** | ja — Standzustand, prüfbar am Bildpunkt, drei Fälle einzeln (Feld 4) |
| **AK 3b** | ja — die Zeichnung zeigt es am Kopf „Gestern" (`border-top` bei ausgewählter Notiz darüber); prüfbar am Bildpunkt |
| **AK 3c** | ja — deckt den gemessenen Fehler vollständig ab, **beide** Richtungen, und benennt die Belegform, die als einzige trägt. Der Satz „ein Bildbeleg genügt hier nicht" ist die Zeile, die den Prüfweg vor dem bequemen Ersatz schützt |
| **AK 4** | ja — Herkunft jetzt richtig (`[KDE]`, `KColorScheme::frameContrast()`, Voreinstellung 0,20), Weg entschieden, Prüfmittel in Feld 4. *Nachgeführt am 21:35 (8.3):* Der Nachtrag von 21:17 stellt zusätzlich klar, dass **das Verfahren** zugesichert ist und zwei Schemata geprüft werden, deren **Ergebnis** auseinanderliegt — die beiden Namen bleiben als Herkunft der Zahlen im Kriterium stehen. Als Prüfvorschrift gelesen bänden sie an zwei Pakete, die der öffentliche Prüflauf nicht hat (B, `mb4`); Feld 4 liest sie als Herkunft |
| **AK 2** | ja — „Breite des Zeilenrechtecks (`option.rect`)" ist die Fassung, die beide Messungen stützen; die Zahl steht zu Recht nicht drin (0.1) |
| **AK 7** | ja — Ansichtsbeleg, Skalierung 1,6 vom Kunden bestätigt, Bildpunktsätze auf 1 |
| **AK 10** | ja — prüfbar am Diff von `SPEC.md` (Feld 4) |
| **AK 1, AK 5, AK 6** | unverändert, tragen nach beiden Messungen |

**Die Definition of Ready ist damit erfüllt:** Der Vorprüfbericht liegt, die
Kriterien sind vollständig und einzeln prüfbar, und zu jedem steht in Feld 4
ein Prüfmittel oder die ausgesprochene Grenze. Die drei Zusatzsätze vom
04.08.2026 greifen nicht dagegen — das Issue führt keine selbstdeklarierten
offenen Punkte mehr (die fünf Schwächen waren der Vorprüfung vorgelegt, was
ihr Zweck war), jeder genannte Prüfort existiert und ist mit `git ls-files`
belegt, und kein Kriterium spricht über Hülle, Rundung, Kontur, Schatten,
Dekoration oder Durchsichtigkeit, so dass B21 hier kein Sitzungsbild verlangt.

**Ein Befund bleibt und blockiert nicht** (Feld 6.1): Zeichnung 3a trägt an
zwei Stellen weiterhin die widerlegte Herkunft „`frameContrast` des
Farbschemas", eine davon im Prüfsatz **P4** selbst. Die Kriterien binden, nicht
die Zeichnung, und AK 4 nennt die Quelle jetzt richtig — deshalb kein „nein".
Wer aber nach P4 baut, baut nach einem Satz, der am 07.08.2026 gefallen ist.
Der PO berichtigt die Zeichnung **vor dem Spawn**; das ist Minutenarbeit und
erspart genau den Fehler, den `CLAUDE.md` als „Wer sich auf eine Zeichnung
beruft, liest sie vorher" führt.

---

## 4. Feld 4 — Prüfmittel

| AK | Prüfmittel | Anmerkung |
|---|---|---|
| **AK 1** | Neuer Bildpunkt-Prüfsatz in `librarytest.cpp`: Liste greifen, in der letzten Bildpunktzeile des Zeilenrechtecks einer Notiz die Farbe bei x = 0…11, x = 12…Breite−13 und x = Breite−12…Breite−1 lesen. Technik fertig in `capturetest.cpp:382–445`. Offscreen, **zwei Fenstergrößen**, Skalierung 1 | **Neuer Prüfweg** — `librarytest.cpp` zählt heute keinen Bildpunkt (kein `QImage`, kein `pixelColor`). Breite = Zeilenrechteck (2.9) |
| **AK 2** | Derselbe Satz auf der obersten Bildpunktzeile jedes Kopfes ab dem zweiten, x = 0 und x = Breite−1 eingeschlossen; der erste Kopf als Gegenprobe | wie AK 1 |
| **AK 3a** | Drei Gegenproben im selben Satz: unter der letzten Notiz einer Gruppe, unter jedem Kopf, an beiden Kanten der ausgewählten Zeile. Szene mit drei Gruppen, die mittlere mit drei Notizen — `sonden/kanten.cpp` baut sie fertig auf | Der einzige Satz, den ein „Linie an jede Zeilenkante"-Bau reißt (2.7) |
| **AK 3b** | Bildpunktprobe an der obersten Zeile eines Kopfes, während die Notiz unmittelbar darüber ausgewählt ist: Linie vorhanden | Ein Fall, kein eigener Aufbau |
| **AK 3c** | **Kein Bildbeleg** (2.3). Malzähler im Test: ein `QStyledItemDelegate`, der die Malvorgänge je Zeile zählt, Auswahl wechseln, `QTest::qWait`, zählen — je einmal aufwärts und abwärts, je einmal Nachbarschritt und weiter Sprung. Fertige Aufbauten: `sonden/nachbarmalung.cpp` (B), `sonden/kanten.cpp` (A). Gleichwertig: ein Filter auf `QEvent::Paint` am `viewport()`, der die angemeldete Region gegen die Nachbarrechtecke hält | **Zweiter neuer Prüfweg.** Fehlt er, ist der einzige echte Baufehler dieser Story unsichtbar und abnahmefähig |
| **AK 4** | Linienfarbe aus dem gegriffenen Bild lesen und gegen die selbst gerechnete Mischung halten; zusätzlich gegen **alle** Palettenrollen prüfen, damit „keine Palettenrolle" belegt ist. Zwei Paletten mit weit auseinanderliegenden Grund- und Textfarben, im Test aufgebaut (Muster: `breezePalette()`, `libraryshots.cpp:84–107`); `frameContrast` über einen Eintrag `[KDE] frameContrast=…` in der Sandkasten-Konfiguration. **Vor jeder Messung den gelesenen Wert zusichern**, sonst schlägt 2.5 zu | **Nicht** über zwei installierte Schemadateien: `KritaDarkOrange` gehört zu `krita`, `IridescentLightly` zu `cachyos-iridescent-kde`, keines steht in der Paketliste des öffentlichen Prüflaufs (`mb4`). Sollwerte je Schema liefert `sonden/farbe.cpp` |
| **AK 5** | Die bestehenden Prüfsätze bleiben unverändert grün: `keepsTheMeasuresOfTheGroupedList()` (`librarytest.cpp:2648`) und die #70-Sätze `bringsTheHead…`. Sollzahlen, zweimal unabhängig gemessen: erster Kopf 27, weiterer Kopf 35, Notiz 72. `ctest --test-dir <bau> -R librarytest` | Kein neuer Prüfweg. Rechnerisch unberührt: Beide Linien liegen in vorhandenen Innenabständen (9 unten, 14 oben) |
| **AK 6** | `groupsTheSearchResultsLikeTheLibrary()` um die Linien-Gegenproben erweitern — gemessen kein Eingriff am Produktivcode (A F8, von B bestätigt, 0.4). Bild: `searchshots`, Szene `2-trefferliste-mit-gruppen.png` (`searchshots.cpp:108`), Läufer vorher bauen | Kein neuer Prüfweg |
| **AK 7** | `cmake --build <bau> --target libraryshots`, dann `QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.6 libraryshots <Ordner>`; Ablage unter `docs/scrum/reviews/` | Ansichtsbeleg, kein Bildpunktbeleg (2.10). B21 verlangt kein Sitzungsbild: kein Kriterium spricht über Hülle, Rundung, Kontur, Schatten oder Dekoration |
| **AK 10** | `git diff` auf `SPEC.md`: Abschnitt 9 trägt die Linienregel (beide Linien, ihre Ausdehnungen, die Ausnahme an der Auswahl), Abschnitt 15 trägt `KColorScheme` **und die Mindestversion 6.20 für diese eine Komponente** — nicht als angehobene Untergrenze für das Ganze (8.1). Gegenprobe, dass die Regel nicht doppelt und widersprüchlich steht: der Ausschluss-Griff aus `CLAUDE.md` über `frameContrast` und über `Trennlinie\|Haarlinie` | SPEC 15 nennt heute **keine** Mindestversion (gemessen). Die Versionsgrenze ist eine entdeckte Bedingung nach DoD 4/B9 und gehört ausgesprochen, nicht nur in die `CMakeLists.txt`. Der Prüfsatz ist damit schärfer als vorher: Steht dort eine allgemeine Untergrenze statt einer Komponentenversion, ist AK 10 **nicht** erfüllt |

**Was ein Agent nicht prüfen kann:**

- **Ob der Kunde die Linie sieht.** 1,24 : 1 im schlechtesten Schema ist
  gemessen und trägt keinen Barrierefreiheitsanspruch. Das Urteil fällt an
  seinem Bildschirm, in der Abnahme.
- **Ob die Gruppenlinie die Trennung stärkt oder schwächt.** Aus einem
  14-px-Loch wird ein Kasten mit Oberkante. Der UI-Review kann es zeigen, nicht
  beweisen — der PO hat den Hebel im Issue benannt (Gruppenlinie fallenlassen,
  eingerückte behalten).
- **Den stehengebliebenen Strich am Bildschirm.** Der Malzähler belegt, dass
  die Ansicht die Nachbarzeile anmeldet. Dass der Bildschirm danach richtig
  aussieht, folgt daraus und ist selbst nicht abgegriffen: An den Bildspeicher
  des Compositors kommt kein Agent, und `grab()` zeichnet neu.
- **Ein Fensterwechsel** kommt in dieser Story nicht vor; die bekannte
  Wayland-Grenze ist hier ohne Belang.

---

## 5. Feld 5 — Größenklasse: **`size:m`**

Beide Messungen kommen unabhängig auf `size:m`; die Zusammenführung ändert
daran nichts. Begründet gegen die Tabelle in `PROZESS.md`:

- **Gegen `size:s`** („wenige Dateien, kein neuer Prüfweg"): Die Dateien sind
  wenige — im Kern eine von 168 Zeilen plus eine Stelle in `showNote()`. Aber
  die Story bringt **zwei** Prüfwege mit, die es in `librarytest.cpp` bis heute
  nicht gibt: das Zählen von Bildpunkten in der Bibliotheksliste und den
  Nachweis über das Neuzeichnen. Dazu der Konfigurationsschalter für AK 4.
- **Gegen `size:l`** („füllt den Sprint"): Kein Maß ändert sich, kein Modell,
  kein Proxy; AK 6 fällt ohne eigenen Produktivcode mit an; die im Issue als
  teuerste geführte Zeile ist zweimal unabhängig als billig gemessen. Zum
  Vergleich trägt #83 `size:l` — Fensterhülle, KSvg, Schatten, Weichzeichner,
  19 Kriterien, Sitzungsbilder.
- **Was dazugekommen ist, verschiebt die Klasse nicht:** `KF6::ColorScheme`
  samt Versionsgrenze sind drei Zeilen in zwei Dateien, AK 10 zwei Absätze in
  `SPEC.md`.

**Zur Sprint-Grenze:** Mit `size:m` neben dem zweiten `m` von Sprint 9 hält die
Klassenregel (kein `xl`, höchstens eine `l`, neben einer `l` nur `s`) — zwei
Issues, `2×m`. Die Klasse ist an der Arbeit bemessen und nicht an dieser
Rechnung; sie fällt hier nur zufällig mit ihr zusammen.

**Das Label `size:m` ist im selben Zug mit diesem Bericht am Issue gesetzt**
(04.08.2026, `PROZESS.md`) — vorher trug #101 keines.

---

## 6. Feld 6 — Offene Fragen

### 6.1 An den PO, vor dem Spawn: Zeichnung 3a berichtigen

Zwei Stellen tragen die am 07.08.2026 widerlegte Herkunft:

- `wireframes/Denkzettel Wireframes.dc.html:553` — „Farbe beider Linien: …
  im Verhältnis `frameContrast` des Farbschemas"
- `…:565` — **Prüfsatz P4** — „… im Verhältnis `frameContrast` des Schemas,
  nicht eine Palettenrolle"

Der Wert steht in der Anwendungskonfiguration, Gruppe `[KDE]`; kein
installiertes Schema trägt ihn. Die Kontrastzahlen 1,24 : 1 bis 1,93 : 1
bleiben richtig — sie entstehen aus den Farben der Schemata bei gleichem
Verhältnis. Nachzuziehen mit datiertem Vermerk, nicht durch Überschreiben
(B17), so wie am 06.08.2026 geschehen.

**Das ist die Fundstelle, die in der Korrektur des PO fehlt.** Sie nennt
`SPEC.md:278` und Zeichnung 4b und übergeht die Zeichnung, auf die die
Kriterien sich ausdrücklich berufen. Dieselbe Bauart wie der Sprint-7-Fall in
`CLAUDE.md`: Wer ein zu enges Werkzeug erweitert, erweitert es um das, woran er
sich erinnert.

Der Vollständigkeitsgriff in der Ausschlussform liefert vier Zeilen:
`SPEC.md:278` und `wireframes …:553`, `:565`, `:813`. Davon gehören `:553` und
`:565` zu dieser Story, `SPEC.md:278` und `:813` zum Erfassungsfenster
(Abschnitt 3.1, Zeichnung 4b) — beide tragen richtige Zahlen ohne
Herkunftsangabe und sind vom PO bereits vorgemerkt.

### 6.2 An den PO: ein halber Satz zu AK 3c

`previous` kann ungültig sein — beim ersten Aufbau und nachdem eine Löschung
die Auswahl geräumt hat. AK 3c spricht von „der Zeile über der alten Auswahl",
ohne diesen Fall zu nennen. Der Bau braucht dort ohnehin eine Wache; ob das
Kriterium es sagen soll, ist Ermessen. **Kein Ready-Hindernis** — die Falle
steht in 2.2 und geht so in den Spawn-Auftrag.

### 6.3 An den PO: die Untergrenze für alle oder nur für eine Komponente

`KF_MIN_VERSION` auf 6.20 hebt die Untergrenze für ECM und alle zehn
KF6-Komponenten zugleich (2.6). Der engere Weg wäre
`find_package(KF6ColorScheme 6.20)` neben der bestehenden Liste. Auf Ganymed
und im öffentlichen Prüflauf ist beides gleichwertig; für ein Paket in einer
Distribution mit älterem KF6 ist es das nicht. ~~Entschieden ist der breite Weg;
die Frage ist, ob das mit dieser Folge entschieden wurde.~~

**Beantwortet am 07.08.2026, 21:17.** Der PO hat die Folge dem Kunden als
Berichtigung seiner eigenen Vorlage vorgelegt; der Kunde hat den **engeren**
Weg gewählt. `KF_MIN_VERSION` bleibt bei 6.0.0, `ColorScheme` bekommt eine
eigene Mindestversion 6.20, SPEC 15 trägt die Version für diese eine
Komponente. Feld 1 ist entsprechend nachgeführt (8.1).

**Erledigt am 07.08.2026, 21:29** (`4be3f8d`, nachgeprüft am Stand): Beide
Stellen tragen jetzt einen datierten Vermerk. `:565` fasst **P4** neu — „zugesichert
ist das Verfahren und keine Palettenrolle; geprüft unter zwei Farbschemata,
deren Ergebnis weit auseinanderliegt" —, `:553` berichtigt die Herkunft und
hält fest, dass die Kontrastwerte mit 0,20 gerechnet sind. Der Punkt ist
geschlossen; er steht hier, weil er der Grund der Berichtigung war.

### 6.4 An den Kunden — keine

Die Gestaltungsfrage ist am 06.08.2026 entschieden, die Skalierung 1,6 am
07.08.2026 bestätigt, der Weg zu `frameContrast` am 07.08.2026 gewählt. Was
offen ist, ist Formulierung und Bauweg.

---

## 7. Empfehlung an das Planning

**#101 ist ziehbar**, `size:m`, ein Strang, Zweig `story/101-listentrenner`,
eigener Worktree. Vier Zeilen gehören in den Spawn-Auftrag und ersparen je
einen Fehlversuch: 2.2 mit dem Heilort, 2.3 mit der Belegform, 2.5 mit der
Sekunde und 2.8 mit der Reihenfolge gegenüber `drawControl`.

---

## 8. Nachtrag 07.08.2026, 21:35 — was sich nach der Konsolidierung geändert hat

Dieser Bericht wurde um 21:04 gegen die erste Korrektur der Akzeptanzkriterien
konsolidiert (Issue-Kommentar von 20:55). Um **21:17** ist der zweite Nachtrag
zu AK 4 erschienen, um **21:29** der Commit `4be3f8d`. Die alten Fassungen
bleiben oben lesbar (B17); hier steht, was gilt.

**8.1 Der Bauplan ist enger** (Feld 1, Zeile „Build"; Feld 4, AK 10; Feld 6.3).
`KF_MIN_VERSION` bleibt bei `6.0.0`, die Komponentenliste `:24–34` bleibt
unberührt, und `ColorScheme` wird mit einem **eigenen** `find_package`-Aufruf
und der Mindestversion **6.20** gesucht. SPEC 15 trägt die Version für diese
eine Komponente. **Falle 2.6 ist damit nicht überholt, sondern eingelöst:** Sie
hat die Entscheidung ausgelöst und bleibt ihre Begründung. Der Satz „von 6.0.0
auf 6.20" in der ersten Fassung von Feld 1 hätte den Umsetzungsstrang den
falschen Bauplan bauen lassen — er ist die einzige Stelle dieses Berichts, an
der das möglich gewesen wäre.

**8.2 Die Ausnahme zur Kollisionsfläche entfällt** (Feld 1.1). Sie hing daran,
dass beide Stränge in `CMakeLists.txt:6–34` hätten liegen können. Beide
Bedingungen sind fort: `:6` bleibt unberührt (8.1), und der konsolidierte
Vorprüfbericht zu #100 hält für **beide** Bearbeiter fest, dass die Story
keinen Build-Eingriff braucht — `KF6::Svg` steht bereits in
`target_link_libraries(denkzettelcapture …)`. Der Bauplan gehört in diesem
Sprint allein #101. Der PO muss nichts takten.

**8.3 AK 4 ist noch einmal geschärft** (Feld 3, Feld 4). Der Nachtrag von 21:17
misst, dass **0 von 19** Schemata den Schlüssel `frameContrast` tragen — die
ursprüngliche Vorgabe „einem mit hohem und einem mit niedrigem Wert" war damit
nicht bloß falsch begründet, sondern unerfüllbar. Zugesichert ist jetzt das
**Verfahren**; geprüft wird unter zwei Schemata, deren **Ergebnis**
auseinanderliegt. Das Prüfmittel in Feld 4 stand bereits so und bleibt
unverändert; die Zeile in Feld 3, die die beiden Schemanamen für verschwunden
erklärte, ist berichtigt.

**8.4 Feld 6.1 ist erledigt** (`4be3f8d`, am Stand nachgeprüft). Zeichnung 3a
trägt an `:553` und `:565` datierte Vermerke, P4 ist neu gefasst. Der Befund
bleibt als Befund stehen — er war der Grund der Berichtigung.

**Nicht berührt:** Feld 2 außer 2.6, Feld 5 (`size:m`) und das Ready-Urteil
in Feld 3. Alle drei stehen unverändert.
