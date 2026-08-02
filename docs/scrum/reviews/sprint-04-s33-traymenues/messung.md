# Messung zu Issue #60 (S33) — trägt die Menütrennung unter Plasma/Wayland?

**Datum:** 02.08.2026 · **Zweig:** `story/60-traymenues` · **Rolle:** Entwickler

Erster Schritt der Story laut Issue: Bevor irgendetwas umgesetzt wird, wird am
**echten Panel der laufenden Sitzung** gemessen, ob getrennte Menüs tragen.
Getrennte Menüs heißen `ItemIsMenu=false` plus ein eigenes `QMenu` im
`activateRequested`-Handler; das Risiko ist die Popup-Positionierung unter
Wayland.

**Ergebnis: sie trägt nicht.** Damit greift die Rückfallregel des Issues —
Stopp der Menütrennung, Meldung an den PO, der Kunde entscheidet.

## Aufbau

Der Dienst wurde nicht angefasst; gemessen hat eine eigenständige Probe
(`sni-trennung-probe.cpp`), die sich als zweites Symbol im Systemabschnitt
anmeldet. So bleibt der Befund unabhängig vom Produktivcode, und der
installierte Dienst des Kunden läuft ungestört weiter.

Die Probe meldet `ItemIsMenu=false` an und baut das Linksklick-Menü der Story
(fünf Arbeitswege, ein Trenner, Symbole aus dem Thema). Ausgelöst wird über
D-Bus:

```
qdbus6 <dienst> /StatusNotifierItem org.kde.StatusNotifierItem.Activate X Y
```

Reproduzierbar mit `bash sni-messung.sh 2050 1324`; das Protokoll steht in
`sni-messung.txt`.

**Sitzung:** Wayland/KDE, ein Bildschirm 3840×2160 bei Skalierung 1,6 — Qt
rechnet also in 2400×1350. Der Systemabschnitt liegt logisch bei etwa
(2050, 1324); mit dieser Position wurde gemessen. *Ein erster Lauf mit
Bildpunkt-Koordinaten (3280, 2119) lief in die Bildschirmbegrenzung von Qt und
ist verworfen — die Skalierung gehört zur Messung.*

## Was ankommt

`ItemIsMenu` steht beim Host auf `false` (über die Eigenschaft zurückgelesen,
nicht aus dem Rückgabewert geschlossen), und `activateRequested` erreicht die
Anwendung mit **genau** der übergebenen Position. Die Kette bis zum Handler
trägt also vollständig. Erst das Anzeigen scheitert.

## Befund 1 — das Menü als Popup wird nie sichtbar

```
[   11 ms] Ereignis auf QMenu: PlatformSurface
[   11 ms] Ereignis auf QMenu: Show
[   13 ms] sofort   geometry=(2050,1192) 254x158 · sichtbar=1
[   13 ms] Ereignis auf QMenu: Close
[   13 ms] Ereignis auf QMenu: Hide
[   13 ms] Ereignis auf QMenu: PlatformSurface
[   63 ms] spaeter  geometry=(2050,1192) 254x158 · sichtbar=0
```

Qt setzt das Menü an die richtige Stelle — (2050, 1192) ist die übergebene
Position, um die Menühöhe nach oben geklappt, weil unten der Bildschirmrand
kommt. **Zwei Millisekunden nach `Show` kommt `Close`.** Das Fenster wird
erzeugt und sofort wieder abgebaut; auf dem Bildschirm erscheint nichts. Bis
4 s nach der Aktivierung bleibt es unsichtbar.

Grund: Ein `QMenu` ist ein `Qt::Popup`. Unter Wayland braucht ein Popup eine
Elternfläche und die laufende Nummer eines Eingabeereignisses, um zu greifen.
Ein Klient, der nur ein Tray-Symbol hat, hat beides nicht — die Probe meldet
`Elternfenster=(nil)`, und der Klick geht an plasmashell, nicht an uns.

Beleg: `sni-popup-bleibt-unsichtbar.png` (Ausschnitt scharf, in dem das Menü
stehen müsste — dort ist nur die Leiste).

## Befund 2 — als gewöhnliches Fenster bleibt es stehen, aber am falschen Ort

Gegenprobe mit denselben Einträgen als rahmenloses Fenster statt als Popup:

```
[   11 ms] Ereignis auf QMenu: Show
[   11 ms] sofort   geometry=(0,0) 254x158 · sichtbar=1
[   14 ms] Ereignis auf QMenu: WindowActivate
[ 4200 ms] spaeter  geometry=(0,0) 254x158 · sichtbar=1
```

Das Fenster bleibt offen — und `move(2050, 1324)` ist **verworfen**: Die
Geometrie steht auf (0, 0), KWin hat das Fenster nach eigener Regel platziert.
Das Bild zeigt es in der Bildschirmmitte, weit weg vom Symbol.

Beleg: `sni-fenster-landet-mittig.png`.

Diese Gegenprobe trennt die beiden möglichen Ursachen und schließt beide:
Wayland kennt keinen Weg, mit dem ein Klient ein gewöhnliches Fenster selbst
positioniert, und ein Popup, das positioniert werden dürfte, kommt ohne
Eingabe-Grab nicht zustande.

**Sie schließt zugleich den einen Unterschied zwischen Messung und echtem
Klick.** Beim echten Klick schickt plasmashell vor `Activate` noch ein
Aktivierungs-Merkmal (`ProvideXdgActivationToken`), das die Messung nicht
hatte. Es wirkt auf die Fenster*aktivierung* — und genau die hat die
Gegenprobe: Das Fenster wurde aktiviert (`WindowActivate` nach 14 ms) und
landete trotzdem bei (0, 0). Am Ort ändert das Merkmal also nichts.

## Was das für die Story heißt

Der Weg „`ItemIsMenu=false` plus eigenes Menü" liefert entweder **kein
sichtbares Menü** oder **ein Menü in der Bildschirmmitte**. Beides ist
schlechter als der heutige Zustand, und zwar auf dem *häufigsten* Weg — der
Linksklick ist der Arbeitsweg.

Nach der Rückfallregel des Issues wird die Menütrennung damit gestoppt und dem
Kunden vorgelegt. Nicht weiter verfolgt wurden Umgehungen (eigene
Wayland-Protokolle, Fremdbibliotheken zur Fensterlage) — das wäre das
Nachbohren, das das Issue ausschließt.

Die Befunde 2, 3 und 5 der Akzeptanzkriterien (Symbole, deutsche Beschriftungen,
Umbenennung) hängen nicht an der Trennung und werden am einen Menü geliefert.
Offen und kundenentscheidungspflichtig bleibt **Befund 1** — „Beenden" nicht
im Linksklick-Menü —, denn er ist ohne getrennte Menüs nicht zu haben.

## Zu den Bildern

Die Schnappschüsse zeigen den ganzen Bildschirm der Kundensitzung. Für das
öffentliche Repository ist alles außerhalb der Belegfläche unkenntlich gemacht;
**Lage und Größe der Fenster sind unverändert**, das Bild beweist also weiter,
wo das Fenster steht. Geschwärzt wurde mit einer Weichzeichnung über das
ganze Bild und dem Zurücklegen der scharfen Belegfläche an derselben Stelle.

Der Zustand der KDE-Einstellung **„Symbole in Menüs anzeigen" war
eingeschaltet**: In `sni-fenster-landet-mittig.png` trägt jeder Eintrag sein
Symbol.

| Datei | Inhalt |
|---|---|
| `sni-trennung-probe.cpp` | die Probe; Bau- und Aufrufbefehl im Kopfkommentar |
| `sni-messung.sh` | Messlauf, der beide Varianten fährt und das Protokoll schreibt |
| `sni-messung.txt` | Protokoll des Laufs vom 02.08.2026 |
| `sni-popup-bleibt-unsichtbar.png` | Popup-Variante: an der Stelle des Symbols steht nichts |
| `sni-fenster-landet-mittig.png` | Gegenprobe: dieselben Einträge, sichtbar, aber in der Bildschirmmitte |
