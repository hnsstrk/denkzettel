# Vorprüfung #101 — Messung B (Scrum Master)

**Bearbeiter B:** Agent `scrum-master`. **Datum:** 07.08.2026, 20:14–20:30.
**Gegenstand:** Issue #101 „Bibliothek: Notizen und Gruppen heben sich optisch
nicht voneinander ab", Akzeptanzkriterien AK 1–AK 7 aus dem PO-Kommentar vom
07.08.2026, Prüfsätze P1–P5 aus Zeichnung 3a.

**Unabhängigkeit:** `messung-a.md` ist nicht gelesen worden, ebensowenig eine
andere Datei, die Bearbeiter A in diesem Lauf angelegt hat. Der Ordner wurde
nicht aufgelistet; `git status --porcelain` meldete ihn als Ganzes und nannte
keine Dateinamen darin. Gelesen wurden Issue, Zeichnung 3a, `CLAUDE.md`,
`PROZESS.md`, der Untersuchungsbericht vom 06.08.2026 und der Produktivcode.

Beim abschließenden Lauf zeigte `messungen/` vier Dateien `m1` bis `m4`, die
nicht von mir stammen. Sie sind ungelesen geblieben; meine Ausgaben tragen das
Kürzel `mb`.

**Eigene Messungen:** drei Sonden unter `sonden/`, Ausgaben unter `messungen/`,
übersetzt und gelaufen in `build-b/`. Kein Bau im Wurzelverzeichnis, keine
Installation nach `/usr`, keine Änderung an Code oder SPEC.

| Beleg | Frage |
|---|---|
| `messungen/mb1-nachbarmalung.txt` | Erreicht `option.widget` den Delegate? Welche Zeilen malt die Ansicht bei einem Auswahlwechsel neu? Was malt `grab()`? |
| `messungen/mb2-framecontrast.txt` | Lässt sich `frameContrast` im Testmodus setzen — auch zweimal im selben Lauf? |
| `messungen/mb3-haarlinie-skalierung.txt` | Wie liegt eine 1 px hohe Linie bei 1,0 · 1,6 · 2,0? |
| `messungen/mb4-schemaherkunft.txt` | Woher kommen die beiden in AK 4 genannten Farbschemata? |

---

## 1. Dateimenge

Am Code vermessen, Notation nach B13.

| | **Strang #101 — Trenner in der Bibliotheksliste** |
|---|---|
| **Quellen & Tests** | `src/ui/notelistdelegate.{h,cpp}` (168 + 51 Zeilen) — **ganz**; die beiden Linien entstehen in `paint()` (`:94–152`), die Nachbarschaftsfragen an `index.model()`, die Farbe neben `groupHeadFont()` (`:36–41`). `src/ui/librarywindow.cpp` — **nur** der Aufbau der Liste (`:197–222`) und die Verbindung am `selectionModel` (`:316`): dort hängt die Neumalung der Nachbarzeilen (Befund 2.2). `tests/librarytest.cpp` (3943 Zeilen) — neue Prüffunktionen, angehängt; `tests/libraryshots.cpp` (521 Zeilen) — das Bild zu AK 7 |
| **Build** | **keine Änderung**. `KF6::ConfigCore` ist an `denkzettelui` bereits gebunden (`src/CMakeLists.txt:84`), und `frameContrast` ist ein Konfigurationswert, kein Palettenwert — eine neue Abhängigkeit entsteht nicht. `KF6::ColorScheme` wird **nicht** gebraucht; es kam nur in den Sonden des 06.08.2026 vor |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-NN-s101-listentrenner/` — Bericht, Bilder, Messprotokolle; darin das Bild nach AK 7 mit `QT_SCALE_FACTOR=1.6` |
| **Fachliche Quellen** | Zeichnung 3a (`wireframes/Denkzettel Wireframes.dc.html:551–569`) — **Referenz, nicht zu ändern**, sie ist mit `36156fd` nachgezogen. **SPEC.md ist nicht betroffen**: Der Ausschluss-Griff aus `CLAUDE.md` über `trennlinie\|weißraum\|haarlinie\|strich` findet in `SPEC.md` vier Zeilen — `:109` und `:212` treffen „Bindestrich" und „Spiegelstrich" und gehen die Sache nichts an, `:269` und `:273` sprechen von der Fußzeile des Erfassungsfensters. Über die Trennung in der Liste sagt `SPEC.md` nichts; SPEC 9 (`:674–700`) beschreibt die Gliederung der Liste und verweist für ihr Aussehen auf die Zeichnungen 3a/3b. Ändert sich das beim Bauen — etwa weil eine Bedingung entdeckt wird —, gilt DoD 4 |
| **Ausdrücklich nicht** | `src/ui/notelistmodel.{h,cpp}` — die Nachbarschaft einer Zeile ist über `index.model()` erreichbar, eine neue Rolle wird nicht gebraucht; `src/capture/*`, `src/shell/*`, `src/store/*`; `tests/capturetest.cpp`, `tests/captureshots.cpp`, `tests/editshots.cpp`, `tests/readmeshots.cpp`; `wireframes/`; `CMakeLists.txt` und `src/CMakeLists.txt` |

**`tests/searchshots.cpp` steht bewusst nicht in der Menge.** AK 6 spricht über
die Trefferliste; die Prüfung dafür liegt in `librarytest.cpp` (Befund 2.5),
und ein zweiter Bildläufer wäre für diese Story ohne Nachweiswert.

---

## 2. Gemessene Fallen

### 2.1 `option.widget` ist gesetzt — die Kopplung ans `selectionModel` ist billig

Der PO legt die Auswahl-Ausnahme in AK 3 als teuerste Zeile vor, mit der
Begründung, der Delegate kenne nur `option.state` seiner eigenen Zeile.

Gemessen (`mb1`, Prüffrage 1): `option.widget` erreicht den Delegate in jedem
Malvorgang, kein einziges Mal `nullptr`. Damit ist
`qobject_cast<const QAbstractItemView *>(option.widget)->selectionModel()` der
Weg zur Nachbarzeile, und `notelistdelegate.cpp:125` benutzt `entry.widget`
bereits. Die Nachbarzeile selbst kommt über `index.model()`; für „ist die
nächste Zeile ein Gruppenkopf" genügt `index.siblingAtRow(row + 1)` mit der
vorhandenen `GroupHeaderRole`.

**Die Kopplung ist damit nicht der Preis der Ausnahme.** Der Preis steht in 2.2.

### 2.2 Die Zeile über der neuen Auswahl wird nicht neu gemalt

Eine Linie an der **Unterkante** einer Zeile, die von der Auswahl der
**Nachbarzeile** abhängt, wird von der oberen Zeile gezeichnet. Wechselt die
Auswahl, muss die obere Zeile neu malen. Gemessen (`mb1`, Prüffrage 2), Liste
hoch genug, dass nicht gerollt wird:

| Wechsel | neu gemalt | Zeile über der **neuen** Auswahl | Zeile über der **alten** Auswahl |
|---|---|---|---|
| 2 → 3 | 2, 3 | ja | **nein** |
| 3 → 2 | 2, 3 | **nein** | ja |
| 1 → 5 | 1 … 5 | ja | **nein** |
| 5 → 1 | 1 … 5 | **nein** | ja |

Die Ansicht malt den umschließenden Bereich aus alter und neuer Auswahl neu.
Die Zeile **direkt darüber** liegt in keinem der vier Fälle darin. In jedem
Wechsel bleibt daher genau eine Linie falsch stehen: aufwärts eine, die
verschwinden müsste, abwärts eine, die zurückkommen müsste.

**Folge für die Dateimenge:** Der Delegate kann das nicht allein heilen. Die
Ansicht muss die Nachbarzeilen anstoßen — eine Verbindung an
`QItemSelectionModel::currentChanged` in `librarywindow.cpp:316`, wo bereits
eine steht.

### 2.3 Ein Bildbeleg kann diesen Fehler nicht zeigen

Gemessen (`mb1`, Prüffrage 4): Nach dem Wechsel 5 → 1 malte die Ansicht die
Zeilen 1–5. Das unmittelbar folgende `grab()` malte **alle zwölf** Zeilen,
Zeile 0 eingeschlossen.

`grab()` ist der Weg aller fünf Bildläufer und der von `capturetest.cpp:382`.
Ein Bild aus diesem Weg zeigt immer den frisch gemalten Zustand und niemals
einen stehengebliebenen Strich. Das ist der Fall, den `CLAUDE.md` benennt: ein
Testaufbau, in dem der Fehler gar nicht auftreten kann.

**Der tragfähige Nachweis ist ein Malzähler**, kein Bild — die Zusicherung
lautet dann „nach einem Auswahlwechsel wird die Zeile über der neuen und die
über der alten Auswahl neu gemalt". Die Sonde `sonden/nachbarmalung.cpp` ist
der fertige Aufbau dafür.

### 2.4 `frameContrast` im Test: der zweite Wert kommt nicht an

`frameContrast` steht in `[General]` der kdeglobals (auf der Maschine des
Kunden gemessen: `0.2`). Im Testmodus, den `librarytest.cpp:364` bereits
setzt, findet `KSharedConfig::openConfig()` keine kdeglobals und liefert den
Vorgabewert `0.20` (`mb2`, Prüffrage 1). Ein Test kann den Wert also selbst
setzen und ist von den Schemadateien der Maschine unabhängig.

**Nur nicht zweimal hintereinander** (`mb2`, Prüffrage 2): Vier Werte
nacheinander geschrieben, je mit `reparseConfiguration()` gelesen — der erste
kam an, die drei folgenden nicht. Mit 1100 ms Pause vor dem Lesen kamen alle
vier an (Prüffrage 3), ebenso alle über eine eigene Konfiguration auf den Pfad
(Prüffrage 4). Die Änderungserkennung von KConfig arbeitet in Sekunden.

**Das trifft AK 4 unmittelbar**, denn AK 4 verlangt zwei Schemata in einem
Lauf. Ein Test, der beide Male denselben Wert liest und seine Erwartung aus
derselben Quelle rechnet, ist **grün und wertlos**. Gegenmittel: vor der
Messung den gelesenen Wert selbst zusichern, so dass ein hängengebliebener
Wert laut fällt.

**Und es trifft die laufende Anwendung**, wenn der Delegate den Wert
zwischenspeichert: Der Bildlauf zum Schemawechsel (`libraryshots.cpp:400–444`,
Issue #58) reicht eine neue Palette an ein stehendes Fenster. `frameContrast`
kommt dabei nicht über die Palette.

### 2.5 Die Trefferliste braucht keinen neuen Prüfweg

`groupsTheSearchResultsLikeTheLibrary()` (`librarytest.cpp:2380`) baut die
gefilterte Liste bereits auf. Das Modell erzeugt die Köpfe aus den ihm
übergebenen Notizen (`notelistmodel.cpp:13–33`), so dass in der Trefferliste
dieselben Regeln greifen. AK 6 ist damit eine Wiederholung von AK 1–AK 3 auf
einem vorhandenen Aufbau.

### 2.6 Die Breite in AK 2 ist die des Sichtfeldes, nicht die des Fensters

Gemessen (`mb1`, Prüffrage 3): Ansicht 300 px breit, Sichtfeld 286 px, weil
der senkrechte Rollbalken steht. Das Rechteck, das der Delegate bekommt, ist
`x=0`, Breite 286, rechte Kante 285.

AK 2 verlangt die Linie „über die volle Breite … einschließlich x = 0 und
x = Breite−1". Gemeint ist das Sichtfeld. Ein Bild des ganzen Fensters trägt
rechts daneben den Rollbalken; die Prüfung nimmt deshalb
`m_list->viewport()->grab()`.

### 2.7 Die Haarlinie unter der Skalierung des Kunden

Gemessen (`mb3`) an einer QPixmap mit gesetztem `devicePixelRatio`, eine
Notizzeile von 300 × 60 logischen Bildpunkten, Linie 1 px hoch und 12 px
eingerückt:

| Skalierung | Bild | bemalte Zeilen | bemalte Spalten |
|---|---|---|---|
| 1,0 | 300 × 60 | y = 59 | x = 12 … 287 |
| 1,6 | 480 × 96 | y = 94 **und** 95 | x = 19 … 460 |
| 2,0 | 600 × 120 | y = 118 **und** 119 | x = 24 … 575 |

Die Linie trägt in allen drei Fällen die volle Farbe, wird aber bei 1,6 zwei
Gerätebildpunkte hoch, und die eingerückte Kante liegt bei 19 statt bei 12.

**AK 1–AK 3 sind Aussagen in logischen Bildpunkten.** Wörtlich prüfbar sind
sie bei Skalierung 1 — der Belegform von AK 1–AK 5 entspricht das, sie nennt
zwei Fenstergrößen und zwei Farbschemata und keine Skalierung. AK 7 verlangt
ein Bild bei 1,6 und ist ein Sichtbeleg, keine Bildpunktzusicherung. Beides
verträgt sich, sobald das Prüfmittel die Skalierung mitnennt.

### 2.8 Die beiden in AK 4 genannten Schemata gibt es im Prüflauf nicht

Gemessen (`mb4`): `KritaDarkOrange.colors` gehört zum Paket `krita`,
`IridescentLightly.colors` zu `cachyos-iridescent-kde`. Die Paketliste des
öffentlichen Bau- und Testlaufs (`.github/workflows/ci.yml:66–73`) enthält
keines von beiden, und `cachyos-iridescent-kde` liegt außerhalb der
Arch-Paketquellen.

Ein Test, der diese Dateien beim Namen liest, fällt dort rot aus oder
überspringt sich still. Der Weg aus 2.4 — `frameContrast` selbst setzen —
umgeht das vollständig.

### 2.9 Was nicht gemessen ist, ausdrücklich

- **Die Wirkung im Auge des Kunden.** 1,24 : 1 im schlechtesten Schema ist die
  bekannte Zahl; ob der Kunde die Linie dort sieht, entscheidet die Abnahme.
- **Der Zeigerzustand (Hover).** Kein Kriterium spricht darüber. Der Stil malt
  die überfahrene Zeile mit eigenem Grund (`notelistdelegate.cpp:126`); ob die
  Linien an ihren Kanten stehenbleiben sollen, ist offen (6.4).
- **Das Bild in der angemeldeten Sitzung.** Kein Kriterium dieser Story spricht
  über Hülle, Rundung, Kontur, Schatten oder Dekoration; B21 verlangt hier
  kein Sitzungsbild. Die Bibliothek ist ein von KWin dekoriertes Fenster
  (SPEC 3), ihre Liste zeichnet aus Palette und Konfiguration.
- **Der gebaute Stand.** Es wurde kein Produktivcode übersetzt; die Sonden
  bilden das Verhalten von `QListView` und `KConfig` nach und kennen weder
  `NoteListModel` noch `NoteListDelegate`.

---

## 3. AK-Urteil: **ready nein**

Das Urteil folgt aus meiner Messung; die Zusammenführung mit Messung A ist ein
späterer Arbeitsgang.

**Sechs der sieben Kriterien tragen.** AK 1, AK 2, AK 4, AK 5, AK 6 und AK 7
sind einzeln prüfbar, und zu jedem ist in Feld 4 ein Prüfmittel benannt.

**AK 3 ist unvollständig.** Es beschreibt einen Zustand („die betreffende
Bildpunktzeile trägt Listengrund beziehungsweise Auswahlfarbe"), während die
Sache ein Übergang ist: Die Ausnahme hängt an der Auswahl, und die Auswahl
wandert. Gemessen (2.2) bleibt bei **jedem** Auswahlwechsel genau eine Linie
falsch stehen, und gemessen (2.3) kann kein Bildbeleg das zeigen. Ein Bau, der
AK 3 wörtlich erfüllt, liefert dem Kunden bei jedem Tastendruck ein falsches
Bild — und würde jede Prüfung bestehen, die das Kriterium in seiner jetzigen
Fassung nahelegt.

Damit steht AK 3 in derselben Reihe wie die drei Sätze, die die Definition of
Ready am 04.08.2026 hinzubekommen hat: Ein Kriterium, dessen naheliegende
Belegform gegen seinen eigenen Fehlerfall blind ist, gehört vor dem Ziehen
geändert, nicht im Review entdeckt.

**Die Behebung ist Sache des PO** (melden, nicht heilen) und in einem von zwei
Sätzen getan:

- **Ergänzen.** AK 3 bekommt einen zweiten Absatz: *„Wechselt die Auswahl,
  werden die Zeile über der neuen und die Zeile über der alten Auswahl neu
  gemalt; die Ansicht malt von sich aus nur den Bereich zwischen beiden
  Auswahlen."* Prüfmittel dazu steht in 4.
- **Streichen.** Die Auswahl-Ausnahme fällt, die Linie bleibt an den Kanten der
  ausgewählten Zeile stehen — der Rückfall, den der PO im Issue selbst benannt
  hat. AK 3 verkürzt sich dann auf die Gruppengrenzen und die Köpfe, und 2.2
  entfällt ersatzlos.

Beides macht die Story ziehbar. Die Wahl gehört dem PO, mit der Zeichnung 3a im
Rücken: Der eine Satz, der von der alten Festlegung wirksam bleibt, ist der,
dass eine zweite Trennung nicht mit der Auswahlmarkierung in Wettbewerb treten
darf — er stützt das Ergänzen.

**Kein zweiter Grund für „nein".** Insbesondere ist das Issue frei von
selbstdeklarierten offenen Punkten: Die fünf Schwächen aus dem Kommentar vom
06.08.2026 sind der Vorprüfung vorgelegt, was ihr Zweck ist, und die
Gestaltungsfrage ist mit der Kundenentscheidung geschlossen.

---

## 4. Prüfmittel

| AK | Prüfmittel | Bemerkung |
|---|---|---|
| **AK 1** (P1) | `librarytest.cpp`, neue Prüffunktion: `m_list->viewport()->grab().toImage()` nach `QTest::qWaitForWindowExposed`, offscreen, **zwei Fenstergrößen**, Skalierung 1. Bildpunktzeile `rect.bottom()` der oberen Notiz gegen die Linienfarbe, `x = 0…11` und `x = Breite−12…Breite−1` gegen den Listengrund | Die 12 px sind bereits zugesichert (`librarytest.cpp:2683`, `NoteListDelegate::textLeft`); die neue Zusicherung fragt dieselbe Zahl über den gemalten Bildpunkt |
| **AK 2** (P2) | Derselbe Aufbau, Bildpunktzeile `rect.top()` des zweiten Gruppenkopfes über die volle Breite, `x = 0` und `x = Breite−1` eingeschlossen; über dem ersten Kopf Listengrund | **Breite ist die des Sichtfeldes** (2.6): 286 von 300 gemessen, sobald der Rollbalken steht |
| **AK 3**, Zustandsteil | Derselbe Aufbau, drei Fälle einzeln: unter der letzten Notiz einer Gruppe, unter einem Kopf, an beiden Kanten der ausgewählten Zeile | Der dritte Fall braucht eine gesetzte Auswahl vor dem Bild |
| **AK 3**, Übergangsteil (fehlt heute) | Malzähler statt Bild: ein `QStyledItemDelegate`, der die Malvorgänge je Zeile zählt, Auswahl wechseln, `QTest::qWait`, zählen. Fertiger Aufbau: `sonden/nachbarmalung.cpp` | **Ein Bild kann diesen Fall nicht prüfen** (2.3) — `grab()` malt alles neu |
| **AK 4** (P4) | `QStandardPaths::setTestModeEnabled(true)` (steht in `librarytest.cpp:364`), kdeglobals mit gewähltem `frameContrast` schreiben, gelesenen Wert **zuerst zusichern**, dann Palette setzen und die gemalte Linienfarbe gegen die selbst gerechnete Mischung halten. Zwei Werte, dazwischen 1100 ms oder eine eigene Konfiguration auf den Pfad | Ohne die Pause liest der zweite Durchgang den ersten Wert (2.4). Die Namen `KritaDarkOrange` und `IridescentLightly` taugen als Herkunft der Zahlen, **nicht** als Prüfmittel (2.8). „Keine Palettenrolle" wird positiv geprüft: Die Mischung ist von jeder Rolle der Palette verschieden |
| **AK 5** (P5) | `ctest --test-dir build -R librarytest` — die vorhandenen Geometrie- und Kopf-Zusicherungen laufen unverändert. Der Bezug zu #70 hängt an `bringsTheHeadOfTheNewGroupIntoView*` und `bringsTheHeadAlongWhenTheSelectionReachesTheFirstNoteOfItsGroup` (`librarytest.cpp:192–200`) | Rechnerisch unberührt: Die Eintragslinie liegt in den vorhandenen 9 px Innenabstand unten, die Kopflinie in den 14 px oben (`notelistdelegate.cpp:26–28`, `:141–164`) |
| **AK 6** | `groupsTheSearchResultsLikeTheLibrary()` (`librarytest.cpp:2380`) als Aufbau, darauf dieselben Bildpunktzusicherungen wie AK 1–AK 3 | Kein neuer Prüfweg (2.5) |
| **AK 7** | `cmake --build build --target libraryshots`, dann `QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.6 libraryshots <Ordner>`, Bild nach `docs/scrum/reviews/` | Erst bauen, dann laufen. Bei 1,6 ist die Linie zwei Gerätebildpunkte hoch und die Kante liegt bei 19 (2.7) — das Bild ist ein Sichtbeleg, keine Bildpunktzusicherung |

**Belegform.** Die Abweichung des UX-Agenten — P1 bis P5 unter zwei
Farbschemata, aber nicht unter zwei Desktop-Themes — habe ich unabhängig
geprüft und halte sie für richtig, mit einer Präzisierung ihrer Begründung:
Die Linienfarbe kommt nicht allein aus der Palette, sondern aus der Palette
**und** dem Konfigurationswert `frameContrast` in kdeglobals (2.4). Beide
Quellen sind vom Desktop-Theme unabhängig; die Bibliothek ist zudem ein von
KWin dekoriertes Fenster und holt sich keine Hülle aus `dialogs/background`
(SPEC 3). Ein Lauf über zwei Themes maße hier nichts. Die Formulierung der
Zeichnung — „die Liste zeichnet aus der Palette" — trägt das Ergebnis, nennt
aber nur die halbe Quelle.

**Was ein Agent nicht prüfen kann:**

- **Ob der Kunde die Linie sieht.** Der schlechteste gemessene Wert ist
  1,24 : 1. Das ist eine Wahrnehmung und gehört in die Abnahme.
- **Den stehengebliebenen Strich am Bildschirm.** Der Malzähler belegt, dass
  die Ansicht die Nachbarzeile anstößt. Dass der Bildschirm danach richtig
  aussieht, folgt daraus, ist selbst aber nicht abgegriffen: Ein Agent kommt
  unter Wayland an den Bildspeicher des Compositors nicht heran, und `grab()`
  malt neu (2.3).
- **Den Fensterwechsel.** Unverändert gilt Sprint 6, §16.1 M-B1: Kein Agent
  holt sich unter Wayland den Fokus zurück.

---

## 5. Größenklasse: **`size:m`**

Begründung an den Vergleichsfällen des Backlogs: #70, #71 und #72 tragen
`size:s` und fassen je eine Stelle in der Bibliothek an; #76 und #85 tragen
`size:m`; #83 trägt `size:l` und war die Fensterhülle mit 19 Kriterien, KSvg,
Schatten, Weichzeichner und Sitzungsbildern.

#101 liegt dazwischen und näher an `m`:

- **Der Bau ist überschaubar.** Zwei Linien in `paint()`, zwei
  Nachbarschaftsfragen, eine Farbmischung, eine Verbindung in
  `librarywindow.cpp`. Keine Build-Änderung, keine neue Abhängigkeit, kein
  neues Modellfeld.
- **Der Prüfweg ist neu.** `librarytest.cpp` enthält heute **keine einzige**
  Bildpunktprüfung — kein `QImage`, kein `pixelColor`. Der Aufbau dafür
  existiert im Repository (`capturetest.cpp:382`), muss hier aber neu
  entstehen, und dazu kommen der Malzähler (2.3) und der
  Konfigurationsschalter (2.4). Das ist der Teil, der die Story über `s` hebt.
- **Für `l` fehlt die Breite.** Die Fläche ist eine Datei von 168 Zeilen plus
  eine Verbindungszeile; es gibt keine zweite Ansicht, kein zweites Fenster
  und keinen zweiten Prozess.

**Was die Klasse verschiebt:** Streicht der PO die Auswahl-Ausnahme (3.,
zweiter Weg), entfallen die Kopplung ans `selectionModel`, die Neumalung und
der Malzähler — dann ist die Story an der Grenze zu `size:s`. Ich lasse sie
gleichwohl bei `m`, solange die Ausnahme steht.

**Zum Label:** Es wird im selben Zug gesetzt wie der konsolidierte Bericht,
vorher gar nicht. Ich setze es hier nicht.

---

## 6. Offene Fragen

**6.1 An den PO — AK 3, die Auswahl-Ausnahme.** Ergänzen oder streichen? Die
Gegenrede, die der PO ausdrücklich in dieses Feld haben wollte, fällt nach der
Messung zweigeteilt aus: Die Kopplung ans `selectionModel` ist **billig** und
kein Grund zum Streichen (2.1). Die Neumalung der Nachbarzeile ist der
eigentliche Preis (2.2) — sie ist klein zu bauen und leicht zu vergessen, und
kein Bild deckt sie auf. Meine Empfehlung: **ergänzen**, weil der eine wirksam
gebliebene Satz der alten Festlegung genau diese Ausnahme trägt.

**6.2 An den PO — AK 4, die beiden Schemanamen.** Sind `KritaDarkOrange` und
`IridescentLightly` als Prüfvorschrift gemeint oder als Herkunft der Zahlen
1,93 : 1 und 1,24 : 1? Als Vorschrift gelesen bindet AK 4 die Prüfung an zwei
Pakete, die der öffentliche Prüflauf nicht kennt (2.8). Ich habe Feld 4 unter
der zweiten Lesart geschrieben.

**6.3 An den PO — AK 2, „volle Breite der Liste".** Bestätigen als **Breite des
Sichtfeldes** (2.6), damit die Prüfung `viewport()->grab()` nimmt und nicht das
Fensterbild.

**6.4 An den PO — der überfahrene Eintrag.** Kein Kriterium sagt, ob die Linien
an den Kanten einer nur überfahrenen Zeile stehenbleiben. Der Stil malt dort
einen eigenen Grund. Vorschlag: stehenlassen und in AK 3 einen Halbsatz
ergänzen, damit der Punkt nicht im UI-Review neu aufgemacht wird.

**6.5 An den Kunden, über den PO — die Skalierung.** AK 7 nennt „seine
Einstellung"; im Repository ist 1,6 belegt (`docs/scrum/reviews/sprint-07-ui-review/bericht.md:26`,
gemessen 04.08.2026). Gilt der Wert noch?

---

**Nachtrag 07.08.2026, 20:59 — zwei Angaben dieses Berichts sind nachgemessen
und eine davon ist falsch.** Der Text oben bleibt, wie er war (B17).

1. **Abschnitt 2.4, „`frameContrast` steht in `[General]` der kdeglobals": Das
   stimmt nicht.** Der Schlüssel steht in der Gruppe **`[KDE]`**; zwischen der
   `[General]`-Überschrift und der Fundzeile liegt eine weitere
   Gruppenüberschrift, die der Griff nicht angezeigt hat.
   `KColorScheme::frameContrast()` liest ebenfalls `[KDE]` — eine Datei mit
   `[General] frameContrast=0.45` allein liefert die Voreinstellung 0,2000
   (`messungen/mb5-nachpruefung-kde.txt`, Prüffrage 1). Messung A hatte recht.
   **Die Aussagen über den Mechanismus bleiben:** Testmodus, Voreinstellung
   0,20 und die Sekunden-Trägheit sind auf dem entschiedenen Weg
   (`KColorScheme::frameContrast()`) noch einmal nachgemessen und gelten
   unverändert.
2. **Abschnitt 2.6, Sichtfeld 286 von 300:** gemessen ohne
   `QT_QPA_PLATFORMTHEME=kde`, also unter dem Stil `fusion` mit einem
   Rollbalken von 14 px. Unter dem Plattformthema des Kunden zeichnet Breeze
   einen Rollbalken von 21 px, und die Zeile ist 279 px breit
   (`mb5`/`mb6`, Prüffrage 3). Die Aussage — die Breite ist die des
   Zeilenrechtecks und nicht die der Ansicht — trägt unverändert; die Zahl
   gehört zum Aufbau und nicht ins Kriterium.
