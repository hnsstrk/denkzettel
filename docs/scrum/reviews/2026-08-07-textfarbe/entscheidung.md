# Entscheidung zur Textfarbe des Notiztextes (#100, AK 3 / Prüfsatz F3)

**Modus:** Gestaltung — der Kunde hat die Frage am 07.08.2026 ausdrücklich der
UI/UX-Rolle übertragen („Der UI/UX-Agent soll das entscheiden. Neutral und im
Sinne der UI/UX"). Die Entscheidung ist verbindlich; das Nachziehen von
Zeichnung, SPEC und Kriterien macht der PO.

**Datum:** 07.08.2026, Ganymed · **Quellstand:** `main` @ `88129ba` ·
**Werkzeuge** (B17): ksvg 6.28.0, qt6-base 6.11.1, plasma-desktop 6.7.4,
kcolorscheme 6.28.0. Acht installierte Desktop-Themes, 19 Farbschemata
(18 unter `/usr/share/color-schemes`, `Ant-Dark` im Benutzerverzeichnis).

**Belege:** `docs/scrum/vorberichte/100-eingabefeld/messungen-ux/`,
Sonden daneben in `sonden-ux/`, wiederholbar über
`bash docs/scrum/vorberichte/100-eingabefeld/pruefen-ux.sh`.

---

## Die Entscheidung: **A — F3 fällt**

Der Notiztext bleibt auf der **Fensterrolle**. Für Desktop-Themes mit eigener
`colors`-Datei bleibt es bei `[Colors:Window] ForegroundNormal` (Weg aus #85),
für die übrigen bei `WindowText` aus dem Farbschema. **AK 3 entfällt**, der
Prüfsatz F3 der Zeichnung 4b wird zurückgenommen, der Prüfsatz „Notiztext auf
der Fensterrolle" tritt wieder in Kraft.

**Was davon unberührt bleibt:** die Kundenentscheidung vom 06.08.2026 zum Feld
selbst. Fläche und Kante kommen weiter aus `widgets/lineedit`, Vorsatz `base`,
aus derselben `KSvg::ImageSet` wie die Hülle. AK 1, AK 2, AK 4, AK 5 und AK 6
sind von dieser Entscheidung nicht betroffen.

### Die tragende Begründung

Der Gewinn, mit dem AK 3 begründet wurde — „12,72 → 17,68 : 1 unter `default`" —
gehört gemessen der **Feldfläche** und nicht der Textfarbe: unter `default`
erreichen Fenster- und Ansichtsrolle auf der Feldfläche **beide** 17,68 : 1,
weil die beiden Farben dort bildpunktgleich sind (UX3). So liegt es unter 14 der
19 Farbschemata (UX4) und unter drei der vier Themes, die eine eigene
`colors`-Datei mitbringen; für die vier übrigen entscheidet das Schema. Wo die beiden Farben
sich unterscheiden, fällt die Messung überwiegend gegen die Ansichtsrolle aus:
in beiden Lagen viermal schlechter, einmal besser, vierzehnmal gleich (UX5).
Der einzige Wert unter 4,5 : 1 in der gesamten Messung entsteht **erst durch**
AK 3 — dort, wo die Feldgrafik nichts zeichnet und die Ansichtsfarbe auf der
Fensterfläche landet (4,74 : 1 → 4,22 : 1 unter `KritaNeutral`). Für Sprint 9
hat der Kunde „Lesbarkeit geht vor" festgelegt, und #84 klagt genau diese
Schwelle ein; eine Umstellung, die keine Zahl hebt und eine Zahl unter die
Schwelle drückt, ist damit nicht zu vereinbaren.

---

## Die Sachlage, an der die Entscheidung hängt

Der Notiztext steht nach der Umsetzung von #100 nicht überall auf demselben
Grund. Gemessen (UX1, UX6) zerfallen die acht installierten Themes in zwei
Lagen:

| Theme | Deckung Hülle | Deckung Feld | Grund unter dem Text | Farbweg |
|---|---|---|---|---|
| `default` | 216 | **255** | Ansichtsfläche | Farbschema |
| `breeze-dark` | 216 | **255** | Ansichtsfläche | Theme-`colors` |
| `breeze-light` | 216 | **255** | Ansichtsfläche | Theme-`colors` |
| `CachyOS-Nord-round` | 255 | 15 | Fensterfläche | Farbschema |
| `Iridescent-round` | 51 | 15 | Fensterfläche, durchscheinend | Farbschema |
| `cachyos-emerald` | 7 | 15 | Bildschirminhalt | Farbschema |
| `cachyos-emerald-color` | 7 | 15 | Bildschirminhalt | Theme-`colors` |
| `cachyos-emerald-light` | 7 | 15 | Bildschirminhalt | Theme-`colors` |

Daraus folgt die Form der Frage: **Eine feste Rollenwahl trifft in einer der
beiden Lagen die Paarung, die das Farbschema vorsieht, und in der anderen eine
gemischte.** Das ist keine Eigenheit dieses Projekts. Der installierte
KColorScheme-Header sagt es zu seinen Farbsätzen ausdrücklich:

> „Color sets define a color »environment«, suitable for drawing all parts of a
> given region. **Colors from different sets should not be combined.**"
> — `/usr/include/KF6/KColorScheme/kcolorscheme.h`, Zeile 64–65

Und zum Satz `View`: „Views; for example, frames, **input fields**, etc."
(Zeile 67). Nach dieser Lehre gehört auf die Feldfläche die Ansichtsfarbe und
auf die Fensterfläche die Fensterfarbe — der Grundsatz trägt also **beide**
Antworten, je nach Lage, und keine der beiden über beide Lagen. Er entscheidet
die Frage deshalb nicht; er erklärt, warum die Zahlen überhaupt schwanken
können. Entschieden wird an der Messung.

---

## Die Messungen

### UX2 — die vier Paarungen über 19 Farbschemata

Reproduziert `messungen-b/m3-schriftgrund-18-schemata.txt` Ziffer für Ziffer
und ergänzt die dort fehlende vierte Spalte (Fensterrolle auf der
Ansichtsfläche) je Schema.

| Paarung | schlechtester Fall | unter 4,5 : 1 |
|---|---|---|
| Fensterrolle auf Fensterfläche (rein) | 4,74 : 1 | **0** von 19 |
| Ansichtsrolle auf Ansichtsfläche (rein) | 5,70 : 1 | **0** von 19 |
| Fensterrolle auf Ansichtsfläche (gemischt) | 5,70 : 1 | **0** von 19 |
| Ansichtsrolle auf Fensterfläche (gemischt) | **4,22 : 1** | **1** von 19 |

### UX5 — A gegen B, getrennt nach Lage

| Lage | A besser | B besser | gleich | A schlechtest | B schlechtest | A < 4,5 | B < 4,5 |
|---|---|---|---|---|---|---|---|
| Lage 1 (Feld deckt) | 4 | 1 | 14 | 5,70 : 1 | 5,70 : 1 | 0 | 0 |
| Lage 2 (Feld deckt nicht) | 4 | 1 | 14 | 4,74 : 1 | **4,22 : 1** | 0 | **1** |

Die fünf Schemata, unter denen A und B sich überhaupt unterscheiden:

| Schema | Lage 1: A | B | Lage 2: A | B |
|---|---|---|---|---|
| `CachyOSNord` | 8,80 | 6,88 | 8,61 | 6,73 |
| `CachyOSNordLightly` | 8,80 | 6,88 | 7,93 | 6,20 |
| `EmeraldDark` | 7,98 | 7,20 | 7,81 | 7,04 |
| `KritaBright` | 13,05 | **16,14** | 9,68 | **11,98** |
| `KritaNeutral` | 8,06 | 7,18 | 4,74 | **4,22** |

### UX3 und UX4 — der zweite Farbweg, den beide Vorprüfungen nicht gerechnet haben

Seit #85 kommt die Schrift aus der `colors`-Datei des Desktop-Themes, sobald
eine da ist. Vier der acht Themes bringen eine mit; für sie gilt die
Schema-Rechnung nicht. Gemessen und in den Dateien nachgelesen:

| Theme | `[Colors:Window] ForegroundNormal` | `[Colors:View] ForegroundNormal` |
|---|---|---|
| `breeze-dark` | 252,252,252 | 252,252,252 |
| `breeze-light` | 35,38,41 | 35,38,41 |
| `cachyos-emerald-light` | 35,38,41 | 35,38,41 |
| `cachyos-emerald-color` | 0,199,144 | 2,189,136 |

**AK 3 ist auf diesem Weg gegenstandslos.** Unter drei der vier Themes sind die
Farben gleich; unter dem vierten liegen sie um zwei bis zehn Stufen je Kanal
auseinander, und die Fensterrolle ist dort die hellere (UX3: 2,10 : 1 gegen
1,90 : 1 gegen denselben Grund).

Über die Farbschemata dasselbe Bild: **14 von 19 setzen für beide Farbsätze
denselben Vordergrund** (UX4).

---

## Warum nicht B

B verbessert keine Zahl an der Stelle, an der eine Verbesserung zählt, und
verschlechtert eine an der Stelle, an der eine Verschlechterung zählt.

- **Der schlechteste Fall in Lage 1 ist bei A und B identisch** (5,70 : 1).
  Der Spitzenwert 17,68 : 1, den das Issue für B anführt, wird von A ebenso
  erreicht (UX3, Zeile `default`).
- **In Lage 2 fällt B unter die Schwelle**, die das Projekt selbst führt.
  Der Fall ist benennbar: Theme `CachyOS-Nord-round` (das einzige Theme dieser
  Lage mit deckender Hülle und Schemaweg) mit Schema `KritaNeutral`, 4,22 : 1.
- **Der Zugewinn von B beschränkt sich auf ein Schema** (`KritaBright`), das
  auch ohne ihn komfortabel liegt (9,68 : 1 beziehungsweise 13,05 : 1).

Die Begründung, die der Kundenentscheidung vom 06.08.2026 beilag, hat den
Gewinn der neuen Feldfläche der Textfarbe zugerechnet. Das ist der Punkt, an
dem diese Entscheidung die Voraussetzung ihrer eigenen Vorgängerin geprüft hat.

## Warum nicht C

C hält den Paar-Grundsatz in beiden Lagen ein und wäre insofern die
lehrbuchtreue Antwort. Gemessen kauft C dafür nichts:

| | Lage 1 | Lage 2 |
|---|---|---|
| A liefert | 5,70 : 1 (schlechtest) | 4,74 : 1 (schlechtest) |
| C liefert | 5,70 : 1 (schlechtest) | 4,74 : 1 (schlechtest) |

**Die schlechtesten Fälle sind identisch**, und in den fünf unterscheidbaren
Schemata von Lage 1 liegt C viermal unter A (Tabelle oben, Spalten „Lage 1").
Dem steht eine Fallunterscheidung gegenüber, die der Nutzer nicht vorhersagen
kann und deren Schwelle niemand begründen könnte: Heute stehen 255 gegen 15,
ein Theme mit Deckung 130 wäre unentscheidbar, und die Textfarbe könnte mit
einem Paket-Update springen. Eine Verhaltensweise, die sich der Erklärbarkeit
entzieht, muss dafür etwas leisten.

Dazu ein Fehler in der Kopplung, den C in seiner vorgelegten Fassung enthält:
C macht die Textfarbe an der **Sichtbarkeit des Feldes** fest. Maßgeblich wäre
die Deckung der **Fläche unter dem Text**. Beides fällt bei den heute
installierten Themes zufällig zusammen und ist der Sache nach zweierlei — die
fünf schwachen Themes zeichnen im Fokus eine voll deckende Kante bei
unverändert durchsichtiger Fläche (UX1).

## Die vierte Möglichkeit, die ich geprüft und verworfen habe

Bevor ich A gewählt habe, habe ich die Frage aufzulösen versucht, statt sie zu
beantworten: **Wenn der Grund unter dem Text überall die Ansichtsfläche wäre,
gäbe es die Streitfrage nicht.** Zwei Wege, beide gemessen und beide verworfen:

1. **Vorsatz `focus` statt `base`.** Die Vermutung war, dass die fünf schwachen
   Themes ihr Feld erst im Fokus füllen — und das Erfassungsfenster zeigt
   seinen Textbereich ausschließlich im Fokus. Gemessen (UX1, UX7) füllen sie
   auch dort die Fläche nicht: Die Deckung der Mitte bleibt bei 15, allein die
   Kante wird deckend. Unter `default`, `breeze-dark` und `breeze-light`
   zeichnet `focus` allein einen 1 px breiten Rahmen und keine Fläche; ein
   **Ersatz** des Vorsatzes ließe das Feld dort verschwinden.

   *Berichtigt am 07.08.2026 im selben Zug:* Der erste Stand dieses Absatzes
   sagte, `focus` zeichne unter den drei Themes „gar nichts". Das war die
   Auskunft einer Punktmessung, die x=1 abgetastet hat; die Kante liegt dort
   bei x=0. Die Flächenmessung UX7 hat es aufgedeckt, UX8 legt die Geometrie
   offen. Der Schluss dieses Absatzes bleibt davon unberührt — ein Ersatz von
   `base` durch `focus` nähme dem Feld seine Fläche. Was sich ändert, ist die
   Lage des Fundes am Ende dieser Datei: Die Schicht ist **zusätzlich** zu
   `base` zu zeichnen, und dafür genügt unter allen acht Themes der eine
   Vorsatz `focus`. Ausgeführt in
   `docs/scrum/reviews/2026-08-07-textfarbe/fokuszustand.md`.
2. **Die Ansichtsfläche selbst füllen, wo die Theme-Grafik sie nicht trägt.**
   Das überstimmt eine Gestaltungsentscheidung des Theme-Autors — die fünf
   Themes bringen eine **eigene** `lineedit`-Grafik mit und zeichnen dort
   absichtlich ein flaches Feld. Es widerspricht der Linie, die das Projekt mit
   #83 und #85 eingeschlagen hat, und dem Kundenmaßstab „das Fenster soll wie
   ein Plasma-Fenster aussehen".

## Die Gegenrede, die ich nicht unterschlage

**Erstens: A verletzt in Lage 1 den Paar-Grundsatz, und zwar auf der
Einstellung des Kunden.** Unter `default` steht dann die Fensterfarbe auf einer
Ansichtsfläche — genau die Kombination, die KColorScheme abrät. Ich halte das
für tragbar, weil die Messung über 19 Schemata keinen Fall zeigt, in dem diese
Kombination unter die Schwelle fällt, und weil jede Wahl den Grundsatz in einer
der beiden Lagen verletzt. Wer den Grundsatz höher gewichtet als die gemessenen
Zahlen, kommt zu C.

**Zweitens: Erwartungskonformität.** Der Notiztext erscheint in der Bibliothek
mit `QPalette::Text` auf `Base` (`src/ui/notelistdelegate.cpp:129`), also in der
Ansichtspaarung. Unter A trägt derselbe Text im Erfassungsfenster eine andere
Rolle. Sichtbar wird das unter fünf der 19 Schemata; unter `CachyOSNord` sind
es 102,194,242 gegen 71,173,218, ein wahrnehmbarer Unterschied. Die beiden
Fenster stehen allerdings nie nebeneinander — das Erfassungsfenster schließt
sich beim Speichern —, und der Kunde hat für Sprint 9 die Lesbarkeit vor die
Einheitlichkeit gestellt.

**Drittens: A hält an einer Rolle fest, deren ursprüngliche Begründung
gefallen ist.** Die Zahl 4,74 : 1 stand für den Text auf der Fensterfläche, und
diese Fläche gibt es unter drei Themes nicht mehr. Die Entscheidung ist deshalb
neu gerechnet und stützt sich nicht auf die alte Begründung: Sie gilt, weil A
in beiden Lagen gemessen mindestens so gut liegt wie B.

## Die Grenze dieser Entscheidung

Unter `Iridescent-round` (Hüllendeckung 51) und den drei `cachyos-emerald`-Themes
(Deckung 7) trägt den Text weder die Fenster- noch die Ansichtsfläche, sondern
das, was hinter dem Fenster steht. Für diese Themes sichert SPEC 3.1 bereits
heute keine Lesbarkeit zu, und keine der drei Möglichkeiten ändert daran etwas.
Die Zahlen dieser Entscheidung gelten für Themes mit tragender Hülle
(`default`, `breeze-dark`, `breeze-light`, `CachyOS-Nord-round`).

---

## Was nachzuziehen ist (Sache des PO)

1. **Zeichnung 4b** — Prüfsatz **F3** zurücknehmen, mit datiertem Vermerk statt
   Überschreiben (B17). Der Prüfsatz „Notiztext auf der Fensterrolle" tritt
   wieder in Kraft; sein Vermerk „Ersetzt am 06.08.2026 durch F3 unten" braucht
   einen Nachtrag. Ebenso der Absatz zum Notiztext in der Farbrollen-Liste
   („Grund gewechselt am 06.08.2026"): Die dort genannte Verbesserung
   12,72 → 17,68 : 1 gehört der Feldfläche und tritt unter A genauso ein.
   *Das Nachziehen ist UX-Fläche; ich habe es unterlassen, weil der Auftrag
   ausdrücklich keine Änderung an der Zeichnung vorsieht. Es ist gern mein
   nächster Schritt, wenn der PO ihn beauftragt.*
2. **Issue #100** — **AK 3 streichen**. Die Nummerierung der übrigen Kriterien
   bleibt am besten stehen, damit Bericht und Zeichnung dieselbe Sprache
   sprechen; ein Vermerk „AK 3 entfällt (Entscheidung vom 07.08.2026)" genügt.
   Der Kommentar vom 06.08.2026 („Die Farbwahl aus #85 wandert mit") braucht
   einen Nachtrag; die dort gestellte Frage, ob AK 3 ein eigenes Issue wird,
   ist damit gegenstandslos.
3. **SPEC 3.1** — der Spiegelstrich zur Schrift (`:241–252`) bleibt inhaltlich
   stehen. Seine Klammerbegründung „über 18 Schemata schlechtestens 4,74 : 1
   gegen 4,22 : 1" ist auf den neuen Grund zu ziehen: Beide Zahlen sind gegen
   die **Fensterfläche** gerechnet, und mit dem Feld gilt die Rechnung nach
   Lage. Vorschlag für den Kern des Satzes: *Der Notiztext trägt die
   Fensterrolle. Er steht je nach Theme auf der Feldfläche oder auf der
   Fensterfläche; die Fensterrolle bleibt in beiden Lagen über 4,5 : 1
   (schlechtestens 5,70 : 1 beziehungsweise 4,74 : 1 über 19 Schemata,
   07.08.2026), die Ansichtsrolle fällt in der zweiten Lage auf 4,22 : 1.*
   Das ist eine entdeckte Bedingung im Sinne von DoD 4 / B9.
4. **Vorprüfbericht** — Feld 6.1 der Messung A („Antwort: ja, AK 3 bleibt")
   und Abschnitt 3 der Messung B tragen die Frage noch als offen. Beide sind
   mit dieser Entscheidung erledigt; die Größenklasse `size:m` bleibt davon
   unberührt, weil AK 3 dort ausdrücklich als nicht klassenhebend gemessen ist.
5. **`src/capture/capturewindow.cpp:496–499`** — der Kommentarblock begründet
   die Farbwahl mit „4,74 : 1 gegen 4,22 : 1 auf der Fensterfläche". Die Wahl
   bleibt richtig, die Begründung braucht den zweiten Grund. Meldung an den PO,
   keine Änderung durch mich.

## Ein Fund außerhalb der Frage — melden, nicht heilen

Beim Messen der Vorsätze (UX1) ist aufgefallen: **Jedes der acht installierten
Themes bringt einen Fokuszustand mit sichtbarer Kante mit.**

| Theme | Vorsatz | Kante | Farbe |
|---|---|---|---|
| alle acht | `focus` | 1 px (`default`, Breeze) bzw. 2 px | `#3daee9` bzw. `#1a73e8` |
| `default`, `breeze-dark`, `breeze-light` zusätzlich | `focusframe` | 3 px, 2 px nach außen gezogen | `#3dade7` |

*Berichtigt am 07.08.2026:* Der erste Stand dieser Tabelle las die beiden
Vorsätze als Alternative je Theme und legte damit eine Fallunterscheidung nahe.
Gemessen führen **alle acht** Themes `focus`; `focusframe` kommt unter dreien
hinzu, und Plasma nimmt es allein beim Tastaturfokus
(`TextField.qml:223`). Für diesen Bau genügt `focus`.

Der Textbereich des Erfassungsfensters hat immer den Fokus — das Fenster geht
nur zum Tippen auf. Ein Bau, der über `base` zusätzlich den Fokuszustand
zeichnet (Plasmas eigene Schichtung: `hover` und `focus` decken bei `default`
in der Mitte 0, sie sind reine Überlagerungen), bekäme unter **allen acht**
Themes ein erkennbares Feld. Das ist genau der Kundenbefund aus #100, und es
würde die Grenze aus AK 6 für die fünf schwachen Themes weitgehend auflösen.

Für die hier entschiedene Frage ändert das nichts: Der Fokuszustand zeichnet
eine Kante und keine Fläche, der Grund unter dem Text bleibt in Lage 2 die
Fensterfläche. Ob daraus ein Kriterium in #100 oder ein eigenes Issue wird,
entscheidet der PO mit dem Kunden.

## Zur Belegform

Die Messungen laufen offscreen, und das genügt hier. Keine Zahl dieser
Entscheidung behauptet etwas über Hülle, Rundung, Kontur, Schatten oder
Dekoration (B21); gemessen sind Farbrollen aus `.colors`-Dateien und
Farbschemata sowie die Deckung zweier Theme-Grafiken, die KSvg offscreen aus
denselben SVG-Quellen zeichnet wie in der Sitzung. Was der Compositor zum Grund
unter dem Text beiträgt, ist oben als Grenze benannt und ausdrücklich nicht
zugesichert. Ein Sitzungsbild wird fällig, sobald die Umsetzung von #100 im
UI-Review steht — dort verlangen AK 2 und AK 4 es ohnehin.
