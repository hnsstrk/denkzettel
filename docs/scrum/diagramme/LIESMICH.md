# Schätzkegel — Datenreihe und Generator

Der Schätzkegel zeigt, **wie stark Schätzungen revidiert wurden**, aufgetragen
über dem Abstand in Sprints zwischen Erstschätzung und Umsetzung.

## Aufruf

```
python3 docs/scrum/diagramme/kegel.py
```

Liest `schaetzhistorie.json`, schreibt `kegel.svg` daneben und meldet auf
stdout eine Bilanz (Punkte in der Kurve, erfasst und nicht gezeichnet,
Hüllwerte je Abstand). Python 3, nur Standardbibliothek — kein matplotlib,
kein numpy, keine Netzverbindung, keine Installation.

## Datenquelle

Einzige Quelle ist **`docs/scrum/sprints/sprint-05.md` §24** und ab Sprint 6
die entsprechende Tabelle im jeweiligen Sprint-Protokoll:

- §24.1 — die Reihe der Stories mit Gelegenheit zur Revision
- §24.2 — erfasst, aber nicht gezeichnet, mit Grund je Zeile
- §24.3 — die Zeilen des laufenden Sprints

## Die Regel

> **Werte kommen aus dem Sprint-Protokoll, nie aus den Issues abgeleitet.**

Das ist keine Stilfrage. Mangel **M2** (`sprint-05.md` §21) hat gemessen, dass
#66 und #67 **kein** `sp:`-Label tragen und #57 noch das veraltete `sp:2` — aus
den Labeln ergäbe sich für Sprint 5 ein Umfang von 3 statt 11 Punkten. Wer die
Reihe aus den Issues nachzieht, schreibt falsche Zahlen fort, und niemand
merkt es, weil das Bild plausibel bleibt. Solange M2 offen ist, ist die Spalte
„End" aus dem Protokoll zu ziehen und nicht aus dem Issue.

Das **Anlass-Kennzeichen** (`gegenstand-geändert` · `erkenntnis` · `keine`)
ist ein **Urteil**. Es fällt der Scrum Master in der DoD-Prüfung (Takt 1) und
steht dort im Sprint-Protokoll. Der `denkzettel-verwalter` überträgt es
mechanisch, lässt den Generator laufen und **meldet den Diff**; er trägt keine
eigene Zeile ein und ändert weder Wert noch Kennzeichen. Geht der Diff über
die neuen Zeilen hinaus, ist das ein Befund an den PO, keine Selbstheilung.
Der PO committet. (`PROZESS.md`, Sprint-Abschluss Punkt 12)

## Warum der Generator deterministisch ist

Gleicher Input → **bytegleiche** Ausgabe. Keine Zeitstempel, kein Zufall,
keine Systemabfragen, keine Bibliothek, deren Version in die Datei
durchschlägt. Der Verwalter fährt den Generator nach jedem Sprint, und der
**Diff ist das Prüfkriterium** — ein Bild, das sich bei gleichen Daten ändert,
macht jede Diff-Prüfung wertlos.

Beleg mit zwei Läufen:

```
python3 docs/scrum/diagramme/kegel.py && md5sum docs/scrum/diagramme/kegel.svg
python3 docs/scrum/diagramme/kegel.py && md5sum docs/scrum/diagramme/kegel.svg
```

Die Datenreihe führt den Faktor **zweimal**: wie im Protokoll gedruckt
(`faktor_protokoll`) und ableitbar aus Erst- und Endwert. Der Generator rechnet
nach und **bricht bei Abweichung ab**. Ein Zahlendreher wird so zum Abbruch
statt zu einem plausiblen falschen Bild.

## Farben

Jede Farbe in `kegel.py` ist gegen **Weiß und Schwarz** gemessen, weil GitHub
das README hell und dunkel rendert und die Datei bewusst **keinen eigenen
Hintergrund** hat. Grafik hält beidseitig 3:1, Text beidseitig 4.5:1 — daher
die gedeckte Palette: Eine Farbe, die auf Schwarz lesbar bleibt, kann nicht
zugleich dunkel sein. **Wer einen Wert ändert, misst ihn neu, beidseitig**,
und sieht sich die Datei auf beiden Hintergründen an:

```
rsvg-convert -b white docs/scrum/diagramme/kegel.svg -o /tmp/hell.png
rsvg-convert -b black docs/scrum/diagramme/kegel.svg -o /tmp/dunkel.png
```

## Was das Bild nicht behauptet

- Es misst **nicht** den Abstand zum tatsächlichen Aufwand; dieser wird im
  Projekt nicht erhoben. Eine Story, die niemand neu geschätzt hat, steht bei
  1,0, auch wenn sie teurer war.
- Die Hüllkurve ist **nicht-fallend, aber nicht streng wachsend**. Im
  Logarithmus sind Abstand 1 und 2 gleich weit (0,51 · 0,51 · 0,92); die
  Weitung tritt erst bei Abstand 3 ein und ruht dort auf einem einzigen Punkt.
  Achsentitel, Legende und Unterschrift dürfen nur das behaupten — die Sätze
  unter dem Bild sind deshalb **gerechnet, nicht getippt**: Der Generator
  bestimmt Form und Weitung aus den Daten und schreibt „streng wachsend" erst,
  wenn die Reihe es hergibt.
- Die 13 nicht gezeichneten Stories sind **keine** Treffer. Ihr Faktor ist 1,0
  von Konstruktion wegen, weil zwischen Erstschätzung und Umsetzung keine
  Gelegenheit zur Revision lag. Sie stehen in der Datenreihe, damit die
  Auslassung sichtbar bleibt und niemand sie später für ein Versehen hält.
