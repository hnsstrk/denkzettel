# Einrichtung der automatischen Testläufe — Belege

**Datum:** 2026-08-04 · **Ausführung:** Product Owner · **Anlass:**
Kundenentscheidung vom 04.08.2026, die den offenen Punkt aus Sprint 4 §15.8
schließt (*„Eine spätere Einführung automatischer Testläufe (CI) … wäre eine
neue Kundenentscheidung; sie ist der einzige der beiden Kriteriumsteile, der
ohne menschliche Disziplin trägt."*).

**Gegenstand:** `.github/workflows/ci.yml`, verankert in `PROZESS.md`,
Sprint-Mechanik, Abschnitt „Automatische Testläufe".

---

## 1. Ausgangsstand, vor jeder Änderung gemessen

Auf Ganymed, vollständiger Neubau (`cmake --build build --clean-first`):

| Messung | Wert |
|---|---|
| Compiler-Warnungen | **0** |
| `ctest` | **7/7 grün**, 5,77 s |

Die Warnungsschwelle der CI steht deshalb auf **null**. Eine Schranke oberhalb
des gemessenen Standes wäre keine Schranke.

## 2. Die Rezeptur ist erprobt, nicht behauptet

Vor dem ersten Push ist der komplette Ablauf in **demselben Container** gelaufen,
den der Workflow benutzt (`archlinux:base-devel`, Docker, Quelle
schreibgeschützt eingehängt). Vollständige Ausgabe:
[`container-probelauf.txt`](container-probelauf.txt).

| | Ganymed | Container |
|---|---|---|
| Compiler-Warnungen | 0 | **0** |
| `ctest` | 7/7 | **7/7** |
| Laufzeit Tests | 5,77 s | 19,68 s |

Werkzeugstand im Container: CMake 4.4.2 · GCC 16.1.1 · Qt 6.11.1 ·
KF6 6.28.0 · plasma-integration 6.7.3 · breeze-icons 6.28.0. Alle über den
Mindestständen aus `CMakeLists.txt` (Qt 6.7, KF6 6.0).

**Warum `plasma-integration` und `breeze-icons` in der Paketliste stehen:**
`librarytest` und `shelltest` laufen unter `QT_QPA_PLATFORMTHEME=kde` und prüfen
Symbol**namen** (#60, #66, #67). Fehlt das Plattformthema, liefert
`QIcon::fromTheme()` ein Nullsymbol mit leerem Namen — die Tests wären dann
**grün und wertlos**. Dass sie im Container grün sind, ist damit zugleich der
Nachweis, dass das Thema geladen hat.

**Die Paketliste ist abgeleitet, nicht geraten:** Jedes Paket stammt aus
`pacman -Qoq` auf die CMake-Suchpfade, die den Bau auf Ganymed tatsächlich
befriedigen.

## 3. Mutationsprobe der beiden Wachen

Eine Wache, die nur im Gutfall geprüft wurde, ist nicht geprüft. Beide
Fehlerfälle sind künstlich erzeugt worden; Skript:
[`wachprobe.sh`](wachprobe.sh), Ausgabe: [`wachprobe.txt`](wachprobe.txt).

| Probe | Erwartet | Ergebnis |
|---|---|---|
| Sauberer Bau läuft durch | grün | **OK** |
| Eine Warnung färbt rot | rot | **OK** |
| **Ohne** `pipefail` bliebe ein Baufehler unsichtbar | grün | **OK** |
| **Mit** `pipefail` schlägt der Baufehler durch | rot | **OK** |

Die dritte Zeile ist kein Beiwerk: Sie belegt, dass `set -o pipefail` im
Schritt „Bauen" **tragend** ist. Ohne ihn liefert die Pipe den Rückgabewert von
`tee` — immer 0 —, und ein gescheiterter Bau liefe grün durch. Genau so sieht
ein Absturz aus wie ein ruhiger Tag.

Ebenso tragend ist das `|| true` am `grep -c`: Ohne Treffer liefert `grep` den
Rückgabewert 1, und unter `set -e` bräche der Schritt ausgerechnet im **Gutfall**
ab.

## 4. Was dieser Lauf nicht leistet

Steht so auch im Kopf des Workflows und in `PROZESS.md`:

- **DoD 1** verlangt die Tests **auf Ganymed**. Der Container ist ein zweites
  Netz, keine Verlegung des Prüfstands.
- **DoD 2** (installierter Stand unter `/usr`) wird nicht erreicht.
- **DoD 3** (Bildprüfung) wird nicht erreicht — kein Compositor, und die
  Bildläufer sind `EXCLUDE_FROM_ALL` und werden hier nicht gebaut.

**Wer die grüne Marke für eine erfüllte DoD hält, hat drei ihrer sechs Punkte
übersprungen.**

## 5. Bekannte Kosten und der rollende Paketstrom

Die Paketinstallation zieht über `plasma-integration` das halbe
Plasma-Grundgerüst nach: **419 MiB Download, 1,78 GiB installiert** je Lauf. Das
ist der Preis dafür, dass die Symbolprüfungen echt sind. Ein vorgebautes Abbild
oder ein Paket-Zwischenspeicher wäre die Optimierung, falls die Laufzeit stört —
heute ist sie nicht gemessen worden und deshalb hier auch nicht behauptet.

Der Arch-Container hält **denselben rollenden Paketstrom wie Ganymed**
(CachyOS). Das ist Absicht: Ein Fund hier ist dort reproduzierbar. Es heißt
umgekehrt, dass ein Upstream-Sprung den Lauf rot färben kann, ohne dass jemand
etwas geändert hat. Der Schritt „Werkzeugstand festhalten" existiert genau
dafür — ohne ihn ist ein solcher roter Lauf nicht von einer Codeänderung zu
unterscheiden.

## 6. Offen

- **Der karpathy-Review über diese Prozessänderung ist nicht geführt.** Die
  globale Regel verlangt ihn bei Änderungen an Regel- und Prozessdateien
  (DoD 3). Diese Sitzung durfte keine Agenten aufrufen; der Lauf ist damit
  **offen und nicht etwa entfallen**.
- **Wirksamkeit ist noch nicht belegt.** Dass der Lauf grün ist, sagt nichts
  darüber, ob er je etwas findet. Die Prüffrage steht in
  `docs/scrum/sprints/sprint-06.md`, §13.1.
