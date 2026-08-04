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
    # Loch: Hintergrundpixel, dessen vier Nachbarn alle KEIN Hintergrund sind
    loch=[]
    for y in range(1,H-1):
        for x in range(1,W-1):
            if m[y,x] and not m[y-1,x] and not m[y+1,x] and not m[y,x-1] and not m[y,x+1]:
                loch.append((x,y))
    # zusaetzlich: diagonal eingeschlossene Pixel am Eckbogen zaehlen wir getrennt
    print(f"{name}: echte Löcher (4 Nachbarn deckend): {len(loch)} {loch[:12]}")
for n in ["01-rand-schmal-hell-leer.png","07-rand-breit-hell-leer.png",
          "04-rand-schmal-dunkel-leer.png","10-rand-breit-dunkel-leer.png",
          "03-rand-schmal-hell-acht-zeilen.png","09-rand-breit-hell-acht-zeilen.png"]:
    check(n)
