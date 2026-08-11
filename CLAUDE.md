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

## Was geprüft wird — und was nicht

**Ein Test bleibt nur, wo das Auge nicht hinkommt** (Kundenentscheidung
11.08.2026). Denkzettel ist ein kleines Werkzeug, keine Raketensteuerung, und
der Kunde sieht sich das Ergebnis selbst an — Farben, Abstände, Linien,
Schriftgrößen und ob ein Fenster aufgeht, prüft er besser und schneller als ein
Pixelvergleich.

Ein Prüfsatz ist deshalb nur gerechtfertigt für das, was **still kaputtgeht**:
Schema-Umstellungen und Datenverlust, der Suchindex, Fehlerpfade,
Zeichenkodierung, Rückgabewerte fremder Dienste, Unterschiede zwischen
Bautypen. Alles, was man ansehen kann, wird angesehen.

Vor jedem neuen Prüfsatz die Frage: *Würde der Kunde diesen Fehler beim
Benutzen bemerken?* Wenn ja, ist das Bild der Nachweis und kein Test.

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

- **Frag vor jedem Griff, dessen Ergebnis in einen Bericht eingeht, was er
  ausgäbe, wenn sein Gegenstand fehlte.** Ist die Antwort dieselbe Ausgabe,
  trägt der Griff nichts. Ein Testaufbau, in dem der Fehler gar nicht auftreten
  *kann*, ist kein Test.
- **Kein Prozess holt sich unter Wayland den Fokus zurück.** Wer eine Prüfung
  mit Fensterwechsel baut, schließt das obenauf liegende Fenster — dann gibt
  der Compositor den Fokus von selbst zurück.
- **Melden, nicht heilen.** Wer außerhalb seiner Fläche einen Fehler findet,
  meldet ihn dem PO, auch wenn die Heilung eine Zeile wäre.

## Rollen

| Rolle | Wer | Modell |
|---|---|---|
| Kunde | hnsstrk — Ziele, Prioritäten, Freigaben, Abnahme | — |
| Product Owner | Claude (Haupt-Session) — Backlog, Schnitt, AK, Prüfung | Opus 5 |
| Entwickler | Agent `denkzettel-dev` | Opus 5, bei Neubau eines Subsystems Fable 5 |
| UI/UX | Agent `denkzettel-ux` | Opus 5 |
| Gegenlesen | Agent `karpathy-reviewer` (global) | Fable 5 |

Der Reviewer läuft in frischem Kontext über abgeschlossene Arbeit und ist die
Qualitätssicherung dieses Projekts — nicht eine eigene Prüfmechanik. Bei
parallelen Strängen laufen mehrere Instanzen desselben Agenten, jede in ihrem
Worktree; das Modell überschreibt der PO beim Aufruf.

Der PO schreibt keinen Produktivcode. Agenten arbeiten nur in ihrer
zugewiesenen Dateimenge.

## So läuft ein Sprint

1. **Ziehen.** Der PO schlägt vor, der Kunde entscheidet. Zwei bis vier Issues.
2. **Bauen.** Der Dev-Agent setzt die Akzeptanzkriterien um. Bei UI-Arbeit
   macht die UX die Bilder. Parallele Stränge je in eigenem Worktree auf einem
   Zweig `story/NN-…`; gemerged wird vom PO.
3. **Prüfen.** Der PO gegen die Akzeptanzkriterien, der Kunde am laufenden
   Programm. Was auffällt und nicht zur Story gehört, wird ein Issue oder ist
   keins.
4. **Abschließen.** Changelog-Zeile, Version in `CMakeLists.txt`, Tag
   `vX.Y.Z`, Issues und Milestone schließen, Zweige und Worktrees räumen.

Wer eine Lehre zieht, schreibt sie hierher. Ein Protokoll liest die nächste
Sitzung nicht.

**Die Installation nach `/usr` taktet der Kunde** — sie braucht sein Passwort.
Agenten installieren nicht: Es gibt nur ein `/usr`, und installieren zwei
Stränge gleichzeitig, prüft einer den Stand des anderen.

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
