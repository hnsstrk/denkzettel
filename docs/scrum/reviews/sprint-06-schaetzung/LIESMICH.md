# Schätzbelege Sprint 6 — vier Messungen zu #56 und #68

**Datum:** 2026-08-02, Ganymed · **Anlass:** Dev-Schätzung für das
Sprint-6-Planning (`docs/scrum/sprints/sprint-06.md`, K2) · **Quellstand:**
`main` @ `59eceb3`

Diese Messungen tragen zwei Zahlen: **#68 = 5 SP** und die Bestätigung von
**#56 = 1 SP**. Sie liegen hier, weil ein unversionierter Beleg kein Beleg ist
(B7) — und weil die dritte von ihnen dem nächsten Strang einen Fehlanlauf
erspart.

Alles ist wiederholbar:

```
bash docs/scrum/reviews/sprint-06-schaetzung/pruefen.sh
```

Das Skript übersetzt die Projektbibliotheken in einen **eigenen** Bauplatz
(`build/` neben dieser Datei, von `.gitignore` gedeckt), baut die vier Sonden,
fährt sie und schreibt die vier `*.txt` neben die Quellen neu. Es fasst weder
`build/` in der Repositoriumswurzel an — dort arbeiten unter Umständen andere
Agenten — noch irgendetwas unter `/usr`.

---

## Was jede Messung zeigt

### Messung 1 · `schriftalterung.cpp` → `schriftalterung.txt`

**`QFontDatabase::systemFont()` altert nicht mit.** Nach einer Verdopplung der
Anwendungsschrift steht sie auf beiden Rängen unverändert bei 10 pt und 8 pt.
Ein Label mit ausdrücklich gesetzter Schrift bleibt bei 15 px, ein erbendes
wächst von 18 auf 37 px.

**Wozu das dient:** Es widerlegt die naheliegende Lesart von #68, die Heilung
sei „`KConfigWatcher` anklemmen und `qApp->setFont()` rufen". Wäre sie es, wäre
#68 eine 3. Sie ist es nicht: Alle drei Fundstellen von
`systemFont(SmallestReadableFont)` — `src/capture/capturewindow.cpp:30`,
`src/ui/librarywindow.cpp:110`, `src/ui/notelistdelegate.cpp:32` — müssen ihre
Quelle wechseln, und die 12 Labels mit ausdrücklich gesetzter Schrift müssen
einzeln nachgezogen werden. 8 davon werden inline erzeugt und nirgends gehalten
(`librarywindow.cpp:154` dreifach instanziiert, `:432`, `:435`, `:455`;
`capturewindow.cpp:60`, `:63`), sind also nicht ohne Weiteres wiederzufinden.

### Messung 2 · `kdeglobals-watcher.cpp` → `kdeglobals-watcher.txt`

**Der Watcher trägt, die Schrift folgt trotzdem nicht.** `KConfigWatcher`
feuert sofort, nennt den geänderten Schlüssel und liefert den neuen Wert
(`readEntry(font)=20`). `QApplication::font()` und
`QFontDatabase::systemFont()` bleiben über sechs Sekunden und zwei
Umstellungen stehen.

**Wozu das dient:** Das ist die unabhängige Bestätigung von **B6** des
Theme-Berichts (`docs/scrum/reviews/2026-08-01-capture-theme-treue.md`) und
zugleich die Aufteilung von #68 in eine billige Hälfte (der Watcher) und eine
teure (jede Stelle, die von Hand nachgefüttert werden muss).

> **Beim Lesen des Protokolls beachten:** Im ersten Watcher-Signal steht
> `readEntry(smallestReadableFont)=10`. Das ist **kein** gespeicherter Wert,
> sondern der mitgegebene Vorgabewert `QFont()` — der Schlüssel steht zu diesem
> Zeitpunkt noch nicht in der Datei (er steht auch in der `kdeglobals` des
> Kunden nicht). Erst das zweite Signal zeigt mit 16 den geschriebenen Wert.

### Messung 3 · `capture-schriftwechsel.cpp` → `capture-schriftwechsel.txt`

Zwei Aussagen am echten `CaptureWindow`:

**A — der Befund von #56 trägt unverändert.** Die Feldhöhe steht bei 98 px,
während der Zeilenabstand von 18 über 28 und 43 auf 59 px wächst: 5,44 → 3,50 →
2,28 → 1,66 Zeilen. Ein frisch gebautes Fenster bei derselben Schrift hätte
148 px. Das sind die drei Zeilen, die der Kunde bei der Sprint-1-Abnahme
zurückgewiesen hat.

**B — wo die Heilung hingehört.** Eine Schriftänderung kann auf drei Wegen
ankommen, und die beiden Kandidatenstellen sehen unterschiedlich viel davon:

| Weg | `FontChange` an `CaptureWindow` | an `QPlainTextEdit` |
|---|---|---|
| A `qApp->setFont()` | 1 | 1 |
| B `window.setFont()` | 1 | 1 |
| C `field.setFont()` | **0** | 1 |

Weg C ist der, den **AK 2 von #56 dem Test vorschreibt** („Der Test setzt die
Schrift des Widgets direkt"). Der `eventFilter`, der ohnehin auf `m_text` liegt
(`capturewindow.cpp:52`), sieht alle drei Wege; ein `changeEvent` am Fenster
sieht Weg C nicht.

**Wozu das dient:** Wer die Heilung ins `changeEvent` des Fensters legt und den
Test nach AK 2 schreibt, bekommt einen roten Test, der nichts über die Sache
sagt — genau die Bauart, gegen die `CLAUDE.md` seine Prüfhaltung fasst. **Der
Punkt gehört in den Spawn-Auftrag.**

### Messung 4 · `bibliothek-zeilenhoehe.cpp` → `bibliothek-zeilenhoehe.txt`

**Der unsichtbare Teil von #68.** Am echten `NoteListDelegate` gemessen:

| | Gruppenkopf | Eintrag |
|---|---|---|
| Start | 27 px | 72 px |
| Anwendungsschrift 2× | **27 px** | 110 px |
| nach `doItemsLayout()` | **27 px** | 110 px |

Der Gruppenkopf steht ganz still — er wird vollständig aus `groupHeadFont()`
gerechnet, also aus `systemFont()`. Der Eintrag wächst nur um seine beiden
Textzeilen; die Zeitstempelzeile darin steuert vorher wie nachher dieselben
15 px bei. Eine Bibliothek nach einem Schriftwechsel zeigt damit **alte
Überschriften über gewachsenen Notizen**.

**Wozu das dient:** Es ist der Beleg, dass #68 nicht capture-only ist und dass
sein Bibliotheksteil eine sichtbare Verschlechterung mitliefert, wenn er halb
gebaut wird. Das Relayout heilt es nicht — die Ursache ist die Schriftquelle,
nicht das Layout.

**Gemessen am echten Delegate, nicht an einer nachgebauten Formel.** Eine erste
Fassung dieser Messung baute die Höhenformel im Prüfprogramm nach; sie wurde
verworfen und durch den Aufruf von `NoteListDelegate::sizeHint()` ersetzt. Ein
Prüfaufbau, in dem der Fehler gar nicht auftreten kann, ist kein Test.

---

## Der Sandkasten für Messung 2 — die Bauart zum Übernehmen

Messung 2 muss `kdeglobals` **schreiben**, um den Watcher auszulösen. Ohne
Vorkehrung verstellte der Lauf die Systemschrift des Kunden. Der Aufbau steht
in `pruefen.sh` und besteht aus zwei Teilen:

1. **Eigener Sitzungsbus.** `dbus-run-session -- bash …` startet einen frischen
   D-Bus für den Lauf. Die Benachrichtigung von `kwriteconfig6 --notify` und
   der `KConfigWatcher` finden sich darauf; der Bus der Kundensitzung sieht
   nichts davon.
2. **Privates `XDG_CONFIG_HOME`.** Die `kdeglobals` des Kunden wird in einen
   Ordner unter `build/sandkasten/` **kopiert**, und `XDG_CONFIG_HOME` zeigt
   dorthin. `kwriteconfig6` schreibt dadurch in die Kopie. Gemessen wird gegen
   die echten Startwerte, geschrieben wird ausschließlich in die Kopie.

Beides zusammen ist die Bauart, die der Theme-Bericht am 01.08.2026 schon für
die verschachtelte Plasma-Sitzung gewählt hat. **Wer künftig etwas misst, das
Systemeinstellungen schreibt, übernimmt sie.**

Gegenprobe nach dem Lauf: `~/.config/kdeglobals` trägt unverändert den
Zeitstempel von vor dem Lauf.

## Umgebung

Beide Umgebungsvariablen setzt `pruefen.sh` selbst:

- `QT_QPA_PLATFORM=offscreen` — kein Compositor nötig.
- `QT_QPA_PLATFORMTHEME=kde` — **nicht verzichtbar.** Ohne das Plattformthema
  misst man eine Ersatzschrift statt der Systemschrift, und sämtliche
  Größenverhältnisse dieser vier Protokolle stimmen nicht mehr (`CLAUDE.md`).

## Grenzen dieser Belege

- Gemessen ist der **Ist-Zustand**, nicht die Heilung. Dass ein
  `KConfigWatcher` die Fenster am Ende wirklich nachzieht, ist damit **nicht**
  gezeigt — das ist der Gegenstand von #68 und am laufenden Plasma zu belegen.
- Messung 2 zeigt den Watcher in einer **eigenen** Sitzung. Dass er in der
  Sitzung des Kunden ebenso feuert, ist wahrscheinlich, aber hier nicht
  gemessen.
- Die vier Programme sind **Sonden, keine Tests**: Sie schreiben Protokolle,
  die ein Mensch liest, und färben keine Suite rot. Sie hängen deshalb
  bewusst nicht am Projektbau.
