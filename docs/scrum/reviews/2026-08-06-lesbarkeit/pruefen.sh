#!/usr/bin/env bash
# Fährt die Messungen zur Gestaltungsfrage vom 06.08.2026 (#100/#101) in einem
# Zug und schreibt sie nach messungen/.
#
# Was das Skript nicht anfasst: `/usr` (es installiert nichts), keinen
# Produktivcode und keine Einstellung des Kunden. Die Sonden setzen ihre
# Farbschemata selbst und laufen in einem eigenen XDG-Sandkasten.
#
# Alles läuft offscreen. Das genügt hier, weil keine der Zahlen über Hülle,
# Rundung, Kontur, Schatten oder Dekoration etwas behauptet (B21) — gemessen
# werden Farbrollen, Flächen und Textsatz. Die eine Aussage, die den
# Compositor braucht, kommt aus einem bereits versionierten Sitzungsbild:
# M5 liest KRunner aus dem UI-Review zu Sprint 7 aus.
#
# Aufruf: bash docs/scrum/reviews/2026-08-06-lesbarkeit/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
PROJEKTBAU="$WURZEL/build"
BAU="$HIER/build"

mkdir -p "$HIER/messungen" "$HIER/bilder"

export QT_QPA_PLATFORMTHEME=kde
export QT_QPA_PLATFORM=offscreen

SCHEMATA=(/usr/share/color-schemes/*.colors)
THEMES="default breeze-dark breeze-light CachyOS-Nord-round Iridescent-round cachyos-emerald cachyos-emerald-color cachyos-emerald-light"

echo "== Projekt bauen =="
cmake -B "$PROJEKTBAU" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$PROJEKTBAU" -j "$(nproc)" > /dev/null

echo "== Sonden übersetzen =="
# Frisch bauen, bevor gemessen wird: eine veraltete Sonde schreibt plausible
# Zahlen eines alten Standes mit frischem Zeitstempel.
cmake -B "$BAU" -S "$HIER/sonden" -DCMAKE_BUILD_TYPE=Release \
    -DDENKZETTEL_BUILD="$PROJEKTBAU" > /dev/null
cmake --build "$BAU" -j "$(nproc)" > /dev/null

echo "== M1: Worauf steht die Liste, und was trüge eine Trennung? =="
{
    echo "M1 — Auf welchem Grund steht die Bibliotheksliste, und was trüge eine Trennung?"
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. Offscreen, QT_QPA_PLATFORMTHEME=kde."
    echo "Gemessen am gebauten LibraryWindow mit gefüllter Liste (showLibrary), 18 Schemata."
    echo
    "$BAU/listengrund" breeze - "${SCHEMATA[@]}"
    echo
    echo "############ Gegenprobe mit dem Stil fusion — hängt der Grund am Stil?"
    echo
    "$BAU/listengrund" fusion - "${SCHEMATA[@]}"
} > "$HIER/messungen/m1-listengrund.txt" 2>&1

echo "== M2: Was zeichnet alternatingRowColors wirklich? =="
{
    echo "M2 — Was zeichnet alternatingRowColors in dieser Liste wirklich?"
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. Offscreen, QT_QPA_PLATFORMTHEME=kde, Stil breeze."
    echo "Der Schalter ist nur in der Sonde gesetzt; im Produktivcode kommt er nicht vor."
    echo "Je Schema zwei Bestände: einmal mit ungerader, einmal mit gerader Gruppengröße."
    echo
    "$BAU/streifenprobe" "$HIER/bilder" "${SCHEMATA[@]}"
} > "$HIER/messungen/m2-streifen.txt" 2>&1

echo "== M3: Welche Kante bekäme das Textfeld? =="
{
    echo "M3 — Welche Kante bekäme das Textfeld des Erfassungsfensters?"
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. Offscreen, QT_QPA_PLATFORMTHEME=kde."
    echo "Geprüft wird der Weg, den Plasmas eigenes TextField geht:"
    echo "widgets/lineedit, Vorsatz base (TextField.qml:187-191)."
    echo
    # shellcheck disable=SC2086
    "$BAU/feldkante" "$HIER/bilder" $THEMES
} > "$HIER/messungen/m3-feldkante.txt" 2>&1

echo "== M4: Zahlen für das Mockup =="
{
    echo "M4 — Zahlen für das Mockup: Schriften, Schemafarben, Trennmittel."
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. QT_QPA_PLATFORMTHEME=kde, Stil breeze."
    echo
    "$BAU/farbtafel" "${SCHEMATA[@]}"
} > "$HIER/messungen/m4-farbtafel.txt" 2>&1
python3 "$HIER/sonden/zusammenzug.py" "$HIER/messungen/m4-farbtafel.txt" >> "$HIER/messungen/m4-farbtafel.txt"

echo "== M5: KRunner als Maßstab, aus dem versionierten Sitzungsbild =="
python3 "$HIER/sonden/krunnerbild.py" "$WURZEL" > "$HIER/messungen/m5-krunner-sitzungsbild.txt"

echo
echo "Fertig. Protokolle in messungen/, Bilder in bilder/, Mockup: mockup-trennung.html"
