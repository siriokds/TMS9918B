# TMS9918B model — unit-exact VRAM calendars for SC-3000 class boards

The TMS9918B is a hypothetical 1983 revision of the TI TMS9918A designed for the same socket, the same
10.738635 MHz crystal and the same VRAM board as a Sega SC-3000: eight 16K x 1 dynamic RAMs. It is the
16 KB, 8-bit sibling of the TMS9918+ study (which uses two parallel 16 KB banks).

This package models the part that decides whether the chip is possible: the DRAM cycles and the calendars
of every scan line, positioned to the half crystal period and checked against the RAM data sheets.

## Evidence the design rests on

| Item | Source |
|---|---|
| SC-3000 VRAM of 1983 | board photos: Fujitsu MB8118-12, date codes 8331/8332; Motorola MCM4517P15, date codes 8332/8333 |
| MB8118-12 timing (cycle 270 ns, page mode 145 ns) | Fujitsu MB8118-10/-12 data sheet |
| MCM4517-12/-15 timing | Motorola MCM4517 data sheet |
| TMS9918A clock input is two-phase (42-52 ns high/low, 42-52 ns apart) | TI TMS9918A/9928A/9929A Data Manual MP010A, section 5.4 |
| TMS9918A memory cycle 372 ns, VRAM selection method | MP010A section 5.4 and Appendix B |
| CPU to VDP access times format | TI Video Display Processors Programmer's Guide SPPU004, Appendix B |

## Timing model

- **Unit:** half a crystal period, 46.56 ns. Line = 1368 units; TMS99xx phase = 2 units; slot = 8 units;
  Z80 T-state at 3.58 MHz = 6 units. Slot s starts at unit (8s + 1364) mod 1368.
- **One cycle shape, 1 to 4 bytes (page mode):** RAS low [0, 5n-1), precharge 3 units; CAS for byte k low
  [1+5k, 4+5k), sampled on its rising edge; writes are single-byte late writes (WE low [2, 4)), because AD0-AD7 carry the
  address and then the data.
  Lengths: 7, 12, 17, 22 units.
- **VRAM requirement:** 16K x 1, 120 ns, page mode (MB8118-12, MCM4517-12). Every read and write cycle passes
  all data sheet limits with a VDP input setup of 40 ns and a 20 ns board delay budget
  (`docs/generated/dram_timing.md`). 150 ns parts fail on data setup and are not supported, in TMS modes as well,
  because the controller always uses the same cycle.
- **TMS modes:** one 7-unit cycle at the start of every TMS99xx slot, 1 unit idle: CPU windows, latency and
  write losses are those of the TMS9918A (checked against `TMS99xxVramSequencer`).
- **Page mode alignment:** RAS carries A0-A6, CAS A7-A13, so a burst stays in a 128-byte page. Every burst
  address starts at a multiple of its length from a base that is a multiple of 128 (checked exhaustively).
- **Design rules:** a dependent address (name -> pattern, colour/BANK -> sprite pattern) is issued at least
  10 units after the byte it depends on is sampled (the TMS9918A Text calendar uses the same distance);
  the last byte of a cell is sampled at least 4 units before the cell's first pixel (unit 252 graphics,
  276 text).

## Calendars (exact positions in `docs/generated/calendars.md`)

| Calendar | Modes | Per group | Group | CPU cycles/line |
|---|---|---|---|---|
| Tiles | Graphics1X, Graphics2Fat | name,attr (2) / sprite Y / pattern row (2) / name,attr (2) / CPU / pattern row (2) | 2 cells = 64 units from 212 | 29 |
| Bitmap | Bitmap, BitmapQ | bitmap (4) / CPU / palette map (2) / sprite Y / CPU | 2 cells = 64 units from 212 | 45 |
| Text40 | Text40X, Text40XQ | char,colour,char,colour (4) / CPU / pattern / pattern | 2 columns = 48 units from 236 | 67 |
| Text64 | Text64, Text64Q (R12 T64) | names (4) / CPU / 4 patterns / CPU | 4 columns = 64 units from 212 | 70 |
| Blanking (Tiles, Bitmap) | - | 8 sprites: SAT X,name,colour (3) + left + right half, 9 CPU cycles | units 1236-1579 | included |

**Sprite Y scan over two lines:** 16 Y reads per line (sprites 0-15 on even lines, 16-31 on odd lines). The
selection is made for the next two lines and tested against both; the 8-channel limit applies to their union.

## Results

| Mode | Sprites per line | Worst CPU latency | Loss-free loops |
|---|---|---|---|
| Graphics I/II (TMS) | 4 | 29 T | OUTI + JR NZ (28 T) |
| Graphics1X, Graphics2Fat | 8 | 18 T | OUTI + NOP (20 T), OTIR |
| Bitmap, BitmapQ | 8 | 15 T | OUTI chain (16 T) |
| Text40X | cursor | 16 T | OUTI chain (16 T) |
| Text64 (64x24, 8x8, 512 pixels) | cursor | 14 T | OUTI chain (16 T) and every longer loop |
| Text80 (6x8) | - | not available: 4 columns need 50 units in 48 | - |

VRAM usage in 16 KB: Graphics1X 13,952 bytes (512 tiles, 512 sprite patterns with BANK), Graphics2Fat 16,000
(768 tiles, no BANK), Bitmap 15,232 (no BANK), Text40X 4,352, Text64 3,968.

## Building

The model is self-contained: `src/tms99xx/` holds the TMS99xx slot grid, slot schedules and VRAM sequencer used
for the TMS9918A calendars and for validating the loss model.

```
g++ -std=c++17 -O2 -Isrc -Isrc/tms99xx tests/test_tms9918b.cpp src/TMS9918BCalendar.cpp src/TMS9918BAnalysis.cpp \
    src/tms99xx/TMS99xxVramSlotSchedule.cpp src/tms99xx/TMS99xxVramSequencer.cpp -o test_tms9918b
g++ -std=c++17 -O2 -Isrc tests/test_tms99xx_table_address.cpp -o test_address
g++ -std=c++17 -O2 -Isrc -Isrc/tms99xx tools/tms9918b_report.cpp src/TMS9918BCalendar.cpp src/TMS9918BAnalysis.cpp \
    src/tms99xx/TMS99xxVramSlotSchedule.cpp -o report && ./report docs/generated
```

## Files

| File | Content |
|---|---|
| `src/tms99xx/` | TMS99xx slot grid, slot schedules and VRAM sequencer (from GearSF7000, same author) |
| `src/TMS9918BTiming.h` | units, cycle waveform, DRAM data sheet limits, read/write verification |
| `src/TMS9918BCalendar.h/.cpp` | all calendars with exact unit positions |
| `src/TMS9918BModes.h` | decode of every TMS mode (Text 1Q, Multicolor Q, bars included) and extended mode, calendar per mode |
| `src/TMS99xxTableAddress.h` | TMS99xx table addresses for all modes, documented or not (verified against openMSX) |
| `src/TMS9918BAddress.h` | byte-layout table addresses and page-mode alignment |
| `src/TMS9918BAnalysis.h/.cpp` | CPU latency and write-loss analysis |
| `tests/test_tms9918b.cpp` | data sheet compliance, structure, dependencies, pixel deadlines, alignment, TMS compatibility, timing, memory maps, all 64 mode-bit combinations |
| `tests/test_tms99xx_table_address.cpp` | exhaustive comparison of TMS99xxTableAddress with openMSX |
| `tools/tms9918b_report.cpp` | generator of `docs/generated/*.md` |

## Open points

1. The 40 ns input setup and 20 ns board budget are assumptions (the TMS9918A specifies 60 ns setup).
2. Refresh is inherited from the TMS9918A (reads during active lines, Refresh calendar elsewhere).
3. Lines before the first active line must run the active calendar with the picture blanked so that the
   two-line sprite scan is ready for lines 0 and 1.

## License

Apache License 2.0 (see `LICENSE` in the repository root). The files in `src/tms99xx/` were written for
GearSF7000 by the same author and are provided here under the same Apache-2.0 terms.
