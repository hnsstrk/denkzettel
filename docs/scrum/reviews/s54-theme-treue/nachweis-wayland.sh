#!/bin/bash
# Wie nachweis.sh, aber der Dienst laeuft als nativer Wayland-Klient und die
# Bilder kommen von spectacle innerhalb der verschachtelten Sitzung. Das ist
# der Weg ohne X-Zwischenschicht; die Farben treffen dadurch exakt die Werte
# der UX-Untersuchung.
#
# Aufruf ueber sitzung.sh:  DZ_INNEN=.../nachweis-wayland.sh ./sitzung.sh
set -u

S="$SANDBOX"
OUT="${DZ_OUT:-$S/bilder-wayland}"
mkdir -p "$OUT"

export QT_QPA_PLATFORM=wayland
export QT_QPA_PLATFORMTHEME=kde

schema() {
    plasma-apply-colorscheme "$1" 2>&1 | sed "s/^/  [schema $1] /"
}

bild() {
    local name="$1"
    sleep 2
    spectacle -b -n -f -o "$OUT/$name.png" 2>&1 | sed "s/^/  [spectacle] /"
    sleep 2
    echo "  [bild] $name  $(stat -c %s "$OUT/$name.png" 2>/dev/null) Bytes"
}

echo "=== Schema vor dem Dienststart: BreezeDark ==="
schema BreezeDark
sleep 1

echo "=== Dienst starten (genau einmal) ==="
"${DZ_BIN:-$S/root/usr/bin/denkzetteld}" 2>&1 | systemd-cat -t denkzetteld &
sleep 5

echo "=== Capture-Fenster oeffnen ==="
gdbus call --session --dest org.denkzettel.Daemon --object-path /Daemon \
    --method org.denkzettel.Daemon.ShowCapture 2>&1 | sed 's/^/  [dbus] /'

bild 1-start-dunkel

echo "=== Farbschema wechseln auf BreezeLight — Dienst laeuft weiter ==="
schema BreezeLight
bild 2-nach-wechsel-hell

echo "=== Farbschema wechseln zurueck auf BreezeDark — Dienst laeuft weiter ==="
schema BreezeDark
bild 3-zurueck-dunkel

echo "=== PID-Nachweis: ein einziger Dienstlauf ==="
pgrep -a denkzetteld

gdbus call --session --dest org.denkzettel.Daemon --object-path /Daemon \
    --method org.denkzettel.Daemon.Quit 2>&1 | sed 's/^/  [dbus] /'
sleep 1
echo "=== fertig ==="
