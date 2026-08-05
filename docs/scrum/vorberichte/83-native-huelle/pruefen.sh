#!/usr/bin/env bash
# Fährt alle Messungen der Vorprüfung zu Issue #83 (Bearbeiter A).
#
# Das Skript baut in einen **eigenen** Bauplatz (`build/` neben dieser Datei,
# von .gitignore gedeckt). Es fasst weder `build/` der Repositoriumswurzel an
# — dort arbeiten unter Umständen andere Stränge — noch irgendetwas unter
# `/usr`, und es ändert keine Einstellung des Kunden.
#
# **Drei Läufe zeigen ein Fenster auf dem Bildschirm** (Sonde 2 unter Wayland,
# Sonde 4 in allen Betriebsarten). Sonde 4 legt dabei ein **Vollbild-Schachbrett**
# über den Schreibtisch und nimmt danach den Bildschirm auf — die Aufnahme
# enthält deshalb nur Inhalt dieser Sonde. Der Vollbildpuffer wird sofort
# gelöscht; gespeichert wird allein ein Ausschnitt um das Prüffenster.
#
# Ohne Wayland-Sitzung werden die Sitzungsläufe übersprungen und das gesagt.
#
# Aufruf: bash docs/scrum/vorberichte/83-native-huelle/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BAU="$HIER/build"
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

mkdir -p "$HIER/messungen" "$HIER/bilder"

echo "== Sonden übersetzen (eigener Bauplatz) =="
cmake -B "$BAU/sonden" -S "$HIER/sonden" > /dev/null
cmake --build "$BAU/sonden" -j "$(nproc)" > /dev/null

export XDG_CACHE_HOME="$FLUECHTIG/cache"
export QT_QPA_PLATFORMTHEME=kde

echo "== Sonde 1: FrameSvg — Bildpunktverhältnis, Maske, Ränder =="
QT_QPA_PLATFORM=offscreen "$BAU/sonden/rahmenmasse" default \
    > "$HIER/messungen/sonde1-rahmenmasse-offscreen.txt"
if [ -n "${WAYLAND_DISPLAY:-}" ]; then
    "$BAU/sonden/rahmenmasse" default \
        > "$HIER/messungen/sonde1-rahmenmasse-wayland.txt"
    echo "   Unterschied offscreen gegen Wayland (erwartet: nur Plattform und qApp-DPR):"
    diff "$HIER/messungen/sonde1-rahmenmasse-offscreen.txt" \
         "$HIER/messungen/sonde1-rahmenmasse-wayland.txt" || true
else
    echo "   (Wayland-Lauf übersprungen — keine angemeldete Sitzung)"
fi

echo "== Sonde 2: das Fenster — Plattformgleichheit, Bindung, Weichzeichner =="
# Der letzte Abschnitt der Sonde ruft mit Absicht enableBlurBehind(nullptr).
# Unter Wayland stürzt das ab (Rückgabe 139) — das **ist** die Messung, deshalb
# darf der Rückgabewert das Skript hier nicht anhalten.
for f in 1 1.6; do
    QT_SCALE_FACTOR=$f QT_QPA_PLATFORM=offscreen \
        "$BAU/sonden/fensterlauf" "$HIER/bilder" default \
        > "$HIER/messungen/sonde2-fensterlauf-offscreen-skala-$f.txt" 2>&1 || true
    if [ -n "${WAYLAND_DISPLAY:-}" ]; then
        QT_SCALE_FACTOR=$f "$BAU/sonden/fensterlauf" "$HIER/bilder" default \
            > "$HIER/messungen/sonde2-fensterlauf-wayland-skala-$f.txt" 2>&1 || true
    fi
done

echo "== Sonde 3: wer meldet Weichzeichner und Kontrast an (Binärmessung) =="
{
    echo "=== Vorpruefung #83, Sonde 3: Was melden Plasmas eigene Ueberlagerungen an? ==="
    echo "Gemessen an den installierten Binaerdateien (Muster: b2-dekorationsquelle.txt)."
    echo
    for f in /usr/lib/libPlasmaQuick.so.7 /usr/lib/libPlasma.so.7; do
        [ -e "$f" ] || continue
        echo "-- $f"
        echo "   Aufrufe von KWindowEffects (undefinierte Symbole): $(nm -D -u "$f" | grep -c KWindowEffects)"
        nm -D -u "$f" | grep KWindowEffects | sed 's/^/     /'
    done
    echo
    echo "-- Wayland-Plugin von KWindowSystem"
    for f in /usr/lib/qt6/plugins/kf6/kwindowsystem/*.so; do
        echo "   $f"
        strings -a "$f" | grep -iE "blur|contrast|background_effect" | sort -u | sed 's/^/     /'
    done
} > "$HIER/messungen/sonde3-plasma-anmeldungen.txt"

echo "== Sonde 5: Stand der Werkzeuge und Wayland-Globale (B17) =="
{
    echo "=== Vorpruefung #83, Sonde 5: Stand der Werkzeuge und Wayland-Globale ==="
    echo "Eine Aussage gilt fuer einen Stand (B17) — deshalb steht er hier."
    echo
    pacman -Q kwindowsystem kwin plasma-desktop qt6-base ksvg
    echo
    echo "-- KWin-Effekt blur geladen?"
    busctl --user call org.kde.KWin /Effects org.kde.kwin.Effects isEffectLoaded s blur 2>&1 || true
    echo
    echo "-- Wayland-Globale zu Weichzeichner, Kontrast und Schatten"
    wayland-info 2>/dev/null | grep -E "shadow|blur|contrast|background_effect|slide" | sed 's/^ *//' || true
} > "$HIER/messungen/sonde5-stand-und-globale.txt"

echo "== Sonde 1 über die Prüf-Themes: decken sie anders als die installierten? =="
WURZEL="$(cd "$HIER/../../../.." && pwd)"
{
  for t in denkzettel-test-schmal denkzettel-test-breit denkzettel-pruef-eckig default; do
    case "$t" in
      denkzettel-pruef-eckig) D="$WURZEL/tests/themes" ;;   # überführt am 05.08.2026, #83 AK 9
      denkzettel-test-*)      D="$WURZEL/tests/themes" ;;
      *)                      D="/usr/share" ;;
    esac
    echo "===== $t"
    XDG_DATA_DIRS="$D:/usr/share" QT_QPA_PLATFORM=offscreen \
      "$BAU/sonden/rahmenmasse" "$t" 2>/dev/null \
      | grep -E "getMargins|oberste Zeile|Alphalauf|Mitte  " | head -8
  done
} > "$HIER/messungen/sonde1-pruefthemes.txt"
grep -E "^=====|Randwert" "$HIER/messungen/sonde1-pruefthemes.txt"

if [ -z "${WAYLAND_DISPLAY:-}" ]; then
    echo "== Sonde 4 übersprungen — sie hat ohne angemeldete Sitzung keinen Gegenstand =="
    exit 0
fi

echo "== Sonde 4: A/B-Beleg der Weichzeichner-Anmeldung (zeigt ein Vollbild) =="
for m in aus an zweimal spaet-leer frueh wiederzeigen-neu wiederzeigen-alt; do
    "$BAU/sonden/weichzeichnerbeleg" "$HIER/bilder" "$m" default \
        > "$HIER/messungen/sonde4-weichzeichner-$m.txt" 2>&1 || true
    printf '   %-18s ' "$m"
    grep -m1 "Spannweite" "$HIER/messungen/sonde4-weichzeichner-$m.txt" || echo "(keine Prüflinie)"
done

echo
echo "Erwartung: nur die Läufe mit **früher** Anmeldung (frueh, wiederzeigen-*)"
echo "zeigen eine kleine Spannweite (222 bis 223 = weichgezeichnet). Alle anderen"
echo "zeigen 203 bis 242 — das ungeglättete Schachbrett."
