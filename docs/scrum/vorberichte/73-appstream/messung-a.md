# Vorprüfung #73 — Messung Bearbeiter A (`denkzettel-dev`)

**Gegenstand:** Issue #73, „appstreamtest prüft nichts: Denkzettel liefert keine
AppStream-Metainfo" · **Datum:** 04.08.2026, Ganymed · **Quellstand:** `main` @
`6acc87e` · **Belege:** `messungen/`, Sonden in `sonden/`, wiederholbar über
`bash docs/scrum/vorberichte/73-appstream/pruefen.sh`

Dieser Bericht trägt die Felder **1, 2, 4, 5** und **6**. **Feld 3 (Ready-Urteil)
fällt der Scrum Master.**

**Stand der Werkzeuge** (B17 — eine Aussage gilt für einen Stand): appstream
1.1.5-1.1 (`appstreamcli` 1.1.5), extra-cmake-modules 6.28.0-1, cmake 4.4.2.
Beleg: `messungen/m1-appstreamtest-quelle.txt`, `m20-stand-und-kollision.txt`.

**Was `pruefen.sh` wiederholt:** M1, M7, M9, M10, M12/M13, M14, M18/M19. Die
übrigen Ausgaben (M2–M6, M11, M15–M17, M20, M21) sind einmalige Ablesungen am
Ist-Stand — Manifest-Zeitstempel, CI-Datei, Zuständigkeiten — und stehen
unverändert im Ordner.

---

## Feld 1 — Dateimenge (am Code vermessen, Notation nach B13)

| | **#73** — AppStream-Metainfo und ein Test, der sie wirklich prüft |
|---|---|
| **Issue** | **#73** (`epic:M7`, `typ:tech`) |
| **Zweig** | `story/73-appstream` |
| **Quellen & Tests** | **Neu:** `desktop/org.denkzettel.Denkzettel.metainfo.xml` (~50 Zeilen; der Ordner ist gesetzt — dort liegt schon `org.denkzettel.Denkzettel.desktop`, und die Wurzel installiert von dort, `CMakeLists.txt:39` und `:49`).<br>`tests/installtest.cmake` (58 Zeilen) — **Erweiterung um die Metainfo-Prüfung empfohlen** statt eines zweiten Tests, siehe F8. Alternativ **neu:** `tests/metainfotest.cmake` plus ein `add_test`-Block in `tests/CMakeLists.txt` (184 Zeilen; `installtest` steht ab `:177` am Ende, dort wird angehängt).<br>`CMakeLists.txt` (131 Zeilen) — **eine** `install(FILES … DESTINATION ${KDE_INSTALL_METAINFODIR})`-Zeile neben die beiden vorhandenen `install(FILES desktop/…)`; bei einem eigenen Test zusätzlich die `add_test`-Argumente in `tests/CMakeLists.txt`. |
| **Build** | Kein neues Paket, keine neue `find_package`-Zeile. Gemessen: `KDE_INSTALL_METAINFODIR` steht bereits über `include(KDEInstallDirs)` bereit, `KDE_INSTALL_FULL_METAINFODIR` = `/usr/share/metainfo` — **auch ohne `-DCMAKE_INSTALL_PREFIX=/usr`**, weil KDEInstallDirs den Qt-Prefix übernimmt (`m7-metainfodir.txt`). `appstreamcli` liegt auf Ganymed und wird im CI-Container über das Paket `appstream` mitinstalliert (`m5-ci.txt`, Zeile `… breeze-icons appstream clazy`). |
| **Belege & Prüfmittel** | `docs/scrum/reviews/sprint-NN-s73-appstream/` — neu anzulegen, mit der Mutationsprobe aus AK 5. **Wiederverwendbar, nicht neu zu erfinden:** `sonden/metainfo/org.denkzettel.Denkzettel.metainfo.xml` dieser Vorprüfung ist ein vollständiger, netzvalidierter Entwurf; `sonden/metainfotest/` ist der lauffähige Bauplan des Tests aus AK 4 samt Mutationsprobe. |
| **Fachliche Quellen** | **SPEC.md kennt AppStream nicht** — kein Treffer für `appstream\|metainfo\|appdata` in `SPEC.md`, `KONZEPT.md`, `README.md`, `PROZESS.md`, `CLAUDE.md` (`m15-fachliche-quellen.txt`). Nach DoD 4 zieht die Story die SPEC nach, sachlich neben SPEC 2.5 (Desktop-Datei und Autostart-Eintrag). `CHANGELOG.md` (79 Zeilen) ist **Quelle** für `releases`, nicht Ziel: der Changelog-Eintrag zu #73 entsteht erst im Sprint-Abschluss, Punkt 9. |
| **Ausdrücklich nicht** | Ganz `src/` (die Story fasst keinen C++-Code an), `tests/*.cpp` — **alle**, einschließlich der fünf Bildläufer —, `desktop/org.denkzettel.Denkzettel.desktop` (die `.desktop`-Datei bleibt, wie sie ist; die Metainfo verweist nur auf sie), `wireframes/`, `docs/bilder/` (die beiden Bilder werden **verlinkt**, nicht verändert), `.github/workflows/ci.yml` (der vorhandene `ctest`-Schritt trägt den neuen Test von selbst), sowie **`docs/scrum/PROZESS.md`** — siehe F10. |

### Kollisionsfläche gegen #83 — **läuft daneben, ohne Berührung**

| Datei | #83 (`vorberichte/83-native-huelle/messung-a.md`, Feld 1) | #73 | Berührung |
|---|---|---|---|
| `src/capture/capturewindow.{h,cpp}` | ganz | — | keine |
| `tests/capturetest.cpp` (669 Z.) | vier Zusicherungen + eine neue | — | keine |
| `tests/captureshots.cpp` (225 Z.) | Bildreihe | — | keine |
| `CMakeLists.txt` (131 Z.) | „**Nichts**" (Feld 1, Zeile Build) | +1 `install`-Zeile | keine |
| `tests/CMakeLists.txt` (184 Z.) | „**Nichts**", *Ausnahme:* ein eigener Läufer für den Weichzeichner-Beleg | +1 `add_test`-Block am Ende — **entfällt**, wenn `installtest.cmake` erweitert wird (F8) | **die einzige mögliche**, und beide Seiten hängen sie ans Dateiende |
| `desktop/`, `docs/bilder/`, `CHANGELOG.md` | — | ja | keine |

**Urteil:** Die beiden Stories teilen **keine Quelldatei und keinen Test**. Die
einzige denkbare Überschneidung ist `tests/CMakeLists.txt`, und sie tritt nur
ein, wenn #83 seine benannte Ausnahme zieht *und* #73 einen eigenen Test statt
der Erweiterung wählt — zwei Anhängungen am Dateiende, die Git ohne Konflikt
mischt. #73 fasst außerdem **keine** der Bildreihen an, an denen sich die
Capture-Stränge sonst gegenseitig die Belege überschreiben. Parallelbetrieb in
eigenen Worktrees ist unbedenklich.

---

## Feld 2 — Gemessene Fallen (die Zeilen für den Spawn-Auftrag)

**F1 — Der ECM-`appstreamtest` ist in vier von fünf Lagen grün, und nur eine
davon prüft etwas.** Direkt gefahren (`m14-ecm-appstreamtest-faelle.txt`):
Manifest fehlt → `Not installed yet, skipping`, rc=0 · Manifest ohne Metainfo
(Lage heute) → rc=0, keine Ausgabe · Manifest nennt eine Metainfo, die dort
nicht liegt → CMake-**Warnung** `Could not find …`, **rc=0** · gültige Metainfo
→ validiert, rc=0 · beschädigte Metainfo → rc=1. Das ganze Skript ist 39 Zeilen
lang (`m1-appstreamtest-quelle.txt`); die entscheidende Stelle ist `:25` —
`if(metadatafiles)`. Ist die Liste leer, wird der `execute_process`-Block
übersprungen und der Test endet grün.

**F2 — Die Metainfo allein macht ihn nicht scharf, und das ist am Zeitstempel
belegt.** `build/install_manifest.txt` (16:02:39.798) und
`build/tests/installtest-root` (16:02:39.797) tragen dieselbe Sekunde auf drei
Nachkommastellen (`m11-manifest-wechselwirkung.txt`) — das Manifest stammt vom
**DESTDIR-Lauf des `installtest`**, nicht von einer echten Installation nach
`/usr`. Es führt trotzdem `/usr/…`-Pfade, weil `cmake --install` das
DESTDIR-Präfix nicht ins Manifest schreibt (in der Sonde nachgemessen,
`m12-destdir-test.txt`). Nach #73 steht dort `/usr/share/metainfo/…`, ohne dass
die Datei je nach `/usr` gelangt sein muss → Fall 3 aus F1: Warnung, grün. **Wer
AK 6 über den ECM-Test abhaken will, hakt einen übersprungenen Test ab.**

**F3 — `appstreamcli validate` prüft von den elf geforderten Feldern genau eines
scharf.** Gemessen durch einzelnes Herausnehmen (`m9-pflichtfelder-und-mutation.txt`):
`metadata_license` fehlt → **Fehler, rc=3**. `categories` als ganzer Block
entfernt → **keine Meldung, rc=0**. `developer` entfernt → nur `I:
developer-info-missing`, rc=0. `content_rating` entfernt → nur `I:
content-rating-missing`, rc=0. AK 2 verlangt alle vier. **Der projekteigene Test
braucht deshalb eine eigene Feldprüfung** (`file(READ …)` + `MATCHES`, wie
`installtest.cmake:37–57` es für die Desktop-Aktion schon tut) — sonst ist AK 2
formal abgehakt und tatsächlich ungeprüft.

**F4 — Warnungen färben rot, Pedantisches nicht.** Schon eine einzelne Warnung
liefert rc=3 — `cid-desktopapp-is-not-rdns` (`m9-pflichtfelder-und-mutation.txt`,
letzte Blöcke) und `desktop-file-not-found` (`m19-launchable-quervergleich.txt`).
Der pedantische Hinweis `cid-contains-uppercase-letter` — unvermeidlich, weil die
Komponenten-Id dem bestehenden Dateinamen `org.denkzettel.Denkzettel.desktop`
folgen muss — bleibt rc=0, auch mit `--pedantic`. **Die Id muss nicht umbenannt
werden.**

**F5 — `validate` sieht die `.desktop`-Datei nicht, `validate-tree` schon.**
Mit einer erfundenen `launchable`-desktop-id: `appstreamcli validate` → rc=0,
keine Meldung; `appstreamcli validate-tree` über denselben Staging-Baum → rc=3,
`W: desktop-file-not-found` (`m19-launchable-quervergleich.txt`). **Aber
`validate-tree` teilt die Blindstelle des ECM-Tests:** über einen Baum ohne jede
Metainfo meldet es „Validierung war erfolgreich", rc=0 (`m18-validate-tree.txt`).
Beide Werkzeuge brauchen die **Existenzprüfung davor**, so wie
`installtest.cmake:24–29` sie führt.

**F6 — Bildschirmfotos werden nur ohne `--no-net` geprüft.** Mit erfundener
Bild-URL: ohne `--no-net` → rc=3, `screenshot-image-not-found`; mit `--no-net`
→ rc=0 (`m10-screenshots-netz.txt`). Der ECM-Test benutzt `--no-net` fest
(`m1-…`, Zeile 27 des ECM-Skripts). Der Entwurf mit den **echten** Roh-URLs validiert über Netz
auf Anhieb (rc=0); beide URLs antworten mit 200, die Bilder stehen auf
`origin/main` (`m10-…`, `m6-projektquellen.txt`), Maße 1200×324 und 2000×1520.
Die vom Kunden bewusst getragene Gefahr — „bricht in jedem Software-Zentrum,
ohne dass ein Test es merkt" — ist damit **messbar, aber nur mit Netz**. Das ist
eine Entscheidung, keine Messung (Feld 6, Frage 1).

**F7 — Der Nachweisweg aus AK 4 trägt, und er ist gebaut.**
`sonden/metainfotest/` ist ein eigenständiges CMake-Projekt nach dem Muster von
`tests/installtest.cmake`: `install(FILES … DESTINATION
${KDE_INSTALL_METAINFODIR})`, dann DESTDIR-Installation in den Bauordner,
Existenzprüfung, `appstreamcli validate --no-net` am Installationsort.
Gemessen: grün in **0,02 s**, ohne Root, ohne Sitzung; drei Mutationen färben
rot — `metadata_license` entfernt, XML-Wurzel unvollständig, Datei gelöscht
(`m12-destdir-test.txt`, `m13-mutationsprobe.txt`). **Der Bauplan der Story ist
damit fertig; sie muss ihn nur an die echten Ziele hängen.**

**F8 — Zwei DESTDIR-Läufe desselben Bauplatzes sind eine unnötige Flanke.**
`installtest` und ein zweiter Metainfo-Test würden beide
`build/install_manifest.txt` schreiben und beide `build/` vollständig
installieren; unter `ctest -j` liefen sie gleichzeitig in dieselbe Datei. Die
Staging-Wurzel des `installtest` enthält heute fünf Dateien, die Metainfo käme
als sechste dazu (`m11-…`). **Empfehlung: die Prüfung in `installtest.cmake`
aufnehmen.** AK 4 verlangt „nach dem Muster von `tests/installtest.cmake`" und
lässt beides zu; der PO entscheidet (Feld 6, Frage 3).

**F9 — `appstreamcli` selbst ist ehrlich, der Rahmen darum nicht.** Auf eine
nicht vorhandene Datei antwortet es mit rc=3, auf eine leere Argumentliste mit
rc=1 (`m21-leerlauf-rueckgabewerte.txt`). Die Blindstelle liegt vollständig im
ECM-Wrapper. **Kein Grund, dem Werkzeug zu misstrauen — der Grund, dem
umgebenden Skript zu misstrauen.**

**F10 — AK 3 verlangt eine Änderung außerhalb der Dev-Dateimenge.** Die
Fortschreibungsregel gehört in `docs/scrum/PROZESS.md`, Sprint-Abschluss
(Punkt 9/10); die Pflege der Prozess-Doku unter `docs/scrum/` liegt laut
`.claude/agents/scrum-master.md:8` beim Scrum Master
(`m17-zustaendigkeit-und-ignore.txt`). Sachstand dazu, gemessen: Punkt 10 führt
die Aussetzung bis #61 bereits im Text (`m16-sprint-abschluss.txt`), und
`CHANGELOG.md` kennt genau **eine** veröffentlichte Version, `[0.1.0] —
2026-08-02`; alles Neuere steht unter `[Unveröffentlicht]`
(`m6-projektquellen.txt`). **`releases` kann heute nur diesen einen Eintrag
tragen** — mehr wäre erfunden.

---

## Feld 4 — Prüfmittel je Akzeptanzkriterium

| AK | Prüfmittel | Urteil |
|---|---|---|
| **1** Datei im Repo, Installation nach `${KDE_INSTALL_METAINFODIR}` | `git ls-files desktop/` (nach der DoR-Regel: ein Dateiname ist erst ein Prüfmittel, wenn `git ls-files` ihn zeigt) · DESTDIR-Lauf + `EXISTS "${STAGING_DIR}${KDE_INSTALL_FULL_METAINFODIR}/…"` — vorgeführt in `m12-destdir-test.txt` | prüfbar |
| **2** elf Pflichtfelder | `appstreamcli validate` deckt **`metadata_license`** scharf ab (rc=3) und `id`/`name`/`summary`/`description`/`launchable`/`project_license`/`url` über die Schema- und Wohlgeformtheitsprüfung. **`categories`, `developer`, `content_rating` deckt es nicht** (F3) → eigene Feldprüfung im Testskript nötig, Bauart `installtest.cmake:37–57` | prüfbar **erst mit der Feldprüfung**; ohne sie ungeprüft |
| **3** `releases` aus `CHANGELOG.md`, Fortschreibungsregel mit Aussetzung von Punkt 10 | Maschinell nur der Abgleich `<release version="0.1.0" date="2026-08-02"/>` gegen `CHANGELOG.md` und `project(denkzettel VERSION 0.1.0)`. **Dass die Regel künftig befolgt wird, prüft kein Lauf** — das ist eine Textänderung in `PROZESS.md` und wird gelesen, nicht gemessen | teils prüfbar; Grenze ausgesprochen |
| **4** projekteigener Test über DESTDIR, ohne Root, in `ctest`, auch im Container | `ctest --test-dir build -R installtest` (bzw. `-R metainfotest`) — vorgeführt, 0,02 s, ohne Root. **„Auch im Container" belegt nur der CI-Lauf**; lokal messbar ist allein die Voraussetzung: `appstream` steht in der `pacman -Syu`-Zeile von `ci.yml` (`m5-ci.txt`) | prüfbar bis auf den Containerteil |
| **5** Mutationsprobe wird rot, Beleg versioniert | Drei Mutationen vorgeführt, alle rot (`m13-mutationsprobe.txt`). Beleg nach `docs/scrum/reviews/sprint-NN-s73-appstream/` (B7) | prüfbar |
| **6** der automatische Lauf installiert und validiert tatsächlich und bleibt grün | `gh run list --commit $(git rev-parse HEAD) …` **am Lauf des eigenen Commits** (B18). Setzt einen Push voraus | prüfbar, **aber nicht durch den Dev-Agenten** — siehe unten |

**Zusätzlich, weil DoD 2 die Story erreicht:** Sie hat kein Fenster, aber einen
installierten Stand. Hauptweg am `/usr`-Stand, vom PO getaktet:
`ls -l /usr/share/metainfo/org.denkzettel.Denkzettel.metainfo.xml` ·
`appstreamcli validate --no-net /usr/share/metainfo/…` ·
`ctest --test-dir build -R appstreamtest` — **dieser eine Lauf ist der einzige,
in dem der ECM-Test etwas prüft** (F1, Fall 4).

### Was ein Agent an dieser Story nicht prüfen kann

1. **Den CI-Lauf (AK 4 „im Container", AK 6 ganz).** Er entsteht erst durch
   einen Push, und die Regelwerke widersprechen sich an genau dieser Stelle:
   `CLAUDE.md` sagt „gepusht wird nach jedem abgeschlossenen Arbeitsblock, ohne
   Rückfrage", `.claude/agents/denkzettel-dev.md` sagt „niemals pushen — Push
   entscheidet der Product Owner". **Melden statt raten** (Feld 6, Frage 2).
2. **Ob ein Software-Zentrum den Eintrag anzeigt und die Bilder lädt.**
   `appstreamcli validate` ist die Grenze; Discover oder GNOME Software liest
   den zusammengeführten Systemkatalog, den diese Story nicht erzeugt.
3. **Ob die Roh-URLs morgen noch stehen.** Messbar ist der heutige Zustand
   (200, `m10-…`). Die vom Kunden getragene Bindung an den Zweignamen `main`
   ist eine Dauerbedingung, kein Prüfergebnis.
4. **B21/DoD 3 ist hier nicht einschlägig.** Kein Akzeptanzkriterium behauptet
   etwas über Hülle, Rundung, Kontur, Schatten, Dekoration oder
   Durchsichtigkeit; ein Sitzungsbild ist kein Prüfmittel dieser Story. Die
   Belege sind Terminalausgaben.

---

## Feld 5 — Größenklasse: **`size:s`**

Gemessen an der Definition in `PROZESS.md` („läuft nebenher — wenige Dateien,
kein neuer Prüfweg"):

- **Wenige Dateien: ja.** Eine neue XML-Datei, eine `install`-Zeile, eine
  erweiterte oder eine neue `.cmake`-Prüfdatei. Kein C++, kein Qt, kein KF6, keine
  neue Abhängigkeit (`m7-metainfodir.txt`, `m5-ci.txt`).
- **Kein neuer Prüfweg: im Ergebnis ja.** Der Weg ist der bestehende von
  `tests/installtest.cmake` — DESTDIR in den Bauordner, dann prüfen. Er ist in
  dieser Vorprüfung **gebaut und gefahren** (`sonden/metainfotest/`, 0,02 s,
  drei Mutationen rot). Die Story übernimmt einen fertigen Bauplan, sie erfindet
  keinen.
- **Der Inhalt ist gemessen billig:** Der vollständige Entwurf validiert auf
  Anhieb, mit Netz wie ohne (`m8-entwurf-validierung.txt`, `m10-…`).
- **Neben einer `size:l` tragbar:** null geteilte Quelldateien mit #83, keine
  gemeinsame Bildreihe (Feld 1).

**Was die Klasse auf `m` heben würde — beides vor dem Ziehen zu klären:**

1. Wenn die **Netzprüfung der Bildschirmfotos** verlangt wird (Feld 6, Frage 1).
   Dann kommt ein zweiter, netzabhängiger Prüfweg dazu, samt der Frage, was bei
   ausgefallenem Netz geschieht — eine Wache, die bei GitHub-Störung rot wird,
   ist eine neue Sorte Problem.
2. Wenn der **Dev auch die Fortschreibungsregel in `PROZESS.md`** schreiben soll
   (F10). Das ist Prozessarbeit in fremder Dateimenge und lässt sich nicht
   testen, nur begründen.

Bleiben beide beim PO bzw. Scrum Master, ist die Story `s`.

---

## Feld 6 — Offene Fragen an PO oder Kunde

1. **`--no-net` oder Netzprüfung?** Mit `--no-net` bleibt der Testlauf
   netzunabhängig und containertauglich, und eine tote Bild-URL fällt nie auf —
   genau der Bruch, den der Kunde am 04.08.2026 bewusst in Kauf genommen hat.
   Ohne `--no-net` wird er rot (gemessen, `m10-…`), dafür hängt jeder `ctest`-Lauf
   an GitHub. **Ein dritter Weg wäre ein getrennter, nicht in `ctest`
   hängender Prüfschritt** — dann bleibt die Sprint-Prüfung netzfrei und die
   Bild-URLs werden trotzdem einmal je Sprint angefasst. Entscheidung PO/Kunde.
2. **Darf der Dev-Agent für AK 6 pushen?** `CLAUDE.md` und
   `.claude/agents/denkzettel-dev.md` widersprechen sich (Feld 4). Ohne Klärung
   ist AK 6 vom Bearbeiter nicht erfüllbar und muss vom PO nachgeführt werden.
3. **Ein zweiter Test oder `installtest.cmake` erweitern?** Empfehlung:
   erweitern (F8). AK 4 lässt beides zu, und die Entscheidung berührt die
   Kollisionsfläche gegen #83 (Feld 1).
4. **`<developer id="…">` — welcher Wert?** Der Entwurf trägt `de.hnsstrk`; das
   ist **von mir gesetzt und nicht abgestimmt**. AppStream erwartet eine
   rDNS-artige Kennung. Ebenso zu bestätigen: `<url type="homepage">` zeigt auf
   `https://github.com/hnsstrk/denkzettel` — soll das die Homepage sein oder nur
   der Bugtracker-Eintrag?
5. **`content_rating type="oars-1.1"` leer** heißt „nichts Bedenkliches". Für ein
   Notizwerkzeug richtig, aber es ist eine Aussage über das Produkt und gehört
   bestätigt.

---

## Was ich nicht klären konnte

- **Der Container-Lauf.** Ich habe die Voraussetzung gemessen (`appstream` liegt
  in der Paketliste von `ci.yml`), nicht den Lauf. Ein Push kam nicht in Frage.
- **Der `/usr`-Stand.** Nach Auftrag und Sprint-3-Mangel M1 wird nicht
  installiert; die Aussage zu F1 Fall 4 ist am DESTDIR-Baum gemessen, nicht an
  `/usr`. Für DoD 2 bleibt sie zu wiederholen, wenn der PO die Installation
  taktet.
- **Der volle Projektbau.** Alle Messungen liefen in einem eigenen Bauplatz
  (`build/` neben dieser Datei) oder lesend am gemeinsamen `build/`. Der
  gemeinsame Bauplatz ist **nicht** angefasst worden — dort arbeiten andere
  Stränge.
