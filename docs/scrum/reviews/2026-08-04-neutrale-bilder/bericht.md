# Neutrale README-Bilder — Zulieferung und eigene Bildprüfung

**Modus:** Gestaltung (Zulieferung von Bildern), mit eigener Bildprüfung nach
DoD 3 / B3.
**Datum:** 04.08.2026 · **Rolle:** `denkzettel-ux`
**Anlass:** Kundenauftrag vom 04.08.2026 — „Für die README.md brauchen wir auch
mal neutrale Screenshots, ohne meine persönlichen Notizen."

Das Repository ist öffentlich. Die beiden bisherigen README-Bilder zeigten den
echten Notizbestand des Kunden, darunter Notizen über dieses Projekt selbst.

## 1. Was geliefert wurde

| Datei | Ersetzt | Maße | Inhalt |
|---|---|---|---|
| `docs/bilder/erfassungsfenster.png` | dieselbe Datei, Stand `326febd` | 1200 × 324 px | Erfassungsfenster mit einem erfundenen Gedanken |
| `docs/bilder/bibliothek.png` | dieselbe Datei, Stand `326febd` | 2000 × 1520 px | Bibliothek, fünf Tagesgruppen, eine Notiz geöffnet |

**Die Dateinamen bleiben.** Damit bleiben `README.md:6` und `README.md:40`
unverändert gültig, und die Bilder lassen sich mit einem Befehl an genau die
Stelle erzeugen, an der sie liegen. Die alten Fassungen sind nicht verloren,
sie stehen in der Historie (`git show 326febd:docs/bilder/bibliothek.png`).
Wenn der PO stattdessen neue Namen will, kostet das je eine Zeile in
`README.md` und je eine Zeichenkette in `tests/readmeshots.cpp`.

## 2. Woher die Bilder kommen

Vorhandene Läufer geprüft: `libraryshots`, `editshots`, `searchshots`. Keiner
liefert die beiden Ansichten:

- Für das **Erfassungsfenster** gibt es gar keinen Läufer — die drei
  vorhandenen linken alle `denkzettelui`, das Erfassungsfenster sitzt in
  `denkzettelcapture`.
- Die **Bibliotheksbilder** von `libraryshots` sind Belegbilder zu den
  Zeichnungen 3a und 3b; ihre Nummerierung bildet die Wireframe-Fälle ab, und
  ihr Notizinhalt handelt durchgehend von diesem Projekt („Idee für Denkzettel
  …", „Tray-Icon im dunklen Theme testen", `journalctl -u whisperd …`). Er
  taugt als Prüfmaterial, aber nicht für die README — und ihn dort zu ändern
  hieße, eine abgenommene Belegreihe umzuschreiben.

Deshalb **ein** neuer Läufer statt zweier Eingriffe:
`tests/readmeshots.cpp`, Ziel `readmeshots`, `EXCLUDE_FROM_ALL` wie die
anderen Bildläufer, eingetragen in `tests/CMakeLists.txt`. Er linkt
`denkzettelcapture` und `denkzettelui` und schreibt beide README-Bilder in
einem Lauf. Geändert wurden nur diese zwei Dateien; an `libraryshots`,
`editshots` und `searchshots` ist nichts angefasst worden.

Der Läufer übersetzt sich **ohne eine einzige Warnung** (frisch übersetzt,
`KDECompilerSettings` gesetzt). Das ist hier keine Nebensache: Seit dem
04.08.2026 lässt der automatische Testlauf jede Compiler-Warnung durchfallen —
allerdings baut er `EXCLUDE_FROM_ALL`-Ziele nicht, dieser Läufer käme dort also
gar nicht erst vorbei. Geprüft wurde er deshalb hier.

## 3. Wiederholbefehl

```
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target readmeshots
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=2 \
    LANG=de_DE.UTF-8 build/bin/readmeshots docs/bilder
```

Der `--target readmeshots` ist kein Schmuck: Der Läufer ist `EXCLUDE_FROM_ALL`,
ein gewöhnlicher Build fasst ihn nicht an, und ein alter Läufer schriebe
plausible Bilder eines alten Standes mit frischem Zeitstempel (Vorfall
Sprint 5). Die drei Umgebungsvariablen ebenso wenig:

- `QT_QPA_PLATFORMTHEME=kde` — sonst liefert `SmallestReadableFont` eine
  Ersatzschrift, die größer ausfällt als die Eintragsschrift, und die
  Rangfolge der Schriftgrößen kehrt sich um.
- `QT_SCALE_FACTOR=2` — siehe Befund B2.
- `LANG=de_DE.UTF-8` — Gruppenköpfe, Wochentagsform und Uhrzeitformat kommen
  aus `QLocale()`. Der Läufer schreibt sich die Sprache nicht selbst vor; unter
  einer anderen Locale entstünden andere Zeitstempel.

Für diesen Lauf wurde außerhalb des Projektbaums gebaut, damit das gemeinsame
`build/` der parallel arbeitenden Stränge unberührt bleibt. Nach `/usr` wurde
nichts installiert.

## 4. Der Inhalt der Notizen

Alle acht Texte sind erfunden: Fahrradschlauch, Kürbissuppe, Fotobuch,
Podcast, Anhänger der Werkstatt, Bildband, Bücherkisten, Wanderung. Keine
Namen, keine Adressen, keine Zugangsdaten, kein Bezug auf dieses Projekt oder
seine Beteiligten. Die Längen sind bewusst ungleich — ein Einzeiler
(„Kürbissuppe braucht mehr Ingwer"), zwei echte Zeilenumbrüche, zwei Texte,
die in der Liste abgeschnitten werden.

**Der Bestand des Kunden ist nicht berührt worden.** Der Läufer legt Store und
Konfiguration in je einem `QTemporaryDir` an und übergibt dem `Store` immer
einen ausdrücklichen Pfad; `Store::defaultPath()` wird nicht aufgerufen.
Gegenprobe nach dem Lauf: `~/.local/share/denkzettel/denkzettel.db` trägt
unverändert den Änderungszeitpunkt **02.08.2026 19:24:30**, zwei Tage vor
diesem Lauf.

## 5. Eigene Bildprüfung

Geprüfte Bilder — angesehen, nicht nur erzeugt:

- `docs/bilder/erfassungsfenster.png` (2×) und
  `docs/bilder/bibliothek.png` (2×) — die Auslieferung.
- `massstabsprobe-1x-erfassungsfenster.png` und
  `massstabsprobe-1x-bibliothek.png` (neben diesem Bericht) — derselbe Lauf
  ohne `QT_SCALE_FACTOR`, als Messung zu B2.

| Nr. | Prüffrage (aus Zeichnung 2b/3a bzw. dem Auftrag) | Befund | Verdikt |
|---|---|---|---|
| B1 | Zeigt die Liste mehrere Tagesgruppen? | Fünf: Heute, Gestern, Diese Woche, Letzte Woche, Älter — alle vollständig, keine wird von der Unterkante abgeschnitten, keine Bildlaufleiste. Dafür misst das Fenster 1000 × 760 statt der 900 × 600 der Prüfläufer; bei 600 px Höhe fiel „Letzte Woche" halb aus dem Bild. | ok |
| B2 | Ist die Auflösung in der README lesbar — und verzerrt der Kunstgriff dafür die Größenverhältnisse? | GitHub passt ein Bild in rund 800 px Spaltenbreite ein; ein 1×-Bild käme herunterskaliert und weich an. Deshalb `QT_SCALE_FACTOR=2`. **Gemessen, nicht angenommen:** Derselbe Lauf ohne den Faktor liefert 1000 × 760 bzw. 600 × 162 px — exakt die Hälfte in beiden Achsen —, und die Umbruchstellen jeder einzelnen Zeile sind dieselben. Der Faktor ändert die Auflösung, nicht das Layout. | ok |
| B3 | Ist Text abgeschnitten, wo er nicht abgeschnitten sein soll? | Die Auslassung am Ende der zweiten Vorschauzeile ist die zugesicherte Machart des Listeneintrags (2b, „Listeneintrag"). Im Lesebereich und im Erfassungsfenster steht jeder Text vollständig. | ok |
| B4 | Raumaufteilung: Kopfzeile oben bündig, feste Höhe, Meldungszeile ohne Höhe? | Am 1×-Bild in Spalte x = 6 gemessen: Fensterfarbe von y = 0 bis y = 47, ab y = 48 die Listenfläche. Kopfzeile also **exakt 48 px** — der Wert, den die Zeichnung nach der Korrektur vom 01.08.2026 nennt —, und darunter beginnt unmittelbar die Liste: die Meldungszeile belegt ohne Meldung keine Höhe. | ok |
| B5 | Liste und Detail nebeneinander, volle Resthöhe, Liste 300 px Startbreite? | Am 1×-Bild in Zeile y = 700 gemessen: Listenfläche bis x = 299, Trennlinie bei x = 300, Detailfläche ab x = 313. Listenbreite **exakt 300 px** wie zugesichert. Die Listenfläche steht in Spalte x = 6 ohne Unterbrechung bis zur letzten Bildzeile (y = 759), reicht also bis an die Unterkante des Fensterinhalts. | ok |
| B6 | Stimmen Zeitstempel und Gruppen zusammen (3b)? | „15:04"/„12:20"/„09:12" unter Heute, „21:38" unter Gestern, „Di., 28. Juli" unter Diese Woche, „Do., 23. Juli" unter Letzte Woche, „10.07.2026" unter Älter; im Lesekopf „Heute 15:04". | ok |
| B7 | Ist versehentlich etwas vom echten Bestand zu sehen? | Nein. Beide Bilder enthalten ausschließlich die acht erfundenen Texte, die Beschriftungen der Anwendung und den Anwendungsnamen „Denkzettel" im Kopf des Erfassungsfensters. | ok |
| B8 | Erfassungsfenster gegen Zeichnung 1a: Name oben, Textfeld, Kürzelhinweis unten? | Alle drei vorhanden, „Esc verwirft · Strg+Enter speichert" mittig unten; der Textcursor steht am Ende des Textes, wie nach dem Tippen. | ok |
| B9 | Farbschema | Beide Bilder in Breeze Dark — dasselbe Schema wie die Bilder, die sie ersetzen, damit sich die README nicht in der Mitte umfärbt. Der Läufer setzt die Palette ausdrücklich; das Konfigurationsverzeichnis ist ein leeres temporäres, es gäbe sonst keine `kdeglobals`, aus der ein Schema zu lesen wäre. Die vier tragenden Farben sind die gemessenen aus `docs/scrum/reviews/2026-08-01-capture-theme/palette.txt`, wie in `libraryshots`. | ok |
| B10 | Fensterrahmen | `QWidget::grab()` nimmt den Fensterinhalt auf, nicht die Dekoration: Titelleiste, Schatten und Rundung fehlen, das alte `bibliothek.png` hatte sie (Aufnahme aus einer laufenden Sitzung). Der Bildinhalt selbst ist vollständig. Dekoration wäre nur über eine verschachtelte KWin-Sitzung zu haben — das ist kein DoD-3-Weg und wäre eine eigene Entscheidung des PO. | warn |
| B11 | Lesebereich | Die geöffnete Notiz füllt drei Zeilen eines hohen Bereichs; darunter steht viel dunkle Fläche. Das ist der ehrliche Anblick der Anwendung bei kurzen Notizen, und es ist besser als das bisherige Bild, das dort „Keine Notiz ausgewählt" zeigte. Wer die Fläche kleiner will, zahlt mit einer Tagesgruppe (B1). Kein Korrekturvorschlag, bewusst so gelassen. | ok |

**Zu B10 — mein Vorschlag:** so lassen. Die README zeigt dann denselben
Bildtyp wie jeder Prüfbericht dieses Projekts, und der fehlende Rahmen kostet
Schmuck, keine Aussage. Wer die Dekoration will, braucht eine verschachtelte
Sitzung, einen menschlichen Auslöser und ein Verfahren, das niemand
wiederholen kann, ohne sie erneut aufzusetzen.

## 6. Beobachtet, nicht geheilt

- `README.md:128` bindet `docs/scrum/diagramme/kegel.svg` ein. Die Datei ist im
  Arbeitsbaum zur Löschung vorgemerkt (Strang Kegel-Rückbau). Fremde Fläche —
  gemeldet, nicht angefasst.
- Keine Änderung an `README.md`, `src/`, `SPEC.md` oder `docs/scrum/PROZESS.md`.
  Nicht committet, nicht gepusht.

## 7. Offene Punkte für den PO

1. Bilder ansehen und den Austausch bestätigen; `README.md` braucht dafür keine
   Änderung, solange die Dateinamen bleiben.
2. Entscheiden, ob der neue Läufer im README-Abschnitt „Bauen und Testen"
   neben `editshots` genannt werden soll — dort steht heute nur ein Beispiel.
   Das ist eine Zeile in `README.md` und gehört dem PO.
