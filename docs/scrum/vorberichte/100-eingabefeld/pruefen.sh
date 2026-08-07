#!/usr/bin/env bash
# Fährt die Messungen der Vorprüfung zu #100 (Bearbeiter A) in einem Zug und
# schreibt sie nach messungen/.
#
# Was das Skript nicht anfasst: `/usr` (es installiert nichts), keinen
# Produktivcode und keine Einstellung des Kunden. Es baut in einen eigenen
# Bauplatz unterhalb dieses Ordners, nicht in `build/` im Projektstamm — dort
# arbeiten weitere Stränge.
#
# Alles läuft offscreen. Das genügt hier, weil keine der Zahlen über Hülle,
# Rundung, Kontur, Schatten oder Dekoration etwas behauptet (B21): gemessen
# werden Auflösung, Farbrollen, Ränder und Geometrie. Die Aussagen der Story,
# die den Compositor brauchen, sind in Feld 4 des Berichts als solche benannt.
#
# Aufruf: bash docs/scrum/vorberichte/100-eingabefeld/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"

mkdir -p "$HIER/messungen"

export QT_QPA_PLATFORMTHEME=kde
export QT_QPA_PLATFORM=offscreen

THEMES="default breeze-dark breeze-light CachyOS-Nord-round Iridescent-round cachyos-emerald cachyos-emerald-color cachyos-emerald-light"

echo "== Projekt in den eigenen Bauplatz bauen =="
cmake -B "$BAU" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$BAU" -j "$(nproc)" > /dev/null

echo "== Sonden übersetzen =="
# Frisch bauen, bevor gemessen wird: eine veraltete Sonde schreibt plausible
# Zahlen eines alten Standes mit frischem Zeitstempel.
cmake -B "$BAU/sonden" -S "$HIER/sonden" -DCMAKE_BUILD_TYPE=Release \
    -DDENKZETTEL_BUILD="$BAU" > /dev/null
cmake --build "$BAU/sonden" -j "$(nproc)" > /dev/null

echo "== A1: Löst widgets/lineedit auf, und was gibt die Ansichtsfarbe her? =="
{
    echo "A1 — Auflösung, Ränder und Farbquellen der Feldgrafik je Theme."
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. Offscreen, QT_QPA_PLATFORMTHEME=kde."
    echo
    # shellcheck disable=SC2086
    "$BAU/sonden/feldprobe" "$WURZEL/tests/themes" $THEMES
} > "$HIER/messungen/a1-feldprobe.txt" 2>&1

echo "== A2: Umfärbung, Farbsatz, Skalierung, Innenrand =="
{
    echo "A2 — Was die Feldgrafik zeichnet, und welcher Weg den Text nach innen rückt."
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. Offscreen, QT_QPA_PLATFORMTHEME=kde."
    echo
    # shellcheck disable=SC2086
    "$BAU/sonden/feldbild" "$WURZEL/tests/themes" \
        denkzettel-test-schmal denkzettel-test-breit denkzettel-pruef-eckig $THEMES
} > "$HIER/messungen/a2-feldbild.txt" 2>&1

echo
echo "Fertig. Protokolle in messungen/, Bericht: messung-a.md"
