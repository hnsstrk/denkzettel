# -*- coding: utf-8 -*-
"""Kontrast der *gedämpften* Kleintexte auf der durchscheinenden Hülle.

Eingaben, alle gemessen:
  - Alpha der Hülle: 216 von 255 (Desktop-Theme `default`, Sonde `deckung`)
  - Farbrollen je Schema aus /usr/share/color-schemes/*.colors

Gerechnet wird, was die AK-Liste von #83 nicht rechnet: nicht `WindowText`
(Notiztext, erscheint erst beim Tippen), sondern `ForegroundInactive` — die
Rolle von App-Name und Fusszeile, und im Ruhezustand der **einzige** Text im
Fenster.
"""
import configparser, glob, os

ALPHA = 216 / 255.0

def lum(c):
    def ch(v):
        v = v / 255.0
        return v / 12.92 if v <= 0.03928 else ((v + 0.055) / 1.055) ** 2.4
    return 0.2126 * ch(c[0]) + 0.7152 * ch(c[1]) + 0.0722 * ch(c[2])

def contrast(a, b):
    la, lb = lum(a), lum(b)
    return (max(la, lb) + 0.05) / (min(la, lb) + 0.05)

def over(front, behind, a):
    return tuple(front[i] * a + behind[i] * (1 - a) for i in range(3))

rows = []
for path in sorted(glob.glob('/usr/share/color-schemes/*.colors')) + sorted(glob.glob(os.path.expanduser('~/.local/share/color-schemes/*.colors'))):
    cp = configparser.ConfigParser(strict=False)
    cp.read(path, encoding='utf8')
    try:
        win = tuple(int(x) for x in cp['Colors:Window']['BackgroundNormal'].split(','))
        fg  = tuple(int(x) for x in cp['Colors:Window']['ForegroundNormal'].split(','))
        inact = tuple(int(x) for x in cp['Colors:Window']['ForegroundInactive'].split(','))
    except Exception:
        continue
    name = os.path.basename(path)[:-7]
    deck_fg = contrast(win, fg)
    deck_in = contrast(win, inact)
    worst_fg = min(contrast(over(win, (255, 255, 255), ALPHA), fg), contrast(over(win, (0, 0, 0), ALPHA), fg))
    worst_in = min(contrast(over(win, (255, 255, 255), ALPHA), inact), contrast(over(win, (0, 0, 0), ALPHA), inact))
    rows.append((name, deck_fg, worst_fg, deck_in, worst_in))

print("Alpha der Hülle: 216/255 = %.1f %% (Desktop-Theme `default`, gemessen)" % (100 * ALPHA))
print()
print("%-26s | %-17s | %-17s" % ("Farbschema", "WindowText", "ForegroundInactive"))
print("%-26s | %8s %8s | %8s %8s" % ("", "deckend", "ungst.", "deckend", "ungst."))
print("-" * 68)
for r in rows:
    mark = ""
    if r[4] < 3.0:
        mark = "  Kleintext < 3:1"
    print("%-26s | %8.2f %8.2f | %8.2f %8.2f%s" % (r[0], r[1], r[2], r[3], r[4], mark))
print()
n = len(rows)
print("Schemata gesamt: %d" % n)
print("WindowText         unter 4,5:1 ungst. : %d" % sum(1 for r in rows if r[2] < 4.5))
print("ForegroundInactive unter 4,5:1 ungst. : %d" % sum(1 for r in rows if r[4] < 4.5))
print("ForegroundInactive unter 3,0:1 ungst. : %d" % sum(1 for r in rows if r[4] < 3.0))
print("ForegroundInactive unter 3,0:1 deckend: %d" % sum(1 for r in rows if r[3] < 3.0))
