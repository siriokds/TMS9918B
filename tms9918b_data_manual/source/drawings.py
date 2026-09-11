# TMS9918B documentation generator
#
# Copyright 2026 Saverio Russo
# SPDX-License-Identifier: Apache-2.0
# The generated documents are licensed under CC BY 4.0 (LICENSE-DOCS).

from reportlab.graphics.shapes import Drawing, Line, Rect, String, PolyLine
from reportlab.lib import colors
from reportlab.lib.units import inch


def waveform(bytes_=1, write=False, width=6.3 * inch):
    """RAS/CAS/address/RD waveform of an n-byte TMS9918B memory cycle, in half-period units."""
    n = bytes_
    total = 5 * n + 2 + 1          # one unit of the next cycle
    left = 62
    usable = width - left - 10
    ux = usable / total
    rows = ['RAS', 'CAS', 'AD (address)', 'RD (data in)']
    if write:
        rows = ['RAS', 'CAS', 'AD (addr/data)', 'R/W']
    rh = 26
    h = rh * len(rows) + 34
    d = Drawing(width, h)
    X = lambda u: left + u * ux
    top = h - 14

    def y(i, level):
        base = top - (i + 1) * rh
        return base + (14 if level else 4)

    # grid
    for u in range(total + 1):
        d.add(Line(X(u), 12, X(u), top - 2, strokeColor=colors.Color(0.82, 0.82, 0.82), strokeWidth=0.4))
        d.add(String(X(u), 2, str(u), fontName='Helvetica', fontSize=6, textAnchor='middle'))
    for i, name in enumerate(rows):
        d.add(String(2, y(i, 0) + 3, name, fontName='Helvetica-Bold', fontSize=7))

    def trace(i, points):
        pts = []
        for u, lvl in points:
            pts += [X(u), y(i, lvl)]
        d.add(PolyLine(pts, strokeColor=colors.black, strokeWidth=1.1))

    ras_rise = 5 * n - 1
    trace(0, [(0, 1), (0, 0), (ras_rise, 0), (ras_rise, 1), (5 * n + 2, 1), (5 * n + 2, 0), (total, 0)])
    pts = [(0, 1)]
    for k in range(n):
        pts += [(1 + 5 * k, 1), (1 + 5 * k, 0), (4 + 5 * k, 0), (4 + 5 * k, 1)]
    pts += [(5 * n + 3, 1)]
    trace(1, [(p, l) for p, l in pts if p <= total])
    # address bus boxes
    base = top - 3 * rh
    def box(u0, u1, label, i):
        yb = top - (i + 1) * rh
        d.add(Rect(X(u0) + 1, yb + 3, X(u1) - X(u0) - 2, 12, strokeColor=colors.black, fillColor=None, strokeWidth=0.8))
        d.add(String((X(u0) + X(u1)) / 2, yb + 6, label, fontName='Helvetica', fontSize=6, textAnchor='middle'))
    box(0, 1, 'row', 2)
    if write:
        box(1, 1.6, 'col', 2)
        box(1.6, 4, 'data', 2)
        trace(3, [(0, 1), (2, 1), (2, 0), (4, 0), (4, 1), (total, 1)])
    else:
        for k in range(n):
            box(1 + 5 * k - (0 if k == 0 else 1), 4 + 5 * k, 'col %d' % k if n > 1 else 'column', 2)
    if write:
        pass
    else:
        for k in range(n):
            u = 4 + 5 * k
            d.add(Line(X(u), top - 4 * rh + 2, X(u), top - 3 * rh - 2, strokeColor=colors.black, strokeWidth=1.4))
            d.add(String(X(u) + 2, top - 4 * rh + 6, 'byte %d' % k if n > 1 else 'sample', fontName='Helvetica', fontSize=6))
    return d


def calendar_bar(segments, title_units, width=6.3 * inch, height=46):
    """segments: list of (start, length, label, shade)."""
    d = Drawing(width, height + 14)
    left, usable = 4, width - 8
    ux = usable / title_units
    for s, l, label, shade in segments:
        g = 1 - shade
        d.add(Rect(left + s * ux, 16, l * ux, height - 18, fillColor=colors.Color(g, g, g), strokeColor=colors.black, strokeWidth=0.7))
        d.add(String(left + (s + l / 2) * ux, 16 + (height - 18) / 2 - 3, label, fontName='Helvetica', fontSize=6, textAnchor='middle'))
    for u in range(0, title_units + 1, 4 if title_units <= 64 else 8):
        d.add(Line(left + u * ux, 12, left + u * ux, 16, strokeColor=colors.black, strokeWidth=0.5))
        d.add(String(left + u * ux, 3, '+%d' % u, fontName='Helvetica', fontSize=5.5, textAnchor='middle'))
    return d


def pinout(left_pins, right_pins, title, width=3.1 * inch):
    n = len(left_pins)
    pitch = 13
    h = n * pitch + 36
    d = Drawing(width, h)
    bx, bw = width / 2 - 26, 52
    top = h - 20
    d.add(Rect(bx, top - n * pitch - 4, bw, n * pitch + 8, strokeColor=colors.black, fillColor=None, strokeWidth=1))
    d.add(String(width / 2, h - 10, title, fontName='Helvetica-Bold', fontSize=7.5, textAnchor='middle'))
    for i in range(n):
        yy = top - i * pitch - 6
        d.add(String(bx - 4, yy, left_pins[i], fontName='Helvetica', fontSize=6.5, textAnchor='end'))
        d.add(String(bx + 3, yy, str(i + 1), fontName='Helvetica', fontSize=6))
        d.add(String(bx + bw + 4, yy, right_pins[i], fontName='Helvetica', fontSize=6.5))
        d.add(String(bx + bw - 3, yy, str(2 * n - i), fontName='Helvetica', fontSize=6, textAnchor='end'))
    return d


def block_diagram(width=6.3 * inch):
    d = Drawing(width, 160)
    def box(x, y, w, h, text, sub=''):
        d.add(Rect(x, y, w, h, strokeColor=colors.black, fillColor=None, strokeWidth=1))
        d.add(String(x + w / 2, y + h / 2 + (2 if sub else -3), text, fontName='Helvetica-Bold', fontSize=7.5, textAnchor='middle'))
        if sub:
            d.add(String(x + w / 2, y + h / 2 - 9, sub, fontName='Helvetica', fontSize=6.5, textAnchor='middle'))
    box(0, 60, 80, 44, 'CPU', 'Z80')
    box(185, 50, 110, 64, 'TMS9918B', 'VDP')
    box(395, 105, 90, 44, 'VRAM', '8 x 16K x 1, 120 ns')
    box(395, 10, 90, 44, 'VIDEO', 'TV or monitor')
    d.add(Line(80, 82, 185, 82, strokeColor=colors.black, strokeWidth=1))
    d.add(String(132, 88, 'CD0-CD7, MODE', fontName='Helvetica', fontSize=6.2, textAnchor='middle'))
    d.add(String(132, 72, 'CSR, CSW, INT', fontName='Helvetica', fontSize=6.2, textAnchor='middle'))
    d.add(Line(295, 100, 395, 127, strokeColor=colors.black, strokeWidth=1))
    d.add(String(345, 124, 'AD0-AD7, RD0-RD7', fontName='Helvetica', fontSize=6.2, textAnchor='middle'))
    d.add(String(345, 97, 'RAS, CAS, R/W', fontName='Helvetica', fontSize=6.2, textAnchor='middle'))
    d.add(Line(295, 64, 395, 32, strokeColor=colors.black, strokeWidth=1))
    d.add(String(345, 60, 'COMVID or', fontName='Helvetica', fontSize=6.2, textAnchor='middle'))
    d.add(String(345, 35, 'Y, R-Y, B-Y', fontName='Helvetica', fontSize=6.2, textAnchor='middle'))
    d.add(String(240, 38, 'XTAL1, XTAL2: 10.738635 MHz', fontName='Helvetica', fontSize=6.5, textAnchor='middle'))
    return d


def memory_map(regions, width=2.6 * inch):
    """regions: list of (start, end_exclusive, label) covering 0000h-4000h in order."""
    bh = 22
    h = bh * len(regions) + 24
    d = Drawing(width, h)
    x, w = 10, width - 70
    top = h - 12
    for i, (s, e, label) in enumerate(regions):
        y = top - (i + 1) * bh
        unused = label.upper() == 'UNUSED'
        d.add(Rect(x, y, w, bh, strokeColor=colors.black, fillColor=colors.Color(0.93, 0.93, 0.93) if unused else None, strokeWidth=0.9))
        d.add(String(x + w / 2, y + bh / 2 - 3, label, fontName='Helvetica-Bold' if not unused else 'Helvetica', fontSize=6.8, textAnchor='middle'))
        d.add(String(x + w + 5, y + bh - 4, '%04X' % s, fontName='Helvetica', fontSize=6.5))
    d.add(String(x + w + 5, top - len(regions) * bh - 2, '3FFF', fontName='Helvetica', fontSize=6.5))
    return d
