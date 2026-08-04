#!/usr/bin/env bash
# Fährt die Messungen der UX-Beratung zu #83 (Planning-Beratung, 04.08.2026).
#
# Was das Skript nicht anfasst: `build/` der Repositoriumswurzel (dort arbeiten
# andere Agenten), `sonden/` und `build/` des Vorberichts (dort arbeitet der
# zweite Bearbeiter der Vorprüfung), und keine Einstellung des Kunden — jedes
# Farbschema bekommt ein eigenes XDG_CONFIG_HOME unter /tmp.
#
# Ein Lauf braucht keine angemeldete Sitzung; der Plattformvergleich wird ohne
# Wayland übersprungen und das gesagt.
#
# Aufruf: bash docs/scrum/vorberichte/83-native-huelle/ux-beratung/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BAU="$HIER/build"
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

mkdir -p "$HIER/messungen" "$HIER/bilder"

export QT_QPA_PLATFORMTHEME=kde
export XDG_CACHE_HOME="$FLUECHTIG/cache"

echo "== Sonden übersetzen (eigener Bauplatz) =="
cmake -B "$BAU" -S "$HIER/sonden" -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build "$BAU" -j "$(nproc)" > /dev/null

# Die vier Schemata: zwei Breeze-Enden, das des Kunden, und das schlechteste der
# Sprint-6-Messung.
SCHEMATA="BreezeLight BreezeDark CachyOSNordLightly KritaNeutral"
for s in $SCHEMATA; do
    mkdir -p "$FLUECHTIG/cfg-$s"
    cp "/usr/share/color-schemes/$s.colors" "$FLUECHTIG/cfg-$s/kdeglobals"
done

echo "== M1: Deckung je Desktop-Theme, über vier Farbschemata =="
{
    echo "Die Sprint-6-Messung (native-ak2-kontrast.txt) hat 20 Farbschemata unter"
    echo "EINEM Desktop-Theme gefahren. Die Deckung gehört aber dem Desktop-Theme."
    echo "Hier steht die andere Achse: acht Themes, vier Schemata."
    echo
    for s in $SCHEMATA; do
        echo "############ Farbschema $s"
        XDG_CONFIG_HOME="$FLUECHTIG/cfg-$s" QT_QPA_PLATFORM=offscreen \
            "$BAU/deckung" "$s"
        echo
    done
} > "$HIER/messungen/m1-deckung-je-theme.txt"

echo "== M2: Kontrast der gedämpften Kleintexte über alle Farbschemata =="
python3 "$HIER/sonden/kleintext.py" > "$HIER/messungen/m2-kleintexte.txt"

echo "== M3: Effektangaben der Desktop-Themes =="
{
    echo "Welche Themes verlangen den Kontrasteffekt des Compositors?"
    echo "Quelle: metadata.desktop / metadata.json des jeweiligen Themes,"
    echo "gelesen von Plasma::Theme (/usr/include/Plasma/plasma/theme.h:192-262)."
    echo
    for d in /usr/share/plasma/desktoptheme/*/; do
        t="$(basename "$d")"
        printf '%-24s ' "$t"
        treffer=""
        for f in "$d/metadata.json" "$d/metadata.desktop"; do
            [ -e "$f" ] || continue
            # `|| true` überall dort, wo ein leerer Treffer selbst die Aussage
            # ist: die meisten Themes machen keine Effektangaben, und das ist
            # kein Fehlschlag des Laufs.
            c="$(grep -oiE 'ContrastEffect|BlurBehindEffect|AdaptiveTransparency' "$f" 2>/dev/null | sort -u | tr '\n' ' ' || true)"
            [ -n "$c" ] && treffer="$treffer[$(basename "$f"): $c]"
        done
        true
        echo "${treffer:-(keine Effektangaben)}"
    done
    echo
    echo "--- die Werte selbst ---"
    for d in /usr/share/plasma/desktoptheme/*/; do
        f="$d/metadata.desktop"
        [ -e "$f" ] || continue
        if grep -qiE '^\[(ContrastEffect|AdaptiveTransparency|BlurBehindEffect)\]' "$f"; then
            echo "=== $(basename "$d")"
            grep -A5 -iE '^\[(ContrastEffect|AdaptiveTransparency|BlurBehindEffect)\]' "$f" || true
            echo
        fi
    done
    echo "--- Varianten des Hüllenbildes je Theme (Auswahlpfade nach ImageSet::setSelectors) ---"
    for d in /usr/share/plasma/desktoptheme/*/; do
        printf '%-24s ' "$(basename "$d")"
        find "$d" -name 'background.svg*' -path '*dialogs*' -printf '%P\n' 2>/dev/null | sort | tr '\n' ' '
        echo
    done
} > "$HIER/messungen/m3-effektangaben.txt"

echo "== M4: welche Bibliothek meldet welchen Effekt an =="
{
    echo "Am Binärcode gemessen, ohne etwas zu installieren."
    echo "libPlasmaQuick ist die Bibliothek hinter Plasmas eigenen Überlagerungen."
    echo
    for lib in /usr/lib/libPlasmaQuick.so.6.7.3 /usr/lib/libKF6Svg.so.6; do
        echo "=== $lib"
        # `|| true`: dass KSvg **keinen** dieser Aufrufe kennt, ist selbst ein
        # Befund — ein leerer Treffer darf den Lauf nicht abbrechen.
        nm -D --undefined-only "$lib" 2>/dev/null | c++filt \
            | { grep -iE 'KWindowEffects|KWindowShadow' || echo "(kein Treffer)"; } | sed 's/^ *//' | sort -u
        echo
    done
} > "$HIER/messungen/m4-effektanmeldungen.txt"

echo "== M5: Bilder — die Hülle über hellem und dunklem Grund =="
XDG_CONFIG_HOME="$FLUECHTIG/cfg-CachyOSNordLightly" QT_QPA_PLATFORM=offscreen \
    "$BAU/huellenbild" "$HIER/bilder" default breeze-light cachyos-emerald Iridescent-round \
    > "$HIER/messungen/m5-bilder.txt"

echo "== M6: liefert der native Weg offscreen und unter Wayland dasselbe? =="
{
    echo "Getrennt geprüft: die Hülle allein gegen die Hülle mit Text."
    echo "Beide Läufe bei Pixelverhältnis 1, damit nur die Plattform verschieden ist."
    echo
    if [ -n "${WAYLAND_DISPLAY:-}" ]; then
        XDG_CONFIG_HOME="$FLUECHTIG/cfg-CachyOSNordLightly" XDG_CACHE_HOME="$FLUECHTIG/c2" \
            "$BAU/huellenbild" "$FLUECHTIG/wayland" default breeze-light cachyos-emerald Iridescent-round > /dev/null
        for f in "$HIER"/bilder/*.png; do
            b="$(basename "$f")"
            [ -e "$FLUECHTIG/wayland/$b" ] || continue
            if [ "$(md5sum < "$f")" = "$(md5sum < "$FLUECHTIG/wayland/$b")" ]; then
                printf '%-34s GLEICH\n' "$b"
            else
                printf '%-34s verschieden\n' "$b"
            fi
        done
    else
        echo "(übersprungen — keine angemeldete Sitzung)"
    fi
} > "$HIER/messungen/m6-plattformvergleich.txt"

echo
echo "Fertig. Protokolle in messungen/, Bilder in bilder/."
