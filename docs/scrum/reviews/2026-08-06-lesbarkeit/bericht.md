# Trennung in der Bibliothek, Kante im Erfassungsfenster

**Modus:** Gestaltung (Auftrag des PO vom 06.08.2026, zu den Kundenbefunden
#100 und #101). Kein UI-Review, kein Vorprüfbericht, keine Akzeptanzkriterien.

**Was hier liegt**

| Datei | Inhalt |
|---|---|
| `mockup-trennung.html` | Die Vorlage. Fünf Varianten der Liste nebeneinander, in hell und dunkel, dazu drei Fassungen des Erfassungsfensters. |
| `bilder/mockup-trennung.png` | Dieselbe Vorlage als Bild, für den Weg ins Design-Projekt. |
| `bilder/streifen-*.png` | Je Schema ein Bild der Liste mit eingeschalteten Zeilenfarben — die Grundlage von M2. |
| `bilder/feldkante-*.png` | Je Desktop-Theme Hülle und Feldgrafik übereinander. |
| `messungen/m1` … `m5` | Die Zahlen. |
| `sonden/` | Die vier Messsonden und die zwei Auswertungsskripte. |
| `pruefen.sh` | Fährt alles von vorn. Läuft ohne `/usr`, ohne Produktivcode, ohne Kundeneinstellung. |

## Die Befunde

**Die Liste steht auf `Base`, nicht auf `Window`.** Gemessen am gebauten
`LibraryWindow`, unter allen 18 installierten Farbschemata und unter zwei
Stilen, am gezeichneten Bildpunkt statt an der angemeldeten Rolle (M1). Die
Annahme im Auftrag war anders herum; sie stammt vermutlich aus einem Blick auf
das **leere** Fenster — dort zeigt die Platzhalterseite `Window`, und genau das
hat die erste Fassung der Sonde auch gemessen, weil sie `show()` statt
`showLibrary()` gerufen hatte.

**Die HIG-Empfehlung fällt durch, zweimal.** Abwechselnde Zeilenfarben liegen
über 18 Schemata zwischen 1,00 : 1 und 1,21 : 1; in 3 Schemata liegt der
Unterschied **unter der Messschwelle** — die beiden Farben sind nicht
buchstäblich gleich, aber ihr Kontrast rundet auf 1,00 : 1 (M1, M4). In 11 von
18 Schemata bleibt er unter 1,10 : 1. Das ist genau die
Gegend, in der die Fußzeilen-Linie am 01.08.2026 gescheitert ist.

Der zweite Grund wiegt schwerer und ist keine Frage des Schemas: Der
Gruppenkopf ist im Modell eine Zeile und verbraucht deshalb einen Streifen.
Zwischen zwei Notizen, die ein Kopf trennt, liegt immer genau eine Zeile — die
beiden bekommen **stets dieselbe Farbe**. In M2 an allen 18 Schemata und an
zwei verschiedenen Gruppengrößen abgelesen: an jeder Gruppengrenze 1,00 : 1.
Der Streifen versagt dort am vollständigsten, wo der Kunde am deutlichsten
hingesehen hat („Gleiches gilt für ‚Gestern' und ‚Letzte Woche'"). Dazu bekommt
der Kopf selbst mal eine Fläche und mal keine, je nach Zahl der Notizen über
ihm.

**Was trägt, ist keine Palettenrolle, sondern eine Mischung.** Grund und
Textfarbe im Verhältnis `frameContrast` des Schemas — das Verfahren, mit dem
Kirigami seine Trennlinien färbt — liegt über dieselben 18 Schemata zwischen
1,24 : 1 und 1,93 : 1, **kein einziges unter 1,20 : 1** (M4). Dieselbe Farbe
zeichnet Plasmas `widgets/lineedit`-Grafik als Kante ihres Eingabefeldes: unter
Breeze Dark beides 66,68,70.

**KRunners Eingabefeld kommt aus der Theme-Grafik.** `KSvg.FrameSvgItem`,
`imagePath: "widgets/lineedit"`, `prefix: "base"`
(`/usr/lib/qt6/qml/org/kde/plasma/components/TextField.qml:187–191`) — dieselbe
Machart wie die Hülle seit #83, eine Grafik tiefer. Offscreen gerechnet hebt
sich das Feld unter `default` mit 1,39 : 1 (Fläche) und 1,33 : 1 (Kante) von
der Hülle ab; im Sitzungsbild des UI-Reviews zu Sprint 7 misst KRunner
1,41 : 1 (M3, M5). Die beiden Wege bestätigen einander bis auf den Bildpunkt.

## Empfehlungen

**Bibliothek — zwei Linien, eine Farbe, zwei Ausdehnungen.** Zwischen zwei
Notizen derselben Gruppe eine Haarlinie, eingerückt auf die Textkante (12 px);
über jedem Gruppenkopf außer dem ersten dieselbe Haarlinie über die volle
Breite. Nicht unter der letzten Notiz einer Gruppe, nicht an der ausgewählten
Zeile. Die Rangfolge entsteht aus der Ausdehnung, nicht aus der Stärke.

Beide Linien liegen in Innenabständen, die es schon gibt (die Eintragslinie im
unteren der 9 px, die Gruppenlinie im oberen der 14 px). **Keine Zeile wächst**
— die Geometrie-Prüfsätze aus Zeichnung 3a und die Zusicherung aus #70 bleiben
unberührt.

**Erfassungsfenster — das Feld aus `widgets/lineedit`, Vorsatz `base`.** Zweite
`KSvg::FrameSvg` hinter dem Textfeld, aus derselben `ImageSet` wie die Hülle.
Kein Eingriff in Hülle, Rundung, Kontur oder Schatten: die liegen eine Ebene
darüber und bleiben, wie #83 sie hinterlassen hat.

Mitentschieden wird dabei der Grund unter dem Text. Bekommt das Feld eine
eigene Fläche, steht der Notiztext nicht mehr auf der Hülle. Die Farbwahl aus
#85 gilt dann für einen anderen Grund und gehört im selben Zug auf die
Ansichts-Textfarbe des Themes umgestellt; gemessen wird der Text dabei besser,
nicht schlechter (unter `default` 12,72 : 1 → 17,68 : 1, unter `breeze-light`
11,42 : 1 → 15,21 : 1, M3). Wer das nicht mitentscheiden will, nimmt die
Fassung „nur die Kante" aus dem Mockup.

## Grenzen

- Unter `CachyOS-Nord-round`, `Iridescent-round` und den drei
  `cachyos-emerald`-Themes zeichnet die eigene `lineedit`-Grafik nur einen
  Hauch (1,03 : 1 bis 1,10 : 1). Dort bliebe das Feld unsichtbar. Das ist
  dieselbe Grenze, die SPEC 3.1 für diese Themes schon benennt; diese Vorlage
  hebt sie nicht auf. Auf der Einstellung des Kunden greift der Rückfall
  `default`, und dort trägt es.
- Alle Zahlen sind offscreen gerechnet. Das genügt, weil keine von ihnen über
  Hülle, Rundung, Kontur, Schatten oder Dekoration etwas behauptet (B21). Die
  eine Aussage, die den Compositor braucht — dass KRunner in der Sitzung ein
  sichtbares Feld zeigt —, kommt aus einem versionierten Sitzungsbild.
- **Nicht belegt:** wie die empfohlenen Linien im gebauten Fenster aussehen.
  Dazu müsste der Delegate sie zeichnen, und das ist Produktivcode. Das Mockup
  zeigt sie mit den gemessenen Farben und den gemessenen Maßen; es ersetzt das
  Bild des gebauten Standes nicht.
