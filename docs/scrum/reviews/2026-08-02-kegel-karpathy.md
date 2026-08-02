# Karpathy-Review — Vorgang „Schätzkegel" (Kundenauftrag 02.08.2026)

**Task:** Schätzhistorie als versionierte Datenreihe mit deterministischem
Diagramm-Generator verankern — Pflicht (Takt 2 Punkt 12), Prüfsatz (DoD),
Rollenteilung (SM urteilt, Verwalter überträgt, PO committet), README-Einbindung.
Geprüfter Diff: `7247500..ca8e75c` auf `PROZESS.md`,
`.claude/agents/denkzettel-verwalter.md`, `docs/scrum/diagramme/`, `README.md`,
`sprint-05.md`.

**Gesamt-Verdict:** fail — ein Think-fail (README-Prosa behauptet Monotonie,
die die Daten nicht tragen), dazu zwei substanzielle Goal-Driven-warns an der
Fortschreibungskette. Alles mit kleinem Aufwand behebbar; Generator, Daten und
Prozessverankerung sind davon abgesehen in ungewöhnlich gutem Zustand.

---

## Prinzip 1 — Think Before Acting

**Verdict:** fail

**Beobachtungen:**

- **fail — `README.md:13-15`:** „Der Kegel zeigt, wie stark Schätzungen später
  revidiert wurden — **je weiter eine Schätzung von der Umsetzung entfernt lag,
  desto mehr.**" Das ist eine Monotonie-Behauptung, die die Daten nicht tragen.
  Nachgerechnet (siehe unten): Die log-Weiten je Abstand sind
  **0,51 · 0,51 · 0,92** — zwischen Abstand 1 und 2 weitet sich nichts, und die
  Weitung bei Abstand 3 ruht auf einem einzigen Punkt (#11), der als
  `gegenstand-geändert` gekennzeichnet ist, also nach eigener Legende „keine
  Schätzabweichung" misst. Das Projekt hat seinen Maßstab selbst gesetzt:
  *„Wer hier ‚monoton weitend' schreibt, sagt mehr, als die sieben Punkte
  hergeben"* (`sprint-05.md:1215-1216`). Verschärfend: Der Satz *„Sie weitet
  sich monoton — das ist der gemessene Kegel"* wurde in Commit `7699791`
  ausdrücklich aus dem Protokoll **gestrichen** — und in `ca8e75c`, also
  danach, sinngemäß ins README geschrieben, an die öffentlichste Stelle des
  Repos. Die SVG-Caption direkt darunter (`kegel.svg:80-82`) widerspricht der
  Prosa daneben. Der Aufwands-Vorbehalt („misst nicht den Abstand zum
  tatsächlichen Aufwand", `README.md:15-16`) ist vorhanden; der
  Monotonie-Vorbehalt fehlt genau dort, wo die Monotonie behauptet wird.
- **ok — Hüllwerte selbst nachgerechnet, sie stimmen:**
  Abstand 1 → [0,60; 1,00] (#5, #6, #7), Abstand 2 → [1,00; 1,67]
  (#8, #9, #57, #58), Abstand 3 → [1,00; 2,50] (#11, #12). Im Logarithmus
  ln(5/3) = 0,5108 (zweimal, betragsgleich für 0,60 und 1,67) und
  ln(2,5) = 0,9163 → die gedruckten 0,51 · 0,51 · 0,92 sind korrekt.
  „Nicht-fallend, aber nicht streng wachsend" ist die richtige Form; die
  Gleichheit der ersten beiden Weiten wird in `kegel.py:210-211` mit
  1e-9-Toleranz sauber behandelt (Float-Differenz der beiden ln(5/3)-Wege
  liegt weit darunter).
- **ok — Faktor-Nachrechnung:** 3/5 = 0,60 · 5/3 = 1,667 gegen gedruckt 1,67 ·
  3/2 = 1,50 · 5/2 = 2,50. Die Toleranz 0,01 in `kegel.py:120` deckt genau die
  Rundung des Protokolls und nicht mehr.
- **ok — Vorbehalte im Bild vollständig und an der richtigen Stelle:** Erster
  Caption-Absatz ist der Revision-nicht-Aufwand-Satz (`kegel.svg:77-79`,
  in `kegel.py:245-250` als Bedingung kommentiert, mit Verweis auf PROZESS
  Punkt 12); „ruht auf einem einzigen Punkt" samt #11-Einordnung steht im Bild
  (`kegel.svg:81-83`); der Alt-Text (`README.md:11`) ist neutral beschreibend.
  Der Untertitel behauptet nur Zählbares (9/13, von mir gegen die JSON
  gezählt: 9 × `in_kurve: true`, 13 × `false` — stimmt).
- **ok — §24.4 (`sprint-05.md:1302-1322`):** Der vom Dev gefundene Widerspruch
  (#11 zugleich aus der Hülle gefordert und Träger der Weitung) ist benannt,
  die Entscheidung dreifach begründet, und der Weg für die Gegenposition
  („`envelope()` **und** 24.1 gemeinsam ändern") steht dabei — Annahmen
  explizit statt still.

## Prinzip 2 — Simplicity First

**Verdict:** warn

**Beobachtungen:**

- **warn — die Verwalter-Choreografie steht vierfach:** „überträgt mechanisch ·
  meldet den Diff · trägt keine eigene Zeile ein · der PO committet" in
  `PROZESS.md:313-322`, `denkzettel-verwalter.md:61-69`, `LIESMICH.md:37-43`
  und `schaetzhistorie.json:5` (`_fortschreibung`). Norm (PROZESS) plus
  Arbeitsanweisung (Agentendatei) sind durch die Projektregel „das Artefakt
  muss automatisch gelesen werden" gedeckt; die dritte und vierte Kopie sind
  Pflegelast in einem Vorgang, dessen erklärtes Ziel Drift-Vermeidung ist.
  Mindestens `_fortschreibung` in der JSON dupliziert die LIESMICH in
  derselben Mappe und könnte ein Verweis sein.
- **ok — Transkriptionsschutz (`kegel.py:106-125`) trägt einen realen Fall:**
  M2 hat gemessen, dass falsche Zahlen still falsch aussehen (3 statt 11 SP
  aus den Labeln); der doppelt geführte Faktor macht den Zahlendreher zum
  Abbruch. Kein Overengineering, sondern die Antwort auf einen belegten
  Vorfall.
- **ok — die 13 nicht gezeichneten Zeilen tragen die Bildaussage** („13
  erfasst und nicht gezeichnet" im Untertitel und in der Caption) und den in
  §24.2 begründeten Zweck, die Auslassung sichtbar zu halten. Ohne sie wären
  die Sätze im Bild nicht rechenbar.
- **ok — `stand`-Feld:** trägt die Standangabe im Untertitel; Commit `6378e7d`
  zeigt die bewusste Verlagerung aus dem Code in die Daten. (Zur offenen
  Trägerfrage siehe Prinzip 4.)
- **ok — Generator-Umfang:** 499 Zeilen ohne Fremdbibliothek sind durch das
  Determinismus-Ziel begründet (keine Bibliotheksversion, die in die Datei
  durchschlägt); keine CLI-Optionen, keine Konfigurierbarkeit, nichts
  Spekulatives. Die Crowding-Logik (`kegel.py:183-191`) löst eine im Bild
  real vorhandene Kollision (#8/#57, sichtbar in `kegel.svg:55`).

## Prinzip 3 — Surgical Changes

**Verdict:** ok

**Beobachtungen:**

- **Punkt 12 ist reine Addition hinter dem bestehenden Punkt 11** — kein
  bestehender Punkt verschoben, kein angrenzender Text verändert (Diff zeigt
  ausschließlich `+`-Blöcke an zwei Stellen). Das Anhängen als 12 statt
  Einschieben ist belegt richtig: Die Nummern 5–11 werden zitiert in
  `sprint-03.md:1682-1683`, `sprint-04.md:359/431/792/817/969` sowie in drei
  Review-Dateien (`2026-08-02-entscheidungen-karpathy.md`,
  `sprint-03-retro-karpathy.md`, `…-nachpruefung.md`) — Einschieben hätte
  diese Verweise gebrochen. Dass der numerisch letzte Punkt nun hinter dem
  Vollzugsvermerk (11) steht, ist unschädlich: Der Vermerk deckt „Takt 2",
  nicht „Punkte 1–10".
- **Agentendatei:** reine Addition zwischen „Milestones schließen" und
  „Abgleichsberichte", Stil (Bindestrichliste, Fetttitel, Klammerverweis)
  gematcht; Verbotsliste unberührt. Kein neuer
  Arbeitsliste-gegen-Verbotsliste-Konflikt: JSON und SVG stehen nicht auf der
  Verbotsliste (`denkzettel-verwalter.md:78-79`), `kegel.py` wird ausgeführt,
  nicht geändert.
- **README:** Der Kegel-Commit `ca8e75c` fügt nur den eigenen Abschnitt ein;
  die im Gesamt-Diff sichtbare Änderung des Status-Absatzes stammt aus
  `460f650` (M3-Behebung, eigener Vorgang vor dem Kegel).
- **`7699791` räumt nur die eigene frühere Fassung** (Anhang von §12.x nach
  §24.x wegen doppelt vergebener Abschnittsnummer, in der Commit-Message
  benannt; dabei die Monotonie-Überbehauptung gestrichen) — eigene Fläche,
  keine fremden Inhalte berührt.
- **Nach meinen Prüfläufen:** `git status --porcelain` leer.

## Prinzip 4 — Goal-Driven Execution

**Verdict:** warn

**Beobachtungen:**

- **Determinismus selbst nachgemessen — hält:** Drei Läufe von `kegel.py`
  liefern bytegleich `77912c3c0f6503ad209855fda0dd5949`, identisch mit dem
  committeten SVG; Arbeitsbaum danach sauber. Im Code keine Zeit-, Zufalls-
  oder Umgebungsabhängigkeit (keine entsprechenden Importe, explizite
  Sortierungen `kegel.py:146/174`, Festpräzision `kegel.py:75`).
- **Der Verwalter kann den Lauf ausführen:** Werkzeuge `Bash, Write`
  vorhanden (`denkzettel-verwalter.md:13`), Aufruf steht in `LIESMICH.md:8-10`
  (nur Standardbibliothek, keine Installation), Datenquelle und
  Abbruchverhalten sind beschrieben. Die Kette Pflicht (Punkt 12) → Prüfsatz
  (DoD) → Ausführender (Agentendatei) → Werkzeug (LIESMICH) existiert
  vollständig, und jedes Glied liegt in einem automatisch geladenen Artefakt.
- **warn — das `stand`-Feld hat keinen benannten Träger:** Der Verwalter
  „ändert keinen Wert" (`denkzettel-verwalter.md:67`, `PROZESS.md:319-320`);
  `schaetzhistorie.json:20` sagt nur passivisch, das Feld „wird beim
  Fortschreiben mitgezogen". Ein wörtlich gehorchender Haiku-Agent lässt
  „Sprint 5" stehen — und der DoD-Prüfsatz (`PROZESS.md:209-217`) fängt das
  nicht: Er prüft „aus der aktuellen Datenreihe erzeugt", und ein Bild mit
  veraltetem Stand *ist* aus der aktuellen Datenreihe erzeugt, wenn die JSON
  den alten Stand trägt. Genau die stille Veralterung, die `_stand_hinweis`
  verhindern will, rutscht durch beide Prüfungen.
- **warn — der Spaltensatz des DoD-Prüfsatzes trägt die Übertragung nicht:**
  `PROZESS.md:210-213` verlangt „Issue · Erstschätzung (Wert, Datum, Quelle,
  Zahl der Schätzer) · Revisionen · Endwert · Umsetzungssprint ·
  Anlass-Kennzeichen" — **weder Abstand noch Faktor**. §24.3 führt beide
  Spalten, deshalb funktioniert es heute. Liefert der Scrum Master ab
  Sprint 6 nur, was der Prüfsatz verlangt: (a) ohne gedruckten Faktor muss
  der Verwalter `faktor_protokoll` selbst rechnen — dann prüft der
  Transkriptionsschutz (`kegel.py:106-125`) den Verwalter gegen sich selbst
  und ist leer; (b) ohne Abstand bricht der Generator bei Kurvenpunkten ab
  (`kegel.py:131-135`), oder der Verwalter muss aus dem Erstschätzungsdatum
  den Sprint ableiten — ein Urteil, das ihm verboten ist.
- **Beobachtung — „Quelle: Sprint-Protokoll §24" ist fest codiert**
  (`kegel.py:315`) und steht so in jedem künftigen Bild; `LIESMICH.md:19-20`
  verspricht nur „die entsprechende Tabelle im jeweiligen Sprint-Protokoll".
  Trägt nur, wenn §24 zur festen Konvention wird — sonst behauptet das Bild
  ab Sprint 6 eine falsche Fundstelle.
- **Beobachtung — `kegel.py:233-241`:** „ruht auf einem einzigen Punkt" wird
  aus der Spaltengröße (≤ 2) gefolgert, nicht aus der Zahl der Punkte am
  Maximum. Heute korrekt (#11 trägt den Rand allein bei zwei Punkten in der
  Spalte); kommt bei Abstand 3 ein dritter Punkt hinzu, druckt die Caption
  „ruht auf 3 Punkten", obwohl der Rand weiter allein auf #11 ruht — ein
  gerechneter Satz, der falsch würde, in einem Werkzeug, dessen Anspruch
  „gerechnet, nicht getippt" ist.

---

## Konkrete Fix-Vorschläge

1. **(zum Think-fail)** `README.md:14-15`: den Nebensatz „— je weiter eine
   Schätzung von der Umsetzung entfernt lag, desto mehr" streichen oder auf
   das Gemessene stellen, z. B. „— in dieser Reihe tritt die Weitung erst bei
   Abstand 3 ein und ruht dort auf einem einzigen Punkt". Maßstab ist der
   eigene Satz `sprint-05.md:1215-1216`.
2. **(zu warn 4b)** `PROZESS.md:210-213`: den Spaltensatz um „Abstand" und
   „Faktor (wie gedruckt)" ergänzen — sonst läuft der Transkriptionsschutz ab
   Sprint 6 leer bzw. der Generator bricht ab.
3. **(zu warn 4a)** Träger der `stand`-Fortschreibung benennen — ein Halbsatz
   in `PROZESS.md` Punkt 12 oder `denkzettel-verwalter.md` („das Feld `stand`
   ziehst du auf den abgeschlossenen Sprint" — mechanisch, kein Urteil) und/
   oder den DoD-Prüfsatz um „Standangabe = geprüfter Sprint" erweitern.
4. **(zu warn 2)** `schaetzhistorie.json:5` (`_fortschreibung`) auf einen
   Verweis auf LIESMICH/PROZESS kürzen — eine Pflegestelle weniger.
5. **(optional, Beobachtungen 4)** `kegel.py:233-241`: `rest` aus der Zahl
   der Punkte am Maximum berechnen statt aus der Spaltengröße;
   `kegel.py:315`: „§24" aus der JSON beziehen oder die §24-Nummer als
   Konvention in `PROZESS.md` festschreiben.

## Was gut ist

- **Der Determinismus ist kein Versprechen, sondern messbar** — bytegleiche
  Läufe, committetes Bild identisch mit Neuerzeugung, sauberer Baum. Der
  „Diff als Prüfkriterium" trägt.
- **Die Caption ist gerechnet, nicht getippt** (`kegel.py:200-262`), und das
  Protokoll hat die eigene Monotonie-Überbehauptung selbst gefunden und
  gestrichen (`7699791`) — die Selbstkorrektur-Disziplin ist da; sie muss nur
  noch bis ins README reichen.
- **§24.4 versteckt den gefundenen Widerspruch nicht**, sondern entscheidet
  ihn begründet und benennt den Weg für die Gegenposition.
- **Punkt 12 wurde mit belegter Begründung angehängt statt eingeschoben** —
  die zitierten Nummern in vier älteren Dokumentsträngen bleiben gültig.
- **Provenienz wird mitgeführt statt geglättet** (Schätzerzahl, Quelle je
  Zeile), und die 13 Auslassungen stehen sichtbar in Datenreihe und Bild —
  das Diagramm untertreibt lieber, als zu behaupten.

*Reviewer: karpathy-reviewer (frischer Kontext, Cherny-Pattern) · 02.08.2026 ·
geprüft am Stand `ca8e75c`*
