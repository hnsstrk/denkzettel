# Der Fokuszustand des Textfeldes — Messung, keine Bewertung

**Modus:** Gestaltung (Zuarbeit zur Vorlage an den Kunden). Der Auftrag lautet
ausdrücklich, **nicht** zu bewerten, ob diese Schicht in #100 gehört; das
entscheidet der Kunde mit dem PO. Hier stehen die Zahlen, auf denen diese
Entscheidung ruhen kann.

**Datum:** 07.08.2026, Ganymed · **Quellstand:** `main` @ `88129ba` ·
**Werkzeuge** (B17): ksvg 6.28.0, qt6-base 6.11.1, plasma-desktop 6.7.4.
Acht installierte Desktop-Themes.

**Belege:** `docs/scrum/vorberichte/100-eingabefeld/messungen-ux/ux7` bis `ux11`,
Sonden in `sonden-ux/`, wiederholbar über
`bash docs/scrum/vorberichte/100-eingabefeld/pruefen-ux.sh`.

---

## Was der Fund ist — und was an seiner ersten Fassung falsch war

Gemeldet hatte ich: Die fünf Themes, unter denen `widgets/lineedit`/`base` nur
einen Hauch zeichnet, führen einen Fokuszustand mit voll deckender Kante, und
`default`, `breeze-dark` und `breeze-light` führen ihn unter einem anderen Namen
(`focusframe`). Das legte eine Fallunterscheidung nach Theme nahe.

**Zwei Dinge daran sind gemessen falsch.** Die erste Messung (UX1) hat zwei
Bildpunkte abgetastet — die Mitte und x=1. Unter `default` liegt die Fokuskante
bei **x=0** und war damit unsichtbar; die Flächenmessung UX7 hat es aufgedeckt,
UX8 legt die Geometrie offen.

| Theme | `focus` | `focusframe` | `hint-focus-over-base` |
|---|---|---|---|
| `default`, `breeze-dark`, `breeze-light` | 1 px bei x=0, `#3daee9` | 3 px, 2 px nach außen, `#3dade7` | vorhanden |
| die fünf übrigen | 2 px bei x=1…2, `#1a73e8` | fehlt | vorhanden |

**Alle acht Themes führen `focus`.** `focusframe` kommt unter dreien hinzu.
Plasmas eigener Bau nimmt `focusframe` allein beim **Tastaturfokus** und sonst
`focus`:

```
prefix: control.visualFocus && hasElement("focusframe-center") ? "focusframe" : "focus"
```
— `/usr/lib/qt6/qml/org/kde/plasma/components/TextField.qml`, Zeile 223

Für das Erfassungsfenster ist das der Fall `focus`: Das Fenster geht auf, der
Textbereich hat den Fokus, ohne dass jemand durch eine Fokuskette getabbt wäre.

**Die Zeichnung hat den Punkt am 06.08.2026 bereits angelegt**, ohne die Folge
zu ziehen. Zeichnung 4a trägt dort den Vermerk:

> „Der Satz »Es geht auch nichts verloren: Das Textfeld hat immer den
> Tastaturfokus … Eine Fokus-Umrandung hätte nichts anzuzeigen.« … stimmt für
> die **Fokus**-Umrandung und geht an der Sache vorbei … **und der Vorsatz
> `base` ist der ruhende Zustand des Feldes, nicht der fokussierte.**"

Der letzte Halbsatz ist genau dieser Fund. Gezogen wurde daraus im selben Zug
allein der Vorsatz `base`; dass das Feld damit unter fünf der acht Themes im
ruhenden Zustand unsichtbar bleibt, stand erst mit der Vorprüfung fest.

---

## 1. Was die Schicht am Produktivcode kostet

**Ein Vorsatz, keine Fallunterscheidung.** `focus` liegt unter allen acht Themes
vor (UX7), und `hint-focus-over-base` ebenfalls — die Schicht gehört also unter
allen achten **über** die Grundfläche, ohne Sonderfall in der Z-Ordnung (UX9;
die Frage stellt sich, weil `TextFieldFocus.qml:18–33` sie stellt).

**Die Geometrie bleibt unverändert.** Gemessen beansprucht `focus` einen Rand
von **0/0/0/0**, während `base` 6/6/6/6 nimmt und `focusframe` 2/2/2/2 (UX9).
Die Schicht deckt genau das Rechteck des Feldes; AK 5 (Innenabstände) ist von
ihr nicht betroffen. Nähme der Bau `focusframe` wie Plasma es beim Tastaturfokus
tut, kämen 2 px nach außen hinzu — mit `focus` allein entfällt das.

### Die Zeilen, gemessen am Stand nach #100

| Ort | Was hinzukommt |
|---|---|
| `capturewindow.h:197–202` | ein `KSvg::FrameSvg *m_focus` |
| `capturewindow.cpp:59–69` | eine Konstante für den Vorsatz `focus` |
| `capturewindow.cpp:214` | der Rahmen tritt in die bestehende Schleife über `{m_hull, m_shadowTiles}` (nach #100: samt Feld) |
| `capturewindow.cpp:318` | dieselbe Schleife in `reloadDesktopTheme()` — hier zieht der Theme-Wechsel |
| `capturewindow.cpp:396–417` | in `paintEvent()`: gezeichnet nach dem Feld, unter der Bedingung aus Punkt 4 |
| `capturewindow.cpp:442–455` | in `resizeHull()`: Größe und Bildpunktverhältnis, dieselbe Falle wie bei #83 |
| `capturewindow.cpp:424–440` | ein Zweig `QEvent::ActivationChange` in `CaptureWindow::event()` |

**Kein neuer Ereignispfad.** `CaptureWindow::event()` ist bereits überschrieben
(für `DevicePixelRatioChange`); der Aktivierungszweig kommt dort hinein.
**Keine neue Bibliothek, kein neues Ziel, keine neue Fixture** — es ist eine
dritte `KSvg::FrameSvg` auf der `ImageSet`, die seit #83 da ist.

**Eine Wache wird gebraucht.** `staysUsableWithoutADesktopTheme()`
(`capturetest.cpp:1149`) läuft in einem eigenen Prozess ohne Theme-Pfad und
verlangt, dass kein Bildpunkt transparent bleibt. Der dritte Rahmen braucht
dieselbe `isValid()`-Wache wie die Hülle (`paintEvent`, `capturewindow.cpp:400`).
Für das Feld ist dieser Punkt schon gemeldet (Vorprüfung B, F6); mit der Schicht
gilt er ein drittes Mal.

---

## 2. Was der Kunde gewinnt

Gerechnet wie in der Entscheidung vom 07.08.2026: Hülle und Feld geschichtet
über einem benannten Grund, je einmal über Schwarz und über Weiß, weil beide
Grafiken durchscheinen können. Gemessen ist die stärkste Abhebung, die der
Feldrand gegen die Hülle daneben erreicht — die Größe, die der Kundenbefund aus
#100 meint. **Maßstab: KRunners Feld hebt sich im Sitzungsbild von Sprint 7 mit
1,41 : 1 ab** (SPEC 3.1).

| Theme | Grund | ohne Schicht | mit Schicht | Faktor |
|---|---|---|---|---|
| `CachyOS-Nord-round` | schwarz | 1,03 | **3,50** | 3,41 |
| `CachyOS-Nord-round` | weiß | 1,03 | **3,50** | 3,41 |
| `Iridescent-round` | schwarz | 1,00 | **4,66** | 4,66 |
| `Iridescent-round` | weiß | 1,13 | **2,81** | 2,48 |
| `cachyos-emerald` | schwarz | 1,00 | **4,66** | 4,66 |
| `cachyos-emerald` | weiß | 1,14 | **4,24** | 3,71 |
| `cachyos-emerald-color` | schwarz | 1,00 | **4,66** | 4,66 |
| `cachyos-emerald-color` | weiß | 1,14 | **4,24** | 3,71 |
| `cachyos-emerald-light` | schwarz | 1,00 | **4,66** | 4,66 |
| `cachyos-emerald-light` | weiß | 1,14 | **4,24** | 3,71 |
| `default` | schwarz | 2,01 | 6,73 | 3,35 |
| `default` | weiß | 1,88 | 3,88 | 2,07 |
| `breeze-dark` | schwarz | 2,01 | 6,73 | 3,35 |
| `breeze-dark` | weiß | 1,88 | 3,88 | 2,07 |
| `breeze-light` | schwarz | 1,62 | 1,62 | **1,00** |
| `breeze-light` | weiß | 1,50 | 2,23 | 1,48 |

**Der Kern für die fünf schwachen Themes:** Die Abhebung steigt von 1,00–1,14
auf **2,81–4,66**. Ohne die Schicht liegt sie unter jeder Wahrnehmungsschwelle
und weit unter KRunners 1,41; mit ihr liegt sie in allen zehn gemessenen
Kombinationen darüber. Dies ist der Punkt, an dem der Kundenbefund aus #100
unter diesen Themes überhaupt erst geheilt wird.

**Der eine Fall ohne Gewinn:** `breeze-light` über schwarzem Grund. Dort hebt
die Feldfläche sich bereits mit 1,62 : 1 ab, und die Fokuskante erreicht gegen
dieselbe Hülle 1,54 : 1 — sie fügt der ohnehin sichtbaren Fläche nichts hinzu.
Der Wert verschlechtert sich nicht; er bleibt stehen.

---

## 3. Welche Prüfsätze brechen

Gemessen an den Abtastpunkten der bestehenden Zusicherungen.

| Prüfsatz | Bricht durch die Schicht? | Befund |
|---|---|---|
| `hullIsCompleteAtFiveAndEightLines()` (`:815`) | **nein, zusätzlich** | Von den fünf Abtastpunkten liegt nur die Fenstermitte im Feld; die vier anderen sitzen auf den Fensterkanten, weit außerhalb des Feldrechtecks (28,40)–(572,130). Die Mitte ist **bereits durch `base`** betroffen (Vorprüfung A, F6). Die Schicht fügt dort unter `default` und Breeze **0** hinzu und unter den fünf übrigen 15 von 255 (UX1). |
| `takesTheOpaqueVariantWithoutABlurringCompositor()` (`:1020`) | **nein, zusätzlich** | Tastet dieselbe Fenstermitte ab und hält sie gegen `themeHull(...)`, also gegen eine Hülle ohne jedes Feld. Gebrochen wird er von `base`; die Schicht verschiebt die Zahl unter den fünf schwachen Themes um dieselben 15. |
| `paintsTheThemesOwnHullInOnePiece()` (`:551`) | **nein** | Fällt planmäßig schon mit #100 (Vorprüfung A, F5) — er sichert die durchgehende Fläche zu, die der Kunde aufgehoben hat. |
| `staysUsableWithoutADesktopTheme()` (`:1149`) | **ja, wenn die Wache fehlt** | Siehe Punkt 1. Mit `isValid()`-Wache bleibt er grün. |

**Zusammengefasst: Die Schicht bricht keinen Prüfsatz, den #100 nicht ohnehin
bricht.** Sie verschiebt unter fünf Themes eine Deckungszahl in der
Fenstermitte um 15 von 255 — in denselben zwei Zusicherungen, die wegen `base`
schon umgebaut werden müssen.

**Neu zu schreiben sind vier Nachweise**: die Schicht wird gezeichnet, wenn das
Fenster aktiv ist; sie wird nicht gezeichnet, wenn es das nicht ist; sie folgt
dem Theme-Wechsel (dieselbe Schleife wie F4); sie folgt der Skalierung
(dieselbe Falle wie #83). Dazu die Bildreihe `captureshots.cpp:186–204`, die ab
dann drei Schichten zeigt.

---

## 4. Der Zustand, in dem die Schicht nichts zeichnen darf

**Der Auftrag geht von „das Fenster schließt bei Fokusverlust" aus. SPEC 3 legt
das Gegenteil fest:**

> Esc: verwerfen, Fenster verstecken. **Fokusverlust: Fenster bleibt** (kein
> Datenverlust durch versehentlichen Klick daneben).
> — `SPEC.md:174–176`

Der Zustand „Fenster offen, Fenster nicht aktiv" ist also gewollt und tritt bei
jedem Klick daneben ein. Eine Schicht, die allein am Widget-Fokus hängt, zeigte
darin einen Fokusrahmen an einem Fenster, das der Compositor gerade nicht
bedient. Der Code kennt heute **keinen** Aktivierungszweig: `capturewindow.cpp`
enthält kein `ActivationChange`, kein `WindowDeactivate` und keine Abfrage von
`isActiveWindow()`; die einzige Fokuszeile ist `setFocusProxy(m_text)` (`:238`).
Der Textbereich ist zudem das **einzige** fokussierbare Kind — die beiden
Beschriftungen sind `QLabel` (`:242`, `:248`).

**Gemessen ist der Zustand offscreen erzeugbar** (UX11): Nach `show()` und
`setFocus()` meldet das Fenster `isActiveWindow() == true`; sobald ein zweites
Fenster aufgeht und aktiviert wird, wechselt es auf `false`, ein
`ActivationChange` trifft ein, und `hasFocus()` des Textbereichs wird ebenfalls
`false`.

**Zwei Folgen, die auseinanderzuhalten sind.** Offscreen genügte demnach schon
`hasFocus()`. Ob das unter Wayland ebenso ausfällt, sagt diese Messung nicht —
offscreen fehlt der Compositor, und der entscheidet über die Aktivierung
(B21). Der Weg, den Plasma selbst geht, deckt beide Fälle ab: `activeFocus`
entspricht in Qt Widgets `hasFocus() && isActiveWindow()`.

**Für die Belegform heißt das:** Der Zustand ist offscreen prüfbar, und ein
Sitzungsbild gehört daneben, weil die Aussage über die Fensteraktivierung vom
Compositor abhängt. Der Fensterwechsel im Sitzungsbild nimmt den
compositor-getriebenen Weg — das obenauf liegende Fenster **schließen** statt
den Fokus zurückzuholen (CLAUDE.md, Sprint 6 §16.1 M-B1).

---

## 5. Die beiden Größenklassen

Nach der Hausdefinition (`docs/scrum/PROZESS.md:275–278`).

### Die Schicht allein: **`size:m`**

*„Trägt einen Strang aus."*

**Wofür nicht `s`:** `size:s` verlangt „wenige Dateien, **kein neuer
Prüfweg**". Der Prüfweg ist neu: Der Zustand „Fenster offen, aber nicht aktiv"
kommt in keinem der heutigen Prüfsätze vor, er braucht ein zweites Fenster im
Testaufbau, und er verlangt nach B21 ein Sitzungsbild neben dem
Offscreen-Nachweis. Dazu vier neue Nachweise und die Bildreihe.

**Wofür nicht `l`:** Gemessen kommen rund ein Dutzend Zeilen Produktivcode
hinzu, verteilt auf sieben Stellen einer Datei, die #100 ohnehin öffnet. Kein
neuer Ereignispfad (`event()` steht), keine Fallunterscheidung (ein Vorsatz
genügt), keine Geometrieänderung (Rand 0/0/0/0), keine neue Bibliothek, kein
neues Ziel, keine neue Fixture.

**Was die Klasse senken würde:** Bliebe die Aktivierungsbedingung weg — die
Schicht also immer gezeichnet, solange der Textbereich den Fokus hat —, entfiele
der neue Prüfweg und die Klasse wäre `size:s`. Das widerspräche SPEC 3 an der
oben zitierten Stelle; die Abwägung gehört dem Kunden und dem PO, die Zahl steht
hier, damit sie mit ihr rechnen können.

### #100 mit der Schicht: **`size:l`**

*„Füllt den Sprint — daneben passt nur `size:s`."*

#100 ist von zwei unabhängigen Bearbeitern auf `size:m` bemessen worden. Der
Wegfall von AK 3 senkt die Klasse nicht — Messung A hat ausdrücklich gemessen,
dass AK 3 sie nicht hob (zwei Ausdrücke, vier Testfunktionen in derselben
Datei). Was hinzukommt, ist ein zweiter `m`-Strang:

- **drei** Zeichenschichten statt einer, zwei davon mit eigener Zustandslogik;
- ein Ereigniszweig, den es heute nicht gibt;
- ein Prüfzustand, den das Projekt noch nie geprüft hat, samt einer Belegform,
  die es noch nie geführt hat (Fenster inaktiv im Sitzungsbild);
- rund neun neue Nachweise zusammen, dazu zwei brechende und vier wandernde
  Zusicherungen und die Nachziehearbeit an SPEC 3.1.

Zwei `m`-Stränge in einer Story sind kein Strang mehr. Die Skala kennt zwischen
`m` und `l` nichts, und die Regel „neben `size:l` steht nur `size:s`" ist genau
für diesen Fall gemacht.

**Die Gegenrede dazu:** Zum Vergleich brachte #83 als `size:l` KSvg, den
Schatten, zwei Effektanmeldungen und die Theme-Wache überhaupt erst ins Haus.
Daran gemessen ist #100 mit der Schicht das kleinere `l`. Wer die Klasse an der
Zahl neuer Mechanismen festmacht, kommt auf `m`; wer sie an Dateimenge,
Nachweisen und Belegformen festmacht — und das tut die Hausdefinition —, kommt
auf `l`. Ich halte `l` für richtig und benenne die Unsicherheit, weil an dieser
Klasse eine Sprint-Grenze hängt.

---

## Was hier nicht gemessen ist

- **Ob die Schicht in #100 gehört.** Auftragsgemäß nicht bewertet.
- **Wie die Kante in der angemeldeten Sitzung aussieht.** Alle Zahlen oben sind
  offscreen erhoben. Sie belegen Geometrie und Farbe der Theme-Grafik; über
  Hülle, Rundung, Kontur und Schatten sagen sie nichts (B21). Die Abhebung
  gegen einen realen Bildschirmhintergrund liegt zwischen den beiden Spalten
  „schwarz" und „weiß".
- **Der Tastaturfokus-Fall.** `focusframe` ist vermessen (UX8, UX9) und in
  diesem Bau nicht einschlägig, weil das Fenster seinen Fokus beim Aufgehen
  bekommt. Käme später eine Fokuskette hinzu, wäre die Fallunterscheidung aus
  `TextField.qml:223` nachzuholen.
- **Ob `hover` eine Rolle spielt.** Gemessen zeichnet `hover` unter allen acht
  Themes dieselbe Geometrie wie `focus` — unter `default` und den beiden
  Breeze-Themes in derselben Farbe (`#3daee9`), unter den fünf übrigen in einer
  helleren (`#8ab4f8` gegen `#1a73e8`); UX1, UX7, UX8.
  Ein Erfassungsfenster, dessen Textbereich immer den Fokus hat, käme in diesen
  Zustand nur beim Überfahren mit der Maus. Nicht weiter verfolgt.
