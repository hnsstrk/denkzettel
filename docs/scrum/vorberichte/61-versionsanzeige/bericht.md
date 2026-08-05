# Vorprüfbericht #61 — Versionsanzeige und Versionsregeln

**Konsolidiert vom PO am 05.08.2026** aus `messung-a.md` (Bearbeiter A,
`denkzettel-dev`) und `messung-b.md` (Bearbeiter B, Scrum Master), beide gegen
Stand `6acc87e`, unabhängig voneinander.

**Ergebnis: `size:m`, ready — nach Nachschärfung der Kriterien durch den PO.**

---

## 1. Der Streitpunkt: `s` gegen `m`

Die einzige echte Abweichung dieses Laufs. **Die Regel greift: Weichen die
Größenklassen um eine Stufe ab, gilt die höhere.**

| | Klasse | Begründung |
|---|---|---|
| **A** | `size:s` — *mit einer Bedingung* | „Am Build ist fast nichts zu tun, und `KAboutData` kostet gar nichts" — `KF6::CoreAddons` steht bereits `PUBLIC` und erreicht `denkzetteld`; wer nach einem `find_package`-Eintrag sucht, sucht umsonst |
| **B** | `size:m` — *ausdrücklich nicht `s`* | „Gemessen sind es **drei Bauteile, die es heute alle nicht gibt**": Durchreichweg von CMake in den Code, Argumentbehandlung vor der Einzelinstanz-Weiche, `KAboutData`-Registrierung im engsten Abschnitt von `main.cpp` |

**Beide haben recht in dem, was sie gemessen haben** — A misst die Kosten des
Bauens (fast null), B misst die Zahl der Bauteile (drei, alle neu). Für die
Klassendefinition zählt B: „läuft nebenher — wenige Dateien, **kein neuer
Prüfweg**". Argumentbehandlung vor `KDBusService` ist ein neuer Weg, gleich wie
billig die Bibliothek ist.

**Der harte Beleg für B:** `git grep` über `src/`, `tests/` und alle drei
`CMakeLists.txt` nach `KAboutData`, `setApplicationVersion`, `PROJECT_VERSION`,
`QCommandLineParser`, `addVersionOption` und `version.h` liefert **null
Treffer**. Es gibt keinen Durchreichweg, keine Argumentbehandlung und kein
`KAboutData`. **`size:m`.**

## 2. Der teuerste Fund — beide unabhängig

**`KAboutData::setApplicationData()` zerstört den D-Bus-Namen oder den
Datenbankpfad.** `src/main.cpp` setzt `applicationName` **absichtlich dreimal**:
`denkzettel` → `Daemon` (damit die Busanmeldung den in SPEC 2.3 festgelegten
Namen bildet) → zurück auf `denkzettel` (damit Konfig- und Datenpfade stimmen).
`setApplicationData()` setzt genau dieses Feld mit.

**Wer es an der falschen Stelle aufruft, ändert entweder den Busnamen oder den
Pfad der Datenbank — und beides fällt beim ersten Start auf, aber in keinem
Testlauf.** A ergänzt die sichtbare Folge: Läge die Auswertung im Namensfenster,
stünde in der Versionszeile `Daemon 0.1.0` statt `denkzettel 0.1.0`.

Daraus ist ein eigenes Akzeptanzkriterium geworden, nachzuweisen **am laufenden,
installierten Stand** — nicht im Testlauf. Kein Rückgabewert meldet den Bruch,
und die globalen Kürzel hängen an derselben Anwendungs-Id: bricht sie, brechen
sie mit.

## 3. Drei weitere Fallen

**Bei laufendem Dienst öffnet `--version` heute ein Notizfenster** (beide). Die
Einzelinstanz-Weiche reicht einen zweiten Start an die laufende Instanz weiter;
die Versionsausgabe muss **davor** abgehandelt werden und den Prozess beenden.

**`setupCommandLine()` bringt sechs Optionen mit, nicht eine** (A) — darunter
`--desktopfile <Dateiname>`, die zur Laufzeit genau den Wert überschreibt, der
SPEC 2.4 trägt. Wer sie mit scharf schaltet, gibt die Anwendungs-Id auf der
Kommandozeile frei.

**`process()` weist unbekannte Optionen ab, `parse()` nicht** (A) — und die
naheliegende Sorge dabei ist gemessen **unbegründet**: `QApplication` entfernt
`-platform offscreen` bereits aus `arguments()`, ehe der Parser sie sieht. Die
Bildläufer und `ctest` stolpern also nicht. Der PO hat zusätzlich geprüft, dass
beide `Exec=`-Zeilen der Desktop-Datei `denkzetteld` **ohne Argument** starten —
das Abweisen bricht den Autostart nicht.

**Das Prüfmittel läuft ohne Sitzungsbus** (A), also auch im automatischen Lauf —
unter der Bedingung, dass die Auswertung vor der Einzelinstanz-Weiche liegt.

## 4. Die sechs Felder

**Feld 1 — Dateimenge.** `src/main.cpp` (Kopfzeilen und neue Argumentbehandlung
vor der Einzelinstanz-Weiche) · `CMakeLists.txt` der Wurzel und
`src/CMakeLists.txt` (Durchreichweg als Compile-Definition oder erzeugte
Kopfdatei) · ein neuer Nachweis in `tests/` · `SPEC.md` Abschnitt 15, **am
Ende**, sowie 2.3 und 2.4 für die entdeckte Bedingung.
**Ausdrücklich nicht:** `CHANGELOG.md` (der Ablauf steht im Sprint-Abschluss),
`docs/scrum/PROZESS.md` (der Ablauf steht dort bereits),
`src/ui/librarywindow.{h,cpp}` (der Über-Dialog ist **#87**).

**Feld 2 — gemessene Fallen:** die vier aus §2 und §3.

**Feld 3 — AK-Urteil.** Ausgangsfassung **nicht ready** (B), drei Gründe:
- **AK 1 war unbestimmt** — „über den Über-Dialog **bzw.** `KAboutData`" ließ
  offen, ob ein Dialog gebaut wird. Das entschied über die halbe Story. *Ein
  Kriterium, das zwei verschiedene Stories zulässt, ist nicht einzeln prüfbar.*
- **AK 2 enthielt eine Kann-Aussage über eine Datei, die es nicht gibt** — das
  PKGBUILD. Nach dem DoR-Zusatz vom 04.08.2026 („ein Dateiname ist erst dann ein
  Prüfmittel, wenn `git ls-files` ihn zeigt") fällt es aus.
- **AK 3 war innerhalb der Story nicht erfüllbar** — „bei der nächsten
  Kundenabnahme wird erhöht und getaggt" beschreibt einen Vorgang **nach** der
  Abnahme. *Dieselbe Fehlerbauart, an der Sprint 3 DoD 5 und DoD 6 als Mängel
  gebucht hat, obwohl sie noch gar nicht dran waren.*

**Behoben durch den PO am 05.08.2026:** acht Kriterien, der Über-Dialog nach
**#87** ausgelagert, AK 3 als Fälligkeit statt als Haken. **Damit ready.**

**Feld 4 — Prüfmittel.** `denkzetteld --version` mit `QT_QPA_PLATFORM=offscreen`
und **ohne** `DBUS_SESSION_BUS_ADDRESS`; `git grep` für die Einzigkeit der
Zahl; `git diff SPEC.md`.
**Grenze, ausgesprochen:** Dass die Busnamen und die globalen Kürzel nach der
Umstellung halten, ist **nur am laufenden, installierten Stand** nachweisbar —
kein Rückgabewert meldet den Bruch, und kein bestehender Test fängt ihn. Das
gehört in die Installationsprüfung des Sprint-Abschlusses, nicht in `ctest`.

**Feld 5 — Größenklasse: `size:m`** (höhere von beiden, §1).

**Feld 6 — offene Fragen: alle vom PO entschieden.**

| Frage | Entscheidung |
|---|---|
| Über-Dialog Teil der Story? | **Nein** — eigene Story **#87**. Es gibt weder einen Dialog noch einen Ort für seinen Aufruf noch eine Zeichnung; `KAboutApplicationDialog` zöge `kxmlgui` nach, das auch in der Paketliste des automatischen Laufs fehlt |
| Bedingung für die Busnamen ins AK? | **Ja**, und zusätzlich als entdeckte Bedingung in SPEC 2.3/2.4 (DoD 4/B9) |
| PKGBUILD-Teil von AK 2 | **Gestrichen**, ersetzt durch das Prüfbare: „die Nummer steht an genau einer Stelle" |
| AK 3 (Erhöhen und Taggen) | **Aus den Kriterien heraus.** Es ist eine Fälligkeit nach der Abnahme und steht bereits im Sprint-Abschluss, Takt 2 |
| Unbekannte Schalter abweisen? | **Ja.** Bewusste Verhaltensänderung, deshalb im Issue und nicht in einer Commit-Botschaft. Geprüft, dass sie den Autostart nicht bricht |
| `--desktopfile` scharf schalten? | **Nein** — sie gäbe die Anwendungs-Id auf der Kommandozeile frei |
| Wo im SPEC? | **Am Ende von Abschnitt 15**, nicht in der KF6-Aufzählung — das hält die Naht zu #83 offen |
| Reihenfolge gegen #73 | **#61 zuerst.** #73 AK 3 verlangt, die Metainfo schreibe „die Aussetzung von Punkt 10 bis #61" mit; laufen beide im selben Sprint, ist der Satz beim Schreiben schon überholt |

## 5. Warum diese Story Prozessgewicht hat

Solange #61 offen ist, führt der Sprint-Abschluss **Punkt 10 als ausgesetzt**:
Version und Tag bleiben aus, weil die Zahl in `CMakeLists.txt` die Anwendung
nicht erreicht und ohne Sichtbarkeit eine Behauptung wäre. Die Einträge sammeln
sich unter `[Unveröffentlicht]`, und Abnahmen davor bekommen keine Version
rückwirkend.

**Mit #61 endet dieser Zustand.** Das ist der Grund, warum die Story in Sprint 8
liegt und nicht später: Der Kunde nimmt nach Sprint 8 ab, und diese Abnahme soll
eine Versionsnummer tragen können.
