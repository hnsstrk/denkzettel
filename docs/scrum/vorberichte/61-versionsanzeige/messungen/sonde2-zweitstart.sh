#!/bin/bash
# Sonde 2 — Was tut `denkzetteld --version`, wenn bereits ein Dienst läuft?
#
# KDBusService::Unique reicht den Start eines zweiten Prozesses an den
# laufenden weiter (SPEC 2.3). Gemessen wird in einer eigenen D-Bus-Sitzung,
# damit der Dienst des Kunden unberührt bleibt: Prozess A ist der "laufende
# Dienst", Prozess B der Aufruf mit --version. dbus-monitor schreibt mit,
# welche Methode B auf dem Bus ruft.
set -u
HIER="$(cd "$(dirname "$0")" && pwd)"
BIN="$HIER/../build/bin/denkzetteld"
SANDBOX="$(mktemp -d)"
export XDG_DATA_HOME="$SANDBOX/data" XDG_CONFIG_HOME="$SANDBOX/config" XDG_CACHE_HOME="$SANDBOX/cache"
export QT_QPA_PLATFORM=offscreen
mkdir -p "$XDG_DATA_HOME" "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME"

dbus-run-session -- bash -c '
    BIN="$1"; SANDBOX="$2"
    dbus-monitor --session "type=method_call,destination=org.denkzettel.Daemon" \
        > "$SANDBOX/bus.txt" 2>/dev/null &
    MON=$!
    "$BIN" > "$SANDBOX/a.txt" 2>&1 &
    A=$!
    sleep 2
    echo "Prozess A (der laufende Dienst) läuft: PID $A"
    echo "Auf dem Bus angemeldet:"
    busctl --user list --no-legend 2>/dev/null | grep -i denkzettel || echo "  (nichts gefunden)"
    echo
    echo "===== Prozess B: denkzetteld --version ====="
    START=$(date +%s.%N)
    timeout 5 "$BIN" --version > "$SANDBOX/b.txt" 2>&1
    RC=$?
    ENDE=$(date +%s.%N)
    echo "Rückgabe B: $RC   (124 wäre Zeitüberschreitung)"
    echo "Dauer B:    $(echo "$ENDE - $START" | bc) s"
    echo "--- Ausgabe von B ---"
    cat "$SANDBOX/b.txt"
    echo "--- Ende Ausgabe B ---"
    sleep 1
    kill $A 2>/dev/null; kill $MON 2>/dev/null
    sleep 1
    echo
    echo "===== Methodenaufrufe an org.denkzettel.Daemon auf dem Bus ====="
    grep -E "member=|interface=" "$SANDBOX/bus.txt" | sed "s/^ */  /" | sort | uniq -c
' _ "$BIN" "$SANDBOX"

echo
echo "===== Zum Vergleich: was die Anwendung selbst über sich weiß ====="
echo "QCoreApplication::applicationVersion() wird in src/main.cpp nicht gesetzt;"
echo "der Vorgabewert ist die leere Zeichenkette."
rm -rf "$SANDBOX"
