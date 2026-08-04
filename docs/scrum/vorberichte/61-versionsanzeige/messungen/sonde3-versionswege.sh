#!/bin/bash
# Sonde 3 — Die drei Wege zu `--version` im Vergleich, gemessen an einer Sonde,
# die die ersten Zeilen von src/main.cpp wörtlich übernimmt.
set -u
HIER="$(cd "$(dirname "$0")" && pwd)"
BIN="$HIER/../sonden/build/versionswege"
export QT_QPA_PLATFORM=offscreen

echo "Sonde: $BIN"
echo "Stand: $(cd "$HIER/../../../../.." && git rev-parse --short HEAD)"
echo "PROJECT_VERSION der Wurzel: $(grep -m1 '^project(' "$HIER/../../../../../CMakeLists.txt")"
echo

for MODUS in roh kf6 kf6lax; do
  for ARG in "--version" "-v" "--help" "--unbekannt" ""; do
    echo "===== DZ_MODUS=$MODUS  Argument: ${ARG:-<keins>} ====="
    OUT=$(DZ_MODUS="$MODUS" timeout 5 "$BIN" $ARG 2>&1)
    echo "Rückgabe: $?"
    echo "$OUT"
    echo
  done
done

echo "===== Qt-Option auf der Kommandozeile (nicht über die Umgebung) ====="
for MODUS in kf6 kf6lax; do
  echo "--- DZ_MODUS=$MODUS  ./versionswege -platform offscreen --version ---"
  OUT=$(DZ_MODUS="$MODUS" timeout 5 env -u QT_QPA_PLATFORM "$BIN" -platform offscreen --version 2>&1)
  echo "Rückgabe: $?"
  echo "$OUT"
  echo
done
