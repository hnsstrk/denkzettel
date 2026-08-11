# Denkzettel — Spezifikation

Stand: 2026-07-31 — abgeleitet aus KONZEPT.md (drei Design-Interviews, alle
Grundsatzfragen entschieden) und den Wireframes (`wireframes/`). Diese Spec ist
die Bau-Grundlage; wo sie das Konzept präzisiert oder davon abweicht, ist das
ausdrücklich als solches markiert.

## 1. Ziel

Denkzettel ist ein Quick-Capture-Werkzeug für KDE Plasma (Wayland): Ein globales
Kürzel öffnet ein zentriertes Eingabefenster, ein Gedanke wird getippt oder
gesprochen und ist gespeichert — null Zeremonie. Ein Hintergrund-Dienst
klassifiziert per KI, vergibt Tags, baut einen Suchindex und legt dem Nutzer
Vorschläge zur Bestätigung vor: thematische Bündel für den Obsidian-Vault und
Tasks für Taskwarrior. Denkzettel ist Durchlauf-Speicher — der Klartext-Ort
bleibt Obsidian, das Task-System bleibt Taskwarrior.

Leitplanken (aus dem Konzept, unverhandelbar):

- Der Capture-Weg wird niemals mit UI belastet — Geschwindigkeit ist der Kern.
- Überführungen passieren nur nach Bestätigung („Melden, nicht heilen").
- Die App darf nicht volllaufen; ein Voll-Export-Rettungsweg existiert immer.

## 2. Architektur

### 2.1 Prozessmodell — Präzisierung gegenüber dem Konzept

Das Konzept nennt „zwei Komponenten" (Capture-Fenster, Hintergrund-Dienst).
Diese Spec präzisiert: **ein Prozess zur Laufzeit** (`denkzetteld`), in dem
Capture-/Aufnahmefenster als vorinstanziierte, versteckte Fenster leben.

Begründung: Der Dienst läuft ohnehin dauerhaft (Tray, Shortcuts, Analyse).
Ein vorgehaltenes Fenster erscheint auf Shortcut-Druck ohne Prozessstart —
schneller geht es nicht, und Geschwindigkeit ist Kernziel Nr. 1. Die Trennung
Capture ↔ Dienst bleibt als **Modulgrenze im Code** erhalten (eigenes
Verzeichnis, keine Abhängigkeit des Capture-Moduls auf Analyse-Code).

Die im dritten Interview entschiedene **D-Bus-Kopplung** bleibt real: Der
Prozess exportiert `org.denkzettel.Daemon` als externe Schnittstelle (siehe
2.3) — nutzbar von CLI, Skripten und für Einzelinstanz-Erzwingung.

### 2.2 Module

| Modul | Aufgabe |
|---|---|
| `capture` | Text-Capture-Fenster, Aufnahmefenster (Sprachnotiz) |
| `store` | SQLite-Zugriff, Datenmodell, FTS-Index, Audio-Dateiverwaltung |
| `analysis` | KI-Pipeline: Klassifikation, Tags, Embeddings, Clustering, Task-Extraktion |
| `transcribe` | Whisper-Backends (whisper.cpp, WhisperX) als Subprozesse, Job-Queue |
| `proposals` | Vorschlags-Erzeugung und -Ausführung (Obsidian-Export, Taskwarrior) |
| `ui` | Bibliothek, Vorschlags-Review, Einstellungen |
| `shell` | Tray (KStatusNotifierItem), KGlobalAccel-Registrierung, KNotifications, D-Bus-Adaptor |

### 2.3 D-Bus-Schnittstelle `org.denkzettel.Daemon`

| Methode | Wirkung |
|---|---|
| `ShowCapture()` | Capture-Fenster zeigen (auch Ziel der KGlobalAccel-Aktion) |
| `ShowRecorder()` | Aufnahmefenster zeigen, Aufnahme startet sofort |
| `AddNote(text) → id` | Notiz ohne UI anlegen (CLI/Skripte) |
| `AnalyzeNow()` | Analyse-Lauf anstoßen |
| `ShowLibrary()`, `ShowProposals()` | Fenster öffnen |
| `Quit()` | Beenden |

Ein zweiter Prozessstart erkennt die belegte D-Bus-Registrierung und ruft
stattdessen `ShowCapture()` (Einzelinstanz).

- **Der Busname hängt an der Organisationsdomäne (entdeckte Bedingung, Befund
  04.08.2026, Issue #61):** KDBusService setzt ihn aus der umgedrehten Domäne
  und dem Anwendungsnamen zusammen. `KAboutData::setApplicationData()`
  überschreibt beide Felder mit seinen eigenen Vorgaben — die Domäne mit
  `kde.org` —, und der Dienst meldet sich dann als `org.kde.Daemon` an.
  Gemessen am tatsächlich angemeldeten Namen, nicht aus der Kopfdatei
  abgeleitet. **Deshalb setzt Denkzettel Domäne und Desktop-Dateiname am
  `KAboutData`-Objekt, bevor es registriert wird, und an keiner zweiten
  Stelle** — zwei Setzer entschieden die Frage nach Zeilenreihenfolge. Kein
  Rückgabewert meldet den Bruch.
- **Die Argumente werden vor der Einzelinstanz-Weiche ausgewertet (entdeckte
  Bedingung, Befund 04.08.2026):** Läuft der Dienst schon, reicht die Weiche
  einen zweiten Start an ihn weiter; eine dahinter liegende Auswertung von
  `--version` öffnete dann ein Erfassungsfenster und gäbe **0** zurück, ohne
  etwas auszugeben. Ohne erreichbaren Sitzungsbus beendet KDBusService den
  Prozess zudem mit 1. Beides macht die Reihenfolge zur Bedingung, nicht zur
  Geschmacksfrage.

### 2.4 Globale Kürzel

- `Meta+N` → `ShowCapture()` · `Meta+Umschalt+N` → `ShowRecorder()`
- Registrierung über **KGlobalAccel** (KF6); die Kürzel erscheinen in den
  Plasma-Systemeinstellungen und sind dort wie in den App-Einstellungen änderbar.
- Belegung auf dem Entwicklungsrechner geprüft (31.07.2026, inkl. Mehrfachbelegungen): beide frei.
- **Konflikterkennung (T1-Befund):** Eine KGlobalAccel-Registrierung kann
  unsichtbar fehlschlagen — der Eintrag entsteht und `invokeShortcut`
  funktioniert, aber der echte Tastendruck geht weiter an den bestehenden
  Besitzer. Beim Erststart und bei Kürzel-Änderung prüft Denkzettel die
  Sequenz gegen die bestehende Belegung (inkl. Mehrfachbelegungen) und
  meldet Konflikte sichtbar statt still zu scheitern.
- **Auslösung über Desktop Actions (entdeckte Bedingung, Befund 01.08.2026):**
  Bei installierter Anwendung endet der Komponentenname auf `.desktop`; damit
  behandelt kglobalacceld die Komponente als *Service-Action-Komponente* und
  startet beim Tastendruck die gleichnamige **Desktop Action** der
  `.desktop`-Datei (ApplicationLauncherJob), statt ein D-Bus-Signal an den
  laufenden Prozess zu senden. Fehlt die Gruppe, protokolliert der Dienst einen
  Fehler und bricht ab — die Registrierung liegt vor, `isActive` ist wahr, und
  der Tastendruck verpufft. Deshalb gilt: **Je Kürzel deklariert die
  Desktop-Datei eine Gruppe `[Desktop Action <Aktions-Id>]` mit eigener
  `Exec`-Zeile, und die Id steht in `Actions=`.** Die Aktions-Id ist zugleich
  der `objectName` der `QAction` und muss ein gültiger XDG-Bezeichner sein
  (Buchstaben, Ziffern, Bindestrich — kein Unterstrich; `desktop-file-validate`
  weist ihn sonst zurück). Die `Exec`-Zeile startet `denkzetteld`; die
  Einzelinstanz-Weiche aus 2.3 macht daraus den Aufruf des Fensters.
- **Rücklesen der Registrierung (entdeckte Bedingung, Befund 01.08.2026;
  Retro-Beschluss B5):** `KGlobalAccel::setGlobalShortcut()` kann einen
  Fehlschlag des Dienstes nicht melden — der Aufruf setzt seine D-Bus-Nachricht
  ab, ohne die Antwort zu lesen, und liefert auch dann `true`, wenn
  kglobalacceld nichts behalten hat. Deshalb fragt Denkzettel nach jeder
  Registrierung beim Dienst nach, welche Sequenz er für die Aktion hält, und
  prüft zugleich, ob die Desktop-Datei die zugehörige Gruppe deklariert. Bleibt
  eines von beiden aus, meldet Denkzettel das sichtbar — **bei jedem Start, nicht
  nur beim ersten**: Anders als ein Konflikt lässt dieser Fehlschlag gar kein
  wirkendes Kürzel zurück. Die Meldung nennt das betroffene Kürzel und einen
  ausführbaren Schritt. **Das gilt je Kürzel:** `Meta+Umschalt+N` durchläuft
  dieselbe Prüfung wie `Meta+N` — ohne sie wiederholt es dessen Fehlschlag, und
  „in den Systemeinstellungen sichtbar“ ist gerade der Zustand, den ein still
  gescheitertes Kürzel erzeugt.
- **Der Komponentenname hängt am Desktop-Dateinamen (entdeckte Bedingung,
  Befund 04.08.2026, Issue #61):** Denkzettel liest
  `QGuiApplication::desktopFileName()` und hängt `.desktop` an — daran hängen
  Komponentenname und, unter Wayland, die Anwendungs-Id.
  `KAboutData::setApplicationData()` überschreibt die Eigenschaft mit seiner
  Vorgabe `org.kde.<Komponentenname>`; die Kürzel liefen danach unter einer
  Komponente, zu der keine installierte Desktop-Datei gehört. Deshalb setzt
  Denkzettel den Namen am `KAboutData`-Objekt vor der Registrierung (siehe 2.3).
- **Die Anwendungs-Id ist kein Kommandozeilenwert (Festlegung 05.08.2026):**
  `KAboutData::setupCommandLine()` brächte die Option `--desktopfile
  <Dateiname>` mit, die genau diesen Wert zur Laufzeit überschreibt. Denkzettel
  meldet sie nicht an; sie fällt damit unter die zurückgewiesenen Schalter
  (Abschnitt 15).

### 2.5 Autostart und Erststart (Ergänzung aus der Schätzklausur)

- `denkzetteld` startet über einen **XDG-Autostart-Eintrag** mit der
  Plasma-Sitzung (das Paket installiert die .desktop-Datei; Deaktivierung
  über die Plasma-Systemeinstellungen). Ohne laufenden Dienst gäbe es keine
  Kürzel — Autostart ist Grundfunktion, keine Politur.
- **Erststart**: legt Datenverzeichnis, DB (aktuelle Schemaversion) und
  Default-Konfiguration an; erkennt die optionalen Werkzeuge (ffmpeg,
  whisper-cli, task, Ollama-Erreichbarkeit) und zeigt Fehlendes in den
  Einstellungen an (vgl. Abschnitt 15).
- **Aufräum-Kontrolle**: Beim Dienststart werden Audio-Dateien ohne
  DB-Verweis (abgebrochene Aufnahmen, unterbrochene Löschungen) entfernt
  und im Log vermerkt — eindeutiger, harmloser, wiederkehrender Fall,
  daher zulässige Selbstheilung im Sinne der Loop-Konventionen.

## 3. Capture-Fenster (Text)

- Rahmenloses Fenster, sofortiger Fokus, immer im Vordergrund.
- **Fokus-Mechanik (T1-Befund, Issue #1):** Vor jedem Zeigen wird das Fenster
  neu gemappt — `hide()` zerstört die Wayland-Surface, `show()` erzeugt ein
  frisches Toplevel, das vom Compositor regulär den Fokus erhält. Der
  XDG-Activation-Token-Weg trägt nachweislich nicht (KGlobalAccel liefert
  kein Token, Zeitstempel immer 0) und wird nicht gebaut.
- **Zentrierung (PO-Entscheidung nach T1):** KWin-Standardplatzierung —
  Plasma 6.7 zentriert standardmäßig, auf dem Entwicklungsrechner verifiziert. Ein
  Wayland-Client kann sich nicht selbst positionieren; weicht die
  Platzierungsrichtlinie des Nutzers ab, ist Layer-Shell (Overlay,
  `AnchorNone`, `KeyboardInteractivityOnDemand`) der gemessene Rückfallweg —
  dokumentiert, in v1 nicht gebaut.
- Inhalt: App-Name klein, mehrzeiliges Textfeld (Platzhalter „Gedanke
  festhalten …"), Fußzeile „Esc verwirft · Strg+Enter speichert".
- **Mitwachsend**: Starthöhe ~5 Zeilen (Sprint-1-Abnahme: 3 waren dem Kunden
  zu wenig), wächst mit dem Text bis ~8 Zeilen, danach Scrollbalken.
- Strg+Enter: Notiz speichern (`store`), Fenster verstecken, Feld leeren.
  Esc: verwerfen, Fenster verstecken. Fokusverlust: Fenster bleibt (kein
  Datenverlust durch versehentlichen Klick daneben).
- **Die Höhe wird auch dann neu gerechnet, wenn sich die Schrift ändert** —
  nicht nur, wenn sich der Text ändert (Issue #56). Die Zeilenzahl bleibt bei
  einem Schriftwechsel gleich, der Zeilenabstand nicht; ohne diese Regel fällt
  ein stehendes Fenster von fünf auf drei Zeilen zurück, also auf genau den
  Zustand, den die Sprint-1-Abnahme zurückgewiesen hat.
- Kein Button, kein Menü, keine Formatierung.

### 3.1 Hülle aus dem Desktop-Theme (Issue #55, Wireframe 4a/4b)

Das rahmenlose Fenster trägt **Rundung, Rand, Deckung und Schatten seines
Desktop-Themes**, gezeichnet aus `dialogs/background` — derselben Grafik, aus
der Plasma seine Popups und KRunner baut. Bibliothek und Dialoge sind davon
nicht berührt: Sie sind dekorierte Fenster und bekommen ihre Hülle von KWin.

**Zur Hülle gehören drei Anmeldungen beim Fenstersystem, nicht eine.** Am
Binärcode von `libPlasmaQuick` gemessen meldet Plasma für seine eigenen
Überlagerungen `KWindowShadow`, `KWindowEffects::enableBlurBehind` und
`KWindowEffects::enableBackgroundContrast` an. Denkzettel macht dieselben drei;
die Bedingungen dazu stehen in 3.2.

**Form und Farbe kommen vom Theme** (Kundenentscheidung 04.08.2026, Issue #83:
„dann eine native Plasma-Überlagerung ohne Anpassungen"). Gezeichnet wird die
Grafik des Themes selbst, **in einem Stück** (`FrameSvg::framePixmap()` beim
Bildpunktverhältnis des Fensters) — keine Alphamaske, keine eigene Füllfarbe,
keine selbst gezogene Kontur.

Bis Sprint 6 war es umgekehrt: Gezeichnet wurde die Alphamaske des Themes,
gefüllt mit Palettenfarben, samt einer Konturlinie aus `Window` und
`WindowText` im Verhältnis 0,20. Der Kunde hat diesen Nachbau abgewählt. Die
Kehrseite ist benannt und angenommen: Von den acht auf der Kundenmaschine
installierten Themes richtet nur `default` seine Füllfarbe am Farbschema aus,
also sind sechs der acht danach schlechter lesbar als zuvor. Auf der
Kundeneinstellung ändert sich nichts, weil dort mangels `[Theme] name` in
`plasmarc` der Rückfall `default` greift.

Issue #85 hat das für die vier Themes mit eigener `colors`-Datei geheilt (siehe
den Spiegelstrich zur Schrift unten) und die Grenze für die übrigen benannt:
Unter den drei `cachyos-emerald`-Themes deckt die Hülle zu 3,5 %, und was den
Text dort tragen soll, ist der Kontrasteffekt des Compositors — der auf diesem
Stand **nicht vorhanden** ist (3.2, Punkt 10). Dort sichert die Spezifikation
keine Lesbarkeit zu.

- **Zwei Flächen, beide aus dem Theme** (Kundenentscheidung 06.08.2026,
  Issue #100). Die **Fensterfläche** zeichnet `dialogs/background`, das
  **Textfeld** `widgets/lineedit` mit dem Vorsatz `base` — dieselbe Quelle,
  eine Grafik tiefer, und dieselbe, aus der KRunners Eingabefeld gezeichnet
  ist. Keine der beiden deckt notwendig: `default` deckt in der Hülle zu
  84,7 %, andere Themes zu 2,7 % bis 100 %. „Geschlossen" heißt hier
  vollständig, nicht undurchsichtig. **Zugesichert ist die Herkunft, nicht
  eine Farbe und nicht eine Kontrastzahl.**
  **Grenze, gemessen und benannt** (07.08.2026, Issue #100 AK 6b): Unter
  `CachyOS-Nord-round`, `Iridescent-round` und den drei
  `cachyos-emerald`-Themes zeichnet `widgets/lineedit` nur einen Hauch — die
  Grafik deckt dort zu **15 von 255**, gegen 255 unter `default`,
  `breeze-dark` und `breeze-light`. Dort bleibt das Feld praktisch
  unsichtbar, und daran ist nichts zu heilen, ohne „Form und Farbe kommen vom
  Theme" (#83) aufzugeben. Es ist dieselbe Grenze, die der Absatz darüber für
  die Schrift benennt; sie wächst hier nicht, sie wiederholt sich.
  **Geprüft wird die Deckung und keine Kontrastzahl**, und das ist gemessen
  begründet: Die Abhebung des Feldes gegen die Hülle schwankt unter `default`
  zwischen 1,08 : 1 über schwarzem und 1,88 : 1 über weißem Grund, weil die
  Hülle durchscheint — sie hängt am Bildschirmhintergrund des Kunden und ist
  keine Eigenschaft des Baus.
  **Bis zum 06.08.2026 galt das Gegenteil:** „eine durchgehende Fläche — kein
  Kasten im Kasten". Der Kunde hat sie auf seinen Befund vom 05.08.2026 hin
  aufgehoben („Das Erfassungsfenster ist ein Farbblock. Der Eingabebereich ist
  nicht klar erkennbar"). Von ihrer Begründung trägt die eine Hälfte weiter:
  Dass **keine Palettenrolle** einen zweiten Kasten konturieren kann, ist über
  18 Schemata gemessen und bleibt richtig — sie schließt Palettenrollen aus,
  nicht die Theme-Grafik. Die andere ist widerlegt: Dass die KDE HIG einen
  solchen Kasten **ablehnten**, ist falsch — *Getting input*, Abschnitt
  *Signaling interactivity*, verlangt das Gegenteil („Use standard controls as
  much as possible to automatically inherit this style of visual
  interactivity"). Belege und Vorlage:
  `docs/scrum/reviews/2026-08-06-lesbarkeit/`, Umsetzung
  `docs/scrum/reviews/sprint-09-s100-eingabefeld/`.
- **Kontur** ist keine eigene Linie mehr. Die Theme-Grafik zeichnet an ihrem
  Rand dieselbe Farbe wie in der Fläche und unterscheidet sich allein in der
  Deckung (gemessen unter `default`: 235 gegen 216 von 255). Der Rand des
  Themes ist ein **Deckungsrand**; sichtbar wird er nur, weil die Hülle
  durchscheint.
- **Die Schrift kommt aus derselben Quelle wie die Fläche** (Kundenentscheidung
  04.08.2026, Issue #85), und das gilt für **beide** Textklassen. Bringt das
  Desktop-Theme eine eigene `colors`-Datei mit, gelten deren Farben:
  `ForegroundNormal` für den Notiztext, `ForegroundInactive` für die gedämpfte
  Klasse. Bringt es keine mit, gilt das Farbschema — dann **Notiztext**
  `WindowText`, **nicht** die Rolle für Eingabefelder (über 18 Schemata
  schlechtestens 4,74:1 gegen 4,22:1, also über beziehungsweise unter dem
  Mindestwert von 4,5:1), und **App-Name und Fußzeile** `PlaceholderText`.
  Von den acht auf der Kundenmaschine installierten Themes bringen vier eine
  eigene Datei mit und vier nicht (gemessen).
- **Zur gedämpften Klasse gehört auch der Platzhaltertext** des leeren
  Eingabefeldes — drei Stellen, nicht zwei.
- **Der Textcursor folgt der Schrift, nicht dem Farbschema** (entdeckte
  Bedingung, 05.08.2026, DoD 4/B9). Qt zeichnet ihn in der Textfarbe des
  Eingabefeldes; sobald diese aus dem Theme kommt, tut er es mit. **Das ist
  gewollt und darf nicht „zurückgeheilt" werden:** Unter `breeze-light` mit
  dunklem Schema steht er im Bild bei 91,91,90 auf einer Fläche von
  242,242,243 — die Schemafarbe wäre 252,252,252 und damit **unsichtbar**.
  Die Zeichnung sagte bis dahin, Auswahl, Cursor und Scrollbalken kämen
  unverändert aus der Palette; für den Cursor stimmt das seit #85 nicht mehr,
  für die **Auswahl** weiterhin schon.
- **Zugesichert ist die Herkunft der Farbe, nicht ihre Kontrastzahl.** Eine
  Kontrastzahl gilt für ein Farbschema, einen Auswahlpfad und einen benannten
  Grund; keines der drei gehört dem Code. Was sich prüfen lässt, ist, aus
  welcher Quelle die Farbe stammt.
- **Innenabstände** (12 seitlich, 10 oben, 8 unten) gelten **zuzüglich** des
  Randes, den das Theme für sich beansprucht — der Inhalt beginnt bei Breeze
  16 px vom Fensterrand, bei einem 8-px-Theme 20 px. Über der Fußzeile steht
  mehr Luft (12) als unter dem App-Namen (8); seit dem Entfall der Trennlinie
  ist dieser Unterschied die gesamte Gliederung.
  **Für den Notiztext kommt seit Issue #100 der Rand des Feldes hinzu** — der
  Streifen, den `widgets/lineedit` für sich beansprucht (gemessen 6 px
  ringsum unter allen acht installierten Themes, die darin nicht auseinander
  gehen). Der Text rückt entsprechend nach innen und der Textbereich wächst um
  zweimal diesen Rand; **App-Name und Fußzeile stehen unverändert**, denn das
  Feld umgibt allein den Textbereich. Zugesichert wird auch hier relativ:
  gegen den Rand, den die Grafik meldet, nicht gegen die Zahl 6 — vier der
  acht Themes melden 5,99999.
  **Der Halbsatz nach dem Semikolon beschreibt den Zustand, den Issue #100
  beanstandet** (05.08.2026) — die Maßangaben davor gelten unverändert. Der
  Entfall der Trennlinie war am 01.08.2026 mit drei Gründen belegt; der dritte
  berief sich auf eine HIG-Aussage, die es nicht gibt, und ist am 06.08.2026
  zurückgezogen (Wireframe 4b). Die beiden anderen tragen weiter, und der
  erste ist zugleich enger als er klang: Er gilt für **Palettenrollen**, nicht
  für jede Farbe. Die Kirigami-Mischung aus Grund und Textfarbe im Verhältnis
  `frameContrast` liegt über 18 Schemata zwischen 1,24 : 1 und 1,93 : 1.
  *Gemessen nachgetragen am 07.08.2026:* Diese Spanne entsteht aus den **Farben**
  der Schemata. `frameContrast` selbst ist über alle 19 geprüften Schemata
  konstant — `KColorScheme::frameContrast()` liest die Gruppe `[KDE]` der
  Anwendungskonfiguration, kein Schema trägt den Schlüssel, überall gilt die
  Voreinstellung 0,20. Der Satz oben ließ sich lesen, als variiere der Wert je
  Schema.

**Rundung und Rand sind keine Zahlen dieser Spezifikation.** Sie gehören dem
Theme. Zugesichert wird **relativ**: Bei zwei Desktop-Themes mit
unterschiedlichem Rand unterscheiden sich Randmaß und Eckform entsprechend.
Die Eckform kommt aus den Eckstücken des Themes und ist **nicht** aus dem
Randmaß abzuleiten — zwei Themes können bei gleichem Rand verschieden gekrümmt
sein.

### 3.2 Bedingungen der Hülle (entdeckt beim Bau, DoD 4/B9)

Sie stehen hier, weil eine Umsetzung ohne sie eine Hülle zeichnet, die richtig
aussieht und falsch ist — belegt in `docs/scrum/reviews/sprint-06-s55-huelle/`
und `docs/scrum/reviews/sprint-07-s83-native-huelle/`, je samt `pruefen.sh`.
Die meisten widersprechen dem, was der Aufbau nahelegt, und **keine einzige
meldet ihren Fehlschlag über einen Rückgabewert.**

1. **KSvg findet das Desktop-Theme nicht selbst.** Der eingestellte Name steht
   in `plasmarc` unter `[Theme] name`; ohne Übergabe bleibt `KSvg::ImageSet`
   auf `default`, was die Datei auch sagt. Die Anwendung liest ihn selbst.
2. **Ein `FrameSvg` folgt nur einem frischen `ImageSet`.** Das Set umzubenennen
   wirkt nicht, den Bildpfad erneut zu setzen wirkt nicht, dasselbe Set erneut
   zuzuweisen wirkt nicht. Bei jedem Theme-Wechsel entsteht ein neues Set; das
   alte fällt erst weg, wenn alle Rahmen auf das neue zeigen.
3. **Der Theme-Wechsel wird über `KDirWatch` auf `plasmarc` zugestellt, nicht
   über `KConfigWatcher`.** Letzterer meldet nur, wenn der *Schreiber*
   `KConfig::Notify` benutzt hat — die Zusicherung hinge damit an der
   Disziplin eines fremden Programms. KConfig ersetzt die Datei beim Schreiben,
   weshalb ein `QFileSystemWatcher` seine Beobachtung dabei verlöre.
4. **Ohne Desktop-Theme keine Hülle, aber ein brauchbares Fenster.** Außerhalb
   einer Plasma-Sitzung fehlt `dialogs/background`. Das Fenster bleibt dann
   deckend und bedienbar — kein Absturz, keine durchsichtige Fläche. Ein
   *unbekannter Theme-Name* erzeugt diesen Zustand **nicht**: KSvg fällt dabei
   auf `default` zurück.
5. **Der Schatten wird nach jedem Neuzeigen neu gebunden.** Vor jedem Zeigen
   wird das Fenster neu gemappt (oben), und die Wayland-Surface verschwindet
   dabei; ein einmal im Konstruktor gebundener Schatten wäre nach dem ersten
   Verstecken weg. Die Bindung gehört deshalb hinter `show()`.
6. **Der Weichzeichner wirkt nur, wenn er unmittelbar nach `show()` angemeldet
   wird.** Über sieben A/B-Läufe gemessen: eine Sekunde später angemeldet
   bleibt der Grund scharf — mit Maskenregion wie mit leerer Region, ein- wie
   zweimal gerufen. `enableBlurBehind()` ist `void`, es gibt also keinen
   Rückgabewert, der den Fehlschlag meldete; sichtbar wird er allein im Bild
   aus einer angemeldeten Sitzung. Die Anmeldung steht deshalb neben der
   Schattenbindung. **Und sie darf kein `nullptr`-Fenster bekommen:**
   `enableBlurBehind(nullptr, …)` stürzt unter Wayland ab (SIGSEGV), während
   derselbe Aufruf offscreen zurückkehrt.
7. **Das Bildpunktverhältnis des Fensters steht nach `show()` noch nicht
   fest.** Unter Wayland meldet Qt zunächst 2 und rund eine Sekunde später
   1,6, zugestellt als `QEvent::DevicePixelRatioChange` **ohne begleitendes
   `QEvent::Resize`**. Ein `KSvg::FrameSvg` folgt dem Bildschirm ohnehin nicht
   von selbst — sein Verhältnis ist nach dem Bau 1, gleich welche Skalierung
   gilt. Wer die Hülle nur im `resizeEvent()` nachzieht, zeichnet dauerhaft
   bei 2 auf einem Fenster, das 1,6 ist. **Offscreen tritt der Fall nicht
   auf.**
8. **Ob der Kontrasteffekt angemeldet wird, sagt das Theme.** Die vier Werte
   stehen in der Gruppe `[ContrastEffect]` der Datei `metadata.desktop` des
   Themes; fehlt die Gruppe — wie bei `default` —, wird nichts angemeldet.
   Gelesen werden sie selbst und nicht über `Plasma::Theme`: Diese Klasse
   liegt in libPlasma, das QtQuick nachzöge (Bauentscheidung zu #83).
9. **Ohne Sitzung mit Weichzeichner gilt der Auswahlpfad `opaque`.** Sonst
   zeichnete das Fenster die durchscheinende Fassung der Theme-Grafik ohne den
   Compositor, der sie tragfähig macht — Punkt 4 verspricht das Gegenteil. Die
   Bedingung hängt **nicht** an
   `KWindowEffects::isEffectAvailable(BlurBehind)`: Dieser Wert ist träge und
   liefert in der Sitzung des Kunden **vor** der ersten eigenen Anmeldung
   `false`, danach `true` — daran aufgehängt schaltete das Fenster ausgerechnet
   beim Start auf deckend. Denkzettel fragt stattdessen KWin selbst
   (D-Bus, `org.kde.kwin.Effects.isEffectLoaded("blur")`) und antwortet ohne
   Rückfrage mit „nein", wo es gar kein zusammensetzendes Fenstersystem gibt.
   **Grenze, benannt statt verdeckt (05.08.2026, karpathy-Befund K4 zu Sprint 7):**
   Dieser Wert wird **einmal beim Anlegen des Fensters** erhoben. Wer den
   Weichzeichner **zur Laufzeit** abschaltet, während der Dienst läuft, bekommt
   bis zum Neustart weiter die durchscheinende Fassung — Punkt 4 gilt für den
   Zustand beim Start, nicht für einen Wechsel danach. Der Theme-Wechsel ist
   davon **nicht** betroffen; ihn fängt die Wache auf `plasmarc`. Ob die Grenze
   geschlossen oder festgeschrieben wird, ist offen (Issue #93).
10. **Der Kontrasteffekt hat auf diesem Stand keinen Empfänger.** KWin 6.7.3
    führt unter 54 geladenen Effekten **keinen** mit „contrast" im Namen;
    `isEffectLoaded("backgroundcontrast")` antwortet `false`, `blur` antwortet
    `true`. Die Anmeldung aus Punkt 8 geht damit ins Leere.
    **Betroffen sind die drei Themes, die den Effekt anfordern** — an den
    Dateien selbst nachgezählt (`/usr/share/plasma/desktoptheme/*/metadata.desktop`,
    Gruppe `[ContrastEffect]`): `cachyos-emerald` und `cachyos-emerald-color`
    (`intensity=0.40`) sowie **`Iridescent-round`** (`0.45`). Ihre Hülle deckt
    fast nichts; dort steht der Text auf dem Bildschirmhintergrund und auf
    nichts sonst. Sie sind darauf gebaut, dass der Compositor den Grund
    abdunkelt — und genau das tut er auf diesem Stand nicht.
    **`cachyos-emerald-light` ist ein eigener Fall und nicht Teil dieser
    Bedingung.** Es fordert **keinen** Kontrasteffekt an; nach Punkt 8 meldet
    Denkzettel dort auch keinen an. Seine dunkle Themeschrift auf einer
    durchscheinenden Hülle bliebe deshalb **auch auf einem KWin mit geladenem
    Kontrasteffekt**, wie sie ist — gemessen über weißem Grund 14,32:1, über
    schwarzem 1,37:1, und **keine Wahl der Schriftfarbe rettet beide
    Richtungen**.
    *Warum das hier so ausdrücklich steht (05.08.2026, UI-Review Sprint 8,
    Befund P2, DoD 4):* Die erste Fassung dieses Punktes zählte die drei
    `cachyos-emerald`-Themes auf und begründete den Absturz unter
    `cachyos-emerald-light` damit, das Theme trage „genau dafür seine
    `[ContrastEffect]`-Gruppe". **Es trägt keine.** Die Begründung trug also für
    den einen Fall nicht, für den sie geschrieben war, und ein Theme mit
    demselben Problem fehlte in der Aufzählung. Belege in
    `docs/scrum/reviews/sprint-08-s85-lesbarkeit/` und
    `docs/scrum/reviews/sprint-08-ui-review/`. Die Bedingung ist benannt und
    nicht geschlossen.
11. **Die Vorrangregel für die Textfarben steht an genau einer Stelle.** Die
    beiden Quellen aus 3.1 bewegen sich zu verschiedenen Zeitpunkten — das
    Theme beim Theme-Wechsel, das Schema beim Palettenwechsel. Eine Umsetzung,
    die die Themefarbe nur beim Theme-Wechsel schreibt, ist bei der Abnahme
    richtig und nach dem ersten Schemawechsel falsch, und zwar **lautlos**:
    kein Rückgabewert, kein Ereignis, kein roter Prüfsatz. Beide Anlässe laufen
    deshalb durch dieselbe Funktion.
12. **KSvg kennt kein Gegenstück zu `ForegroundInactive`.** Die Aufzählung
    `KSvg::Svg::StyleSheetColor` führt je Farbsatz `Text`, `Background`,
    `Highlight`, `HighlightedText` und drei Signalfarben. Der Notiztext kommt
    deshalb über `KSvg::Svg::color(Text)` bei gesetztem `colorSet(Window)` —
    das **ist** bereits die Regel aus 3.1, über acht Themes und drei Schemata
    gemessen —, die gedämpfte Klasse dagegen aus der `colors`-Datei des Themes,
    selbst gelesen wie die Gruppe aus Punkt 8.

## 4. Aufnahmefenster (Sprachnotiz)

- Gleiche Machart wie Capture; die **Aufnahme läuft ab Fensteröffnung** —
  kein Start-Knopf.
- Inhalt: Aufnahme-Indikator (roter Punkt), einfacher Pegel, laufende
  Zeitanzeige, gleiche Fußzeile.
- Strg+Enter: Aufnahme stoppen, Audio speichern, Notiz vom Typ `audio`
  anlegen, Transkriptions-Job einreihen, Fenster verstecken. Esc: Aufnahme
  verwerfen (Datei löschen).
- Technik: QtMultimedia (PipeWire-Backend), Format **Opus in OGG**
  (`audio/*.ogg`), mono, 48 kHz — klein und von Qt direkt abspielbar.
- Obergrenze 15 Minuten (Schutz vor vergessener Aufnahme); Hinweis in der
  Zeitanzeige ab Minute 14.

## 5. Datenhaltung

### 5.1 SQLite (eine DB: `~/.local/share/denkzettel/denkzettel.db`)

```sql
notes(id INTEGER PK, created_at TEXT ISO8601, type TEXT 'text'|'audio',
      content TEXT,            -- Text bzw. Transkript
      audio_path TEXT NULL,    -- relativ zu audio/, nur type='audio'
      audio_duration_s INTEGER NULL,
      category TEXT NULL,      -- KI-Kategorie, NULL = unanalysiert
      state TEXT 'neu'|'transkribiert'|'analysiert',
      needs_reembed INTEGER NOT NULL DEFAULT 0,  -- nach Bearbeitung (Abschn. 9)
      analysis_attempts INTEGER NOT NULL DEFAULT 0,  -- Fehlerzähler 7.2
      analysis_last_error TEXT NULL)
tags(note_id FK, tag TEXT)
embeddings(note_id FK PK, model TEXT, vector BLOB)  -- float32-Array
proposals(id INTEGER PK, kind TEXT 'bundle'|'task', created_at TEXT,
          status TEXT 'offen'|'zurueckgestellt',
          payload TEXT JSON)   -- Bündel: Titel+Markdown; Task: Felder
proposal_notes(proposal_id FK, note_id FK)
transcribe_jobs(note_id FK PK, enqueued_at TEXT, attempts INTEGER,
                last_error TEXT NULL)
meta(key TEXT PK, value TEXT)  -- Schema-Version u. Ä.
```

- Volltextindex: **FTS5**-Tabelle `notes_fts(content)`, per Trigger synchron.
  Sie hält **keinen eigenen Text**, sondern verweist mit `content='notes'`,
  `content_rowid='id'` auf die Notiztabelle (Schemaversion 2, Issue #8) — der
  Notiztext existiert genau einmal. Daraus folgt eine Bedingung, ohne die der
  Index still verwahrlost: Die Trigger für Ändern und Löschen müssen FTS5 den
  **alten** Text mitgeben (`'delete'`-Kommando mit `old.content`). Mit dem
  neuen Text bleiben die alten Wörter auffindbar, und weder ein Fehler noch
  FTS5s `integrity-check` zeigen das an — nur eine Suche nach dem alten Wort
  (siehe `StoreTest::keepsSearchIndexInSync()`).
- Audio liegt als Datei unter `audio/` (Name = Notiz-ISO-Zeitstempel), die DB
  hält den Verweis. Löschen einer Notiz löscht Tags, Embedding, FTS-Eintrag,
  `proposal_notes`-Verweise und Audio-Datei in einer Transaktion +
  Dateisystem-Aufräumen.

### 5.2 Einstellungen und Geheimnisse

- Einstellungen: **KConfig** (`~/.config/denkzettelrc`).
- API-Keys (openrouter, OpenAI): **KWallet** — nie im Klartext in
  Config-Dateien. (OAuth-Tokens erst, falls der spätere
  Codex-App-Server-Zusatzpfad aus 7.5 je gebaut wird.)

## 6. Suche

Volltextsuche über FTS5 mit **Operatoren im Suchfeld** (Entscheidung drittes
Interview). Syntax-Umfang (damit ist offene Frage 3 des Konzepts beantwortet):

| Operator | Bedeutung |
|---|---|
| `tag:backup` | Notizen mit KI-Tag `backup` |
| `kat:todos` | Kategorie (alle, todos, ideen, cli, persoenlich, software) |
| `typ:text` / `typ:audio` | Notiztyp |
| `vor:2026-07` / `vor:2026-07-15` | erstellt vor Datum (Monat oder Tag) |
| `nach:2026-06` | erstellt nach Datum |
| `"exakte Phrase"` | Phrasensuche (FTS5-Phrase) |
| freier Text | FTS5-Volltext (UND-verknüpfte Terme) |

- Alle Bestandteile sind **UND-verknüpft**; kein OR, keine Klammern (V1).
- Unbekannte `xyz:`-Präfixe werden als Volltext behandelt (kein Fehler).
- Parser ist reine Funktion `QString → SearchQuery` — unit-testbar.
- FTS5-Tokenizer: **`trigram remove_diacritics 1`** (Kundenentscheidung
  01.08.2026, Issue #8). Ein Suchbegriff findet **Wortteile an jeder Stelle**:
  „grafieren" findet „fotografieren", „bahn" findet „Straßenbahn", „sprech"
  findet „Besprechung". „bucher" findet „Bücher" — die Umlaut-Toleranz bleibt
  Kernanforderung und ist mit `remove_diacritics 1` erhalten.
  - **Damit ist die Präfixsuche keine eigene Festlegung mehr.** Sie ist im
    Teilstring-Verhalten enthalten. Die Abfrage hängt **kein** `*` an: Am
    System gemessen sind `"foto"` und `"foto"*` beim trigram-Tokenizer
    identisch — er erzeugt ausschließlich vollständige Drei-Zeichen-Tokens,
    an denen ein Präfixzeichen nichts erweitern kann. Ein `prefix=`-Index
    wird ebenfalls nicht angelegt (Beschluss E2).
  - **Preis, gemessen an 20 000 Notizen:** Der trigram-Index ist rund
    **sechsmal** so groß wie ein `unicode61`-Index (1,8 MiB → 10,9 MiB) und
    damit gut dreimal so groß wie der Rohtext selbst. Für den erwarteten
    Bestand ist das tragbar; bei sechsstelligen Notizzahlen wäre es neu zu
    bewerten.
  - **Grenze (Befund Issue #8, SQLite 3.53.4):** Der Tokenizer entfernt
    diakritische Zeichen. `ß` trägt keines — es ist ein eigener Buchstabe und
    bleibt stehen. „strassenbahn" findet „Straßenbahn" deshalb **nicht**,
    „grosse" nicht „Größe". Gilt für `unicode61` wie für `trigram`; die
    ß/ss-Faltung verlangt einen eigenen Tokenizer und ist eigene Story (S30).
- **Suchbegriffe unter drei Zeichen (Entscheidung Issue #8):** Ein
  trigram-Index kann sie prinzipbedingt nicht enthalten — ein Trigramm ist
  drei Zeichen lang. Solche Begriffe werden deshalb **als Teilstring direkt
  auf `notes.content` verglichen** (`LIKE '%…%'`), die übrigen weiterhin über
  den Index; beide Wege sind UND-verknüpft. Begründung: „KI", „PO" oder „ad"
  sind echte Suchbegriffe, und eine Suche, die dabei wortlos leer bleibt,
  wäre ein Fehler, den niemand als solchen erkennt. Der Alternativweg — ein
  Hinweis im Leerzustand — würde eine reine Umsetzungsgrenze zur Regel
  machen, die der Nutzer lernen muss. **Kosten gemessen** (20 000 Notizen):
  3 ms je Abfrage, weniger als die Indexabfrage selbst (9 ms) — der
  Einwand des vollen Tabellendurchlaufs trägt in dieser Größenordnung nicht.
  - Grenze dieses Wegs: Er ignoriert Groß-/Kleinschreibung nur für ASCII
    („ki" findet „KI"), faltet aber keine diakritischen Zeichen („u" findet
    kein „ü") und keine Groß-/Kleinschreibung darüber hinaus („ü" findet
    kein „Ü"). Betrifft ausschließlich Begriffe mit ein oder zwei Zeichen.
- Die Trefferliste behält die Ordnung der Bibliothek (neueste zuerst, 9.)
  statt der FTS5-Relevanzsortierung — nur so trägt sie deren Tagesgruppen.
  - **BM25 ist am 04.08.2026 geprüft und verworfen** (Kundenentscheidung;
    Belege unter `docs/scrum/reviews/2026-08-04-bm25/`). Der Grund ist **nicht**
    der Trigramm-Tokenizer — die Vermutung, ein in Trigramme zerfallender
    Suchbegriff verzerre die Formel, ist widerlegt: FTS5 summiert BM25 über
    **Phrasen**, nicht über Tokens, und die Abfrage ist phrasenweise gebaut.
    Verworfen wurde BM25 aus drei anderen Gründen:
    1. **Bei kurzen Notizen entartet es zu „kürzeste Notiz zuerst".** Ein
       gesuchter Begriff steht dort fast immer genau einmal (`f=1`); bei einem
       einzelnen Suchbegriff kürzt sich der IDF-Anteil heraus, und übrig bleibt
       die Längennormierung. Eine Einzeilernotiz, in der „Backup" beiläufig
       vorkommt, stünde vor der ausführlichen Notiz über Backups. `k1` und `b`
       sind in FTS5 **nicht einstellbar** — es gibt keinen Regler dagegen.
    2. **Zwei Suchwege hätten gar keinen Rang:** Begriffe mit ein oder zwei
       Zeichen laufen über `content LIKE` und stehen nicht im FTS-Index; reine
       Filtersuchen (`tag:`, `kat:`, `vor:`) enthalten keinen Volltextterm.
    3. **Die Sortierung ist keine Eigenschaft der Suche, sondern die
       Voraussetzung der Listendarstellung.** Der Zeilenbau gruppiert, er
       sortiert nicht; nach Rang geordnete Eingabe erzeugt Tagesköpfe mehrfach
       und in wechselnder Folge. Dazu hängt der Zeitstempel je Eintrag an seiner
       Gruppe (9., Zeichnung 3b): Ohne Kopf verlöre jeder Treffer von heute und
       gestern seine Tagesangabe. Und da bei jedem Tastendruck gesucht wird,
       spränge die Liste, während der Nutzer noch tippt.
  - **Was stattdessen fehlt, ist die Lesbarkeit des Treffers**, nicht seine
    Rangfolge: Der gefundene Begriff wird in der Liste nicht hervorgehoben, und
    die Fundstelle liegt oft im abgeschnittenen Teil (Hinweis 2 des
    S6-Reviews, Sprint 3). Eigene Story.

## 7. KI-Pipeline

### 7.1 Provider-Abstraktion

Interface `AiProvider` mit zwei Fähigkeiten: `chat(prompt) → text/json` und
`embed(text) → vector`. Implementierungen:

- **Ollama** (lokal/eigene URL): `/api/chat` + `/api/embed`.
  Defaults: LLM `qwen3:8b`, Embedding `bge-m3` (mehrsprachig).
- **openrouter.ai**: OpenAI-kompatible API, API-Key aus KWallet — nur `chat`.
- **OpenAI**: per Platform-API-Key (siehe 7.5) — nur `chat`.

**Embeddings kommen in v1 immer aus Ollama** (Präzisierung nach Befund der
Schätzklausur 31.07.2026: openrouter bietet keinen Embedding-Endpunkt).
Der LLM-Provider ist frei wählbar, das Embedding-Modell läuft lokal —
einheitlich, kostenlos, und die Cluster-Schwelle (7.3) bleibt an ein
Modell gebunden. Ohne erreichbares Ollama degradiert Denkzettel sichtbar:
Klassifikation über den gewählten Provider funktioniert weiter, Themen-
Bündel entfallen (Hinweis in Einstellungen und Tray-Tooltip).

Alle Aufrufe über Qt Network, asynchron, mit Timeout (30 s) und einem
Wiederholungsversuch. „Verbindung testen" in den Einstellungen macht je einen
Mini-`chat`- und (bei Ollama) `embed`-Aufruf und zeigt Latenz oder Fehler.

### 7.2 Analyse-Lauf

Auslöser je Einstellung: **sofort** (nach Speichern/Transkription),
**periodisch** (Intervall, Default 30 min) oder **auf Abruf** (Tray/D-Bus).
Ein Lauf verarbeitet alle Notizen mit `state != 'analysiert'` sowie —
nur für Schritt 2 — solche mit `needs_reembed = 1`; höchstens 50 Notizen
pro Lauf (Budget, Abschnitt 14), der Rest folgt im nächsten Lauf:

1. **Klassifikation + Tags** (ein LLM-Aufruf pro Notiz, JSON-Schema:
   `{category, tags[], is_todo, task?}`): Kategorie aus fester Liste (TODOs,
   Ideen, CLI-Befehle, Persönlich, Software-Ideen), 1–4 Tags kleingeschrieben.
   Für `is_todo=true` extrahiert derselbe Aufruf die Task-Felder
   (`description, project, tags, due, priority` — `due`/`priority` nur bei
   klarem Signal, sonst null).
2. **Embedding** (ein `embed`-Aufruf pro Notiz) → `embeddings`-Tabelle;
   setzt `needs_reembed` zurück.
3. **Clustering + Vorschlags-Erzeugung** (7.3/7.4).

Fehlerbehandlung nach Loop-Konventionen: Fehlversuche werden persistent
gezählt (`notes.analysis_attempts`/`analysis_last_error`, überlebt
Daemon-Neustarts; Erfolg setzt zurück). Ab dem zweiten Fehlschlag wird die
Notiz übersprungen und der Fehler im Tray-Tooltip + Log gemeldet — kein
endloses Wiederholen, keine Selbstheilung.

### 7.3 Themen-Clustering (beantwortet offene Frage 1)

- Grundlage: Cosine-Ähnlichkeit der Embeddings aller **unexportierten,
  analysierten** Notizen (Brute-Force-Paarvergleich; bei Volllauf-Schwelle
  ~200 Notizen sind das ≤ 20k Vergleiche — unkritisch, keine Vektor-DB).
- Verfahren: Single-Linkage-Verkettung — Notizenpaare mit Ähnlichkeit ≥
  **0,60** (interne Konstante, kalibrierbar, kein User-Setting) landen im
  selben Cluster.
- Cluster mit ≥ **Bündel-Schwelle** Notizen (Einstellung, Default **3**)
  werden dem LLM vorgelegt: Es benennt das Thema, darf offensichtliche
  Ausreißer entfernen (Plausibilisierung) und erzeugt die Sammelnotiz
  (siehe 8.1). Ergebnis: ein `bundle`-Vorschlag.
- **Notizen ohne Cluster** bleiben einfach im Bestand. Reißt die
  Alters-Schwelle des Volllauf-Schutzes, erzeugt der Dienst zusätzlich ein
  Bündel „Vermischtes vom <Zeitraum>" aus den ältesten clusterlosen Notizen —
  ebenfalls nur als Vorschlag, jede Notiz abwählbar.
- Bereits vorgeschlagene, aber zurückgestellte Notizen (`status =
  'zurueckgestellt'`) werden beim nächsten Lauf erneut geclustert — ein
  „Später" verschiebt nur, es versteckt nichts dauerhaft.

### 7.4 Task-Vorschläge

Für jede als TODO klassifizierte Notiz entsteht ein `task`-Vorschlag mit den
extrahierten Feldern. Kein Auto-`task add` — Ausführung erst nach Bestätigung
im Review (Abschnitt 9).

### 7.5 OpenAI-Anbindung

Die Entscheidung des dritten Interviews (OAuth „Sign in with ChatGPT" von
Anfang an) stand ausdrücklich unter Recherchevorbehalt. Die Recherche
(31.07.2026, vollständig mit Quellen:
`recherche/2026-07-31-openai-oauth-machbarkeit.md`) hat den Vorbehalt
ausgelöst:

- **„Sign in with ChatGPT" ist ein reines Identitätsverfahren** (Beta, sechs
  kuratierte Partner, keine öffentliche Selbstregistrierung). Die App erhielte
  Name, E-Mail und Profilbild — laut OpenAI-Doku ausdrücklich keine Tokens
  und keinen Modellzugriff.
- Der inoffizielle **Codex-OAuth-Abo-Weg** (Codex CLI, OpenClaw) ist technisch
  einsehbar, aber für Dritt-Apps nie freigegeben (ToS-Grauzone, von OpenAI
  seit 12/2025 mehrfach unbeantwortet) — und liefert nur Chat-Endpunkte:
  **keine Embeddings**, die Denkzettels KI-Architektur (7.1/7.3) zwingend
  braucht.

**Konsequenz für v1: OpenAI per manuellem Platform-API-Key** (Ablage in
KWallet), gleichberechtigt neben Ollama und openrouter. Die
Einstellungen-Seite zeigt bei OpenAI einen kurzen Hinweistext, warum es kein
„Mit ChatGPT anmelden" gibt. Ein Abo-Weg über den Codex App Server
(JSON-RPC/stdio) bleibt als optionaler späterer Zusatzpfad denkbar, wird für
v1 aber nicht gebaut.

## 8. Überführungen

### 8.1 Obsidian-Export (Bündel)

- Zielnotiz: **eine Sammelnotiz pro Thema** in `<Vault>/_INBOX/`, Dateiname
  `Denkzettel <Thema> <YYYY-MM-DD>.md`.
- Aufbau: Vault-konformes Frontmatter (`type`, `tags`, `created` — beim Bauen
  gegen die Vault-CLAUDE.md-Konventionen verifizieren), `# <Thema>`, dann
  `## <YYYY-MM-DD>`-Abschnitte mit den Notizen als Absätze/Bullets
  (chronologisch), wie die Markdown-Vorschau im Wireframe.
- Sprachnotizen exportieren ihr **Transkript**; die Audio-Datei wird beim
  Export gelöscht (Konzept-Entscheidung: Audio lebt nur solange die Notiz).
- Nach bestätigtem Export: Notizen in einer Transaktion löschen (inkl. Audio,
  Tags, Embeddings, FTS) und den **Vorschlag samt `proposal_notes`-Verweisen
  entfernen** — Übernehmen und Verwerfen enden gleich, nur dass Übernehmen
  vorher ausführt. Ein „übernommen"-Zustand existiert nicht (Durchlauf-
  Speicher, keine Vorschlags-Historie).

### 8.2 Taskwarrior

- Ausführung per `QProcess`: `task add <description> project:<p> +tag1 +tag2
  due:<d> priority:<p>` (nur gesetzte Felder), danach bei längerem Notiztext
  `task <uuid> annotate <volltext>` (UUID aus `task add`-Ausgabe).
- Fehlerfall (task-Binary fehlt, Exit ≠ 0): Vorschlag bleibt offen, Fehler
  wird an der Karte angezeigt — nichts geht verloren.
- Nach Erfolg: Notiz löschen und Vorschlag entfernen (wie 8.1).

### 8.3 Voll-Export (Rettungsweg)

- Menüpunkt in der Bibliothek: exportiert **alle** Notizen als Ordner
  `denkzettel-export-<datum>/` mit einer `.md` je Notiz (ISO-Name, Frontmatter
  mit Kategorie/Tags/Typ) plus `audio/`-Unterordner mit den Originaldateien.
- Rein lesend — der Bestand bleibt unverändert. Kein KI-Aufruf nötig.

## 9. Bibliothek und Vorschlags-Review

- **Bibliothek** (Fenster): Sidebar mit KI-Kategorien + Zählern, chronologische
  Notizliste, wie ein Posteingang in Tagesgruppen gegliedert (**Heute ·
  Gestern · Diese Woche · Letzte Woche · Älter**; „Woche" ist die
  Kalenderwoche, ihr Anfang folgt der Locale des Systems —
  `QLocale::firstDayOfWeek`, in Deutschland Montag), innerhalb der Gruppen
  neueste zuerst.
  Ein Eintrag zeigt Zeitstempel, die erste Zeile als Betreff, den Folgetext
  als Vorschau und Tag-Chips; Sprachnotizen zusätzlich ▶ und Dauer. Der
  Zeitstempel folgt der Gruppe: in Heute/Gestern die Uhrzeit, in den
  Wochengruppen Wochentag und Datum, in Älter das absolute Datum; im
  Detailbereich die volle Form. Die Gliederung ist fest — kein Umschalter,
  keine einklappbaren Gruppen (Wireframes 3a/3b).
  **Getrennt wird durch zwei Haarlinien einer Farbe, deren Ausdehnung die
  Rangfolge trägt** (Wireframe 3a, Kundenentscheidung 06.08.2026 auf Issue
  #101, nachdem der Weißraum allein die Notizen nicht mehr auseinanderhielt):
  zwischen zwei aufeinanderfolgenden Notizen **derselben** Gruppe eine auf die
  Textkante eingerückte Linie (12 px links und rechts, dieselben 12 px, auf
  denen Zeitstempel und Kopftext beginnen), über jedem Gruppenkopf **außer dem
  ersten** dieselbe Linie über die volle Breite der Zeile. Keine unter der
  letzten Notiz einer Gruppe, keine unter einem Kopf, und **keine an einer
  Kante der ausgewählten Zeile** — eine zweite Trennung dort träte mit der
  Auswahlmarkierung in Wettbewerb. Die Ausnahme gilt allein der eingerückten
  Linie: über einem Kopf bleibt die Linie stehen, auch wenn die Notiz darüber
  die ausgewählte ist. Die Farbe ist **keine Palettenrolle**, sondern die
  Mischung aus Listengrund und Textfarbe im Verhältnis
  `KColorScheme::frameContrast()` — das Verfahren, mit dem Kirigami seine
  Trennlinien färbt; über 18 Schemata liegt sie zwischen 1,24 : 1 und
  1,93 : 1 gegen den Listengrund, abwechselnde Zeilenfarben dagegen zwischen
  1,00 : 1 und 1,21 : 1 und an jeder Gruppengrenze bei 1,00 : 1 (gemessen
  06.08.2026). **Kein Maß ändert sich dadurch**: Beide Linien liegen in
  Innenabständen, die es schon gibt.
  **Die Stärke ist ein Maß in Gerätebildpunkten** (entdeckte Bedingung,
  07.08.2026, DoD 4): ganze Gerätebildpunktzeilen, mindestens eine, gerundet
  und nicht abgeschnitten. Der
  Satz steht hier, weil sein Fehlen einen Fehler getragen hat: Als logisches
  Rechteck gefüllt, belegte dieselbe Linie unter der Skalierung 1,6 mal einen
  und mal zwei Gerätebildpunkte — im Normalfall waren zwei von vier
  Gruppenlinien halb so stark wie die anderen, und in einem Lauf stand eine
  Gruppenlinie von einem Punkt über Eintragslinien von zwei. Dort sagte die
  Stärke das Gegenteil dessen, was die Ausdehnung sagt. Unter Skalierung 1
  ändert sich nichts.
  *Berichtigt am 08.08.2026 (karpathy-Nachlauf N1 und N2, beides nachgemessen).*
  Hier stand zusätzlich „**die Oberkante auf der Gerätebildpunktgrenze**". Die
  Zusicherung ist **gefallen**, und der Term, der sie herstellen sollte, ist aus
  dem Bau entfernt. **Die Grenze, gegen die er rundete, war nicht die des
  Bildschirms** — die Funktion sieht allein widget-lokale Koordinaten, während
  das Sichtfeld der Liste bei logisch 48 beginnt, unter 1,6 also bei 76,8
  Gerätebildpunkten. *Berichtigt am 11.08.2026 (karpathy-Nachlauf 2, K6):* Hier
  stand „er erreichte keine Grenze". Das war die falsche Ursache und nicht die
  halbe — er erreichte sehr wohl eine, nur die des Widgets. Bei den
  Verhältnissen, unter denen der Ursprung selbst auf dem Geräteraster liegt
  (1,0 · 1,25 · 1,5 · 2,0 · 2,5 — dort ist logisch 48 gerade 48, 60, 72, 96
  und 120 Gerätebildpunkte), ändert der Term nichts; **alle acht** gemessenen
  Verschiebungen liegen bei **1,4 und 1,6**, wo er es nicht tut.
  Über 280 gemessene Lagen (sieben
  Verhältnisse, zwanzig Zeilenlagen, zwei Malerursprünge) verschob er die Linie
  in 8 Lagen um einen Bildpunkt und änderte **keine einzige Höhe**. Die
  Einheitlichkeit trägt allein die ganzzahlige Höhe: Zwei Kanten, die ganzzahlig
  viele Gerätebildpunkte auseinanderliegen, runden auf Werte, die um dieselbe
  ganze Zahl auseinanderliegen. Ebenfalls berichtigt ist die Begründung des
  Rundens: Sie berief sich darauf, dass die Linie nie dünner werden dürfe, und
  das trifft **unterhalb 1,5** nicht zu (1,25 → 1 Bildpunkt → 0,80 logische
  Punkte). Was gilt: Eine Gerätebildpunktzeile ist die Untergrenze, und genau so
  breit ist die Linie unter Skalierung 1 — dem Zustand, den der Kunde
  abgenommen hat. **Aufrunden** statt Runden machte sie bei 1,25 auf 1,6
  logische Punkte dicker als gezeichnet.
  *Ausdrücklich ungeregelt bleibt die seitliche Kante:* Sie liegt unter 1,6 bei
  11,88 statt 12,0 Punkten — 0,12 Punkte, während der Seitenrand der Glyphen
  daneben im selben Bild zwischen 0,5 und 2,4 Punkten schwankt.
  **Bedingung, entdeckt bei der Umsetzung (DoD 4/B9): Die Ansicht zeichnet den
  oberen Nachbarn eines Auswahlwechsels nicht von sich aus neu.** Sie malt nur
  die Strecke zwischen alter und neuer Auswahl, und die Zeile über beiden Enden
  liegt außerhalb — ohne ausdrückliche Anmeldung bleibt dort eine Linie stehen
  oder fehlt eine (Issue #101, sechs Wechsel gemessen am 07.08.2026). Ein
  Standbild zeigt das nicht: `grab()` zeichnet jede Zeile neu.
  Die Gruppen werden beim Aufbau der Liste und bei jeder Fensteraktivierung
  nachgerechnet — es gibt keinen Mitternachtszeitgeber (Wireframe 3b).
  **Neu gruppiert wird dabei nur, wenn der Kalendertag ein anderer ist als
  beim letzten Aufbau** (entdeckt bei der Umsetzung, DoD 4): Neugruppieren
  setzt das Modell zurück und stellt die Auswahl wieder her, was die Liste zu
  ihr scrollt — ohne Tageswechsel warf ein Alt-Tab den Leser um 459 px auf
  seine Auswahl zurück (Issue #59, gemessen 04.08.2026). Der Kalendertag
  genügt als Bedingung, weil alle vier Gruppengrenzen Tagesgrenzen sind.
  Springt die Auswahl **per Taste** über eine Gruppengrenze, holt die Liste den
  Kopf der neuen Gruppe ins Bild (Wireframe 3b, Fall 4) — **und ebenso, wenn
  die Auswahl per Taste die erste Notiz einer Gruppe erreicht, ohne eine Grenze
  zu überschreiten** (Issue #70, Kundenentscheidung 04.08.2026): Wer aufwärts
  von der zweiten auf die erste Notiz geht, bekäme den Kopf sonst nie zu sehen,
  und ohne ihn steht unter „Heute" und „Gestern" nur eine Uhrzeit ohne Tag —
  die Zeitstempel-Regel oben setzt den Kopf voraus. Der Preis ist, dass die
  Liste sich in diesem Fall weiter bewegt als zuvor. Es gilt weiter für beide
  Fälle: Passen Kopf und Auswahl nicht zusammen ins Bild, bleibt der Kopf
  draußen.
  **Ein Mausklick bewegt die Liste überhaupt nicht** — weder holt er den Kopf,
  noch rückt er zur
  Auswahl nach: Wer zeigt, erwartet, dass die gezeigte Stelle bleibt, und ein
  Vorscrollen risse sie ihm unter dem Zeiger weg (gemessen 387 px, Issue #57).
  **Dass auch das Nachrücken zur Auswahl darunterfällt, ist bei der Umsetzung
  entdeckt worden** (DoD 4): Ein Klick auf eine angeschnittene Zeile rückte das
  Bild um eine Zeilenhöhe, und weil der View seine Auswahl erst danach aus dem
  beim Druck gemerkten Rechteck bestimmt, markierte er die Nachbarzeile — in
  13 von 14 gemessenen Fällen die falsche (Issue #71, gemessen 05.08.2026).
  Der Preis, ausdrücklich: Eine angeschnittene Zeile bleibt nach dem Klick
  angeschnitten. Sie ganz sichtbar zu machen hieße, sie unter dem Zeiger
  wegzuziehen — das ist der Fehler selbst.
  **Bedingung, entdeckt bei der Sichtprüfung (05.08.2026, DoD 4/B9): „bewegt die
  Liste überhaupt nicht" gilt für den Druck, nicht für die Sekunde danach.**
  `QAbstractItemView` startet beim Mausdruck einen **verzögerten Autoscroll**
  und holt die angeschnittene Zeile rund eine halbe Sekunde später doch ins
  Bild; die Markierung bleibt dabei auf der geklickten Zeile. Gemessen: Rollwert
  bis 500 ms unverändert, ab 550 ms um eine Zeile gerückt
  (`docs/scrum/reviews/sprint-07-s71-ruhige-liste/messungen/71-nachlaufender-autoscroll.txt`);
  am Bild bestätigt im UI-Review (11a gegen 11b, 71,9 logische Bildpunkte).
  **Der Absatz oben beschreibt damit, was die Story herstellt, nicht, was der
  Nutzer nach einer Sekunde sieht.** Ob der Nachlauf bleibt oder abgeschaltet
  wird, ist eine Produktentscheidung und offen (Issue #89); bis sie fällt, steht
  die Bedingung hier — ein Satz, der mehr zusichert, als der Bau hält, ist eine
  Falle, gleich wie die Entscheidung ausgeht.
  Der Tag geht dabei nicht verloren — der Detailbereich trägt den vollen
  Zeitstempel.
  Dazu Suchfeld (Abschnitt 6) und Button „Vorschläge" mit Badge.
- Detailansicht: **Lese- und Bearbeiten-Ansicht** (Entscheidung drittes
  Interview — v. a. für fehlerhafte Transkripte). Bearbeiten behält
  Kategorie/Tags und `state`, setzt aber `needs_reembed = 1` — der nächste
  Analyse-Lauf erneuert nur das Embedding (7.2), denn es veraltet mit dem
  Text. Löschen-Aktion mit 5-Sekunden-Undo (Spec-Ergänzung, nicht im
  Konzept: rein client-seitig verzögertes Löschen, kein Soft-Delete-Zustand
  in der DB).
- **Bedingungen des Bearbeiten-Zustands** (S8; die letzten beiden entdeckt
  bei der Umsetzung, DoD 4):
  - Ungespeicherte Änderungen werden **nie ohne Nachfrage** geschrieben oder
    verworfen. Auswahlwechsel, Fensterschließen, Esc und „Abbrechen“ führen
    denselben Dialog mit **Speichern · Verwerfen · Abbrechen**. Das weicht
    bewusst vom Capture-Fenster ab, wo Esc still verwirft (3): dort steht ein
    nie gespeicherter Entwurf, hier eine bereits gespeicherte Notiz.
    „Abbrechen“ liegt mit im Dialog, weil Schaltfläche und Kürzel dieselbe
    Handlung sind — ein Fehlklick auf sie ist genau der Fall, gegen den der
    Dialog gefasst ist.
  - Die gespeicherte Notiz **bleibt in der laufenden Trefferliste stehen**,
    auch wenn ihr neuer Text nicht mehr auf den Suchbegriff (6) passt; erst
    die nächste Änderung des Suchbegriffs liest den Store neu. Sonst
    verschwände die Notiz unter der Hand, die sie eben berichtigt hat.
  - Das **Suchfeld ist währenddessen abgeschaltet**. Eine Suche baut die
    Liste neu auf; die Notiz unter dem Editor kann dabei aus ihr
    herausfallen, und dann hat der Dialog keine Zeile mehr, auf die er die
    Auswahl zurücknehmen könnte.
  - **Bauart des Dialogs (entschieden in Sprint 5, #66):** Der Wächter ist
    ein **`KMessageDialog`** vom Typ `WarningTwoActionsCancel` mit
    `KStandardGuiItem`-Symbolen; **Vorgabeantwort ist „Speichern"**.
    Grund ist die in Sprint 4 entdeckte Bedingung (DoD 4/B9): Unter der
    KDE-Plattformintegration (`QT_QPA_PLATFORMTHEME=kde`) beantwortet das
    System einen gebauten `QMessageBox` mit einem **eigenen Meldungsfenster
    samt eigenen Knopfobjekten** — es übernimmt Beschriftung, Rollen und
    Reihenfolge, aber nichts, was nachträglich am `QPushButton` gesetzt wird
    (Symbole, Vorgabe-/Escape-Knopf). Ein `KMessageDialog` ist ein
    gewöhnlicher `QDialog` und bleibt der eigene. Daraus folgt für die
    Prüfung: Der Dialogtest misst den Dialog, den die Anwendung **zeigt**
    (`QApplication::activeModalWidget()`), unter gesetztem Plattform-Thema —
    ein Test ohne Plattform-Thema misst einen Dialog, den kein
    KDE-Sitzungsnutzer sieht.
  - **Bedingungen dieser Bauart, alle am 02.08.2026 gemessen** (DoD 4/B9):
    - `KMessageDialog` kennt **keinen Zweittext** (`informativeText`); Frage
      und Erläuterung stehen in einem Text, durch eine Leerzeile getrennt.
    - Die Antwortrollen sind `Yes` · `No` · `Reject` statt
      `Accept` · `Destructive` · `Reject`. **Zugesichert ist die Bedeutung**
      (Speichern schreibt und führt die Handlung aus, Verwerfen führt sie
      ohne Schreiben aus, Abbrechen bleibt im Editor), nicht die Rolle und
      nicht die Reihenfolge.
    - Die Vorgabeantwort **folgt dem Fokus**: Unter selbstvorgabefähigen
      Knöpfen macht der Fokuswechsel den fokussierten Knopf zur Vorgabe. Die
      KDE-Bauart gibt beim Sichtbarwerden Fokus und Vorgabe an „Abbrechen";
      „Speichern" muss deshalb **nach** dem Anzeigen Fokus *und* Vorgabe
      erhalten, und eine Zusicherung darüber ist erst gültig, wenn sie am
      **sichtbaren** Dialog gemessen wird.
    - Wird der Dialog von Hand angezeigt, ist er **nicht mehr modal** durch
      ein späteres `exec()`; die Modalität ist dann selbst zu setzen.
    - Das **Warnsymbol** (`dialog-warning`) wird **ausdrücklich gesetzt**.
      `KMessageDialog::setIcon()` sagt zwar zu, bei leerem Symbol eines nach
      Dialogtyp zu wählen — gemessen kommt keines, und der Dialog trägt dann
      gar kein Bildetikett. Ein Dialog über drohenden Datenverlust ist der
      Kernfall des Warnsymbols (PO-Entscheidung 02.08.2026; Zeichnung 2a,
      Zustand C nachgezogen).
    - **Die Bauart klingt** (entdeckt am 04.08.2026): `showEvent()` meldet bei
      jedem Anzeigen das KNotification-Ereignis `messageWarning`, dem
      `plasma_workspace.notifyrc` den Systemklang `dialog-warning` zuordnet —
      abgespielt im eigenen Prozess über libcanberra. Das ist
      KDE-Plattformstandard und **bleibt so**: Lautstärke und Stummschaltung
      regelt der Nutzer im System. `KMessageDialog::setNotifyEnabled(false)`
      würde den Klang abschalten; **genau das ist bewusst unterlassen**
      (Kundenentscheidung 04.08.2026), damit ihn niemand später für ein
      Versehen hält und wegmacht. Still sind allein die Test- und Bildläufer:
      sie lenken libcanberra vor `main()` auf den Null-Treiber
      (`tests/testsilence.cpp`) — oberhalb des Audiogeräts bleibt alles
      unverändert, und kein Test misst Klang.
- Steckt die Notiz in einem **offenen Vorschlag**, verwirft Bearbeiten oder
  Löschen diesen Vorschlag (seine Vorschau wäre veraltet); der nächste
  Analyse-Lauf erzeugt ihn auf aktuellem Stand neu.
- Bei Sprachnotizen: Audio-Player (Play/Pause, Fortschritt, Zeit) über dem
  Transkript.
- **Vorschlags-Review**: Liste offener Vorschläge beider Arten. Bündel-Karte:
  Titel, Notizliste mit Abwahl-Checkboxen, Markdown-Vorschau, Ziel „→ Obsidian
  _INBOX". Task-Karte: editierbare Felder (Beschreibung, project, tags, due,
  priority), Annotation-Vorschau, Ziel „→ Taskwarrior". Aktionen je Karte:
  **Übernehmen · Später · Verwerfen** (Verwerfen löscht nur den Vorschlag,
  nie Notizen).

## 10. Tray und Benachrichtigungen

- **KStatusNotifierItem**, dauerhaft. **Ein** Menü, in drei Gruppen; Wortlaut
  und Symbolnamen sind verbindlich (Wireframe 5a, Issue #60). Die Symbole
  kommen ausnahmslos aus dem Symbol-Thema: Nur ein Symbol aus dem Thema trägt
  einen Namen, und über das Tray-Protokoll geht der **Name**, nicht das Bild.

  | Eintrag | Symbolname | Zustand | Kürzel-Hinweis |
  |---|---|---|---|
  | Notiz erfassen | `document-edit` | aktiv | Meta+N |
  | Sprachnotiz aufnehmen | `audio-input-microphone` | inaktiv bis M4 | Meta+Umschalt+N ab M4 |
  | *— Trenner —* | | | |
  | Bibliothek öffnen | `view-list-text` | aktiv | — |
  | Jetzt analysieren | `system-run` | inaktiv bis M5 | — |
  | Vorschläge (Zähler) | `tools-wizard` | inaktiv bis M5 | — |
  | *— Trenner —* | | | |
  | Denkzettel einrichten … | `configure` | **erst mit #16** | — |
  | Beenden | `application-exit` | aktiv | — |

  **„Beenden" steht abgesetzt in der letzten Gruppe** und nie neben dem
  häufigsten Eintrag: Es beendet den Dienst und mit ihm das Kürzel. Bis der
  Einstellungs-Dialog (#16) steht, ist „Denkzettel einrichten …" **gar kein
  Eintrag** — ein dauerhaft ausgegrauter erklärt dem Nutzer nicht, warum er
  grau ist (KDE HIG).
- **Der Kürzel-Hinweis ist ein Hinweis.** Meta+N steht als Text am Eintrag und
  darf die Registrierung bei KGlobalAccel nicht doppeln; das Kürzel der
  Menü-Aktion trägt deshalb `Qt::WidgetShortcut` und erreicht nur das Fenster
  seines Menüs — und das hat keines, plasmashell zeichnet es.
- **Linksklick auf das Tray-Icon öffnet dasselbe Menü wie der Rechtsklick**
  (`ItemIsMenu`). Das weicht bewusst vom KDE-Standard ab, der den Linksklick
  für eine Hauptaktion vorsieht: Denkzettel hat kein Hauptfenster, sondern
  mehrere gleichrangige Wege, und die Recherche zum KDE-Verhalten wurde dem
  Kunden vorgelegt. Kundenentscheidung vom 01.08.2026, belegt in Issue #44, am
  02.08.2026 nach erneuter Vorlage bestätigt — **bei HIG- oder UI-Reviews kein
  Befund.**
- **Entdeckte Bedingung (Messung 02.08.2026, Issue #60): Getrennte Menüs für
  Links- und Rechtsklick sind unter Plasma/Wayland nicht zu haben.** Sie
  hießen `ItemIsMenu=false` plus ein eigenes Menü im
  `activateRequested`-Handler; das Menü müsste dann Denkzettel selbst
  zeichnen. Als Popup wird es erzeugt und zwei Millisekunden später wieder
  geschlossen — ein `Qt::Popup` braucht unter Wayland eine Elternfläche und
  einen Eingabe-Grab, und ein Klient mit nichts als einem Tray-Symbol hat
  beides nicht. Als gewöhnliches Fenster bleibt es stehen, aber die
  gewünschte Lage wird verworfen und KWin setzt es in die Bildschirmmitte.
  Deshalb bleibt es bei einem Menü; der Kunde hat den Rückfall am 02.08.2026
  entschieden. Beleg: `docs/scrum/reviews/sprint-04-s33-traymenues/`. Von den
  drei in Wireframe 5a benannten HIG-Abweichungen bleibt damit **nur A1**
  (Linksklick öffnet ein Menü); A2 (zwei verschiedene Menüs) und A3 („Beenden"
  nur über den Rechtsklick) entfallen ersatzlos, weil es die zweite Liste
  nicht gibt.
- **Zweite entdeckte Bedingung (ebenda): Die Symbolnamen erreichen Plasma nur,
  solange ein Symbol-Thema auflösbar ist.** Ohne Plattform-Thema enthalten die
  Suchpfade nichts als die Qt-Ressource, `QIcon::fromTheme()` liefert ein
  leeres Symbol ohne Namen, und das Menü käme unbebildert an. Unter Plasma ist
  das Thema da; für Testläufe ist `QT_QPA_PLATFORMTHEME=kde` deshalb
  Voraussetzung, nicht Zierde.
- Icon-Zustände: normal · „Vorschlag wartet" (Badge) · Fehlerzustand
  (Analyse-/Transkriptionsfehler, Tooltip nennt Ursache).
- Icons in v1 aus dem Breeze-Bestand (App- und Tray-Icon abgeleitet, Badge
  als Overlay gezeichnet) — keine eigene Grafikarbeit.
- **KNotification** bei: neuem Vorschlags-Paket, Volllauf-Mahnung,
  wiederholtem Fehler. Keine Benachrichtigung für Routineläufe.

## 11. Volllauf-Schutz

- Zwei Kriterien, beide einstellbar: **Anzahl** unexportierter Notizen
  (Default 200) ODER **Alter** der ältesten unexportierten Notiz (Default
  30 Tage).
- Bei Überschreiten: Tray-Mahnung + Benachrichtigung; der nächste Analyse-Lauf
  erzeugt bevorzugt Bündel (inkl. „Vermischtes", 7.3). **Nie** automatischer
  Export.

## 12. Transkription

- Job-Queue (`transcribe_jobs`), seriell abgearbeitet (eine GPU); überlebt
  Neustarts (Queue in der DB).
- **whisper.cpp** (Default): Vulkan-Build aus dem AUR (`whisper.cpp-vulkan`
  o. ä.; Paketname bei Umsetzung prüfen). Aufruf als Subprozess:
  Audio per `ffmpeg` nach 16-kHz-Mono-WAV (temporär), dann
  `whisper-cli -m <modell> -f <wav> -l de -oj` → JSON-Transkript.
  Modellgröße einstellbar (Default `small`, Auswahl tiny–large-v3; Download
  der GGML-Modelle beim ersten Gebrauch mit Fortschritt, Ablage unter
  `~/.local/share/denkzettel/models/`).
- **WhisperX (ROCm/GPU)**: konfigurierbarer Aufruf-Pfad, Subprozess mit
  `--language de`, JSON-Auswertung, ohne Diarisierung. **Vorbedingung**
  (Befund Schätzklausur 31.07.2026): Auf dem Entwicklungsrechner ist WhisperX noch nicht
  installiert — der PoC des RPG-Audio-Projekts lief mit whisper.cpp. Die
  Anbindung wird erst gebaut/abgenommen, wenn die Installation existiert
  (entsteht im RPG-Audio-Projekt); bis dahin ist whisper.cpp der einzige
  aktive Weg.
- Fehlerpfad: 2 Fehlversuche → Job pausiert, Tray-Fehlerzustand, Notiz bleibt
  als `audio`-Notiz ohne Transkript sichtbar/abspielbar (nichts geht verloren).

## 13. Einstellungen (Dialog)

Seitenliste gemäß Konzept: **KI-Provider** (Provider-Wahl, LLM- und
Embedding-Modell, Verbindung testen), **Analyse** (sofort/periodisch mit
Intervall/auf Abruf), **Export** (Vault-Pfad mit Ordner-Wahl, Volllauf-Schwellen
Anzahl + Tage, Bündel-Schwelle), **Sprachnotizen** (Backend whisper.cpp/
WhisperX, Modellgröße, WhisperX-Pfad), **Kürzel** (KKeySequenceWidget für
beide Shortcuts).

## 14. Fehlerbehandlung und Loop-Disziplin

Der periodische Analyse-Lauf ist ein Loop im Sinne der Loop-Konventionen:

- **goal met**: alle Notizen `analysiert`, Vorschläge erzeugt → Lauf endet.
- **budget**: ein Lauf verarbeitet max. 50 Notizen (Rest im nächsten Lauf).
- **stalled**: gleicher Fehler 2× → betroffene Notiz überspringen, melden.
- **needs a human**: alle Überführungen sind ohnehin bestätigungspflichtig.

Meldewege: Tray-Zustand + Tooltip (leise), KNotification (wichtig), Logdatei
`~/.local/share/denkzettel/denkzettel.log` (Details, mit Rotation).

## 15. Build, Abhängigkeiten, Paketierung

- **CMake** + ECM (Extra CMake Modules), C++20.
- Qt 6: Widgets, Sql, Network, Multimedia, **DBus** (nicht nur für die
  Einzelinstanz und die Dienst-Schnittstelle: das Erfassungsfenster fragt
  darüber KWin, ob diese Sitzung überhaupt weichzeichnet — 3.2, Punkt 9).
  KF6: KGlobalAccel,
  KConfig, KNotifications, KStatusNotifierItem, KWallet (Framework: KWallet),
  **KDBusAddons** (KDBusService/Einzelinstanz), **KWidgetsAddons**
  (KMessageWidget — Meldungen im Fenster; KMessageDialog samt
  KStandardGuiItem — Wächterdialog, Abschnitt 9), **KWindowSystem**
  (KWindowConfig — Fenstergröße über Sitzungen; die Position setzt ein
  Wayland-Client nicht selbst, siehe Abschnitt 3; **KWindowShadow** — der
  Schatten des Erfassungsfensters, Abschnitt 3.1; **KWindowEffects** — die
  beiden anderen Anmeldungen der Hülle, Weichzeichner und Kontrasteffekt,
  Abschnitt 3.1/3.2), **KSvg** (`FrameSvg` und
  `ImageSet` — die Hülle aus `dialogs/background` des Desktop-Themes),
  **KCoreAddons** (`KDirWatch` — die Wache auf `plasmarc`, über die ein
  Theme-Wechsel ein stehendes Fenster erreicht; warum nicht `KConfigWatcher`,
  steht in 3.2), **KColorScheme** (`KColorScheme::frameContrast()` — das
  Verhältnis, in dem die Bibliotheksliste ihre Trennlinien mischt, Abschnitt 9),
  KI18n (App-Sprache Deutsch;
  `i18n()`-Aufrufe sind KDE-Standardpraxis für alle sichtbaren Strings —
  keine Übersetzungs-Roadmap, nur Konvention). UI-Fließtexte (Platzhalter,
  Hinweise, Dialoge) sprechen den Nutzer in unpersönlicher Infinitivform an
  („Zum Lesen links eine Notiz auswählen.") — einmal app-weit festgelegt
  statt je Fenster (PO-Entscheidung 31.07.2026, Gestaltungsauftrag S8).
- **Mindestversionen:** Die allgemeine Untergrenze für ECM und die
  KF6-Komponenten liegt bei **6.0.0**. Eine Ausnahme trägt ihre Version selbst:
  **KColorScheme wird mit einem eigenen `find_package`-Aufruf und der
  Mindestversion 6.20 gesucht**, weil `KColorScheme::frameContrast()` erst dort
  hinzugekommen ist. Der Grund für den eigenen Aufruf ist gemessen (Issue #101,
  Kundenentscheidung 07.08.2026): Die eine Zahl `KF_MIN_VERSION` speist **beide**
  `find_package`-Aufrufe der Wurzel-`CMakeLists.txt`, so dass eine 6.20 an
  dieser Stelle die Untergrenze für ECM und alle zehn Komponenten mit anhöbe —
  für eine Distribution mit älterem KF6 ohne Not. Die Untergrenze steigt dort,
  wo die Funktion sitzt.
- Laufzeit-Abhängigkeiten: `ffmpeg` (Audio-Konvertierung), optional
  `whisper.cpp` (Vulkan) und `task` (Taskwarrior) — beides wird zur Laufzeit
  erkannt; fehlt eines, sind nur die betroffenen Funktionen deaktiviert
  (mit Hinweis in den Einstellungen), die App bleibt nutzbar.
- Paketierung: zunächst lokales `cmake --install` — mit
  `-DCMAKE_INSTALL_PREFIX=/usr`, denn nur dann landet der
  XDG-Autostart-Eintrag in `/etc/xdg/autostart`; unter dem
  CMake-Standard `/usr/local` liest ihn keine Plasma-Sitzung
  (Sprint-2-Befund, Issue #6). PKGBUILD/AUR nach Stabilisierung.

### 15.1 Versionsregeln und Kommandozeile (Issue #61)

- **Eine einzige Quelle:** die Nummer steht in `project(denkzettel VERSION …)`
  der Wurzel-`CMakeLists.txt` und sonst nirgends. Von dort reicht
  `src/CMakeLists.txt` sie als Übersetzungsdefinition `DENKZETTEL_VERSION` an
  den Code weiter, der sie in `KAboutData` einträgt. Eine zweite Kopie in einer
  Quelldatei wäre die, die unbemerkt veraltet.
- **Schema 0.x-SemVer**, solange die Anwendung vor 1.0 steht:
  - **MINOR** (`0.1.0` → `0.2.0`) mit **jeder Kundenabnahme**.
  - **PATCH** (`0.2.0` → `0.2.1`) für außerplanmäßige Behebungen zwischen zwei
    Abnahmen.
  - **Jede Schemamigration der Datenbank erzwingt mindestens MINOR** — auch
    wenn sonst nichts dazukommt. Ein Datenbestand, der nicht mehr zur
    Vorgängerversion passt, ist keine Kleinigkeit.
- **Der Tag ist das Siegel:** `vMAJOR.MINOR.PATCH` auf dem abgenommenen Stand.
  Erhöhung und Tag stehen im Sprint-Abschluss (`docs/scrum/PROZESS.md`,
  Takt 2) und folgen der Abnahme; sie gehen ihr nicht voraus.
- **Sichtbar wird die Nummer über `denkzetteld --version`** — Ausgabe
  `denkzettel <Nummer>`, Rückgabe 0, auch bei laufendem Dienst und ohne
  Sitzungsbus (Bedingung in 2.3). Ein Über-Dialog ist eine eigene Story (#87).
- **Unbekannte Schalter werden zurückgewiesen** (Rückgabe ≠ 0). Der
  argumentlose Start bleibt der Start des Dienstes — beide `Exec=`-Zeilen der
  Desktop-Datei rufen ohne Argument auf.

## 16. Teststrategie

- **Unit (QTest)**: Suchoperator-Parser, Clustering (mit synthetischen
  Vektoren), Prompt-/JSON-Schema-Verarbeitung (Provider gemockt),
  Export-Formatter (Sammelnotiz-Markdown), Taskwarrior-Kommandozeilen-Bau,
  Dateinamens-/Pfadlogik.
- **Integration**: Store-Schicht gegen echte SQLite (Tempfile), FTS-Trigger,
  Lösch-Transaktion inkl. Audio-Datei.
- **Manuell (Checkliste je Meilenstein)**: Shortcut-Weg unter Wayland,
  Fokusverhalten, Aufnahme mit echtem Mikrofon, Whisper-Durchlauf auf der
  7900 XTX, Export in einen **Test-Vault** (nie der echte), Taskwarrior
  gegen ein **eigenes `TASKDATA`-Testverzeichnis** (nie der Produktivbestand).
- **Migrationstest**: Sobald die erste reale Schema-Migration existiert,
  prüft ein Test das Upgrade einer Bestands-DB von Version n auf n+1.
- **Bedingung für Symbol- und Dialogzusicherungen** (entdeckt bei #60,
  bestätigt bei #66/#67 — DoD 4/B9): Tests, die Symbolnamen oder das
  Aussehen eines Meldungsdialogs zusichern, laufen mit
  `QT_QPA_PLATFORM=offscreen` **und `QT_QPA_PLATFORMTHEME=kde`. Ohne das
  Plattform-Thema** löst `QIcon::fromTheme()` nichts auf und liefert ein
  Symbol **ohne Namen** — die Zusicherung ist dann rot, ohne dass am Bau
  etwas fehlt —, und die Plattformintegration baut den Meldungsdialog nicht
  so, wie ein Sitzungsnutzer ihn sieht (Abschnitt 9). Das gilt derzeit für
  `shelltest`, `librarytest` und — seit der Hülle des Erfassungsfensters
  (3.1) — `capturetest`: Dessen Geometriezusicherungen messen Abstände, über
  die die Schrift entscheidet, und unter einer Ersatzschrift maßen sie etwas
  anderes. **Es ersetzt keine Plasma-Sitzung:** Der Bildnachweis am
  installierten Stand bleibt.
- **Was offscreen prinzipbedingt nicht zu belegen ist** (gemessen zu #55,
  nicht abgewogen): Der **Schatten** des Erfassungsfensters. Ohne Compositor
  gibt es niemanden, dem `KWindowShadow::create()` die Kacheln übergäbe — der
  Rückgabewert ist dort **immer** falsch, und das ist kein Fehler des Codes.
  `QWidget::grab()` zeigt ihn ebenfalls nie, weil er außerhalb des Widgets
  liegt. Ein offscreen entstandenes Bild ohne Schatten ist deshalb **kein
  Prüfbefund**. An seine Stelle treten zwei benannte Ersatzformen: im Test die
  Zusicherung, dass ein Schattenobjekt besteht und **seine Kacheln die des
  Desktop-Themes sind**, und in der Abnahme ein Bild aus der Plasma-Sitzung.
  Dass der Schatten nach **jedem** Neuzeigen wieder daliegt (3.2, Punkt 5),
  ist von keiner der beiden Formen gedeckt und gehört in die manuelle
  Checkliste.
- **Und der Weichzeichner, schärfer als der Schatten** (gemessen zu #83): Ihn
  zeigt auch eine **Fensteraufnahme** nicht. `spectacle -a` liefert wie
  `QWidget::grab()` die eigene Fläche des Fensters; was hinter der Hülle liegt,
  steht in beiden nicht darin. Belegbar ist er allein an einer Aufnahme des
  **zusammengesetzten Bildes** über einem bekannten Muster, im A/B-Vergleich
  gegen einen Lauf ohne Anmeldung. `enableBlurBehind()` und
  `enableBackgroundContrast()` sind `void`, und der eine verfügbare
  Rückgabewert — `isEffectAvailable()` — lügt vor der ersten Anmeldung (3.2,
  Punkt 9). Ob der Kontrasteffekt auf dem geprüften Stand überhaupt **wirkt**,
  ist unvermessen: `org_kde_kwin_contrast_manager` steht nicht mehr in der
  Globalenliste des Compositors. Der Aufruf ist belegt, die Wirkung nicht.
- **Was offscreen ebenfalls nicht auftritt: der Sprung des
  Bildpunktverhältnisses** (3.2, Punkt 7). Offscreen bleibt es bei einem
  `Resize`, und der einzige bekannte Fehlermechanismus ist damit systematisch
  unsichtbar. Ein Beleg dazu kommt aus der angemeldeten Sitzung und **ohne**
  `QT_SCALE_FACTOR` — unter Wayland multipliziert die Variable mit der
  Sitzungsskalierung (1 → 1,6; 1,6 → 2,56), offscreen nicht.
- **Ein Bildvergleich ganzer Fenster über Plattformgrenzen hinweg ist kein
  Prüfmittel** (gemessen zu #83): Die **Hülle** ist offscreen und unter Wayland
  bei gleichem Bildpunktverhältnis byteweise gleich, ein **gegrabbtes Fenster**
  nicht — die Schriftrasterung weicht ab (1.587 von 154.440 Bildpunkten,
  sämtlich im Textbereich; Ursache Fontconfig, nicht KSvg).
- **Ein Nachbau der Skalierung im selben Prozess ist keine Prüflage** (entdeckt
  zu #101 bei L9 — DoD 4/B9): Die Liste durch einen Maler auf ein Bild mit
  `devicePixelRatio` 1,6 zu zeichnen, misst etwas anderes als eine Sitzung unter
  `QT_SCALE_FACTOR=1.6`. *Gemessen:* Der Nachbau zeigte den Fehler bei **1,25**
  und **bei 1,6 nicht** — dort, wo die echte Skalierung ihn zeigt. Die
  Zeilenhöhen einer skalierten Sitzung sind nicht die einer unskalierten; der
  Nachbau maß Zeilenlagen, die es gar nicht gibt. **Ein Prüfsatz, der gerade
  dort besteht, wo der Fehler sitzt, ist schlimmer als keiner.** Die taugliche
  Form ist eine **eigene Prüflage**: derselbe Prüfsatz ein zweites Mal in
  `tests/CMakeLists.txt` angemeldet, mit `QT_SCALE_FACTOR` in der Umgebung und
  **nur** den Prüffunktionen, die unter Skalierung etwas aussagen
  (`librarytestskaliert`). Die Bildpunktzusicherungen aus AK 1 bis AK 3 messen
  in logischen Punkten und gehören nicht hinein — unter 1,6 prüften sie die
  Skalierung statt der Linie.
  **Die Szene entscheidet mit:** Der Fehler hängt an der Rasterlage. Auch der
  erste Lauf mit echter Skalierung war grün, weil die Szene nur vier Linien in
  drei Lagen kannte; sie trägt jetzt eine Gruppe mit **acht** Notizen, die alle
  fünf Phasen durchläuft (`storedALongGroup()`).
- **Zustände, die im Prüfprozess selbst nicht herstellbar sind, brauchen einen
  eigenen Prozess** (entdeckt zu #55, AK 8): Das Fenster ohne Desktop-Theme
  lässt sich nicht durch einen erfundenen Theme-Namen erzeugen — KSvg fällt
  dabei auf `default` zurück, und der Test prüfte einen Fall, der gar nicht
  eintreten kann. `capturetest` startet sich für diese Zusicherung mit
  beschnittenem `XDG_DATA_DIRS` selbst neu.
- **Keine Zusicherung hängt an einem Namen, den nur diese Maschine kennt**
  (entdeckt zu #55, DoD 4/B9). Der Prüfsatz zur Hülle hält zwei Desktop-Themes
  mit verschiedenem Rand gegeneinander — und **ein solches Paar gibt es nicht
  überall**: Die drei Themes des offiziellen KDE-Bestands (`default`,
  `breeze-dark`, `breeze-light`) tragen sämtlich 4 px; jedes breitere Theme auf
  der Entwicklungsmaschine stammt aus einem CachyOS-Paket. Ein Bauplatz, der
  nur die KF6-Teile dieses Projekts installiert, hat **gar kein**
  Desktop-Theme — `ksvg` hängt nicht an `libplasma`. Daraus zwei Quellen mit
  verschiedener Aufgabe, und keine ersetzt die andere:
  **`tests/themes/`** liefert zwei eigene Themes, damit die Zusicherung überall
  läuft; sie belegen aber nur, dass der Code *unser* SVG liest. Der Lauf gegen
  **installierte** Themes belegt das Echte, wird zur Laufzeit **gemessen statt
  benannt** und übersprungen, wenn kein Paar da ist — mit benanntem Grund.
- KI-Qualität (Klassifikation/Clustering) wird nicht automatisiert getestet —
  der Vorschlags-Review ist die menschliche Kontrollinstanz.

## 17. Meilensteine

1. **M1 Capture-Kern**: Daemon, Tray, KGlobalAccel, Text-Capture, SQLite-Store.
   *Nutzbar: Gedanken festhalten.*
2. **M2 Bibliothek + Suche**: Fenster, Liste, Detail (lesen/bearbeiten/löschen),
   FTS + Operatoren.
3. **M3 KI-Basis**: Provider-Abstraktion (Ollama), Klassifikation + Tags,
   Kategorien-Sidebar, Einstellungen-Seiten KI/Analyse.
4. **M4 Sprachnotizen**: Aufnahmefenster, Queue, whisper.cpp-Backend,
   Player in der Bibliothek; WhisperX-Anbindung.
5. **M5 Vorschläge**: Embeddings + Clustering, Bündel- und Task-Vorschläge,
   Review-UI, Obsidian- und Taskwarrior-Ausführung, Volllauf-Schutz,
   Voll-Export.
6. **M6 Provider-Ausbau**: openrouter, OpenAI per API-Key (gemäß 7.5), KWallet.
7. **M7 Politur**: Icon-Zustände, Benachrichtigungs-Feinschliff, Logging,
   PKGBUILD.

Jeder Meilenstein endet mit der manuellen Checkliste (16) und einem
lauffähigen Stand.
