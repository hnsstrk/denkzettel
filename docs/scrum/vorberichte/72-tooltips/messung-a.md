# Vorprüfung #72 — Messung Bearbeiter A (`denkzettel-dev`)

**Gegenstand:** Issue #72, „Bibliothek: Tooltips mit Tastenkürzel an
Bearbeiten/Löschen/Rückgängig (UI-Review S5, H1)" · **Datum:** 04.08.2026,
Ganymed · **Quellstand:** `main` @ `80b52ae` · **Belege:** `messungen/`,
Sonden in `sonden/`, wiederholbar über
`bash docs/scrum/vorberichte/72-tooltips/pruefen.sh`

Dieser Bericht trägt die Felder **1, 2, 4, 5 und 6**. **Feld 3 (Ready-Urteil)
fällt der Scrum Master.** Die Story ist klein; der Bericht ist entsprechend
kurz gehalten.

**Stand der Werkzeuge** (B17): qt6-base 6.11.1, kwidgetsaddons 6.28.0,
kxmlgui 6.28.0. Sitzungsgebiet `de_DE.UTF-8` — das ist für F6 tragend.

---

## Feld 1 — Dateimenge (am Code vermessen, Notation nach B13)

| | **#72** — Tooltips mit Kürzel an den drei Aktionsflächen |
|---|---|
| **Issue** | **#72** (`epic:M2`, `typ:story`) |
| **Zweig** | `story/72-tooltips` |
| **Quellen & Tests** | `src/ui/librarywindow.cpp` — **drei kleine Stellen**, keine davon strukturell: die Aktionserzeugung `:168–176`, der Kürzelblock `:249–278` (dort hängen die drei `QKeySequence`), der Aufbau der beiden Schaltflächen `:361–369`. Die Meldungszeile `:234` (`m_message->addAction(m_undoAction)`) wird **gelesen, nicht geändert** — „Rückgängig" ist keine Schaltfläche im Code (F2).<br>`tests/librarytest.cpp` — neue Zusicherungen; die Slotliste `:90–290` bekommt Einträge, sachlich in der Nachbarschaft von `saysWhyTheSearchFieldRestsWhileEditing()` (`:191`), wo die einzige heutige Tooltip-Zusicherung steht (`:3109–3124`).<br>`src/ui/librarywindow.h` — **nur falls** ein Zugriffsweg für den Test fehlt; nach heutigem Stand nicht nötig, `findChild` reicht (so arbeiten `searchOf()` und `actionNamed()` bereits). |
| **Build** | **Nichts** — solange der Text von Hand gesetzt wird. **`KF6::XmlGui` neu**, falls der PO den KDE-Automatikweg will (F4): Komponente `XmlGui` in `CMakeLists.txt:24–35` und `KF6::XmlGui` in `src/CMakeLists.txt` bei `denkzettelui` (`:77–83`). Dann kommt **`src/main.cpp` dazu** — der Filter wird app-weit installiert; das ist eine Datei außerhalb der obigen Menge und hebt die Größenklasse (Feld 5). |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-NN-s72-tooltips/`, neu. **Wiederverwendbar, nicht neu zu erfinden:** `sonden/tooltipsonde3.cpp` dieser Vorprüfung ist der fertige Bauplan des Bildbelegs (Tooltip-Fenster als `QTipLabel` greifen und `grab()`en, gemessen: 103×26, `messungen/sonde5-tooltip-offscreen.png`), `sonden/tooltipsonde2.cpp` der des Textbelegs auf der Anzeigeebene. |
| **Fachliche Quellen** | **SPEC 15** (`:720–738`) — `i18n()` für alle sichtbaren Strings **und** die unpersönliche Infinitivform; letzteres nennt das AK 2 ausdrücklich. Die KF6-Liste in demselben Absatz kennt `KXmlGui` nicht — sie ist nachzuziehen, **wenn** der Automatikweg gewählt wird (DoD 4). **UI-Review S5** (`docs/scrum/reviews/sprint-05-ui-review/bericht.md:275–281`) — die Herkunft, samt der drei dort vorgeschlagenen Wortlaute. **Zeichnung 2a/3a** (`wireframes/Denkzettel Wireframes.dc.html:255, :456`) — sie legen fest, dass die Kürzel *Beschleuniger zu sichtbaren Schaltflächen* sind; **`wireframes/` bleibt UX**, der Dev liest sie nur. |
| **Ausdrücklich nicht** | `src/capture/*`, `src/shell/*`, `src/store/*`, `src/analysis/*`, `src/transcribe/*`; in `librarywindow.cpp` die Bereiche `:517–535` (`eventFilter`) und `:700–800` (`showNote`, `scrollTo` `:786`) — **das ist #71**; `src/ui/notelistdelegate.*`, `elidedlines.*`, `pendingdeletion.*`; `tests/libraryshots.cpp` (der Bildläufer der Bibliothek zeigt Tooltips prinzipbedingt nicht, F10 — wer ihn anfasst, arbeitet an der falschen Stelle); `wireframes/`, `SPEC.md` außer der KF6-Liste im Bedingungsfall. |

### Kollisionsfläche — **beide Nachbarn laufen nebenher, mit einer Auflage**

| Vorgang | Berührung | Kleinster Abstand | Urteil |
|---|---|---|---|
| **#83** (native Hülle, `size:l`, ready) | **keine gemeinsame Datei.** #83 lebt in `src/capture/capturewindow.{h,cpp}`, `tests/capturetest.cpp`, `tests/captureshots.cpp`; #72 in `src/ui/librarywindow.cpp` und `tests/librarytest.cpp` | — | **parallel, ohne Auflage** |
| **#71** (Klick auf angeschnittene Zeile) | **dieselbe Datei** `src/ui/librarywindow.cpp`, **und** dieselbe Testdatei | **148 Zeilen** (#72 endet bei `:369`, #71 beginnt bei `eventFilter` `:517`); in `tests/librarytest.cpp` liegen die Einfügestellen der Slotliste rund **47 Zeilen** auseinander (`:136–144` gegen `:191`) | **parallel möglich** — kein Hunk berührt einen anderen, Git mischt mit drei Zeilen Kontext. **Auflage:** getrennte Worktrees und getrennte Zweige (ohnehin Prozess), und wer als Zweiter merged, rebased. Beide fassen `librarywindow.cpp` an; die Konfliktwahrscheinlichkeit ist gering, aber nicht null |

**Ausdrücklich:** Neben `size:l` steht laut `PROZESS.md` nur `size:s`. #72 und
#71 **zusammen** neben #83 wären zwei weitere Stories — das ist eine
Schnittfrage des SM, keine Messfrage. Aus der Dateimenge spricht nichts dagegen.

---

## Feld 2 — Gemessene Fallen

**F1 — Die drei Kürzel sind registriert und feuern.** Eine Beschriftung, die
sie verspricht, verspricht nichts Falsches. `F2`, `Entf` und `Strg+Z` am
gebauten Stand ausgelöst, drei grüne Tests.
*Beleg:* `messungen/sonde4-kuerzel-registriert.txt`
(`undoesTheDeletionByKeyboard`, `deletesWithTheDeleteKey`,
`opensTheEditorWithF2`, 5 passed / 0 failed).

**F2 — „Rückgängig" ist keine Schaltfläche.** `m_editButton` und
`m_deleteButton` sind `QPushButton` (`:361`, `:368`); „Rückgängig" ist eine
`QAction`, die per `m_message->addAction(m_undoAction)` (`:234`) an die
`KMessageWidget` gegeben wird. Diese baut daraus intern einen `QToolButton`
mit `setDefaultAction()`. **Der Knopf ist im Denkzettel-Code nicht greifbar** —
sein Tooltip kommt über `m_undoAction->setToolTip()`.
*Beleg:* `messungen/sonde1-tooltipquellen-offscreen.txt`, Abschnitt C
(2 `QToolButton`-Kinder, beide mit gesetzter `defaultAction`; der gesetzte
Aktionstooltip erscheint am Knopf).

**F3 — `QAction::toolTip()` ist nie leer, und das macht die naheliegende
Zusicherung wertlos.** Ohne eigenes `setToolTip()` liefert die Aktion ihren
`text()` zurück: heute schon `"Rückgängig"`. Ein Test der Bauart
„Tooltip ist nicht leer" ist an der Rückgängig-Fläche **von Anfang an grün**
und prüft nichts.
*Beleg:* `sonde1-…-offscreen.txt`, Abschnitt A (`toolTip()="Rückgängig"` bei
leerem `setToolTip`), Abschnitt C zweiter Teil.

**F4 — Den KDE-Automatikweg gibt es, und er trägt genau eine der drei
Flächen.** `KToolTipHelper` (KF6::XmlGui) hängt das Kürzel selbsttätig an:
gemessen `"Bearbeiten (F2)"`. Er greift aber **nur an `QToolButton` mit
`setDefaultAction()`**; ein `QPushButton` bleibt unberührt — auch nach
`addAction()`. Und `LibraryWindow` ist ein `QWidget`, **kein `KMainWindow`**
(`librarywindow.h:34`), der Filter kommt also nicht von selbst.
Für die zwei Schaltflächen hieße der Automatikweg: Umbau auf `QToolButton`,
neue Bibliothek, Filter in `src/main.cpp`.
*Beleg:* `messungen/sonde2-helfertext-offscreen.txt`, Abschnitte A und B.

**F5 — Wo der Helfer greift, bestimmt die Aktion den Text, nicht der Knopf.**
Ein am Knopf gesetzter Tooltip wird **ignoriert**; komponiert wird aus
`defaultAction()->toolTip()`. Wer den Wortlaut steuern will, setzt ihn an der
`QAction`.
*Beleg:* `sonde2-…-offscreen.txt`, Abschnitt A, Zeilen 3 und 4
(`"KNOPF-EIGENER-TEXT"` erscheint in keinem Lauf).

**F6 — Der Kürzeltext ist gebietsabhängig, und der CI-Lauf steht im
C-Gebiet.** `QKeySequence::toString(NativeText)` liefert unter
`LANG=de_DE.UTF-8` `"Entf"` und `"Strg+Z"`, unter `LANG=C` dagegen `"Del"` und
`"Ctrl+Z"`. Der CI-Container ist `archlinux:base-devel` ohne gesetztes `LANG`
(`.github/workflows/ci.yml:43`). **Eine Zusicherung auf `"Entf"` wird auf
Ganymed grün und im öffentlichen Lauf rot.**
*Beleg:* `messungen/sonde2-helfertext-offscreen.txt` gegen
`messungen/sonde3-locale-C.txt` (`Entf="Del"  Undo="Ctrl+Z"`).

**F7 — `Strg+Z` ist eine Einstellung, kein Fakt.** `KStandardShortcut::undo()`
liest `kdeglobals`; hier ist es **ein** Kürzel, `Strg+Z`. Ein von Hand in den
`i18n()`-String geschriebenes „Strg+Z" wäre bei jedem Nutzer falsch, der es
umgestellt hat — und bei jedem Lauf im C-Gebiet (F6). Der Text gehört aus
`action->shortcut().toString(QKeySequence::NativeText)` gebaut.
*Beleg:* `sonde1-…-offscreen.txt`, Abschnitt A (`undo hat 1 Kürzel: [Strg+Z]`).

**F8 — SPEC 15 gilt auch für den zusammengesetzten Text.** Der Kürzelteil kommt
aus Qt, der Rest muss durch `i18n()`; die Zusammensetzung braucht also einen
Platzhalterstring (`i18nc("@info:tooltip", "%1 (%2)", …)`), keine
`+`-Verkettung. Dazu die Infinitivform desselben Absatzes.
*Beleg:* `SPEC.md:733–738`; heutige Machart am Suchfeld
`librarywindow.cpp:1070`.

**F9 — Beide Schaltflächen starten abgeschaltet, und „Rückgängig" existiert
meist gar nicht.** `setEnabled(false)` in `:250`, `:255`, `:263`; die
Meldungszeile ist `hide()` (`:235`) und kommt nur während der Löschfrist. Wer
einen Zustand prüfen oder abbilden will, muss ihn erst herstellen — eine Notiz
auswählen bzw. löschen. Qt zeigt Tooltips an abgeschalteten Widgets weiterhin
an (gemessen: `QApplication::widgetAt` trifft den abgeschalteten Knopf,
Tooltip erscheint).
*Beleg:* `sonde2-…-offscreen.txt`, Abschnitt C.

**F10 — `QWidget::grab()` zeigt einen Tooltip nie.** Er ist ein eigenes
Fenster. Der Bildläufer `libraryshots` kann diese Story deshalb nicht belegen —
und ist entsprechend aus der Dateimenge ausgenommen.
*Beleg:* `messungen/sonde5-tooltipbild-offscreen.txt`, Abschnitt A.

---

## Feld 4 — Prüfmittel, und was ein Agent nicht kann

| Was | Prüfmittel | Was es **nicht** belegt |
|---|---|---|
| Der hinterlegte Text | `QWidget::toolTip()` bzw. `QAction::toolTip()` im QTest — so arbeitet die einzige heutige Tooltip-Zusicherung (`librarytest.cpp:3109–3124`) | **Nicht, dass je ein Tooltip erscheint**, und nicht, was angezeigt wird: greift `KToolTipHelper`, weicht der angezeigte Text vom hinterlegten ab (F4/F5). An der Rückgängig-Aktion ist er zudem nie leer (F3) |
| Der **angezeigte** Text, ohne Zeigerbewegung | `QHelpEvent(QEvent::ToolTip)` an das Widget senden, danach `QToolTip::text()` und `QToolTip::isVisible()` lesen. **Gemessen: geht offscreen** und liefert den zusammengesetzten Text (`"Bearbeiten (F2)"`) | Nicht die **Verweilzeit** und nicht die Stelle, an der der Tooltip im Betrieb aufgeht — das Ereignis wird hier von Hand zugestellt, nicht vom Zeiger ausgelöst |
| Bildbeleg | Das Tooltip-Fenster ist als `QTipLabel` unter `QApplication::topLevelWidgets()` greifbar und `grab()`bar. **Gemessen: 103×26, `messungen/sonde5-tooltip-offscreen.png`**, Text lesbar | Offscreen zeichnet weder Theme noch Compositor vollständig (B21): belegt sind **Text und Textsatz**, nicht Hülle, Rundung, Schatten oder Durchsichtigkeit des Tooltip-Fensters. Behauptet ein AK darüber etwas, gehört ein Sitzungsbild dazu — die AK-Entwürfe tun es nicht |
| Der Kürzeltext | Zusicherung gegen `action->shortcut().toString(NativeText)`, **nicht** gegen ein Literal (F6/F7) | — |

**Was ein Agent grundsätzlich nicht kann:** *Den Tooltip durch Verweilen
auslösen.* Unter Wayland kann ein Prozess den Zeiger nicht bewegen — dieselbe
Bauart wie das nicht auslösbare Alt-Tab (Sprint 6 §16.1, M-B1) und wie
`activateWindow()` (`.claude/agents/denkzettel-dev.md`, Punkt 3). Belegbar ist,
**was** ein zugestelltes `QEvent::ToolTip` erzeugt; **dass** der Zeiger es nach
der üblichen Verweilzeit erzeugt, bleibt dem Blick des Kunden. Diese Grenze ist
benannt, nicht überbrückt.

---

## Feld 5 — Größenklasse: **`size:s`**

Bedeutung laut `PROZESS.md`: *„läuft nebenher — wenige Dateien, kein neuer
Prüfweg"*.

**Wofür `s`:** eine Quelldatei, drei kleine Stellen, keine Kopfdatei, **kein
Bau** (F4, Bedingungsfall ausgenommen), keine SPEC-Änderung, keine
Bibliothek. Der Prüfweg ist nicht neu erfunden, sondern liegt als Sonde vor —
Textbeleg und Bildbeleg sind in dieser Vorprüfung schon gelaufen. Kein
Kriterium spricht über Hülle, Compositor oder Skalierung.

**Die Bedingung, unter der es `m` wird** (Feld 6, Frage 1): Verlangt der PO den
KDE-Automatikweg über `KToolTipHelper`, kommen `KF6::XmlGui` in zwei
CMake-Dateien, ein app-weiter Ereignisfilter in `src/main.cpp`, der Umbau
zweier `QPushButton` auf `QToolButton` und ein SPEC-15-Nachzug hinzu. Dann
trägt es einen Strang.

---

## Feld 6 — Offene Fragen an PO oder Kunde

1. **Handarbeit oder KDE-Automatik?** `KToolTipHelper` trägt ohne Umbau genau
   die Rückgängig-Fläche; die zwei Schaltflächen bräuchten `QToolButton`,
   `KF6::XmlGui` und einen Filter in `src/main.cpp` (F4). *Dev-Empfehlung:
   Handarbeit* — drei `setToolTip()`-Zeilen gegen eine neue Bibliothek und
   einen Widget-Umbau; die Automatik zahlt sich erst aus, wenn viele Flächen
   dazukommen. Das entscheidet Dateimenge, Build **und** Größenklasse.
2. **Kürzeltext aus `toString(NativeText)` statt Literal — ins AK?**
   *Dev-Empfehlung: ja.* Sonst steht „Strg+Z" auch dort, wo es „Ctrl+Z" heißt
   (F6) oder wo der Nutzer es umgestellt hat (F7), und eine Zusicherung darauf
   wird im CI-Lauf rot.
3. **AK 1 nennt „Rückgängig" als eine der Schaltflächen.** Im Code ist es eine
   `QAction` in der Meldungszeile (F2). Wortlaut anpassen — oder bleibt es bei
   „Fläche"?
4. **Wortlaut der drei Tooltips.** Der UI-Review schlägt „Bearbeiten (F2)" vor
   — das wiederholt die Beschriftung. SPEC 15 verlangt die unpersönliche
   Infinitivform; „Notiz bearbeiten (F2)" erfüllt beides und sagt mehr.
   Kundenwort.
5. **Bleiben „Speichern" (Strg+Enter) und „Abbrechen" (Esc) außen vor?** Ihre
   Kürzel stehen heute nur als Fußzeilentext (`librarywindow.cpp:455`), an den
   Knöpfen nicht. Das AK nennt sie nicht — Absicht oder Lücke?
6. **Reihenfolge gegen #71.** Beide fassen `librarywindow.cpp` an, ohne
   Überschneidung (Feld 1). Parallel zulässig; ob der Schnitt es will,
   entscheidet der SM.

---

## Was ich **nicht** klären konnte

- Ob Qt in der **angemeldeten Sitzung** einen Tooltip an einem abgeschalteten
  Knopf tatsächlich aufgehen lässt (F9). Gemessen ist, dass
  `QApplication::widgetAt` ihn trifft und ein zugestelltes Ereignis den Tooltip
  zeigt; die Zustellung durch den Zeiger konnte ich nicht auslösen.
- Wie der Tooltip **aussieht** — Hülle, Schatten, Durchsichtigkeit. Offscreen
  belegt nur den Text (B21).

## Befehle, mit denen ich gemessen habe

```
bash docs/scrum/vorberichte/72-tooltips/pruefen.sh
gh issue view 72 ; gh issue view 71 ; gh issue view 83
grep -n "ToolTip\|QAction\|QKeySequence\|QPushButton\|i18n" src/ui/librarywindow.cpp
grep -n "^void LibraryWindow::" src/ui/librarywindow.cpp
grep -rn -i "shortcut" /usr/include/KF6/KXmlGui/ktooltiphelper.h
pacman -Q kwidgetsaddons kxmlgui qt6-base
```

**Nicht getan:** nichts committet, nichts gepusht, nichts nach `/usr`
installiert, keine Zeile unter `src/`, `tests/`, `SPEC.md` oder `wireframes/`
geändert. Der Bauplatz der Sonden liegt unter
`docs/scrum/vorberichte/72-tooltips/build/` und ist von `.gitignore` gedeckt;
`build/` der Repositoriumswurzel wurde nur **gelesen** (Sonde 4 startet den dort
gebauten `librarytest`, ohne ihn neu zu bauen).
