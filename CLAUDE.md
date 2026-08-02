# Denkzettel — Arbeitsanweisung für Claude Code

Quick-Capture-Werkzeug für KDE Plasma (Wayland), C++/Qt6/KF6, CMake, QTest.
Dieses Projekt arbeitet mit einem **Agenten-Scrum-Team**. Wer hier arbeitet,
arbeitet nach dessen Regeln — sie stehen vollständig in
**`docs/scrum/PROZESS.md`** und sind vor jeder Arbeit am Produkt zu lesen.

## Die Regeln, die am häufigsten übergangen werden

Sie stehen hier, weil sie in diesem Projekt bereits verletzt wurden — jede
einzelne hat einen Mangel oder Kundenbefund im Rücken.

**Geprüft wird am installierten Stand** (`-DCMAKE_INSTALL_PREFIX=/usr`), nicht
im Build-Verzeichnis (DoD 2). *Bei mehreren gleichzeitig arbeitenden Agenten
gilt zusätzlich:* Es gibt nur ein `/usr`. Installieren zwei gleichzeitig, prüft
einer den Stand des anderen — deshalb taktet der PO die Installation, und **am
Sprint-Ende wird der Endstand einmal installiert und geprüft**. Ohne diesen
Schritt ist die Sprint-Abnahme unvollständig (Sprint-3-Mangel M1).

**Ein UI-Review ohne eigenes Bild ist nicht geführt** (DoD 3, Beschluss B3).
Tests ersetzen die Bildprüfung nicht, und Bilder ersetzen die Tests nicht:

> Bei Bewegungen ist der Weg der Prüfgegenstand, nicht das Ziel.
> Bei Zuständen ist das Bild der Prüfgegenstand, nicht die Zusicherung.

Für Bildläufe **muss `QT_QPA_PLATFORMTHEME=kde` gesetzt sein** — sonst
verfälscht eine Ersatzschrift die Größenverhältnisse.

**Ein unversionierter Beleg ist kein Beleg** (B7). Prüfberichte und Bilder
gehören nach `docs/scrum/reviews/`, nicht ins Scratchpad und nicht in einen
Arbeitsbaum, der mit dem Agenten verschwindet.

**Melden, nicht heilen.** Wer außerhalb seiner Fläche einen Fehler findet,
meldet ihn dem PO. Das gilt auch, wenn die Heilung eine Zeile wäre.

**Entdeckte Bedingungen ziehen die SPEC nach** (DoD 4 in der Fassung nach B9) —
nicht nur geänderte Festlegungen. Wenn beim Bauen herauskommt, dass etwas nur
unter einer Bedingung gilt, gehört die Bedingung in die SPEC.

**Ein Sprint endet nicht mit dem letzten Commit.** Der Abschluss steht als
Liste in `docs/scrum/PROZESS.md`, Abschnitt „Sprint-Abschluss", und wird
Punkt für Punkt im Sprint-Protokoll abgehakt: vor der Kundenabnahme
installieren, Belege ablegen, prüfen — nach ihr Issues schließen, Journal,
Push, Zweige und Worktrees räumen. Acht der neun Mängel aus Sprint 3 waren
Abschlussmängel.

**Jeder Zugang nach der Sprint-Freigabe wird gebucht** — Issues *und* Punkte.
Berührt er eine der beiden Grenzen (2–4 Stories, ~13 SP), legt der PO ihn dem
Kunden als Grenzüberschreitung vor. In Sprint 3 wurde bei jedem Zugang die
Punktzahl mitgezählt, die Zahl der Issues nicht — die Grenze fiel niemandem
auf.

**Flüchtige Belege sofort sichern.** Kundenbilder liegen in temporären
Ordnern; von acht Bildern der Sprint-3-Abnahme überlebten sieben Minuten nur
eines.

## Prüfhaltung

Dieses Projekt hat an einem einzigen Abend **vier grüne Tests entlarvt, die
nichts prüften**, und drei plausibel begründete Fehlentscheidungen kassiert.
Alle sieben fielen durch Messung, keine durch Nachdenken. Daraus:

- **Prüfe am Einzelfall, nicht an der Plausibilität.** Eine Begründung, die
  trägt und trotzdem den falschen Schluss stützt, fällt nicht auf.
- **Eine Vereinfachung ist erst geprüft, wenn sie gegen die zuletzt geheilten
  Fälle gehalten wurde.**
- **Prüfe die Voraussetzung deiner eigenen Begründung.** Wer sich auf eine
  Zeichnung beruft, liest sie vorher.
- Ein Testaufbau, in dem der Fehler gar nicht auftreten *kann*, ist kein Test.

## Retrospektiven

**Ergebnisse sind Änderungen, keine Absichtserklärungen** — Skills, Regeln,
Agentendateien, Memory-Einträge oder Anpassungen an `PROZESS.md`. Eine
Erkenntnis, die nur im Sprint-Protokoll steht, ist nicht verankert; sie wirkt
erst, wenn sie an dem Ort steht, den die nächste Sitzung von selbst liest.
**Das ist die häufigste Lücke: die Regel existiert, aber niemand sieht sie.**

Prüfe deshalb am Ende jeder Retro ausdrücklich: Ist jeder Beschluss in einem
Artefakt gelandet — und wird dieses Artefakt automatisch geladen?

## Rollen

| Rolle | Wer |
|---|---|
| Kunde | hnsstrk — Ziele, Prioritäten, Freigaben, Abnahme |
| Product Owner | Claude (Haupt-Session) — Backlog, Story-Schnitt, AK, Kundenkontakt |
| Scrum Master | Agent `scrum-master` — DoD, Schätz-Moderation, Retro, Impediments |
| Entwickler | Agent `denkzettel-dev` |
| UI/UX | Agent `denkzettel-ux` |

**Der PO schreibt keinen Produktivcode**, der Scrum Master ändert weder Code
noch SPEC, und Agenten arbeiten nur in ihrer zugewiesenen Dateimenge.

## Technisches

- Bauen: `cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug && cmake --build build`
- Testen: `ctest --test-dir build`
- Linter: `cmake --build build --target lint-tidy` bzw. `lint-clazy`
- Installieren (braucht das Kundenpasswort, grafischer Dialog):
  `pkexec /usr/bin/cmake --install /home/hnsstrk/Projekte/denkzettel/build`
- Das Repository ist **öffentlich**. Was in Issues und Commits steht, ist
  veröffentlicht.
