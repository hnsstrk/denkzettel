# UI-Review Sprint 9 — #100 „Eingabefeld aus der Theme-Grafik" und #101 „Haarlinien in der Notizliste"

**Modus:** UI-Review (DoD 3) · **Rolle:** UI/UX · **Datum:** 07.08.2026 ·
**Geprüfter Stand:** `main`, `70902a4` (beide Stränge zusammengeführt).

**Prüfstand** (eine Aussage gilt für einen Stand, B17): CachyOS, Plasma auf
Wayland, angemeldete Sitzung, Bildschirm 3840×2160 Gerätebildpunkte,
Sitzungsskalierung **1,6** (`kwinrc`, im Bild nachgemessen: der 1100×800
Punkte große Hintergrund der Sonde belegt 1760×1280 Gerätebildpunkte).
Farbschema Breeze Dark, Desktop-Theme über den Rückfall `default`
(`plasmarc` trägt keinen Namen). Acht installierte Desktop-Themes.

## Wie dieser Review geführt wurde

Alle Bilder unter `bilder/` sind in diesem Review entstanden, aus einem
**eigenen** Bau des Sprint-Standes außerhalb des Repositoriums — warnungsfrei
gebaut, `ctest` darauf **9 von 9 grün**. Die Bilder der beiden
Umsetzungsstränge sind gelesen und als Material verwendet worden; sie ersetzen
die eigenen nicht (B3).

| Prüfmittel | Einstellung |
|---|---|
| Eigene Messsonde `sonden/uxsonde.cpp` | gegen `libdenkzettelui.a` und `libdenkzettelcapture.a` des eigenen Baus gelinkt — geprüft wird der gelieferte Code, kein Nachbau |
| Offscreen, Skalierung 1 | `QT_SCALE_FACTOR=1` — dort gelten die Bildpunktzusicherungen AK 1 bis AK 3 von #101 |
| Offscreen, Skalierung 1,6 | `QT_SCALE_FACTOR=1.6` — die Einstellung des Kunden |
| Angemeldete Sitzung | `QT_SCALE_FACTOR` **ungesetzt**; unter Wayland multiplizierte die Variable mit der Sitzungsskalierung |
| Vergleichsmontagen | `bilder/offscreen-1-6/vergleich-*.png` — Linien im gerenderten Bild entfernt, um die Kippfrage am **gebauten** Stand zu stellen |

`QT_QPA_PLATFORMTHEME=kde` war in jedem Lauf gesetzt.

**Wie die Sitzungsbilder entstanden sind:** Die Sonde legt ein eigenes
Hintergrundfenster mit magentafarbenem Rahmen unter das Prüffenster. Die
Bildschirmaufnahme wird an diesem Rahmen zugeschnitten (`zuschnitt.py`) und die
Vollaufnahme sofort gelöscht — so kommt der Schatten des Compositors ins Bild,
ohne dass vom Schreibtisch des Kunden ein Bildpunkt in das öffentliche
Repository gerät.

**Zwei Fehler der Sonde, die wie Fehler des Erzeugnisses aussahen.** Beide sind
hier festgehalten, weil sie zeigen, woran ein Bildbeleg scheitert, ohne rot zu
werden:

1. Der erste Sitzungslauf zeigte ein **helles Fenster mit hellem, unlesbarem
   Text**. Ursache war der Sandkasten der Sonde: Ohne `kdeglobals` färbt die
   Theme-Grafik aus den Voreinstellungen um, während die Qt-Palette schon das
   dunkle Schema trug. Die Sonde kopiert das Farbschema seither in ihren
   Sandkasten.
2. Der zweite Lauf zeigte ein Fenster **ohne Schatten**. Ursache war
   `show()` statt `showCapture()`: Der Schatten wird in `present()` an die
   frische Wayland-Fläche gebunden, und wer das Fenster am regulären Weg vorbei
   zeigt, bekommt ein Bild, auf dem ein zugesicherter Zustand fehlt, den das
   Erzeugnis sehr wohl herstellt.

Keiner der beiden ist ein Befund gegen die Umsetzung.

---

## Die drei offenen Fragen

### 1. Stärkt die Gruppenlinie die Trennung oder schwächt sie sie? — **Sie stärkt sie.**

Geprüft am gebauten Stand unter der Skalierung des Kunden, an einer Datenlage
mit drei gefüllten Gruppen, und gegen drei Montagen desselben Bildes:
`bilder/offscreen-1-6/vergleich-tafel.png` zeigt nebeneinander (A) keine
Linien, (B) nur die eingerückte, (C) nur die volle, (D) den gelieferten Stand.

Der befürchtete Kasten entsteht nicht, und zwar aus einem Grund, der im Bau
steht: Unter der letzten Notiz einer Gruppe steht **keine** Linie. Jede Gruppe
wird oben abgetrennt und unten offen gelassen — das ist ein Abschnitt und kein
Kasten. Ein Kasten bräuchte beide Kanten.

Der 14-px-Zwischenraum bleibt dabei erhalten; die Linie sitzt an seiner
Oberkante und liest sich als Eröffnung der neuen Gruppe. In der Montage B
(Gruppenlinie fallengelassen) trägt die Gruppengrenze allein der größere
Abstand — erkennbar, aber sichtbar schwächer als in D. **Der benannte Hebel
wird nicht gezogen.**

Beleg: `bilder/offscreen-1-6/vergleich-tafel.png`,
`bilder/sitzung/s101-notiz-und-gruppengrenze-3x.png`.

### 2. Trägt die Rangfolge? — **Aus der Länge ja, aus der Stärke nicht.** (warn)

Die Länge trägt, und zwar in jeder gemessenen Linie: Unter Skalierung 1 ist die
eingerückte 255 von 279 Punkten breit und die volle 279 von 279; unter 1,6
sind es 408 von 446 gegen 446 von 446. Der Unterschied ist im Bild zu sehen.

**Die Stärke trägt nicht.** Unter der Skalierung 1,6 belegt dieselbe
1-Punkt-Linie mal einen, mal zwei Gerätebildpunkte — je nachdem, wo ihre
Zeile im Raster landet:

| Lauf | Eintragslinien | Gruppenlinien |
|---|---|---|
| offscreen 1,6 (`s101-rhythmus`) | y=234 **1**, y=349 **1**, y=635 **2**, y=750 **2** | y=466 **1**, y=867 **2** |
| Sitzung (`s101-bibliothek-sitzung`) | y=393 **1** | y=509 **2**, y=681 **1**, y=852 **1**, y=1023 **2** |
| Beleg des Stranges (`…-s101-listentrenner/bilder/skalierung-1-6/09-…`) | y=190 **2**, y=306 **1**, y=592 **2**, y=707 **2** | y=422 **2** |

Dreimal unabhängig, auf zwei Wegen und einmal im Material des Umsetzungsstranges
selbst. Im ersten Lauf steht eine **Gruppen**linie von einem Punkt über
**Eintrags**linien von zwei Punkten: dort widerspricht die Stärke der
Rangfolge, die die Länge herstellt.

Die Festlegung des Kunden lautet „Gleiche Farbe, verschiedene Ausdehnung — die
Rangfolge entsteht aus der Länge des Strichs statt aus seiner Stärke"; der
Bau hält die gleiche Stärke unter Skalierung 1 ein und unter 1,6 nicht. Auf
Skalierung 1 (wo AK 1 bis AK 3 messen) ist jede Linie genau ein Punkt — der
Fehler ist für die Prüfsätze unsichtbar.

**Korrekturvorschlag** (`src/ui/notelistdelegate.cpp:183` und `:225`): Die
Linien in Gerätebildpunkten ausrichten, statt ein logisches Rechteck zu füllen.
Also Oberkante und Stärke aus `painter->device()->devicePixelRatioF()` rechnen
und als `QRectF` füllen — Stärke `std::max(1.0, std::round(dpr)) / dpr`,
Oberkante `std::round(kante * dpr) / dpr`. Dann sind alle Linien gleich stark
und wachsen mit der Skalierung mit. Die Bildpunkt-Prüfsätze auf Skalierung 1
bleiben davon unberührt; ein Prüfsatz auf Skalierung 1,6 wäre der neue
Nachweis.

### 3. Ist der Eingabebereich als solcher erkennbar? — **Ja, unter `default` deutlich.** (pass, mit benannter Grenze)

In der angemeldeten Sitzung, Skalierung 1,6, Theme `default`, Schema
Breeze Dark:

| Fläche | Farbe | |
|---|---|---|
| Hülle (über hellem Grund, sie deckt nicht voll) | 63,66,66 | |
| Feldkante | 76,78,81 | eine Punktzeile, 1,22 : 1 gegen die Hülle |
| Feldfläche (voll deckend) | 20,22,24 | |

**Feld gegen Hülle: 1,79 : 1.** KRunners Feld misst im Sitzungsbild des
Sprint-7-Reviews 1,41 : 1 — der Maßstab des Kunden ist übertroffen. Der
Vorher-Nachher-Vergleich steht in einem Bild:
`bilder/sitzung/s100-vorher-nachher.png` (oben der Farbblock aus Sprint 7,
unten der gelieferte Stand).

Offscreen misst dieselbe Lage nur 1,15 : 1, weil dort der Alphakanal der Hülle
verlorengeht und die Durchsicht fehlt, die die Abhebung in der Sitzung
vergrößert. Das ist B21 an einer weiteren Größe: **Eine Aussage über die
Sichtbarkeit des Feldes ist offscreen nicht zu treffen.**

Unter den Themes, die die Grafik nur andeuten, tritt die Sichtbarkeit nicht
ein — gemessen `CachyOS-Nord-round` 1,03 : 1 und `cachyos-emerald` 1,00 : 1,
im Bild ein kaum wahrnehmbarer dunkler Hauch ohne Kante
(`bilder/offscreen-1-6/s100-feld-CachyOS-Nord-round.png`). Das deckt sich mit
AK 6b und SPEC 3.1; die Grenze ist benannt und nicht verschwiegen.

---

## Die beiden Festlegungen des PO

### „Der überfahrene Eintrag behält seine Linien." — **bestätigt**

Beleg: `bilder/offscreen-1-6/s101-rhythmus-hover-notizgrenze-4x.png`. Der
Hover-Zustand trägt in Breeze eine eigene, gerundete Fläche mit hellblauer
Kontur; die graue Trennlinie darüber unterscheidet sich davon in Farbe und
Einrückung deutlich genug, um nicht als Teil davon gelesen zu werden. Der
Gegenvorschlag hätte einen Preis, den das Bild nicht zeigt, die Bewegung aber
schon: Eine Liste, deren Linien bei jeder Mausbewegung verschwinden und
wiederkehren, ist unruhig. Der schlichtere Bau ist hier auch der ruhigere.

### „Die Ausnahme in AK 3 gilt allein der Auswahl." — **bestätigt, mit einer Beobachtung**

Die Gruppenlinie bleibt über einem Kopf stehen, auch wenn die Notiz unmittelbar
darüber die ausgewählte ist (AK 3b). Das ist richtig so: Die Gliederung darf
nicht davon abhängen, welche Zeile gerade ausgewählt ist — und die letzte Notiz
einer Gruppe ist ein häufiger Auswahlfall.

**Beobachtung ohne Änderungsforderung:** In dieser Lage stößt die Linie bündig
an die Unterkante der Auswahlfläche und schneidet deren Rundung optisch ab; sie
liest sich dort eher als Abschluss der Auswahl denn als Gruppengrenze
(`bilder/offscreen-1-6/s101-normalfall-gruppengrenze-4x.png`). Ein Ausweg wäre,
die Linie in genau diesem Fall um einen Punkt nach unten zu setzen — das wäre
eine Ecke mehr in einer Regel, vor der die Zeichnung selbst warnt. Der
Vorschlag geht deshalb als Beobachtung an den PO und nicht als Mangel.

---

## Prüfpunkte, je gezeichneter Bereich einer

### Zeichnung 3a — Notizliste (#101)

| # | Prüffrage aus der Zeichnung | Verdikt | Fundstelle |
|---|---|---|---|
| L1 | P1/AK 1 — Eintragslinie in der letzten Bildpunktzeile, nur zwischen 12 px und Breite−12 | **pass** | `offscreen-1/s101-rhythmus.txt`: y=146, 218, 397, 469 je x=12..266 von 279 |
| L2 | P2/AK 2 — Gruppenlinie über jedem Kopf außer dem ersten, volle Breite des Zeilenrechtecks | **pass** | ebd.: y=291, 542 je x=0..278; über Kopf 0 keine |
| L3 | P3/AK 3a — keine Linie unter der letzten Notiz einer Gruppe, keine unter einem Kopf, keine an den Kanten der Auswahl | **pass** | `offscreen-1/s101-rhythmus-auswahl.txt`: bei Auswahl auf Zeile 2 fehlen y=146 und y=218, alles andere steht |
| L4 | AK 3b — die Gruppenlinie bleibt über dem Kopf, auch wenn die Notiz darüber ausgewählt ist | **pass** | ebd.: y=291 vorhanden; Bild `s101-normalfall-gruppengrenze-4x.png` |
| L5 | P4/AK 4 — Farbe aus Grund und Text im Verhältnis `frameContrast`, keine Palettenrolle | **pass** | gemessen 66,68,70 auf Grund 20,22,24 = **1,85 : 1**. Die Rechnung geht auf: 0,8 × (20,22,24) + 0,2 × (252,252,252) ergibt (66,4 · 68,0 · 69,6) — die Mischung mit dem vorgefundenen Verhältnis 0,20, Kanal für Kanal. Grund und Textfarbe sind dabei **im selben Bild** abgelesen, nicht aus der Palette geholt |
| L6 | P5/AK 5 — kein Maß hat sich geändert | **pass** | Kopf 0: 27 px, jeder weitere 35 px (Differenz 8 = 14−6), jede Notiz 72 px — in allen fünf Läufen gleich |
| L7 | AK 6 — Trefferliste: über dem ersten sichtbaren Kopf keine Linie | **pass** | `offscreen-1/s101-suche-Werkstatt.txt` (eine Gruppe, keine Linie) und `s101-suche-für.txt` (zwei Gruppen, Linie nur über der zweiten) |
| L8 | AK 7 — Bild unter der Skalierung des Kunden | **pass** | `sitzung/s101-bibliothek-sitzung.png`, die Linien sind zu sehen |
| L9 | Rangfolge Zeile → Notiz → Gruppe aus der Länge, nicht aus der Stärke | **warn** | siehe Frage 2 |
| L10 | Raumaufteilung: Listenbreite und Splitterstellung unverändert | **pass** | Sichtfeld 279 px bei 900 px Fensterbreite, wie vor den Linien; im Sitzungsbild dieselbe Aufteilung |
| L11 | Gruppenkopf ohne Fläche, ohne Fettdruck im Eintrag | **pass** | `sitzung/s101-bibliothek-sitzung.png`: Köpfe halbfett auf blankem Grund, Betreff und Vorschau unterscheiden sich allein in der Farbe |

### Zeichnung 4b — Erfassungsfenster (#100)

| # | Prüffrage aus der Zeichnung | Verdikt | Fundstelle |
|---|---|---|---|
| E1 | F1/AK 1 — Fläche und Kante aus `widgets/lineedit`, Vorsatz `base`, derselben `ImageSet` wie die Hülle | **pass** | Die Erscheinung folgt dem Theme: `default` 20,22,24 mit Kante, `breeze-light` 255,255,255, `CachyOS-Nord-round` 28,32,48 ohne Kante — eine Palettenrolle könnte das nicht |
| E2 | F2/AK 2 — Hülle unberührt: Randmaß, Eckform, Kontur, Schatten | **pass** | `sitzung/s100-erfassung-sitzung.png`: **Deckungsrand** zwei Punktzeilen (48,50,52 und 53,55,57 vor der Fläche 63,66,66), **Eckform** gerundet (die Kante rückt über sechs Zeilen um vier Punkte ein), **Schatten** über dem Fenster messbar (der Grund fällt von 228,225,219 auf 224,221,215) und vom Fenster selbst als angelegt gemeldet — die Belegform, die Zeichnung 4b für den Schatten ausdrücklich zulässt. Ein zweiter Rahmen um die Hülle entsteht nirgends. Dass diese Größen **unverändert gegenüber #83** sind, trägt der reparierte Prüfsatz aus AK 9, nicht mein Bild |
| E3 | F4/AK 4 — das Feld folgt dem Theme-Wechsel am stehenden Fenster | **pass**, gestützt | Eigene Bilder zeigen die Erscheinung unter drei Themes je eigener Prozess; den **Wechsel am stehenden Fenster** belegen die Sitzungsbilder des Stranges (`…-s100-eingabefeld/bilder/sitzung/sitzung-1..3`), gesichtet und schlüssig. Das Theme des Kunden habe ich nicht umgestellt |
| E4 | F5/AK 5 — 12/10/8 zuzüglich des Feldrandes; App-Name und Fußzeile stehen still | **pass** | App-Name beginnt bei 18,8 px, die Feldkante bei 18,1 px — beide auf derselben linken Kante; der Notiztext rückt auf 28,1 px nach innen |
| E5 | AK 6b — die Grenze der schwach zeichnenden Themes ist benannt | **pass** | 1,03 : 1 (`CachyOS-Nord-round`), 1,00 : 1 (`cachyos-emerald`); steht in SPEC 3.1 und im Issue |
| E6 | Raumaufteilung: über der Fußzeile mehr Luft als unter dem App-Namen | **pass** | gemessen 15,7 px gegen 11,3 px (`offscreen-1-6/s100-feld-default.png`) |
| E7 | Notiztext auf der Fensterrolle, lesbar auf dem neuen Grund | **pass** | `sitzung/s100-erfassung-sitzung.png`: heller Text auf der dunklen Feldfläche, gut lesbar |
| E8 | Leerer Zustand: der Platzhalter steht seit dem Feld auf der Feldfläche | **pass** | `offscreen-1-6/s100-feld-leer-default.png`: „Gedanke festhalten …" klar lesbar, Cursor sichtbar |
| E9 | Der Eingabebereich ist als solcher erkennbar (der Kundenbefund) | **pass** | siehe Frage 3 |

---

## Befunde

**Kein `fail`.** DoD 3 ist aus Sicht des UI/UX-Reviews erfüllt.

**Ein `warn`** — L9, die Strichstärke unter der Skalierung des Kunden. Er trifft
keine Zusicherung der Akzeptanzkriterien (die messen auf Skalierung 1), wohl
aber die Festlegung des Kunden, aus der die Kriterien hervorgegangen sind. Er
ist behebbar, ohne eine Gestaltungsfrage neu zu stellen, und die Korrektur
gehört mit einem Prüfsatz auf Skalierung 1,6 belegt.

**Zwei Beobachtungen ohne Änderungsforderung:** die Gruppenlinie an der
Unterkante der Auswahl (siehe oben) und der Umstand, dass der Notiztext seit
dem Feld gegenüber dem App-Namen um rund 9 px eingerückt steht — beides ist in
Zeichnung 4b so vorgesehen.

**Für den PO:** Zeichnung 3a schweigt darüber, in welchen Punkten die
Haarlinie ihre Stärke misst. Wenn die Korrektur zu L9 gefahren wird, gehört das
im selben Zug in die Zeichnung — ein Gerätebildpunkt, nicht ein Layoutpunkt.
Ich habe die Zeichnung nicht angefasst, weil mein Review keine Festlegung
ändert.

## Belege

```
docs/scrum/reviews/sprint-09-ui-review/
├── bericht.md                  dieser Bericht
├── bilder-erzeugen.sh          alle Läufe, reproduzierbar
├── zuschnitt.py                Zuschnitt der Sitzungsaufnahmen am Rahmen
├── sonden/uxsonde.cpp          Messsonde, gegen die Bibliotheken des Standes gelinkt
├── sonden/CMakeLists.txt
├── bilder/offscreen-1/         Skalierung 1 — hier gelten die Bildpunktzusicherungen
├── bilder/offscreen-1-6/       Skalierung 1,6 — die Einstellung des Kunden,
│                               darunter vergleich-tafel.png (Kippfrage)
└── bilder/sitzung/             angemeldete Sitzung (B21), darunter
                                s100-vorher-nachher.png
```

Zu jedem Bibliotheksbild liegt ein `.txt` daneben: Zeilenrechtecke der Ansicht,
gefundene Farbläufe mit Strecke und Kontrast, Listengrund. Zu jedem
Erfassungsbild ein Schnitt durch Fenster- und Feldmitte mit jedem Farbwechsel.
