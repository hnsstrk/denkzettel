from PIL import Image
import numpy as np
FRAME=24
BASE="/home/hnsstrk/Projekte/denkzettel/docs/scrum/reviews/sprint-06-ux-review/bilder/"
HATCH=[(0xf2,0xf0,0xeb),(0xe9,0xe7,0xe2)]
def check(name):
    a=np.asarray(Image.open(BASE+name).convert("RGB")).astype(int)
    ph,pw=a.shape[:2]; W,H=pw-2*FRAME,ph-2*FRAME
    win=a[FRAME:FRAME+H,FRAME:FRAME+W]
    m=np.zeros((H,W),bool)
    for h in HATCH: m|=(np.abs(win-np.array(h)).max(axis=2)<=1)
    ecken=[]
    for (yy,xx,ry,rx) in [(0,0,1,1),(0,W-1,1,-1),(H-1,0,-1,1),(H-1,W-1,-1,-1)]:
        k=0
        while m[yy, xx+rx*k]: k+=1
        ecken.append(k)
    # Deckung der Kantenmitten und der Fenstermitte
    deckend=[not m[0,W//2], not m[H-1,W//2], not m[H//2,0], not m[H//2,W-1], not m[H//2,W//2]]
    # Zahl der Loecher: nicht-Rand-Pixel, die Hintergrund zeigen, obwohl ringsum Fenster
    innen=m[2:H-2,2:W-2]
    loecher=int(innen.sum())
    print(f"{name}: {W}x{H}  Eckanlauf oben (li,re,ul,ur) {ecken}  "
          f"Kantenmitten+Mitte deckend {deckend}  Hintergrundpixel im Inneren {loecher}")
for n in ["01-rand-schmal-hell-leer.png","03-rand-schmal-hell-acht-zeilen.png",
          "07-rand-breit-hell-leer.png","09-rand-breit-hell-acht-zeilen.png",
          "04-rand-schmal-dunkel-leer.png","06-rand-schmal-dunkel-acht-zeilen.png",
          "10-rand-breit-dunkel-leer.png","12-rand-breit-dunkel-acht-zeilen.png"]:
    check(n)
