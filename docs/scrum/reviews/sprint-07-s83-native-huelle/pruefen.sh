#!/usr/bin/env bash
# Fährt alle Messungen zu #83 — „Hülle als native Plasma-Überlagerung, der
# native Vertrag" — und schreibt die Protokolle nach messungen/, die Bilder
# nach bilder/.
#
# Bauart wie bei den Abnahmebefunden von Sprint 6:
#   * eigener Bauplatz (`build/` neben dieser Datei, von .gitignore gedeckt).
#     Weder `build/` der Repositoriumswurzel noch irgendetwas unter `/usr` wird
#     angefasst — dort arbeiten unter Umständen andere Agenten, und den Takt
#     der Installation setzt der Product Owner.
#   * die Sonden bekommen ein eigenes XDG_CONFIG_HOME bzw. XDG_DATA_DIRS unter
#     /tmp; keine Einstellung des Kunden wird gelesen oder verstellt.
#   * die Läufe in der angemeldeten Sitzung zeigen für einige Sekunden Fenster
#     auf dem Bildschirm. Die Weichzeichner-Läufe nehmen den **Bildschirm** auf
#     — sie müssen, weil eine Fensteraufnahme den Weichzeichner nicht zeigt —,
#     legen aber ein Vollbild-Schachbrett darunter, das den Schreibtisch
#     verdeckt, löschen den Vollbildpuffer sofort und behalten nur den
#     Ausschnitt um die Hülle.
#
# Ohne Wayland-Sitzung werden diese Läufe übersprungen und das gesagt.
#
# Aufruf: bash docs/scrum/reviews/sprint-07-s83-native-huelle/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

mkdir -p "$HIER/messungen" "$HIER/bilder/offscreen" "$HIER/bilder/sitzung"

echo "== Denkzettel übersetzen (eigener Bauplatz) =="
cmake -B "$BAU/projekt" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$BAU/projekt" -j "$(nproc)" --target denkzettelcapture denkzettelstore > /dev/null

echo "== Sonden übersetzen =="
cmake -B "$BAU/sonden" -S "$HIER/sonden" -DDENKZETTEL_LIB_DIR="$BAU/projekt/lib" > /dev/null
cmake --build "$BAU/sonden" -j "$(nproc)" > /dev/null

export XDG_CACHE_HOME="$FLUECHTIG/cache"
# Ohne das Plattformthema misst man eine Ersatzschrift statt der Systemschrift,
# und alle Größenverhältnisse stimmen nicht mehr (CLAUDE.md).
export QT_QPA_PLATFORMTHEME=kde
# Die mitgelieferten Prüf-Themes neben die installierten, nicht an ihre Stelle.
export XDG_DATA_DIRS="$WURZEL/tests/themes:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

echo "== M1: die Grafik der Hülle, offscreen (AK 4, AK 7, AK 11) =="
QT_QPA_PLATFORM=offscreen "$BAU/sonden/huellengrafik" \
    > "$HIER/messungen/m1-huellengrafik-offscreen.txt"

echo "== M2: dieselbe Grafik unter Wayland (AK 11) =="
if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    # Derselbe Binärcode, dasselbe Theme, derselbe Zwischenspeicher — nur die
    # Plattform ist verschieden. Das ist der ganze Versuchsaufbau. Verglichen
    # werden die Prüfsummen der Bildpunkte, nicht Aufnahmen ganzer Fenster:
    # deren Schriftrasterung weicht offscreen ab (SPEC 16).
    "$BAU/sonden/huellengrafik" > "$HIER/messungen/m2-huellengrafik-wayland.txt"
    {
        echo "=== M3: Vergleich der beiden Läufe (AK 11) ==="
        echo "Verglichen werden die Prüfsummen je Zeile — Theme, Auswahlpfad, Verhältnis."
        echo
        if diff <(grep -E '^[A-Za-z]' "$HIER/messungen/m1-huellengrafik-offscreen.txt" | tail -n +2) \
                <(grep -E '^[A-Za-z]' "$HIER/messungen/m2-huellengrafik-wayland.txt" | tail -n +2) \
                > "$FLUECHTIG/diff.txt"; then
            echo "Kein Unterschied: Die Hülle ist offscreen und unter Wayland byteweise gleich."
        else
            echo "UNTERSCHIED:"
            cat "$FLUECHTIG/diff.txt"
        fi
    } > "$HIER/messungen/m3-plattformvergleich.txt"
else
    echo "   (übersprungen — keine angemeldete Sitzung)"
fi

echo "== M4: das echte Fenster, offscreen (AK 3, AK 5, AK 9, AK 10) =="
QT_QPA_PLATFORM=offscreen "$BAU/sonden/fensterlage" "$HIER/bilder/offscreen" \
    > "$HIER/messungen/m4-fensterlage-offscreen.txt"

echo "== M5: dasselbe Fenster in der angemeldeten Sitzung (AK 3, AK 4, AK 13) =="
if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    # **Ohne QT_SCALE_FACTOR**, und das ist der Punkt: Unter Wayland
    # multipliziert die Variable mit der Sitzungsskalierung (1 → 1,6;
    # 1,6 → 2,56). Die Skalierung des Kunden ist hier vorzufinden, nicht
    # einzustellen.
    "$BAU/sonden/fensterlage" "$HIER/bilder/sitzung" \
        > "$HIER/messungen/m5-fensterlage-sitzung.txt"
else
    echo "   (übersprungen — keine angemeldete Sitzung)"
fi

echo "== M6: wirkt der Weichzeichner? (AK 5) =="
if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    # `aus` ist der Gegenlauf, `an` das echte Fenster, `wiederzeigen` die zweite
    # Öffnung, `groesse` die Größenänderung nach der Anmeldung.
    for lauf in aus an wiederzeigen groesse; do
        "$BAU/sonden/weichzeichner" "$HIER/bilder/sitzung" "$lauf" \
            > "$HIER/messungen/m6-weichzeichner-$lauf.txt" || true
    done
else
    echo "   (übersprungen — keine angemeldete Sitzung)"
fi

echo "== M7: Flächenfarbe über alle Farbschemata unter default (AK 8) =="
{
    echo "=== #83, M7: folgt die Flächenfarbe dem Farbschema? (AK 8) ==="
    echo "Desktop-Theme default, ein Prozess je Farbschema, Toleranz ein Zählschritt."
    echo
    abweichend=0
    gesamt=0
    for schema in /usr/share/color-schemes/*.colors "$HOME"/.local/share/color-schemes/*.colors; do
        [ -e "$schema" ] || continue
        name="$(basename "$schema" .colors)"
        mkdir -p "$FLUECHTIG/schemata/$name"
        cp "$schema" "$FLUECHTIG/schemata/$name/kdeglobals"
        gesamt=$((gesamt + 1))
        if ! XDG_CONFIG_HOME="$FLUECHTIG/schemata/$name" DENKZETTEL_SCHEMA="$name" \
             QT_QPA_PLATFORM=offscreen "$BAU/sonden/schemafarbe" default 2>/dev/null; then
            abweichend=$((abweichend + 1))
        fi
    done
    echo
    echo "Über der Toleranz: $abweichend von $gesamt Farbschemata."
} > "$HIER/messungen/m7-schemafarbe.txt"

echo "== M8: die Kontrastgruppe der installierten Themes (AK 6) =="
{
    echo "=== #83, M8: welches Theme verlangt den Kontrasteffekt? (AK 6) ==="
    echo "Gelesen wird die Gruppe [ContrastEffect] aus metadata.desktop des Themes —"
    echo "dieselbe Datei und dieselbe Gruppe, die Plasma::Theme liest."
    echo
    for d in /usr/share/plasma/desktoptheme/*/ "$WURZEL"/tests/themes/plasma/desktoptheme/*/; do
        name="$(basename "$d")"
        if [ -f "$d/metadata.desktop" ] && grep -q '^\[ContrastEffect\]' "$d/metadata.desktop"; then
            printf '%-28s angemeldet:  %s\n' "$name" \
                "$(sed -n '/^\[ContrastEffect\]/,/^\[/p' "$d/metadata.desktop" \
                   | grep -E '^(enabled|contrast|intensity|saturation)=' | tr '\n' ' ')"
        else
            printf '%-28s keine Gruppe — kein Kontrasteffekt\n' "$name"
        fi
    done
} > "$HIER/messungen/m8-kontrastgruppen.txt"

echo "== M10: der helle Streifen am Bogen (AK 13) =="
if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    "$BAU/sonden/weichzeichner" "$HIER/bilder/sitzung" ecke \
        > "$HIER/messungen/m10-eckhelligkeit.txt" || true
else
    echo "   (übersprungen — keine angemeldete Sitzung)"
fi

echo "== M11: das Fenster neben KRunner (AK 12) =="
if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    "$BAU/sonden/weichzeichner" "$HIER/bilder/sitzung" krunner \
        > "$HIER/messungen/m11-krunner.txt" || true
else
    echo "   (übersprungen — keine angemeldete Sitzung)"
fi

echo "== M13: teilen sich zwei ImageSets ihre Auswahlpfade? =="
QT_QPA_PLATFORM=offscreen "$BAU/sonden/ksvgselektoren" \
    > "$HIER/messungen/m13-ksvg-selektoren.txt"

echo "== M9: Testlauf des Projekts =="
cmake --build "$BAU/projekt" -j "$(nproc)" > /dev/null
(cd "$BAU/projekt" && ctest --output-on-failure) > "$HIER/messungen/m9-testlauf.txt" 2>&1 \
    || echo "   ACHTUNG: ctest war nicht grün — siehe m9-testlauf.txt"

echo
echo "Fertig. Protokolle in messungen/, Bilder in bilder/."
