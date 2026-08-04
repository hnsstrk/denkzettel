#!/usr/bin/env bash
# Wiederholt die Messungen der Vorprüfung zu #76 (Bearbeiter A, 04.08.2026).
#
# Der Bauplatz liegt bewusst NEBEN diesem Skript und nicht in build/ der
# Repositoriumswurzel — dort arbeiten möglicherweise andere Stränge.
# Es wird nichts am Projekt geändert: die Fix-Proben laufen auf einer
# Wegwerfkopie unter $KOPIEN, die das Skript vorher anlegt.
#
# Aufruf:  bash docs/scrum/vorberichte/76-linterbefunde/pruefen.sh
set -o pipefail

HIER="$(cd "$(dirname "$0")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"
MESS="$HIER/messungen"
KOPIEN="${TMPDIR:-/tmp}/denkzettel-76-proben"
mkdir -p "$MESS" "$KOPIEN"

echo "== Stand =="
git -C "$WURZEL" log --oneline -1
pacman -Q clang clazy cmake qt6-base

echo
echo "== A — Ist-Stand am Arbeitsbaum =="
cmake -B "$BAU" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > "$MESS/00-configure.txt" 2>&1
cmake --build "$BAU" -j "$(nproc)" > "$MESS/01-bau.txt" 2>&1
echo "Bau rc=$?  Compilerwarnungen=$(grep -c 'warning:' "$MESS/01-bau.txt")"

cmake --build "$BAU" --target lint-clazy > "$MESS/02-clazy.txt" 2>&1
echo "lint-clazy rc=$?  warnings=$(grep -c 'warning:' "$MESS/02-clazy.txt")  errors=$(grep -c 'error:' "$MESS/02-clazy.txt")"

cmake --build "$BAU" --target lint-tidy > "$MESS/03-tidy.txt" 2>&1
echo "lint-tidy  rc=$?  warnings=$(grep -c 'warning:' "$MESS/03-tidy.txt")  errors=$(grep -c 'error:' "$MESS/03-tidy.txt")"
grep 'warning:' "$MESS/03-tidy.txt" | sed "s|^$WURZEL/||" | sort -u > "$MESS/04-tidy-eindeutig.txt"
echo "  eindeutig: $(wc -l < "$MESS/04-tidy-eindeutig.txt")"
grep -o '\[[a-z0-9-]*\]$' "$MESS/04-tidy-eindeutig.txt" | sort | uniq -c | sort -rn

echo
echo "== B — Wie weit trägt die maschinelle Heilung? (Wegwerfkopie) =="
K="$KOPIEN/fixprobe"; rm -rf "$K"; mkdir -p "$K"
git -C "$WURZEL" archive HEAD | tar -x -C "$K"
cd "$K" || exit 1
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug > /dev/null 2>&1
cmake --build build -j "$(nproc)" > bau0.log 2>&1
DATEIEN=$(ls src/*.cpp src/*/*.cpp tests/*.cpp | grep -v spellfixspike | sed "s|^|$K/|")
for runde in 1 2 3; do
    run-clang-tidy -p "$K/build/lint" -quiet -fix $DATEIEN > "fix$runde.log" 2>&1
    cmake --build build -j "$(nproc)" > "bau$runde.log" 2>&1; brc=$?
    cmake --build build --target lint-tidy > "tidy$runde.log" 2>&1
    echo "Runde $runde: Bau rc=$brc  Baufehler=$(grep -c 'error:' "bau$runde.log")" \
         " Compilerwarnungen=$(grep -c 'warning:' "bau$runde.log")" \
         " Linter-Rohzeilen=$(grep -c 'warning:' "tidy$runde.log")"
done
ctest --test-dir build > testend.log 2>&1; echo "ctest nach den Fixes rc=$?"
grep 'warning:' tidy3.log | sed "s|^$K/||" | sort -u > "$MESS/10-rest-nach-maschine.txt"
echo "Rest, den keine Maschine heilt: $(wc -l < "$MESS/10-rest-nach-maschine.txt") eindeutige Befunde"

echo
echo "== C — Kann ein Lauf 0 melden, ohne etwas geprüft zu haben? =="
N="$KOPIEN/nullprobe"; rm -rf "$N"; mkdir -p "$N"
git -C "$WURZEL" archive HEAD | tar -x -C "$N"
cd "$N" || exit 1
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug > /dev/null 2>&1
cmake --build build -j "$(nproc)" > bau.log 2>&1
cmake --build build --target lint-clazy > gut.log 2>&1
echo "gesund : lint-clazy rc=$?  warnings=$(grep -c 'warning:' gut.log)  errors=$(grep -c 'error:' gut.log)"
find build \( -name 'librarytest.moc' -o -name 'shelltest.moc' \) -delete
cmake --build build -j "$(nproc)" > bau2.log 2>&1
cmake --build build --target lint-clazy > kaputt.log 2>&1
echo "kaputt : lint-clazy rc=$?  warnings=$(grep -c 'warning:' kaputt.log)  errors=$(grep -c 'error:' kaputt.log)"
cmake --build build --target lint-tidy > kaputt-tidy.log 2>&1
echo "kaputt : lint-tidy  rc=$?  warnings=$(grep -c 'warning:' kaputt-tidy.log)  errors=$(grep -c 'error:' kaputt-tidy.log)"

echo
echo "== D — Was meldet der Rückgabewert von setGlobalShortcut()? =="
cd "$HIER/sonden" || exit 1
g++ -std=c++20 -fPIC $(pkg-config --cflags Qt6Gui Qt6DBus) \
    -I/usr/include/KF6/KGlobalAccel -I/usr/include/KF6 \
    rueckgabeprobe.cpp -o "$KOPIEN/rueckgabeprobe" \
    $(pkg-config --libs Qt6Gui Qt6DBus) -lKF6GlobalAccel > /dev/null 2>&1 \
    && "$KOPIEN/rueckgabeprobe" | tee "$MESS/15-rueckgabeprobe.txt"
