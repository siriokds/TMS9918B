# TMS9918B documentation generator
#
# Copyright 2026 Saverio Russo
# SPDX-License-Identifier: Apache-2.0
# The generated documents are licensed under CC BY 4.0 (LICENSE-DOCS).

import sys
import os; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
from drawings import waveform, calendar_bar, pinout, block_diagram

OUT = sys.argv[1] if len(sys.argv) > 1 else 'TMS9918B_Data_Manual.pdf'
W = PAGE_W - LM - RM
class Cover(Flowable):
    def wrap(self, aw, ah):
        return aw, ah - 1
    def draw(self):
        c = self.canv
        w = PAGE_W - LM - RM
        h = PAGE_H - TM - BM
        c.setFont('Helvetica-Bold', 11)
        c.drawString(0, h - 20, 'TEAM B EUROPE')
        c.setFont('Helvetica', 9)
        c.drawRightString(w, h - 20, 'TB-9918B-01')
        c.setLineWidth(2)
        c.line(0, h - 30, w, h - 30)
        c.setFont('Helvetica-Bold', 30)
        c.drawString(0, h - 250, 'TMS9918B/TMS9928B/TMS9929B')
        c.setFont('Helvetica-Bold', 22)
        c.drawString(0, h - 285, 'Video Display Processors')
        c.setFont('Helvetica', 16)
        c.drawString(0, h - 312, 'Data Manual')
        c.setLineWidth(0.8)
        c.line(0, h - 330, w, h - 330)
        c.setFont('Helvetica', 9.5)
        lines = ['Pin-for-pin replacement for the TMS9918A/TMS9928A/TMS9929A',
                 'Same crystal, same 16K x 1 dynamic VRAM board (120 ns, page mode)',
                 'Extended modes: 2 bpp tiles with palettes, bitmap, 64-column text',
                 'Eight sprites per line, hardware scrolling, scanline interrupt']
        for i, t in enumerate(lines):
            c.drawString(0, h - 355 - i * 14, t)
        c.setFont('Helvetica-Bold', 10)
        c.drawString(0, 60, 'ADVANCE INFORMATION')
        c.setFont('Helvetica', 8)
        c.drawString(0, 46, 'Hypothetical 1983 device - design study prepared for the GearSF7000 emulator.')
        c.drawString(0, 34, 'Not a Texas Instruments product. TMS9918A is a trademark reference used for compatibility only.')
        c.drawRightString(w, 34, 'September 2026')


def build_story(entries):
    story = []

    # ---------------------------------------------------------------- cover
    story += [Cover(), NextPageTemplate('front'), PageBreak()]

    # ---------------------------------------------------------------- notices + contents
    story += [Spacer(1, 2.6 * inch), Paragraph('<b>IMPORTANT NOTICES</b>', ST['notehead']),
              P('This manual describes the TMS9918B, TMS9928B and TMS9929B, a hypothetical revision of the Texas Instruments '
                'TMS9918A family developed as a design study. Every electrical and timing value marked as derived was computed '
                'from the data sheets of the dynamic RAMs fitted to 1983 Sega SC-3000 boards and from the TMS9918A/9928A/9929A '
                'Video Display Processors Data Manual (MP010A). Values not restated here are those of MP010A.', 'note'),
              P('Copyright 2026 Saverio Russo. This document is licensed under the Creative Commons Attribution 4.0 International '
                'License (CC BY 4.0). Anyone may implement the device it describes, in hardware or software, and may copy and '
                'adapt the document, provided that credit is given. Implementations may call themselves TMS9918B compatible '
                'when they meet the conditions of COMPATIBILITY.md.', 'note'),
          P('The design rationale, cost analysis and rejected alternatives are in the separate TMS9918B Design Notes. '
                'Programming examples are in the TMS9918B Programmer\'s Guide Supplement.', 'note'),
              NextPageTemplate('front'), PageBreak()]
    story += [Paragraph('<b>TABLE OF CONTENTS</b>', ST['h1']), static_toc(entries, W), NextPageTemplate('body'), PageBreak()]

    # ---------------------------------------------------------------- 1 introduction
    story += [SectionMarker('1'), H(0, '1. INTRODUCTION'), H(1, '1.1 Description')]
    story += [P('The TMS9918B/9928B/9929B video display processors (VDP) are N-channel MOS LSI devices that generate all video, '
                'control and synchronization signals of a raster-scanned colour television or monitor and control the storage, '
                'retrieval and refresh of display data in a dynamic screen refresh memory. They are pin-for-pin replacements of the '
                'TMS9918A/9928A/9929A and use the same 10.738635 MHz crystal and the same eight 16K x 1 dynamic RAMs.'),
              P('After reset the TMS9918B behaves as a TMS9918A: the same registers, the same four display modes and undocumented '
                'modes, the same VRAM access sequence and the same CPU access windows. Software unlocks the extended functions with a '
                'register command. The extended functions add a 16-entry palette of TMS colours with four luminance levels, '
                'six extended display modes, eight sprites per line with three-colour sprite pairs, hardware scrolling and a '
                'scanline interrupt.'),
              P('The VDP reads VRAM with a memory cycle placed on half-periods of the crystal and fetches adjacent bytes in page '
                'mode. The extended functions therefore require dynamic RAMs with 120 ns access time and page-mode '
                'capability (Section 3.1 and Appendix A).'),
              P('The TMS9928B/9929B are functionally identical to the TMS9918B except for the colour encoding, which is replaced by '
                'luminance and colour-difference outputs; the TMS9929B uses the 625-line format.')]
    story += [H(1, '1.2 Features')]
    story += B(['Pin-for-pin and software compatible with the TMS9918A/9928A/9929A',
                'Same crystal (10.738635 MHz) and same 16K x 1 dynamic VRAM (120 ns, page mode)',
                '256 x 192 resolution; 512-pixel lines in 64-column text',
                '15 TMS colours at 4 luminance levels through a 16-entry write-only palette',
                'Extended modes: Graphics1X (512 two-bit tiles), Graphics2Fat (16 colours per 2-pixel dot), linear Bitmap, '
                'Text 40 with colours per character, Text 64 with 768 characters',
                'Eight sprites per line, 512 sprite patterns, three-colour sprite pairs, tile priority over sprites',
                'Horizontal and vertical hardware scroll with left-column blanking and locked areas',
                'Scanline interrupt with programmable line count',
                'Faster CPU access: 14 to 18 T-states worst case in extended modes (29 T in Graphics mode)',
                'Standard 40-pin package'])
    story += [H(1, '1.3 Typical Applications')]
    story += B(['Home computers and video game consoles based on the TMS9918A family', 'Colour terminals with 64-column text',
                'Home educational systems', 'European 625-line TV (TMS9929B)'])
    story += [Spacer(1, 6), block_diagram(W), caption('FIGURE 1-1 - SYSTEM BLOCK DIAGRAM')]
    story += [H(1, '1.4 Acronyms and Glossary')]
    story += [table([['Term', 'Meaning'],
                     ['Unit', 'Half a period of the 10.738635 MHz crystal, 46.56 ns. Timing positions in this manual are given in units.'],
                     ['Slot', 'TMS9918A memory cycle position, 8 units (372 ns). 171 slots per line.'],
                     ['Burst', 'Page-mode memory cycle reading 2 to 4 adjacent bytes under one RAS pulse.'],
                     ['Extended mode', 'Display mode selected with MX = 1 after the unlock command (Section 2.5).'],
                     ['Palette entry', '6-bit value: TMS colour code and luminance level.'],
                     ['Stratum', 'One bit plane of a 2-bit-per-pixel pattern.'],
                     ['T-state', 'Z80 clock period at 3.58 MHz, 6 units.'],
                     ['VRAM', 'Video refresh memory, 16384 bytes of dynamic RAM.']],
                    [1.2 * inch, W - 1.2 * inch])]
    story += [PageBreak()]

    # ---------------------------------------------------------------- 2 architecture
    story += [SectionMarker('2'), H(0, '2. ARCHITECTURE'), H(1, '2.1 CPU Interface')]
    story += [P('The CPU interface is identical to the TMS9918A: an 8-bit bidirectional bus (CD0-CD7, CD0 is the MSB), MODE, CSR, '
                'CSW and INT. Bit 0 is the most significant bit throughout this manual, as in MP010A.'),
              H(2, '2.1.1 Control port codes'),
              P('A two-byte control transfer carries an address or a register value in the first byte and a code in the two most '
                'significant bits of the second byte.')]
    story += [table([['Second byte D0 D1', 'TMS9918A', 'TMS9918B, XE = 0', 'TMS9918B, XE = 1'],
                     ['0 0', 'VRAM read address', 'VRAM read address', 'VRAM read address'],
                     ['0 1', 'VRAM write address', 'VRAM write address', 'VRAM write address'],
                     ['1 0', 'register write', 'register write', 'register write'],
                     ['1 1', 'register write', 'register write', 'palette write address (first byte D4-D7 = entry)']],
                    [1.3 * inch, 1.5 * inch, 1.5 * inch, W - 4.3 * inch], align_center_cols=(0,)),
              tcaption('TABLE 2-1 - CONTROL PORT CODES')]
    story += [H(2, '2.1.2 Register unlock'),
              P('After reset the register number decodes three bits, as on the TMS9918A: register 8 aliases R0 and register 15 '
                'aliases R7. Two consecutive register writes of <b>5Ah</b> to register 59 (second byte <b>BBh</b>), without another '
                'register write in between, unlock registers 8 to 15. Only RESET locks them again. While locked the command lands in '
                'R3 (59 AND 7 = 3); software rewrites R3 afterwards, and the same restore also covers the XE write of '
            'Section 4.2, which aliases R3 too.'),
              P('While locked, a write to any register number above 7 lands in the register given by the three low bits: register 8 '
            'in R0, register 11 in R3, register 15 in R7. Software that uses the extended registers must therefore detect the '
            'device first (Section 4.2), and every probe must restore the registers its own writes aliased.'),
          note('Register 59 and the value 5Ah do not collide with the F18A/PICO9918 unlock (1Ch to register 57, which lands in R1 '
                   'on a locked TMS9918B) nor with R15, the status register select of the V9938 and F18A.'),
              H(2, '2.1.3 CPU access to VRAM'),
              P('The 14-bit autoincrementing address register, the read-ahead byte and the transfer sequences are those of the '
                'TMS9918A. The VDP performs a CPU transfer in the next memory cycle reserved for the CPU; worst-case delays for every '
                'mode are listed in Appendix B.'),
              H(2, '2.1.4 VDP interrupt'),
              P('INT is active (low) when F = 1 and IE = 1 (frame interrupt, as on the TMS9918A) or when FL = 1 and IE1 = 1 '
                '(scanline interrupt, Section 2.7). Reading S0 clears F; reading S1 clears FL.'),
              H(2, '2.1.5 VDP initialization'),
              P('RESET clears R0, R1, R11 and R12, locks the extended registers and selects status register S0. The palette, '
                'R8-R10 and R13-R15 are undefined after power-up and must be written before extended modes are used.')]

    story += [H(1, '2.2 Write-Only Registers')]
    story += [P('Registers R0 to R7 keep their TMS9918A functions (MP010A Section 2.2). Registers R11 to R15 exist after the unlock '
                'command. Figure 2-1 shows the extended registers; reserved bits must be written as 0.')]
    figs = [
        ('R8 H SCROLL', [('COLUMNS', 5), ('FINE', 3)]),
        ('R9 V SCROLL', [('LINES (0-191)', 8)]),
        ('R10 LINE COUNT', [('RELOAD VALUE', 8)]),
        ('R11 MODE', [('IE1', 1), ('0', 1), ('0', 1), ('0', 1), ('0', 1), ('0', 1), ('MX', 1), ('XE', 1)]),
        ('R12 SCREEN', [('0', 1), ('0', 1), ('0', 1), ('0', 1), ('HLOCK', 1), ('VLOCK', 1), ('MASK', 1), ('T64', 1)]),
        ('R13', [('RESERVED', 8)]),
        ('R14', [('RESERVED', 8)]),
        ('R14', [('RESERVED', 8)]),
        ('R15 STATUS SEL', [('0', 1), ('0', 1), ('0', 1), ('0', 1), ('STATUS REGISTER NUMBER', 4)]),
    ]
    for name, cells in figs:
        story += [register_figure(name, cells), Spacer(1, 3)]
    story += [caption('FIGURE 2-1 - EXTENDED REGISTERS (D0 = MSB)')]
    story += [table([['Register / bit', 'Function'],
                     ['R8', 'World X = screen X + R8 (modulo 256). D0-D4 whole columns, D5-D7 pixels. Sampled once per line.'],
                     ['R9', 'World line = (line + R9) modulo 192. Sampled once per line.'],
                     ['R10', 'Scanline interrupt reload value (Section 2.7).'],
                     ['R11 D7 XE', 'Extended enable: palette port, S1, R8-R15 and IE1 become active. With XE = 0 the VDP is a TMS9918A.'],
                     ['R11 D6 MX', 'Extended display modes (requires XE). Selects the modes of Table 2-3 together with M1, M2, M3.'],
                     ['R11 D0 IE1', 'Scanline interrupt enable.'],
                     ['R12 D7 T64', 'Extended text modes use 64 columns instead of 40.'],
                     ['R12 D6 MASK', 'The leftmost 8 pixels show the backdrop colour, sprites included.'],
                     ['R12 D5 VLOCK', 'Columns 24-31 do not scroll vertically.'],
                     ['R12 D4 HLOCK', 'Lines 0-15 do not scroll horizontally.'],
                     ['R15 D4-D7', 'Status register returned by a status read: 1 selects S1, any other value selects S0.']],
                    [1.3 * inch, W - 1.3 * inch]),
              tcaption('TABLE 2-2 - EXTENDED REGISTER FUNCTIONS')]
    story += [note('In extended modes R3 is not used, R2 and R4 locate the extended tables (Section 3.2) and R7 supplies the '
                   'backdrop colour and the text colours. R5 and R6 keep their TMS9918A meaning.')]

    story += [H(1, '2.3 Status Registers')]
    story += [register_figure('S0', [('F', 1), ('5S', 1), ('C', 1), ('SPRITE NUMBER', 5)]), Spacer(1, 3),
              register_figure('S1', [('0', 1), ('0', 1), ('0', 1), ('1', 1), ('1', 1), ('0', 1), ('0', 1), ('FL', 1)]),
              caption('FIGURE 2-2 - STATUS REGISTERS')]
    story += [P('S0 is the TMS9918A status register. In extended graphics modes 5S reports the ninth sprite found on a line pair '
                '(Section 2.6). S1 returns the identification value 18h plus FL (01h), the scanline interrupt flag, cleared when '
                'S1 is read. The identification fails the published detection masks of the F18A (111xxxxx), PICO9918 (E8h), V9938 '
                'and V9958, and reads as 18h when masked with 3Eh.')]

    story += [H(1, '2.4 Palette')]
    story += [register_figure('ENTRY', [('0', 1), ('0', 1), ('LUMINANCE', 2), ('TMS COLOUR CODE', 4)]),
              caption('FIGURE 2-3 - PALETTE ENTRY')]
    story += [P('The palette holds 16 write-only entries on the chip: four palettes of four entries. The colour code selects one of '
                'the 15 TMS9918A colours; the luminance level scales the video output above black level: 00 full, 01 3/4, '
                '10 1/2, 11 1/4. A palette write sets the entry number with control code 11 (Table 2-1); every following data port '
                'write stores one entry and increments the entry number modulo 16. Palette writes do not wait for a memory cycle.'),
              P('Entry 0 of each palette is stored but never displayed: pixel value 0 shows the backdrop (R7 D4-D7) in playfield '
                'modes and is transparent for sprites.')]

    story += [H(1, '2.5 Video Display Modes')]
    story += [table([['XE MX', 'M3', 'M2', 'M1', 'Mode', 'Resolution', 'Sprites/line'],
                     ['0 x or 1 0', '-', '-', '-', 'TMS9918A modes, documented and undocumented', '-', '4 (Text: 0)'],
                     ['1 1', '0', '0', '0', 'Graphics1X', '256 x 192, 32 x 24 tiles, 512 patterns, 2 bpp', '8'],
                     ['1 1', '1', '0', '0', 'Graphics2Fat', '128 x 192 (2-pixel dots), 768 patterns', '8'],
                     ['1 1', '0', '0', '1', 'Text40X / Text64 (T64 = 1)', '40 x 24 (6 x 8) / 64 x 24 (8 x 8)', 'cursor'],
                     ['1 1', '1', '0', '1', 'Text40XQ / Text64Q', 'as above, up to 768 characters', 'cursor'],
                     ['1 1', '0', '1', '0', 'Bitmap', '256 x 192, 2 bpp', '8'],
                     ['1 1', '1', '1', '0', 'BitmapQ', '256 x 192, 2 bpp, third banking', '8'],
                     ['1 1', 'x', '1', '1', 'bars (as TMS9918A)', '-', '0']],
                    [0.7 * inch, 0.35 * inch, 0.35 * inch, 0.35 * inch, 1.65 * inch, W - 4.2 * inch, 0.8 * inch], align_center_cols=(0, 1, 2, 3, 6)),
              tcaption('TABLE 2-3 - DISPLAY MODES (M1 = R1 D3, M2 = R1 D4, M3 = R0 D6)')]
    story += [note('With XE = 0 or MX = 0 all eight M1-M2-M3 combinations behave as on the TMS9918A, including the undocumented '
                   'Text 1Q (M1 + M3), Multicolor Q (M2 + M3) and bar modes (M1 + M2, M1 + M2 + M3), with the TMS9918A table '
                   'addresses and memory access sequences: Text for M1 = 1, Multicolor for M2 = 1 and M1 = 0, Graphics otherwise.')]
    story += [P('M3 selects the third-banking variant of each extended mode: R4 D6-D7 give the middle and bottom thirds of the '
                'screen their own table blocks, as in Graphics II.')]

    story += [H(2, '2.5.1 Graphics1X Mode'),
              P('Each of the 768 tile positions has a 2-byte name entry: the pattern name and an attribute. A pattern has 16 bytes, '
                'two for each of its eight rows (stratum 0, stratum 1). Pixel value = stratum 0 bit + 2 x stratum 1 bit; value 0 is '
                'the backdrop and values 1-3 are entries of the palette selected by the attribute.'),
              register_figure('ATTRIBUTE', [('0', 1), ('0', 1), ('0', 1), ('0', 1), ('NAME8', 1), ('PRIOR', 1), ('PALETTE', 2)]),
              caption('FIGURE 2-4 - GRAPHICS1X ATTRIBUTE'),
              P('NAME8 is the ninth bit of the pattern name (512 patterns). PRIOR places the non-backdrop pixels of the tile in front '
                'of all sprites.')]
    story += [H(2, '2.5.2 Graphics2Fat Mode'),
              P('The name entry and the 16-byte pattern layout are those of Graphics1X, with third banking (768 patterns) and the '
                'attribute PRIOR bit only. Each pattern row describes four dots, each two pixels wide: dot p takes bits (2p, 2p+1) of '
                'stratum 0 as its low bits and the same bits of stratum 1 as its high bits (D0 = MSB). The 4-bit value is a TMS '
                'colour code at full luminance; 0 is the backdrop.')]
    story += [H(2, '2.5.3 Bitmap and BitmapQ Modes'),
              P('The bitmap stores two bytes (stratum 0, stratum 1) for every 8 pixels of every line: 64 bytes per line, 12288 bytes '
                'for the screen. A palette map holds one byte for every 8 x 8 area; its D6-D7 select the palette of the area. In '
                'BitmapQ each third of the screen uses a 4096-byte block selected as in Graphics II.')]
    story += [H(2, '2.5.4 Text40X and Text64 Modes'),
              P('Text40X keeps the 40 x 24 layout of Text mode with a 2-byte entry per character: the pattern name and a colour byte '
                'whose D0-D3 give the foreground and D4-D7 the background; a zero nibble uses the corresponding nibble of R7, so '
                'a colour byte of 00h displays exactly as Text mode.'),
              P('With T64 = 1 the VDP displays 64 x 24 characters of 8 x 8 pixels on 512-pixel lines (one name byte per character, '
                'all 8 pattern bits used). The two colours of the whole screen come from R7. Monitors with RGB or component input '
                'are required for legible 64-column text.'),
              P('In both text modes sprites 0 and 1 are displayed over the text, a pair when sprite 1 has PAIR set; sprites 2 to '
                '31 are not displayed, because a text line has no room to read their vertical positions. Their patterns are '
                'not stretched: a sprite eight pixels wide covers one character cell in every mode.'),
              P('At 64 columns the picture is 512 pixels wide and the X byte reaches 255, so in that mode the coordinate '
                'counts two-pixel steps and XFINE, bit 1 of the colour byte, supplies the odd pixel: the sprite is displayed '
                'at 2 x X + XFINE. A cursor sits on a character boundary and leaves XFINE at 0; an object that moves needs it, '
                'otherwise it would cross the line in 256 steps instead of 512. The bit is the low one of the position and not '
                'a ninth bit at the top so that an update caught between the two writes costs one pixel for one frame instead '
                'of 256, and so that software which ignores it behaves as it always did.')]

    story += [H(1, '2.6 Sprites')]
    story += [P('The Sprite Attribute Table and Sprite Pattern Table keep the TMS9918A layout. In extended graphics modes the fourth '
                'attribute byte has the format of Figure 2-5 and up to eight sprites are displayed per line.'),
              register_figure('COLOUR BYTE', [('EC', 1), ('XFINE', 1), ('BANK', 1), ('PAIR', 1), ('PALETTE', 2), ('ENTRY', 2)]),
              caption('FIGURE 2-5 - SPRITE ATTRIBUTE BYTE 3 IN EXTENDED MODES')]
    story += [table([['Field', 'Function'],
                     ['EC', 'Early clock: X - 32, as TMS9918A. It acts on the coarse coordinate, so in a 512-pixel mode the sprite moves left by 64 pixels.'],
                 ['XFINE', 'In a 512-pixel mode, the low bit of the horizontal position: the sprite is displayed at 2 x X + XFINE. Channels 0 and 1 only; reserved and written as 0 in every other mode.'],
                     ['BANK', 'Pattern taken from the 2048-byte table that follows the one located by R6.'],
                     ['PAIR', 'Odd sprites only: sprite 2k+1 becomes the second bit plane of sprite 2k.'],
                     ['PALETTE, ENTRY', 'Single sprite colour: entry 1-3 of the palette; entry 0 makes the sprite invisible '
                                        '(it still takes part in coincidence).'],
                     ['Pair colour', 'Pixel value = bit of sprite 2k + 2 x bit of sprite 2k+1, using entries 1-3 of the palette of '
                                     'sprite 2k; the palette and entry fields of sprite 2k+1 are ignored.']],
                    [1.2 * inch, W - 1.2 * inch]),
              tcaption('TABLE 2-4 - SPRITE COLOUR BYTE FIELDS')]
    story += [P('Both sprites of a pair keep their own position, name and pattern; the combined value exists where their pixels '
                'overlap. Their overlap does not set the coincidence flag C; overlaps with other sprites do. Priority is the '
                'priority of sprite 2k.'),
              P('Sprite selection is evaluated over line pairs: the vertical positions of sprites 0-15 are compared during even '
                'lines and those of sprites 16-31 during odd lines, and the selection is used for the next two lines. The eight-sprite '
                'limit applies to the sprites visible on either line of the pair; 5S and the sprite number in S0 report the ninth.')]

    story += [H(1, '2.7 Scrolling and Scanline Interrupt')]
    story += [P('R8 and R9 scroll Graphics1X, Graphics2Fat, Bitmap and BitmapQ; R9 also scrolls the text modes. The memory '
                'access sequence does not change: the fetched tile column is (c + 1 + R8/8) modulo 32 and the fine scroll selects '
                'the output tap of the pixel shift register. MASK blanks the first 8 pixels so that 32 fetched columns cover the '
                'visible picture for every fine scroll value. The world line selects name row, pattern row and third together.'),
              P('Both scroll registers are sampled once per line, at the first background access of that line. A value written '
                'later takes effect on the next line, on both axes: a cell can never take its name from one world position and '
                'its pattern from another, and a scanline interrupt can change either register for the lines that follow.'),
              P('The line counter is loaded from R10 at the first active line and whenever R10 is written. It is decremented at the end '
                'of every active line 0-191; when it would become negative, FL is set and the counter is reloaded. R10 = 0 '
                'interrupts on every line; R10 = n interrupts every n+1 lines.')]
    story += [PageBreak()]

    # ---------------------------------------------------------------- 3 interfaces
    story += [SectionMarker('3'), H(0, '3. VDP INTERFACES AND OPERATION'), H(1, '3.1 VDP/VRAM Interface')]
    story += [P('The VRAM interface uses the TMS9918A terminals: AD0-AD7 (address and write data, AD0 = MSB), RD0-RD7 (read data), '
                'RAS, CAS and R/W, connected to eight 16K x 1 dynamic RAMs as described in MP010A Table 3-1.'),
              H(2, '3.1.1 VRAM Memory Types'),
              P('The TMS9918B requires 16K x 1 dynamic RAMs with 120 ns row access time, 270 ns cycle time and page-mode operation, '
                'single +5 V types included. Examples: Fujitsu MB8118-12, Motorola MCM4517-12. 150 ns types do not meet the data '
                'setup requirement in any mode. R1 D0 (4/16K) must be 1. Appendix A gives the selection equations.'),
              H(2, '3.1.2 Address Multiplexing'),
              P('A0-A6 (the most significant address bits) are output with RAS as row address and A7-A13 with CAS as column address. '
                'A page-mode cycle keeps the row and changes only the column, so its bytes lie in one 128-byte page; every table '
                'layout of Section 3.2 starts its multi-byte fetches at a multiple of the fetch length from a base that is a '
                'multiple of 128.'),
              H(2, '3.1.3 Memory Cycles'),
              P('Every memory cycle reads or writes 1 to 4 bytes under one RAS pulse. Edges are placed on half-periods of the '
                'crystal (units of 46.56 ns). An n-byte cycle lasts 5n + 2 units:')]
    story += [table([['Signal', 'Timing (units from RAS falling edge)'],
                     ['RAS', 'low from 0 to 5n - 1, precharge 3 units'],
                     ['CAS, byte k', 'low from 1 + 5k to 4 + 5k; RD0-RD7 are latched on the rising edge'],
                     ['R/W (write, n = 1)', 'low from 2 to 4 (late write); AD0-AD7 carry the column address at the CAS falling edge, then the data, latched by the RAM on the R/W falling edge'],
                     ['Length', '1 byte 7 units (326 ns), 2 bytes 12, 3 bytes 17, 4 bytes 22']],
                    [1.5 * inch, W - 1.5 * inch]),
              tcaption('TABLE 3-1 - MEMORY CYCLE STRUCTURE')]
    story += [waveform(1, False, W), caption('FIGURE 3-1 - SINGLE-BYTE READ CYCLE'),
              waveform(3, False, W), caption('FIGURE 3-2 - THREE-BYTE PAGE-MODE READ CYCLE'),
              waveform(1, True, W), caption('FIGURE 3-3 - WRITE CYCLE')]
    story += [H(2, '3.1.4 Memory Access Sequence'),
              P('In TMS9918A modes the VDP runs a single-byte cycle at the start of every TMS9918A slot followed by one idle unit, so '
                'the positions of the CPU access windows equal those of the TMS9918A. In extended modes each line follows one of the '
                'sequences of Appendix C. The positions refer to phase 0 of the line: unit = 2 x phase; TMS9918A slot s starts at '
                'unit (8s + 1364) modulo 1368.')]

    story += [H(1, '3.2 VRAM Memory Address Derivation')]
    story += [table([['Table', 'Mode', 'Address of the first byte fetched', 'Bytes'],
                     ['Name entry', 'Graphics1X, Graphics2Fat', '(R2) x 400h + 2 x (row x 32 + column)', '2'],
                     ['Pattern row', 'Graphics1X', '(R4) x 800h + 16 x name9 + 2 x row', '2'],
                     ['Pattern row', 'Graphics2Fat', '(R4 D5) x 2000h + 16 x (third x 100h + name AND mask) + 2 x row', '2'],
                     ['Bitmap', 'Bitmap', '(R4) x 800h + 64 x line + 4 x (column pair)', '4'],
                     ['Bitmap', 'BitmapQ', '(R4 D5) x 2000h + 1000h x block + 64 x (line AND 63) + 4 x (column pair)', '4'],
                     ['Palette map', 'Bitmap, BitmapQ', '(R2) x 400h + 32 x row + 2 x (column pair)', '2'],
                     ['Character entry', 'Text40X', '(R2) x 400h + 4 x (row x 20 + column pair)', '4'],
                     ['Name', 'Text64', '(R2) x 400h + 64 x row + 4 x (column group)', '4'],
                     ['Pattern', 'text modes', '(R4) x 800h + 8 x name + row (TMS layout; thirds with M3)', '1'],
                     ['Sprite X, name, colour', 'extended graphics', '(R5) x 80h + 4 x sprite + 1', '3'],
                     ['Sprite Y', 'all', '(R5) x 80h + 4 x sprite', '1'],
                     ['Sprite pattern', 'all', '(R6) x 800h + (BANK) x 800h + 8 x name + row (+16 right half)', '1']],
                    [1.15 * inch, 1.3 * inch, W - 3.0 * inch, 0.55 * inch], align_center_cols=(3,)),
              tcaption('TABLE 3-2 - EXTENDED MODE ADDRESS DERIVATION'),
              P('Row, column, line and third refer to the world position after scrolling. In Graphics2Fat the mask is (R4 D6 D7) x '
                '100h + FFh; in BitmapQ block t of the middle or bottom third is used when the corresponding R4 bit is 1, otherwise '
                'block 0. Addresses are modulo 4000h.')]
    story += [table([['Mode', 'Tables (bytes)', 'Total'],
                     ['Graphics1X', 'patterns 8192, sprite patterns with BANK 4096, names 1536, SAT 128', '13952'],
                     ['Graphics2Fat', 'patterns 12288, sprite patterns 2048, names 1536, SAT 128', '16000'],
                     ['Bitmap, BitmapQ', 'bitmap 12288, sprite patterns 2048, palette map 768, SAT 128', '15232'],
                     ['Text40X', 'patterns 2048, cursor patterns 256, entries 1920, SAT 128', '4352'],
                     ['Text64', 'patterns 2048, cursor patterns 256, names 1536, SAT 128', '3968']],
                    [1.2 * inch, W - 2.0 * inch, 0.8 * inch], align_center_cols=(2,)),
              tcaption('TABLE 3-3 - VRAM REQUIREMENTS OF THE EXTENDED MODES')]

    story += [H(1, '3.3 Monitor Interfaces')]
    story += [P('The composite video output of the TMS9918B and the Y, R-Y and B-Y outputs of the TMS9928B/9929B have the levels and '
                'loads of MP010A Section 3.4. Luminance levels below full scale reduce luminance and colour difference in the same '
                'ratio above black level.')]
    story += [H(1, '3.4 External VDP Operation')]
    story += [P('External VDP operation of the TMS9918B (EXTVDP, R0 D7) is that of the TMS9918A.')]
    story += [H(1, '3.5 Oscillator and Clock Generation')]
    story += [P('The VDP uses a 10.738635 MHz crystal on XTAL1/XTAL2 or an external two-phase clock with the MP010A Section 5.4 '
                'limits (high and low pulse widths 42-52 ns, 42-52 ns phase delay from XTAL1 to XTAL2). Both clock phases time the '
                'memory cycle edges. CPUCLK (fext / 3) and GROMCLK (fext / 24) are unchanged.')]
    story += [H(1, '3.6 VDP Terminal Assignments')]
    left = ['RAS', 'CAS', 'AD7', 'AD6', 'AD5', 'AD4', 'AD3', 'AD2', 'AD1', 'AD0', 'R/W', 'VSS', 'MODE', 'CSW', 'CSR', 'INT', 'CD7', 'CD6', 'CD5', 'CD4']
    right = ['XTAL1', 'XTAL2', 'CPUCLK', 'GROMCLK', 'COMVID', 'EXTVDP', 'RESET/SYNC', 'VCC', 'RD0', 'RD1', 'RD2', 'RD3', 'RD4', 'RD5', 'RD6', 'RD7', 'CD0', 'CD1', 'CD2', 'CD3']
    right28 = ['XTAL1', 'XTAL2', 'R-Y', 'GROMCLK', 'Y', 'B-Y', 'RESET/SYNC', 'VCC', 'RD0', 'RD1', 'RD2', 'RD3', 'RD4', 'RD5', 'RD6', 'RD7', 'CD0', 'CD1', 'CD2', 'CD3']
    pins = Table([[pinout(left, right, 'TMS9918B'), pinout(left, right28, 'TMS9928B/TMS9929B')]], colWidths=[W / 2, W / 2])
    story += [pins, caption('FIGURE 3-4 - TERMINAL ASSIGNMENTS (IDENTICAL TO THE TMS9918A/9928A/9929A)')]
    story += [PageBreak()]

    # ---------------------------------------------------------------- 4 applications
    story += [SectionMarker('4'), H(0, '4. DEVICE APPLICATIONS'), H(1, '4.1 SC-3000 Class Systems')]
    story += [P('The TMS9918B replaces the TMS9918A or TMS9929A of an SC-3000 class system without board changes when the fitted '
                'VRAM meets Section 3.1.1. Boards produced in 1983 carry, among others, Fujitsu MB8118-12 (qualified) and Motorola '
                'MCM4517P15 (not qualified for the TMS9918B) dynamic RAMs. A board with 150 ns VRAM needs eight 120 ns page-mode '
                'RAMs in the same positions.'),
              table([['VDP terminal', 'Connected to'],
                 ['AD0', 'D input of RAM 0 (data only)'],
                 ['AD1 ... AD7', 'A6 ... A0 of all RAMs, and D input of RAM 1 ... RAM 7'],
                 ['RD0 ... RD7', 'Q output of RAM 0 ... RAM 7'],
                 ['RAS, CAS, R/W', 'RAS, CAS, WE of all RAMs']],
                [1.4 * inch, W - 1.4 * inch]),
          tcaption('TABLE 4-1 - VDP TO VRAM CONNECTIONS (AS TMS9918A, MP010A TABLE 3-1)'),
          H(1, '4.2 Software Detection'),
              P('Recommended detection sequence: (1) run F18A/PICO9918 detection first if supported; (2) write 5Ah twice to register '
                '59; (3) write 01h to R11 (XE, bit D7); both land in R3 on other devices, so one restore of R3 covers them; (4) write 01h to R15, read the '
                'status port twice and compare the second value AND 3Eh with 18h; (5) write 00h to R15 and restore R3 and R7.'),
              H(1, '4.3 Initialization'),
              P('Extended modes are selected by writing XE and MX in R11, M1-M3 in R0/R1 and the table bases in R2, R4, R5 and R6, and by '
                'loading the palette. Register values, memory maps and example programs for every mode are given in the TMS9918B '
                'Programmer\'s Guide Supplement.')]
    story += [PageBreak()]

    # ---------------------------------------------------------------- 5 electrical
    story += [SectionMarker('5'), H(0, '5. TMS9918B/9928B/9929B ELECTRICAL SPECIFICATIONS')]
    story += [P('Absolute maximum ratings, recommended operating conditions, electrical characteristics, CPU-VDP timing '
                'requirements and switching characteristics, video output characteristics and external clock requirements are '
                'those of the TMS9918A/9928A/9929A (MP010A Sections 5.1 to 5.5) unless listed below. Supply current is higher than '
                'the TMS9918A because of the added logic; its limits are to be determined.')]
    story += [H(1, '5.1 Timing Requirements, VDP-VRAM Interface')]
    story += [table([['Parameter', 'Description', 'MIN', 'NOM', 'MAX', 'Unit'],
                     ['tsu(D-CH)', 'RD0-RD7 setup time before CAS high', '40', '', '', 'ns'],
                     ['th(CH-D)', 'RD0-RD7 hold time after CAS high', '0', '', '', 'ns'],
                     ['ta(R)', 'VRAM access time from RAS (Appendix A)', '', '', '120', 'ns'],
                     ['ta(C)', 'VRAM access time from CAS (Appendix A)', '', '', '65', 'ns']],
                    [0.9 * inch, W - 2.9 * inch, 0.45 * inch, 0.45 * inch, 0.45 * inch, 0.65 * inch], align_center_cols=(2, 3, 4, 5)),
              tcaption('TABLE 5-1 - VDP-VRAM TIMING REQUIREMENTS')]
    story += [H(1, '5.2 Switching Characteristics, VDP-VRAM Interface')]
    story += [table([['Parameter', 'Description', 'MIN', 'NOM', 'MAX', 'Unit'],
                     ['tc(1)', 'Memory cycle time, one byte (7 units)', '321', '326', '331', 'ns'],
                     ['tc(P)', 'Page-mode byte period, CAS low to CAS low (5 units)', '228', '233', '238', 'ns'],
                     ['tw(RL)', 'RAS low, one byte (4 units)', '181', '186', '191', 'ns'],
                     ['tw(RL)n', 'RAS low, n bytes (5n - 1 units)', '', '46.56 x (5n-1)', '', 'ns'],
                     ['tw(RH)', 'RAS precharge (3 units)', '135', '140', '145', 'ns'],
                     ['td(RL-CL)', 'RAS low to CAS low (1 unit)', '42', '47', '52', 'ns'],
                     ['tw(CL)', 'CAS low (3 units)', '135', '140', '145', 'ns'],
                     ['tw(CH)P', 'CAS high inside a page-mode cycle (2 units)', '88', '93', '98', 'ns'],
                     ['tw(CH)', 'CAS high between cycles (4 units)', '181', '186', '191', 'ns'],
                     ['th(RL-RA)', 'Row address hold after RAS low', '20', '', '', 'ns'],
                     ['tsu(CA-CL)', 'Column address setup before CAS low', '0', '', '', 'ns'],
                     ['th(CH-CA)', 'Column address hold after CAS high', '5', '', '', 'ns'],
                     ['tw(W)', 'R/W low, write cycle (4 units)', '181', '186', '191', 'ns'],
                     ['tsu(D-WL)', 'Write data setup before R/W low', '0', '', '', 'ns'],
                     ['th(RH-D)', 'Write data hold after RAS high', '5', '', '', 'ns']],
                    [0.9 * inch, W - 2.9 * inch, 0.45 * inch, 0.55 * inch, 0.45 * inch, 0.55 * inch], align_center_cols=(2, 3, 4, 5)),
              tcaption('TABLE 5-2 - VDP-VRAM SWITCHING CHARACTERISTICS (CL = 50 pF)'),
              note('Nominal values assume a 50 % clock duty cycle. MIN and MAX of odd unit counts follow the 42-52 ns clock phase '
                   'limits; even unit counts equal whole clock periods.')]
    story += [H(1, '5.3 CPU-VDP Interface')]
    story += [P('CPU-VDP timing requirements and switching characteristics are those of MP010A. Memory access delays are listed in '
                'Appendix B; palette and register writes are not delayed by memory cycles.')]
    story += [PageBreak()]

    # ---------------------------------------------------------------- 6 mechanical
    story += [SectionMarker('6'), H(0, '6. MECHANICAL DATA')]
    story += [P('The TMS9918B/9928B/9929B use the 40-pin plastic dual-in-line package of the TMS9918A/9928A/9929A (MP010A Section 6.1).')]
    story += [PageBreak()]

    # ---------------------------------------------------------------- Appendix A
    story += [SectionMarker('A'), H(0, 'APPENDIX A - CHOOSING VRAM MEMORY')]
    story += [P('The VDP latches RD0-RD7 on the rising edge of CAS. The first byte of a cycle is latched 4 units (186.2 ns) after '
                'the falling edge of RAS; a following byte of a page-mode cycle is latched 3 units (139.7 ns) after the falling '
                'edge of its CAS. With td(RAS) and td(data) the board delays of the strobe and of the data:'),
              Paragraph('186.2 ns >= max[ta(R), 46.6 ns + ta(C)] + td(RAS) + td(data) + tsu(D-CH)', ST['mono']),
              Paragraph('139.7 ns >= ta(C) + td(CAS) + td(data) + tsu(D-CH)', ST['mono']),
              P('With tsu(D-CH) = 40 ns the memories of Table A-1 allow the listed board delay. All other memory limits (cycle time, '
                'RAS and CAS pulse widths, precharge, page-mode timing, write timing) are met with the margins of Table A-2.')]
    story += [table([['Part', 'ta(R)', 'ta(C)', 'Board delay allowed (first byte)', 'Board delay allowed (page byte)', 'Result'],
                     ['Fujitsu MB8118-12', '120 ns', '65 ns', '26 ns', '35 ns', 'qualified'],
                     ['Motorola MCM4517-12', '120 ns', '65 ns', '26 ns', '35 ns', 'qualified'],
                     ['Motorola MCM4517-15', '150 ns', '80 ns', '-4 ns', '20 ns', 'not qualified'],
                     ['TI TMS4116-20', '200 ns', '135 ns', '-54 ns', '-', 'not qualified']],
                    [1.35 * inch, 0.55 * inch, 0.55 * inch, 1.35 * inch, 1.35 * inch, W - 5.15 * inch], align_center_cols=(1, 2, 3, 4, 5)),
              tcaption('TABLE A-1 - VRAM ACCESS TIME BUDGET')]
    story += [table([['Parameter (MB8118-12 limit)', 'VDP provides', 'Margin'],
                     ['tRC 270 ns', '326 ns', '56 ns'], ['tRAS 140 ns', '186 ns', '46 ns'], ['tRP 120 ns', '140 ns', '20 ns'],
                     ['tRCD 25-55 ns', '47 ns', 'within'], ['tCAS 65 ns', '140 ns', '75 ns'], ['tCSH 120 ns', '186 ns', '66 ns'],
                     ['tRSH 85 ns', '140 ns', '55 ns'], ['tCPN 55 ns', '186 ns', '131 ns'], ['tCP 70 ns', '93 ns', '23 ns'],
                     ['tPC 145 ns', '233 ns', '88 ns'], ['tWP 35 ns', '93 ns', '58 ns'], ['tRWL 65 ns', '93 ns', '28 ns'],
                 ['tCWL 50 ns', '93 ns', '43 ns'], ['tWCR 90 ns', '186 ns', '96 ns'], ['tDH 35 ns', '93 ns', '58 ns']],
                    [2.6 * inch, 1.6 * inch, W - 4.2 * inch], align_center_cols=(1, 2)),
              tcaption('TABLE A-2 - MEMORY LIMITS AND MARGINS (NOMINAL CLOCK)')]
    story += [PageBreak()]

    # ---------------------------------------------------------------- Appendix B
    story += [SectionMarker('B'), H(0, 'APPENDIX B - CPU TO VDP ACCESS TIMES')]
    story += [table([['Condition', 'Mode', 'VDP delay', 'Time waiting for an access window', 'Total time', 'T-states @ 3.58 MHz'],
                     ['Active display area', 'Text (TMS)', '2 us', '0 - 1.1 us', '2 - 3.2 us', '12'],
                     ['Active display area', 'Graphics I, II (TMS)', '2 us', '0 - 6.0 us', '2 - 8.0 us', '29'],
                     ['Active display area', 'Multicolor (TMS)', '2 us', '0 - 1.5 us', '2 - 3.5 us', '13'],
                     ['Any time', 'Graphics1X, Graphics2Fat', '2 us', '0 - 3.0 us', '2 - 5.0 us', '18'],
                     ['Any time', 'Bitmap, BitmapQ', '2 us', '0 - 1.9 us', '2 - 4.0 us', '15'],
                     ['Any time', 'Text40X, Text40XQ', '2 us', '0 - 2.2 us', '2 - 4.3 us', '16'],
                     ['Any time', 'Text64, Text64Q', '2 us', '0 - 1.6 us', '2 - 3.7 us', '14'],
                     ['4300 us after vertical interrupt', 'All', '2 us', '0 us', '2 us', '8'],
                     ['Register 1 blank bit 0', 'All', '2 us', '0 us', '2 us', '8']],
                    [1.45 * inch, 1.4 * inch, 0.6 * inch, 1.3 * inch, 0.85 * inch, W - 5.6 * inch], align_center_cols=(2, 3, 4, 5)),
              tcaption('TABLE B-1 - CPU TO VDP ACCESS TIMES'),
              P('A continuous write loop never loses data when its period is at least the total time of the mode. In Bitmap, Text40X '
                'and Text64 modes an OUTI chain (16 T-states) is loss-free; in Graphics1X and Graphics2Fat an OTIR (21 T-states per '
                'byte) is loss-free.')]
    story += [PageBreak()]

    # ---------------------------------------------------------------- Appendix C
    story += [SectionMarker('C'), H(0, 'APPENDIX C - MEMORY ACCESS SEQUENCES')]
    story += [P('Offsets are in units (46.56 ns) from the first unit of a group; figures in parentheses are bytes fetched. The '
                'complete position of every cycle of every line is listed in the TMS9918B model report (calendars.md).')]
    story += [H(2, 'C.1 Graphics1X and Graphics2Fat - two cells per 64 units from unit 212'),
              calendar_bar([(0, 12, 'name|attr (2)', 0.25), (12, 7, 'Y', 0.1), (19, 12, 'pattern (2)', 0.45),
                            (31, 12, 'name|attr (2)', 0.25), (43, 7, 'CPU', 0.0), (50, 12, 'pattern (2)', 0.45)], 64, W),
              caption('FIGURE C-1 - TILE MODE CELL PAIR')]
    story += [H(2, 'C.2 Bitmap and BitmapQ - two cells per 64 units from unit 212'),
              calendar_bar([(0, 22, 'bitmap (4)', 0.45), (22, 7, 'CPU', 0.0), (29, 12, 'palette map (2)', 0.25),
                            (41, 7, 'Y', 0.1), (48, 7, 'CPU', 0.0)], 64, W),
              caption('FIGURE C-2 - BITMAP MODE CELL PAIR')]
    story += [H(2, 'C.3 Text40X - two characters per 48 units from unit 236'),
              calendar_bar([(0, 22, 'char|colour x2 (4)', 0.25), (22, 7, 'CPU', 0.0), (29, 7, 'pat', 0.45), (36, 7, 'pat', 0.45)], 48, W),
              caption('FIGURE C-3 - TEXT40X CHARACTER PAIR')]
    story += [H(2, 'C.4 Text64 - four characters per 64 units from unit 212'),
              calendar_bar([(0, 22, 'names (4)', 0.25), (22, 7, 'CPU', 0.0), (29, 7, 'pat', 0.45), (36, 7, 'pat', 0.45),
                            (43, 7, 'pat', 0.45), (50, 7, 'pat', 0.45), (57, 7, 'CPU', 0.0)], 64, W),
              caption('FIGURE C-4 - TEXT64 CHARACTER GROUP')]
    story += [H(2, 'C.5 Horizontal blanking of the extended graphics modes (units 1236-1579)'),
              P('One CPU cycle; four groups of two sprites a, b of 76 units: sprite X, name and colour of a (3), of b (3), CPU, left '
                'half a, right half a, left half b, right half b, CPU; four CPU cycles. Text modes fetch the cursor sprites 0 and 1 '
                'in the same way and use the rest of the blanking for CPU cycles.'),
              table([['Sequence', 'CPU cycles per line', 'Sprites per line', 'Worst CPU delay'],
                     ['Graphics1X, Graphics2Fat', '29', '8', '18 T'], ['Bitmap, BitmapQ', '45', '8', '15 T'],
                     ['Text40X', '67', 'cursor', '16 T'], ['Text64', '70', 'cursor', '14 T']],
                    [2.2 * inch, 1.4 * inch, 1.3 * inch, W - 4.9 * inch], align_center_cols=(1, 2, 3)),
              tcaption('TABLE C-1 - SEQUENCE SUMMARY')]

    return story

import copy
import os
title = 'TMS9918B/TMS9928B/TMS9929B Video Display Processors Data Manual'
header = 'TMS9918B/TMS9928B/TMS9929B VIDEO DISPLAY PROCESSORS'
first = Doc(OUT + '.tmp.pdf', title, header)
first.build(build_story([]))
doc = Doc(OUT, title, header)
doc.build(build_story(first.toc_entries))

# ---- Markdown edition
from md_export import Exporter, toc_markdown
md_path = os.path.splitext(OUT)[0] + '.md'
img_dir = os.path.join(os.path.dirname(os.path.abspath(OUT)), 'images')
os.makedirs(img_dir, exist_ok=True)
story = build_story(first.toc_entries)
cover_md = ('# TMS9918B/TMS9928B/TMS9929B Video Display Processors - Data Manual\n\n'
            '**Team B Europe, TB-9918B-01, September 2026 - ADVANCE INFORMATION**\n\n'
            '- Pin-for-pin replacement for the TMS9918A/TMS9928A/TMS9929A\n'
            '- Same crystal, same 16K x 1 dynamic VRAM board (120 ns, page mode)\n'
            '- Extended modes: 2 bpp tiles with palettes, bitmap, 64-column text\n'
            '- Eight sprites per line, hardware scrolling, scanline interrupt\n\n'
            '*Hypothetical 1983 device - design study prepared for the GearSF7000 emulator. Not a Texas Instruments product.*\n\n'
            '*Copyright 2026 Saverio Russo - licensed under CC BY 4.0.*\n\n')
toc_pos = next(i for i, f in enumerate(story) if isinstance(f, Table) and not hasattr(f, '_md_data') and not hasattr(f, '_md_register') and i < 20)
front = story[1:toc_pos]
body = story[toc_pos + 1:]
ex = Exporter(img_dir, 'images')
text = cover_md + ex.export(front) + '\n' + toc_markdown(first.toc_entries) + '\n' + ex.export(body)
open(md_path, 'w').write(text)
print('written', md_path, 'with', ex.n, 'figures')
os.remove(OUT + '.tmp.pdf')
print('written', OUT)
