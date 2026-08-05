# Hauptwege der sieben Stories am installierten Stand

**Zweck:** Takt 1, Punkt 1 des Sprint-Abschlusses verlangt, dass der Endstand
**einmal** nach `/usr` installiert und **der Hauptweg jeder Story** daran
ausgeführt wird — mit Belegform (Terminalausgabe, Journalauszug oder Bild).

**Warum diese Datei existiert:** Die DoD-Prüfung zu Sprint 8 hat als Mangel M3
festgestellt, dass die vorhandene Befehlsliste (`sprint-08-s61-versionsanzeige/bericht.md`
§4) sorgfältig gearbeitet ist und am laufenden Dienst nachgeprüft wurde — aber
**nur den Hauptweg von #61 führt.** Sechs weitere fehlten. Hier stehen alle
sieben.

**Stand:** 05.08.2026, Endstand der Sprints 7 und 8.

---

## 0. Voraussetzungen — sonst prüft der Lauf den falschen Stand

```bash
# Der Bildschirm muss entsperrt sein: bei gesperrter Sitzung liefert
# spectacle -f ein schwarzes Bild mit Rückgabe 0, und der Lauf meldet
# "das Fenster hebt sich nirgends ab" (Fall 8 in denkzettel-dev.md).
loginctl show-session "$XDG_SESSION_ID" -p LockedHint -p Type -p Active

# Skalierung des Kunden, für alle Bildbelege (DoD 3)
#   erwartet: Fensterverhältnis 1,6 — ohne QT_SCALE_FACTOR, der multipliziert
```

## 1. Installieren

```bash
pkexec /usr/bin/cmake --install /home/hnsstrk/Projekte/denkzettel/build
```

## 2. Installieren heißt nicht laufen (B16)

Ein laufender Dienst hält nach `cmake --install` die **gelöschte** alte
Binärdatei weiter und zeigt das an nichts.

```bash
pkill -x denkzetteld
setsid /usr/bin/denkzetteld >/dev/null 2>&1 &
sleep 2
readlink /proc/$(pgrep -x denkzetteld)/exe
#   erwartet: /usr/bin/denkzetteld — und NICHT auf "(deleted)" endend
```

**Ohne diesen Beleg prüft die Abnahme den Stand des vorigen Sprints.**

---

## 3. Die sieben Hauptwege

### #61 — Versionsanzeige *(Belegform: Terminalausgabe)*

```bash
/usr/bin/denkzetteld --version
#   erwartet: "denkzettel <Nummer>", Rückgabe 0 — der Anwendungsname,
#   NICHT "Daemon <Nummer>" (das wäre der Busname, AK 2)

busctl --user list | grep denkzettel
#   erwartet: org.denkzettel.Daemon   (SPEC 2.3, AK 4)

gdbus call --session --dest org.kde.kglobalaccel --object-path /kglobalaccel \
  --method org.kde.KGlobalAccel.allMainComponents | tr ',' '\n' | grep -i denkzettel
#   erwartet: org.denkzettel.Denkzettel.desktop   (SPEC 2.4, AK 4)
#   NICHT org.kde.denkzettel.desktop

gdbus call --session --dest org.kde.kglobalaccel \
  --object-path /component/org_denkzettel_Denkzettel_desktop \
  --method org.kde.kglobalaccel.Component.allShortcutInfos
#   erwartet: 268435534 = 0x1000004E = Meta+N   (AK 5)

/usr/bin/denkzetteld --unbekannt ; echo "Rückgabe $?"
#   erwartet: Rückgabe != 0 und eine Fehlermeldung   (AK 6)
```

**Der Teil, den kein Agent führen kann:** `Meta+N` **tatsächlich drücken**. Die
Zahl oben belegt den Eintrag, nicht die Wirkung — unter Wayland kann ein Prozess
keinen Tastendruck auslösen.

### #83 — Hülle als native Plasma-Überlagerung *(Belegform: Bild aus der Sitzung, B21)*

```bash
qdbus6 org.denkzettel.Daemon /Daemon ShowCapture
```

Am Bild zu sehen und zu beurteilen:

- Runde Ecken, Kontur, Schatten — **aus dem Desktop-Theme**, nicht nachgebaut.
- **Neben KRunner gestellt** (AK 12): Sieht das aus wie ein Fenster von Plasma?
  *Das Urteil fällt der Kunde; kein Prüfmittel dieses Projekts kann es fällen —
  „die Frage stellt nur, wer das Fenster neben andere Fenster stellt und
  hinsieht."*
- Der Grund hinter dem Fenster ist **weichgezeichnet**.
- **Nicht enthalten und kein Befund:** derselbe Schatten wie bei dekorierten
  Fenstern. Breeze rechnet ihn selbst; unser Weg ist der der Überlagerungen.

### #85 — Lesbarkeit unter fremden Desktop-Themes *(Belegform: Bild aus der Sitzung)*

Der Kunde fährt `default` (Rückfall) — dort ändert sich nichts. Der Nachweis
braucht ein Theme mit eigener `colors`-Datei:

```bash
# Ein anderes Desktop-Theme setzen, Fenster öffnen, hinsehen, zurückstellen.
# Erwartet: Notiztext und die gedämpften Texte tragen die Farbe DES THEMES.
qdbus6 org.denkzettel.Daemon /Daemon ShowCapture
```

**Nicht behauptet:** dass die gedämpfte Klasse damit lesbar wird (unter
`breeze-light` erreicht keine Quelle 4,5 : 1 — das ist **#84**), und unter den
drei Themes, die den Kontrasteffekt anfordern, sichert die Story gar nichts zu:
Der Effekt ist auf diesem KWin **nicht geladen** (SPEC 3.2 Punkt 10).

### #71 — Klick auf angeschnittene Zeile *(Belegform: Blick des Kunden)*

```bash
qdbus6 org.denkzettel.Daemon /Daemon ShowLibrary
```

Auf eine **angeschnittene** Zeile am unteren Listenrand klicken.

- **Erwartet:** Genau diese Zeile ist markiert, der Lesebereich zeigt dieselbe
  Notiz, und das Bild bleibt im Augenblick des Klicks stehen.
- **Erwartet und kein Fehler:** Etwa eine halbe Sekunde später rückt die Zeile
  ganz ins Bild. Das ist Qts eigener verzögerter Autoscroll, als **#89**
  gebucht und **Deinem Blick vorbehalten** — die Markierung bleibt dabei richtig.

**Das ist der einzige Beleg, den kein Agent führen kann.** Alle Messungen
beruhen auf zugestellten Ereignissen; dass ein echter Mausklick denselben Weg
nimmt, ist geschlossen, nicht gemessen.

### #70 — Erste Notiz einer Gruppe holt ihren Kopf *(Belegform: Blick des Kunden)*

In derselben Bibliothek mit der **Pfeiltaste aufwärts** auf die **erste Notiz
einer Gruppe** navigieren.

- **Erwartet:** Der Gruppenkopf („Gestern", „Letzte Woche") steht danach
  vollständig im Bild, zusammen mit der Auswahl.
- **Erwartet und kein Fehler:** Die Liste bewegt sich dabei mehr als früher.
  Das ist der bewusst getragene Preis — ohne den Kopf sagt der Zeitstempel „08:00"
  und nichts sagt, von welchem Tag.
- **Gegenprobe:** Ein Schritt *innerhalb* einer Gruppe auf eine Notiz, die nicht
  die erste ist, rollt **nicht** zusätzlich.

### #72 — Tooltips mit Tastenkürzel *(Belegform: Blick des Kunden)*

In der Bibliothek eine Notiz auswählen und mit dem Zeiger über
**Bearbeiten** und **Löschen** verweilen; dann eine Notiz löschen und über
**Rückgängig** in der Meldungszeile verweilen.

- **Erwartet:** „Notiz bearbeiten (F2)", „Notiz löschen (Entf)", „Löschen
  rückgängig machen (Strg+Z)" — das Kürzel jeweils aus der Aktion gelesen.
- **Das ist der Teil, den kein Agent belegen kann:** Ein Tooltip ist ein eigenes
  Fenster, das der Zeiger auslöst; unter Wayland bewegt ein Prozess den Zeiger
  nicht. Belegt ist der hinterlegte Text, nicht sein Erscheinen.

### #76 — Linterbefunde geheilt *(Belegform: Terminalausgabe + Journal)*

Aufräumarbeit ohne sichtbare Wirkung — **wird eine sichtbar, ist das ein
Fehler**. Der Hauptweg ist deshalb: die zwei Stellen, an denen „nur Aufräumen"
aufhört, eines zu sein.

```bash
journalctl --user -t denkzetteld -n 20
#   erwartet: keine neuen Fehlermeldungen

# Die Registrierung der Kürzel und der Bibliotheksweg — beide berührt von
# den const-Änderungen in main.cpp und globalshortcuts.cpp
qdbus6 org.denkzettel.Daemon /Daemon ShowLibrary
```

---

## 4. Was danach im Protokoll steht

Je Story: **Prüfmittel, Ergebnis, Belegform**. Wo der Beleg ein Blick des Kunden
ist, steht das so da — nicht als Messwert verkleidet.

**Vier der sieben Hauptwege enden bei einem Menschen** (#61 Tastendruck, #71
Mausklick, #70 Tastaturweg, #72 Verweilen). Das ist keine Lücke des Verfahrens,
sondern seine Grenze: Unter Wayland teilt sich kein Prozess Zeiger oder Fokus
selbst zu.
