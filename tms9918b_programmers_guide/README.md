# TMS9918B Programmer's Guide Supplement

Two editions of the same content, in the style of TI's Video Display Processors Programmer's Guide (SPPU004, 1984):

| File | Edition |
|---|---|
| `TMS9918B_Programmers_Guide_Supplement.pdf` | 23 pages: sections 1-9, appendices A-D, section-numbered pages |
| `TMS9918B_Programmers_Guide_Supplement.md` | Markdown edition with linked contents; memory map figures in `images_guide/` (SVG) |

Contents: unlocking and detecting the TMS9918B, palette writes, status register S1, extended registers, register
initialization tables and VRAM memory maps for every extended mode, two-bit and fat pattern encoding, bitmap and
64-column text addressing, sprite colour byte, BANK and sprite pairs, hardware scrolling, scanline interrupts,
palette effects, fast VRAM transfers, quick reference and address location tables, Z80 support routines for
SC-3000 class systems (ports BEh/BFh).

Bit numbering is D0 = MSB, as in TI documents. Timing values come from the TMS9918B model.

Rebuild both editions: `python3 source/build_guide.py TMS9918B_Programmers_Guide_Supplement.pdf` (ReportLab 4).

## License

The documents are licensed under CC BY 4.0 (`LICENSE-DOCS` in the repository root); the example programs may also be used under Apache-2.0; the generator in `source/` under Apache-2.0 (`LICENSE`).
