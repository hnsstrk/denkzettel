#!/usr/bin/env bash
# Mutationsproben zu #100 — und für AK 9 sind sie nicht eine Beigabe, sondern
# das **einzige** Prüfmittel.
#
# Der Grund steht im Vorprüfbericht, §0.1: Die beiden Prüfsätze, die am
# Fenstermittelpunkt abgreifen, werden vom Feld nicht rot gemacht. Sie bleiben
# grün — überall, wo sie heute laufen — und messen ab dann das Feld statt der
# Hülle. Ein grüner Lauf belegt hier also nichts; nur die Frage „findet der
# reparierte Prüfsatz die Mutation wieder, gegen die er gebaut wurde?" tut es.
# Probe 5 fährt dieselbe Mutation am **alten** Abgriff und ist die Gegenprobe:
# Sie muss grün bleiben, sonst hätte die Reparatur nichts repariert.
#
# Das Skript arbeitet auf einer Kopie des Arbeitsbaums unter /tmp. Es ändert
# keine Datei des Repositoriums und hinterlässt keinen halb mutierten Stand,
# auch wenn es mittendrin abbricht.
#
# Aufruf: bash docs/scrum/reviews/sprint-09-s100-eingabefeld/mutationsproben.sh

set -uo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
ARBEIT="$(mktemp -d)"
trap 'rm -rf "$ARBEIT"' EXIT

echo "Kopie des Arbeitsbaums nach $ARBEIT"
git -C "$WURZEL" ls-files -z | xargs -0 -I{} cp --parents "{}" "$ARBEIT/" 2>/dev/null
cd "$ARBEIT"

FENSTER="src/capture/capturewindow.cpp"
PRUEFSATZ="tests/capturetest.cpp"
BAU="$ARBEIT/build"

lauf() {
    local nummer="$1"
    local was="$2"
    local erwartet="$3"
    shift 3
    echo
    echo "===== Probe $nummer — $was"
    echo "Erwartet: $erwartet"
    # **Jeder** Eingriff wird einzeln nachgewogen, nicht die Probe als Ganzes.
    # Der Unterschied hat einmal ein falsches Ergebnis geliefert: Probe 5 hat
    # zwei Eingriffe, und als eine Signatur im Prüfsatz sich änderte, griff der
    # zweite nicht mehr. Der erste griff weiter, die Wache sah eine veränderte
    # Datei und ließ die Probe laufen — sie meldete „rot", wo „grün" der Beleg
    # gewesen wäre (08.08.2026). Eine Wache über die Summe der Eingriffe wacht
    # über keinen einzelnen.
    local eingriff
    local vorher
    local nachher
    for eingriff in "$@"; do
        vorher="$(cat "$ARBEIT/$FENSTER" "$ARBEIT/$PRUEFSATZ" | sha256sum)"
        eval "$eingriff" || echo "  ACHTUNG: Eingriff schlug fehl: $eingriff"
        nachher="$(cat "$ARBEIT/$FENSTER" "$ARBEIT/$PRUEFSATZ" | sha256sum)"
        if [ "$vorher" = "$nachher" ] && [ "$eingriff" != "true" ]; then
            echo "ERGEBNIS: ABGEBROCHEN — dieser Eingriff hat nichts verändert:"
            echo "  $eingriff"
            cp "$WURZEL/$FENSTER" "$ARBEIT/$FENSTER"
            cp "$WURZEL/$PRUEFSATZ" "$ARBEIT/$PRUEFSATZ"
            return
        fi
    done
    if ! cmake --build "$BAU" -j "$(nproc)" > "$ARBEIT/bau.log" 2>&1; then
        echo "ERGEBNIS: übersetzt nicht mehr — die Zusicherung hängt am Bau selbst."
        grep -m3 "error" "$ARBEIT/bau.log" | sed 's/^/  /'
    else
        local ausgabe
        ausgabe="$(ctest --test-dir "$BAU" -R capturetest --output-on-failure 2>&1)"
        if grep -q "100% tests passed" <<< "$ausgabe"; then
            echo "ERGEBNIS: **GRÜN GEBLIEBEN** — kein Prüfsatz deckt diesen Eingriff."
        else
            echo "ERGEBNIS: rot. Gefallene Prüfsätze:"
            grep -E "^FAIL!" <<< "$ausgabe" | sed -E 's/^/  /; s/ \(.*//' | sort -u
        fi
    fi
    cp "$WURZEL/$FENSTER" "$FENSTER"
    cp "$WURZEL/$PRUEFSATZ" "$PRUEFSATZ"
}

# Die Gegenprobe zum Überspringen (Proben 8 und 9). Sie fährt den Prüflauf ohne
# Plasma-Grafiken auf dem Datenpfad — die Lage des öffentlichen Läufers, der am
# 07.08.2026 mit sechs gefallenen Prüfsätzen rot war.
#
# Zwei Zahlen entscheiden, und beide stehen im Ergebnis: die **Fehlschläge**
# müssen null sein, und die **Übersprungenen** müssen größer als null sein. Ein
# Lauf ohne Übersprungene wäre kein Beleg, sondern ein Hinweis darauf, dass die
# Lage gar nicht hergestellt wurde.
ohne_grafik() {
    local nummer="$1"
    local was="$2"
    local erwartet="$3"
    shift 3
    echo
    echo "===== Probe $nummer — $was"
    echo "Erwartet: $erwartet"
    local eingriff
    for eingriff in "$@"; do
        eval "$eingriff" || echo "  ACHTUNG: Eingriff schlug fehl: $eingriff"
    done
    if ! cmake --build "$BAU" -j "$(nproc)" > "$ARBEIT/bau.log" 2>&1; then
        echo "ERGEBNIS: übersetzt nicht mehr."
        grep -m3 "error" "$ARBEIT/bau.log" | sed 's/^/  /'
    else
        local ausgabe
        # XDG_DATA_DIRS zeigt ins Leere: die mitgelieferten Prüf-Themes kommen
        # über DENKZETTEL_TEST_THEMES trotzdem auf den Pfad, die Plasma-Grafiken
        # nicht. Genau das ist der Unterschied zwischen dieser Maschine und dem
        # Läufer — `ksvg` hängt nicht von `libplasma` ab.
        ausgabe="$(XDG_DATA_DIRS=/nonexistent QT_QPA_PLATFORM=offscreen \
                   QT_QPA_PLATFORMTHEME=kde "$BAU/bin/capturetest" 2>&1)"
        echo "ERGEBNIS: $(grep -E '^Totals' <<< "$ausgabe")"
        grep -E "^FAIL!" <<< "$ausgabe" | sed -E 's/^/  /; s/ \(.*//' | sort -u
        echo "  Übersprungene Prüfsätze zum Feld:"
        grep -E "^SKIP" <<< "$ausgabe" | grep -oE "CaptureTest::[a-zA-Z]+" | sed 's/^/    /'
    fi
    cp "$WURZEL/$FENSTER" "$FENSTER"
    cp "$WURZEL/$PRUEFSATZ" "$PRUEFSATZ"
}

echo "== Ausgangsstand übersetzen =="
cmake -B "$BAU" -S . -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$BAU" -j "$(nproc)" > /dev/null
ctest --test-dir "$BAU" -R capturetest 2>&1 | tail -3

lauf 1 "Das Feld wird gar nicht gezeichnet (AK 1, AK 4, AK 6b)" \
    "rot" \
    'sed -i "s|if (m_field->isValid()) {|if (false) {|" "$FENSTER"'

lauf 2 "Das Feld bleibt aus der Schleife über das frische ImageSet (AK 4, F5)" \
    "rot — sonst folgt das Feld dem Theme-Wechsel lautlos nicht" \
    'perl -0pi -e "s/\{m_hull, m_shadowTiles, m_field\}\) \{\n        frame->setImageSet/{m_hull, m_shadowTiles}) {\n        frame->setImageSet/s" "$FENSTER"'

lauf 3 "Der Feldrand rückt den Text nicht nach innen (AK 5)" \
    "rot" \
    'sed -i "s|^    applyFieldMargin();$|    // mutiert|" "$FENSTER"'

lauf 4 "Der Auswahlpfad opaque fällt weg — am **reparierten** Abgriff (AK 9)" \
    "rot; das ist das Prüfmittel von AK 9" \
    'perl -0pi -e "s/    if \(!m_blursBehind\) \{\n        imageSet->setSelectors\(\{QString\(OpaqueSelector\)\}\);\n    \}/    \/\/ mutiert/s" "$FENSTER"'

lauf 5 "Derselbe Wegfall am **alten** Abgriff, dem Fenstermittelpunkt (AK 9, Gegenprobe)" \
    "GRÜN — genau das ist der Befund: der alte Abgriff hätte die Mutation durchgelassen" \
    'perl -0pi -e "s/    if \(!m_blursBehind\) \{\n        imageSet->setSelectors\(\{QString\(OpaqueSelector\)\}\);\n    \}/    \/\/ mutiert/s" "$FENSTER"' \
    'perl -0pi -e "s/const QPoint sample = inPicture\(besideTheField\(\*m_window, narrowText\)\);/const QPoint sample = inPicture(QPoint(m_window->width() \/ 2, m_window->height() \/ 2));/" "$PRUEFSATZ"' \
    'perl -0pi -e "s/    QVERIFY2\(sample\.y\(\) < inPicture.*?\)\)\);\n/    Q_UNUSED(text);\n/s" "$PRUEFSATZ"'

lauf 6 "Das Feld wird über das ganze Fenster gelegt (AK 9, zweiter Abgriff)" \
    "rot — der Abgriff von hullIsCompleteAtFiveAndEightLines() misst wieder die Hülle" \
    'sed -i "s|painter.drawPixmap(m_text->pos(), m_field->framePixmap());|painter.drawPixmap(QPoint(0, 0), m_field->framePixmap());|" "$FENSTER"'

lauf 7 "Das Feld bekommt das Bildpunktverhältnis des Fensters nicht (F4)" \
    "rot allein über den Lauf bei 1,6 — bei Verhältnis 1 ist der Fehler unsichtbar" \
    'sed -i "s|    m_field->setDevicePixelRatio(devicePixelRatioF());|    // mutiert|" "$FENSTER"'

ohne_grafik 8 "Ohne Plasma-Grafik, **unmutiert** — die Lage des öffentlichen Läufers" \
    "0 Fehlschläge und mehr als 0 Übersprungene; jeder Skip nennt seinen Grund" \
    'true'

ohne_grafik 9 "Ohne Plasma-Grafik, **mit** Mutation 1 (Feld nicht zeichnen)" \
    "grün — und das ist keine Lücke, sondern die Kehrseite: wo nichts zu messen ist, fängt der Läufer diese Mutation nicht. Probe 1 fängt sie dort, wo die Grafik da ist." \
    'sed -i "s|if (m_field->isValid()) {|if (false) {|" "$FENSTER"'

echo
echo "Fertig."
