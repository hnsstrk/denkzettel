#!/usr/bin/env bash
# Fährt die Messungen der Vorprüfung zu Issue #73 (Bearbeiter A) erneut.
#
# Das Skript baut in einen **eigenen** Bauplatz (`build/` neben dieser Datei,
# von .gitignore gedeckt). Es fasst `build/` der Repositoriumswurzel nur
# **lesend** an, schreibt nichts unter `/usr` und ändert keine Einstellung des
# Kunden. Es zeigt kein Fenster; alles läuft ohne Sitzung und ohne Compositor.
#
# **Ein Lauf braucht das Netz** (M10, Abruf der Roh-URLs der Bildschirmfotos).
# Ohne Netz meldet er das und wird übersprungen.
#
# Aufruf: bash docs/scrum/vorberichte/73-appstream/pruefen.sh

set -euo pipefail

HIER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WURZEL="$(cd "$HIER/../../../.." && pwd)"
BAU="$HIER/build"
M="$HIER/messungen"
FLUECHTIG="$(mktemp -d)"
trap 'rm -rf "$FLUECHTIG"' EXIT

mkdir -p "$M"
ENTWURF="$HIER/sonden/metainfo/org.denkzettel.Denkzettel.metainfo.xml"

echo "== M1: Werkzeugstand und Quelltext des ECM-Tests =="
{
    pacman -Q appstream extra-cmake-modules cmake
    appstreamcli --version
    echo "--- /usr/share/ECM/kde-modules/appstreamtest.cmake ---"
    cat /usr/share/ECM/kde-modules/appstreamtest.cmake
} > "$M/m1-appstreamtest-quelle.txt"

echo "== M7: Installationsziel der Metainfo =="
cmake -B "$BAU/dirs" -S "$HIER/sonden/dirs" > "$M/m7-metainfodir.txt" 2>&1

echo "== M8/M9: Entwurf validieren, Pflichtfelder einzeln herausnehmen =="
{
    echo "--- unversehrt ---"
    appstreamcli validate --no-net "$ENTWURF" || echo "rc=$?"
    for feld in metadata_license content_rating; do
        echo "--- ohne ${feld} ---"
        grep -v "$feld" "$ENTWURF" > "$FLUECHTIG/x.metainfo.xml"
        appstreamcli validate --no-net "$FLUECHTIG/x.metainfo.xml" && echo "rc=0" || echo "rc=$?"
    done
    # categories braucht den ganzen Block heraus, nicht nur die Zeilen mit dem
    # Wort: bliebe <category>Utility</category> stehen, meldete appstreamcli
    # `unknown-tag category` und man mäße die falsche Sache.
    echo "--- ohne categories (ganzer Block) ---"
    awk '/<categories>/{weg=1} !weg{print} /<\/categories>/{weg=0}' "$ENTWURF" \
        > "$FLUECHTIG/x.metainfo.xml"
    grep -c 'categor' "$FLUECHTIG/x.metainfo.xml" || echo "0 Zeilen mit 'categor' — Block ist raus"
    appstreamcli validate --no-net "$FLUECHTIG/x.metainfo.xml" && echo "rc=0" || echo "rc=$?"
    echo "--- ohne developer (ganzer Block) ---"
    awk '/<developer /{weg=1} !weg{print} /<\/developer>/{weg=0}' "$ENTWURF" \
        > "$FLUECHTIG/x.metainfo.xml"
    appstreamcli validate --no-net "$FLUECHTIG/x.metainfo.xml" && echo "rc=0" || echo "rc=$?"
    # Zeigt, wo die Grenze zwischen rot und grün liegt: eine einzelne WARNUNG
    # färbt rot, der pedantische Hinweis zur Groß-/Kleinschreibung der Id nicht.
    echo "--- Id ohne rDNS-Form (erzeugt eine Warnung) ---"
    sed 's|<id>org.denkzettel.Denkzettel</id>|<id>Denkzettel</id>|' "$ENTWURF" \
        > "$FLUECHTIG/x.metainfo.xml"
    appstreamcli validate --no-net "$FLUECHTIG/x.metainfo.xml" && echo "rc=0" || echo "rc=$?"
    echo "--- pedantische Meldungen des unversehrten Entwurfs ---"
    appstreamcli validate --no-net --pedantic "$ENTWURF" && echo "rc=0" || echo "rc=$?"
} > "$M/m9-pflichtfelder-und-mutation.txt" 2>&1

echo "== M10: Roh-URLs der Bildschirmfotos =="
if curl -s -o /dev/null --max-time 10 https://raw.githubusercontent.com/ ; then
    {
        for u in erfassungsfenster bibliothek; do
            url="https://raw.githubusercontent.com/hnsstrk/denkzettel/main/docs/bilder/${u}.png"
            printf '%s -> ' "$url"
            curl -s -o /dev/null -w '%{http_code}\n' --max-time 20 "$url"
        done
        echo "--- Entwurf mit den echten Bild-URLs, OHNE --no-net ---"
        appstreamcli validate "$ENTWURF" && echo "rc=0" || echo "rc=$?"
        echo "--- mit erfundener Bild-URL, OHNE --no-net ---"
        sed 's|bibliothek.png|gibtsnicht.png|' "$ENTWURF" > "$FLUECHTIG/n.metainfo.xml"
        appstreamcli validate "$FLUECHTIG/n.metainfo.xml" && echo "rc=0" || echo "rc=$?"
        echo "--- dieselbe Datei MIT --no-net ---"
        appstreamcli validate --no-net "$FLUECHTIG/n.metainfo.xml" && echo "rc=0" || echo "rc=$?"
    } > "$M/m10-screenshots-netz.txt" 2>&1
else
    echo "  kein Netz — M10 übersprungen" | tee "$M/m10-screenshots-netz.txt"
fi

echo "== M12/M13: DESTDIR-Validierungstest und Mutationsprobe =="
# Die Mutationen laufen auf einer **Kopie** der Sonde im flüchtigen Ordner. Die
# versionierte Sonde unter sonden/ bleibt unangetastet — ein abgebrochener Lauf
# hinterließe sonst eine beschädigte Datei im Repo, und der nächste Lauf mäße
# dann seinen eigenen Rest statt der Sache.
SONDE="$FLUECHTIG/metainfotest"
cp -r "$HIER/sonden/metainfotest" "$SONDE"
SONDENXML="$SONDE/org.denkzettel.Denkzettel.metainfo.xml"
cp "$SONDENXML" "$FLUECHTIG/orig.xml"
# Frischer Bauplatz je Lauf: Die Quelle liegt in einem flüchtigen Ordner, dessen
# Name sich jedes Mal ändert — ein alter Cache verweigert dann den Dienst.
rm -rf "$BAU/metainfotest"
cmake -B "$BAU/metainfotest" -S "$SONDE" > /dev/null
{
    echo "--- Lauf 1: unversehrt ---"
    ctest --test-dir "$BAU/metainfotest" --output-on-failure 2>&1 | tail -6 || true
    for mutation in "metadata_license entfernt" "XML-Wurzel unvollstaendig" "Datei geloescht"; do
        echo "--- Mutation: ${mutation} ---"
        case "$mutation" in
            "metadata_license entfernt") grep -v metadata_license "$FLUECHTIG/orig.xml" > "$SONDENXML" ;;
            "XML-Wurzel unvollstaendig") sed 's|</component>||' "$FLUECHTIG/orig.xml" > "$SONDENXML" ;;
            "Datei geloescht") rm -f "$SONDENXML" ;;
        esac
        ctest --test-dir "$BAU/metainfotest" 2>&1 | tail -4 || true
    done
    cp "$FLUECHTIG/orig.xml" "$SONDENXML"
    echo "--- zurueckgesetzt ---"
    ctest --test-dir "$BAU/metainfotest" 2>&1 | tail -3 || true
} > "$M/m13-mutationsprobe.txt" 2>&1
# Der Staging-Baum aus dem letzten (unversehrten) Lauf trägt M18/M19.
cp -r "$BAU/metainfotest/metainfotest-root" "$FLUECHTIG/staging" 2>/dev/null || true

echo "== M14: ECM-appstreamtest in fünf Manifest-Fällen =="
STAGING="$BAU/metainfotest/metainfotest-root"
{
    awk '{print}' "$WURZEL/build/install_manifest.txt" > "$FLUECHTIG/ohne.txt"
    echo "--- Fall 1: Manifest fehlt ganz (Lage im CI) ---"
    cmake -DAPPSTREAMCLI=/usr/bin/appstreamcli -DINSTALL_FILES="$FLUECHTIG/fehlt.txt" \
        -P /usr/share/ECM/kde-modules/appstreamtest.cmake && echo "rc=0" || echo "rc=$?"
    echo "--- Fall 2: Manifest ohne Metainfo (Lage heute) ---"
    cmake -DAPPSTREAMCLI=/usr/bin/appstreamcli -DINSTALL_FILES="$FLUECHTIG/ohne.txt" \
        -P /usr/share/ECM/kde-modules/appstreamtest.cmake && echo "rc=0" || echo "rc=$?"
    echo "--- Fall 3: Manifest nennt eine Metainfo, die dort nicht liegt ---"
    cp "$FLUECHTIG/ohne.txt" "$FLUECHTIG/fehlend.txt"
    echo "/usr/share/metainfo/org.denkzettel.Denkzettel.metainfo.xml" >> "$FLUECHTIG/fehlend.txt"
    cmake -DAPPSTREAMCLI=/usr/bin/appstreamcli -DINSTALL_FILES="$FLUECHTIG/fehlend.txt" \
        -P /usr/share/ECM/kde-modules/appstreamtest.cmake && echo "rc=0" || echo "rc=$?"
    echo "--- Fall 4: Manifest nennt eine vorhandene, gueltige Metainfo ---"
    cp "$FLUECHTIG/ohne.txt" "$FLUECHTIG/da.txt"
    echo "${STAGING}/usr/share/metainfo/org.denkzettel.Denkzettel.metainfo.xml" >> "$FLUECHTIG/da.txt"
    cmake -DAPPSTREAMCLI=/usr/bin/appstreamcli -DINSTALL_FILES="$FLUECHTIG/da.txt" \
        -P /usr/share/ECM/kde-modules/appstreamtest.cmake && echo "rc=0" || echo "rc=$?"
    echo "--- Fall 5: dieselbe Datei, beschaedigt ---"
    mkdir -p "$FLUECHTIG/usr/share/metainfo"
    grep -v metadata_license "$STAGING/usr/share/metainfo/org.denkzettel.Denkzettel.metainfo.xml" \
        > "$FLUECHTIG/usr/share/metainfo/org.denkzettel.Denkzettel.metainfo.xml"
    cp "$FLUECHTIG/ohne.txt" "$FLUECHTIG/kaputt.txt"
    echo "$FLUECHTIG/usr/share/metainfo/org.denkzettel.Denkzettel.metainfo.xml" >> "$FLUECHTIG/kaputt.txt"
    cmake -DAPPSTREAMCLI=/usr/bin/appstreamcli -DINSTALL_FILES="$FLUECHTIG/kaputt.txt" \
        -P /usr/share/ECM/kde-modules/appstreamtest.cmake && echo "rc=0" || echo "rc=$?"
} > "$M/m14-ecm-appstreamtest-faelle.txt" 2>&1

echo "== M18/M19: validate-tree gegen validate =="
BAUM="$FLUECHTIG/baum"
mkdir -p "$BAUM/usr/share/metainfo"
cp -r "$WURZEL/build/tests/installtest-root/." "$BAUM/" 2>/dev/null || true
{
    echo "--- validate-tree mit gueltiger Metainfo ---"
    cp "$ENTWURF" "$BAUM/usr/share/metainfo/"
    appstreamcli validate-tree --no-net "$BAUM" && echo "rc=0" || echo "rc=$?"
    echo "--- validate-tree OHNE jede Metainfo ---"
    rm "$BAUM/usr/share/metainfo/org.denkzettel.Denkzettel.metainfo.xml"
    appstreamcli validate-tree --no-net "$BAUM" && echo "rc=0" || echo "rc=$?"
    echo "--- falsche desktop-id: erst validate, dann validate-tree ---"
    sed 's|org.denkzettel.Denkzettel.desktop</launchable>|org.denkzettel.GibtsNicht.desktop</launchable>|' \
        "$ENTWURF" > "$BAUM/usr/share/metainfo/org.denkzettel.Denkzettel.metainfo.xml"
    appstreamcli validate --no-net "$BAUM/usr/share/metainfo/org.denkzettel.Denkzettel.metainfo.xml" \
        && echo "rc=0" || echo "rc=$?"
    appstreamcli validate-tree --no-net "$BAUM" && echo "rc=0" || echo "rc=$?"
} > "$M/m19-launchable-quervergleich.txt" 2>&1

echo
echo "Fertig. Ausgaben in ${M}/"
