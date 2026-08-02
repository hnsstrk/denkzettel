# Karpathy-Review — Sprint 5 (DoD 3)

**Reviewer:** karpathy-reviewer (frischer Kontext) · **Datum:** 2026-08-02
**Prüfgegenstand:** `git diff sprint-05-basis..HEAD` auf `main` — Merges `32bb5fc`
(#66/#67, Strang A) und `1adffd3` (#57/#58, Strang B), Wireframe-Nachträge
(`88e6dc8`, `8a8c652`), SPEC-9-Korrekturen, CLAUDE.md-Absatz (`5a4a83a`).
**Task:** Sprint-5-Diff gegen die vier Karpathy-Prinzipien halten; Berichte der
Stränge sind die Behauptung, eigene Messung der Beleg.

**Gesamt-Verdict:** warn

Alle Messungen dieses Reviews sind selbst gefahren: Build am Merge-Endstand,
`librarytest` in beiden Umgebungen, vier Quell-Mutationen (danach
zurückgenommen, Binary auf Originalstand zurückgebaut, Abschlusslauf 107/0).

## Prinzip 1 — Think Before Acting

**Verdict:** warn

**Beobachtungen:**

- Die im Sprint gekippten Annahmen sind an den Hauptstellen konsistent
  nachgezogen. Stichproben: SPEC 9 sagt jetzt „eigenes Meldungsfenster samt
  eigenen Knopfobjekten" statt der widerlegten Fassung; `librarywindow.cpp:946`
  („a built QMessageBox is not the dialog the user gets … a QMessageBox, but
  not this one") und `librarytest.cpp:2786` sagen dasselbe. Die widerlegte
  `KMessageDialog::setIcon()`-Zusage steht gleichlautend an `SPEC.md:460`, im
  Kommentar von `askAboutUnsavedChanges()` und in der Wireframe-Tafel. Der
  `editshots.cpp:209`-Kommentar ist von „kann die Symbole nicht zeigen" auf
  „kann sie seit #66 wieder zeigen" umgeschrieben — die alte Begründung lebt
  dort nicht weiter.
- **Eine Nebenstelle ist stehen geblieben** (die Fehlerklasse des Tages):
  `wireframes/Denkzettel Wireframes.dc.html:345` schließt mit
  „`librarytest` setzt es heute nicht." Seit `6e227b9`
  (`tests/CMakeLists.txt:50`) setzt `librarytest` das Plattformthema. Die
  Tafel wurde im Sprint zweimal angefasst (`88e6dc8`, `8a8c652`), der Satz
  blieb. Wer die Zeichnung als Erstes liest, hält die Testumgebung für
  ungedeckt — das Gegenteil dessen, was #66 AK 3 hergestellt hat.
- Entscheidungen sind als Entscheidungen ausgewiesen, nicht als Fakten
  (F3 Vorgabeknopf „PO decision", Warnsymbol „PO-Entscheidung 02.08.2026",
  beide mit dem Hinweis, dass auch die andere Lesart HIG-konform wäre).
  Kein sykophanter Ton in den Berichten; Messung und Schluss sind getrennt.

## Prinzip 2 — Simplicity First

**Verdict:** ok

**Beobachtungen:**

- Der Ereignisfilter samt `QScopeGuard` (#57) trägt an jeder Konstruktion
  einen gemessenen Fall: `QListView::pressed` kommt nach `currentChanged`
  (Kommentar `librarywindow.cpp`, `eventFilter()`); ein gepostetes
  Zurücksetzen liefe im modalen Umlauf des Wächterdialogs (Kommentar in
  `showNote()`); die Tastendruck-Rücksetzung ist durch
  `testlauf-2-gegenprobe.txt` belegt und fiel bei meiner Mutation 2 rot
  („Kopf bei y=-35"). Nichts davon ist spekulativ.
- `putReturnOnThePrimaryAction()` mit zwei Aufrufen statt einem: die dritte
  Variante (autoDefault abschalten) wurde gemessen und mit Begründung
  verworfen (Bericht A §2.3, Messtabelle). Kein Overengineering — die
  einfachere Fassung hält nachweislich nicht.
- Der Testhelfer-Umbau (`waitForGuardDialog`, `dialogAnswerButtons`,
  `dialogButton`, `dialogAnswers`) ist mehrfachgenutzt (fünf Aufrufstellen in
  `librarytest.cpp`) und trägt je einen gemessenen Fall (Checkbox-Falle,
  Vorgabe-Kippen beim Sichtbarwerden). `qWaitForWindowExposed` ist die
  Konsequenz aus dem selbst entlarvten grünen Test (rot-03), keine Zierde.
- Grenzfall, kein Befund: `breezePalette()` in `libraryshots.cpp` setzt
  13 Rollen, von denen vier gemessen und neun abgeleitet sind — mit
  benanntem Zweck (ein Bild, in dem nur die Textfarbe wechselt, bewiese
  nichts). Vertretbar für einen Bildläufer.

## Prinzip 3 — Surgical Changes

**Verdict:** ok

**Beobachtungen:**

- Dateimengen nach Planning §5.2 eingehalten: Strang A fasste
  `librarywindow.cpp`, `librarytest.cpp`, `editshots.cpp`,
  `tests/CMakeLists.txt`, SPEC 9+16 und seinen Belegordner an
  (`librarywindow.h` blieb unberührt, wie der Bericht sagt); Strang B
  `librarywindow.{h,cpp}`, `librarytest.cpp`, `libraryshots.cpp`, den
  SPEC-Listenteil und seinen Belegordner. Kein Übergriff gefunden; die
  Belegordner-Umbenennung ist im Bericht B selbst deklariert.
- Die Stränge kamen sich trotz des 38-Zeilen-Abstands in `librarytest.cpp`
  nicht in die Quere: Merge-Endstand 102 + 3 + 2 = **107 Tests, 0 failed**,
  mit und ohne Plattformthema (selbst gefahren — die Grün-Belege der
  Stränge stammen aus den Zweigen, nicht vom Merge; das hier ist der
  fehlende Endstand-Nachweis).
- „Melden, nicht heilen" gehalten: Strang A meldet die unvollständige
  SPEC-15-Klammer (`SPEC.md:597` nennt bei KWidgetsAddons nur
  `KMessageWidget`) und fasst sie nicht an; Strang B ließ den roten
  Strang-A-Test ausdrücklich stehen.
- Die Test-Umkehrung (K5) ist eine Umkehrung, keine Löschung: der Kommentar
  in `librarytest.cpp:1479` trägt die alte Fassung samt ihres Schlusssatzes
  weiter — die Spur, wo entschieden wurde, bleibt lesbar.
- Der CLAUDE.md-Absatz (`5a4a83a`) verankert den EXCLUDE_FROM_ALL-Vorfall
  am Ort, den die nächste Sitzung von selbst liest — das ist die
  Retro-Regel des Projekts, kein Scope-Drift.

## Prinzip 4 — Goal-Driven Execution

**Verdict:** ok

**Beobachtungen:**

- **Alle vier Kernzusicherungen fallen am Endstand rot, wenn ihre Heilung
  entfernt wird** (Mutation im Arbeitsbaum, danach zurückgenommen,
  `git status` sauber, Binary zurückgebaut):
  1. `&& !m_selectionFollowsAPress` entfernt →
     `leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked` FAIL.
  2. Tastendruck-Rücksetzung im `eventFilter` entfernt →
     `keepsTheHeadFetchAfterAClickThatSelectedNothing` FAIL („Kopf bei y=-35").
  3. `dialog.setIcon(…dialog-warning…)` entfernt →
     `showsTheWarningSymbolInTheGuardDialog` FAIL.
  4. `putReturnOnThePrimaryAction(&dialog)` entfernt →
     `namesTheThreeAnswersOfTheGuardDialog` FAIL (Vorgabeantwort).
  Kein weiterer grüner Test gefunden, der am geprüften Aufbau nicht
  scheitern kann.
- Die Umkehrung von #57 ist vollständig: `grep mouseClick` liefert nur die
  zwei neuen Teststellen (`librarytest.cpp:1540`, `:1612`); keine
  Zusicherung fordert mehr das alte Klick-Vorscrollen. Die Tastatur-Seite
  hält `bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready` (`:1424`)
  unverändert.
- Rot-vor-Grün ist durchgängig belegt (rot-01…04, testlauf-1/2), inklusive
  des Falls, in dem der Rot-Nachweis erst gebaut werden musste (Vorgabeknopf)
  und des Tests, der prinzipiell nicht rot sein konnte und deshalb gegen
  einen mutierten Stand gefahren wurde (Bericht B §2) — von mir unabhängig
  bestätigt (Mutation 2).
- Offene Enden sind benannt und adressiert, nicht verschwiegen: Kundenfoto
  am echten Plasma und Installation (PO-Takt), Szene N2 als vorgeschlagenes
  Folge-Issue, SPEC 15. Beobachtung ohne Verdict-Wirkung:
  `namesTheEditedNoteWithoutBreakingTheSentence` prüft jetzt
  `shown.contains(expected)` über alle Dialog-Labels statt Gleichheit gegen
  ein benanntes Feld; da `expected` der vollständige Meldungstext ist und
  ein Label ihn exakt ganz tragen muss, bleibt die Prüfung streng.

## Konkrete Fix-Vorschläge

1. `wireframes/Denkzettel Wireframes.dc.html:345`: den Satz
   „`librarytest` setzt es heute nicht." streichen oder auf „seit Sprint 5
   gesetzt (`tests/CMakeLists.txt`)" berichtigen — eine Zeile, PO-Fläche.
2. `SPEC.md:597`: `KMessageDialog` in die KWidgetsAddons-Klammer aufnehmen —
   von Strang A gemeldet, seit dem Merge ist der Adressat der PO.
3. Sprint-Abschluss: das von Bericht B §8 vorgeschlagene N2-Issue
   (Kopf y=−35 innerhalb der Gruppe) anlegen, sonst verdunstet der
   Vorschlag im Bericht.

## Was gut ist

- Rot-vor-Grün lückenlos, einschließlich des selbst entlarvten grünen Tests
  und des eigens nachgebauten Rot-Nachweises — die Prüfhaltung des Projekts
  wird gelebt, nicht zitiert.
- Alle vier Kernzusicherungen sind mutationsfest; die Testbasis prüft
  Bedeutung (Klick per Beschriftung, Wirkung erwartet) statt Zirkelschluss
  über Rollen.
- Widerlegte Annahmen sind an SPEC, Code-Kommentaren und Läufer-Kommentaren
  konsistent nachgezogen; nur eine Nebenstelle (Wireframe:345) blieb stehen.
- Beide Berichte trennen Beobachtung von Schlussfolgerung, benennen eigene
  Grenzen (EXCLUDE_FROM_ALL-Vorfall selbst berichtet und in CLAUDE.md
  verankert) und weisen offene Enden mit Adressat aus.
