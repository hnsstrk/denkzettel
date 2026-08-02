# karpathy-Review Sprint 4 — PR #65, PR #64 und main-Artefakte seit `sprint-04-basis`

**Datum:** 02.08.2026 · **Anlass:** DoD 3 am Sprint-Ende · **Prüfer:** Agent
`karpathy-reviewer` (frischer Kontext)
**Prüfgegenstand:** PR #65 (`story/11-bearbeiten`, 9 Commits), PR #64
(`story/60-traymenues`, 5 Commits), beide gegen `sprint-04-basis`; dazu die
main-seitigen Artefakte `475874c`, `8f6d20c`, `433e87e`, `2e6aebe`, `95bcab6`.
Beide Diffs vollständig gelesen; Tests in beiden Arbeitsbäumen ausgeführt
(librarytest 1/1, shelltest 1/1 grün) und mit **drei gezielten Mutationen**
auf Schärfe geprüft — alle drei wurden rot, die Arbeitsbäume sind danach
wieder sauber (`git status` leer).

**Task:** Sprint-4-Diff gegen die vier Karpathy-Prinzipien prüfen; ohne
offene fail-Befunde ist die DoD erfüllt.

**Gesamt-Verdikt: `fail`** — ein einzelner, präzise umrissener fail-Befund
(Prinzip 1, eine Kommentarpassage in PR #65), vor dem Merge mit einer Zeile
behebbar. Alles andere ist `ok` oder `warn`.

| Prinzip | Verdikt |
|---|---|
| 1 — Think Before Acting | **`fail`** (1 Befund) |
| 2 — Simplicity First | `ok` |
| 3 — Surgical Changes | `warn` |
| 4 — Goal-Driven Execution | `warn` |

---

## Prinzip 1 — Think Before Acting

**Verdikt: `fail`**

**Befund 1.1 (fail) — Ein Test-Kommentar behauptet die Ursache, die derselbe
Commit als widerlegt ausweist.** PR #65, `tests/librarytest.cpp:2964–2967`
(neu in `60cae75`):

> „The stack that fixed finding 1 made them grow: its editing page carries a
> stretch of its own, which turned the whole stack horizontally expanding,
> and the buttons inside it took the surplus"

Das ist die Ursachenangabe der Prüferin aus Befund 7 — und exakt die, die die
Commit-Botschaft von `60cae75` selbst als gemessen widerlegt benennt („Nimmt
man das addStretch() der Bearbeiten-Seite heraus, bleiben die Knöpfe exakt
gleich breit … Nicht der Zwischenraum will die Breite, sondern der Stapel
selbst"). Der Code-Kommentar `src/ui/librarywindow.cpp:336–339` trägt die
richtige, gemessene Fassung; der Test-Kommentar im selben Commit die falsche.

Warum fail und nicht warn: Das Projekt hat diese Fehlerklasse selbst zur
Risikoklasse erklärt — `646804a` existiert nur, um zwei solcher Kommentare zu
tilgen, mit der Begründung „eine Begründung, die trägt und trotzdem das
Falsche behauptet, wird beim nächsten Aufräumen zur Fehlentscheidung". Wer
später am Test aufräumt und dem Kommentar glaubt, entfernt das addStretch()
oder hält es für breitenwirksam — beides gemessen falsch.

**Befund 1.2 (warn) — Der Zweig-Beleg zu Befund 5 trägt die überholte
Ursache weiter.** PR #65,
`docs/scrum/reviews/sprint-04-s8-bearbeiten/LIESMICH.md:57 ff.` („Grenze
dieser Bildstrecke"): Die Passage erklärt die symbollosen Dialogknöpfe mit
„`QIcon::fromTheme()` liefert … `null`" und schließt „Das gehört nachgeprüft,
sobald jemand einen Bildlauf mit auflösendem Symbolthema fahren kann." Die
Nachprüfung auf main (`95bcab6`, `sprint-04-s8-ui-review/bericht.md`,
Stelle 5) misst das Gegenteil (fromTheme löst auf, 7 Größen) und isoliert die
tatsächliche Ursache: Unter `QT_QPA_PLATFORMTHEME=kde` ersetzt die
KDE-Plattformintegration den gebauten `QMessageBox` durch einen Dialog mit
eigenen Knopfobjekten, der nachträglich gesetzte Symbole nicht übernimmt —
ein auflösendes Symbolthema würde also nichts ändern. Der Strang hat dieselbe
Nachprüfung nachweislich verarbeitet (`60cae75` heilt ihren Befund 7), den
eigenen Beleg aber nicht nachgezogen. Zwei versionierte Belege widersprechen
sich jetzt; der im Zweig schickt künftige Leser auf die falsche offene Frage.

**Beobachtungen ohne Verdikt:**

- Der Fehlerpfad des Speicherns (`saveEdit()`, Store-Fehler) endet in einem
  `qWarning` ohne sichtbare Rückmeldung; Editor und Text bleiben erhalten.
  Das ist deckungsgleich mit dem Erfassungsfenster
  (`capturewindow.cpp:118–136`) und im Kommentar als Konvention benannt —
  kein Befund, nur festgehalten, dass die Konvention geprüft wurde und trägt.
- Strang B arbeitet vorbildlich nach Prinzip 1: SNI-Messung **vor** der
  Bauentscheidung (erster Story-Schritt, Rückfallregel griff wie geplant);
  Meta+N-Nichtdopplung „gemessen, nicht angenommen"
  (`kuerzel-nachpruefung.txt`: genau ein Eintrag für `show-capture`); die
  Grenze der noch installierten alten Desktop-Datei ist benannt statt
  beschönigt, samt Nachprüfkommandos für den Installationstakt.

## Prinzip 2 — Simplicity First

**Verdikt: `ok`** — die drei benannten Kandidaten tragen jeweils einen realen
Fall, keine Eventualität:

- **`DialogWatch`** (`tests/librarytest.cpp:39–74`): trägt einen gemessenen
  Vorfall — ohne sie hing die Suite im `exec()` des fälschlich erschienenen
  Dialogs statt rot zu werden (Kommentar nennt Datum und Hergang). Zweifach
  genutzt, ~30 Zeilen, kein Ausbau auf Vorrat.
- **`QStackedWidget`-Kopf** (`m_headPages`,
  `src/ui/librarywindow.cpp:308–341`): trägt den gemessenen 19-px-Sprung
  (UI-Review Befund 1); die billigere Alternative (Mindesthöhe einmal
  kopieren) ist im Kommentar mit der #54-Lehre begründet verworfen, nicht
  stillschweigend.
- **`NoteListModel::replaceNote()`** (`src/ui/notelistmodel.cpp:154–164`):
  trägt AK K2 (Notiz bleibt in der laufenden Trefferliste); Mutation 1
  bestätigt, dass zwei Tests genau daran hängen.

Beobachtungen: `keepsTheQuitApartInTheLastGroup` überlappt mit dem
Strukturtest (Reihenfolge und letzter Eintrag doppelt geprüft) — im README
als bewusst deklariert („hält die Eigenschaft künftig fest"), kosmetisch. Die
hartkodierte Splitterbreite 300 in `keepsTheMeasuresOfTheEditState`
(`librarytest.cpp:3050`) folgt dem bestehenden Muster (`:2045`), kein neuer
Stil.

## Prinzip 3 — Surgical Changes

**Verdikt: `warn`**

- **Die Codeflächen sind strikt getrennt** — sauberster Punkt des Sprints:
  #11 ausschließlich `src/ui/*` + `librarytest`, #60 ausschließlich
  `src/shell/*` + `desktop` + `shelltest`; die SPEC-Änderungen liegen
  disjunkt in §9 bzw. §10. Der `sprint-04.md`-Hunk in beiden Diffs ist der
  gemeinsame Basis-Commit `b67a348`, kein Strang-Drift.
- **Befund 3.1 (warn) — Beide Stränge schreiben außerhalb der in
  Planning 4.3/B13 notierten Dateimenge:** Strang A legt `tests/editshots.cpp`
  neu an und ändert `tests/CMakeLists.txt`; Strang B ändert
  `tests/CMakeLists.txt` (ENV-Zeile shelltest); beide schreiben unter
  `docs/scrum/reviews/<story>/`. Jede dieser Zeilen ist auf eine Pflicht
  rückführbar (DoD 3: eigene Bilder; B7: versionierte Belege; die
  Icon-Zusicherungen brauchen die Testumgebung) — der Mangel liegt in der zu
  engen Dateimengen-Notation des Plannings, nicht in Eigenmacht der Stränge.
  In `tests/CMakeLists.txt` fasst jeder nur den eigenen Block an; der Merge
  beider PRs erzeugt dort zwei getrennte, konfliktfreie Hunks. Fix gehört in
  den Prozess, nicht in den Code (siehe Vorschläge).
- Die Kommentar-Ergänzungen an `desktop/org.denkzettel.Denkzettel.desktop`
  und `globalshortcuts.cpp` (warum `Keywords=` und die Aktions-Id bleiben)
  sind auf die AK-Festlegungen von #60 rückführbar — kein Nebenbei-Verbessern.
- **Gemeldet, nicht geheilt:** SPEC §2/§3 sprechen weiter vom
  „Capture-Fenster" (Modul- und Architektursprache), während die
  Meldungstexte jetzt „Erfassungsfenster" sagen. Das liegt außerhalb des
  #60-Umbenennungsumfangs (nur sichtbare Texte; Ids ausdrücklich nicht) und
  ist korrekt nicht angefasst worden — die Wortfamilien-Divergenz in der SPEC
  sollte der PO aber kennen.

## Prinzip 4 — Goal-Driven Execution

**Verdikt: `warn`**

**Testschärfe — stichprobenhaft per Mutation belegt** (Build im jeweiligen
Arbeitsbaum, Änderungen danach zurückgenommen, `git status` sauber):

| Mutation | Erwartung | Ergebnis |
|---|---|---|
| `replaceNote()` → No-op (`notelistmodel.cpp`) | K2-Tests rot | **rot**: `savesTheChangedTextWithTheButton`, `keepsTheSavedNoteInTheResultListUntilTheSearchChanges` |
| Auswahl-Rücknahme in `showNote()` deaktiviert (`librarywindow.cpp:610–614`) | Dialog-Auswahl-Tests rot | **rot**: `keepsTheSelectionOnTheEditedNoteWhileTheDialogAsks`, `asksBeforeUnsavedChangesAreLost(auswahlwechsel-abbrechen)` |
| `WidgetShortcut` → `ApplicationShortcut` (`trayicon.cpp:80`) | Kürzel-Test rot | **rot**: `hintsTheShortcutWithoutBindingItASecondTime` |

Keine der geprüften Zusicherungen ist tautologisch. Die neuen Tests prüfen
zudem ihre eigenen Vorbedingungen („Die Zeile ist gar nicht verrutscht",
„Breeze fehlt — SPEC 10 holt die Menü-Symbole aus dem Breeze-Bestand") — die
Gegenmaßnahme aus Sprint 3 wird fortgeführt.

**AK-Deckung:** #11: alle elf AK haben Test und/oder Bild; die 3×3-Matrix
`asksBeforeUnsavedChangesAreLost` deckt jeden Auslösepfad mit jeder Antwort.
#60: Struktur, `GetLayout` und Meta+N-Kette belegt; die AK-Änderungen beider
Issues sind protokollierte Kundenentscheidungen (sprint-04.md §14; #11 K1–K3),
kein stiller Scope-Drift.

**Befund 4.1 (warn) — Drei benannte Loose Ends, alle am installierten Stand
zu schließen:** (a) #60: die Panel-Fotos je Maustaste fehlen (AK) — die
Unmöglichkeit für Agenten ist gemessen (xdotool-Beweis im README), die
Kundensitzung als Folge-Schritt benannt; (b) #60: die Meta+N-Kette gilt am
Build-Stand, die installierte Desktop-Datei trägt noch `Name=Capture öffnen` —
Nachprüfkommandos für den PO-Installationstakt liegen im README bei; (c) #11
Befund 5 (Dialogsymbole) — Entscheidung am installierten Stand, im Bericht
sauber vorbereitet. Alle drei sind ordnungsgemäß eskaliert statt verschwiegen;
solange sie offen sind, ist die Sprint-Abnahme unvollständig — dieselbe
Bauart wie Sprint-3-Mangel M1 (Endstand einmal installieren und prüfen).

**Befund 4.2 (warn, klein) — Bytegleiche Belegbilder ohne Vermerk:** In der
Nachprüfung (`95bcab6`) sind `n04-waechter-abbrechen.png` und
`n05-waechter-auswahlwechsel.png` bytegleich (Blob `a95acf0`) — und zudem
identisch mit `03-waechterdialog.png` der Entwickler-Bildstrecke im Zweig;
ebenso `n06-nach-verwerfen.png` und `n07-nach-speichern.png` (Blob
`c886bc5`). Das ist bei deterministischem Offscreen-Rendering erklärbar —
nach der Heilung von Befund 2 sind die Szenen pixelgleich, und das geänderte
Wort liegt hinter der Zeilen-Elision der Liste („Transkript: Idee für
Denkzettel — den Bündel-Expor…"). Aber der Bericht zitiert n06/n07 als Belege
zweier **verschiedener** Speicherzustände („ohne Vault"/„mit Vault"), die nur
in `n-messwerte.txt` unterscheidbar sind, nicht im Bild. Ein Vermerk fehlt;
Strang B führt mit seinem Ehrlichkeitsvermerk („war schon vorher grün") vor,
wie so etwas aussieht.

---

## Konkrete Fix-Vorschläge

1. **(fail, vor dem Merge von PR #65)** `tests/librarytest.cpp:2964–2967`:
   die Ursachenangabe an die gemessene Fassung angleichen — der Stapel selbst
   fordert die Breite an, das addStretch() der Bearbeiten-Seite ist gemessen
   nicht die Ursache (Wortlaut steht in `src/ui/librarywindow.cpp:336–339`
   und in der Commit-Botschaft von `60cae75`).
2. **(warn)** `docs/scrum/reviews/sprint-04-s8-bearbeiten/LIESMICH.md`,
   Absatz „Grenze dieser Bildstrecke": Ursache auf den Stand der Nachprüfung
   bringen (Ersatzdialog der Plattformintegration übernimmt `setIcon()`
   nicht; Entscheidung am installierten Stand) statt „sobald ein auflösendes
   Symbolthema verfügbar ist".
3. **(warn)** `docs/scrum/reviews/sprint-04-s8-ui-review/bericht.md`,
   Nachprüfung Stelle 2: einen Satz ergänzen, dass n04/n05 sowie n06/n07
   bytegleich sind und der Speicher-Unterschied in `n-messwerte.txt` steht.
4. **(Prozess, Retro-Kandidat)** B13-Dateimengen-Vorlage um die
   story-eigenen Beleg- und Prüfmittelpfade ergänzen
   (`docs/scrum/reviews/<story>/`, Bildläufer + CMake-Verdrahtung) — sonst
   ist künftig jede Story mit Bildpflicht formal im Verstoß.
5. **(Merken für den Installationstakt)** Die drei Loose Ends aus
   Befund 4.1 sind dort zu schließen; die Kommandos stehen in
   `docs/scrum/reviews/sprint-04-s33-traymenues/README.md`.

## PR-Probelauf — Bewertung aus Sicht dieses Reviews

Der fail-Befund ankert an neu hinzugefügten Diff-Zeilen von PR #65
(`tests/librarytest.cpp:2964–2967`, Commit `60cae75`). Sichtbar wird der
Widerspruch aber erst im Nebeneinander von Diff-Zeile, Commit-Botschaft und
`librarywindow.cpp:336–339` — mit `git show 60cae75` genauso auffindbar wie
im PR. Ehrlich gemessen trug der PR-Weg diesem Review **keinen** Befund zu,
der ohne PR nicht auffindbar gewesen wäre; automatische Testläufe liefen auf
beiden PRs nicht (`statusCheckRollup` leer). Gegen das vorab festgelegte
Kriterium aus `PROZESS.md` ist der Probelauf damit aus meiner Sicht nicht
bestanden — die Bewertung obliegt dem PO.

## Was gut ist

- **Die Begründungs-Korrektur als eigene Commit-Gattung** (`646804a`,
  `b67a348`, die „Richtigstellung" in `95bcab6`): Das Projekt korrigiert
  seine eigenen Begründungen nachweislich und mit Messung. Befund 1.1 ist
  ein Ausreißer dieses Musters, nicht sein Widerlegen.
- **Messung vor Bauentscheidung** in #60: Die SNI-Messung als erster
  Story-Schritt, die Rückfallregel griff wie geplant, die Kundenentscheidung
  ist protokolliert — genau die Loop-Regel „needs a human", gelebt.
- **Der Ehrlichkeitsvermerk** im S33-README (ein Test war schon vor der
  Änderung grün und beweist die Änderung nicht) — diese Selbstauskunft
  gehört zum Besten, was der Sprint an Prüfkultur zeigt.
- **Die Wächterdialog-Matrix** (3 Auslösepfade × 3 Antworten) lässt keinen
  Weg unbeobachtet; `DialogWatch` sichert zusätzlich die Gegenrichtung (kein
  Dialog, wo keiner hingehört) gegen Hängen statt Rotwerden.
- **SPEC-Nachzug beider Stränge** als ausdrücklich gekennzeichnete entdeckte
  Bedingungen (DoD 4/B9), mit tatsächlichen statt nachgeschobener
  Begründungen.
- **`GetLayout` als zweites Prüfmittel**: nicht die eigenen QActions,
  sondern das, was wirklich über das Tray-Protokoll beim Host ankommt.
