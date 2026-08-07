#!/usr/bin/env bash
# Erzeugt sämtliche Bilder und Messberichte des UI-Reviews zu Sprint 9.
#
# Zwei Läufe, aus zwei Gründen getrennt:
#
#   offscreen — QT_SCALE_FACTOR setzt die Skalierung, einmal 1 (dort gelten die
#               Bildpunktzusicherungen AK 1 bis AK 3 von #101) und einmal 1,6
#               (die Einstellung des Kunden).
#   Sitzung   — QT_SCALE_FACTOR bleibt ungesetzt. Unter Wayland multipliziert
#               die Variable mit der Sitzungsskalierung; die 1,6 des Kunden ist
#               dort vorzufinden, nicht einzustellen.
#
# QT_QPA_PLATFORMTHEME=kde ist in beiden Läufen Pflicht — ohne sie springt Qt
# auf eine Ersatzschrift, und ein Bild, dessen Größenverhältnisse falsch sind,
# ist schlechter als keines.
#
# Aufruf: bilder-erzeugen.sh <Bauverzeichnis> <Sondenverzeichnis>

set -euo pipefail

BUILD="${1:?Bauverzeichnis des Projekts (enthält lib/)}"
SONDE="${2:?Verzeichnis mit der übersetzten uxsonde}"
HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BILDER="$HIER/bilder"

export QT_QPA_PLATFORMTHEME=kde

echo "== offscreen, Skalierung 1 =="
export QT_QPA_PLATFORM=offscreen
QT_SCALE_FACTOR=1 "$SONDE/uxsonde" bibliothek "$BILDER/offscreen-1" s101-normalfall auswahl normal
QT_SCALE_FACTOR=1 "$SONDE/uxsonde" bibliothek "$BILDER/offscreen-1" s101-rhythmus keine rhythmus
QT_SCALE_FACTOR=1 "$SONDE/uxsonde" bibliothek "$BILDER/offscreen-1" s101-rhythmus-auswahl auswahl rhythmus
QT_SCALE_FACTOR=1 "$SONDE/uxsonde" bibliothek "$BILDER/offscreen-1" s101-rhythmus-hover hover rhythmus
QT_SCALE_FACTOR=1 "$SONDE/uxsonde" bibliothek "$BILDER/offscreen-1" s101-suche-Werkstatt "suche:Werkstatt" rhythmus
QT_SCALE_FACTOR=1 "$SONDE/uxsonde" bibliothek "$BILDER/offscreen-1" s101-suche-für "suche:für" rhythmus

echo "== offscreen, Skalierung 1,6 (Einstellung des Kunden) =="
for lauf in "s101-normalfall auswahl normal" "s101-rhythmus keine rhythmus" \
            "s101-rhythmus-auswahl auswahl rhythmus" "s101-rhythmus-hover hover rhythmus"; do
    # shellcheck disable=SC2086
    QT_SCALE_FACTOR=1.6 "$SONDE/uxsonde" bibliothek "$BILDER/offscreen-1-6" $lauf
done
for theme in "" CachyOS-Nord-round breeze-light cachyos-emerald; do
    QT_SCALE_FACTOR=1.6 "$SONDE/uxsonde" erfassung "$BILDER/offscreen-1-6" "s100-feld-${theme:-default}" "$theme"
done
QT_SCALE_FACTOR=1.6 "$SONDE/uxsonde" erfassung "$BILDER/offscreen-1-6" s100-feld-leer-default ""

echo "== angemeldete Sitzung (B21) =="
# Die Sonde zeigt für vierzehn Sekunden ein Fenster auf einem eigenen
# Hintergrund; die Vollaufnahme wird an dessen Rahmen zugeschnitten und danach
# gelöscht, damit vom Schreibtisch des Kunden kein Bildpunkt in das öffentliche
# Repository gerät.
unset QT_QPA_PLATFORM
unset QT_SCALE_FACTOR
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

for was in erfassung bibliothek; do
    "$SONDE/uxsonde" "sitzung-$was" 14 &
    sleep 6
    spectacle -b -n -f -o "$FLUECHTIG/voll.png"
    case "$was" in
        erfassung) ziel="$BILDER/sitzung/s100-erfassung-sitzung.png" ;;
        bibliothek) ziel="$BILDER/sitzung/s101-bibliothek-sitzung.png" ;;
    esac
    mkdir -p "$BILDER/sitzung"
    python3 "$HIER/zuschnitt.py" "$FLUECHTIG/voll.png" "$ziel"
    rm -f "$FLUECHTIG/voll.png"
    wait
done

echo "Fertig. Bau: $BUILD"
