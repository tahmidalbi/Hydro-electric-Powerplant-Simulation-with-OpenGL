from pathlib import Path
import json, math, html
from reportlab.pdfgen import canvas
from reportlab.platypus import Paragraph, Table, TableStyle
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib import colors
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.pagesizes import A4

BASE = Path(__file__).resolve().parent
OUT = BASE.parent / 'Confluence_Graphics_Project_Roadmap.pdf'
PAGES = json.loads((BASE / 'roadmap.json').read_text(encoding='utf-8'))
FONT = Path('C:/Windows/Fonts')
for name, file in [('Body','segoeui.ttf'),('Bold','segoeuib.ttf'),('Mono','consola.ttf')]:
    pdfmetrics.registerFont(TTFont(name,str(FONT/file)))
pdfmetrics.registerFontFamily('Body',normal='Body',bold='Bold',italic='Body',boldItalic='Bold')
W,H=A4
M=43
CW=W-2*M
INK=colors.HexColor('#163b45')
TEXT=colors.HexColor('#2c4149')
TEAL=colors.HexColor('#087f8c')
PALE=colors.HexColor('#edf7f7')
GRAY=colors.HexColor('#637a82')
LINE=colors.HexColor('#d4e4e8')

def style(size=10.2, bold=False, leading=None, color=TEXT):
    return ParagraphStyle('x',fontName='Bold' if bold else 'Body',fontSize=size,leading=leading or size*1.42,textColor=color,spaceAfter=0,allowWidows=0,allowOrphans=0)

def para(s,size=10.2,bold=False,color=TEXT):
    return Paragraph(s,style(size,bold,color=color))

def wrap_text(s):
    return html.escape(s).replace('\n','<br/>')

class Diagram:
    def __init__(self,kind): self.kind=kind
    def wrap(self,w,h): return (w,190 if self.kind=='cover' else 137)
    def drawOn(self,c,x,y,w=CW):
        c.saveState();c.translate(x,y)
        if self.kind=='cover': self.cover(c,w)
        elif self.kind=='flow': self.flow(c,w)
        else: self.rays(c,w)
        c.restoreState()
    def box(self,c,x,y,w,h,label,fill=PALE):
        c.setFillColor(fill);c.setStrokeColor(LINE);c.roundRect(x,y,w,h,7,fill=1,stroke=1)
        c.setFillColor(INK);c.setFont('Bold',9);c.drawCentredString(x+w/2,y+h/2-3,label)
    def arrow(self,c,x1,y1,x2,y2,col=TEAL):
        c.setStrokeColor(col);c.setFillColor(col);c.setLineWidth(1.3);c.line(x1,y1,x2,y2)
        a=math.atan2(y2-y1,x2-x1)
        p=c.beginPath();p.moveTo(x2,y2);p.lineTo(x2-7*math.cos(a-.4),y2-7*math.sin(a-.4));p.lineTo(x2-7*math.cos(a+.4),y2-7*math.sin(a+.4));p.close();c.drawPath(p,fill=1,stroke=0)
    def label(self,c,s,x,y,size=8,color=GRAY):
        c.setFillColor(color);c.setFont('Body',size);c.drawString(x,y,s)
    def cover(self,c,w):
        c.setFillColor(PALE);c.roundRect(0,0,w,190,10,fill=1,stroke=0)
        c.setFillColor(colors.HexColor('#d9ecef'));p=c.beginPath();p.moveTo(0,90);p.lineTo(55,150);p.lineTo(105,95);p.lineTo(161,146);p.lineTo(215,88);p.lineTo(w,120);p.lineTo(w,0);p.lineTo(0,0);p.close();c.drawPath(p,fill=1,stroke=0)
        c.setFillColor(colors.HexColor('#b9dce3'));c.rect(175,24,w-188,61,fill=1,stroke=0)
        c.setFillColor(colors.HexColor('#9abcb7'));p=c.beginPath();p.moveTo(0,0);p.lineTo(0,85);p.lineTo(155,92);p.lineTo(222,0);p.close();c.drawPath(p,fill=1,stroke=0)
        for tx,ty,sz in [(63,91,1),(138,102,.72)]:
            c.setStrokeColor(INK);c.setLineWidth(2);c.line(tx,ty,tx,ty+55*sz)
            hx,hy=tx,ty+55*sz
            for a in [90,210,330]:
                a=math.radians(a);c.line(hx,hy,hx+26*sz*math.cos(a),hy+26*sz*math.sin(a))
            c.setFillColor(TEAL);c.circle(hx,hy,3*sz,fill=1,stroke=0)
        c.setFillColor(colors.HexColor('#8fa6af'));c.rect(226,39,140,34,fill=1,stroke=0)
        c.setFillColor(INK);c.rect(252,43,18,22,fill=1,stroke=0);c.rect(284,43,18,22,fill=1,stroke=0)
        c.setStrokeColor(colors.HexColor('#edfafa'));c.setLineWidth(6);c.line(261,42,257,24);c.line(293,42,290,24)
        c.setStrokeColor(TEAL);c.setLineWidth(3);c.circle(365,41,16,fill=0,stroke=1)
        for i in range(8):
            a=i*math.pi/4;c.line(365,41,365+14*math.cos(a),41+14*math.sin(a))
        for r in range(2):
            for k in range(4):
                px,py=32+k*25+r*5,25+r*16
                c.setFillColor(TEAL);p=c.beginPath();p.moveTo(px,py);p.lineTo(px+22,py);p.lineTo(px+27,py+12);p.lineTo(px+5,py+12);p.close();c.drawPath(p,fill=1,stroke=0)
        c.setFillColor(INK);p=c.beginPath();p.moveTo(393,77);p.lineTo(430,77);p.lineTo(424,70);p.lineTo(400,70);p.close();c.drawPath(p,fill=1,stroke=0)
        c.setFillColor(colors.white);c.rect(403,78,14,8,fill=1,stroke=0)
        self.label(c,'Wind',44,173,9,INK);self.label(c,'Solar',41,9,9,INK);self.label(c,'Hydro',292,9,9,INK);self.label(c,'Reservoir + boat',374,98,9,INK)
        self.label(c,'Conceptual scene map',w-129,174,8,GRAY)
    def flow(self,c,w):
        bw=91;gap=(w-5*bw)/4
        for i,s in enumerate(['Input','Scene state','Update','Draw items','Snapshot']):
            self.box(c,i*(bw+gap),89,bw,36,s)
            if i<4:self.arrow(c,i*(bw+gap)+bw+1,107,(i+1)*(bw+gap)-1,107)
        self.box(c,230,26,118,35,'Raster renderer');self.box(c,380,26,125,35,'Ray renderer')
        self.arrow(c,3*(bw+gap)+bw/2,87,288,64);self.arrow(c,4*(bw+gap)+bw/2,87,443,64)
        self.label(c,'Live animation',244,10);self.label(c,'Frozen comparison',390,10)
    def rays(self,c,w):
        c.setFillColor(PALE);c.roundRect(0,0,w,137,8,fill=1,stroke=0)
        self.box(c,17,46,73,30,'Camera');self.box(c,211,39,81,34,'Surface');self.box(c,407,38,79,35,'Object')
        self.arrow(c,91,62,209,57);self.label(c,'camera ray',118,76)
        self.arrow(c,292,57,404,57);self.label(c,'reflection ray',316,75)
        c.setFillColor(colors.HexColor('#e9c276'));c.circle(331,117,10,fill=1,stroke=0)
        self.arrow(c,269,75,319,110,colors.HexColor('#c18d24'));self.label(c,'shadow ray',263,101,8)
        self.label(c,'Find a surface → check light visibility → trace reflected view',20,17,9,INK)

def block_items(block,size):
    typ=block['type']
    if typ=='p': return [(para(block['text'],size),6)]
    if typ=='h': return [(para(block['text'],size+1.6,True,INK),6)]
    if typ=='diagram': return [(Diagram(block['kind']),10)]
    if typ=='code':
        ps=ParagraphStyle('mono',fontName='Mono',fontSize=8.45,leading=11.7,textColor=INK)
        p=Paragraph(wrap_text(block['text']).replace('  ','&nbsp;&nbsp;'),ps)
        t=Table([[p]],colWidths=[CW]);t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,-1),PALE),('BOX',(0,0),(-1,-1),.4,LINE),('LEFTPADDING',(0,0),(-1,-1),12),('RIGHTPADDING',(0,0),(-1,-1),10),('TOPPADDING',(0,0),(-1,-1),10),('BOTTOMPADDING',(0,0),(-1,-1),10)]))
        return [(t,8)]
    if typ=='list':
        result=[]
        for s in block['items']:
            t=Table([[para('•',size,False,TEAL),para(s,size)]],colWidths=[14,CW-14])
            t.setStyle(TableStyle([('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),0),('RIGHTPADDING',(0,0),(-1,-1),0),('TOPPADDING',(0,0),(-1,-1),0),('BOTTOMPADDING',(0,0),(-1,-1),0)]))
            result.append((t,5))
        return result
    if typ=='callout':
        rows=[[para(block['title'],size,True,TEAL)],[para(block['text'],size)]]
        t=Table(rows,colWidths=[CW]);t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,-1),PALE),('LINEBEFORE',(0,0),(0,-1),3,TEAL),('LEFTPADDING',(0,0),(-1,-1),12),('RIGHTPADDING',(0,0),(-1,-1),12),('TOPPADDING',(0,0),(-1,0),8),('BOTTOMPADDING',(0,0),(-1,0),4),('TOPPADDING',(0,1),(-1,1),0),('BOTTOMPADDING',(0,1),(-1,1),9)]))
        return [(t,8)]
    if typ=='table':
        ts=size-(.9 if block.get('compact') else .6)
        rows=[[para(wrap_text(s),ts,True,colors.white) for s in block['headers']]]
        rows.extend([[para(wrap_text(s),ts) for s in row] for row in block['rows']])
        t=Table(rows,colWidths=[CW*f for f in block.get('widths',[1/len(rows[0])]*len(rows[0]))])
        cmds=[('BACKGROUND',(0,0),(-1,0),INK),('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),8),('RIGHTPADDING',(0,0),(-1,-1),8),('TOPPADDING',(0,0),(-1,-1),3 if block.get('compact') else 6),('BOTTOMPADDING',(0,0),(-1,-1),3 if block.get('compact') else 6),('LINEBELOW',(0,0),(-1,0),.4,INK)]
        for ri in range(1,len(rows)):
            cmds += [('BACKGROUND',(0,ri),(-1,ri),PALE if ri%2 else colors.white),('LINEBELOW',(0,ri),(-1,ri),.35,LINE)]
        t.setStyle(TableStyle(cmds));return [(t,9)]
    raise ValueError(typ)

c=canvas.Canvas(str(OUT),pagesize=A4,pageCompression=1)
c.setTitle('Confluence — Graphics Project Roadmap')
c.setAuthor('Prepared for Md. Tahmid Islam')
c.setSubject('Step-by-step plan for a 3D hybrid renewable energy graphics project')
metrics=[]
for i,page in enumerate(PAGES,1):
    c.setFillColor(TEAL);c.rect(0,H-9,W,9,fill=1,stroke=0)
    c.setFont('Bold',9);c.setFillColor(TEAL);c.drawString(M,H-36,'CONFLUENCE  /  PROJECT ROADMAP')
    c.setFont('Body',9);c.setFillColor(GRAY);c.drawRightString(W-M,H-36,'8 OCT 2026')
    y=H-56
    title=para(page['title'],30 if i==1 else 22,True,INK)
    _,ht=title.wrap(CW,1000);title.drawOn(c,M,y-ht);y-=ht+8
    subtitle=para(page['subtitle'],12 if i==1 else 10.5,False,GRAY)
    _,ht=subtitle.wrap(CW,1000);subtitle.drawOn(c,M,y-ht);y-=ht+17
    available=y-58
    chosen=None
    for size in [10.2,9.9,9.6,9.3]:
        items=[]
        for b in page['blocks']:items.extend(block_items(b,size))
        total=sum(obj.wrap(CW,2000)[1]+gap for obj,gap in items)
        if total<=available:chosen=(size,items,total);break
    if chosen is None: raise RuntimeError(f'Page {i} overflows: {total:.1f} > {available:.1f}: {page["title"]}')
    size,items,total=chosen
    for obj,gap in items:
        _,ht=obj.wrap(CW,2000)
        obj.drawOn(c,M,y-ht);y-=ht+gap
    c.setStrokeColor(LINE);c.setLineWidth(.6);c.line(M,43,W-M,43)
    c.setFillColor(GRAY);c.setFont('Body',8);c.drawString(M,28,'Md. Tahmid Islam  •  CSE 4101  •  Implementation plan')
    c.setFont('Bold',8);c.drawRightString(W-M,28,f'{i:02d} / {len(PAGES):02d}')
    c.bookmarkPage(f'page{i}');c.addOutlineEntry(page['title'],f'page{i}',level=0,closed=False)
    metrics.append({'page':i,'title':page['title'],'body_size':size,'bottom_y':round(y,1)})
    c.showPage()
c.save()
(BASE/'layout_metrics.json').write_text(json.dumps(metrics,indent=2),encoding='utf-8')
print(f'Created {OUT}: {len(PAGES)} pages')
print(json.dumps(metrics))

# A readable editable companion, generated from the same content.
md=[]
for i,page in enumerate(PAGES,1):
    md += [f'# {page["title"]}',page['subtitle'],'']
    for b in page['blocks']:
        t=b['type']
        if t=='p': md += [b['text'],'']
        elif t=='h': md += ['## '+b['text'],'']
        elif t=='list': md += ['- '+s for s in b['items']]+['']
        elif t=='code': md += ['~~~text',b['text'],'~~~','']
        elif t=='callout': md += ['**'+b['title']+'** — '+b['text'],'']
        elif t=='table':
            md += ['| '+' | '.join(b['headers'])+' |','| '+' | '.join(['---']*len(b['headers']))+' |']
            md += ['| '+' | '.join(row)+' |' for row in b['rows']]+['']
    md += ['---','']
(BASE.parent/'Confluence_Graphics_Project_Roadmap.md').write_text('\n'.join(md),encoding='utf-8')
