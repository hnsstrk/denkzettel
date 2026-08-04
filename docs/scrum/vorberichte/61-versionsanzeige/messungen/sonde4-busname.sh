#!/bin/bash
# Sonde 4 — Was macht KAboutData::setApplicationData mit dem Busnamen (SPEC 2.3)
# und mit dem Namen der Desktop-Datei (SPEC 2.4)?
#
# Jeder Lauf in einer eigenen D-Bus-Sitzung; der Dienst des Kunden wird nicht
# berührt.
set -u
HIER="$(cd "$(dirname "$0")" && pwd)"
BIN="$HIER/../sonden/build/busname"
SANDBOX="$(mktemp -d)"
export XDG_DATA_HOME="$SANDBOX/data" XDG_CONFIG_HOME="$SANDBOX/config" XDG_CACHE_HOME="$SANDBOX/cache"
export QT_QPA_PLATFORM=offscreen
mkdir -p "$XDG_DATA_HOME" "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME"

echo "Stand: $(cd "$HIER/../../../../.." && git rev-parse --short HEAD)"
echo
for MODUS in ohne mit heilung; do
    echo "=============================================="
    DZ_MODUS="$MODUS" timeout 10 dbus-run-session -- "$BIN" 2>&1 | grep -v "^dbus-daemon"
    echo
done
rm -rf "$SANDBOX"
