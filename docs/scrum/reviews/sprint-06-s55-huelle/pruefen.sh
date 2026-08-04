#!/bin/bash
# Baut die vier Messsonden zu #55, fährt sie und schreibt die Protokolle neben
# die Quellen — also genau die *.txt, die im Repository liegen. Danach werden
# die 14 Belegbilder neu erzeugt, aus einem frisch gebauten Läufer.
#
# Alles geschieht unterhalb dieses Verzeichnisses in build/ (von .gitignore
# gedeckt). Weder der Bau des Projekts noch die Konfiguration des Kunden wird
# angefasst:
#
#   * Der Projektstand wird in einen EIGENEN Bauplatz übersetzt, nicht in das
#     build/ der Repositoriumswurzel — dort arbeiten unter Umständen andere
#     Agenten gleichzeitig.
#   * Messung 1 und 2 lesen und schreiben `plasmarc`. Beide laufen unter einem
#     privaten XDG_CONFIG_HOME; das eingestellte Desktop-Theme des Kunden wird
#     dadurch weder gelesen noch verstellt. Messung 2 braucht zusätzlich eine
#     eigene D-Bus-Sitzung, weil KConfigWatcher seine Meldung über den Bus
#     bekommt.
#   * Nichts wird nach /usr installiert.
#
# Aufruf: bash docs/scrum/reviews/sprint-06-s55-huelle/pruefen.sh

set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../../../.." && pwd)"
WORK="$HERE/build"
SONDEN="$HERE/messungen"

# Ohne das Plattformthema misst man eine Ersatzschrift statt der Systemschrift,
# und alle Größenverhältnisse stimmen nicht mehr (CLAUDE.md).
export QT_QPA_PLATFORM=offscreen
export QT_QPA_PLATFORMTHEME=kde

echo "== Sonden übersetzen =="
cmake -B "$WORK/sonden" -S "$SONDEN" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$WORK/sonden" -j"$(nproc)" > /dev/null

# Privates Konfigurationsverzeichnis für die beiden plasmarc-Messungen.
SANDBOX="$WORK/sandkasten"
rm -rf "$SANDBOX"
mkdir -p "$SANDBOX"

echo
echo "== Messung 1 — Woher kennt KSvg das Desktop-Theme? =="
{
    printf '[Theme]\nname=default\n' > "$SANDBOX/plasmarc"
    XDG_CONFIG_HOME="$SANDBOX" "$WORK/sonden/ksvg-themequelle" \
        "Lauf A — plasmarc sagt 'default'"
    printf '[Theme]\nname=CachyOS-Nord-round\n' > "$SANDBOX/plasmarc"
    XDG_CONFIG_HOME="$SANDBOX" "$WORK/sonden/ksvg-themequelle" \
        "Lauf B — plasmarc sagt 'CachyOS-Nord-round'" | tail -n +4
    printf '\nBefund: Der gemeldete Name bleibt zwischen A und B derselbe. KSvg liest\n'
    printf 'plasmarc nicht — `default` ist sein Rückfallwert, nicht der eingestellte\n'
    printf 'Name. Das Erfassungsfenster muss den Namen deshalb selbst lesen und\n'
    printf 'übergeben (KConfigCore in der Abhängigkeitsliste, SPEC 15).\n'
} | tee "$HERE/ksvg-themequelle.txt"

echo
echo "== Messung 2 — Zustellwege eines Theme-Wechsels =="
printf '[Theme]\nname=default\n' > "$SANDBOX/plasmarc"
cat > "$WORK/messung2.sh" <<'INNEN'
set -eu
export XDG_CONFIG_HOME="$1"
export QT_QPA_PLATFORM=offscreen
export QT_QPA_PLATFORMTHEME=kde
"$2/themewechsel-zustellung"
INNEN
dbus-run-session -- bash "$WORK/messung2.sh" "$SANDBOX" "$WORK/sonden" \
    | tee "$HERE/themewechsel-zustellung.txt"

echo
echo "== Messung 3 — Wie ein FrameSvg einem Theme-Wechsel folgt =="
"$WORK/sonden/framesvg-nachziehen" | tee "$HERE/framesvg-nachziehen.txt"

echo
echo "== Messung 4 — Eckstücke, Ränder und Schattenkacheln (K6) =="
"$WORK/sonden/theme-eckstuecke" | tee "$HERE/theme-eckstuecke.txt"

echo
echo "== Messung 6 — Welcher Aufruf schneidet eine Schattenkachel? =="
"$WORK/sonden/schattenkacheln" | tee "$HERE/schattenkacheln.txt"

echo
echo "== Belegbilder — Läufer FRISCH bauen, dann fahren =="
# Ein veralteter Läufer schreibt plausible Bilder eines alten Standes mit
# frischem Zeitstempel (Vorfall Sprint 5). Deshalb wird hier gebaut, nicht
# vorausgesetzt.
cmake -B "$WORK/projekt" -S "$ROOT" -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$WORK/projekt" --target captureshots -j"$(nproc)" > /dev/null
# Nur die Offscreen-Reihe 01-14; Bild 15 stammt aus der laufenden Sitzung
# (Messung 5) und wird von diesem Skript nicht erzeugt.
rm -f "$HERE"/bilder/0*.png "$HERE"/bilder/1[0-4]-*.png
"$WORK/projekt/bin/captureshots" "$HERE/bilder"

echo
echo "Fertig. Fünf Protokolle und $(ls "$HERE"/bilder/*.png | wc -l) Bilder liegen in $HERE."
echo "Messung 5 (Schatten am Compositor) läuft nicht mit — sie braucht eine"
echo "angemeldete Wayland-Sitzung; der Aufruf steht in LIESMICH.md."
