#!/bin/bash
# Alle Sonden der Vorprüfung zu Issue #61, wiederholbar.
#
#   bash docs/scrum/vorberichte/61-versionsanzeige/pruefen.sh
#
# Baut zwei Bauplätze *innerhalb dieses Ordners* (beide von .gitignore gedeckt)
# und schreibt die Ausgaben nach messungen/. `build/` der Repositoriumswurzel
# wird nicht angefasst, und es wird nichts nach /usr installiert.
set -eu
HIER="$(cd "$(dirname "$0")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"

echo "== Bauplatz 1: denkzetteld aus dem Projektstand =="
cmake -B "$HIER/build" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF > /dev/null
cmake --build "$HIER/build" --target denkzetteld -j "$(nproc)" > /dev/null

echo "== Bauplatz 2: die Sonden =="
cmake -B "$HIER/sonden/build" -S "$HIER/sonden" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$HIER/sonden/build" -j "$(nproc)" > /dev/null

echo "== Sonde 1: was tut denkzetteld heute mit --version =="
bash "$HIER/messungen/sonde1-optionen.sh" > "$HIER/messungen/sonde1-optionen.txt" 2>&1

echo "== Sonde 2: --version bei bereits laufendem Dienst =="
bash "$HIER/messungen/sonde2-zweitstart.sh" > "$HIER/messungen/sonde2-zweitstart.txt" 2>&1

echo "== Sonde 3: die drei Wege zu --version =="
bash "$HIER/messungen/sonde3-versionswege.sh" > "$HIER/messungen/sonde3-versionswege.txt" 2>&1

echo "== Sonde 4: Busname und Desktop-Dateiname unter KAboutData =="
bash "$HIER/messungen/sonde4-busname.sh" > "$HIER/messungen/sonde4-busname.txt" 2>&1

echo "== Sonde 5: --version ohne Sitzungsbus (Lage des automatischen Laufs) =="
cd "$HIER"
{
  echo "== Ohne Sitzungsbus (wie im automatischen Testlauf) =="
  for M in kf6 kf6lax; do
    echo "--- DZ_MODUS=$M --version ---"
    DZ_MODUS=$M QT_QPA_PLATFORM=offscreen timeout 5 \
      env -u DBUS_SESSION_BUS_ADDRESS -u XDG_RUNTIME_DIR ./sonden/build/versionswege --version 2>&1 | tail -3
    echo "Rückgabe: $?"
  done
} > "$HIER/messungen/sonde5-ohne-bus.txt" 2>&1

echo
echo "Fertig. Ausgaben in $HIER/messungen/"
