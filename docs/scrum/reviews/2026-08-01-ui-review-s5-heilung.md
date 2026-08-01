# UI-Review der Sprint-2-Heilung — Bibliothek (S5, Issue #7)

**Modus:** UI-Review, erstmals mit eigener Bildprüfung nach Retro-Beschluss B3
und DoD 3. **Datum:** 01.08.2026. **Prüfer:** `denkzettel-ux`.

**Geprüfter Stand:** Branch `worktree-agent-a3e69b026b55f1442`, Commits
`9ee8676`, `7130f81`, `bfc17af`. Gebaut out-of-source im Scratchpad
(`cmake -S <worktree> -B <scratchpad>/build-ux -DCMAKE_BUILD_TYPE=Debug`); im
Arbeitsbaum wurde nichts geschrieben, an Quellcode, SPEC und Wireframes nichts
geändert.

**Ergebnis: kein offener `fail`.** DoD 3 ist von meiner Seite nicht blockiert.
Zwei `warn` und vier `Hinweis` stehen unten; die beiden `warn` betreffen das
Meldungsband (sichtbar, aber gering) und Lücken im Regressionsnetz, nicht die
Richtigkeit der geheilten Oberfläche.

## Prüfmittel

- Testlauf des Worktree-Stands: `ctest` offscreen, **7 von 7 grün**, darunter
  `librarytest` mit den beiden neuen Geometriefunktionen.
- Eigener Bildlauf gegen die gebaute `denkzettelui`,
  `QT_QPA_PLATFORM=offscreen`, `-style breeze`, `QWidget::grab().save()`.
  Quelle: `2026-08-01-ui-review-s5-heilung/ux-shot-s5.cpp`,
  Messwerte: `ux-geometrie.txt`.
- Zwei Gegenproben zur Laufzeit am Widget-Baum, ohne Änderung einer Quelldatei:
  `ux-probe-s5.cpp`, Messwerte `ux-proben.txt`.

### Geprüfte Bilddateien (alle unter `2026-08-01-ui-review-s5-heilung/`)

| Datei | Zustand nach Wireframe |
|---|---|
| `ux-normal-900x600.png` | 2b, Normalfall, erste Fenstergröße |
| `ux-normal-1200x800.png` | 2b, Normalfall, zweite Fenstergröße |
| `ux-leer-1-900x600.png` | 2c, Leerzustand 1 — noch keine Notiz |
| `ux-leer-2-900x600.png` | 2c, Leerzustand 2 — nichts ausgewählt |
| `ux-meldung-900x600.png` | 2b/2c, Meldungszustand nach dem Löschen |
| `ux-leer-1-mit-symbol-900x600.png` | Gegenprobe zu B6 (Fenstersymbol gesetzt) |
| `ux-meldung-ohne-umbruch-900x600.png` | Gegenprobe zu B3 (ohne Wortumbruch) |

Die Prüfpunkte sind aus Wireframe 2b/2c abgeleitet, Bereich für Bereich; die
Raumaufteilung ist dabei ein eigener Punkt (Retro Sprint 2, Auflage an mich).

## Befunde

### B1 `pass` — Die senkrechte Raumaufteilung ist geheilt

Wireframe 2b (`wireframes/Denkzettel Wireframes.dc.html:326–337`) zeichnet die
Kopfzeile als schmale Leiste am oberen Rand und darunter den Inhaltsbereich.
Gemessen am gebauten Stand (`ux-geometrie.txt`):

| | 900×600 | 1200×800 |
|---|---|---|
| Kopfzeile | y=0, h=47 (sizeHint 47) | y=0, h=47 (sizeHint 47) |
| Suchfeld | y=8 | y=8 |
| Splitter | y=47, h=553 → 92 % | y=47, h=753 → 94 % |
| Leerfläche unten | 0 px | 0 px |

Bilder `ux-normal-900x600.png` und `ux-normal-1200x800.png` zeigen Kopfzeile
oben bündig, Liste und Detailbereich über die volle Resthöhe, keine Leerfläche
zwischen Kopfzeile und Inhalt. Das schwebende Suchfeld aus der Kundenabnahme
ist weg.

### B2 `pass` — Das Meldungsband sitzt richtig

Wireframe 2c, Festlegungstafel (`:413`): die Meldung sitzt als
`KMessageWidget` im Fenster unter der Kopfzeile. Gemessen: Meldung y=47 =
Kopfzeilenhöhe, Splitter y=119 = 47 + 72, Splitter weiterhin 80 % der
Fensterhöhe, Leerfläche unten 0 px. Kein Overlay, kein Bildschirmecken-Toast,
Typ Warning, kein Schließen-Knopf (gemessen `Schliessknopf=0`), Text
„Notiz gelöscht — noch 5 s" mit Restzeit und Knopf „Rückgängig".
Bild `ux-meldung-900x600.png`.

### B3 `warn` — Das Meldungsband ist zweizeilig, gezeichnet ist eine Zeile

Wireframe 2b (`:335`) zeichnet Symbol, Text, Restzeit und „Rückgängig" in
**einer** Zeile. Im gebauten Stand steht der Knopf in einer zweiten Reihe unter
dem Text; das Band ist 72 px hoch und nimmt 12 % der Fensterhöhe
(`ux-meldung-900x600.png`).

Ursache ist `setWordWrap(true)` in `src/ui/librarywindow.cpp:153`:
`KMessageWidget` wechselt mit dem Wortumbruch auf ein zweireihiges Layout.
Gemessene Gegenprobe (`ux-proben.txt`, Bild
`ux-meldung-ohne-umbruch-900x600.png`):

| | Bandhöhe | Anteil an 600 px |
|---|---|---|
| mit Wortumbruch (Ist) | 72 px | 12 % |
| ohne Wortumbruch | 49 px | 8 % |

Ohne Wortumbruch steht der Knopf rechts in derselben Zeile — genau die
Zeichnung. Die Mindestbreite des Fensters steigt dabei auf 414 px und bleibt
damit unter jeder brauchbaren Fensterbreite (die Liste allein fordert 220 px).

**Vorschlag:** `setWordWrap(false)`. Der Text ist kurz und in seiner Länge
fest — „Notiz gelöscht — noch N s" wächst nicht.

### B4 `Hinweis` — Der Meldung fehlt das gezeichnete Symbol

Wireframe 2b (`:335`) zeichnet ein Warnsymbol im Band. `KMessageWidget` setzt
von sich aus keines; im Bild ist keines zu sehen. Geringe Schwere, weil Breeze
den Typ über die Rahmenfarbe trägt. **Vorschlag:**
`m_message->setIcon(QIcon::fromTheme(QStringLiteral("dialog-warning")))`.

### B5 `warn` — Zwei Raumaufteilungs-Aussagen des Wireframes sind nicht zugesichert

Die beiden neuen Testfunktionen decken die senkrechte Achse gut ab: Kopfzeile
bei y=0 und nicht höher als ihr `sizeHint`, Splitter unmittelbar darunter,
Suchfeld oben, Meldung als Band dazwischen, beides bei zwei Fenstergrößen. Zwei
Aussagen des Wireframes fehlen aber:

**(a) Der Inhalt reicht bis zur Fensterunterkante.** Der Test fordert nur
`splitter->height() >= window.height() * 3 / 4`
(`tests/librarytest.cpp`, `keepsTheHeaderAtTheTopAndTheRestForTheNotes`). Eine
Leerfläche von bis zu 103 px am **unteren** Rand bliebe bei 900×600
unentdeckt. Gemessen ist der Rest 0 px — die Zusicherung fehlt trotzdem.
Vorschlag:
`QCOMPARE(splitter->mapTo(&window, QPoint()).y() + splitter->height(), window.height())`.

**(b) Die waagerechte Verteilung.** Wireframe 2b zeichnet die Liste mit fester
Breite (`width:280px`) und den Detailbereich mit `flex:1` — überschüssige
Breite gehört dem Detail. Gemessen erfüllt (Liste bleibt bei beiden Größen
300 px, Detail wächst von 599 auf 899 px), aber kein Test hält es fest;
`keepsTheListWideEnoughForThePreview` prüft allein die Mindestbreite. Das ist
dieselbe Fehlerklasse wie der Kundenbefund, nur auf der anderen Achse.
Vorschlag: im datengetriebenen Test zusätzlich
`QCOMPARE(splitter->widget(0)->width(), 300)`.

Kein `fail`, weil die gebaute Oberfläche an beiden Punkten richtig ist; es
fehlt das Regressionsnetz. Ob DoD 1 („jede Aussage des Wireframes über die
Raumaufteilung ist als Geometrie-Zusicherung festgehalten") damit erfüllt ist,
entscheidet der Scrum Master, nicht ich.

### B6 `pass` — Die Belege des Entwicklers zeigen denselben Zustand

Alle vier Bilder unter `docs/scrum/reviews/sprint-02-heilung/` decken sich mit
meinen; die Unterschiede sind Notizanzahl und Uhrzeit der Testdaten, nicht der
Zustand. Keine Abweichung.

Zur Einordnung, ohne Befund gegen die Umsetzung: Im Leerzustand fehlt auf dem
Bild des Entwicklers **und** auf meinem erstem Bild das Symbol über dem Text.
Das ist ein Artefakt beider Helferprogramme — sie setzen kein Fenstersymbol,
und `placeholderPage()` liest `qApp->windowIcon()`. Meine Gegenprobe mit
gesetztem Symbol (wie `src/main.cpp:29`) zeigt es:
`ux-leer-1-mit-symbol-900x600.png`. Für künftige Bildstrecken heißt das: das
Fenstersymbol gehört in den Helfer, sonst belegt der Leerzustand weniger, als
er zu belegen vorgibt.

### B7 `pass` — Beide Leerzustände stimmen mit Wireframe 2c überein

Leerzustand 1 (`ux-leer-1-900x600.png`): nur die Liste trägt den Platzhalter
„Noch keine Notizen / Mit Meta+N einen Gedanken festhalten.", der
Detailbereich bleibt leer — wie in der Festlegung `:416` verlangt, damit ein
leeres Fenster nicht zweimal dasselbe sagt. Leerzustand 2
(`ux-leer-2-900x600.png`): „Keine Notiz ausgewählt / Zum Lesen links eine
Notiz auswählen." im Detailbereich, Liste gefüllt. Beide Texte in
unpersönlicher Infinitivform (SPEC 15).

Am Rande: Leerzustand 2 fehlt in der Bildstrecke des Entwicklers. Die DoD
verlangt „einen Screenshot pro Wireframe-Zustand", und 2c zeichnet zwei.

### B8 `pass` — Fehlschlagmeldung, Zweig „nicht installiert"

`src/shell/shortcutregistration.cpp`, `ApplicationNotInstalled`: nennt die
Ursache (Denkzettel ist nicht systemweit installiert, ohne Desktop-Datei legt
der Dienst keinen Eintrag an), die Abhilfe (Installation) und den Weg, der
bis dahin funktioniert (Symbol im Systemabschnitt). Handlungsleitend im Sinne
der KDE HIG, und es ist der Zweig, den der Kunde am 01.08.2026 getroffen hat.
Kein Verstoß gegen SPEC 15: der Text spricht den Nutzer nicht an, weder mit Du
noch mit Sie.

### B9 `warn` — Fehlschlagmeldung, Zweig „Dienst hat nichts behalten"

Derselbe Ort, `DaemonKeptNothing`: „… — er hat die Registrierung nicht
behalten. **Läuft kglobalacceld?** Das Capture-Fenster bleibt über das Symbol
im Systemabschnitt erreichbar."

Der Prozessname ist Fachjargon, und eine Rückfrage ist kein Schritt: Wer nicht
weiß, was `kglobalacceld` ist, weiß nach dem Satz nicht mehr als vorher. Die
KDE HIG verlangen von Meldungen eine Sprache, die der Nutzer versteht, und
einen Weg nach vorn. **Vorschlag:** den überprüfbaren Schritt nennen statt des
Prozesses, etwa „Die Kurzbefehle lassen sich in den Systemeinstellungen unter
‚Kurzbefehle' prüfen; hilft das nicht, bringt eine neue Anmeldung den Dienst
zurück." Nicht blockierend: es ist der seltene Zweig, nicht der aufgetretene.

### B10 `Hinweis` — Länge der Benachrichtigung

Der erste Text ist rund 270 Zeichen; Plasma kürzt lange Benachrichtigungen.
Der Kern („Meta+N ist beim Kurzbefehl-Dienst nicht angekommen") steht am
Anfang und überlebt jede Kürzung — deshalb nur ein Hinweis, keine Auflage.

### B11 `pass` — Keine Regression an früheren Auflagen

Mindestbreite der Liste 220 px steht (`librarywindow.cpp:45` und `:127`,
gemessen 220 px, Test `keepsTheListWideEnoughForThePreview`). Das
`KMessageWidget` verhält sich unverändert: Typ Warning, kein Schließen-Knopf,
unter der Kopfzeile, mit Aktion „Rückgängig", verschwindet mit der Frist. Das
Suchfeld ist deaktiviert und trägt den Tooltip „Die Volltextsuche steht noch
nicht zur Verfügung." (gemessen, `ux-geometrie.txt`).

### B12 `Hinweis` — Der Transkriptbereich sieht aus wie ein Eingabefeld

Wireframe 2b (`:368`) zeichnet das Transkript als Fließtext auf dem
Fensterhintergrund. Im gebauten Stand steht ein weißer, umrandeter Kasten über
die volle Resthöhe des Detailbereichs (`ux-normal-900x600.png`). Da S8
(Bearbeiten) zurückgestellt ist, verspricht dieser Kasten eine Editierbarkeit,
die es im M2-Stand nirgends gibt.

Der Befund stammt nicht aus der Heilung, sondern aus der ursprünglichen
Umsetzung — mein damaliger, rein statischer Review konnte ihn nicht sehen; er
ist ein Nebenertrag des neuen Verfahrens. **Vorschlag:** Basisfarbe des
`QTextBrowser` auf die Fensterfarbe setzen, damit der Lesebereich als Text
liest. Einordnung und Terminierung sind PO-Sache.

### B13 `Hinweis` — Gezeichnete, aber noch nicht umgesetzte Bereiche

Der Vollständigkeit der Wireframe-Ableitung halber, ohne Befund gegen S5:
Tag-Chips an Listeneinträgen und im Detail, ▶ und Dauer bei Sprachnotizen, der
Player und der Knopf „Bearbeiten" fehlen. Alle liegen jenseits von M2
beziehungsweise in der zurückgestellten Story S8 — so beim Planning
beschlossen.

## Offene Punkte

- Zwei `warn` liegen beim Entwickler: das Meldungsband (B3, eine Zeile) und
  die beiden fehlenden Geometrie-Zusicherungen (B5). Beide sind klein und
  gemessen belegt; ob sie noch in diesen Sprint gehören, entscheiden PO und
  Scrum Master.
- B12 (Transkript als Kasten) ist älter als die Heilung und braucht eine
  Einordnung durch den PO, keine sofortige Änderung.
- Grenzen dieser Prüfung, ehrlich benannt: offscreen gibt es keine
  Fensterdekoration, kein Farbschema des Nutzers, kein HiDPI und keine
  Compositor-Effekte. Der Tooltip am deaktivierten Suchfeld ist im Code und in
  der Messung belegt, nicht im Bild. Die Benachrichtigung des
  Kürzel-Fehlschlags ist am Text geprüft, nicht am gerenderten Plasma-Popup.
