# Belege zur Untersuchung der Kundenbefunde, Sprint-6-Abnahme (#55)

**Datum:** 2026-08-04, Ganymed · **Quellstand:** `main` @ `a01afd5`
(Produktivcode identisch mit dem installierten Stand `977e804`; dazwischen
liegen nur Dokumente) · **Bericht:** `bericht.md`

Hier liegen die Messungen, auf die sich `bericht.md` beruft: achtzehn Protokolle,
die Sonden, die sie erzeugen, ein Prüf-Theme und ein Skript, das alles
wiederholbar macht.

## Wiederholen

```
bash docs/scrum/reviews/2026-08-04-abnahme-befunde/pruefen.sh
```

Das Skript baut in einen **eigenen** Bauplatz (`build/` neben dieser Datei, von
`.gitignore` gedeckt), fasst weder `build/` der Repositoriumswurzel noch `/usr`
an und schreibt keine Einstellung des Kunden: Was ein Farbschema oder ein
Desktop-Theme braucht, bekommt ein eigenes `XDG_CONFIG_HOME` bzw.
`XDG_DATA_DIRS` unter `/tmp`.

**Zwei Messungen laufen in der angemeldeten Sitzung** (`b1-echtelage*`) und
zeigen dabei für gut eine Sekunde ein Fenster. Sie nehmen den Bildschirm
**nicht** auf — `QWidget::grab()` zeichnet allein das Fenster, vom Schreibtisch
des Kunden kommt kein Bildpunkt mit. Ohne Wayland-Sitzung werden sie
übersprungen und das Skript sagt es.

## Was jede Messung zeigt

| Datei | Befund in einem Satz |
|---|---|
| `b1-huellenring-offscreen.txt` | **`tinted()` liefert unter `offscreen` eine Pixmap ohne Alphakanal** (Format 4) — auf dem Bogen fehlt die Kontur und zwischen Kontur und Fläche steht ein durchsichtiges Loch. |
| `b1-huellenring-live.txt` | **Derselbe Binärcode unter `wayland` liefert Format 6 mit Alphakanal** — Bogen und Kontur vollständig. Der einzige Unterschied der beiden Läufe ist die Plattform. |
| `b1-eckenraster-skala-1.txt`, `-1.6.txt`, `-2.txt` | **`alphaMask()` bleibt 600×150 bei DPR 1**, gleich welche Skalierung gilt; die Fensterfläche wächst auf 960×278 bzw. 1200×348. Die Maske wird also hochskaliert. |
| `b1-echtelage.txt` | **In der Sitzung des Kunden: Fenster-DPR 1,6, `grab()` 960×278.** Die Kontur läuft in **drei Stufen** um die Ecke (Spalte 6 → 2 → 0). |
| `b1-huellenring-*.txt`, Abschnitt e | **Mit `setDevicePixelRatio(1,6)` liefert KSvg die Maske als 960×279** mit kantengeglättetem Bogen — ein Schritt je Zeile statt drei Stufen. |
| `b1-eckiges-theme-skala-1.txt`, `-1.6.txt`, `b1-echtelage-eckiges-theme.txt` | **Ein Desktop-Theme mit rechteckigen Eckstücken ergibt eine rechteckige Hülle** — offscreen wie in der laufenden Sitzung. Denkzettel rundet nichts von sich aus. |
| `b1-eckhelligkeit.txt` | **Außen an Denkzettels Bogen steht ein 2 Bildpunkte breiter Streifen, der heller ist als der Grund weit draußen** (+1 bis +3). An der nativen Ecke gibt es ihn nicht. |
| `b2-schattenherkunft.txt` | **Die Schattenkacheln stammen aus `/usr/share/plasma/desktoptheme/default/dialogs/background.svgz`**, Ausmaß 10 logische Pixel, Höchstdeckung 28/255 ≈ 11 %. |
| `b2-schattenprofil.txt` | **Gleiche Stärke an der Kante (10 % gegen 11 %), sehr verschiedene Reichweite**: Denkzettels Verlauf ist nach 8 Bildpunkten fertig, der native läuft über 21 hinaus. |
| `b3-flaechenfarbe-nord.txt` | **Fläche, Anwendungsname und Fußzeile treffen die Palettenfarbe exakt**; der Notiztext trägt `WindowText`; die Schreibmarke ist die Umkehrung der Fläche. |

## Nachtrag: der native Weg (Kundenentscheidung 04.08.2026)

| Datei | Befund in einem Satz |
|---|---|
| `native-huelle-nord.txt`, `native-huelle-breeze.txt` | **`framePixmap()` hat null Stufen, `alphaMask()` eine** — die Rahmenelemente des Themes sind feiner als seine `mask-`Elemente. Und: die Hülle deckt nur zu **84,7 %**. |
| `native-huelle-nord-wayland.txt` | **Derselbe Lauf unter Wayland liefert Zahl für Zahl dasselbe** — der native Weg ist plattformunabhängig, der heutige nicht. |
| `native-huelle-eckiges-theme.txt` | **Eckiges Theme, eckige Hülle**, null Stufen bei Verhältnis 1 und 1,6. |
| `native-ak2-kontrast.txt` | **17 von 20 Schemata treffen `Window` exakt**; deckend gerechnet 4,74:1 im schlechtesten Fall (die Zahl von AK 2), durchscheinend **3,57:1** — fünf Schemata unter 4,5:1. |
| `b2-dekorationsquelle.txt` | **Die Fensterdekoration kennt das Desktop-Theme nicht** — keine der sechs Zeichenketten, weder KSvg noch libplasma gebunden. Zwei getrennte Schattenwege ohne gemeinsame Quelle. |
| `b2-schattenherkunft.txt`, Abschnitt Polsterung | **Der Schatten ragt 6 Bildpunkte ins Fensterrechteck hinein** (Polsterung 10 gegen Kachel 16). Das widerlegt die erste Ursachenvermutung zu F3. |

Bilder dazu in `bilder/native/` — `native-ecke-dpr-1.6.png` gegen
`bilder/b1/echt/echte-ecke.png` (heutiger Stand am laufenden Fenster), und
`native-huelle-dpr-1.png` auf der Schraffur der Zeichnung 4a, an der die
Durchsichtigkeit abzulesen ist.

## Das Prüf-Theme

**Nicht mehr hier:** Das Theme ist am 05.08.2026 mit #83 AK 9 nach
`tests/themes/plasma/desktoptheme/denkzettel-pruef-eckig/` überführt worden,
wo `themes::addBundledThemesToDataPath()` es findet. Der Absatz beschreibt es
weiter, `pruefen.sh` dieses Ordners zeigt auf den neuen Platz.

`pruef-theme/plasma/desktoptheme/denkzettel-pruef-eckig/` ist Zeichen für
Zeichen der Aufbau von `tests/themes/.../denkzettel-test-schmal`, nur sind die
vier Eckstücke rechteckig statt Viertelkreise. Es existiert, weil **alle acht
auf der Maschine installierten Desktop-Themes runden** — die Frage des Kunden,
ob Denkzettel „stumpf alles abrundet", war an installierten Themes nicht zu
beantworten. Kein Gestaltungsvorschlag, ein Messmittel.

## Die Bilder

`bilder/b1/live/` und `bilder/b1/offscreen/` zeigen denselben Ring auf
denselben zwei Plattformen, zwanzigfach und ungeglättet vergrößert —
`ring-ist.png` gegen `ring-gegenprobe-alphakanal.png`. `bilder/b1/echt/` und
`bilder/b1/echt-eckig/` stammen aus der angemeldeten Sitzung bei DPR 1,6.
`bilder/b3/flaechenfarbe.png` zeigt das Fenster unter dem Farbschema des Kunden.

Die Aufnahmen des Kunden liegen daneben in `kundenbilder/` mit eigenem
LIESMICH; die Sonden `eckhelligkeit.py` und `schattenprofil.py` messen an
diesen Originalen.

---

**Nachtrag 05.08.2026 — das Prüf-Theme ist umgezogen.** `pruef-theme/` lag
hier und liegt seit #83 unter `tests/themes/plasma/desktoptheme/denkzettel-pruef-eckig/`
— dort sucht der Testaufbau es (`#83` AK 9). Die Belege dieses Ordners bleiben,
wie sie waren; nur der Pfad, den sie nennen, ist ein anderer.
