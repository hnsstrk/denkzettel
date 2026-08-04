from PIL import Image
import numpy as np
FRAME=24
BASE="/home/hnsstrk/Projekte/denkzettel/docs/scrum/reviews/sprint-06-ux-review/bilder/"
def karte(name, surface, contour, n=14):
    a=np.asarray(Image.open(BASE+name).convert("RGB")).astype(int)
    ph,pw=a.shape[:2]; W,H=pw-2*FRAME,ph-2*FRAME
    win=a[FRAME:FRAME+H,FRAME:FRAME+W]
    print(f"== {name}")
    print("   K=Konturfarbe(+/-8)  k=halbwegs Kontur  .=Flaeche(+/-2)  #=Hintergrund  ?=sonst")
    for y in range(n):
        row=""
        for x in range(n):
            p=win[y,x].astype(float)
            dc=np.abs(p-np.array(contour)).max(); ds=np.abs(p-np.array(surface)).max()
            dh=min(np.abs(p-np.array((242,240,235))).max(), np.abs(p-np.array((233,231,226))).max())
            if dc<=8: row+="K"
            elif dh<=2: row+="#"
            elif ds<=2: row+="."
            elif dc<ds and dc<dh: row+="k"
            else: row+="?"
        print("   "+row)
    # Wie lang ist die Unterbrechung? erste Zeile der linken Spalte mit Kontur
    col=[np.abs(win[y,0].astype(float)-np.array(contour)).max()<=8 for y in range(40)]
    rowt=[np.abs(win[0,x].astype(float)-np.array(contour)).max()<=8 for x in range(40)]
    print(f"   linke Kante: Kontur ab Zeile {col.index(True) if True in col else '-'}")
    print(f"   obere Kante: Kontur ab Spalte {rowt.index(True) if True in rowt else '-'}")
mix=lambda a,b,t: tuple(round(a[i]+(b[i]-a[i])*t) for i in range(3))
ls=(0xef,0xf0,0xf1); lt=(0x23,0x26,0x29); ds=(0x20,0x23,0x26); dt=(0xfc,0xfc,0xfc)
karte("01-rand-schmal-hell-leer.png", ls, mix(ls,lt,.2))
karte("07-rand-breit-hell-leer.png", ls, mix(ls,lt,.2))
karte("10-rand-breit-dunkel-leer.png", ds, mix(ds,dt,.2))
karte("04-rand-schmal-dunkel-leer.png", ds, mix(ds,dt,.2))
