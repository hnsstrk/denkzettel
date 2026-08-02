#!/bin/bash
# Prüfmittel (b) zu Issue #60: Was bekommt der Host wirklich?
#
# Der Strukturtest prüft die QAction-Objekte im eigenen Prozess. Ob daraus über
# das Tray-Protokoll auch beim Panel ankommt, was dort ankommen soll, sagt nur
# der Host. Diese Abfrage holt das exportierte Menü mit
# com.canonical.dbusmenu.GetLayout am laufenden Dienst ab.
#
# Aufruf:  bash menue-abfrage.sh [ausgabedatei]
# Erwartet einen laufenden denkzetteld in der Sitzung.

set -u
ZIEL=${1:-/dev/stdout}

# KStatusNotifierItem setzt die Id aus Anwendungsname und der im Konstruktor
# übergebenen Kennung zusammen; unsere heißt deshalb "denkzettel_denkzettel".
DIENST=""
for KANDIDAT in $(qdbus6 org.kde.StatusNotifierWatcher /StatusNotifierWatcher \
                  RegisteredStatusNotifierItems | cut -d/ -f1); do
    ID=$(qdbus6 "$KANDIDAT" /StatusNotifierItem org.freedesktop.DBus.Properties.Get \
         org.kde.StatusNotifierItem Id 2>/dev/null)
    if [ "$ID" = "denkzettel_denkzettel" ]; then
        DIENST=$KANDIDAT
        break
    fi
done

if [ -z "$DIENST" ]; then
    echo "Kein Tray-Symbol von Denkzettel angemeldet — läuft der Dienst?" >&2
    exit 1
fi

# qdbus6 gibt Objektpfade leer aus, deshalb hier gdbus.
MENUEPFAD=$(gdbus call --session --dest "$DIENST" --object-path /StatusNotifierItem \
            --method org.freedesktop.DBus.Properties.Get org.kde.StatusNotifierItem Menu \
            | sed "s/.*objectpath '\([^']*\)'.*/\1/")

{
    echo "Abfrage $(date '+%Y-%m-%d %H:%M')"
    echo "Dienst: $DIENST · Menüpfad: $MENUEPFAD"
    echo -n "ItemIsMenu: "
    qdbus6 "$DIENST" /StatusNotifierItem org.freedesktop.DBus.Properties.Get \
        org.kde.StatusNotifierItem ItemIsMenu
    echo
    echo "=== com.canonical.dbusmenu.GetLayout(0, -1, [label, icon-name, enabled, type, shortcut]) ==="
    # Das -- muss sein: ohne es liest gdbus die Rekursionstiefe -1 als Schalter.
    gdbus call --session --dest "$DIENST" --object-path "$MENUEPFAD" \
        --method com.canonical.dbusmenu.GetLayout \
        -- 0 -1 "['label','icon-name','enabled','type','shortcut']" \
        | sed 's/, \[</,\n  [</; s/)>, <(/)>,\n  <(/g'
} >"$ZIEL"
