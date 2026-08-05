# Vorprüfbericht #72 — Tooltips mit Tastenkürzel

**Konsolidiert vom PO am 05.08.2026** aus `messung-a.md` (Bearbeiter A,
`denkzettel-dev`, Stand `80b52ae`) und `messung-b.md` (Bearbeiter B,
Scrum Master, Stand `6acc87e`). Beide Messungen entstanden unabhängig
voneinander; keiner der beiden Bearbeiter hat den Bericht des anderen gelesen.

**Ergebnis: `size:s`, ready — nach Nachschärfung der Kriterien durch den PO.**

---

## 1. Wo die beiden Messungen übereinstimmen

Beide kommen unabhängig auf **`size:s`**, beide auf **ready = nein** in der
Ausgangsfassung, und beide benennen dieselben drei tragenden Sachverhalte:

| Sachverhalt | A | B |
|---|---|---|
| „Rückgängig" ist eine `QAction` in einem `KMessageWidget`, kein Knopf | F2 | Falle 2 |
| „Strg+Z" kommt aus `KStandardShortcut::undo()` und ist eine Einstellung | F7 | Falle 1 |
| Die Meldungsfläche existiert nur nach einer Löschung | F9 | Falle 3 |
| Prüfmittel liegt im Bestand (`librarytest.cpp:3109–3124`) | Feld 4 | Feld 4 |
| Ein Agent kann den Tooltip nicht durch Verweilen auslösen | Feld 4 | Feld 4 |
| B21 nicht einschlägig | Feld 4 | Feld 4 |

Die Übereinstimmung ist hier ungewöhnlich vollständig. Das ist kein Zufall:
Die Story ist klein und ihre Fläche vollständig auslesbar.

## 2. Was nur **ein** Bearbeiter gefunden hat

Beide Funde stammen von A und beide sind tragend — sie hätten je einen
Fehlversuch gekostet:

**F6 — der Kürzeltext ist gebietsabhängig.** `QKeySequence::toString(NativeText)`
liefert unter `LANG=de_DE.UTF-8` `"Entf"` und `"Strg+Z"`, unter `LANG=C`
dagegen `"Del"` und `"Ctrl+Z"`. Der CI-Container ist `archlinux:base-devel`
**ohne gesetztes `LANG`** (`.github/workflows/ci.yml:43`). Eine Zusicherung auf
das Literal `"Entf"` wäre auf Ganymed grün und im öffentlichen Lauf rot.
*Beleg:* `messungen/sonde2-helfertext-offscreen.txt` gegen
`messungen/sonde3-locale-C.txt`.

**Dieser Fund traf die erste Nachschärfung des PO unmittelbar.** Meine Fassung
vom selben Tag nannte „Notiz löschen (Entf)" als Musterzeile und hätte damit
genau die Zusicherung nahegelegt, die im CI-Lauf rot wird. Die Kriterien
tragen deshalb jetzt den Satz, dass der Kürzelteil **an keiner Stelle als
Literal wiederholt** wird — auch nicht im Test.

**F3 — `QAction::toolTip()` ist nie leer.** Ohne eigenes `setToolTip()` gibt
die Aktion ihren `text()` zurück, heute `"Rückgängig"`. Eine Zusicherung der
Bauart „Tooltip ist nicht leer" wäre an dieser Fläche **ohne jede Änderung
grün**. Das ist die Bauart „ein Testaufbau, in dem der Fehler gar nicht
auftreten kann" aus `CLAUDE.md` — gefunden, bevor er gebaut wurde.
*Beleg:* `messungen/sonde1-tooltipquellen-offscreen.txt`, Abschnitt A.

**F4/F5 — der KDE-Automatikweg existiert und trägt genau eine der drei
Flächen.** `KToolTipHelper` (KF6::XmlGui) hängt das Kürzel selbsttätig an,
greift aber nur an `QToolButton` mit `setDefaultAction()`. `LibraryWindow` ist
zudem ein `QWidget`, kein `KMainWindow` — der Filter kommt nicht von selbst.

## 3. Die sechs Felder

**Feld 1 — Dateimenge.** Beide Messungen decken sich; A ist genauer.

| | |
|---|---|
| **Quellen und Tests** | `src/ui/librarywindow.cpp` — drei kleine Stellen: Aktionserzeugung `:168–176`, Kürzelblock `:249–278`, Aufbau der beiden Schaltflächen `:361–369`. Die Meldungszeile `:234` wird **gelesen, nicht geändert**.<br>`tests/librarytest.cpp` — Slotliste im Klassenkopf und neue Zusicherungen im Muster von `:3109–3124`. |
| **Build** | **nichts** (Folge der PO-Entscheidung gegen `KToolTipHelper`, siehe unten) |
| **Belege und Prüfmittel** | keine neuen; die Sonden dieser Vorprüfung liegen als Bauplan bereit |
| **Fachliche Quellen** | `SPEC.md` Abschnitt 15 — `i18n()` und die unpersönliche Infinitivform. **Lesen, nicht ändern** |
| **Ausdrücklich nicht** | `src/shell/trayicon.cpp` (eigene Tooltips, andere Story) · `tests/libraryshots.cpp` (`grab()` zeigt einen Tooltip nie) · in `librarywindow.cpp` die Bereiche `:517–535` und `:700–800` — **das ist #71** · `wireframes/` · `SPEC.md` |

**Feld 2 — gemessene Fallen.** Vier wandern in den Spawn-Auftrag: die
Gebietsabhängigkeit des Kürzeltextes (F6), die nie leere `QAction::toolTip()`
(F3), „Rückgängig" als Aktion statt Knopf (F2/Falle 2), und die Meldungsfläche,
die erst hergestellt werden muss (F9/Falle 3).

**Feld 3 — AK-Urteil.** Ausgangsfassung **nicht ready** (Urteil B, gedeckt
durch A). Der Kopf trug „Akzeptanzkriterien (Entwurf)"; inhaltlich fehlten das
Prüfmittel für den Kürzeltext, die Aussage über das Ankommen an der Fläche und
eine Musterzeile für den Wortlaut.
**Behoben durch den PO am 05.08.2026:** sechs Kriterien, jedes gegen eine
gemessene Falle geschnitten. **Damit ready.**

**Feld 4 — Prüfmittel.** `QWidget::toolTip()` bzw. `QAction::toolTip()` im
QTest für den hinterlegten Text; ein zugestelltes `QHelpEvent(QEvent::ToolTip)`
mit anschließendem `QToolTip::text()` für den angezeigten Text — beides
offscreen gemessen und tragfähig.
**Grenze:** Ein Agent kann den Tooltip nicht durch Verweilen auslösen; unter
Wayland bewegt ein Prozess den Zeiger nicht. Belegbar ist, *was* ein
zugestelltes Ereignis erzeugt, nicht *dass* der Zeiger es erzeugt.

**Feld 5 — Größenklasse: `size:s`.** Beide Bearbeiter unabhängig, ohne
Abweichung. B vermerkt ausdrücklich: „Von den fünf Kandidaten dieses Laufs ist
#72 der einzige, bei dem die Einstufung ohne Vorbehalt trägt."
Die Bedingung, unter der es `m` würde, ist durch die PO-Entscheidung gegen
`KToolTipHelper` **entfallen**.

**Feld 6 — offene Fragen: alle vom PO entschieden.**

| Frage | Entscheidung |
|---|---|
| Handarbeit oder `KToolTipHelper`? | **Handarbeit.** Drei `setToolTip()`-Zeilen gegen eine neue Bibliothek, einen app-weiten Ereignisfilter und den Umbau zweier Knöpfe. Die Automatik zahlt sich erst bei vielen Flächen aus |
| Kürzeltext aus `toString(NativeText)` statt Literal ins AK? | **Ja**, und schärfer als vorgeschlagen: an keiner Stelle als Literal, auch nicht im Test |
| Wortlaut der drei Tooltips | Musterzeile `<Tätigkeit im Infinitiv> (<Kürzel>)`, zusammengesetzt über einen Platzhalterstring |
| „Speichern" und „Abbrechen" mit aufnehmen? | **Nein.** Der UI-Review-Befund H1 nennt sie nicht — bewusste Auslassung, eigene Story, wer sie will |
| Ist #72 eine UI-Story im Sinne von DoD 3? | **Nein.** Ein UI-Review kann hier kein eigenes Bild erzeugen, und ein UI-Review ohne eigene Bildprüfung zählt für DoD 3 nicht. Die Story als UI-Story zu führen, eröffnete einen nie erfüllbaren Punkt |
| Reihenfolge gegen #71 | Gemeinsamer Strang — beide schreiben in `librarywindow.cpp` und in die Slotliste von `librarytest.cpp`. Siehe Sprint-7-Planning |

## 4. Was dieser Lauf über das Verfahren sagt

Die beiden Messungen sind zu **fast identischen** Ergebnissen gekommen —
sechs Sachverhalte deckungsgleich, Größenklasse identisch, Ready-Urteil
identisch. Wer daraus schließt, die zweite Messung habe nichts getragen,
liest die Tabelle in Abschnitt 2 nicht: **Drei der vier tragenden Fallen
stehen nur in einem der beiden Berichte**, und eine davon hat einen bereits
geschriebenen PO-Text als falsch erwiesen. Die Übereinstimmung liegt in den
Feldern, die man aus dem Code abliest; die Abweichung liegt in dem, was man
misst.
