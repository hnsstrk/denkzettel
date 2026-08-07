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
# **Der Rückgabewert ist der Befund.** Ungleich null heißt: mindestens eine
# Probe hat nichts gemessen — sie brach ab, blieb grün, oder der Stand ohne
# Eingriff war rot. Das steht hier auf einen gemessenen Vorfall hin
# (karpathy-Review Sprint 9, K3, 07.08.2026): Probe 5 brach ab, weil der
# Produktivcode nach einer Heilung anders lautete als das gesuchte Muster. Der
# Wächter hat es gemeldet — mitten in ein Protokoll, das niemand mehr las, und
# der Bericht führte die Probe als bestanden. **Eine Meldung, die nur im
# Protokoll steht, erreicht niemanden; ein Rückgabewert erreicht auch den, der
# das Skript aus einem anderen Skript ruft.**
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

ABBRUECHE=0
STUMME=0
PROBEN=0

# Der Eingriff selbst liegt in einer eigenen Datei statt in einem Here-Dokument:
# nur so lässt sich seine Fehlermeldung einfangen und ins Protokoll schreiben,
# statt sie an der Ausgabe vorbeilaufen zu lassen.
EINGRIFF="$(mktemp)"
MELDUNG="$(mktemp)"
trap 'rm -f "$EINGRIFF" "$MELDUNG"' EXIT

cat > "$EINGRIFF" <<'PY'
import sys

pfad, alt, neu = sys.argv[1], sys.argv[2], sys.argv[3]
text = open(pfad, encoding="utf-8").read()
treffer = text.count(alt)
if treffer != 1:
    sys.exit(f"Muster kommt {treffer}-mal vor, erwartet genau einmal: {alt!r}")
open(pfad, "w", encoding="utf-8").write(text.replace(alt, neu))
PY

{
    echo "Mutationsproben zu #101 — kann jeder Prüfsatz fallen?"
    echo "Stand: $(date '+%F %H:%M %Z'), Ganymed. $(cd "$WURZEL" && git rev-parse --short HEAD) auf $(cd "$WURZEL" && git rev-parse --abbrev-ref HEAD)."
    echo
} > "$ausgabe"

zuruecknehmen() {
    local datei="$1"
    mv "$datei.orig" "$datei"
    # Angefasst, weil `cp` der Kopie die Uhrzeit des Kopierens gibt und die vor
    # dem Eingriff liegt. Ohne das hielte make die Quelle für älter als das
    # Objekt des mutierten Baus und baute nicht neu — der nächste Lauf misst
    # dann den vorigen Eingriff mit. Genau so ist es beim ersten Durchgang am
    # 07.08.2026 passiert: der Lauf nach der letzten Rücknahme war rot.
    touch "$datei"
}

probe() {
    local name="$1" datei="$2" alt="$3" neu="$4"

    PROBEN=$((PROBEN + 1))
    cp -p "$datei" "$datei.orig"

    if ! python3 "$EINGRIFF" "$datei" "$alt" "$neu" > "$MELDUNG" 2>&1; then
        ABBRUECHE=$((ABBRUECHE + 1))
        {
            echo "== $name =="
            echo "Ergebnis : ABBRUCH — der Eingriff ließ sich nicht anbringen"
            echo "Grund    : $(cat "$MELDUNG")"
            echo "           Diese Probe hat NICHTS gemessen."
            echo
        } >> "$ausgabe"
        zuruecknehmen "$datei"
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
            STUMME=$((STUMME + 1))
            echo "Ergebnis : GRÜN — der Prüfsatz hat den Eingriff NICHT bemerkt"
        else
            echo "Ergebnis : rot — gefallen sind: $gefallen"
        fi
        echo
    } >> "$ausgabe"

    zuruecknehmen "$datei"
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

# Der VOLLE Ausdruck, nicht der Aufruf allein: `KColorScheme::frameContrast()`
# steht auch im Kommentar der Funktion und käme damit zweimal vor. Genau daran
# ist diese Probe am 07.08.2026 gescheitert — das Muster stammte aus der Fassung
# vor der Heilung von Fund 4.1, in der `share` noch ein `qreal` war.
probe "5. Linienfarbe aus einem festen Verhältnis statt aus der Konfiguration (AK 4)" "$DELEGATE" \
    'static_cast<float>(KColorScheme::frameContrast())' \
    '0.20F'

probe "6. Ohne das Neuzeichnen der oberen Nachbarn (AK 3c)" "$FENSTER" \
    '    repaintTheRowAbove(index);
    repaintTheRowAbove(previous);' \
    '    // Eingriff der Mutationsprobe: hier stand das Neuzeichnen.'

echo "== Stand wiederherstellen und noch einmal bauen =="
cmake --build "$BAU" -j "$(nproc)" > /dev/null 2>&1
gegenprobe=$(ctest --test-dir "$BAU" -R librarytest 2>&1 | grep -c 'tests passed')
gegenprobe_zeile=$(ctest --test-dir "$BAU" -R librarytest 2>&1 | grep -E 'tests passed')

ROT_OHNE_EINGRIFF=0
case "$gegenprobe_zeile" in
    *"100% tests passed"*) ;;
    *) ROT_OHNE_EINGRIFF=1 ;;
esac

{
    echo "== Gegenprobe: unveränderter Stand =="
    echo "$gegenprobe_zeile"
    echo
    echo "== Bilanz =="
    echo "Proben insgesamt      : $PROBEN"
    echo "davon abgebrochen     : $ABBRUECHE"
    echo "davon stumm geblieben : $STUMME"
    echo "Stand ohne Eingriff   : $([ "$ROT_OHNE_EINGRIFF" -eq 0 ] && echo 'grün' || echo 'ROT')"
} >> "$ausgabe"

cat "$ausgabe"

if [ "$ABBRUECHE" -gt 0 ] || [ "$STUMME" -gt 0 ] || [ "$ROT_OHNE_EINGRIFF" -ne 0 ]; then
    echo
    echo "FEHLGESCHLAGEN: $ABBRUECHE Abbruch/Abbrüche, $STUMME stumme Probe(n)," \
         "Stand ohne Eingriff $([ "$ROT_OHNE_EINGRIFF" -eq 0 ] && echo 'grün' || echo 'ROT')." >&2
    echo "Eine Probe, die abbricht oder grün bleibt, hat nichts gemessen." >&2
    exit 1
fi

echo
echo "Alle $PROBEN Proben haben gemessen. Ergebnis in $ausgabe"
