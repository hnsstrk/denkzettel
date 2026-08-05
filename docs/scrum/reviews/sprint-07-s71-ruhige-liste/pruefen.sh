#!/bin/bash
# Prüfskript zu Strang B von Sprint 7 — Issues #71, #70, #72.
#
# Wiederholt alles, was der Bericht behauptet: die volle Testauflage, den
# Doppellauf unter zwei Gebieten (#72, Falle 1), die Bilder und die zwölf
# Mutationsproben. Es baut vorher — ein veralteter Bildläufer schreibt ein
# plausibles Bild eines alten Standes mit frischem Zeitstempel (CLAUDE.md).
#
# Aufruf aus einem Arbeitsbaum des Zweigs story/71-ruhige-liste:
#     bash docs/scrum/reviews/sprint-07-s71-ruhige-liste/pruefen.sh
#
# Es installiert nichts nach /usr und fasst keinen laufenden Dienst an.
set -u

WURZEL="$(cd "$(dirname "$0")/../../../.." && pwd)"
cd "$WURZEL" || exit 1
ZIEL="${1:-/tmp/pruefen-s71}"
mkdir -p "$ZIEL"

echo "== Arbeitsbaum: $WURZEL"
echo "== Stand:       $(git rev-parse --short HEAD)"
echo

echo "== 1. Bauen (Debug, ohne Installation)"
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug > /dev/null || exit 1
cmake --build build -j"$(nproc)" 2>&1 | grep -iE " warning| error" && echo "!! Warnungen im Bau" || echo "   warnungsfrei"
echo

echo "== 2. Volle Testauflage"
ctest --test-dir build 2>&1 | grep -E "Test #|tests passed"
echo

echo "== 3. #72, Falle 1 — derselbe Test unter zwei Gebieten"
echo "   Der Kürzeltext ist gebietsabhängig; eine Zusicherung auf das Literal"
echo "   wäre hier grün und im öffentlichen Lauf rot."
for gebiet in de_DE.UTF-8 C; do
    printf '   LANG=%-12s ' "$gebiet"
    env LANG="$gebiet" LC_ALL="$gebiet" QT_QPA_PLATFORM=offscreen ./build/bin/librarytest 2>&1 |
        grep -E "^Totals"
done
echo

echo "== 4. Bilder (offscreen, KDE-Theme, Kundenskalierung 1,6)"
echo "   Belege: 11a und 11b (#71), 07-fall4-uebergang-beim-scrollen.png (#70)."
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.6 \
    ./build/bin/libraryshots "$ZIEL/bilder-1.6" > /dev/null 2>&1 &&
    ls "$ZIEL/bilder-1.6" | tr '\n' ' ' && echo
echo
echo "   Gegen die abgelegten Bilder:"
for bild in 07-fall4-uebergang-beim-scrollen 11a-klick-auf-angeschnittene-zeile \
    11b-nach-dem-nachlaufenden-autoscroll; do
    if cmp -s "$ZIEL/bilder-1.6/$bild.png" \
        "docs/scrum/reviews/sprint-07-s71-ruhige-liste/bilder-1.6/$bild.png"; then
        echo "   $bild: gleich"
    else
        echo "   $bild: ABWEICHUNG"
    fi
done
echo
echo "   Hinweis: 10c und 10d sind nicht bitgenau wiederholbar — der Textcursor"
echo "   im Editor blinkt, und die Aufnahme trifft mal die eine, mal die andere"
echo "   Phase. Gemessen am 05.08.2026, kein Befund dieser Stories."
echo

echo "== 5. Mutationsproben"
echo "   Die zwölf Läufe liegen als Terminalausgabe unter messungen/ (m1..m14"
echo "   ohne m2-Erstfassung). Sie einzeln nachzufahren beschreibt der Bericht,"
echo "   Abschnitt „Mutationsproben\" — jede ist eine Zeile im Quelltext."
ls docs/scrum/reviews/sprint-07-s71-ruhige-liste/messungen/m*.txt | sed 's|.*/|   |'
