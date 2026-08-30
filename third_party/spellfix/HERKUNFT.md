# spellfix.c — Herkunft und Fassung

Fremdcode, unverändert übernommen. Nicht bearbeiten: Änderungen brächen die
Prüfsumme und damit den Nachweis, dass hier der Originalstand liegt.

| Angabe | Wert |
|---|---|
| Datei | `ext/misc/spellfix.c` aus dem SQLite-Quellbaum |
| Bezugsquelle | `https://sqlite.org/src/raw/ext/misc/spellfix.c?ci=version-3.53.4` |
| Fassung | SQLite 3.53.4 (Prüf-Tag `version-3.53.4`) |
| SHA-256 | `f679ae6f10181608be4119c35eedb4276612ef81890ca534e41810deb3e5dea2` |
| Umfang | 3084 Zeilen |
| Lizenz | Public Domain (Segenswunsch im Dateikopf, wie SQLite selbst) |

Gegenprobe: derselbe Abruf über den Zweig `release` statt über den Versions-Tag
liefert dieselbe Prüfsumme — die Fassung ist die des installierten SQLite
(3.53.4), gegen das der Qt-Treiber gelinkt ist.

Prüfen:

```sh
sha256sum third_party/spellfix/spellfix.c
```

## Warum die Datei überhaupt hier liegt

spellfix1 steckt in keinem Standard-Build von SQLite — weder in der
Amalgamation noch als ausgeliefertes Modul. Anwendungen, die es wollen,
kompilieren diese eine C-Datei mit ein und registrieren sie am Datenbank-Handle
(`recherche/2026-08-02-fuzzy-suche.md`, Befund 1).

Seit Issue #69 (30.08.2026) ist das **Produktivcode**: Die Datei wird ohne
Schalter mitübersetzt (`SQLITE_CORE=1`, `src/CMakeLists.txt`), und
`Store::open()` registriert sie an dem `sqlite3*`, den der Qt-Treiber
herausgibt. Der Bauschalter `DENKZETTEL_SPIKE_SPELLFIX` des Spikes ist damit
weg — die tolerante Suche aus SPEC 6 hängt daran, und ein Bau ohne sie wäre ein
anderes Produkt.

Die Lint-Sperre ist doppelt und beide Hälften sind gemessen (30.08.2026): Die
Dateiliste der Lint-Ziele sammelt nur `src/*.cpp` und `tests/*.cpp` ein, deshalb
kommt `spellfix.c` in keinem der 71 Läufe von `lint-tidy` und `lint-clazy` vor,
obwohl es dreimal in der Compile-Datenbank steht. Und die `.clang-tidy` daneben
fängt den Lauf von Hand ab: `clang-tidy -p build/lint third_party/spellfix/spellfix.c`
meldet mit ihr keinen einzigen Befund und ohne sie **27**.
