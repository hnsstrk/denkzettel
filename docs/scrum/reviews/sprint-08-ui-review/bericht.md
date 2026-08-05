# UI-Review Sprint 8 — #85 „Capture: Lesbarkeit unter fremden Desktop-Themes"

**Modus:** UI-Review (DoD 3) · **Rolle:** UI/UX · **Datum:** 05.08.2026 ·
**Geprüfter Stand:** `main`, `1795c87` (beide Stränge zusammengeführt).

**#61** (Versionsanzeige) und **#76** (Linterbefunde) sind nach Festlegung des
PO keine UI-Stories dieses Sprints und werden hier nicht unter DoD 3 geführt.

**Prüfstand** (eine Aussage gilt für einen Stand, B17): Wayland-Sitzung,
angemeldet und **entsperrt** (`LockedHint=no`, im Protokoll jeder Aufnahme),
Bildschirm 3840×2160 Gerätebildpunkte, **Fensterverhältnis 1,6**, Farbschema
des Kunden (Breeze Dark: `WindowText` 252,252,252 · `PlaceholderText`
161,169,177). Acht installierte Desktop-Themes. Der Auswahlpfad war in allen
Läufen **durchscheinend** — die Sitzung weichzeichnet.

## Wie dieser Review geführt wurde

Alle Bilder unter `bilder/` sind in diesem Review entstanden, aus einem
**eigenen** Bau des Sprint-Standes außerhalb des Repositoriums; `ctest` auf
diesem Bau: **9 von 9 grün**, der Bau warnungsfrei. Die Bilder der Stränge sind
gelesen und als Material verwendet worden; sie ersetzen die eigenen nicht.

| Prüfmittel | Einstellung |
|---|---|
| Zustände je Theme, angemeldete Sitzung | `sonden/sitzungsbeleg.cpp` des Strangs A, gegen den **eigenen** Bau gelinkt, gefahren von `bilder-erzeugen.sh` |
| Theme- und Schemawechsel am stehenden Fenster | eigene Sonde `sonden/wechselbeleg.cpp` |
| Schrift **im Bild** statt an der Palette | eigenes Messprogramm `messungen/schriftimbild.py` |
| Zeichnung nach der Heilung | Rendern in Chrome, Ausschnitte in `bilder/zeichnung-*.png` |

**Warum der ganze Review in der angemeldeten Sitzung stattfindet (B21):** Für
#85 geht es um Farbe **auf** der Theme-Grafik. Offscreen weichen Palette und
KSvg-Grundlage gemessen auseinander (`capturetest` sieht `default` mit hellem
Grund unter dunkler Schrift, 1,11 : 1 — ein Zustand, den es nirgends gibt), und
ohne Compositor gilt der Auswahlpfad `opaque` statt der durchscheinenden
Fassung. Ein offscreen erzeugtes Bild belegt für diese Story nichts.

**`QT_SCALE_FACTOR` ist ausdrücklich nicht gesetzt.** Das Verhältnis 1,6 der
Kundenmaschine kommt in der Sitzung vom Compositor; ein zusätzlicher Faktor
multiplizierte sich darauf. Jede Sondenausgabe schreibt das am Fenster
gemessene Verhältnis mit — in allen Läufen **1,6**.

**Zwei Voraussetzungen des Messwegs, vor dem Messen geprüft:** Die Sitzung war
entsperrt, und der Prüfgrund ist ein gewöhnliches Fenster in Bildschirmgröße
und kein Vollbildfenster. Die Selbstprüfung der Sonde (Abbruch mit Rückgabe 3,
wenn die erste Aufnahme den eigenen Grund nicht zeigt) hat in keinem der
fünfzehn Läufe angeschlagen.

---

## 1. Was gemessen wurde, Theme für Theme

Eigene Zahlen, je Zahl mit Grund und Auswahlpfad. Der **Kontrast im Bild**
stammt aus `schriftimbild.py`: Fläche aus dem Leerbild, Schriftfarbe aus dem
Kern der Glyphen. Er liegt durchweg wenige Hundertstel unter der Zahl aus der
Palette — das ist der Kantenausgleich der Schrift und keine Abweichung.

| Theme | `colors`? | Grund | Notiztext (Palette / Bild) | gedämpft (Palette / Bild) | *vor #85* |
|---|---|---|---|---|---|
| `default` | nein | weiß | 9,42 / 9,34 | 4,06 / 3,90 | unverändert |
| `default` | nein | schwarz | 16,33 / 16,17 | 7,04 / 6,62 | unverändert |
| `breeze-dark` | ja | weiß | 9,42 / 9,34 | 4,06 / 3,90 | gleiche Farbe |
| `breeze-light` | ja | weiß | **13,59 / 13,33** | 3,76 / 3,49 | 1,09 : 1 |
| `breeze-light` | ja | schwarz | **9,38 / 9,27** | 2,60 / 2,32 | 1,58 : 1 |
| `CachyOS-Nord-round` | nein | weiß | 15,37 / 15,19 | 6,63 / 6,25 | unverändert |
| `CachyOS-Nord-round` | nein | schwarz | 15,37 / 15,19 | 6,63 / 6,25 | unverändert |
| `Iridescent-round` | nein | weiß | **1,57 / 1,56** | 1,48 / 1,47 | unverändert |
| `Iridescent-round` | nein | schwarz | 20,35 / 19,95 | 8,77 / 8,09 | unverändert |
| `cachyos-emerald` | nein | weiß | **1,04 / im Bild nicht auffindbar** | 2,24 / 2,12 | unverändert |
| `cachyos-emerald` | nein | schwarz | 20,35 / 19,95 | 8,77 / 8,09 | unverändert |
| `cachyos-emerald-color` | ja | weiß | **2,07 / 2,06** | 5,10 / 4,64 | 1,04 : 1 |
| `cachyos-emerald-color` | ja | schwarz | 9,51 / 9,37 | 3,85 / 3,62 | 20,35 : 1 |
| `cachyos-emerald-light` | ja | weiß | **14,32 / 13,95** | 3,96 / 3,66 | 1,04 : 1 |
| `cachyos-emerald-light` | ja | schwarz | **1,37 / 1,36** | 4,96 / 4,62 | 20,35 : 1 |

Vier Themes bringen eine eigene `colors`-Datei mit, vier nicht. Die Herkunft
der Farbe stimmt in allen fünfzehn Läufen mit der Kundenregel überein.

**Die eigene Messung im Bild ist der Zusatz dieses Reviews.** Die Belegsonde
des Strangs liest die Farbe aus der Palette und rechnet sie gegen die Fläche
aus der Aufnahme; belegt ist damit, welche Farbe das Fenster tragen **soll**.
`schriftimbild.py` kennt keine Palette und nimmt allein die Aufnahme. Beide
Wege kommen zum selben Ergebnis — das ist der Beleg dafür, dass das Fenster
mit der Farbe zeichnet, die es sich gibt.

---

## 2. Prüfpunkte aus der Zeichnung

Ein Punkt je gezeichnetem Bereich und je Festlegung der Tafeln 4a und 4b,
soweit #85 sie berührt. Geometrie und Raumaufteilung sind mit #83 in Sprint 7
über zwanzig Punkte belegt worden und von dieser Story nicht angefasst; die
Punkte 12 bis 14 halten sie am neuen Stand nach.

| # | Prüfpunkt (Fundstelle) | Befund | Verdikt |
|---|---|---|---|
| 1 | 4b: Notiztext trägt die Farbe des Themes, wo das Theme eine `colors`-Datei mitbringt | vier von vier Themes mit Datei; `breeze-light` 35,38,41, `cachyos-emerald-color` 0,199,144 — im Bild gemessen dieselben Werte | **pass** |
| 2 | 4b: sonst trägt er die Schemafarbe | vier von vier Themes ohne Datei zeigen 252,252,252 | **pass** |
| 3 | 4b: App-Name und Fußzeile folgen derselben Regel | `breeze-light` 112,125,138 aus dem Theme, `default` 161,169,177 aus dem Schema | **pass** |
| 4 | SPEC 3.1: die gedämpfte Klasse hat **drei** Stellen — App-Name, Fußzeile, Platzhaltertext | `bilder/fenster-breeze-light-weiss-leer.png`: alle drei tragen dieselbe Farbe; die Zeichnung nannte zwei, geheilt als **V3** | **pass** (Zeichnung geheilt) |
| 5 | 4b „Bekannte Grenze der Kleintexte": Mindestwert 4,5 : 1 | unter `breeze-light` erreicht **keine** Quelle den Wert (Theme 3,49 : 1, Schema 2,83 : 1, im Bild). AK 4 benennt die Grenze, die Behebung ist **#84** | **warn** (benannt) |
| 6 | 4b „Auswahl, Cursor, Scrollbalken — unverändert aus der Palette" | Die **Auswahl** folgt weiter dem Schema (`wechsel-5`, heller Text auf blauem Balken). Der **Cursor** nicht mehr: unter `breeze-light` im Bild 91,91,90 auf der Fläche 242,242,243, während das Schema 252,252,252 sagt. Zeichnung geheilt als **V5** | **warn** (Zeichnung geheilt) |
| 7 | 4a: „Die Farbe kommt nicht aus dem Theme, sondern aus der Palette", und der Vermerk vom 04.08.2026 dazu | Beides ist nach #85 an zwei Stellen überholt; der Vermerk sagt weiter „sechs der acht Themes schlechter lesbar" und „aus der Palette kommen weiterhin Text …". Geheilt als **V1** | **fail** (Zeichnung), behoben |
| 8 | 4b Prüfsatz „Notiztext auf der Fensterrolle" (`WindowText`, 4,74 : 1) | gilt nur noch für Themes ohne eigene `colors`-Datei; geheilt als **V6** | **warn** (Zeichnung geheilt) |
| 9 | AK 5: nach einem **Theme-Wechsel** am stehenden Fenster stimmt die Farbe | eigene Sonde, ein Fenster, hin und zurück: `default` → `breeze-light` → `default`, Bilder `wechsel-1`, `-2`, `-4` | **pass** |
| 10 | AK 7: nach einem **Schemawechsel** bleibt die Themefarbe | Schema auf Magenta (255,0,255) gesetzt, Bild `wechsel-3` zeigt die Themefarben unverändert; kein Magenta im Fenster | **pass** |
| 11 | Zustände: Normalfall und Leerzustand je Theme | je Lauf zwei Aufnahmen, 30 Bilder aus 15 Läufen. Einen Meldungszustand hat das Erfassungsfenster nicht | **pass** |
| 12 | 4b: Fensterbreite 600 logisch | 960 Gerätebildpunkte bei 1,6 in allen Läufen | **pass** |
| 13 | 4b: Innenabstand zuzüglich Themerand | Fensterhöhe 174 logisch unter `default`/`breeze-*`, 182 unter den 8-px-Themes — wie in Sprint 7 | **pass** |
| 14 | 4b: linke Textkante von App-Name, Notiztext und Fußzeile bündig | im eigenen Bild weiter versetzt (`breeze-light`: App-Name ab Spalte 33, Notiztext ab 40); bekannt und gebucht als **#81** | **warn** (Bestand) |
| 15 | Die Rangfolge der beiden Schriftklassen: der Notiztext ist die Hauptsache | Unter zwei Themes kehrt sie sich um — siehe Befund **P1** | **warn** |
| 16 | SPEC 3.2 Punkt 10: die Grenze ist vollständig und richtig benannt | Sie nennt drei Themes; gemessen sind es andere drei — siehe Befund **P2** | **fail** (SPEC/Begründung) |

---

## 3. Sieht das Ergebnis unter fremden Themes richtig aus?

Die Frage des PO ist nicht die nach der Zahl. Was ich an den eigenen Bildern
sehe:

**Wo die Story greift, sieht das Fenster richtig aus.** Unter `breeze-light`
steht dunkle Schrift auf heller Hülle, App-Name und Fußzeile stehen leise
darüber und darunter, die Gliederung stimmt
(`bilder/fenster-breeze-light-weiss.png`). Vor dieser Story stand hier fast
weiße Schrift auf fast weißer Fläche. Dasselbe unter
`cachyos-emerald-light` über hellem Grund.

**Drei Bilder zeigen etwas, das keine Kontrastzahl allein sagt.**

1. `fenster-cachyos-emerald-color-weiss.png` — der Notiztext steht im
   Smaragdgrün des Themes (0,199,144) und ist dünn; App-Name und Fußzeile
   darüber und darunter sind **kräftiger**. Der Nebentext liest sich besser als
   der Haupttext.
2. `fenster-cachyos-emerald-light-schwarz.png` — dasselbe umgekehrt: Die
   Fußzeile „Esc verwirft · Strg+Enter speichert" ist das Lesbarste im Fenster,
   während der eingetippte Text im Dunkeln verschwindet.
3. `fenster-cachyos-emerald-color-weiss-leer.png` gegen
   `…-weiss.png` — der Platzhaltertext „Gedanke festhalten …" ist deutlich
   besser zu lesen als der Text, der ihn ersetzt. Wer tippt, sieht seine
   Aufforderung verschwinden und etwas Blasseres an ihre Stelle treten.

Das ist kein Fehler der Umsetzung: Die Farben kommen genau dorther, wo die
Kundenentscheidung sie holt. Es ist die **Folge** der Entscheidung an einer
Stelle, die im Issue nicht steht, weil dort je Klasse eine eigene Zahl
verhandelt wurde und nie ihr Verhältnis. Es gehört dem PO (Befund P1).

**Und ein Bild zeigt, dass die Grenze weiter reicht als benannt.** Unter
`Iridescent-round` über hellem Grund steht der Notiztext bei 1,57 : 1 — vor wie
nach #85, denn das Theme bringt keine `colors`-Datei mit
(`bilder/fenster-Iridescent-round-weiss.png`). Dieses Theme steht in SPEC 3.2
Punkt 10 nicht (Befund P2).

---

## 4. Befunde für den Product Owner

Melden, nicht heilen: An Quellcode, SPEC und Tests ist für diesen Review nichts
geändert worden. Geändert ist allein `wireframes/` — meine Fläche, Abschnitt 5.

| # | Befund | Verdikt | Vorschlag |
|---|---|---|---|
| **P1** | **Unter zwei Themes kehrt sich die Rangfolge der beiden Schriftklassen um.** Der Notiztext ist die Hauptsache des Fensters; die gedämpfte Klasse soll leiser sein. Gemessen im Bild steht sie unter `cachyos-emerald-color` über hellem Grund bei **4,64 : 1** gegen **2,06 : 1** des Notiztextes, unter `cachyos-emerald-light` über dunklem bei **4,62 : 1** gegen **1,36 : 1**. Der Sonderfall daraus: Der **Platzhaltertext** ist besser lesbar als der Text, der ihn ersetzt. Das Issue verhandelt beide Klassen einzeln und ihr Verhältnis nirgends | **warn** | Als eigene Frage an den Kunden, nicht als Mangel dieser Story. Die Kundenregel („dieselbe Quelle wie die Fläche") entscheidet sie nicht, denn beide Farben kommen aus derselben Datei. Sachlich hängt sie an **#84** und am selben Impediment wie #83; ein Bild neben das andere gelegt ist die Frage in zwei Sekunden beantwortet |
| **P2** | **SPEC 3.2 Punkt 10 nennt drei Themes, und zwei davon sind die falschen.** Der Satz sagt, die Grenze treffe „die drei `cachyos-emerald`-Themes". Gemessen an den Dateien selbst (`/usr/share/plasma/desktoptheme/*/metadata.desktop`): Eine `[ContrastEffect]`-Gruppe tragen **`Iridescent-round`** (`intensity=0.45`), **`cachyos-emerald`** (0.40) und **`cachyos-emerald-color`** (0.40). **`cachyos-emerald-light` trägt keine.** Damit gilt zweierlei. Erstens fehlt `Iridescent-round` in der Aufzählung, obwohl es denselben Fall bildet — Notiztext 1,57 : 1 über hellem Grund, mein Bild. Zweitens **trägt die Begründung des Übergabeberichts für den einen Fall nicht, für den sie geschrieben wurde**: §4 dort sagt zum Absturz von 20,35 : 1 auf 1,37 : 1 unter `cachyos-emerald-light`, die Rechnung gehe auf, „wenn der Compositor den Grund hinter der Hülle abdunkelt — genau dafür trägt das Theme seine `[ContrastEffect]`-Gruppe". Dieses Theme fordert den Effekt gar nicht an; nach SPEC 3.2 Punkt 8 meldet Denkzettel dort nichts an. Auf einem KWin **mit** geladenem Kontrasteffekt bliebe der Fall also, wie er ist | **fail** (an SPEC und Begründung, nicht am Code) | Punkt 10 auf die drei Themes mit `[ContrastEffect]`-Gruppe umstellen und `cachyos-emerald-light` als **eigenen** Fall führen: durchscheinende Hülle, dunkle Themeschrift, kein angeforderter Effekt. Das ändert die Dringlichkeit des Impediments — es heilt diesen Fall nicht. DoD 4 (entdeckte Bedingungen ziehen die SPEC nach) |
| **P3** | **Der Cursor folgt nicht mehr dem Farbschema.** Wireframe 4b sagt „Auswahl, Cursor, Scrollbalken — unverändert aus der Palette; sie folgen dem Schema bereits heute." Gemessen unter `breeze-light` bei dunklem Schema: der Cursor steht im Bild bei 91,91,90 auf der Fläche 242,242,243. Die Schemafarbe wäre 252,252,252 und auf dieser Fläche unsichtbar. Für das Fenster ist die neue Lage die bessere; die Zusicherung der Zeichnung beschreibt sie nicht mehr | **warn** | In der Zeichnung als **V5** geheilt. Ob SPEC 3.1 den Cursor eigens nennt, ist eine Entscheidung des PO — heute steht er dort nicht |
| **P4** | **`Iridescent-round` ist im Übergabebericht des Strangs nicht in der Sitzung gemessen.** Die Sitzungstabelle (P5 dort) führt sechs Läufe über fünf Themes; `Iridescent-round` und `cachyos-emerald` fehlen. Beide habe ich nachgeholt, und einer der beiden trägt den Befund P2 | **warn** | Zur Kenntnis. AK 3 ist über die deckende Messung (P3 dort, acht Themes) erfüllt; die Lücke betrifft die Sitzungsmessung, die AK 2 verlangt |

**Was ausdrücklich kein Befund ist**, weil die Story es nicht zusichert und der
PO es so festgelegt hat: dass die gedämpfte Klasse unter `breeze-light` lesbar
wird (#84), und dass unter den durchscheinenden Themes Lesbarkeit entsteht,
solange der Kontrasteffekt fehlt.

---

## 5. Heilung der Zeichnung — sechs datierte Vermerke

**Geänderte Datei:** `wireframes/Denkzettel Wireframes.dc.html`, sechs Stellen.
An Quellcode, SPEC und Tests ist nichts geändert worden.

**Bauart, für alle sechs gleich** — die von Sprint 7: Der überholte Absatz
bleibt Wort für Wort stehen, darunter steht ein datierter Vermerk in der
Warnfarbe der Datei (`#b3701f`) mit Datum, Story und dem Messwert, der an die
Stelle der alten Aussage tritt. Ein Wireframe ist die Beweislage seines
Standes; ein geglätteter Absatz nimmt einer künftigen Prüfung den Beleg, den
sie braucht.

| # | Stelle | Was der Vermerk sagt | Womit geprüft |
|---|---|---|---|
| **V1** | 4a, Schlussabsatz (unter dem Vermerk vom 04.08.2026) | Zwei Sätze des Vermerks darüber gelten nicht mehr: die Kehrseite ist für die vier Themes mit eigener `colors`-Datei geheilt (`breeze-light` 1,09 → 13,59 : 1), für die vier ohne bleibt sie (`Iridescent-round` 1,57 : 1, vor wie nach); und „aus der Palette kommen weiterhin Text …" gilt nur noch ohne `colors`-Datei. Dazu die Gegenrichtung: über schwarzem Grund fällt `cachyos-emerald-light` auf 1,37 : 1 | eigene Sitzungsläufe, alle acht Themes, `messungen/ux-m1`; Bild `zeichnung-v1-4a-lesbarkeit.png` |
| **V2** | 4b, Farbrolle „Notiztext" | `WindowText` gilt, solange das Theme keine eigene `colors`-Datei mitbringt; sonst deren `ForegroundNormal` über `KSvg::Svg::color(Text)`. Die Zahl 4,74 : 1 gilt weiter für den Weg über das Schema | fünfzehn Läufe, `ux-m1` und `ux-m2`; SPEC 3.1 trägt dieselbe Aussage |
| **V3** | 4b, Farbrolle „App-Name und Fußzeile" | Die Klasse hat **drei** Stellen; der Platzhaltertext gehört dazu. Und `PlaceholderText` gilt nur ohne `colors`-Datei des Themes | eigenes Bild `fenster-breeze-light-weiss-leer.png`, alle drei Stellen in derselben Farbe |
| **V4** | 4b, „Bekannte Grenze der Kleintexte" | Unter einem Theme mit eigener Datei bestimmt das **Theme** die Dämpfung; der Boden ist damit nicht angehoben (`breeze-light` 3,49 : 1 aus dem Theme, 2,83 : 1 aus dem Schema). Dazu die Umkehrung der Rangfolge unter zwei Themes | eigene Bildmessung `ux-m2` |
| **V5** | 4b, „Auswahl, Cursor, Scrollbalken" | Die Auswahl folgt weiter dem Schema, der Cursor nicht mehr — er wird mit der Textrolle des Feldes gezeichnet | Bilder `wechsel-5-breeze-light-auswahl.png` und `wechsel-2-breeze-light.png`, Spaltenprofil am Cursor |
| **V6** | 4b, Prüfsatz „Notiztext auf der Fensterrolle" | Der Prüfsatz gilt für Themes ohne eigene `colors`-Datei; wo eine da ist, ist die **Herkunft** zugesichert und keine Kontrastzahl | SPEC 3.1, dieselbe Formulierung |

**Prüfung der Zeichnung selbst:** Die Datei ist nach der Änderung in Chrome
gerendert und an allen sechs Stellen angesehen worden; vier Ausschnitte liegen
als `bilder/zeichnung-*.png` daneben. Alle Vermerke stehen in der Warnfarbe und
im Aufbau der übrigen, der Textfluss bricht an keiner Stelle. Auf- und
zugehende Auszeichnungen sind gezählt: `span` 369/369, `div` 570/570, `b`
274/274, `a` 77/77 — vor der Änderung stand jedes Paar ebenso gleich
(336/336, 570/570, 248/248, 77/77).

**Die vier Vermerke aus Sprint 7 sind gegen den neuen Stand gehalten worden.**
**W1** (4a, Farbe und Fläche) gilt nicht mehr — daraus ist V1 geworden, und das
war die Frage des PO. **W2** (Deckungsrand statt eigener Kontur), **W3** (3b
Fall 4 mit der Erweiterung aus #70) und **W4** (der Mausklick bewegt die Liste
nicht) berühren keine Aussage, die #85 ändert und stehen unverändert. Für
**W2** habe ich die Richtung an den eigenen Bildern nachgemessen: Über
schwarzem Grund steht am Rand ein **höherer** Wert als in der Fläche
(`default` 30,32,35 gegen 27,30,32; `breeze-light` 221,222,222 gegen
203,203,204) — die Hülle deckt am Rand stärker, wie es der Vermerk sagt. Den
Alphawert selbst zeigt eine Aufnahme des zusammengesetzten Bildes nicht; die
Zahlen 235 gegen 216 stammen aus dem Grab von Sprint 7 und sind hier nicht
nachgemessen.

**Nicht angefasst:** die Maßtafel „Rundung und Rand", der Befund **W5** aus
Sprint 7 (Aufnahmefenster 1f) und **W6** (die Belegform für Hülle, Rundung und
Kontur) — beide liegen außerhalb dieser Story und warten auf eine Entscheidung
des PO.

---

## 6. Was dieser Review nicht trägt

- **Die Wirkung des Kontrasteffekts.** Sie ist auf diesem Compositor-Stand
  nicht zu beobachten; ich habe die `[ContrastEffect]`-Gruppen der acht Themes
  gelesen und daraus Befund P2 abgeleitet, den Effekt selbst nicht erzeugt.
- **Die Farbschemata außer dem des Kunden.** Der Schemawechsel ist als
  Übergang belegt (AK 7), nicht als Reihe über mehrere Schemata; die führt der
  Strang in `p4-folgt-dem-schema.txt`.
- **Der installierte Stand unter `/usr`.** Den taktet der PO; geprüft wurde am
  eigenen Bau des Sprint-Standes.
- **Ein echter Tastendruck des Nutzers.** Der Text kommt in allen Läufen über
  `setPlainText()` ins Feld.
- **#61 und #76.** Nach Festlegung des PO keine UI-Stories dieses Sprints.

---

## 7. Bilder und Messungen dieses Reviews

**Bilder** (`bilder/`, alle in diesem Review erzeugt):

| Datei | Woher | Wozu |
|---|---|---|
| `fenster-<theme>-<grund>.png` (15) | Sitzung, `sitzungsbeleg` gegen den eigenen Bau | Normalfall je Theme und Grund |
| `fenster-<theme>-<grund>-leer.png` (15) | ebenda | Leerzustand, dritte Stelle der gedämpften Klasse |
| `wechsel-1-default.png` | Sitzung, `wechselbeleg` | Ausgangslage: Theme ohne `colors`-Datei |
| `wechsel-2-breeze-light.png` | ebenda | AK 5: Theme-Wechsel am stehenden Fenster; dazu die Cursorfarbe |
| `wechsel-3-breeze-light-nach-schemawechsel.png` | ebenda | AK 7: Schema auf Magenta, Farben unverändert |
| `wechsel-4-zurueck-auf-default.png` | ebenda | AK 5 rückwärts: Themefarbe geräumt |
| `wechsel-5-breeze-light-auswahl.png` | ebenda | Auswahl aus dem Schema über Themeschrift |
| `zeichnung-v1-4a-lesbarkeit.png` | Chrome | 4a: alter Vermerk und neuer untereinander |
| `zeichnung-v2-v4-4b-farbrollen.png` | ebenda | 4b: die drei Farbrollen-Vermerke |
| `zeichnung-v5-4b-cursor.png` | ebenda | 4b: „Auswahl, Cursor, Scrollbalken" mit Vermerk |
| `zeichnung-v6-4b-pruefsatz.png` | ebenda | 4b: Prüfsatz „Notiztext auf der Fensterrolle" |

**Messungen und Prüfmittel:**

| Datei | Inhalt |
|---|---|
| `bilder-erzeugen.sh` | fährt die Belegsonde über acht Themes und zwei Gründe |
| `sonden/wechselbeleg.cpp` | eigene Sonde: Theme- und Schemawechsel an **einem** stehenden Fenster |
| `sonden/CMakeLists.txt` | Bauplan, linkt gegen den eigenen Bau des Sprint-Standes |
| `messungen/schriftimbild.py` | misst die Schrift im Bild statt an der Palette |
| `messungen/ux-m1-sitzungsbelege.txt` | fünfzehn Läufe, je mit Grund, Auswahlpfad und Bildpunktverhältnis |
| `messungen/ux-m2-schrift-im-bild.txt` | dieselben Bilder, unabhängig vom Palettenweg gemessen |
| `messungen/ux-m3-wechsel.txt` | die fünf Zustände der Wechselsonde |

**Zwei Fehlläufe auf dem Weg, beide gemessen und beide behoben** — sie stehen
hier, weil ein Prüfmittel, das leise die falsche Stelle misst, in diesem
Projekt schon zweimal wie ein Beleg aussah:

1. **Die Wechselsonde schnitt vier weiße Bilder.** Unter Wayland meldet
   `QWidget::x()/y()` nicht die Lage auf dem Bildschirm; aus `0,0`
   geschnitten entstand viermal blanker Grund. Das Fenster wird jetzt über den
   Unterschied zweier Aufnahmen gesucht und an seiner von außen bekannten
   Größe erkannt — einmal, und dasselbe Rechteck gilt für alle fünf Zustände,
   damit eine Verschiebung sichtbar bliebe statt weggerechnet zu werden.
2. **Die Bildmessung nahm für kurze Zeilen die falschen Bildpunkte.** Der
   erste Lauf wählte den Glyphenkern als feste 5 % der Bildpunkte des Bandes
   und maß für den App-Namen — zehn Zeichen auf 957 Bildpunkten Breite —
   überwiegend Fläche: **1,04 : 1** für eine Schrift, die bei 4,06 : 1 steht.
   Die Auswahl läuft jetzt über die Schwelle statt über einen festen Anteil.
   Im selben Zug ist der Schnitt auf das Fensterinnere dazugekommen; ohne ihn
   meldete das Skript bei dunkler Hülle über hellem Grund ein Band über die
   ganze Bildhöhe.
