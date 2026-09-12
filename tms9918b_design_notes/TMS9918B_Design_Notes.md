# TMS9918B Design Notes

**Why the chip is the way it is - Team B Europe, TB-9918B-03, September 2026**

*Hypothetical 1983 device - design study. Not a Texas Instruments product. Copyright 2026 Saverio Russo - licensed under CC BY 4.0.*

## Table of Contents

- [1. THE QUESTION](#1-the-question) (1-1)
  - [1.1 Rules of the Design](#11-rules-of-the-design) (1-1)
- [2. THE 1983 SETTING](#2-the-1983-setting) (2-1)
  - [2.1 What Texas Instruments Planned](#21-what-texas-instruments-planned) (2-1)
  - [2.2 What Sega Did](#22-what-sega-did) (2-1)
- [3. TWO ANSWERS](#3-two-answers) (3-1)
- [4. THE MEMORY DECIDES EVERYTHING](#4-the-memory-decides-everything) (4-1)
  - [4.1 The TMS9918A Was Sized for the 4116](#41-the-tms9918a-was-sized-for-the-4116) (4-1)
  - [4.2 The RAMs on a 1983 SC-3000](#42-the-rams-on-a-1983-sc-3000) (4-1)
  - [4.3 The Clock Was Already There](#43-the-clock-was-already-there) (4-1)
  - [4.4 The Memory Cycle](#44-the-memory-cycle) (4-1)
- [5. SPENDING THE BANDWIDTH](#5-spending-the-bandwidth) (5-1)
  - [5.1 The Line-Pair Sprite Scan](#51-the-line-pair-sprite-scan) (5-1)
  - [5.2 What Did Not Fit](#52-what-did-not-fit) (5-1)
  - [5.3 A Faster CPU Interface](#53-a-faster-cpu-interface) (5-1)
- [6. FEATURES AND THEIR PRECEDENTS](#6-features-and-their-precedents) (6-1)
- [7. WHAT WAS LEFT OUT](#7-what-was-left-out) (7-1)
- [8. COST](#8-cost) (8-1)
  - [8.1 Transistors](#81-transistors) (8-1)
  - [8.2 Die and Board](#82-die-and-board) (8-1)
- [9. VERIFICATION](#9-verification) (9-1)
  - [9.1 Open Assumptions](#91-open-assumptions) (9-1)
- [10. AN OPEN SPECIFICATION](#10-an-open-specification) (10-1)
- [APPENDIX A - SOURCES](#appendix-a---sources) (A-1)

## 1. THE QUESTION

In 1983 the TMS9918A was four years old and inside a large family of machines: the TI-99/4A, the ColecoVision, the Sega SG-1000 and SC-3000, the first MSX computers. In the same months Nintendo shipped the Famicom with a palette-based picture processor and eight sprites per line. Sega answered only in 1985 with the Mark III, whose video chip descends from the TMS9918A.

These notes record how one question was answered: **what could a revision of the TMS9918A have offered in 1983, in the same socket, with the same crystal and on the same boards?** The answer is the TMS9918B. The Data Manual says what the chip does; the Programmer's Guide Supplement says how to use it; this document says why.

### 1.1 Rules of the Design

- **Nothing changes until software asks.** After reset the chip is a TMS9918A, including undocumented modes and CPU access windows.
- **Measured constraints, not estimates.** Every memory cycle is checked against the data sheet of RAMs found on real 1983 boards.
- **A precedent for every feature.** Each function exists in a chip of 1982-1985 or follows a practice documented at the time.
- **Only what software cannot do goes into silicon.** Sprite flipping and colour-per-row text were left to software; palettes, sprite planes and scrolling were not.
- **What does not fit is left out, not forced.** 80-column text and colours per character in 64 columns were removed when the timing model showed they did not fit.

## 2. THE 1983 SETTING

| Year | Device or machine | What it shows |
|---|---|---|
| 1979 | TMS9918 (TI-99/4) | 16 KB dynamic VRAM, 171 memory cycles per line, 4 sprites per line |
| 1981 | TMS9918A (TI-99/4A) | Graphics II mode; the chip sold to the rest of the industry |
| 1982 | ColecoVision; Commodore 64 | TMS9918A console; "multicolour" fat pixels with 2 bits per double-width pixel |
| 1983 | SG-1000 and SC-3000 (July), MSX, Famicom (July) | TMS9918A everywhere; the Famicom PPU brings 4 x 4 palettes, 8 sprites per line and a 21.477 MHz clock |
| 1984 | SG-1000 II (July), TMS9118, Super Cassette Vision | Sega 315-5066 integrates VDP, decoding and sound with two 16K x 4 VRAMs; TI moves to 4-bit VRAM; Epoch combines sprites |
| 1985 | Mark III, Yamaha V9938 (MSX2) | Sega 315-5124: palette, scrolling, line interrupt, 8 sprites; V9938: 1-bit sprites combined by the CC bit |

**TABLE 2-1 - TIMELINE**

### 2.1 What Texas Instruments Planned

TI's Programmer's Guide of August 1984 already speaks of a "next generation Advanced Video Display Processor": unused register bits must be written as 0 to stay compatible with it, and Multicolor mode will not be supported. The TMS9918B follows both hints: reserved bits stay reserved, and the extended modes replace Multicolor with a linear bitmap.

### 2.2 What Sega Did

Sega's path shows how quickly the TMS9918A became a constraint. Some SG-1000 II units of 1984 replace the TMS9918A with the Sega 315-5066, a 64-pin part that integrates the VDP, decoding and sound and uses two MB81416 16K x 4 RAMs as VRAM. The Mark III of 1985 adds the features the TMS9918A lacked. The TMS9918B asks what could have been done a year or two earlier, without a new board.

## 3. TWO ANSWERS

The study produced two designs. The first, the TMS9918+, doubles the VRAM; the second, the TMS9918B, keeps it. The TMS9918B became the reference design because it needs no change to existing boards.

|  | TMS9918+ | TMS9918B |
|---|---|---|
| VRAM | 2 x 16 KB read in parallel (16-bit word per cycle) | 16 KB, 8-bit, adjacent bytes read in page mode |
| Board | 16 RAM chips, about 49 pins, new board | same board, same socket; 120 ns RAMs required |
| Memory cycle | TMS9918A cycle (372 ns) | half-period timing, 7 units single, 5n + 2 units for n bytes |
| Extra board cost (1983) | about $9.44-10.00 of RAM | none when the fitted RAMs qualify |
| Extended modes | tiles, fat pixels, bitmap, Text 40, Text 80 | tiles, fat pixels, bitmap, Text 40, Text 64 |
| Sprites per line | 8 | 8 (selection over line pairs) |
| Worst CPU delay, tile modes | 24 T | 18 T |

**TABLE 3-1 - THE TWO DESIGNS**

The TMS9918+ remains a valid alternative for a new machine. The rest of these notes concern the TMS9918B.

## 4. THE MEMORY DECIDES EVERYTHING

### 4.1 The TMS9918A Was Sized for the 4116

The TMS9918A performs one memory cycle every 372 ns (MP010A section 5.4). The minimum cycle time of TI's own TMS4116, in the -15 and -20 grades alike, is 375 ns. The VDP therefore ran its RAM at its limit, and TI's Appendix B verified the chip with -15 and -20 parts and excluded -25 parts. A smarter controller could not have read more bytes from a 4116.

### 4.2 The RAMs on a 1983 SC-3000

Board photographs of 1983 SC-3000 units show eight **Fujitsu MB8118-12** RAMs with date codes 8331 and 8332. The MB8118 is a single +5 V 16K x 1 RAM with a 270 ns minimum cycle, a 145 ns page-mode cycle and 120 ns access time. The TMS9918A used it with its 372 ns cycle, leaving about 27 % of the RAM speed unused. MP010A already allowed single +5 V RAMs.

Other boards of the same months carry **Motorola MCM4517P15** RAMs (date codes 8332 and 8333): 320 ns cycle, 150 ns access. Sega bought from several suppliers because the TMS9918A accepted any of these parts. A faster VDP must choose: the TMS9918B requires 120 ns page-mode RAM, as TI's own Appendix B once excluded the 4116-25.

### 4.3 The Clock Was Already There

MP010A specifies the external clock as two phases, XTAL1 and XTAL2, each high and low for 42-52 ns and 42-52 ns apart. The TMS9918A thus receives both edges of the 10.738635 MHz clock. The TMS9918B places its RAS and CAS edges on these half-periods (46.56 ns, one "unit") and needs no new crystal. An early plan to use a 21.477 MHz crystal, as the Famicom did, became unnecessary.

### 4.4 The Memory Cycle

Every access is one RAS pulse reading 1 to 4 adjacent bytes: RAS low for 5n - 1 units, precharge 3 units, CAS low for 3 units per byte with 2 units between bytes. Writes are single-byte late writes because the TMS9918A address and data share the AD bus. With a 40 ns VDP input setup and a 20 ns board budget, the tightest MB8118-12 margins are RAS precharge (20 ns) and write-to-RAS lead (28 ns).

| Variant considered | Result | Why |
|---|---|---|
| V9938-style 6-unit single cycle | rejected | RAS low 139.7 ns against 140 ns minimum |
| 4 units per page-mode byte | rejected | data valid only 28 ns before latching, less than setup plus board delays |
| Early write | rejected | the AD bus still carries the column address when CAS falls |
| 21.477 MHz crystal | not needed | the two-phase clock input already provides half-periods |
| Two cycle shapes (TMS and extended) | rejected | one controller for every mode; TMS modes run 7-unit cycles at slot starts |
| 5n + 2 units, late write | chosen | all MB8118-12 and MCM4517-12 limits met |

**TABLE 4-1 - MEMORY CYCLE ALTERNATIVES**

## 5. SPENDING THE BANDWIDTH

A line has 1368 units. The TMS9918A spends them as 171 identical slots; the TMS9918B spends them as bursts of adjacent bytes. The design of every extended table follows from one rule: **store together what is read together**. A tile's name and attribute are adjacent, a pattern row's two strata are adjacent, a character and its colour are adjacent, four text names are adjacent. Two bytes then cost 12 units instead of 16.

### 5.1 The Line-Pair Sprite Scan

Eight sprites need 40 SAT and pattern reads in the horizontal blanking, which fits. Scanning 32 Y positions on every line does not: the tile calendar leaves only two single reads per 64-unit cell pair. The TMS9918B scans sprites 0-15 on even lines and 16-31 on odd lines and uses the selection for the next two lines. Sprites are at least 8 lines tall, so the only visible effect is that the eight-sprite limit counts the sprites of both lines of a pair.

### 5.2 What Did Not Fit

| Feature | Needed | Available | Decision |
|---|---|---|---|
| 80 columns of 6 x 8 characters | 50 units per 4 columns | 48 units | replaced by 64 columns of 8 x 8 characters |
| Colours per character in 64 columns | 70 units per 4 columns | 64 units | two colours from R7 |
| 8 sprites with 150 ns RAMs | page-mode setup margin | negative | 120 ns RAM requirement |
| CPU slot in every tile cell | 69 units per cell pair | 64 units | one CPU cycle per cell pair (18 T) |

**TABLE 5-1 - FEATURES REMOVED BY THE TIMING MODEL**

### 5.3 A Faster CPU Interface

The same bursts leave more room for the CPU than the TMS9918A calendars did: 18 T-states worst case in tile modes against 29 in Graphics mode, and loss-free OUTI chains in Bitmap, Text40X and Text64. For Text64 the CPU cycles were placed first, as a terminal mode is limited by how fast text can be written.

## 6. FEATURES AND THEIR PRECEDENTS

| Feature | Precedent | Why it is in the chip |
|---|---|---|
| Palette: 4 palettes x 4 entries | Famicom PPU (1983) | two reads per tile instead of three; colours remain the TMS9918A ones |
| Four luminance levels | half-bright colours (Amiga EHB, 1985) | about 57 colours and real grey ramps for two bits per entry |
| Graphics1X: 512 two-bit tiles | Famicom, Mark III (512 tiles) | arcade-quality playfields in 16K |
| Graphics2Fat: 16 colours per fat dot | Commodore 64 multicolour (1982) | Graphics II colour detail without a colour table |
| Linear bitmap | TI: Multicolor dropped from the Advanced VDP | title screens and plotting; the bitmap scrolls |
| Text40X: colours per character | home computer colour text of the period | a free byte next to each name |
| Text64 | terminals with narrow characters | 80 columns did not fit; 64 x 24 at 512 pixels did |
| 8 sprites per line | Famicom (1983), V9938 (1985) | fits the blanking with SAT bursts |
| Sprite pairs (PAIR) | TI Programmer's Guide sprite overlay (section 10.2); V9938 CC bit | three colours and meaningful collisions for about 90 transistors |
| BANK: 512 sprite patterns | Mark III: sprites use either of two 256-pattern sets | mirrored frames stored instead of hardware flips |
| Tile priority | Famicom, Mark III | software cannot put tiles in front of sprites |
| Scroll registers, MASK, locks | Mark III (1985) | pixel scrolling without VRAM writes |
| Scanline interrupt | Mark III, simplified | split screens, status bars, palette changes |
| Hardware text cursor | terminal practice | blinking without rewriting characters |
| Register 63 unlock, S1 = 18h | F18A and PICO9918 conventions | coexists with existing detection code |

**TABLE 6-1 - FEATURES**

## 7. WHAT WAS LEFT OUT

| Alternative | Reason |
|---|---|
| Sprite and tile flip bits | software stores mirrored patterns; BANK provides the room; about 500 transistors saved |
| Two-bit native sprites | more logic per channel and a new pattern format; PAIR gives three colours with the TMS format |
| Colour table in extended modes | a third read per tile, no bandwidth for eight sprites |
| RGB palette | new analogue output; the TMS9918A colours were kept |
| Colours per text row in hardware | about 1,150 transistors; the scanline interrupt can change R7 |
| NEGATIVE character bit in Text64 | would halve the character set; not needed |
| Readable vertical counter | every raster effect is possible with the scanline interrupt |
| Exact Mark III line counter | its reload and line 192 details serve no 1983 software |
| Unlock through R15 | R15 is the status register select of the V9938 and F18A |
| A second VRAM bank | kept for the TMS9918+; it needs a new board |
| 150 ns RAM support | negative data setup margin in page mode |

**TABLE 7-1 - REJECTED ALTERNATIVES**

## 8. COST

### 8.1 Transistors

Estimates per block for 1983 depletion-load NMOS, within about 50 %. For scale, the 6502 has about 3,500 transistors.

| Block | Transistors |
|---|---|
| Memory cycle generator, bursts and calendars | ~600 |
| Unlock, R11-R15, status select, S1 | ~300 |
| Palette (96 bits) and luminance scaling | ~810 |
| Eight sprite channels, PAIR, row latches, BANK, line-pair scan | ~1,520 |
| Scroll, MASK, locks | ~750 |
| Scanline interrupt | ~100 |
| Extended modes (NAME8, fat dots, bitmap, text colours, Text64, cursor) | ~525 |
| Half-period clock timing | ~30 |
| Total | ~4,600 |

**TABLE 8-1 - TRANSISTOR BUDGET**

### 8.2 Die and Board

The number of transistors of the TMS9918A is not published. With a Poisson yield model, adding about 4,600 transistors raises the die cost by about 25 % (if the TMS9918A has 20,000) to about 65 % (if it has 10,000). The package is unchanged. A board whose VRAM already meets the 120 ns requirement costs nothing more; other boards need faster RAMs, whose 1983 price difference is not documented (for the 4116, the -15 grade cost 6 % more than the -20 grade in August 1983).

The real limit in 1983 was not money but time: a new NMOS chip took one to two years. Sega reached comparable features in 1985.

## 9. VERIFICATION

The reference model checks every statement of the Data Manual that concerns timing, addresses or modes.

| Check | Result |
|---|---|
| Read and write cycles against MB8118-12 and MCM4517-12 limits | all pass; MCM4517-15 fails data setup |
| Cycle overlaps, dependencies (10 units), pixel deadlines (4 units) | all pass in every calendar |
| Page-mode alignment of every burst address | all addresses stay in their 128-byte page |
| TMS modes against the GearSF7000 sequencer | same CPU windows and write losses; 29, 12 and 13 T as in TI's table |
| All 64 combinations of M1, M2, M3, XE, MX, T64 | expected mode, calendar and sprite count |
| TMS table addresses against openMSX | 0 differences |
| VRAM maps of every extended mode | fit in 16384 bytes |

**TABLE 9-1 - MODEL CHECKS**

### 9.1 Open Assumptions

- VDP input setup of 40 ns and a board delay budget of 20 ns (the TMS9918A specifies 60 ns setup).
- The TMS9918A behaviour used as reference is GearSF7000's cycle model; real hardware measurements are still needed.
- Refresh is inherited from the TMS9918A.

## 10. AN OPEN SPECIFICATION

The TMS9918B is published as an open specification in the spirit of RISC-V: the documents are licensed under CC BY 4.0 and the reference model under Apache-2.0, so anyone may implement the chip in an emulator, an FPGA or silicon. Compatibility is defined by guidelines instead of licence conditions: three levels (TMS9918A mode, TMS9918B functional, TMS9918B timing), the reference tests for each, and conventions for hardware integration and software, similar in purpose to the rules that let MSX machines of different makers run the same programs.

The name keeps TI's part number style to place the design in its family; it is not a trademark and implies no endorsement by Texas Instruments.

## APPENDIX A - SOURCES

| Source | Used for |
|---|---|
| Texas Instruments, TMS9918A/TMS9928A/TMS9929A Video Display Processors Data Manual, MP010A, November 1982 | memory cycle, clock input, VRAM selection, register and address definitions |
| Texas Instruments, Video Display Processors Programmer's Guide, SPPU004, August 1984 | access time table, Advanced VDP notes, TMS9118 with TMS4416, sprite overlay |
| Fujitsu, MB8118-10/MB8118-12 NMOS 16,384-bit Dynamic RAM data sheet | timing limits of the reference VRAM |
| Motorola, MCM4517 16,384-bit Dynamic RAM data sheet | timing limits of -12 and -15 grades |
| Texas Instruments, TMS4116 data sheet | 375 ns minimum cycle |
| Photographs of 1983 SC-3000 VRAM areas | MB8118-12 (8331, 8332) and MCM4517P15 (8332, 8333) |
| Leaded Solder, "An SG-1000 II gets a second chance", 2022 | Sega 315-5066 with two MB81416 RAMs and RGB markings |
| Nicole Express, articles on the SG-1000 II and Sega VDPs | 315-5066 as the first integrated Sega VDP |
| Microcomputing, August 1983; BYTE, April 1983; JDR Microdevices, January 1989 | RAM prices |
| openMSX VDP source code | reference for TMS99xx table addresses |

**TABLE A-1 - SOURCES**
