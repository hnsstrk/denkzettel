# Retro-Stellungnahme Sprint 2 — UI/UX (Agent `ux-retro-s2`, 01.08.2026, eingegangen 08:01)

**Modus:** Retro-Stellungnahme (weder Review noch Gestaltung). Keine Datei im Repository geändert, nichts installiert, nichts committet. Alle Prüfungen liefen im Scratchpad; der Build war out-of-source.

**Kurzfassung vorweg:** Die Kundenfrage ist berechtigt. Mein UI-Review hat das Fenster nie gesehen — er lief rein statisch am Code. Das ist eine strukturelle Lücke meines Verfahrens. Es ist aber **auch** ein Übersehen: Die Ursache stand an genau einer Zeile im Code, sie war ohne laufendes Programm erkennbar, und der Testkopf der Datei, die ich gelesen habe, hat mich schriftlich auf die Lücke hingewiesen. Ich habe den Hinweis gelesen und nicht die Konsequenz gezogen.

---

## 1. Warum der Review das kaputte Layout nicht gefunden hat

**Meine Methode war: Code gegen Wireframe, SPEC und HIG.** Ich habe geprüft, ob die richtigen Bedienelemente existieren, richtig beschriftet, übersetzt, tastaturbedienbar und in der richtigen Reihenfolge angelegt sind. Ich habe **nicht** geprüft, wo sie landen. Ein laufendes Fenster oder einen Screenshot hatte ich zu keinem Zeitpunkt. Für die gesamte Fehlerklasse „Raumaufteilung" hatte mein Verfahren schlicht kein Prüfmittel — das ist die strukturelle Lücke, und sie erklärt, warum das Ergebnis `pass` lauten konnte, während das Fenster sichtbar kaputt war.

**Die Ursache war statisch erkennbar.** Sie sitzt in `src/ui/librarywindow.cpp:162`:

```cpp
layout->addWidget(m_splitter);   // ohne Stretch-Faktor
```

Ein horizontaler `QSplitter` hat vertikal die Größenrichtlinie **Preferred**, nicht Expanding (gemessen: `verticalPolicy() == 5`). Damit hat im äußeren `QVBoxLayout` **kein einziges** Element eine vertikale Ausdehnungsrichtung, und das Layout verteilt die überschüssige Fensterhöhe zu gleichen Teilen auf Kopfzeile und Splitter. In der so aufgeblähten Kopfzeile zentriert Qt dann das `QLineEdit` vertikal, weil ein Eingabefeld eine feste Höhe hat — das ist das „schwebende Suchfeld".

**Gemessen am echten, unveränderten Sprint-2-Stand** (Helferprogramm im Scratchpad, gegen die gebaute `denkzettelui` gelinkt, `QT_QPA_PLATFORM=offscreen`):

| | Ist (Sprint-2-Stand) | nach `setStretchFactor(splitter, 1)` |
|---|---|---|
| Kopfzeile | y=0, **h=300** | y=0, h=41 |
| Suchfeld | y=137 | y=8 |
| Liste/Detail | y=300 — **50 % der Fensterhöhe** | y=41, h=559 |

Der Screenshot dieses Laufs (`scratchpad/echt-ist.png`) zeigt exakt das Bild aus der Kundenabnahme. Die Gegenprobe habe ich zur Laufzeit am Widget-Baum gemacht, nicht an der Quelldatei — der Befund lautet trotzdem: **eine Zeile**.

**Verschärfend, und deshalb nenne ich es:** Der Kopfkommentar von `tests/librarytest.cpp:27-31` sagt wörtlich, die Tests deckten die Bausteine ab, „the window itself — layout, empty states, the look of the message widget — stays on the manual checklist". Der Code hat mir die Lücke also angezeigt. Wenn das Layout auf die manuelle Liste wandert, muss das Layout auf der manuellen Liste **stehen** — meine fünf Sichtprüfpunkte (Protokoll 7.4, Punkte 9–13: Tooltip, Orca, Größenpersistenz, Splitter-Minimum, Löschen/Undo) enthalten keinen Punkt zur Raumaufteilung. Dass ich es prinzipiell gekonnt hätte, zeigt meine eigene Auflage aus demselben Review: die Mindestbreite von 220 px, umgesetzt als Test `keepsTheListWideEnoughForThePreview` (`tests/librarytest.cpp:790`). Ich habe die waagerechte Achse abgesichert und die senkrechte nicht — das war kein Nicht-Können, sondern ein blinder Fleck ohne Systematik.

**Verdikt auf meinen eigenen Review: `fail`.** Methodisch und im Einzelfall.

Gegen den Wireframe ist der Sollzustand unstrittig: `wireframes/Denkzettel Wireframes.dc.html:331` zeichnet die Kopfzeile als schmale Leiste am oberen Rand, Zeile 337 den Inhaltsbereich mit `min-height:300px` darunter. Die KDE HIG sagen dasselbe in der Tabelle „Common layouts and responsiveness" (Layout and navigation): Toolbar **above the content area**, und Layoutelemente müssen sich der Fenstergröße fließend anpassen.

Nebenbefund: Dieselbe Fehlerklasse steckt **nicht** im Capture-Fenster — dort setzt `capturewindow.cpp:71/154` die Fensterhöhe auf den `sizeHint`, es gibt also gar keine Überschusshöhe. Die Bibliothek ist das erste Fenster mit freier Restfläche.

## 2. Was der Review geleistet hat

Kurz, zur Einordnung: Der eine `fail` (Nachladen bei offenem Fenster, plus Fokusklau beim erneuten Zeigen) war ein echter Fund, deckungsgleich mit dem karpathy-Befund A, geheilt in `54ae35d` und vom Scrum Master per Mutationstest nachgeprüft. Die Planning-Beratung hat neun Akzeptanzkriterien ergänzt, die Sidebar aus M2 entfernt und S8 wegen fehlender Zeichnung zurückgestellt — letzteres blockierte unabhängig von jeder AK-Formulierung und hat Fehlplanung verhindert.

Das relativiert Abschnitt 1 nicht. Ein Review, der Existenz und Semantik prüft, aber nie das Bild sieht, ist ein Code-Review mit UI-Vokabular.

## 3. Vorschlag für das neue UI-Review-Verfahren

Ich habe die Optionen heute durchgespielt, statt sie zu vermuten.

**(a) Screenshot-Pflicht per Offscreen-Rendering — erprobt, empfohlen.** Der gesamte Weg funktioniert auf Ganymed, ohne jede Installation: `cmake` out-of-source bauen, ein kleines Helferprogramm gegen `denkzettelui` linken, `QT_QPA_PLATFORM=offscreen`, `QWidget::grab().save()`, PNG lesen. Gesamtdauer rund fünf Minuten, davon eine Minute Build. Mit `-style breeze` rendert der Lauf die echten Breeze-Maße. Wichtig: Das Testbinary zeigt das Fenster ohnehin schon rund zwanzigmal (`QTest::qWaitForWindowExposed`) — die Infrastruktur ist da, es fehlt nur der Auslöser. Grenzen ehrlich benannt: keine Fensterdekoration, kein Farbschema des Nutzers, kein HiDPI, keine Compositor-Effekte. Für die Fehlerklasse „Raumaufteilung" reicht es vollständig, wie der Beweis oben zeigt.

**(b) Geometrie-Zusicherungen als Tests — empfohlen, ergänzend.** Billig und regressionsfest, aber sie prüfen nur, was jemand vorher zu fragen wusste; deshalb Ergänzung, nicht Ersatz. Für S5 wären es drei Sätze: Kopfzeile beginnt bei y=0 und ist nicht höher als ihr `sizeHint`; der Splitter beginnt unmittelbar unter Kopfzeile und Meldung; der Splitter belegt mindestens 80 Prozent der Fensterhöhe.

**(c) Dev-Screenshot als Übergabeartefakt — empfohlen, aber nicht allein.** Nötig für alles, was erst am echten Compositor entsteht: Dekoration, Farbschema, HiDPI, der Tooltip am deaktivierten Suchfeld, Orca. Als alleiniges Mittel taugt es nicht, sonst hängt meine Prüfung wieder an einer fremden Meldung.

**Nicht empfohlen, weil unerprobt:** Nested `kwin_wayland --virtual`. Die Optionen existieren (`--virtual`, `--width`, `--height`, `--exit-with-session`), aber ich habe bewusst keinen nested Compositor in der laufenden Sitzung des Kunden gestartet. Falls (a) einmal nicht reicht, ist das der nächste Schritt — dann mit eigenem Prüfauftrag.

**Konkrete DoD-Änderung** (`docs/scrum/PROZESS.md`, Punkt 3, Zusatz für UI-Stories):

> Der UI-Review ist ohne Bild nicht geführt. Der Entwickler legt mit der Übergabe je UI-Story mindestens einen Screenshot des gebauten Fensters pro Wireframe-Zustand ab (Normalfall, Leerzustand, Meldungszustand). `denkzettel-ux` erzeugt zusätzlich **eigene** Bilder aus dem Sprint-Stand und prüft sie gegen den Wireframe. Jede Aussage des Wireframes über Raumaufteilung wird zusätzlich als Geometrie-Zusicherung im Test festgehalten. Ein UI-Review ohne eigene Bildprüfung zählt für DoD 3 nicht.

Zwei Anschlussänderungen gehören dazu. Erstens: Der UI-Review-Bericht wird als Artefakt abgelegt (etwa `docs/scrum/reviews/`) und nennt die geprüften Bilddateien — in Sprint 2 lag mein Bericht dem Scrum Master gar nicht vor (Protokoll, Zeile 632), er musste sich auf die Zusammenfassung des PO stützen. Zweitens: Meine Checklistenpunkte dürfen nicht mehr aus dem Gedächtnis entstehen, sondern aus dem Wireframe — jeder gezeichnete Bereich erzeugt genau eine Prüffrage. Ergänzend schlage ich vor, in die Festlegungstafel zu 2b/2c (Wireframe-Zeile 410 ff.) eine Zeile „Raumaufteilung" aufzunehmen, die als Prüfsatz taugt: Kopfzeile oben bündig, Liste und Detail über die volle Resthöhe, keine Leerfläche zwischen Kopfzeile und Inhalt.

## 4. Werkzeuge aus UX-Sicht

Ehrliche Lage zuerst: Das Feld der visuellen Regressionstests ist fast vollständig Web (BackstopJS, Playwright, Percy, jest-image-snapshot). Für Qt-Widgets ist davon nichts brauchbar.

| Werkzeug | Stand auf Ganymed | Einschätzung |
|---|---|---|
| Qt offscreen + `QWidget::grab()` | vorhanden (qt6-base 6.11.1) | **Sofort umsetzbar**, heute erprobt. Das eigentliche „Tool" ist ein Skript. |
| ImageMagick `compare` | installiert (7.1.2.29) | Referenzbildvergleich mit Toleranz. Nur als Wächter „hat sich das Bild geändert", nicht als Abnahmekriterium — Theme- und Schriftwechsel erzeugen sonst Dauerfehlalarm. |
| KDE `selenium-webdriver-at-spi` | nicht in den Arch-Repos | Der offizielle KDE-Weg für Blackbox-GUI-Tests über AT-SPI, aktiv gepflegt. Prüft Bedienung und Zugänglichkeit in einem: Was der Treiber nicht findet, findet Orca auch nicht. Lohnt ab M3; Installation wäre eine Kundenentscheidung nach Werkzeug-Evaluationsregel. |
| openQA, dogtail | nicht installiert | Zu schwer beziehungsweise zu alt für dieses Projekt. Verworfen. |

**Meine Priorität:** Erstens ein Screenshot-Helfer als Projekt-Skill unter `.claude/skills/` — dort liegt bisher nur `agents/`, es ist also anzulegen; nichts zu installieren, sofort wirksam, und er ist zugleich das Werkzeug, mit dem ich diese Stellungnahme belegt habe. Zweitens die Geometrie-Zusicherungen in `librarytest.cpp`. Drittens der Bildvergleich, sobald es Referenzbilder gibt. Viertens der AT-SPI-Treiber als Prüfauftrag vor M3.

Zum semgrep-Vergleich des Kunden: Eine statische Regel „`QSplitter` oder Scrollbereich per `addWidget` in ein `QBoxLayout` ohne Stretch-Faktor" wäre für genau diesen Fehler denkbar; semgrep führt C++ als unterstützte Sprache. Ob das praktikabel ist, müsste geprüft werden — als Ergänzung, nie als Ersatz für das Bild.

## Offene Punkte

- Die Heilung selbst gehört dem Dev; ich habe keine Quelldatei angefasst. Der Befund ist `librarywindow.cpp:162`, der Fix ist ein Stretch-Faktor, die Wirkung ist oben gemessen.
- Nested `kwin_wayland --virtual` ist unerprobt und braucht einen eigenen Auftrag, falls gewünscht.
- Die DoD-Änderung und die Wireframe-Ergänzung sind Vorschläge; über beides entscheiden PO und Scrum Master, nicht ich.
- Belege liegen unter `scratchpad/`: `echt-ist.png` und `echt-soll.png` (echtes Fenster, Breeze), `shot.cpp` (Helfer gegen `denkzettelui`), `layoutprobe.cpp` (isolierter Nachbau der Layout-Verschachtelung). Das Verzeichnis ist sitzungsgebunden — falls die Bilder in die Retro-Akte sollen, müssen sie vom PO gesichert werden.
