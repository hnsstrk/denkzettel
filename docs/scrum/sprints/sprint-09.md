# Sprint 9

**Sprint-Ziel:** Der Nutzer erkennt, wo er tippt — und wo eine Notiz aufhört.

**Basis-Tag:** `sprint-09-basis` = `4be3f8d` · **Milestone:** Sprint 9 ·
**Planning:** 07.08.2026

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

*Am Sprint-Ende auszufüllen.*

## 8. Mängelliste

*Am Sprint-Ende auszufüllen.*

## 9. done/next

**done (Planning, 07.08.2026):** Vier Vorprüfungsmessungen mit je zwei
unabhängigen Bearbeitern; zwei konsolidierte Berichte; beide Stories `size:m`
und ready; Kriterien beider Issues zweimal korrigiert; Zeichnung 3a und 4b
nachgezogen; SPEC 3.1 berichtigt; acht Issue-Titel auf echte Umlaute gezogen;
#102 angelegt; Milestone und Basis-Tag gesetzt.

**next:** Beide Stränge spawnen (Worktrees `../denkzettel-100` und
`../denkzettel-101`), mit den gemessenen Fallen aus Feld 2 der jeweiligen
Berichte im Auftrag.
