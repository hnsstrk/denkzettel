# UX-Beratung zur Vorprüfung von #83 — native Plasma-Überlagerung

**Modus:** Planning-Beratung · **Datum:** 04.08.2026 · **Verfasser:** `denkzettel-ux`
**Gegenstand:** die acht Akzeptanzkriterien von [#83](https://github.com/hnsstrk/denkzettel/issues/83)
**Stand:** `main` @ `80b52ae`, Arbeitsbaum sauber bis auf `docs/scrum/vorberichte/`
**Eigene Belege:** `ux-beratung/` neben dieser Datei — `pruefen.sh`, `sonden/`, `messungen/`, `bilder/`
**Fremde Belege, gelesen:** `docs/scrum/reviews/2026-08-04-abnahme-befunde/` (Untersuchungsbericht,
Kundenbilder, Messungen), `docs/scrum/reviews/sprint-06-ux-review/`, `docs/scrum/sprints/sprint-06.md`
§24–§26, `SPEC.md` 3/3.1/3.2, `wireframes/Denkzettel Wireframes.dc.html` 4a/4b,
KDE HIG (Quelltext vom 04.08.2026 aus `invent.kde.org/documentation/develop-kde-org`)

**Was diese Beratung nicht ist:** keine Schätzung, keine Größenklasse, keine
Priorisierung. Keine Änderung an `wireframes/`, `src/`, `SPEC.md` oder Issues.

---

## 0. Zwei Vorbemerkungen zur Lage

### 0.1 Ich habe die Messungen des zweiten Bearbeiters nicht gelesen

Im Ordner `83-native-huelle/` arbeitet parallel ein zweiter Bearbeiter
(`sonden/rahmenmasse.cpp`, `fensterlauf.cpp`, `weichzeichnerbeleg.cpp`,
`themefarbe.cpp`, `testfolgen.cpp` samt `messungen/sonde1…sonde4`, alle
unversioniert). `PROZESS.md` verlangt, dass die beiden Bearbeiter unabhängig
messen. Ich habe deshalb **keine** seiner Protokolle geöffnet; wo unten etwas
steht, das er womöglich schon gemessen hat, sage ich es dazu.

Meine eigenen Sonden liegen in **`ux-beratung/sonden/`**, mit eigenem Bauplan
und eigenem Bauplatz, damit sich die beiden Läufe nicht in die Quere kommen.
Ich habe drei Dateien versehentlich kurz in seinen `sonden/`-Ordner kopiert und
sie wieder entfernt; sein Bestand ist unverändert, seine `CMakeLists.txt` habe
ich nicht angefasst.

### 0.2 Abweichung von der Ablagekonvention — bewusst, nicht übersehen

`PROZESS.md` (Artefakte) legt Vorprüfberichte als **eine Datei**
`docs/scrum/vorberichte/NN-<kurzname>.md` fest. Der PO hat mir für diese
Beratung `docs/scrum/vorberichte/83-native-huelle/ux-beratung.md` genannt, und
der Ordner besteht bereits mit fremdem Inhalt. Ich folge dem Auftrag und melde
die Abweichung: Entweder die Konvention wird auf „Ordner je Story" nachgezogen,
oder der Ordner heißt am Ende `83-native-huelle.md` mit Beilagen daneben. Das
gehört dem Scrum Master.

---

## 1. Frage 1 — decken die acht AK ab, was der Kunde beanstandet hat?

Fünf Befunde, weil K-A1 im Wortlaut des Kunden **zwei** Beobachtungen enthält
(*„Sie ist nicht wirklich rund"* und *„Der weiße Hintergrund ragt über den
Rahmen hinaus"*), die verschiedene Ursachen haben und im Untersuchungsbericht
als F1 und F3 getrennt geführt werden.

| # | Kundenbefund | deckendes AK | Verdikt |
|---|---|---|---|
| B-1 | K-A1a — die Ecke ist eine Treppe | AK 2 + AK 3 | **warn** |
| B-2 | K-A1b — der helle Streifen am Bogen | **keines** | **fail** |
| B-3 | K-A2 — der Schatten passt nicht | **keines**, ausdrücklich ausgenommen | **warn** |
| B-4 | K-A3 — die Schriftfarben des Themes | AK 6, misst die falsche Achse | **fail** |
| B-5 | K-A4 — „das sollte KDE nativ sein" | **keines** in seiner Prüfform | **warn** |

### B-1 · Die Treppe · **warn**

**Fundstelle:** AK 3 — *„Kein Stufenlauf mehr an der Ecke: der Alphaverlauf der
obersten Zeile ist monoton, kein Plateau."*

AK 2 (Bildpunktverhältnis bei 1 und 1,6) trifft die Ursache, und der native Weg
ist dafür gemessen (N1). Was AK 3 prüft, ist aber **eine Zeile**. Der Befund des
Kunden ist über **Zeilen** gemessen: `b1-echtelage.txt` liest die Spalte der
Kontur je Zeile als `6 · 6 · 3 · 2 · 2 · 2 · 0 · 0 · 0` — drei Plateaus, und
genau das nennt er „drei bis vier Stufen". Eine Hülle kann in der obersten Zeile
einen sauberen Verlauf haben und quer über die Zeilen trotzdem springen. AK 3
prüft damit die Achse, auf der der Befund **nicht** entstanden ist.

**Korrekturvorschlag:** zwei Sätze statt einem.
> *Der Alphaverlauf der obersten Zeile des Eckbogens ist monoton ohne Plateau,
> **und** die Spalte, in der die Hülle je Zeile beginnt, wächst über die ersten
> zehn Zeilen monoton mit einem Schritt von höchstens einem Bildpunkt je Zeile.
> Beides bei Bildpunktverhältnis 1 **und** 1,6. Messweg wie
> `b1-echtelage.txt`.*

### B-2 · Der helle Streifen · **fail**

**Fundstelle:** Issue #83, Abschnitt „Offene Punkte, die vor dem Ziehen
entschieden gehören", Punkt 2 — *„mit dieser Story kostenlos prüfbar: Fällt die
Treppe, muss er kleiner werden oder verschwinden."*

Das ist eine **Erwartung**, kein Kriterium. Der Kunde hat den Streifen im
Wortlaut beanstandet, er ist an seiner eigenen Aufnahme in Zahlen belegt
(`b1-eckhelligkeit.txt`: zwei Bildpunkte mit 239 und 248 vor der Kontur, wobei
248 **heller** ist als der ungeschattete Grund weit draußen mit 245), und die
Erklärung dafür ist ausdrücklich unbewiesen (N6: *„Beweisen kann ich sie hier
nicht"*). Unter der jetzigen AK-Liste kann die Story vollständig abgenommen
werden und der Streifen ist noch da. **Das ist der Befund, für den diese
Beratung da ist.**

**Korrekturvorschlag:** ein AK, das die Prüfung erzwingt und den Fehlschlag
ordnet.
> *Am Bogen der linken oberen Ecke steht in der angemeldeten Sitzung bei
> Skalierung 1,6 kein Bildpunkt, der heller ist als der ungeschattete Grund
> außerhalb des Schattens. Messweg: `eckhelligkeit.py` auf einer neuen Aufnahme
> derselben Stelle. **Trifft das nicht zu, ist die Erklärung aus N6 widerlegt;
> der Befund geht dann als eigenes Issue an den PO zurück und blockiert die
> Abnahme dieser Story nicht.***

Der zweite Satz ist der wichtigere: Er macht den Befund prüfbar, ohne die Story
an eine Ursache zu binden, die außerhalb unseres Codes liegen könnte.

### B-3 · Der Schatten · **warn**

**Fundstelle:** Issue #83, Abschnitt „Was diese Story ausdrücklich **nicht**
löst".

Die Begründung trägt und ist am Binärcode belegt (N6: Breeze bindet weder KSvg
noch libplasma, rechnet seinen Schatten selbst). Was fehlt, ist die
Entscheidung. Der Kunde hat gesagt *„Der Schatten passt auch nicht zu dem
Schatten, den andere Fenster werfen."* Das Issue antwortet, der Schatten der
Überlagerungs-Familie sei mit seiner Entscheidung konsistent — das ist eine
plausible Ableitung aus einem Satz, den er zu einer **anderen** Frage gesagt
hat. Offener Punkt 10 des Untersuchungsberichts sagt bereits, dass es dem Kunden
gehört; im Issue steht es als erledigt.

**Korrekturvorschlag:** keine AK-Änderung. Der Satz gehört als *ausdrücklich
nicht enthalten* auf die Abnahme-Checkliste, damit der Kunde ihn bei der Abnahme
noch einmal vor sich hat und nicht ein zweites Mal denselben Befund meldet.

**Randbefund zur Umsetzung:** Die Schattenkacheln kommen aus dem `shadow`-Prefix
**derselben** Grafik wie die Hülle. Wenn die Story einen Auswahlpfad setzt
(siehe B-9), wechseln die Kacheln mit. Das gehört benannt, sonst ändert sich der
Schatten als Nebenwirkung.

### B-4 · Die Schriftfarben · **fail**

**Fundstelle:** AK 6 — *„Die Flächenfarbe folgt weiterhin dem Farbschema
(Messweg: 20 Schemata, `native-farben.txt`)."*

Drei Einwände, der dritte ist der schwere.

**Erstens:** `native-farben.txt` **gibt es nicht.** Der Messordner enthält
`native-ak2-kontrast.txt`, `native-huelle-{nord,breeze,eckiges-theme,nord-wayland}.txt`.
Ein AK, dessen benannter Messweg nicht auflösbar ist, ist nach Definition of
Ready (Feld 4) nicht prüfbar.

**Zweitens:** Es sind heute **19** installierte Farbschemata, nicht 20.
`Win11OSDark` — das Schema aus der Gegenprobe des Kunden, Kundenbild
`02-win11-dark-theme.png` — ist auf der Maschine nicht mehr auffindbar
(systemweite Suche, 04.08.2026). Ein Wiederholungslauf trifft die 20 nicht mehr.
Die fünf Schemata unter 4,5:1 bleiben dieselben fünf.

**Drittens, und darum geht es:** AK 6 misst **20 Farbschemata unter EINEM
Desktop-Theme**. Die Sonde der Sprint-6-Messung baut
`KSvg::ImageSet("default", …)` fest ein (`sonden/nativekontrast.cpp:78`). Die
Farbtreue der Grafik ist aber eine Eigenschaft des **Desktop-Themes** —
Zeichnung 4a hat das am 01.08.2026 gemessen und daraus die ganze Entscheidung
„Form vom Theme, Farbe aus der Palette" begründet: *„Von den acht Themes richtet
nur `default` seine Füllfarbe am Farbschema aus."* **AK 6 hält die Achse fest,
auf der die Zusicherung fällt.**

Eigene Messung (`messungen/m1-deckung-je-theme.txt`, acht Themes × vier
Schemata, Sonde `deckung.cpp`), Spalte „= Window?":

| Desktop-Theme | folgt dem Farbschema? |
|---|---|
| `default` | **ja**, in allen vier Schemata |
| `breeze-dark` | nur unter BreezeDark |
| `breeze-light` | nur unter BreezeLight |
| `CachyOS-Nord-round` | nur unter CachyOSNordLightly |
| `Iridescent-round`, `cachyos-emerald`, `-color`, `-light` | **nie** |

Konkret unter dem Farbschema des Kunden (CachyOSNordLightly, `WindowText` ist
dort das Blau `#66c2f2`) und dem Desktop-Theme `breeze-light`: Fläche
`240,240,241`, Text `102,194,242` — **1,23:1**. Hellblau auf Weiß. Bild:
`bilder/breeze-light-hell.png`.

Und der Kundenbefund selbst wird von #83 **überhaupt nicht** berührt: K-A3
spricht von *Schrift*farben. Der Untersuchungsbericht hat gezeigt, dass die
Textrollen stimmen (F6) und woran der Eindruck wirklich hängt (F7: im
Ruhezustand trägt jeder sichtbare Text `PlaceholderText`, und CachyOSNordLightly
ist in sich uneinheitlich). Weder F7 noch der dortige Vorschlag steht in #83.
**#83 darf dem Kunden nicht als Antwort auf K-A3 vorgelegt werden.**

**Korrekturvorschlag:**
1. AK 6 auf beide Achsen stellen und den Messweg richtigstellen:
   > *Die Flächenfarbe folgt dem Farbschema unter dem Desktop-Theme `default`
   > über alle installierten Farbschemata (Messweg `native-ak2-kontrast.txt`),
   > **und** es ist über die installierten Desktop-Themes gemessen und im Issue
   > vermerkt, unter welchen sie es nicht tut (Messweg
   > `ux-beratung/messungen/m1-deckung-je-theme.txt`).*
   Die zweite Hälfte fordert keine Lösung, sondern eine benannte Grenze — das
   ist DoD 4 in der Fassung nach B9.
2. K-A3 bekommt ein eigenes Issue (F7-Vorschlag: App-Name auf `WindowText`
   statt `PlaceholderText`). Das ist eine Entscheidung des PO an Zeichnung 4b,
   wie der Untersuchungsbericht schon sagt.

### B-5 · „Das sollte KDE nativ sein" · **warn**

**Fundstelle:** keine — die Story antwortet darauf durch ihre Bauart.

§25.1 hält fest, dass kein Prüfmittel dieses Projekts den Kategorienfehler
gefunden hat und keines ihn gefunden hätte: *„Gefehlt hat die Frage, zu welcher
Familie das Fenster gehört — und die stellt nur, wer das Fenster neben andere
Fenster stellt und hinsieht."* Genau diese Prüfform steht in keinem AK.

**Korrekturvorschlag:** ein AK, das die Prüfform des Kunden übernimmt.
> *Zur Abnahme liegt ein Bild aus der angemeldeten Sitzung bei Skalierung 1,6
> vor, das das Erfassungsfenster **und** eine native Plasma-Überlagerung
> (KRunner) nebeneinander zeigt. Beide tragen dieselbe Eckform, dasselbe
> Randmaß und dieselbe Deckung.*

Das kostet einen Bildschirmschuss und ist der einzige Prüfsatz, der den Fehler
von #55 gefunden hätte. Er passt zu B21 (DoD 3), weil er über Hülle, Rundung und
Dekoration spricht.

---

## 2. Frage 2 — was fehlt aus UX-Sicht

Die acht AK sprechen über **einen** Zustand: ein stehendes Fenster, einmal
gezeichnet. Das Erfassungsfenster hat mehr davon, und SPEC 3.2 zählt vier
Bedingungen auf, die beim Bau von #55 entdeckt wurden — die AK-Liste von #83
nimmt keine davon auf.

| # | Zustand oder Übergang | AK | Verdikt |
|---|---|---|---|
| B-6 | das zweite Öffnen | keines | **fail** |
| B-7 | das mitwachsende Fenster | keines | **fail** |
| B-8 | Theme-Wechsel im Betrieb | keines | **fail** |
| B-9 | ohne Compositor, außerhalb Plasma | keines | **fail** |
| B-10 | der Ruhezustand (Leerfall) | keines | **fail** |
| B-11 | Theme ohne Rundung | AK 7 | **ok** |
| B-12 | AK 4 „dasselbe Bild" — welches Bild? | AK 4 | **warn** |
| B-13 | AK 5 nennt eine von drei nativen Anmeldungen | AK 5 | **fail** |

### B-6 · Das zweite Öffnen · **fail**

**Fundstelle:** AK 5 — *„Der Weichzeichner ist über `enableBlurBehind` mit der
Maskenregion angemeldet."* Vergleiche `capturewindow.cpp:419-429`
(`CaptureWindow::present()`) und SPEC 3.2 Punkt 5.

SPEC 3.2 Punkt 5 steht als entdeckte Bedingung im Bestand: *„Der Schatten wird
nach jedem Neuzeigen neu gebunden. Vor jedem Zeigen wird das Fenster neu
gemappt, und die Wayland-Surface verschwindet dabei; ein einmal im Konstruktor
gebundener Schatten wäre nach dem ersten Verstecken weg."* Der Kommentar im Code
sagt dazu: *„This is the line no test of this project would notice missing — it
shows only on the second opening."*

`enableBlurBehind` hängt am selben `QWindow`. Ob die Anmeldung das Neumappen
übersteht, weiß ich nicht — ich habe es nicht gemessen, und es ist eine
Messfrage, keine Meinungsfrage. Der zweite Bearbeiter hat eine Sonde namens
`weichzeichnerbeleg`; möglicherweise steht die Antwort schon in seinem Bericht.

**Korrekturvorschlag:** AK ergänzen.
> *Der Weichzeichner ist auch beim **zweiten** und jedem weiteren Öffnen
> angemeldet. Prüfweg wie beim Schatten (`CaptureWindow::present()`), Nachweis
> mit einer Testfolge Öffnen → Verstecken → Öffnen.*

Ohne diesen Satz ist der Fall gebaut, in dem ein Kunde beim ersten Öffnen ein
weichgezeichnetes Fenster sieht und danach nie wieder — genau die Falle, die
SPEC 3.2 Punkt 5 schon einmal beschrieben hat.

### B-7 · Das mitwachsende Fenster · **fail**

**Fundstelle:** SPEC 3 — *„Starthöhe ~5 Zeilen, wächst mit dem Text bis ~8
Zeilen"*; `capturewindow.cpp:462-471` (`adjustHeight()` ruft `resize()`).

Die Region des Weichzeichners **ist die Maske** (Issue #83, Abschnitt „Was
dazukommt"). Die Maske hängt an der Fenstergröße. Das Fenster ändert seine Größe
**während es sichtbar ist**, bei jedem Zeilenumbruch. Die AK-Liste sagt kein
Wort dazu.

Was ein Nutzer sonst sieht: Er tippt, das Fenster wächst nach unten, und der
untere Streifen ist nicht weichgezeichnet — oder, umgekehrt beim Löschen, ein
weichgezeichneter Streifen steht unter dem Fenster. Das ist ein sichtbarer
Fehler in der Hauptbewegung dieses Fensters.

**Korrekturvorschlag:**
> *Die Region des Weichzeichners folgt jeder Größenänderung des Fensters —
> belegt an einer Folge, die das Fenster von fünf auf acht Zeilen wachsen und
> wieder schrumpfen lässt.*

Dieselbe Frage stellt sich für die Polsterung des Schattens; die ist heute in
`bindShadow()` an die Ränder gebunden und wird bei `resizeHull()` nicht neu
gesetzt. Das ist Bestand, kein Zugang durch #83 — **melden, nicht heilen.**

### B-8 · Theme-Wechsel im Betrieb · **fail**

**Fundstelle:** SPEC 3.2 Punkte 1–3, `capturewindow.cpp:223-252`
(`reloadDesktopTheme()`).

Das Projekt hat sich in #55 erheblich Mühe gegeben, dass ein Theme-Wechsel ein
stehendes Fenster erreicht (KDirWatch statt KConfigWatcher, frisches ImageSet
statt Umbenennen). Mit dem Theme ändern sich Randmaß, Eckform **und Deckung** —
gemessen von 2,7 % bis 100 % über die acht installierten Themes
(`m1-deckung-je-theme.txt`). Die Region des Weichzeichners muss also beim
Theme-Wechsel neu gesetzt werden, und falls ein Auswahlpfad gesetzt wird (B-9),
auch der.

**Korrekturvorschlag:**
> *Nach einem Wechsel des Desktop-Themes im laufenden Betrieb stimmen Hülle,
> Weichzeichner-Region und Schattenkacheln mit dem neuen Theme überein — belegt
> unter zwei Themes mit verschiedenem Randmaß **und verschiedener Deckung**
> (z. B. `default` mit 85 % gegen `CachyOS-Nord-round` mit 100 %).*

### B-9 · Ohne Compositor, außerhalb einer Plasma-Sitzung · **fail**

**Fundstelle:** SPEC 3.2 Punkt 4 — *„Ohne Desktop-Theme keine Hülle, aber ein
brauchbares Fenster. Das Fenster bleibt dann deckend und bedienbar."*

Diese Zusicherung fällt mit dem nativen Weg, und zwar unbemerkt. Heute füllt
`paintEvent()` bei ungültiger Hülle jeden Bildpunkt deckend
(`capturewindow.cpp:314-320`). Nimmt man die Theme-Grafik unverändert, ist das
Fenster **durchsichtig, auch wenn kein Compositor da ist, der weichzeichnet**.
Der Fall ist nicht exotisch: er tritt bei abgeschaltetem Compositor auf und bei
jedem Betrieb außerhalb Plasma.

**KDE hat dafür einen nativen Weg, und er ist keine Anpassung.**
`KSvg::ImageSet::setSelectors()` — Kopfdatei
`/usr/include/KF6/KSvg/ksvg/imageset.h:98-108`:

> *„The Plasma desktop for instance uses `opaque` or `translucent` based on
> presence of compositing and KWin blur effects."*

Gemessen (`m3-effektangaben.txt`), welche Themes eine deckende Fassung
mitbringen:

| Theme | Auswahlpfade vorhanden | Deckung ohne / mit `opaque` |
|---|---|---|
| `default`, `breeze-dark`, `breeze-light` | `opaque/`, `solid/`, `translucent/` | 84,7 % → **100 %** |
| die drei `cachyos-emerald` | `opaque/`, `solid/`, `translucent/` | 2,7 % → 3,5 % |
| `CachyOS-Nord-round` | keine | 100 % → 100 % |
| `Iridescent-round` | keine | 20,0 % → 20,0 % |

Für die Breeze-Familie — und damit für den eingestellten Stand des Kunden, seine
`plasmarc` nennt kein Theme und fällt auf `default` — trägt der Auswahlpfad
vollständig. Für die anderen nicht. Auch das ist eine benennbare Grenze.

**Korrekturvorschlag:**
> *Ohne verfügbaren Weichzeichner (`KWindowEffects::isEffectAvailable(BlurBehind)`
> ist falsch) wird der Auswahlpfad `opaque` gesetzt. Belegt unter `default`:
> Deckung 100 % statt 84,7 %. Wo ein Theme keine deckende Fassung mitbringt,
> bleibt es bei der durchscheinenden — das ist im Issue vermerkt, nicht
> behoben.*

### B-10 · Der Ruhezustand · **fail**

**Fundstelle:** keine — kein AK spricht über den Zustand ohne getippten Text.

Das ist der Zustand, den der Kunde **zweimal fotografiert** hat
(`kundenbilder/01`, `kundenbilder/02`). In ihm zeigt das Fenster ausschließlich
gedämpfte Texte: „Denkzettel", „Gedanke festhalten …", die Fußzeile — alle drei
`PlaceholderText` (F7). Der Notiztext in `WindowText`, über den die ganze
Kontrastrechnung von AK 2/#55 und der offene Punkt 1 von #83 sprechen, ist dort
**nicht sichtbar**.

Zahlen dazu in Abschnitt 3.2. Kurz: unter dem Schema des Kunden fällt der
gedämpfte Text von 2,91:1 deckend auf **1,79:1** durchscheinend.

**Korrekturvorschlag:** ein AK, das den Zustand benennt, den der Kunde sieht.
> *Der Kontrast wird für **beide** Textklassen ausgewiesen: `WindowText`
> (Notiztext, sichtbar nach dem ersten Zeichen) und `ForegroundInactive`
> (App-Name und Fußzeile, im Ruhezustand der einzige Text). Für die zweite
> Klasse ist eine Zahl anzugeben und die Grenze auszusprechen — behoben wird sie
> mit dieser Story nicht.*

Die letzte Halbzeile ist wichtig: Zeichnung 4b führt die Schwäche der Kleintexte
schon heute als *„Bekannte Grenze der Kleintexte"* und sagt ausdrücklich, dass
sie nicht gelöst wird. Der native Weg **verschlechtert** sie — das gehört
gemessen, nicht geheilt.

### B-11 · Theme ohne Rundung · **ok**

AK 7 deckt das ab, ein Prüf-Theme liegt vor (`pruef-theme/`), und der native Weg
ist dafür bereits gemessen (N2: null Stufen, Kontur ab Zeile 0 und Spalte 0, bei
Verhältnis 1 und 1,6). Nichts zu ergänzen.

### B-12 · AK 4 — welches Bild? · **warn**

**Fundstelle:** AK 4 — *„Offscreen und Wayland liefern dasselbe Bild. Das ist
neu und ausdrücklich zuzusichern."*

Der Satz nennt seinen Gegenstand nicht, und der Unterschied ist gemessen
(`m6-plattformvergleich.txt`, Sonde `huellenbild.cpp`, beide Läufe bei
Pixelverhältnis 1, damit nur die Plattform verschieden ist):

| Gegenstand | offscreen gegen Wayland |
|---|---|
| die Hülle allein (`framePixmap()`) | **byteweise gleich**, alle vier geprüften Themes |
| dieselbe Hülle mit Text darauf | **verschieden**, alle vier |

Wo die Unterschiede sitzen, ist ebenfalls gemessen: 1.587 von 154.440
Bildpunkten, sämtlich in den Zeilen 49–190 und Spalten 47–416 — der Textbereich,
nicht der Rand der Hülle. Es ist die Rasterung der Schrift, und die hängt an
Fontconfig, nicht an KSvg.

AK 4 wörtlich genommen ist damit **unerfüllbar**, sobald das „Bild" ein
gegrabbtes Fenster ist. Und die Bildbelege dieses Projekts sind gegrabbte
Fenster.

**Korrekturvorschlag:**
> *Die **Hülle** — `framePixmap()` bei gleichem Maß und gleichem
> Bildpunktverhältnis — ist offscreen und unter Wayland byteweise gleich.
> Belegt über mindestens zwei Desktop-Themes. **Für ein gegrabbtes Fenster gilt
> das nicht**: die Schriftrasterung weicht offscreen ab (gemessen, 1.587
> Bildpunkte, sämtlich im Textbereich). Bildvergleiche ganzer Fenster über
> Plattformgrenzen hinweg sind deshalb kein Prüfmittel.*

Der letzte Satz ist mehr wert als der erste: Er verhindert, dass jemand die
Zusicherung mit einem Fenstervergleich zu belegen versucht und an der Schrift
scheitert.

### B-13 · AK 5 nennt eine von drei nativen Anmeldungen · **fail**

**Fundstelle:** AK 5 — *„Der Weichzeichner ist über `enableBlurBehind` mit der
Maskenregion angemeldet."*

Am Binärcode gemessen (`m4-effektanmeldungen.txt`), was `libPlasmaQuick` — die
Bibliothek hinter Plasmas eigenen Überlagerungen — beim Fenstersystem anmeldet:

- `KWindowShadow` samt Kacheln und Polsterung → **hat Denkzettel schon**
- `KWindowEffects::enableBlurBehind` → **AK 5**
- `KWindowEffects::enableBackgroundContrast` → **fehlt**

`KSvg` selbst meldet nichts davon an (kein Treffer). Die Kopfdatei von
`KWindowEffects` (Zeile 80–84) sagt, wofür der dritte Aufruf da ist:

> *„Instructs the window manager to modify the color of the background […]
> **in order to improve the contrast and readability of any text in the
> translucent window**."*

Und die Werte dafür kommen aus dem Theme: `Plasma::Theme`
(`/usr/include/Plasma/plasma/theme.h:192-262`) liest sie aus der Gruppe
`[ContrastEffect]` der Theme-Metadaten — `enabled`, `contrast`, `intensity`,
`saturation`. Dazu `[BlurBehindEffect] enabled` und `[AdaptiveTransparency]`.

**Vier theme-gesteuerte Schalter, und #83 nennt einen.**

Wie sehr das trägt, zeigt der Abgleich: Genau die Themes, deren Grafik fast
nichts füllt, sind die, die einen `[ContrastEffect]` fordern
(`m3-effektangaben.txt`):

| Theme | Deckung | `[ContrastEffect]` |
|---|---|---|
| `Iridescent-round` | 20,0 % | `enabled=true, contrast=1.0, intensity=0.45, saturation=1.4` |
| `cachyos-emerald` | 2,7 % | `enabled=true, contrast=1.0, intensity=0.40, saturation=1.4` |
| `cachyos-emerald-color` | 2,7 % | `enabled=true, contrast=1.0, intensity=0.40, saturation=3` |
| `cachyos-emerald-light` | 2,7 % | **keiner** — nur `[AdaptiveTransparency]` |
| `default`, `breeze-*`, `CachyOS-Nord-round` | 84,7 / 100 % | keiner |

Diese Themes sind nicht kaputt. Sie sind darauf gebaut, dass der Compositor den
Grund hinter ihnen abdunkelt, und sie schreiben in ihre eigene Beschreibung, wie
stark. Wer nur zeichnet und weichzeichnet, bekommt unter ihnen ein Fenster ohne
Fläche — Bild `bilder/cachyos-emerald-hell.png`, dort steht der Notiztext auf
dem Hintergrundmuster.

**Das ist der Kern der ganzen Vorprüfung:** „ohne Anpassungen" heißt, den
nativen Vertrag **ganz** zu erfüllen. Die AK-Liste erfüllt ein Drittel davon,
und die fehlenden zwei Drittel sind ausgerechnet der Lesbarkeitsmechanismus.

**Korrekturvorschlag:**
> *Neben dem Weichzeichner wird der Kontrasteffekt angemeldet
> (`KWindowEffects::enableBackgroundContrast`), mit den Werten des eingestellten
> Desktop-Themes aus `[ContrastEffect]`; fehlt die Gruppe, wird er nicht
> angemeldet. Belegt unter `default` (ohne Gruppe → nicht angemeldet) und unter
> `cachyos-emerald` (mit Gruppe → angemeldet, `intensity=0.40`).*

**Offene Frage an PO und Dev, nicht von mir zu entscheiden:** Die Werte stehen
in `Plasma::Theme`, also in `libPlasma` — einer Abhängigkeit, die Denkzettel
heute nicht hat (es benutzt `KSvg`, das die Werte nicht führt). Entweder man
nimmt die Abhängigkeit, oder man liest die vier Schlüssel selbst aus der
Theme-Metadatei. Das ist eine Bauentscheidung mit Umfangswirkung und gehört in
den Vorprüfbericht des zweiten Bearbeiters, nicht in meinen.

---

## 3. Frage 3 — der Textkontrast

### 3.1 Was gemessen ist, und wo die vorliegende Zahl zu eng ist

Meine Rechnung reproduziert die Sprint-6-Messung, bevor sie sie erweitert: über
19 Schemata stimmen meine Werte mit `native-ak2-kontrast.txt` auf ±0,02 überein
(KritaNeutral 4,74 / 3,59 gegen dort 4,74 / 3,57; CachyOSNordLightly 7,93 / 4,88
gegen dort 7,93 / 4,88). Die Grundlage steht.

**Was daran zu eng ist:** Die 84,7 % gehören dem Desktop-Theme `default`, nicht
dem nativen Weg. Über die acht installierten Themes reicht die Deckung von
**2,7 % bis 100 %** (`m1-deckung-je-theme.txt`). Die Aussage „die native Hülle
deckt zu 84,7 %" gilt für drei von acht.

### 3.2 Der Zustand, in dem der Kunde hinsieht

`m2-kleintexte.txt` rechnet über alle 19 Schemata, was die AK-Liste nicht
rechnet: nicht `WindowText`, sondern `ForegroundInactive` — die Rolle von
App-Name und Fußzeile, im Ruhezustand der einzige Text im Fenster.

| | deckend | durchscheinend (84,7 %, ungünstigster Grund) |
|---|---|---|
| `WindowText`, schlechtestes Schema | 4,74:1 | **3,57:1** — 5 von 19 unter 4,5:1 |
| `ForegroundInactive`, schlechtestes Schema | 1,98:1 | **1,50:1** — **16 von 19** unter 4,5:1, **15** unter 3,0:1 |
| `ForegroundInactive` beim Kunden (CachyOSNordLightly) | 2,91:1 | **1,79:1** |

Die deckende Spalte ist nicht neu und steht seit dem 01.08.2026 als benannte
Grenze in Zeichnung 4b (Breeze 3,69:1, KritaNeutral 1,98:1 — meine Rechnung
liefert dieselben Zahlen). **Neu ist, dass der native Weg sie um eine Stufe
senkt, und zwar in dem Zustand, den der Kunde beanstandet hat.**

Die Fußzeile ist dabei kein Zierrat: *„Esc verwirft · Strg+Enter speichert"* ist
der einzige Tastenhinweis, den diese Anwendung hat — sie hat keinen Knopf und
kein Menü (SPEC 3). Nach WCAG ist sie damit kein „incidental text", und die
anzuwendende Schwelle ist 4,5:1, nicht die 3:1 für Großtext: beide Kleintexte
laufen in der *kleinsten lesbaren Systemschrift*.

Die KDE HIG nennt keine Kontrastzahl (Quelltext geprüft: `accessibility.md`
verlangt nur *„Change the system-wide color scheme […] to verify that everything
adapts as expected"*). Was sie an dieser Stelle **doch** sagt, steht in
`displaying_content.md:62` und trifft die Hülle:

> *„Whenever overlaying a popup, box, or dialog on top of the app's main content
> area, add a contrasting outline around the edge of the overlaid element.
> Without this, visual recognizability suffers when using a dark color scheme,
> and the popup can appear to blend into the background."*

Der native Weg liefert **keine** kontrastierende Kontur (N5: Kante und Fläche
haben dieselbe Farbe, 7,5 % Deckungsunterschied). Das ist keine Verletzung, denn
die HIG spricht vom Inhaltsbereich der eigenen Anwendung — es ist aber genau die
Sorge, die der Kunde formuliert hat, und die Zeichnung schuldet dazu eine
Aussage (Abschnitt 4).

### 3.3 Entschärft der Weichzeichner es?

**Nein, nicht von sich aus** — und diese Antwort ist Rechnung, nicht Messung,
darum steht sie als solche hier.

Ein Weichzeichner ist ein Tiefpass. Er lässt den Mittelwert einer Fläche stehen
und nimmt ihr die Struktur. Ein weißes Dokumentfenster hinter der Einblendung
bleibt nach jedem Weichzeichnen weiß; das Fenster ist 600 × 174 Punkte groß und
damit weit größer als jeder Weichzeichnerkern, sein Inneres sieht also den
unveränderten Mittelwert. Was der Weichzeichner leistet, ist echt und wichtig:
Ein Foto oder ein Text hinter der Einblendung hört auf, mit den Buchstaben zu
konkurrieren. Was er nicht leistet, ist ein Anheben des Kontrasts.

Was den Kontrast anhebt, ist der **zweite** Aufruf — `enableBackgroundContrast`
mit `intensity`, laut Kopfdatei genau dafür gebaut. Von den vier Themes, unter
denen die Hülle fast nichts füllt, verlangen ihn **drei** ausdrücklich (B-13);
`cachyos-emerald-light` verlangt ihn nicht und bliebe auch mit ihm bei 2,7 %
Deckung — eine Grenze, die zu benennen und nicht zu heilen ist.

**Was ich nicht gemessen habe:** die Ausgabe von KWins Weichzeichner selbst. Der
zweite Bearbeiter hat eine Sonde `weichzeichnerbeleg` und Bilder
`weichzeichner-*.png`; wenn dort eine Messung steht, gilt sie vor meiner
Rechnung.

### 3.4 Wie diese Frage dem Kunden vorzulegen wäre

Der Kunde entscheidet. Meine Aufgabe ist, ihm die Entscheidung so vorzulegen,
dass sie eine ist. Vorschlag im Wortlaut:

---

> **Die Durchsichtigkeit — eine Entscheidung, die zu „ohne Anpassungen" gehört**
>
> Eine native Plasma-Überlagerung ist durchsichtig. Das gehört zur Bauart:
> KRunner und die Aufklapper sind es auch. Wie stark, entscheidet Dein
> Desktop-Theme. Gemessen an den acht Themes auf Deiner Maschine:
>
> | Desktop-Theme | Deckung der Hülle |
> |---|---|
> | Breeze (Dein eingestelltes), Breeze hell, Breeze dunkel | 85 % |
> | CachyOS-Nord-round | 100 % |
> | Iridescent-round | 20 % |
> | die drei cachyos-emerald | 3 % |
>
> **Unter Deiner eigenen Einstellung ist der Notiztext in Ordnung:** 4,88:1,
> auch wenn ein weißes Fenster dahintersteht — über dem Mindestwert von 4,5:1.
> Da ist nichts zu entscheiden.
>
> Zwei Dinge sind zu entscheiden.
>
> **Erstens die beiden kleinen Texte** — „Denkzettel" oben und der Tastenhinweis
> unten. Sie sind unter Deinem Farbschema schon heute blass (2,91:1); mit einer
> durchsichtigen Hülle werden 1,79:1 daraus. Solange Du nichts getippt hast,
> sind sie der einzige Text im Fenster — es ist genau das Bild, das Du zweimal
> fotografiert hast. Anheben lässt sich das nur, indem wir eine Farbe wählen,
> die das Schema nicht vorsieht, und das wäre eine Anpassung.
>
> **Zweitens die vier durchsichtigen Themes.** Sie zeichnen fast keine Fläche,
> weil sie darauf gebaut sind, dass der Bildschirmverwalter den Grund hinter dem
> Fenster abdunkelt — sie schreiben in ihre eigene Beschreibung, wie stark.
> Melden wir das an, sieht Denkzettel unter ihnen aus wie jede andere
> Plasma-Überlagerung. Melden wir es nicht an, steht der Text auf dem
> Hintergrundbild (Bild `cachyos-emerald-hell.png`).
>
> Drei Wege:
>
> **A — nativ, ganz.** Weichzeichner **und** Abdunkeln anmelden, wie Plasmas
> eigene Überlagerungen es tun; ohne Bildschirmverwalter auf die deckende
> Fassung umschalten, die die Breeze-Themes mitbringen. Nichts davon ist ein
> Nachbau, es ist der Rest desselben nativen Wegs. Kostet Umfang.
>
> **B — nativ, aber deckend.** Immer die deckende Fassung nehmen. Auch das ist
> nativ — der Auswahlweg dafür stammt von Plasma selbst. Das Fenster ist dann
> nicht mehr durchsichtig und sieht damit anders aus als KRunner.
>
> **C — nur zeichnen und weichzeichnen.** Unter Deinem Theme sieht das gut aus.
> Unter vier der acht installierten Themes ist das Fenster kaum zu lesen.
>
> Ich empfehle nichts. Was sich sagen lässt: Weg A ist das, was „ohne
> Anpassungen" wörtlich bedeutet — Plasmas eigene Bibliothek meldet beide
> Effekte an, am Binärcode nachgesehen.

---

Warum in dieser Form: Der Kunde hat am 04.08. eine Kategorienfrage entschieden,
die ihm nie vorgelegt worden war (§25.1). Die Durchsichtigkeit ist die zweite
Frage derselben Art. Sie ihm als Messwert zu zeigen statt als Empfehlung ist
das, was aus dem Sprint-6-Befund folgt.

---

## 4. Frage 4 — was Zeichnung 4b künftig aussagen muss

Ich zeichne nichts; das ist ein eigener Schritt nach der Entscheidung des
Kunden. Was die neue Aussage tragen muss, sind sechs Punkte. Fünf betreffen 4b,
der sechste 4a — der lässt sich nicht trennen.

**A1 — Die Hülle hat keine Linie.** Heute steht in 4b unter „Farbrollen":
*„Kontur der Hülle — Mischung aus `Window` und `WindowText` im Verhältnis
`frameContrast` […] Es ist die **einzige Linie** im Fenster."* Gemessen ist
Kante gleich Fläche in der Farbe (30,35,51 gegen 30,34,51) und verschieden
allein in der Deckung (235 gegen 216 von 255). Die Zeichnung darf für diesen
Rand weder das Wort „Kontur" noch ein Farbverhältnis führen. **Was an die Stelle
tritt: ein Deckungsrand — ein Sprung in der Durchsichtigkeit innerhalb
derselben Farbe.**

**A2 — Der Rand ist nur sichtbar, solange die Hülle durchscheint.** Auf einer
deckenden Hülle gibt es ihn nicht. Damit hängt seine Sichtbarkeit an der
Deckung, und die Deckung gehört dem Desktop-Theme — gemessen 2,7 % bis 100 % über
acht Themes. **Die Zeichnung darf keinen sichtbaren Rand zusichern.** Sie kann
zusichern, dass der Rand der Form des Themes folgt, und die beobachtete
Spannweite nennen — wie sie es bei Randmaß und Eckform bereits tut („4 px und
8 px sind Messwerte, keine Vorgabe").

**A3 — Woher die Erkennbarkeit stattdessen kommt.** Die KDE HIG verlangt für
Überlagerungen eine kontrastierende Umrandung
(`displaying_content.md:62`). Der native Weg liefert keine. Die Zeichnung schuldet
an dieser Stelle eine Antwort und darf sie nicht auslassen: Erkennbarkeit trägt
dann der **Schatten** und der **Kontrasteffekt des Compositors** — oder sie ist
unter bestimmten Themes nicht zugesichert. Beides ist eine Aussage; keine
Aussage ist keine.

**A4 — Der Prüfsatz zum Notiztext bekommt eine Bedingung.** Heute: *„schlechtester
Fall über alle 18 Schemata 4,74:1 […] die Bedingung, unter der die durchgehende
Fläche trägt."* Auf einer durchscheinenden Hülle gilt die 4,74 nicht mehr
unbedingt — sie gilt **deckend**. Die Zeichnung muss den Grund nennen, gegen den
gerechnet wird, und die zweite Zahl dazu (3,57:1 auf ungünstigstem Grund, 5 von
19 Schemata unter 4,5:1).

**A5 — Die Kleintexte bekommen ihre eigene Zahl.** 4b führt sie heute als
*„Bekannte Grenze der Kleintexte"* mit deckenden Werten (Breeze 3,69:1,
KritaNeutral 1,98:1). Unter einer durchscheinenden Hülle sind es 2,60:1 und
1,50:1. Die Grenze bleibt benannt und ungelöst — aber mit der Zahl, die gilt,
und mit dem Hinweis, dass diese Texte im Ruhezustand die einzigen sind.

**A6 — In 4a fällt ein Satz.** Dort steht: *„Form und Schatten sind farbneutral
— die kommen vom Theme; alles Farbige kommt aus der Palette."* Unter dem nativen
Weg kommt die Füllfarbe aus der **Grafik**, und die folgt der Palette nur unter
`default` (B-4). Der Satz war für den Nachbau richtig und ist für den nativen Weg
falsch. Er trägt heute die gesamte Begründung von 4a und muss durch eine ersetzt
werden, die den gemessenen Bestand nennt: welche Themes dem Farbschema folgen
und welche nicht.

**Was 4b behalten kann:** die eine durchgehende Fläche, die Farbrollen für Text,
die Innenabstände, die Fußzeilen-Gliederung über Abstand, den Entfall der
Trennlinie und den Vermerk zu ihrem Entscheidungsweg. Davon berührt der native
Weg nichts.

---

## 5. Befundliste

| # | Gegenstand | Fundstelle | Verdikt |
|---|---|---|---|
| B-1 | AK 3 prüft eine Zeile, der Befund lief über Zeilen | AK 3 | warn |
| B-2 | heller Streifen am Bogen ohne Kriterium | „Offene Punkte" 2 | **fail** |
| B-3 | Schatten als Ausnahme statt als Kundenentscheidung | „Was nicht gelöst wird" | warn |
| B-4 | AK 6 misst 20 Schemata unter 1 Theme; Messweg-Datei fehlt | AK 6 | **fail** |
| B-5 | kein AK in der Prüfform, die #55 gefunden hätte | — | warn |
| B-6 | zweites Öffnen — Weichzeichner nicht zugesichert | AK 5, SPEC 3.2.5 | **fail** |
| B-7 | mitwachsendes Fenster — Region folgt der Größe nicht | AK 5, SPEC 3 | **fail** |
| B-8 | Theme-Wechsel im Betrieb — Region und Deckung | AK 5, SPEC 3.2.1–3 | **fail** |
| B-9 | ohne Compositor — SPEC 3.2.4 fällt, `opaque` fehlt | AK 5, SPEC 3.2.4 | **fail** |
| B-10 | Ruhezustand — keine Zahl für die Kleintexte | — | **fail** |
| B-11 | eckiges Theme | AK 7 | ok |
| B-12 | AK 4 nennt seinen Gegenstand nicht; für Fenster unerfüllbar | AK 4 | warn |
| B-13 | AK 5 nennt eine von drei nativen Anmeldungen | AK 5 | **fail** |
| B-14 | Zeichnung 4a/4b brauchen sechs neue Aussagen | 4a, 4b | warn |

**8 fail, 5 warn, 1 ok.** Aus UX-Sicht ist die AK-Liste in ihrer heutigen
Fassung **nicht ready**. Das Ready-Urteil fällt der Scrum Master (Feld 3), nicht
ich; ich liefere den Befund dazu.

Die acht `fail` zerfallen in zwei Gruppen, und die zweite ist die schwerere:

- **Zwei Kundenbefunde ohne Kriterium** (B-2, B-4). Sie können nach der Abnahme
  unverändert dastehen.
- **Sechs Stellen, an denen „nativ" unvollständig gefasst ist** (B-6 bis B-10,
  B-13). Sie haben alle dieselbe Wurzel: Die AK-Liste beschreibt, wie die Hülle
  **gezeichnet** wird, und der native Weg besteht zur Hälfte aus dem, was beim
  Fenstersystem **angemeldet** wird — Schatten, Weichzeichner, Kontrasteffekt,
  Auswahlpfad — und aus dem Nachziehen dieser Anmeldungen bei jedem Öffnen, bei
  jeder Größenänderung und bei jedem Theme-Wechsel.

---

## 6. Offene Punkte

1. **An den PO:** K-A3 wird von #83 nicht beantwortet. Es braucht ein eigenes
   Issue mit dem F7-Vorschlag (App-Name auf `WindowText`), und #83 darf dem
   Kunden nicht als Antwort darauf vorgelegt werden.
2. **An den Kunden:** die Vorlage aus 3.4 — Weg A, B oder C.
3. **An den Kunden:** der Schatten (B-3), offener Punkt 10 des
   Untersuchungsberichts, weiterhin unbeantwortet.
4. **An den zweiten Bearbeiter / Dev:** übersteht `enableBlurBehind` das
   Neumappen des Fensters? (B-6). Und: `Plasma::Theme` als Abhängigkeit oder die
   vier Schlüssel selbst lesen? (B-13).
5. **An den Scrum Master:** die Ablagekonvention für Vorprüfberichte (0.2).
6. **Bestandsbefund, melden statt heilen:** Die Polsterung des Schattens wird in
   `bindShadow()` gesetzt und bei `resizeHull()` nicht nachgezogen; das Fenster
   ändert seine Höhe im Betrieb. Das ist heutiger Stand, kein Zugang durch #83.
7. **Zahlenpflege:** `Win11OSDark` ist nicht mehr installiert; Messungen über
   „20 Schemata" laufen heute über 19. Nach B17 gehört das an die Aussage, nicht
   in die alte Messung.

---

## 7. Belege

Alles unter `docs/scrum/vorberichte/83-native-huelle/ux-beratung/`, ein Lauf
erzeugt sie neu:

```
bash docs/scrum/vorberichte/83-native-huelle/ux-beratung/pruefen.sh
```

| Datei | Inhalt |
|---|---|
| `messungen/m1-deckung-je-theme.txt` | 8 Desktop-Themes × 4 Farbschemata × 2 Auswahlpfade: Deckung, Flächenfarbe gegen `Window`, ungünstigster Kontrast |
| `messungen/m2-kleintexte.txt` | 19 Farbschemata: `WindowText` und `ForegroundInactive`, deckend und durchscheinend |
| `messungen/m3-effektangaben.txt` | `[ContrastEffect]`, `[BlurBehindEffect]`, `[AdaptiveTransparency]` und die Auswahlpfade je Theme |
| `messungen/m4-effektanmeldungen.txt` | was `libPlasmaQuick` beim Fenstersystem anmeldet, am Binärcode |
| `messungen/m6-plattformvergleich.txt` | Hülle allein gegen Hülle mit Text, offscreen gegen Wayland |
| `bilder/<theme>-{hell,dunkel}.png` | die Hülle über hellem und dunklem Grund, unter dem Farbschema des Kunden |
| `bilder/<theme>-nurhuelle.png` | dieselbe Hülle ohne einen Buchstaben |
| `sonden/deckung.cpp`, `huellenbild.cpp`, `kleintext.py` | die Quelltexte |

**Was diese Bilder belegen und was nicht (B21):** Sie sind offscreen entstanden
und belegen **Deckung, Flächenfarbe und Textkontrast** — Größen, die gerechnet
werden. Sie belegen **nicht** Hülle, Rundung, Kontur, Schatten oder Dekoration
in der angemeldeten Sitzung; dafür zeichnen Theme und Compositor, und offscreen
fehlt beiden die Grundlage. Dass der offscreen-Vorbehalt die Hülle selbst hier
nicht trifft, ist gemessen und nicht übernommen: `m6-plattformvergleich.txt`
zeigt die Hüllenbilder unter beiden Plattformen byteweise gleich.

**Und ein Bild aus der angemeldeten Sitzung fehlt mit Absicht:** Es gibt zu #83
noch keinen gebauten Stand. Ein solches Bild gehört zur Abnahme der Story, nicht
zu ihrer Vorprüfung — und B-5 verlangt es dort ausdrücklich.
