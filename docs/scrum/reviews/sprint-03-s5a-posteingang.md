# UI-Review Sprint 3 — Notizliste als Posteingang (S5a, Issue #46)

**Modus:** UI-Review mit eigener Bildprüfung (DoD 3, Retro-Beschluss B3).
**Datum:** 01.08.2026. **Prüfer:** `denkzettel-ux`.

**Geprüfter Stand:** `main`, Merge `91ee5ec` über den Commits `4746358` und
`e18630c`. Gebaut out-of-source im Scratchpad
(`cmake -S /home/hnsstrk/Projekte/denkzettel -B <scratchpad>/build-ux
-DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON`); im Arbeitsbaum wurde nur unter
`docs/scrum/reviews/` geschrieben, an Quellcode, SPEC und Wireframes nichts
geändert.

**Ergebnis: ein `fail`, drei `warn`.** Der `fail` betrifft AK 7 (Kopf der neuen
Gruppe beim Sprung über die Gruppengrenze) und ist mit einer kleinen Auflage zu
räumen — wahlweise am Code oder an der Zeichnung. Die Gliederung selbst ist
richtig gebaut und liest sich als Posteingang; die Farbfrage (AK 5), die keinen
Test hat, ist am Bild gemessen und in Ordnung.

## Prüfmittel

- Testlauf des gemergten Standes: `ctest` offscreen, **7 von 7 grün**
  (`librarytest` 4,13 s).
- Eigener Bildlauf gegen die gebaute `denkzettelui`, `QT_QPA_PLATFORM=offscreen`,
  `-style breeze`, `QWidget::grab().save()`. Quelle: `ux-shot-s5a.cpp` im
  Ordner neben diesem Bericht, Messwerte `messung.txt`.
- **Zwei Läufe, weil das Plattformthema die Schriftmaße bestimmt:** der
  Hauptlauf mit `QT_QPA_PLATFORMTHEME=kde` (Plasma-Schriften und -Farben, das
  Zielumfeld der App) und ein Gegenlauf ohne, abgelegt unter
  `ohne-plasma-thema/`. Siehe Hinweis 1.
- **Pixelmessung statt Zusicherung** für AK 5 und für die linke Textkante: das
  Programm rastert den Listen-Viewport und liest je Zeile die linke Tintenkante
  und die kräftigste Farbe je Textzeile aus dem Bild. Die Bänder folgen den
  Maßen aus Wireframe 3a (9 px, Zeitstempel, 3 px, zwei Textzeilen).
- Bezugszeitpunkte fest gesetzt (`LibraryWindow::setReferenceTime`), damit die
  Bilder kalenderunabhängig sind: **Fr 31.07.2026 16:20** für den Normalfall,
  **Mo 03.08.2026 10:00** für die Sonderfälle.

### Geprüfte Bilddateien (alle unter `sprint-03-s5a-posteingang/`)

| Datei | Zustand nach Wireframe |
|---|---|
| `s5a-a-posteingang-900x600.png` | 3a, alle fünf Gruppen, Auswahl auf der zweiten Notiz |
| `s5a-a-posteingang-1200x800.png` | 3a, zweite Fenstergröße |
| `s5a-a-schmal-220px.png` | 3a, Liste auf das Minimum von 220 px geschoben |
| `s5a-a-ohne-auswahl-900x600.png` | 3a ohne Auswahl (Farben ohne Auswahlfläche) |
| `s5a-b-sonderfaelle-montag-900x600.png` | 3b Fall 1–3 und die Montagsprobe („Diese Woche“ leer) |
| `s5a-b-geloescht-kopf-weg-900x600.png` | AK 9, letzte Notiz einer Gruppe gelöscht |
| `s5a-b-undo-kopf-zurueck-900x600.png` | AK 9, Undo bringt Notiz und Kopf zurück |
| `s5a-c1-vor-der-grenze-900x600.png` | 3b Fall 4, Auswahl auf der letzten Notiz von „Heute“ |
| `s5a-c1-abwaerts-ueber-die-grenze-900x600.png` | 3b Fall 4, abwärts über die Grenze |
| `s5a-c2-aufwaerts-erste-notiz-der-gruppe-900x600.png` | 3b Fall 4, aufwärts auf die **erste** Notiz einer Gruppe |
| `s5a-c3-aufwaerts-letzte-notiz-der-gruppe-900x600.png` | 3b Fall 4, aufwärts auf die **letzte** Notiz einer großen Gruppe |
| `s5a-c4-kleine-gruppe-aufwaerts-900x600.png` | 3b Fall 4, aufwärts in eine **kleine** Gruppe — Beleg des `fail` |
| `s5a-d-leere-bibliothek-900x600.png` | 2c, Leerzustand 1 |

Im Bild der leeren Bibliothek fehlt das Fenstersymbol: mein Helfer setzt
`qApp->windowIcon()` nicht, die Anwendung tut es in `main.cpp`. Das ist ein
Artefakt der Prüfvorrichtung, kein Rückfall hinter Beschluss B6.

## Die beiden ausdrücklich gestellten Fragen

### Frage 1 — trägt die Auslegung von AK 7? — **fail**

**Kurz: das Ergebnis trägt, die Regel nicht.** Der Entwickler holt den
Gruppenkopf genau dann ins Bild, wenn die Auswahl auf der **ersten Notiz ihrer
Gruppe** steht (`src/ui/librarywindow.cpp:389–393`: er prüft, ob die
*unmittelbar vorangehende Zeile* ein Kopf ist). Das ist eine andere Regel als
die gezeichnete, und sie ist in einem Fall zu eng.

**Wo er recht hat** (`s5a-c3-…png`, Messwerte in `messung.txt`): Springt die
Auswahl aufwärts auf die letzte Notiz einer **großen** Gruppe, liegt deren Kopf
531 px über dem Viewport-Rand, der 552 px hoch ist. Kopf und Auswahl passen
nicht gleichzeitig ins Bild; die Zusage des PO vom 01.08.2026 (Auswahl
vollständig sichtbar) hat Vorrang. Hier ist das gebaute Verhalten das einzig
richtige, und es gehört als Regel aufgeschrieben.

**Wo er nicht recht hat** (`s5a-c4-kleine-gruppe-aufwaerts-900x600.png`): Die
Auswahl springt per Pfeiltaste aufwärts aus „Letzte Woche“ in die dreiköpfige
Gruppe „Gestern“ und landet auf deren letzter Notiz. Der Kopf „Gestern“ steht
bei **y = −35 px** — er ist um genau seine eigene Höhe aus dem Bild geschoben
und hätte mühelos hineingepasst (Viewport 552 px, Kopf 35 px, Auswahl 72 px).
Er wird nicht geholt. Was der Nutzer sieht: drei Einträge mit reiner Uhrzeit
(„10:00“, „09:00“, „08:00“, denn in „Gestern“ trägt der Kopf den Tag), keine
Überschrift darüber — und die einzige sichtbare Überschrift, „Letzte Woche“,
steht **unter** der Auswahl. Der Tag der ausgewählten Notiz steht damit nirgends
auf dem Bildschirm. Genau das wollte die Zeichnung verhindern.

Die Berufung auf Fall 4 der Zeichnung trägt nicht: Der dort ohne Überschrift
gezeichnete oberste Eintrag ist **nicht die Auswahl** — die Auswahl steht in
Fall 4 unter ihrem frisch hereingescrollten Kopf „Letzte Woche“. Die Zeichnung
erlaubt also, dass *andere* Einträge ohne ihre Überschrift dastehen, nicht die
Auswahl.

**Auflage (eine der beiden, PO entscheidet):**

1. *Bevorzugt:* Die Regel wird an dem festgemacht, was ins Bild passt, statt an
   der Stellung der Notiz in ihrer Gruppe. Fachlich: Beim Wechsel der Auswahl
   den Kopf **ihrer Gruppe** suchen (von der Auswahl aufwärts bis zur ersten
   Zeile mit `GroupHeaderRole`); ihn nur dann vor der Auswahl ins Bild holen,
   wenn Kopf und Auswahl zusammen in den Viewport passen
   (`auswahl.bottom() − kopf.top() <= viewport.height()`), sonst wie bisher nur
   die Auswahl. Beides ist mit `visualRect` prüfbar, also mit dem Werkzeug, auf
   das die Schätzklausur die Geometrie-Zusicherungen schon festgelegt hat; ein
   Testfall wie `bringsTheHeadOfTheNewGroupIntoView`, nur mit einer kleinen
   Gruppe, deckt ihn ab. **Ohne die Passt-Bedingung nicht bauen** — ein
   bedingungsloses Vorscrollen würde in der großen Gruppe die Auswahl an den
   *unteren* Rand werfen und einen einzelnen Tastendruck um einen ganzen
   Bildschirm springen lassen.
2. *Alternativ:* Die Zeichnung (3b, Fall 4) und AK 7 werden auf das gebaute
   Verhalten zurückgeschnitten — ausdrücklich, nicht stillschweigend: „Der Kopf
   kommt mit ins Bild, wenn die Auswahl auf der ersten Notiz ihrer Gruppe steht;
   springt sie aufwärts in eine Gruppe hinein, gilt die vollständige
   Sichtbarkeit der Auswahl vor der Überschrift.“ Dann ist der Fall aus
   `s5a-c4-…png` eine bewusst hingenommene Grenze und muss so in der
   Festlegungstafel stehen.

Was **nicht** zulässig ist: den Widerspruch stehen lassen. Wer die heutige
Zeichnung liest, baut etwas anderes als das, was heute läuft.

### Frage 2 — AK 5, die Farben ohne Test — **ok**

Am gerasterten Bild gemessen, Plasma-Palette (Hauptlauf, `messung.txt`):

| Stelle | gemessene Farbe | erwartete Rolle | Palette |
|---|---|---|---|
| Zeitstempel, nicht ausgewählt | `#9ea6ae` … `#a1a9b1` | `PlaceholderText` | `#a1a9b1` |
| Betreff, nicht ausgewählt | `#fcfcfc` | `Text` | `#fcfcfc` |
| Vorschau, nicht ausgewählt | `#a1a9b1` | `PlaceholderText` | `#a1a9b1` |
| Zeitstempel, ausgewählt | `#f7fafb` | `HighlightedText` | `#fcfcfc` |
| Betreff, ausgewählt | `#fcfcfc` | `HighlightedText` | `#fcfcfc` |
| Vorschau, ausgewählt | `#fcfcfc` | `HighlightedText` | `#fcfcfc` |
| Gruppenkopf | `#fcfcfc` | `Text` (nicht ausgegraut) | `#fcfcfc` |

Abweichungen um wenige Stufen sind Kantenglättung — gemessen wird der kräftigste
Bildpunkt der Textzeile. Der Gegenlauf ohne Plasma-Thema bestätigt dasselbe
Muster in der hellen Qt-Palette (Betreff `#000000`, Vorschau `#7f7f7f`,
Auswahl `#ffffff`).

Damit ist belegt: Betreff in Textfarbe, Vorschau gedämpft, in der Auswahl beide
in `HighlightedText`, der Kopf in voller Textfarbe statt in Grau (er kommt vom
View deaktiviert an — der Delegate fragt bewusst die `Normal`-Gruppe der
Palette ab, `src/ui/notelistdelegate.cpp:118`). **Kein Fettdruck im Eintrag**:
der Delegate malt Betreff und Vorschau mit `entry.font`, also unverändert; nur
der Kopf setzt `QFont::DemiBold`. Im Bild ist kein fetter Eintragstext zu sehen.

Ein Nebenbefund zur Sorgfalt: Der Unterschied Betreff/Vorschau ist in der hellen
Qt-Standardpalette schwächer als in Plasma, weil `PlaceholderText` dort nur
Text mit halber Deckkraft ist (Alpha 128 → `#7f7f7f` auf Weiß). Das ist Qt, nicht
Denkzettel, und in beiden Fällen deutlich genug. Für das dunkle Plasma-Schema
gilt: `#a1a9b1` gegen `#fcfcfc` ist gut unterscheidbar, ohne dass die Vorschau
unlesbar wird.

## Prüfpunkte aus der Zeichnung, Bereich für Bereich

Jeder gezeichnete Bereich aus 3a und 3b erzeugt genau eine Frage.

| # | Bereich der Zeichnung | Befund | Verdikt |
|---|---|---|---|
| 1 | Kopfzeile mit Suchfeld (aus 2b unverändert) | sitzt oben bündig, wächst nicht mit; **48 px hoch, die Raumaufteilung in 2c und AK 12 nennen „unter 40 px“** | **warn** (Befund 1) |
| 2 | Gruppenköpfe, Reihenfolge Heute · Gestern · Diese Woche · Letzte Woche · Älter | in dieser Reihenfolge, leere Gruppe fehlt ganz | ok |
| 3 | Kopf: Kleinschrift wie der Zeitstempel, halbfett, normale Textfarbe | 8,25 pt gegen 9,75 pt Eintragsschrift, DemiBold, `Text` | ok (siehe Hinweis 1) |
| 4 | Kopfabstände 6/6 (erster) und 14/6 (jeder weitere) | Zeilenhöhen 27 px und 35 px bei 15 px Schrifthöhe — exakt 6+15+6 und 14+15+6 | ok |
| 5 | Kopftext linksbündig mit dem Zeitstempel, 12 px | gemessene Tintenkante 12–13 px in beiden Zeilenarten; Rest ist Glyphen-Seitenlage. Konstruktiv gehalten: beide Malwege gehen durch `drawLine()`/`textLeft()` | ok |
| 6 | Kopf ohne Linie, ohne Fläche | keine Linie, kein Balken, keine Auswahlfläche | ok |
| 7 | Eintrag 9 px oben/unten, 12 px seitlich, Zeitstempel + zwei Textzeilen | 72 px = 9+15+3+2×18+9, **jeder** Eintrag gleich hoch, auch der Einzeiler | ok |
| 8 | Trennlinien zwischen den Einträgen (in 2b und 3a gezeichnet) | im Bau nicht vorhanden — schon seit S5, nie beanstandet | **warn** (Befund 2) |
| 9 | Auswahlmarkierung | Stil-Auswahlfläche über die ganze Zeile, Text in `HighlightedText` | ok |
| 10 | Detailbereich, volle Zeitform | „Heute 11:05“, „Gestern 21:48“, „Do., 30. Juli“ | ok |
| 11 | Listenbreite 300 px, Minimum 220 px | Splitter 300/599, `minimumWidth()` 220 | ok |
| 12 | „▶ 0:41“ und Tag-Chips in 3a/2b | planmäßig nicht gebaut: Sprachnotizen sind M4, Tags M3 (3b sagt es selbst) | ok |
| 13 | 3b Fall 1 — Gruppe mit genau einem Eintrag | „Gestern“ und „Letzte Woche“ tragen je einen Eintrag und ihren Kopf | ok |
| 14 | 3b Fall 1 — leere Gruppe wird nicht gezeichnet | Montagsbild: „Diese Woche“ fehlt vollständig | ok |
| 15 | 3b Fall 2 — lange erste Zeile ohne Umbruch | Betreff bricht an der Zeilenbreite, Vorschau setzt fort und elidiert | ok |
| 16 | 3b Fall 3 — echter Umbruch, Trenner „ · “ | „Einkauf Samstag“ / „Mehl · Hefe · Zitronen · Sahne · Butter“ | ok |
| 17 | 3b Fall 3 — Einzeiler, leere Vorschauzeile, gleiche Höhe | „Reifen wechseln lassen“, Vorschauband ohne Tinte, Zeile weiter 72 px | ok |
| 18 | 3b Fall 4 — Köpfe scrollen mit, kleben nicht oben | „Heute“ verlässt das Bild nach oben (y = −171 bzw. −531) | ok |
| 19 | 3b Fall 4 — Auswahl vollständig sichtbar | in allen vier Scroll-Bildern innerhalb des Viewports | ok |
| 20 | 3b Fall 4 — Kopf der neuen Gruppe mit im Bild | nur beim Sprung auf die erste Notiz einer Gruppe | **fail** (Frage 1) |
| 21 | 3b — Leerzustände unverändert | keine Gruppe, kein Kopf, nur der Platzhalter | ok |
| 22 | Festlegungstafel — Kalenderwoche, erste passende Gruppe gewinnt | Montagsbild: Sonntag unter „Gestern“, Donnerstag der Vorwoche unter „Letzte Woche“, obwohl nur vier Tage her | ok |
| 23 | Festlegungstafel — Zeitstempel folgt der Gruppe | „14:32“ · „21:48“ · „Di., 28. Juli“ · „Do., 23. Juli“ · „12.07.2026“ | ok |
| 24 | Festlegungstafel — Kopf ist keine Notiz | Pfeiltasten überspringen ihn lückenlos (C1: Zeile 8 → Zeile 10), Auswahl landet nie auf ihm, Löschen findet dort nichts | ok |
| 25 | Festlegungstafel — bewusste Auslassungen | kein Umschalter, keine Aufklapp-Pfeile, kein Zähler am Kopf | ok |
| 26 | Raumaufteilung bleibt (Liste über 450 px bei 900×600) | Splitter 552 px hoch | ok |

## Weitere Befunde

### Befund 1 — Kopfzeile 48 px statt „unter 40 px“ — **warn**

Gemessen 48 px (Plasma-Thema) bzw. 47 px (Qt-Standard), bei beiden
Fenstergrößen gleich. Die Zahl „unter 40 px“ steht in der Raumaufteilung von 2c
und ist in AK 12 dieser Story übernommen worden. Sie war schon am S5-Stand
verfehlt (die frühere Messung in
`2026-08-01-ui-review-s5-heilung/ux-geometrie.txt` weist 47 px aus) und ist
diesmal wortwörtlich zum Akzeptanzkriterium erhoben worden, ohne dass jemand
nachgemessen hätte. Der *Sinn* des Satzes — schmales Band, wächst nicht mit dem
Fenster — ist erfüllt.

**Auflage:** Die Zahl in 2c auf das korrigieren, was ein Breeze-`QLineEdit` plus
8 px Rand kostet (**48 px**), oder den Rand auf 4 px setzen, wenn der Kunde ein
schmaleres Band will. Nicht: die Zahl weiter mitschleppen. Ich schlage die
Korrektur der Zeichnung vor — 48 px sind für ein Suchband regelgerecht, und die
Ränder zu kürzen brächte die Kopfzeile unter das, was die KDE-HIG an Luft um ein
Eingabefeld vorsehen. — *Erledigt, siehe Nachtrag.*

### Befund 2 — Trennlinien zwischen den Einträgen fehlen — **warn**

2b und 3a zeichnen unter jedem Eintrag eine Haarlinie; gebaut ist keine. Die
Maßtafel von 3a betont eigens, dass der **Kopf** keine Linie trägt — was sich
liest, als hätten die Einträge welche. Der Bau hat die Linien nie gehabt; der
Kunde hat den linienlosen Stand in der Sprint-2-Abnahme gesehen und daran nichts
beanstandet.

**Auflage:** Die Zeichnung nachziehen und die Haarlinien aus den Listenspalten
in 2b, 3a und 3b entfernen. Begründung, weshalb ich die Zeichnung ändern will
und nicht den Bau: Eine Liste, deren Gruppen durch Überschriften gegliedert
sind, braucht keine zweite Trennung; Linien zwischen den Einträgen träten in
Wettbewerb mit den Köpfen und schwächten genau die Gliederung, für die diese
Story gebaut wurde (dieselbe Begründung, aus der die Maßtafel dem Kopf seine
Linie verweigert). Die KDE-HIG trennen Listeneinträge über Weißraum. Das ist
eine Änderung an `wireframes/`, also meine Arbeit — ich hole sie auf Zuruf des
PO nach, nicht im Review-Lauf. — *Erledigt, siehe Nachtrag.*

### Hinweis 1 — die Kopfschrift hängt am Plattformthema

`QFontDatabase::systemFont(SmallestReadableFont)` liefert im Plasma-Umfeld
Noto Sans 8,25 pt (Zeilenhöhe 15) gegen 9,75 pt (18) der Eintragsschrift — die
Rangfolge stimmt. **Ohne** das KDE-Plattformthema liefert dieselbe Funktion hier
DejaVu Sans 12 pt (Zeilenhöhe 19) und ist damit *größer* als die
Eintragsschrift; Kopf und Zeitstempel wirken dann schwerer als der Notiztext
(zu sehen im Gegenlauf unter `ohne-plasma-thema/`). Denkzettel ist eine
Plasma-Anwendung, der Fall tritt im Zielumfeld nicht ein — er betrifft aber
jeden künftigen Bildlauf nach DoD 3.

**Empfehlung:** Bildläufe künftig mit `QT_QPA_PLATFORMTHEME=kde` fahren, sonst
beurteilt man Schriftgrößen an einer Ersatzschrift. Das gehört in die
Prüfanleitung, nicht in den Code.

### Hinweis 2 — die Gliederung im Ganzen liest sich als Posteingang

Der Kundenwunsch „wie ein Posteingang“ ist eingelöst. Was das Bild zeigt und
keine Zusicherung sagt:

- Die Gruppentrennung **trägt ohne Linie und ohne Balken**. Der Kopf steht
  näher an der Gruppe, die er benennt (15 px darunter), als an der darüber
  (23 px) — die Zuordnung ergibt sich aus dem Abstand, wie sie soll.
- Das Verhältnis Kopf zu Eintrag ist ungewöhnlich, aber tragfähig: Der Kopf ist
  **kleiner** als der Betreff, den er überschreibt, und gewinnt seine Geltung
  aus Halbfett und Luft. Im Bild funktioniert das. Sollte der Kunde die
  Gliederung bei größerem Bestand als zu schwach empfinden, ist der Hebel die
  Schriftstärke oder eine gedämpfte Farbe des Kopfes — **nicht** eine Linie und
  nicht ein farbiges Band.
- Die **zweizeilige Vorschau erfüllt bei 220 px ihren Zweck**: Betreff und
  Vorschau tragen dort noch je rund 25 Zeichen, die Köpfe passen unverkürzt
  (`s5a-a-schmal-220px.png`). Ein Einzeiler wie
  „journalctl -u whisperd --since today“ bricht bei dieser Breite in Betreff und
  Vorschau um, wie Fall 2 es vorsieht.
- Der Einzeiler mit leerer Vorschauzeile hinterlässt sichtbar Luft. Das ist der
  Preis der einheitlichen Zeilenhöhe, die die Zeichnung ausdrücklich verlangt,
  und im Bild kein Störfaktor.

## Die vierzehn Akzeptanzkriterien

| AK | Gegenstand | Verdikt | Beleg |
|---|---|---|---|
| 1 | Fünf Gruppen, Reihenfolge, Kalenderwoche, erste passende gewinnt | ok | `s5a-a-…`, `s5a-b-sonderfaelle-montag-…` |
| 2 | Leere Gruppe ohne Kopf, Einzelgruppe mit Kopf | ok | Montagsbild („Diese Woche“ fehlt) |
| 3 | Zeitstempel folgt der Gruppe, Detail behält die volle Form | ok | alle Bilder der Szenen A und B |
| 4 | Betreff/Vorschau, drei Eingaben, gleiche Höhe beim Einzeiler | ok | Montagsbild, Zeilenhöhe 72 px durchgängig |
| 5 | Farben, kein Fettdruck | **ok** | Pixelmessung, Frage 2 |
| 6 | Kopf nicht auswählbar/fokussierbar, Pfeiltasten lückenlos | ok | C1 (Zeile 8 → 10), Löschbild |
| 7 | Kopf der neuen Gruppe im Bild, Auswahl vollständig sichtbar | **fail** | `s5a-c4-…`, Frage 1 |
| 8 | Bezugszeitpunkt bis in die Widget-Ebene | ok | diese Bildstrecke ist der Beleg — ohne ihn gäbe es das Montagsbild nicht |
| 9 | Löschen und Undo an der Gruppengrenze | ok | `s5a-b-geloescht-…`, `s5a-b-undo-…` |
| 10 | Neu gruppieren bei Neuaufbau und Aktivierung, kein Zeitgeber | ok | `changeEvent`/`regroupList`, Test `regroupsWhenTheWindowIsActivated` |
| 11 | Leerzustände unverändert | ok | `s5a-d-leere-bibliothek-…` |
| 12 | Geometrie nach 3a, zwei Fenstergrößen | teilweise | Kopfabstände, Textkante, Eintragshöhe ok; Kopfzeilenhöhe siehe Befund 1 → **warn** |
| 13 | Bewusste Auslassungen nicht gebaut | ok | alle Bilder |
| 14 | Gliederung in Modell und Delegate, nicht im Datenweg | ok | `NoteListModel::buildRows`, `Store::notes()` unverändert |

## Offene Punkte

1. **AK 7 räumen** — Auflage aus Frage 1, Weg 1 (Code, empfohlen) oder Weg 2
   (Zeichnung und AK zurückschneiden). PO-Entscheidung.
2. **Befund 1** — Kopfzeilenmaß in 2c korrigieren (48 px), damit die Zahl nicht
   in die nächste Story weiterwandert.
3. **Befund 2** — Haarlinien aus den Listenspalten in 2b/3a/3b entfernen; UX
   führt die Änderung auf Zuruf aus.
4. **Hinweis 1** — `QT_QPA_PLATFORMTHEME=kde` in die Prüfanleitung zu DoD 3
   aufnehmen.

Im Prüflauf selbst keine Änderung an Quellcode, SPEC oder Wireframes; kein
Commit. Was danach auf Weisung des PO an der Zeichnung geschah, steht im
Nachtrag.

## Nachtrag (01.08.2026, nach der PO-Entscheidung)

Der PO hat entschieden und beauftragt. Stand der vier Punkte:

1. **AK 7 — Weg 1 wird gebaut.** Der Entwickler ist mit der Passt-Bedingung und
   einem Testfall für die kleine Gruppe beauftragt. Der `fail` bleibt bis zur
   Heilung stehen; er ist damit adressiert, nicht erledigt. Die Trefferliste der
   Suche erbt die Heilung ohne eigene Arbeit — sie geht durch dasselbe Modell
   (siehe [Zugabe S6](sprint-03-s6-trefferliste.md)).
2. **Befund 1 — erledigt.** In der Festlegungstafel zu 2b/2c steht jetzt
   „rund 48 px" statt „unter 40 px", zusammen mit der Begründung: Die Zahl kam
   aus der Zeichnung, nicht vom Kunden, und war nie erfüllt; was sie schützen
   sollte — dass die Kopfzeile nicht mitwächst —, prüft derselbe Satz
   unverändert weiter. Die Begründung steht ausdrücklich in der Tafel, damit die
   Zielverschiebung später nicht für Nachgeben gehalten wird.
3. **Befund 2 — erledigt.** Die Haarlinien unter den Einträgen sind aus den
   Listenspalten von **2a, 2b, 2c, 3a und 3b** entfernt (22 Stellen). Der
   Auftrag nannte 2b und 3a; ich bin auf alle geltenden Zeichnungen derselben
   Liste gegangen, weil die Zeichnung sonst in sich widersprüchlich wäre — 3b
   hätte Linien gezeigt, 3a keine. **Nicht angefasst** ist der Entwurf der
   ersten Runde (1b): Er ist überholter Entwurfsstand und belegt, was damals
   gezeichnet wurde. Stehen bleiben außerdem die Linien, die keine
   Eintragstrennung sind — Beschriftungsbalken der Ausschnitts-Karten,
   Fensterrahmen, die Trennung unter der Suchleiste und die gestrichelte
   Scroll-Andeutung in 3b Fall 4.
   Die Festlegung ist dabei **positiv** formuliert worden: Der Maßpunkt in 3a
   heißt jetzt „Liste ohne Trennlinien, Kopf ohne Fläche" und verbietet die
   Linie ausdrücklich auch zwischen den Einträgen. Vorher stand dort nur, dass
   der *Kopf* keine trägt — woraus die alte Lesart überhaupt erst entstand.
4. **Hinweis 1 — beim PO.** Geht als Prozess-Erkenntnis in die Bildlauf-Regel
   von Beschluss B3 und in die Retro; dem Entwickler für seine Heilungsbilder
   bereits mitgegeben.

Geändert wurde dafür ausschließlich
`wireframes/Denkzettel Wireframes.dc.html` (25 Zeilen). Kein Produktivcode,
kein Commit — der PO nimmt die Änderung zusammen mit der Heilung auf.
