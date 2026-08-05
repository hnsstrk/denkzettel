#!/usr/bin/env bash
# Fährt die Messungen der Vorprüfung zu #85, Bearbeiter A (UX), 05.08.2026.
#
# Was das Skript nicht anfasst: `build/` der Repositoriumswurzel (dort arbeiten
# andere Stränge), `/usr` (es installiert nichts), und keine Einstellung des
# Kunden — seine `kdeglobals` wird gelesen und kopiert, nie geschrieben, und
# seine `plasmarc` wird gar nicht gebraucht: die Sonden bekommen den Themenamen
# übergeben.
#
# Eigener Bauplatz des Projekts: `build-vor85` in der Repositoriumswurzel.
#
# M5 braucht eine **entsperrte** angemeldete Sitzung. Bei gesperrtem Bildschirm
# liefert `spectacle -f` ein schwarzes Bild, und die Sonde bricht ab, statt eine
# Zahl daraus zu erfinden (gemessen am 05.08.2026).
#
# Aufruf: bash docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"
PROJEKTBAU="$WURZEL/build-vor85"
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

mkdir -p "$HIER/messungen" "$HIER/bilder"
mkdir -p "$FLUECHTIG/cfg"
cp "$HOME/.config/kdeglobals" "$FLUECHTIG/cfg/kdeglobals"
chmod u+w "$FLUECHTIG/cfg/kdeglobals"

export QT_QPA_PLATFORMTHEME=kde
export XDG_CACHE_HOME="$FLUECHTIG/cache"

THEMES="default breeze-dark breeze-light CachyOS-Nord-round Iridescent-round cachyos-emerald cachyos-emerald-color cachyos-emerald-light"

echo "== Projektbibliothek in eigenem Bauplatz =="
cmake -B "$PROJEKTBAU" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$PROJEKTBAU" --target denkzettelcapture -j "$(nproc)" > /dev/null

echo "== Sonden übersetzen =="
cmake -B "$BAU" -S "$HIER/sonden" -DCMAKE_BUILD_TYPE=Release -DDENKZETTEL_BUILD="$PROJEKTBAU" > /dev/null
cmake --build "$BAU" -j "$(nproc)" > /dev/null

echo "== M1: Woher käme die Schriftfarbe? Ein Prozess je Theme =="
{
    echo "M1 — Woher käme die Schriftfarbe? Je Theme ein eigener Prozess."
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. Offscreen, QT_QPA_PLATFORMTHEME=kde."
    echo
    for t in $THEMES; do
        QT_QPA_PLATFORM=offscreen "$BAU/themetext" "$t" | tail -n +3
    done
} > "$HIER/messungen/m1-schriftquelle-je-theme.txt"

echo "== M2: Reihenlauf gegen Einzelläufe =="
{
    echo "M2 — Acht Themes in EINEM Prozess gegen acht eigene Prozesse."
    echo
    echo "### Reihenlauf"
    # shellcheck disable=SC2086
    QT_QPA_PLATFORM=offscreen "$BAU/themetext" $THEMES \
        | grep -E '^====|KSvg::Svg::color\(Text\)|gezeichnet'
    echo
    echo "### Einzelläufe (aus M1)"
    grep -E '^====|KSvg::Svg::color\(Text\)|gezeichnet' "$HIER/messungen/m1-schriftquelle-je-theme.txt"
} > "$HIER/messungen/m2-reihenlauf-gegen-einzellauf.txt"

echo "== M3: Nebenlauf und Theme-Wechsel =="
{
    echo "M3 — Färbt ein lebendes Fenster einen Bildsatz daneben?"
    echo
    echo "############ Fenster breeze-light, gemessen breeze-light und default"
    XDG_CONFIG_HOME="$FLUECHTIG/cfg" QT_QPA_PLATFORM=offscreen "$BAU/nebenlauf" breeze-light default
    echo
    echo "############ Fenster cachyos-emerald-color, gemessen dieses und -light"
    XDG_CONFIG_HOME="$FLUECHTIG/cfg" QT_QPA_PLATFORM=offscreen \
        "$BAU/nebenlauf" cachyos-emerald-color cachyos-emerald-light
} > "$HIER/messungen/m3-nebenlauf-und-wechsel.txt"

echo "== M4: das gebaute Fenster von heute =="
{
    echo "M4 — Was das gebaute Fenster (Stand nach #83) mit beiden Textklassen tut."
    echo
    # shellcheck disable=SC2086
    XDG_CONFIG_HOME="$FLUECHTIG/cfg" QT_QPA_PLATFORM=offscreen "$BAU/fenstertext" $THEMES
} > "$HIER/messungen/m4-fenster-heute.txt"

echo "== M5: Sitzungsbeleg über benanntem Grund (braucht entsperrten Bildschirm) =="
{
    echo "M5 — AK 2 am wirklichen Fall."
    echo
    echo "### Lage der Sitzung zur Messzeit"
    loginctl show-session "$(loginctl | awk '/'"$USER"'/{print $1; exit}')" -p Type -p Active -p LockedHint || true
    echo
    if [ -n "${WAYLAND_DISPLAY:-}" ]; then
        XDG_CONFIG_HOME="$FLUECHTIG/cfg" "$BAU/sitzungsgrund" "$HIER/bilder" \
            cachyos-emerald-light weiss 255 255 255 || echo "Rückgabe: $?"
    else
        echo "(übersprungen — keine angemeldete Sitzung)"
    fi
} > "$HIER/messungen/m5-sitzung-benannter-grund.txt"

echo "== M6: was der Testmodus mit der Messgrundlage macht =="
{
    echo "M6 — Sieht ein QTest dieses Projekts dasselbe Farbschema wie der Kunde?"
    echo
    echo "### ohne Testmodus"
    QT_QPA_PLATFORM=offscreen "$BAU/themetext" default breeze-light | head -20
    echo
    echo "### mit Testmodus (wie capturetest, tests/capturetest.cpp:157)"
    DENKZETTEL_SONDE_TESTMODUS=1 QT_QPA_PLATFORM=offscreen "$BAU/themetext" default breeze-light | head -20
} > "$HIER/messungen/m6-testmodus.txt"

echo "== M7: folgt die Flächenfarbe dem Farbschema? Zwei Schemata =="
{
    echo "M7 — Zwei Farbschemata, acht Themes."
    echo
    for s in BreezeLight CachyOSNordLightly; do
        mkdir -p "$FLUECHTIG/$s"
        cp "/usr/share/color-schemes/$s.colors" "$FLUECHTIG/$s/kdeglobals"
        echo "############ Farbschema $s"
        # shellcheck disable=SC2086
        XDG_CONFIG_HOME="$FLUECHTIG/$s" QT_QPA_PLATFORM=offscreen "$BAU/themetext" $THEMES \
            | grep -E 'Farbschema der Anwendung|^====|gezeichnet lose|KSvg::Svg::color\(Text\)'
        echo
    done
} > "$HIER/messungen/m7-folgt-dem-schema.txt"


echo "== M8: hat die Kontrast-Anmeldung einen Empfänger? =="
{
    echo "M8 — Kontrasteffekt des Compositors."
    echo
    echo -n "isEffectLoaded(blur)              : "
    busctl --user call org.kde.KWin /Effects org.kde.kwin.Effects isEffectLoaded s blur || true
    echo -n "isEffectLoaded(backgroundcontrast): "
    busctl --user call org.kde.KWin /Effects org.kde.kwin.Effects isEffectLoaded s backgroundcontrast || true
    echo -n "Treffer 'contrast' in listOfEffects: "
    busctl --user get-property org.kde.KWin /Effects org.kde.kwin.Effects listOfEffects \
        | tr ' ' '\n' | tr -d '"' | grep -ic contrast || echo 0
} > "$HIER/messungen/m8-kontrasteffekt.txt"

echo
echo "Fertig. Protokolle in messungen/, Bilder in bilder/."
