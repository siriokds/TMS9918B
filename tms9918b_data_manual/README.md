# TMS9918B/TMS9928B/TMS9929B Data Manual

Two editions of the same content:

| File | Edition |
|---|---|
| `TMS9918B_Data_Manual.pdf` | 18 pages in the style of TI's MP010A (November 1982): sections 1-6, appendices A-C, section-numbered pages |
| `TMS9918B_Data_Manual.md` | Markdown edition with linked contents; figures in `images/` (SVG) |

Bit numbering is D0 = MSB, as in TI documents. Values not restated are those of MP010A.

All timing figures come from the TMS9918B model (`tms9918b_sc3000_model.zip`): memory cycle waveforms and
margins against the Fujitsu MB8118-12 data sheet, CPU access times, memory access sequences and VRAM maps.

Rebuild both editions: `python3 source/build_datasheet.py TMS9918B_Data_Manual.pdf` (ReportLab 4). The
Markdown file and the `images/` directory are written next to the PDF.

## License

The documents are licensed under CC BY 4.0 (`LICENSE-DOCS` in the repository root); the generator in `source/` under Apache-2.0 (`LICENSE`).
