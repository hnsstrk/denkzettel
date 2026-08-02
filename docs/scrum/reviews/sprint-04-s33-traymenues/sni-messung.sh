#!/bin/bash
# Messlauf zum ersten Schritt von Issue #60 (S33): Traegt ein eigenes
# Linksklick-Menue unter Plasma/Wayland? Baut die Probe daneben, laesst sie am
# echten Panel der laufenden Sitzung anmelden, loest Activate ueber D-Bus aus
# und schreibt das Protokoll nach sni-messung.txt.
#
# Aufruf:  bash sni-messung.sh [x y]
#   x y — Position in logischen Qt-Koordinaten (Bildpunkte geteilt durch die
#         Bildschirmskalierung, siehe `kscreen-doctor -o`). Ohne Angabe wird
#         die Mitte des Systemabschnitts geschaetzt.
#
# Bilder macht der Aufrufer; sie muessen vor der Veroeffentlichung geschwaerzt
# werden, weil ein Vollbild die Fenster des Kunden zeigt.

set -u
HIER=$(cd "$(dirname "$0")" && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

X=${1:-2050}
Y=${2:-1324}
PROTOKOLL="$HIER/sni-messung.txt"

g++ -std=c++20 -fPIC "$HIER/sni-trennung-probe.cpp" -o "$TMP/probe" \
    $(pkg-config --cflags --libs Qt6Widgets) \
    -I/usr/include/KF6/KStatusNotifierItem -lKF6StatusNotifierItem || exit 1

export QT_QPA_PLATFORM=wayland
export QT_QPA_PLATFORMTHEME=kde

{
    echo "Messlauf $(date '+%Y-%m-%d %H:%M') · Sitzung: $XDG_SESSION_TYPE/$XDG_CURRENT_DESKTOP"
    kscreen-doctor -o 2>/dev/null | grep -E "Geometry|Scale" | sed 's/\x1b\[[0-9;]*m//g'
    echo "Activate-Position (logisch): ($X,$Y)"
} >"$PROTOKOLL"

for VARIANTE in popup fenster; do
    "$TMP/probe" "$VARIANTE" >"$TMP/lauf.log" 2>&1 &
    PID=$!
    sleep 3

    DIENST=$(qdbus6 org.kde.StatusNotifierWatcher /StatusNotifierWatcher \
        RegisteredStatusNotifierItems | tail -1 | cut -d/ -f1)

    {
        echo
        echo "=== Variante $VARIANTE · Dienst $DIENST ==="
        echo -n "ItemIsMenu laut Host: "
        qdbus6 "$DIENST" /StatusNotifierItem org.freedesktop.DBus.Properties.Get \
            org.kde.StatusNotifierItem ItemIsMenu
    } >>"$PROTOKOLL"

    qdbus6 "$DIENST" /StatusNotifierItem org.kde.StatusNotifierItem.Activate "$X" "$Y" >/dev/null
    sleep 5

    cat "$TMP/lauf.log" >>"$PROTOKOLL"
    kill "$PID" 2>/dev/null
    wait "$PID" 2>/dev/null
    sleep 1
done

cat "$PROTOKOLL"
