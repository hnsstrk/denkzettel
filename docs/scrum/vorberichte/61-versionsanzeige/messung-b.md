# Vorprüfung #61 — Messung B (Scrum Master)

Gemessen am 2026-08-04, 20:30–20:38 CEST, Stand `6acc87e`. Unabhängig von
Bearbeiter A; dessen Messung war zum Zeitpunkt dieser Arbeit nicht gelesen.

Messbeleg: `messung-b-bestand.txt` in diesem Ordner.

## Feld 1 — Dateimenge (so weit ohne A gemessen)

| | **#61** |
|---|---|
| **Quellen und Tests** | `src/main.cpp` (87 Z.) — die Kopfzeilen `:17–42`, siehe Falle 2. **Neu:** Argumentbehandlung vor `KDBusService`.<br>`src/ui/librarywindow.{h,cpp}` — **nur, falls der Über-Dialog gebaut wird** (siehe Feld 3).<br>`tests/` — ein neuer Nachweis, dass die Zahl aus `CMakeLists.txt` ankommt; Ablage offen (`shelltest.cpp` liegt am nächsten). |
| **Build** | `CMakeLists.txt` (Wurzel) — `project(… VERSION 0.1.0)` steht in `:3`, ein Durchreichweg fehlt vollständig.<br>`src/CMakeLists.txt` — `target_compile_definitions` oder eine erzeugte Kopfdatei; bei Über-Dialog zusätzlich `KF6::XmlGui`.<br>`.github/workflows/ci.yml` — **nur bei Über-Dialog**, siehe Falle 3. |
| **Fachliche Quellen** | `SPEC.md` Abschnitt 15 (`:719–747`) — die Versionsregeln haben dort keinen Platz und brauchen einen neuen Aufzählungspunkt, vermutlich am Paketierungspunkt `:743–747`. |
| **Ausdrücklich nicht** | `CHANGELOG.md` (Abgrenzung im Issue), `docs/scrum/PROZESS.md` (der Ablauf steht dort bereits, Sprint-Abschluss Punkt 10). |

## Feld 2 — Fallen (Teilmessung)

1. **Die Version erreicht den Code heute an keiner Stelle.** `git grep` über
   `src/`, `tests/` und alle drei `CMakeLists.txt` nach `KAboutData`,
   `setApplicationVersion`, `PROJECT_VERSION`, `QCommandLineParser`,
   `addVersionOption`, `version.h` liefert **null Treffer**. Es gibt keinen
   Durchreichweg, keine Argumentbehandlung und kein `KAboutData`. Die Story baut
   drei Dinge, nicht eines.
2. **`KAboutData::setApplicationData()` läuft in den Namens-Tanz von
   `main.cpp:21–42`.** Dort wird `applicationName` **absichtlich dreimal**
   gesetzt: `denkzettel` → `Daemon` (damit `KDBusService` den in SPEC 2.3
   festgelegten Busnamen `org.denkzettel.Daemon` bildet) → zurück auf
   `denkzettel` (damit Konfig- und Datenpfade stimmen). `setApplicationData()`
   setzt genau dieses Feld mit. Wer es an der falschen Stelle aufruft, ändert
   entweder den Busnamen oder den Pfad der Datenbank — und beides fällt beim
   ersten Start auf, aber nicht im Testlauf.
3. **`denkzetteld --version` bei laufendem Dienst.** `KDBusService::Unique`
   (`main.cpp:41`) reicht einen zweiten Start an die laufende Instanz weiter.
   Die Versionsausgabe muss deshalb **vor** Zeile 41 abgehandelt werden und den
   Prozess beenden, sonst öffnet `--version` das Erfassungsfenster.
4. **Ein Über-Dialog zieht eine neue Abhängigkeit nach.**
   `KAboutApplicationDialog` liegt in **`kxmlgui`** (nachgemessen:
   `/usr/include/KF6/KXmlGui/KAboutApplicationDialog`, Paket `kxmlgui 6.28.0`).
   `KF6::XmlGui` steht in keiner `CMakeLists.txt` des Projekts und **`kxmlgui`
   steht nicht in der Paketliste von `.github/workflows/ci.yml`** — der
   automatische Lauf bräche ohne diese Zeile.
5. **Für einen Über-Dialog gibt es keine Zeichnung.** `wireframes/` enthält eine
   Datei (`Denkzettel Wireframes.dc.html`); eine Suche nach „about", „Version"
   oder „Über-Dialog" darin liefert nichts. Ein UI-Element ohne Wireframe kann
   `denkzettel-ux` nicht gegen die Referenz prüfen — DoD 3 hinge in der Luft.

## Feld 3 — AK-Urteil: **ready = nein**

**AK 1 ist unbestimmt.** „`denkzetteld --version` (und die Bibliothek über den
Über-Dialog **bzw.** `KAboutData`)" lässt offen, ob ein Dialog gebaut wird.
Diese Frage entscheidet über die halbe Story: mit Dialog kommen eine neue
KF6-Abhängigkeit, eine CI-Paketzeile, ein UI-Review und eine fehlende Zeichnung
dazu (Fallen 4 und 5); ohne Dialog ist es Argumentbehandlung plus
`KAboutData`-Registrierung. Ein Kriterium, das zwei verschiedene Stories
zulässt, ist nicht einzeln prüfbar.

**AK 2 enthält eine Kann-Aussage.** „das PKGBUILD (#41) **kann** die Nummer aus
`CMakeLists.txt` beziehen" — es gibt kein PKGBUILD im Repo (`git ls-files`
zeigt keines; #41 ist offen und heißt S28). Ein Kriterium über eine Datei, die
nicht existiert, ist nicht prüfbar; nach dem DoR-Zusatz vom 04.08.2026 („ein
Dateiname ist erst dann ein Prüfmittel, wenn `git ls-files` ihn zeigt") fällt es
aus. Der prüfbare Teil von AK 2 — „Die Versionsregeln stehen in der SPEC" —
trägt, und zwar allein.

**AK 3 ist innerhalb der Story nicht erfüllbar.** „Bei der nächsten
Kundenabnahme wird erstmalig erhöht (→ 0.2.0) und `v0.2.0` getaggt" beschreibt
einen Vorgang **nach** der Abnahme (Sprint-Abschluss, Takt 2, Punkte 9 und 10).
Zum Zeitpunkt der DoD-Prüfung in Takt 1 ist es unerfüllbar — dieselbe
Fehlerbauart, an der Sprint 3 DoD 5 und DoD 6 als Mängel gebucht hat, obwohl sie
noch gar nicht dran waren. Es ist kein Akzeptanzkriterium, sondern eine
Folgehandlung.

**Behebung (PO), drei Sätze:**
- AK 1 entscheiden: Über-Dialog ja oder nein. Bei „ja" gehört ein UX-Schritt
  davor.
- AK 2 auf den SPEC-Teil kürzen; die PKGBUILD-Zeile als Hinweissatz aus den
  Kriterien nehmen.
- AK 3 streichen und stattdessen als Vollzugszeile in den Sprint-Abschluss
  nehmen — dort steht der Ablauf ohnehin schon.

## Feld 5 — Größenklasse: **`size:m`**

**Ausdrücklich nicht `size:s`**, obwohl das Issue nach einer Zeile Durchreichen
klingt. Gemessen sind es drei Bauteile, die es heute alle nicht gibt (Falle 1):
Durchreichweg von CMake in den Code, Argumentbehandlung vor `KDBusService`
(Falle 3), `KAboutData`-Registrierung im engsten Abschnitt von `main.cpp`
(Falle 2). Dazu die SPEC-Nachziehung (DoD 4) und ein neuer Test.

**Kommt der Über-Dialog dazu, ist es an der Grenze zu `size:l`:** neue
KF6-Abhängigkeit, CI-Paketliste, UI-Story mit Bildprüfung und eine Zeichnung,
die erst entstehen muss. Ich stufe auf `m` ein, weil AK 1 diese Hälfte nur
zulässt und nicht verlangt; wird sie bei der Behebung von AK 1 hineingenommen,
ist die Klasse neu zu setzen.

## Feld 6 — Offene Fragen

**An den Kunden:**

1. **Soll die Version im Fenster sichtbar sein — und wenn ja, wo?** Das ist eine
   Produktentscheidung, keine technische. Heute hat die Bibliothek weder
   Menüleiste noch Hilfemenü (gemessen: kein `QMenuBar`, kein `KHelpMenu`, kein
   `QMenu` außer dem Tray-Menü). Ein Über-Dialog braucht also zusätzlich einen
   **Ort, von dem aus er aufgerufen wird**. Das steht in keinem AK.

**An den PO:**

2. **Kollision mit #73, inhaltlich statt textlich.** #73 AK 3 verlangt, dass die
   Metainfo „die Aussetzung von Punkt 10 bis #61 mitschreibt". Laufen beide im
   selben Sprint, ist dieser Satz beim Schreiben schon überholt. Die Reihenfolge
   ist zu entscheiden, nicht die Parallelität.
3. **Kollision mit #83 in SPEC 15.** #83 fasst nach meinem konsolidierten
   Bericht `SPEC.md:720–735` an (Abhängigkeitenliste, `KWindowEffects`); #61
   braucht einen Punkt im selben Listenblock, vermutlich bei `:743–747`. Acht
   Zeilen Abstand — über der Mischbreite von Git, aber im selben Absatz. Bei
   gleichzeitiger Arbeit ist ein Rebase fällig, kein Konflikt.
