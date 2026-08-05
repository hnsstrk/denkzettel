#!/usr/bin/env bash
# Mutationsproben zu #61: Je tragender Zusicherung wird die Heilung **entfernt**
# und gezeigt, dass der Prüfsatz rot wird. Ein Prüfsatz, der ohne die Heilung
# grün bleibt, prüft die Heilung nicht.
#
# Diese Proben haben sich schon beim Entwerfen bezahlt gemacht: Die ersten
# Fassungen von M5 und M6 liefen gegen einen unerreichbaren Sitzungsbus, und
# dort endet der Dienst ohnehin mit 1 — ein **angenommener** Schalter wäre grün
# durchgelaufen. Der Prüfsatz wurde daraufhin auf den Sitzungsbus des Tests
# umgestellt (tests/commandlinetest.cpp, expectRefusal).
#
# M9 ist die Gegenprobe und bleibt erwartungsgemäß grün: Sie zeigt, dass der
# Bestand vor dieser Story den teuersten Bruch nicht gefangen hätte.
#
# Das Skript arbeitet auf einer **Kopie** des Arbeitsbaums unter /tmp: Es ändert
# keine Datei des Repositoriums und hinterlässt keinen halb mutierten Stand,
# auch wenn es mittendrin abbricht.
#
# Aufruf: bash docs/scrum/reviews/sprint-08-s61-versionsanzeige/mutationsproben.sh

set -uo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
ARBEIT="$(mktemp -d)"
trap 'rm -rf "$ARBEIT"' EXIT

echo "Kopie des Arbeitsbaums nach $ARBEIT"
git -C "$WURZEL" ls-files -z | xargs -0 -I{} cp --parents "{}" "$ARBEIT/" 2>/dev/null
cd "$ARBEIT" || exit 1

IDENTITAET="src/shell/appidentity.cpp"
START="src/main.cpp"
BAUWERK="src/CMakeLists.txt"
BAU="$ARBEIT/build"

lauf() {
    local nummer="$1"
    local was="$2"
    local erwartet="$3"
    shift 3
    echo
    echo "===== Probe $nummer — $was"
    echo "Eingriff: $*"
    echo "Erwartet: $erwartet"
    "$@"
    if ! cmake --build "$BAU" -j "$(nproc)" \
             --target denkzetteld identitytest commandlinetest > "$ARBEIT/bau.log" 2>&1; then
        echo "ERGEBNIS: übersetzt nicht mehr — die Zusicherung hängt am Bau selbst."
        grep -m3 " error" "$ARBEIT/bau.log" | sed 's/^/  /'
    else
        local ausgabe
        ausgabe="$(ctest --test-dir "$BAU" -R 'identitytest|commandlinetest' --output-on-failure 2>&1)"
        if grep -q "100% tests passed" <<< "$ausgabe"; then
            echo "ERGEBNIS: **GRÜN GEBLIEBEN** — kein Prüfsatz deckt diese Zusicherung."
        else
            echo "ERGEBNIS: rot."
            grep -E "^(FAIL!|Errors|.*Assertion|Der Schalter|.*Failed)" <<< "$ausgabe" \
                | head -4 | sed 's/^/  /'
        fi
        grep -E "tests passed|tests failed" <<< "$ausgabe" | sed 's/^/  /'
    fi
    cp "$WURZEL/$IDENTITAET" "$IDENTITAET"
    cp "$WURZEL/$START" "$START"
    cp "$WURZEL/$BAUWERK" "$BAUWERK"
}

echo "== Ausgangsstand übersetzen =="
# Die Kopie ist kein Git-Repositorium; der Bau kommt ohne aus.
cmake -B "$BAU" -S . -DCMAKE_BUILD_TYPE=Debug > /dev/null
cmake --build "$BAU" -j "$(nproc)" > /dev/null
ctest --test-dir "$BAU" 2>&1 | grep -E "tests passed"

echo
echo "########## Die Namen, an denen SPEC 2.3 und 2.4 hängen ##########"

lauf 1 "Die Organisationsdomäne am KAboutData (SPEC 2.3)" \
    "rot: organizationDomain 'kde.org' statt 'denkzettel.org', und der Bus führt org.kde.Daemon" \
    sed -i '/about.setOrganizationDomain(QByteArrayLiteral("denkzettel.org"));/d' "$IDENTITAET"

lauf 2 "Der Desktop-Dateiname am KAboutData (SPEC 2.4)" \
    "rot: desktopFileName 'org.kde.denkzettel' statt 'org.denkzettel.Denkzettel'" \
    sed -i '/about.setDesktopFileName(QStringLiteral("org.denkzettel.Denkzettel"));/d' "$IDENTITAET"

lauf 3 "Der Komponentenname trägt den Pfad der Notizen" \
    "rot: der Datenbankpfad wandert unter ~/.local/share/Denkzettel" \
    sed -i 's|    KAboutData about(QStringLiteral("denkzettel"),|    KAboutData about(QStringLiteral("Denkzettel"),|' \
    "$IDENTITAET"

echo
echo "########## Die Reihenfolge in main.cpp ##########"

lauf 4 "Die Auswertung liegt vor der Einzelinstanz-Weiche (F3)" \
    "rot: ohne erreichbaren Bus endet KDBusService mit 1, die Versionszeile bleibt aus" \
    bash -c "
        sed -i '/^    processCommandLineArguments(app);\$/d' '$START'
        sed -i 's|^    KDBusService service(KDBusService::Unique);\$|    KDBusService service(KDBusService::Unique);\n    processCommandLineArguments(app);|' '$START'
    "

lauf 5 "Die Registrierung liegt vor der Auswertung" \
    "rot: 'denkzettel ' ohne Nummer — applicationVersion ist zur Ausgabezeit noch leer" \
    bash -c "
        sed -i '/^    registerApplicationIdentity();\$/d' '$START'
        sed -i 's|^    processCommandLineArguments(app);\$|    processCommandLineArguments(app);\n    registerApplicationIdentity();|' '$START'
    "

echo
echo "########## Die Schalter ##########"

lauf 6 "process() weist zurück, parse() nicht (AK 6)" \
    "rot: --kennt-keiner wird angenommen, der Dienst läuft damit weiter" \
    sed -i 's|    parser.process(app);|    parser.parse(QCoreApplication::arguments());|' "$IDENTITAET"

lauf 7 "--desktopfile bleibt unangemeldet (AK 7)" \
    "rot: mit setupCommandLine() ist der Schalter bekannt und der Dienst startet damit" \
    bash -c "
        sed -i 's|^    parser.addHelpOption();\$|    KAboutData angemeldet = KAboutData::applicationData();\n    angemeldet.setupCommandLine(\&parser);|' '$IDENTITAET'
        sed -i '/^    parser.addVersionOption();\$/d' '$IDENTITAET'
    "

lauf 8 "Der argumentlose Start bleibt der Start des Dienstes (AK 6)" \
    "rot: org.denkzettel.Daemon meldet sich nicht mehr an" \
    sed -i 's|    parser.process(app);|    parser.process(app);\n    ::exit(2);|' "$IDENTITAET"

echo
echo "########## Der Durchreichweg aus CMake ##########"

lauf 9 "Die Nummer kommt aus PROJECT_VERSION (AK 3)" \
    "rot: die Ausgabe trägt 9.9.9, der Prüfsatz vergleicht gegen 0.1.0 aus project()" \
    sed -i 's|    DENKZETTEL_VERSION="\${PROJECT_VERSION}")|    DENKZETTEL_VERSION="9.9.9")|' "$BAUWERK"

echo
echo "########## Gegenprobe: was der Bestand vor dieser Story gefangen hätte ##########"
echo
echo "===== Probe 10 — derselbe Eingriff wie Probe 1, gemessen am Altbestand"
echo "Eingriff: sed -i '/about.setOrganizationDomain(...)/d' $IDENTITAET"
echo "Erwartet: GRÜN — kein Prüfsatz vor #61 sieht den Busnamen"
sed -i '/about.setOrganizationDomain(QByteArrayLiteral("denkzettel.org"));/d' "$IDENTITAET"
cmake --build "$BAU" -j "$(nproc)" > "$ARBEIT/bau.log" 2>&1
altbestand="$(ctest --test-dir "$BAU" -E 'identitytest|commandlinetest' 2>&1)"
if grep -q "100% tests passed" <<< "$altbestand"; then
    echo "ERGEBNIS: **GRÜN GEBLIEBEN** — und das ist der Befund: Der Bruch, der den"
    echo "          Busnamen und die globalen Kürzel kostet, war vor dieser Story"
    echo "          von keinem Prüfsatz gedeckt."
else
    echo "ERGEBNIS: rot — ein bestehender Prüfsatz fängt es doch."
    grep -E "^(FAIL!|.*Failed)" <<< "$altbestand" | head -4 | sed 's/^/  /'
fi
grep -E "tests passed|tests failed" <<< "$altbestand" | sed 's/^/  /'
cp "$WURZEL/$IDENTITAET" "$IDENTITAET"

echo
echo "Fertig. Der Arbeitsbaum des Repositoriums wurde nicht angefasst."
echo "Eine Probe bleibt erwartungsgemäß grün: M10 (Gegenprobe am Altbestand)."
