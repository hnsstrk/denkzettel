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
getragener Mangel, kein übersehener — und er endet mit diesem Sprint.

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

## 7. DoD-Prüfung

*(Takt 1, vor der Abnahme — wird beim Sprint-Ende gefüllt)*

## 8. Mängelliste

*(wird beim Sprint-Ende gefüllt)*

## 9. done/next

*(wird beim Sprint-Ende gefüllt)*
