# Karpathy-Review — Sprint 7

**Prüfgegenstand:** `git diff sprint-07-basis..main` (146 Dateien, +7.577/−115),
Prüflauf vom 05.08.2026, Reviewer in frischem Kontext (Fable 5).

## Karpathy-Review

**Task:** Der Sprint-7-Diff (Strang A #83, Strang B #71/#70/#72, PO-Nacharbeit
und Vorprüfungen) gegen die vier Arbeitsprinzipien prüfen, mit Zweitblick auf
Mutationsproben-Zählung, die D-Bus-Bauentscheidung und die benannten Grenzen.

**Gesamt-Verdict:** warn

### Befunde

| ID | Prinzip | Verdict | Ort | Befund | Status |
|----|---------|---------|-----|--------|--------|
| K1 | 4 Goal-Driven | warn | reviews/sprint-07-s83-native-huelle/bericht.md:159,182 | „Fünfzehn tragende Zusicherungen … Vierzehn von fünfzehn belegt" zählt falsch: Es sind **zwölf** Zusicherungen (S1–S3 sind laut eigener Tabelle „dieselbe wie 10/11/12, in der Sitzung"), davon **elf** durch rote Proben belegt und eine benannt; die Proben 10–12 blieben offscreen grün und belegen selbst nichts. Die abgelegte Probenzahl (12 in `m12` + 3 in `m14` = 15) stimmt, die Kopfzahlen nicht | offen |
| K2 | 4 Goal-Driven | warn | reviews/sprint-07-s71-ruhige-liste/bericht.md:103 | „beide Läufe 113/113 grün" widerspricht den abgelegten Belegen: `messungen/72-lauf-C.txt` meldet **112 passed**, `72-lauf-de_DE.txt` **110 passed** (beide 0 failed). Grün stimmt, die Zahl nicht — und dass der Prüfsatzumfang gebietsabhängig ist, bleibt unerklärt | verworfen \|
| K3 | 4 Goal-Driven | warn | reviews/sprint-07-s71-ruhige-liste/bericht.md:15; pruefen.sh:5,69 | „Alles hier ist mit pruefen.sh wiederholbar" trägt für die Mutationsproben nicht: Das Skript listet die 14 Ausgabedateien nur auf, die Eingriffe selbst sind nirgends maschinenlesbar festgehalten (Strang A zeigt mit `mutationsproben.sh`, wie es geht). Dazu zählt das Skript zweimal „zwölf" bei vierzehn abgelegten Läufen | offen |
| K4 | 1 Think Before Acting | warn | src/capture/capturewindow.h:166; .cpp:177 | `m_blursBehind` wird **einmal** im Konstruktor erhoben und ist `const`: Schaltet der Kunde den Weichzeichner-Effekt zur Laufzeit um, behält das Fenster die alte Antwort und damit die falsche Grafik-Fassung bis zum Dienstneustart. Weder Bericht §4 (Grenzen) noch SPEC 3.2 Punkt 9 nennen diese Grenze | offen |
| K5 | 3 Surgical (B17) | warn | SPEC.md:151; wireframes/Denkzettel Wireframes.dc.html:775 | SPEC 3.1 verweist weiter auf „Wireframe 4a/4b", und die Zeichnung beschreibt noch den abgewählten Nachbau — `frameContrast`-Kontur als „die **einzige Linie** im Fenster". Gemeldet hat es niemand: Der dokumentierte B17-Griff (sprint-07.md:178) durchsucht `wireframes/` nicht | offen |
| K6 | 4 Goal-Driven | warn | docs/scrum/sprints/sprint-07.md:196 gegen Commit c488ab5 | Die eigene Festlegung lautet „Fällig **nach** der Abnahme von #83, nicht davor" — der Nachzug der drei `tinted()`-Fundstellen ist aber committet, während §7 (DoD-Prüfung) noch leer steht. Sachlich ungefährlich (`tinted()` ist auf `main` gefallen), aber gegen den eigenen Takt und unprotokolliert | offen |
| K7 | 4 Goal-Driven | warn | docs/scrum/reviews/sprint-07-ui-review/ (leer, unversioniert) | Der UI-Review zu Sprint 7 fehlt im Diff — der Ordner existiert nur leer und untracked. Drei der vier Stories sind laut Planning UI-Stories (DoD 3); vor der Abnahme fällig. Vermutlich in Arbeit — geführt, damit der Punkt nicht zwischen zwei Takten durchfällt | offen |

**Berichtigung 05.08.2026 — K2 verworfen (Reviewer-Fehler, nicht Berichtsfehler):**
Der Befund beruhte auf einer Verwechslung zweier Dateipaare desselben Ordners.
Tatsächlich steht in den Dateien: `72-lauf-C.txt:1` und `72-lauf-de_DE.txt:1`
melden **je 113 passed, 0 failed** — der Satz „beide Läufe 113/113 grün" im
Übergabebericht ist richtig. Die Zahlen 112 und 110 stehen in
`70-testauflage-nach-heilung.txt:113` und `71-testauflage-nach-heilung.txt:111`
und sind stimmig statt widersprüchlich: 110 nach #71, 112 nach #70 (zwei neue
Prüfsätze), 113 nach #72 (einer) — die Folge belegt die Baureihenfolge. Die
Verwechslung lag nahe, weil die vier Dateien nebeneinander liegen und sich nur
im `70-`/`71-`/`72-`-Präfix unterscheiden; der Reviewer hatte die Totals ohne
Dateinamen (`grep -h`) erhoben und den Zeilen die falschen Dateien zugeordnet.
Nachgeprüft am 05.08.2026 mit `grep -n` je Datei.

### Antworten auf die drei PO-Fragen

1. **Sind die Belege echt?** Ja, mit einer Zählkorrektur. **Strang A:** 15
   Proben behauptet, 15 abgelegt (`m12`: Proben 1–12, `m14`: S1–S3) — aber nur
   12 verschiedene Sachverhalte; S1–S3 wiederholen 10–12 in der Sitzung, was
   die Tabelle selbst offen sagt und was sachlich richtig ist (offscreen kann
   diese drei Fehler nicht zeigen). Die Kopfzahlen sind falsch (K1). Die roten
   Proben 1–8 treffen nachweislich je einen anderen Eingriff mit je anderen
   fallenden Prüfsätzen. **Strang B:** 14 behauptet, 14 abgelegt (m1–m14),
   **alle 14 treffen verschiedene Sachverhalte** — nachgezählt an Eingriff und
   fallender Zusicherung; M10 ist grün geblieben und als Grenze ausgewiesen
   (B-2/B-3), M5 ist eine saubere Negativkontrolle (zeigt, welche Zusicherungen
   den Beweis *nicht* tragen). Der Sprint-6-Fehler (8 von 11 gedeckt) wiederholt
   sich nicht.
2. **Ist etwas gebaut worden, das niemand verlangt hat?** Nein. Die
   D-Bus-Abfrage folgt direkt aus AK 7; die Alternativen sind gemessen
   verworfen (`isEffectAvailable` lügt vor der Erstanmeldung — Vorprüfung F9;
   `wayland-client` wäre eine neue harte Abhängigkeit samt CI-Nachzug).
   `Qt6::DBus` steht tatsächlich schon in SPEC 15 als Projektabhängigkeit —
   am Quelltext bestätigt; neu ist nur der Link an `denkzettelcapture`. Die
   Gleichwertigkeit beider Wege ist gemessen, die Plattformwache hält den
   Testlauf deterministisch, die 1-s-Zeitgrenze ist begründet. Einziger
   unbenannter Rest: die einmalige Erhebung (K4).
3. **Sind Grenzen benannt oder verdeckt?** Benannt, und zwar mit Beleg: §4/§7
   (A) und B-1–B-7 (B) tragen je Messung und Folge-Schritt; nichts davon
   umgeht erkennbar unbequeme Arbeit — B-2 und B-3 sind Bestandsbefunde
   außerhalb der Story-Fläche („melden, nicht heilen" korrekt angewandt), B-6
   ist im Diff bereits behoben (Vorbericht #70 ist eingecheckt). **Eine**
   unbenannte Grenze hat der Zweitblick gefunden: K4.

### Fix-Vorschläge

- **K1** → Zwei Sätze berichtigen: „Zwölf tragende Zusicherungen, fünfzehn
  Proben — elf Zusicherungen rot belegt, eine benannt; drei Proben laufen
  doppelt, weil ihr Fehler nur in der Sitzung auftritt."
- **K2** → entfällt — Befund verworfen, siehe Berichtigung unter der Tabelle.
- **K3** → In `pruefen.sh` zweimal „zwölf" → „vierzehn"; den Satz in
  bericht.md:15 einschränken **oder** die 14 Eingriffe als je eine Zeile
  (sed-Fassung) in Bericht oder Skript aufnehmen.
- **K4** → Ein Satz in SPEC 3.2 Punkt 9 und Bericht §4: Die Antwort wird beim
  Start einmal erhoben; ein Laufzeit-Umschalten des Effekts greift erst nach
  Dienstneustart. Ob das ein Issue wert ist, entscheidet der PO.
- **K5** → Als Befund an UX/PO buchen (Wireframes sind nicht Strang-Fläche);
  zusätzlich erwägen, den B17-Griff in `CLAUDE.md` um `wireframes/` zu
  erweitern — sonst bleibt die Lücke strukturell.
- **K6** → Datierte Zeile in §6a: Nachzug erfolgte nach dem Merge, vor der
  protokollierten Abnahme, mit Grund (`tinted()` auf `main` gefallen; eine
  stehengelassene Fundstelle wäre die B17-Falle gewesen) — oder die Festlegung
  anpassen.
- **K7** → UI-Review abschließen und versionieren, bevor §7 (DoD-Prüfung)
  gefüllt wird.

### Was gut ist

- **Die Mutationsproben von Strang A sind skriptiert, laufen auf einer Kopie
  und sind damit wirklich wiederholbar** — genau die Konsequenz aus dem
  Sprint-6-Nachzählbefund. Der erste `m12`-Lauf hat mit Probe 7 einen Prüfsatz
  entlarvt, der nichts prüfte, und der Bericht sagt es offen (§5, Fund 2).
- **Strang B prüft nicht nur, dass Prüfsätze fallen, sondern welche
  Zusicherung den Beweis trägt** (M5 als Negativkontrolle) — das ist über die
  Anforderung hinaus ehrlich.
- **Surgical gehalten:** Beide Stränge sind in ihrer Dateimenge geblieben; die
  zwei Berührungen fremder Belegordner sind ausgewiesen und minimal. Verwaiste
  Verweise auf `tinted()`/`m_hullInner`/`OutlineWidth` im Produktbaum: keine
  (per grep geprüft; einzige Ausnahme ist der Wireframe, K5).
- **Der Regelstellen-Nachzug (c488ab5) trifft exakt die drei angekündigten
  Fundstellen** und unterscheidet sauber Beispiel und Mechanismus; die zwei
  neuen Fälle in `denkzettel-dev.md` (ImageSet-Selektoren, Theme-Wahl des
  Prüfsatzes) sind die verlangte Verankerung von Funden, nicht Scope-Drift.
- **SPEC 3.2 Punkte 6–9** dokumentieren die beim Bau entdeckten Bedingungen
  mit Messbezug (DoD 4/B9), einschließlich der unbequemen („keine einzige
  meldet ihren Fehlschlag über einen Rückgabewert").
- **Der Autoscroll-Nachtrag zu #71** nimmt eine eigene Aussage ausdrücklich
  zurück statt sie zu glätten, und eskaliert die Heilung als
  Produktentscheidung (B-1) — Prinzip 1 wie gefordert.
- **Die Vorberichte #61/#70/#72/#76 sind beauftragte Vorarbeit** (Kundenauftrag
  05.08.2026, sprint-07.md §1), kein ungefragter Umfang; das Sprint-Protokoll
  benennt die Rollenabweichung selbst und sammelt die PO-Entscheidungen an
  Kundenstelle prüfbar an einem Ort.
