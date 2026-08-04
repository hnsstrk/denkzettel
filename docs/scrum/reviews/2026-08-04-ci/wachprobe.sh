#!/bin/bash
# Mutationsprobe der beiden Wachen in .github/workflows/ci.yml.
#
# Eine Wache, die nur im Gutfall geprüft wurde, ist nicht geprüft: Deckt der
# Filter allein den grünen Weg ab, sieht ein Absturz aus wie ein ruhiger Tag.
# Diese Probe erzeugt beide Fehlerfälle künstlich und verlangt Rot.
#
# Wiederholbar: bash docs/scrum/reviews/2026-08-04-ci/wachprobe.sh
# Rückgabe 0 = alle vier Proben wie erwartet.

set -u
fehler=0

pruefe() {          # pruefe <Name> <Erwartet> <Ist>
    if [ "$2" = "$3" ]; then
        echo "  OK    $1 — Rückgabe $3 (erwartet $2)"
    else
        echo "  FEHLT $1 — Rückgabe $3, erwartet $2"
        fehler=1
    fi
}

# --- Wache 1: Warnungszähler ------------------------------------------------
# Wortlaut aus dem Schritt „Warnungen zählen", unverändert übernommen.
warnungswache() {
    local log="$1"
    anzahl=$(grep -c 'warning:' "$log" || true)
    if [ "${anzahl}" -ne 0 ]; then
        return 1
    fi
    return 0
}

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

printf '[ 50%%] Building CXX object src/foo.cpp.o\n[100%%] Built target denkzetteld\n' > "$tmp/sauber.log"
printf '[ 50%%] Building CXX object src/foo.cpp.o\n/src/foo.cpp:12:5: warning: unused variable [-Wunused-variable]\n' > "$tmp/warnung.log"

echo "Wache 1 — Warnungszähler"
warnungswache "$tmp/sauber.log";  pruefe "sauberer Bau läuft durch"   0 $?
warnungswache "$tmp/warnung.log"; pruefe "eine Warnung färbt rot"     1 $?

# --- Wache 2: pipefail beim Bauen -------------------------------------------
# Der Bau wird durch `tee` geleitet. Ohne pipefail liefert die Pipe den
# Rückgabewert von tee — immer 0 —, und ein Baufehler liefe grün durch.
echo "Wache 2 — pipefail"

( set +o pipefail; false 2>&1 | tee "$tmp/ohne.log" >/dev/null )
pruefe "OHNE pipefail bliebe ein Baufehler unsichtbar" 0 $?

( set -o pipefail; false 2>&1 | tee "$tmp/mit.log" >/dev/null )
pruefe "MIT pipefail schlägt der Baufehler durch"      1 $?

echo
if [ "$fehler" -eq 0 ]; then
    echo "Ergebnis: beide Wachen schlagen im Fehlerfall an."
else
    echo "Ergebnis: mindestens eine Wache bleibt im Fehlerfall stumm."
fi
exit "$fehler"
