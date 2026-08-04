from PIL import Image
import numpy as np
FRAME=24
BASE="/home/hnsstrk/Projekte/denkzettel/docs/scrum/reviews/sprint-06-ux-review/bilder/"
def bands(name, inset=14):
    a=np.asarray(Image.open(BASE+name).convert("RGB")).astype(int)
    ph,pw=a.shape[:2]; W,H=pw-2*FRAME,ph-2*FRAME
    win=a[FRAME:FRAME+H,FRAME:FRAME+W]
    surf=win[H//2, W//2]
    dev=np.abs(win-surf).max(axis=2)
    ink=dev>35
    ink[:inset,:]=False; ink[H-inset:,:]=False; ink[:,:inset]=False; ink[:,W-inset:]=False
    rows=np.where(ink.any(axis=1))[0]
    print(f"== {name}  Fenster {W}x{H}  Flaeche {tuple(surf)}")
    if not len(rows): return
    out=[]; s=rows[0]; p=rows[0]
    for r in rows[1:]:
        if r-p>3: out.append((s,p)); s=r
        p=r
    out.append((s,p))
    for (s,e) in out:
        seg=ink[s:e+1]; cols=np.where(seg.any(axis=0))[0]
        print(f"   Band y {s}..{e}  Hoehe {e-s+1}  Tinte x {cols.min()}..{cols.max()}")
    # Flaeche links neben dem Textfeld gegen Flaeche im Textfeld, gleiche Zeile
    y=out[1][0]+2 if len(out)>1 else H//2
    print(f"   Flaechenvergleich in Zeile {y}: links des Feldes x=8 {tuple(win[y,8])}"
          f" | im Feld x={W//2} {tuple(win[y,W//2])} | rechts x={W-9} {tuple(win[y,W-9])}")
for n in ["02-rand-schmal-hell-getippt.png","08-rand-breit-hell-getippt.png","03-rand-schmal-hell-acht-zeilen.png","13-schrift-9pt-fuenf-zeilen.png","14-schrift-24pt-fuenf-zeilen.png"]:
    bands(n); print()
