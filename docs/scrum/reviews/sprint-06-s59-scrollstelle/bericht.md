# Story #59 — Fensteraktivierung ohne Tageswechsel verliert die Scrollstelle

**Sprint 6, Strang B · Zweig `fix/59-scrollstelle` · 04.08.2026**

## 1. Nachmessung des Befundes — vor jeder Heilung

Der Befund stammte aus dem karpathy-Review und war ausdrücklich **nicht**
empirisch belegt. Erster Schritt war deshalb die Messung, nicht die Heilung.

**Aufbau:** 16 Notizen (8 von heute, 8 von gestern), Fenster 900 × 600,
Auswahl auf Notiz 12 — Zeile 14 der Liste, also **nicht** Zeile 0 —, dann
zurück an den Listenanfang gerollt, sodass die Auswahl außerhalb des Bildes
steht. Danach Fenster verlassen und wiederkommen, ohne dass ein Tag vergeht.

| Messung | Rollwert vorher | Rollwert nachher | Sprung |
|---|---|---|---|
| Unit-Test, offscreen (`librarytest`) | 0 | **7** | 7 Zeilen |
| Sichtprüfung unter Wayland (`probe59`) | 0 | **7** | 7 Zeilen |

In Pixeln gemessen (Versatz der ersten Listenzeile, Zwischenmessung während
der Entwicklung): **459 px** bei einem Viewport von **552 px** — der Leser
wurde um 83 % der sichtbaren Listenhöhe zurückgeworfen.

**Der Befund tritt auf.** Er ist damit von zwei unabhängigen Aufbauten
belegt, einer davon unter dem echten Compositor.

Belege: `befund-vor-der-heilung.txt`, `probe59-wayland-ungeheilt.txt`.

## 2. Heilung

`src/ui/librarywindow.cpp`, `changeEvent()`: Das Neugruppieren bei
Fensteraktivierung ist an die Bedingung geknüpft, dass der Kalendertag ein
anderer ist als beim letzten Aufbau der Liste.

Dazu merkt sich das Fenster in `m_groupedOn` (neu, `librarywindow.h`), für
welchen Kalendertag die Liste zuletzt gruppiert wurde; gesetzt an den beiden
Stellen, die die Gruppen bilden — `reload()` und `regroupList()`.

Der Kalendertag genügt als Bedingung, weil **alle vier Gruppengrenzen
Tagesgrenzen sind** (Heute · Gestern · Diese Woche · Letzte Woche · Älter,
SPEC 9). Es gibt keinen Übergang, der innerhalb eines Tages fällig würde.

**Nicht gebaut:** der Schwellwert-Weg („nur vorscrollen, wenn die Auswahl
nicht ohnehin sichtbar ist"). Er ist in Sprint 3 geprüft und nach eigener
Nachmessung zurückgenommen worden; er ist hier nicht angefasst worden.
Ebenso wenig #70 (Tageskopf der ersten Notiz einer Gruppe) und #57
(Sprung beim Klick) — beide liegen außerhalb dieser Story.

## 3. Akzeptanzkriterien

### AK 1 — Aktivierung ohne Tageswechsel lässt Scrollstelle und Auswahl unverändert, gemessen am Rollwert

**Erfüllt.** Gemessen wird `QScrollBar::value()` vor und nach der Aktivierung,
nicht das Endbild.

- Test `LibraryTest::staysPutWhenTheWindowIsActivatedWithoutADayChange`
  (`tests/librarytest.cpp:1835`): `QCOMPARE(list->verticalScrollBar()->value(), rolledTo)`
  und `QCOMPARE(list->currentIndex(), selected)`.
- Ohne die Heilung ist derselbe Test rot mit *Actual 7 / Expected 0*
  (`befund-vor-der-heilung.txt`) — der Test kann den Fehler also sehen.
- Sichtprüfung unter Wayland: Rollwert 0 → 0, Auswahl unverändert
  (`probe59-wayland-geheilt.txt`).

### AK 2 — Nach einem Tageswechsel wird weiterhin neu gruppiert; `regroupsWhenTheWindowIsActivated` bleibt gültig

**Erfüllt, Test unverändert.** Der bestehende Test ist nicht angefasst worden
und ist grün — sowohl im roten Lauf vor der Heilung (`PASS`) als auch danach.
Er prüft, dass „Heute" nach dem Tageswechsel zu „Gestern" wird.

### AK 3 — Der neue Test setzt die Auswahl nicht in Zeile 0

**Erfüllt.** Die Auswahl steht auf Notiz 12, das ist **Listenzeile 14**
(zwei Gruppenköpfe liegen darüber). Der Test sichert das zusätzlich ab:

- `QVERIFY(selected.row() > 0)` — die Auswahl liegt nicht in Zeile 0.
- `QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(selected)), …)`
  — die Auswahl liegt außerhalb des Bildes, sonst gäbe es nichts
  zurückzuspringen. Der Aufbau, in dem der Fehler nicht auftreten *kann*, ist
  damit ausgeschlossen.

Der Wayland-Läufer druckt beides mit aus: „Auswahl in Zeile 14", „Auswahl im
Bild: nein".

## 4. Voller Testlauf

`ctest --test-dir build` — **100 % tests passed out of 7**, vollständige
Ausgabe in `ctest.txt`. Bau warnungsfrei. `lint-tidy` über den gesamten Baum:
die drei Befunde zu `librarywindow.{h,cpp}` (`bugprone-easily-swappable-parameters`
in `placeholderPage`, zwei `performance-enum-size`) sind Bestand und liegen
nicht in den geänderten Zeilen.

## 5. Sichtprüfung am gebauten Stand (DoD 2)

Nach `/usr` wurde **nicht** installiert — der PO taktet die Installation, ein
zweiter Strang arbeitet parallel.

Der laufende installierte Daemon (`/usr/bin/denkzetteld`) hält den
eindeutigen D-Bus-Namen `org.denkzettel.Daemon`. Ein Start des Debug-Builds
hätte über `KDBusService::Unique` nur diesen aktiviert — also den falschen
Stand geprüft (genau die Falle aus Sprint-3-Mangel M1) — und dabei auf der
Notizdatenbank des Kunden gearbeitet.

Stattdessen ein eigenständiger Prüfläufer `probe59.cpp` (in diesem Ordner,
versioniert), der gegen die frisch gebauten Bibliotheken `libdenkzettelui.a`
und `libdenkzettelstore.a` gebunden wird, eine eigene temporäre Datenbank
anlegt und **unter der echten Wayland-Plattform** ein echtes
Bibliotheksfenster auf den Bildschirm bringt.

Übersetzen (die Flags stammen aus `build/tests/CMakeFiles/librarytest.dir/`,
kein `CMakeLists.txt` wurde angefasst):

```
c++ -fno-exceptions -g -std=c++20 -fvisibility=hidden -fvisibility-inlines-hidden \
    -mno-direct-extern-access -DQT_NO_KEYWORDS -DQT_NO_CAST_FROM_ASCII \
    -DQT_NO_CAST_TO_ASCII -DQT_USE_QSTRINGBUILDER -I src -I build/src \
    -isystem /usr/include/qt6/QtCore -isystem /usr/include/qt6 \
    -isystem /usr/lib/qt6/mkspecs/linux-g++ -isystem /usr/include/qt6/QtSql \
    -isystem /usr/include/qt6/QtWidgets -isystem /usr/include/qt6/QtGui \
    -isystem /usr/include/KF6/KConfig -isystem /usr/include/KF6/KConfigCore \
    -isystem /usr/include/KF6/KConfigGui -isystem /usr/include/KF6/KI18n \
    -isystem /usr/include/KF6/KWidgetsAddons -isystem /usr/include/KF6/KWindowSystem \
    docs/scrum/reviews/sprint-06-s59-scrollstelle/probe59.cpp -o /tmp/probe59 \
    build/lib/libdenkzettelui.a build/lib/libdenkzettelstore.a \
    /usr/lib/libQt6Sql.so.6 /usr/lib/libKF6ConfigGui.so.6 /usr/lib/libKF6ConfigCore.so.6 \
    /usr/lib/libKF6I18n.so.6 /usr/lib/libKF6WidgetsAddons.so.6 /usr/lib/libQt6Widgets.so.6 \
    /usr/lib/libKF6WindowSystem.so.6 /usr/lib/libQt6Gui.so.6 /usr/lib/libQt6Core.so.6
```

Starten: `XDG_CONFIG_HOME=<eigener Ordner> QT_QPA_PLATFORMTHEME=kde /tmp/probe59`
(eigenes `XDG_CONFIG_HOME`, damit die Fenstergeometrie des Kunden unberührt
bleibt).

**Entdeckung dabei, für künftige Sichtläufe wichtig:** `activateWindow()`
holt unter Wayland den Fokus **nicht** zurück — ein Prozess kann sich den
Fokus nicht selbst zuteilen, dazu bräuchte er ein xdg-activation-Token. Der
erste Lauf meldete deshalb „zurückgekommen: nein" und maß nichts. Der Weg,
der wirklich funktioniert, ist der compositor-getriebene: das obenauf
liegende Fenster schließen, dann gibt der Compositor den Fokus von sich aus
zurück — derselbe Weg, den ein Alt-Tab nimmt. Damit kam der
`ActivationChange` an, belegt durch die Gegenprobe (ungeheilt: Rollwert
springt auf 7).

## 6. SPEC.md (DoD 4)

**Nachgezogen.** `SPEC.md`, Abschnitt 9, direkt nach der Beschreibung der
Tagesgruppen: Die Gruppen werden beim Aufbau und bei jeder Fensteraktivierung
nachgerechnet, **neu gruppiert wird aber nur bei einem anderen Kalendertag**
— mit der Begründung (Modell-Reset zieht die Liste auf die Auswahl, 459 px
gemessen) und dem Grund, warum der Kalendertag als Bedingung genügt (alle
vier Gruppengrenzen sind Tagesgrenzen).

Das ist eine **entdeckte Bedingung** im Sinne von DoD 4 in der Fassung nach
B9: Die bisherige Festlegung „bei Aktivierung wird nachgerechnet" gilt nur
unter dieser Einschränkung, sonst arbeitet sie gegen den Leser.

`KONZEPT.md` und die Wireframes sind nicht angefasst — Wireframe 3b sagt
nichts über die Häufigkeit des Neugruppierens, nur dass es keinen
Mitternachtszeitgeber gibt, und das gilt unverändert.

## 7. Außerhalb meiner Fläche aufgefallen — melden, nicht heilen

1. **Der Debug-Build ist als Anwendung nicht startbar, solange der
   installierte Daemon läuft.** `KDBusService::Unique` gibt den Start an die
   laufende Instanz weiter. Das betrifft jede Story, deren Sichtprüfung die
   Anwendung selbst braucht, nicht nur diese. Ein Prüfläufer gegen die
   Bibliotheken ist der Ausweg, aber er ist von Hand zu bauen.
2. **`activateWindow()` unter Wayland** (siehe 5.) — das ist eine Bedingung
   für jede künftige Sicht- oder Bildprüfung mit Fensterwechsel und gehört
   nach Einschätzung dieses Strangs in die Prüfhaltung von `CLAUDE.md` oder
   in `PROZESS.md`. Beide liegen außerhalb meiner Dateimenge; Entscheidung
   und Eintrag sind Sache von PO und Scrum Master.
3. **clang-tidy-Bestandswarnungen** in `note.h`, `timestampformat.h`,
   `notelistmodel.h`, `trayicon.cpp`, `globalshortcuts.cpp`,
   `notelistdelegate.cpp` und `librarywindow.{h,cpp}` — nicht von dieser
   Story verursacht, nicht angefasst. Auffällig darunter:
   `globalshortcuts.cpp:93` — `bugprone-unused-return-value`.

## 8. Was ich nicht belegen konnte

- **Kein Alt-Tab durch eine menschliche Hand.** Die Rückkehr in das Fenster
  wurde durch das Schließen des davorliegenden Fensters ausgelöst, nicht
  durch einen Tastendruck. Beides läuft über denselben
  `QEvent::ActivationChange` des Compositors, aber der Tastenweg ist von
  einem Agenten unter Wayland nicht auslösbar. Das schließt der PO mit der
  Installation am Sprint-Ende (DoD 2, Ersatzpflicht nach Sprint-3-Mangel M1).
- **Kein Lauf am installierten Stand** — untersagt, solange ein zweiter
  Strang arbeitet.
- **Keine Bildbelege** — vom PO für #59 ausdrücklich nicht verlangt: Der
  Prüfgegenstand ist eine Bewegung, und dafür ist der Rollwert das richtige
  Mittel (B3).
- **DoD 5 und DoD 6** (Issue schließen, Journal) liegen in Takt 2 nach der
  Kundenabnahme und sind nicht Sache dieses Strangs.
