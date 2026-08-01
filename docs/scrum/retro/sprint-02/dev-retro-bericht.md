# Retro-Stellungnahme Sprint 2 — Entwicklersicht (Agent `dev-retro-s2`, 01.08.2026, eingegangen 08:22)

Alle Befunde am Quellstand `091fcc5` geprüft, dazu Systemjournal, kglobalacceld-Fremdquellcode und eine eigene Messung am laufenden Fenster. Zwei meiner Aussagen widersprechen der bisherigen Protokolllage; ich benenne das jeweils.

## 1. Befund 1 — Meta+N ohne Wirkung (#5)

**Die Ursachenkette ist lückenlos belegbar, und sie liegt nicht dort, wo der Issue-Kommentar sie vermutet.**

**(a) Warum kglobalacceld die Komponente nicht anlegt.** `componentName()` bildet den Namen als `desktopFileName() + ".desktop"` (`src/shell/globalshortcuts.cpp:23`), also `org.denkzettel.Denkzettel.desktop`. kglobalacceld behandelt jeden Komponentennamen mit `.desktop`-Endung als Service-Action-Komponente und verlangt dafür eine auflösbare Desktop-Datei — aus `src/globalshortcutsregistry.cpp` von `plasma/kglobalacceld`:

```cpp
KService::Ptr service = KService::serviceByStorageId(uniqueName);
if (!service) {
    const QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                    QLatin1String("kglobalaccel/") + uniqueName);
    if (filePath.isEmpty()) {
        return nullptr;      // keine Komponente
    }
    ...
```

Findet sich weder ein KService noch eine Datei unter `<Datenverzeichnis>/kglobalaccel/<name>`, gibt `createServiceActionComponent()` `nullptr` zurück, und `getOrCreateComponent()` reicht das weiter. Genau das erklärt den PO-Befund „`allMainComponents` ohne denkzettel, Komponentenpfad `UnknownObject`" — es gibt nicht einmal einen leeren Eintrag.

Die Desktop-Datei war zum Prüfzeitpunkt nirgends im System, und sie ist es bis jetzt nicht:

- `/usr/share/applications/` enthält keinen denkzettel-Eintrag, `/etc/xdg/autostart/` ebenfalls nicht.
- `build/install_manifest.txt` verweist auf `/usr/local/share/applications/org.denkzettel.Denkzettel.desktop` — **keine der fünf dort gelisteten Dateien existiert**; der Installationslauf ist nie durchgekommen.
- Unabhängige Bestätigung aus dem Journal des Kundenlaufs: `Aug 01 01:39:06 denkzetteld[395043]: Failed to register with host portal … "Could not register app ID: App info not found for 'org.denkzettel.Denkzettel'"`. Ein zweites Subsystem meldet dieselbe Ursache.
- Gegenprobe: Alle elf bei kglobalacceld registrierten Komponenten (`org.kde.spectacle.desktop`, `com.mitchellh.ghostty.desktop`, …) gehören zu Anwendungen mit installierter Desktop-Datei. `~/.config/kglobalshortcutsrc` enthält keinen denkzettel-Abschnitt.

Die fehlende systemweite Installation ist damit nicht ein „Verdachtsmoment", sondern die belegte Ursache.

**(b) Warum keine Meldung kam — und warum die Erststart-Bindung dafür nichts kann.** Hier korrigiere ich die bisherige Einschätzung. Der Rückgabewert von `KGlobalAccel::setGlobalShortcut()` kann einen Backend-Fehlschlag strukturell nicht anzeigen; aus `frameworks/kglobalaccel/src/kglobalaccel.cpp`:

```cpp
if (checkGarbageKeycode(shortcut)) { return false; }
if (!doRegister(action))          { return false; }
...
```

`doRegister()` prüft nur, ob die Aktion einen brauchbaren `objectName()` hat, setzt den D-Bus-Aufruf `iface()->doRegister(actionId)` ab **und wertet dessen Ergebnis nicht aus**. Ein `false` gibt es also nur bei Müll-Tastencodes oder namenloser Aktion — nie, wenn der Daemon ablehnt oder gar nicht erreichbar ist. Deshalb lief der Meldezweig in `globalshortcuts.cpp:65-74` nicht an.

Die Erststart-Bindung in `src/main.cpp:82` (`if (firstRun && !conflicts.isEmpty())`) ist an diesem Befund **unbeteiligt**, und sie ist auch keine Umsetzungslücke: SPEC 2.4 schreibt wörtlich „**Beim Erststart und bei Kürzel-Änderung** prüft Denkzettel die Sequenz gegen die bestehende Belegung". Die Umsetzung folgt der Spec. Wichtiger noch: Es gab überhaupt nichts zu melden — dieselbe SPEC-Stelle hält fest, dass Meta+N auf Ganymed frei ist. Die Konfliktliste war korrekt leer. **Auch mit gelöschtem `FirstRunDone`-Marker wäre keine Meldung erschienen**; der in Protokoll 7.4, Punkt 4 vorgesehene Prüfweg hätte den Fehler nicht aufgedeckt.

Die eigentliche Lücke ist eine andere und sie liegt bei mir: Das Akzeptanzkriterium lautet „ein **Fehlschlag** erzeugt eine sichtbare Meldung statt still zu scheitern". Gebaut wurde eine **Konflikt**erkennung (fremde Belegung derselben Taste) plus ein Fehlschlagzweig an einem Rückgabewert, der Fehlschläge nicht kennt. Dass dieser Rückgabewert Backend-Probleme nicht abfängt, steht als Grenze im S4-Bericht — sie wurde protokolliert statt geschlossen. Eine erkannte Grenze in eine Fußnote zu schreiben und die Story trotzdem als AK-erfüllt zu melden, ist der Kernfehler des Laufs.

**(c) Was der Entwicklerlauf hätte tun können.** Der Beleg stand im Systemjournal, während gebaut wurde:

```
Jul 31 23:51:45 denkzetteld[326944]: Couldn't start kglobalaccel from org.kde.kglobalaccel.service:
    QDBusError("org.freedesktop.DBus.Error.ServiceUnknown", …)
Jul 31 23:58:55 denkzetteld[334534]: (dieselbe Zeile)
```

Beide Läufe liegen um den S4-Commit `3fe30b9` (23:53). Der Daemon lief, KGlobalAccel erreichte kglobalacceld nicht einmal — und das Programm lief stumm weiter (eine D-Bus-Aktivierungsdatei `org.kde.kglobalaccel.service` gibt es auf diesem System nicht; kglobalacceld kommt mit der Plasma-Sitzung). Drei Dinge hätten gereicht, jedes einzeln:

1. Nach dem Start ins Journal sehen (`journalctl --user -t denkzetteld -n 20`).
2. Ein einziger Lesebefehl gegen den Daemon: `qdbus6 org.kde.kglobalaccel /kglobalaccel org.kde.KGlobalAccel.allMainComponents | grep denkzettel` — exakt der Befehl, mit dem der PO den Fehler dann in einer Minute gefunden hat.
3. Programmatische Rückprüfung im Code: `KGlobalAccel::self()->globalShortcut("org.denkzettel.Denkzettel.desktop", "show_capture")` liest laut Header (`/usr/include/KF6/KGlobalAccel/kglobalaccel.h:325`) den Stand „as defined in global settings", also beim Daemon. Kommt eine leere Liste zurück, ist die Registrierung nicht angekommen — das ist der Fehlschlag-Detektor, den AK 4 verlangt und den der Rückgabewert nicht liefern kann.

Bemerkenswert und für die Retro der eigentliche Stachel: Das nötige Wissen lag im Team vor. Protokoll 7.4, Punkt 2 sagt „**erst nach systemweiter Installation**, weil kglobalacceld die Komponente über die Desktop-Datei auflöst". Diese Erkenntnis wurde nur auf die Sichtbarkeit im Systemeinstellungs-Modul angewandt — nicht auf die Frage, ob das Kürzel ohne Installation überhaupt funktionieren kann. Die Antwort ist nein, und sie stand implizit schon da.

## 2. Befund 2 — Bibliotheks-Layout (#7)

**Ursache: eine fehlende Zahl in `src/ui/librarywindow.cpp:162`.**

```cpp
layout->addWidget(buildHeader());
layout->addWidget(m_message);
layout->addWidget(m_splitter);      // ohne Stretch-Faktor
```

Ein horizontaler `QSplitter` hat vertikal die Größenpolitik **Preferred**, nicht Expanding (gemessen: `vPolicy=5`). Die Kopfzeile ist ein nacktes `QWidget`, ebenfalls Preferred. Damit gibt es im äußeren `QVBoxLayout` keinen Posten mit Expand-Flag und keinen Stretch-Faktor — Qt verteilt die überschüssige Höhe gleichmäßig auf alle wachstumsfähigen Posten. Innerhalb der übergroßen Kopfzeile zentriert deren eigenes Layout das Suchfeld, weil ein `QLineEdit` vertikal fix ist.

Gemessen am **echten** `LibraryWindow` (gegen `build/lib/libdenkzettelui.a` gelinkte Probe, offscreen, 900×600):

```
Suchfeld : y=137 h=25   vPolicy=0
Kopfzeile: y=0   h=300  vPolicy=5
Splitter : y=300 h=300  vPolicy=5
```

Kopfzeile und Splitter je exakt die halbe Fensterhöhe — das ist der Kundenbefund in Zahlen. Mit `layout->addWidget(m_splitter, 1)` ergibt dieselbe Messung Kopfzeile `h=41`, Splitter `y=41 h=559`, Suchfeld bei `y=8`. Der UX-Kollege hat parallel dieselbe Reproduktion mit Bildern gebaut (`ist.png`/`soll.png` im Session-Scratchpad); unsere Zahlen decken sich.

**Warum 38 Tests das nicht fingen.** Nicht wegen fehlender Infrastruktur — die steht vollständig: `tests/CMakeLists.txt` setzt `QT_QPA_PLATFORM=offscreen`, und `tests/librarytest.cpp` zeigt an neunzehn Stellen echte Fenster mit `QTest::qWaitForWindowExposed()`. Es fehlt ausschließlich die **Eigenschaft**, die geprüft wird: In der ganzen Datei steht keine einzige Zusicherung über `y()`, `height()`, `geometry()` oder `sizeHint()`. Über das Suchfeld sagt der Test nur `QVERIFY(search->isVisible())` — und ein Widget in der Fenstermitte ist sichtbar. Der Test war nicht falsch, er prüfte das Falsche.

Der Kopfkommentar der Datei sagt das sogar selbst (`tests/librarytest.cpp:29-32`): „Unit tests of the library building blocks that work without a visible window … The window itself — layout, empty states, the look of the message widget — stays on the manual checklist." Das war eine bewusste Grenzziehung, und sie war falsch gezogen: Geometrie ist offscreen genauso messbar wie jeder andere Zustand. Meine eigene Probe hat das in zwanzig Minuten gezeigt, ohne Compositor, ohne Screenshot-Vergleich.

**Zur Frage, ob ich das Fenster je angesehen habe: nein.** Kein Dev-Lauf dieses Sprints hat ein gerendertes Bibliotheksfenster erzeugt oder Geometrie ausgelesen; im Repo gibt es kein Bild-Artefakt und in den Tests keine Positions-Zusicherung. Sichtbar gerendert haben es nur die `uiprobe`-Läufe des UI-Reviews (Journal, 01.08. 00:58–01:02) — auch die haben Texte und Zustände geprüft, nicht Positionen. Es gibt keine Umgebung, in der das Fenster anders ausgesehen hätte; der Fehler ist deterministisch und tritt bei jeder Fenstergröße auf, in der Kopfzeile und Splitter zusammen kleiner sind als das Fenster.

## 3. Was gut lief

Kurz und ohne Schönfärberei:

- **Das Herausziehen der Auswertungslogik nach `shortcutconflict`** war richtig: Der nicht testbare Teil (D-Bus-Gespräch mit dem Daemon) ist von der testbaren Regel getrennt, vier `shelltest`-Funktionen decken sie ab. Das Muster taugt auch für die Heilung.
- **Die Mutationstests des Scrum Masters** haben echten Erkenntniswert erzeugt, nicht nur Bestätigung: Mutation 2 hat die im Testkommentar behauptete Qt-Mechanik widerlegt und den tatsächlichen Weg über `closeEvent` belegt. Eine Testsuite, deren Aussagen jemand aktiv anzugreifen versucht, ist mehr wert als eine mit mehr Testfunktionen.
- **Die Logikabdeckung von S5 trägt tatsächlich**: Löschfrist, Undo-Kanten, Zeitstempel-Umschaltpunkte, Nachladen bei offenem Fenster — dieser Teil war beim Kunden inhaltlich in Ordnung, und das war kein Zufall.
- **Die Selbstauskunft von `installtest.cmake`** im Kopfkommentar („dass die Install-Regeln existieren, wird hier geprüft; ob die Sitzung sie liest, nicht") ist die Art Ehrlichkeit, die wir brauchen. Sie hat allerdings auch gezeigt, wo ihre Grenze liegt: Der Test installiert nach `DESTDIR`-Staging und sagt deshalb nichts darüber, ob am Prüfsystem etwas installiert **ist**.

## 4. Prozess-Änderungsvorschläge

Vier Punkte, jeder als überprüfbarer DoD-Satz formuliert.

**V1 — Selbst-Sichtprüfung des Devs am laufenden System, als DoD-Punkt.** Ergänzung zu PROZESS.md, DoD 1: *Bei jeder Story mit sichtbarem oder systemweit registriertem Verhalten startet der Dev den gebauten Stand, führt den Hauptweg der Story einmal selbst aus und legt den Nachweis in den Bericht (Terminalausgabe, Journalauszug oder Bild).* Beide Sprint-2-Befunde wären daran gescheitert — Meta+N an der leeren Komponentenliste, das Layout am ersten Blick auf ein `grab()`.

**V2 — Layout-Invarianten als Testfunktionen, ab sofort verbindlich für jede Ansicht.** Kein neues Werkzeug nötig; offscreen und `qWaitForWindowExposed` sind da. Form:

```cpp
window.resize(900, 600);
window.showLibrary();
QVERIFY(QTest::qWaitForWindowExposed(&window));
QVERIFY(search->mapTo(&window, QPoint()).y() < 40);          // Kopfzeile oben
QVERIFY(splitter->height() > window.height() * 3 / 4);       // Rest gehört der Liste
```

Am heutigen Stand wären beide Zeilen rot (137 statt <40; 300 statt >450) — die Wirksamkeit ist gemessen, nicht behauptet. Regel: *Jede Ansicht bekommt eine Testfunktion, die bei zwei Fenstergrößen prüft, welcher Bereich mitwächst und welcher nicht.* Vom Pixelvergleich rate ich ab: Schriftrendering und Theme machen ihn flackrig, und eine Wache, die ständig grundlos anschlägt, wird nach zwei Wochen ignoriert.

**V3 — Definierte Abnahme-Umgebung: geprüft wird der installierte Stand.** Der Sprint hat die Regel bereits zweimal berührt (7.4 Punkt 2 und Punkt 6), ohne sie zu ziehen. Vorschlag: *Vor der Sichtprüfung wird mit `-DCMAKE_INSTALL_PREFIX=/usr` installiert; der Prüfling ist die installierte Binärdatei, nicht das Build-Verzeichnis. Die Vorbedingungsliste im Sprint-Protokoll nennt Installationsschritt und Prüfbefehl.* Der Nebeneffekt ist wichtig: Damit wird auch der D-Bus-/Portal-/Desktop-Datei-Pfad mitgeprüft, der sich im Build-Verzeichnis prinzipiell nicht prüfen lässt.

**V4 — Stille Fehlpfade sind verboten; jede Registrierung wird zurückgelesen.** Nicht „meldet, wenn der Rückgabewert `false` ist", sondern: *Wo eine Registrierung bei einem fremden Dienst stattfindet (KGlobalAccel, D-Bus-Namen, Tray, Portale), prüft der Code anschließend beim Dienst nach, ob sie angekommen ist, und meldet den Fehlschlag bei jedem Start sichtbar.* Für S4 heißt das konkret: nach `setGlobalShortcut` ein `globalShortcut(componentName, "show_capture")` lesen und bei leerer Antwort die Notification auslösen, mit dem Hinweis auf die fehlende Installation. Und die Ergänzung an uns selbst: *Wenn ein Bericht eine Grenze der Prüfbarkeit benennt, ist die Story nicht fertig, sondern die Grenze ist entweder geschlossen oder als Impediment eskaliert* — nicht als Fußnote abgelegt.

## 5. Werkzeuge

Recherche durchgeführt (nichts installiert). Vorab der unbequeme Befund: **keiner der sechs genannten Kandidaten hätte einen der beiden Sprint-2-Befunde gefunden.** Kein Linter kennt einen Check zu Stretch-Faktoren oder Größenpolitik, und keiner prüft, ob eine D-Bus-Registrierung beim Daemon ankommt. Wer hier Werkzeuge kauft, kauft sie gegen andere Probleme — das kann sinnvoll sein, löst aber nicht das, was uns die Abnahme gekostet hat.

Die Messlatte ist außerdem hoch: `KDECompilerSettings` setzt bereits `-Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -Wzero-as-null-pointer-constant -Wsuggest-override -Wlogical-op`, dazu `QT_NO_CAST_FROM_ASCII`, `QT_NO_KEYWORDS`, `QT_NO_NARROWING_CONVERSIONS_IN_CONNECT`.

**Empfehlung, zwei Werkzeuge:**

**clazy (Rang 1).** Das einzige Werkzeug im Feld, das Qt-Semantik versteht, und das, was KDE selbst in seiner CI fährt. Version 1.17.1 (März 2026), liegt fertig in `extra/`. Es findet, was weder GCC noch clang-tidy sehen: sechzehn Signal/Slot-Checks — darunter `connect-3arg-lambda` (Lambda ohne Kontextobjekt, Absturz nach Empfängertod), `connect-by-name`, `connect-non-signal` — sowie `auto-unexpected-qstringbuilder` (dangling `QStringBuilder`), `range-loop-detach` (versehentliche Deep-Copies) und `qstring-allocations`. Für unseren Code mit vielen `connect()`-Aufrufen ist das real. Einstieg `level0,level1`, Fixit-Automatik nicht einschalten (das Upstream-README warnt selbst davor; in einem Agentenbetrieb editiert sie hinter dem Rücken des Agenten). Blinder Fleck, den man wissen muss: **für `i18n()` ist clazy blind** — es kennt nur `tr()`-Checks. Diese Lücke schließt eine ripgrep-Zeile in einem PostToolUse-Hook billiger als jedes Werkzeug.

**clang-tidy mit engem Checkset (Rang 2).** Bereits installiert, kostet also nur Konfiguration. `Checks: '-*, bugprone-*, performance-*, misc-const-correctness'`, `HeaderFilterRegex` ohne `_autogen`, `SKIP_LINTING` auf `mocs_compilation.cpp`, und als **eigenes CMake-Target**, nicht als `CMAKE_CXX_CLANG_TIDY` — sonst zahlt jeder Build die Analyse mit. Ein Check verdient besondere Erwähnung, weil er V4 teilweise erzwingt: `bugprone-unused-return-value` nimmt in `CheckedFunctions` eine eigene Funktionsliste entgegen; damit ist „der Rückgabewert einer Registrierung darf nicht ignoriert werden" reine Konfiguration. Noch billiger und vom Compiler geprüft: `[[nodiscard]]` an unseren eigenen Wrappern. `cppcoreguidelines-*` bitte nicht — es widerspricht dem Qt-Parent-Child-Modell systematisch.

**Abgelehnt, mit Begründung:** *semgrep* — die C++-Unterstützung ist in der OSS-Engine ausdrücklich experimentell (Hersteller wörtlich: „the languages will stay experimental"; interprozedural nur im Bezahlprodukt), die Regel-Registry enthält kein `cpp/`-Verzeichnis, und `#ifdef` bricht den Parser (offenes P0 von April 2026). Der gefährliche Teil ist der stille Fehlermodus: Eine Datei, die nicht vollständig geparst wird, liefert keine Treffer und sieht aus wie eine bestandene Prüfung. *include-what-you-use* — kein Qt-6-Mapping, das Qt-Kernproblem ist seit 2015 offen, kein KDE-Projekt nutzt es. *cppcheck* — echter Zusatznutzen durch ValueFlow, aber starke Überlappung mit clang-tidy; allenfalls als seltener CI-Lauf mit `--library=qt` und ohne `unusedFunction` (das meldet jeden Slot als tot). *pre-commit* — greift zum falschen Zeitpunkt (ein Agent schreibt zwanzigmal und committet einmal) und schafft ein zweites Werkzeugverzeichnis neben dem Build-Compiler; sinnvoll wäre allein `clang-format`, und dafür braucht es das Framework nicht.

**Was ich dem Kunden stattdessen ans Herz lege:** V1 und V2 kosten zusammen etwa eine Stunde und hätten beide Befunde verhindert. clazy und clang-tidy sind gute Ergänzungen für Fehlerklassen, die wir bisher nicht getroffen haben — aber die Lehre aus Sprint 2 ist nicht „uns fehlt ein Linter", sondern: **die Tests haben die falsche Eigenschaft geprüft, und niemand hat das Programm angesehen, bevor es der Kunde ansah.**

---

**Belege zum Nachvollziehen:** Messprogramm und Ausgabe unter `scratchpad/dev-nachpruefung/` (`probe.cpp`, `echt.png`); Reproduktion des UX-Kollegen unter `scratchpad/layoutprobe.cpp` mit `ist.png`/`soll.png`. Kein Projektcode geändert, kein Commit, nichts installiert.
