#!/bin/bash
# Vorprüfung #72 — alle Sonden dieses Berichts, wiederholbar.
# Bauplatz liegt in diesem Ordner, nicht in build/ der Repositoriumswurzel.
set -u
cd "$(dirname "$0")" || exit 1

cmake -B build -S sonden -DCMAKE_BUILD_TYPE=Debug >/dev/null || exit 1
cmake --build build >/dev/null || exit 1

QT_QPA_PLATFORM=offscreen ./build/tooltipsonde | tee messungen/sonde1-tooltipquellen-offscreen.txt
QT_QPA_PLATFORM=offscreen ./build/tooltipsonde2 | tee messungen/sonde2-helfertext-offscreen.txt
LANG=C LC_ALL=C QT_QPA_PLATFORM=offscreen ./build/tooltipsonde2 | head -6 \
    | tee messungen/sonde3-locale-C.txt
(cd build && QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde ./tooltipsonde3) \
    | tee messungen/sonde5-tooltipbild-offscreen.txt

# Sonde 4 misst am gebauten Stand des Projekts, nicht an einer eigenen Binärdatei.
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde ../../../../build/bin/librarytest \
    undoesTheDeletionByKeyboard deletesWithTheDeleteKey opensTheEditorWithF2 2>&1 \
    | grep -E "^(PASS|FAIL|Totals)" | tee messungen/sonde4-kuerzel-registriert.txt
