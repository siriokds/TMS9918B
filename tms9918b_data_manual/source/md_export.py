# TMS9918B documentation generator
#
# Copyright 2026 Saverio Russo
# SPDX-License-Identifier: Apache-2.0
# The generated documents are licensed under CC BY 4.0 (LICENSE-DOCS).

"""Markdown export of a ReportLab story built with common.py helpers."""
import html
import os
import re

from reportlab.graphics import renderSVG
from reportlab.graphics.shapes import Drawing
from reportlab.platypus import Paragraph, Table, KeepTogether, Spacer, PageBreak, NextPageTemplate, Preformatted

from common import Heading, SectionMarker, ST


def inline(text):
    t = text
    t = re.sub(r'<b>(.*?)</b>', r'**\1**', t, flags=re.S)
    t = re.sub(r'<i>(.*?)</i>', r'*\1*', t, flags=re.S)
    t = re.sub(r'<br\s*/?>', '  \n', t)
    t = re.sub(r'<[^>]+>', '', t)
    t = html.unescape(t)
    t = re.sub(r'\s+', ' ', t).strip()
    return t.replace('|', '\\|')


def anchor(text):
    a = inline(text).lower()
    a = re.sub(r'[^a-z0-9 \-]', '', a)
    return a.strip().replace(' ', '-')


def md_table(rows, header_rows=1):
    width = max(len(r) for r in rows)
    rows = [list(r) + [''] * (width - len(r)) for r in rows]
    out = []
    head = rows[0] if header_rows else [''] * width
    out.append('| ' + ' | '.join(inline(c) for c in head) + ' |')
    out.append('|' + '---|' * width)
    for r in rows[header_rows:]:
        out.append('| ' + ' | '.join(inline(c) for c in r) + ' |')
    return '\n'.join(out) + '\n'


class Exporter:
    def __init__(self, img_dir, img_prefix):
        self.img_dir = img_dir
        self.img_prefix = img_prefix
        self.n = 0
        self.lines = []

    def image(self, drawing):
        self.n += 1
        name = 'figure_%02d.svg' % self.n
        renderSVG.drawToFile(drawing, os.path.join(self.img_dir, name))
        self.lines.append('![figure %d](%s/%s)\n' % (self.n, self.img_prefix, name))

    def walk(self, f):
        if isinstance(f, (Spacer, PageBreak, NextPageTemplate, SectionMarker)):
            return
        if isinstance(f, KeepTogether):
            items = f._content
            if len(items) == 2 and isinstance(items[0], Paragraph) and items[0].style.name == 'notehead':
                self.lines.append('> **%s**  \n> %s\n' % (inline(items[0].text).replace('**', ''), inline(items[1].text)))
                return
            for x in items:
                self.walk(x)
            return
        if isinstance(f, Preformatted):
            self.lines.append('\n```\n%s\n```\n' % '\n'.join(''.join(l) if isinstance(l, list) else str(l) for l in f.lines))
            return
        if isinstance(f, Heading):
            level = f.toc_level + 1
            self.lines.append('%s %s\n' % ('#' * (level + 1), inline(f.toc_text)))
            return
        if isinstance(f, Paragraph):
            style = f.style.name
            text = f.text
            if getattr(f, 'bulletText', None):
                self.lines.append('- %s' % inline(text))
                return
            plain = inline(text).replace('**', '')
            if style in ('caption', 'tcaption'):
                self.lines.append('\n**%s**\n' % plain)
            elif style == 'mono':
                self.lines.append('```\n%s\n```\n' % html.unescape(re.sub(r'<[^>]+>', '', text)).strip())
            elif style == 'notehead':
                self.lines.append('**%s**\n' % plain)
            elif style == 'h1':
                if plain != 'TABLE OF CONTENTS':
                    self.lines.append('## %s\n' % plain)
            else:
                self.lines.append('\n%s\n' % inline(text))
            return
        if isinstance(f, Drawing):
            self.image(f)
            return
        if isinstance(f, Table):
            if hasattr(f, '_md_register'):
                name, cells = f._md_register
                rows = [['Bits', 'Field']]
                bit = 0
                for label, span in cells:
                    bits = 'D%d' % bit if span == 1 else 'D%d-D%d' % (bit, bit + span - 1)
                    rows.append([bits, label])
                    bit += span
                self.lines.append('\n**%s** (D0 = MSB)\n\n%s' % (inline(name), md_table(rows)))
                return
            if hasattr(f, '_md_data'):
                self.lines.append('\n' + md_table(f._md_data, f._md_header_rows))
                return
            for row in f._cellvalues:
                for cell in row:
                    if isinstance(cell, Drawing):
                        self.image(cell)
            return
        # other flowables (cover) are rendered by the caller

    def export(self, story):
        self.lines = []
        for f in story:
            self.walk(f)
        out = []
        for i, line in enumerate(self.lines):
            out.append(line)
            nxt = self.lines[i + 1] if i + 1 < len(self.lines) else ''
            if line.startswith('- ') and not nxt.startswith('- '):
                out.append('')
        text = '\n'.join(out)
        return re.sub(r'\n{3,}', '\n\n', text)


def toc_markdown(entries):
    out = ['## Table of Contents\n']
    for level, text, page in entries:
        if level > 1:
            continue
        indent = '  ' * level
        out.append('%s- [%s](#%s) (%s)' % (indent, inline(text), anchor(text), page))
    return '\n'.join(out) + '\n'
