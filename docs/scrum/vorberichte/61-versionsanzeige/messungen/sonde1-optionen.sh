#!/bin/bash
# Sonde 1 — Was tut denkzetteld heute mit --version, -v und --help?
# Läuft in einer eigenen D-Bus-Sitzung und mit eigenen XDG-Verzeichnissen,
# damit weder der laufende Dienst des Kunden noch seine Datenbank berührt wird.
BIN="$(dirname "$0")/../build/bin/denkzetteld"
SANDBOX="$(mktemp -d)"
export XDG_DATA_HOME="$SANDBOX/data" XDG_CONFIG_HOME="$SANDBOX/config" XDG_CACHE_HOME="$SANDBOX/cache"
export QT_QPA_PLATFORM=offscreen
mkdir -p "$XDG_DATA_HOME" "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME"

echo "Binärdatei: $(readlink -f "$BIN")"
echo "Stand:      $(cd "$(dirname "$0")/../../../../.." && git rev-parse --short HEAD)"
echo

for OPT in "--version" "-v" "--help" ""; do
    echo "===== Aufruf: denkzetteld ${OPT:-<ohne Argument>} ====="
    OUT=$(timeout 4 dbus-run-session -- "$BIN" $OPT 2>&1)
    RC=$?
    echo "Rückgabe: $RC   (124 = Zeitüberschreitung, der Dienst lief also weiter)"
    echo "--- Ausgabe ---"
    echo "$OUT"
    echo
done
rm -rf "$SANDBOX"
