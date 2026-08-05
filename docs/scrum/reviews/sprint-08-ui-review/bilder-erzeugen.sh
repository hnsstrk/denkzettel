#!/bin/bash
# Eigene Bilder des UI-Reviews zu Sprint 8, Story #85.
#
# Fährt die Belegsonde des Strangs A
# (docs/scrum/reviews/sprint-08-s85-lesbarkeit/sonden/sitzungsbeleg.cpp) gegen
# einen **eigenen** Bau des Sprint-Standes, über acht Themes und zwei benannte
# Gründe. Je Lauf entstehen zwei Bilder: der Normalfall und der Leerzustand.
#
# Die Sonde wird von ihrem Ort gebaut und nicht hierher kopiert — zwei Fassungen
# derselben Sonde gehen auseinander, und dann misst man mit der falschen. Was
# dieser Review eigenständig beisteuert, ist der Bau, der Lauf und die Messung
# **im Bild** (messungen/schriftimbild.py) statt an der Palette.
#
# QT_SCALE_FACTOR wird ausdrücklich **nicht** gesetzt: Das Fensterverhältnis 1,6
# kommt in der angemeldeten Sitzung vom Compositor; ein zusätzlicher Faktor
# multiplizierte sich darauf. Jede Ausgabe schreibt das gemessene Verhältnis mit.
#
# Aufruf: bilder-erzeugen.sh <Bauplatz der Sonde> <Zielordner> <Protokoll>

set -u
SONDE="${1:?Bauplatz der Sonde}/sitzungsbeleg"
ZIEL="${2:?Zielordner der Bilder}"
LOG="${3:?Protokolldatei}"

export QT_QPA_PLATFORMTHEME=kde
unset QT_SCALE_FACTOR

: > "$LOG"
{
    echo "### UI-Review Sprint 8 — eigene Sitzungsbelege zu #85"
    echo "Datum: $(date -Is)"
    echo "Sitzung: $(loginctl show-session "$(loginctl show-user "$USER" -p Display --value)" -p Type --value)," \
         "gesperrt: $(loginctl show-session "$(loginctl show-user "$USER" -p Display --value)" -p LockedHint --value)"
    echo "Sonde: $SONDE"
    echo
} >> "$LOG"

lauf() {
    local theme="$1" grundname="$2" r="$3" g="$4" b="$5"
    {
        echo "----------------------------------------------------------------"
        echo "LAUF: $theme über $grundname"
    } >> "$LOG"
    "$SONDE" "$ZIEL" "$theme" "$grundname" "$r" "$g" "$b" >> "$LOG" 2>&1
    echo "Rückgabe: $?" >> "$LOG"
    echo >> "$LOG"
    sleep 1
}

for theme in default breeze-light breeze-dark CachyOS-Nord-round Iridescent-round \
             cachyos-emerald cachyos-emerald-color cachyos-emerald-light; do
    lauf "$theme" weiss 255 255 255
done

# Der dunkle Grund ist kein Beiwerk: Die Schrift kommt seit #85 aus dem Theme,
# und ein Theme mit heller Hülle reicht eine dunkle Schrift auch dorthin, wo der
# Grund dunkel ist.
for theme in cachyos-emerald-light cachyos-emerald-color breeze-light default; do
    lauf "$theme" schwarz 0 0 0
done

echo "Fertig." >> "$LOG"
