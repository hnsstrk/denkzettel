#!/usr/bin/env bash
# Fährt alle Messungen zu #100 — „Erfassungsfenster: Der Eingabebereich ist
# nicht als solcher erkennbar" — und schreibt die Protokolle nach messungen/,
# die Bilder nach bilder/.
#
# Bauart wie bei #83:
#   * eigener Bauplatz (`build/` neben dieser Datei, von .gitignore gedeckt).
#     Weder `build/` der Repositoriumswurzel noch irgendetwas unter `/usr` wird
#     angefasst — den Takt der Installation setzt der Product Owner.
#   * die Sonden bekommen ein eigenes XDG_CACHE_HOME unter /tmp; keine
#     Einstellung des Kunden wird gelesen oder verstellt.
#   * der Lauf in der angemeldeten Sitzung zeigt das Fenster für einige
#     Sekunden auf dem Bildschirm. Er nimmt **das Fenster** auf und nicht den
#     Bildschirm — vom Schreibtisch kommt kein Bildpunkt mit.
#
# Ohne Wayland-Sitzung wird M5 übersprungen und das gesagt. Ist die Sitzung
# **gesperrt**, wird er ebenfalls übersprungen: Bei gesperrter Sitzung liefert
# eine Aufnahme ein Bild des Rollladens und meldet Erfolg (gemessen bei der
# Vorprüfung zu #85). Ein Sitzungsbild ist nur ein Beleg, wenn jemand hätte
# hinsehen können.
#
# Aufruf: bash docs/scrum/reviews/sprint-09-s100-eingabefeld/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

mkdir -p "$HIER/messungen" "$HIER/bilder/offscreen" "$HIER/bilder/sitzung"

echo "== Denkzettel übersetzen (eigener Bauplatz) =="
cmake -B "$BAU/projekt" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
# Alles, kein Zielauswahl: `ctest` unten fährt neun Prüfläufe, und eine Auswahl
# ließe die übrigen als „Not Run" durchgehen — ein Testlauf, der aussieht wie
# ein Ergebnis und keines ist (gemessen beim ersten Lauf dieses Skripts). Die
# Bildläufer kommen damit ebenfalls frisch: Ein veralteter Läufer schreibt
# plausible Bilder eines **alten** Standes mit frischem Zeitstempel (Sprint 5).
cmake --build "$BAU/projekt" -j "$(nproc)" > /dev/null

echo "== Sonden übersetzen =="
cmake -B "$BAU/sonden" -S "$HIER/sonden" -DDENKZETTEL_LIB_DIR="$BAU/projekt/lib" > /dev/null
cmake --build "$BAU/sonden" -j "$(nproc)" > /dev/null

export XDG_CACHE_HOME="$FLUECHTIG/cache"
# Ohne das Plattformthema misst man eine Ersatzschrift statt der Systemschrift,
# und alle Größenverhältnisse stimmen nicht mehr (CLAUDE.md).
export QT_QPA_PLATFORMTHEME=kde
# Die mitgelieferten Prüf-Themes neben die installierten, nicht an ihre Stelle.
export XDG_DATA_DIRS="$WURZEL/tests/themes:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

echo "== M1: Deckung, Rand und Farben der Feldgrafik je Theme (AK 6b, AK 1) =="
QT_QPA_PLATFORM=offscreen "$BAU/sonden/feldgrafik" > "$HIER/messungen/m1-feldgrafik-je-theme.txt"

echo "== M2: Mutationsproben (AK 9 — hier trägt nichts anderes) =="
QT_QPA_PLATFORM=offscreen bash "$HIER/mutationsproben.sh" \
    > "$HIER/messungen/m2-mutationsproben.txt" 2>&1

echo "== M3: Testlauf des Projekts (AK 1, 4, 5, 6a, 6b, 8, 9) =="
(cd "$BAU/projekt" && QT_QPA_PLATFORM=offscreen ctest --output-on-failure) \
    > "$HIER/messungen/m3-testlauf.txt" 2>&1 \
    || echo "   ACHTUNG: ctest war nicht grün — siehe m3-testlauf.txt"

echo "== M4: Bildreihe offscreen, auf der Skalierung des Kunden (AK 1, AK 5) =="
# 1,6 ist die Einstellung des Kunden, am 07.08.2026 bestätigt. Offscreen liefert
# QT_SCALE_FACTOR genau diesen Wert; unter Wayland multiplizierte es sich mit
# der Sitzungsskalierung, und deshalb steht es hier und nicht bei M5 (F10).
QT_QPA_PLATFORM=offscreen QT_SCALE_FACTOR=1.6 \
    "$BAU/projekt/bin/captureshots" "$HIER/bilder/offscreen" \
    > "$HIER/messungen/m4-bildreihe.txt"

echo "== M5: das Fenster in der angemeldeten Sitzung (AK 2, AK 4 — B21) =="
GESPERRT="$(loginctl show-session "$(loginctl show-user "$USER" -p Display --value)" \
            -p LockedHint --value 2>/dev/null || echo unbekannt)"
if [ -z "${WAYLAND_DISPLAY:-}" ]; then
    echo "   (übersprungen — keine angemeldete Sitzung)"
elif [ "$GESPERRT" != "no" ]; then
    echo "   (übersprungen — Sitzung gesperrt oder Zustand unbekannt: LockedHint=$GESPERRT)"
else
    # **Ohne QT_SCALE_FACTOR** (F10). Zwei Themes, die sich in der Feldgrafik
    # gemessen unterscheiden: `default` zeichnet dunkel, `breeze-light` hell.
    "$BAU/sonden/sitzungsbild" "$HIER/bilder/sitzung" default breeze-light \
        > "$HIER/messungen/m5-sitzungsbild.txt"
fi

echo "== M6: die Grenze steht in SPEC 3.1 (AK 6b, DoD 4/B9) =="
{
    echo "=== #100, M6: Textnachweis der Bedingung aus AK 6b in SPEC.md ==="
    echo "Gesucht wird die Deckungsgrenze, nicht eine Kontrastzahl."
    echo
    git -C "$WURZEL" grep -n "15 von 255" -- SPEC.md || echo "NICHT GEFUNDEN"
    echo
    echo "--- und der Feldrand bei den Innenabständen (AK 5):"
    git -C "$WURZEL" grep -n "widgets/lineedit" -- SPEC.md || echo "NICHT GEFUNDEN"
} > "$HIER/messungen/m6-spec-nachweis.txt"

echo
echo "Fertig. Protokolle in messungen/, Bilder in bilder/."
