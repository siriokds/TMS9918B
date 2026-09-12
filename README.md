# TMS9918B — a 1983 video chip that could have been

The **TMS9918B/TMS9928B/TMS9929B** is a hypothetical revision of the Texas Instruments TMS9918A family,
designed as it could have been built in 1983 for the machines that already used it: **same 40-pin socket, same
10.738635 MHz crystal, same eight 16K × 1 dynamic RAMs** found on 1983 Sega SC-3000 boards.

After reset it is a TMS9918A, bit for bit and memory cycle for memory cycle. Behind an unlock command it adds
palettes, extended display modes, eight sprites per line, hardware scrolling and a scanline interrupt — all of
it verified against the data sheets of the RAMs and the timing of the original chip.

> **Design study. Not a Texas Instruments product.**

## Documents

| Document | Read online | Print |
|---|---|---|
| **Data Manual** (TB-9918B-01) — hardware, registers, modes, VRAM interface, timing | [Markdown](tms9918b_data_manual/TMS9918B_Data_Manual.md) | [PDF](tms9918b_data_manual/TMS9918B_Data_Manual.pdf) |
| **Programmer's Guide Supplement** (TB-9918B-02) — unlocking, initialization, patterns, sprites, scrolling, Z80 examples | [Markdown](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.md) | [PDF](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.pdf) |
| **Design Notes** (TB-9918B-03) — 1983 setting, memory constraints, precedents, rejected alternatives, cost | [Markdown](tms9918b_design_notes/TMS9918B_Design_Notes.md) | [PDF](tms9918b_design_notes/TMS9918B_Design_Notes.pdf) |
| **Model** — unit-exact VRAM calendars, DRAM verification, CPU timing, tests | [README](tms9918b_model/README.md) | — |
| **Tutorials** — 33 Z80 ROMs, one per mode and feature, built with sjasmplus | [README](tms9918b_tutorials/README.md) | — |

The two manuals follow the structure and conventions of TI's own documents, the *TMS9918A/9928A/9929A Data
Manual* (MP010A, 1982) and the *Video Display Processors Programmer's Guide* (SPPU004, 1984), including
bit 0 = MSB numbering.

### Quick links

- Registers: [extended registers](tms9918b_data_manual/TMS9918B_Data_Manual.md#22-write-only-registers) ·
  [status registers](tms9918b_data_manual/TMS9918B_Data_Manual.md#23-status-registers) ·
  [palette](tms9918b_data_manual/TMS9918B_Data_Manual.md#24-palette)
- Modes: [display mode table](tms9918b_data_manual/TMS9918B_Data_Manual.md#25-video-display-modes) ·
  [initialization tables and memory maps](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.md#5-initializing-the-extended-modes)
- Hardware: [VRAM interface and memory cycles](tms9918b_data_manual/TMS9918B_Data_Manual.md#31-vdpvram-interface) ·
  [choosing VRAM memory](tms9918b_data_manual/TMS9918B_Data_Manual.md#appendix-a---choosing-vram-memory) ·
  [switching characteristics](tms9918b_data_manual/TMS9918B_Data_Manual.md#52-switching-characteristics-vdp-vram-interface)
- Software: [detecting the TMS9918B](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.md#32-detecting-the-tms9918b) ·
  [sprite pairs](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.md#83-sprite-pairs) ·
  [scanline interrupts](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.md#92-scanline-interrupts) ·
  [Z80 routines](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.md#appendix-d---z80-support-routines)
- Generated data: [VRAM calendars, every cycle of every line](tms9918b_model/docs/generated/calendars.md) ·
  [DRAM data sheet verification](tms9918b_model/docs/generated/dram_timing.md) ·
  [CPU access times](tms9918b_model/docs/generated/cpu_timing.md)

## At a glance

| | TMS9918A | TMS9918B |
|---|---|---|
| Socket, crystal, VRAM board | 40 pins, 10.738635 MHz, 8 × 16K × 1 | **identical** |
| VRAM requirement | 4116-15/-20 class | 120 ns, page mode (Fujitsu MB8118-12, Motorola MCM4517-12) |
| Colours | 15 | 15 TMS colours × 4 luminance levels, 4 palettes of 4 entries |
| Display modes | Graphics I, II, Multicolor, Text (+ undocumented) | all of those, plus Graphics1X, Graphics2Fat, Bitmap, Text40X, Text64 |
| Tiles | 256 (768 with thirds), 1 bit per pixel | 512 two-bit tiles with palette and priority |
| Text | 40 × 24, 2 colours | 40 × 24 with colours per character; 64 × 24 with 768 characters |
| Sprites per line | 4 | 8, with 3-colour sprite pairs and 512 patterns |
| Scrolling | none | horizontal and vertical, left-column mask, locked areas |
| Interrupts | frame | frame and scanline |
| Scroll registers | — | R8 and R9, sampled once per line |
| Worst CPU delay | 29 T-states (Graphics) | 14–18 T-states in extended modes |

## Why it is plausible

| Evidence | Source |
|---|---|
| 1983 SC-3000 boards carry Fujitsu **MB8118-12** (date codes 8331/8332) | board photographs |
| MB8118-12: 270 ns cycle, 145 ns page-mode cycle | Fujitsu MB8118-10/-12 data sheet |
| Other 1983 boards carry Motorola **MCM4517P15** (150 ns) — not enough, hence the 120 ns requirement | board photographs, Motorola MCM4517 data sheet |
| The TMS9918A clock input is already two-phase: memory cycle edges can be placed on half-periods (46.56 ns) | TI MP010A, section 5.4 |
| The TMS9918A memory cycle (372 ns) was sized for the 4116 minimum cycle (375 ns) | TI MP010A section 5.4, TI TMS4116 data sheet |
| TI itself planned an "Advanced Video Display Processor" and asked for reserved bits to stay 0 | TI SPPU004, section 5.1 |

## How the chip works

Every VRAM access is one RAS cycle of **5n + 2 half-periods** reading 1 to 4 adjacent bytes in page mode.
Extended tables store what is fetched together in adjacent bytes (name + attribute, stratum 0 + stratum 1,
character + colour), so a two-byte entry costs 12 half-periods instead of two 8-unit TMS9918A slots.

| Sequence | Group | Content | CPU cycles/line |
|---|---|---|---|
| Graphics1X, Graphics2Fat | 2 cells, 64 units | name+attr, sprite Y, pattern, name+attr, CPU, pattern | 29 |
| Bitmap, BitmapQ | 2 cells, 64 units | bitmap ×4, CPU, palette map ×2, sprite Y, CPU | 45 |
| Text40X | 2 characters, 48 units | char+colour ×2, CPU, pattern, pattern | 67 |
| Text64 | 4 characters, 64 units | names ×4, CPU, 4 patterns, CPU | 70 |
| Horizontal blanking | 344 units | 8 sprites (X+name+colour burst, two pattern halves), CPU | — |

What did not fit was left out rather than forced: 80-column text (50 units needed in 48), colours per character
in 64 columns, 150 ns RAMs.

## Repository layout

```
.
├── COMPATIBILITY.md                     compatibility levels and rules
├── LICENSE                              Apache-2.0 (code)
├── LICENSE-DOCS                         CC BY 4.0 (documents)
├── README.md
├── tms9918b_design_notes/
│   ├── TMS9918B_Design_Notes.md
│   ├── TMS9918B_Design_Notes.pdf        (13 pages)
│   └── source/
├── tms9918b_data_manual/
│   ├── TMS9918B_Data_Manual.md          Markdown edition
│   ├── TMS9918B_Data_Manual.pdf         print edition (18 pages)
│   ├── images/                          figures (SVG)
│   └── source/                          generator of both editions
├── tms9918b_programmers_guide/
│   ├── TMS9918B_Programmers_Guide_Supplement.md
│   ├── TMS9918B_Programmers_Guide_Supplement.pdf   (23 pages)
│   ├── images_guide/                    memory maps (SVG)
│   └── source/
├── tms9918b_tutorials/                  33 lessons, shared includes, build.sh
└── tms9918b_model/
    ├── src/                             calendars, cycle timing, addresses, modes, analysis
    │   └── tms99xx/                     TMS99xx slot grid, schedules, VRAM sequencer
    ├── tests/                           model checks, openMSX address comparison
    ├── tools/                           report generator
    └── docs/generated/                  calendars, DRAM verification, CPU timing
```

## Building

**Documents** (Python 3, ReportLab 4) — each command rebuilds both the PDF and the Markdown edition:

```sh
python3 tms9918b_data_manual/source/build_datasheet.py tms9918b_data_manual/TMS9918B_Data_Manual.pdf
python3 tms9918b_programmers_guide/source/build_guide.py tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.pdf
python3 tms9918b_design_notes/source/build_notes.py tms9918b_design_notes/TMS9918B_Design_Notes.pdf
```

**Model** (C++17, self-contained):

```sh
cd tms9918b_model
g++ -std=c++17 -O2 -Isrc -Isrc/tms99xx tests/test_tms9918b.cpp src/TMS9918BCalendar.cpp src/TMS9918BAnalysis.cpp \
    src/tms99xx/TMS99xxVramSlotSchedule.cpp src/tms99xx/TMS99xxVramSequencer.cpp -o test_tms9918b && ./test_tms9918b
g++ -std=c++17 -O2 -Isrc tests/test_tms99xx_table_address.cpp -o test_address && ./test_address
g++ -std=c++17 -O2 -Isrc -Isrc/tms99xx tools/tms9918b_report.cpp src/TMS9918BCalendar.cpp src/TMS9918BAnalysis.cpp \
    src/tms99xx/TMS99xxVramSlotSchedule.cpp -o report && ./report docs/generated
```

## Status

| Part | State |
|---|---|
| Model and tests | complete — all checks pass |
| Data Manual | complete |
| Programmer's Guide Supplement | complete |
| Design Notes (history, costs, rejected alternatives) | complete |
| Compatibility guidelines | complete |
| Emulator integration guide | planned |

Open assumptions are listed in the [model README](tms9918b_model/README.md#open-points): VDP input setup of
40 ns with a 20 ns board delay budget, and sprite selection over line pairs.

## Implementing the TMS9918B

The specification is open, in the spirit of open instruction-set architectures such as RISC-V: anyone may build
an emulator, an FPGA core or a chip that implements it, under any license. Implementations that want to call
themselves compatible follow the [compatibility guidelines](COMPATIBILITY.md), which define three levels
(TMS9918A mode, TMS9918B functional, TMS9918B timing), the reference tests for each, and conventions for
integrating the chip in hardware and writing software around it.

## License

| Part | License | File |
|---|---|---|
| Reference model, tests, tools, document generators | Apache License 2.0 | [LICENSE](LICENSE) |
| Data Manual, Programmer's Guide Supplement, Design Notes, README and guideline files | Creative Commons Attribution 4.0 (CC BY 4.0) | [LICENSE-DOCS](LICENSE-DOCS) |
| Example programs in the Programmer's Guide | CC BY 4.0 or, at your choice, Apache License 2.0 | — |

Copyright 2026 Saverio Russo. TMS9918A, TMS9928A and TMS9929A are Texas Instruments part numbers, referenced for
compatibility only; this project is not affiliated with or endorsed by Texas Instruments.
