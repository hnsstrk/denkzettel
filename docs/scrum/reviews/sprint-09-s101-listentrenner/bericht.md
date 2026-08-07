# Umsetzungsbericht #101 — Trenner in der Bibliotheksliste

**Story:** #101 „Bibliothek: Notizen und Gruppen heben sich optisch nicht
voneinander ab" (`epic:M2`, `typ:story`, `size:m`) ·
**Strang:** `story/101-listentrenner`, Worktree `/home/hnsstrk/Projekte/denkzettel-101` ·
**Boden:** `366b69f` · **Bearbeiter:** `denkzettel-dev`, 07.08.2026, Ganymed.

Maßgeblich sind die Akzeptanzkriterien in der Fassung des PO-Nachtrags vom
07.08.2026, 19:17 (AK 1, 2, 3a, 3b, 3c, 4, 5, 6, 7, 10), und der
Vorprüfbericht `docs/scrum/vorberichte/101-listentrenner/bericht.md` samt
seinem Abschnitt 8.

**Alles hier Behauptete ist wiederholbar:** `bash docs/scrum/reviews/sprint-09-s101-listentrenner/pruefen.sh`
baut von Grund auf, fährt Prüfsätze, Linter und Bilder; `bash …/mutationsprobe.sh`
nimmt sechsmal je eine Zusicherung aus dem Produktivcode heraus und zeigt, dass
sie einzeln bemerkt wird — **und endet mit einem Rückgabewert ungleich null,
wenn eine Probe nichts gemessen hat** (3.1). Beide bauen ausschließlich in
eigenen Verzeichnissen und installieren nichts nach `/usr`.

---

## 1. Was gebaut ist

**Drei Stellen im Produktivcode, keine vierte.**

| Datei | Was |
|---|---|
| `CMakeLists.txt` | `find_package(KF6ColorScheme 6.20 REQUIRED)` als **eigener** Aufruf. `KF_MIN_VERSION` `:6` steht unverändert auf `6.0.0`, die Komponentenliste ist unberührt (Kundenentscheidung 07.08.2026, Falle 2.6). |
| `src/CMakeLists.txt` | `KF6::ColorScheme` an `denkzettelui`. |
| `src/ui/notelistdelegate.{h,cpp}` | Zwei Zeichenstellen und zwei Hilfsfunktionen: `separatorColor()` mischt Listengrund und Textfarbe im Verhältnis `KColorScheme::frameContrast()`, `isSelectedIn()` fragt die Ansicht nach der Auswahl einer **anderen** Zeile. Die Gruppenlinie steht im Kopfzweig **vor** dem `return`, die Eintragslinie im Notizzweig **nach** `style->drawControl(...)` (Falle 2.8). |
| `src/ui/librarywindow.{h,cpp}` | `repaintTheRowAbove()` und zwei Aufrufe am Anfang von `showNote()` — für `index` und für `previous` (AK 3c, Falle 2.2). |

**Kein neues Maß.** Beide Linien liegen in Innenabständen, die es schon gab:
die eingerückte im unteren der 9 px eines Eintrags, die volle im oberen der
14 px über einem Kopf. `sizeHint()` ist nicht angefasst.

**Kein Modell, kein Proxy, keine Kopplung an das Fenster.** Die Art der
Nachbarzeile liefert `GroupHeaderRole`, die Auswahl der Nachbarzeile
`option.widget` — ein `qobject_cast` und eine `nullptr`-Wache, wie beide
Vorprüfungen gemessen hatten.

### Zwei Entscheidungen, die ich getroffen habe

**Die Eintragslinie entfällt an beiden Kanten der Auswahl, die Gruppenlinie
nicht.** Das ist AK 3a/3b, hier nur als Bauform benannt: Der Notizzweig prüft
`!selected && !isSelectedIn(entry, below)`, der Kopfzweig prüft nichts
dergleichen.

**`repaintTheRowAbove()` steht vor allen Wachen von `showNote()`, auch vor
`m_restoringSelection`.** Die Wache dort hält die *Rückfrage* auf, nicht das
Zeichnen; ein Neuzeichnen stellt keine Frage und kostet nichts, wenn sich
nichts ändert. Läge der Aufruf hinter der Wache, bliebe der Weg zurück aus
einem abgebrochenen Wechsel ungezeichnet.

---

## 2. Die Kriterien, und womit sie belegt sind

| AK | Erfüllt | Beleg |
|---|---|---|
| **1** — Lage der Eintragslinie | ja | `drawsAnInsetHairlineBetweenTwoNotesOfAGroup`, zwei Fenstergrößen, Skalierung 1. Gelesen wird die letzte Bildpunktzeile bei x = 0, 11, 12, Mitte, Breite−13, Breite−12, Breite−1: `Grund Grund Linie Linie Linie Grund Grund`. Zwei Notizgrenzen im selben Aufbau. |
| **2** — Lage der Gruppenlinie | ja | `drawsAFullWidthHairlineOverEveryGroupHeadButTheFirst`. Oberste Bildpunktzeile des zweiten und dritten Kopfes, **x = 0 und x = Breite−1 eingeschlossen**, überall Linie; über dem ersten Kopf überall Grund. |
| **3a** — wo keine Linie steht | ja | `leavesTheEntryLineOutWhereTheRankingDoesNotAskForIt`. Drei Fälle: unter der letzten Notiz jeder Gruppe, unter jedem der drei Köpfe, an beiden Kanten der ausgewählten Zeile. Die Auswahl wird als **Vorher/Nachher derselben zwei Bildpunktzeilen** gemessen — und der Prüfsatz sichert vorher zu, dass ohne Auswahl dort eine Linie steht, sonst misst er nichts. |
| **3b** — die Ausnahme gilt allein der Eintragslinie | ja | `keepsTheGroupLineOverAHeadUnderTheSelectedNote`: Auswahl auf die letzte Notiz der mittleren Gruppe, die Linie über dem Kopf darunter steht weiter, x = 0 bis Breite−1. |
| **3c** — der Auswahlwechsel | ja | `paintsBothUpperNeighboursAgainWhenTheSelectionMoves`. **Malzähler, kein Bild** — ein `PaintCounter`, der von `NoteListDelegate` erbt und ihn ruft. Vier Wechsel: 3→5, 5→3, 4→5, 5→4; in dreien liegt ein oberer Nachbar außerhalb der Strecke, die die Ansicht von sich aus malt. Geprüft wird für jeden, dass **beide** oberen Nachbarn neu gezeichnet sind. |
| **4** — Farbe | ja | `mixesTheSeparatorOutOfGroundAndTextInsteadOfTakingAPaletteRole`, zwei Paletten (Breeze hell und dunkel), deren Ergebnis weit auseinanderliegt. Das Verhältnis kommt aus `[KDE]` der Sandkasten-Konfiguration und wird **vor jeder Messung zurückgelesen und zugesichert** (Falle 2.5). Zusätzlich zwei Gegenproben: der gewählte Wert 0,45 muss ein anderes Ergebnis liefern als die Voreinstellung 0,20, und die Linienfarbe wird gegen **alle** `QPalette`-Rollen gehalten. Der Sandkasten wird am Ende des Laufs zurückgesetzt (4.3). |
| **5** — kein Maß ändert sich | ja | `keepsTheMeasuresOfTheGroupedList` (beide Größen) und die fünf #70-Sätze `bringsTheHead…` sind **unverändert** grün, `messungen/b3-pruefsaetze.txt`. Am Code: `sizeHint()` ist nicht angefasst. |
| **6** — Trefferliste der Suche | ja | `separatesTheSearchResultsLikeTheLibrary`: nach dem Filtern behält „Heute" eine, „Gestern" zwei Notizen. Über dem ersten sichtbaren Kopf keine Linie, über dem zweiten eine volle, unter den letzten sichtbaren Notizen keine, zwischen den zwei verbliebenen Notizen die eingerückte. Bild: `bilder/skalierung-1/2-trefferliste-mit-gruppen.png`. **Kein Eingriff am Produktivcode nötig**, wie die Vorprüfung gemessen hatte. |
| **7** — Bild unter der Skalierung des Kunden | ja | `bilder/skalierung-1-6/01-normalfall.png`, erzeugt mit `QT_SCALE_FACTOR=1.6`, `QT_QPA_PLATFORMTHEME=kde`, offscreen; 1440×960 Bildpunkte auf 900×600 logischen. Ansichtsbeleg, **keine Bildpunktzusicherung**. Daneben `09-ruhiges-bild-innerhalb-der-gruppe.png` unter derselben Skalierung: **im Normalfall ist die zweite Notiz die ausgewählte**, und damit entfällt gerade die eine eingerückte Linie, die er zeigen könnte — er belegt allein die Gruppenlinie. Die ruhige Liste zeigt beide. |
| **10** — die Regel steht in der SPEC | ja | SPEC 9 trägt beide Linien, ihre Ausdehnungen, die Ausnahme an der Auswahl und die entdeckte Bedingung zum Neuzeichnen (DoD 4/B9). SPEC 15 nennt **KColorScheme** in der Abhängigkeitsliste und trägt einen eigenen Punkt „Mindestversionen": allgemeine Untergrenze 6.0.0, **6.20 für diese eine Komponente**, mit dem gemessenen Grund. |

---

## 3. Der Nachweis, der diesen Bericht trägt: die Mutationsproben

Ein grüner Prüfsatz sagt nichts darüber, ob er etwas prüft. `mutationsprobe.sh`
nimmt sechsmal je **eine** Zusicherung aus dem Produktivcode, baut und fährt
`librarytest`. Vollständige Ausgabe: `messungen/mutationsprobe.txt`.

| Eingriff | Ergebnis |
|---|---|
| Ausnahme an der Auswahl gestrichen | rot — `leavesTheEntryLineOutWhereTheRankingDoesNotAskForIt`, beide Größen |
| Linie auch unter der letzten Notiz einer Gruppe (die Doppellinie aus Falle 2.7) | rot — derselbe Satz und `separatesTheSearchResultsLikeTheLibrary` |
| Gruppenlinie auch über dem ersten Kopf | rot — `drawsAFullWidthHairlineOverEveryGroupHeadButTheFirst`, beide Größen |
| Eintragslinie über die volle Breite statt eingerückt | rot — `drawsAnInsetHairlineBetweenTwoNotesOfAGroup`, beide Größen |
| ~~`frameContrast()` durch die feste 0,20 ersetzt~~ → **berichtigt, siehe 3.1** | ~~rot — `mixesTheSeparator…`, beide Schemata~~ · **richtig ist, nachgefahren am 07.08.2026: rot** — `mixesTheSeparatorOutOfGroundAndTextInsteadOfTakingAPaletteRole` in beiden Schemata und `leavesTheEntryLineOutWhereTheRankingDoesNotAskForIt` in beiden Größen |
| Neuzeichnen der oberen Nachbarn entfernt | rot — `paintsBothUpperNeighboursAgainWhenTheSelectionMoves`, und **nur** dieser Satz |
| **kein Eingriff** | grün, 124 Prüffunktionen |

### 3.1 Berichtigung vom 07.08.2026 — diese Zeile war falsch, und sie war die wichtigste

**Die Zeile zu Probe 5 stand hier als „rot", gemessen war „ABBRUCH".** Die alte
Fassung bleibt oben durchgestrichen lesbar (B17); sie ist der Grund für den
Wächter, den das Skript jetzt trägt.

Gefunden hat es der karpathy-Review zu Sprint 9 (K3, `fail`), nicht ich. Der
Hergang:

1. Das Skript suchte `const qreal share = KColorScheme::frameContrast();`.
2. **Meine eigene Heilung von Fund 4.1** machte daraus
   `const auto share = static_cast<float>(…)` — die Verengungswarnung des
   Linters. Damit traf das Muster nicht mehr.
3. Die „genau einmal"-Wache im Skript **hat angeschlagen** und
   `ABBRUCH: Eingriff ließ sich nicht anbringen` ins Protokoll geschrieben.
4. Gelesen hat es niemand. Ich habe die Zeile im Bericht nicht gegen die
   Messdatei gehalten, sondern gegen den Lauf davor.

**Damit war AK 4 die einzige Zusicherung dieser Story ohne Mutationsnachweis —
ausgerechnet in dem Abschnitt, der „Der Nachweis, der diesen Bericht trägt"
heißt.** Der Bericht behauptete einen Nachweis, den es nicht gab.

**Das Muster, das der Review vorschlug, hätte erneut abgebrochen.** Er empfahl,
auf `KColorScheme::frameContrast()` allein zu kürzen. Der Ausdruck steht aber
**zweimal** in der Datei — einmal im Dokumentationskommentar der Funktion
(`notelistdelegate.cpp:59`), einmal im Code (`:85`). Gemessen mit `grep -c`,
bevor das Muster gesetzt wurde. Genommen ist deshalb der volle Ausdruck
`static_cast<float>(KColorScheme::frameContrast())`, einmalig belegt.

**Was daraus im Skript steht.** `mutationsprobe.sh` endet mit einem
Rückgabewert ungleich null, sobald eine Probe **abbricht**, **grün bleibt** oder
der Stand **ohne** Eingriff rot ist — die drei Arten, auf die eine Probe nichts
misst. Dazu eine Bilanz am Ende des Protokolls (`Proben insgesamt`,
`davon abgebrochen`, `davon stumm geblieben`). Der Grund des Abbruchs steht
jetzt im Protokoll statt auf der Standardfehlerausgabe.

**Der Wächter ist selbst geprüft.** Eine Probe mit absichtlich unauffindbarem
Muster, gefahren am 07.08.2026 im Belegordner: Rückgabewert **1**, Meldung
`Muster kommt 0-mal vor, erwartet genau einmal`, Bilanz `davon abgebrochen: 1`,
Gegenprobe ohne Eingriff grün — der Abbruch allein löst also aus. Das Prüfskript
ist danach gelöscht; es war ein Versuch, kein Artefakt. **Ein Rückgabewert, den
niemand ausgelöst hat, ist dieselbe Sorte Behauptung wie die Zeile, die dieser
Abschnitt berichtigt.**

**Der erste Durchgang war selbst ein Befund und steht deshalb im Skript.** Die
Rücknahme eines Eingriffs geschah mit `cp`/`mv`, und eine schlichte Kopie trägt
die Uhrzeit des Kopierens — die liegt **vor** dem Eingriff. Nach der Rücknahme
hielt make die Quelle für älter als das Objekt des mutierten Baus und baute
nicht neu; der nächste Lauf maß den vorigen Eingriff mit, und die Schlussprobe
auf dem unveränderten Stand war rot. `cp -p` plus `touch` beim Zurücknehmen
behebt es. Dieselbe Bauart wie Sprint-8-Fall 10: **eine Zahl, die aussieht wie
ein Ergebnis, und ein Werkzeug, das gar nicht gelaufen ist.**

---

## 4. Zwei Funde am eigenen Prüfweg, beide geheilt

**4.1 `QColor::fromRgbF` vergleicht sich nie gleich mit einem Bildpunkt.** Der
erste Farbprüfsatz meldete „Compared values are not the same" und druckte
zweimal `#ff9c9d9f`. Eine aus Fließkommazahlen gebaute `QColor` behält ihre
Fließkommazahlen, und `QColor::operator==` vergleicht die; `pixelColor()`
liefert eine 8-Bit-Farbe. **Die schlimmere Hälfte war der Vergleich gegen die
Palettenrollen**: Rollen sind 8-Bit, also wäre „die Linienfarbe ist keine
Palettenrolle" **immer** grün gewesen, ohne je etwas zu vergleichen. Geheilt,
indem die Testhilfe ihre Farbe über `rgb()` zurückgibt — den Wert, der im Bild
landet.

**4.2 Der Beleglauf meldete „0 Warnungen" bei 0 übersetzten Dateien.** Beim
zweiten Lauf von `pruefen.sh` war der Bauplatz schon warm; nichts wurde
übersetzt, und `grep -c warning` meldete dieselbe Null wie ein sauberer Neubau.
`pruefen.sh` räumt seinen Bauplatz jetzt zuerst und schreibt die **Zahl der
übersetzten Dateien** neben die Zahl der Warnungen — 49. Dasselbe gilt für die
Linter: 30 angefasste Dateien stehen neben den Nullen, weil ein Lauf über null
Dateien genauso aussieht.

**4.3 Mein Testwert überlebte den Lauf.** Der Farbprüfsatz schreibt
`[KDE] frameContrast=0.45` in die Sandkasten-Konfiguration — der einzige Ort,
den `KColorScheme::frameContrast()` liest. Unter dem Testmodus liegt die Datei
in `~/.qttest/config/` und **überdauert den Prozess**: Was ein Lauf schreibt,
wäre die Ausgangslage jedes späteren. Heute fällt nichts darüber, und das ist
gerade das Unangenehme — es ist die Sorte hinterlassener Zustand, die später
einen Prüfsatz grün färbt, ohne dass jemand etwas geändert hat. Gefunden hat es
der karpathy-Review (K7).

Geheilt in `initTestCase()`/`cleanupTestCase()`: der Wert von vorher wird
gemerkt und am Ende zurückgeschrieben, fehlte er, wird der Schlüssel gelöscht.
**Einmal je Lauf, nicht je Prüffunktion** — der Satz ist datengetrieben, und
eine Rücknahme zwischen zwei Zeilen liefe in die Sekunden-Trägheit von KConfig
(Falle 2.5).

Beide Pfade gemessen: Schlüssel fehlt vorher → fehlt nachher; fremder Wert
`0.11` vorher → `0.11` nachher.

---

## 5. Was ich nicht prüfen konnte

- **Ob der Kunde die Linie sieht.** 1,24 : 1 im schlechtesten Schema ist
  gemessen und trägt keinen Barrierefreiheitsanspruch. Das Urteil fällt an
  seinem Bildschirm, in der Abnahme. Das Bild aus AK 7 liegt unter seiner
  Skalierung vor, damit er nicht raten muss.
- **Ob die Gruppenlinie die Trennung stärkt oder schwächt.** Aus dem 14-px-Loch
  über einem Kopf wird ein Kasten mit Oberkante. Im gebauten Bild
  (`bilder/skalierung-1/09-ruhiges-bild-innerhalb-der-gruppe.png`) liest sich die
  Rangfolge nach meinem Auge richtig — die volle Linie trennt sichtbar
  stärker als die eingerückte —, aber das ist eine Beobachtung und kein Beleg.
  Der UI-Review entscheidet; den Hebel hat der PO benannt (Gruppenlinie
  fallenlassen, eingerückte behalten).
- **Den stehengebliebenen Strich am Bildschirm.** Der Malzähler belegt, dass
  die Ansicht die Nachbarzeile anmeldet. Dass der Bildschirm danach richtig
  aussieht, folgt daraus und ist selbst nicht abgegriffen: `grab()` zeichnet
  neu, und an den Bildspeicher des Compositors kommt kein Agent.
- **Der installierte Stand.** Ich installiere nichts nach `/usr` — das taktet
  der PO am Sprint-Ende. Alle Belege stehen am gebauten Stand.

---

## 6. Außerhalb meiner Fläche — nichts zu melden

Der Ausschluss-Griff aus `CLAUDE.md` über `frameContrast` und über
`Trennlinie\|Haarlinie` liefert vier beziehungsweise sechs Zeilen. Alle
Fundstellen außerhalb meiner Dateimenge (`wireframes/…:553`, `:565`, `:813`,
`:849–853` und `SPEC.md` Abschnitt 3.1) tragen bereits die datierten Vermerke
des PO vom 06. und 07.08.2026 und sind auf dem Stand, den diese Umsetzung
voraussetzt. Zeichnung 3a habe ich gelesen und nicht angefasst.

**Ein Fund, der dem PO gehört: Szenen von `libraryshots` sind nicht
bildpunktgleich wiederholbar.**

~~Drei der fünfzehn: `02-leerzustand.png`, `10c-schema-dunkel-bearbeiten.png`
und `10d-schema-hell-bearbeiten.png`. Zwei davon zeigen den
Bearbeitungszustand, also ein blinkendes Textzeichen; für den Leerzustand habe
ich die Ursache nicht gesucht. **Keine der drei zeigt die Notizliste**, mit
dieser Story hat es nichts zu tun.~~

**Zweimal berichtigt am 07.08.2026**, und beide Male, weil zwei Läufe zu wenig
sind. Der karpathy-Review (K4) hat die Begründung widerlegt, die Nachmessung
danach die Zahl.

**Es sind vier von fünfzehn, nicht drei.** Über **acht** Läufe desselben
Binärcodes, offscreen unter `QT_QPA_PLATFORMTHEME=kde`, je sieben Vergleiche
gegen den ersten:

| Szene | wie oft abweichend | Abweichungsfläche | Ursache |
|---|---|---|---|
| `10c-schema-dunkel-bearbeiten.png` | 4 von 7 | **1 × 18** bei x = 530, y = 106 | Textcursor im Editor des Detailbereichs |
| `10d-schema-hell-bearbeiten.png` | 4 von 7 | **1 × 18** bei x = 530, y = 106 | ebenda |
| `02-leerzustand.png` | 3 von 7 | **1 × 18** bei x = 16, y = 15 | Textcursor im **Suchfeld** der Kopfzeile |
| **`03-meldungszustand.png`** | 2 von 7 | **900 × 50** bei y = 48 | **das Meldungsband** — es fährt animiert auf und zählt Sekunden herunter |

**Drei Ursachen sind dieselbe, die vierte ist eine andere.** Der ein Bildpunkt
breite, achtzehn Bildpunkte hohe senkrechte Balken ist ein blinkendes
Textzeichen — auch im Leerzustand, denn das Suchfeld steht dort ebenso. Der
Meldungszustand dagegen weicht über die **volle Breite** und fünfzig
Bildpunkte Höhe ab, genau dort, wo das Band sitzt. Das ist keine
Cursorphase, sondern ein Ablauf.

**Was das für #91 heißt** (dessen Titel den Textcursor als Ursache nennt):

- Die Frage aus dem Kommentar vom 05.08.2026 — „gibt es eine zweite Ursache?" —
  ist mit **ja** beantwortet, nur nicht dort, wo sie vermutet wurde. Der
  Leerzustand hat dieselbe Ursache wie 10c/10d; der **Meldungszustand** hat eine
  eigene.
- Das Kriterium „über mindestens drei Läufe bitgleich" **kann durch Zufall
  erfüllt sein**: Bei 2 von 7 abweichenden Vergleichen ist ein dreifach
  gleicher Lauf nichts Besonderes. Die Zahl der Läufe gehört höher, oder das
  Kriterium an die Ursache statt an die Wiederholung.

**Zur Ausgangsfrage:** `10c` und `10d` **zeigen** die Notizliste samt beider
Gruppenköpfe und der vollen Linie über „Gestern" — mein durchgestrichener Satz
war falsch. Der Schluss trägt trotzdem, jetzt am Gemessenen: Die Liste liegt
bei x < 300 und unterhalb von y ≈ 48; **keine der vier Abweichungsflächen liegt
in ihr**. Mit den Trennlinien aus #101 hat der Befund nichts zu tun.

Es steht hier, weil daraus etwas folgt: **ein Bildpunktvergleich zweier Läufe
taugt für diese vier Szenen nicht als Regressionsprüfung** — er meldet einen
Unterschied ohne Ursache im Code. Gemeldet, nicht geheilt.

**Ein Hinweis ohne Handlungsbedarf:** Die Textzeichenfunktion des Delegates
heißt `drawLine()` und zeichnet **Text**; die beiden neuen Trennlinien
entstehen mit `fillRect()`. Der Name stammt aus Sprint 2 und ist jetzt
zweideutig. Umbenennen wäre eine Änderung außerhalb dieser Story — gemeldet,
nicht geheilt.

---

## 7. Belege

| Datei | Inhalt |
|---|---|
| `messungen/b1-bau.txt` | Neubau: 49 Dateien, 0 Warnungen, 0 Fehler; die Meldung von CMake zu `KF6ColorScheme (required version >= 6.20)` |
| `messungen/b2-ctest.txt` | `ctest` über alle neun Prüfprogramme, 100 % |
| `messungen/b3-pruefsaetze.txt` | die Prüffunktionen dieser Story einzeln, dazu die Bestandssätze aus AK 5 |
| `messungen/b4-linter.txt` | `lint-tidy` und `lint-clazy`: je Rückgabewert 0, **30 angefasste Dateien**, 0 Warnungen, 0 Fehler |
| `messungen/b5-bilder.txt` | Maße der Bildbelege und die Skalierung, unter der sie entstanden sind |
| `messungen/mutationsprobe.txt` | die sechs Eingriffe mit ihrem Ergebnis |
| `bilder/skalierung-1/` | die fünfzehn Szenen von `libraryshots` und die sechs von `searchshots`, Skalierung 1 |
| `bilder/skalierung-1-6/` | zwei Szenen unter `QT_SCALE_FACTOR=1.6`: der Normalfall (AK 7) und die ruhige Liste — die einzige, in der beide Linien zugleich stehen |
| `pruefen.sh`, `mutationsprobe.sh` | beide Läufe, wiederholbar |
