# Diagnose: Hörbare Systemklänge beim Ausführen der GUI-Tests

Datum: 04.08.2026 · Diagnose-Auftrag des PO, keine Änderung am Projekt.
Kundenbefund: `librarytest` spielte im Messfenster 10:00–10:10 Uhr den
KDE-Klang `dialog-warning` 98-mal ab (PipeWire-Streams mit
`application.name = "librarytest"`); `libraryshots` um 10:04:25 „mit demselben
Muster“. Geprüfte Stände: kwidgetsaddons 6.28.0, knotifications 6.28.0,
plasma-integration 6.7.3 (`pacman -Q`).

Beobachtung und Schlussfolgerung sind in jedem Abschnitt getrennt.

---

## 1. Die auslösende Codestelle

### Beobachtung

Die einzige Stelle im Projekt, die einen `KMessageDialog` baut, ist der
Wächterdialog über ungespeicherte Änderungen:

- `src/ui/librarywindow.cpp:968` — `KMessageDialog dialog(KMessageDialog::WarningTwoActionsCancel, …)`
  in `LibraryWindow::askAboutUnsavedChanges()`, angezeigt über
  `dialog.show()` in `src/ui/librarywindow.cpp:1014` (und `exec()` in Zeile 1019).
- Kein weiterer `KMessageDialog`/`QMessageBox` in `src/` oder `tests/`
  (`grep` über `src/shell/`, `src/capture/`, alle Testdateien: leer).

Der Klang entsteht nicht in unserem Code, sondern in der Bauart selbst.
Beweiskette, Glied für Glied:

1. **`KMessageDialog::showEvent()` spielt bei jedem nicht-spontanen Anzeigen
   eine Benachrichtigung.** Quelle kwidgetsaddons v6.28.0,
   `src/kmessagedialog.cpp` (invent.kde.org, wörtlich):

   ```cpp
   void KMessageDialog::showEvent(QShowEvent *event)
   {
       if (d->m_notifyEnabled && !event->spontaneous()) {
           beep(d->m_type, d->m_messageLabel->text(), topLevelWidget());
       }
       QDialog::showEvent(event);
   }
   ```

   `m_notifyEnabled` ist standardmäßig wahr; es gibt eine öffentliche
   Abschalt-API `setNotifyEnabled(bool)` (Symbole
   `KMessageDialog::showEvent`, `KMessageDialog::beep`,
   `KMessageDialog::setNotifyEnabled` in `nm -D /usr/lib/libKF6WidgetsAddons.so.6`).

2. **`beep()` bildet den Dialogtyp auf ein QMessageBox-Icon ab und ruft die
   KMessageBox-Benachrichtigungsschnittstelle** (`KMessageBox::notifyInterface()
   ->sendNotification(…)`, gleiche Quelle). Unser Typ
   `WarningTwoActionsCancel` wird zu `QMessageBox::Warning`.

3. **Das FrameworkIntegrationPlugin übersetzt `QMessageBox::Warning` in das
   KNotification-Event `messageWarning`** und sendet es mit
   `KNotification::DefaultEvent` (Quelle frameworkintegration,
   `src/integrationplugin/frameworkintegrationplugin.cpp`; Plugin installiert
   als `/usr/lib/qt6/plugins/kf6/FrameworkIntegrationPlugin.so`, linkt
   `libKF6Notifications` → `libcanberra`, `ldd`-Beleg).

4. **`DefaultEvent` schlägt im Plasma-Arbeitsbereich nach:**
   `/usr/share/knotifications6/plasma_workspace.notifyrc`, Abschnitt
   `[Event/messageWarning]` (Zeile 1665), enthält `Sound=dialog-warning`
   (Zeile 1779). KNotifications spielt den Klang im eigenen Prozess über
   libcanberra ab — deshalb erscheint der Stream als
   `application.name = "librarytest"`, genau wie gemessen.

### Schlussfolgerung

**Ein Klang pro Anzeige des Wächterdialogs.** Auslösende Stelle im Projekt:
`src/ui/librarywindow.cpp:968–1014` (Bauart `KMessageDialog`, eingeführt mit
Commit `c3419c5` „Wächterdialog auf KDE-Bauart“ zu Issue #66, Sprint 5). Der
eigentliche Klangmechanismus liegt in KF6, nicht in unserem Code.

### Prüfung der mitgegebenen Hypothese

- **Bestätigt:** Die Umstellung auf die KDE-Bauart in Sprint 5 (#66) ist die
  Quelle. Der Klang ist eine entdeckte Bedingung dieser Umstellung, die
  niemand bemerkte, weil beim Testlauf niemand zuhörte. `QT_QPA_PLATFORM=
  offscreen` schaltet nur die Darstellung ab — der Kundenlauf beweist es.
- **Korrigiert:** Der Weg führt **nicht** über `QT_QPA_PLATFORMTHEME=kde` /
  plasma-integration. `KDEPlasmaPlatformTheme6.so` enthält keine
  canberra-Symbole und keinen `messageWarning`-String (`nm -D`, `strings`);
  libcanberra hängt dort nur transitiv an KIO. Das FrameworkIntegrationPlugin
  wird von KWidgetsAddons unabhängig vom Plattformthema geladen. Aus dem Code
  folgt: Der Klang käme auch ohne das KDE-Plattformthema. (Schlussfolgerung
  aus Quelltext, nicht empirisch gegengeprüft.)
- **Nicht belegbar ohne Lauf:** ob der frühere QMessageBox-Ersatzdialog (vor
  #66) ebenfalls klang. Ein einfacher `QMessageBox` ruft die
  KMessageBox-Schnittstelle nicht auf; was der Ersatzdialog der
  Plattformintegration intern tat, ist ungeprüft.

---

## 2. Sind die Dialoge im Test gewollt? (die wichtigere Frage)

### Beobachtung

Wächterdialog-Anzeigen pro **einem** vollen `librarytest`-Lauf, aus dem Code
gezählt:

| Test | Fundstelle | Anzeigen |
|---|---|---|
| `namesTheThreeAnswersOfTheGuardDialog` | `tests/librarytest.cpp:2771` | 1 |
| `showsTheWarningSymbolInTheGuardDialog` | `tests/librarytest.cpp:2832` | 1 |
| `keepsTheSelectionOnTheEditedNoteWhileTheDialogAsks` | `tests/librarytest.cpp:2947` | 1 |
| `namesTheEditedNoteWithoutBreakingTheSentence` | `tests/librarytest.cpp:2998,3002` (2 Datenzeilen) | 2 |
| `asksBeforeUnsavedChangesAreLost` | `tests/librarytest.cpp:3091–3096` (3 Auslöser × 3 Antworten) | 9 |
| **Summe pro Lauf** | | **14** |

Die beiden `DialogWatch`-Tests (`tests/librarytest.cpp:3183,3217`) prüfen
gerade, dass **kein** Dialog erscheint, und erzeugen keinen. `editshots`
öffnet den Dialog genau **1-mal** (`tests/editshots.cpp:314–382`).
`libraryshots` öffnet **keinen einzigen Dialog** (kompletter Läufer gelesen,
`tests/libraryshots.cpp`; auch das `KMessageWidget` der Löschmeldung ist
klanglos — KWidgetsAddons exportiert dafür keinerlei beep/notify-Symbol).

Zum Messwert 98: `98 = 7 × 14`. `build/Testing/Temporary/LastTest.log`
belegt einen Suitenlauf um 10:09; heute Vormittag arbeiteten mehrere Stränge
parallel am Projekt.

### Schlussfolgerung

**Die 14 Dialoge pro Lauf sind erwartetes, gewolltes Testverhalten** — die
3×3-Matrix aus Wireframe 2a Zustand C plus die Symbol-, Vorgabe- und
Textprüfungen. Sie verdecken keinen Fehler; jeder Dialog wird von genau
einem Test absichtlich herbeigeführt und beantwortet.

**Was ich nicht eindeutig klären konnte — ausdrücklich:**

1. **Die 98 sind mit einem Lauf nicht erklärbar.** Sieben Läufe im
   Messfenster würden exakt passen (7 × 14) und sind bei mehreren parallel
   arbeitenden Strängen plausibel — aber ich kann die Zahl der Läufe
   zwischen 10:00 und 10:10 nicht rekonstruieren; `LastTest.log` hält nur
   den letzten Lauf. Auch die 15 Klänge in 2 Sekunden (10:08:50–51)
   übersteigen die dichteste Stelle eines Einzellaufs (die 9er-Matrix);
   zwei überlappende Läufe würden es erklären. **Nicht belegt.**
2. **Das Klangmuster von `libraryshots` (10:04:25) erklärt der gefundene
   Mechanismus nicht** — der Läufer zeigt keinen Dialog. Erklärbar wäre
   `editshots` (1 Dialog pro Lauf), der im selben Übergabepaket läuft; eine
   Verwechslung der Läufer in der Messung ist möglich, ein zweiter,
   unentdeckter Auslöser aber nicht ausgeschlossen. **Offen.** Hier hilft
   nur eine Wiederholungsmessung je Läufer (Vorschlag unten).

---

## 3. Lösungswege mit Abwägung

Maßstäbe des Kunden: Test bleibt realitätsnah; echte Anwendung unverändert;
Test bleibt aussagekräftig.

### W1 — Audioausgabe der Testprozesse auf den canberra-Null-Treiber lenken (empfohlen)

`CANBERRA_DRIVER=null` in die `ENVIRONMENT`-Eigenschaft von `librarytest`
(`tests/CMakeLists.txt:52`) aufnehmen; in den von Hand laufenden Bildläufern
(`editshots`, ggf. alle Läufer) ein `qputenv("CANBERRA_DRIVER", "null")` am
Anfang von `main()`. Der Null-Treiber ist installiert
(`/usr/lib/libcanberra-0.30/libcanberra-null.so`), die Variable wird von
libcanberra ausgewertet (String `CANBERRA_DRIVER` in `libcanberra.so.0`).

- Realitätsnah: **ja.** Dialog, Buttons, Modalität, Fokus, sogar das
  KNotification-Event — alles läuft unverändert; nur die letzte Stufe
  (Audiogerät) wird zum Null-Treiber. Der Klang ist Systemverhalten und in
  keinem Test Prüfgegenstand.
- Echte Anwendung: **unverändert.** Reine Test-/Läufer-Umgebung.
- Aussagekraft: **unverändert**; kein Test misst Klang.
- Rest: Der Schalter ist canberra-spezifisch. Spielte künftig etwas anderes
  Töne ab, würde es wieder hörbar — im Sinne von „melden, nicht heilen“
  eher ein Vorteil als ein Risiko.

### W2 — `setNotifyEnabled(false)` im Produktivcode

Eine Zeile vor `dialog.show()` in `src/ui/librarywindow.cpp`.

- Echte Anwendung: **verändert** — genau das, was der Kunde nicht will. Der
  Klang beim Warndialog ist KDE-Plattformstandard (jeder KDE-Warndialog
  klingt, Lautstärke/Stummschaltung regeln die Systemeinstellungen des
  Nutzers). Ihn produktseitig abzuschalten wäre eine Produktentscheidung
  des Kunden, kein Testfix. **Nicht empfohlen** — es sei denn, der Kunde
  entscheidet ausdrücklich, dass der Dialog auch beim echten Nutzer stumm
  sein soll.

### W3 — Testseitiger Eventfilter, der `setNotifyEnabled(false)` vor dem Anzeigen setzt

Ein `qApp`-Eventfilter im Testcode fängt `QEvent::Show` auf
`KMessageDialog`-Instanzen ab und schaltet das Notify aus, bevor
`showEvent()` läuft (Anwendungsfilter laufen vor dem Widget-Handler).

- Echte Anwendung unverändert, Test realitätsnah bis auf das dann
  unterdrückte KNotification-Event.
- Aber: in jeder betroffenen Testdatei und jedem Läufer zu wiederholen,
  abhängig von der Filter-Reihenfolge, und das „läuft vor showEvent“ wäre
  erst noch zu messen. **Zerbrechlicher als W1, nachrangig.**

*(Systemklänge beim Testen abschalten scheidet aus — Leitplanke: das
Problem gehört ins Projekt, nicht ins System.)*

### Unabhängig vom gewählten Weg: SPEC-Nachtrag

Die Bedingung ist beim Bauen entdeckt worden und gehört nach DoD 4 (Fassung
B9) in die SPEC (Abschnitt 9, wo die KMessageDialog-Bauart festgelegt ist):
*Die KDE-Bauart spielt bei jedem Anzeigen das Systemklang-Event
`messageWarning` (Ocean: `dialog-warning`) ab; Standard eingeschaltet,
abschaltbar über `KMessageDialog::setNotifyEnabled()`. In Testläufen wird
die Audioausgabe über den canberra-Null-Treiber unterdrückt.* (Formulierung
Sache des PO.)

---

## 4. Vorgeschlagener Bestätigungslauf (nicht ausgeführt)

Zur Bestätigung von W1 und zur Klärung der offenen Punkte (Klangzahl pro
Lauf, `libraryshots`-Rätsel) böte sich an — **lautlos**, Dauer unter einer
Minute, erst nach Freigabe durch den PO:

```
CANBERRA_DRIVER=null ctest --test-dir build -R librarytest
```

parallel mit `pactl subscribe` beim Kunden: erscheinen trotz Null-Treiber
Streams, gibt es einen zweiten Auslöser; erscheinen keine, ist W1 belegt.
Ein zweiter Lauf ohne die Variable würde die 14 Klänge pro Lauf zählbar
machen — der wäre hörbar und braucht die ausdrückliche Zustimmung des
Kunden.

---

## 5. Nichts verändert

Dieser Bericht ist die einzige neue Datei. Produktivcode, Testcode,
Buildsystem, Systemeinstellungen: unberührt. Die vorgefundenen
uncommitteten Änderungen anderer Stränge wurden nicht angefasst.
