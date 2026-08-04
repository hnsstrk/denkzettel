#!/usr/bin/env bash
# Messsonde zur Frage des PO:
#
#   „Zieht KWin für native Fenster überhaupt seine Schatten aus denselben
#    Kacheln, die wir übergeben?"
#
# Die Frage ist am Binärcode zu beantworten, ohne etwas zu installieren: Wenn
# die Fensterdekoration das Desktop-Theme nicht kennt, kann sie ihren Schatten
# nicht daraus nehmen.
#
# Aufruf: bash dekorationsquelle.sh

set -uo pipefail

DEKO=/usr/lib/qt6/plugins/org.kde.kdecoration3/org.kde.breeze.so
KCM=/usr/lib/qt6/plugins/org.kde.kdecoration3.kcm/kcm_breezedecoration.so

echo "=== Woher nimmt die Fensterdekoration ihren Schatten? ==="
echo "Geprüft: $DEKO"
echo

echo "--- Kennt sie das Desktop-Theme, aus dem wir unsere Kacheln nehmen?"
for muster in desktoptheme "dialogs/background" "plasma/desktoptheme" KSvg FrameSvg shadow-topleft; do
    if strings -a "$DEKO" | grep -qF -- "$muster"; then
        printf '    %-24s GEFUNDEN\n' "$muster"
    else
        printf '    %-24s nicht enthalten\n' "$muster"
    fi
done

echo
echo "--- Bindet sie die Bibliotheken, mit denen man es lesen würde?"
if ldd "$DEKO" | grep -qiE "ksvg|libplasma"; then
    ldd "$DEKO" | grep -iE "ksvg|libplasma" | sed 's/^/    /'
else
    echo "    weder KSvg noch libplasma"
fi

echo
echo "--- Womit erzeugt sie ihren Schatten stattdessen?"
strings -a "$DEKO" | grep -oE "KDecoration3[0-9]*(DecorationShadow[0-9]*[A-Za-z]+)" | sort -u | sed 's/^/    /'
echo "    (KDecoration3::DecorationShadow — setShadow(QImage), setPadding(QMarginsF),"
echo "     setInnerShadowRect(QRectF): ein selbst gerechnetes Bild, kein Kachelsatz)"

echo
echo "--- Und welche Stellschrauben sie dafür anbietet:"
strings -a "$KCM" | grep -E "^shadow(Size|Strength|Color)$" | sort -u | sed 's/^/    /'

echo
echo "Schluss: Zwei getrennte Wege. Ein dekoriertes Fenster bekommt seinen Schatten"
echo "von der Dekoration, die ihn selbst rechnet; ein undekorierter Client wie"
echo "Denkzettel meldet ihn über KWindowShadow als Kachelsatz aus dem Desktop-Theme"
echo "an. Es gibt keine gemeinsame Quelle — „derselbe Schatten wie native Fenster\""
echo "ist über unseren Weg nicht einzustellen, sondern nur nachzubauen."

echo
echo "=== Was eine durchscheinende Überlagerung zusätzlich braucht ==="
echo "KWindowEffects (KF6::WindowSystem) bietet:"
grep -nE "enableBlurBehind|enableBackgroundContrast|isEffectAvailable" \
    /usr/include/KF6/KWindowSystem/kwindoweffects.h | sed 's/^/    /'
echo "    (Die Hülle des Themes deckt nur zu 85 % — ohne Weichzeichner hinter dem"
echo "     Fenster schlägt der Untergrund auf die Flächenfarbe durch. Plasmas eigene"
echo "     Überlagerungen melden dafür eine Region an; die Region ist die Maske,"
echo "     also genau das, wofür die mask-Elemente des Themes da sind.)"
