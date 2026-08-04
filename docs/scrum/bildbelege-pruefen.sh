#!/bin/bash
# Sucht prüfsummengleiche Bildbelege INNERHALB desselben Belegordners.
#
# Zwei Belege sind erst zwei, wenn sie zwei Aufnahmen sind (Sprint 5, V2).
# Diese Prüfung findet Dateien mit verschiedenen Namen und identischen Bytes —
# also zwei Zustände, die denselben Beleg vorzeigen.
#
# Die Abgrenzung ist der Kern: Verglichen wird nur je Ordner. Dass dasselbe
# Bild in der Dev-Übergabe und im UX-Bericht liegt, ist normal;
# ordnerübergreifend gäbe es 44 Gruppen (Messung 04.08.2026), und eine Wache,
# die immer anschlägt, ist keine.
#
# Geprüft werden nur versionierte Dateien — ein unversionierter Beleg ist
# kein Beleg (B7).
#
# Das Skript findet, der Mensch urteilt: Ein Zustand, der nach einer Rückkehr
# wieder derselbe ist, darf zweimal gleich aussehen. Ein Befund ist es, wo
# zwei VERSCHIEDENE Zustände denselben Beleg tragen.
#
# Aufruf:  bash docs/scrum/bildbelege-pruefen.sh [Pfad ...]
#          ohne Pfad: alles unterhalb von docs/scrum
#          am Sprint-Ende: die Belegordner dieses Sprints angeben
#
# Rückgabe: 0 = keine Gruppe gefunden
#           1 = mindestens eine Gruppe, Bewertung durch einen Menschen nötig
#           2 = Aufrufsfehler

set -uo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"

if [ "$#" -gt 0 ]; then
    pfade=("$@")
else
    pfade=("docs/scrum")
fi

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

cd "$ROOT" || exit 2

# git ls-files gibt sortiert aus; Dateien eines Ordners stehen dadurch beieinander.
git ls-files -z -- "${pfade[@]}" \
    | grep -z -i -E '\.(png|jpg|jpeg|webp|gif)$' \
    | xargs -0 -r sha256sum -- > "$tmp/summen" 2>"$tmp/fehler"

if [ -s "$tmp/fehler" ]; then
    echo "Abbruch — Prüfsummen konnten nicht gebildet werden:" >&2
    cat "$tmp/fehler" >&2
    exit 2
fi

anzahl=$(wc -l < "$tmp/summen")
if [ "$anzahl" -eq 0 ]; then
    echo "Keine versionierten Bilder unter: ${pfade[*]}" >&2
    exit 2
fi

echo "Geprüft: $anzahl versionierte Bilder unter ${pfade[*]}"
echo "Verglichen wird ausschließlich innerhalb desselben Ordners."
echo

awk '
{
    hash = $1
    pfad = substr($0, index($0, "  ") + 2)
    ordner = pfad;  sub(/\/[^\/]*$/, "", ordner)
    datei  = pfad;  sub(/^.*\//, "", datei)
    schluessel = ordner SUBSEP hash
    if (schluessel in dateien)
        dateien[schluessel] = dateien[schluessel] ", " datei
    else {
        dateien[schluessel] = datei
        reihenfolge[++n] = schluessel
    }
    zahl[schluessel]++
}
END {
    letzter = ""
    for (i = 1; i <= n; i++) {
        s = reihenfolge[i]
        if (zahl[s] < 2) continue
        split(s, teil, SUBSEP)
        if (teil[1] != letzter) {
            print teil[1]
            letzter = teil[1]
        }
        printf "    %dx %s: %s\n", zahl[s], substr(teil[2], 1, 8), dateien[s]
        gruppen++
        betroffen += zahl[s]
    }
    if (gruppen == 0) {
        print "Keine prüfsummengleichen Dateien innerhalb eines Ordners."
        exit 0
    }
    printf "\n%d %s — %d Dateien tragen %d %s.\n", \
        gruppen, (gruppen == 1 ? "Gruppe" : "Gruppen"), \
        betroffen, gruppen, (gruppen == 1 ? "Aufnahme" : "Aufnahmen")
    print "Jede Gruppe einzeln bewerten: gleicher Zustand ist erlaubt,"
    print "zwei verschiedene Zustände mit demselben Bild sind ein Mangel."
    exit 1
}
' "$tmp/summen"
