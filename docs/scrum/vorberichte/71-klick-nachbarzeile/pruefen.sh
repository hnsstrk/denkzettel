#!/usr/bin/env bash
# Fährt die Messungen der Vorprüfung zu Issue #71 (Bearbeiter A).
#
# Das Skript baut in einen **eigenen** Bauplatz (`build/` neben dieser Datei,
# von .gitignore gedeckt). Es fasst weder `build/` der Repositoriumswurzel an
# — dort arbeiten unter Umständen andere Stränge — noch irgendetwas unter
# `/usr`, und es ändert keine Einstellung des Kunden: Die Sonde legt sich ihre
# Datenbank in ein temporäres Verzeichnis und bekommt ein eigenes
# XDG_CONFIG_HOME, damit die gespeicherte Fenstergeometrie unberührt bleibt.
#
# Der Wayland-Lauf zeigt kurz Fenster auf dem Schirm. Ohne angemeldete Sitzung
# wird er übersprungen und das gesagt.
#
# Aufruf: bash docs/scrum/vorberichte/71-klick-nachbarzeile/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

mkdir -p "$HIER/messungen"

echo "== Projektbibliotheken im eigenen Bauplatz bauen =="
cmake -B "$BAU/projekt" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$BAU/projekt" --target denkzettelui denkzettelstore -j "$(nproc)" > /dev/null

echo "== Sonde übersetzen =="
cmake -B "$BAU/sonden" -S "$HIER/sonden" \
    -DDENKZETTEL_BUILD="$BAU/projekt" -DDENKZETTEL_SRC="$WURZEL/src" > /dev/null
cmake --build "$BAU/sonden" -j "$(nproc)" > /dev/null

# Eigene Umgebung: weder die Einstellungen noch der Zwischenspeicher des
# Kunden werden angefasst.
export XDG_CONFIG_HOME="$FLUECHTIG/config"
export XDG_CACHE_HOME="$FLUECHTIG/cache"
export QT_QPA_PLATFORMTHEME=kde
mkdir -p "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME"

echo "== Klicksonde offscreen =="
QT_QPA_PLATFORM=offscreen "$BAU/sonden/klicksonde" \
    > "$HIER/messungen/klicksonde-offscreen.txt"
tail -n 3 "$HIER/messungen/klicksonde-offscreen.txt"

if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    echo "== Klicksonde in der angemeldeten Sitzung (Wayland) =="
    "$BAU/sonden/klicksonde" > "$HIER/messungen/klicksonde-wayland.txt"
    echo "   Unterschied offscreen gegen Wayland:"
    diff "$HIER/messungen/klicksonde-offscreen.txt" \
         "$HIER/messungen/klicksonde-wayland.txt" || true
else
    echo "== Wayland-Lauf übersprungen — keine angemeldete Sitzung =="
fi

echo "== fertig =="
