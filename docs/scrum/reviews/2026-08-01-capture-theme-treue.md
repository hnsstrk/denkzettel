# Untersuchung: Theme-Treue des Capture-Fensters

**Datum:** 01.08.2026 · **Modus:** UI-Review (Untersuchungsauftrag, keine Story)
**Anlass:** Kundenbefund vom 01.08.2026 — „Wirkt wie ein Fremdkörper. Sieht nicht
aus als ob es zu KDE gehören würde. Wenn ich ein anderes Theme verwende muss es
sich anpassen." Präzisiert: „Das Capture Fenster muss auch zum ausgewählten Theme
passen. Bspw. Farben, Schriften, abgerundete Ecken."

Geprüft am gebauten Stand `build/bin/denkzetteld` (identisch mit dem beim Kunden
installierten `/usr/bin/denkzetteld`), Commit `7ff90d2`. Bilder, Messwerte und die
Quellen der Prüfprogramme liegen unter
`docs/scrum/reviews/2026-08-01-capture-theme/`.

**Maßstab:** nicht ein bestimmtes Aussehen, sondern Mitgehen. Je Achse lautet die
Frage: Ändert sie sich beim Theme-Wechsel — und wenn nein, liegt es an unserem
Code oder an der Bauart rahmenloser Fenster?

## Kurzantwort auf die drei Achsen

| Achse | Geht mit? | Ursache, wenn nein |
|---|---|---|
| **Farben** | überwiegend ja | die beiden Kleintexte nicht — **unser Code** (B1) |
| **Schriften** | beim Start ja, im Betrieb nein | überwiegend **Bauart** (B6), plus ein eigener Folgefehler (B7) |
| **Abgerundete Ecken** | nein | **Bauart** rahmenloser Fenster (B2) — heilbar über das Theme, nicht über eine feste Zahl |

## Wie geprüft wurde

Die Sitzung des Kunden wurde nicht angefasst. Alle Bilder stammen aus einer
**verschachtelten Plasma-Sitzung** (`kwin_wayland --socket wayland-probe` mit
eigenem D-Bus, eigenem `XDG_CONFIG_HOME` und `plasmashell`), in der Farbschema und
Systemschrift mit `plasma-apply-colorscheme` bzw. `kwriteconfig6 --notify`
gewechselt wurden. Der echte Daemon lief darin. Geprüfte Farbschemata:
BreezeLight, BreezeDark, CachyOSNord, KritaBright; rechnerisch alle 18
installierten.

Ergänzend sieben Prüfprogramme, die gegen `libdenkzettelcapture.a` linken und
keinen Projekt-Code verändern (Quellen liegen beim Bericht).

**Zwei Zwischenmessungen waren durch die Prüfprogramme selbst verfälscht** und
sind mit `verworfen-` gekennzeichnet: eines setzte eine eigene
Anwendungs-Palette, ein anderes eine Sandbox-Konfiguration — beides schneidet die
Anwendung von Systemänderungen ab. Alle Befunde unten sind mit dem echten
`build/bin/denkzetteld` nachgemessen.

---

## Achse 1 · Farben

**Antwort: Die Flächen gehen mit, zwei Textstellen nicht.**

Geprüft wurden Fensterhintergrund, Feldhintergrund, Textfarbe, **Auswahlfarbe**,
**Auswahltext**, **Scrollbalken** und der Rand (`achse1.txt`,
`achse1-auswahl-*.png`):

| | BreezeLight | BreezeDark | CachyOSNord |
|---|---|---|---|
| Fenster | `#eff0f1` | `#202326` | `#181b28` |
| Feld | `#ffffff` | `#141618` | `#161925` |
| Text | `#232629` | `#fcfcfc` | `#47adda` |
| Auswahl | `#3daee9` | `#3daee9` | `#47add6` |
| Auswahltext | `#ffffff` | `#fcfcfc` | `#0a0b0c` |
| Scrollbalken | folgt | folgt | folgt |

Alle diese Werte kommen aus der Palette und wechseln mit dem Schema — auch beim
Wechsel im laufenden Betrieb (`daemon-1-dark.png` → `daemon-2-nach-wechsel-light.png`).
Der Befund des PO — keine hartkodierten Farben, kein Stylesheet — trifft zu.

**Einen Rand gibt es nicht** (siehe B2 und B3). Und zwei Textstellen gehen nicht
mit — siehe B1.

## Achse 2 · Schriften

**Antwort: Beim Start ja, im laufenden Betrieb nein — und das liegt überwiegend an
der Bauart.**

*Beim Start* nimmt das Fenster die Systemschriften korrekt: Das Textfeld erbt die
allgemeine Schrift, die Kleintexte nehmen die eingestellte kleinste lesbare
Schrift (`capturewindow.cpp:30`). Nach Umstellung auf „Noto Serif" und Neustart
des Dienstes sind **beide** in Serifenschrift und in der neuen Größe —
`achse2-schriftvergleich.png` zeigt Textfeld und Fußzeile vorher/nachher.

*Im Betrieb* geht nichts mit: `achse2-B-nach-wechsel-beide-laufen.png` zeigt
KWrite und das Capture-Fenster nebeneinander, beide liefen während der
Umstellung — KWrite ist umgesprungen, unser Fenster nicht.

**Die Ursache ist aber nicht unser Code.** Eine nackte Qt-Widgets-Anwendung ohne
jedes eigene Zutun bekommt die Änderung ebenfalls nicht (siehe B6): über 25
Sekunden und zwei Umstellungen blieb `QApplication::font()` unverändert. Plasma
reicht Qt-Widgets-Anwendungen eine geänderte Systemschrift nicht von selbst nach;
KWrite geht mit, weil die KDE-Anwendungen selbst auf die Konfiguration lauschen.
Wer sich einreihen will, muss dasselbe tun.

Farbe und Schrift verhalten sich also unterschiedlich: Das Farbschema wird von
Plasma an alle Qt-Anwendungen verteilt, die Schrift nicht.

## Achse 3 · Abgerundete Ecken

**Antwort: Nein — und die Ursache ist die Bauart rahmenloser Fenster.**

Ein Fenster ohne Dekoration (`capturewindow.cpp:41`) bekommt von der
Fensterverwaltung weder Rundung noch Kontur noch Schatten; KWin hat für Clients
keinen Rundungseffekt (geladen ist von den einschlägigen nur `blur`). Sichtbar in
`capture-neben-plasma-popup-BreezeDark.png` und im 4-fach vergrößerten
`ecken-vergleich-BreezeDark.png`.

**Die Frage des PO — Theme oder feste Zahl — ist eindeutig beantwortbar: Theme.**
Plasma-Popups und KRunner zeichnen ihren Hintergrund nicht selbst, sondern
rendern `dialogs/background` aus dem **Desktop-Theme des Nutzers**. Diese Grafik
enthält die Eckstücke (`topleft`, `topright`, …), die Ränder (`hint-*-margin`,
`hint-*-inset`) und beim Standard-Theme auch die Schattenteile (`shadow-corner`,
`shadow-side`).

Nachgerendert mit `KSvg::FrameSvg` in der Größe des Capture-Fensters
(`achse3-huellen.txt`, `achse3-huelle-*.png`):

| Desktop-Theme | Ränder l/r/o/u |
|---|---|
| default, breeze-light | 4 / 4 / 4 / 4 px |
| CachyOS-Nord-round, Iridescent-round, cachyos-emerald | 8 / 8 / 8 / 8 px |

Die gerenderten Hüllen unterscheiden sich sichtbar in Radius, Deckkraft und Farbe.
Auf dem Rechner des Kunden sind **acht Desktop-Themes** installiert, zwei davon
tragen „round" im Namen. **Eine feste Zahl im Code wäre derselbe Fehler in Grün** —
sie bliebe rund, wenn der Kunde ein eckiges Theme wählt. Die Theme-Hülle geht mit.

---

## Befunde

### B1 · fail — Die Kleintexte frieren beim Themewechsel ein (Achse 1, unser Code)

**Fundstelle:** `src/capture/capturewindow.cpp:32–34`

`subtleLabel()` liest die Platzhalterfarbe einmalig und schreibt sie als feste
Farbe zurück ins Label. Damit ist die Rolle überschrieben; ein späterer
Palettenwechsel erreicht sie nicht mehr. Weil der Dienst das Fenster dauerhaft
vorhält (SPEC 2.1), wird es genau einmal beim Dienststart gebaut — der Fehler
bleibt bis zum nächsten Neustart des Dienstes.

| Zustand | Fenstergrund | Kleintext | Kontrast |
|---|---|---|---|
| Start unter BreezeDark | `#202326` | `#a1a9b1` | 6,64:1 |
| **nach Wechsel auf BreezeLight** | `#eff0f1` (folgt) | `#a1a9b1` (bleibt) | **2,09:1** |
| Dienst unter BreezeLight neu gestartet | `#eff0f1` | `#707d8a` | 3,69:1 |

2,09:1 liegt weit unter dem WCAG-Minimum von 4,5:1.

**Korrekturvorschlag:** die Farbe nicht festschreiben, sondern die Zeichenrolle
setzen — `label->setForegroundRole(QPalette::PlaceholderText)` statt der drei
Palettenzeilen. Nachgemessen (`varianten.txt`, `variante-B-rolle.png`): nach dem
Wechsel 3,69:1, identisch zum frisch gebauten Fenster.
**Aufwand:** Zeile.

### B2 · fail — Fensterhülle ohne Rundung, Kontur und Schatten (Achse 3, Bauart)

**Fundstelle:** `src/capture/capturewindow.cpp:41`

Siehe Achse 3. Das ist der „Fremdkörper"-Eindruck.

**Der Wireframe fordert die Hülle bereits** — `border-radius:8px`,
`border:1.5px solid`, `box-shadow` in `wireframes/Denkzettel Wireframes.dc.html:23`
und `:32` (hell) sowie `:210` (dunkle Fassung), dort mit an die Helligkeit
angepasster Konturfarbe. Es ist also keine Geschmacksfrage, sondern eine
Abweichung vom eigenen Wireframe.

**Korrekturvorschlag:** `KSvg::FrameSvg` mit `dialogs/background` aus dem
Desktop-Theme des Nutzers rendern (KF6::Svg ist installiert), dazu
`WA_TranslucentBackground`, die Maske aus `FrameSvg::mask()` und Unschärfe über
`KWindowEffects`. Dann kommen Rundung, Kontur, Deckkraft und Schatten aus dem
Theme und wechseln mit ihm. **Nicht** die Fensterdekoration einschalten — das
brächte eine Titelleiste gegen SPEC 3. **Nicht** einen Radius fest verdrahten.
**Aufwand:** eigene Story mit Wireframe-Vorlauf.

### B3 · warn — Das Eingabefeld hängt an einem Farbunterschied, den kein Schema garantiert (Achse 1)

**Fundstelle:** `src/capture/capturewindow.cpp:47` (`setFrameShape(QFrame::NoFrame)`)

Ohne Rahmen ist die einzige Abgrenzung des Felds der Unterschied zwischen
`QPalette::Window` und `QPalette::Base`. Über alle 18 installierten Schemata
gemessen (`feldabgrenzung.txt`): **neun liegen unter 1,07:1** — darunter
CachyOSNord und EmeraldDark mit je 1,02:1, beide beim Kunden installiert. Dort
verschwindet das Eingabefeld optisch ganz. Selbst Breeze kommt nur auf 1,14:1
bzw. 1,15:1. Zusätzlich fehlt die Fokus-Hervorhebung, die Breeze sonst zeichnet.

**Korrekturvorschlag:** zusammen mit B2 entscheiden — entweder eine eigene Kontur
aus der Palette, oder das Fenster wird wie im Wireframe **eine einzige Fläche**.
**Aufwand:** Story (mit B2).

### B4 · warn — Die Trennlinie über der Fußzeile fehlt

Der Wireframe zeichnet über „Esc verwirft · Strg+Enter speichert" eine Trennlinie
(`:26`, `:35`). Im Layout (`capturewindow.cpp:55–63`) gibt es kein Trennelement.
**Aufwand:** Zeile (mit B3).

### B5 · ok — Die Flächenfarben folgen der Palette

Siehe Achse 1.

### B6 · warn — Die Schrift geht im Betrieb nicht mit (Achse 2, überwiegend Bauart)

Belegt durch eine Wache in der Testsitzung: über 25 Sekunden mit zwei
Umstellungen der Systemschrift blieb `QApplication::font()` einer nackten
Qt-Widgets-Anwendung unverändert bei „Noto Serif 15 pt". Plasma reicht
Qt-Widgets-Anwendungen Schriftänderungen nicht nach — anders als Farbschemata.
KWrite geht mit, weil es selbst lauscht (`achse2-B-nach-wechsel-beide-laufen.png`).

**Kein Fehler in unserem Code**, aber ein sichtbarer Unterschied zu KDE-eigenen
Anwendungen. Wer ihn schließen will, muss wie KWrite selbst auf die Konfiguration
lauschen (`KConfigWatcher` auf `kdeglobals`) und die Schriften neu setzen.
**Aufwand:** Story.

### B7 · warn — Die Fensterhöhe wird bei einer Schriftänderung nicht neu berechnet

**Fundstelle:** `src/capture/capturewindow.cpp:65–68` und `:146–155`

`adjustHeight()` hängt allein an `documentSizeChanged`. Eine Schriftänderung
ändert die Zeilenzahl des Dokuments nicht, also läuft die Höhenrechnung nicht
neu — das Textfeld behält seine mit `setFixedHeight` gesetzte alte Höhe.
Gemessen (`schriftwechsel.txt`): nach einer Umstellung von 9 pt auf 16 pt bleibt
das Feld 93 px hoch bei einem Zeilenabstand von 28 px — **rund 3 Zeilen statt der
in SPEC 3 geforderten 5**; ein frisch gebautes Fenster misst 148 px. Der Kunde
hatte in der Sprint-1-Abnahme bereits moniert, dass 3 Zeilen zu wenig sind.

Heute tritt das kaum auf, weil B6 die Änderung ohnehin nicht durchlässt. Es wird
akut, sobald B6 geschlossen wird — und vermutlich schon beim Wechsel auf einen
Bildschirm mit anderer Skalierung (nicht nachgemessen).
**Korrekturvorschlag:** `adjustHeight()` zusätzlich bei `QEvent::FontChange`
aufrufen. **Aufwand:** Zeile.

---

## Trennung: Geschmack, unser Code oder Bauart

- **Unser Code:** B1 (Farbe folgt nicht), B3 (Abgrenzung nicht garantiert),
  B4 (fehlende Linie), B7 (Höhe wird nicht nachgerechnet).
- **Bauart, heilbar:** B2 (rahmenlose Fenster bekommen nichts geschenkt — die
  Theme-Hülle ist der KDE-eigene Weg), B6 (Plasma reicht Schriften nicht nach —
  selbst lauschen ist der KDE-eigene Weg).
- **Reine Geschmacksfragen** sind bei der Prüfung keine aufgefallen.

## Empfehlung, in dieser Reihenfolge

1. **B1 sofort heilen — Zeile.** Trifft den Kundensatz wörtlich; ohne sie ist das
   Fenster nach jedem Farbschemawechsel messbar falsch, bis der Dienst neu startet.
2. **B2 als eigene Story — die Theme-Hülle.** Das ist der „Fremdkörper" und
   zugleich die Achse „abgerundete Ecken". Über `KSvg::FrameSvg` kommt die Form
   aus dem Theme und geht mit; eine feste Zahl täte das nicht.
3. **B3 und B4 in dieselbe Story** wie B2 — dieselben Stellen, und B3 entscheidet
   die Frage „eine Fläche oder zwei".
4. **B7 mitnehmen — Zeile.** Billig und verhindert, dass die Höhenrechnung falsch
   wird, sobald die Schrift wechselt.
5. **B6 zuletzt und nur, wenn der Kunde darauf besteht.** Es ist der einzige
   Punkt, an dem wir mehr tun müssten als Qt-Anwendungen von sich aus tun, und der
   Fall ist der seltenste der drei Achsen.

**Wireframe-Arbeit ist nötig** (Hüllenmaße für B2, Flächenfrage für B3). Sie wurde
auftragsgemäß **noch nicht** ausgeführt.

## Grenzen dieser Untersuchung

- **KRunner** ließ sich in der Testsitzung nicht im Bild halten — es schließt bei
  Fokusverlust, und der Schnappschuss-Weg über die KWin-Schnittstelle ist ohne
  Autorisierung gesperrt. Als Vertreter derselben Bauart diente ein
  Plasma-Benachrichtigungs-Popup; der Vergleichspunkt ist derselbe. Die Machart
  von KRunner ist stattdessen über die Theme-Ressource `dialogs/background`
  belegt, die es benutzt.
- Fremde **Plasma-Stile** (Fensterdekoration, Desktop-Theme) wurden in der
  laufenden Sitzung nicht durchgespielt; die Theme-Hüllen wurden gerendert, nicht
  im Betrieb gewechselt.
- Der Skalierungsfall zu B7 ist eine begründete Vermutung, keine Messung.

---

## Nachtrag vom 04.08.2026 — 23 Dateinamen tragen 6 Aufnahmen

Aufgefallen bei der Erstanwendung des Prüfskripts `docs/scrum/bildbelege-pruefen.sh`
(Beschluss V2 vom 04.08.2026), das prüfsummengleiche Bilder innerhalb eines
Belegordners meldet. Bewertung: `docs/scrum/reviews/2026-08-04-bildbelege-bewertung.md`,
Mängel M-A und M-B.

**Die Aussage dieses Berichts bleibt richtig, die Zahl der Belege nicht.**
Oben heißt es: *„Die gerenderten Hüllen unterscheiden sich sichtbar in Radius,
Deckkraft und Farbe."* Das trifft zu — sechs verschiedene Hüllen unterscheiden
sich tatsächlich. Die **23 `achse3-huelle-*`-Dateien tragen aber nur sechs
Aufnahmen**, und die Dateiliste überzeichnet damit die Zahl der Messungen.

Zwei Einzelheiten, die erst dadurch sichtbar wurden:

1. **Das Farbschema ändert die Hülle nicht.** Je Desktop-Theme sind die
   Aufnahmen unter `BreezeDark`, `BreezeLight` und `CachyOSNord` bytegleich.
   Das ist ein **Ergebnis dieser Messreihe**, das oben nicht ausgesprochen ist:
   Die Hülle folgt dem Desktop-Theme, nicht dem Farbschema. Es stützt die
   Aussage „Die Theme-Hülle geht mit" — sie stand nur nie als Befund da.
2. **`cachyos-emerald`, `-color` und `-light` liefern eine einzige Hülle**
   (sieben Dateien, eine Aufnahme). Für dieses Tripel trägt der Satz
   „unterscheiden sich sichtbar" **nicht**.

**Ein Dateiname stammt aus einem Skriptfehler.** Zwei Dateien heißen
`achse3-huelle-default breeze-light breeze-dark cachyos-emerald
cachyos-emerald-color cachyos-emerald-light CachyOS-Nord-round
Iridescent-round-<Schema>.png` — eine nicht in Anführungszeichen gesetzte
Schleifenvariable hat sämtliche Theme-Namen in **einen** Namen geschrieben. Sie
sind bytegleich mit den `-default-`-Aufnahmen und tragen keine eigene Messung.

**Nichts wird gelöscht.** Dieser Bericht ist Beweislage; ein nachträglich
geglätteter Beleg ist keiner mehr. Der Nachtrag steht hier, damit niemand die
Dateizahl für die Messzahl hält.
