# Untersuchung der Kundenbefunde der Sprint-6-Abnahme (#55)

**Modus:** UI-Review, messend · **Datum:** 04.08.2026 · **Prüfer:** `denkzettel-ux`
**Stand:** `main` @ `a01afd5`, Produktivcode gleich dem installierten `977e804`
**Belege:** `LIESMICH.md`, `messungen/`, `bilder/`, `kundenbilder/`, `pruefen.sh`

---

## Die Bedingung, die alles andere ordnet

Der Bildschirm des Kunden läuft mit **Skalierung 1,6** — 3840×2160 physisch,
2400×1350 logisch (`kscreen-doctor`). Das Erfassungsfenster ist damit 600×174
logische und **960×278 tatsächliche Bildpunkte** groß (`b1-echtelage.txt`).

Diese Zahl steht in keinem Artefakt des Sprints. Die Bildbelege zu #55 sind
sämtlich bei Verhältnis 1 entstanden, die Zeichnung 4a kennt kein
Pixelverhältnis, und die SPEC nennt keines. **Zwei der vier Befunde bestehen
nur bei einem gebrochenen Verhältnis** — bei 1 sind sie unsichtbar. Das ist der
Grund, warum ein grüner Sprint einen roten Abnahmetermin hatte.

---

## B1 — Die Ecke ist eine Treppe, nicht ein Bogen

### F1 · Die Treppe · **fail**

Der Befund stimmt und hat eine einzige Ursache.

**Gemessen:** `alphaMask()` liefert die Hülle in **600×150 Bildpunkten bei
DPR 1** — unter Skalierung 1, 1,6 und 2 dieselbe Zahl, während die Fensterfläche
auf 960×278 bzw. 1200×348 wächst (`b1-eckenraster-skala-*.txt`). Die Maske wird
also von QPainter um den Faktor 1,6 hochgezogen, und zwar ohne Glättung. Der
Bogen des Themes ist 6 logische Bildpunkte groß und hat bei 1× nur vier
unterscheidbare Lagen; hochskaliert werden daraus Stufen.

Am laufenden Fenster in der Sitzung des Kunden (`b1-echtelage.txt`) sitzt die
Kontur je Zeile in Spalte 6 · 6 · 3 · 2 · 2 · 2 · 0 · 0 · 0 — **drei Plateaus,
Sprünge von 3 und 2 Bildpunkten**. Genau das nennt der Kunde „drei bis vier
Stufen". Die native Ecke seiner Vergleichsaufnahme wandert eine Spalte je Zeile,
mit Zwischenwerten (`kundenbilder/04-ecke-nativ.png`).

**Fundstelle:** `src/capture/capturewindow.cpp:143-145` (die drei `FrameSvg`
werden ohne Pixelverhältnis gebaut) und `:338-343` (`resizeHull()` gibt
logische Maße weiter).

**Korrekturvorschlag, gemessen statt geraten:** Abschnitt e von
`b1-huellenring-live.txt` zeigt, dass KSvg die Maske sehr wohl in
Gerätebildpunkten liefert — `setDevicePixelRatio(1.6)` ergibt eine Maske von
960×279 mit kantengeglättetem Bogen, ein Schritt je Zeile. Es genügt also, den
drei Rahmen das Pixelverhältnis des Fensters mitzugeben und es nachzuziehen,
wenn das Fenster den Bildschirm wechselt (`QEvent::DevicePixelRatioChange`).
Eine Randnotiz für die Umsetzung: KSvg lieferte 960×**279** statt 278 — eine
Zeile zu viel, die beim Zeichnen abzuschneiden ist.

### F2 · Die Kontur fehlt auf dem Bogen — aber nur offscreen · **fail**

Hier hat sich meine erste Erklärung als falsch erwiesen, und der Weg dahin ist
Teil des Befundes.

`tinted()` (`capturewindow.cpp:83-95`) baut die eingefärbte Form so:
`QPixmap result(shape.size()); result.fill(colour);` und danach
`CompositionMode_DestinationIn`. Füllt man eine QPixmap mit einer **deckenden**
Farbe, wählt Qt ein Format **ohne Alphakanal**. Gemessen
(`b1-huellenring-offscreen.txt`): `Alphakanal NEIN, Format 4` (RGB32). Die
Folge ist zweifach — die innere Pixmap wird als **Rechteck** geblittet und
löscht die Kontur, die die äußere gerade auf den Bogen gemalt hatte, und
zwischen Kontur und Fläche bleibt ein **vollständig durchsichtiger Bildpunkt**
stehen. Die Gegenprobe im selben Protokoll (Abschnitt d, echter Alphakanal)
liefert einen lückenlosen, kantengeglätteten Bogen; Bilder:
`bilder/b1/offscreen/ring-ist.png` gegen `ring-gegenprobe-alphakanal.png`.

**Und dann trägt es doch nicht.** Derselbe Binärcode, dasselbe Theme, derselbe
Zwischenspeicher, nur die Plattform gewechselt: unter `wayland` liefert
dieselbe Zeile `Alphakanal ja, Format 6`
(`b1-huellenring-live.txt`) — der Bogen ist vollständig. Das Format der Pixmap
hängt an der Plattform, nicht am Code.

**Was daraus folgt, ist schlimmer als der ursprüngliche Verdacht:**

1. Am Bildschirm des Kunden fehlt die Kontur **nicht**. Sein Bild zeigt sie in
   jeder Zeile des Bogens (`b1-eckhelligkeit.txt`: Werte zwischen 199,7 und
   210 quer durch die Rundung, und dahinter kein durchsichtiger Punkt, sondern
   232 bis 240 auf dem Weg in die Fläche). Was er sieht, ist die Treppe aus F1
   und der helle Streifen aus F3 — nicht dieser Fehler.
2. **Der Fehler steckt in den Bildbelegen des Sprints.** `tests/captureshots.cpp`
   läuft unter `QT_QPA_PLATFORM=offscreen`; die zwölf Bilder zu #55 AK 1 und
   AK 7 zeigen deshalb eine Ecke, die es am laufenden Stand nicht gibt.
   Nachzusehen ohne jedes Werkzeug in
   `docs/scrum/reviews/sprint-06-s55-huelle/bilder/01-rand-schmal-hell-leer.png`:
   Die Hülle beginnt dort bei (24,24), und Zeile 6 liest sich **198 · 242 · 239** —
   Kontur, dann die **Schraffur des Untergrunds** durch ein durchsichtiges Loch,
   dann erst die Fläche. Auf dem Bogen selbst (Zeilen 1 bis 5) steht kein
   einziger Konturwert. Wer die Rundung an dieser Bildreihe abgenommen hat, hat
   einen schlechteren Stand abgenommen als den ausgelieferten.
3. Der Code ist trotzdem zu heilen: Er verlässt sich unausgesprochen darauf,
   dass die Plattform ein Format mit Alphakanal wählt. **Vorschlag:** in
   `tinted()` ein `QImage(size, QImage::Format_ARGB32_Premultiplied)` statt
   einer QPixmap — dann ist das Ergebnis auf jeder Plattform dasselbe, und die
   Bildbelege zeigen wieder, was der Kunde sieht.

Damit ist auch [#80](https://github.com/hnsstrk/denkzettel/issues/80) neu zu
lesen: Was dort als fehlende Konturfarbe auf dem Bogen beschrieben ist, ist
**offscreen wahr und am laufenden Stand nicht**. Ob #80 aus einem Bild oder aus
der laufenden Anwendung stammt, entscheidet, ob es ein Fehler des Erzeugnisses
oder des Prüfwegs ist. Das gehört geklärt, bevor jemand daran arbeitet.

### F3 · „Der weiße Hintergrund ragt über den Rahmen hinaus" · **fail**

Der Satz des Kunden ist eine Zahlenaussage, und die Zahlen geben ihm recht.

**Gemessen** (`b1-eckhelligkeit.txt`, an seiner eigenen Aufnahme): Quer durch
den Bogen läuft der Schatten von 245 auf 222 herunter — und dann steht
unmittelbar vor der Kontur ein Streifen von **zwei Bildpunkten mit 239 und 248**.
248 ist **heller als der ungeschattete Grund weit draußen** (245). Dort ist also
weder Fenster noch Schatten.

Die native Ecke derselben Sitzung hat das nicht: dort fällt die Helligkeit von
225 monoton bis 200 an die Kontur, ohne einen einzigen Punkt, der heller wäre
als der Grund.

**Ursache — Vermutung, ausdrücklich als solche.** Die zwei hellen Punkte liegen
bei x=8 und x=9; die Kante des Fenster**rechtecks** liegt bei x=8, die Kontur
des gerundeten Bogens in dieser Zeile bei x=10/11. Der helle Streifen ist also
genau der Teil des Fensterrechtecks, den die Rundung durchsichtig lässt. Dass
der Schatten des Compositors nur **außerhalb** des Fensterrechtecks liegt, würde
das erklären. **Belegen kann ich es nicht** — dazu müsste man KWin ansehen, wie
es die Kacheln legt, und dafür fehlt mir der Zugriff. Die Übereinstimmung der
drei Koordinaten ist stark, sie ist aber ein Indiz und kein Beweis.

Für die Behebung heißt das: **erst diese Frage klären.** Trifft die Vermutung
zu, hilft an F1 und F2 keine Änderung — der Streifen bliebe.

### F4 · „Rundet Denkzettel stumpf alles ab?" · **ok — die Annahme ist widerlegt**

Die Frage war unbeantwortet, weil **alle acht installierten Desktop-Themes
runden**. Ich habe deshalb ein Prüf-Theme mit rechteckigen Eckstücken gebaut
(`pruef-theme/`, Aufbau wie `tests/themes/`) und gemessen, was die Hülle daraus
macht.

**Ergebnis, offscreen bei Skalierung 1 und 1,6 und noch einmal am laufenden
Fenster in der Sitzung des Kunden** (`b1-eckiges-theme-skala-*.txt`,
`b1-echtelage-eckiges-theme.txt`): Die Kontur sitzt in Zeile 0 und Spalte 0, die
Fläche beginnt bei (1,1), **kein einziger Zwischenwert**. Die Hülle ist
rechteckig.

Denkzettel rundet nichts von sich aus. Es nimmt die Form, die das Theme gibt —
und wenn die eckig ist, ist die Hülle eckig. Der Widerspruch zur Anforderung
„nativ KDE", den der Kunde vermutet, besteht nicht.

---

## B2 — Der Schatten · **warn**

**Belegt ist die Herkunft** (`b2-schattenherkunft.txt`): Die Kacheln kommen aus
`/usr/share/plasma/desktoptheme/default/dialogs/background.svgz`, also aus dem
eingestellten Desktop-Theme des Kunden. Ihr Alphaverlauf ist der des Themes,
nicht linear und nicht gleichförmig (die Eckkachel steigt 1·2·3·5·6·10·16·17
und bricht dann an der Rundung ab); die Farbe ist reines Schwarz, die
Höchstdeckung 28 von 255, das Ausmaß 10 logische Bildpunkte. **Der Schatten
kommt aus dem KDE-Theme.** Die Vermutung des Kunden trifft insofern nicht zu.

**Sein Eindruck trifft trotzdem etwas.** Denkzettel trägt den Schatten eines
**Plasma-Dialogs** — denselben, den Plasmas Aufklapper tragen. Ein
Anwendungsfenster mit Titelleiste bekommt seinen von der **Fensterdekoration**,
einer anderen Quelle mit anderem Maß. Gemessen an seinen beiden Eckaufnahmen
(`b2-schattenprofil.txt`):

| | Verdunkelung an der Kante | Länge des Verlaufs |
|---|---|---|
| Denkzettel | −25 von 244 = **10 %** | **8** Bildpunkte, dann fertig |
| natives Fenster | −24 von 224 = **11 %** | **über 21** Bildpunkte, am Rand des Ausschnitts noch nicht zu Ende |

Gleich stark an der Kante, **etwa dreimal so kurz**. Ein kurzer, harter Schatten
liest sich als Aufklapper, ein langer weicher als Fenster — das ist genau der
Eindruck „das ist kein Anwendungsfenster".

**Wo diese Messung endet:** Beide Ausschnitte sind abgeschnitten, bevor der
Schatten ausläuft; die 8 und die 21 sind **Untergrenzen**. Die Reichweite des
nativen Schattens in Zahlen habe ich **nicht** gemessen — die Dekoration ist ein
KWin-Plugin, keine Bibliothek, die man fragen kann, und eine aus dem Gedächtnis
abgeschriebene Konstante wäre kein Messwert. Am großen Nebeneinander-Bild ist
nichts zu holen: Das Hintergrundbild ist gestreift und schwankt von Zeile zu
Zeile um mehr, als der ganze Schatten ausmacht (nachgerechnet, verworfen).

**Kein Vorschlag von mir.** Ob Denkzettel den Schatten eines Dialogs oder den
eines Fensters tragen soll, ist eine Produktentscheidung — das Erfassungsfenster
*ist* eine kurzlebige Einblendung, und dafür ist die Dialogquelle die richtige.
Das gehört dem PO und dem Kunden, nicht mir.

---

## B3 — „Die Schriftfarben des Themes werden nicht übernommen" · **ok mit einem UX-Befund**

### F6 · Die Farben stimmen · **ok**

Das Farbschema der Aufnahme ist **CachyOSNordLightly** — erkannt daran, dass
seine `Colors:Window/BackgroundNormal` mit 30,34,51 auf den Punkt der Fläche im
Bild des Kunden entspricht. Unter genau diesem Schema gemessen
(`b3-flaechenfarbe-nord.txt`, eigenes `XDG_CONFIG_HOME`, das Schema des Kunden
nicht angefasst):

| | gezeichnet | Palette | |
|---|---|---|---|
| Fläche | 30,34,51 | `Window` 30,34,51 | **gleich** |
| Anwendungsname | 102,106,115 | `PlaceholderText` 102,106,115 | **gleich** |
| Fußzeile | 102,106,115 | `PlaceholderText` 102,106,115 | **gleich** |
| Notiztext | — | `WindowText` 102,194,242 | **vorhanden**, 12 Treffer |

Die Fläche trägt die Palettenfarbe. Die Zusicherung „Form vom Theme, Farbe aus
der Palette" hält am laufenden Stand. Die Vermutung, die Farbe komme aus dem
Desktop-Theme-SVG, ist **widerlegt**.

Gegenprobe an der Aufnahme des Kunden selbst: Denkzettels Fläche und Dolphins
Seitenleiste sind dort **beide** 30,34,51, Bildpunkt für Bildpunkt.

### F7 · Warum es trotzdem falsch aussieht · **warn**

Im Ruhezustand zeigt das Fenster **ausschließlich** gedämpfte Texte:
„Denkzettel", „Gedanke festhalten …", die Fußzeile — alle drei
`PlaceholderText`, also `ForegroundInactive` des Schemas. Der Notiztext, der
`WindowText` trägt, erscheint erst, wenn jemand tippt; der Kunde hat auf beiden
Aufnahmen nichts getippt.

Und `CachyOSNordLightly` ist in sich uneinheitlich: `ForegroundNormal` ist blau
(102,194,242), `ForegroundInactive` ist ein neutrales Grau (102,106,115) — aus
einem anderen Schema übernommen und nicht angepasst. Ein Fenster, das nur
Gedämpftes zeigt, wirkt unter diesem Schema deshalb themafremd, obwohl jede
einzelne Farbe stimmt.

Das erklärt auch die Gegenprobe des Kunden, ohne dass man sie glauben muss:
Unter `Win11OSDark` sind `ForegroundNormal` (222,222,222) und
`ForegroundInactive` (136,136,136) **beide** neutral — dort fällt nichts auf,
und er schreibt „scheint es zu passen".

**Vorschlag:** Der Anwendungsname „Denkzettel" ist die Überschrift des Fensters
und kein Platzhalter. Ihn auf `WindowText` zu stellen — gern eine Spur gedämpft
über die Deckkraft statt über die Rolle — gäbe dem ruhenden Fenster wenigstens
einen Text in der Normalfarbe des Schemas. Die Fußzeile ist ein Tastenhinweis
und darf gedämpft bleiben; das Eingabefeld ist ein Platzhalter und muss es. Das
berührt Zeichnung 4b und ist deshalb eine Entscheidung des PO, kein Fehler.

### F8 · Die Schreibmarke · **info**

Die Schreibmarke im Textfeld ist 225,221,204 — auf den Punkt die **Umkehrung**
der Fläche (255 − 30, 255 − 34, 255 − 51). Sie kommt aus keinem Palettenwert.
Das ist Qts übliches Verhalten und stört unter diesem Schema nicht sichtbar; es
steht hier, weil es beim Messen der Textfarben zuerst wie eine Abweichung aussah
und der nächste Prüfer sonst dieselbe Runde dreht.

---

## B4 — Nicht aufgenommen

Wie vom PO angewiesen: Der Kunde hat zu Punkt 5 der Checkliste `Tab` statt
`Alt+Tab` gedrückt. Das ist nicht der geprüfte Fall; der PO klärt es.

---

## Die Frage über allem

> **Ist die Hülle als Nachbau gebaut, wo sie es nicht müsste?**

**Die Rundung nicht.** F4 beweist es an einem Theme mit eckigen Ecken: Was
Denkzettel zeichnet, ist Bildpunkt für Bildpunkt die Form, die das Theme liefert.
Es gibt keinen eingebauten Radius.

**Die Kontur schon.** Die einzige Linie des Fensters wird aus zwei Masken
konstruiert, die um einen Bildpunkt gegeneinander versetzt gezeichnet werden.
Diese Konstruktion ist die Stelle, an der **beide** sichtbaren Fehler sitzen —
die Treppe (F1, weil die Masken in 1× gerechnet werden) und die Lücke im Bogen
(F2, weil die Einfärbung keinen Alphakanal führt). Native Fenster haben diese
Konstruktion nicht; sie zeichnen einen kantengeglätteten Pfad.

Dass die Hülle überhaupt nachgebaut wird statt die Pixel des Themes zu nehmen,
ist eine **begründete und dokumentierte** Entscheidung (Zeichnung 4a: sieben von
acht Themes richten ihre Füllung nicht nach dem Farbschema, ihre Pixel gäben
dunklen Text auf dunkler Fläche). Diese Entscheidung stelle ich nicht in Frage.
Was sie **nicht** erzwingt, ist beides oben: Auch eine selbst gefärbte Hülle
darf ihre Maske in Gerätepixeln rechnen und ihre Farben mit Alphakanal führen.

Mein Urteil: **Der Kunde sieht richtig, und die Ursache liegt nicht darin, dass
zu viel nachgebaut wurde, sondern darin, dass der Nachbau in zwei Punkten unter
seiner eigenen Möglichkeit bleibt.** Beide sind gemessen und beide haben einen
belegten Weg (`b1-huellenring-*.txt`, Abschnitte d und e). Der dritte Punkt (F3,
der helle Streifen) ist gemessen, aber in der Ursache offen — und er ist der
einzige, bei dem ich nicht weiß, ob eine Änderung an unserem Code ihn
überhaupt erreicht.

---

## Offene Punkte

1. **F3** — Ursache des hellen Streifens am Bogen nicht belegt. Wer KWin
   ansehen kann, sollte prüfen, ob der Schatten das Fensterrechteck aussparen
   muss.
2. **#80 neu einordnen.** Stammt der Befund aus einem offscreen-Bild, ist er ein
   Fehler des Prüfwegs; stammt er aus der laufenden Anwendung, widerspricht er
   meiner Messung und ich will das wissen.
3. **Die Bildpflicht braucht ein Pixelverhältnis.** `captureshots` fährt bei 1;
   der Kunde fährt 1,6. Solange das so ist, sagt die Bildreihe zu AK 7 nichts
   über das, was er sieht. Das ist eine DoD-Frage und gehört dem Scrum Master.
4. **F7** ist eine Entscheidung des PO an Zeichnung 4b, kein Fehler.
5. **B2** ist eine Produktentscheidung, keine Korrektur.
