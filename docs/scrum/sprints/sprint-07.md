# Sprint 7

**Sprint-Ziel:** Das Erfassungsfenster wird eine native Plasma-Überlagerung,
und die Bibliothek wird ruhig.

**Basis-Tag:** `sprint-07-basis` = `7afe022` · **Milestone:** Sprint 7 ·
**Planning:** 05.08.2026

---

## 1. Freigabe und Rollenlage — eine Abweichung, ausdrücklich benannt

Das Freigabemodell (Kundenentscheidung 31.07.2026) verlangt, dass jeder Sprint
erst nach Freigabe durch den Kunden startet und der Kunde die Abnahme führt.
**Für Sprint 7 und Sprint 8 hat der Kunde beides am 05.08.2026 vorab an den PO
übertragen:**

> „Erledige alle noch offenen Vorarbeiten und schließe Sprint 7 und 8 ab. Das
> Review macht der PO. Ich als Kunde schau mit das Gesamtergebnis nach Sprint 8
> an. Fragen muss der Product Owner beantworten."

**Was daraus folgt, und was ausdrücklich nicht:**

- Die Freigabe der beiden Sprint-Zuschnitte und die Abnahme der Stories liegen
  beim PO. Produktentscheidungen, die sonst dem Kunden vorlägen, entscheidet
  der PO und **protokolliert sie als solche** — sie stehen in diesem Protokoll
  unter „PO-Entscheidungen an Kundenstelle", damit der Kunde sie nach Sprint 8
  in einem Zug prüfen kann statt sie zu suchen.
- **Die Definition of Done bleibt vollständig in Kraft.** Die Übertragung
  betrifft die Rolle des Abnehmenden, nicht den Prüfumfang. karpathy-Review,
  UI-Review, Installation nach `/usr` und Bildbelege gelten unverändert.
- **Der Kunde nimmt nach Sprint 8 ab.** Bis dahin ist keine Story
  kundenabgenommen; Abschluss-Punkt 10 (Version und Tag) hängt an *seiner*
  Abnahme und bleibt bis dahin ausgesetzt — für **beide** Sprints eine einzige
  Abnahme.

## 2. Vorarbeiten vor der Freigabe

Zum Stand vom 04.08.2026 abends war **kein einziges Issue mit `size:s`
vorhanden** und damit kein Sprint mit #83 (`size:l`) schneidbar — Labels
entstehen erst mit einem Vorprüfbericht. Vor der Freigabe sind deshalb gelaufen:

| Arbeit | Ergebnis |
|---|---|
| Vorprüfung **#70** (zwei Bearbeiter) | siehe §3 |
| **Reproduktionsmessung #71** — die Messung, die Bearbeiter B ausdrücklich *vor* der Freigabe verlangt hatte | Vorbehalt **nicht eingetreten**, `size:s` bestätigt |
| Konsolidierte Berichte **#71** und **#72** | nachgetragen — die fünf Ordner vom 04.08. trugen nur die Einzelmessungen, die DoR verlangt den Bericht |
| Kriterien nachgeschärft: **#72**, **#71**, **#70**, **#61** | je aus dem „ready = nein" der Vorprüfung |
| Gebucht: **#86** Schattenpolsterung, **#87** Über-Dialog | #86 war ein offener Bestandsbefund aus der #83-Vorprüfung, #87 aus #61 herausgeschnitten |
| Neuer Eintrag in `denkzettel-dev.md` | Punkt 5 der Liste „Rückgabewerte und Läufe, die nichts belegen" |

**Ausgangsstand gemessen grün** vor dem Sprintbeginn: `ctest --test-dir build`
meldet 7 von 7 bestanden.

## 3. Vorprüfung #70 — und wie sie den Zuschnitt verändert hat

#70 stand nicht im Kandidatenfeld des PO. **Der Scrum Master hat es
eingefordert**, mit einem Sachargument: #70 arbeitet in denselben Zeilen wie
#71, und #70 AK 3 verlangte ausdrücklich, dass genau der Klickpfad ruhig bleibt,
den #71 ändert. Der Beleg dafür, dass der Einwand nötig war: **#71 nennt #70 im
eigenen Text** unter „Nachbarschaft" — *„die drei zusammen wären ein natürliches
‚ruhige Liste'-Paket"* —, und dieser Satz stand da, bevor die Auswahl getroffen
wurde. #59, das dritte Stück, ist in Sprint 6 geliefert.

Ergebnis: **`size:s`, beide Bearbeiter unabhängig, ready nach Nachschärfung.**
Vollständig in `docs/scrum/vorberichte/70-gruppenkopf-tastatur/bericht.md`.

**Ein Befund dieser Vorprüfung erwies sich als falsch, und das ist festgehalten
statt weggelassen.** Der Scrum Master hatte als schwersten Grund gemessen, #71
wechsle die **Einheit des Rollwerts** — unter `ScrollPerPixel` wären drei der
vier Prüfsätze, auf die #70 baut, grün geblieben und hätten etwas anderes
geprüft. Die Prämisse stimmte nicht: `ScrollPerPixel` ist Lesart 3 von dreien,
der PO hatte Lesart 2 entschieden. Der Scrum Master hat nach der Korrektur
**nachgeprüft statt übernommen** und den Fund als gegenstandslos geankert; die
Messung bleibt stehen und beschreibt, was unter Lesart 3 gegolten *hätte*
(B17).

**Und der Scrum Master hat die Begründung des PO für den Vier-Issue-Schnitt
widerlegt.** Der PO hatte argumentiert, eine Trennung mache ein abgenommenes
Kriterium im Folgesprint falsch. Das gilt nur für die Reihenfolge #70 vor #71 —
in der umgekehrten Trennung entstünde der Widerspruch nicht. **Der tragende
Grund ist ein anderer:** Eine Trennung macht **zweimal dieselben vierzehn Zeilen
und denselben SPEC-Absatz** auf — #70 AK 7 zieht die Kopfregel nach
(`SPEC.md:513–514`), #71 AK 7 die Mausklick-Bedingung (`:514–518`), gemeinsame
Zeile **514** — ohne dafür etwas einzusparen. Das Ergebnis des PO steht, seine
Begründung ist ersetzt.

## 4. Der Zuschnitt

**Vier Issues, zwei Stränge.**

| | **Strang A** | **Strang B** |
|---|---|---|
| **Issues** | #83 (`l`) | #71 (`s`) → #70 (`s`) → #72 (`s`), **in dieser Reihenfolge** |
| **Zweig / Worktree** | `story/83-native-huelle` · `../denkzettel-83` | `story/71-ruhige-liste` · `../denkzettel-71` |
| **Quellen und Tests** | `src/capture/capturewindow.{h,cpp}`, `tests/capturetest.cpp`, `tests/captureshots.cpp`, `tests/themes/` | `src/ui/librarywindow.cpp`, `tests/librarytest.cpp`, `tests/libraryshots.cpp` |
| **Build** | `CMakeLists.txt`, `src/CMakeLists.txt`, `tests/CMakeLists.txt` | **keine** |
| **SPEC** | Abschnitte 3.1, 3.2, 15, 16 | Abschnitt 9 |

**Kleinster gemessener Abstand zwischen den Strängen: 115 Zeilen**, und er liegt
in `SPEC.md` (Ende Abschnitt 9 → Anfang Abschnitt 15). **In Code und Tests
teilen die Stränge keine einzige Datei.** Zwei Worktrees sind damit nicht nur
zulässig, sondern angezeigt.

**Warum Strang B *ein* Strang ist und nicht drei:** Der Abstand zwischen #70 und
#71 ist **null** — dieselbe vierzehnzeilige Bedingung in derselben Funktion.
Zwei parallele Stränge sind dort ausgeschlossen. #72 liegt rund 400 Zeilen
entfernt und berührt keinen Prüfsatz der anderen; es steht deshalb am Ende und
ist das Stück, das bei Überlänge ohne Schaden an der Beweislage der anderen
herausfällt.

**Warum #71 vor #70 gebaut wird:** #71 AK 6 heilt die Klebrigkeit der Marke
`m_selectionFollowsAPress`, und #70 hängt seinen neuen Auslöser hinter genau
diese Marke. #71 zuerst heilt sie **einmal für beide Auslöser**.

### UI-Stories im Sinne von DoD 3 (PO-Festlegung beim Planning)

| Issue | UI-Story? | Begründung |
|---|---|---|
| **#83** | **ja** | Hülle, Rundung, Kontur, Durchsichtigkeit — der Kern von B21 |
| **#71** | **ja** | Auswahl und Anzeige gehen sichtbar auseinander; Zeichnung 3b ist die Referenz |
| **#70** | **ja** | dieselbe Ansicht, dieselbe Zeichnung |
| **#72** | **nein** | Ein Tooltip ist ein eigenes Fenster, das der Zeiger auslöst; unter Wayland bewegt ein Prozess den Zeiger nicht, und `grab()` zeigt ihn nie. Ein UI-Review ohne eigene Bildprüfung zählt für DoD 3 nicht — die Story als UI-Story zu führen, eröffnete einen **nie erfüllbaren** Punkt |

## 5. Sprint-Konto (B12)

| Zeitpunkt | Stand | Anlass |
|---|---|---|
| Freigabe 05.08.2026 | **4 Issues · 1×l, 3×s** | Zuschnitt oben |

**Die obere Grenze der Issue-Zahl ist erreicht, nicht nur berührt.** Die Regel
lautet 2–4 Stories; der Sprint startet bei vier. **Jeder** Zugang nach der
Freigabe ist damit eine **Grenzüberschreitung** nach B12 und als solche
vorzulegen — nicht nur als Story. Bei drei Issues wäre das nicht so, und genau
diese Blindstelle hat Sprint 3 gekostet: Dort wurde bei jedem Zugang die
Punktzahl mitgezählt, die Zahl der Issues nicht.

Die Klassenregel ist mit Luft gehalten: eine `size:l`, daneben ausschließlich
`size:s`.

## 6. PO-Entscheidungen an Kundenstelle

Gesammelt, damit der Kunde sie nach Sprint 8 in einem Zug prüfen kann.

| Nr. | Gegenstand | Entscheidung | Begründung in Kürze |
|---|---|---|---|
| **K1** | **#71 — welche der drei gemessenen Lesarten?** | **Lesart 2:** beim Mausdruck gar nicht nachrücken | Erfüllt beide Kriterien wörtlich und messbar, ist eine Zeile groß, und schreibt dieselbe Regel weiter, die SPEC 9 schon trägt. **Was das kostet, ausdrücklich:** Lesart 3 (`ScrollPerPixel`) wäre die schönere Bedienung — die angeschnittene Zeile würde ganz sichtbar, ohne unter dem Zeiger wegzurutschen. Sie ändert aber nebenbei das Verhalten des Mausrads, und das hat niemand verlangt |
| **K2** | **#72 — Handarbeit oder `KToolTipHelper`?** | **Handarbeit** | Der KDE-Automatikweg trüge von drei Flächen genau eine; die anderen zwei bräuchten `KF6::XmlGui`, einen app-weiten Ereignisfilter und den Umbau zweier Knöpfe |
| **K3** | **#72 — „Speichern" und „Abbrechen" mit aufnehmen?** | **Nein** | Der UI-Review-Befund H1 nennt sie nicht. Bewusste Auslassung, eigene Story, wer sie will |
| **K4** | **#61 — Über-Dialog Teil der Story?** | **Nein**, eigene Story **#87** | Es gibt weder einen Dialog noch einen Ort für seinen Aufruf noch eine Zeichnung — und `KAboutApplicationDialog` zöge `kxmlgui` nach, eine Bibliothek, die auch in der Paketliste des automatischen Laufs fehlt |
| **K5** | **#61 — unbekannte Schalter abweisen?** | **Ja** | Heute nimmt der Dienst jedes Argument stillschweigend an. Geprüft, dass es den Start nicht bricht: Beide `Exec=`-Zeilen der Desktop-Datei starten ohne Argument |
| **K6** | **#72 ist keine UI-Story im Sinne von DoD 3** | siehe §4 | Sonst stünde am Sprint-Ende ein Punkt offen, der nie erfüllbar war |

## 6a. Nacharbeit, die an diesen Sprint gebunden ist

**Die README-Bilder zeigen einen Stand vor #55** (Sprint 6, N1, nachgemessen:
`docs/bilder/erfassungsfenster.png` ist rechteckig und deckend, ohne Hülle).
Der PO hat in Sprint 6 entschieden, sie **nicht** damals nachzuziehen — ein
frischer Lauf hätte ein Bild mit dem Alphakanal-Fehler erzeugt, und das Aussehen
ändert sich mit #83 ohnehin. **Der Befund ist ausdrücklich an die Abnahme von
#83 gebunden** und steht hier, damit er nicht zwischen zwei Protokollen
durchfällt.

Bis dahin beschreibt die README nicht den gelieferten Stand. Das ist ein bewusst
getragener Mangel, kein übersehener.

**Nachtrag 05.08.2026 — er endet nicht mit diesem Sprint, und der Grund ist
gemessen.** Der Versuch, die Bilder nach der Lieferung von #83 zu erneuern, ist
ausgeführt und **zurückgenommen** worden. Der Läufer wurde frisch gebaut
(`cmake --build build --target readmeshots`) und lieferte ein **unlesbares**
Bild:

| | Fläche | häufigste Tinte im Notiztext | Kontrast |
|---|---|---|---|
| vor #83 | (20, 22, 24) | weiß | lesbar |
| **nach #83** | (239, 240, 241) | **(251, 251, 251)** | **1,10 : 1** |

Die Fläche kommt jetzt aus der Theme-Grafik, die Schrift weiter aus dem
Farbschema. **Das ist die Kehrseite von #83, und sie trifft nicht nur fremde
Themes auf fremden Rechnern, sondern den eigenen Dokumentations-Läufer im
Standardlauf.** Das Bild ging nicht ins Repository; die alten Bilder stehen
unverändert.

*Eine Messfalle, die hierhergehört:* Der erste Messversuch ergab **16,78 : 1**
und sah nach einem guten Ergebnis aus — gesucht war der dunkelste Bildpunkt, und
das war der **Textcursor**. Erst die häufigste Tinte im Textband zeigt die
1,10 : 1. **Wer Kontrast am Extremwert misst statt an der Fläche, misst das
Falsche** — und bekommt eine Zahl, die das Gegenteil belegt.

**Nachtrag zwei Stunden später — die Zuordnung war falsch, und das ist der
zweite Fehler in derselben Sache.** Ich hatte den Befund #85 zugeschrieben
(„Fläche aus dem Theme, Schrift aus dem Farbschema"). Eine Zahl aus der
Vorprüfung zu #85 passte nicht dazu: Das Farbschema des Kunden hat `Window`
(30, 34, 51) — **dunkel**, mein Bild hatte eine **helle** Fläche.

**Der Läufer erklärt es selbst.** `tests/readmeshots.cpp` setzt eine
**Qt-Palette** (Breeze Dark) und legt sein `XDG_CONFIG_HOME` **absichtlich auf
ein frisches leeres Verzeichnis** — mit dem ausdrücklichen Kommentar, es gebe
dort kein `kdeglobals`. Das trug, solange die Fläche aus der Qt-Palette kam.
**Seit #83 kommt sie von KSvg, und KSvg liest `kdeglobals`** — ohne eines fällt
es auf seinen eigenen hellen Standard zurück.

**Damit heilt #85 den Fall nicht:** Jene Story nimmt die Textfarbe aus der
`colors`-Datei des Themes; der Läufer läuft unter `default`, und `default` hat
keine. Gebucht als **#96**; die Bindung der README-Bilder hängt dort, nicht an
#85.

**Und es ist kein Anwendungsfehler.** Auf der Maschine des Kunden gibt es ein
`kdeglobals`, KSvg liest es, und Fläche und Schrift stimmen überein — im
UI-Review am Sitzungsbild belegt (Notiztext 12,58 : 1).

*Zwei Fehler in derselben Stunde, beide derselben Bauart:* erst am Extremwert
gemessen statt an der Fläche, dann eine plausible Ursache zugeordnet, ohne sie
zu prüfen. **Beide Male sah das Ergebnis aus wie ein Befund.** Gefunden hat den
zweiten nicht eine Prüfung, sondern eine Zahl aus einem anderen Bericht, die
nicht dazu passte.

**Drei B17-Fundstellen zu `tinted()`.** #83 entfernt die Funktion; drei Regeln
außerhalb der Dateimenge des Strangs nennen sie als **Messgrundlage** von B21
(was ein offscreen erzeugtes Bild belegt und was nicht):

| Datei | Rolle der Stelle |
|---|---|
| `CLAUDE.md` | die Kurzfassung von B21, die jede Sitzung liest |
| `docs/scrum/PROZESS.md` (DoD 3) | die ausführliche Fassung mit Messbeleg |
| `.claude/agents/denkzettel-dev.md` | Punkt 4 der Liste „Rückgabewerte und Läufe, die nichts belegen" |

Gefunden mit dem Griff aus `CLAUDE.md`:
`git grep -n "tinted\|frameContrast\|Alphamaske\|alphaMask" -- CLAUDE.md README.md docs/ .github/ .claude/`
— fünf Zeilen Ausgabe, drei Treffer.

**Das Prüf-Theme wandert aus einem fremden Belegordner heraus.** #83 AK 9
verlangt, das rechteckige Prüf-Theme nach `tests/themes/` zu überführen, wo der
Testaufbau es sucht. Es liegt heute unter
`docs/scrum/reviews/2026-08-04-abnahme-befunde/pruef-theme/` — einem
**abgeschlossenen Belegordner**. Nach dem Verschieben zeigt der dortige Bericht
auf einen Pfad, den es nicht mehr gibt.
**Das ist PO-Arbeit, nicht Strangarbeit** (Belegordner fremder Sprints sind aus
der Dateimenge ausgenommen): Beim Merge kommt in den alten Ordner eine datierte
Zeile mit dem neuen Ort. Der Beleg bleibt damit auffindbar, ohne dass der
Berichtstext geglättet wird (B17).

**Sie sind ausdrücklich aus der Dateimenge von Strang A ausgenommen und gehören
dem PO.** Und sie werden **nicht gestrichen:** Die Regel ist gemessen und gilt
weiter, nur ihr Beispiel ist dann Geschichte. Nachzuziehen ist die
**Fundstellenangabe**, nicht die Aussage — sonst entsteht der umgekehrte Fehler:
eine Regel ohne Beleg. Fällig **nach** der Abnahme von #83, nicht davor.

## 6b. Lieferung — was die beiden Stränge gebaut haben

Zusammengeführt am 05.08.2026, beide mit `--no-ff`. **Bau warnungsfrei, `ctest`
7 von 7 grün, volle Testauflage 113 von 113.**

| Strang | Commits | Codeänderung |
|---|---|---|
| **A** (#83) | 4 | `tinted()`, `mixed()`, `FrameContrast`, `OutlineWidth` und der Zwei-Masken-Ring entfallen; `framePixmap()` zeichnet in einem Stück beim Bildpunktverhältnis des Fensters; neuer `event()`-Zweig für `DevicePixelRatioChange`; die beiden Anmeldungen `enableBlurBehind` und `enableBackgroundContrast`; Auswahlpfad `opaque` ohne weichzeichnende Sitzung |
| **B** (#71, #70, #72) | 4 | #71: das `scrollTo(index)` steht unter dem Mausdruck-Merker, der Merker endet mit dem Loslassen (4+3 Zeilen) · #70: ein Oder-Zweig im Bedingungskopf (2 Zeilen) · #72: drei `setToolTip()` und eine Hilfsfunktion (3+4 Zeilen) |

**Die Belege sind der weitaus größere Teil des Diffs** — 146 Dateien, 7.577
Zeilen, davon rund 20 Produktivcode und Tests. Das ist gewollt (B7).

### Die Mutationsproben, gezählt

**#83: fünfzehn Proben, vierzehn belegt, eine als Grenze benannt.** Drei davon
laufen in der angemeldeten Sitzung, weil ihr Fehler offscreen **gar nicht
auftreten kann** — ohne `DevicePixelRatioChange`-Zweig hinkt die Hülle bei 2,
ohne die `nullptr`-Wache stürzt der Aufruf ab (Rückgabe 139, Signal 11), ohne
die Anmeldung bleibt der Grund scharf. Offscreen sind alle drei **grün**.

**Strang B: vierzehn Proben.** Zwei Zusicherungen tragen den Beweis
ausdrücklich **nicht** und sind als Regressionsschutz ausgewiesen — im
Fehlerbild von #71 waren beide richtig, das war ja der Befund.

### Vier Funde, die die Arbeit selbst hervorgebracht hat

1. **Ein Prüfsatz war grün, als die geprüfte Zeile entfernt wurde** (#83 AK 7).
   Er lief gegen ein Theme ohne zweite Fassung — die Wahl konnte dort nichts
   ändern. Gefunden hat ihn die Mutationsprobe, nicht das Nachdenken.
2. **Zwei lebende `KSvg::ImageSet` desselben Themenamens teilen ihre
   Auswahlpfade.** Eine zweite Instanz meldet `opaque`, ohne dass ihr jemand
   einen gegeben hätte — jeder Vergleich zweier Fassungen derselben Grafik läuft
   sonst gegen sich selbst.
3. **Die Überführung des Prüf-Themes hat drei Zusicherungen in die stille
   Richtung umgeworfen** — der Testaufbau reichte danach ein *eckiges* Theme an
   drei Prüfsätze weiter, die ein rundendes brauchen. Gefangen hat es der
   Testlauf.
4. **Qt rollt eine halbe Sekunde nach dem Klick doch nach** (Sichtprüfung
   Strang B). Gebucht als **#89**, dem Blick des Kunden vorbehalten.

Die Fälle 1 und 2 stehen seit dem 05.08.2026 als Punkte 6 und 7 in der Liste
„Rückgabewerte und Läufe, die nichts belegen" (`.claude/agents/denkzettel-dev.md`).

### Eine Messung des PO am Bild neben KRunner

Ich hatte beim Hinsehen den **Eindruck**, das Erfassungsfenster sei heller als
KRunner und die Benachrichtigung daneben. Nachgemessen im selben Bild:

```
KRunner  Fläche      (47, 50, 52)
Denkzettel Fläche    (47, 50, 52)
Meldung  Fläche      (47, 50, 52)
Schreibtisch        (128,128,128)
```

**Byteidentisch über drei verschiedene Plasma-Überlagerungen.** Mein Eindruck
kam aus dem Umfeld, nicht aus dem Fenster — genau die Sorte Fehlurteil, gegen
die „prüfe am Einzelfall, nicht an der Plausibilität" geschrieben ist. Es steht
hier, weil ein widerlegter PO billiger ist als ein übersehener Befund.

## 6c. karpathy-Review — sieben Warnungen, kein `fail`, einer widerlegt

Bericht: `docs/scrum/reviews/sprint-07-karpathy.md`. Auftrag nach B19 über den
**Diff** (`sprint-07-basis..main`, 146 Dateien), nicht über die Stories.

| ID | Sache | Behandlung |
|---|---|---|
| **K1** | Die Kopfzahl der Mutationsproben von #83 zählt drei Sachverhalte doppelt | **bestätigt**, Berichtigung an den Übergabebericht gehängt: 12 Zusicherungen in 15 Läufen, 11 belegt, 1 als Grenze benannt. AK 14 bleibt erfüllt |
| **K2** | „beide Läufe 113/113 grün" widerspreche den Belegen (112 und 110) | **widerlegt** — siehe unten |
| **K3** | Die Mutationsproben von Strang B sind nicht wiederholbar; die Eingriffe sind nirgends festgehalten | **bestätigt**, an Strang B zur Nacharbeit |
| **K4** | `m_blursBehind` wird einmal im Konstruktor erhoben — Abschalten des Effekts zur Laufzeit erreicht das Fenster nicht | **bestätigt**, Grenze in SPEC 3.2 Punkt 9 benannt (DoD 4/B9), Entscheidung als **#93** gebucht |
| **K5** | Zeichnung 4a/4b zeigt weiterhin den abgewählten Nachbau, SPEC 3.1 verweist darauf | **bestätigt**, **#94** gebucht — und der B17-Griff erweitert, siehe unten |
| **K6** | Der Regelstellen-Nachzug lief **vor** der Abnahme, entgegen der eigenen Festlegung in §6a | **angenommen**, siehe unten |
| **K7** | Der UI-Review fehlte im Diff | in Arbeit, war zum Prüfzeitpunkt richtig |

### K2 — warum ein Befund zurückgewiesen wird, und wie

Der Reviewer hat zwei Dateipaare verwechselt. Die von ihm genannten Dateien
tragen beide `113 passed, 0 failed`; die Zahlen 112 und 110 stehen in
`70-testauflage-nach-heilung.txt` und `71-testauflage-nach-heilung.txt`.

**Sie sind nicht widersprüchlich, sondern belegen die Reihenfolge:** 110 nach
#71, 112 nach #70 (zwei neue Prüfsätze), 113 nach #72 (einer). Der Satz im
Übergabebericht stimmt.

**Der Befund wird nicht gelöscht, sondern auf `verworfen` gesetzt und mit einer
datierten Zeile versehen** (B17). Ein zurückgezogener Befund, der spurlos
verschwindet, sieht später aus, als hätte niemand hingesehen.

### K5 — der wertvollste Befund trifft nicht den Sprint, sondern das Werkzeug

Der B17-Griff in `CLAUDE.md` durchsuchte `CLAUDE.md README.md docs/ .github/`.
**`wireframes/` und `SPEC.md` standen nicht darin.** Der PO hat den Griff in
diesem Sprint selbst gefahren (§6a), fünf Zeilen Ausgabe bekommen und ihn für
vollständig gehalten — **er hatte Treffer, also sah er richtig aus.**

Unsichtbar blieb dabei, dass Zeichnung 4a/4b weiterhin den vom Kunden
abgewählten Nachbau zeigt, samt einer Kontur, die es nicht mehr gibt.

Der Griff ist am 05.08.2026 erweitert worden, mit diesem Vorfall als Begründung:
**Ein Werkzeug, dessen Suchraum kleiner ist als der Geltungsbereich der Regel,
meldet Vollständigkeit und liefert sie nicht.**

### K6 — eine eigene Festlegung, gegen die der PO verstoßen hat

§6a hält fest, der Regelstellen-Nachzug sei „fällig **nach** der Abnahme von
#83, nicht davor". Gefahren wurde er unmittelbar nach dem Merge, vor jeder
Prüfung. Sachlich ohne Folge — die Regeln sind inhaltlich unverändert, nur ihre
Fundstelle ist geankert. **Es steht hier, weil der Satz im selben Protokoll
steht und ein unbemerkter Verstoß gegen die eigene Festlegung derselbe Fehler
ist wie eine Regel, die niemand liest.**

## 7. DoD-Prüfung

**Takt 1, vor der Abnahme.** Geführt vom Scrum Master am 05.08.2026 gegen den
Stand `268a7c5` auf `main`. **Gemessen, nicht übernommen:** Bau, Testlauf,
Prüfsummen der Bildbelege, Installationsstand, README-Bilder und Doku-Abgleich
sind eigene Läufe; die Berichte der Stränge dienen als Anspruch, gegen den
gemessen wurde, nicht als Beleg.

### 7.1 Eigene Messungen

| Gegenstand | Lauf | Ergebnis |
|---|---|---|
| Bau warnungsfrei | frischer Bauplatz außerhalb des Repositoriums, `cmake -B … -DCMAKE_BUILD_TYPE=Debug` + `cmake --build … -j12` | **0 Warnungen** in 124 Zeilen Bauausgabe (`grep -ci warning` = 0). Nicht in `build/` gemessen — der gehört den Strängen |
| Tests auf Ganymed | `ctest --test-dir build` | **7 von 7** bestanden, 6,48 s |
| Volle Auflage | `capturetest`, `librarytest`, offscreen mit `QT_QPA_PLATFORMTHEME=kde` | **27** und **113** Prüfsätze, je 0 gescheitert |
| Automatischer Lauf | `gh run list --commit c488ab5` | `completed success` — **gilt nur bis `c488ab5`**; die vier jüngsten Commits sind ungepusht und haben keine Marke |
| Installierter Stand | `readlink /proc/$(pgrep -x denkzetteld)/exe`, `stat` | `/usr/bin/denkzetteld`, **kein `(deleted)`** — und die Datei trägt den **04.08.2026, 16:05** bei 7.913.024 Bytes gegen den gebauten Stand vom 05.08.2026, 19:38 bei 7.979.744 Bytes. **Der laufende Dienst ist der Stand von Sprint 6** |
| Zwei Fenstergrößen (DoD 1, Satz 2) | `tests/capturetest.cpp` | `hullIsCompleteAtFiveAndEightLines` prüft fünf und acht Zeilen; `checkHullDiffersBetween` zwei Themes. Die Raumaufteilungs-Zusicherungen der Bibliothek stehen unverändert im Bestand |

### 7.2 DoD 1–4 je Story

| Story | DoD 1 Bau/Tests | DoD 2 AK + installierter Stand | DoD 3 Reviews | DoD 4 SPEC |
|---|---|---|---|---|
| **#83** | **erfüllt** | **nicht geführt** — AK am gebauten Stand belegt, Hauptweg am **installierten** Stand nicht ausgeführt (M1) | **nicht geführt** — zwei offene `fail` im UI-Review (M2); karpathy ohne `fail` | **erfüllt** — 3.1 umgedreht, 3.2 Punkte 6–9, 15 und 16; ein falscher Verweis (M6) |
| **#71** | **erfüllt** | **nicht geführt** — dieselbe Ursache (M1) | **erfüllt** — UI-Review Punkte 21–25 `pass`, Punkt 24 `warn`; karpathy ohne `fail` | **erfüllt** — SPEC 9 samt Nachlauf-Bedingung aus Befund P2 |
| **#70** | **erfüllt** | **nicht geführt** — dieselbe Ursache (M1) | **erfüllt** — Punkte 32–34 `pass`, 35/36 `warn` (Zeichnung) | **erfüllt** — SPEC 9 als Ergänzung |
| **#72** | **erfüllt** | **nicht geführt** — dieselbe Ursache (M1) | **entfällt** (K6: keine UI-Story); karpathy ohne `fail` | **erfüllt, geprüft statt unterstellt** — die entdeckte Bedingung („die *Aktionen* starten abgeschaltet, nicht die Knöpfe") berührt keine SPEC-Festlegung; SPEC 9 sagt über den Zustand der Schaltflächen nichts zu. Kein Nachzug nötig |

**Zu DoD 2, damit die Zeile nicht milder gelesen wird als sie ist:** Die
Akzeptanzkriterien sind je Story einzeln belegt, und die Belege tragen — aber
DoD 2 verlangt den Hauptweg **am installierten Stand**, und Takt 1 Punkt 1
verlangt ihn für **jede** Story. Beides ist nicht geschehen. Die vier Stories
sind damit nicht abnahmereif, unabhängig von der Qualität ihrer Belege.

### 7.3 Doku-Abgleich (B10 in der Fassung nach B17)

Geprüft: `README.md`, `docs/`, `CLAUDE.md`, der Kommentarkopf von
`.github/workflows/ci.yml` und die Kommentarköpfe von `CMakeLists.txt`,
`src/CMakeLists.txt`, `tests/CMakeLists.txt`. **Das Ergebnis steht hier auch
dort, wo nichts zu melden war.**

| Ort | Befund |
|---|---|
| `.github/workflows/ci.yml`, Kopf | **in Ordnung.** Beschreibt Zweck, die drei nicht erreichten DoD-Punkte und die fünf Bildläufer richtig. Die Paketliste enthält `qt6-base` — die mit #83 an `denkzettelcapture` gelinkte `Qt6::DBus` steckt darin, der Lauf auf `c488ab5` bestätigt es |
| `CMakeLists.txt`, `src/`, `tests/` | **in Ordnung.** Die Silence-Begründung in `tests/CMakeLists.txt` und die spellfix-Zeile in `src/CMakeLists.txt` beschreiben den gelieferten Stand |
| `CLAUDE.md`, B21-Absatz | **in Ordnung und mustergültig geankert** (`c488ab5`): `tinted()` bleibt als Messung stehen, der Fall ist datiert, drei jüngere Geschwister sind benannt |
| `CLAUDE.md`, B17-Griff (Zeile 108) | **Befund M5** — die Liste ist um `SPEC.md` und `wireframes/` erweitert worden, **`.claude/` fehlt weiterhin** |
| `README.md`, Statuszeile | **existiert in dieser README nicht** — dieselbe Feststellung wie in Sprint 6 §19.7. Ihre Funktion nimmt der Absatz „Auf der Liste stehen noch" (`:49–52`) wahr; er nennt keinen Sprint und verweist auf die Issues. **Durch die Abnahme wird daran nichts falsch** |
| `README.md:56–59`, Abhängigkeiten | **in Ordnung, geprüft statt unterstellt.** #83 verlinkt `Qt6::DBus` neu an `denkzettelcapture` und liest die Theme-Gruppe mit `KConfigGroup` — DBus, Config und Svg stehen bereits in der Liste, sie wächst nicht. Das ist die Stelle, an der Sprint-6-Mangel M4 saß |
| `README.md:151` | **Befund M4** — „Sprints mit **geschätzten** Stories". Die Story-Point-Schätzung ist am 04.08.2026 beendet |
| `README.md:111–129` | **in Ordnung.** Fünf Bildläufer, seit dem 04.08.2026 im gewöhnlichen Build, weiterhin nicht in `ctest` — trifft den Stand |
| **README-Bilder** | **Mangel bleibt offen, jetzt mit gemessenem Grund** (§6a-Nachtrag). Eigene Nachmessung: `docs/bilder/erfassungsfenster.png` ist 600×178 Bildpunkte, die Fläche (239, 240, 241) deckend, und außer dem Eckpunkt (0,0) ist keine Zeile durchscheinend — ein Fenster **ohne** Theme-Hülle. Das Bild zeigt den Stand vor #55. **Befund M3** |
| `docs/scrum/` | Die Prozessdateien tragen den Stand; `PROZESS.md` DoD 3 ist mit `c488ab5` geankert |

### 7.4 Prüfsummen der Bildbelege — das Urteil des PO nachgeprüft

Eigener Lauf über die drei Belegordner dieses Sprints:
**Rückgabe 1** (#83, fünf Gruppen), **Rückgabe 0** (Strang B), **Rückgabe 0**
(UI-Review). Gleiche Zahlen wie beim PO. Die fünf Gruppen habe ich **nicht
übernommen, sondern gegen die Quelle des Läufers gelesen**
(`sonden/fensterlage.cpp:194–251`, `sonden/weichzeichner.cpp:283`, die vier
`m6-*`-Ausgaben):

| Gruppe | Urteil | Grund |
|---|---|---|
| `ecke-ruhe` = `ecke-theme-default`, `fenster-ruhe` = `fenster-theme-default` (offscreen **und** Sitzung) | **kein Mangel — und stärker als der PO es gefasst hat** | Der PO nennt es „derselbe Zustand unter zwei Namen". Gemessen liegt zwischen den beiden Aufnahmen **eine ganze Wegstrecke**: acht Zeilen Text hinein und wieder heraus (Zeilen 204–218), Wechsel auf das eckige Prüf-Theme (225), Rückwechsel auf `default` (234). Byteidentität ist hier **kein Duplikat, sondern der Nachweis, dass der Rückweg vollständig ist** — genau der ausdrücklich erlaubte Fall |
| `weichzeichner-an` = `weichzeichner-wiederzeigen` | **kein Mangel — mit einer Berichtigung an der Begründung** | Es sind zwei getrennte Prozessaufrufe mit je eigenem `spectacle -f`; der Lauf `wiederzeigen` führt vorher `hide()`/`showCapture()` aus. Byteidentität heißt: die Anmeldung überlebt das Neuzeigen. **Nicht zutreffend ist der Zusatz des PO, die Spannweite 6 sei „unabhängig gemessen":** Sie ist aus **derselben** Aufnahme gerechnet. Unabhängig ist die *Aufnahme*, nicht die Zahl. Das Urteil bleibt richtig, sein Beleg ist ein anderer als angegeben |

**Stop-Bedingung nach PROZESS.md:** Dieser Lauf hat neue Gruppen gefunden, die
Prüfung bleibt Vollprüfung.

### 7.5 Vollzähligkeit der Prüfberichte (Takt 1, Punkt 2)

Vier Prüfläufe, vier Berichte als Datei, alle vor dieser Prüfung abgelegt:
`sprint-07-s83-native-huelle/bericht.md`, `sprint-07-s71-ruhige-liste/bericht.md`,
`sprint-07-karpathy.md`, `sprint-07-ui-review/bericht.md`.

Gegenprobe nach dem vorgeschriebenen Weg — alle Commit-Botschaften des
Sprint-Diffs nach Befund-Nennungen durchsucht: Jede genannte Prüfung
(karpathy-Lauf, UI-Review, die beiden Übergabeberichte, der Nachlauf der
Mutationsproben von Strang B) hat ihren Bericht oder ihre Messausgabe im Repo.
**Ein Lauf ohne Bericht ist nicht aufgefallen.**

Zwei Beobachtungen an den Berichten selbst:

- Der karpathy-Bericht führt in der Statusspalte **sechs Befunde weiterhin als
  „offen"**, obwohl das Protokoll sie als bearbeitet ausweist. Nur K2 ist
  nachgeführt. **Befund M7** — die Prüfung soll gegen das Artefakt laufen, nicht
  gegen die Behauptung des Protokolls; das ist der Zweck von Takt 1 Punkt 2.
- Der UI-Bericht trägt **eine unversionierte Zeile** (Befund P5). **Befund M8.**

### 7.6 Sprint-Konto (B12)

| Zeitpunkt | Stand | Anlass |
|---|---|---|
| Freigabe 05.08.2026 | 4 Issues · 1×l, 3×s | Zuschnitt §4 |
| Sprint-Ende 05.08.2026 | **4 Issues · 1×l, 3×s** | **kein Zugang** |

Nachgemessen am Milestone: `Sprint 7` trägt genau **#83, #72, #71, #70**. Die
zehn im Sprint entstandenen Issues **#86–#95** tragen **keinen Milestone** und
sind sämtlich offen im Backlog. Beide Grenzen sind gehalten; die Issue-Grenze
war von Anfang an ausgereizt und ist nicht angetastet worden.

---

## 8. Mängelliste

Melden, nicht heilen. Die Behebung liegt beim PO.

| # | Mangel | DoD/Regel | Gewicht |
|---|---|---|---|
| **M1** | **Der Endstand ist nicht nach `/usr` installiert, und der Hauptweg keiner der vier Stories ist am installierten Stand ausgeführt.** `pkexec` wartet seit über einer Stunde auf das Passwort des Kunden. Gemessen: `/usr/bin/denkzetteld` trägt den 04.08.2026, 16:05 — der laufende Dienst ist der Stand von **Sprint 6**; alle Läufe dieses Sprints liefen am gebauten Stand | **DoD 2**, Takt 1 Punkt 1 | **schwer** |
| **M2** | **Zwei offene `fail`-Befunde im UI-Review** (W1: Zeichnung 4a behauptet das Gegenteil des gelieferten Standes; W2: Zeichnung 4b führt eine Farbrolle, die es nicht mehr gibt). Sie sind an #94 gebucht — **eine Buchung ist keine Heilung.** Der Präzedenzfall des Projekts ist Sprint 4: dort wurde der `fail` geheilt und **nachgeprüft**, erst dann galt DoD 3 als erfüllt | **DoD 3** | **schwer** (blockiert die Abnahme von #83) |
| **M3** | **Die README beschreibt den gelieferten Stand weiter nicht.** Der Nachzug ist versucht, gemessen und begründet zurückgenommen worden (§6a-Nachtrag) — das ist sauber gearbeitet, hebt den Mangel aber nicht auf. §6a sagte „er endet mit diesem Sprint"; er endet nicht. Die Bindung wandert auf #85 | **DoD 2**/B10 | **mittel, bewusst getragen** |
| **M4** | **`README.md:151` nennt „Sprints mit geschätzten Stories".** Die Story-Point-Schätzung ist am 04.08.2026 beendet und durch Vorprüfbericht und Größenklasse ersetzt. Der öffentliche Text beschreibt ein Verfahren, das es nicht mehr gibt | B10 | leicht |
| **M5** | **Der B17-Griff in `CLAUDE.md:108` durchsucht `.claude/` nicht** — obwohl die dritte der drei `tinted()`-Fundstellen dieses Sprints in `.claude/agents/denkzettel-dev.md` lag und §6a den Griff nur deshalb fand, weil dort **von Hand** `.claude/` angehängt war. Das ist **dieselbe Fehlerklasse wie K5, eine Runde später**: ein Werkzeug, dessen Suchraum kleiner ist als der Geltungsbereich der Regel | B17 | mittel |
| **M6** | **`SPEC.md` 3.2 Punkt 9 schreibt die Grenze dem „karpathy-Befund **K5**" zu.** Es ist **K4**; K5 ist der Zeichnungsbefund. Ein Verweis, der auf den falschen Befund zeigt, führt jeden Nachprüfer an die falsche Stelle | DoD 4 | leicht |
| **M7** | **Der karpathy-Bericht führt K1, K3, K4, K5, K6 und K7 in der Statusspalte weiterhin als „offen".** Nur K2 ist nachgeführt. DoD 3 wird damit gegen die Behauptung des Protokolls geprüft statt gegen das Artefakt — genau die Lage, gegen die B7 und Takt 1 Punkt 2 gefasst sind. Nachzuführen ist **datiert und anhängend** (B17), nicht durch Überschreiben | Takt 1 Punkt 2 | mittel |
| **M8** | **Der UI-Bericht trägt eine unversionierte Zeile** — Befund P5 steht im Arbeitsbaum und nicht im Commit. Ein unversionierter Beleg ist kein Beleg | B7 | leicht |
| **M9** | **`build-vor85/` ist mit `402ee8b` ins Repository geraten: 445 Dateien, 9,8 MB, darunter `.o`- und `.a`-Dateien, `a.out` und `CMakeCache.txt`.** `.gitignore` deckt `build/` und `build-install/` ab, diesen Bauplatz nicht. **Drei Zeilen der `CMakeCache.txt` nennen Pfade unterhalb von `~/.local`** — Pfade außerhalb des Projekts sind nach der Kundenentscheidung vom 02.08.2026 tabu, und das Repository ist öffentlich. **Der Commit ist noch nicht gepusht; der Fehler ist bis dahin ohne Außenwirkung reparabel** | Repo-Grenzen (Artefakte), B7 | **schwer, aber zeitkritisch statt dauerhaft** |
| **M10** | **`main` ist seit sieben Commits nicht gepusht** (`4a4d249` … `268a7c5`), weil der SSH-Schlüssel nicht freigegeben ist — dieselbe Ursache wie M1. Folge für die Beweislage: Die vier jüngsten Commits haben **keine Marke des automatischen Laufs**; grün ist zuletzt `c488ab5`, und das ist der Stand **vor** UI-Review, SPEC-9-Berichtigung und K3-Heilung | Push-Kadenz | mittel |

**Was aus M1 für die Abnahme folgt, ausdrücklich:** Der Sprint ist **nicht
abnahmefähig**. Takt 1 Punkt 1 ist der erste Punkt der Liste, und DoD 2 ist für
alle vier Stories offen. Eine Abnahme jetzt wäre eine Abnahme des **gebauten**
Standes — und genau diese Verwechslung ist der Sprint-3-Mangel M1, um
dessentwillen die Regel überhaupt in `CLAUDE.md` steht. Zwei Wege stehen offen,
und beide gehören dem PO: die Installation nachholen, sobald der Kunde am
Rechner ist — oder die Lücke dem Kunden **als benannte Grenze** vorlegen und die
Abnahme mit ihr treffen. Was nicht geht, ist die Punkte still abzuhaken.

**Was aus M2 folgt:** #71, #70 und #72 sind nach DoD 3 sauber. **#83 ist es
nicht.** W1 und W2 betreffen keine Zeile Code — sie sind mit einem datierten
Vermerk an der Zeichnung heilbar und einer Nachprüfung durch `denkzettel-ux`,
beides innerhalb dieses Sprints machbar. Wird stattdessen entschieden, einen
`fail` an einem Dokumentationsartefakt als nicht abnahmeblockierend zu führen,
ist das eine **Änderung der DoD** und gehört dem Kunden vorgelegt, nicht
nebenbei entschieden.

---

## 9. done/next

**done**

- **Vier Stories gebaut und belegt**, zwei Stränge, beide mit `--no-ff`
  zusammengeführt. Bau **warnungsfrei** (eigener frischer Bauplatz),
  `ctest` **7 von 7**, `capturetest` **27**, `librarytest` **113** — alles
  eigenständig nachgemessen.
- **DoD 1 und DoD 4 sind für alle vier Stories erfüllt.** DoD 3 für #71 und
  #70; für #72 entfällt der UI-Teil nach K6.
- **Vier Prüfläufe, vier abgelegte Berichte**, keiner ohne Datei.
- **Prüfsummen der Bildbelege gelaufen und einzeln beurteilt** — fünf Gruppen,
  kein Mangel, eine Begründung des PO berichtigt.
- **Sprint-Konto gehalten:** vier Issues, `1×l, 3×s`, **kein Zugang** nach der
  Freigabe. Die zehn neuen Issues #86–#95 sind im Backlog geblieben.
- **Der Sprint hat mehr Befunde hervorgebracht als er Stories hatte** — zehn
  Issues, davon sieben aus der Arbeit selbst. Zwei Fälle, in denen ein Lauf
  grün war und nichts prüfte, sind in `denkzettel-dev.md` verankert.

**next — in dieser Reihenfolge**

1. **M9 vor dem Push.** `build-vor85/` aus dem Baum nehmen und `.gitignore`
   nachziehen, solange `402ee8b` lokal ist. Danach ist es öffentlich.
2. **M1: installieren**, sobald der Kunde am Rechner ist — dann den Hauptweg
   **jeder** Story am installierten Stand ausführen, vorher `denkzetteld`
   beenden und neu starten und den `readlink`-Beleg ablegen (B16). Ohne diesen
   Schritt bleibt Takt 1 unvollständig.
3. **M2: W1 und W2 heilen oder eskalieren.** Ohne das ist #83 nicht abnehmbar.
4. **M7, M8, M6, M5, M4** — Nachträge an Bericht, SPEC, `CLAUDE.md` und README;
   je datiert und anhängend, nicht überschreibend.
5. **Dann erst die Abnahme** durch den PO (Rollenlage §1), danach Takt 2:
   AK-Haken, Issues und Milestone schließen, Journal, Push, Zweige und
   Worktrees räumen — `story/71-ruhige-liste` und `story/83-native-huelle`
   samt `../denkzettel-71` und `../denkzettel-83` stehen noch —, Changelog.
   **Punkt 10 (Version und Tag) bleibt ausgesetzt:** Der Kunde nimmt nach
   Sprint 8 ab, und Abschnitt 6b von #61 ist nicht geliefert.
6. **Nicht zu diesem Sprint:** Die unversionierten Dateien unter
   `docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/` gehören zur Vorprüfung
   von Sprint 8 und sind hier kein Mangel — sie gehören in den Commit, der die
   Vorprüfung abschließt.
