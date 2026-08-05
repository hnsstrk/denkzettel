# Übergabebericht #61 — Versionsanzeige und Versionsregeln

**Sprint 8, Strang B** · Zweig `story/61-versionsanzeige`, Ausgangsstand
`sprint-08-basis` · Ganymed, 05.08.2026

**Commits:** `de2ae9a` (Umsetzung und Prüfmittel), `e79977b` (SPEC),
`3905438` (Kommentare und `const`-Hinweise)

**Werkzeugstand** (B17): kcoreaddons 6.28.0, kdbusaddons 6.28.0, qt6-base
6.11.1, extra-cmake-modules 6.28.0, cmake 4.4.2, dbus 1.16.2, clazy 1.17.1,
gcc 16.1.1

**Wiederholbar:** `bash docs/scrum/reviews/sprint-08-s61-versionsanzeige/pruefen.sh`
· `bash docs/scrum/reviews/sprint-08-s61-versionsanzeige/mutationsproben.sh`
Alle Ausgaben liegen in `messungen/`.

---

## 1. Was gebaut wurde

Drei Bauteile, wie der Vorprüfbericht sie gezählt hat.

**Der Durchreichweg.** `src/CMakeLists.txt:120–124` gibt `${PROJECT_VERSION}`
als Übersetzungsdefinition `DENKZETTEL_VERSION` an `denkzettelshell` weiter.
Die Nummer selbst steht unverändert allein in `CMakeLists.txt:3`.

**Die Registrierung.** `src/shell/appidentity.cpp` legt das `KAboutData` an und
setzt **vor** `setApplicationData()` Organisationsdomäne und Desktop-Dateinamen
am Objekt. `src/main.cpp` setzt sie nicht mehr daneben — die drei Handsetzer
sind entfallen. Das ist die Antwort auf die teuerste Falle: Zwei Setzer für
denselben Wert entschieden die Frage nach Zeilenreihenfolge, und die falsche
Reihenfolge meldet niemand.

**Die Argumentbehandlung.** `processCommandLineArguments()` läuft in
`src/main.cpp:35`, also vor `KDBusService` (`:49`). Angemeldet sind `--help`
und `--version`, sonst nichts; `parser.process()` weist alles andere zurück.

**Warum nicht `KAboutData::setupCommandLine()`:** Es brächte `--desktopfile
<Dateiname>` mit — genau den Wert, an dem SPEC 2.4 hängt, auf der
Kommandozeile. Unangemeldet fällt der Schalter unter die zurückgewiesenen. Das
ist die schärfere Lesart von AK 7 („wird nicht ausgewertet"): Er wird nicht
einmal angenommen. Der Preis sind `--author` und `--license`, die niemand
verlangt hat und die das Projekt inhaltlich auch nicht füllt.

**Zwei neue Prüfmittel.** `tests/identitytest.cpp` liest die drei Namen nach der
Registrierung im Prozess zurück; `tests/commandlinetest.cpp` misst den
**gebauten Dienst** über `QProcess` und läuft dazu unter `dbus-run-session`.

## 2. Akzeptanzkriterien

| AK | Prüfmittel | Ergebnis | Mutationsprobe |
|---|---|---|---|
| **1** `--version` gibt die Version aus, Rückgabe 0, auch bei laufendem Dienst und ohne Sitzungsbus | `commandlinetest::writesTheVersionOfTheBuildAndEndsWithoutASessionBus`; dazu `messungen/ak1-version-ohne-bus.txt` (Wortlaut `env -u DBUS_SESSION_BUS_ADDRESS`) und `messungen/ak1-version-bei-laufendem-dienst.txt` | **erfüllt.** `denkzettel 0.1.0`, Rückgabe 0 — für `--version` und `-v`, ohne Busvariable, mit unerreichbarem Bus und gegen den **echten** Sitzungsbus bei laufendem Dienst; der Dienst lief danach unberührt weiter | **M4** (Auswertung hinter `KDBusService`) → rot. **M5** (Registrierung hinter der Auswertung) → rot |
| **2** Die Zeile trägt den Anwendungsnamen, nicht den Busnamen | derselbe Prüfsatz, Vergleich auf `denkzettel <Nummer>` im Wortlaut | **erfüllt.** `denkzettel 0.1.0`, nie `Daemon 0.1.0` | **M5** → rot; **M3** (Komponentenname `Denkzettel`) → rot |
| **3** Die Nummer steht an genau einer Stelle, der Weg von CMake in den Code ist gebaut | `messungen/ak3-einzige-stelle.txt`; der Prüfsatz vergleicht die Ausgabe gegen `${PROJECT_VERSION}` | **erfüllt.** `git grep -F 0.1.0 -- src tests`: **0 Treffer**. Einzige Fundstelle `CMakeLists.txt:3` | **M9** (`DENKZETTEL_VERSION="9.9.9"` statt `${PROJECT_VERSION}`) → rot |
| **4** Die Registrierung bricht die Busnamen nicht | `commandlinetest::startsTheDaemonWithoutAnArgument` liest den **tatsächlich angemeldeten** Namen in einer eigenen Sitzung zurück; `identitytest` prüft Domäne, Desktop-Datei und Datenpfad; `messungen/ak4-busname.txt` | **im Testlauf erfüllt** — `org.denkzettel.Daemon`. **Am installierten Stand offen**, siehe Abschnitt 4 | **M1** (Domäne) → rot in beiden Prüfsätzen. **M2** (Desktop-Datei) → rot. **M3** (Name/Datenpfad) → rot |
| **5** Die globalen Kürzel funktionieren unverändert | mittelbar über AK 4: die Kürzel hängen an `desktopFileName()`, das `identitytest` zurückliest | **am installierten Stand offen**, siehe Abschnitt 4 | **M2** → rot |
| **6** Unbekannter Schalter abgewiesen, argumentloser Start startet den Dienst | `commandlinetest::refusesASwitchNobodyDeclared` und `::startsTheDaemonWithoutAnArgument`; `messungen/ak6-ak7-schalter.txt` | **erfüllt.** `--kennt-keiner` → Rückgabe 1 mit Meldung; argumentlos → `org.denkzettel.Daemon` meldet sich an, Prozess läuft weiter | **M6** (`parse()` statt `process()`) → rot. **M8** (`::exit(2)` nach `process()`) → rot |
| **7** `--desktopfile` wird nicht ausgewertet | `commandlinetest::refusesTheSwitchThatWouldRenameTheApplication` | **erfüllt, schärfer als verlangt:** Rückgabe 1, „Unbekannte Option 'desktopfile'" | **M7** (`setupCommandLine()` statt der zwei Optionen) → rot |
| **8** SPEC 15 trägt die Versionsregeln, 2.3 und 2.4 die entdeckte Bedingung | `git show e79977b -- SPEC.md` | **erfüllt.** Neuer Abschnitt **15.1** am Ende von 15 (nicht in der KF6-Aufzählung — die Naht zum Nachbarstrang bleibt offen); 2.3 bekommt zwei Bedingungen (Busname an der Domäne, Auswertung vor der Weiche), 2.4 zwei (Komponentenname am Desktop-Dateinamen, Anwendungs-Id nicht auf der Kommandozeile) | — |

## 3. Die Mutationsproben

`mutationsproben.sh` hält je Probe den Eingriff im Wortlaut und arbeitet auf
einer Kopie unter `/tmp`. **Neun Proben, neun rot; eine Gegenprobe, grün wie
erwartet** (`messungen/mutationsproben.txt`).

**M10 ist der eigentliche Befund.** Derselbe Eingriff wie M1 — die
Organisationsdomäne fällt weg, der Dienst meldet sich als `org.kde.Daemon` an
und die globalen Kürzel brechen mit —, gemessen gegen den Prüfsatz **vor**
dieser Story: `100% tests passed out of 7`. Der teuerste Bruch dieser Story war
von keinem bestehenden Prüfsatz gedeckt. Das war die Behauptung des
Vorprüfberichts; jetzt ist sie gemessen.

**Und die Proben haben schon beim Entwerfen etwas gefunden.** Die ersten
Fassungen von M6 und M7 liefen gegen einen unerreichbaren Sitzungsbus — und
dort beendet `KDBusService` den Prozess ohnehin mit 1. Ein **angenommener**
Schalter hätte damit dieselbe Rückgabe hinterlassen wie ein zurückgewiesener,
und beide Prüfsätze wären grün geblieben, was immer der Code tut. Der Prüfsatz
läuft seither über den Sitzungsbus des Tests, wo die beiden Ausgänge nicht zu
verwechseln sind: zurückgewiesen endet der Prozess von selbst, angenommen wird
er zum Dienst und endet gar nicht. Siehe Abschnitt 6.

## 4. Befehlsliste für den PO — AK 4 und AK 5 am installierten Stand

Was hier fehlt, kann ein Agent nicht schließen: Ob die Kürzel nach der
Umstellung noch auslösen, hängt an `kglobalacceld` und an der installierten
Desktop-Datei. **Alle Befehlsformen unten sind am heute laufenden installierten
Dienst nachgeprüft** — sie liefern also im Gutfall genau die gezeigte Antwort.
Zwei Hinweise vorweg: `qdbus` liegt auf dieser Maschine als **`qdbus6`**, und
`busctl`/`gdbus` kommen mit `dbus` bzw. `glib2` — deshalb stehen unten die
letzten beiden.

```bash
# 1. Installieren (Kundenpasswort, grafischer Dialog) — Bauplatz des Sprint-Endstandes
pkexec /usr/bin/cmake --install /home/hnsstrk/Projekte/denkzettel/build

# 2. Installieren heißt nicht laufen (B16): beenden, neu starten, Pfad prüfen
pkill -x denkzetteld
setsid /usr/bin/denkzetteld >/dev/null 2>&1 &
sleep 2
readlink /proc/$(pgrep -x denkzetteld)/exe        # erwartet: /usr/bin/denkzetteld, ohne "(deleted)"

# 3. AK 4, erste Hälfte — der angemeldete Busname (SPEC 2.3)
busctl --user list | grep denkzettel
# erwartet: eine Zeile "org.denkzettel.Daemon" — nicht org.kde.Daemon
busctl --user introspect org.denkzettel.Daemon /Daemon | head -6
# erwartet: .AddNote .Quit .ShowCapture .ShowLibrary

# 4. AK 4, zweite Hälfte — die Anwendungs-Id (SPEC 2.4)
gdbus call --session --dest org.kde.kglobalaccel --object-path /kglobalaccel \
  --method org.kde.KGlobalAccel.allMainComponents | tr ',' '\n' | grep -i denkzettel
# erwartet: 'org.denkzettel.Denkzettel.desktop' — nicht org.kde.denkzettel.desktop

# 5. AK 5 — die globalen Kürzel
gdbus call --session --dest org.kde.kglobalaccel \
  --object-path /component/org_denkzettel_Denkzettel_desktop \
  --method org.kde.kglobalaccel.Component.allShortcutInfos
# erwartet: ein Eintrag ('show-capture', 'Notiz erfassen', …, [268435534], [268435534])
# 268435534 ist Meta+N. Danach die Taste tatsächlich drücken: das
# Erfassungsfenster muss erscheinen — die Zahl belegt den Eintrag, nicht die Wirkung.

# 6. Stumme Fehlermeldungen fremder Dienste
journalctl --user -t denkzetteld -n 20

# 7. Die Versionsanzeige am installierten Stand
/usr/bin/denkzetteld --version                     # erwartet: denkzettel <Nummer>, Rückgabe 0
```

**Punkt 5 ist der einzige, der einen Menschen braucht.** Ein Agent kann sich
unter Wayland den Fokus nicht holen und einen Tastendruck nicht auslösen.

## 5. Grenzen der Prüfbarkeit — benannt und, wo möglich, geschlossen

**Geschlossen:** Der Vorprüfbericht führte „dass die Busnamen nach der
Umstellung halten, ist **nur** am laufenden, installierten Stand nachweisbar"
als Grenze. Das gilt für den installierten Stand weiterhin, aber der
**automatische** Teil ist jetzt gedeckt: `commandlinetest` startet den gebauten
Dienst in einer eigenen Sitzung und liest den Namen zurück, den der Bus führt —
nicht den, den eine Kopfdatei vermuten lässt. M1 zeigt, dass die Zusicherung
greift. Das schließt die Grenze nicht für `/usr`, aber es nimmt ihr den Teil,
der ohne Menschen prüfbar ist.

**Offen und gemeldet:** AK 4 und AK 5 am installierten Stand — Abschnitt 4. Das
ist kein Versäumnis dieses Strangs, sondern DoD 2 und der Takt des PO: Es gibt
nur ein `/usr`, und zwei gleichzeitig arbeitende Stränge prüften sonst den Stand
des jeweils anderen.

**Nicht gemessen:** Dass bei laufendem Dienst nach der Umstellung *kein*
Erfassungsfenster mehr aufgeht. Gemessen ist, dass `--version` gegen den echten
Sitzungsbus die Zeile schreibt und mit 0 endet, ohne dass der laufende Dienst
etwas tut — der Weg über den Bus wird gar nicht erst betreten (M4 zeigt es von
der anderen Seite). Das ausbleibende Fenster selbst hat niemand gesehen; ich
kann es offscreen nicht sehen, und im Bild der Sitzung wäre die Abwesenheit
eines Fensters kein Beleg.

**Ohne Bildbeleg, mit Begründung:** Diese Story ändert nichts Sichtbares. Kein
Fenster, kein Zustand, keine Zeichnung — DoD 3 hat hier keinen Gegenstand. Das
systemweit registrierte Verhalten (DoD 2) deckt Abschnitt 4 ab.

## 6. Ein Fall für die Liste „Rückgabewerte und Läufe, die nichts belegen"

Vorschlag zur Aufnahme in `.claude/agents/denkzettel-dev.md` — **ich habe die
Agentendatei nicht angefasst**, das ist die Fläche des PO:

> **Ein Prüfsatz gegen einen unerreichbaren Sitzungsbus kann Zurückweisung nicht
> von Absturz unterscheiden.** `KDBusService` beendet den Prozess mit **1**,
> wenn es keinen Bus erreicht — und zwar **stumm**. Wer damit prüfen will, ob
> ein unbekannter Schalter zurückgewiesen wird, misst „Rückgabe ≠ 0" und bekommt
> dieselbe Zahl, wenn der Schalter **angenommen** wurde und der Dienst danach am
> fehlenden Bus starb. Beide Prüfsätze waren in ihrer ersten Fassung grün, was
> immer der Code tat (gefunden beim Entwerfen der Mutationsproben zu #61,
> 05.08.2026; die Fassung danach steht über `expectRefusal()` in
> `tests/commandlinetest.cpp`). **Die Regel dahinter:** Wenn zwei verschiedene
> Ausgänge dieselbe Zahl hinterlassen, hat der Prüfsatz kein Kriterium. Gib ihm
> eine Bedingung, unter der die beiden auseinandergehen — hier ein **erreichbarer**
> Bus, unter dem der angenommene Schalter den Prozess weiterlaufen lässt.

## 7. Was nicht Gegenstand war

`CHANGELOG.md` · das Erhöhen auf 0.2.0 und der Tag `v0.2.0` (Fälligkeit **nach**
der Abnahme, Sprint-Abschluss Takt 2) · das PKGBUILD (existiert nicht) · der
Über-Dialog (**#87**) · `.github/workflows/ci.yml` — **nicht nötig**: `qt6-base`
zieht `dbus` mit, der Container hat `dbus-run-session` also ohne neue Paketzeile.

**Hinweis an den PO, außerhalb meiner Fläche:** Mit dieser Story entfällt der
Grund für die Aussetzung von **Punkt 10** im Sprint-Abschluss
(`docs/scrum/PROZESS.md`). Das Aufheben ist Scrum-Master-Fläche; ich melde es
nur.

## 8. Baulauf und Prüfsatz

- **Bau:** `cmake --build build -j 12` → Rückgabe 0, **0 Compiler-Warnungen**.
- **`ctest`:** `100% tests passed out of 9`, 6,6 s (`messungen/ctest.txt`).
  Neu darin: `identitytest` (0,02 s) und `commandlinetest` (0,15 s).
- **clazy:** 3 Befunde, **alle drei aus dem Altbestand** vom 04.08.2026
  (2× `range-loop-detach` in `librarytest.cpp`, 1× `detaching-temporary` in
  `shelltest.cpp`). Die Schwelle des automatischen Laufs steht auf 3; die neuen
  Dateien tragen nichts bei.
- **clang-tidy** über die drei neuen Dateien: nur `misc-const-correctness`.
  Die drei `QTemporaryDir` sind daraufhin `const` (`3905438`); der Hinweis auf
  `QApplication app` im Prüfsatz-`main()` bleibt stehen — jede andere
  Prüfsatzdatei des Projekts schreibt dieselbe Zeile.

**Eine Beobachtung zu `commandlinetest`, die keine Fußnote verdient:** In der
ersten Fassung lief er 138 ms und 60 s zugleich. Der Dienst schickt beim ersten
Start eine Benachrichtigung, die private Sitzung startete daraufhin den
Benachrichtigungsdienst des Schreibtischs, und der überlebte den Bus und hielt
die Ausgabeleitung des Laufs offen. `XDG_DATA_DIRS` zeigt für diesen Test
deshalb auf ein Verzeichnis ohne Dienstdateien — gesetzt in
`tests/CMakeLists.txt` und nicht im Prüfsatz, weil der Bus seine Liste einmal
beim Start liest und der Prüfsatz sein Kind ist.
