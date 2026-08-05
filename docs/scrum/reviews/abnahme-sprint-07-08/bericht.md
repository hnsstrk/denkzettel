# Abnahme der Sprints 7 und 8 durch den Product Owner

**05.08.2026, 22:52–22:55** · Endstand `main` · installiert nach `/usr` ·
Belege in `belege/`

**Rollenlage:** Der Kunde hat Freigabe und Abnahme beider Sprints am 05.08.2026
vorab an den PO übertragen und sieht das Gesamtergebnis danach an. Die
Definition of Done bleibt vollständig in Kraft (`sprint-07.md` §1).

---

## 1. Takt 1, Punkt 1 — installiert und laufend

```
-- vorher --            PID 505843   /usr/bin/denkzetteld (deleted)
-- nach Neustart --     PID 571864   /usr/bin/denkzetteld
-- Stand --             8.355.456 Bytes, 05.08.2026 22:18
```

**B16 hat sich dabei selbst bewiesen.** Vor dem Neustart hielt der laufende
Dienst die **gelöschte** alte Binärdatei — `readlink` endete auf `(deleted)`.
Wer ohne diesen Schritt geprüft hätte, hätte den Stand von Sprint 6 geprüft und
es an nichts gemerkt. Beleg: `belege/00-b16-installiert-und-laeuft.txt`.

## 2. Die sieben Hauptwege

| Story | Hauptweg | Ergebnis | Belegform |
|---|---|---|---|
| **#61** | `--version`, Busname, Anwendungs-Id, Kürzel, unbekannter Schalter | **belegt** | Terminalausgabe |
| **#76** | Journal, Bibliotheksweg, Kürzel danach noch registriert | **belegt** | Terminalausgabe |
| **#83** | Erfassungsfenster am installierten Stand | **belegt** | Bild aus der Sitzung |
| **#85** | dieselbe Aufnahme, Farbherkunft | **belegt** | Bild aus der Sitzung |
| **#71** | Klick auf eine angeschnittene Zeile | **dem Kunden vorbehalten** | sein Blick |
| **#70** | Pfeiltaste auf die erste Notiz einer Gruppe | **dem Kunden vorbehalten** | sein Blick |
| **#72** | Verweilen über den drei Schaltflächen | **dem Kunden vorbehalten** | sein Blick |

### Was gemessen wurde

**#61** (`belege/01-hauptweg-61.txt`): `denkzetteld --version` → `denkzettel
0.1.0`, Rückgabe 0 — **auch ohne Sitzungsbus** und **bei laufendem Dienst**. Der
Anwendungsname, nicht der Busname. `org.denkzettel.Daemon` steht auf dem Bus
(SPEC 2.3), `org.denkzettel.Denkzettel.desktop` ist die Anwendungs-Id
(SPEC 2.4), und `show-capture` trägt `268435534` = `Meta+N`. Ein unbekannter
Schalter wird mit Rückgabe **1** abgewiesen, der argumentlose Start läuft.

**#76** (`belege/02-hauptweg-76.txt`): Journal seit dem Neustart **leer**, der
Bibliotheksweg antwortet mit Rückgabe 0, und das Kürzel ist danach **weiterhin**
registriert. Das sind genau die zwei Stellen, an denen „nur Aufräumen" aufhört,
eines zu sein.

**#83 und #85** (`belege/03-…png`, `belege/04-…txt`): Das Fenster trägt einen
Alphakanal, die Fläche liegt bei **84,7 %**, der Rand bei **92,2 %** — der
Deckungsrand, der die eigene Kontur ersetzt hat. Die Eckform läuft über zehn
Zeilen aus. **Die Flächenfarbe `(32, 35, 38)` ist die des eingestellten
Farbschemas** — #83 AK 8 am installierten Stand.

*Einschränkung, im Beleg selbst vermerkt:* Die **Stufenzahl** von AK 4 ruht auf
dem Messweg des Strangs, nicht auf dieser Aufnahme; mein Schwellwert ist ad hoc
gewählt, um Hülle von Schatten zu trennen, und liefert andere Zahlen.

### Die vier Wege, die ein Agent nicht führen kann

`belege/05-grenzen.txt`. Unter Wayland teilt sich kein Prozess Tastatur, Zeiger
oder Fokus selbst zu. **Das ist keine Lücke des Verfahrens, sondern seine
Grenze**, und sie war in jedem der vier Issues vorher benannt — nicht
nachträglich als Entschuldigung.

## 3. Abnahmeurteil des PO

**Alle sieben Stories sind abgenommen.** Die vier Wege oben stehen dem Blick des
Kunden offen; die Prüflisten dafür liegen versioniert in `HAUPTWEGE.md`. Zwei
Punkte gehören zum Urteil dazu:

**Was die Abnahme ausdrücklich nicht behauptet.** #85 macht die gedämpfte
Textklasse nicht lesbar (unter `breeze-light` erreicht keine Quelle 4,5 : 1 —
das ist #84), und unter den drei Themes, die den Kontrasteffekt anfordern,
sichert sie gar nichts zu: Der Effekt ist auf diesem KWin **nicht geladen**.
#83 löst den Schatten dekorierter Fenster nicht — das steht seit dem Schnitt im
Issue.

**Ein Befund liegt außerhalb beider Sprints und ist schwerer als alles darin.**
Im Release-Bau gehen **Kategorien verloren** (#99). Das besteht auf `main` vor
Sprint 8, ist unabhängig nachgemessen und trifft den Kunden heute nicht — sein
Dienst ist ein Debug-Bau. **Es trifft ihn an dem Tag, an dem ein Paket
entsteht.**
