#!/usr/bin/env bash
# Fährt die Belege der Story #85 („Capture: Lesbarkeit unter fremden
# Desktop-Themes") in einem Zug.
#
# Was das Skript nicht anfasst: `/usr` (es installiert nichts) und keine
# Einstellung des Kunden — seine `kdeglobals` wird gelesen und in ein eigenes
# XDG_CONFIG_HOME kopiert, seine `plasmarc` gar nicht gebraucht: die Sonden
# bekommen den Themenamen übergeben.
#
# P5 braucht eine **entsperrte** angemeldete Sitzung. Bei gesperrtem Bildschirm
# liefert `spectacle -f` ein durchweg schwarzes Bild mit Rückgabe 0, und die
# Sonde bricht dann ab, statt eine Zahl daraus zu erfinden. Der Sperrzustand
# steht zu Beginn von P5 im Protokoll.
#
# Aufruf: bash docs/scrum/reviews/sprint-08-s85-lesbarkeit/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
VOR="$WURZEL/docs/scrum/vorberichte/85-lesbarkeit-fremde-themes"
PROJEKTBAU="$WURZEL/build"
BAU_VOR="$HIER/build-vorsonden"
BAU_HIER="$HIER/build"
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

mkdir -p "$HIER/messungen" "$HIER/bilder"
mkdir -p "$FLUECHTIG/cfg"
cp "$HOME/.config/kdeglobals" "$FLUECHTIG/cfg/kdeglobals"
chmod u+w "$FLUECHTIG/cfg/kdeglobals"

export QT_QPA_PLATFORMTHEME=kde
export XDG_CACHE_HOME="$FLUECHTIG/cache"

THEMES="default breeze-dark breeze-light CachyOS-Nord-round Iridescent-round cachyos-emerald cachyos-emerald-color cachyos-emerald-light"
PRUEFTHEMES="$WURZEL/tests/themes"

echo "== Projekt bauen =="
cmake -B "$PROJEKTBAU" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$PROJEKTBAU" -j "$(nproc)" > /dev/null

echo "== Sonden übersetzen =="
# Die Messsonden der Vorprüfung werden von dort gebaut und nicht hierher
# kopiert: zwei Fassungen derselben Sonde gehen auseinander.
cmake -B "$BAU_VOR" -S "$VOR/sonden" -DCMAKE_BUILD_TYPE=Release \
    -DDENKZETTEL_BUILD="$PROJEKTBAU" > /dev/null
cmake --build "$BAU_VOR" -j "$(nproc)" > /dev/null
cmake -B "$BAU_HIER" -S "$HIER/sonden" -DCMAKE_BUILD_TYPE=Release \
    -DDENKZETTEL_BUILD="$PROJEKTBAU" > /dev/null
cmake --build "$BAU_HIER" -j "$(nproc)" > /dev/null

echo "== P1: Prüfsätze =="
{
    echo "P1 — ctest, vollständiger Lauf."
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed."
    echo
    QT_QPA_PLATFORMTHEME=kde ctest --test-dir "$PROJEKTBAU" --output-on-failure
    echo
    echo "### Die Prüfsätze dieser Story einzeln"
    QT_QPA_PLATFORMTHEME=kde "$PROJEKTBAU/bin/capturetest" \
        readsTheTextColoursOfTheDesktopTheme \
        noteTextComesFromTheThemesOwnColours \
        subtleTextsComeFromTheThemesOwnColours \
        textColoursFollowADesktopThemeChange \
        themeTextColoursOutlastAColourSchemeChange \
        noteTextUsesTheWindowTextRole \
        textsFollowAColourSchemeChange
} > "$HIER/messungen/p1-pruefsaetze.txt" 2>&1

echo "== P2: die Schriftquelle je Theme, beide Auswahlpfade (AK 1, AK 3) =="
{
    echo "P2 — Woher kommt die Schriftfarbe? Je installiertes Theme ein eigener Prozess."
    echo "Offscreen, QT_QPA_PLATFORMTHEME=kde, Farbschema aus der kdeglobals des Kunden."
    echo "Je Theme beide Auswahlpfade — seit #83 hat jedes Theme zwei Flächen."
    echo
    for t in $THEMES; do
        XDG_CONFIG_HOME="$FLUECHTIG/cfg" QT_QPA_PLATFORM=offscreen "$BAU_VOR/themetext" "$t" \
            | tail -n +3
    done
    echo
    echo "### Und die beiden mitgelieferten Prüf-Themes — das Paar, an dem die Prüfsätze hängen"
    XDG_DATA_DIRS="$PRUEFTHEMES:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}" \
        XDG_CONFIG_HOME="$FLUECHTIG/cfg" QT_QPA_PLATFORM=offscreen \
        "$BAU_VOR/themetext" denkzettel-test-breit denkzettel-test-schmal | tail -n +3
} > "$HIER/messungen/p2-schriftquelle-je-theme.txt"

echo "== P3: was das gebaute Fenster trägt (AK 1, AK 4, AK 5) =="
{
    echo "P3 — Beide Textklassen am gebauten CaptureWindow, Theme für Theme in EINEM Prozess."
    echo "Der Reihenlauf ist unbedenklich: zwei lebende ImageSet teilen ihre Auswahlpfade,"
    echo "ihre Farben nicht (Vorprüfung F4/F5) — und hier überlebt ohnehin keiner."
    echo "Zugleich der Beleg zu AK 5: jede Zeile entsteht nach einem Theme-Wechsel am"
    echo "stehenden Fenster, und die Farbe zieht jedes Mal nach."
    echo
    # shellcheck disable=SC2086
    XDG_CONFIG_HOME="$FLUECHTIG/cfg" QT_QPA_PLATFORM=offscreen "$BAU_VOR/fenstertext" $THEMES
} > "$HIER/messungen/p3-fenster-je-theme.txt"

echo "== P4: folgt das Theme dem Farbschema? Drei Schemata (AK 3) =="
{
    echo "P4 — Dieselben acht Themes unter drei Farbschemata."
    echo "Eine Aussage „folgt dem Schema\" braucht zwei Schemata; unter einem misst man"
    echo "Übereinstimmung, nicht Folgen. Deshalb drei."
    echo
    for s in BreezeLight BreezeDark CachyOSNordLightly; do
        mkdir -p "$FLUECHTIG/$s"
        cp "/usr/share/color-schemes/$s.colors" "$FLUECHTIG/$s/kdeglobals"
        echo "############ Farbschema $s"
        # shellcheck disable=SC2086
        XDG_CONFIG_HOME="$FLUECHTIG/$s" QT_QPA_PLATFORM=offscreen "$BAU_VOR/themetext" $THEMES \
            | grep -E 'Farbschema der Anwendung|^====|colors-Datei|KSvg::Svg::color\(Text\)'
        echo
    done
} > "$HIER/messungen/p4-folgt-dem-schema.txt"

echo "== P5: Sitzungsbeleg über benanntem Grund (AK 2) — braucht entsperrten Bildschirm =="
{
    echo "P5 — AK 2 am wirklichen Fall: die Zahl aus einer Aufnahme der angemeldeten"
    echo "Sitzung, über einem benannten Grund, mit den Farben, die das Fenster trägt."
    echo
    echo "### Lage der Sitzung zur Messzeit"
    date '+%F %H:%M %Z'
    loginctl show-session "$(loginctl | awk '/'"$USER"'/{print $1; exit}')" \
        -p Type -p Active -p LockedHint || true
    echo
    if [ -n "${WAYLAND_DISPLAY:-}" ]; then
        for fall in "cachyos-emerald-light weiss 255 255 255" \
                    "cachyos-emerald-light schwarz 0 0 0" \
                    "cachyos-emerald-color weiss 255 255 255" \
                    "breeze-light weiss 255 255 255" \
                    "default weiss 255 255 255" \
                    "CachyOS-Nord-round weiss 255 255 255"; do
            # shellcheck disable=SC2086
            XDG_CONFIG_HOME="$FLUECHTIG/cfg" "$BAU_HIER/sitzungsbeleg" "$HIER/bilder" $fall \
                || echo "Rückgabe: $?"
            echo
        done
    else
        echo "(übersprungen — keine angemeldete Sitzung)"
    fi
} > "$HIER/messungen/p5-sitzung-benannter-grund.txt" 2>&1

echo "== P6: hat die Kontrast-Anmeldung einen Empfänger? (geerbte Grenze) =="
{
    echo "P6 — Kontrasteffekt des Compositors. Unter den drei Emerald-Themes hängt die"
    echo "Lesbarkeit an diesem Effekt und nicht an der Textfarbe; diese Story erbt die"
    echo "Grenze aus #83 und schließt sie nicht."
    echo
    kwin_wayland --version 2>/dev/null || true
    echo -n "isEffectLoaded(blur)               : "
    busctl --user call org.kde.KWin /Effects org.kde.kwin.Effects isEffectLoaded s blur || true
    echo -n "isEffectLoaded(backgroundcontrast) : "
    busctl --user call org.kde.KWin /Effects org.kde.kwin.Effects isEffectLoaded s backgroundcontrast || true
    echo -n "Treffer 'contrast' in listOfEffects : "
    busctl --user get-property org.kde.KWin /Effects org.kde.kwin.Effects listOfEffects \
        | tr ' ' '\n' | tr -d '"' | grep -ic contrast || echo 0
    echo -n "Zahl der geladenen Effekte          : "
    busctl --user get-property org.kde.KWin /Effects org.kde.kwin.Effects listOfEffects \
        | tr ' ' '\n' | grep -c '"' || true
} > "$HIER/messungen/p6-kontrasteffekt.txt" 2>&1

echo "== P7: Prüfstand (B17 — eine Aussage gilt für einen Stand) =="
{
    echo "P7 — Der Stand, für den die Zahlen dieses Ordners gelten."
    date '+%F %H:%M %Z'
    echo
    pacman -Q kwin ksvg qt6-base libplasma plasma-desktop 2>/dev/null || true
    echo
    echo "Farbschema der Sitzung ([Colors:Window] der kdeglobals):"
    sed -n '/^\[Colors:Window\]/,/^\[/p' "$HOME/.config/kdeglobals" \
        | grep -E 'ForegroundNormal|ForegroundInactive|BackgroundNormal' || true
    echo
    echo -n "Desktop-Theme laut plasmarc: "
    (grep -A2 '^\[Theme\]' "$HOME/.config/plasmarc" 2>/dev/null | grep '^name=') \
        || echo "keines genannt — Rückfall default"
} > "$HIER/messungen/p7-pruefstand.txt" 2>&1

echo
echo "Fertig. Protokolle in messungen/, Bilder in bilder/."
