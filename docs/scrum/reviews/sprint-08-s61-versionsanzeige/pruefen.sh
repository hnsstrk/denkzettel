#!/usr/bin/env bash
# Prüfmittel zu #61 — Versionsanzeige und Versionsregeln.
#
# Führt jeden Nachweis vor, den ein Agent führen kann, und schreibt seine
# Ausgabe nach messungen/. Was er **nicht** kann, steht im Bericht unter
# „Befehlsliste für den PO": Busname und globale Kürzel am installierten Stand.
#
# Der Lauf fasst weder den laufenden Dienst des Kunden noch dessen Notizen an:
# Jeder gestartete Dienst bekommt eine eigene Sitzungsbus-Instanz und eigene
# XDG-Verzeichnisse. Die eine Ausnahme ist Nachweis 2 — er ruft `--version`
# absichtlich gegen den **echten** Sitzungsbus, weil genau das der Fall „auch
# bei laufendem Dienst" ist; er meldet sich dort nicht an und schickt keine
# Nachricht.
#
# Aufruf: bash docs/scrum/reviews/sprint-08-s61-versionsanzeige/pruefen.sh

set -uo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$WURZEL/build"
BIN="$BAU/bin/denkzetteld"
MESS="$HIER/messungen"
mkdir -p "$MESS"

VERSION="$(sed -n 's/^project(denkzettel VERSION \([0-9.]*\).*/\1/p' "$WURZEL/CMakeLists.txt")"

echo "Stand:   $(git -C "$WURZEL" rev-parse --short HEAD)"
echo "Version: $VERSION (aus CMakeLists.txt)"

if [ ! -x "$BIN" ]; then
    echo "FEHLT: $BIN — erst bauen (cmake --build build)."
    exit 1
fi

# --------------------------------------------------------------------------
# 1) AK 1/2/3 — die Zeile, ohne Sitzungsbus, im Wortlaut des Kriteriums.
#
# Hier steht `env -u`, nicht der unerreichbare Bus des Prüfsatzes: Das Kriterium
# nennt den Fall „ohne DBUS_SESSION_BUS_ADDRESS", und der wird hier gemessen.
# Der Prüfsatz nimmt die schärfere Form, weil D-Bus bei entfernter Variablen
# einen eigenen Bus starten kann.
# --------------------------------------------------------------------------
{
    echo "== AK 1/2/3 — --version ohne DBUS_SESSION_BUS_ADDRESS =="
    echo "Erwartet: 'denkzettel $VERSION', Rückgabe 0"
    for schalter in --version -v; do
        echo "--- $schalter ---"
        env -u DBUS_SESSION_BUS_ADDRESS QT_QPA_PLATFORM=offscreen \
            timeout 10 "$BIN" "$schalter"
        echo "Rückgabe: $?"
    done
    echo
    echo "== Gegenprobe: dieselben Schalter mit unerreichbarem Bus =="
    env DBUS_SESSION_BUS_ADDRESS="unix:path=/nonexistent/denkzettel-kein-bus" \
        QT_QPA_PLATFORM=offscreen timeout 10 "$BIN" --version
    echo "Rückgabe: $?"
} > "$MESS/ak1-version-ohne-bus.txt" 2>&1

# --------------------------------------------------------------------------
# 2) AK 1 — bei laufendem Dienst.
#
# Gegen den echten Sitzungsbus dieser Anmeldung. Läge die Auswertung hinter der
# Einzelinstanz-Weiche, stünde hier keine Ausgabe, die Rückgabe wäre trotzdem 0
# und auf dem Bildschirm des Kunden ginge ein Erfassungsfenster auf.
# --------------------------------------------------------------------------
{
    echo "== AK 1 — --version bei laufendem Dienst (echter Sitzungsbus) =="
    if pgrep -x denkzetteld > /dev/null; then
        echo "Laufender Dienst: ja (das ist die Voraussetzung dieses Nachweises)"
    else
        echo "Laufender Dienst: NEIN — dieser Nachweis misst dann nichts."
    fi
    echo "--- gebauter Stand, --version ---"
    QT_QPA_PLATFORM=offscreen timeout 10 "$BIN" --version
    echo "Rückgabe: $?"
    echo "Dienst danach noch da: $(pgrep -x denkzetteld > /dev/null && echo ja || echo nein)"
} > "$MESS/ak1-version-bei-laufendem-dienst.txt" 2>&1

# --------------------------------------------------------------------------
# 3) AK 3 — die Nummer steht an genau einer Stelle.
# --------------------------------------------------------------------------
{
    echo "== AK 3 — Fundstellen der Nummer $VERSION im versionierten Bestand =="
    echo "Gesucht wird ohne Ordnerfilter; die Vorberichte und Reviews nennen sie"
    echo "als Messwert, das ist kein Verdrahten. Entscheidend ist src/ und tests/."
    echo
    echo "--- src/ und tests/ ---"
    git -C "$WURZEL" grep -n -F "$VERSION" -- src tests
    echo "Treffer: $(git -C "$WURZEL" grep -c -F "$VERSION" -- src tests | wc -l)"
    echo
    echo "--- CMakeLists.txt aller drei Ebenen ---"
    git -C "$WURZEL" grep -n -F "$VERSION" -- CMakeLists.txt src/CMakeLists.txt tests/CMakeLists.txt
    echo
    echo "--- der Durchreichweg ---"
    git -C "$WURZEL" grep -n "DENKZETTEL_VERSION" -- src tests
} > "$MESS/ak3-einzige-stelle.txt" 2>&1

# --------------------------------------------------------------------------
# 4) AK 4 — der angemeldete Busname, in einer eigenen Sitzung.
#
# Gemessen wird der Name, den der Bus führt, nicht der, den eine Kopfdatei
# vermuten lässt.
# --------------------------------------------------------------------------
{
    echo "== AK 4 — angemeldeter Busname des gebauten Standes =="
    echo "Erwartet: org.denkzettel.Daemon (SPEC 2.3)"
    HEIM="$(mktemp -d)"
    XDG_DATA_HOME="$HEIM/data" XDG_CONFIG_HOME="$HEIM/config" \
    XDG_CACHE_HOME="$HEIM/cache" XDG_STATE_HOME="$HEIM/state" \
    XDG_DATA_DIRS="$HEIM/ohne-dienste" QT_QPA_PLATFORM=offscreen \
    dbus-run-session -- bash -c '
        "'"$BIN"'" > "'"$HEIM"'/dienst.log" 2>&1 &
        PID=$!
        for _ in $(seq 1 50); do
            sleep 0.2
            dbus-send --session --print-reply --dest=org.freedesktop.DBus \
                /org/freedesktop/DBus org.freedesktop.DBus.ListNames 2>/dev/null \
                | grep -q "org.denkzettel.Daemon" && break
        done
        echo "angemeldete Namen (gefiltert):"
        dbus-send --session --print-reply --dest=org.freedesktop.DBus \
            /org/freedesktop/DBus org.freedesktop.DBus.ListNames 2>/dev/null \
            | grep -i "denkzettel\|daemon" | sed "s/^/  /"
        echo "Prozess lebt noch: $(kill -0 $PID 2>/dev/null && echo ja || echo NEIN)"
        kill $PID 2>/dev/null; wait $PID 2>/dev/null
    '
    echo "Ausgabe des Dienstes:"
    sed 's/^/  /' "$HEIM/dienst.log"
    rm -rf "$HEIM"
} > "$MESS/ak4-busname.txt" 2>&1

# --------------------------------------------------------------------------
# 5) AK 6/7 — Schalter.
# --------------------------------------------------------------------------
{
    echo "== AK 6/7 — Schalter =="
    for schalter in --kennt-keiner --desktopfile; do
        echo "--- $schalter ---"
        if [ "$schalter" = "--desktopfile" ]; then
            env DBUS_SESSION_BUS_ADDRESS="unix:path=/nonexistent/kein-bus" \
                QT_QPA_PLATFORM=offscreen timeout 10 "$BIN" "$schalter" org.example.Fremd
        else
            env DBUS_SESSION_BUS_ADDRESS="unix:path=/nonexistent/kein-bus" \
                QT_QPA_PLATFORM=offscreen timeout 10 "$BIN" "$schalter"
        fi
        echo "Rückgabe: $? (erwartet: ungleich 0)"
    done
    echo
    echo "--- --help, zur Ansicht ---"
    env DBUS_SESSION_BUS_ADDRESS="unix:path=/nonexistent/kein-bus" \
        QT_QPA_PLATFORM=offscreen timeout 10 "$BIN" --help
    echo "Rückgabe: $?"
} > "$MESS/ak6-ak7-schalter.txt" 2>&1

# --------------------------------------------------------------------------
# 6) Der Testsatz.
# --------------------------------------------------------------------------
ctest --test-dir "$BAU" --output-on-failure > "$MESS/ctest.txt" 2>&1
echo "ctest: $(tail -3 "$MESS/ctest.txt" | head -1)"

echo "Alle Ausgaben liegen in $MESS"
