#!/bin/bash
# Messungen des Bearbeiters B zur Vorpruefung #85. Wiederholbar.
#
# Baut nicht, installiert nicht, aendert keine Einstellung. Gelesen werden:
# der Quellcode dieses Repositoriums, /usr/share/plasma/desktoptheme/*/colors
# und die bereits abgelegte Compile-Datenbank unter build/lint/.
#
# Schreibt nach docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/messungen-b/.
set -u

WURZEL="$(cd "$(dirname "$0")/../../../.." && pwd)"
ZIEL="$WURZEL/docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/messungen-b"
cd "$WURZEL" || exit 1
mkdir -p "$ZIEL"

STAND="$(git rev-parse --short HEAD)"
ZEIT="$(date '+%d.%m.%Y %H:%M %Z')"

# b1 — Kontrast beider Textklassen je Desktop-Theme, deckend und ungestuetzt.
python3 "$ZIEL/kontraste.py" > "$ZIEL/b1-kontraste.txt" 2>&1

# b2 — Dateimenge am Code, Pruef-Themes, Sortierfalle, Farbquelle von KSvg.
{
    echo "=== Dateimenge #85, am Code vermessen — Stand $STAND, $ZEIT ==="
    echo
    echo "--- 1. Wo die Textfarbe heute entsteht (src/) ---"
    git grep -n "applyTextColours\|QPalette::Text\|QPalette::WindowText\|QPalette::PlaceholderText\|setForegroundRole" -- src/ | sed "s|^|  |"
    echo
    echo "--- 2. Das Muster, dem eine Themefarb-Funktion folgen wuerde ---"
    echo "  capture::contrastEffectOf() — Deklaration capturewindow.h:36-45,"
    echo "  Definition capturewindow.cpp:114-142 (29 Zeilen). Sie liest"
    echo "  metadata.desktop ueber QStandardPaths::locate + KConfigGroup; die"
    echo "  colors-Datei liegt daneben und ist dieselbe Dateiart."
    echo
    echo "--- 3. Zeilenumfaenge ---"
    wc -l src/capture/capturewindow.h src/capture/capturewindow.cpp \
        tests/capturetest.cpp tests/captureshots.cpp tests/desktopthemes.h | sed "s|^|  |"
    echo
    echo "--- 4. Pruefsaetze, die heute ueber Textfarben urteilen ---"
    grep -n "void CaptureTest::textsFollowAColourSchemeChange\|void CaptureTest::noteTextUsesTheWindowTextRole\|void CaptureTest::footerHasMoreAirThanTheApplicationName" tests/capturetest.cpp | sed "s|^|  |"
    echo
    echo "--- 5. Bringen die mitgelieferten Pruef-Themes eine colors-Datei mit? ---"
    for t in tests/themes/plasma/desktoptheme/*/; do
        n="$(basename "$t")"
        if [ -f "$t/colors" ]; then echo "  $n: colors JA"; else echo "  $n: colors nein"; fi
        echo "    Dateien: $(ls "$t" | tr '\n' ' ')"
    done
    echo
    echo "--- 6. Die Wahl von anyInstalledTheme() haengt an der Sortierung ---"
    echo "  QDir::entryList(..., QDir::Name) ohne QDir::IgnoreCase."
    printf "  ASCII-Sortierung, erstes Theme:              "
    ls /usr/share/plasma/desktoptheme/ | LC_ALL=C sort | head -1
    printf "  Sortierung ohne Gross-/Kleinunterscheidung:  "
    ls /usr/share/plasma/desktoptheme/ | sort -f | head -1
    echo "  Das eine bringt keine colors-Datei mit, das andere bringt eine mit."
    echo
    echo "--- 7. Aus welcher Datei liest KSvg seine Farben? ---"
    printf "  NEEDED KF6ColorScheme: "
    objdump -p /usr/lib/libKF6Svg.so | grep -c "NEEDED.*ColorScheme"
    printf "  Treffer auf \"/colors\": "
    strings -a /usr/lib/libKF6Svg.so | grep -c "^/colors$"
    echo "  KSvg::Svg::color(StyleSheetColor) ist oeffentlich (svg.h:629)."
    echo "  GRENZE: was color(Text) je Theme liefert, ist NICHT gemessen."
} > "$ZIEL/b2-dateimenge.txt" 2>&1

# b3 — Schnitt Sprint 8: Fundstellen von #76 in den Dateien von #61.
{
    echo "=== Sprint-8-Schnitt — Stand $STAND, $ZEIT ==="
    echo
    echo "--- clang-tidy auf src/main.cpp (Dateimenge #61 gegen #76) ---"
    clang-tidy -p build/lint --quiet src/main.cpp 2>&1 \
        | grep -E "warning:|error:" | sed "s|$WURZEL/||" | sed "s|^|  |"
    echo
    echo "--- Pruefsatzgrenzen in tests/capturetest.cpp ---"
    grep -n "^void CaptureTest::" tests/capturetest.cpp | sed "s|^|  |"
    echo
    echo "--- SPEC-Abschnitte: #85 (3.1) gegen #61 (2.3, 2.4, 15) ---"
    grep -n "^## \|^### 2.3\|^### 2.4\|^### 3.1\|^### 3.2" SPEC.md | sed "s|^|  |"
} > "$ZIEL/b3-schnitt-abstand.txt" 2>&1

# b4 — Linterbefunde in den drei Capture-Dateien nach #83.
{
    echo "=== clang-tidy auf den drei Capture-Dateien — Stand $STAND, $ZEIT ==="
    echo "Datenbank: build/lint/compile_commands.json (Stand 04.08.2026, also VOR #83)"
    echo
    for f in src/capture/capturewindow.cpp tests/capturetest.cpp tests/captureshots.cpp; do
        echo "--- $f ---"
        clang-tidy -p build/lint --quiet "$f" 2>&1 \
            | grep -E "warning:|error:" | sed "s|$WURZEL/||" | sed "s|^|  |"
        echo
    done
} > "$ZIEL/b4-linterbefunde-nach-83.txt" 2>&1

# b5 — Existenzprobe der in #85 genannten Pruefmittel (DoR-Zusatz 04.08.2026).
{
    echo "=== Existenzprobe der Pruefmittel — Stand $STAND, $ZEIT ==="
    echo
    for f in \
        docs/scrum/vorberichte/83-native-huelle/sonden/weichzeichnerbeleg.cpp \
        docs/scrum/vorberichte/83-native-huelle/ux-beratung/messungen/m1-deckung-je-theme.txt \
        docs/scrum/vorberichte/83-native-huelle/messung-b-themefarbe.txt \
        docs/scrum/vorberichte/83-native-huelle/po-themeschrift.py \
        docs/scrum/vorberichte/83-native-huelle/po-themeschrift.txt \
        docs/scrum/reviews/2026-08-04-abnahme-befunde/sonden/echtelage.cpp; do
        if git ls-files --error-unmatch "$f" > /dev/null 2>&1; then
            echo "  JA   $f"
        else
            echo "  NEIN $f"
        fi
    done
    echo
    echo "--- Zeilenangabe von AK 4 nachgeprueft ---"
    grep -n "QLabel \*subtleLabel\|setForegroundRole(QPalette::PlaceholderText)" src/capture/capturewindow.cpp | sed "s|^|  |"
    echo "  AK 4 nennt capturewindow.cpp:132 — dort steht heute:"
    sed -n "132p" src/capture/capturewindow.cpp | sed "s|^|    |"
} > "$ZIEL/b5-pruefmittel-existenz.txt" 2>&1

echo "Fertig. Ausgaben in $ZIEL"
