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

Installieren braucht das Passwort des Nutzers über einen grafischen Dialog:
`pkexec /usr/bin/cmake --install <Projektpfad>/build`

## Was geprüft wird — und was nicht

**Ein Test bleibt nur, wo das Auge nicht hinkommt** (Entscheidung des Nutzers,
11.08.2026). Denkzettel ist ein kleines Werkzeug, keine Raketensteuerung, und
der Nutzer sieht sich das Ergebnis selbst an — Farben, Abstände, Linien,
Schriftgrößen und ob ein Fenster aufgeht, prüft er besser und schneller als ein
Pixelvergleich.

Ein Prüfsatz ist deshalb nur gerechtfertigt für das, was **still kaputtgeht**:
Schema-Umstellungen und Datenverlust, der Suchindex, Fehlerpfade,
Zeichenkodierung, Rückgabewerte fremder Dienste, Unterschiede zwischen
Bautypen. Alles, was man ansehen kann, wird angesehen.

Vor jedem neuen Prüfsatz die Frage: *Würde der Nutzer diesen Fehler beim
Benutzen bemerken?* Wenn ja, ist das Bild der Nachweis und kein Test.

## Die vier Regeln, die Fehler gefunden haben

Alles andere ist Ermessen. Diese vier nicht — jede hat in diesem Projekt
mindestens einen Fehler aufgedeckt, den sonst der Nutzer gefunden hätte.

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
läuft mit `QT_SCALE_FACTOR` auf der Skalierung des Nutzers.

**3. Ein offscreen erzeugtes Bild zeigt nicht, was der Nutzer sieht.** Es belegt
Geometrie, Textsatz und Farbrollen — nicht Hülle, Rundung, Kontur, Schatten
oder Dekoration. Die zeichnen Theme und Compositor, und offscreen fehlt beiden
die Grundlage. Wo ein Akzeptanzkriterium über Theme oder Compositor etwas
behauptet, gehört ein Bild aus der angemeldeten Sitzung dazu.

**4. Ein Bildbeleg ist erst ein Beleg, wenn sein Läufer frisch gebaut ist.**
Ein veralteter Läufer schreibt plausible Bilder eines *alten* Standes mit
frischem Zeitstempel. Vor jedem Bildbeleg:
`cmake --build build --target readmeshots`. Wo ein Bild aus einer Story einen
Befund tragen soll, entsteht es aus der angemeldeten Sitzung — einen Läufer
dafür gibt es nicht mehr.

## Prüfhaltung

- **Frag vor jedem Griff, dessen Ergebnis in einen Bericht eingeht, was er
  ausgäbe, wenn sein Gegenstand fehlte.** Ist die Antwort dieselbe Ausgabe,
  trägt der Griff nichts. Ein Testaufbau, in dem der Fehler gar nicht auftreten
  *kann*, ist kein Test.
- **Kein Prozess holt sich unter Wayland den Fokus zurück.** Wer eine Prüfung
  mit Fensterwechsel baut, schließt das obenauf liegende Fenster — dann gibt
  der Compositor den Fokus von selbst zurück.

## Läufe, die nichts belegen

Gemessene Fälle, in denen etwas nach Beleg aussah und keiner war. Prüfe dagegen,
bevor du einen Nachweis meldest, und **ergänze die Liste** um jeden neuen Fund.

1. **`KGlobalAccel::setGlobalShortcut()` liefert `true`, auch wenn der Daemon
   nicht erreichbar ist.** Beleg führt nur das Zurücklesen beim Dienst.
2. **`KWindowShadow::create()` meldet `true`, auch bei achtmal demselben Bild
   statt acht Kacheln.** Offscreen ist es **immer** `false` — dort belegt weder
   `true` noch `false` etwas.
3. **`activateWindow()` holt unter Wayland den Fokus nicht zurück.** Kein
   Prozess kann sich den Fokus selbst zuteilen. Ein Alt-Tab lässt sich nicht
   auslösen; der Weg über das obenauf liegende Fenster steht oben.
4. **Zwei lebende `KSvg::ImageSet` desselben Themes teilen ihre Auswahlpfade.**
   Jeder Vergleich zweier Fassungen derselben Grafik läuft sonst gegen sich
   selbst — und ist grün. Für die zweite Fassung ein **anderes** Theme nehmen.
5. **Ein Prüfsatz, der sich sein Theme nicht aussucht, prüft womöglich an einem,
   das den Unterschied nicht kennt.** Wähle den Prüfgegenstand danach, dass die
   Wahl überhaupt etwas ändert.
6. **Bei gesperrter Sitzung liefert `spectacle -f` ein schwarzes Bild mit
   Rückgabe 0.** Sperrzustand vorher abfragen und abbrechen, statt zu messen.
7. **Ein Vollbildfenster als Prüfgrund verdeckt, was du messen willst** — der
   Compositor legt es über das Erfassungsfenster.
8. **Ein Sandkasten ohne `kdeglobals` färbt die Theme-Grafik anders als die
   Qt-Palette** — das Bild sieht dann nach einem Fehler des Erzeugnisses aus.
9. **`show()` statt `showCapture()` liefert ein Fenster ohne Schatten** — der
   wird erst in `present()` gebunden.
10. **Ein Vergleich kann auf beiden Seiten falsch sein und „stimmt" melden.**
    Vergleichst du zwei Größen, die derselbe Fehler gemeinsam verschiebt, misst
    du nichts. Halte mindestens eine Seite gegen einen **von außen gesetzten**
    Wert.

**Der gemeinsame Nenner** ist jedes Mal die erste Regel der Prüfhaltung: Der
Griff hätte dieselbe Ausgabe geliefert, wenn sein Gegenstand gefehlt hätte.

## Vor der Übergabe

Den gebauten Stand starten und den Hauptweg der Story einmal selbst gehen. Für
Bilder `QT_QPA_PLATFORM=offscreen`, `QT_QPA_PLATFORMTHEME=kde` und
`QT_SCALE_FACTOR` auf der Skalierung des Nutzers (**1,5**).

Nach dem Start des Daemons ins Journal sehen
(`journalctl --user -t denkzetteld -n 20`) — stumme Fehler fremder Dienste
stehen dort und nirgends sonst.

## UI-Prüfung

Maßstab sind `wireframes/Denkzettel Wireframes.dc.html` als UI-Referenz des
Projekts und die KDE Human Interface Guidelines (develop.kde.org/hig) —
Denkzettel ist eine Qt6/KF6-App für KDE Plasma. Die Prüfpunkte kommen aus dem
Wireframe, nicht aus dem Gedächtnis: Jeder gezeichnete Bereich erzeugt genau
eine Prüffrage, die Raumaufteilung eingeschlossen. Bilder, die einen Befund
tragen, liegen unter `docs/bilder/reviews/`.

## Wenn es nicht vorangeht

Gleicher Fehler zweimal ohne neue Erkenntnis: aufhören und melden. Widerspruch
zwischen Issue und SPEC oder eine Entscheidung, die nur der Nutzer treffen
kann: fragen statt raten.

## Abschluss

Changelog-Zeile, Version in `CMakeLists.txt`, Tag `vX.Y.Z`, Issues und
Milestone schließen.

Wer eine Lehre zieht, schreibt sie hierher. Ein Protokoll liest die nächste
Sitzung nicht.

**Die Installation nach `/usr` taktet der Nutzer** — sie braucht sein Passwort.

## Veröffentlichung

Das Repository ist **öffentlich**. Was in Issues und Commits steht, ist
veröffentlicht. Zugelassen sind Zitate des Nutzers und Messwerte; nicht
zugelassen sind Systemdetails (Rechnernamen, Kernel-Versionen, Pfade außerhalb
des Projekts, Interna des Heimnetzes) und personenbezogene Angaben.

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

Code und Kommentare englisch, UI-Zeichenketten deutsch über `i18n()`,
Commit-Betreffzeilen deutsch.
