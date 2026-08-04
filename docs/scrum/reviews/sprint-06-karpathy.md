# Karpathy-Review — Sprint 6 (Sprint-Diff)

**Datum:** 2026-08-04 · **Reviewer:** karpathy-reviewer (Fable, frischer Kontext)
**Prüfgegenstand:** `git diff sprint-06-basis..main` (73 Dateien, +6.025/−16),
einschließlich der im Auftrag nicht genannten Diff-Teile (siehe K4).
**Gelesen, nicht gestichprobt:** der vollständige Code-Diff (`src/`, `tests/`,
drei `CMakeLists.txt`, `SPEC.md`, `README.md`, `ci.yml`, `PROZESS.md`,
`tests/testsilence.cpp`), beide Übergabeberichte, das Sprint-Protokoll
(§1–§16), `PROZESS.md` und `CLAUDE.md`.

**Task:** Sprint-Ende-Review nach DoD 3 über den gesamten Sprint-Diff —
#56 (Feldhöhe bei Schriftänderung), #55 (Fensterhülle aus dem Desktop-Theme),
#59 (ruhige Liste bei Aktivierung), PO-Arbeit (CI, README, Protokoll).

**Gesamt-Verdict:** **ok** — alle vier Befunde behoben (Nachprüfung 04.08.2026,
Abschnitt „Wiedervorlage" unten; erster Durchgang: fail)

## Befunde

| ID | Prinzip | Verdict | Ort | Befund | Status |
|----|---------|---------|-----|--------|--------|
| K1 | 4 Goal-Driven | **fail** | `sprint-06-s55-huelle/bericht.md:89–93` gegen die Tabellen §3.2/§10.3 | Der Bericht behauptet „jede tragende Zusicherung gegen eine Mutation des Produktivcodes gehalten" — die dokumentierten Mutationstabellen decken aber nur 8 der 11 neuen `capturetest`-Zusicherungen. Ohne Mutationszeile: `paintsOneSurfaceInThePaletteColours` (tragend, AK 2 von #55), `wearsNoDecoration` (AK 6), `staysUsableWithoutADesktopTheme` (AK 8). Keine der sieben dokumentierten Mutationen würde `paintsOneSurfaceInThePaletteColours` rot machen — die Mutation „Hülle gar nicht zeichnen (immer die Ersatzfläche)" füllt die Fläche gerade einfarbig, der Test bliebe grün. Die Behauptung ist damit breiter als ihr Beleg — im selben Projekt, das sieben grüne Nichts-Prüfer durch Messung gefunden hat, und in dem Sprint, dessen Review-Auftrag ausdrücklich nach der Vollständigkeit dieser Liste fragte. | **behoben** (`daf1afd`, §11.1) |
| K2 | 4 Goal-Driven | warn | `src/capture/capturewindow.cpp:160` und `:361`; Test `capturetest.cpp` (`paintsOneSurfaceInThePaletteColours`) | Verdacht, am Code begründet, nicht gefahren (**unbelegt**): Die „eine durchgehende Fläche" wird von zwei redundanten Hälften erzeugt — `viewport()->setAutoFillBackground(false)` (:160) und `Base = Qt::transparent` (:361). Jede Hälfte allein verhindert den Kasten im Kasten; eine Einzelmutation einer Hälfte bliebe voraussichtlich grün. Ob der Test beim Entfernen *beider* rot wird, ist nirgends belegt. Das ist der konkrete Kandidat, an dem K1 zu heben ist. | **behoben** (`daf1afd`, §11.2 — Verdacht gemessen: Einzelmutation grün, Doppelmutation rot; kein fünfter Fall) |
| K3 | 3 Surgical | warn | `tests/capturetest.cpp:504, :565`; `tests/captureshots.cpp:158–160` | Drei deutsche Kommentarblöcke in durchgehend englisch kommentierten Dateien; in beiden capturetest-Stellen zudem „zusaetzlich" als ae-Digraph — die Encoding-Regel des Projekts verbietet Digraphen ausdrücklich, und der Stilbruch fällt in Dateien auf, deren übrige Kommentare der Konvention folgen. | **behoben** (`daf1afd`, §11.3) |
| K4 | 1 Think Before Acting | warn | Review-Auftrag; `docs/scrum/sprints/sprint-06.md:1171–1175` | Der Diff-Inhalt ist im Auftrag und in §15.1 unvollständig benannt: Neben den drei Stories und der genannten PO-Arbeit enthält der Sprint-Diff die Klangfrei-Arbeit (`tests/testsilence.cpp`, Kopfblock `tests/CMakeLists.txt:1–17`, SPEC-9-Absatz), den BM25-Absatz (SPEC 6), den Altbestands-Absatz in `PROZESS.md`, `sprint-05-installationstakt.md` und rund 2.900 Zeilen Berichte vom 04.08. Wer die DoD-3-Deckung später am Protokoll abliest, hält diese Teile entweder für ungeprüft oder stillschweigend für geprüft. **Ich habe die codetragenden Teile mitgeprüft** (testsilence-Bauart, SPEC-Absätze, PROZESS-Diff): kein Befund — aber die Deckung gehört ausgesprochen, nicht unterstellt. | **behoben** (`sprint-06.md` §17.1) |

## Fix-Vorschläge

- **K1** → Entweder die drei fehlenden Mutationen fahren und die Tabelle in
  §10.3 ergänzen, oder den Satz in §3.2 auf die belegten acht einschränken.
  Konkrete Mutationen: für `paintsOneSurface…` beide Hälften aus K2 zugleich
  entfernen; für `staysUsable…` den Ersatzflächen-Fill
  (`capturewindow.cpp:306`) entfernen; `wearsNoDecoration` prüft vorbestehende
  Flags (Konstruktor unverändert) — hier reicht die Kennzeichnung „nicht
  tragend, Bestandszusicherung".
- **K2** → wird mit K1 miterledigt; falls die Doppelmutation grün bleibt, ist
  der fünfte Fall gefunden und der Test braucht eine schärfere Sonde.
- **K3** → zwei Kommentare übersetzen oder die Umlaute korrigieren —
  Ein-Zeilen-Fixes in der Fläche von Strang A.
- **K4** → ein Absatz in der DoD-Prüfung des Sprint-Protokolls, der den
  Beifang des Diffs aufzählt und je auf den vorhandenen Bericht verweist
  (`2026-08-04-klangfrei.md`, `2026-08-04-bm25/`, dieser Bericht).

## Was gut ist

- **Surgical durchgehend:** Jede geänderte Produktivzeile führt auf ein AK,
  eine Planning-Festlegung oder einen dokumentierten PO-Auftrag zurück. Die
  #59-Heilung ist minimal (ein Gate in `changeEvent`, ein Member, zwei
  Setzstellen — beide gruppenbildenden Stellen erfasst, keine dritte
  vorhanden). Strang B hat den naheliegenden Schwellwert-Weg ausdrücklich
  **nicht** gebaut.
- **Melden, nicht heilen gelebt:** B1 (mit Messung gegen den Ausgangsstand —
  vorbildlich), B2, B3, M-B1 bis M-B3; nichts davon vom Finder angefasst.
- **Die drei Planning-Festlegungen sind eingehalten und am Code verifiziert:**
  #56 im `eventFilter` (`capturewindow.cpp`, `FontChange`-Zweig), keine Zahl
  in den Hüllen-Zusicherungen (alle relativ, `wideCorner != narrowCorner`
  statt `>` — die Korrektur aus §10.1 steht wirklich im Test), Läufer auf #55
  gebucht (Bedingung 2.3 hält, #56 bleibt 1 SP).
- **Rot-vor-Heilung versioniert** für #56 (*Actual 85 / Expected 215*) und #59
  (*Actual 7 / Expected 0*, `befund-vor-der-heilung.txt`); der neue
  `staysPut…`-Test sichert seine eigene Voraussetzung zu
  (`librarytest.cpp:1601–1602` — Auswahl außerhalb des Bildes) und schließt
  damit genau den Aufbau aus, in dem der Fehler nicht auftreten kann.
- **Benannte Grenzen statt stiller Lücken:** Schatten-Augenschein und
  Remap-Sichtprüfung als Abnahme-Punkte, der letzte Meter des Theme-Wechsels
  (KDirWatch-Zusammenbau) ausdrücklich ungeprüft, #56 am Kundenblick nicht
  prüfbar — alles ausgesprochen, nichts wegerklärt.
- **DoD 4 in der B9-Fassung ernst genommen:** SPEC 3.2 hält fünf entdeckte
  Bedingungen fest, vier davon kontraintuitiv; SPEC 16 die Grenzen der
  Offscreen-Prüfbarkeit und die Namensregel. Der CI-Paketnachzug ist begründet
  kommentiert.
- **Simplicity trotz großer Summe:** Vier Abhängigkeiten, Bildläufer,
  `desktopthemes.h` und `tests/themes/` sind je einzeln auf Messung, AK 8 oder
  die PO-Auflage aus dem Nachtrag rückführbar; Spekulatives habe ich nicht
  gefunden. `testsilence.cpp` ist die kleinste Bauart, die ihren Zweck erfüllt,
  und begründet, warum sie kein Objekt mit Konstruktor ist.

---

## Wiedervorlage 04.08.2026 — Nachprüfung der Behebung (`daf1afd`)

Geprüft am Commit und am Quellstand, nicht an der Meldung. Nur K1–K3;
K4 hat der PO selbst geschlossen (§17.1 des Sprint-Protokolls, Existenz
geprüft).

**Frage 1 — ist K1 wirklich geschlossen? Nachgezählt: ja, 11 von 11.**
`capturetest.cpp` führt elf neue Slots (`:45`, `:53–:62`). Deckung:
`heightFollowsAFontChange` (§3.2 Z. 7) · `hullFollowsTheDesktopTheme` (Z. 1–2)
· `hullFollowsAnInstalledDesktopTheme` (§10.3 Z. 1–2) ·
`readsTheDesktopThemeFromPlasmarc` (Z. 1) · `noteTextUsesTheWindowTextRole`
(Z. 3) · `footerHasMoreAirThanTheApplicationName` (Z. 5) ·
`hullIsCompleteAtFiveAndEightLines` (Z. 2) · `bindsAShadowFromTheThemeTiles`
(Z. 4 und 6) · `paintsOneSurfaceInThePaletteColours` (§11.1 Z. 1) ·
`staysUsableWithoutADesktopTheme` (§11.1 Z. 1–2) · `wearsNoDecoration`
(§11.1 Z. 3). Die neue Überschrift („Alle elf") trägt jetzt genau so weit wie
die Tabellen. *Evidenzform wie bei den ersten sieben:* die Mutationsläufe sind
im Bericht tabelliert, nicht als Laufprotokoll versioniert — derselbe Maßstab,
den der Sprint durchgehend angelegt hat, kein neuer Befund.

**K2, mitgeprüft:** Die geforderte Messung ist gefahren und in §11.2
dokumentiert, samt der unangenehmen Hälfte (keine der beiden Zeilen einzeln
testgedeckt). Der Entscheidung, **keine** Hälfte zu entfernen, stimme ich zu:
Eine Streichung auf Basis einer Ein-Plattform-Messung wäre genau die Bauart
„Vereinfachung, die nicht gegen die geheilten Fälle gehalten wurde"
(`CLAUDE.md`, Prüfhaltung); behalten und die Messung für spätere Streicher
hinterlegen ist der richtige Umgang.

**Frage 2 — ist die K3-Behebung über ihren Anlass hinausgeschossen? Nein,
gedeckt.** Die zusätzlichen Stellen (Kopfkommentare beider Prüf-Themes,
`Name`/`Description` der `metadata.json`) sind **Instanzen desselben
Befundes** — dieselbe Digraphen-Klasse, dieselbe Verursachung, dieselbe neue
Datei-Familie aus diesem Sprint —, nicht benachbarte Verbesserungen. Sie sind
im Commit benannt statt still mitgeheilt, das Verhalten ist unverändert
(SVG-Diff ausschließlich Kommentarzeilen, `KPlugin.Id` gleich, Geometrie
gleich), und `spellfixspike.cpp` wurde zu Recht verschont. Das ist
Befundbehebung in voller Breite, keine Scope-Erweiterung.
*Eine Randnotiz dazu, Bestand und außerhalb des Sprint-Diffs, nur gemeldet:*
`tests/libraryshots.cpp:306` vergibt den Bildnamen
`07-fall4-uebergang-beim-scrollen.png` mit ue-Digraph — aus einem früheren
Sprint, und an dem Namen hängen versionierte Belegbilder. Der Satz „alles
unter `tests/` auf Englisch gezogen" liest sich eine Spur breiter als dieser
Bestand; entscheiden, ob und wann das mitgezogen wird, ist PO-Sache.

**Gesamt-Verdict nach Wiedervorlage: ok** — kein Befund offen.
