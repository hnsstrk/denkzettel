# Nachprüfung der AK-7-Heilung — Notizliste als Posteingang (S5a, Issue #46)

**Modus:** UI-Review, Nachprüfung zum `fail` aus
[sprint-03-s5a-posteingang.md](sprint-03-s5a-posteingang.md). **Datum:**
01.08.2026. **Prüfer:** `denkzettel-ux`.

**Geprüfter Stand:** `main`, Merge `f739751` über `7bc24ae`. Bibliothek neu
gebaut, `ctest` offscreen **7 von 7 grün**. Eigener Bildlauf gegen
`libdenkzettelui.a`, `QT_QPA_PLATFORM=offscreen`, `QT_QPA_PLATFORMTHEME=kde`,
`-style breeze`, Bezugszeitpunkt fest auf Fr 31.07.2026 16:00. Quelle:
`ux-nachpruefung-ak7.cpp`, Messwerte `messung.txt`, beides im Ordner neben
diesem Bericht. Die Bildstrecke des Entwicklers unter `s5a-posteingang/` ist
sein Selbstnachweis und war nicht meine Prüfgrundlage.

## Ergebnis

**AK 7: `ok` — der `fail` ist geheilt.** Alle fünf Szenen verhalten sich wie
gefordert.

**Ein neuer Befund: `warn`.** Das Vorscrollen zum Gruppenkopf greift bei
**jedem** Auswahlwechsel, nicht nur beim Sprung über eine Gruppengrenze. Nach
manuellem Scrollen springt die Liste dadurch bis zu **387 px entgegen der
Bedienrichtung** — derselbe Schaden, gegen den die Passt-Bedingung im großen
Fall gebaut wurde, an einer Stelle, die sie nicht abdeckt. Der Befund hängt
nicht an AK 7 und blockiert die Abnahme der Story nicht.

## Prüftechnik

Gemessen wurde **der Rollwert vor und nach jedem Tastendruck**, nicht der
Endzustand. Der Grund ist der, den der Entwickler bei seinem zweiten Test
selbst gefunden hat: Der nachfolgende `scrollTo(selection)` stellt die
Sichtbarkeit der Auswahl in beiden Fällen her, ein Endbild unterscheidet sie
also nicht. Sichtbar wird der Unterschied allein an der Bewegung.

Zwei Bestände, beide bei 900×600 (Viewport 279×552, Eintrag 72 px, Folgekopf
35 px):

- **groß** — zwei Gruppen zu je acht Notizen,
- **gemischt** — „Heute" mit sechs, „Gestern" mit drei, „Letzte Woche" mit vier
  Notizen.

## Die geprüften Szenen

| Szene | Erwartung | Messung | Verdikt |
|---|---|---|---|
| **N1** abwärts über die Grenze | Kopf der neuen Gruppe im Bild, Auswahl ganz sichtbar | Kopf „Gestern" y=432 im Bild, Auswahl y=467 ganz sichtbar, Rollwert +1 | ok |
| **N2** aufwärts auf die erste Notiz einer Gruppe | Kopf im Bild | Kopf „Gestern" y=0, Auswahl y=35 | ok |
| **N3** aufwärts auf die letzte Notiz einer **großen** Gruppe | Kopf bleibt draußen, **kein** Bildschirmsprung | Kopf „Heute" y=−531 draußen, Auswahl y=0 ganz sichtbar, Rollwert **−1** (eine Zeile) | ok |
| **N4** aufwärts auf die letzte Notiz einer **kleinen** Gruppe | Kopf jetzt im Bild — der geheilte Befund | Kopf „Gestern" **y=0, im Bild**, Auswahl y=179 ganz sichtbar, Rollwert −1 | **ok, geheilt** |
| **N5** weiter auf die mittlere Notiz derselben Gruppe | Kopf bleibt im Bild, keine Bewegung | Kopf y=0, Rollwert 7 → 7 | ok |
| **N7** abwärts durch eine große Gruppe, Zeile für Zeile | keine überflüssige Bewegung | sechsmal ±0, dann +2 und +1 — beide minimal und nach unten | ok |

**N4 gegen den alten Stand gehalten:** Im Review-Bild
`sprint-03-s5a-posteingang/s5a-c4-kleine-gruppe-aufwaerts-900x600.png` stand
oben ein kopfloser Block, und die einzige sichtbare Überschrift („Letzte
Woche") stand **unter** der Auswahl. Jetzt trägt `ak7-n4-kleine-gruppe-letzte-notiz.png`
den Kopf „Gestern" über seinen drei Notizen, die letzte davon ausgewählt und
vollständig sichtbar. Das ist genau die Auflage.

**N3 gegen denselben Stand gehalten:** unverändert. Der Kopf bleibt draußen,
die Bewegung beträgt eine Zeile. Die Passt-Bedingung tut, was sie soll.

## Neuer Befund — Vorscrollen ohne Gruppenwechsel — **warn**

**Beleg:** `ak7-n8a-vor-dem-tastendruck.png` und
`ak7-n8b-nach-dem-tastendruck.png`.

Ausgangslage (n8a): Der Nutzer hat mit dem Rad gescrollt. Sichtbar ist der
Übergang von „Heute" zu „Gestern"; die Auswahl steht auf „10:00" am oberen
Rand, der Kopf „Heute" liegt 387 px darüber außerhalb des Bildes.

Ein Druck auf **Pfeil nach unten** (n8b): Die Auswahl wandert eine Zeile
abwärts auf „09:00" — und **das Bild springt um 387 px nach oben**, auf 70 %
der Listenhöhe. Oben steht wieder „Heute", die Auswahl klebt am unteren Rand,
und der Gruppenübergang, den der Nutzer sich gerade erscrollt hatte, ist aus
dem Bild.

Derselbe Effekt in kleinerem Maßstab in `ak7-n6*.png`: Kopf 99 px über dem
Rand, Pfeil nach unten, Liste springt 99 px nach oben.

**Ursache** (`src/ui/librarywindow.cpp:435–443`): `showNote()` sucht den
Gruppenkopf bei jedem Auswahlwechsel und scrollt ihn ins Bild, sobald er mit
der Auswahl zusammen hineinpasst. Die Bedingung, unter der das laut Vorgabe
überhaupt geschehen soll, fehlt: AK 7 und Wireframe 3b Fall 4 sagen beide
„**Springt die Auswahl über eine Gruppengrenze**, ist der Kopf der neuen
Gruppe mit im Bild". Steht die Auswahl schon in der Gruppe, ist gar nichts zu
holen — der Nutzer hat den Kopf dann bewusst weggescrollt.

Der Fall entsteht nicht durch Tastaturnavigation allein: Läuft die Auswahl mit
den Pfeiltasten, bleibt der Kopf ohnehin im Bild, solange er passt. Er entsteht
durch **Rad oder Rollbalken, gefolgt von einer Pfeiltaste** — eine gewöhnliche
Bedienfolge.

**Auflage:** Das Vorscrollen an den Gruppenwechsel binden. Der Kopf wird nur
geholt, wenn die neue Auswahl in einer anderen Gruppe steht als die vorige;
die Passt-Bedingung bleibt unverändert daneben stehen. Die vorige Auswahl ist
ohne Umbau zu haben — `QItemSelectionModel::currentChanged` liefert sie als
zweiten Parameter, den `showNote()` heute nicht entgegennimmt. **Beim ersten
Setzen** (vorige Auswahl ungültig, etwa nach dem Öffnen oder einem Neuaufbau
der Liste) weiterhin vorscrollen, damit eine wiederhergestellte Auswahl ihre
Überschrift bekommt.

**Prüfsatz dazu**, mit derselben Technik, die der Entwickler schon gefunden
hat: Steht die Auswahl in einer Gruppe, deren Kopf aus dem Bild gescrollt ist,
und wandert sie innerhalb dieser Gruppe auf eine bereits sichtbare Zeile, darf
sich der Rollwert **nicht ändern**. Gegen den heutigen Stand gehalten schlägt
dieser Satz fehl (Rollwert 6 → 0).

## Geprüfte Bilddateien (alle unter `sprint-03-s5a-ak7-nachpruefung/`)

| Datei | Szene |
|---|---|
| `ak7-n1-vor-der-grenze.png` | letzte Notiz von „Heute", vor dem Grenzübertritt |
| `ak7-n1-abwaerts-ueber-die-grenze.png` | N1 — abwärts über die Grenze |
| `ak7-n2-aufwaerts-erste-notiz-der-gruppe.png` | N2 — aufwärts auf die erste Notiz |
| `ak7-n3-aufwaerts-grosse-gruppe.png` | N3 — große Gruppe, Kopf bleibt draußen |
| `ak7-n4-kleine-gruppe-letzte-notiz.png` | N4 — **der geheilte Befund** |
| `ak7-n5-kleine-gruppe-mittlere-notiz.png` | N5 — mittlere Notiz derselben Gruppe |
| `ak7-n6a-ausgangslage.png` | N6 — vor dem Raddreh |
| `ak7-n6b-nach-dem-raddreh.png` | N6 — Kopf aus dem Bild, Auswahl unverändert |
| `ak7-n6c-nach-dem-tastendruck.png` | N6 — Rücksprung um 99 px |
| `ak7-n8a-vor-dem-tastendruck.png` | N8 — **Beleg des neuen Befunds**, vorher |
| `ak7-n8b-nach-dem-tastendruck.png` | N8 — **Beleg des neuen Befunds**, nachher |

## Nachtrag — drei Szenen für den Abschlusslauf (Vorher-Stand)

Nach diesem Bericht ist `ux-nachpruefung-ak7.cpp` um drei Szenen erweitert und
gegen denselben Stand `f739751` einmal gelaufen, damit für den Lauf nach der
Heilung des `warn` Vorher-Werte vorliegen. Messwerte:
`messung-neue-szenen.txt`.

| Szene | Vorher-Stand (`f739751`) | Wozu |
|---|---|---|
| **N10** Grenzübertritt auf eine schon vollständig sichtbare Zeile (`ak7-n10-…png`) | Zielzeile y=144 h=72, **schon ganz sichtbar**; Kopf „Gestern" kommt trotzdem auf y=0 | Prüfsatz gegen den Rückfall: Eine Bedingung auf die Sichtbarkeit der Auswahl hätte hier den Kopf draußen gelassen — also den geheilten `fail` wiederhergestellt |
| **N11** Mausklick in eine andere Gruppe (`ak7-n11a/b-…png`) | Klickziel bei y=0 sichtbar, Kopf bei y=−387; nach dem Klick **Rollwert 6 → 0**, die geklickte Zeile wandert auf y=387 | Belegt **Issue #57**; der Fall besteht schon in diesem Stand und wird von der Gruppenwechsel-Bedingung nicht beseitigt, weil ein Klick in eine andere Gruppe selbst ein Gruppenwechsel ist |
| **N12** Löschen und Undo an der Gruppengrenze (`ak7-n12a/b/c-…png`) | 12 Zeilen → nach dem Löschen 10, Auswahl auf der Folgenotiz unter „Letzte Woche" → nach dem Undo wieder 12, Auswahl auf der wiederhergestellten Notiz unter „Gestern" | Hier bricht ein Gruppenvergleich, der an Zeilennummern hängt: `takeNote`/`insertNote` bauen die Zeilen neu auf. Sonst fiele es nirgends auf |

**Zu N11 / #57:** Der PO hat entschieden, den Fall nicht in Sprint 3 zu heilen —
die naheliegende Zusatzbedingung ist erwiesen falsch (sie bricht N10), der
eigentliche Unterschied ist die Entfernung und nicht die Sichtbarkeit, und ein
willkürlicher Schwellwert im Bedienverhalten wäre schlechter als der Fehler.
Der Fall ist damit **benannte Entscheidung, nicht Versäumnis**. Abbruchkriterium
für #57, unabhängig vom gewählten Weg: *Ein Mausklick auf eine bereits
vollständig sichtbare Zeile darf den Rollwert nicht verändern.*

## Stand der Punkte aus dem Ursprungsreview

1. **AK 7** — geheilt, Verdikt `ok`.
2. **Kopfzeilenmaß in 2c** — erledigt (Commit `1aa280e`).
3. **Haarlinien in 2a/2b/2c/3a/3b** — erledigt (Commit `1aa280e`).
4. **Plattformthema im Bildlauf** — vom Entwickler übernommen
   (`tests/libraryshots.cpp`), Prozessteil beim PO.
5. **Neu: Vorscrollen ohne Gruppenwechsel** — `warn`, Auflage oben.

Keine Änderung an Quellcode, SPEC oder Wireframes in diesem Lauf; kein Commit.
