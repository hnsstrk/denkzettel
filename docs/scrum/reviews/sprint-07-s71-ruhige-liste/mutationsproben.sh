#!/usr/bin/env bash
# Mutationsproben zu #71, #70 und #72: Je tragender Zusicherung wird die Heilung
# **entfernt** und gezeigt, dass der Prüfsatz rot wird.
#
# Ein Prüfsatz, der ohne die Heilung grün bleibt, prüft die Heilung nicht. Der
# Bericht dieses Strangs zählte die vierzehn Proben zwar einzeln auf, hielt aber
# den **Eingriff** nirgends fest — damit war nachprüfbar nur, dass irgendetwas
# rot wurde (karpathy-Befund K3, 05.08.2026). Dieses Skript holt das nach: Jede
# Probe steht hier mit ihrem Eingriff, ihrem Lauf und ihrem erwarteten Ergebnis.
#
# Vier der vierzehn greifen absichtlich **in den Test** statt in den Code
# (M3–M6). Sie belegen Zusicherungen, die sonst von einer früheren Zeile
# verdeckt würden: QTest hält beim ersten Fehlschlag an, also fällt immer nur
# die vorderste. Wer die hinteren belegen will, muss die vorderen aussetzen.
#
# Das Skript arbeitet auf einer **Kopie** des Arbeitsbaums unter /tmp: Es ändert
# keine Datei des Repositoriums und hinterlässt keinen halb mutierten Stand,
# auch wenn es mittendrin abbricht.
#
# Aufruf: bash docs/scrum/reviews/sprint-07-s71-ruhige-liste/mutationsproben.sh

set -uo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
ARBEIT="$(mktemp -d)"
trap 'rm -rf "$ARBEIT"' EXIT

echo "Kopie des Arbeitsbaums nach $ARBEIT"
git -C "$WURZEL" ls-files -z | xargs -0 -I{} cp --parents "{}" "$ARBEIT/" 2>/dev/null
cd "$ARBEIT" || exit 1

FENSTER="src/ui/librarywindow.cpp"
PRUEFSAETZE="tests/librarytest.cpp"
BAU="$ARBEIT/build"

# Der Prüfsatz von #71 — sein Rumpf ist der Adressbereich für die Eingriffe in
# die Zusicherungen, damit sed nicht eine gleichlautende Zeile in einem der
# anderen achtzehn Prüfsätze trifft, die ebenfalls `currentIndex` vergleichen.
KLICKSATZ="/void LibraryTest::selectsTheClippedRowThatWasClickedAndLeavesThePictureWhereItIs/,/^}/"

lauf() {
    local nummer="$1"
    local was="$2"
    local erwartet="$3"
    local gebiet="$4"
    shift 4
    echo
    echo "===== Probe $nummer — $was"
    echo "Eingriff: $*"
    echo "Erwartet: $erwartet"
    "$@"
    if ! cmake --build "$BAU" -j "$(nproc)" --target librarytest > "$ARBEIT/bau.log" 2>&1; then
        echo "ERGEBNIS: übersetzt nicht mehr — die Zusicherung hängt am Bau selbst."
        grep -m3 " error" "$ARBEIT/bau.log" | sed 's/^/  /'
    else
        local ausgabe
        ausgabe="$(env LANG="$gebiet" LC_ALL="$gebiet" QT_QPA_PLATFORM=offscreen \
            "$BAU/bin/librarytest" 2>&1 | grep -vE "QWARN|QDEBUG")"
        if grep -qE "^Totals: [0-9]+ passed, 0 failed" <<< "$ausgabe"; then
            echo "ERGEBNIS: **GRÜN GEBLIEBEN** — kein Prüfsatz deckt diese Zusicherung."
        else
            echo "ERGEBNIS: rot."
            grep -A3 -E "^FAIL!" <<< "$ausgabe" | sed 's/^/  /'
        fi
        grep -E "^Totals" <<< "$ausgabe" | sed 's/^/  /'
    fi
    cp "$WURZEL/$FENSTER" "$FENSTER"
    cp "$WURZEL/$PRUEFSAETZE" "$PRUEFSAETZE"
}

echo "== Ausgangsstand übersetzen =="
# Die Kopie ist kein Git-Repositorium, deshalb schlägt der Versionsgriff des
# Bauwerks hier fehl und meldet es zweimal. Das ist folgenlos — gebaut wird
# derselbe Quelltext.
cmake -B "$BAU" -S . -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$BAU" -j "$(nproc)" --target librarytest > /dev/null
QT_QPA_PLATFORM=offscreen "$BAU/bin/librarytest" 2>&1 | grep -E "^Totals"

echo
echo "########## #71 — der Klick auf die angeschnittene Zeile ##########"

lauf 1 "Der Mausdruck rückt die Liste nicht nach (AK 1)" \
    "rot an selectedRows: „keine\" statt der geklickten Zeile" C \
    sed -i 's|        if (!m_selectionFollowsAPress) {|        if (true) {|' "$FENSTER"

lauf 2 "Der Merker des Drucks endet mit dem Loslassen (AK 6)" \
    "rot an dropsTheMarkOfAPressThatSelectedNothingWhenItEnds: Kopf bei y=-39" C \
    sed -i 's|    if (watched == m_list->viewport() \&\& event->type() == QEvent::MouseButtonRelease) {|    if (false) {|' \
    "$FENSTER"

# Ab hier greift jede Probe zusätzlich in den Prüfsatz: Die Zusicherung, die in
# der Probe davor gefallen ist, wird ausgesetzt, damit die nächste dahinter
# überhaupt erreicht wird.
lauf 3 "Der Rollwert steht still (AK 2, erste Hälfte)" \
    "rot am Rollwert: 2 statt 0" C \
    bash -c "
        sed -i 's|        if (!m_selectionFollowsAPress) {|        if (true) {|' '$FENSTER'
        sed -i '$KLICKSATZ s|^        QCOMPARE(selectedRows(list)|        // M3: |' '$PRUEFSAETZE'
        sed -i '$KLICKSATZ s|^        QCOMPARE(list->currentIndex(), target);|        // M3|' '$PRUEFSAETZE'
        sed -i '$KLICKSATZ s|^        QCOMPARE(readerOf(window)->toPlainText()|        // M3: |' '$PRUEFSAETZE'
    "

lauf 4 "Der Wächter des Aufbaus (AK 3)" \
    "rot an checked >= 10: „Nur 0 angeschnittene Rollwerte geprüft\"" C \
    sed -i '/^int bottomClippedRow(const QListView \*list)$/,/^{$/ s|^{$|{\n    return -1; // M4|' \
    "$PRUEFSAETZE"

lauf 5 "Aktuelle Zeile und Detailbereich — die Gegenprobe" \
    "GRÜN erwartet: beide sind im Fehlerbild von #71 richtig, sie fangen ihn nicht" C \
    bash -c "
        sed -i 's|        if (!m_selectionFollowsAPress) {|        if (true) {|' '$FENSTER'
        sed -i '$KLICKSATZ s|^        QCOMPARE(selectedRows(list)|        // M5: |' '$PRUEFSAETZE'
        sed -i '$KLICKSATZ s|^        QCOMPARE(list->verticalScrollBar()->value(), rolledTo);|        // M5|' '$PRUEFSAETZE'
        sed -i '$KLICKSATZ s|^        QCOMPARE(list->visualRect(target).y(), targetBefore);|        // M5|' '$PRUEFSAETZE'
    "

lauf 6 "Die Zeilenlage steht still (AK 2, zweite Hälfte)" \
    "rot an visualRect(target).y(): 444 statt 549" C \
    bash -c "
        sed -i 's|        if (!m_selectionFollowsAPress) {|        if (true) {|' '$FENSTER'
        sed -i '$KLICKSATZ s|^        QCOMPARE(selectedRows(list)|        // M6: |' '$PRUEFSAETZE'
        sed -i '$KLICKSATZ s|^        QCOMPARE(list->currentIndex(), target);|        // M6|' '$PRUEFSAETZE'
        sed -i '$KLICKSATZ s|^        QCOMPARE(readerOf(window)->toPlainText()|        // M6: |' '$PRUEFSAETZE'
        sed -i '$KLICKSATZ s|^        QCOMPARE(list->verticalScrollBar()->value(), rolledTo);|        // M6|' '$PRUEFSAETZE'
    "

echo
echo "########## #70 — der Kopf der ersten Notiz einer Gruppe ##########"

lauf 7 "Der neue Oder-Zweig holt den Kopf (AK 1)" \
    "rot an bringsTheHeadAlongWhenTheSelectionReachesTheFirstNoteOfItsGroup" C \
    sed -i 's|(crossesAGroupBoundary \|\| isFirstOfItsGroup)|crossesAGroupBoundary|' "$FENSTER"

lauf 8 "Der Grenzübertritt-Zweig bleibt erhalten (AK 2)" \
    "rot an vier Prüfsätzen, darunter bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready" C \
    sed -i 's|(crossesAGroupBoundary \|\| isFirstOfItsGroup)|isFirstOfItsGroup|' "$FENSTER"

lauf 9 "Der Merker bleibt im selben if — sonst öffnet #57 wieder (AK 3)" \
    "rot an leavesThePictureWhereItIsWhenTheFirstNoteOfAGroupIsClicked und am #57-Prüfsatz" C \
    sed -i 's| \&\& !m_selectionFollowsAPress) {|) {|' "$FENSTER"

lauf 10 "Die Passbedingung des Kopfholens (AK 5)" \
    "GRÜN erwartet — und das ist der Befund: kein Prüfsatz deckt sie (Issue #90)" C \
    sed -i 's|            if (selected.bottom() - heading.top() <= m_list->viewport()->height()) {|            if (true) {|' \
    "$FENSTER"

echo
echo "########## #72 — die Tooltips mit Tastenkürzel ##########"

lauf 11 "Alle drei Flächen tragen einen Tooltip (AK 1)" \
    "rot an namesTheShortcutInTheTooltipOfEachActionSurface: leerer Tooltip" C \
    sed -i '/setToolTip(tooltipNaming/d' "$FENSTER"

lauf 12 "„Rückgängig\" kommt über die QAction an der Fläche an (AK 5)" \
    "rot mit „Rückgängig\" statt „Löschen rückgängig machen (Ctrl+Z)\" — unter LANG=C heißt die Taste so, in einer deutschen Sitzung „Strg+Z\". Zugleich der Beleg zu AK 4: der Tooltip ist ohne setToolTip **nicht leer**" C \
    sed -i '/m_undoAction->setToolTip(tooltipNaming/d' "$FENSTER"

lauf 13 "Der Kürzelteil steht im Wortlaut (AK 3)" \
    "rot mit „Notiz bearbeiten\" statt „Notiz bearbeiten (F2)\"" C \
    sed -i 's|    return i18nc("@info:tooltip", "%1 (%2)", activity, action->shortcut().toString(QKeySequence::NativeText));|    Q_UNUSED(action)\n    return activity;|' \
    "$FENSTER"

# Diese eine läuft unter LANG=C, weil sie genau dort greift: Auf einer deutschen
# Sitzung wäre das Literal „Entf" für die Löschtaste richtig und die Probe bliebe
# grün — der öffentliche Lauf hat kein LANG und sagt „Del".
lauf 14 "Das Kürzel wird gelesen, nicht hingeschrieben (AK 2)" \
    "rot unter LANG=C mit „Notiz bearbeiten (Entf)\" statt „(F2)\"" C \
    sed -i 's|    return i18nc("@info:tooltip", "%1 (%2)", activity, action->shortcut().toString(QKeySequence::NativeText));|    Q_UNUSED(action)\n    return i18nc("@info:tooltip", "%1 (%2)", activity, QStringLiteral("Entf"));|' \
    "$FENSTER"

echo
echo "Fertig. Der Arbeitsbaum des Repositoriums wurde nicht angefasst."
echo "Zwei Proben bleiben erwartungsgemäß grün: M5 (Gegenprobe) und M10 (Issue #90)."
