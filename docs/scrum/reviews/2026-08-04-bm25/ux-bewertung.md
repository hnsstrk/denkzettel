# BM25 in der Trefferliste — UX-Bewertung des Zielkonflikts Relevanz gegen Tagesgruppen

**Modus:** Planning-Beratung (Bewertung einer Produktfrage vor dem Schnitt).
**Kein UI-Review** — es liegt keine umgesetzte Story vor, also greift DoD 3
hier nicht. Ich habe auftragsgemäß **kein Bild erzeugt**, keinen Bau- und
keinen Messlauf gefahren; wo ich ein Bild oder eine Messung für nötig halte,
steht sie unter Abschnitt 7 als **Vorschlag**.

**Datum:** 04.08.2026 · **Prüfer:** `denkzettel-ux` · **Zweig:** `main`
(`80b52ae`).

**Geprüfte Artefakte**

| Artefakt | Stellen |
|---|---|
| `SPEC.md` | Abschnitt 6 (Suche), Abschnitt 9 (Bibliothek), Abschnitt 11 |
| `wireframes/Denkzettel Wireframes.dc.html` | 2b, 2c (Festlegungsblock), **3a**, **3b** |
| `docs/scrum/reviews/sprint-03-s6-trefferliste.md` | Trefferliste und Leerzustand, 01.08.2026 |
| `docs/scrum/reviews/sprint-03-s6-trefferliste/` | Bilder E0–E7, `messung.txt` (bestehende, versionierte Belege) |
| `src/ui/timestampformat.h`, `src/ui/notelistmodel.h`, `src/store/store.cpp` | nur gelesen, um Zusicherungen zu belegen |

**Änderungen in diesem Lauf:** ausschließlich diese Datei. Keine Zeichnung,
kein Code, keine SPEC, kein Commit.

---

## Kurzurteil

**Die Tagesgliederung ist hier mehr wert als eine Relevanzsortierung.** Ich
empfehle, die Zusicherung aus `SPEC.md:261` unangetastet zu lassen.

Das ist keine Bequemlichkeitsentscheidung: Die Gliederung trägt in der
Trefferliste **Information, die sonst nirgends steht** (Befund B1), und der
Nutzen einer Rangfolge ist bei ein bis zwei Zeilen langen Notizen strukturell
klein (B5, B6). Der billigste Weg ist hier zugleich der bessere.

**Es gibt einen dritten Weg** — aber nicht in der Sortierung, sondern in der
**Lesbarkeit des Treffers** (Abschnitt 5, Weg E). Er kostet die Ordnung nichts
und behebt das Problem, das der Kunde in einer langen Trefferliste tatsächlich
hat.

---

## 1. Beobachtungen (getrennt von der Schlussfolgerung)

### B1 — Der Eintrag kennt seinen Tag nicht; der Gruppenkopf trägt ihn

`src/ui/timestampformat.h:38-48` sagt es wörtlich: Der Zeitstempel eines
Eintrags „says what its group leaves open and nothing twice" — unter „Heute"
und „Gestern" steht **nur die Uhrzeit** („14:32"), weil der Kopf den Tag trägt.
Festgelegt ist das in Zeichnung **3b**, Festlegungsblock, Absatz *„Zeitstempel
je Eintrag, abgeleitet aus der Gruppe"*, und in `SPEC.md:405-409`.

Beleg am vorhandenen Bild
`sprint-03-s6-trefferliste/s6-e2-ein-treffer-je-gruppe-900x600.png`
(Suchbegriff „vortrag", fünf Treffer, fünf Köpfe): Die beiden obersten Treffer
tragen „11:05" und „09:15" und sonst nichts.

Der frühere UI-Review hat denselben Punkt schon einmal notiert
(`sprint-03-s6-trefferliste.md:54`): *„ohne den Kopf stünde bei einem Treffer
von heute nirgends, dass er von heute ist."*

**Beobachtung, nicht Meinung:** Fallen die Köpfe, fehlt bei jedem Treffer aus
„Heute" und „Gestern" die Tagesangabe — es sei denn, `entryTimestamp()` wird
für den Suchfall umgebaut. Dieser Umbau ist in keiner BM25-Betrachtung
enthalten, die ich gesehen habe.

### B2 — In einer Rangliste stünden vier Zeitstempelformate ohne Ordnung nebeneinander

Heute läuft das Format von oben nach unten monoton mit der Zeit: Uhrzeit →
Uhrzeit → Wochentag+Datum → Wochentag+Datum → absolutes Datum
(`messung.txt`, Fall E2). In Rangordnung stünden dieselben fünf Formate in
beliebiger Folge untereinander. Die Formatwahl wäre dann kein Signal mehr,
sondern Rauschen.

### B3 — Die Suche läuft bei jedem Tastendruck

Zeichnung **2c**, Festlegungsblock, Absatz *„Deaktiviertes Suchfeld"*,
Überholt-Vermerk vom 01.08.2026: *„Es sucht bei jedem Tastendruck."*

Bei chronologischer Ordnung ist die Wirkung jedes Tastendrucks **monoton**: Die
Liste wird kürzer, die verbliebenen Einträge behalten ihre relative Lage. Bei
Rangordnung ändert jeder Tastendruck **auch die Reihenfolge** — Einträge wandern
nach oben und unten, während der Nutzer noch tippt.

### B4 — Die Trefferliste hat keine Obergrenze

`src/store/store.cpp:448` baut die Abfrage mit
`ORDER BY created_at DESC, id DESC` und **ohne `LIMIT`**. Weder SPEC 6 noch die
Zeichnungen nennen eine Obergrenze der Trefferliste. Es gibt also keine
festgelegte Zahl, ab der „zu viele Treffer" beginnt.

Die einzige Bestandszahl, die die SPEC führt, ist der Volllauf-Schutz
(`SPEC.md:568`): **200** unexportierte Notizen als Vorgabe, dann mahnt die App.
Das ist keine Trefferzahl, aber es beschreibt die Größenordnung, in der dieses
Werkzeug arbeiten soll.

### B5 — Am Bild ablesbar: rund sieben Einträge sind gleichzeitig sichtbar

Gelesen am Bild `s6-e2-ein-treffer-je-gruppe-900x600.png` (Fenster 900×600,
Listenspalte 300 px): Fünf Köpfe und fünf Einträge füllen die Spalte bis zur
Unterkante; ein Block aus Kopf und Eintrag misst dort rund 107 px, ein Eintrag
allein rund 78 px. Ohne dazwischenstehende Köpfe passen in dieselbe Spalte also
etwa **sieben** Einträge.

*Diese Zahl ist am Bild abgelesen, nicht von einem Läufer gemessen* — sie ist
für die Größenordnung belastbar, nicht auf den Eintrag genau. Vorschlag zur
exakten Messung: Abschnitt 7, M1.

### B6 — Die Notizen sind kurz und untereinander etwa gleich lang

`SPEC.md:10-16` („ein Gedanke wird getippt oder gesprochen"), Zeichnung **3b**,
Fall 2 und Fall 3: der Regelfall ist *„ein Gedanke, kein Zeilenumbruch"*, der
Einzeiler ist ausdrücklich vorgesehen. Der Listeneintrag zeigt genau zwei
Textzeilen, und die Zeichnung nennt das den Regelfall, nicht die Kürzung eines
längeren Textes.

### B7 — Die Zeichnung hat einen Sortier-Umschalter bereits einmal abgelehnt

Zeichnung **3b**, Festlegungsblock, Absatz *„Bewusst weggelassen: der
Umschalter ‚In Gruppen anzeigen'"*: *„Denkzettel hat genau eine Sortierung
(chronologisch, neueste zuerst, SPEC 9). Die Gliederung ist damit nur die
lesbare Form dieser einen Ordnung … ein Schalter dafür wäre eine Entscheidung,
die der Nutzer treffen muss, ohne etwas davon zu haben (KDE HIG: Einstellungen
kosten Aufmerksamkeit)."*

Der Absatz spricht vom Gruppen-Umschalter, seine Begründung hängt aber an der
**einen Sortierung** — sie trägt für einen Sortier-Umschalter genauso.

### B8 — Zwei geltende Zusicherungen hängen an der chronologischen Ordnung

- `SPEC.md:435-438` und `src/ui/notelistmodel.h:71-79`: Eine bearbeitete Notiz
  **bleibt in der laufenden Trefferliste stehen**, an ihrer Zeile und in ihrer
  Gruppe. Begründung im Quelltext: *„Editing leaves `created_at` alone, so no
  note can move to another group."* Diese Zusicherung gilt **nur, weil nach
  `created_at` sortiert wird**. Unter BM25 ändert das Bearbeiten den Text und
  damit den Rang; die Notiz behielte dann eine Position, die ihr neuer Text
  nicht mehr rechtfertigt — oder sie spränge, und dann bricht die Zusicherung.
- `SPEC.md:411-416` (Zeichnung **3b**, Fall 4): Springt die Auswahl per Taste
  über eine Gruppengrenze, holt die Liste den **Kopf der neuen Gruppe** ins
  Bild; ein Mausklick tut das nicht (gemessen 387 px, Issue #57). Ohne Gruppen
  ist diese in Sprint 5/6 gebaute und abgenommene Mechanik gegenstandslos.

### B9 — Es gibt keine eigene Zeichnung der Trefferliste

Die Regeln der Trefferliste stehen als Fließtext im Festlegungsblock von **2c**
(Überholt-Vermerk 01.08.2026) und stützen sich auf **3a**. Der damalige Review
hat die fehlende Zeichnung ausdrücklich für vertretbar gehalten
(`sprint-03-s6-trefferliste.md:36-38`) — für den *jetzigen* Fall ist das eine
Lücke: Eine Ordnungsänderung ließe sich an keiner Zeichnung prüfen, weil keine
Zeichnung die Trefferliste zeigt. **Verdikt: warn** (siehe Abschnitt 8).

---

## 2. Frage 1 — Was verliert der Nutzer ohne Tagesgruppen?

**V1 — Die Tagesangabe der neuesten Treffer.** Aus B1: Treffer aus „Heute" und
„Gestern" trügen nur noch eine Uhrzeit. Das ist kein Schönheitsverlust, sondern
fehlende Information: „14:32" ohne Tag ist in einer nach Rang geordneten Liste
nicht auflösbar. Wer das heilen will, muss `entryTimestamp()` für den Suchfall
umbauen — dann tragen alle Treffer die volle Form, und die Liste wird an ihrer
schmalsten Stelle (220 px, Zeichnung 3a) breiter beschriftet als heute.
**Verdikt: fail** für eine reine Umstellung auf Rang ohne diesen Umbau.

**V2 — Die Zeitachse als Abbruchkriterium.** Denkzettel ist ein Werkzeug für
flüchtige Notizen; „wann war das noch" ist die naheliegende Suchhaltung, und
`SPEC.md:399-404` gibt ihr mit der Posteingangsform die passende Gestalt. In
einer chronologischen Trefferliste weiß der Nutzer, **wann er aufhören darf zu
lesen** — sobald er an dem Zeitraum vorbei ist, den er meint. Eine Rangliste
hat kein solches Kriterium: Sie muss bis zum Ende gelesen werden, weil der
gesuchte Eintrag überall stehen kann.

Ich halte diesen Punkt für den sachlich stärksten, und er ist zugleich der, den
die Zeichnungen nur mittelbar hergeben. Was sie hergeben: Die Zeichnungen
kennen **keine andere Ordnung als die zeitliche** — die fünf Gruppen, die
Zeitstempelregel, der Scroll-Vertrag aus 3b Fall 4 und der Detailzeitstempel
sind sämtlich Zeitaussagen. Es gibt in der ganzen Bibliothek kein einziges
gezeichnetes Element, das eine Rangaussage macht.

**V3 — Ruhe beim Tippen.** Aus B3: Heute schrumpft die Liste beim Tippen
monoton; unter Rangordnung ordnet sie sich bei jedem Zeichen neu. Ein Eintrag,
den der Nutzer schon ins Auge gefasst hat, wandert unter dem Zeiger weg. Genau
dieses Argument hat das Team bei Issue #57 schon einmal zugunsten der Ruhe
entschieden (`SPEC.md:412-414`: *„Wer zeigt, erwartet, dass die gezeigte Stelle
bleibt"*). **Verdikt: warn** — nicht tödlich, aber eine Regression gegen eine
bereits getroffene Entscheidung derselben Art.

**V4 — Zwei Ordnungsgesetze in einem Fenster.** Die Bibliothek wäre
chronologisch, die Trefferliste nach Rang, und der Wechsel geschähe beim
Tippen des ersten Zeichens, ohne Ankündigung. Der Nutzer bekäme kein Signal,
dass sich das Gesetz der Liste geändert hat. Das ist ein unsichtbarer Modus.

**V5 — Verworfene Arbeit.** Aus B8: Der Kopf-ins-Bild-Vertrag (Issue #57,
gemessen und abgenommen) und die Bearbeiten-Zusicherung (Issue #11, K2) hängen
beide an der zeitlichen Ordnung. Das ist kein Argument gegen eine sachlich
richtige Änderung, gehört aber ehrlich in die Kostenrechnung.

**Was der Nutzer *nicht* verliert:** Die Vollständigkeitsaussage. Ein Kopf
behauptet nicht, seine Gruppe sei vollständig — er sagt nur, wann das darunter
entstand (`sprint-03-s6-trefferliste.md:47`). Wer gegen die Gruppen
argumentiert, kann sich auf diesen Punkt also nicht berufen.

---

## 3. Frage 2 — Was gewinnt er? Und ab welcher Trefferzahl?

**Ab wann Relevanz überhaupt helfen kann.** Eine Rangfolge hilft erst, wenn der
Nutzer die Treffer **nicht mehr alle ansehen kann oder will**. Die praktische
Schwelle ist der sichtbare Ausschnitt: rund **sieben Einträge** bei 900×600
(B5). Bis dahin liest man die Liste, statt sie zu sortieren — bei sieben
zweizeiligen Einträgen ist die Rangfolge Aufwand ohne Wirkung.

**Eine Obergrenze der Trefferliste gibt es nicht** (B4) — weder in der SPEC
noch im Code. Es gibt also auch keinen Zustand „zu viele Treffer", auf den sich
ein Relevanzargument stützen könnte. Wer BM25 mit der Länge der Trefferliste
begründet, begründet damit zuerst eine Obergrenze oder einen Trefferzähler,
nicht eine Sortierung.

**Warum BM25 bei diesen Notizen wenig zu ordnen hat.** BM25 gewinnt seine
Trennschärfe aus zwei Größen: der **Termhäufigkeit im Dokument** und der
**Dokumentlänge** relativ zur mittleren Länge. Beide sind hier fast konstant:

- Nach B6 ist die Regel-Notiz ein bis drei Zeilen lang. Die Längennormierung —
  der eigentliche Vorzug von BM25 gegenüber einfachem tf·idf — hat damit kaum
  Streuung, an der sie arbeiten könnte.
- In einer zweizeiligen Notiz steht ein Suchbegriff üblicherweise **einmal**.
  Bei gleicher Termhäufigkeit und ähnlicher Länge liegen die Punktwerte dicht
  beieinander.

Die Folge ist die eigentliche UX-Gefahr: **Nahezu gleichrangige Treffer in
Rangordnung wirken willkürlich sortiert.** Der Nutzer sieht eine Reihenfolge,
kann sie sich nicht erklären, und die Ordnung, die er sich erklären könnte —
die zeitliche —, ist weg. Eine Sortierung, deren Grund unsichtbar ist, ist
schlechter als gar keine erkennbare Sortierung, weil sie zum Suchen nach einem
Muster verleitet, das es nicht gibt.

**Offene technische Frage an die beiden anderen Agenten** (nicht meine
Fläche, aber sie entscheidet über die Größe dieses Effekts): Der Index ist
`trigram remove_diacritics 1` (`SPEC.md:225`). BM25 zählt Tokens — hier
Drei-Zeichen-Gruppen, nicht Wörter. Ob und wie sinnvoll ein bm25-Wert über
Trigrammen für einen Nutzer erklärbar ist, sollte geklärt sein, **bevor** über
die Anzeigeordnung entschieden wird. Nach meinem Verständnis würde ein langer
Suchbegriff dabei viele Trigramme beitragen und ein kurzer wenige — die
Rangfolge hinge dann auch an der Länge des Suchworts. Bitte technisch prüfen;
falls es zutrifft, ist der Punkt allein schon erledigt.

**Der eine reale Gewinn:** Notizen, in denen der Begriff mehrfach vorkommt oder
im Betreff statt im Vorschautext steht, kämen nach oben. Das ist ein echter,
aber schmaler Gewinn — und er tritt genau dann ein, wenn die Trefferliste lang
ist, also im Fall, den B4 heute gar nicht kennt.

---

## 4. Frage 3 — Dritte Wege

### Weg A — Relevanz nur bei Volltextsuche, Tagesgruppen sonst · **fail**

Klingt sparsam, ist aber der schlechteste der Vorschläge, und zwar aus einem
Grund, der erst mit S7 voll sichtbar wird: SPEC 6 erlaubt **gemischte
Anfragen**. `tag:backup foto` ist Filter *und* Volltext. Nach dieser Regel
kippte die Ordnung der Liste in dem Moment, in dem der Nutzer hinter einen
Filter ein freies Wort setzt — dieselbe Liste, dieselbe Ansicht, ein anderes
Ordnungsgesetz, ausgelöst durch ein Zeichen im Suchfeld.

Das ist ein **unsichtbarer Modus**, den der Nutzer weder sieht noch abschalten
kann und dessen Regel er nur aus der SPEC lernen könnte. Die KDE HIG verlangen
für Zustandswechsel Sichtbarkeit und Vorhersagbarkeit; beides fehlt hier. Dazu
kommt V1: In den Volltextfällen fehlte die Tagesangabe, in den Filterfällen
wäre sie da — dieselbe Notiz sähe je nach Anfrage anders aus.

### Weg B — Umschalter „nach Relevanz / nach Datum" · **warn, nicht empfohlen**

Ehrlicher als A, weil sichtbar, und es ist das, was vergleichbare Werkzeuge tun
(Abschnitt 5). Aber:

- Er widerspricht der bereits begründeten Ablehnung aus B7 in ihrem Kern
  („genau eine Sortierung"), und diese Ablehnung ist eine Kundenentscheidung im
  Umfeld vom 01.08.2026, keine Verlegenheit.
- Er kostet Platz an der einzigen Stelle, an der die Zeichnungen ihn nicht
  haben: Die Kopfzeile ist nach Zeichnung **2c**, Absatz *„Raumaufteilung"*,
  ein Suchfeld und 8 px Rand, rund 48 px hoch, und sie **wächst nicht mit dem
  Fenster**. Ein Sortier-Bedienelement müsste dort hinein oder eine zweite
  Zeile eröffnen — beides ändert eine geprüfte Geometrie-Zusicherung (B2 der
  Sprint-2-Retro).
- Er bringt einen Zustand mit, der über Sitzungen aufbewahrt werden will, und
  damit die Frage, in welchem Zustand die App startet.
- Der Nutzen, den er verteilt, ist nach Abschnitt 3 klein.

**Bedienlast größer als Nutzen.** Falls der Kunde ihn trotzdem will, ist die
sparsamste Bauform ein Menüeintrag oder ein Werkzeugknopf mit Menü **rechts im
Suchfeld**, kein zweites Bedienband — und die Vorgabe bleibt „nach Datum".

### Weg C — Zwei Ansichten (Bibliothek zeitlich, Suchergebnis eigene Ansicht) · **fail**

Das ist Weg A mit mehr Bau: eine zweite Ansicht, ein Übergang, ein Rückweg, ein
eigener Leerzustand — für eine Liste derselben Notizen. Die Suche ist in diesem
Werkzeug ein **Filter auf der Bibliothek**, keine eigene Ansicht; der
Löschknopf im Feld führt in einem Griff zurück (Zeichnung 2c,
Überholt-Vermerk). Eine zweite Ansicht wäre der Punkt, an dem dieses Werkzeug
aufhört, einfach zu sein.

### Weg D — Gruppe „Beste Treffer" über den Tagesgruppen · **warn, nicht empfohlen**

Der einzige Hybrid, der in sich stimmig wäre: Die fünf Tagesgruppen bleiben,
darüber steht — nur bei vielen Treffern — eine sechste Gruppe mit den zwei bis
drei bestbewerteten Treffern. So machen es Suchoberflächen, die „Top-Treffer"
zeigen.

Ich empfehle ihn trotzdem nicht, und zwar wegen dreier Folgen, die alle in
Zeichnung **3b** an geltende Festlegungen stoßen:

1. Es wäre die **erste Gruppe, die keine Zeitgruppe ist**. Die Gruppenordnung
   ist heute fest und vollständig durch die Zeit erklärt („die erste passende
   gewinnt"); eine Ranggruppe bräuchte eine zweite, andersartige Regel.
2. Ein Treffer stünde **zweimal in der Liste** — oben und in seiner Tagesgruppe
   — oder er fehlte in seiner Tagesgruppe. Beides muss der Nutzer verstehen,
   und beides bringt eine Tastaturfrage mit (besucht die Pfeiltaste ihn
   zweimal?). Zeichnung 3b legt fest, dass Köpfe übersprungen werden; über
   Doppelvorkommen sagt sie nichts, weil es sie nicht gibt.
3. Die Gruppe erschiene **abhängig von der Trefferzahl**. Dieselbe Anfrage mit
   einer Notiz mehr sähe strukturell anders aus — wieder ein Zustandswechsel
   ohne sichtbaren Auslöser.

### Weg E — Die Ordnung bleibt, der Treffer wird lesbar · **empfohlen**

Der Vorschlag, den ich tatsächlich mache. Er greift die Ordnung **nicht an**
und behebt das Problem, das eine lange Trefferliste dem Nutzer wirklich macht.

Der frühere Review hat es vorhergesagt
(`sprint-03-s6-trefferliste.md:112-117`, Hinweis 2, ausdrücklich ohne Auflage):
Die Trefferliste **hebt den gefundenen Begriff nicht hervor**, und die
Fundstelle liegt oft im elidierten Teil. Wer heute zwölf Treffer sieht, fragt
nicht „welcher ist der beste", sondern „warum ist das ein Treffer". Genau diese
Frage beantwortet eine Rangfolge nicht.

Zwei Ausbaustufen, die kleinere zuerst:

- **E1 — Fundstelle im vorhandenen Text hervorheben.** Betreff und Vorschau
  bleiben, wie 3b sie festlegt; der gefundene Teilstring wird ausgezeichnet.
  Rührt an keiner Zeichnung, an keiner Zusicherung und an keiner Ordnung.
  Offene Frage: die Auszeichnungsform — nach KDE-Art eher eine Hinterlegung in
  der Akzentfarbe als Fettdruck, denn 3b verbietet Fettdruck im Eintrag
  ausdrücklich („fett heißt in Listen ‚ungelesen'"), und in der ausgewählten
  Zeile muss die Auszeichnung gegen `Highlight` noch lesbar sein.
- **E2 — Vorschauzeile auf die Fundstelle setzen**, wenn der Begriff weder in
  Betreff noch im sichtbaren Vorschautext steht (FTS5 `snippet()`-Machart).
  **Achtung:** Das ändert die Vorschau-Festlegung aus 3b („der unmittelbar
  folgende Text") und zöge DoD 4 nach sich. Eigene Story, nicht nebenbei.

E1 ist nach meiner Einschätzung die beste Wirkung je Aufwand in dieser ganzen
Frage. Ob und wann sie gebaut wird, entscheidet der PO — ich schätze und
priorisiere nicht.

---

## 5. Frage 4 — Was vergleichbare Werkzeuge tun

**Belegstand dieses Abschnitts, ausdrücklich:** In diesem Lauf stand mir **kein
Webzugriff** zur Verfügung, und ich habe auftragsgemäß nichts installiert. Die
folgenden Angaben stammen aus Kenntnis der Oberflächen, nicht aus einer heute
geführten Prüfung. Ich nenne zu jeder Aussage die Stelle, an der sie **in einer
Minute nachprüfbar** ist — bitte vor einer Entscheidung, die sich darauf
stützt, nachsehen (oder `Recherche-BM25` fragen, falls dort Webzugriff
besteht).

| Werkzeug | Ordnung der Trefferliste | Nachprüfbar an |
|---|---|---|
| Outlook (Desktop und Web) | Vorgabe **Relevanz**, Umschalter auf „Neueste"; der Posteingang selbst bleibt nach Datum gegliedert | Kopf der Ergebnisliste, „Sortieren nach" |
| Notion | Vorgabe **Relevanz** („Best matches"), Umschalter „Last edited" | Suchdialog, Sortierauswahl |
| Joplin | Sortierung der Notizliste einstellbar, **Relevanz als eigene Option** im Suchfall | Einstellungen → Notizen, „Notizen sortieren nach" |
| Apple Mail | Vorgabe **Datum** | Ergebnisliste |
| Google Keep, Apple Notes (Schnellnotizen) | Vorgabe **Datum**, keine sichtbare Rangordnung | Suchergebnis |

**Das Muster, und es spricht nicht für BM25 hier:** Relevanz als Vorgabe findet
sich dort, wo die Dokumente **lang und ungleich lang** sind und der Bestand
groß ist (Postfach, Wissensarbeitsplatz) — also dort, wo BM25 tatsächlich etwas
zu normieren hat. Bei kurzen Schnellnotizen bleibt es bei der Zeit.

Und der zweite, ehrlichere Befund: Die Werkzeuge, die Relevanz anbieten, bieten
sie fast durchweg **neben** einer Datumssortierung an, nicht anstelle. Der
Marktstandard ist also nicht „Relevanz statt Zeit", sondern **„beides mit einem
Schalter"** — und der Schalter ist genau die Bedienlast, die Zeichnung 3b für
dieses Werkzeug schon einmal abgelehnt hat (B7). Wer sich auf den Marktvergleich
beruft, um BM25 zu begründen, begründet damit Weg B, nicht die Abschaffung der
Gruppen.

---

## 6. Urteil und Bedingung

**Urteil zum Zielkonflikt:** Die Tagesgliederung gewinnt. `SPEC.md:261` bleibt,
wie es steht. Der Satz *„nur so trägt sie deren Tagesgruppen"* ist nach B1
sogar stärker, als er sich liest: Die Gruppen tragen in der Trefferliste
**Information, die der Eintrag selbst nicht hat**.

**Dritter Weg:** Weg E — die Ordnung bleibt, die Fundstelle wird sichtbar
(E1 als kleine, zeichnungstreue Stufe). Alle Wege, die an der Sortierung
drehen, kosten mehr Bedienlast oder mehr Erklärung, als sie einbringen; Weg A
und Weg C halte ich für **fail**, Weg B und Weg D für **warn**.

**Bedingung, unter der Relevanzsortierung hier Nutzen brächte** — alle vier
Punkte müssten zutreffen, nicht einer:

1. **Typische Trefferzahl über einem Bildschirm.** Gemessen am Bestand des
   Kunden, nicht geschätzt: Wenn übliche Anfragen regelmäßig mehr als rund
   sieben Treffer liefern (B5), hört der Nutzer auf, alles zu lesen.
2. **Punktwerte, die sich unterscheiden.** Die Rangfolge muss aus einem für den
   Nutzer nachvollziehbaren Grund entstehen. Bei zweizeiligen Notizen mit
   einmaligem Vorkommen ist das nicht gegeben (B6); zu klären ist außerdem der
   Trigramm-Punkt aus Abschnitt 3.
3. **Der Tag bleibt am Eintrag.** Ohne Umbau von `entryTimestamp()` für den
   Suchfall ist jede rangsortierte Liste eine Verschlechterung (V1). Der Umbau
   gehört in die Schätzung, nicht in die Fußnote.
4. **Ein Kundenbefund, der die Sortierung meint.** Klagt der Kunde „ich finde
   in der Trefferliste nicht, was ich suche", ist die erste richtige Antwort
   Weg E, nicht BM25. Erst wenn er sagt „die Reihenfolge ist falsch", ist die
   Sortierung gemeint.

Trifft nur Punkt 1 zu, sind Suchverfeinerung (die Operatoren aus S7) und
Weg E die billigeren Antworten — S7 verkürzt die Trefferliste, statt sie
umzusortieren, und ist ohnehin geplant.

---

## 7. Vorschläge — nicht ausgeführt

Ich habe auftragsgemäß nichts gebaut, gemessen oder aufgenommen. Was ich für
eine belastbare Entscheidung für nötig halte:

- **M1 — Sichtbare Einträge exakt messen.** Bildlauf der Bibliothek mit langer
  Trefferliste bei 900×600 und 1400×900, ausgezählt: wie viele Einträge stehen
  im Bild, mit Köpfen und ohne. Ersetzt die am Bild abgelesene Zahl aus B5
  durch eine gemessene. Läufer `searchshots`, vorher frisch bauen
  (CLAUDE.md, Prüfhaltung), `QT_QPA_PLATFORM=offscreen`,
  `QT_QPA_PLATFORMTHEME=kde`.
- **M2 — Trefferzahlen am echten Bestand.** Zehn Anfragen, die der Kunde
  tatsächlich stellen würde, gegen seine eigene Datenbank; erfasst wird allein
  die Trefferzahl. Das ist die Messung, die Bedingung 1 aus Abschnitt 6
  entscheidet, und sie ist billig.
- **B — Bild einer rangsortierten Trefferliste**, falls der Kunde die Frage
  offenhalten will: dieselben fünf Treffer aus `s6-e2`, einmal chronologisch
  mit Köpfen und einmal flach in beliebiger Rangfolge, nebeneinander. Der
  Verlust aus V1 und B2 ist am Bild in zwei Sekunden zu sehen und in Prosa
  nicht. **Ich habe dieses Bild nicht erzeugt** — es bräuchte einen Läufer, der
  eine Ordnung darstellt, die es im Produkt nicht gibt; ob dieser Aufwand
  gerechtfertigt ist, entscheidet der PO.

## 8. Offene Punkte

- **warn (B9) — Die Trefferliste hat keine Zeichnung.** Ihre Regeln leben als
  Fließtext im Festlegungsblock von 2c. Solange nur „dieselbe Gliederung wie
  3a" gilt, trägt das; sobald jemand über ihre Ordnung entscheidet, fehlt der
  Prüfgegenstand. **Vorschlag, nicht ausgeführt:** eine Zeichnung 3c
  „Trefferliste" mit den Fällen Lücke, ein Treffer je Gruppe, keine Treffer —
  die drei Fälle sind aus dem S6-Review samt Bildern bereits belegt. Das wäre
  Gestaltungsarbeit und braucht einen Auftrag.
- **Technisch offen (an `Recherche-BM25` und `Dev-BM25-Iststand`):** Verhalten
  von bm25 über einem trigram-Index; insbesondere, ob die Länge des Suchworts
  in den Rang eingeht (Abschnitt 3).
- **Nicht geprüft:** die Sortierfrage für den Fall gemischter Anfragen nach S7
  im Detail (der Parser ist nicht gebaut), und die Wirkung auf die
  Kategorien-Sidebar ab M3 — beide liegen jenseits des heutigen Standes.

---

*Erstellt von `denkzettel-ux` am 04.08.2026. Geändert wurde in diesem Lauf
ausschließlich diese Datei.*
