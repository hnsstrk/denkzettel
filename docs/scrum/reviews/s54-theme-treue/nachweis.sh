#!/bin/bash
# Laeuft INNERHALB der verschachtelten Sitzung (siehe sitzung.sh). Startet den
# installierten Dienst genau einmal, oeffnet das Capture-Fenster und
# fotografiert es vor und nach zwei Farbschema-Wechseln — ohne Neustart
# dazwischen, denn genau darum geht es in Issue #54.
#
# Stellschrauben: DZ_BIN (Dienst), DZ_OUT (Bildordner), DZ_TEXT=1 (Text tippen).
set -u

S="$SANDBOX"
OUT="${DZ_OUT:-$S/bilder}"
mkdir -p "$OUT"

# xcb, damit die Bilder ueber den X-Weg abgeholt werden koennen; das
# Plattform-Theme kde ist noetig, sonst greifen Ersatzschriften.
export QT_QPA_PLATFORM=xcb
export QT_QPA_PLATFORMTHEME=kde

echo "DISPLAY=$DISPLAY WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-}"

for i in $(seq 1 40); do
    xdotool search --name . >/dev/null 2>&1 && break
    sleep 0.25
done

schema() {
    plasma-apply-colorscheme "$1" 2>&1 | sed "s/^/  [schema $1] /"
}

bild() {
    local name="$1"
    sleep 2
    local wid
    wid=$(xdotool search --name '^Denkzettel$' | head -1)
    if [ -n "$wid" ]; then
        import -window "$wid" "$OUT/$name.png" 2>/dev/null
        echo "  [bild] $name  Fenster-ID $wid"
    else
        echo "  [bild] $name  KEIN FENSTER GEFUNDEN"
    fi
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

if [ "${DZ_TEXT:-0}" = "1" ]; then
    sleep 1
    wid=$(xdotool search --name '^Denkzettel$' | head -1)
    xdotool windowactivate "$wid"
    sleep 2
    # Die ersten Anschlaege verschluckt der Fokuswechsel: erst ein Vorlauf,
    # der verworfen wird, dann der eigentliche Text.
    xdotool type --window "$wid" --delay 30 '     '
    sleep 0.5
    xdotool key --window "$wid" ctrl+a
    xdotool type --window "$wid" --delay 30 'Bücher über Straßenbahnen ansehen'
    sleep 1
fi

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
