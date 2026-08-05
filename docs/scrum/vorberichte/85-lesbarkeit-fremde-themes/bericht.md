# Vorprüfbericht #85 — Lesbarkeit unter fremden Desktop-Themes

**Konsolidiert vom PO am 05.08.2026** aus `messung-a.md` (Bearbeiter A,
`denkzettel-ux`) und `messung-b.md` (Bearbeiter B, Scrum Master), beide gegen
den Stand **nach #83**, unabhängig voneinander.

**Ergebnis: `size:m` — beide Bearbeiter unabhängig, jetzt gemessen statt
fortgeschrieben. Ready nach Nachschärfung durch den PO.**

---

## 1. Warum diese Vorprüfung erst jetzt lief

#85 wurde am 04.08.2026 von #83 abgetrennt. **Sein Boden ist der Code *nach*
#83** — den es damals nicht gab. Das Label `size:m` war bis zu diesem Bericht
eine **Fortschreibung, kein geprüftes Urteil**; der Sprint-Abschluss von #83 hat
das ausdrücklich so festgehalten. Beide Bearbeiter haben die Klasse ohne
Rücksicht darauf neu gesetzt und kommen unabhängig wieder auf `m`.

## 2. Der Befund, der alles andere überlagert: die Zahlen des Issues waren tot

**Vier unabhängige Gründe, jeder für sich ausreichend:**

1. **Das Farbschema des Kunden hat sich geändert** — `kdeglobals`, 05.08.2026
   17:41, jetzt Breeze Dark. **Vor** dieser Sitzung, also nicht durch das Team.
   Alle Kontrastzahlen des Issues rechneten gegen das vorherige Schema:
   `breeze-light` Schemaschrift **1,11 : 1 statt 1,75 : 1**, `breeze-dark`
   **15,39 statt 7,94**, die Ausgangslage von AK 4 **6,64 statt 2,91**.
2. **#83 hat jedem Theme eine zweite Fläche gegeben.** Der Auswahlpfad `opaque`
   greift ohne weichzeichnende Sitzung; die drei Emerald-Themes drehen dabei von
   schwarz/2,7 % nach hell/3,5 %. „84,7 % Deckung" ist ohne Angabe des
   Auswahlpfads mal richtig, mal falsch.
3. **`CachyOS-Nord-round` folgt dem Farbschema nicht** — gemessen über drei
   Schemata. Die Tabelle des Issues führte es als folgend; sie las einen
   **Zufall** unter dem damaligen Schema.
4. **Es gibt keine *eine* Kontrastzahl.** Für `cachyos-emerald-light` liegt die
   Themeschrift bei **14,32 : 1 über weißem** und **1,38 : 1 über schwarzem**
   Grund, die Schemaschrift genau umgekehrt (1,04 / 20,47). Die 1,38 des Issues
   ist das dunkle Ende einer Spanne — keine Fiktion, aber ohne benannten Grund
   auch keine Aussage.

**Was das über das Verfahren sagt:** Eine Story, die drei Wochen im Backlog
liegt, trägt Zahlen, die niemand mehr nachrechnet. Genau dafür ist die
Vorprüfung da — und genau deshalb steht ihr Label erst mit ihrem Bericht.

## 3. Was die Story leichter macht, als sie aussieht — der Fund von A

**`KSvg::Svg::color(Text)` mit gesetztem `colorSet(Window)` *ist* bereits die
Kundenregel:** Themefarbe, wenn eine `colors`-Datei da ist, sonst Schemafarbe.
Gemessen über **acht Themes × drei Schemata**. Und sie zieht beim Theme-Wechsel
von selbst nach, womit AK 5 fast von allein fällt. `KF6::Svg` ist verlinkt, am
Build ist nichts zu tun.

**Die Lücke, und sie ist der eigentliche Umfang:** Für `ForegroundInactive` —
die gedämpfte Textklasse — hat KSvg **kein** Gegenstück. Diese zweite Klasse
braucht den KConfig-Weg, wie ihn `contrastEffectOf()` aus #83 zwanzig Zeilen
weiter oben schon geht.

**Bearbeiter B hatte genau diese Frage als offen an den PO gemeldet** („ein Lauf
darüber ist die erste Handlung des Strangs und entscheidet, ob AK 1 eine Zeile
kostet oder dreißig"). A hat den Lauf gemacht. Das ist der Wert zweier
Bearbeiter an diesem Fall: einer benennt die Frage, der andere misst sie.

## 4. Die Entscheidung, die die Messung nicht treffen konnte

Das Fenster hat **zwei** Schriften. Die frühere Fassung sprach nur vom
Notiztext — damit wäre offen geblieben, ob nach dieser Story zwei Schriften aus
zwei Quellen auf einer Fläche stehen.

**Keine der beiden Quellen gewinnt überall** (gemessen, durchscheinend,
Kleintext):

| Theme | aus dem Schema | aus dem Theme |
|---|---|---|
| `breeze-light` | 2,09 : 1 | **3,70 : 1** |
| `breeze-dark` | 6,64 : 1 | 6,64 : 1 (dieselbe Farbe) |
| `cachyos-emerald-color` | 8,83 / 2,24 : 1 | 3,87 / 5,10 : 1 |
| `cachyos-emerald-light` | 8,83 / 2,24 : 1 | 4,99 / 3,96 : 1 |

**PO-Entscheidung: die Regel gilt für beide Klassen.** Da die Zahlen nicht
entscheiden, entscheidet die Konsistenz — und die Kundenentscheidung sagt
„immer". Zwei Schriften aus zwei Quellen auf einer Fläche wären derselbe Bruch,
den diese Story heilt, nur eine Ebene tiefer.
**Ausdrücklich nicht behauptet:** dass die gedämpfte Klasse damit lesbar wird.
Unter `breeze-light` erreicht **keine** Quelle 4,5 : 1. Das bleibt **#84**.

## 5. Drei gemessene Fallen

**Der Kontrasteffekt hat auf diesem Stand keinen Empfänger** — schärfer als das
Impediment aus Sprint 7 („nicht beobachtbar"). KWin 6.7.3 führt unter **54**
Effekten **keinen** mit „contrast" im Namen; `isEffectLoaded("backgroundcontrast")`
ist `false`, `blur` ist `true`. Unter den Emerald-Themes steht der Text damit auf
dem Bildschirmhintergrund und auf nichts sonst. **Nicht zu umgehen, nur zu
benennen.**

**Im Testmodus weichen Palette und KSvg-Grundlage auseinander.** `capturetest`
sieht `default` mit hellem Grund unter der dunklen Schrift des Kunden —
**1,11 : 1, ein Zustand, den es nirgends gibt.** Folge fürs Kriterium:
Kontrastzahlen sind in `ctest` nicht zusicherbar; zusicherbar ist die **Herkunft
der Farbe**.

**Die Bildsatz-Falle greift enger als befürchtet.** Zwei lebende `ImageSet`
teilen die **Auswahlpfade**, und nur bei gleichem Themenamen; die **Farben**
bleiben in allen Fällen richtig. Ein Reihenlauf über acht Themes in einem
Prozess ist damit unbedenklich — 32 Messzeilen identisch zu acht
Einzelprozessen.

## 6. Zwei Prüfmittel-Fallen, die ein Mensch öffnen muss

Beide sind am 05.08.2026 als Punkte 8 und 9 in `.claude/agents/denkzettel-dev.md`
aufgenommen worden, weil sie allgemeiner sind als diese Story:

- **Bei gesperrter Sitzung liefert `spectacle -f` ein schwarzes Bild mit
  Rückgabe 0.** Die Sitzung war während des ganzen Vorprüflaufs gesperrt
  (`LockedHint=yes`); die Sonde hätte „das Fenster hebt sich nirgends ab"
  gemeldet — **ein Befund über den Bildschirmschoner, der aussieht wie einer
  über das Fenster.** Die Sonde prüft es jetzt selbst und bricht ab.
- **Ein echtes Vollbildfenster als Prüfgrund** legt der Compositor **über** das
  Erfassungsfenster; gemessen wird dann der Grund über der Hülle statt unter
  ihr.

## 7. Die sechs Felder

**Feld 1 — Dateimenge.** `src/capture/capturewindow.cpp` an fünf kleinen
Stellen (freie Funktion neben `contrastEffectOf()`, `reloadDesktopTheme()`,
`applyTextColours()`, Ereignisfilter), `src/capture/capturewindow.h`,
`tests/capturetest.cpp`, **eine `colors`-Datei an einem mitgelieferten
Prüf-Theme** (heute hat keines eine). **Build: nichts** — `KF6::Svg` und
`KF6::ConfigCore` sind verlinkt, `tests/CMakeLists.txt` reicht den **Ordner**
durch.
**Ausdrücklich nicht:** `subtleLabel()` als Rollenfrage (**#84**), `src/ui/`,
`src/shell/`, `src/store/`, `textareaheight.*`, die Bildläufer der anderen
Ansichten, `wireframes/`, `CLAUDE.md`, `PROZESS.md`, Belegordner fremder
Sprints.

**Feld 2 — gemessene Fallen:** die drei aus §5 und die zwei aus §6.

**Feld 3 — AK-Urteil: ready = nein** (B, gedeckt durch den Vorschlag von A).
Der Befund von B: **es fehlte ein Kriterium für den Schemawechsel.** AK 1 sagt,
woher die Farbe kommt, AK 5 sichert den Theme-Wechsel — den **Schemawechsel**
sicherte nichts, und genau dort sitzt der Mechanismus. Eine Umsetzung, die AK 1
nur beim Theme-Wechsel erfüllt, ist bei der Abnahme richtig und nach dem ersten
Schemawechsel falsch, **lautlos**: Der einzige einschlägige Prüfsatz sichert
heute das Gegenteil zu und bleibt grün, weil sein Theme keine `colors`-Datei
hat.
**Behoben durch den PO am 05.08.2026:** sieben Kriterien, alle Zahlen ersetzt
oder gestrichen, die beiden Sachfehler berichtigt, der Auswahlpfad in den
Messweg aufgenommen, die zwei stummen Voraussetzungen benannt, AK 7 neu.
**Damit ready.**

**Feld 4 — Prüfmittel.** Zusicherbar ist die **Herkunft** der Farbe (§5), nicht
die Kontrastzahl. Sitzungsschiene aus #83 vorhanden.
**Grenze, die die Story erbt und nicht schließen kann:** die Wirkung von
`enableBackgroundContrast`. Kein Kriterium sichert Lesbarkeit unter den
durchscheinenden Themes zu — und das ist ehrlich so, nicht ausweichend.

**Feld 5 — Größenklasse: `size:m`.** Beide unabhängig. Nicht `s`: neuer
Mechanismus, neues Prüfmittel (kein Prüf-Theme hat eine `colors`-Datei),
sitzungsgebundener Bildbeleg. Nicht `l`: kein Bauzugang, rund fünfzig Zeilen in
zwei Dateien nach einem Muster, das zwanzig Zeilen weiter oben schon steht.

**Feld 6 — offene Fragen: alle vom PO entschieden.** Die Regel gilt für beide
Textklassen (§4) · Zahlen ersetzt · Sachfehler berichtigt · **#81 nicht im
selben Sprint** (drei Zeilen Abstand) · der Bestandsbefund Schattenpolsterung
ist seit dem 05.08.2026 als **#86** gebucht (dritte Meldung desselben Befunds —
er war zum Zeitpunkt der ersten beiden Meldungen tatsächlich ungebucht).
