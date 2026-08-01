# Zugabe zum UI-Review Sprint 3 — Trefferliste und Leerzustand der Suche (S6, Issue #8)

**Modus:** UI-Review, Zugabe zum Hauptauftrag
[S5a/#46](sprint-03-s5a-posteingang.md). **Datum:** 01.08.2026.
**Prüfer:** `denkzettel-ux`.

**Geprüfter Stand:** `main`, Merge `7c26ebe` über `14bf65d`. Bibliothek neu
gebaut, eigener Bildlauf gegen `libdenkzettelui.a`,
`QT_QPA_PLATFORM=offscreen`, `QT_QPA_PLATFORMTHEME=kde`, `-style breeze`,
Bezugszeitpunkt fest auf **Fr 31.07.2026 16:20**. Quelle: `ux-shot-s6.cpp`,
Messwerte `messung.txt`, beides im Ordner neben diesem Bericht. Die sechs
Bilder unter `s6-suche/` sind der Selbstnachweis des Entwicklers und waren
nicht meine Prüfgrundlage.

**Der PO hat S6 ausdrücklich nicht als UI-Story eingestuft** (Issue #8). Diese
Zugabe prüft daher nicht die Story, sondern die drei Punkte, die der Auftrag
benannt hat: die nachzuziehende Stelle der Zeichnung, die Gliederung in einer
gefilterten Liste und den neuen Leerzustand.

**Ergebnis: kein `fail`, kein `warn`.** Zwei Hinweise ohne Auflage.

## 1. Zeichnung nachgezogen — erledigt

`wireframes/Denkzettel Wireframes.dc.html`, Festlegungsblock zu 2b/2c, Absatz
**„Deaktiviertes Suchfeld"** (Zeile 421). Die alte Festlegung bleibt stehen und
trägt jetzt den Überholt-Vermerk, wie ihn die Datei an zwei weiteren Stellen
schon führt (Listenspalte in 2b, Zeitstempel in 2c) — der Weg der Entscheidung
soll lesbar bleiben, nicht verschwinden.

Aufgenommen sind: das Feld ist aktiv, Tooltip und Umhüllung sind fort, der
Löschknopf ist der Ein-Griff-Weg zurück; die Trefferliste zeigt die Gliederung
aus 3a und lässt Gruppen ohne Treffer aus; der Leerzustand „Keine Treffer" samt
Hinweiszeile, ohne Symbol, mit leerem Detailbereich; und als offener Rest die
Operatoren aus SPEC 6, die erst S7 parst.

Der Leerzustand steht damit im Festlegungsblock von 2c, also dort, wo die
beiden anderen Leerzustände wohnen. Eine eigene Zeichnung bekommt er nicht —
Begründung unter Punkt 3.

## 2. Gliederung in der gefilterten Liste — **ok**

Zwei Fälle geprüft, beide mit demselben Bestand über alle fünf Gruppen.

**Lücke zwischen den Gruppen** (`s6-e1-treffer-mit-luecke-900x600.png`):
Der Begriff „backup" trifft nur in „Heute" und in „Älter". Die Liste zeigt
genau diese beiden Köpfe mit je einem Eintrag, die drei Gruppen dazwischen
fehlen ganz. Das liest sich richtig und verbirgt nichts: Ein Kopf behauptet
nicht, vollständig zu sein, er sagt nur, wann das darunter entstand.

**Kopflastige Trefferliste** (`s6-e2-ein-treffer-je-gruppe-900x600.png`):
Der Begriff „vortrag" trifft in jeder der fünf Gruppen genau einmal — die Liste
besteht dann zur Hälfte aus Köpfen (5 Köpfe, 5 Einträge). Auch das trägt. Die
Köpfe wirken hier eher noch nützlicher als in der vollen Liste, denn in „Heute"
und „Gestern" zeigt der Eintrag nur die Uhrzeit; **ohne den Kopf stünde bei
einem Treffer von heute nirgends, dass er von heute ist.** Die Gliederung ist in
der Trefferliste damit kein Erbstück, das man mitschleppt, sondern der Ort, an
dem sie am meisten leistet.

**Auswahl in der Trefferliste**
(`s6-e3-auswahl-in-der-trefferliste-900x600.png`): Auswahlfarben und
Detailbereich verhalten sich wie in der vollen Liste, der Detailzeitstempel
behält die volle Form („Heute 11:05").

**Rückweg** (`s6-e7-feld-geleert-900x600.png`): Das geleerte Feld stellt die
vollständige Liste mit allen fünf Gruppen wieder her — Zeile für Zeile
identisch mit dem Ausgangsbild `s6-e0-volle-liste-900x600.png`.

Die Auflagen aus dem Review zu #46 erbt die Trefferliste tatsächlich mit: Sie
geht durch dasselbe Modell und denselben Delegate. Wird AK 7 dort geräumt, ist
sie hier ohne eigene Arbeit mitgeräumt.

## 3. Leerzustand „Keine Treffer" — **ok**, und der PO hat recht

`s6-e4-keine-treffer-900x600.png`: „Keine Treffer" über „Den Suchbegriff ändern
oder das Feld leeren.", zentriert in der Listenspalte, **ohne Symbol**, der
Detailbereich daneben leer.

**Ich sehe es nicht anders als der PO.** Das ist keine neue Gestaltung: Es ist
die dritte Seite derselben `placeholderPage(…)`-Machart, die Wireframe 2c für
die beiden bestehenden Leerzustände festlegt; neu sind nur die Wörter, und die
stehen im Akzeptanzkriterium. Eine Zeichnung hätte hier nichts gezeigt, was
nicht schon in 2c steht. Die beiden Entscheidungen, die tatsächlich zu treffen
waren, sind beide richtig getroffen und beide begründet:

- **Kein Symbol.** Das Symbol gehört dem Erststart — einem Zustand, den der
  Nutzer nicht selbst herbeigeführt hat und nur durch Schreiben verlässt. „Keine
  Treffer" hat er gerade selbst erzeugt und verlässt es mit dem nächsten
  Tastendruck; ein Bild dafür wäre Aufwand ohne Aussage.
- **Detailbereich bleibt leer.** Dieselbe Regel wie bei der leeren Bibliothek:
  ein leeres Fenster soll nicht zweimal dasselbe sagen (2c).

Der Hinweistext nennt beide Wege hinaus (ändern, leeren) und ist in der
unpersönlichen Infinitivform gehalten, die SPEC 15 app-weit festlegt. Passt.

Geprüft habe ich außerdem, ob der Zustand auch dann greift, wenn er greifen
muss, und nicht öfter: Bei einem Suchbegriff aus zwei Zeichen („ra",
`s6-e5-zwei-zeichen-900x600.png`) erscheint er **nicht** — der
Teilstring-Vergleich aus SPEC 6 findet die Notizen, in denen die zwei Zeichen
stecken. Die Entscheidung zu kurzen Begriffen ist damit nicht nur dokumentiert,
sondern am Bild belegt.

## Zwei Hinweise, keine Auflage

**Hinweis 1 — Operatoren vor S7** (`s6-e6-operator-vor-s7-900x600.png`): Wer
`tag:backup` eintippt, bekommt „Keine Treffer". SPEC 6 führt den Operator, der
Parser kommt erst mit S7, bis dahin ist die Eingabe schlichter Suchtext. Für den
Nutzer, der die SPEC nicht kennt, ist das unauffällig; das Feld verspricht mit
seinem Platzhalter „Volltextsuche …" auch nicht mehr. Ich halte das für eine
tragbare Zwischenstufe und habe sie in der Zeichnung als offenen Rest vermerkt,
damit sie bei S7 nicht neu entdeckt werden muss.

**Hinweis 2 — keine Hervorhebung der Fundstelle:** Die Trefferliste zeichnet den
gesuchten Begriff im Eintrag nicht aus. Weder SPEC noch Wireframe verlangen es,
und für einen Betreff, der ohnehin elidiert wird, ist der Gewinn zweifelhaft —
die Fundstelle liegt oft im abgeschnittenen Teil. **Ausdrücklich keine Auflage**,
nur eine Notiz für den Fall, dass der Kunde die Suche später als „ich sehe
nicht, warum das ein Treffer ist" beanstandet.

## Geprüfte Bilddateien (alle unter `sprint-03-s6-trefferliste/`)

| Datei | Zustand |
|---|---|
| `s6-e0-volle-liste-900x600.png` | leeres Suchfeld, volle Liste, fünf Gruppen |
| `s6-e1-treffer-mit-luecke-900x600.png` | „backup" — Treffer nur in „Heute" und „Älter" |
| `s6-e2-ein-treffer-je-gruppe-900x600.png` | „vortrag" — ein Treffer je Gruppe, halbe Liste aus Köpfen |
| `s6-e3-auswahl-in-der-trefferliste-900x600.png` | Auswahl und Detailbereich in der Trefferliste |
| `s6-e4-keine-treffer-900x600.png` | Leerzustand „Keine Treffer" |
| `s6-e5-zwei-zeichen-900x600.png` | Suchbegriff aus zwei Zeichen findet trotzdem |
| `s6-e6-operator-vor-s7-900x600.png` | `tag:backup` vor dem Parser aus S7 |
| `s6-e7-feld-geleert-900x600.png` | geleertes Feld, volle Liste wiederhergestellt |

Geändert wurde in diesem Lauf nur `wireframes/Denkzettel Wireframes.dc.html`
(der eine Absatz aus Punkt 1) sowie dieser Bericht samt Bildern. Kein
Produktivcode, kein Commit.
