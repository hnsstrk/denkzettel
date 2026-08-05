#!/usr/bin/env bash
# Mutationsproben zu #85 (AK 6): Je tragender Zusicherung wird die Heilung
# **entfernt** und gezeigt, dass ein Prüfsatz rot wird.
#
# Ein Prüfsatz, der ohne die Heilung grün bleibt, prüft die Heilung nicht. Und
# ein Bericht, der die Proben nur aufzählt, belegt bloß, dass irgendetwas rot
# wurde — deshalb steht hier jede Probe mit ihrem **Eingriff im Wortlaut**,
# ihrem Lauf und ihrem erwarteten Ergebnis (karpathy-Befund K3 zu Sprint 7).
#
# Zwei der zwölf greifen nicht in den Code, sondern in das **Prüfgut** (M8) und in
# einen **Prüfsatz** (M11). Sie belegen Zusicherungen, die im Code gar nicht
# stehen: dass der Nachweis an einem mitgelieferten Prüf-Theme hängt und nicht
# an der Paketlage der Maschine, und dass ein Prüfsatz sein Theme selbst nennt,
# statt zu erben, was der Lauf davor hinterlassen hat.
#
# Das Skript arbeitet auf einer **Kopie** des Arbeitsbaums unter /tmp: Es ändert
# keine Datei des Repositoriums und hinterlässt keinen halb mutierten Stand,
# auch wenn es mittendrin abbricht.
#
# Aufruf: bash docs/scrum/reviews/sprint-08-s85-lesbarkeit/mutationsproben.sh

set -uo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
ARBEIT="$(mktemp -d)"
trap 'rm -rf "$ARBEIT"' EXIT

echo "Kopie des Arbeitsbaums nach $ARBEIT"
git -C "$WURZEL" ls-files -z | xargs -0 -I{} cp --parents "{}" "$ARBEIT/" 2>/dev/null
cd "$ARBEIT" || exit 1

FENSTER="src/capture/capturewindow.cpp"
PRUEFSAETZE="tests/capturetest.cpp"
FARBDATEI="tests/themes/plasma/desktoptheme/denkzettel-test-breit/colors"
BAU="$ARBEIT/build"

echo "Erstbau der Kopie"
cmake -B "$BAU" -S "$ARBEIT" -DCMAKE_BUILD_TYPE=Debug > "$ARBEIT/cmake.log" 2>&1
cmake --build "$BAU" -j "$(nproc)" --target capturetest > "$ARBEIT/bau.log" 2>&1 \
    || { echo "Die Kopie baut nicht — Abbruch."; tail -20 "$ARBEIT/bau.log"; exit 1; }

# Die Grundlinie, und sie gehört ins Protokoll: „rot nach dem Eingriff" sagt
# nichts, solange nicht dasteht, dass vorher alles grün war.
echo
echo "===== Grundlinie ohne jeden Eingriff"
env QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde "$BAU/bin/capturetest" 2>&1 \
    | grep -vE "QWARN|QDEBUG" | grep -E "^FAIL!|^Totals" | sed 's/^/  /'

# Adressbereiche, damit sed nicht eine gleichlautende Zeile anderswo trifft:
# `applyTextColours();` steht an zwei Stellen, und die beiden tragen
# verschiedene Zusicherungen.
LADEN="/^void CaptureWindow::reloadDesktopTheme/,/^}/"
FILTER="/^bool CaptureWindow::eventFilter/,/^}/"
ROLLENSATZ="/void CaptureTest::noteTextUsesTheWindowTextRole/,/^}/"
TOR="/^ThemeTextColours themeTextColoursOf/,/^}/"

lauf() {
    local nummer="$1"
    local was="$2"
    local erwartet="$3"
    shift 3
    echo
    echo "===== Probe $nummer — $was"
    echo "Eingriff: $*"
    echo "Erwartet: $erwartet"
    "$@"
    if ! cmake --build "$BAU" -j "$(nproc)" --target capturetest > "$ARBEIT/bau.log" 2>&1; then
        echo "ERGEBNIS: übersetzt nicht mehr — die Zusicherung hängt am Bau selbst."
        grep -m3 " error" "$ARBEIT/bau.log" | sed 's/^/  /'
    else
        local ausgabe
        # Dieselbe Umgebung, die `ctest` dem Prüfsatz gibt
        # (tests/CMakeLists.txt): Ohne `offscreen` läuft er in der angemeldeten
        # Sitzung, `sessionBlursBehindWindows()` sagt dort `true`, und vier
        # Prüfsätze der Hülle fallen unabhängig von jeder Mutation. Eine
        # Grundlinie, die schon rot ist, macht jedes „rot" bedeutungslos.
        ausgabe="$(env QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde \
            "$BAU/bin/capturetest" 2>&1 | grep -vE "QWARN|QDEBUG")"
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
    cp "$WURZEL/$FARBDATEI" "$FARBDATEI"
}

echo
echo "############ AK 1 — die Farbe kommt aus derselben Quelle wie die Fläche"

lauf M1 "Notiztext nimmt wieder die Schemafarbe, auch wenn das Theme eine hat" \
    "rot in noteTextComesFromTheThemesOwnColours, textColoursFollowADesktopThemeChange, themeTextColoursOutlastAColourSchemeChange" \
    sed -i 's|^        ? m_hull->color(KSvg::Svg::Text)$|        ? this->palette().color(QPalette::WindowText)|' "$FENSTER"

lauf M2 "gedämpfte Klasse nimmt wieder die Schemafarbe" \
    "rot in subtleTextsComeFromTheThemesOwnColours" \
    sed -i 's|^        ? m_themeText.inactive$|        ? this->palette().color(QPalette::PlaceholderText)|' "$FENSTER"

lauf M3 "das Tor sagt immer „dieses Theme bringt nichts mit\"" \
    "rot in readsTheTextColoursOfTheDesktopTheme und in beiden AK-1-Sätzen" \
    sed -i "$TOR s|^    if (file.isEmpty()) {$|    if (true) {|" "$FENSTER"

# Diese Probe hat bis zum 05.08.2026 nichts Eigenes geprüft: Das Prüfgut trug
# nur `[Colors:Window]`, also endete „falsche Gruppe gelesen" auf demselben
# `return {}` wie M3 („keine Datei gefunden") — dieselben fünf Fehlschläge,
# zwölf Eingriffe, aber elf Sachverhalte (karpathy-Befund K1 zu Sprint 8). Seit
# das Prüf-Theme eine `[Colors:View]`-Gruppe mit **abweichenden** Werten trägt,
# liefert die falsche Gruppe eine Farbe statt nichts, und die beiden Fehlerbilder
# trennen sich: M3 lässt den Notiztext auf die Schemafarbe fallen, M4 lässt ihn
# bei der Themefarbe und bricht die Zusicherung, dass die beiden Lesewege
# dasselbe sagen.
lauf M4 "das Tor liest die falsche Gruppe der colors-Datei" \
    "rot in readsTheTextColoursOfTheDesktopTheme und subtleTextsComeFromTheThemesOwnColours mit den View-Farben als Ist-Wert; noteTextComesFromTheThemesOwnColours fällt an der Zeile, die beide Lesewege gegeneinander hält" \
    sed -i 's|QStringLiteral("Colors:Window")|QStringLiteral("Colors:View")|' "$FENSTER"

echo
echo "############ AK 5 — die Farbe folgt dem Wechsel des Desktop-Themes"

lauf M5 "der Theme-Wechsel schreibt die Farben nicht mehr nach" \
    "rot in textColoursFollowADesktopThemeChange" \
    sed -i "$LADEN s|^    applyTextColours();$|    ;|" "$FENSTER"

echo
echo "############ AK 7 — die Themefarbe überlebt den Wechsel des Farbschemas"

# Die Falle der Story. **Ein Eingriff, der allein den AK-7-Satz fallen lässt,
# ist auf diesem Fenster nicht zu bauen, und das ist gemessen statt vermutet:**
# `QWidget::setPalette()` stellt seinem Widget das `QEvent::PaletteChange`
# unmittelbar zu — noch innerhalb des Aufrufs. Jede Mutation am Ereignisfilter
# läuft deshalb auch bei dem `setPalette()`, das applyTextColours() selbst
# auslöst, und trifft damit den Theme-Wechsel mit. Zwei Anläufe, beide gemessen:
# diese Probe und M6b daneben, beide fallen breiter als vorhergesagt.
#
# Was AK 7 trotzdem trägt: Der Satz ist der einzige, der **nach** einem
# Schemawechsel unter einem Theme mit `colors`-Datei vergleicht. Er fällt in
# beiden Proben an derselben Zeile mit `#ff232629` — der Schemafarbe, die er
# selbst gesetzt hat, und einem Wert, den kein anderer Prüfsatz erzeugt.
lauf M6 "die Themefarbe überlebt den Palettenwechsel nicht" \
    "rot in themeTextColoursOutlastAColourSchemeChange (Zeile des Schemawechsels); die AK-1-Sätze fallen mit, weil der Filter wieder einläuft" \
    sed -i "$FILTER s|^        applyTextColours();$|        m_themeText = {};\n        applyTextColours();|" "$FENSTER"

# Und der naive Bau daneben, weil er etwas zeigt, das man nicht vermutet: Wer
# die Themefarbe **außerhalb** von applyTextColours() schreibt, macht sie sich
# im selben Atemzug wieder zunichte. `QWidget::setPalette()` stellt seinem
# Widget das `QEvent::PaletteChange` unmittelbar zu, der Ereignisfilter läuft
# noch innerhalb des `setPalette()`-Aufrufs, und applyTextColours() überschreibt
# die eben gesetzte Farbe mit der des Schemas. Die Vorrangregel an einer Stelle
# ist damit nicht nur die sauberere Bauform, sondern die einzige, die trägt.
lauf M6b "die Vorrangregel wandert aus applyTextColours() in reloadDesktopTheme()" \
    "rot in den AK-1-Sätzen — der Ereignisfilter nimmt die Farbe sofort zurück" \
    bash -c "
        sed -i 's|^        ? m_hull->color(KSvg::Svg::Text)\$|        ? this->palette().color(QPalette::WindowText)|' '$FENSTER'
        sed -i '$LADEN s|^    applyTextColours();\$|    applyTextColours();\n    if (m_themeText.normal.isValid()) {\n        QPalette mutation = m_text->palette();\n        mutation.setColor(QPalette::Text, m_hull->color(KSvg::Svg::Text));\n        m_text->setPalette(mutation);\n    }|' '$FENSTER'
    "

lauf M7 "der Palettenwechsel erreicht die Textfarben nicht mehr" \
    "rot in textsFollowAColourSchemeChange und noteTextUsesTheWindowTextRole" \
    sed -i "$FILTER s|^        applyTextColours();$|        ;|" "$FENSTER"

echo
echo "############ Die beiden Klassen einzeln"

lauf M9 "der Platzhaltertext des leeren Feldes bekommt die Themefarbe nicht" \
    "rot in subtleTextsComeFromTheThemesOwnColours" \
    sed -i 's|^    palette.setColor(QPalette::PlaceholderText, subtleColour);$|    ;|' "$FENSTER"

lauf M10 "App-Name und Fußzeile bekommen die Themefarbe nicht" \
    "rot in subtleTextsComeFromTheThemesOwnColours" \
    sed -i 's|^        labelPalette.setColor(label->foregroundRole(), subtleColour);$|        ;|' "$FENSTER"

echo
echo "############ Prüfgut und Prüfsatz — Zusicherungen, die im Code nicht stehen"

lauf M8 "dem breiten Prüf-Theme wird seine colors-Datei genommen" \
    "rot — ohne mitgeliefertes Theme mit colors-Datei hinge AK 1 an installierten Themes" \
    rm -f "$FARBDATEI"

lauf M11 "noteTextUsesTheWindowTextRole() nennt sein Theme nicht mehr selbst" \
    "rot — der Satz erbt dann das plasmarc des Laufs davor, und das nennt seit #85 ein Theme mit colors-Datei" \
    sed -i "$ROLLENSATZ s|^    m_window->reloadDesktopTheme(NarrowBorderTheme);$|    ;|" "$PRUEFSAETZE"

echo
echo "Fertig. Zwölf Proben."
