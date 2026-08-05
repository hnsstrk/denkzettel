#!/usr/bin/env bash
# Mutationsproben zu #83, zweiter Teil: die drei Zusicherungen, die **kein
# Prüfsatz dieses Projekts decken kann**, weil ihr Fehler offscreen gar nicht
# auftritt (AK 14, und der Grund steht in SPEC 16).
#
# `mutationsproben.sh` hat sie einzeln benannt und grün gelassen — das ist
# ehrlich, aber kein Nachweis. Hier wird derselbe Eingriff gemacht und in der
# **angemeldeten Sitzung** gemessen, was dabei herauskommt.
#
#   S1  das Verhältnis wird bei DevicePixelRatioChange nachgezogen
#       → ohne die Zeile hinkt die Hülle bei 2, während das Fenster 1,6 ist
#   S2  die Anmeldungen bekommen kein nullptr-Fenster
#       → ohne die Wache stürzt enableBlurBehind unter Wayland ab (SIGSEGV)
#   S3  der Weichzeichner wird angemeldet
#       → ohne den Aufruf bleibt der Grund hinter der Hülle scharf
#
# Arbeitet auf einer Kopie unter /tmp; das Repositorium wird nicht angefasst.
#
# Aufruf: bash docs/scrum/reviews/sprint-07-s83-native-huelle/mutationsproben-sitzung.sh

set -uo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
ARBEIT="$(mktemp -d)"
trap 'rm -rf "$ARBEIT"' EXIT

if [ -z "${WAYLAND_DISPLAY:-}" ]; then
    echo "Keine angemeldete Sitzung — diese Proben haben hier keinen Gegenstand."
    exit 0
fi

echo "Kopie des Arbeitsbaums nach $ARBEIT"
git -C "$WURZEL" ls-files -z | xargs -0 -I{} cp --parents "{}" "$ARBEIT/" 2>/dev/null
cd "$ARBEIT"

FENSTER="src/capture/capturewindow.cpp"
export QT_QPA_PLATFORMTHEME=kde
export XDG_DATA_DIRS="$WURZEL/tests/themes:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

uebersetzen() {
    cmake -B "$ARBEIT/bau/projekt" -S "$ARBEIT" -DCMAKE_BUILD_TYPE=Debug > /dev/null
    cmake --build "$ARBEIT/bau/projekt" -j "$(nproc)" \
        --target denkzettelcapture denkzettelstore > /dev/null
    cmake -B "$ARBEIT/bau/sonden" -S "$HIER/sonden" \
        -DDENKZETTEL_LIB_DIR="$ARBEIT/bau/projekt/lib" > /dev/null
    cmake --build "$ARBEIT/bau/sonden" -j "$(nproc)" > /dev/null
}

echo "== Ausgangsstand =="
uebersetzen
"$ARBEIT/bau/sonden/fensterlage" "$ARBEIT/bilder" 2>&1 | grep -E "Fenster-DPR"  | head -1

echo
echo "===== S1 — DevicePixelRatioChange wird nicht mehr beachtet"
sed -i 's|    if (event->type() == QEvent::DevicePixelRatioChange) {|    if (false) {|' "$FENSTER"
uebersetzen
"$ARBEIT/bau/sonden/fensterlage" "$ARBEIT/bilder" 2>&1 | grep -E "Fenster-DPR|Bild " | head -2
cp "$WURZEL/$FENSTER" "$FENSTER"

echo
echo "===== S2 — die Wache gegen ein nullptr-Fenster fällt weg"
sed -i 's|    if (!windowHandle() \|\| !m_hull->isValid()) {|    if (false) {|' "$FENSTER"
uebersetzen
"$ARBEIT/bau/sonden/fensterlage" "$ARBEIT/bilder" > /dev/null 2>&1
rc=$?
if [ "$rc" -ge 128 ]; then
    echo "Rückgabe $rc — Signal $((rc - 128)) (11 = SIGSEGV). Der Aufruf stürzt ab."
else
    echo "Rückgabe $rc — kein Absturz."
fi
cp "$WURZEL/$FENSTER" "$FENSTER"

echo
echo "===== S3 — der Weichzeichner wird nicht mehr angemeldet"
sed -i 's|    KWindowEffects::enableBlurBehind(windowHandle(), true, region);|    // entfernt (Mutationsprobe)|' "$FENSTER"
uebersetzen
"$ARBEIT/bau/sonden/weichzeichner" "$ARBEIT/bilder" an 2>&1 | grep -E "Spannweite|Hülle im Bild"
cp "$WURZEL/$FENSTER" "$FENSTER"

echo
echo "Fertig. Zum Vergleich der Ausgangsstand: Hülle und Fenster bei 1,6 gleich,"
echo "kein Absturz, Spannweite im Innenstreifen 6 (gegen 39 ohne Anmeldung)."
