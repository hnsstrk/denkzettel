#!/usr/bin/env bash
# Fährt die Belege zu Issue #101 — „Trenner in der Bibliotheksliste" — in einem
# Zug und schreibt die Protokolle nach messungen/, die Bilder nach bilder/.
#
# Bauart nach dem Muster von sprint-07-s83-native-huelle/:
#   * eigener Bauplatz (`build/` neben dieser Datei, von .gitignore gedeckt).
#     Weder `build/` der Repositoriumswurzel noch irgendetwas unter `/usr` wird
#     angefasst — dort arbeiten unter Umständen andere Stränge, und den Takt
#     der Installation setzt der Product Owner.
#   * `QT_QPA_PLATFORMTHEME=kde` überall, sonst misst man eine Ersatzschrift.
#   * die Bildläufer werden vor jedem Bild **gebaut**: ein veralteter Läufer
#     schreibt ein plausibles Bild eines alten Standes mit frischem Zeitstempel.
#
# Alles läuft offscreen. Kein Kriterium dieser Story spricht über Hülle,
# Rundung, Kontur, Schatten, Dekoration oder Durchsichtigkeit, so dass B21 hier
# kein Sitzungsbild verlangt. Die Bildpunkt-Prüfsätze laufen auf Skalierung 1
# (AK 1 bis AK 3); das Bild aus AK 7 läuft auf der Skalierung des Kunden und
# ist ein Ansichtsbeleg ohne Bildpunktzusicherung.
#
# Aufruf: bash docs/scrum/reviews/sprint-09-s101-listentrenner/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"

mkdir -p "$HIER/messungen" "$HIER/bilder/skalierung-1" "$HIER/bilder/skalierung-1-6"

export QT_QPA_PLATFORMTHEME=kde
export QT_QPA_PLATFORM=offscreen

kopf() {
    echo "$1"
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. $(cd "$WURZEL" && git rev-parse --short HEAD) auf $(cd "$WURZEL" && git rev-parse --abbrev-ref HEAD)."
    echo "Offscreen, QT_QPA_PLATFORMTHEME=kde."
    echo
}

echo "== Denkzettel übersetzen (eigener Bauplatz, von Grund auf) =="
# Der Bauplatz wird zuerst geräumt. Ohne das übersetzt ein zweiter Lauf nichts,
# meldet „0 Warnungen" und sieht aus wie ein sauberer Neubau — dieselbe Null,
# die ein Lauf über null Dateien meldet (Sprint 8, #76). Beim ersten Durchgang
# am 07.08.2026 stand hier genau das: 0 übersetzte Dateien, 0 Warnungen.
rm -rf "$BAU"

# Das Bauprotokoll geht ins Flüchtige, nur seine Zahlen werden abgelegt: ein
# vollständiges Protokoll ist hundert Kilobyte, in denen der Nachweis untergeht.
BAUPROTOKOLL="$(mktemp)"
trap 'rm -f "$BAUPROTOKOLL"' EXIT
cmake -B "$BAU" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > "$BAUPROTOKOLL" 2>&1
cmake --build "$BAU" -j "$(nproc)" >> "$BAUPROTOKOLL" 2>&1

echo "== B1: der Bau, warnungsfrei (DoD) =="
{
    kopf "B1 — vollständiger Neubau, Zahl der Compilerwarnungen"
    echo "übersetzte Dateien   : $(grep -c 'Building CXX object' "$BAUPROTOKOLL" || true)"
    echo "Compilerwarnungen    : $(grep -ci 'warning' "$BAUPROTOKOLL" || true)"
    echo "Fehler               : $(grep -ci 'error' "$BAUPROTOKOLL" || true)"
    echo
    echo "Die neue Abhängigkeit, wie CMake sie meldet:"
    grep -i 'colorscheme' "$BAUPROTOKOLL" || echo "  (keine Zeile — die Komponente meldet sich still)"
} > "$HIER/messungen/b1-bau.txt"

echo "== B2: der gesamte Prüfsatz (AK 1 bis AK 6, AK 5 als Bestandsprüfung) =="
{
    kopf "B2 — ctest über alle Prüfprogramme"
    ctest --test-dir "$BAU" 2>&1
} > "$HIER/messungen/b2-ctest.txt"

echo "== B3: die neuen Prüfsätze einzeln =="
{
    kopf "B3 — die Prüffunktionen dieser Story, mit Namen und Ergebnis"
    # Nur die Ergebniszeilen: die Warnungen von KI18n stehen mit demselben
    # Funktionsnamen davor, und ein Filter ohne Zeilenanfang zieht sie mit —
    # 178 KB Datei, in der der Nachweis untergeht.
    "$BAU/bin/librarytest" 2>/dev/null \
        | grep -E '^(PASS|FAIL|Totals)' \
        | grep -E 'Hairline|RankingDoesNotAskForIt|GroupLineOverAHead|UpperNeighbours|SeparatorOutOfGround|SearchResultsLikeTheLibrary|MeasuresOfTheGroupedList|OneThicknessUnderTheCustomers|bringsTheHead|^Totals' \
        || true
} > "$HIER/messungen/b3-pruefsaetze.txt"

echo "== B4: beide Linter auf Schwelle null =="
{
    kopf "B4 — clang-tidy und clazy, je mit der Zahl der angefassten Dateien"
    for ziel in lint-tidy lint-clazy; do
        protokoll="$(mktemp)"
        set +e
        cmake --build "$BAU" --target "$ziel" > "$protokoll" 2>&1
        rc=$?
        set -e
        echo "$ziel: Rückgabewert $rc"
        echo "  angefasste Dateien : $(grep -cE 'clang-tidy -p=|Processing file' "$protokoll" || true)"
        echo "  Warnungen          : $(grep -c 'warning:' "$protokoll" || true)"
        echo "  Fehler             : $(grep -c 'error:' "$protokoll" || true)"
        rm -f "$protokoll"
    done
    echo
    echo "Die Zahl der angefassten Dateien steht hier, weil ein Werkzeuglauf über NULL"
    echo "Dateien dieselbe Null meldet wie ein sauberer Lauf (Sprint 8, #76)."
} > "$HIER/messungen/b4-linter.txt"

echo "== B5: Bilder, Skalierung 1 — der Normalfall und die Trefferliste =="
cmake --build "$BAU" -j "$(nproc)" --target libraryshots searchshots > /dev/null
"$BAU/bin/libraryshots" "$HIER/bilder/skalierung-1" > /dev/null
"$BAU/bin/searchshots" "$HIER/bilder/skalierung-1" > /dev/null

echo "== B6: das Bild aus AK 7 — Skalierung des Kunden =="
# 1,6 ist die Einstellung des Kunden (bestätigt 07.08.2026). Bei ihr belegt eine
# Linie von einem logischen Punkt 1,6 Gerätebildpunktzeilen, und die eingerückte
# Kante liegt bei 19 statt bei 12 — deshalb ist dieses Bild ein Ansichtsbeleg
# und trägt keine Bildpunktzusicherung.
#
# Abgelegt werden zwei der fünfzehn Szenen. Der Normalfall, weil AK 7 ihn
# verlangt — und die ruhige Liste, weil in ihr als einziger beide Linien
# zugleich stehen: im Normalfall ist die zweite Notiz die ausgewählte, und
# damit entfällt die einzige eingerückte Linie, die er hätte. Die übrigen
# dreizehn sagen unter 1,6 nichts, was ihre Geschwister unter 1 nicht sagen.
SECHZEHNTEL="$(mktemp -d)"
trap 'rm -f "$BAUPROTOKOLL"; rm -rf "$SECHZEHNTEL"' EXIT
QT_SCALE_FACTOR=1.6 "$BAU/bin/libraryshots" "$SECHZEHNTEL" > /dev/null
cp "$SECHZEHNTEL/01-normalfall.png" \
   "$SECHZEHNTEL/09-ruhiges-bild-innerhalb-der-gruppe.png" \
   "$HIER/bilder/skalierung-1-6/"

{
    kopf "B5/B6 — Bildbelege, Maße und Skalierung"
    for ordner in skalierung-1 skalierung-1-6; do
        datei="$HIER/bilder/$ordner/01-normalfall.png"
        echo "$ordner/01-normalfall.png: $(identify -format '%wx%h' "$datei" 2>/dev/null || echo 'Maß nicht ermittelbar, ImageMagick fehlt')"
    done
    echo "Trefferliste (AK 6): skalierung-1/2-trefferliste-mit-gruppen.png"
} > "$HIER/messungen/b5-bilder.txt"

echo "== B7: Strichstärke unter der Skalierung des Kunden (UI-Review L9) =="
# Die Zusicherung des Kunden lautet, dass die Rangfolge aus der LÄNGE des
# Strichs kommt und nicht aus seiner Stärke. Unter 1,6 belegt eine Linie von
# einem logischen Punkt 1,6 Gerätebildpunktzeilen — vor der Ausrichtung am
# Geräteraster wurden daraus mal eine, mal zwei. Hier wird jede Linie im Bild
# gezählt, damit die Zusicherung eine Zahl bekommt.
{
    kopf "B7 — Stärke jeder Trennlinie in GERÄTEBILDPUNKTEN, Skalierung 1,6"
    python3 "$HIER/strichstaerke.py" "$HIER/bilder/skalierung-1-6" \
        || echo "(nicht ermittelbar — python3 mit Pillow fehlt)"
} > "$HIER/messungen/b7-strichstaerke.txt"

echo "Fertig. Messungen unter $HIER/messungen/, Bilder unter $HIER/bilder/"
