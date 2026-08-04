# Vorprüfung #72 — Messung B (Scrum Master)

Gemessen am 2026-08-04, 20:30–20:38 CEST, Stand `6acc87e`. Unabhängig von
Bearbeiter A; dessen Messung war zum Zeitpunkt dieser Arbeit nicht gelesen.

Messbeleg: `messung-b-bestand.txt` in diesem Ordner.

## Feld 1 — Dateimenge (so weit ohne A gemessen)

| | **#72** |
|---|---|
| **Quellen und Tests** | `src/ui/librarywindow.cpp` — drei Stellen: `m_editButton` `:361`, `m_deleteButton` `:368`, `m_undoAction` `:254` (bzw. `:173`).<br>`tests/librarytest.cpp` — Slot-Liste im Klassenkopf und ein Fall im Muster von `:3109–3124`. |
| **Belege und Prüfmittel** | keine neuen; `toolTip()` wird im Bestand bereits zugesichert. |
| **Fachliche Quellen** | `SPEC.md:735–738` (Sprachregel, Infinitivform) — **lesen, nicht ändern**. |
| **Ausdrücklich nicht** | `src/shell/trayicon.cpp` (trägt eigene Tooltips, andere Story), `tests/libraryshots.cpp`. |

## Feld 2 — Fallen (Teilmessung)

1. **„Rückgängig" hat kein festes Kürzel im Code.** `:254` setzt
   `m_undoAction->setShortcuts(KStandardShortcut::undo())` — eine Liste aus der
   Benutzereinstellung, nicht `Strg+Z`. Ein Tooltip, der „Strg+Z" hart
   hineinschreibt, ist eine zweite Quelle für einen Wert, den die erste jederzeit
   ändern kann. Die drei Kürzel sind unterschiedlich gebunden, gemessen:
   `m_editAction` → `Qt::Key_F2` (`:262`), `m_deleteAction` →
   `QKeySequence::Delete` (`:249`), `m_undoAction` → `KStandardShortcut::undo()`
   (`:254`).
2. **„Rückgängig" ist kein Knopf, sondern eine `QAction` in einem
   `KMessageWidget`** (`:234`, `m_message->addAction(m_undoAction)`). Ob der
   Tooltip einer Action an der Fläche ankommt, die `KMessageWidget` daraus baut,
   ist eine Messung, keine Annahme — anders als bei den beiden `QPushButton`s,
   die direkt `setToolTip()` nehmen.
3. **Die Meldungsfläche ist nicht immer da.** `m_undoAction` steht nur nach einer
   Löschung zur Verfügung (`:255` setzt sie auf `disabled`, `:296` schaltet sie
   frei). Ein Test muss den Zustand erst herstellen.

## Feld 3 — AK-Urteil: **ready = nein**

**Der Kopf sagt es selbst:** „Akzeptanzkriterien (Entwurf)". Der DoR-Zusatz vom
04.08.2026 greift, und inhaltlich fehlen drei Dinge:

1. **AK 1 schreibt „Strg+Z" fest, der Code tut das nicht** (Falle 1). So
   formuliert verlangt das Kriterium eine doppelte Pflege. Prüfbare Fassung:
   *„…nennt die tatsächlich gebundene Tastenfolge, aus der Aktion gelesen
   (`QAction::shortcut()`), nicht als Text wiederholt."*
2. **Kein Kriterium sagt, dass der Tooltip an der Fläche ankommt** (Falle 2).
   Für die beiden Knöpfe ist das trivial, für „Rückgängig" nicht.
3. **Der Wortlaut ist offen.** AK 2 verweist auf die Sprachregel — die
   existiert und ist prüfbar (`SPEC.md:735–738`, „unpersönliche Infinitivform",
   PO-Entscheidung 31.07.2026) —, aber „mit Kürzel-Nennung" legt die Form nicht
   fest. „Bearbeiten (F2)" und „Zum Bearbeiten F2 drücken" erfüllen beide den
   Wortlaut des Kriteriums und sehen verschieden aus. Bei drei Tooltips lohnt
   eine Musterzeile im Issue.

Sonst ist die Story in guter Form: die drei Flächen sind benannt, das
Prüfmittel existiert im Bestand (`librarytest.cpp:3109–3124` sichert bereits
`toolTip()` des Suchfelds zu, in beiden Richtungen), B21 ist nicht einschlägig.

**Behebung (PO):** drei Sätze, keine Messung nötig.

## Feld 4 — Prüfmittel, und was ein Agent nicht prüfen kann

**Vorhanden:** `widget->toolTip()` gegen den erwarteten Text, Muster
`:3109–3124`. Das deckt alle drei Flächen ab, sobald die Fläche zu greifen ist.

**Grenze, ausgesprochen:** Ein Agent kann den Tooltip nicht **erscheinen**
lassen und ihn nicht im Bild belegen — ein Tooltip ist ein eigenes Fenster, das
der Zeiger auslöst, und offscreen gibt es keinen Zeiger, der über einer Fläche
verweilt. Was prüfbar ist, ist der hinterlegte Text; was nicht prüfbar ist, ist
sein Erscheinen. Das ist keine Lücke der Story, sondern eine Grenze, die im
Bericht stehen muss.

## Feld 5 — Größenklasse: **`size:s`**

Drei `setToolTip()`-Aufrufe in einer Datei, ein Testfall im vorhandenen Muster,
keine neue Datei, keine neue Abhängigkeit, kein neuer Prüfweg, keine
SPEC-Änderung (die Sprachregel wird gelesen, nicht geschrieben). Von den fünf
Kandidaten dieses Laufs ist #72 der einzige, bei dem die Einstufung ohne
Vorbehalt trägt.

## Feld 6 — Offene Fragen

**An den PO:**

1. **Ist #72 eine UI-Story im Sinne von DoD 3?** Der PO legt das beim Planning
   fest. Nach Feld 4 kann ein UI-Review hier kein eigenes Bild des Tooltips
   erzeugen; ein UI-Review ohne eigene Bildprüfung zählt für DoD 3 nicht. Wird
   #72 als UI-Story geführt, ist die Belegform vorher zu klären — sonst steht am
   Sprint-Ende ein Punkt offen, der nie erfüllbar war.
2. **Musterzeile für den Wortlaut** (Feld 3, Punkt 3). Eine reicht für alle
   drei.
3. **Gemeinsamer Strang mit #71.** Beide schreiben in `librarywindow.cpp` und in
   die Slot-Liste von `librarytest.cpp`. Der Textabstand ist groß, die Naht
   liegt im Klassenkopf des Tests.

**Kundenentscheidung:** keine offen.
