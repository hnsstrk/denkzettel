#!/bin/bash
# Fährt die drei Sonden der Messung B von vorn (Vorprüfung #101, 07.08.2026).
#
# Läuft ohne /usr, ohne Produktivcode und ohne Kundeneinstellung: Die Sonden
# kennen weder NoteListModel noch NoteListDelegate, sie messen das Verhalten
# von QListView, KConfig und QPainter.
#
# Aufruf aus diesem Ordner:  bash bauen-b.sh
set -euo pipefail

hier="$(cd "$(dirname "$0")" && pwd)"
bau="$hier/build-b"
mkdir -p "$bau" "$hier/messungen"

g++ -std=c++20 -fPIC -w "$hier/sonden/nachbarmalung.cpp" -o "$bau/nachbarmalung" \
    $(pkg-config --cflags --libs Qt6Widgets Qt6Test)
QT_QPA_PLATFORM=offscreen "$bau/nachbarmalung" > "$hier/messungen/mb1-nachbarmalung.txt"

g++ -std=c++20 -fPIC -w "$hier/sonden/framecontrast.cpp" -o "$bau/framecontrast" \
    -I/usr/include/KF6/KConfigCore -I/usr/include/KF6/KConfig \
    $(pkg-config --cflags --libs Qt6Core) -lKF6ConfigCore
"$bau/framecontrast" > "$hier/messungen/mb2-framecontrast.txt"

g++ -std=c++20 -fPIC -w "$hier/sonden/haarlinie-skalierung.cpp" -o "$bau/haarlinie" \
    $(pkg-config --cflags --libs Qt6Gui)
QT_QPA_PLATFORM=offscreen "$bau/haarlinie" > "$hier/messungen/mb3-haarlinie-skalierung.txt"

# Die Nachprüfung der Befunde aus Messung A. Sie läuft zweimal: einmal unter
# dem Plattformthema des Kunden und einmal ohne — die beiden Rollbalkenbreiten
# sind der Grund, warum die beiden Messungen verschiedene Zahlen tragen.
g++ -std=c++20 -fPIC -w "$hier/sonden/nachpruefung.cpp" -o "$bau/nachpruefung" \
    -I/usr/include/KF6/KColorScheme -I/usr/include/KF6/KConfigCore -I/usr/include/KF6/KConfig \
    $(pkg-config --cflags --libs Qt6Widgets Qt6Test) -lKF6ColorScheme -lKF6ConfigCore -lKF6ConfigGui
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde "$bau/nachpruefung" \
    > "$hier/messungen/mb5-nachpruefung-kde.txt"
QT_QPA_PLATFORM=offscreen "$bau/nachpruefung" 2>/dev/null \
    | sed -n '/Prüffrage 3/,/Prüffrage 4/p' > "$hier/messungen/mb6-breite-ohne-plattformthema.txt"

echo "Ausgaben in $hier/messungen/"
