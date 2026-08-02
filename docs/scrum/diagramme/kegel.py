#!/usr/bin/env python3
"""Draws the estimation cone of the Denkzettel project as a plain SVG file.

Reads ``schaetzhistorie.json`` next to this script, writes ``kegel.svg`` next
to it, and prints a short balance to stdout.

Determinism is the point of this tool: the maintainer (``denkzettel-verwalter``)
runs it after every sprint and reports the *diff*.  Same input therefore has to
produce a byte-identical file.  No timestamps, no randomness, no environment or
system queries, no external libraries -- standard library only, and the SVG is
written by hand rather than by a plotting library whose version would leak into
the output.

Usage:  python3 docs/scrum/diagramme/kegel.py
"""

import json
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
DATA = HERE / "schaetzhistorie.json"
OUT = HERE / "kegel.svg"

# --- Colours -----------------------------------------------------------------
# Every colour below was measured against #ffffff AND #000000, because GitHub
# renders the README in both a light and a dark theme and this file carries no
# background of its own.  Graphical elements clear 3:1 both ways, text clears
# 4.5:1 both ways.  That two-sided bound is what keeps the palette muted: a
# colour that stays readable on black cannot also be dark, so nothing here is
# saturated.  Changing a value means measuring it again, both ways.
INK = "#757575"          # text          -- 4.61 : 1 on white, 4.56 : 1 on black
RULE = "#909090"         # grid lines    -- 3.19 : 1 on white, 6.58 : 1 on black
DATA_BLUE = "#2F7FC4"    # regular point -- 4.24 : 1 on white, 4.95 : 1 on black
DATA_RUST = "#B85C2E"    # marked point  -- 4.56 : 1 on white, 4.61 : 1 on black

# --- Layout ------------------------------------------------------------------
W = 920
MARGIN = 48
PLOT_L, PLOT_R = 104, 690
PLOT_T, PLOT_B = 108, 440
LEGEND_X = 716
X0, X3 = PLOT_L + 52, PLOT_R - 30      # centres of the columns for distance 0/3
F_MIN, F_MAX = 1.0 / 2.75, 2.75        # y range, symmetric around 1.0 in log
JITTER = 27.0                          # px between points sharing a coordinate
LINE_H = 18                            # caption line height

# Character width of the caption font at 12.5 px, measured on the rendered
# file rather than guessed: 122 characters occupied 853 px.  Used only to wrap
# the caption, where a few px of slack are harmless -- but a wrong value here
# lets text run off the edge, which is how the first draft failed.
CHAR_W = 7.0

# Reciprocal pairs, so that the log symmetry is visible on the axis itself:
# 0,40 <-> 2,50   0,50 <-> 2,00   0,67 <-> 1,50   0,80 <-> 1,25.
Y_TICKS = [1 / 2.5, 1 / 2.0, 1 / 1.5, 1 / 1.25, 1.0, 1.25, 1.5, 2.0, 2.5]

ANLAESSE = ("gegenstand-geändert", "erkenntnis", "keine")


# --- Helpers -----------------------------------------------------------------

def de(value, digits=2):
    """German decimal notation, fixed width -- no locale, no host dependency."""
    return f"{value:.{digits}f}".replace(".", ",")


def esc(text_):
    return text_.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def num(value):
    """Fixed-precision coordinate, so the output cannot wobble between runs."""
    return f"{value:.2f}"


def x_of(distance):
    return X0 + (X3 - X0) * distance / 3.0


def y_of(factor):
    span = math.log(F_MAX) - math.log(F_MIN)
    rel = (math.log(factor) - math.log(F_MIN)) / span
    return PLOT_B - rel * (PLOT_B - PLOT_T)


def wrap(paragraph, width_px):
    """Greedy word wrap on an estimated character width."""
    limit = int(width_px / CHAR_W)
    lines, current = [], ""
    for word in paragraph.split():
        candidate = f"{current} {word}".strip()
        if len(candidate) > limit and current:
            lines.append(current)
            current = word
        else:
            current = candidate
    if current:
        lines.append(current)
    return lines


# --- Data --------------------------------------------------------------------

def load():
    """Read the series and check it against itself before drawing anything.

    The factor is stored twice on purpose: as printed in the sprint protocol,
    and derivable from first and final estimate.  Recomputing it here turns a
    transcription slip into an abort instead of into a plausible wrong picture.
    """
    with DATA.open(encoding="utf-8") as handle:
        raw = json.load(handle)

    for story in raw["stories"]:
        first = story["erstschaetzung"]["wert"]
        computed = story["endwert"] / first
        printed = story["faktor_protokoll"]
        if abs(computed - printed) > 0.01:
            raise SystemExit(
                f"#{story['issue']}: Faktor im Protokoll {de(printed)}, "
                f"aus Erst- und Endwert gerechnet {de(computed)} — "
                f"die Datenreihe widerspricht sich."
            )
        if story["anlass"] not in ANLAESSE:
            raise SystemExit(
                f"#{story['issue']}: unbekanntes Anlass-Kennzeichen "
                f"{story['anlass']!r}."
            )
        if story["in_kurve"] and story["abstand_sprints"] is None:
            raise SystemExit(
                f"#{story['issue']}: steht in der Kurve, hat aber keinen "
                f"Abstand — ohne Abstand ist der Punkt nicht platzierbar."
            )
        if not story["in_kurve"] and not story["ausschlussgrund"]:
            raise SystemExit(
                f"#{story['issue']}: aus der Kurve genommen ohne Grund."
            )
        story["faktor"] = computed

    drawn = [s for s in raw["stories"] if s["in_kurve"]]
    left_out = [s for s in raw["stories"] if not s["in_kurve"]]
    # Explicit sort: the offset below depends on the order, so the order must
    # not depend on how the file happens to be written.
    drawn.sort(key=lambda s: (s["abstand_sprints"], s["faktor"], s["issue"]))
    return raw, drawn, left_out


def envelope(drawn):
    """Extreme values per distance -- the hull, as far as it is measured."""
    hull = {}
    for story in drawn:
        distance = story["abstand_sprints"]
        low, high = hull.get(distance, (story["faktor"], story["faktor"]))
        hull[distance] = (min(low, story["faktor"]), max(high, story["faktor"]))
    return dict(sorted(hull.items()))


def placed(drawn):
    """Give each point its pixel position, nudging coincident ones apart.

    Four stories sit on factor 1,0 in pairs; without an offset the picture
    would show fewer points than the series has.  The offset comes from the
    sorted order, not from a random generator.
    """
    groups = {}
    for story in drawn:
        groups.setdefault(
            (story["abstand_sprints"], round(story["faktor"], 4)), []
        ).append(story)

    spots = []
    for key in sorted(groups):
        members = groups[key]
        for index, story in enumerate(members):
            shift = (index - (len(members) - 1) / 2.0) * JITTER
            spots.append([story, x_of(key[0]) + shift, y_of(story["faktor"])])

    # A label above the marker would land on the neighbour above when two
    # points share a column and sit close in log space (#8 and #57 do).  Those
    # labels go to the right instead.
    crowded = [
        any(
            other[0] is not story
            and abs(other[1] - x) < 24
            and 0 < y - other[2] < 26
            for other in spots
        )
        for story, x, y in spots
    ]
    return [
        (story, x, y, "rechts" if flag else "oben")
        for (story, x, y), flag in zip(spots, crowded)
    ]


# --- Caption -----------------------------------------------------------------

def caption(drawn, left_out, hull):
    """Build the text under the picture from the data, not from memory.

    The sentence about the hull is the one claim that would quietly become a
    lie if somebody added rows later, so it is computed rather than typed.
    """
    spans = {d: math.log(hi) - math.log(lo) for d, (lo, hi) in hull.items()}
    keys = sorted(spans)
    ordered = [spans[d] for d in keys]

    non_falling = all(b >= a - 1e-9 for a, b in zip(ordered, ordered[1:]))
    strict = all(b > a + 1e-9 for a, b in zip(ordered, ordered[1:]))
    if strict:
        shape = "streng wachsend"
    elif non_falling:
        shape = "nicht-fallend, aber nicht streng wachsend"
    else:
        shape = "nicht durchgehend nicht-fallend"

    widening = [
        keys[i] for i in range(1, len(keys))
        if ordered[i] > ordered[i - 1] + 1e-9
    ]
    where = (
        f"die Weitung tritt erst bei Abstand {widening[0]} ein"
        if widening else "eine Weitung ist nirgends gemessen"
    )

    # Does a marked point carry an outer edge of the hull? If so, say so.
    edge = ""
    for distance in keys:
        column = [s for s in drawn if s["abstand_sprints"] == distance]
        top = max(column, key=lambda s: s["faktor"])
        if top["anlass"] == "gegenstand-geändert":
            rest = ("einem einzigen Punkt" if len(column) <= 2
                    else f"{len(column)} Punkten")
            edge = (
                f" Der obere Rand bei Abstand {distance} ruht auf {rest}, und "
                f"#{top['issue']} ist als „gegenstand-geändert“ "
                f"gekennzeichnet: der größte Faktor der Reihe ist zugleich "
                f"der, der am wenigsten über Schätzgenauigkeit sagt."
            )
            break

    paragraphs = [
        # This sentence is required by PROZESS.md, Sprint-Abschluss Punkt 12.
        # It is a condition of the picture, not an ornament -- do not trim it.
        "Der Faktor misst, wie stark eine Schätzung revidiert wurde — nicht, "
        "wie weit sie vom tatsächlichen Aufwand lag; dieser wird im Projekt "
        "nicht erhoben. Eine Story, die niemand neu geschätzt hat, steht bei "
        "1,0, auch wenn sie teurer war.",

        f"Belegdichte: {len(drawn)} Punkte. Die Hüllkurve ist {shape} — im "
        f"Logarithmus messen die Abstände "
        f"{' · '.join(de(v) for v in ordered)}; {where}.{edge} Ein Kegel im "
        f"Sinne der Lehrbuchfigur wäre erst mit mehr Punkten je Abstand "
        f"belegt.",

        f"Nicht gezeichnet: {len(left_out)} Stories, zwischen deren "
        f"Erstschätzung und Umsetzung keine Gelegenheit zur Revision lag. Ihr "
        f"Faktor ist 1,0 von Konstruktion wegen und keine Messung; sie stehen "
        f"in schaetzhistorie.json, damit die Auslassung sichtbar bleibt.",
    ]

    lines = []
    for index, paragraph in enumerate(paragraphs):
        if index:
            lines.append("")
        lines.extend(wrap(paragraph, W - 2 * MARGIN))
    return lines


# --- Drawing -----------------------------------------------------------------

def text(x, y, content, size=13, colour=INK, anchor="start", weight="normal"):
    return (
        f'<text x="{num(x)}" y="{num(y)}" font-size="{size}" fill="{colour}" '
        f'text-anchor="{anchor}" font-weight="{weight}">{esc(content)}</text>'
    )


def turned(x, y, content, size=13):
    """Text rotated a quarter turn, anchored at its own middle."""
    return (
        f'<text transform="translate({num(x)} {num(y)}) rotate(-90)" '
        f'font-size="{size}" fill="{INK}" text-anchor="middle">'
        f'{esc(content)}</text>'
    )


def draw(stand, drawn, left_out, hull):
    lines = caption(drawn, left_out, hull)
    caption_top = PLOT_B + 108
    height = caption_top + len(lines) * LINE_H + 14
    hull_keys = sorted(hull)

    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {height}" '
        f'width="{W}" height="{height}" font-family="DejaVu Sans, Segoe UI, '
        f'Helvetica, Arial, sans-serif">',
        "<title>Schätzkegel des Projekts Denkzettel — Revisionsfaktor über "
        "dem Abstand zwischen Erstschätzung und Umsetzung</title>",
        # No background rectangle on purpose: the picture sits on the README's
        # own background, light or dark.
    ]

    parts.append(text(MARGIN, 44, "Schätzkegel — Revisionsfaktor über dem "
                                  "Abstand zur Umsetzung", size=21,
                      weight="bold"))
    # Stand comes from the series, the counts are computed: neither may be
    # typed here, or the subtitle keeps claiming Sprint 5 forever.
    parts.append(text(
        MARGIN, 68,
        f"Denkzettel · Stand {stand} · {len(drawn)} Punkte in der Kurve, "
        f"{len(left_out)} erfasst und nicht gezeichnet · "
        f"Quelle: Sprint-Protokoll §24", size=12.5))

    # --- grid and y axis
    for tick in Y_TICKS:
        y = y_of(tick)
        if abs(tick - 1.0) < 1e-9:
            parts.append(
                f'<line x1="{num(PLOT_L)}" y1="{num(y)}" x2="{num(PLOT_R)}" '
                f'y2="{num(y)}" stroke="{INK}" stroke-width="1.6" />'
            )
            parts.append(text(PLOT_L - 12, y + 4, de(tick), size=12,
                              anchor="end", weight="bold"))
        else:
            parts.append(
                f'<line x1="{num(PLOT_L)}" y1="{num(y)}" x2="{num(PLOT_R)}" '
                f'y2="{num(y)}" stroke="{RULE}" stroke-width="0.8" '
                f'stroke-dasharray="2 4" />'
            )
            parts.append(text(PLOT_L - 12, y + 4, de(tick), size=12,
                              anchor="end"))
    # Sits in the empty distance-0 column, where nothing can collide with it.
    parts.append(text(x_of(0) + 14, y_of(1.0) - 9, "1,0 — nicht revidiert",
                      size=11.5))

    parts.append(turned(34, (PLOT_T + PLOT_B) / 2,
                        "Revisionsfaktor: Endwert ÷ Erstwert"))
    parts.append(turned(52, (PLOT_T + PLOT_B) / 2,
                        "logarithmisch — 0,60 und 1,67 stehen gleich weit "
                        "von 1,0", size=11))

    # --- x axis
    parts.append(
        f'<line x1="{num(PLOT_L)}" y1="{num(PLOT_B)}" x2="{num(PLOT_R)}" '
        f'y2="{num(PLOT_B)}" stroke="{INK}" stroke-width="1.2" />'
    )
    for distance in (0, 1, 2, 3):
        x = x_of(distance)
        parts.append(
            f'<line x1="{num(x)}" y1="{num(PLOT_B)}" x2="{num(x)}" '
            f'y2="{num(PLOT_B + 6)}" stroke="{INK}" stroke-width="1.2" />'
        )
        parts.append(text(x, PLOT_B + 24, str(distance), size=13,
                          anchor="middle"))
        if distance not in hull_keys:
            parts.append(text(x, PLOT_B + 42, "kein Datenpunkt", size=11,
                              anchor="middle"))
    parts.append(text((PLOT_L + PLOT_R) / 2, PLOT_B + 68,
                      "Abstand in Sprints zwischen Erstschätzung und "
                      "Umsetzung", size=13, anchor="middle"))

    # --- hull.  The vertices are measured, the connection between them is
    # not, so the outline is dashed and the measured extremes carry caps.
    upper = [(x_of(d), y_of(hull[d][1])) for d in hull_keys]
    lower = [(x_of(d), y_of(hull[d][0])) for d in reversed(hull_keys)]
    ring = " ".join(f"{num(x)},{num(y)}" for x, y in upper + lower)
    parts.append(
        f'<polygon points="{ring}" fill="{DATA_BLUE}" fill-opacity="0.13" '
        f'stroke="{INK}" stroke-width="1.4" stroke-dasharray="7 5" />'
    )
    for distance in hull_keys:
        for factor in hull[distance]:
            x, y = x_of(distance), y_of(factor)
            parts.append(
                f'<line x1="{num(x - 11)}" y1="{num(y)}" x2="{num(x + 11)}" '
                f'y2="{num(y)}" stroke="{INK}" stroke-width="2.2" />'
            )

    # --- points
    for story, x, y, where in placed(drawn):
        marked = story["anlass"] == "gegenstand-geändert"
        colour = DATA_RUST if marked else DATA_BLUE
        if marked:
            parts.append(
                f'<polygon points="{num(x)},{num(y - 8)} {num(x + 8)},{num(y)} '
                f'{num(x)},{num(y + 8)} {num(x - 8)},{num(y)}" fill="{colour}" '
                f'stroke="{colour}" stroke-width="1.6" />'
            )
        elif story["anlass"] == "erkenntnis":
            parts.append(
                f'<circle cx="{num(x)}" cy="{num(y)}" r="6.2" fill="{colour}" />'
            )
        else:
            parts.append(
                f'<circle cx="{num(x)}" cy="{num(y)}" r="6.2" fill="none" '
                f'stroke="{colour}" stroke-width="2.2" />'
            )
        label = f"#{story['issue']}"
        if where == "rechts":
            parts.append(text(x + 12, y + 4.5, label, size=11.5,
                              colour=colour, weight="bold"))
        else:
            parts.append(text(x, y - 13, label, size=11.5, colour=colour,
                              anchor="middle", weight="bold"))

    # --- legend
    ly = PLOT_T + 6
    parts.append(text(LEGEND_X, ly, "Anlass der Revision", size=12.5,
                      weight="bold"))
    ly += 24
    parts.append(
        f'<circle cx="{num(LEGEND_X + 8)}" cy="{num(ly - 4)}" r="6.2" '
        f'fill="none" stroke="{DATA_BLUE}" stroke-width="2.2" />'
    )
    parts.append(text(LEGEND_X + 26, ly, "keine — nicht revidiert", size=12))
    ly += 26
    parts.append(
        f'<circle cx="{num(LEGEND_X + 8)}" cy="{num(ly - 4)}" r="6.2" '
        f'fill="{DATA_BLUE}" />'
    )
    parts.append(text(LEGEND_X + 26, ly, "erkenntnis — Gegenstand", size=12))
    parts.append(text(LEGEND_X + 26, ly + 16, "blieb, Wert verschoben",
                      size=12))
    ly += 42
    parts.append(
        f'<polygon points="{num(LEGEND_X + 8)},{num(ly - 12)} '
        f'{num(LEGEND_X + 16)},{num(ly - 4)} {num(LEGEND_X + 8)},{num(ly + 4)} '
        f'{num(LEGEND_X)},{num(ly - 4)}" fill="{DATA_RUST}" />'
    )
    parts.append(text(LEGEND_X + 26, ly, "gegenstand-geändert —", size=12,
                      colour=DATA_RUST))
    parts.append(text(LEGEND_X + 26, ly + 16, "keine Schätzabweichung",
                      size=12, colour=DATA_RUST))
    ly += 52
    parts.append(text(LEGEND_X, ly, "Hüllkurve", size=12.5, weight="bold"))
    ly += 24
    parts.append(
        f'<line x1="{num(LEGEND_X)}" y1="{num(ly - 4)}" '
        f'x2="{num(LEGEND_X + 16)}" y2="{num(ly - 4)}" stroke="{INK}" '
        f'stroke-width="2.2" />'
    )
    parts.append(text(LEGEND_X + 26, ly, "Extremwert, gemessen", size=12))
    ly += 24
    parts.append(
        f'<line x1="{num(LEGEND_X)}" y1="{num(ly - 4)}" '
        f'x2="{num(LEGEND_X + 16)}" y2="{num(ly - 4)}" stroke="{INK}" '
        f'stroke-width="1.4" stroke-dasharray="7 5" />'
    )
    parts.append(text(LEGEND_X + 26, ly, "Verbindung, nicht", size=12))
    parts.append(text(LEGEND_X + 26, ly + 16, "gemessen", size=12))

    # --- caption
    y = caption_top
    for line in lines:
        if line:
            parts.append(text(MARGIN, y, line, size=12.5))
        y += LINE_H

    parts.append("</svg>")
    return "\n".join(parts) + "\n"


# --- Balance -----------------------------------------------------------------

def main():
    raw, drawn, left_out = load()
    hull = envelope(drawn)
    OUT.write_text(draw(raw["stand"], drawn, left_out, hull), encoding="utf-8")

    print(f"Quelle     : {DATA.name} — {raw['_quelle']}")
    print(f"Stand      : {raw['stand']}")
    print(f"Geschrieben: {OUT}")
    print()
    print(f"Punkte in der Kurve      : {len(drawn)}")
    print(f"Erfasst, nicht gezeichnet: {len(left_out)}")
    print()
    print("Hüllwerte je Abstand (Sprints):")
    for distance in (0, 1, 2, 3):
        if distance not in hull:
            print(f"  Abstand {distance}: kein Datenpunkt")
            continue
        low, high = hull[distance]
        column = [s for s in drawn if s["abstand_sprints"] == distance]
        span = math.log(high) - math.log(low)
        print(f"  Abstand {distance}: [{de(low)}; {de(high)}] — "
              f"{len(column)} Punkte, log-Weite {de(span)}")
    print()
    marked = [s["issue"] for s in drawn
              if s["anlass"] == "gegenstand-geändert"]
    print("Gezeichnet und markiert (gegenstand-geändert): "
          + (", ".join(f"#{i}" for i in marked) if marked else "keine"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
