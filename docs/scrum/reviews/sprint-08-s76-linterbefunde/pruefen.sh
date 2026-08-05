#!/usr/bin/env bash
# Wiederholt die Messungen zu #76 — „Linterbefunde heilen".
#
# BAUPLATZ: liegt unter $TMPDIR, nicht im Repositorium. Weder build/ der
# Wurzel noch ein Ordner neben diesem Skript werden angefasst — in build/
# arbeiten möglicherweise andere Stränge, und ein Bauplatz im Baum ist kein
# Beleg, sondern Abfall.
#
# NICHTS AM PROJEKT WIRD GEÄNDERT. Die Gegenprobe, die absichtlich einen
# Befund einbaut, läuft auf einer Wegwerfkopie aus `git archive HEAD`.
#
# Aufruf:  bash docs/scrum/reviews/sprint-08-s76-linterbefunde/pruefen.sh
#
# WARUM DIESES SKRIPT AUSDRÜCKLICH MIT bash LÄUFT UND NICHT MIT sh ODER zsh:
# Es übergibt eine Dateiliste als getrennte Argumente. In zsh zerlegt eine
# ungeschützte Parameterexpansion keine Wörter — `run-clang-tidy $DATEIEN`
# bekäme dort **ein** Argument mit Zeilenumbrüchen darin, baute daraus einen
# regulären Ausdruck, der auf nichts passt, und meldete eine saubere
# Konvergenz über null geprüfte Dateien. Gemessen am 05.08.2026; der einzige
# Hinweis stand in `for 0 files out of 49`. Deshalb übergibt dieses Skript die
# Liste über `xargs -a` und **liest die Zeile mit der Dateizahl vor**.
set -o pipefail

HIER="$(cd "$(dirname "$0")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
ARBEIT="${TMPDIR:-/tmp}/denkzettel-76-pruefung"
# Die Ausgabe dieses Laufs geht NICHT nach messungen/ daneben. Dort steht der
# Nachweis des Laufs vom 05.08.2026, gegen den verglichen wird; ein Wiederholer
# soll ihn lesen und nicht überschreiben.
MESS="$ARBEIT/messungen"
BAU="$ARBEIT/build"
BAU_SPIKE="$ARBEIT/build-spike"
mkdir -p "$MESS" "$ARBEIT"
echo "Ausgabe dieses Laufs: $MESS"
echo "Nachweis vom 05.08.2026 zum Vergleich: $HIER/messungen"

echo "== Stand und Werkzeuge =="
git -C "$WURZEL" log --oneline -1
pacman -Q clang clazy cmake qt6-base

echo
echo "== A — Standardkonfiguration (-DDENKZETTEL_SPIKE_SPELLFIX=OFF) =="
echo "   Das ist die Schalterstellung, für die „0\" gilt."
cmake -B "$BAU" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug > "$MESS/00-configure.txt" 2>&1
cmake --build "$BAU" -j "$(nproc)" > "$MESS/01-bau.txt" 2>&1
echo "Bau        rc=$?  Compilerwarnungen=$(grep -c 'warning:' "$MESS/01-bau.txt")  Fehler=$(grep -c 'error:' "$MESS/01-bau.txt")"

cmake --build "$BAU" --target lint-tidy > "$MESS/02-tidy.txt" 2>&1
echo "lint-tidy  rc=$?  Warnungen=$(grep -c 'warning:' "$MESS/02-tidy.txt")  Fehler=$(grep -c 'error:' "$MESS/02-tidy.txt")"
# Die Vollständigkeitszeile: ohne sie sagt eine 0 nichts darüber, ob überhaupt
# etwas analysiert wurde.
grep -m1 'files out of' "$MESS/02-tidy.txt" || echo "  ACHTUNG: keine Zeile 'files out of' — der Lauf hat womöglich nichts geprüft"

cmake --build "$BAU" --target lint-clazy > "$MESS/03-clazy.txt" 2>&1
echo "lint-clazy rc=$?  Warnungen=$(grep -c 'warning:' "$MESS/03-clazy.txt")  Fehler=$(grep -c 'error:' "$MESS/03-clazy.txt")"

ctest --test-dir "$BAU" > "$MESS/04-ctest.txt" 2>&1
echo "ctest      rc=$?  $(grep -E 'tests passed' "$MESS/04-ctest.txt")"

echo
echo "== B — Gegen-Schalterstellung (-DDENKZETTEL_SPIKE_SPELLFIX=ON) =="
echo "   Erwartung: 3 Befunde, alle in tests/spellfixspike.cpp (Entscheidung 5)."
cmake -B "$BAU_SPIKE" -S "$WURZEL" -DCMAKE_BUILD_TYPE=Debug -DDENKZETTEL_SPIKE_SPELLFIX=ON > "$MESS/05-spike-configure.txt" 2>&1
cmake --build "$BAU_SPIKE" -j "$(nproc)" > "$MESS/06-spike-bau.txt" 2>&1
cmake --build "$BAU_SPIKE" --target lint-tidy > "$MESS/07-spike-tidy.txt" 2>&1
echo "lint-tidy  rc=$?  Warnungen=$(grep -c 'warning:' "$MESS/07-spike-tidy.txt")"
grep 'warning:' "$MESS/07-spike-tidy.txt" | sed "s|^$WURZEL/||" | sort -u
cmake --build "$BAU_SPIKE" --target lint-clazy > "$MESS/08-spike-clazy.txt" 2>&1
echo "lint-clazy rc=$?  Warnungen=$(grep -c 'warning:' "$MESS/08-spike-clazy.txt")"

echo
echo "== C — Gegenprobe: schlagen die Wachen überhaupt an? =="
echo "   Eine 0 ist erst eine Aussage, wenn ein eingebauter Fehler rot wird."
K="$ARBEIT/gegenprobe"; rm -rf "$K"; mkdir -p "$K"
git -C "$WURZEL" archive HEAD | tar -x -C "$K"
cd "$K" || exit 1
# Zwei Muster, die #76 geheilt hat, wieder eingesetzt: ein `first()` auf einem
# Temporär (clazy) und eine lokale Variable ohne `const` (clang-tidy).
python3 - <<'PY'
import io
p = 'tests/shelltest.cpp'
s = io.open(p, encoding='utf-8').read()
alt = ("    const QList<QAction *> entries = icon.item()->contextMenu()->actions();\n"
       "    const QAction *capture = entries.constFirst();")
neu = ("    const QAction *capture = icon.item()->contextMenu()->actions().first();\n"
       "    QString probeVariableForTidy = QStringLiteral(\"nur fuer die Gegenprobe\");\n"
       "    Q_UNUSED(probeVariableForTidy)")
assert s.count(alt) == 1, 'Die geheilte Stelle steht nicht mehr so da — Gegenprobe anpassen'
io.open(p, 'w', encoding='utf-8').write(s.replace(alt, neu))
PY
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug > /dev/null 2>&1
cmake --build build -j "$(nproc)" > gegenprobe-bau.log 2>&1
cmake --build build --target lint-clazy > "$MESS/09-gegenprobe-clazy.txt" 2>&1
echo "lint-clazy mit eingebautem Fehler: rc=$?  Warnungen=$(grep -c 'warning:' "$MESS/09-gegenprobe-clazy.txt")  (erwartet: 1)"
cmake --build build --target lint-tidy > "$MESS/10-gegenprobe-tidy.txt" 2>&1
echo "lint-tidy  mit eingebautem Fehler: rc=$?  Warnungen=$(grep -c 'warning:' "$MESS/10-gegenprobe-tidy.txt")  (erwartet: 1)"
grep 'warning:' "$MESS/09-gegenprobe-clazy.txt" "$MESS/10-gegenprobe-tidy.txt" | sed "s|$K/||"

echo
echo "== D — Wie viele NOLINT stehen im Baum, und trägt jedes eine Begründung? =="
cd "$WURZEL" || exit 1
# Nur wirksame Marken werden gezählt: eine Zeile, die mit `// NOLINT…` beginnt.
# Eine Nennung im Fließtext eines Kommentars sieht sonst aus wie eine Marke und
# bläht die Zahl — im Kopf von librarytest.cpp steht genau so eine.
git grep -c -E '^\s*// NOLINT' -- src tests | tee "$MESS/11-nolint-je-datei.txt"
echo "NOLINT-Marken insgesamt: $(git grep -h -E '^\s*// NOLINT' -- src tests | wc -l)"
echo "nach Prüfung:"
git grep -h -oE '^\s*// NOLINT[A-Z]*\([a-z-]*\)' -- src tests | sed 's/.*(\(.*\))/\1/' | sort | uniq -c | sort -rn
echo
echo "Trägt jede Marke eine Begründung? Die Zeile davor bzw. der Kommentarblock:"
git grep -n -E '^\s*// NOLINT' -- src tests
