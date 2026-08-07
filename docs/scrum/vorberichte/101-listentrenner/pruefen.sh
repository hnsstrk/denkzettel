#!/usr/bin/env bash
# Fährt die Messungen der Vorprüfung zu Issue #101 (Bearbeiter A) in einem Zug
# und schreibt sie nach messungen/.
#
# Was das Skript nicht anfasst: `/usr` (es installiert nichts), keinen
# Produktivcode, keine SPEC und nicht den Bauplatz `build/` im
# Projektwurzelverzeichnis — dort arbeiten die Umsetzungsstränge. Gebaut wird
# ausschließlich unter diesem Ordner.
#
# Alles läuft offscreen. Das genügt, weil keine Zahl über Hülle, Rundung,
# Kontur, Schatten oder Dekoration etwas behauptet (B21) — gemessen werden
# Zeilengeometrie, Neuzeichnen und Farbrollen.
#
# Aufruf: bash docs/scrum/vorberichte/101-listentrenner/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
PROJEKTBAU="$HIER/build/projekt"
SONDENBAU="$HIER/build/sonden"

mkdir -p "$HIER/messungen"

export QT_QPA_PLATFORMTHEME=kde
export QT_QPA_PLATFORM=offscreen

echo "== Projekt in den eigenen Bauplatz bauen =="
cmake -B "$PROJEKTBAU" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$PROJEKTBAU" -j "$(nproc)" > /dev/null

echo "== Sonden übersetzen =="
# Frisch bauen, bevor gemessen wird: eine veraltete Sonde schreibt plausible
# Zahlen eines alten Standes mit frischem Zeitstempel.
cmake -B "$SONDENBAU" -S "$HIER/sonden" -DCMAKE_BUILD_TYPE=Release \
    -DDENKZETTEL_BUILD="$PROJEKTBAU" > /dev/null
cmake --build "$SONDENBAU" -j "$(nproc)" > /dev/null

kopf() {
    echo "$1"
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. Offscreen, QT_QPA_PLATFORMTHEME=kde."
    echo
}

echo "== M1: Was der Delegate sieht, wie die Zeilen liegen, wer neu gezeichnet wird =="
{
    kopf "M1 — Zeilenkanten, Nachbarauswahl und Neuzeichnen (ohne QT_SCALE_FACTOR)"
    "$SONDENBAU/kanten" -
} > "$HIER/messungen/m1-kanten.txt"

echo "== M2: dasselbe unter der Skalierung des Kunden =="
{
    kopf "M2 — dieselbe Sonde mit QT_SCALE_FACTOR=1.6 (Einstellung des Kunden)"
    QT_SCALE_FACTOR=1.6 "$SONDENBAU/kanten" -
} > "$HIER/messungen/m2-kanten-skaliert.txt"

echo "== M3: Linienfarbe über alle installierten Schemata =="
{
    kopf "M3 — frameContrast und die daraus gemischte Linienfarbe, 18 Schemata"
    "$SONDENBAU/farbe" /usr/share/color-schemes/*.colors
} > "$HIER/messungen/m3-farbe.txt"

echo "== M4: Woher kommt frameContrast wirklich? =="
{
    kopf "M4 — Gruppe, Schlüssel und Herkunft des Wertes frameContrast"
    "$SONDENBAU/kontrastwert" /usr/share/color-schemes/*.colors
} > "$HIER/messungen/m4-kontrastwert.txt"

echo "Fertig. Messungen unter $HIER/messungen/"
