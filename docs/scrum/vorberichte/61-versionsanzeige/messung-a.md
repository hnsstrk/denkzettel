# Vorprüfung #61 — Messung Bearbeiter A (`denkzettel-dev`)

**Gegenstand:** Issue #61, „S34: Versionsanzeige und Versionsregeln (0.x-SemVer,
MINOR je Kundenabnahme)" · **Datum:** 04.08.2026, Ganymed · **Quellstand:**
`main` @ `6acc87e` · **Belege:** `messungen/`, Sonden in `sonden/`,
wiederholbar über `bash docs/scrum/vorberichte/61-versionsanzeige/pruefen.sh`

Dieser Bericht trägt die Felder **1, 2, 4 und 5**. **Feld 3 (Ready-Urteil) fällt
der Scrum Master**, Feld 6 steht als „Offene Fragen" am Ende.

**Stand der Werkzeuge** (B17): kcoreaddons 6.28.0, kdbusaddons 6.28.0,
kxmlgui 6.28.0, qt6-base 6.11.1, extra-cmake-modules 6.28.0, cmake 4.4.2.

---

## Der Ist-Zustand in vier Sätzen

1. Die Version steht an **genau einer Stelle**: `CMakeLists.txt:3`,
   `project(denkzettel VERSION 0.1.0 LANGUAGES CXX)`. Sie wird von dort **nirgends
   weitergereicht** — kein `ecm_setup_version`, keine generierte Kopfdatei,
   keine `target_compile_definitions`, kein `setApplicationVersion`.
   `git grep -n "PROJECT_VERSION\|applicationVersion\|KAboutData" -- src tests`
   ist leer.
2. `QCoreApplication::applicationVersion()` ist zur Laufzeit die **leere
   Zeichenkette** (Sonde 3, Modus `roh`).
3. **Es gibt kein `KAboutData` und keinen Über-Dialog** — auch keinen Ort, an
   dem einer hinge: `LibraryWindow` hat keine Menüleiste, das Tray-Menü
   (`src/shell/trayicon.cpp:75–103`) führt sechs Einträge, keinen davon „Über".
4. Die Option `--version` beantwortet **niemand** — weder Qt noch KF6 noch wir.

---

## Feld 1 — Dateimenge (am Code vermessen, Notation nach B13)

| | **#61** — die Version sichtbar machen |
|---|---|
| **Issue** | **#61** (`epic:M7`, `typ:story`) |
| **Zweig** | `story/61-versionsanzeige` |
| **Quellen & Tests** | `src/main.cpp` (87 Zeilen) — **eine Stelle**: der Block `:17–42` zwischen `QApplication app(argc, argv)` und `KDBusService service(...)`. Dort entstehen `KAboutData` samt `setupCommandLine()`/`parse()`; die Auswertung **muss vor Zeile 41** liegen (F3). Kein anderer Quelltext wird berührt.<br>**Neu**: ein Prüfmittel für „nicht doppelt gepflegt" — nach dem Muster von `tests/installtest.cmake` ein `tests/versiontest.cmake`, das `denkzetteld --version` startet und gegen `${PROJECT_VERSION}` vergleicht. Ein QTest kann das **nicht**: die Zahl liegt in CMake, nicht im Code. |
| **Build** | `CMakeLists.txt:3` bleibt die Quelle und wird nicht angefasst. **Neu, eine Zeile:** entweder `target_compile_definitions(denkzetteld PRIVATE DENKZETTEL_VERSION="${PROJECT_VERSION}")` in `src/CMakeLists.txt` (nach `add_executable`, `:110–125`) oder `ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX DENKZETTEL VERSION_HEADER …)` in der Wurzel — beide gemessen verfügbar (`/usr/share/ECM/modules/ECMSetupVersion.cmake`).<br>`tests/CMakeLists.txt` — Registrierung des neuen Tests, Muster `:176–184`.<br>**`KF6::CoreAddons` ist bereits verkabelt** und erreicht `denkzetteld` (F6): kein `find_package`-Eintrag nötig. |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-NN-s34-versionsanzeige/` — neu anzulegen. **Wiederverwendbar:** `sonden/busname.cpp` dieser Vorprüfung ist der fertige Nachweis, dass SPEC 2.3 und 2.4 die Umstellung überleben; `messungen/sonde2-zweitstart.sh` ist der fertige Nachweis für den Zweitstart-Weg. |
| **Fachliche Quellen** | **SPEC hat heute keine einzige Zeile über Produktversionen** (`grep -n -i version SPEC.md` findet nur Schema- und Bibliotheksversionen). Die Regeln aus dem Issue brauchen einen **neuen Absatz**; naheliegender Ort ist **SPEC 15** (`:719–748`, „Build, Abhängigkeiten, Paketierung"), weil dort schon Paketierung und `cmake --install` stehen.<br>**SPEC 2.3** (`:54–67`) und **SPEC 2.4** (`:68–108`) bekommen nach DoD 4/B9 je eine **entdeckte Bedingung**: Busname und Desktop-Dateiname überleben `KAboutData` nur, wenn beide dort gesetzt werden (F4).<br>**SPEC 15 KF6-Liste** — **nur falls** ein Über-Dialog gebaut wird, dann kommt `KXmlGui` dazu (F7). |
| **Ausdrücklich nicht** | `src/capture/*`, `src/ui/*`, `src/store/*`, `src/shell/*`, `tests/capturetest.cpp`, `tests/captureshots.cpp`, `tests/librarytest.cpp`, sämtliche Bildläufer (`editshots`, `libraryshots`, `searchshots`, `readmeshots`, `captureshots`), `wireframes/`, `desktop/`.<br>**Und ausdrücklich nicht:** `CHANGELOG.md` (die Überschrift `[Unveröffentlicht]` → `[0.2.0]` schreibt die **Abnahme** fort, nicht diese Story), `docs/scrum/PROZESS.md` Punkt 10 (`:606–620`, die Aussetzung aufzuheben ist Scrum-Master-Fläche), der Tag `v0.2.0` (Sprint-Abschluss, nicht Dev). |

### Kollisionsfläche gegen #83 — **läuft daneben**

| Datei | #83 | #61 | Urteil |
|---|---|---|---|
| `src/capture/capturewindow.{h,cpp}` | ganz | — | kein Berührungspunkt |
| `tests/capturetest.cpp`, `tests/captureshots.cpp` | ja | — | kein Berührungspunkt |
| `src/main.cpp` | — | `:17–42` | #61 allein |
| `CMakeLists.txt` (Wurzel), `src/CMakeLists.txt`, `tests/CMakeLists.txt` | **nichts** (Feld 1 der #83-Messung: „Build: Nichts") | je eine Zeile | kein Berührungspunkt |
| **`SPEC.md`** | 3.1 (`:152–186`), 3.2 (`:188–214`), **15 (`:719–748`)**, 16 (`:749–809`) | **15 (`:719–748`)**, 2.3, 2.4 | **die einzige Naht** |

**Woran ich das messe:** an den Dateilisten beider Stories und an den
Abschnittsgrenzen von `SPEC.md` (`grep -n "^## " SPEC.md`). Der Sprint-6-Maßstab
war „der kleinste Abstand ist null: beide schreiben in dieselbe Datei"; hier ist
der Abstand im **Code unendlich** — keine gemeinsame Quelldatei, kein
gemeinsamer Test, kein gemeinsamer Bildläufer.

**Die eine Naht, benannt:** In `SPEC.md` treffen sich beide in Abschnitt 15.
#83 ändert dort die KF6-Aufzählung (`KWindowEffects` fehlt); #61 hängt einen
neuen Absatz über Versionsregeln an — und **falls** der Über-Dialog gebaut wird,
zusätzlich `KXmlGui` in dieselbe Aufzählung. Das ist ein 30-Zeilen-Abschnitt;
zwei Stränge, die gleichzeitig hineinschreiben, kollidieren im Rebase.
**Empfehlung:** #61 schreibt seinen Versionsabsatz **ans Ende von 15**, nicht in
die KF6-Aufzählung — dann bleibt der Abstand bei rund 25 Zeilen und Git mischt.
Kommt `KXmlGui` dazu, ist die Naht wieder null und der PO taktet die beiden
SPEC-Änderungen.

**Antwort auf die gestellte Frage:** **Ja, #61 kann neben #83 laufen** —
vorausgesetzt, der Über-Dialog fällt weg oder die SPEC-15-Änderung wird
getaktet.

---

## Feld 2 — Gemessene Fallen (die Zeilen für den Spawn-Auftrag)

**F1 — `--version` wird heute stillschweigend verschluckt und startet den
Dienst.** `denkzetteld --version`, `-v`, `--help` und der Aufruf ohne Argument
verhalten sich **identisch**: keine Ausgabe, kein Fehler, der Prozess läuft
weiter bis zur Zeitüberschreitung (Rückgabe 124). Wer glaubt, Qt oder KF6
beantworte die Option von selbst, irrt.
*Beleg:* `messungen/sonde1-optionen.txt`, alle vier Abschnitte.

**F2 — Bei laufendem Dienst öffnet `--version` heute ein Notizfenster.**
Gemessen in einer eigenen D-Bus-Sitzung: Der zweite Prozess ruft
`org.kde.KDBusService.CommandLine` auf dem ersten, kehrt nach **22 ms** mit
**Rückgabe 0** zurück und gibt **nichts** aus. `KDBusService` stellt diesen
Aufruf als `activateRequested` zu, und `src/main.cpp:54–57` hängt daran
`showCapture()`.
*Beleg:* `messungen/sonde2-zweitstart.txt` (Busmitschrift, Dauer, Rückgabe).
*Grenze:* Der Busaufruf und die Rückgabe sind **gemessen**; dass daraufhin das
Fenster erscheint, ist am Code gelesen (`main.cpp:54–57`), nicht im Bild
gesehen — offscreen ist nichts zu sehen.

**F3 — Die Auswertung muss vor `KDBusService` stehen.** Aus F2 folgt: Läge sie
danach, wäre `--version` bei laufendem Dienst weiterhin ein Fensteraufruf, weil
der zweite Prozess seine eigene Auswertung nie erreicht — `KDBusService::Unique`
beendet ihn vorher. Der Rückgabewert 0 sähe dabei **wie ein Erfolg aus**.
Kandidat für die Liste „Rückgabewerte, die nichts belegen" in
`.claude/agents/denkzettel-dev.md`.

**F4 — `KAboutData::setApplicationData()` zerstört den D-Bus-Namen und den
Desktop-Dateinamen.** Der naheliegende Dreizeiler überschreibt drei der vier
Eigenschaften, die `src/main.cpp:21–26` mit SPEC-Begründung von Hand setzt:

| Eigenschaft | vorher | nach `setApplicationData` |
|---|---|---|
| `organizationDomain` | `denkzettel.org` | **`kde.org`** |
| `desktopFileName` | `org.denkzettel.Denkzettel` | **`org.kde.denkzettel`** |
| `applicationDisplayName` | `denkzettel` | `Denkzettel` |
| `applicationName` | `denkzettel` | `denkzettel` (unverändert) |

Am **tatsächlich angemeldeten Busnamen** nachgemessen, nicht aus der Kopfdatei
abgeleitet: Der Dienst meldet dann **`org.kde.Daemon`** statt
`org.denkzettel.Daemon`. Damit fallen die D-Bus-Einstiegspunkte (SPEC 2.3) und
über den Desktop-Dateinamen die Kürzel-Komponente (SPEC 2.4).
Die Heilung ist gemessen und kostet zwei Zeilen: `setOrganizationDomain()` und
`setDesktopFileName()` **auf dem `KAboutData`-Objekt**, vor
`setApplicationData()` — danach steht wieder `org.denkzettel.Daemon`.
*Beleg:* `messungen/sonde4-busname.txt`, die drei Modi `ohne` / `mit` /
`heilung`.
**Kein bestehender Test fängt das:** `git grep -n "org.denkzettel.Daemon" --
tests/` ist **leer**; `installtest` prüft nur die Dateinamen der
`.desktop`-Einträge, nicht den Busnamen zur Laufzeit.

**F5 — `setupCommandLine()` bringt vier Optionen mit, nicht eine.** Gemessen:
`-h/--help`, `--help-all`, `-v/--version`, `--author`, `--license` und
**`--desktopfile <Dateiname>`**. Die letzte überschreibt zur Laufzeit genau den
Wert aus F4 — wer sie mit `processCommandLine()` scharf schaltet, gibt die
Anwendungs-Id auf der Kommandozeile frei.
*Beleg:* `messungen/sonde3-versionswege.txt`, Zeile „bekannte Optionen".

**F6 — Am Build ist fast nichts zu tun, und `KAboutData` kostet gar nichts.**
`KF6::CoreAddons` steht bereits `PUBLIC` an `denkzettelcapture`
(`src/CMakeLists.txt:57`) und erreicht `denkzetteld`: die Linkzeile führt
`libKF6CoreAddons.so.6.28.0`, die Übersetzungszeile von `main.cpp` führt
`/usr/include/KF6/KCoreAddons`. Wer nach einem `find_package`-Eintrag sucht,
sucht umsonst.
*Beleg:* `docs/scrum/vorberichte/61-versionsanzeige/build/src/CMakeFiles/denkzetteld.dir/link.txt`
und `compile_commands.json` desselben Bauplatzes.

**F7 — Ein Über-Dialog kostet ein neues Framework.**
`KAboutApplicationDialog` liegt in **`kxmlgui`**
(`pacman -Qo /usr/include/KF6/KXmlGui/KAboutApplicationDialog`). `KXmlGui` steht
**weder in `CMakeLists.txt` noch in SPEC 15** — es wäre eine neue Bau- und
Laufzeitabhängigkeit und zöge #41 (PKGBUILD) mit. Dazu kommt: Es gibt **keinen
Ort** für den Eintrag (kein Menü in `LibraryWindow`, kein „Über" im Tray-Menü)
und **keine Wireframe-Aussage** — `wireframes/` ist UX-Fläche.

**F8 — `process()` weist unbekannte Optionen ab, `parse()` nicht. Qt-Optionen
sind gemessen unkritisch.** `parser.process(app)` beendet bei `--unbekannt` mit
**Rückgabe 1** und einer Fehlermeldung; heute nimmt `denkzetteld` jedes Argument
stillschweigend an. Das ist eine **Verhaltensänderung** und gehört entschieden,
nicht nebenbei mitgenommen.
Die naheliegende Sorge dabei ist gemessen **unbegründet**: `QApplication` hat
`-platform offscreen` bereits aus `QCoreApplication::arguments()` entfernt, ehe
der Parser sie sieht — `arguments()` enthält nur noch `--version`. Die
Bildläufer und `ctest` stolpern also nicht.
*Beleg:* `messungen/sonde3-versionswege.txt`, Abschnitte „--unbekannt" und
„Qt-Option auf der Kommandozeile".

**F9 — Das Prüfmittel läuft ohne Sitzungsbus, also auch im automatischen
Lauf.** `--version` mit `QT_QPA_PLATFORM=offscreen` und **ohne**
`DBUS_SESSION_BUS_ADDRESS` liefert `denkzettel 0.1.0`, Rückgabe 0. Bedingung
ist F3: Die Auswertung muss vor `KDBusService` liegen, sonst braucht der Lauf
einen Bus.
*Beleg:* `messungen/sonde5-ohne-bus.txt`.

**F10 — Die sichtbare Zeile lautet `denkzettel 0.1.0`, nicht `Denkzettel …`.**
`QCommandLineParser` setzt sie aus `applicationName` und `applicationVersion`
zusammen — und `src/main.cpp:40` stellt `applicationName` für die
Busanmeldung vorübergehend auf **`Daemon`**. Läge die Auswertung in diesem
Fenster, stünde dort `Daemon 0.1.0`.
*Beleg:* `messungen/sonde3-versionswege.txt`, Modus `kf6`.

---

## Feld 4 — Prüfmittel je Akzeptanzkriterium, und was ein Agent nicht kann

Das Issue trägt drei Kriterien; das erste zerfällt beim Messen in zwei.

| AK | Prüfmittel | Grenze |
|---|---|---|
| **1a** — `denkzetteld --version` zeigt die Version aus `CMakeLists.txt` | Ein CMake-Test nach dem Muster `tests/installtest.cmake`: `denkzetteld --version` starten (offscreen, ohne Bus, F9), Ausgabe gegen `${PROJECT_VERSION}` vergleichen, Rückgabe 0 und **Rückkehr in unter einer Sekunde** fordern — die Frist ist zugleich der Nachweis von F3 | keine |
| **1b** — „der Über-Dialog bzw. `KAboutData` zeigt sie" | Für `KAboutData`: ein QTest auf `KAboutData::applicationData().version()`. **Für einen Über-Dialog gibt es heute kein Prüfmittel** — es gibt weder den Dialog noch seinen Aufrufort noch eine Wireframe-Aussage (F7) | **Wird ein Dialog gebaut, ist es eine UI-Story:** DoD 3 verlangt ein eigenes Bild je Zustand, der UX-Agent ist zu beteiligen, und `wireframes/` liegt außerhalb der Dev-Dateimenge. Der Wortlaut lässt beide Lesarten zu — siehe Offene Frage 1 |
| **1c, nicht im Issue, aber vom Messen erzwungen** — die Umstellung bricht SPEC 2.3/2.4 nicht | `sonden/busname.cpp` misst den **tatsächlich angemeldeten** Busnamen in einer eigenen D-Bus-Sitzung (F4) — das ist der einzige Beleg, den es dafür gibt | Läuft nur, wo `dbus-run-session` verfügbar ist. Am **installierten** Stand prüft der PO-Takt (DoD 2), nicht der Agent |
| **2a** — die Versionsregeln stehen in der SPEC | `git grep -n "SemVer\|MINOR" SPEC.md` findet den neuen Absatz; DoD 4 | keine |
| **2b** — „das PKGBUILD (#41) kann die Nummer aus `CMakeLists.txt` beziehen" | **Kein Prüfmittel.** `git ls-files | grep PKGBUILD` ist **leer** — #41 ist offen, die Datei existiert nicht. Prüfbar ist allein die Voraussetzung: dass die Nummer an genau einer Stelle steht (Ist-Zustand 1) | **Ein Kriterium über eine Datei, die es nicht gibt.** Nach der DoR-Regel „ein Dateiname ist erst dann ein Prüfmittel, wenn `git ls-files` ihn zeigt" trägt dieses AK nicht — siehe Offene Frage 3 |
| **3** — „bei der nächsten Kundenabnahme wird erstmalig erhöht (→ 0.2.0) und `v0.2.0` getaggt" | **Kein Prüfmittel innerhalb der Story.** Der Auslöser ist die Kundenabnahme, die **nach** der Umsetzung liegt; Tag und Erhöhung sind Sprint-Abschluss (PROZESS.md Punkt 10, `:606–620`) | **Das ist kein Akzeptanzkriterium dieser Story, sondern eine Fälligkeit danach.** Es kann zum Zeitpunkt des Reviews nicht abgehakt werden — weder erfüllt noch verletzt. Siehe Offene Frage 4 |

**Was ein Agent an dieser Story grundsätzlich nicht prüfen kann:**

1. **Die Wirkung am installierten Stand.** Ob `/usr/bin/denkzetteld --version`
   die Zahl zeigt, gehört DoD 2 und dem PO-Takt. Ich habe nicht installiert.
2. **Ob der Kunde die Zahl an der richtigen Stelle sucht.** Kommandozeile,
   Tray-Menü, Über-Dialog — das ist eine Gestaltungsfrage, kein Messwert.
3. **Dass `--version` bei laufendem Dienst nach der Umstellung *kein* Fenster
   mehr öffnet**, lässt sich offscreen nur am ausbleibenden Busaufruf zeigen
   (Sonde 2 misst ihn), nicht am ausbleibenden Fenster.

---

## Feld 5 — Größenklasse: **`size:s`** — mit einer Bedingung

Bedeutung laut `PROZESS.md`: *„läuft nebenher — wenige Dateien, kein neuer
Prüfweg"*.

**Wofür `s`:** Die Änderung ist ein Block in **einer** Quelldatei
(`src/main.cpp:17–42`), **eine** Build-Zeile und **ein** SPEC-Absatz. Keine
Bibliothek kommt hinzu (F6), kein Bildläufer wird angefasst, kein UI-Zustand
ändert sich, keine bestehende Testzusicherung fällt. Der Prüfweg ist **nicht
neu**, sondern der vorhandene: `tests/installtest.cmake` ist genau die Bauart,
die AK 1a braucht. Die Kollisionsfläche gegen #83 ist im Code leer.

**Wofür nicht `m`:** `size:m` „trägt einen Strang aus". Zum Vergleich hat #83
denselben Bericht mit vier SPEC-Stellen, einem neuen Ereignisweg, drei fallenden
Zusicherungen und einem neuen Prüfweg begründet. Davon trifft hier **nichts** zu.
Auch die schärfste gemessene Falle (F4) kostet zwei Zeilen und einen Test.

**Die Bedingung, unter der es `m` wird — bitte vor dem Ziehen entscheiden:**
Wird AK 1 als **Über-Dialog** gelesen, dann kommen `KXmlGui` als neues Framework
(F7), ein Eintrag im Tray-Menü oder in der Bibliothek, eine SPEC-15-Ergänzung in
derselben Aufzählung wie #83, eine Wireframe-Aussage durch den UX-Agenten und
ein Bildbeleg je Zustand nach DoD 3 dazu. Damit ist es eine **UI-Story** und
mindestens `size:m` — und **neben `size:l` (#83) hätte sie in Sprint 7 keinen
Platz**. Die Klasse dieser Story hängt an genau dieser einen Lesart.

**`xl` messe ich nicht**, und eine Teilung empfehle ich nicht: Fällt der Dialog
weg, ist nichts zu teilen; fällt er nicht weg, ist die Naht ohnehin die zwischen
Kommandozeile und Dialog — und dann gehört der Dialog in eine eigene Story mit
UX-Beteiligung, nicht in eine Teilung dieser hier.

---

## Feld 6 — Offene Fragen an PO oder Kunde

1. **Ist ein Über-Dialog Teil dieser Story?** AK 1 sagt „die Bibliothek über den
   Über-Dialog **bzw.** `KAboutData`". Gemessen gibt es weder einen Dialog noch
   einen Ort für seinen Aufruf noch eine Wireframe-Aussage, und er zöge `KXmlGui`
   nach (F7). *Dev-Empfehlung:* Diese Story füllt `KAboutData` und beantwortet
   `--version`; der Dialog wird eine eigene UI-Story mit UX-Beteiligung. Das
   hält sie bei `size:s` und damit neben #83 ziehbar.
2. **AK 1 braucht eine Bedingung**, die heute nicht dasteht (F4): Die Umstellung
   auf `KAboutData` darf `org.denkzettel.Daemon` (SPEC 2.3) und
   `org.denkzettel.Denkzettel` (SPEC 2.4) nicht verlieren. *Dev-Empfehlung:* als
   eigenes Kriterium ins Issue **und** als entdeckte Bedingung in SPEC 2.3/2.4
   (DoD 4/B9). Ohne das baut ein Strang den Dreizeiler, sieht keinen Fehler und
   bricht die Kürzel — kein Rückgabewert meldet es, und kein Test fängt es.
3. **AK 2, zweiter Teil, trägt nicht:** „das PKGBUILD (#41) kann die Nummer aus
   `CMakeLists.txt` beziehen" — es gibt kein PKGBUILD im Repository (#41 offen).
   Nach der DoR-Regel vom 04.08.2026 ist das kein Prüfmittel. *Vorschlag:*
   streichen und durch das Prüfbare ersetzen — „die Nummer steht an genau einer
   Stelle; `git grep` findet sie außerhalb von `CMakeLists.txt` nirgends fest
   verdrahtet."
4. **AK 3 ist keine Bedingung dieser Story, sondern eine Fälligkeit danach.**
   „Bei der nächsten Kundenabnahme wird erstmalig erhöht (→ 0.2.0) und `v0.2.0`
   getaggt" kann im Review nicht abgehakt werden — die Abnahme kommt später, und
   der Ablauf steht bereits in `PROZESS.md` Punkt 10. *Vorschlag:* aus den
   Kriterien herausnehmen und als Vollzugsvermerk am Sprint-Abschluss führen;
   sonst bleibt eine Story mit einem Haken offen, den niemand setzen kann.
5. **`--version` auf einen unbekannten Schalter: abweisen oder ignorieren?**
   (F8) `process()` beendet mit Rückgabe 1, heute nimmt der Dienst alles an.
   *Dev-Empfehlung:* abweisen — aber es ist eine bewusste Verhaltensänderung und
   gehört ins Issue, nicht in einen Commit-Kommentar.
6. **Soll `--desktopfile` scharf geschaltet werden?** (F5) `setupCommandLine()`
   bringt die Option mit; `processCommandLine()` gibt damit die Anwendungs-Id auf
   der Kommandozeile frei — genau den Wert, der SPEC 2.4 trägt.
   *Dev-Empfehlung:* nicht auswerten.
7. **Wo im SPEC steht die Versionsregel?** Vorschlag: am **Ende** von Abschnitt
   15, nicht in der KF6-Aufzählung — das hält die Naht zu #83 offen (Feld 1).

---

## Was ich **nicht** klären konnte

- Ob nach der Umstellung tatsächlich **kein Fenster** mehr aufgeht, wenn
  `--version` bei laufendem Dienst gerufen wird. Gemessen ist der Busaufruf, der
  es heute auslöst (F2); das Fenster selbst ist offscreen nicht zu sehen.
- Das Verhalten am **installierten** Stand — DoD 2, PO-Takt, ich habe nicht
  installiert.
- Ob der Kunde die Versionsanzeige **auf der Kommandozeile** überhaupt sucht
  oder im Tray-Menü. Das ist Frage 1 und keine Messfrage.
- Den **Aufwand in Zeit**. Wird in diesem Projekt nicht erhoben; die
  Größenklasse ist an Dateimenge und Prüfwegen gemessen.

---

## Befehle, mit denen ich gemessen habe

```
bash docs/scrum/vorberichte/61-versionsanzeige/pruefen.sh     # alle Sonden
gh issue view 61 --comments ; gh issue view 73 ; gh issue view 41
git grep -n "KAboutData\|setApplicationVersion\|PROJECT_VERSION\|QCommandLineParser" -- src tests
git grep -n "org.denkzettel.Daemon" -- tests/
git ls-files | grep -i -E "metainfo|appstream|PKGBUILD"
grep -n "^## " SPEC.md ; grep -n -i "version" SPEC.md
pacman -Qo /usr/include/KF6/KXmlGui/KAboutApplicationDialog
pacman -Q kcoreaddons kdbusaddons kxmlgui qt6-base extra-cmake-modules cmake
```

**Nicht getan:** nichts committet, nichts gepusht, nichts nach `/usr`
installiert, keine Zeile unter `src/`, `tests/`, `SPEC.md` oder `wireframes/`
geändert, den laufenden Dienst des Kunden nicht berührt (alle Busmessungen in
eigenen `dbus-run-session`-Sitzungen mit eigenen XDG-Verzeichnissen). Die beiden
Bauplätze liegen unter `docs/scrum/vorberichte/61-versionsanzeige/build/` und
`…/sonden/build/`, beide von `.gitignore` gedeckt; `build/` der
Repositoriumswurzel wurde nicht angefasst.
