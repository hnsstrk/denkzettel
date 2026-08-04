# Rückbau des Schätzkegels — Ausführungsbericht

**Datum:** 04.08.2026, 08:43 · **Ausführung:** Scrum Master · **Grundlage:**
Kundenentscheidung vom 04.08.2026, Weg 2 („entfernen") aus den drei Wegen in
`docs/scrum/sprints/sprint-06.md` §13.4.

**Sachgrund der Entscheidung** (nicht neu hergeleitet, sondern übernommen):
Der Kegel misst den **Revisionsfaktor** (Endwert ÷ Erstwert) über dem
Sprint-Abstand — also **wie oft neu geschätzt wurde**, nicht **ob richtig
geschätzt wurde**. Der tatsächliche Aufwand wird im Projekt nie erhoben; eine
Story ohne Revision steht bei 1,0, auch wenn sie das Doppelte kostete. Er kann
per Konstruktion nie belegen, dass eine Schätzung falsch war.

**Nicht committet** — der PO committet den Gesamtstand.

## 1. Geänderte und gelöschte Dateien

### `docs/scrum/PROZESS.md` (drei Streichungen, eine Ergänzung)

| Stelle (alte Zeilen) | Was | Neuer Stand |
|---|---|---|
| 62 | In den Pflichtteilen der Sprint-Protokolle stand „**DoD-Prüfung** samt Schätzhistorie" | Zusatz entfernt; die DoD-Prüfung bleibt Pflichtteil (Z. 62) |
| 238–261 | Absatz „Ebenfalls zur Sprint-Ende-Prüfung gehört die **Schätzhistorie** …" samt Tabellenspalten (Erstschätzung · Abstand · Faktor · Anlass-Kennzeichen) **und** dem darauffolgenden Absatz mit den beiden Begründungen für Abstand- und Faktor-Spalte | ersatzlos gestrichen; die DoD-Sektion geht vom Doku-Abgleich direkt zu „Prüfzeitpunkte: siehe ‚Sprint-Abschluss'" über |
| 369–395 | Sprint-Abschluss **Punkt 12** („Schätzhistorie fortgeschrieben", Datenreihe, Feld `stand`, Rollenteilung, „Was das Diagramm zeigt und was nicht") | ersatzlos gestrichen — **die Abschlussliste endet jetzt bei Punkt 11** |
| — (neu, Z. 122–133) | Gedächtnis-Vermerk in der **Sprint-Mechanik**, direkt hinter der Schätzregel | Wortlaut siehe Abschnitt 2 |

**Nummerierung geprüft.** Punkt 12 war der letzte der Liste; ihr Fortfall
verschiebt keine anderen Nummern. Ein Vorspann oder Querverweis mit der Zahl
(„zwölf Punkte") existiert nicht — geprüft mit
`git grep -n -E "Punkt 12|zw(ö|oe)lf"`; die verbleibenden Treffer außerhalb der
Protokolle sind `SPEC.md:540` („## 11. Volllauf-Schutz", unbeteiligt) und
`PROZESS.md:405` („seitdem sind sie die Punkte 9 und 10", unbeteiligt).
Der Ablageort des Vermerks ist die Sprint-Mechanik, weil dort die Schätzregel
steht und dort bereits der beendete PR-Probelauf als Vorbild in derselben Form
verzeichnet ist.

### `.claude/agents/denkzettel-verwalter.md`

Alte Zeilen 61–69: der Auftragspunkt „**Schätzhistorie fortschreiben und
Diagramm erzeugen**" (mechanische Übertragung nach `schaetzhistorie.json`,
Generatorlauf, Diff melden) ersatzlos entfernt. Die übrigen Auftragspunkte und
die Verbotsliste bleiben unverändert; der Punkt „Abgleichsberichte" rückt
unmittelbar hinter „Milestones schließen".

**`scrum-master.md`, `denkzettel-dev.md`, `denkzettel-ux.md`: nichts zu ändern.**
Geprüft mit `git grep -ni -E "kegel|sch(ä|ae)tzhistorie|diagramm|revisionsfaktor|anlass|erstsch|punkt 12" -- .claude/`
— einziger Treffer im ganzen Verzeichnis war `denkzettel-verwalter.md`.

### `docs/scrum/diagramme/` — gelöscht

`git rm -r docs/scrum/diagramme` (Löschung steht im Index). Vier Dateien:

- `LIESMICH.md`
- `kegel.py`
- `kegel.svg`
- `schaetzhistorie.json`

Vor der Löschung mit `git grep -n "diagramme"` auf Verweise geprüft; Ergebnis
in Abschnitt 3.

### `docs/scrum/sprints/sprint-06.md` §13.4 — einziger Protokolleingriff

Überschrift von „**Nicht** entschieden — der Schätzkegel (P4, Kundenfrage)" auf
„Entschieden — der Schätzkegel wird entfernt (P4, Kundenentscheidung)"; die
Wegeliste als vorgelegt (Vergangenheit) formuliert; angefügt ist der Block
„**Entscheidung des Kunden vom 04.08.2026: Weg 2 — entfernen**" mit der Liste
des tatsächlich Zurückgebauten und dem ausdrücklichen Hinweis, dass die
Protokolle unangetastet bleiben. Der übrige Text des Abschnitts steht
unverändert.

## 2. Wortlaut des Gedächtnis-Vermerks (`PROZESS.md`, Sprint-Mechanik)

> - **Schätzkegel: eingeführt am 02.08.2026, entfernt am 04.08.2026.** Ein
>   Diagramm samt Datenreihe (`docs/scrum/diagramme/`) trug den
>   **Revisionsfaktor** (Endwert ÷ Erstwert) über dem Sprint-Abstand zwischen
>   Erstschätzung und Umsetzung auf; daran hingen ein eigener Abschluss-Punkt,
>   ein DoD-Prüfsatz und eine Übertragungsregel für den Verwalter. Er maß damit,
>   **wie oft neu geschätzt wurde**, nicht **ob richtig geschätzt wurde** — der
>   tatsächliche Aufwand wird in diesem Projekt nicht erhoben, und eine nie
>   revidierte Story steht bei 1,0 auch dann, wenn sie das Doppelte kostete.
>   Dem Kunden lagen drei Wege vor (Aufwandserhebung nachrüsten · entfernen ·
>   unverändert lassen); er hat am **04.08.2026** das Entfernen gewählt. **Wer
>   dieses Werkzeug erneut vorschlägt, braucht zuerst die gemessene
>   Aufwandszahl je Story** — ohne sie kehrt dieselbe Lücke wieder.

## 3. Bewusst nicht angefasst

**Regeln werden zurückgebaut, Protokolle nicht.** `PROZESS.md` und die
Agentendateien sind geltende Anweisungen — dort verschwindet der Kegel. Die
Sprint-Protokolle sind **historische Berichte**: Sie haben damals richtig
protokolliert, was damals galt. Wer sie nachträglich glättet, zerstört die
Beweislage, auf der dieses Projekt seine Prüfungen führt. Unverändert bleiben
deshalb, obwohl sie den Kegel tragen:

| Datei | Stellen |
|---|---|
| `docs/scrum/sprints/sprint-05.md` | 1155–1168 (Punkt 12 als neu), **§24** ab 1162 (Anhang Schätzhistorie, Ausgangsbestand, Tabellen bis 1282), 1316, 1405, **§ Punkt 12** ab 1451 (Erstlauf), 1509, 1595–1606, 1643–1645 |
| `docs/scrum/sprints/sprint-06.md` | 20–22, 53–56, 687, **§9** ab 750 (Schätzhistorie der gezogenen Stories, bis 803), 914–915, 926 — alles außer §13.4 |
| `docs/scrum/reviews/2026-08-02-kegel-karpathy.md` | ganze Datei (Prüfbericht des karpathy-Laufs zum Kegel, u. a. Z. 4, 7, 55, 104, 143, 192, 213) — ein abgelegter Prüfbeleg, kein Regelwerk |

Ebenfalls nicht angefasst, weil außerhalb der Dateimenge: `README.md`,
`SPEC.md`, `KONZEPT.md`, `src/`, `tests/`, `CHANGELOG.md`.

**Nicht nachgezogen, gemeldet:** Die Kopfzeile von `PROZESS.md` trägt
weiterhin „Stand: 2026-08-02", obwohl die Datei am 04.08.2026 bereits zweimal
geändert wurde (CI-Entscheidung, Pflicht-/Kürteil) und heute ein drittes Mal.
Das ist ein Altbefund, kein Ergebnis dieses Rückbaus — die Entscheidung, ob die
Zeile fortgeschrieben oder gestrichen wird, liegt beim PO.

## 4. Meldung an den PO — Fundstellen außerhalb meiner Dateimenge

Melden, nicht heilen. Beide Stellen zeigen nach der Löschung von
`docs/scrum/diagramme/` ins Leere und **müssen** vom PO abgearbeitet werden:

1. **`README.md:128`** — eingebettetes Bild:
   `![Schätzkegel: Revisionsfaktor (Endwert ÷ Erstwert) über dem Abstand in Sprints zwischen Erstschätzung und Umsetzung; 9 Punkte, Stand Sprint 5](docs/scrum/diagramme/kegel.svg)`.
   Das Repo ist öffentlich; ein toter Bildverweis ist auf GitHub sofort
   sichtbar. **Dringlichkeit: hoch.**
2. **`README.md:134`** — Fließtextverweis „liegen in
   [`docs/scrum/diagramme/`](docs/scrum/diagramme/)" samt dem tragenden Satz
   davor. Auch hier zeigt der Link nach der Löschung auf einen nicht mehr
   existierenden Ordner.

Der PO hat sich `README.md` ausdrücklich vorbehalten, weil ein zweiter Strang
parallel neue Bilder dafür liefert.

**Keine Fundstelle im Produktivcode oder in der Build-Kette.** Geprüft mit
`git grep -n "diagramme"` über das ganze Repo: außer `README.md`, den
Protokollen, dem karpathy-Bericht und `PROZESS.md` (dort nur noch im
Gedächtnis-Vermerk als Nennung des ehemaligen Ortes) verweist nichts auf den
Ordner — insbesondere weder `CMakeLists.txt` noch `.github/workflows/ci.yml`.
Der Generator war ein freistehendes Python-Skript ohne Anbindung; sein Fortfall
kann keinen Bau und keinen Testlauf brechen.

## 5. Zustand des Arbeitsbaums bei Übergabe

```
 M .claude/agents/denkzettel-verwalter.md
 M docs/scrum/PROZESS.md
D  docs/scrum/diagramme/LIESMICH.md
D  docs/scrum/diagramme/kegel.py
D  docs/scrum/diagramme/kegel.svg
D  docs/scrum/diagramme/schaetzhistorie.json
 M docs/scrum/sprints/sprint-06.md
```

Umfang: 39 eingefügte, 69 entfernte Zeilen in den drei geänderten Dateien,
dazu die vier gelöschten Dateien des Ordners.
