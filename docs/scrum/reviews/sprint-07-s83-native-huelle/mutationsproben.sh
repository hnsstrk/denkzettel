#!/usr/bin/env bash
# Mutationsproben zu #83 (AK 14): Je tragender Zusicherung wird die Heilung
# **entfernt** und gezeigt, dass der Prüfsatz rot wird.
#
# Ein Prüfsatz, der auch ohne die Heilung grün bleibt, prüft die Heilung nicht.
# Derselbe Satz ohne Zählbarkeit hat in Sprint 6 acht von elf gedeckt und ist
# durch Nachzählen aufgefallen — deshalb steht hier je Probe eine eigene
# nummerierte Stufe, und der Bericht zählt sie einzeln auf.
#
# Das Skript arbeitet auf einer **Kopie** des Arbeitsbaums unter /tmp: Es ändert
# keine Datei des Repositoriums und hinterlässt keinen halb mutierten Stand,
# auch wenn es mittendrin abbricht.
#
# Aufruf: bash docs/scrum/reviews/sprint-07-s83-native-huelle/mutationsproben.sh

set -uo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
ARBEIT="$(mktemp -d)"
trap 'rm -rf "$ARBEIT"' EXIT

echo "Kopie des Arbeitsbaums nach $ARBEIT"
git -C "$WURZEL" ls-files -z | xargs -0 -I{} cp --parents "{}" "$ARBEIT/" 2>/dev/null
cd "$ARBEIT"

FENSTER="src/capture/capturewindow.cpp"
BAU="$ARBEIT/build"

lauf() {
    local nummer="$1"
    local was="$2"
    shift 2
    echo
    echo "===== Probe $nummer — $was"
    echo "Eingriff: $*"
    "$@"
    if ! cmake --build "$BAU" -j "$(nproc)" > "$ARBEIT/bau.log" 2>&1; then
        echo "ERGEBNIS: übersetzt nicht mehr — die Zusicherung hängt am Bau selbst."
        grep -m3 "error" "$ARBEIT/bau.log" | sed 's/^/  /'
    else
        local ausgabe
        ausgabe="$(ctest --test-dir "$BAU" -R capturetest --output-on-failure 2>&1)"
        if grep -q "100% tests passed" <<< "$ausgabe"; then
            echo "ERGEBNIS: **GRÜN GEBLIEBEN** — kein Prüfsatz deckt diese Zusicherung."
        else
            echo "ERGEBNIS: rot. Gefallene Prüfsätze:"
            grep -E "^FAIL!" <<< "$ausgabe" | sed 's/^/  /' | sort -u
        fi
    fi
    cp "$WURZEL/$FENSTER" "$FENSTER"
}

echo "== Ausgangsstand übersetzen =="
cmake -B "$BAU" -S . -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$BAU" -j "$(nproc)" > /dev/null
ctest --test-dir "$BAU" -R capturetest 2>&1 | tail -3

lauf 1 "Die Hülle ist die Grafik des Themes, in einem Stück" \
    sed -i 's|painter.drawPixmap(0, 0, m_hull->framePixmap());|painter.fillRect(rect(), palette().color(QPalette::Window));|' "$FENSTER"

lauf 2 "Die Hülle trägt das Bildpunktverhältnis des Fensters" \
    sed -i 's|    m_hull->setDevicePixelRatio(devicePixelRatioF());|    // entfernt (Mutationsprobe)|' "$FENSTER"

lauf 3 "Die Hülle wird auf die Fenstergröße gebracht" \
    sed -i 's|    m_hull->resizeFrame(size());|    // entfernt (Mutationsprobe)|' "$FENSTER"

lauf 4 "Gezeichnet wird framePixmap(), nicht die gröbere Alphamaske" \
    sed -i 's|painter.drawPixmap(0, 0, m_hull->framePixmap());|painter.drawPixmap(0, 0, m_hull->alphaMask());|' "$FENSTER"

lauf 5 "Die Eckform kommt aus dem Theme, nicht aus einem eigenen Radius" \
    sed -i 's|    painter.drawPixmap(0, 0, m_hull->framePixmap());|    painter.setPen(Qt::NoPen); painter.setBrush(QColor(30, 34, 51)); painter.drawRoundedRect(rect(), 12, 12);|' "$FENSTER"

lauf 6 "Die Kontrastwerte kommen aus der Gruppe des Themes" \
    sed -i 's|                             QStringLiteral("ContrastEffect"));|                             QStringLiteral("KeineSolcheGruppe"));|' "$FENSTER"

lauf 7 "Ohne weichzeichnende Sitzung gilt der Auswahlpfad opaque" \
    sed -i 's|        imageSet->setSelectors({QString(OpaqueSelector)});|        // entfernt (Mutationsprobe)|' "$FENSTER"

lauf 8 "Die Ermittlung antwortet ohne Compositor mit nein" \
    sed -i 's|    if (!KWindowSystem::isPlatformWayland() \&\& !KWindowSystem::isPlatformX11()) {|    if (false) {|' "$FENSTER"

lauf 9 "Der Farbsatz der Hülle ist der eines Dialoggrundes" \
    sed -i 's|    m_hull->setColorSet(KSvg::Svg::Window);|    // entfernt (Mutationsprobe)|' "$FENSTER"

lauf 10 "Das Verhältnis wird bei DevicePixelRatioChange nachgezogen" \
    sed -i 's|    if (event->type() == QEvent::DevicePixelRatioChange) {|    if (false) {|' "$FENSTER"

lauf 11 "Die Anmeldungen bekommen kein nullptr-Fenster" \
    sed -i 's|    if (!windowHandle() \|\| !m_hull->isValid()) {|    if (false) {|' "$FENSTER"

lauf 12 "Der Weichzeichner wird überhaupt angemeldet" \
    sed -i 's|    KWindowEffects::enableBlurBehind(windowHandle(), true, region);|    // entfernt (Mutationsprobe)|' "$FENSTER"

echo
echo "Fertig. Der Arbeitsbaum des Repositoriums wurde nicht angefasst."
