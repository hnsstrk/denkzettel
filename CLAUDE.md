# Denkzettel — Arbeitsanweisung für Claude Code

Quick-Capture-Werkzeug für KDE Plasma (Wayland), C++/Qt6/KF6, CMake, QTest.

Bindend ist `SPEC.md`. Der Backlog sind die GitHub Issues mit ihren
Akzeptanzkriterien — sie sagen, wann eine Story fertig ist.

## Bauen, prüfen, installieren

```
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug && cmake --build build
ctest --test-dir build
cmake --build build --target lint-tidy      # bzw. lint-clazy
```

Installieren braucht das Kundenpasswort über einen grafischen Dialog:
`pkexec /usr/bin/cmake --install <Projektpfad>/build`

## Die vier Regeln, die Fehler gefunden haben

Alles andere ist Ermessen. Diese vier nicht — jede hat in diesem Projekt
mindestens einen Fehler aufgedeckt, den sonst der Kunde gefunden hätte.

**1. Geprüft wird am installierten Stand, und Installieren heißt nicht
laufen.** Nach `cmake --install` hält ein laufender Dienst die gelöschte alte
Datei weiter; umgekehrt reicht `KDBusService::Unique` den Start eines
Debug-Builds an den laufenden Dienst weiter. Beide Male prüft man unbemerkt den
falschen Stand. Also: Dienst beenden, neu starten, dann
`readlink /proc/$(pgrep -x denkzetteld)/exe` — ohne `(deleted)`. Wer den
Debug-Stand prüfen will, beendet vorher den installierten Dienst.
`readlink` sagt *welche Datei*, erst eine Prüfsumme sagt *welcher Stand*.

**2. Ein UI-Review ohne eigenes Bild ist nicht geführt.** Tests ersetzen die
Bildprüfung nicht, und Bilder ersetzen die Tests nicht:

> Bei Bewegungen ist der Weg der Prüfgegenstand, nicht das Ziel.
> Bei Zuständen ist das Bild der Prüfgegenstand, nicht die Zusicherung.

Für Bildläufe muss `QT_QPA_PLATFORMTHEME=kde` gesetzt sein, sonst verfälscht
eine Ersatzschrift die Größenverhältnisse. Ein Bild, das als Beleg dient,
läuft mit `QT_SCALE_FACTOR` auf der Skalierung des Kunden.

**3. Ein offscreen erzeugtes Bild zeigt nicht, was der Kunde sieht.** Es belegt
Geometrie, Textsatz und Farbrollen — nicht Hülle, Rundung, Kontur, Schatten
oder Dekoration. Die zeichnen Theme und Compositor, und offscreen fehlt beiden
die Grundlage. Wo ein Akzeptanzkriterium über Theme oder Compositor etwas
behauptet, gehört ein Bild aus der angemeldeten Sitzung dazu.

**4. Ein Bildbeleg ist erst ein Beleg, wenn sein Läufer frisch gebaut ist.**
Es gibt fünf: `editshots`, `libraryshots`, `searchshots`, `readmeshots`,
`captureshots`. Ein veralteter Läufer schreibt plausible Bilder eines *alten*
Standes mit frischem Zeitstempel. Vor jedem Bildbeleg:
`cmake --build build --target <läufer>`.

## Prüfhaltung

- **Prüfe am Einzelfall, nicht an der Plausibilität.** Eine Begründung, die
  trägt und trotzdem den falschen Schluss stützt, fällt nicht auf.
- **Frag vor jedem Griff, dessen Ergebnis in einen Bericht eingeht, was er
  ausgäbe, wenn sein Gegenstand fehlte.** Ist die Antwort dieselbe Ausgabe,
  trägt der Griff nichts.
- Ein Testaufbau, in dem der Fehler gar nicht auftreten *kann*, ist kein Test.
- **Kein Agent kann sich unter Wayland den Fokus zurückholen.**
  `activateWindow()` tut es nicht, ein Alt-Tab lässt sich nicht auslösen. Wer
  eine Prüfung mit Fensterwechsel baut, schließt das obenauf liegende Fenster —
  dann gibt der Compositor den Fokus von selbst zurück.
- **Melden, nicht heilen.** Wer außerhalb seiner Fläche einen Fehler findet,
  meldet ihn dem PO. Das gilt auch, wenn die Heilung eine Zeile wäre.

## Rollen

| Rolle | Wer |
|---|---|
| Kunde | hnsstrk — Ziele, Prioritäten, Freigaben, Abnahme |
| Product Owner | Claude (Haupt-Session) — Backlog, Story-Schnitt, AK, Kundenkontakt |
| Entwickler | Agent `denkzettel-dev` |
| UI/UX | Agent `denkzettel-ux` |

Der PO schreibt keinen Produktivcode. Agenten arbeiten nur in ihrer
zugewiesenen Dateimenge.

## Veröffentlichung

Das Repository ist **öffentlich**. Was in Issues und Commits steht, ist
veröffentlicht. Zugelassen sind Kundenzitate und Messwerte; nicht zugelassen
sind Systemdetails (Rechnernamen, Kernel-Versionen, Pfade außerhalb des
Projekts, Interna des Heimnetzes) und personenbezogene Angaben.

Gepusht wird nach jedem abgeschlossenen Arbeitsblock, ohne Rückfrage. Jeder
Push auf `main` löst einen öffentlichen Bau- und Testlauf aus
(`.github/workflows/ci.yml`); er schlägt bei jeder Compiler-Warnung, jedem
roten Test und jedem Linterbefund fehl. Wer pusht, sieht nach — **am Lauf des
eigenen Commits**, nicht am obersten der Liste:

```
gh run list --commit $(git rev-parse HEAD) --json status,conclusion \
    --jq '.[]|[.status,.conclusion]|@tsv'
```

Erst `completed` **und** `success` ist ein Nachschlag. Umgekehrt ist die grüne
Marke kein Ersatz für die Prüfung am installierten Stand — der Lauf erreicht
sie nicht.

Code und Kommentare englisch, UI-Zeichenketten deutsch über `i18n()`.
