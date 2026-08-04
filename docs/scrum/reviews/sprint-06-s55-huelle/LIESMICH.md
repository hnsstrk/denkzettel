# Belege Sprint 6, Strang A — Fensterhülle (#55) und Feldhöhe (#56)

**Datum:** 2026-08-04, Ganymed · **Zweig:** `story/55-fensterhuelle` ·
**Quellstand:** `main` @ `0a229d2`

Hier liegen die Belege, auf die sich `bericht.md` beruft: sechs Messprotokolle,
fünfzehn Bilder und ein Skript, das die Messungen und die Bilder wiederholbar
macht. Ein unversionierter Beleg ist kein Beleg (B7) — und ein Beleg, den
niemand nachfahren kann, ist einer, den niemand widerlegen kann.

## Wiederholen

```
bash docs/scrum/reviews/sprint-06-s55-huelle/pruefen.sh
```

Das Skript übersetzt die Sonden in einen **eigenen** Bauplatz (`build/` neben
dieser Datei, von `.gitignore` gedeckt), fährt die Messungen 1, 2, 3, 4 und 6,
schreibt die Protokolle neben diese Datei und erzeugt die Bilder 01–14 aus
einem **frisch gebauten** Läufer. Es fasst weder das `build/` der
Repositoriumswurzel an — dort arbeiten unter Umständen andere Agenten — noch
irgendetwas unter `/usr`. Messung 1 und 2 lesen und schreiben `plasmarc`; beide
laufen unter einem privaten `XDG_CONFIG_HOME`, das eingestellte Desktop-Theme
des Kunden wird weder gelesen noch verstellt.

**Messung 5 läuft nicht mit.** Sie braucht eine angemeldete Wayland-Sitzung und
zeigt für ein paar Sekunden ein Fenster auf dem Bildschirm:

```
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug && cmake --build build
cmake -B docs/scrum/reviews/sprint-06-s55-huelle/build/messung5 \
      -S docs/scrum/reviews/sprint-06-s55-huelle/messungen \
      -DDENKZETTEL_LIB_DIR="$PWD/build/lib"
cmake --build docs/scrum/reviews/sprint-06-s55-huelle/build/messung5
./docs/scrum/reviews/sprint-06-s55-huelle/build/messung5/schatten-am-compositor
```

## Was jede Messung zeigt

| | Datei | Befund in einem Satz |
|---|---|---|
| 1 | `ksvg-themequelle.txt` | **KSvg liest `plasmarc` nicht** — `default` ist sein Rückfallwert, nicht der eingestellte Name. |
| 2 | `themewechsel-zustellung.txt` | **`KDirWatch` sieht jeden Schreibvorgang, `KConfigWatcher` nur den mit `KConfig::Notify`.** |
| 3 | `framesvg-nachziehen.txt` | **Nur ein frisches `ImageSet` zieht einen `FrameSvg` nach** — umbenennen, Pfad neu setzen und dasselbe Set erneut zuweisen wirken alle drei nicht. |
| 4 | `theme-eckstuecke.txt` | **Alle acht installierten Themes bringen Eckstücke und Schattenkacheln** (K6); die Eckform unterscheidet sich (6 gegen 7) und ist nicht aus dem Randmaß ableitbar. |
| 5 | `schatten-am-compositor.txt` | **`KWindowShadow::create()` liefert am laufenden Plasma wahr — nach dem ersten *und* nach dem zweiten Zeigen.** |
| 6 | `schattenkacheln.txt` | **`Svg::pixmap(id)` ignoriert die Element-Kennung** und liefert das ganze Bild; `image(elementSize(id), id)` liefert das Element. |

Messung 3 und 6 sind aus **Fehlern dieses Strangs** entstanden, nicht aus
Vorsicht: Der erste Bau schrieb unter zwei Desktop-Themes byteweise identische
Bilder (3), und er übergab dem Compositor achtmal den kompletten Schatten statt
acht Kacheln (6). Beide Fehler sahen richtig aus, und im zweiten Fall war die
Zusicherung grün, weil sie gegen denselben falschen Aufruf verglich.

## Die Bilder

`bilder/01` bis `bilder/14` entstehen offscreen aus `tests/captureshots.cpp`,
unter `QT_QPA_PLATFORMTHEME=kde`. **Welche zwei Desktop-Themes eine Reihe
zeigt, steht in `bilder/themes.txt`** — der Läufer sucht sie zur Laufzeit und
nimmt bevorzugt installierte; nur wo keine zwei mit verschiedenem Rand liegen,
weicht er auf die Prüf-Themes der Testsuite aus und sagt es. Am Bild ist das
nicht abzulesen, deshalb steht es daneben.

- **01–12** — drei Zustände (leer · getippt · acht Zeilen mit Scrollbalken) ×
  zwei Farbschemata (hell · dunkel) × zwei Desktop-Themes (`schmal` = 4 px
  Rand, `breit` = 8 px). Das ist die Bildpflicht aus #55, AK 7.
- **13–14** — die Feldhöhe bei kleiner und großer Schrift (#56). Sie liegen im
  selben Läufer, weil #55 ihn stellt und #56 ihn mitbenutzt (Planning 2.3).

Jedes Bild sitzt auf einer schraffierten Unterlage, wie die Zeichnung 4a sie
zeichnet. Das ist kein Schmuck: Ein blanker `grab()` hat durchsichtige Ecken,
und Durchsichtigkeit sieht im Betrachter aus wie das, was er dahinterlegt —
meist Weiß, auf dem eine runde weiße Ecke unsichtbar ist. Die Rundung ist der
ganze Punkt von AK 1, also bringt das Bild seinen eigenen Grund mit.

`bilder/15-schatten-am-plasma.png` stammt aus der laufenden Sitzung (Messung 5),
aufgenommen mit `spectacle -b -n -a`. **Der Schatten ist darauf nicht zu
sehen** — `-a` schneidet auf das Fensterrechteck, und der Schatten liegt
außerhalb davon. Ein Bild, das ihn zeigte, müsste einen Bereich des Desktops
mitnehmen, und dieses Repository ist öffentlich. Der Schattennachweis ist
deshalb der Rückgabewert in Messung 5, nicht dieses Bild; das Bild belegt Hülle
und Farben am **echten** Compositor statt offscreen.
