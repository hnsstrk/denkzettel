#!/usr/bin/env bash
# Mutationsproben zu Issue #101: Kann jeder neue Prüfsatz überhaupt fallen?
#
# Sechs Eingriffe in den Produktivcode, je einer nimmt genau eine Zusicherung
# heraus. Nach jedem Eingriff wird gebaut und `librarytest` gefahren; erwartet
# wird ein ROTER Lauf mit dem Prüfsatz, der die Zusicherung trägt. Bleibt er
# grün, prüft der Satz nichts — das ist der Befund, den diese Datei sucht.
#
# Der Eingriff wird nach jedem Lauf zurückgenommen; am Ende steht der
# unveränderte Stand. Das Skript fasst weder `/usr` noch einen anderen
# Arbeitsbaum an und baut ausschließlich im mitgegebenen Bauplatz.
#
# Aufruf: bash docs/scrum/reviews/sprint-09-s101-listentrenner/mutationsprobe.sh

set -uo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$WURZEL/build"
DELEGATE="$WURZEL/src/ui/notelistdelegate.cpp"
FENSTER="$WURZEL/src/ui/librarywindow.cpp"

ausgabe="$HIER/messungen/mutationsprobe.txt"
mkdir -p "$HIER/messungen"

{
    echo "Mutationsproben zu #101 — kann jeder Prüfsatz fallen?"
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. $(cd "$WURZEL" && git rev-parse --short HEAD) auf $(cd "$WURZEL" && git rev-parse --abbrev-ref HEAD)."
    echo
} > "$ausgabe"

probe() {
    local name="$1" datei="$2" alt="$3" neu="$4"

    # Mit -p, und beim Zurücknehmen wird die Datei angefasst: eine schlichte
    # Kopie bekommt die Uhrzeit des Kopierens, und die liegt VOR dem Eingriff.
    # Nach dem Zurücknehmen hielte make die Quelle für älter als das Objekt des
    # mutierten Baus und baute nicht neu — der nächste Lauf misst dann den
    # vorigen Eingriff mit. Genau so ist es beim ersten Durchgang am 07.08.2026
    # passiert: der Lauf nach der letzten Rücknahme war rot.
    cp -p "$datei" "$datei.orig"
    python3 - "$datei" "$alt" "$neu" <<'PY'
import sys
pfad, alt, neu = sys.argv[1], sys.argv[2], sys.argv[3]
text = open(pfad, encoding="utf-8").read()
if text.count(alt) != 1:
    sys.exit(f"Muster {alt!r} kommt {text.count(alt)}-mal vor, erwartet genau einmal")
open(pfad, "w", encoding="utf-8").write(text.replace(alt, neu))
PY
    if [ $? -ne 0 ]; then
        echo "== $name ==" >> "$ausgabe"
        echo "ABBRUCH: Eingriff ließ sich nicht anbringen" >> "$ausgabe"
        mv "$datei.orig" "$datei"
        touch "$datei"
        return
    fi

    cmake --build "$BAU" -j "$(nproc)" > /dev/null 2>&1
    local gefallen
    gefallen=$(ctest --test-dir "$BAU" -R librarytest --output-on-failure 2>&1 \
        | grep '^FAIL!' | sed 's/^FAIL!  : LibraryTest:://; s/ .*//' | sort -u | tr '\n' ' ')

    {
        echo "== $name =="
        echo "Eingriff : $alt"
        echo "     →     $neu"
        if [ -z "$gefallen" ]; then
            echo "Ergebnis : GRÜN — der Prüfsatz hat den Eingriff NICHT bemerkt"
        else
            echo "Ergebnis : rot — gefallen sind: $gefallen"
        fi
        echo
    } >> "$ausgabe"

    mv "$datei.orig" "$datei"
    touch "$datei"
}

probe "1. Eintragslinie ohne die Ausnahme an der Auswahl (AK 3a)" "$DELEGATE" \
    '!isGroupHead(below) && !selected && !isSelectedIn(entry, below)' \
    '!isGroupHead(below)'

probe "2. Eintragslinie auch unter der letzten Notiz einer Gruppe (AK 3a, Doppellinie)" "$DELEGATE" \
    'below.isValid() && !isGroupHead(below) &&' \
    'below.isValid() &&'

probe "3. Gruppenlinie auch über dem ersten Kopf (AK 2)" "$DELEGATE" \
    'if (index.row() > 0) {' \
    'if (index.row() >= 0) {'

probe "4. Eintragslinie über die volle Breite statt eingerückt (AK 1)" "$DELEGATE" \
    'QRect(textLeft(entry.rect), entry.rect.bottom(), width, 1)' \
    'QRect(entry.rect.x(), entry.rect.bottom(), entry.rect.width(), 1)'

probe "5. Linienfarbe aus einer Palettenrolle statt aus der Mischung (AK 4)" "$DELEGATE" \
    'const qreal share = KColorScheme::frameContrast();' \
    'const qreal share = 0.20;'

probe "6. Ohne das Neuzeichnen der oberen Nachbarn (AK 3c)" "$FENSTER" \
    '    repaintTheRowAbove(index);
    repaintTheRowAbove(previous);' \
    '    // Eingriff der Mutationsprobe: hier stand das Neuzeichnen.'

echo "== Stand wiederherstellen und noch einmal bauen =="
cmake --build "$BAU" -j "$(nproc)" > /dev/null 2>&1
{
    echo "== Gegenprobe: unveränderter Stand =="
    ctest --test-dir "$BAU" -R librarytest 2>&1 | grep -E 'tests passed|Passed|Failed'
} >> "$ausgabe"

echo "Fertig. Ergebnis in $ausgabe"
cat "$ausgabe"
