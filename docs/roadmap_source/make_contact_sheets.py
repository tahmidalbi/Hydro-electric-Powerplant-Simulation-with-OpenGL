from PIL import Image, ImageOps, ImageDraw
from pathlib import Path
base=Path(__file__).resolve().parent.parent/'roadmap_preview'
imgs=sorted(base.glob('page-*.png'))
for start in range(0,len(imgs),6):
    sheet=Image.new('RGB',(1050,1030),'#dce5e8')
    draw=ImageDraw.Draw(sheet)
    for j,p in enumerate(imgs[start:start+6]):
        im=Image.open(p).convert('RGB');im.thumbnail((330,470))
        x=(j%3)*350+10;y=(j//3)*510+23
        sheet.paste(im,(x,y));draw.text((x,y-16),p.stem,fill='black')
    sheet.save(base/f'contact-{start//6+1:02}.png')
print(f'Checked {len(imgs)} rendered page images')
