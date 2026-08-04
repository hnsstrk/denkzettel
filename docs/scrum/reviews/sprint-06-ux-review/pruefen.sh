#!/bin/bash
# Wiederholt den UI-Review Sprint 6 (#55, #56) vollständig: baut den Sprint-Stand
# und die Prüfsonden in einen EIGENEN Bauplatz, erzeugt alle Bilder neu und
# fährt die Messungen. Fasst weder `build/` der Repositoriumswurzel an — dort
# arbeiten unter Umständen andere Agenten — noch irgendetwas unter /usr.
#
#   bash docs/scrum/reviews/sprint-06-ux-review/pruefen.sh
set -eu

HIER="$(cd "$(dirname "$0")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"
BILDER="$HIER/bilder"

export QT_QPA_PLATFORM=offscreen
# Pflicht: ohne Plattformthema fällt Qt auf eine Ersatzschrift zurück, und ein
# Bild eines Fensters, dessen Höhe eine Zeilenzahl ist, verfälscht damit genau
# das, wofür es aufgenommen wird.
export QT_QPA_PLATFORMTHEME=kde

echo "== Sprint-Stand bauen (eigener Bauplatz, Bildläufer frisch)"
cmake -B "$BAU/produkt" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$BAU/produkt" --target captureshots capturetest -j "$(nproc)" > /dev/null

echo "== Pflichtläufe von capturetest"
QT_QPA_PLATFORMTHEME= "$BAU/produkt/bin/capturetest" 2>&1 | tail -1
"$BAU/produkt/bin/capturetest" 2>&1 | tail -1

echo "== Bilder 01-14 aus dem frisch gebauten Läufer"
mkdir -p "$BILDER"
"$BAU/produkt/bin/captureshots" "$BILDER" | tee "$BILDER/themes.txt"

echo "== Prüfsonden bauen"
cmake -B "$BAU/sonden" -S "$HIER/sonden" \
    -DDZ_SRC="$WURZEL" -DDZ_BUILD="$BAU/produkt" > /dev/null
cmake --build "$BAU/sonden" -j "$(nproc)" > /dev/null

echo
echo "== Messung 1: Maße aus Wireframe 4b am gebauten Fenster"
"$BAU/sonden/mass" | tee "$HIER/masse.txt"

echo
echo "== Messung 2: Wiederöffnen nach acht Zeilen (Bilder 16 und 17)"
"$BAU/sonden/wieder" "$BILDER" | tee "$HIER/wiederoeffnen.txt"

echo
echo "== Messung 3: Schriftwechsel in beide Richtungen"
"$BAU/sonden/schrift" | tee "$HIER/schriftwechsel.txt"

echo
echo "== Messung 4: der Rollbalken im Zustand 'getippt'"
"$BAU/sonden/rollbalken2" | tee "$HIER/rollbalken.txt"
"$BAU/sonden/setzen" "$BILDER"

echo
echo "== Messung 5: ARGB der Fensterecke und die beiden Alphamasken"
{ "$BAU/sonden/ecke"; echo; "$BAU/sonden/maske"; } | tee "$HIER/ecke.txt"

echo
echo "== Bildauswertung (Lupen 18 und 19, Kontur, Tinte, Vollständigkeit)"
python3 "$HIER/sonden/ecken.py"
python3 "$HIER/sonden/lupe4.py"
{
    python3 "$HIER/sonden/kontur.py"
    python3 "$HIER/sonden/eckkarte2.py"
    python3 "$HIER/sonden/tinte.py"
    python3 "$HIER/sonden/vollstaendig.py"
    python3 "$HIER/sonden/loecher.py"
} | tee "$HIER/bildauswertung.txt"

echo
echo "Fertig. Bilder in $BILDER, Protokolle neben dieser Datei."
