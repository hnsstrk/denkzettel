#!/bin/bash
# Nachprüfung des Meta+N-Wegs nach der Umbenennung (Issue #60, Beschluss B5).
#
# Die Umbenennung fasst den Anzeigenamen der Kürzel-Aktion und den Namen der
# Desktop-Aktion an; die Aktions-Id `show-capture` bleibt, denn an ihr hängt die
# Registrierung. Geprüft wird deshalb die ganze Kette, nicht nur der Name:
#
#   1. Kennt kglobalacceld unsere Komponente, hält sie Meta+N — und **wie oft**?
#      Der Kürzel-Hinweis im Menü ist ein QAction-Kürzel und darf die
#      Registrierung nicht doppeln; hier steht, wie viele es wirklich sind.
#   2. Erklärt die Desktop-Datei die Aktion, die der Tastendruck startet, und
#      steht dort der neue Name?
#   3. Trägt der Weg des Tastendrucks? invokeShortcut ist genau das, was
#      kglobalacceld beim Tastendruck selbst tut — ohne dass jemand eine Taste
#      drücken muss.
#
# Aufruf: bash kuerzel-nachpruefung.sh [ausgabedatei]
# Erwartet einen laufenden denkzetteld.

set -u
ZIEL=${1:-/dev/stdout}
KOMPONENTE=org.denkzettel.Denkzettel.desktop
PFAD=/component/org_denkzettel_Denkzettel_desktop
AKTION=show-capture

{
    echo "Nachprüfung $(date '+%Y-%m-%d %H:%M')"
    echo "laufender Dienst: $(pgrep -a denkzetteld)"
    echo

    echo "=== 1. Was kglobalacceld hält ==="
    echo "friendlyName der Komponente: $(qdbus6 org.kde.kglobalaccel "$PFAD" \
        org.freedesktop.DBus.Properties.Get org.kde.kglobalaccel.Component friendlyName)"
    echo "aktiv: $(qdbus6 org.kde.kglobalaccel "$PFAD" org.kde.kglobalaccel.Component.isActive)"
    echo "Aktionen:"
    qdbus6 org.kde.kglobalaccel "$PFAD" org.kde.kglobalaccel.Component.shortcutNames | sed 's/^/  /'
    echo "allShortcutInfos (Id · Anzeigename · Komponente · … · Tasten · Vorgabe):"
    gdbus call --session --dest org.kde.kglobalaccel --object-path "$PFAD" \
        --method org.kde.kglobalaccel.Component.allShortcutInfos \
        | sed "s/), (/),\n  (/g" | sed 's/^/  /'
    echo "  0x1000004E = 268435534 = Meta+N"
    echo -n "Einträge für $AKTION (1 erwartet, mehr hieße gedoppelt): "
    qdbus6 org.kde.kglobalaccel "$PFAD" org.kde.kglobalaccel.Component.shortcutNames \
        | grep -c "^$AKTION$"
    echo

    echo "=== 2. Was die Desktop-Datei erklärt ==="
    DATEI=$(find "$HOME/.local/share/applications" /usr/share/applications \
            -name "$KOMPONENTE" 2>/dev/null | head -1)
    echo "gefunden: ${DATEI:-keine}"
    [ -n "${DATEI:-}" ] && grep -E "^Actions=|^\[Desktop Action|^Name=|^Exec=|^Keywords=" "$DATEI" | sed 's/^/  /'
    echo

    echo "=== 3. Auslösen wie beim Tastendruck ==="
    DB="$HOME/.local/share/denkzettel/denkzettel.db"
    echo "Notizen vorher: $(sqlite3 "$DB" 'select count(*) from notes' 2>/dev/null || echo '?')"
    qdbus6 org.kde.kglobalaccel "$PFAD" org.kde.kglobalaccel.Component.invokeShortcut "$AKTION"
    sleep 2
    echo "ausgelöst — ob das Erfassungsfenster steht, zeigt das Bild daneben."
    echo "Notizen nachher: $(sqlite3 "$DB" 'select count(*) from notes' 2>/dev/null || echo '?')"
} >"$ZIEL"

cat "$ZIEL"
