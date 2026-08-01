# karpathy-Review Sprint 3 — Produktivcode-Diff `59d0d3f..HEAD`

**Datum:** 01.08.2026 · **Anlass:** DoD 3 am Sprint-Ende · **Gesamt-Verdikt:** `warn`, kein `fail`
**Umfang:** 24 Dateien, rund 2.970 Zeilen. Der Reviewer hat den Diff vollständig gelesen, das Projekt selbst gebaut und die Suite ausgeführt: 7/7 Tests grün.

Abgelegt vom PO als Beleg nach Beschluss B7 (Mangel M4 der DoD-Prüfung). Der Bericht ist die Fassung des Reviewers, gekürzt um Wiederholungen.

## Verdikt je Prinzip

| Prinzip | Verdikt |
|---|---|
| 1 — Think Before Acting | `warn` |
| 2 — Simplicity First | **`ok`** |
| 3 — Surgical Changes | `warn` |
| 4 — Goal-Driven Execution | `warn` |

## Die vier Befunde

**1 · Überholter Kommentar (Prinzip 1).** `tests/storetest.cpp:342` begründete die ß-Grenze mit `remove_diacritics 2`; Code und SPEC 6 sagen `1`. Die Aussage über ß blieb richtig, ihre Begründung zitierte eine Konfiguration, die es nicht mehr gibt. **Behoben** mit `7787339`.

**2 · Fehlende Warnung in der zweiten Bildstrecke (Prinzip 1).** `libraryshots.cpp` warnt vor der verfälschenden Ersatzschrift ohne `QT_QPA_PLATFORMTHEME=kde`; `searchshots.cpp` erzeugt Bilder desselben Fensters und erwähnte sie nicht. **Behoben** mit `7787339` — dabei fand der Entwickler, dass die angeglichenen Programme die echte Konfiguration des Nutzers lasen, und setzte `XDG_CONFIG_HOME` auf ein leeres Verzeichnis.

**3 · Doppelte Infrastruktur zwischen den Strängen (Prinzip 3).** Beide Bildstrecken bauen eigene, fast gleiche Hilfsfunktionen. Bei zwei Dateien kein Zusammenlegungszwang; die Inkonsistenzen sind angeglichen. **Bei einer dritten Bildstrecke gehört die Infrastruktur geteilt.**

**4 · Tautologische Zusicherung (Prinzip 4).** `tests/librarytest.cpp:1897` verglich `textLeft(headRect)` mit `textLeft(entryRect)` — beide addieren dieselbe Konstante auf dasselbe `x`. Der Test konnte nicht fehlschlagen, sein Name versprach eine Ausrichtungsprüfung, die nicht stattfand. **Behoben** mit `7787339`: Zeile gestrichen, Test umbenannt in `keepsTheMeasuresOfGroupedList`, und an der Stelle der toten Zusicherung steht jetzt, warum die Ausrichtung nicht prüfbar ist und was sie stattdessen hält.

**5 · Regroup bei jeder Fensteraktivierung (Prinzip 4, nicht behoben).** `librarywindow.cpp:311–322` gruppiert bei **jeder** Aktivierung neu, nicht nur nach Tageswechsel — wer weggescrollt hat und zurückkommt, verliert die Stelle. Vom Reviewer logisch hergeleitet, ausdrücklich nicht empirisch verifiziert. **Als #59 im Backlog**, neben #57 zu entscheiden.

## Urteil zur Kernfrage: trägt die Zeilensemantik ihre Komplexität?

**Ja.** Der Kopf als eigene, nicht selektierbare Modellzeile ist der übliche Qt-Weg; der Scroll-Mechanismus und das Mitnehmen des Kopfes beim Löschen brauchen ihn als eigene Zeile. Die Übersetzung zwischen Zeilen- und Notizindex ist auf zwei kleine Funktionen begrenzt, und `buildRows` baut bei jeder Änderung vollständig neu statt inkrementell — einfach und korrekt. **Einen einfacheren Schnitt ohne Verlust sieht der Reviewer nicht.**

## Was er ausdrücklich lobt

- Der Migrationstest friert das V1-Schema als Literal ein, statt Produktivcode wiederzuverwenden — und prüft Rebuild für Altbestand, Weiterlaufen und Idempotenz beim zweiten Öffnen.
- Die FTS5-Trigger-Falle (altes Wort bleibt auffindbar, kein Fehler, kein `integrity-check` schlägt an) ist an **drei** Orten konsistent dokumentiert und durch einen Test gehalten, der genau sie fangen würde.
- Entscheidungen tragen ihre Herkunft im Code: Tests benennen, wo ein Trade beschlossen wurde — wer ihn ändern will, findet die Begründung am Prüfpunkt.
- `searchTerms()` macht FTS5-Syntax konstruktiv unerreichbar, statt zu escapen — die einfachste robuste Lösung.
- **Auffällig viele Tests prüfen ihre eigenen Vorbedingungen** (`QVERIFY2(…, "der Fall verlangt …")`). Das ist das Gegenteil des grünen Tests, der nichts prüft — und angesichts der vier an diesem Abend entlarvten wertlosen Tests die wirksamste Gegenmaßnahme, die der Sprint hervorgebracht hat.

## Suche nach weiteren wertlosen Tests

Der Reviewer hat gezielt danach gesucht und **keine weiteren gefunden**. Er hat außerdem geprüft, dass `capturetest::textsFollowAColourSchemeChange` mit dem alten Code nachweislich rot würde, und dass der Randfall der Suche während einer laufenden Löschfrist über `PendingDeletion::committed` abgedeckt ist.
