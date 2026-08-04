# Prüfung: Ist `Meta+Umschalt+N` lückenlos festgehalten?

**Datum:** 04.08.2026 · **Anlass:** Kundenaussage vom 04.08.2026 — „Außerdem
möchte ich unbedingt Meta+Shift+N als Shortcut für das Öffnen einer Audio-Notiz
festhalten." · **Auftrag:** prüfen, nicht bauen. Geprüft wurde gegen Quelltext,
SPEC und Backlog, nicht gegen die Erinnerung.

**Kurzantwort:** In der SPEC ja — nach einer Ergänzung, die eine belegte Lücke
schließt. **Im Backlog nein:** Die Story, die das Kürzel bauen soll (#21), hat
Akzeptanzkriterien aus der Zeit *vor* dem Befund vom 01.08.2026 und würde in
genau den stillen Fehlschlag laufen, den die SPEC seither beschreibt.

---

## 1. Vollständigkeit in der SPEC

### Beobachtung

| Stelle | Was dort steht | Fundstelle |
|---|---|---|
| 2.3 D-Bus | `ShowRecorder()` — „Aufnahmefenster zeigen, Aufnahme startet sofort" | `SPEC.md:59` |
| 2.4 Kürzel | „`Meta+N` → `ShowCapture()` · `Meta+Umschalt+N` → `ShowRecorder()`" | `SPEC.md:70` |
| 2.4 Belegung | „Belegung auf Ganymed geprüft (31.07.2026, inkl. Mehrfachbelegungen): **beide** frei" | `SPEC.md:73` |
| 2.4 Konflikte | Konflikterkennung „Beim Erststart und bei Kürzel-Änderung" — ohne Einschränkung auf ein Kürzel | `SPEC.md:74–79` |
| 2.4 Desktop-Actions | „**Je Kürzel** deklariert die Desktop-Datei eine Gruppe `[Desktop Action <Aktions-Id>]`" | `SPEC.md:87–89` |
| 4 Aufnahmefenster | beschreibt Inhalt, Technik, Grenzen — **nennt kein Kürzel** | `SPEC.md:147–159` |
| 10 Tray | Zeile „Sprachnotiz aufnehmen · `audio-input-microphone` · inaktiv bis M4 · Kürzel-Hinweis Meta+Umschalt+N ab M4" | `SPEC.md:501` |
| 13 Einstellungen | Seite „**Kürzel** (KKeySequenceWidget für **beide** Shortcuts)" | `SPEC.md:590–591` |
| 17 Meilensteine | M4 = „Aufnahmefenster, Queue, whisper.cpp-Backend, Player in der Bibliothek; WhisperX-Anbindung" — **nennt kein Kürzel** | `SPEC.md:665–666` |

(Zeilennummern der SPEC nach der unter Punkt 6 beschriebenen Ergänzung.)

### Schlussfolgerung

Abschnitte 2.3, 2.4, 10 und 13 decken das Kürzel ab, und zwar mit ausdrücklichem
Bezug auf *beide* Sequenzen (`73`, `87`, `590`). Zwei Stellen nennen es nicht:

- **Abschnitt 4 nennt kein Kürzel — das ist kein Mangel.** Abschnitt 3
  (Capture-Fenster) nennt `Meta+N` ebenfalls nicht; die Zuordnung
  Sequenz → Fenster steht einmal in 2.4. Die SPEC ist hier symmetrisch, nicht
  lückenhaft. Eine Ergänzung in 4 wäre eine Doppelung mit zwei Pflegestellen.
- **Abschnitt 17 nennt es bei M4 nicht.** Die Information geht nicht verloren —
  `SPEC.md:501` sagt „ab M4". Aber M1 listet „KGlobalAccel" ausdrücklich auf,
  und wer den M4-Schnitt vorbereitet, liest Abschnitt 17. **Keine Änderung
  vorgenommen** (kein belegter Informationsverlust); Empfehlung an den PO unter
  Punkt 7.

## 2. Widerspruchsfreiheit der Schreibweise

### Beobachtung

Vollständige Suche über Repository (ohne `.git`, `build`) und über alle 73
Issues:

```
KONZEPT.md:130, 208            Meta+Umschalt+N
SPEC.md:70, 501                Meta+Umschalt+N
docs/scrum/sprints/sprint-02.md:83   Meta+Umschalt+N
wireframes/…dc.html:191, 940   Meta+Umschalt+N
src/shell/globalshortcuts.h:14 Meta+Umschalt+N
Issue #5, Issue #21            Meta+Umschalt+N
```

Kein einziges Vorkommen von „Meta+Shift+N". Der Kunde hat die englische Form
gesagt, das Projekt schreibt durchgehend die deutsche.

### Schlussfolgerung

Einheitlich, kein Befund. **Zur Sorge um die Zeichenkette:** Sie trägt hier
nicht — die Sequenz wird im Code nicht aus Text geparst, sondern aus
Qt-Konstanten gebaut (`globalshortcuts.cpp:79`: `QKeySequence(Qt::META |
Qt::Key_N)`). „Meta+Umschalt+N" ist die *Anzeigeform*, nicht der Schlüssel.
Uneinheitlichkeit wäre damit ein Lesbarkeits-, kein Funktionsproblem — sie liegt
ohnehin nicht vor.

## 3. Der T1-Befund und die Desktop-Action

### Beobachtung

- `SPEC.md:87–89` formuliert die Bedingung ausdrücklich pro Kürzel: „**Je Kürzel**
  deklariert die Desktop-Datei eine Gruppe … und die Id steht in `Actions=`."
- `desktop/org.denkzettel.Denkzettel.desktop` deklariert `Actions=show-capture;`
  und genau eine Gruppe `[Desktop Action show-capture]`. Eine zweite Gruppe für
  den Recorder fehlt.
- **Nicht in der SPEC stand** die Gegenmaßnahme, die im Code existiert: Die
  Registrierung wird beim Dienst *zurückgelesen*.
  `src/shell/shortcutregistration.h:17–34` und `globalshortcuts.cpp:99–111`
  prüfen drei Fehlschläge (`ApplicationNotInstalled`, `DaemonKeptNothing`,
  `DesktopActionMissing`) und melden sie bei jedem Start. Der Grund steht im
  Code: „`setGlobalShortcut()` … sends the D-Bus call and never looks at the
  answer". Die Regel selbst ist Retro-Beschluss **B5**
  (`docs/scrum/sprints/sprint-02.md:1106–1117`), verankert in
  `.claude/agents/denkzettel-dev.md` und `PROZESS.md` (DoD 2) — **nicht** in der
  SPEC (Suche nach „zurückgelesen/zurücklesen/Rückles" in `SPEC.md`: kein
  Treffer).

### Schlussfolgerung

Die *Desktop-Action*-Bedingung deckt beide Kürzel ab — hier keine Lücke. Die
fehlende Gruppe in der Desktop-Datei ist **kein Befund**, sondern der korrekte
Zustand vor M4 (`Meta+Umschalt+N` existiert noch nicht).

**Die Lücke lag eine Ebene tiefer:** Die SPEC beschrieb den stillen Fehlschlag,
aber nicht, woran Denkzettel ihn merkt. Wer die M4-Story gegen die SPEC schneidet,
liest die Warnung und hat kein Prüfmittel danebenstehen — und genau das ist bei
#21 passiert (Punkt 5). Das ist eine entdeckte Bedingung im Sinne von DoD 4/B9:
gebaut, wirksam, aber nicht in der SPEC. Ergänzt unter Punkt 6.

## 4. Umsetzungsstand im Code (nur festgestellt, nichts gebaut)

### Beobachtung

| Erwartet | Stand | Fundstelle |
|---|---|---|
| `ShowRecorder()` im D-Bus-Dienst | **fehlt** — exportiert sind `ShowCapture`, `AddNote`, `ShowLibrary`, `Quit` | `src/shell/daemonservice.h:28–40` |
| zweites Kürzel bei KGlobalAccel | **fehlt** — `GlobalShortcuts` hält genau eine `QAction` (`m_captureAction`) und bietet nur `registerCaptureShortcut()` | `src/shell/globalshortcuts.h:26–40` |
| Tray-Eintrag „Sprachnotiz aufnehmen" | vorhanden als Stummel ohne Kürzel-Hinweis | `src/shell/trayicon.cpp:84` |
| Kommentar zur Absicht | „Meta+Umschalt+N for the recorder follows with M4, which is when `ShowRecorder()` exists." | `src/shell/globalshortcuts.h:14–15` |

### Schlussfolgerung

Der Stand entspricht dem bezahlten Umfang: #5 hat ausdrücklich nur `Meta+N`
gebaut. Kein Befund am Code — **aber zwei Befunde für die kommende Story**, weil
die vorhandenen Bauteile auf genau ein Kürzel zugeschnitten sind:

- **B-1 — Die Meldungstexte verdrahten „Meta+N" fest.** Alle drei
  Fehlschlag-Meldungen (`shortcutregistration.cpp:46, 54, 60`) und die
  Konfliktmeldung (`globalshortcuts.cpp:126`) beginnen wörtlich mit „Meta+N".
  Wird die Registrierung des Recorders über denselben Weg geführt, meldet
  Denkzettel einen Fehlschlag von `Meta+Umschalt+N` unter dem Namen des anderen
  Kürzels — der Nutzer prüft dann das Kürzel, das funktioniert.
- **B-2 — `GlobalShortcuts` kennt keine Mehrzahl.** Ein Feld, eine Aktion, eine
  Methode. Die M4-Story trägt die Verallgemeinerung, nicht nur eine zweite
  Zeile. Das gehört in ihre Schätzung.

Beides ist gemeldet, nicht geheilt.

## 5. Backlog

### Beobachtung

- **#21 (S13b: Aufnahmefenster mit Pegel und Timer, 3 SP, M4)** trägt das Kürzel:
  Scope „…, Kürzel Meta+Umschalt+N, Tray-Eintrag"; AK 2: „Pegel reagiert sichtbar
  auf Mikrofoneingang; **Kürzel registriert und in Systemeinstellungen sichtbar**".
- **#5 (S4, geschlossen)** hat die Übergabe protokolliert: „Meta+Umschalt+N /
  `ShowRecorder()` erst mit M4/S13b — SPEC nennt beide, die Story bezahlt nur
  eines."
- **#21 nennt `ShowRecorder()` nicht.** Die von #5 benannte Empfängerin der
  D-Bus-Methode nimmt sie in ihrem Text nicht auf. Auch #20 (S13a,
  Aufnahme-Pipeline) nennt sie nicht.
- **Keine Story für die Einstellungsseite „Kürzel"** (`SPEC.md:590–591`,
  KKeySequenceWidget für beide Shortcuts). #16 (S11) deckt nur die Seiten
  KI-Provider und Analyse, #27 (S15) nur Sprachnotizen. Suche über alle 73
  Issues nach „Kürzel/Kurzbefehl/KKeySequence/Einstellungen-Seite": kein Treffer,
  der diese Seite trägt.
- *Nebenbefund außerhalb meiner Fläche:* Der Einstellungsseite **Export**
  (`SPEC.md:588–589`) fehlt ebenfalls eine Story. Gemeldet, nicht angefasst.

### Schlussfolgerung

Das Kürzel ist im Backlog **vorhanden, aber nicht abgesichert**:

- **B-3 — Die Akzeptanzkriterien von #21 sind ein Testaufbau, in dem der Fehler
  nicht auftreten kann.** „Kürzel registriert und in Systemeinstellungen
  sichtbar" ist wörtlich der Zustand, den `SPEC.md:85–87` als *Fehlschlag*
  beschreibt: „die Registrierung liegt vor, `isActive` ist wahr, und der
  Tastendruck verpufft." #21 wurde am 31.07.2026 geschrieben, der Befund fiel am
  01.08.2026 — die AK können ihn nicht kennen. Sie müssen vor dem M4-Planning
  nachgezogen werden: Rücklesen, eigene Desktop-Action, echter Tastendruck in der
  Sitzung.
- **B-4 — `ShowRecorder()` hat keine Heimat.** #5 hat die Methode an S13b
  überwiesen, S13b hat sie nicht angenommen. Ohne Nachtrag baut niemand sie.
- **B-5 — Der Weg, das Kürzel zu ändern, hat keine Story.** Die SPEC verspricht
  eine Einstellungsseite für beide Kürzel; im Backlog steht sie nirgends. Die
  Konflikterkennung „bei Kürzel-Änderung" (`SPEC.md:77`) hat damit ebenfalls
  keinen Auslöser.

## 6. Was ich geändert habe

**Eine Änderung, in `SPEC.md`, Abschnitt 2.4** — neuer Aufzählungspunkt
„Rücklesen der Registrierung" nach dem Desktop-Actions-Punkt (`SPEC.md:94–107`).

*Begründung nach Fundstelle:* Die SPEC beschrieb den stillen Fehlschlag
(`SPEC.md:74–79`, `85–87`), nannte aber das gebaute Gegenmittel nicht. Das
Gegenmittel existiert (`src/shell/shortcutregistration.h:17–34`,
`globalshortcuts.cpp:99–111`), beruht auf einer beim Bauen entdeckten Bedingung
(Befund 01.08.2026, Retro-Beschluss B5,
`docs/scrum/sprints/sprint-02.md:1106–1117`) und stand bislang nur in
`PROZESS.md` (DoD 2) und `.claude/agents/denkzettel-dev.md` — also an Orten, die
beim Schneiden einer Story gegen die SPEC niemand liest. Genau diese Lücke hat
sich in den AK von #21 niedergeschlagen (B-3). DoD 4 in der Fassung nach B9:
entdeckte Bedingungen ziehen die SPEC nach.

Der Punkt schließt mit „**Das gilt je Kürzel:** `Meta+Umschalt+N` durchläuft
dieselbe Prüfung wie `Meta+N`" — das ist die Stelle, an der die Festlegung des
Kunden lückenlos wird.

**Ein Satz darin geht über das Gebaute hinaus und ist als solcher zu prüfen:**
„Die Meldung nennt das betroffene Kürzel und einen ausführbaren Schritt." Der
zweite Halbsatz beschreibt den Ist-Zustand (UI-Review B9, alle drei Meldungen
enden mit einem ausführbaren Schritt); der erste ist heute *nicht* erfüllt, weil
die Texte „Meta+N" fest verdrahten (B-1). Ich habe ihn aufgenommen, weil eine
Meldung, die das falsche Kürzel nennt, kein Entwurf, sondern ein Fehler wäre.
**Will der PO das nicht als Zusicherung, ist der Halbsatz zu streichen** — die
Aussage zu B-1 bleibt davon unberührt.

Nichts anderes geändert. Kein Commit, kein Push.

## 7. Was der PO entscheiden muss

1. **Nachtrag an #21 vor dem M4-Planning?** Vorschlag: AK ersetzen durch
   Rücklesen (`SPEC.md:94–107`), zweite Desktop-Action, `ShowRecorder()`,
   Verallgemeinerung der Meldungstexte (B-1, B-2) — und die 3 SP neu schätzen,
   weil B-2 mehr ist als eine zweite Zeile.
2. **Wohin mit `ShowRecorder()`?** #21 oder #20 — beide nennen sie heute nicht
   (B-4).
3. **Story für die Einstellungsseite „Kürzel"** anlegen (B-5)? Ohne sie ist die
   SPEC-Zusicherung `SPEC.md:590–591` unbesetzt.
4. **`Meta+Umschalt+N` in Abschnitt 17 bei M4 aufnehmen?** Nicht von mir geändert
   (Punkt 1); der PO verantwortet die Meilensteinliste.

## 8. Fragen an den Kunden

**Nur eine, und sie steht dem PO nicht zu:**

> **Was soll `Meta+Umschalt+N` tun, solange M4 nicht gebaut ist?** Heute ist es
> unregistriert — der Tastendruck geht ins Leere, der Tray-Eintrag ist grau, und
> der Kürzel-Hinweis erscheint erst „ab M4" (`SPEC.md:501`). Denkbar wäre auch:
> das Kürzel schon jetzt registrieren und mit einer Meldung beantworten
> („Sprachnotizen kommen später"). Dagegen spricht die Regel aus `SPEC.md:510–514`
> (kein dauerhaft ausgegrauter Eintrag, KDE HIG) und dass ein belegtes, aber
> wirkungsloses Kürzel schlechter ist als ein freies. **Empfehlung: beim heutigen
> Zustand bleiben.** Der Kunde entscheidet.

Zwei Punkte brauchen **keine** Kundenentscheidung und sind hier nur zur
Klarstellung genannt: Die Sequenz selbst ist entschieden (`SPEC.md:70`, beide auf
Ganymed frei geprüft), und die Schreibweise ist projektweit einheitlich
(Punkt 2).
