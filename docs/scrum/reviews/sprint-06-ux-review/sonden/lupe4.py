from PIL import Image, ImageDraw
FRAME=24; SIZE=18; ZOOM=12
BASE="/home/hnsstrk/Projekte/denkzettel/docs/scrum/reviews/sprint-06-ux-review/bilder/"
panels=[("01-rand-schmal-hell-leer.png","schmal / hell"),
        ("07-rand-breit-hell-leer.png","breit / hell"),
        ("04-rand-schmal-dunkel-leer.png","schmal / dunkel"),
        ("10-rand-breit-dunkel-leer.png","breit / dunkel")]
tiles=[]
for name,label in panels:
    im=Image.open(BASE+name).convert("RGB")
    crop=im.crop((FRAME-3,FRAME-3,FRAME-3+SIZE,FRAME-3+SIZE)).resize((SIZE*ZOOM,SIZE*ZOOM),Image.NEAREST)
    tiles.append((crop,label))
pad,gap,texth=14,18,18
tw=tiles[0][0].width
w=pad*2+tw*4+gap*3; h=pad*2+tiles[0][0].height+texth
sheet=Image.new("RGB",(w,h),(255,255,255)); d=ImageDraw.Draw(sheet)
x=pad
for tile,label in tiles:
    sheet.paste(tile,(x,pad))
    d.rectangle([x-1,pad-1,x+tw,pad+tile.height],outline=(120,120,120))
    d.text((x,pad+tile.height+4),label,fill=(50,50,50))
    x+=tw+gap
sheet.save(BASE+"19-eckenlupe-12-fach-vier-faelle.png")
print(sheet.size)
