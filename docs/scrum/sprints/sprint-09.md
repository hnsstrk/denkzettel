# Sprint 9

**Sprint-Ziel:** Der Nutzer erkennt, wo er tippt — und wo eine Notiz aufhört.

**Basis-Tag:** `sprint-09-basis` = `366b69f` · **Milestone:** Sprint 9 ·
**Planning:** 07.08.2026

> *Berichtigt am 08.08.2026 (Mangel M6).* Hier stand `4be3f8d` — der Stand, auf
> den der Tag beim Anlegen zeigte. Der PO hat ihn danach auf `366b69f`
> vorgezogen, weil die Worktrees sonst ohne die Berichtigung des #101-Berichts
> gestartet wären, und die Angabe hier nicht mitgezogen. Beides zeigt auf
> denselben Zweig; die Zahl war trotzdem falsch, und ein Basis-Tag ist die
> Bezugsgröße jedes Sprint-Diffs.

---

## 1. Freigabe und Rollenlage

**Der Kunde hat Sprint 9 am 07.08.2026 selbst freigegeben** und damit die
Übertragung aus den Sprints 7 und 8 beendet. Vorgelegt wurden drei Zuschnitte;
gewählt hat er **#100 + #101** mit der Begründung, die der PO empfohlen hatte:
#84 und #97 haben keinen Vorprüfbericht, sind damit nach der Definition of
Ready nicht ziehbar, und sie müssten nach #100 ohnehin neu gemessen werden.

Zwei weitere Entscheidungen des Kunden vom selben Tag greifen in diesen Sprint:

- **`KF6::ColorScheme` wird Abhängigkeit** — mit eigener Mindestversion 6.20,
  während `KF_MIN_VERSION` bei 6.0.0 bleibt. Der PO hatte die Frage zuerst
  unvollständig vorgelegt (die Zeile `CMakeLists.txt:6` speist **zwei**
  `find_package`-Aufrufe und hebt damit ECM und alle zehn Komponenten mit); auf
  die berichtigte Vorlage hat der Kunde den engeren Weg gewählt.
- **Die Prüf-Skalierung 1,6 gilt weiter.** Bildpunkt-Prüfsätze laufen deshalb
  auf Skalierung 1; das Bild unter 1,6 ist Sichtbeleg.

**Eine dritte Frage hat der Kunde ausdrücklich abgegeben:** die Farbrolle des
Notiztextes („Der UI/UX-Agent soll das entscheiden. Neutral und im Sinne der
UI/UX"), später auch die Frage nach der Fokusschicht („PO und UI/UX Experte
entscheiden"). Beide sind unten unter §4 verzeichnet.

## 2. Der Zuschnitt

**Zwei Issues, zwei Stränge, parallel.**

| | **Strang A** | **Strang B** |
|---|---|---|
| **Issue** | #100 Eingabefeld erkennbar (`m`) | #101 Trennlinien in der Bibliothek (`m`) |
| **Epic** | M1 | M2 |
| **Zweig / Worktree** | `story/100-eingabefeld` · `../denkzettel-100` | `story/101-listentrenner` · `../denkzettel-101` |
| **Quellen und Tests** | `src/capture/capturewindow.{h,cpp}`, `tests/capturetest.cpp`, `tests/captureshots.cpp`, `tests/themes/…/denkzettel-test-breit/colors` (nur der Kommentarkopf) | `src/ui/notelistdelegate.{h,cpp}`, `src/ui/librarywindow.cpp` (nur `showNote()`), `tests/librarytest.cpp`, `tests/libraryshots.cpp` |
| **Build** | nichts — `KF6::Svg` steht bereits an `denkzettelcapture` (`src/CMakeLists.txt:66`), von beiden Bearbeitern gemessen | `KF6::ColorScheme` an `denkzettelui`, dazu ein **eigener** `find_package`-Aufruf mit Mindestversion 6.20. `KF_MIN_VERSION` `:6` und die Komponentenliste `:24–34` bleiben unberührt |
| **SPEC** | 3.1, 3.2 | 9 und 15 |
| **Start** | sofort | sofort |

**Klassenprofil `m m`.** Beide Grenzen sind eingehalten: kein `xl`, keine `l`,
zwei Stories. **Sprint-Konto:** 2 Issues · 2 × `m` · keine Grenzüberschreitung
vorzulegen.

### 2.1 Kollisionsfläche

Gemessen in beiden Vorprüfungen: **keine gemeinsame Quell- oder Testdatei.**
Strang A arbeitet in `src/capture/`, Strang B in `src/ui/`. Gemeinsam berührt
werden `src/CMakeLists.txt` (verschiedene Ziele), `CMakeLists.txt` (nur B) und
`SPEC.md` (verschiedene Abschnitte).

Kleinster Abstand: **eine Zeilengruppe in derselben Datei, nie dieselbe Zeile.**
Worktree-Trennung nach B13 und getrennte Belegordner genügen.

**Der Bauplan gehört in diesem Sprint allein Strang B.** Die Vorprüfung zu #100
hält für **beide** Bearbeiter „kein Build-Eingriff" fest, und die
Kundenentscheidung vom 07.08.2026 hält `KF_MIN_VERSION` und die Komponentenliste
frei. Damit entfällt die Klausel, die hier zunächst stand — Strang A und B
liegen im Bauplan nicht mehr aufeinander, und es gibt nichts zu takten.

Zu takten bleibt allein die **Installation nach `/usr`**: am Sprint-Ende,
einmal, für den Endstand beider Stränge — durch den PO, nicht durch die Agenten.

## 3. Was die Vorprüfung gefunden hat

Vier Messungen mit je zwei unabhängigen Bearbeitern, zwei konsolidierte
Berichte. Drei Funde wiegen über ihren Anlass hinaus.

### 3.1 Ein Kriterium, das einen Zustand beschrieb, während die Sache ein Übergang ist

**Beide Bearbeiter von #101 haben unabhängig denselben Fehler gefunden.** Beim
Auswahlwechsel zeichnet die Ansicht nur die Strecke zwischen alter und neuer
Auswahl neu; die Zeile darüber nie. Ein Bau nach dem ursprünglichen AK 3 ließe
bei jedem Tastendruck genau eine Linie falsch stehen.

**Und `grab()` verdeckt es** — der Weg aller fünf Bildläufer zeichnet alles neu.
Ein solcher Bau bestünde jede Prüfung, die das Kriterium nahelegt, und wäre beim
Kunden sichtbar falsch. AK 3 zerfällt deshalb in 3a/3b/3c, und die Belegform von
3c ist das **Neuzeichnen** (Malzähler oder Filter auf die Malregion), nicht ein
Bild.

### 3.2 Ein Prüfsatz, der grün bleibt und ab dann das Falsche misst

`hullIsCompleteAtFiveAndEightLines()` und
`takesTheOpaqueVariantWithoutABlurringCompositor()` tasten den Fenstermittelpunkt
ab — der liegt mit dem Feld im Textbereich.

Der Scrum Master hatte zuerst gemeldet, sie würden unter `default` rot. **Er hat
sich selbst widerlegt:** Seine Hüllendeckung 216 war ohne den Auswahlpfad
`opaque` gemessen, den der Bau setzt, sobald nichts weichzeichnet — offscreen
immer. Bearbeiter A hatte 255 gemessen und recht.

Die Folge wiegt schwerer als der Zahlendreher: Die Prüfsätze bleiben **grün,
überall wo sie heute laufen**, und messen ab dann das Feld statt der Hülle. Rot
würden sie allein unter vier Themes, die kein Lauf abgreift. Bis zur
Mutationsprobe nachgemessen: Die Mutation, gegen die
`takesTheOpaqueVariantWithoutABlurringCompositor()` gebaut wurde, liefe
unentdeckt durch. **Prüfmittel von AK 9 ist deshalb die Mutationsprobe**, nicht
ein grüner Lauf.

### 3.3 Eine Zahl, die aus der falschen Quelle stammte

`frameContrast` kommt aus der Gruppe `[KDE]` der **Anwendungskonfiguration**.
Kein einziges der 19 geprüften Schemata trägt den Schlüssel; überall gilt die
Voreinstellung 0,20. Die Kontrastwerte 1,24 : 1 bis 1,93 : 1 bleiben gültig und
entstehen aus den **Farben**.

Prüfsatz P4 der Zeichnung verlangte „eines mit hohem und eines mit niedrigem
Wert" — eine unerfüllbare Vorgabe, weil es diese zwei Werte nicht gibt. Er ist
neu gefasst: zugesichert ist das **Verfahren**, geprüft wird an zwei Schemata mit
weit auseinanderliegendem **Ergebnis**.

## 4. Zwei Festlegungen, die vor dem Sprint gefallen sind

### 4.1 Prüfsatz F3 ist zwei Tage nach seiner Aufnahme wieder abgetreten

Der Kunde hat die Frage nach der Farbrolle des Notiztextes dem UX-Agenten
übertragen. Ergebnis: **Der Text bleibt auf der Fensterrolle**, AK 3 in #100
entfällt.

Gemessen gehört der Gewinn „12,72 → 17,68 : 1", mit dem die Umstellung begründet
war, der **Feldfläche** und nicht der Textfarbe — unter 14 von 19 Schemata sind
beide Rollen dort bildpunktgleich. Wo sie sich unterscheiden, fällt die Messung
viermal gegen die Ansichtsrolle aus und einmal für sie. **Der einzige Wert unter
4,5 : 1 in der gesamten Messung entstand erst durch das Kriterium.**

Damit ist die Begründung des PO vom 06.08.2026 widerlegt, auf der das Kriterium
ruhte. Zeichnung 4b hat F3 zurückgenommen, der alte Prüfsatz gilt wieder.

### 4.2 Die Fokusschicht wird eine eigene Story

Der UX-Agent hat bei der Messung gefunden, dass **alle acht Themes einen
Fokuszustand mit sichtbarer Kante führen** und der Textbereich des
Erfassungsfensters immer den Fokus hat. Unter den fünf Themes, unter denen das
Feld sonst unsichtbar bliebe, hebt diese Schicht es von 1,00–1,14 : 1 auf
2,81–4,66 : 1 — gegen KRunners 1,41 : 1.

Der Kunde hat die Frage an PO und UX-Agenten gegeben; beide sind unabhängig zu
**#102** gekommen. Gründe: Die Schicht ist ein anderer Gestaltungsgegenstand
(sie führt den Zustand „offen, aber nicht aktiv" ein, den SPEC 3 zusichert und
kein Prüfsatz kennt), und eine einzelne Messung darf die zwei unabhängigen
Bearbeiter der Definition of Ready nicht umgehen.

**Bedingung, unter der beide zugestimmt haben:** #102 wird angelegt, **bevor**
#100 geschlossen wird, und AK 6b benennt die Grenze unverändert. Beides ist
erfüllt.

Zwei Wege sind ausdrücklich verworfen: die Schicht immer zu zeichnen (ein
Fenster, das dauerhaft fokussiert aussieht, sagt über seinen Zustand etwas
Falsches — wer nicht sieht, ob es die Tastatur hat, tippt ins Leere), und sie
zu bauen und den Aktivierungsfall zu vertagen. Zum zweiten, einem Vorschlag des
PO, sagte der UX-Agent: „Er baut einen Zustand und lässt ungeprüft, wie er sich
in seiner Hälfte verhält; das ist die Lage, in der ein Fehler grün aussieht. Die
Klasse hielte er vermutlich, und das ist gerade das Verführerische daran."

## 5. Der B17-Griff hat zweimal zu wenig gefunden

Beide Male lag es am selben Fehler, und beide Male hat ihn ein anderer gefunden.

- **Zeichnung 3a** trug weiterhin „`frameContrast` des Farbschemas" bei `:553`
  und `:565` — und `:565` **ist Prüfsatz P4 selbst**, also der Satz, gegen den
  gebaut wird. Der PO hatte SPEC 3.1 und Zeichnung 4b genannt und die Zeichnung
  übersehen, auf die das Kriterium sich beruft. **Gefunden vom Scrum Master.**
- **Acht Issue-Titel** trugen ASCII-Ersatz statt Umlaute (`ueberlebt`,
  `Bildlaeufer`, `Flaeche`, `abgewaehlten`, `Bildschirmverhaeltnis`, `schlaegt`,
  `ungepruft`, `gruen`). **Gefunden vom UX-Agenten an einem einzigen Titel**;
  der PO fand beim Nachsehen zunächst nur fünf, weil er mit einer **Wortliste**
  filterte statt mit einer Ausschlussform — genau der Fehler, den `CLAUDE.md`
  für den B17-Griff beschreibt.

**Das ist der dritte Beleg für denselben Satz:** Ein Werkzeug, dessen Suchraum
enger ist als der Geltungsbereich der Regel, meldet Vollständigkeit und liefert
sie nicht — und es meldet sie besonders überzeugend, weil es Treffer hatte.

## 6. Sprint-Konto

| | |
|---|---|
| **Gezogen beim Planning** | #100 (`m`), #101 (`m`) |
| **Zugänge nach der Freigabe** | keine |
| **Größenklassen** | 2 × `m` — kein `xl`, keine `l` |
| **Grenzen** | 2–4 Stories: eingehalten · höchstens eine `l`: eingehalten |
| **Backlog-Zugang ohne Sprint-Bezug** | #102 (Fokuszustand) — angelegt, nicht gezogen |

## 7. DoD-Prüfung

**Geprüft am 07.08.2026, 23:41–23:55 durch den Scrum Master, am Stand
`d9fb10b`.** Jede Zeile unten ist an einem eigenen Lauf oder an der Messdatei
gemessen, nicht an der Meldung eines Stranges.

### 7.1 DoD 1 — Bau, Tests, Linter · **erfüllt**

| Prüfung | Ergebnis |
|---|---|
| Neubau `cmake --build build` | Rückgabe 0, **0 Compilerwarnungen** |
| `ctest` auf Ganymed, offscreen mit `QT_QPA_PLATFORMTHEME=kde` | **10 von 10 grün** (neu: `librarytestskaliert`) |
| `lint-tidy` | rc **0**, **30 Dateien**, 0 Warnungen, 0 Fehler |
| `lint-clazy` | rc **0**, **30 Dateien**, 0 Warnungen, 0 Fehler |
| Öffentlicher Lauf zum **eigenen** Commit (B18) | `d9fb10b` → Lauf `31220962285`, **completed und success**, 10/10, beide Linter über 30 Dateien |

Die Dreizahl ist je Linter vollständig erhoben; die Zahl der angefassten
Dateien steht daneben, weil ein Lauf über null Dateien genauso aussieht.

**Der öffentliche Lauf war in diesem Sprint einmal rot** — `70902a4`,
Lauf `31216657864`, `capturetest` 28/6/3. Ursache und Heilung stehen in
`sprint-09-s100-eingabefeld/bericht.md` §7. Nachgeprüft: `0b3019b` und jeder
Lauf danach ist grün, der Endstand ebenfalls.

**Zwei Fenstergrößen** (DoD 1, Satz 2): #101 misst an 900×600 und 1200×800
(27 Fundstellen in `tests/librarytest.cpp`); #100 an den beiden Zeilenlagen aus
SPEC 3 — die Fensterbreite ist dort fest, die Höhe ist die veränderliche Größe.

### 7.2 DoD 2 — installierter Stand · **erfüllt, unabhängig nachgemessen**

Der Bericht des PO (`sprint-09-abnahme/bericht.md`) belegt Installation,
Dienstneustart und beide Hauptwege mit Bild. **Unabhängig davon nachgemessen:**

```
md5  /usr/bin/denkzetteld    7e23862e745a9381670ca89cae6b3d35
md5  build/bin/denkzetteld   7e23862e745a9381670ca89cae6b3d35
readlink /proc/<pid>/exe  →  /usr/bin/denkzetteld        (ohne „(deleted)")
```

Mein Neubau am Endstand hat **null Dateien übersetzt** — das
Build-Verzeichnis stand also bereits auf `d9fb10b`, und die installierte Datei
ist byteweise dieselbe. Damit ist B16 nicht nur behauptet, sondern von zwei
Seiten belegt.

### 7.3 DoD 3 — Reviews · **nicht vollständig erfüllt (M1)**

**UI-Review (`denkzettel-ux`): geführt, kein `fail`.** Eigene Bilder aus einem
eigenen Bau (B3), drei Prüflagen — offscreen 1, offscreen 1,6 und angemeldete
Sitzung —, je gezeichneter Bereich ein Prüfpunkt. Der eine `warn` (L9,
Strichstärke) ist behoben (`78eeaff`), mit eigenem Prüfsatz in eigener
Prüflage (`librarytestskaliert`) und als **Mutationsprobe 7** verankert;
nachgelesen in `messungen/mutationsprobe.txt`, dort rot und nur dieser Satz.

**karpathy-Review: vier `fail`, drei am Stand geschlossen.** Nachgeprüft am
Code und an den Messdateien, nicht an den Meldungen der Stränge:

| | Befund | Stand am `d9fb10b` |
|---|---|---|
| **K1** | Bericht meldete einen Lauf, den es nicht gab | **geschlossen** — Feld 3 trägt den datierten Vermerk, die alte Fassung steht lesbar darunter (B17); der Läufer ist ab `0b3019b` grün |
| **K2** | `QSKIP` auf der falschen Hälfte | **geschlossen** — `tests/capturetest.cpp:1185–1192` fragt die Vorbedingung **vor** dem `QVERIFY2`, mit Begründung im Kommentar; `ring > 0` (`:925`) und `border > 0` (`:1099`) stehen jetzt hinter `whyNoFieldGraphic()` und sagen im Kommentar, was von ihnen übrig ist |
| **K3** | Mutationsprobe brach ab und stand als „rot" im Bericht | **geschlossen** — `messungen/mutationsprobe.txt`: Probe 5 rot mit fünf namentlich gefallenen Prüfsätzen; Bilanz `Proben insgesamt 7 · davon abgebrochen 0 · davon stumm geblieben 0` |
| **K5** | README-Bilder zeigen das Produkt vor beiden Kundenbefunden | **nicht behoben, an #96 verwiesen** — Urteil siehe unten |
| K4, K6, K7, K8, K9, L9 (`warn`) | | sämtlich am Stand geprüft und geschlossen: §6 des #101-Berichts über acht Läufe nachgemessen · `kcolorscheme` mit Begründung in `ci.yml:66–78` · `frameContrast` wird in `initTestCase()`/`cleanupTestCase()` gemerkt und zurückgeschrieben (`tests/librarytest.cpp:496–550`) · `m3-testlauf.txt` trägt M3a **und** M3b · `besideTheField()` liest die Lücke aus den ausgelegten Widgets und nimmt ihre Mitte (`tests/capturetest.cpp:276–287`) |

**Urteil zu K5 — die Verweisung trägt, mit einer Auflage.** Der Läufer
`readmeshots` erzeugt heute ein unbrauchbares Bild: Fläche und Schrift kommen
seit #83 aus zwei Quellen, gemessen 1,10 : 1 (#96). Wer die Bilder jetzt
erneuerte, tauschte einen veralteten Beleg gegen einen falschen. Der Weg ist
richtig; das Zugeständnis steht im README als datierter Hinweis mit
Issue-Verweis. **Auflage:** #96 gehört in die Kandidatenliste des nächsten
Sprints — die Heilung liegt bereits vor und ist gemessen (siehe 8, B2).

**Was DoD 3 offen lässt:** Der Review lief über `sprint-09-basis..70902a4`.
Geliefert wird `d9fb10b`. Dazwischen liegen **455 geänderte Zeilen** in `src/`,
`tests/`, `CMakeLists.txt` und `.github/`, darunter **41 Zeilen Produktivcode**
(`src/ui/notelistdelegate.cpp`, die L9-Heilung) und eine neue Testanmeldung.
Diese Änderungen hat kein Review gesehen. → **M1**

### 7.4 DoD 4 — SPEC nachgezogen · **teilweise (M2, M3)**

**Nachgezogen und belegt:** SPEC 3.1 trägt die zweite Fläche, die
Deckungsgrenze und den Feldrand bei den Innenabständen (`SPEC.md:220–288`,
Textnachweis in `m6-spec-nachweis.txt`). SPEC 9 trägt beide Linien, ihre
Ausdehnungen, die Ausnahme an der Auswahl **und** die entdeckte Bedingung zum
Neuzeichnen des oberen Nachbarn. SPEC 15 nennt KColorScheme und führt einen
eigenen Punkt „Mindestversionen" mit der 6.20 und ihrem Grund
(`SPEC.md:1015–1032`).

**Nicht nachgezogen:** die beiden Bedingungen, die erst mit L9 entstanden sind
— die Stärke misst in **Gerätebildpunkten** (M2), und ein **Nachbau** der
Skalierung im selben Prozess taugt nicht als Prüflage (M3).

### 7.5 DoD 5 und DoD 6 — **noch nicht fällig**

Beide gehören in Takt 2 und sind vor der Kundenabnahme nicht erfüllbar
(PROZESS.md, „Prüfzeitpunkte"). Stand: #100 und #101 offen, Milestone
Sprint 9 offen, kein Verwalter-Auftrag ergangen. Ein Vermerk zum Vollzug von
Takt 2 folgt nach der Abnahme.

### 7.6 Belegprüfung nach Takt 1, Punkt 2

**Berichte gelaufener Prüfungen, gegen die Commit-Botschaften gehalten:**
Jeder Commit, der Befunde eines Prüflaufs nennt, hat seinen Bericht im Repo —
`sprint-09-karpathy-review.md`, `sprint-09-ui-review/bericht.md`, die beiden
Strangberichte samt ihren Nachträgen, `sprint-09-abnahme/bericht.md`. Kein Lauf
ohne Bericht gefunden.

**Prüfsummen der Bildbelege** (`docs/scrum/bildbelege-pruefen.sh`, je Ordner):

| Ordner | Bilder | Gruppen | Urteil |
|---|---|---|---|
| `sprint-09-s100-eingabefeld` | 20 | 3 | kein Mangel — siehe unten |
| `sprint-09-s101-listentrenner` | 23 | 1 | kein Mangel — `1-volle-liste` = `6-feld-geleert-volle-liste`: derselbe Zustand nach einer Rückkehr |
| `sprint-09-ui-review` | 94 | 7 | kein Mangel — siehe unten |
| `sprint-09-abnahme` | 2 | 0 | keine gleichen Bytes |

Jede Gruppe einzeln bewertet, wie die Regel es verlangt:

- **Die drei `s101-*-gruppengrenze`-Gruppen sind der Befund L4 selbst.**
  Normalfall, Auswahl und Überfahren tragen denselben Ausschnitt der
  Gruppengrenze — dass die Bytes gleich sind, **ist** die zugesicherte Aussage
  „die Gruppenlinie bleibt stehen".
- **`s100-feld-nord*` = `s100-feld-CachyOS-Nord-round*`:** zwei Namen, eine
  Grafik. Der Bericht zieht daraus nur einen Schluss, nicht zwei.
- **`01-rand-schmal-hell-leer` = `04-rand-schmal-dunkel-leer`** (und dasselbe
  Paar im Zustand „getippt"): **0 von 354.816 Bildpunkten verschieden.**
  Erlaubt, aber die Begründung im Läufer stimmt nicht — siehe **M4**.

**Stop-Bedingung der Wache:** In diesem Sprint hat sie neue Gruppen gefunden.
Sie bleibt in voller Form.

### 7.7 Doku-Abgleich (B10, erweitert nach B17)

| Ort | Ergebnis |
|---|---|
| `README.md` | Statuszeile nennt keinen Verfahrensstand — richtig so. Der Bildhinweis ist datiert und verweist auf #96. **Beobachtung B1:** Er steht unter dem *zweiten* Bild; das erste steht 45 Zeilen darüber und trägt keinen Hinweis |
| `.github/workflows/ci.yml` | Kommentarkopf und Paketliste auf Stand; `kcolorscheme` mit Begründung und Untergrenze |
| `CMakeLists.txt` | Begründung zu `KF6ColorScheme 6.20` bei `:37`, eigener `find_package`-Aufruf bei `:43` |
| `tests/CMakeLists.txt` | die neue Anmeldung `librarytestskaliert` trägt ihren Grund im Kommentar (`:96–104`) |
| `CLAUDE.md` | keine falsch gewordene Aussage gefunden — fünf Bildläufer, Linterschwelle null, B18-Griff, B21-Passage sämtlich auf Stand |
| `SPEC.md` | siehe 7.4 |
| `wireframes/` | Zeichnung 3a ist nachgezogen (P6), liegt aber **unversioniert** im Arbeitsbaum → **M2** |
| `tests/captureshots.cpp` | Kommentarkopf sagt etwas, das gemessen nicht gilt → **M4** |
| `.claude/agents/denkzettel-dev.md` | Überschrift „Neun gemessene Fälle", die Liste führt **zehn** → **M5** |

### 7.8 Zwei Zahlen im Protokollkopf und in den Belegen

- **Der Kopf dieses Protokolls nennt `sprint-09-basis` = `4be3f8d`.** Der Tag
  zeigt auf **`366b69f`**, zwei Commits später; alle vier Berichte des Sprints
  rechnen mit `366b69f`. Der Kopf ist der Ausreißer. → **M6**
- **Die Standzeilen der Belege** nennen zu #101 durchweg `46bb5b5` — einen
  Commit, an dem `hairline()` **nicht existiert** (0 Treffer, gegen 4 in
  `78eeaff`). Zu #100 nennt keine einzige Messdatei einen Stand. → **M7**

**Dieser Abschnitt gilt für `d9fb10b`.** Während der Prüfung ist `58bd93f`
dazugekommen; was sich dadurch geändert hat, steht in **8.1** — hier nicht,
weil ein Prüfbericht die Beweislage seines Standes ist (B17).

## 8. Mängelliste

Melden, nicht heilen. Nichts davon habe ich verändert; sieben Punkte gehen an
den PO, drei Beobachtungen stehen ohne Änderungsforderung daneben.

| | Mangel | Beleg | Vorschlag |
|---|---|---|---|
| **M1** | **DoD 3: der Review-Diff endet vor dem Lieferstand.** Geprüft wurde `sprint-09-basis..70902a4`, geliefert wird `d9fb10b`; dazwischen 455 Zeilen in `src/`, `tests/`, `CMakeLists.txt`, `.github/`, davon 41 Produktivcode | `git diff --stat 70902a4..d9fb10b -- src/ tests/ CMakeLists.txt .github/` | karpathy-Nachlauf über `70902a4..d9fb10b` **vor** der Kundenabnahme. Der Auftrag benennt den Diff, nicht die Stories (B19) |
| **M2** | **Zeichnung 3a ist nachgezogen, aber unversioniert.** P6 („die Stärke misst in Gerätebildpunkten") liegt allein im Arbeitsbaum | `git status --short` → ` M wireframes/…dc.html` | Festschreiben. Ein unversionierter Beleg ist kein Beleg (B7) |
| **M3** | **DoD 4: eine entdeckte Bedingung fehlt in der SPEC.** Der Nachbau der Skalierung im selben Prozess zeigte den Fehler bei 1,25 und **nicht** bei 1,6 — die Zeilenhöhen einer skalierten Sitzung sind nicht die einer unskalierten. SPEC führt für genau diese Klasse eine Liste (`SPEC.md:1130 ff.`), die Bedingung steht nicht darin. Auch SPEC 9 schweigt zur Stärke in Gerätebildpunkten, während die Zeichnung sie trägt | `#101`-Bericht §8.1; `grep -n "QT_SCALE_FACTOR" SPEC.md` → eine Fundstelle, eine andere Sache | Beide Sätze in `SPEC.md` nachziehen (Dev, nicht PO — es ist eine Festlegung über das Erzeugnis) |
| **M4** | **Der Kommentarkopf der Bildreihe misst falsch.** `tests/captureshots.cpp:185–187` sagt, die beiden Schema-Spalten zeigten „die Texte und nichts sonst". Gemessen: in der **schmalen** Spalte zeigen sie gar nichts — `01`/`04` und `02`/`05` sind byteweise gleich (0 von 354.816 Bildpunkten). Ursache ist nicht die Feldgrafik, sondern die `colors`-Datei: `breeze-dark` bringt eine mit, `CachyOS-Nord-round` nicht (#85) | eigener Bildpunktvergleich, 07.08.2026; `test -f /usr/share/plasma/desktoptheme/breeze-dark/colors` | Kommentar auf das Gemessene setzen: In der Spalte des Themes mit eigener `colors`-Datei trägt die Schema-Achse nichts |
| **M5** | **Die Fallenliste zählt neun und führt zehn.** `.claude/agents/denkzettel-dev.md:59` | Nachzählen | Zahl aus der Überschrift nehmen, statt sie mitzuführen — sie wird bei jedem Zugang wieder falsch. Der Sprint bringt vier Zugänge mit (siehe Retro) |
| **M6** | **Der Protokollkopf nennt einen falschen Basis-Tag** (`4be3f8d` statt `366b69f`) | `git rev-parse --short sprint-09-basis` | Kopf berichtigen |
| **M7** | **Die Belege nennen einen Stand, an dem der gemessene Code nicht existiert** (#101, sieben Dateien, `46bb5b5`) oder gar keinen (#100, sieben Dateien) | `git show 46bb5b5:src/ui/notelistdelegate.cpp \| grep -c hairline` → 0 | Datierte Zeile anhängen, nicht umschreiben (B17). Ursache und Abhilfe: Retro-Beschluss **B24** |
| **M8** | **Vier Datumsangaben liegen einen Tag in der Zukunft** (`08.08.2026` in `sprint-09-s100-eingabefeld/bericht.md:232, 240, 416` und `mutationsproben.sh:47`). Sämtliche Commits des Sprints tragen den 07.08.2026 | `git log --date=format:'%F'`, `date` | Auf 07.08.2026 setzen. Reale Zeitstempel, keine geschätzten |

### 8.1 Nachtrag 07.08.2026, 23:58 — zwei Mängel sind während der Prüfung gefallen

Die Prüfung oben steht auf `d9fb10b`. Um **23:47** ist `58bd93f` dazugekommen
(„Die Strichstärke bekommt ihre Maßeinheit — in Zeichnung 3a und SPEC 9"). Der
Text oben bleibt stehen, wie er war (B17); hier steht, was sich daran geändert
hat:

- **M2 ist behoben.** Zeichnung 3a ist festgeschrieben, der Arbeitsbaum ist
  insoweit sauber.
- **M3 ist zur Hälfte behoben.** SPEC 9 trägt jetzt „Die Stärke ist ein Maß in
  Gerätebildpunkten" als entdeckte Bedingung, samt der ausdrücklich
  ungeregelten seitlichen Kante. **Offen bleibt die zweite Hälfte:** dass ein
  **Nachbau** der Skalierung im selben Prozess keine taugliche Prüflage ist —
  er zeigte den Fehler bei 1,25 und bei 1,6 nicht. Diese Bedingung betrifft den
  Prüfweg, nicht die Gestaltung, und gehört in die Liste ab `SPEC.md:1130`.
  Gesucht wurde danach; die einzige Fundstelle von „Nachbau" in `SPEC.md`
  (`:204`) meint eine andere Sache.
- **M1 ist unverändert.** `58bd93f` fasst nur `SPEC.md` und `wireframes/` an;
  der Griff nach B25 liefert am neuen Stand dieselben 455 Zeilen.
- **DoD 1 hält am neuen Stand:** der öffentliche Lauf zu `58bd93f` ist
  completed und success.

### 8.2 Nachtrag 08.08.2026, 00:22 — M1 ist gefahren und hat einen neuen `fail` gebracht

Der PO hat den in M1 verlangten karpathy-Nachlauf über `70902a4..HEAD`
ausgeführt; der Bericht liegt als
`docs/scrum/reviews/sprint-09-karpathy-nachlauf.md`. **M1 ist damit erledigt**
— der Diff hat seinen Prüfer bekommen, und der Prüfer hat eigene Messungen
gefahren (Neubau, `ctest` 10/10, `uxsonde` in acht Skalierungslagen, eine
eigenständige Rastersonde).

**Was er gefunden hat, ändert das DoD-3-Urteil erneut:**

- **N1, `fail`** — die Zusicherung „die Oberkante auf der
  Gerätebildpunktgrenze" aus der L9-Heilung hält nicht
  (`src/ui/notelistdelegate.cpp:123`, dazu `SPEC.md:739` und Zeichnung 3a).
  **DoD 3 bleibt damit offen**, nur an einer anderen Stelle als vorher: nicht
  mehr am ungeprüften Diff, sondern an einer Zusicherung, die drei Artefakte
  behaupten und der Code nicht einlöst. Melden, nicht heilen — die Fläche ist
  Dev und PO.
- **N6 bestätigt M7 unabhängig** (beide #101-Messdateien nennen `46bb5b5`).
- **N4 und N5** zeigen, dass K3 nur in **einem** der beiden Mutationsskripte
  geheilt ist; `mutationsproben.sh` von #100 hat in `ohne_grafik()` gar keine
  Eingriffs-Wache. Das ist B23 in seiner eigenen Sache, halb umgesetzt.
- **N7** — der öffentliche Lauf ist jetzt dauerhaft grün über **neun
  übersprungenen** Prüfsätzen. Grün und wertlos ist eine bekannte Klasse
  dieses Projekts; die Zahl gehört beobachtet, nicht hingenommen.

**Und zwei Befunde gegen meine eigenen Beschlüsse** (K10, K11 `fail`; K12, K13
`warn`) — beide Fassungen sind berichtigt, siehe 10.2. Der Nachlauf hat damit
in einem Zug den Sprint und die Retro geprüft, die ihn auswertet.

### 8.3 Nachtrag 08.08.2026, 00:40 — Nachprüfung am Endstand `01e1c6b`

Alles hier ist am Stand nachgemessen, nicht der Meldung entnommen.

**Geschlossen, je mit eigener Prüfung:**

| | geprüft an | Ergebnis |
|---|---|---|
| **N1** | `src/ui/notelistdelegate.cpp` | Der Term ist fort. Einziger Treffer von `round(top …)` ist der Kommentar bei `:127`, der ihn als bis 08.08.2026 bestehend ausweist — die Ankerform, nicht lebender Code. Im Bau steht nur noch `std::max(1.0, std::round(ratio))`, also die Stärke |
| **N2** | ebenda | kaufmännisch gerundet, Begründung berichtigt statt des Baus — richtig, Aufrunden machte die Linie bei 1,25 dicker als gezeichnet |
| **N3/N4/N5** | `…-s100-eingabefeld/mutationsproben.sh`, `messungen/m2-…txt` | Proben 8, 9, 10 laufen über `ohne_grafik()`; Probe 10 ist die **Positivprobe** des N3-Wächters („hat angeschlagen"), das Skript endet mit `exit 1`, das Protokoll schließt mit „alle Proben haben gemessen" |
| **N7** | `.github/workflows/ci.yml:141–148` | Der Lauf zählt die Übersprungenen aus `ctest -V` und vergleicht gegen die hinterlegte 9; bei Abweichung `::error::` und die Liste. Das ist die Wache über den Fall „grün und wertlos" |
| **M4** | `tests/captureshots.cpp:186–195` | Der Kommentar sagt jetzt das Gemessene, einschließlich der Ursache (#85, eigene `colors`-Datei) |
| **M6** | Protokollkopf | berichtigt, mit datiertem Vermerk und der Begründung, warum der Tag vorgezogen wurde |
| **M7** | alle acht #101- und alle #100-Messdateien | Kopfzeile nennt Code-Stand, Zweig **und** Sauberkeit. Die #101-Belege stehen auf `e8b20a6` — **nach** der N1-Heilung `dad9948` —, die #100-Belege auf `230b86c`; beide Angaben treffen zu. B24 wirkt |
| **M8** | `git grep` nach dem Folgetag | kein Zukunftsdatum mehr im Repositorium |

**DoD 1 am Endstand:** Bau rc=0 und **0 Warnungen** · `ctest` **10/10** ·
`lint-tidy` und `lint-clazy` je rc=0, **30 Dateien**, 0/0 · öffentlicher Lauf zu
`01e1c6b` **completed und success**.

**DoD 3 ist damit erreicht.** Kein offener `fail` — weder aus dem ersten
Review, noch aus dem Nachlauf, noch aus dem UI-Review. K5 bleibt als
begründete Verweisung an #96 bestehen (Auflage in 9, Punkt 5).

### M9 — DoD 2 ist durch die eigene Heilung ungültig geworden

**Gemessen, nicht vermutet:**

```
md5  /usr/bin/denkzetteld     7e23862e745a9381670ca89cae6b3d35   (Stand a15470f)
md5  build/bin/denkzetteld    b206a483dd251150dc4598ad3cd4c9f7   (Stand 01e1c6b)
readlink /proc/<pid>/exe  →  /usr/bin/denkzetteld, md5 7e23862e…
```

Der Abnahmebericht belegt Installation und beide Hauptwege am Stand `a15470f`.
Seither hat sich **Produktivcode geändert** — `src/ui/notelistdelegate.cpp`,
25 Zeilen, die N1-Heilung. Der laufende Dienst führt weiterhin die **alte**
Binärdatei aus; mein Neubau übersetzte null Dateien, `build/` steht also auf
dem Endstand, und die installierte Datei ist es nicht.

**Warum das kein formaler Punkt ist:** Die Änderung verschiebt die Trennlinie
in 8 von 280 gemessenen Lagen um einen Gerätebildpunkt — und die Trennlinien
sind genau das, was das Abnahmebild von #101 zeigt. Der Beleg für den Hauptweg
zeigt damit ein Bild, das der ausgelieferte Stand so nicht mehr zeichnet. Das
ist der Fall, gegen den B16 geschrieben ist, nur eine Ebene höher: nicht ein
Dienst, der die gelöschte Datei weiterhält, sondern eine Abnahme, die einen
überholten Stand belegt.

**Takt 1, Punkt 1 verlangt den Endstand.** Der Endstand ist `01e1c6b`.

*Abhilfe:* einmal installieren, Dienst beenden und neu starten, `readlink`
belegen, beide Hauptwege erneut ausführen und die zwei Bilder ersetzen — mit
datiertem Vermerk im Abnahmebericht, nicht durch Überschreiben (B17). Der
Aufwand ist der eines Durchgangs; ohne ihn ist DoD 2 für den gelieferten Stand
nicht geführt.

**Beobachtungen ohne Änderungsforderung**

- **B1** — Der README-Bildhinweis erreicht das erste Bild nicht (siehe 7.7).
- **B2** — **Die Heilung für #96 liegt bereits gemessen vor.** Der UX-Agent hat
  denselben Fehler in seiner eigenen Sonde gehabt und behoben: Ohne
  `kdeglobals` im Sandkasten färbt die Theme-Grafik aus den Voreinstellungen,
  während die Qt-Palette schon das dunkle Schema trägt — genau die zwei
  Quellen aus #96. Die Sonde kopiert das Schema seither in ihren Sandkasten
  (`sprint-09-ui-review/sonden/uxsonde.cpp`). Das ist Feld 2 einer Vorprüfung
  zu #96, fertig gemessen.
- **B3** — `b7-strichstaerke.txt` gibt die **Menge** der gefundenen Stärken aus
  (`[2]`), nicht ihre Zahl. Ein Lauf, der eine einzige Linie fände, sähe
  genauso aus wie einer, der vier findet. Dieselbe Klasse wie die Zahl der
  angefassten Linter-Dateien; behoben wäre es mit einer Zahl daneben.

## 9. done/next

**done (Planning, 07.08.2026):** Vier Vorprüfungsmessungen mit je zwei
unabhängigen Bearbeitern; zwei konsolidierte Berichte; beide Stories `size:m`
und ready; Kriterien beider Issues zweimal korrigiert; Zeichnung 3a und 4b
nachgezogen; SPEC 3.1 berichtigt; acht Issue-Titel auf echte Umlaute gezogen;
#102 angelegt; Milestone und Basis-Tag gesetzt.

**done (Umsetzung und Prüfung, 07.08.2026):** Beide Stories gebaut und
zusammengeführt; #100 zeichnet `widgets/lineedit` aus derselben `ImageSet` wie
die Hülle, #101 trennt mit zwei Haarlinien einer Farbe. Ein roter öffentlicher
Lauf gefunden, nachgestellt und geheilt. Ein karpathy-Review mit vier `fail`,
ein UI-Review ohne `fail` und mit einem `warn`, der zu einer echten Heilung im
Produktivcode geführt hat (L9). Sieben Mutationsproben zu #100, sieben zu #101,
beide Skripte mit einem Wächter, der Abbruch und Stummheit in den Rückgabewert
schreibt. Endstand einmal installiert, beide Hauptwege daran belegt.
DoD 1, 2 und die Belegprüfung erfüllt; DoD 3 mit einem offenen Punkt (M1),
DoD 4 mit zweien (M2, M3).

**next:**

1. **M9 vor der Kundenabnahme** — Endstand `01e1c6b` installieren, Dienst neu
   starten, `readlink` belegen, beide Hauptwege daran ausführen, die zwei
   Abnahmebilder mit datiertem Vermerk ersetzen. Der einzige noch offene Punkt
   aus Takt 1 (siehe 8.3 und 11).
2. M3, zweite Hälfte — die Nachbau-Bedingung in die Prüfmittel-Liste ab
   `SPEC.md:1130`. *(N1, N2, N3, N4, N5, N7 sowie M1, M2, M4, M6, M7, M8 sind
   erledigt und in 8.1 bis 8.3 einzeln nachgemessen.)*
3. M5 — die Zahl aus der Überschrift der Fallenliste ist entfernt, die Fälle
   11 bis 15 stehen; erledigt mit `da5e448`.
4. Nach der Abnahme Takt 2: AK-Haken, Issues und Milestone schließen, Journal,
   Push, Zweige und Worktrees räumen, Changelog, **Version erhöht und getaggt**
   (die Aussetzung ist seit #61 beendet — es wird der erste MINOR-Sprung nach
   der neuen Regel), Vollzugsvermerk.
5. **#96 als Kandidat für Sprint 10** — die Auflage aus K5, und die Vorarbeit
   dafür liegt gemessen vor (Beobachtung B2).

## 10. Retrospektive (nach Sprint 9)

Moderation Scrum Master, 07.08.2026. Grundlage: die Sprint-Protokolle 7 bis 9,
die Befunde beider Reviews, die Messdateien beider Stränge und die Prüfungen
aus Abschnitt 7. Kadenz nach PROZESS.md: erste Retro nach Sprint 3, danach jede
dritte.

### 10.1 Das Muster, geprüft statt übernommen

Der PO hat sechs Funde nebeneinandergelegt und gefragt, ob dahinter einer
steckt. **Es sind zwei, und die Trennung liegt nicht dort, wo man sie erwartet.**

**Fünf der sechs teilen eine Eigenschaft, die sich prüfen lässt:** Das
Prüfmittel hätte **dieselbe Ausgabe geliefert, wenn sein Gegenstand gefehlt
hätte.**

| Fund | Was fehlte, ohne dass die Ausgabe sich geändert hätte |
|---|---|
| Die zwei Abgriffe im Fenstermittelpunkt | die Hülle — der Punkt lag mit dem Feld im Textbereich |
| Strichstärke auf Skalierung 1 | der Fehler selbst; auf 1 ist jede Linie ein Punkt |
| Der Nachbau der Skalierung | die Zeilenlagen einer skalierten Sitzung — er zeigte den Fehler bei 1,25 und ausgerechnet bei 1,6 nicht |
| Der B17-Ausschluss-Griff | die Bilder; er durchsucht keine PNG und kann K5 prinzipiell nicht finden |
| Die drei Prüfgriffe des PO | die geprüfte Sache: `file` an zitierten Pfaden prüft das Zitat, ein Wortlisten-Filter findet nur Wörter der Liste, ein Zeilenzähler zählt Zeilen |

Dazu ein sechster, den ich heute selbst gefunden habe: `b7-strichstaerke.txt`
gibt die **Menge** der Stärken aus und nicht ihre Zahl — ein Lauf über eine
einzige Linie sähe aus wie einer über vier (Beobachtung B3).

**Der sechste Fund des PO gehört nicht dazu.** Bei der abgebrochenen
Mutationsprobe hat das Prüfmittel **richtig gemessen und richtig gemeldet**;
gerissen ist die Kette dahinter — der Befund stand im Protokolltext, nicht im
Rückgabewert, und der Bericht wurde gegen den Lauf davor geschrieben statt
gegen die Messdatei. Das ist ein Lesefehler, kein Messfehler, und er braucht
eine andere Abhilfe.

**Warum die Unterscheidung zählt:** Gegen die erste Klasse hilft eine Frage vor
dem Lauf, gegen die zweite ein Rückgabewert nach ihm. Wer beide unter einen
Beschluss zwingt, bekommt einen, der keinen von beiden trifft.

### 10.2 Beschlüsse

**Vier Beschlüsse, je mit Artefakt.** Die ersten drei setze ich selbst (Prozess
ist meine Fläche); B22 und die Fallenliste setzt der PO.

---

**B22 — Die Negativprobe gilt auch für Prüfgriffe, nicht nur für Prüfsätze.**

Vor jedem Griff, dessen Ergebnis in einen Bericht eingeht, eine Frage:
*Was gäbe dieses Werkzeug aus, wenn sein Gegenstand fehlte?* Ist die Antwort
dieselbe Ausgabe, prüft es nichts. Belegform ist die **Positivprobe** — der
Griff wird einmal gegen einen Fall gefahren, in dem das Gesuchte nachweislich
vorhanden ist; erst dann ist ein Nullbefund lesbar.

*Warum das über den Bestand hinausgeht:* Die Regel existiert schon — dreimal,
je für einen Einzelfall. „Ein Testaufbau, in dem der Fehler gar nicht auftreten
kann, ist kein Test" (`CLAUDE.md`), die Zahl der angefassten Linter-Dateien
(DoD 1), der Suchraum des Ausschluss-Griffs (`CLAUDE.md`). Dieser Sprint hat
sie **sechs weitere Male** neu erfinden müssen, weil sie an ihre Beispiele
gebunden war und nicht an ihre Eigenschaft. Die Mutationsprobe leistet für
Prüfsätze genau das, was hier für Prüfgriffe fehlt.

*Artefakt:* `CLAUDE.md`, Abschnitt „Prüfhaltung" — der Ort, den jede Sitzung
ohne Zutun liest. **Der PO setzt ihn**, `CLAUDE.md` ist nicht meine Fläche.

---

**B23 — Ein Befund, der nur im Protokolltext steht, erreicht niemanden.**

Jedes Prüfskript dieses Projekts endet mit einem Rückgabewert ≠ 0, sobald eine
Probe **abbricht**, **stumm bleibt** oder der Ausgangsstand ohne Eingriff rot
ist — die drei Arten, auf die eine Probe nichts misst. Und: **jede Zahl im
Bericht wird gegen die Messdatei gehalten**, nicht gegen den Lauf davor.

*Evidenz:* K3. Der Wächter hat angeschlagen und
`ABBRUCH: Eingriff ließ sich nicht anbringen` geschrieben; die Zeile im Bericht
sagte „rot". Damit war AK 4 die einzige Zusicherung von #101 ohne
Mutationsnachweis, ausgerechnet in dem Abschnitt, der „Der Nachweis, der diesen
Bericht trägt" heißt. `mutationsprobe.sh` trägt die Abhilfe bereits und hat sie
an einer eigens gebauten Probe nachgewiesen; der Beschluss hebt sie vom Skript
auf das Verfahren.

*Artefakt:* `docs/scrum/PROZESS.md`, Artefakte → Belege. **Gesetzt.**

---

**B24 — Jede Messdatei nennt Stand *und* Sauberkeit des Arbeitsbaums.**

`git rev-parse HEAD` in einem geänderten Arbeitsbaum benennt einen Commit, an
dem der gemessene Code nicht existiert. Die Kopfzeile einer Messdatei trägt
künftig Commit, Zweig **und** den Befund von

```
git status --porcelain -- . ':(exclude)<Belegordner>/messungen' \
                            ':(exclude)<Belegordner>/bilder'
```

Ist er leer, steht `sauber` da; sonst die Zahl **und die Namen** der Dateien.

*Evidenz:* Alle sieben Belege zu #101 nennen `46bb5b5` — dort gibt es
`hairline()` nicht, und Mutationsprobe 7 ließe sich dort gar nicht anbringen.
Die Ursache steht in einer Zeile: `pruefen.sh:35` schreibt
`git rev-parse --short HEAD`, ohne zu fragen, ob der Baum sauber ist. Die
Gegenrichtung ist ebenso teuer: Zu #100 nennt keine Messdatei einen Stand,
und ein Beleg ohne Stand lässt sich gar nicht erst nachfahren (B17).

**Zwei Einzelheiten des Griffs, beide gegen die naheliegende Fassung, beide
nicht von mir gefunden.** Meine erste Fassung schrieb `-uno` fest; ein
karpathy-Lauf über diese Beschlüsse hat es als `fail` zurückgegeben, weil
`-uno` ungetrackte Dateien ausblendet — eine neue, noch nicht getrackte Sonde
wäre für die Wache unsichtbar, und das ausgerechnet im Beschluss über
Nachfahrbarkeit. Und der #101-Strang hat beim Bauen gemessen, dass
„Belegordner ausnehmen" die falsche Fassung ist: Die Prüfskripte liegen darin.
Sein erster Anlauf nahm den ganzen Ordner aus und meldete „sauber", während
beide Skripte geändert waren. Ausgenommen gehören allein die
**Ausgabeverzeichnisse**, die der Lauf selbst vollschreibt.

*Artefakt:* `docs/scrum/PROZESS.md`, Artefakte → Belege. **Gesetzt.**

---

**B25 — Der Review-Diff endet am Stand, an dem der Review lief.**

Vor der Kundenabnahme wird gefahren:

```
git diff --stat <Reviewstand>..HEAD
```

Ist die Ausgabe nicht leer, braucht es einen Nachlauf über genau diesen Bereich
oder eine Feststellung im Protokoll, die **je geänderter Datei** sagt, warum
sie keinen zweiten Blick braucht.

*Evidenz:* In diesem Sprint 455 Zeilen nach dem Review, davon 41 Produktivcode
— und der Anlass war der Review selbst. **Das ist die Bauart jedes Sprints:**
Der Review findet Befunde, die Befunde werden geheilt, die Heilungen sieht
niemand. B19 hat den Anfang des Diffs festgenagelt; sein Ende stand nirgends.

**Ohne Pfadfilter — und diese Zeile ist mein eigener Fehler, korrigiert.**
Meine erste Fassung zählte `src/ tests/ CMakeLists.txt .github/` auf. Der
karpathy-Lauf über diese Beschlüsse hat sie als `fail` zurückgegeben, mit dem
Satz, ein Beschluss über die Vollständigkeit eines Diffs, der selbst
unvollständig aufzählt, sei eine Falle mit Anlauf. Er hat recht, und beim
Nachmessen wird es schärfer, als er es formuliert hat: **Auch die
Ausschlussform trüge hier nicht.** Von den neun Befunden des
Sprint-9-Reviews zeigen sechs auf `docs/scrum` und zwei auf `docs/bilder`;
**K5 saß ausschließlich in zwei PNG-Dateien.** Ein Ausschluss der Belegarchive
— die Form, die der B17-Griff zu Recht nimmt — hätte K1, K3, K4 und K8
verworfen, sämtlich Befunde gegen Bericht- und Messdateien. Bei diesem Griff
darf **nichts** ausgeschlossen werden: Was ein Filter spart, ist Lesezeit; was
er kostet, ist die Vollständigkeitsaussage, für die der Griff gefahren wird.
**Der Unterschied zum B17-Griff ist die Frage, nicht die Technik** — B17 fragt,
welche Aussagen nachzuziehen sind (und ein Beleg wird geankert, nicht
nachgezogen), B25 fragt, was ungeprüft ausgeliefert wird.

*Artefakt:* `docs/scrum/PROZESS.md`, **Definition of Done, Punkt 3** — neben
B19, der den Anfang des Diffs festlegt. Nicht in die Abschlussliste, weil die
Nummern dort von mehreren Stellen zitiert werden und eine Einfügung sie still
verschieben würde; DoD 3 wird ohnehin je Story geprüft (Takt 1, Punkt 3).
**Gesetzt.**

### 10.3 Vier Zugänge zur Fallenliste

Der `denkzettel-dev` und der `denkzettel-ux` haben je zwei Fälle gemeldet.
Sie gehören in `.claude/agents/denkzettel-dev.md` — die Liste „Rückgabewerte
und Läufe, die nichts belegen" ist die gemeinsame Liste dieses Projekts, und
sie fordert ihre Erweiterung selbst ein. **Der PO setzt sie** (Agentendateien
sind nicht meine Fläche). Mit ihnen fällt die Zahl aus der Überschrift (M5).

| | Fall | Fundstelle |
|---|---|---|
| **11** | **Der Vergleich einer Datei mit sich selbst.** Die Rücknahme eines Eingriffs mit `cp`/`mv` trägt die Uhrzeit des Kopierens — die liegt **vor** dem Eingriff. `make` hält die Quelle danach für älter als das Objekt und baut nicht neu; der nächste Lauf misst den vorigen Eingriff mit. `cp -p` plus `touch` behebt es | `#101`-Bericht §3.1 |
| **12** | **Eine Wache über die Summe der Eingriffe wacht über keinen einzelnen.** Eine Probe mit drei Eingriffen, von denen der zweite ins Leere lief: Die Wache sah eine veränderte Datei, ließ die Probe laufen, und sie meldete „rot" — plausibel, weil die Nachbarprobe dasselbe meldet. Die Wache prüft **jeden** Eingriff einzeln über eine Prüfsumme | `#100`-Bericht §8 |
| **13** | **Ein Sandkasten ohne `kdeglobals` färbt die Theme-Grafik anders als die Qt-Palette.** Das Bild zeigt dann helle Schrift auf hellem Grund und sieht aus wie ein Fehler des Erzeugnisses. Die Sonde kopiert das Farbschema in ihren Sandkasten. **Das ist zugleich die Ursache von #96** | `sprint-09-ui-review/bericht.md`, „Zwei Fehler der Sonde" |
| **14** | **`show()` statt `showCapture()` liefert ein Fenster ohne Schatten.** Der Schatten wird in `present()` an die frische Wayland-Fläche gebunden; wer am regulären Weg vorbei zeigt, bekommt ein Bild, auf dem ein zugesicherter Zustand fehlt, den das Erzeugnis herstellt | ebenda |

In `.claude/agents/denkzettel-ux.md` gehört eine Zeile, die auf diese Liste
verweist — die Fälle 13 und 14 stammen von dort und stehen nicht dort.

### 10.4 Revision der Modellzuordnung (fällig nach Sprint 9)

**Bestätigt.** Die Belege dieses Sprints:

- **Das Sicherheitsnetz auf Fable fand vier `fail`** (K1, K2, K3, K5) und im
  Nachlauf einen weiteren (N1). Der aussagekräftigste ist **K3**: Der
  Bearbeiter — ein `denkzettel-dev` auf Opus — hatte den Prüflauf selbst
  gebaut, sein eigener Wächter schrieb `ABBRUCH` in die Messdatei, und im
  Bericht stand „rot". Ein Befund, den das Werkzeug bereits gedruckt hatte,
  brauchte einen frischen Leser.
- **Opus trägt seine Rollen.** Der `denkzettel-ux` fand L9 — einen echten
  Fehler im ausgelieferten Bild, im Material des Umsetzungsstranges selbst —,
  und der karpathy-Reviewer fand ihn nicht. Der Befund führte zur einzigen
  Produktivcode-Heilung nach dem Review. Kein Anlass, an der Zuordnung der
  bauenden und beratenden Rollen etwas zu ändern.
- **`denkzettel-verwalter` (Haiku): kein Auftrag in diesem Sprint**, weil
  Takt 2 noch aussteht. Keine neue Evidenz, keine Änderung.

**Meine erste Fassung behauptete mehr, als sie gemessen hatte — zwei
karpathy-Läufe haben sie zurückgegeben, und beide hatten recht.** Sie zählte
den PO mit („zweimal an einer Opus-Rolle vorbeigegangen") und sprach von „vier
Opus-Rollen zuvor". Beides trägt nicht:

1. **Das Modell des PO ist nirgends festgehalten.** Die Modellzuordnung in
   `PROZESS.md` regelt Agenten; der PO ist „Claude (Haupt-Session)" und
   modellseitig unbestimmt. Eine Aussage über seine Rolle kann in einer
   Modellrevision nicht als Beleg dienen. Damit ist der Fall **nicht schärfer**
   als der aus Sprint 6, sondern ein zweiter seiner Art.
2. **Zwei der vier Prüfungen liefen nach dem Reviewstand.** `70902a4` ist von
   22:35; der UI-Review-Commit von 23:08, meine DoD-Prüfung von 23:41
   (nachgemessen). Wer nach dem Reviewer prüft, hat nicht zuvor nichts
   gefunden.
3. **Und die tiefere Grenze, die keiner der beiden Reviewer genannt hat:**
   Dieses Verfahren kann **Modell und frischen Kontext nicht trennen**. Der
   Reviewer bringt beides zugleich mit, also ist jeder seiner Funde mit beiden
   Erklärungen verträglich. Wer die Zuordnung künftig **ändern** will, braucht
   einen Lauf, der genau eine der beiden Größen bewegt — etwa denselben Diff
   von einem Opus-Reviewer in frischem Kontext. Bis dahin ist „bestätigt" die
   einzige Aussage, die diese Belege tragen.

Das steht hier, weil eine Modellentscheidung sonst in einem Jahr auf einer
ungezählten Zählung ruht. Der Befund gegen die eigene Retro ist zugleich der
beste Beleg dafür, dass der frische Kontext trägt.

**Nächste Revision: Retro nach Sprint 12.**

### 10.5 Abschlussprüfung der Retro — zwei Fragen, beide beantwortet

**1. Ist jeder Beschluss in einem Artefakt gelandet?**

| Beschluss | Artefakt | Stand |
|---|---|---|
| B22 | `CLAUDE.md`, „Prüfhaltung" | **offen — der PO setzt** |
| B23 | `PROZESS.md`, Artefakte → Belege | gesetzt |
| B24 | `PROZESS.md`, Artefakte → Belege | gesetzt |
| B25 | `PROZESS.md`, Definition of Done, Punkt 3 | gesetzt |
| Fälle 11–14 | `.claude/agents/denkzettel-dev.md` | **offen — der PO setzt** |
| Modellzuordnung | `PROZESS.md`, Modellzuordnung | gesetzt |

**2. Wird dieses Artefakt automatisch gelesen?**

- `CLAUDE.md` — ja, von jeder Sitzung im Projektstamm. Deshalb liegt B22 dort
  und nicht in `PROZESS.md`: Er richtet sich an jeden, der einen Prüfgriff
  fährt, und das ist nicht nur das Scrum-Team.
- `.claude/agents/denkzettel-dev.md` — ja, beim Spawn jedes Dev-Stranges.
- `docs/scrum/PROZESS.md` — **nicht von selbst.** B23, B24 und B25 richten sich
  an Rollen, die `PROZESS.md` als verbindliche Grundlage vor der Arbeit lesen
  (Scrum Master, PO, Dev laut Agentendatei); für sie trägt es. Ein Beschluss,
  der jede Sitzung erreichen soll, gehört nach `CLAUDE.md` — das ist der Grund
  für die Aufteilung oben und nicht ein Versehen.

**Was diese Retro nicht geleistet hat, ausdrücklich:** Die Änderungen an
`PROZESS.md` sind Prozess-Artefakt-Änderungen und durchlaufen nach der globalen
Regel den karpathy-Reviewer. **Der Aufruf ist inzwischen erfolgt — zweimal**,
und beide Läufe haben `fail` gegen diese Retro gemeldet: B25 zählte Pfade auf,
statt auszuschließen; B24 schrieb `-uno` fest, das ungetrackte Dateien
ausblendet; die Modellrevision behauptete eine Zählung, die ihre Grundlage
nicht hatte. Alle drei sind berichtigt (10.2, 10.4). **Der Beschluss über
Prüfmittel, die ihren Gegenstand nicht erreichen, ist an seinem eigenen
Nachbarbeschluss gerissen** — das ist kein Nebenbefund, sondern der beste
verfügbare Beleg dafür, dass B22 nötig ist.

## 11. Vollzugsvermerk Takt 1

Geführt vom Scrum Master, 08.08.2026, 00:40, am Endstand `01e1c6b`. Jeder Punkt
mit Beleg abgehakt, wie die Liste in `PROZESS.md` es verlangt.

| Punkt | Stand | Beleg |
|---|---|---|
| **1** Endstand installiert, Hauptweg jeder Story daran ausgeführt | **offen — M9** | `/usr/bin/denkzetteld` trägt `7e23862e…` (Stand `a15470f`), der Endstand `b206a483…`. Der Abnahmebericht belegt einen überholten Stand; die Trennlinie hat sich seither bewegt |
| **2** jeder Prüflauf hat einen Bericht als Datei · Prüfsummen der Bildbelege | **erfüllt** | Fünf Berichte im Repo, Commit-Botschaften dagegengehalten, kein Lauf ohne Bericht. Prüfsummenlauf über vier Ordner, 12 Gruppen einzeln bewertet, kein Mangel (7.6) |
| **3** DoD 1–4 je Story · Doku-Abgleich · **B25-Griff einmal je Sprint** | **erfüllt** | DoD 1 und 3 in 8.3 am Endstand nachgemessen, DoD 4 mit `58bd93f` und `dad9948`/`01e1c6b` geschlossen; Doku-Abgleich in 7.7, Restpunkte M4 und M5 behoben. Der B25-Griff hat in diesem Sprint gegriffen und den Nachlauf ausgelöst |
| **4** Mängelliste an den PO | **erfüllt** | Abschnitt 8 samt der Nachträge 8.1 bis 8.3; neun Nummern, acht geschlossen, **M9 offen** |

**Urteil: Takt 1 ist nicht abgeschlossen.** Es fehlt ein einziger Punkt, und es
ist der erste. Ohne ihn legt der PO dem Kunden einen Stand vor, dessen
Abnahmebilder ein anderes Bild zeigen als die Anwendung, die auf seiner
Maschine liegt.

**Takt 2** folgt nach der Kundenabnahme; die offene Arbeit steht in 9, Punkt 4,
einschließlich des ersten Versionssprungs nach dem Ende der Aussetzung
(`v0.3.0`) und des Verwalter-Berichts, dessen Datei ich mit `git ls-files`
prüfe.
