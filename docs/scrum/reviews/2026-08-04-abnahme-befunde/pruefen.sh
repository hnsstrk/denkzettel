#!/usr/bin/env bash
# Fährt alle Messungen zu den Kundenbefunden der Sprint-6-Abnahme.
#
# Das Skript baut in einen **eigenen** Bauplatz (`build/` neben dieser Datei,
# von .gitignore gedeckt). Es fasst weder `build/` der Repositoriumswurzel an
# — dort arbeiten unter Umständen andere Agenten — noch irgendetwas unter
# `/usr`. Es schreibt keine Einstellung des Kunden: die Sonden, die ein
# Farbschema oder ein Desktop-Theme brauchen, bekommen ein eigenes
# XDG_CONFIG_HOME bzw. XDG_DATA_DIRS unter /tmp.
#
# Zwei Messungen laufen in der **angemeldeten Sitzung** und zeigen dabei für gut
# eine Sekunde ein Fenster auf dem Bildschirm (b1-echtelage*). Sie nehmen den
# Bildschirm nicht auf — QWidget::grab() zeichnet nur das Fenster. Ohne
# Wayland-Sitzung werden sie übersprungen und das gesagt.
#
# Aufruf: bash docs/scrum/reviews/2026-08-04-abnahme-befunde/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

mkdir -p "$HIER/messungen" "$HIER/bilder/b1" "$HIER/bilder/b3"

echo "== Denkzettel übersetzen (eigener Bauplatz) =="
cmake -B "$BAU/projekt" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$BAU/projekt" -j "$(nproc)" --target denkzettelcapture denkzettelstore > /dev/null

echo "== Sonden übersetzen =="
cmake -B "$BAU/sonden" -S "$HIER/sonden" -DDENKZETTEL_LIB_DIR="$BAU/projekt/lib" > /dev/null
cmake --build "$BAU/sonden" -j "$(nproc)" > /dev/null

export XDG_CACHE_HOME="$FLUECHTIG/cache"
export QT_QPA_PLATFORMTHEME=kde

echo "== B1: Ring aus zwei Rahmen, offscreen gegen Wayland =="
# Derselbe Binärcode, dasselbe Theme, derselbe Zwischenspeicher — nur die
# Plattform ist verschieden. Das ist der ganze Versuchsaufbau.
QT_QPA_PLATFORM=offscreen "$BAU/sonden/huellenring" default "$HIER/bilder/b1/offscreen" \
    > "$HIER/messungen/b1-huellenring-offscreen.txt"
if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    "$BAU/sonden/huellenring" default "$HIER/bilder/b1/live" \
        > "$HIER/messungen/b1-huellenring-live.txt"
else
    echo "   (Wayland-Vergleich übersprungen — keine angemeldete Sitzung)"
fi

echo "== B1: Eckenraster offscreen, drei Skalierungen =="
for f in 1 1.6 2; do
    QT_SCALE_FACTOR=$f QT_QPA_PLATFORM=offscreen \
        "$BAU/sonden/eckenraster" "$HIER/bilder/b1/skala-$f" default \
        > "$HIER/messungen/b1-eckenraster-skala-$f.txt"
done

echo "== B1: Prüf-Theme mit eckigen Ecken =="
for f in 1 1.6; do
    XDG_DATA_DIRS="$WURZEL/tests/themes:/usr/share" QT_SCALE_FACTOR=$f QT_QPA_PLATFORM=offscreen \
        "$BAU/sonden/eckenraster" "$HIER/bilder/b1/eckig-$f" denkzettel-pruef-eckig \
        > "$HIER/messungen/b1-eckiges-theme-skala-$f.txt"
done

echo "== B1: die echte Lage in der angemeldeten Sitzung =="
if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    "$BAU/sonden/echtelage" "$HIER/bilder/b1/echt" \
        > "$HIER/messungen/b1-echtelage.txt"
    XDG_DATA_DIRS="$WURZEL/tests/themes:/usr/share" \
        "$BAU/sonden/echtelage" "$HIER/bilder/b1/echt-eckig" denkzettel-pruef-eckig \
        > "$HIER/messungen/b1-echtelage-eckiges-theme.txt"
else
    echo "   (übersprungen — keine angemeldete Sitzung)"
fi

echo "== B1: Helligkeit quer durch den Bogen (Kundenaufnahmen) =="
python3 "$HIER/sonden/eckhelligkeit.py" > "$HIER/messungen/b1-eckhelligkeit.txt"

echo "== B2: Schattenherkunft und Schattenprofil =="
QT_QPA_PLATFORM=offscreen "$BAU/sonden/schattenherkunft" default \
    > "$HIER/messungen/b2-schattenherkunft.txt"
python3 "$HIER/sonden/schattenprofil.py" > "$HIER/messungen/b2-schattenprofil.txt"

echo "== B3: Flächenfarbe unter dem Farbschema des Kunden =="
# Eigenes XDG_CONFIG_HOME: das Schema wird gelesen, das des Kunden nicht angefasst.
mkdir -p "$FLUECHTIG/cfg-nord"
cp /usr/share/color-schemes/CachyOSNordLightly.colors "$FLUECHTIG/cfg-nord/kdeglobals"
XDG_CONFIG_HOME="$FLUECHTIG/cfg-nord" QT_QPA_PLATFORM=offscreen \
    "$BAU/sonden/flaechenfarbe" "$HIER/bilder/b3" \
    > "$HIER/messungen/b3-flaechenfarbe-nord.txt"

echo "== B2: kommt der Schatten nativer Fenster aus denselben Kacheln? =="
bash "$HIER/sonden/dekorationsquelle.sh" > "$HIER/messungen/b2-dekorationsquelle.txt"

# ---------------------------------------------------------------------------
# Der native Weg — Kundenentscheidung „native Plasma-Überlagerung ohne
# Anpassungen" vom 04.08.2026.
# ---------------------------------------------------------------------------

echo "== Nativ: FrameSvg in einem Stück, unter zwei Farbschemata =="
mkdir -p "$FLUECHTIG/cfg-breeze"
cp /usr/share/color-schemes/BreezeLight.colors "$FLUECHTIG/cfg-breeze/kdeglobals"
for schema in nord:cfg-nord breeze:cfg-breeze; do
    name="${schema%%:*}"
    XDG_CONFIG_HOME="$FLUECHTIG/${schema##*:}" QT_QPA_PLATFORM=offscreen \
        "$BAU/sonden/nativehuelle" "$HIER/bilder/native/$name" default \
        > "$HIER/messungen/native-huelle-$name.txt"
done

echo "== Nativ: derselbe Lauf unter Wayland (Plattformabhängigkeit prüfen) =="
if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    XDG_CONFIG_HOME="$FLUECHTIG/cfg-nord" \
        "$BAU/sonden/nativehuelle" "$HIER/bilder/native/nord-live" default \
        > "$HIER/messungen/native-huelle-nord-wayland.txt"
else
    echo "   (übersprungen — keine angemeldete Sitzung)"
fi

echo "== Nativ: das Prüf-Theme mit eckigen Ecken =="
XDG_DATA_DIRS="$WURZEL/tests/themes:/usr/share" XDG_CONFIG_HOME="$FLUECHTIG/cfg-nord" \
    QT_QPA_PLATFORM=offscreen \
    "$BAU/sonden/nativehuelle" "$HIER/bilder/native/eckig" denkzettel-pruef-eckig \
    > "$HIER/messungen/native-huelle-eckiges-theme.txt"

echo "== Nativ: AK 2 über alle installierten Farbschemata =="
{
    printf "%-26s|%-11s|%-11s|%-10s|%4s|%6s|%6s|\n" \
        "Farbschema" "Window" "gezeichnet" "" "Alph" "deckd" "ungst"
    printf '%.0s-' {1..90}; echo
    for schema in /usr/share/color-schemes/*.colors "$HOME"/.local/share/color-schemes/*.colors; do
        [ -e "$schema" ] || continue
        name="$(basename "$schema" .colors)"
        mkdir -p "$FLUECHTIG/sweep/$name"
        cp "$schema" "$FLUECHTIG/sweep/$name/kdeglobals"
        XDG_CONFIG_HOME="$FLUECHTIG/sweep/$name" QT_QPA_PLATFORM=offscreen \
            "$BAU/sonden/nativekontrast" "$name" 2>/dev/null | tail -1
    done
} > "$HIER/messungen/native-ak2-kontrast.txt"

echo
echo "Fertig. Protokolle in messungen/, Bilder in bilder/."
