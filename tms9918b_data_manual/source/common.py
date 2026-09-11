# TMS9918B documentation generator
#
# Copyright 2026 Saverio Russo
# SPDX-License-Identifier: Apache-2.0
# The generated documents are licensed under CC BY 4.0 (LICENSE-DOCS).

from reportlab.lib.pagesizes import letter
from reportlab.lib.units import inch
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_CENTER, TA_LEFT, TA_JUSTIFY
from reportlab.platypus import (BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, Table, TableStyle,
                                PageBreak, KeepTogether, Flowable, NextPageTemplate, CondPageBreak)
from reportlab.platypus.tableofcontents import TableOfContents
from reportlab.graphics.shapes import Drawing, Line, Rect, String, PolyLine

PAGE_W, PAGE_H = letter
LM = RM = 0.85 * inch
TM = 0.8 * inch
BM = 0.8 * inch

base = dict(fontName='Helvetica', fontSize=9.2, leading=12)
ST = {
    'body': ParagraphStyle('body', alignment=TA_JUSTIFY, spaceAfter=6, **base),
    'bodyl': ParagraphStyle('bodyl', alignment=TA_LEFT, spaceAfter=4, **base),
    'bullet': ParagraphStyle('bullet', leftIndent=14, bulletIndent=4, spaceAfter=2, **base),
    'h1': ParagraphStyle('h1', fontName='Helvetica-Bold', fontSize=12.5, leading=16, spaceBefore=4, spaceAfter=10, keepWithNext=1),
    'h2': ParagraphStyle('h2', fontName='Helvetica-Bold', fontSize=10.5, leading=14, spaceBefore=10, spaceAfter=5, keepWithNext=1),
    'h3': ParagraphStyle('h3', fontName='Helvetica-Bold', fontSize=9.5, leading=13, spaceBefore=8, spaceAfter=4, keepWithNext=1),
    'extitle': ParagraphStyle('extitle', fontName='Helvetica', fontSize=9.2, leading=12, spaceAfter=4, keepWithNext=1),
    'caption': ParagraphStyle('caption', fontName='Helvetica-Bold', fontSize=8.2, leading=11, alignment=TA_CENTER, spaceBefore=4, spaceAfter=10),
    'tcaption': ParagraphStyle('tcaption', fontName='Helvetica-Bold', fontSize=8.2, leading=11, alignment=TA_CENTER, spaceBefore=6, spaceAfter=4),
    'cell': ParagraphStyle('cell', fontName='Helvetica', fontSize=7.8, leading=9.6),
    'cellc': ParagraphStyle('cellc', fontName='Helvetica', fontSize=7.8, leading=9.6, alignment=TA_CENTER),
    'cellb': ParagraphStyle('cellb', fontName='Helvetica-Bold', fontSize=7.8, leading=9.6, alignment=TA_CENTER),
    'note': ParagraphStyle('note', fontName='Helvetica', fontSize=8.4, leading=11, leftIndent=40, rightIndent=40, spaceAfter=8),
    'notehead': ParagraphStyle('notehead', fontName='Helvetica-Bold', fontSize=8.8, leading=11, alignment=TA_CENTER, spaceBefore=6, spaceAfter=2),
    'toc0': ParagraphStyle('toc0', fontName='Helvetica-Bold', fontSize=9, leading=13, leftIndent=0),
    'toc1': ParagraphStyle('toc1', fontName='Helvetica', fontSize=8.8, leading=12, leftIndent=18),
    'toc2': ParagraphStyle('toc2', fontName='Helvetica', fontSize=8.4, leading=11, leftIndent=36),
    'mono': ParagraphStyle('mono', fontName='Courier', fontSize=8.2, leading=10.4, leftIndent=20, spaceAfter=6),
}


class SectionMarker(Flowable):
    """Starts a numbered section: resets the per-section page counter used in the footer."""
    def __init__(self, label):
        super().__init__()
        self.label = label
        self.width = self.height = 0

    def draw(self):
        c = self.canv
        c._sec_label = self.label
        c._sec_start = c.getPageNumber()


class Heading(Paragraph):
    def __init__(self, text, level, style):
        super().__init__(text, style)
        self.toc_level = level
        self.toc_text = text


def H(level, text):
    return Heading(text, level, ST['h1' if level == 0 else 'h2' if level == 1 else 'h3'])


def P(text, style='body'):
    return Paragraph(text, ST[style])


def B(items):
    return [Paragraph(t, ST['bullet'], bulletText='\u2022') for t in items]


def table(data, widths, header_rows=1, align_center_cols=(), font=7.8, zebra=False):
    rows = []
    for r, row in enumerate(data):
        cells = []
        for ci, v in enumerate(row):
            if isinstance(v, Flowable):
                cells.append(v)
            else:
                st = ST['cellb'] if r < header_rows else (ST['cellc'] if ci in align_center_cols else ST['cell'])
                cells.append(Paragraph(str(v), st))
        rows.append(cells)
    t = Table(rows, colWidths=widths, repeatRows=header_rows)
    t._md_data = [[v if isinstance(v, str) else '' for v in row] for row in data]
    t._md_header_rows = header_rows
    style = [
        ('GRID', (0, 0), (-1, -1), 0.6, colors.black),
        ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
        ('TOPPADDING', (0, 0), (-1, -1), 2.2),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 2.2),
        ('LEFTPADDING', (0, 0), (-1, -1), 3),
        ('RIGHTPADDING', (0, 0), (-1, -1), 3),
        ('LINEBELOW', (0, header_rows - 1), (-1, header_rows - 1), 1.1, colors.black),
    ]
    if header_rows:
        style.append(('BACKGROUND', (0, 0), (-1, header_rows - 1), colors.Color(0.9, 0.9, 0.9)))
    t.setStyle(TableStyle(style))
    return t


def note(text, head='NOTE'):
    return KeepTogether([Paragraph(head, ST['notehead']), Paragraph(text, ST['note'])])


def caption(text):
    return Paragraph(text, ST['caption'])


def tcaption(text):
    return Paragraph(text, ST['tcaption'])


def register_figure(name, cells, widths=None):
    """cells: list of (label, span) left (MSB, D0) to right (LSB, D7)."""
    head = [Paragraph('<b>%s</b>' % name, ST['cell'])]
    bits = []
    labels = []
    spans = []
    col = 0
    for label, span in cells:
        labels.append(Paragraph(label, ST['cellc']))
        labels.extend([''] * (span - 1))
        if span > 1:
            spans.append(('SPAN', (col + 1, 1), (col + span, 1)))
        col += span
    bitrow = [Paragraph('', ST['cell'])] + [Paragraph('D%d' % i, ST['cellc']) for i in range(8)]
    labrow = [Paragraph('', ST['cell'])] + labels
    data = [bitrow, labrow]
    data[0][0] = Paragraph('<b>%s</b>' % name, ST['cell'])
    w = widths or [1.15 * inch] + [0.62 * inch] * 8
    t = Table(data, colWidths=w)
    t._md_register = (name, cells)
    st = [('GRID', (1, 1), (-1, 1), 0.8, colors.black), ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
          ('TOPPADDING', (0, 0), (-1, -1), 2), ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
          ('SPAN', (0, 0), (0, 1))] + spans
    t.setStyle(TableStyle(st))
    return t


class Doc(BaseDocTemplate):
    def __init__(self, filename, title, header, **kw):
        super().__init__(filename, pagesize=letter, leftMargin=LM, rightMargin=RM, topMargin=TM, bottomMargin=BM,
                         title=title, author='Team B Europe', **kw)
        self.header_text = header
        frame = Frame(LM, BM, PAGE_W - LM - RM, PAGE_H - TM - BM, id='normal', leftPadding=0, rightPadding=0,
                      topPadding=0, bottomPadding=0)
        self.addPageTemplates([
            PageTemplate(id='cover', frames=[frame], onPageEnd=self._cover_page),
            PageTemplate(id='front', frames=[frame], onPageEnd=self._front_page),
            PageTemplate(id='body', frames=[frame], onPageEnd=self._body_page),
        ])
        self._front_counter = 0
        self.toc_entries = []

    def _cover_page(self, canv, doc):
        pass

    def _front_page(self, canv, doc):
        roman = ['i', 'ii', 'iii', 'iv', 'v', 'vi', 'vii', 'viii']
        idx = canv.getPageNumber() - 2
        canv.setFont('Helvetica', 8.5)
        canv.drawCentredString(PAGE_W / 2, 0.5 * inch, roman[idx] if 0 <= idx < len(roman) else '')

    def _body_page(self, canv, doc):
        canv.setFont('Helvetica-Bold', 8)
        canv.drawRightString(PAGE_W - RM, PAGE_H - 0.55 * inch, self.header_text)
        canv.setLineWidth(0.6)
        canv.line(LM, PAGE_H - 0.6 * inch, PAGE_W - RM, PAGE_H - 0.6 * inch)
        label = getattr(canv, '_sec_label', '')
        start = getattr(canv, '_sec_start', canv.getPageNumber())
        num = '%s-%d' % (label, canv.getPageNumber() - start + 1)
        canv.setFont('Helvetica', 8.5)
        if canv.getPageNumber() % 2 == 0:
            canv.drawString(LM, 0.5 * inch, num)
        else:
            canv.drawRightString(PAGE_W - RM, 0.5 * inch, num)
        canv.setFont('Helvetica', 6.5)
        canv.drawCentredString(PAGE_W / 2, 0.5 * inch, 'ADVANCE INFORMATION - design study, not a Texas Instruments product')

    def afterFlowable(self, flowable):
        if isinstance(flowable, Heading):
            c = self.canv
            label = getattr(c, '_sec_label', '')
            start = getattr(c, '_sec_start', c.getPageNumber())
            page = '%s-%d' % (label, c.getPageNumber() - start + 1)
            self.toc_entries.append((flowable.toc_level, flowable.toc_text, page))


def static_toc(entries, width):
    rows = []
    for level, text, page in entries:
        if level > 1:
            continue
        st = ST['toc0'] if level == 0 else ST['toc1']
        rows.append([Paragraph(text, st), Paragraph(page, ST['toc0'] if level == 0 else ST['toc1'])])
    if not rows:
        rows = [[Paragraph('', ST['toc1']), Paragraph('', ST['toc1'])]]
    t = Table(rows, colWidths=[width - 0.7 * inch, 0.7 * inch])
    t.setStyle(TableStyle([('ALIGN', (1, 0), (1, -1), 'RIGHT'), ('TOPPADDING', (0, 0), (-1, -1), 0.5),
                           ('BOTTOMPADDING', (0, 0), (-1, -1), 0.5), ('VALIGN', (0, 0), (-1, -1), 'TOP')]))
    return t
