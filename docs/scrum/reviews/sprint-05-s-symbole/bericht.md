# Strang A — #66 Wächterdialog auf KDE-Bauart · #67 Bibliotheks-Symbole

**Datum:** 2026-08-02 · **Zweig:** `story/66-67-symbole` · **Rolle:** Entwickler
**Prüfstand:** Arbeitsbaum-Build (`build/`, Debug), **nicht** installiert — die
Installation nach `/usr` taktet der PO (DoD 2, Sprint-5-Planning §7 Risiko 5).

Alle Aussagen unten sind gemessen. Wo eine Messung eine Annahme des Auftrags
oder der SPEC widerlegt hat, steht es dabei.

## 1. Rot-Nachweise, in der Reihenfolge, in der sie fielen

### 1.1 Die Plattformthema-Zeile macht den Kundenbefund reproduzierbar

Erster Schritt war die eine Zeile in `tests/CMakeLists.txt` (§5.3.1 des
Plannings). Sie allein färbt den Bestand rot — genau wie der Scrum Master es
gemessen hatte:

```
FAIL!  : LibraryTest::namesTheThreeAnswersOfTheGuardDialog() '!icons.contains(QString())' returned FALSE. (||)
Totals: 101 passed, 1 failed
```

`||` sind drei leere Symbolnamen. Beleg: `rot-01-plattformthema.txt`.
Vorlauf ohne die Zeile: 102 passed, 0 failed.

### 1.2 Die neuen Zusicherungen vor der Heilung

```
FAIL!  : namesTheThreeAnswersOfTheGuardDialog()  Actual (icons…"Speichern"): ""   Expected: "document-save"
FAIL!  : namesTheSymbolsOfTheDetailButtons()     Actual (edit->icon().name()): ""  Expected: "document-edit"
FAIL!  : namesTheSymbolOfTheUndoAction()         Actual (undo->icon().name()): ""  Expected: "edit-undo"
FAIL!  : namesTheEditedNoteWithoutBreakingTheSentence(heute|älter)
Totals: 99 passed, 5 failed
```

Beleg: `rot-02-tests-vor-der-heilung.txt`. Dreimal die leere Zeichenkette —
dieselbe Signatur wie im Kundenbild.

### 1.3 Der Vorgabeknopf — der Rot-Nachweis, den ich mir selbst erst bauen musste

Nach der Heilung stand die Zusicherung auf den Vorgabeknopf **grün, und sie
prüfte nichts.** Aufgefallen ist das nicht am Test, sondern daran, dass der
Bildläufer etwas anderes meldete als der Test:

| Quelle | Vorgabeantwort |
|---|---|
| `librarytest` (las, sobald der Dialog *existiert*) | Speichern |
| `editshots` (las, nachdem der Dialog *auf dem Schirm* war) | **Abbrechen** |

Gemessen (`probe66e`): Die Vorgabe steht unmittelbar nach dem Setzen auf
Speichern, in der ersten Runde der Ereignisschleife noch, und **kippt beim
Sichtbarwerden auf Abbrechen**. Der Test hat also einen Zustand geprüft, den
kein Nutzer je sieht — ein grüner Test, der nichts prüft.

Geheilt in zwei Schritten, damit der Nachweis rot ist, bevor er grün wird:

1. `waitForGuardDialog()` wartet jetzt zusätzlich auf `qWaitForWindowExposed`.
   → **rot**: `Actual (defaultAnswer): "Abbrechen"` (`rot-03-vorgabeknopf.txt`)
2. Erst dann die Heilung im Produkt.

## 2. Was gemessen wurde, und was davon der SPEC widersprach

### 2.1 Die Plattformintegration *ersetzt* den Dialog — nicht nur seine Knöpfe

SPEC 9 (Fassung Sprint 4) sagte: das System ersetzt den `QMessageBox` durch
„einen **eigenen Dialog** mit eigenen Knopfobjekten". Gemessen (`probe66`):

```
QMessageBox nach show(): activeModalWidget ist QMessageBox — dieselbe Instanz: nein
  Symbole an unseren Knöpfen: document-save edit-delete dialog-cancel
  Knopf 'Speichern' Rolle=0 Symbol='' default=ja
  Knopf 'Abbrechen' Rolle=1 Symbol='' default=nein
  Knopf 'Verwerfen' Rolle=2 Symbol='' default=nein
```

Der Ersatz ist **wieder ein `QMessageBox`, nur eine andere Instanz**. Das ist
der Grund, warum der alte Test überhaupt bis zur Symbolzeile kam: sein
`qobject_cast<QMessageBox*>` gelang, Beschriftungen und Rollen stimmten — nur
die Symbole waren die unserer Knöpfe, und die standen im Bild nicht.
SPEC 9 ist entsprechend nachgezogen.

### 2.2 `KStandardGuiItem` deckt die Festlegungstafel

Ausgelesen, nicht erinnert: `save()` → `document-save`, `discard()` →
`edit-delete`, `cancel()` → `dialog-cancel`, `del()` → `edit-delete`.
Deckungsgleich mit der Tafel „Symbole an den Schaltflächen" (2a).

### 2.3 Drei Bedingungen der KDE-Bauart, alle in SPEC 9 nachgetragen

| Beobachtung | Folge im Bau |
|---|---|
| Vorgabe folgt dem **Fokus**; beim Sichtbarwerden bekommt „Abbrechen" beides | „Speichern" erhält **nach** `show()` Fokus *und* Vorgabe |
| Ein von Hand gezeigter Dialog wird durch späteres `exec()` **nicht mehr modal** | Modalität wird selbst gesetzt |
| `KMessageDialog` kennt kein `informativeText` | Frage und Erläuterung in einem Text, durch Leerzeile getrennt |

Die dritte Variante — Selbstvorgabe der anderen Knöpfe abschalten — wurde
gemessen und **verworfen**: sie hält die Vorgabe, lässt aber den Fokusring auf
„Abbrechen" stehen, während Return „Speichern" auslöst.

```
Variante 0 (nur setDefault)      nach Sichtbarwerden: Vorgabe=Abbrechen Fokus=Abbrechen
Variante 1 (setDefault+setFocus) nach Sichtbarwerden: Vorgabe=Speichern Fokus=Speichern   ← gebaut
Variante 2 (autoDefault aus)     nach Sichtbarwerden: Vorgabe=Speichern Fokus=Abbrechen
```

### 2.4 Antwort-Zuordnung, geklickt statt gelesen

`Yes`→PrimaryAction, `No`→SecondaryAction, `Reject`→Cancel, **Esc→Cancel**
(nicht `QDialog::Rejected`), Return→PrimaryAction. Der Bau bildet nur
Primary/Secondary ab; **alles andere fällt auf „Abbrechen"** — die Antwort, die
nichts verliert.

## 3. Die 3×3-Matrix prüft jetzt Bedeutung

Vorher wurde der Knopf **über seine Rolle gesucht** und dann die Rolle
erwartet. Das ist zirkulär: Tauschten zwei Antworten ihre Beschriftung, klickte
der Test weiter den Knopf im „Verwerfen"-Fach und bliebe grün.

Jetzt wird **über die Beschriftung geklickt** — so, wie ein Nutzer es tut — und
die **Wirkung** erwartet: Speichern schreibt, Verwerfen verwirft, Abbrechen
bleibt im Editor. Neun Fälle, drei Auslöser × drei Antworten.

## 4. Testbilanz

| Lauf | Ergebnis | Beleg |
|---|---|---|
| `librarytest` **mit** `QT_QPA_PLATFORMTHEME=kde` | **104 passed, 0 failed** | `gruen-mit-plattformthema.txt` |
| `librarytest` **ohne** Plattformthema | **104 passed, 0 failed** | `gruen-ohne-plattformthema.txt` |
| `ctest` vollständig | **7/7 passed** | `gruen-ctest.txt` |

Vorher 102 Tests, jetzt 104 (zwei neue für #67). `lint-clazy` und `lint-tidy`
melden auf `src/ui/librarywindow.cpp` **nichts Neues**; die drei von mir
eingeführten clazy-Befunde und vier tidy-Befunde sind behoben. Was stehen
bleibt, steht unter 7.

## 5. Selbstprüfung am gebauten Stand (DoD 2)

**Hauptweg beider Stories**, gefahren von `editshots` gegen die echte
`LibraryWindow` (`bildlauf-messwerte.txt`):

```
Detailkopf: „Bearbeiten“ Symbol „document-edit“, 104 px breit (natürlich 104 px)
Detailkopf: „Löschen“ Symbol „edit-delete“, 88 px breit (natürlich 88 px)
Bearbeiten-Fußzeile: „Speichern“ Symbol „document-save“, 98 px (natürlich 98 px)
Bearbeiten-Fußzeile: „Abbrechen“ Symbol „dialog-cancel“, 104 px (natürlich 104 px)
Wächterdialog: „Abbrechen“ dialog-cancel · „Speichern“ document-save (Vorgabe) · „Verwerfen“ edit-delete
Meldungszeile: „Rückgängig“ Symbol „edit-undo“
vor dem Speichern: „Fold“ 1 Treffer, „Vault“ 0 Treffer, needs_reembed=0
nach dem Speichern: „Fold“ 0 Treffer, „Vault“ 1 Treffer, needs_reembed=1
Fenster nach „Abbrechen“ noch offen: ja
Notizen nach „Rückgängig“: 3
```

Die Knopfbreiten sind gleich ihrer natürlichen Breite — die Symbole haben die
Knöpfe verbreitert, ohne dass sie mit dem Fenster mitwachsen
(`keepsTheMeasuresOfTheEditState` hält unverändert, ohne Nachziehen).

**Daemon-Start**: `build/bin/denkzetteld` auf einem **eigenen Sitzungsbus**
(`dbus-run-session`) mit Wegwerf-Datenverzeichnis gestartet — der laufende
Daemon des Kunden und dessen Notizen bleiben unberührt. `AddNote` → `(int64 1,)`,
`ShowLibrary()` → `()`, Daemon lebt weiter. Im Journal steht, was auf einem
privaten Bus zu erwarten ist und **korrekt zurückgemeldet wird**:

```
Couldn't start kglobalaccel … The name org.kde.kglobalaccel was not provided
Meta+N ist beim Kurzbefehl-Dienst nicht angekommen — …
```

Kein Befund: Auf dem privaten Bus gibt es weder Kurzbefehl-Dienst noch Tray.

**Bilder** (je Wireframe-Zustand, `QT_QPA_PLATFORMTHEME=kde`), unter `bilder/`:

| Bild | Zustand |
|---|---|
| `01-lesen.png` | 2a Zustand A — Detailkopf mit Bearbeiten · Löschen |
| `02-bearbeiten.png` | 2a Zustand B — Fußzeile mit Speichern · Abbrechen |
| `03-waechterdialog.png` | 2a Zustand C — drei Antworten mit Symbolen, Fokus auf Speichern |
| `04-wiedergefunden.png` | Hauptweg S8, nach dem Speichern über die Suche gefunden |
| `05-loeschmeldung.png` | 2b Meldungszeile — „Rückgängig" mit `edit-undo` |

## 6. Akzeptanzkriterien

### #66 — Wächterdialog auf KDE-Bauart

| AK | Stand | Nachweis |
|---|---|---|
| Die drei Knöpfe tragen die `KStandardGuiItem`-Symbole (Foto-Nachweis) | **erfüllt bis auf das Kundenfoto** | `03-waechterdialog.png` + Namen im Messprotokoll; Foto am echten Plasma bleibt Kundensache (AK 3 sagt es selbst) |
| Die drei Antworten behalten ihre **Bedeutung**; Rollen ordnet die Bauart; 3×3-Matrix auf Bedeutungsebene | **erfüllt** | Abschnitt 3; `asksBeforeUnsavedChangesAreLost` 9 Fälle grün |
| Testumgebung K1: `librarytest` mit `QT_QPA_PLATFORMTHEME=kde`, Dialogtest misst `activeModalWidget()`, Befund vorher rot | **erfüllt** | 1.1, `tests/CMakeLists.txt` |
| SPEC 9 nachgezogen | **erfüllt** | SPEC 9, zwei Absätze neu |
| *Zusatz F3:* Vorgabeknopf ausdrücklich Speichern | **erfüllt** | 1.3, 2.3 |
| *Zusatz F4:* `KMessageDialog` ohne `informativeText` — Zweisatz zusammengezogen | **erfüllt** | 2.3, `namesTheEditedNoteWithoutBreakingTheSentence` |

### #67 — Bibliotheks-Symbole

| AK | Stand | Nachweis |
|---|---|---|
| Detailkopf: Bearbeiten `document-edit`, Löschen `edit-delete` | **erfüllt** | `namesTheSymbolsOfTheDetailButtons`, `01-lesen.png` |
| Bearbeiten-Fußzeile: Speichern/Abbrechen nach `KStandardGuiItem` | **erfüllt** | derselbe Test, `02-bearbeiten.png` |
| Rückgängig-Aktion trägt `edit-undo` | **erfüllt** | `namesTheSymbolOfTheUndoAction`, `05-loeschmeldung.png` |
| Symbolwahl konsistent mit Tray (#60); Zeichnungen 2a/2b ergänzt | **erfüllt** | Namen aus der Festlegungstafel übernommen; Zeichnung lag bei Sprintbeginn vor (`711d899`) |
| Geprüft wird der Symbol-**Name**, braucht `QT_QPA_PLATFORMTHEME=kde` | **erfüllt** | alle Zusicherungen auf `icon().name()` |
| Bildnachweis am echten Plasma | **offen — Kundensache** | siehe 7.1 |

## 7. Offen und gemeldet (melden, nicht heilen)

1. **Bild am echten Plasma / installierter Stand.** Nicht selbst gefahren: Die
   Installation nach `/usr` taktet der PO (DoD 2 bei Parallelarbeit), und das
   Abnahmebild ist nach #66 AK 3 ausdrücklich Kundensache. **Das ist die eine
   Grenze der Prüfbarkeit dieses Strangs**; sie ist nicht offen gelassen,
   sondern liegt bei PO und Kunde — Sprint-Abschluss Takt 1, Punkt 1.
2. **Der Wächterdialog hat kein Warnsymbol mehr.** Der Ersatzdialog der
   Plattform zeichnete ein großes oranges Ausrufezeichen
   (`sprint-04-kundenabnahme/waechterdialog-ohne-symbole.png`), der
   `KMessageDialog` zeichnet keines: Gemessen setzt er **nur ein ausdrücklich
   übergebenes** Symbol, obwohl sein Stil ein Warnsymbol hätte (32×32
   vorhanden). **Zeichnung 2a Zustand C zeichnet ebenfalls keines** — der Bau
   folgt also der Zeichnung, weicht aber vom abgenommenen Sprint-4-Bild ab. Ich
   habe **nichts hinzugefügt**: Ein Symbol wäre eine Änderung gegen die
   Zeichnung und über den Auftrag hinaus. **Entscheidung für UX/PO.**
3. **SPEC 15** nennt bei KWidgetsAddons nur „KMessageWidget"; jetzt kommt
   `KMessageDialog` aus demselben Framework dazu. Abhängigkeit unverändert, die
   Klammer unvollständig. Außerhalb meiner Dateimenge — **nicht geändert.**
4. **Zwei clazy-Befunde bleiben** in `tests/librarytest.cpp:2241,2247`
   (`range-loop-detach` in `putsTheMessageBetweenTheHeaderAndTheNotes`) — beide
   **Altbestand**, nicht von diesem Zweig berührt. Ebenso in `tests/shelltest.cpp:361`
   und der moc-Fehler in `tests/spellfixspike.cpp` (Spike-Prototyp).
5. **`qInfo()` erreicht auf dieser Maschine das Terminal nicht** — Qt liefert
   an das Journal. Der Selbstprüfungs-Nachweis eines Bildläufers ist also nur
   über `journalctl --user -t editshots` zu lesen; wer nur auf die
   Terminalausgabe sieht, hält einen stummen Lauf für einen wortlosen. Betrifft
   `editshots`, `libraryshots`, `searchshots` gleichermaßen.

## 7a. Nachgeprüft: die Zeichnung ist auf `main` weitergerückt

Nach dem Abzweig dieses Zweiges hat `main` den Commit `88e6dc8` bekommen
(„Wireframe-Symboltafel: Zählkorrektur — fünf Namen für sechs Stellen"). Mein
Arbeitsbaum trug also die Fassung davor. Gegen die **neue** Fassung geprüft:

- **Kein Name hat sich geändert.** Die Korrektur ist redaktionell — sie hält
  fest, dass `edit-delete` Löschen *und* Verwerfen trägt, und nimmt die drei
  Dialogknöpfe aus dem Prüfsatz der Bibliotheks-Schaltflächen heraus, weil ihre
  Symbole aus der KDE-Bauart kommen. Genau so ist es gebaut.
- Dieser Zweig fasst `wireframes/` nicht an; der Merge bleibt konfliktfrei.
  Ein Rebase war nicht nötig — der Zweig braucht `main` nicht.

## 8. Berührte Dateien

| Datei | Was |
|---|---|
| `tests/CMakeLists.txt` | `QT_QPA_PLATFORMTHEME=kde` für `librarytest` |
| `src/ui/librarywindow.cpp` | Wächterdialog auf `KMessageDialog`; fünf Symbole |
| `tests/librarytest.cpp` | Dialogzugriff rollen- → text-/bedeutungsbasiert; zwei neue Tests |
| `tests/editshots.cpp` | Dialogzugriff, Messausgaben, fünftes Bild |
| `SPEC.md` | 9 (Bauart und ihre Bedingungen), 16 (Testumgebungs-Bedingung) |

`src/ui/librarywindow.h` blieb unberührt.
