#!/bin/bash
# Baut die vier Schätzsonden und fährt sie. Schreibt die Protokolle neben die
# Quellen, also genau die *.txt, die im Repository liegen.
#
# Alles geschieht unterhalb dieses Verzeichnisses in build/ (von .gitignore
# gedeckt). Weder der Bau des Projekts noch die Konfiguration des Kunden wird
# angefasst:
#
#   * Der Projektstand wird in einen EIGENEN Bauplatz übersetzt, nicht in das
#     build/ der Repositoriumswurzel — dort arbeiten unter Umständen andere
#     Agenten gleichzeitig.
#   * Messung 2 läuft in einer eigenen D-Bus-Sitzung (dbus-run-session) mit
#     kopierter kdeglobals unter privatem XDG_CONFIG_HOME. kwriteconfig6
#     schreibt dadurch in die Kopie. Ohne diesen Aufbau würde der Lauf die
#     Systemschrift des Kunden verstellen.
#
# Aufruf: bash docs/scrum/reviews/sprint-06-schaetzung/pruefen.sh

set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../../../.." && pwd)"
WORK="$HERE/build"

# Ohne das Plattformthema misst man eine Ersatzschrift statt der Systemschrift,
# und alle Größenverhältnisse stimmen nicht mehr (CLAUDE.md).
export QT_QPA_PLATFORM=offscreen
export QT_QPA_PLATFORMTHEME=kde

echo "== Projektbibliotheken übersetzen (eigener Bauplatz) =="
cmake -B "$WORK/projekt" -S "$ROOT" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$WORK/projekt" --target denkzettelstore denkzettelcapture denkzettelui -j"$(nproc)" > /dev/null

echo "== Sonden übersetzen =="
cmake -B "$WORK/sonden" -S "$HERE" -DCMAKE_BUILD_TYPE=Debug \
    -DDENKZETTEL_LIB_DIR="$WORK/projekt/lib" > /dev/null
cmake --build "$WORK/sonden" -j"$(nproc)" > /dev/null

echo "== Messung 1 — Alterung der Systemschrift =="
"$WORK/sonden/schriftalterung" | tee "$HERE/schriftalterung.txt"

echo
echo "== Messung 2 — KConfigWatcher im Sandkasten =="
# Die Kopie der kdeglobals: gemessen wird gegen die echten Startwerte des
# Kunden, geschrieben wird ausschließlich in die Kopie.
SANDBOX="$WORK/sandkasten"
rm -rf "$SANDBOX"
mkdir -p "$SANDBOX"
if [ -f "$HOME/.config/kdeglobals" ]; then
    cp "$HOME/.config/kdeglobals" "$SANDBOX/kdeglobals"
else
    printf '[General]\n' > "$SANDBOX/kdeglobals"
fi

cat > "$WORK/messung2.sh" <<'INNEN'
set -eu
export XDG_CONFIG_HOME="$1"
export QT_QPA_PLATFORM=offscreen
export QT_QPA_PLATFORMTHEME=kde
"$2/kdeglobals-watcher" &
PROBE=$!
sleep 2
kwriteconfig6 --notify --file kdeglobals --group General \
    --key font "Noto Sans,20,-1,5,400,0,0,0,0,0,0,0,0,0,0,1"
kwriteconfig6 --notify --file kdeglobals --group General \
    --key smallestReadableFont "Noto Sans,16,-1,5,400,0,0,0,0,0,0,0,0,0,0,1"
wait $PROBE
INNEN

dbus-run-session -- bash "$WORK/messung2.sh" "$SANDBOX" "$WORK/sonden" \
    | tee "$HERE/kdeglobals-watcher.txt"

echo
echo "== Messung 3 — Feldhöhe und Zustellwege (#56) =="
"$WORK/sonden/capture-schriftwechsel" | tee "$HERE/capture-schriftwechsel.txt"

echo
echo "== Messung 4 — Zeilenhöhen der Bibliotheksliste (#68) =="
"$WORK/sonden/bibliothek-zeilenhoehe" | tee "$HERE/bibliothek-zeilenhoehe.txt"

echo
echo "Fertig. Die vier Protokolle liegen in $HERE."
