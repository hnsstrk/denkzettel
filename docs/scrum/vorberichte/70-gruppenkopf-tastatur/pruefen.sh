#!/bin/bash
# Vorprüfung #70 — alle Messungen dieses Berichts, wiederholbar.
#
# Der Bauplatz liegt in diesem Ordner. `build/` der Wurzel gehört einem
# anderen Strang, `build-vor70/` dieser Vorprüfung. Nach /usr wird nichts
# installiert, und src/ wird nicht angefasst: die Kandidatenfassung entsteht
# aus flicken/a-70.patch in einer Kopie unter kandidat/.
set -u
cd "$(dirname "$0")" || exit 1
WURZEL=../../../..

# --- Kandidatenfassung herstellen -------------------------------------------
# Der Dateiname bleibt librarywindow.cpp und der Header liegt daneben: sonst
# findet AUTOMOC den Q_OBJECT-Header nicht und das Linken bricht ab.
mkdir -p kandidat
cp "$WURZEL/src/ui/librarywindow.h" kandidat/
cp "$WURZEL/src/ui/librarywindow.cpp" kandidat/librarywindow.cpp
patch -s kandidat/librarywindow.cpp < flicken/a-70.patch || {
    echo "Der Flicken passt nicht mehr auf src/ui/librarywindow.cpp —"
    echo "der Stand hat sich geändert, die Messung gilt für den alten (B17)."
    exit 1
}

# --- Bauen ------------------------------------------------------------------
cmake -B build -S sonden -DCMAKE_BUILD_TYPE=Debug >/dev/null || exit 1
cmake --build build >/dev/null || exit 1

# Der Bildläufer wird frisch gebaut, sonst schreibt er Bilder eines alten
# Standes mit frischem Zeitstempel (Regel aus CLAUDE.md).
cmake -B "$WURZEL/build-vor70" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug >/dev/null || exit 1
cmake --build "$WURZEL/build-vor70" --target libraryshots librarytest >/dev/null || exit 1

mkdir -p messungen

# --- Sonde 1: der heutige Stand ---------------------------------------------
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde ./build/kopfsonde \
    | tee messungen/sonde1-heutiger-stand-offscreen.txt

# --- Sonde 2: Mischprobe #70 gegen die beiden Lesarten von #71 ---------------
{
    echo "=== Mischprobe: #70 gegen die beiden Lesarten von #71 ==="
    echo "Basis: src/ui/librarywindow.cpp @ $(git -C "$WURZEL" rev-parse --short HEAD)"
    B=$(mktemp -d)
    cp "$WURZEL/src/ui/librarywindow.cpp" "$B/basis.cpp"
    for v in a-70 b-71-lesart2 b-71-lesart1; do
        cp "$B/basis.cpp" "$B/$v.cpp"
        patch -s "$B/$v.cpp" < "flicken/$v.patch"
    done
    echo
    echo "--- Lage der Änderungsstellen ---"
    for v in a-70 b-71-lesart2 b-71-lesart1; do
        printf '%-16s ' "$v"
        diff -u "$B/basis.cpp" "$B/$v.cpp" | grep '^@@'
    done
    echo
    echo "--- #70 gegen #71-Lesart 2 (Merker am scrollTo) ---"
    git merge-file -p "$B/a-70.cpp" "$B/basis.cpp" "$B/b-71-lesart2.cpp" > "$B/m2.cpp"
    echo "Rückgabewert: $?  (0 = sauber, >0 = Zahl der Konflikte)"
    echo
    echo "--- #70 gegen #71-Lesart 1 (aufgeschobene Bewegung) ---"
    git merge-file -p "$B/a-70.cpp" "$B/basis.cpp" "$B/b-71-lesart1.cpp" > "$B/m1.cpp"
    echo "Rückgabewert: $?  (0 = sauber, >0 = Zahl der Konflikte)"
    rm -rf "$B"
} | tee messungen/sonde2-mischprobe.txt

# --- Sonde 3: Kandidatenfassung gegen die volle Testauflage ------------------
{
    echo "=== Sonde 3 · Kandidatenfassung (#70) gegen die volle Testauflage ==="
    echo "Kandidat: kandidat/librarywindow.cpp — src/ unangetastet"
    echo
    echo "--- A · librarytest gegen den HEUTIGEN Stand ---"
    QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde \
        "$WURZEL/build-vor70/bin/librarytest" 2>&1 | grep -E "^Totals|^FAIL"
    echo
    echo "--- B · dieselbe Testauflage gegen die KANDIDATENFASSUNG ---"
    QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde \
        ./build/librarytest70 2>&1 | grep -E "^Totals|^FAIL"
    echo
    echo "--- C · die Sondenfälle gegen die Kandidatenfassung ---"
    QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde ./build/kopfsonde70
} | tee messungen/sonde3-kandidat-gegen-testauflage.txt

# --- Sonde 4: was die SPEC heute sagt ---------------------------------------
{
    echo "=== Sonde 4 · Was die SPEC heute über die Rollregel sagt ==="
    echo
    echo "--- Die Rollregel in SPEC 9 ---"
    grep -n "Springt die Auswahl" "$WURZEL/SPEC.md"
    echo
    echo "--- Steht die Passbedingung aus librarywindow.cpp:788 in der SPEC? ---"
    grep -c "passen zusammen\|Passbedingung\|zusammen ins Bild" "$WURZEL/SPEC.md"
    echo "(0 = nein)"
} | tee messungen/sonde4-spec-stand.txt

# --- Bildbeleg: Szene 7 des Bildläufers, auf der Skalierung des Kunden -------
mkdir -p messungen/libraryshots-1.6
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.6 \
    "$WURZEL/build-vor70/bin/libraryshots" messungen/libraryshots-1.6 >/dev/null
echo "Bild 07 geschrieben: messungen/libraryshots-1.6/07-fall4-uebergang-beim-scrollen.png"
