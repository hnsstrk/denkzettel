# Bewertung der 23 prüfsummengleichen Bildgruppen

**Datum:** 04.08.2026 · **Bewertung:** Product Owner · **Anlass:** Kundenentscheidung
vom 04.08.2026, den Altbestand einmalig durchzusehen, nachdem V2
(`docs/scrum/bildbelege-pruefen.sh`) ihn sichtbar gemacht hat.

**Erhebung:** `bash docs/scrum/bildbelege-pruefen.sh` — 260 versionierte Bilder,
**23 Gruppen, 58 Dateien tragen 23 Aufnahmen**. Verglichen wird ausschließlich
innerhalb desselben Ordners.

**Maßstab:** Ein Zustand, der nach einer Rückkehr wieder derselbe ist, darf
zweimal gleich aussehen. Ein Befund ist es, wo zwei **verschiedene** Zustände
denselben Beleg tragen oder wo die Zahl der Dateien die Zahl der Messungen
überzeichnet.

---

## Ergebnis in einem Satz

**Von 23 Gruppen sind 18 berechtigt, 5 sind Mängel** — und alle fünf sitzen in
**zwei** Ordnern. Kein einziger Mangel betrifft eine inhaltliche Aussage eines
Prüfberichts; alle fünf betreffen die **Belegführung**.

---

## Der wichtigste Befund ist ein Freispruch

**Der Sprint-4-UI-Review hat seine eigenen fünf Dubletten bereits erkannt und
begründet** (`sprint-04-s8-ui-review/bericht.md:385–398`) — vier Monate bevor es
ein Prüfskript gab, von Hand, und mit der schärferen Aussage:

> `n01` = `n03`: der Lesezustand vor dem Bearbeiten und der nach dem Abbrechen.
> Pixelgleich heißt hier, dass der Rückweg aus dem Editor wirklich im
> Ausgangszustand landet — **der schärfste Beleg zu Stelle 1, schärfer als die
> Messung y = 102.**

> `n06` = `n07`: … Der Unterschied zwischen den beiden Antworten liegt also **im
> Speicher, nicht im Bild** — gemessen ist er in der Tabelle unter Stelle 2.

Das ist die Umkehrung dessen, was ich beim Öffnen der Liste erwartet hatte:
`n06`/`n07` sah nach dem gefährlichsten Fall aus (Verwerfen gegen Speichern
*muss* sich unterscheiden), und der Bericht hatte die Frage längst beantwortet.
**Prüfe am Einzelfall, nicht an der Plausibilität** — hier gegen mich selbst.

## Die zweite Lehre: Bewegungstests erzeugen berechtigte Dubletten

`ak7-n4` = `ak7-n10` sah nach einem harten Fall aus, weil der Bericht zwei
verschiedene Szenarien beschreibt. Ich habe das Bild angesehen: Es zeigt Kopf
„Gestern" über drei Notizen, die letzte ausgewählt und vollständig sichtbar —
**genau das, was der Bericht für N4 *und* für N10 beschreibt.** Beide Wege enden
im selben Zustand.

Nach der eigenen Regel des Projekts ist das kein Mangel, sondern zu erwarten:

> Bei Bewegungen ist der **Weg** der Prüfgegenstand, nicht das Ziel.

Beide Fälle sind an **Rollwerten** entschieden, nicht am Endbild; die Bilder sind
Beiwerk. Dasselbe gilt für `s5-57-b1b`/`b6` und `s5-57-b3`/`b4`.

**Folge für V2:** Das Skript wird in Ordnern mit Bewegungstests **dauerhaft**
anschlagen. Das ist kein Fehlalarm, aber es muss beim Bewerten mitgedacht
werden, sonst wird aus der Wache eine Abhakübung.

---

## Die 23 Gruppen einzeln

| # | Ordner | Gruppe | Urteil |
|---|---|---|---|
| 1–5 | `2026-08-01-capture-theme` | `achse3-huelle-*` (3×, 3×, 4×, 5×, 7×) | **Mangel** — siehe unten |
| 6 | `2026-08-01-capture-theme` | `capture-leer-BreezeDark` = `wechsel-1-vorher` | berechtigt, Benennung überzeichnet |
| 7 | `2026-08-01-capture-theme` | `capture-leer-BreezeLight` = `wechsel-3-neu-gebaut` | berechtigt, Benennung überzeichnet |
| 8 | `2026-08-01-capture-theme` | `variante-A-ist` = `wechsel-2-nachher` | berechtigt, Benennung überzeichnet |
| 9–11 | `s54-theme-treue` | `*-1-start-dunkel` = `*-3-zurueck-dunkel` (3×) | **berechtigt** — Rückkehr in denselben Zustand; das *ist* die Aussage |
| 12 | `s6-suche` | `1-volle-liste` = `6-feld-geleert-volle-liste` | **berechtigt** — geleertes Feld stellt die volle Liste wieder her |
| 13 | `sprint-03-s5a-ak7-nachpruefung` | `ak7-n4` = `ak7-n10` | **berechtigt** — zwei Wege, ein Endzustand (Bild selbst geprüft) |
| 14 | `…/nach-der-warn-heilung` | dieselbe Paarung | **berechtigt**, dito |
| 15 | `sprint-03-s6-trefferliste` | `s6-e0-volle-liste` = `s6-e7-feld-geleert` | **berechtigt** — wie 12 |
| 16 | `sprint-04-s8-ui-review` | `01-lesen` = `17-sprung-lesen` | **berechtigt** — im Bericht begründet |
| 17 | `sprint-04-s8-ui-review` | `03-bearbeiten-frisch` = `16-sprung-bearbeiten` | **berechtigt** — im Bericht begründet |
| 18 | `sprint-04-s8-ui-review` | `n01-lesen` = `n03-lesen-nach-abbrechen` | **berechtigt** — ausdrücklich als *Beleg der Heilung* geführt |
| 19 | `sprint-04-s8-ui-review` | `n04-waechter-abbrechen` = `n05-waechter-auswahlwechsel` | **berechtigt** — derselbe Dialog aus zwei Anlässen; im Bericht behandelt |
| 20 | `sprint-04-s8-ui-review` | `n06-nach-verwerfen` = `n07-nach-speichern` | **berechtigt** — Unterschied liegt im Speicher, in der Tabelle gemessen |
| 21 | `sprint-05-installationstakt` | `bibliothek-installiert` = `schema-dunkel-installiert` | **Mangel** — bereits in Sprint 5 als V2 gefunden, bis heute offen |
| 22 | `sprint-05-ui-review/bilder` | `b1b-nach-dem-klick` = `b6-nach-dem-loslassen` | **berechtigt** — dass sich nichts bewegt, *ist* die Aussage von #57 |
| 23 | `sprint-05-ui-review/bilder` | `b3-taste-nach-klick` = `b4-taste-nach-kopfklick` | **berechtigt** — Bewegungstest, an Rollwerten entschieden |

---

## Die fünf Mängel

### M-A · `achse3-huelle-*`: 23 Dateinamen für 6 Aufnahmen (Gruppen 1–5)

Der Bericht `2026-08-01-capture-theme-treue.md:121` sagt: *„Die gerenderten
Hüllen unterscheiden sich sichtbar in Radius, Deckkraft und Farbe."*

**Die Aussage bleibt richtig** — sechs verschiedene Hüllen unterscheiden sich
tatsächlich. Falsch ist die **Menge**, die die Dateinamen behaupten. Zwei
Einzelheiten:

1. **Das Farbschema ändert die Hülle nicht.** Je Desktop-Theme sind alle drei
   Schema-Varianten bytegleich. Das ist ein **Ergebnis**, das der Bericht nicht
   ausspricht: Die Hülle folgt dem Desktop-Theme, nicht dem Farbschema. Es stützt
   Zeile 124 (*„Die Theme-Hülle geht mit"*) — steht dort aber nicht.
2. **`cachyos-emerald`, `-color` und `-light` liefern eine einzige Hülle** (7
   Dateien). Für dieses Tripel trägt der Satz „unterscheiden sich sichtbar"
   nicht.

### M-B · Ein Dateiname ist das Produkt eines Skriptfehlers (in Gruppe 3 und 4)

```
achse3-huelle-default breeze-light breeze-dark cachyos-emerald
cachyos-emerald-color cachyos-emerald-light CachyOS-Nord-round
Iridescent-round-BreezeDark.png
```

Eine nicht in Anführungszeichen gesetzte Schleifenvariable hat sämtliche
Theme-Namen in **einen** Dateinamen geschrieben. Zwei solcher Dateien liegen im
Ordner. Sie sind bytegleich mit den `-default-`-Aufnahmen, tragen also keine
eigene Messung — aber sie zeigen, dass der Messläufer fehlerhaft war und niemand
die Dateiliste danach angesehen hat.

### M-C · `bibliothek-installiert` = `schema-dunkel-installiert` (Gruppe 21)

Zwei verschiedene Nachweise des Installationstakts aus **einer** Aufnahme. In
Sprint 5 von Hand gefunden und als V2 protokolliert (*„Zwei Belege sind erst
zwei, wenn sie zwei Aufnahmen sind"*), mit dem Vorschlag, eine Zeile
nachzutragen oder die Doppelung zu löschen. **Bis heute nicht geschehen.**

---

## Empfehlung an den Kunden

**Nichts davon ist dringend, und keine inhaltliche Aussage fällt.** Die Mängel
betreffen die Belegführung, nicht die Befunde.

1. **M-C** ist mit einer Zeile Nachtrag erledigt und seit Sprint 5 offen —
   sollte beim nächsten Sprint-Abschluss mitlaufen.
2. **M-A und M-B** betreffen einen abgeschlossenen Bericht. Vorschlag: einen
   datierten Nachtrag anhängen (*23 Dateinamen tragen 6 Aufnahmen; das Farbschema
   ändert die Hülle nicht; ein Dateiname stammt aus einem Skriptfehler*), statt
   Dateien zu löschen — der Bericht ist Beweislage und wird nicht geglättet.
3. **Für V2 künftig:** Ordner mit Bewegungstests schlagen dauerhaft an. Das ist
   kein Fehlalarm; das Bewerten bleibt Kopfarbeit. Wer die Bewertung mechanisiert,
   verliert genau die Unterscheidung, die diesen Durchgang trägt.

## Was diese Durchsicht über V2 selbst sagt

**Der Nutzen ist kleiner, als der erste Lauf vermuten ließ, und liegt woanders
als erwartet.** 18 der 23 Gruppen sind berechtigt, und fünf davon hatte ein
Prüfbericht bereits von Hand erkannt und begründet. Das Skript findet also
überwiegend, was gute Reviews schon fanden.

Sein Wert ist ein anderer: Es findet sie **vollständig und in einer Sekunde**,
und es hätte M-A gefunden — den Fall, den kein Mensch gefunden hat, weil niemand
23 Dateinamen gegen 6 Prüfsummen hält. Die Stop-Bedingung aus dem
Sprint-Abschluss (drei Sprints ohne neue Gruppe → Stichprobe) ist damit richtig
angesetzt.
