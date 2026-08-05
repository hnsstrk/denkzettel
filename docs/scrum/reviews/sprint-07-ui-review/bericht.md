# UI-Review Sprint 7 — #83, #71, #70

**Modus:** UI-Review (DoD 3) · **Rolle:** UI/UX · **Datum:** 05.08.2026 ·
**Geprüfter Stand:** `main`, `c488ab5` (beide Stränge zusammengeführt) ·
**Geprüfte Stories:** #83, #71, #70. **#72 ist nach Festlegung des PO keine
UI-Story dieses Sprints** und wird hier nicht unter DoD 3 geführt.

**Prüfstand** (eine Aussage gilt für einen Stand, B17): kwin 6.7.3, Wayland,
Bildschirm 2400×1350 logisch bei Bildschirm-DPR 2, Fensterverhältnis **1,6**;
Desktop-Theme `default` (Rückfall, `plasmarc` nennt keins), Farbschema des
Kunden, dunkel. Während der Prüfung ist `main` auf `29e2ed0` weitergelaufen;
an `src/`, `tests/` und `wireframes/` hat sich dabei nichts geändert, an
`SPEC.md` allein der Zusatz zu Befund K5 in Abschnitt 3.2. Die Befunde dieses
Berichts gelten damit unverändert für `29e2ed0`.

## Wie dieser Review geführt wurde

Alle Bilder unter `bilder/` sind in diesem Review entstanden, aus einem eigenen
Bau des Sprint-Standes. Der Bau liegt außerhalb des Repositoriums
(`…/scratchpad/build-ux`), damit `build/` der Wurzel unangetastet bleibt; die
Bildläufer wurden vor jedem Bildbeleg frisch gebaut. `ctest` auf diesem Bau:
**7 von 7 grün**, `librarytest` und `capturetest` eingeschlossen.

| Prüfmittel | Einstellung |
|---|---|
| Bibliotheksbilder | `libraryshots`, `QT_QPA_PLATFORM=offscreen`, `QT_QPA_PLATFORMTHEME=kde`, `QT_SCALE_FACTOR=1.6` |
| Erfassungsbilder offscreen | `captureshots`, dieselben Einstellungen |
| Erfassungsfenster in der Sitzung | Sonden `fensterlage` und `weichzeichner` aus `docs/scrum/reviews/sprint-07-s83-native-huelle/sonden/`, gegen den eigenen Bau gelinkt, **ohne `QT_SCALE_FACTOR`** — das Fensterverhältnis 1,6 kommt vom Compositor |
| Bildmessung | `messungen/bildvergleich.py`, läuft gegen die Bilder in `bilder/` |

**Warum ein Teil der Prüfung in der angemeldeten Sitzung stattfindet (B21):**
Hülle, Rundung, Kontur, Schatten und Durchsichtigkeit zeichnen Theme und
Compositor. Offscreen fehlt beiden die Grundlage, und ein offscreen erzeugtes
Bild belegt für #83 die entscheidenden Größen nicht. Für #70 und #71 geht es um
Rollwert, Zeilenlage und Auswahlzustand — dafür genügt offscreen.

Die Bilder der Stränge A und B sind gelesen worden und als Material verwendet;
sie ersetzen die eigenen nicht.

---

## 1. Die Frage, um die es bei #83 geht

Der Kunde hat die Vorgängerstory #55 mit der Frage abgelehnt, wie es sein kann,
dass das Fenster völlig anders aussieht als alle anderen Fenster. AK 12
verlangt deshalb ein Bild, das das Erfassungsfenster **neben KRunner** zeigt.
Das Urteil fällt der Kunde. Was hier steht, ist die fachliche Vorlage dafür.

**Eigenes Bild:** `bilder/ux-83-fenster-neben-krunner.png` (angemeldete Sitzung,
Fensterverhältnis 1,6, beide Fenster auf demselben glatten Untergrund vom
Grauwert 128). **Eigene Messung:** `messungen/ux-m2-bildvergleich.txt`.

| Größe | KRunner | Erfassungsfenster |
|---|---|---|
| Flächenfarbe | (47, 50, 52) | **(47, 50, 52)** |
| äußerster Bildpunkt der Kante | (40, 42, 45) | **(40, 42, 45)** |
| Schattenverlauf nach außen (Rotkanal) | 115·114·117·119·121·124·125·126·126·126·127·127·128 | **113·114·117·119·121·124·125·126·126·126·127·127·128** |
| Kantenlauf am Eckbogen | 5·3·2·1·1·0 (unten links) | 6·4·2·2·1·0 (oben links), 3·2·1·1·0 (unten links) |

**Was ich sehe:** Fläche und Kante sind bildpunktgleich. Der Schattenverlauf
ist ab dem zweiten Wert bildpunktgleich; der erste Wert weicht um zwei
Zählschritte ab, was der Lage des Fensters im Bildpunktraster entspricht. Der
Eckbogen läuft bei beiden Fenstern über fünf bis sechs Zeilen aus und beginnt in
Spalte 5 bis 6. Die Ecken unten am Erfassungsfenster laufen eine Zeile kürzer
aus als die oben; das Fenster ist 174 logische Bildpunkte hoch, bei 1,6 also
278,4 Gerätebildpunkte, und diese Viertelstelle sitzt an der Unterkante. Das
liegt weit unterhalb dessen, was am Bildschirm zu sehen ist.

**Der Unterschied, der bleibt, ist der gewollte:** KRunner setzt ein
umrandetes Eingabefeld in seine Hülle, das Erfassungsfenster hat eine
durchgehende Fläche ohne Kasten. Das ist die Entscheidung aus Wireframe 4b und
SPEC 3.1, getroffen am 01.08.2026 und seither unverändert. Zur Hülle selbst —
dem Gegenstand des Kundenbefunds — finde ich keinen messbaren Unterschied.

**Verdikt: pass.** Die Frage „sieht das aus wie ein Fenster von Plasma"
beantwortet die Messung mit ja; das Urteil bleibt beim Kunden.

---

## 2. #83 — Prüfpunkte aus Wireframe 4a und 4b

Die Prüfpunkte sind aus der Zeichnung abgeleitet, ein Punkt je gezeichnetem
Bereich und je Festlegung der beiden Tafeln.

| # | Prüfpunkt (Fundstelle) | Befund | Verdikt |
|---|---|---|---|
| 1 | 4a: Randmaß kommt aus dem Theme, zwei Themes unterscheiden sich | eigener Sitzungslauf: Innenrand links **16** unter `default`, **20** unter `CachyOS-Nord-round` (`ux-m1`) | **pass** |
| 2 | 4a: Radius kommt aus dem Theme | `cornerRun` **4** (`default`), **12** (`CachyOS-Nord-round`), **0** (eckiges Prüf-Theme); Bilder `ux-83-sitzung-ecke-*.png` | **pass** |
| 3 | 4a: Schatten aus den Schattenkacheln des Themes | oberste Kachel 32×16 (`default`), 20×24 (`Nord-round`), 12×6 (eckig); im Sitzungsbild derselbe Verlauf wie bei KRunner | **pass** |
| 4 | 4a: „Die Farbe kommt nicht aus dem Theme, sondern aus der Palette" | **Die Zeichnung sagt das Gegenteil des gebauten und spezifizierten Standes.** Siehe Befund W1 | **fail** (Zeichnung) |
| 5 | 4b: eine durchgehende Fläche, kein Kasten im Kasten | 97,88 % der Stichproben im Fensterinneren genau (47, 50, 52); Zeile 1080 und Spalte 960 durchgehend einfarbig | **pass** |
| 6 | 4b: Fläche folgt dem Farbschema (unter `default`) | AK 8 des Strangs: 0 von 19 Schemata über der Toleranz; im eigenen Sitzungsbild dieselbe Fläche wie KRunner | **pass** |
| 7 | 4b: Notiztext auf `WindowText`, Mindestwert 4,5 : 1 | Kundeneinstellung: Tinte (252, 252, 252) gegen Fläche (47, 50, 52) — **12,58 : 1** | **pass** |
| 8 | 4b: App-Name und Fußzeile auf `PlaceholderText` | **5,43 : 1** auf der Kundeneinstellung; die Rollenfrage selbst ist als #84 gebucht | **pass** (mit Verweis) |
| 9 | 4b: „Kontur der Hülle — Mischung aus `Window` und `WindowText`… die einzige Linie im Fenster" | **entfallen mit #83 AK 1.** Die Zeichnung führt eine Farbrolle, die es nicht mehr gibt. Siehe Befund W2 | **fail** (Zeichnung) |
| 10 | 4b: Fensterbreite 600 px | 960 Gerätebildpunkte bei 1,6 = **600 logisch** | **pass** |
| 11 | 4b: Innenabstand 12 seitlich zuzüglich Themerand | 16 unter `default` (4 px Rand), 20 unter `Nord-round` (8 px Rand) | **pass** |
| 12 | 4b: linke Textkante von App-Name, Notiztext und Fußzeile bündig | Notiztext beginnt 6 Gerätebildpunkte (**3,75 logisch**) weiter rechts als der App-Name. Bekannt und gebucht als **#81** | **warn** |
| 13 | 4b: Fußzeile mittig, mehr Luft über der Fußzeile als unter dem App-Namen | im Bild bestätigt; im Code 12 gegen 8 | **pass** |
| 14 | 4b: keine Trennlinie über der Fußzeile | keine vorhanden | **pass** |
| 15 | 4b: Starthöhe 5 Zeilen, wächst bis 8, danach Scrollbalken | 174 logisch bei fünf, 228 bei acht Zeilen; Scrollbalken im Bild `ux-83-acht-zeilen.png` | **pass** |
| 16 | 4b: Rundung und Rand sind keine Zahlen — die Zusicherung ist relativ | Punkte 1 und 2 belegen sie relativ, nicht absolut | **pass** |
| 17 | 4b: der Schatten braucht eine eigene Belegform (Sitzung) | in der Sitzung belegt, siehe Punkt 3 und Abschnitt 1 | **pass** |
| 18 | 4a: dieselbe Zeichnung unter zwei Themes, sonst gleich | Fensterhöhe wächst unter `Nord-round` von 174 auf 182 logisch, weil der Themerand breiter ist — das folgt aus 4b Punkt 11 und ist keine Abweichung | **pass** |
| 19 | AK 13: kein Bildpunkt am Bogen heller als der ungeschattete Grund | eigenes Sitzungsbild, drei Bögen geprüft: hellster Wert je **128**, der Grund ist 128. KRunner ebenso | **pass** |
| 20 | Lesbarkeit unter einem fremden Desktop-Theme | `breeze-dark` unter hellem Schema: Notiztext **1,04 : 1**. Siehe Befund P1 | **warn** |

### Zustände

Für das Erfassungsfenster gibt es zwei Zustände, und beide sind im Bild:
**Ruhe** (fünf Zeilen, leeres Feld, Platzhalter) und **gewachsen** (acht Zeilen
mit Scrollbalken). Einen Meldungszustand hat dieses Fenster nicht.

---

## 3. #71 — Prüfpunkte aus Wireframe 3a/3b und 2c, SPEC 9

**Eigene Bilder:** `ux-71-11a-klick-auf-angeschnittene-zeile.png`,
`ux-71-11b-nach-dem-nachlauf.png`, dazu die drei Zustände
`ux-bibliothek-01-normalfall.png`, `-02-leerzustand.png`,
`-03-meldungszustand.png`.

| # | Prüfpunkt | Befund | Verdikt |
|---|---|---|---|
| 21 | Der Klick wählt die geklickte Zeile | 11a zeigt die angeschnittene Zeile am unteren Rand markiert; der Detailbereich zeigt dieselbe Notiz | **pass** |
| 22 | Genau eine Zeile ist markiert | im Bild eine einzige Markierung | **pass** |
| 23 | Das Bild bleibt beim Klick stehen | in 11a steht die Zeile an ihrer Stelle | **pass** |
| 24 | SPEC 9: „Ein Mausklick bewegt die Liste überhaupt nicht" und „Eine angeschnittene Zeile bleibt nach dem Klick angeschnitten" | **Zwischen meinen eigenen Bildern 11a und 11b wandert die Markierung von y 882 auf y 767** — 115 Gerätebildpunkte, bei 1,6 also 71,9 logisch, eine Zeilenhöhe. Die Zeile steht danach ganz im Bild. Der Vorgang ist als **#89** gebucht; der Satz in SPEC 9 sagt heute mehr zu, als der Bau hält. Siehe Befund P2 | **warn** |
| 25 | 3b: Köpfe scrollen mit und kleben nicht oben | „Gestern" wandert zwischen 11a und 11b mit | **pass** |
| 26 | 2c Leerzustand 1: nur die Liste trägt den Platzhalter, der Detailbereich bleibt stumm | Bild 02: „Noch keine Notizen / Mit Meta+N einen Gedanken festhalten.", Detailbereich leer | **pass** |
| 27 | 2c Leerzustand 2: „Keine Notiz ausgewählt" im Detailbereich | Bild 04: „Keine Notiz ausgewählt / Zum Lesen links eine Notiz auswählen." | **pass** |
| 28 | 2c Meldungszeile: `KMessageWidget`, Typ Warning, ohne Schließen-Knopf, unter der Kopfzeile | Bild 03: „Notiz gelöscht — noch 5 s" mit Schaltfläche „Rückgängig", kein Schließen-Knopf, sitzt zwischen Kopfzeile und Inhalt | **pass** |
| 29 | 2c Raumaufteilung: Kopfzeile oben bündig, feste Höhe; Meldungszeile ohne Meldung ohne Höhe; Liste und Detail nebeneinander über die volle Resthöhe; Liste 300 px breit | in Spalte x = 6 gemessen: Fensterfarbe bis y = 76, ab y = 77 die Listenfläche — Kopfzeile **77 Gerätebildpunkte = 48,1 logisch**, und ohne Meldung beginnt die Liste unmittelbar darunter. Rechte Kante der Liste bei 480 Gerätebildpunkten = **300 logisch**; Liste und Detail reichen bis zur Unterkante | **pass** |
| 30 | 3a: keine Trennlinien zwischen Einträgen, kein Fettdruck im Eintrag | Bild 01: getrennt wird durch Weißraum; fett sind allein die Gruppenköpfe | **pass** |
| 31 | 3b: Zeitstempel folgt der Gruppe | Bild 01: Heute „14:32", Diese Woche „Di., 28. Juli", Älter „10.07.2026" | **pass** |

---

## 4. #70 — Prüfpunkte aus Wireframe 3b Fall 4 und SPEC 9

**Eigenes Bild:** `ux-70-szene7-kopf-im-bild.png` (Szene 7 des Bildläufers).

| # | Prüfpunkt | Befund | Verdikt |
|---|---|---|---|
| 32 | Erreicht die Auswahl per Taste die erste Notiz einer Gruppe, steht ihr Kopf im Bild | Szene 7: der Kopf „Letzte Woche" steht über der markierten ersten Notiz, beide vollständig sichtbar | **pass** |
| 33 | Die Auswahl steht nie ohne ihre Überschrift da (3b Fall 4) | im Bild erfüllt | **pass** |
| 34 | Der Klickpfad bleibt davon unberührt | 11a zeigt einen Klick auf eine Zeile ohne Kopfnachlauf | **pass** |
| 35 | 3b Fall 4 beschreibt allein den Grenzübertritt; die neue Regel fehlt in der Zeichnung | Siehe Befund W3 | **warn** (Zeichnung) |
| 36 | Die Zeichnungen tragen die Regel „ein Mausklick bewegt die Liste nicht" nirgends | Siehe Befund W4 | **warn** (Zeichnung) |

---

## 5. Beitrag zu #89 — der nachlaufende Autoscroll

Als Beitrag gedacht, nicht als Mangel an #71.

Meine beiden Bilder zeigen zwei Zustände, die je für sich in Ordnung sind:
unmittelbar nach dem Klick steht die Liste still und die geklickte Zeile ist
angeschnitten markiert (11a); eine halbe Sekunde später steht dieselbe Zeile
vollständig im Bild, weiterhin richtig markiert (11b). **Der Endzustand von 11b
ist der bessere von beiden** — eine vollständig sichtbare Auswahl liest sich
besser als eine angeschnittene.

Was daran stört, ist der Weg dorthin: eine Bewegung um eine volle Zeilenhöhe,
die eine halbe Sekunde nach der letzten Handlung des Nutzers ohne erkennbaren
Auslöser einsetzt. Die KDE HIG binden Bewegung an die Handlung, die sie auslöst,
und verlangen, dass sie erklärbar bleibt. Eine verzögerte Bewegung ohne Anlass
liest sich als Unruhe der Liste.

Damit liegen zwei in sich stimmige Zustände vor, zwischen denen zu entscheiden
ist:

- **Die Liste steht.** Das Autoscrollen des Views wird für diesen Weg
  abgeschaltet. Das entspricht SPEC 9 in der heute geschriebenen Fassung und
  ist die kleinere Änderung; der Preis steht schon im Issue — eine
  angeschnittene Zeile bleibt angeschnitten.
- **Die Liste rückt sofort und nur so weit wie nötig.** Das ist Lesart 3 aus der
  Vorprüfung (`ScrollPerPixel`): die Zeile wird ganz sichtbar, ohne unter dem
  Zeiger wegzurutschen. Sie ändert nebenbei das Verhalten des Mausrads, was
  niemand verlangt hat, und wäre eine eigene Story.

Der heutige Stand liegt zwischen beiden. Meine Empfehlung an den PO ist, dem
Kunden beide Bilder nebeneinander vorzulegen; die Frage ist in zwei Sekunden
beantwortet, und die Entscheidung ist seine.

---

## 6. Befunde für den Product Owner

Melden, nicht heilen — an Quellcode und an `wireframes/` ist für diesen Review
nichts geändert worden.

### Zeichnung

| # | Befund | Verdikt | Vorschlag |
|---|---|---|---|
| **W1** | **Wireframe 4a behauptet das Gegenteil des gebauten Standes.** Dort steht: „Die Farbe kommt nicht aus dem Theme, sondern aus der Palette" und „Form und Schatten sind farbneutral — die kommen vom Theme; alles Farbige kommt aus der Palette." Mit der Kundenentscheidung vom 04.08.2026 und #83 kommt die **Fläche selbst** aus der Theme-Grafik; unter `default` nimmt sie die Schemafarbe an, unter sechs von acht installierten Themes nicht. SPEC 3.1 ist nachgezogen, die Zeichnung nicht | **fail** | Absatz umdrehen, den Messwert (84,7 % Deckung unter `default`) und den Verweis auf #85 aufnehmen. Der alte Absatz gehört als datierter Vermerk stehengelassen, wie es 4b bei der Trennlinie vormacht |
| **W2** | **Wireframe 4b führt eine Farbrolle, die es nicht mehr gibt.** „Kontur der Hülle — Mischung aus `Window` und `WindowText` im Verhältnis `frameContrast` … Es ist die einzige Linie im Fenster." `frameContrast` ist mit #83 AK 1 entfallen; der Rand des Themes ist heute ein Deckungsrand (235 gegen 216 unter `default`) | **fail** | Farbrolle streichen, den Deckungsrand an ihre Stelle setzen. Die Maßtafel „Rundung und Rand" bleibt gültig |
| **W3** | **3b Fall 4 kennt die Regel aus #70 nicht.** Gezeichnet ist allein der Grenzübertritt. Die Erweiterung („erste Notiz einer Gruppe, ohne Grenze") steht in SPEC 9 und im Bau | **warn** | Einen Satz in Fall 4 aufnehmen. Das Issue hat den Punkt ausdrücklich als UX-Frage zurückgestellt |
| **W4** | **Keine Zeichnung trägt die Regel „ein Mausklick bewegt die Liste nicht".** Sie ist seit #57 tragend und mit #71 erweitert worden; die Festlegungstafel zu 3a/3b schweigt dazu | **warn** | Zeile in die Festlegungstafel zu 3a/3b, neben „Gruppenkopf ist keine Notiz" |
| **W5** | Der Vermerk unter 4a/4b, das Aufnahmefenster 1f erbe Hülle und Fläche und seine Zeichnung sei „noch nicht nachgezogen", steht seit dem 01.08.2026. Mit #83 ist der Abstand größer geworden | **warn** | Zur Kenntnis; eigene Gestaltungsaufgabe, wenn das Aufnahmefenster ansteht |

### Produkt und Prüfmittel

| # | Befund | Verdikt | Vorschlag |
|---|---|---|---|
| **P1** | **#85 ist schwerer, als seine Beschreibung sagt.** Das Issue spricht von „sechs von acht Themes schlechter lesbar als zuvor". Gemessen am eigenen Bild (`ux-83-fremdes-theme-notiztext-unsichtbar.png`, `breeze-dark` unter hellem Schema): der Notiztext steht bei **1,04 : 1** und ist damit nicht zu sehen, die Fußzeile bei 3,75 : 1. Auf der Einstellung des Kunden ändert sich nichts, weil dort `default` greift | **warn** | Die Zahl in #85 nachtragen. Sie ändert die Dringlichkeit, nicht den Umfang |
| **P2** | **SPEC 9 sagt für den Mausklick mehr zu, als der Bau hält.** Der Absatz sagt „Ein Mausklick bewegt die Liste überhaupt nicht" und „Eine angeschnittene Zeile bleibt nach dem Klick angeschnitten". Gemessen an den eigenen Bildern 11a und 11b wandert die Liste eine halbe Sekunde später um eine Zeilenhöhe, und die Zeile steht danach ganz im Bild. Strang B hat den Vorgang gemeldet (B-1, jetzt #89); dass der im selben Zug geschriebene SPEC-Satz ihn ausschließt, steht dort nicht | **warn** | Unabhängig vom Ausgang von #89 gehört die Bedingung in SPEC 9 (DoD 4). Wird #89 zugunsten der Ruhe entschieden, wird der Satz wieder wahr; wird er zugunsten des Nachlaufs entschieden, muss er ihn nennen |
| **P3** | **#81 ist am Sitzungsbild bestätigt.** Der Notiztext steht 3,75 logische Bildpunkte weiter rechts als App-Name und Fußzeile. Auf der einen durchgehenden Fläche ist die linke Textkante die einzige verbliebene Gliederung neben dem Abstand | **warn** | Bekannt und gebucht; die Bestätigung am Bild des neuen Standes ist der Zusatz |
| **P4** | **Die Weichzeichner-Sonde kann unbemerkt die falsche Stelle messen.** `sonden/weichzeichner.cpp:395` rechnet die erwartete Hüllenbreite aus `app.devicePixelRatio()` — das ist das Verhältnis des **Bildschirms** (2), nicht das des **Fensters** (1,6). Erwartet werden dadurch 1200 statt 960 Bildpunkte; das Suchband `[expected/2, expected·5/4]` reicht damit bis 1500, und gesucht wird der **längste** passende Lauf. In den Protokollen von Strang A steht dieselbe Zahl (`erwartet 1200`, gefunden 967) — dort ging es gut. In meinen vier eigenen Läufen ging es zweimal daneben: der längste Lauf lag bei 1500 Bildpunkten am unteren Bildschirmrand, und die Sonde schrieb daraus eine Spannweite von 81 beziehungsweise 62, die wie ein Ergebnis aussieht. Zweimal fand sie gar nichts | **warn** | Erwartete Breite aus dem Fensterverhältnis rechnen (`window->devicePixelRatio()`), das Band entsprechend enger ziehen und einen Wächter setzen, der abbricht, wenn der gefundene Lauf mehr als wenige Bildpunkte von der Fensterbreite abweicht. Bis dahin gilt: die Anmeldung des Weichzeichners ist über Strang As `m6` und die Sitzungsprobe S3 belegt, nicht über meine Läufe |

---

## 7. Was dieser Review nicht trägt

- **Die Wirkung des Weichzeichners** habe ich nicht selbst belegt. Vier eigene
  Läufe der Sonde haben die Hülle im Bild nicht gefunden (Befund P4); das
  Protokoll liegt als `messungen/ux-m3-weichzeichner-fehllauf.txt` bei. Was ich
  selbst gemessen habe: die Sitzung zeichnet weich (`ux-m1`), die Hülle ist
  durchscheinend (Alpha 216 in der Mitte, 235 am Rand) und sie ist im
  Sitzungsbild bildpunktgleich mit KRunner. Die Wirkung selbst steht auf
  Strang As `m6` und Mutationsprobe S3.
- **Die Wirkung von `enableBackgroundContrast`** ist auf diesem
  Compositor-Stand nicht zu beobachten; das ist als Impediment von Strang A
  gemeldet und hier bestätigt, nicht nachgeprüft.
- **Ein echter Mausklick.** Alle Belege zu #71 beruhen auf zugestellten
  Ereignissen. Das Issue nennt diese Grenze und legt sie in die Abnahme.
- **Der installierte Stand unter `/usr`.** Den taktet der PO; geprüft wurde am
  eigenen Bau des Sprint-Standes.
- **#72** ist nach Festlegung des PO keine UI-Story dieses Sprints und wurde
  nicht geprüft.

---

## 8. Bilder und Messungen dieses Reviews

**Bilder** (`bilder/`, alle in diesem Review erzeugt):

| Datei | Woher | Wozu |
|---|---|---|
| `ux-83-fenster-neben-krunner.png` | Sitzung, 1,6, Sonde `weichzeichner … krunner` | AK 12, Abschnitt 1 |
| `ux-83-sitzung-ecke-default.png` | Sitzung, Sonde `fensterlage` | Eckform unter `default` |
| `ux-83-sitzung-ecke-nord-round.png` | ebenda | Eckform unter `CachyOS-Nord-round` |
| `ux-83-sitzung-ecke-eckiges-theme.png` | ebenda | eckiges Theme, eckige Ecken |
| `ux-83-acht-zeilen.png` | `captureshots`, offscreen, 1,6 | gewachsener Zustand, Scrollbalken |
| `ux-83-fremdes-theme-notiztext-unsichtbar.png` | ebenda | Befund P1 |
| `ux-bibliothek-01-normalfall.png` | `libraryshots`, offscreen, 1,6 | Normalfall, Gliederung, Zeitstempel |
| `ux-bibliothek-02-leerzustand.png` | ebenda | Leerzustand |
| `ux-bibliothek-03-meldungszustand.png` | ebenda | Meldungszustand |
| `ux-bibliothek-04-nichts-ausgewaehlt.png` | ebenda | Leerzustand 2 aus 2c, Gruppe mit einem Eintrag |
| `ux-70-szene7-kopf-im-bild.png` | ebenda (Szene 7) | #70 |
| `ux-71-11a-klick-auf-angeschnittene-zeile.png` | ebenda (Szene 11a) | #71 |
| `ux-71-11b-nach-dem-nachlauf.png` | ebenda (Szene 11b) | #89 |

**Messungen** (`messungen/`):

| Datei | Inhalt |
|---|---|
| `ux-m1-fensterlage-sitzung.txt` | eigener Sitzungslauf: Fensterverhältnis, Kantenlauf, Region, Theme-Wechsel |
| `ux-m2-bildvergleich.txt` | Ausgabe von `bildvergleich.py` — alle Bildzahlen dieses Berichts |
| `ux-m3-weichzeichner-fehllauf.txt` | die beiden erfolglosen Läufe, Beleg zu Befund P4 |
| `bildvergleich.py` | das Messprogramm; läuft gegen die Bilder im Nachbarordner |
