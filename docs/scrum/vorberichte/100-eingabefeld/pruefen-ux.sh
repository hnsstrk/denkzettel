#!/usr/bin/env bash
# Fährt die Messungen der UX-Entscheidung zur Textfarbe (#100, 07.08.2026) und
# schreibt sie nach messungen-ux/.
#
# Das Skript installiert nichts, fasst keinen Produktivcode an und ändert keine
# Einstellung des Kunden. Es baut in einen eigenen Bauplatz (build-ux/), nicht
# in `build/` im Projektstamm — dort arbeiten während der Vorprüfung weitere
# Stränge.
#
# Alles läuft offscreen. Das genügt für diese Frage, weil keine der Zahlen über
# Hülle, Rundung, Kontur, Schatten oder Dekoration etwas behauptet (B21):
# gemessen werden Farbrollen, die Deckung zweier Theme-Grafiken und Kontraste.
# Was der Compositor beiträgt, ist in der Entscheidung als Grenze benannt.
#
# Aufruf: bash docs/scrum/vorberichte/100-eingabefeld/pruefen-ux.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BAU="$HIER/build-ux"

mkdir -p "$HIER/messungen-ux"

export QT_QPA_PLATFORMTHEME=kde
export QT_QPA_PLATFORM=offscreen

echo "== Sonden übersetzen =="
# Frisch bauen, bevor gemessen wird: eine veraltete Sonde schreibt plausible
# Zahlen eines alten Standes mit frischem Zeitstempel.
cmake -B "$BAU" -S "$HIER/sonden-ux" -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build "$BAU" -j "$(nproc)" > /dev/null

echo "== UX1: was widgets/lineedit unter base, hover, focus und focusframe zeichnet =="
"$BAU/vorsatzsonde" > "$HIER/messungen-ux/ux1-vorsaetze.txt" 2>/dev/null

echo "== UX2: die vier Paarungen von Vordergrund und Grund je Farbschema =="
"$BAU/paarungssonde" /usr/share/color-schemes/*.colors \
    "$HOME"/.local/share/color-schemes/*.colors \
    > "$HIER/messungen-ux/ux2-paarungen.txt" 2>/dev/null

echo "== UX3: derselbe Vergleich auf dem Theme-Farbweg (#85) =="
"$BAU/themefarbsonde" > "$HIER/messungen-ux/ux3-themefarben.txt" 2>/dev/null

echo "== UX6: die Deckung der Hülle je Theme =="
"$BAU/deckungssonde" > "$HIER/messungen-ux/ux6-huellendeckung.txt" 2>/dev/null

echo
echo "UX4 (Farbabstand je Schema) und UX5 (Auswertung nach Lage) sind aus UX2"
echo "und den .colors-Dateien gerechnet; die Rechenwege stehen im Kopf der"
echo "jeweiligen Datei in messungen-ux/."
echo
echo "Fertig. Protokolle in messungen-ux/,"
echo "Entscheidung: docs/scrum/reviews/2026-08-07-textfarbe/entscheidung.md"
