# Belege zu Issue #60 (S33) — Tray-Menü

**Datum:** 02.08.2026 · **Zweig:** `story/60-traymenues` · **Rolle:** Entwickler

Zwei Teile: die **Messung**, die den technischen Weg der Story verworfen hat
(`messung.md`), und die **Prüfnachweise** des daraus folgenden Rückfallstands
— ein Menü für beide Maustasten, „Beenden" abgesetzt als letzte Gruppe
(Kundenentscheidung 02.08.2026 nach Vorlage der Messung).

## Was gemessen wurde und was folgte

Getrennte Menüs tragen unter Plasma/Wayland nicht — Einzelheiten in
`messung.md`. Der Kunde hat daraufhin den Rückfall entschieden und den
Direktstart (Linksklick öffnet ohne Umweg das Erfassungsfenster) ausdrücklich
verworfen; die Entscheidung aus #44 gilt weiter.

Geliefert sind damit die Befunde 2, 3 und 4 des Kunden (Symbole, deutsche
Beschriftungen, Umbenennung) und von Befund 1 der Teil, der ohne zweites Menü
zu haben ist: der Abstand zwischen „Beenden" und dem häufigsten Eintrag.

## Prüfmittel (a) — Strukturtest

`tests/shelltest.cpp`, drei Prüfungen:

- `showsTheEntriesOfTheWireframeWithTheirIcons` — Beschriftung, `QIcon::name()`
  und Zustand jedes Eintrags, Trenner an beiden Stellen, Reihenfolge.
- `keepsQuitApartInTheLastGroup` — „Beenden" ist letzter Eintrag und steht
  hinter einem Trenner, nicht an erster Stelle.
- `hintsTheShortcutWithoutBindingItASecondTime` — der Eintrag trägt Meta+N zur
  Anzeige, und zwar mit `Qt::WidgetShortcut`.

**Ehrlichkeitsvermerk:** `keepsQuitApartInTheLastGroup` war schon vor der
Änderung grün — der alte Menüaufbau hatte „Beenden" ebenfalls hinter einem
Trenner am Ende. Der Test beweist die Änderung also nicht, er hält die
Eigenschaft künftig fest. Die beiden anderen waren rot: die Symbolnamen kamen
leer zurück, das Kürzel fehlte ganz.

Der Test braucht ein auflösbares Symbol-Thema, sonst hat `QIcon::fromTheme()`
keinen Namen zurückzugeben — `tests/CMakeLists.txt` setzt dafür
`QT_QPA_PLATFORMTHEME=kde`, `initTestCase()` heftet das Thema auf `breeze`,
damit der Lauf nicht davon abhängt, welches der Benutzer gerade eingestellt hat.

## Prüfmittel (b) — was der Host wirklich bekommt

`menue-abfrage.sh` fragt am laufenden Dienst
`com.canonical.dbusmenu.GetLayout` ab; das Ergebnis steht in
`menue-getlayout.txt`. Der Strukturtest prüft QAction-Objekte im eigenen
Prozess — erst diese Abfrage zeigt, was über das Tray-Protokoll beim Panel
ankommt:

| Id | `icon-name` | `label` | Zustand |
|---|---|---|---|
| 1 | `document-edit` | Notiz erfassen | aktiv, `shortcut [['Super','N']]` |
| 2 | `audio-input-microphone` | Sprachnotiz aufnehmen | `enabled false` |
| 3 | — | — | `type separator` |
| 4 | `view-list-text` | Bibliothek öffnen | aktiv |
| 5 | `system-run` | Jetzt analysieren | `enabled false` |
| 6 | `tools-wizard` | Vorschläge | `enabled false` |
| 7 | — | — | `type separator` |
| 8 | `application-exit` | Beenden | aktiv |

Kein englisches Wort, jeder Eintrag mit Symbolnamen, beide Trenner da,
„Beenden" zuletzt. Der Kürzel-Hinweis reist mit — `[['Super','N']]` ist die
Schreibweise des Protokolls für Meta+N.

## Prüfmittel (c) — Panel-Foto: **offen**

Das Foto des geöffneten Menüs am echten Panel fehlt und ist **nicht
erbracht**. Es braucht einen echten Mausklick auf das Symbol; das Menü zeichnet
plasmashell, nicht Denkzettel.

Dass kein Agent diesen Klick erzeugen kann, ist gemessen und nicht vermutet:
`xdotool mousemove 3230 2120` lässt den Zeiger stehen, wo er stand (vorher wie
nachher `x:3707 y:34`) — KWin reicht die XTEST-Zeigerbewegung aus XWayland
nicht in den Wayland-Eingabestrom weiter. Werkzeuge, die über `/dev/uinput`
gingen, sind nicht installiert; sie zu installieren ist keine Entscheidung des
Entwicklers.

Was ohne Klick geht, ist geliefert: das Tray-Symbol am Panel
(`tray-symbol-am-panel.png`) und die vollständige Menüstruktur, wie der Host
sie sieht (oben). Wie ein solches Menü mit denselben Einträgen aussieht, zeigt
außerdem `sni-fenster-landet-mittig.png` aus der Messung — dort zeichnet es
Qt, nicht plasmashell, deshalb ersetzt es das Panel-Foto nicht.

**Zustand der KDE-Einstellung „Symbole in Menüs anzeigen": eingeschaltet.**
`ShowIconsInMenuItems` steht nicht in `kdeglobals`, es gilt also die Vorgabe
(`true`); das Bild aus der Messung bestätigt es — dort trägt jeder Eintrag sein
Symbol.

## Meta+N nach der Umbenennung

`kuerzel-nachpruefung.sh`, Ergebnis in `kuerzel-nachpruefung.txt`. Gemessen am
laufenden Dienst aus dem Build-Verzeichnis:

- kglobalacceld kennt die Komponente, sie ist aktiv, und hält
  `show-capture` → `268435534` = `0x1000004E` = Meta+N.
- Der Anzeigename der Aktion ist **„Notiz erfassen"** — die Umbenennung ist
  beim Kurzbefehl-Dienst angekommen.
- **Genau ein** Eintrag für `show-capture`. Der Kürzel-Hinweis im Menü doppelt
  die Registrierung also nicht; gemessen, nicht angenommen.
- `invokeShortcut show-capture` — das, was kglobalacceld beim Tastendruck
  selbst tut — öffnet das Erfassungsfenster
  (`kuerzel-erfassungsfenster.png`). Die Zahl der Notizen bleibt vorher wie
  nachher 4, es wurde also nichts nebenbei gespeichert.
- Das Journal (`journalctl --user -t denkzetteld`) meldet zum Start des neuen
  Dienstes **nichts** — keine Kürzel-Warnung, kein Fehlschlag.

**Grenze, benannt statt beschönigt:** Die Desktop-Datei, die kglobalacceld
auflöst, ist die **installierte** unter `/usr/share/applications`, und dort
steht noch `Name=Capture öffnen`. Die Installation nach `/usr` taktet der PO
(DoD 2, Parallelarbeit), sie war mir untersagt. Was die Umbenennung an der
Desktop-Datei ändert, ist ausschließlich dieser Anzeigename — `Actions=`, der
Gruppenname `[Desktop Action show-capture]` und `Exec=` sind unverändert, und
`Keywords=` bewusst auch. Zu prüfen bleibt im Installationstakt des PO:

```
# nach der Installation, wegen des KService-Zwischenspeichers von kglobalacceld
qdbus6 org.kde.kglobalaccel /kglobalaccel org.kde.KGlobalAccel.unregister \
    org.denkzettel.Denkzettel.desktop show-capture
systemctl --user restart plasma-kglobalaccel   # oder neu anmelden
# danach erneut:
bash docs/scrum/reviews/sprint-04-s33-traymenues/kuerzel-nachpruefung.sh
```

## Dateien

| Datei | Inhalt |
|---|---|
| `messung.md` | die Messung zur Menütrennung — Aufbau, Befunde, Folgerung |
| `sni-trennung-probe.cpp` · `sni-messung.sh` · `sni-messung.txt` | Probe, Messlauf, Protokoll |
| `sni-popup-bleibt-unsichtbar.png` | Popup-Variante: am Symbol steht nichts |
| `sni-fenster-landet-mittig.png` | Gegenprobe: sichtbar, aber in der Bildschirmmitte |
| `menue-abfrage.sh` · `menue-getlayout.txt` | Prüfmittel (b): das exportierte Menü |
| `kuerzel-nachpruefung.sh` · `kuerzel-nachpruefung.txt` | Meta+N-Kette nach der Umbenennung |
| `kuerzel-erfassungsfenster.png` | das Erfassungsfenster, ausgelöst wie durch den Tastendruck |
| `tray-symbol-am-panel.png` | das Symbol des gebauten Dienstes im Systemabschnitt |

Die Vollbilder zeigen den Bildschirm der Kundensitzung. Alles außerhalb der
Belegfläche ist weichgezeichnet; **Lage und Größe der Fenster sind
unverändert**, die Bilder beweisen also weiter, was sie beweisen sollen.
