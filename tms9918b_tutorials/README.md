# TMS9918B executable tutorial

This directory is both a regression ROM suite and a programming tutorial for
the TMS9918B. Each numbered ROM introduces one observable concept. Programs do
not rotate through unrelated scenes: load one ROM, read its screen and source,
then inspect the corresponding VRAM resources and registers.

The directory is self-contained so it can later be copied to the standalone
TMS9918B repository. Shared hardware definitions, conservative VDP transfer
routines and the Sega 6x8 font live under `include/`.

| Include | Contents |
|---|---|
| `tms9918b_hardware.inc` | ports, legacy table addresses, ROM layout macros |
| `tms9918b_runtime.inc` | conservative register, VRAM and font routines shared by every lesson |
| `tms9918b_extended.inc` | extended register and attribute bit names, `tms9918b_require`, register lists, palette loading, two-bit font expansion, two-byte entry helpers |
| `extended_sprite_overlay.inc` | nine animated sprites with extended colour bytes, shared by the extended graphics lessons |
| `text_cursor.inc` | hardware cursor of the extended text modes, included by the text lessons only |
| `sega_font_6x8.inc` | ASCII 20h..5Ah, ink in bits 7..3 |
| `lesson_legacy_graphics_i.inc`, `legacy_sprite_overlay.inc` | bodies shared by the paired legacy lessons |

Table addresses used by the extended lessons are cross-checked against the
TMS9918B reference model (`src/TMS9918BAddress.h`).

## Build

Install `sjasmplus`, then run:

```sh
./build.sh
```

Every source produces a flat 32 KiB `.sg` ROM with the same base name.

## Numbering

| Range | Subject |
|---|---|
| 000-009 | Power-up, safe detection and extension control |
| 010-049 | Locked-device TMS9918A video-mode regression |
| 050-099 | Locked-device sprite regression |
| 100-199 | Graphics1X |
| 200-299 | Graphics2Fat |
| 300-399 | Bitmap and BitmapQ |
| 400-499 | Text40X and Text40XQ |
| 500-599 | Text64 and Text64Q |
| 600-699 | Extended sprites, pairs, priority and limits |
| 700-799 | Scrolling, masks, interrupts and raster behavior |
| 900-999 | Integrated demonstrations and stress tests |

Gaps are intentional. A new lesson can be inserted near the concept it teaches
without renaming later ROMs.

When a lesson is added, three places are updated together and kept in numerical
order: the lesson index below, the section that describes it under *Lessons*,
and, when it introduces a display mode, the *Mode coverage* table. `build.sh`
runs `check_readme.py` first, which fails the build when a source has no index
row or no section, when the index names a source that does not exist, or when
either list is out of order.

## Lesson index

| Lesson | Program | What it establishes | Key registers / data |
|---:|---|---|---|
| 000 | `000_init_and_detect` | Safe unlock, S1 detection and return to legacy rendering | R63, R11, R15, S1 |
| 010 | `010_legacy_graphics_i` | 32x24 tile map and one colour pair per eight patterns | R2, R3, R4; NAME, PAT, COL |
| 011 | `011_legacy_graphics_i_sprites` | The same Graphics I background with the standard sprite overlay | R5, R6; SAT, SPRPAT |
| 020 | `020_legacy_graphics_ii` | Three screen thirds with independent pattern and colour data | M2, R3/R4 masks; NAME, PAT, COL |
| 021 | `021_legacy_graphics_ii_sprites` | The same Graphics II frame with the standard sprite overlay | R5, R6; SAT, SPRPAT |
| 030 | `030_legacy_multicolor` | 64x48 logical colour blocks and canonical name mapping | M3; NAME, PAT |
| 031 | `031_legacy_multicolor_sprites` | Sprite composition over Multicolor blocks | R5, R6; SAT |
| 032 | `032_legacy_multicolor_q` | Mode 6: Multicolor with a third-banked pattern generator | M3, R4; PAT |
| 040 | `040_legacy_text` | 40x24 characters, 6x8 cells and global colours | M1, R7; NAME, PAT |
| 041 | `041_legacy_text1q` | Mode 3: text geometry with Graphics II pattern banking | M3, R4; NAME, PAT |
| 045 | `045_legacy_bars` | Modes 5 and 7 read no table, locked and with XE/MX set | M1, M2, M3, R11 |
| 050 | `050_legacy_sprites` | Y+1, terminator, early clock, line limit and collision | R1, R5, R6; SAT, SPRPAT, S0 |
| 100 | `100_graphics1x` | Two-byte name entries, sixteen-byte patterns and one palette per tile | R11, R2, R4; NAME/ATTR, PAT, CRAM |
| 101 | `101_graphics1x_sprites` | Nine extended sprites over Graphics1X: the ninth is dropped | R5, R6; SAT, CRAM, S0 |
| 110 | `110_graphics1x_name8_prior` | NAME8 selects 512 patterns; PRIOR puts a tile in front of the sprites | R4; NAME/ATTR, SAT |
| 200 | `200_graphics2fat` | Four dots per pattern row and sixteen colour codes | R0 M3, R4; PAT |
| 201 | `201_graphics2fat_sprites` | The same overlay over a fat-dot playfield | R5, R6; SAT |
| 210 | `210_graphics2fat_banked` | One pattern block per third, R4 = 03h against 00h | R4; PAT |
| 211 | `211_graphics2fat_banked_sprites` | Banked patterns with the overlay: R4 does not move sprite patterns | R4, R6; PAT, SAT |
| 300 | `300_bitmap` | Linear 64-byte lines and one palette per 8x8 area | R1 M2, R2, R4; BITMAP, PALMAP |
| 301 | `301_bitmap_sprites` | Sprites over a bitmap keep their palette across the colour zones | R5, R6; SAT, PALMAP |
| 310 | `310_bitmapq` | One bitmap block per third, R4 = 03h against 00h | M3, R4; BITMAP |
| 311 | `311_bitmapq_sprites` | Banked bitmap blocks with the overlay | R4, R6; BITMAP, SAT |
| 400 | `400_text40x` | Character and colour byte per cell, R7 fallback | R2; ENTRIES, R7 |
| 401 | `401_text40x_cursor` | Hardware cursor at forty columns: X in pixels | R5, R6; SAT |
| 410 | `410_text40xq` | Text40X with a third-banked character generator | M3, R4; PAT |
| 411 | `411_text40xq_cursor` | Hardware cursor over the third-banked forty-column font | R4, R5, R6; SAT |
| 500 | `500_text64` | 64x24 at 512 pixels and the hardware cursor | R12 T64, R5, R6; NAMES, SAT |
| 510 | `510_text64q` | Text64 with a third-banked character generator, 768 characters | M3, R4, R12 T64; PAT |
| 511 | `511_text64q_cursor` | Hardware cursor at 64 columns: X in two-pixel steps | R12 T64, R5, R6; SAT |
| 600 | `600_extended_sprites` | Three-colour pairs, BANK and the eight-channel limit | R5, R6; SAT, CRAM, S0 |
| 700 | `700_scrolling` | R8, R9, MASK, HLOCK and VLOCK on a coordinate grid | R8, R9, R12 |
| 710 | `710_scanline_interrupt` | R10, IE1 and S1: four bands from one playfield | R11 IE1, R10, R15, S1 |

Each entry below explains what to observe before introducing the next lesson.

## Lessons

### 000 — Init and detect

Source: `000_init_and_detect.asm`

This lesson implements the complete safe probe:

1. Start through the legacy TMS9918A register interface with the display off.
2. Write the `5Ah` key to R63 twice.
3. Restore R7, which received both writes while the device was locked.
4. Set `XE` through R11.
5. Select S1 through R15 and use the second status read.
6. Mask the result with `3Eh` and compare it with `18h`.
7. Restore R15, R0 and R7.
8. Disable `XE` and display the result through legacy Graphics I.

Expected TMS9918B result: a green screen containing `PASS` and `S1 ID 18H`.
A solid red screen means that the device did not answer the documented probe.
Running the ROM on an original TMS9918A is safe because every register alias is
restored before the failure screen is enabled.

### 010 — Legacy Graphics I

The top of the screen identifies the mode and its table geometry. The lower
half repeats one shape selected as pattern 64, 72, 80 and 88. Those four names
belong to different eight-pattern groups, so the same geometry appears with
four colour pairs. Pattern Table and Name Table are both conventional legacy
resources in this lesson.

### 011 — Legacy Graphics I with sprites

This lesson keeps every background byte from lesson 010. Four sprites move up
and down together while a fifth remains fixed: the fifth disappears when all
five share a scanline and reappears as the moving group leaves it. Every two
seconds R1 cycles through 8x8, 16x16, magnified 8x8 and magnified 16x16.
Comparing lessons 010 and 011 isolates sprite composition from the background.

### 020 and 021 — Legacy Graphics II

The same name byte, 59, fills three bands. R4 and R3 enable all three 64-line
banks, so each band resolves to a different pattern and a different sequence
of per-row colours. Text is repeated in all three pattern banks as well. This
is the baseline for lesson 210. Lesson 021 keeps every background byte and
adds the sprite overlay of lesson 011, so the four-sprite line limit can be
compared across modes. The overlay prints the current sprite size on character
row 7, which belongs to the first pattern bank.

### 030, 031 and 032 — Legacy Multicolor and Multicolor Q

The screen exposes the complete 64x48 logical block grid. Every pattern byte
contains two colour nibbles, and the canonical name formula repeats every four
character rows. A small sprite caption identifies the lesson without treating
Multicolor data as a text font.

Lesson 031 replaces the caption with the animated overlay: Multicolor has no
character layer, so the overlay label is disabled through
`SPRITE_OVERLAY_LABEL`. Lesson 032 selects mode 6, the undocumented M3 variant:
names and colour blocks behave as in Multicolor, but the pattern generator is
banked per third exactly like Graphics II, so with R4 = 03h the same names
produce three different colour bands.

### 040 — Legacy Text

The screen itself documents the 40x24 geometry. The font keeps all ink in bits
7..3, proving that the visible six-bit shifter does not clip it. The two least
significant pattern bits are ignored and R7 supplies the global ink and paper.

### 041 — Legacy Text 1Q

Mode 3 keeps the 40x24 text geometry, including the 40-name rows and the
`line AND 7` pattern row, but banks the character generator per third. The same
string is printed in the three regions and appears plain, bold and inverse
because only the block changes. There is no colour table: the two colours still
come from R7, which is what separates mode 3 from Graphics II.

### 045 — Legacy bars

Modes 5 and 7 read no table at all, so VRAM, R2 to R6 and the sprite attribute
table have no effect and sprites are not displayed: the lesson cannot label its
own screen. It cycles through four states of two seconds each and records the
current one at `C100h`: mode 5 and mode 7 on a locked device, then the same two
modes with `XE` and `MX` set. The last two states are the point of the lesson,
because the extended device must leave the bar modes exactly as they were.

### 050 — Legacy sprites

The static SAT contains three independent checks. Five sprites meet the first
test line, so four are drawn and sprite 4 is reported as the fifth. A later
sprite uses early clock, and the final pair overlaps opaque pixels. Raw Y `D0h`
then terminates the table. The program accumulates status reads for a complete
frame and publishes the expected `E4h` at CPU address `C100h`.

### 100 — Graphics1X

Every lesson from 100 on begins with `tms9918b_require`, which performs the
lesson 000 probe and stops with a red screen on a device that does not answer:
an extended lesson never runs on a chip that cannot display it.

The top of the screen is written with two-byte entries whose attribute is zero,
so the text uses palette 0. Below, one ring pattern is repeated in four blocks
of six by six cells. The sixteen pattern bytes are identical in every block and
only the two palette bits of the attribute change, so the same geometry appears
with four colour sets. Palette 1 holds one colour at three luminance levels,
which is the clearest way to see what the luminance bits do.

### 101 — Graphics1X with sprites

The background is that of lesson 100. Nine sprites share the same scanlines:
eight are displayed and the ninth is dropped, which is the visible difference
from the four-sprite limit of a locked device. The colour bytes use the
extended format, palette in bits 4..5 and entry in bits 6..7, so the same nine
sprites show every palette and every visible entry; entry 0 would make a sprite
invisible. The last value read from S0 is kept at `C152h`, where 5S and the
number of the dropped sprite can be inspected.

### 110 — NAME8 and PRIOR

The two attribute bits that lesson 100 left alone. The upper half prints the
same name byte twice, once with NAME8 clear and once with it set: the two
halves of the 512-pattern set are independent, so the same byte resolves to a
disc and to a cross. The lower half draws the same band of bars twice, with
PRIOR clear and with PRIOR set, while the sprite overlay crosses both: the
sprites pass in front of the first band and behind the second one, and show
through the transparent column of the pattern in both, because pixel value 0 is
always the backdrop.

### 200 — Graphics2Fat

The cell and the pattern length are those of Graphics1X, but a pattern row
holds four dots two pixels wide, each carrying a four-bit TMS colour code. The
screen is a complete sixteen-colour ramp built from four patterns whose stratum
0 byte is always `1Bh`: only stratum 1 changes. The patterns are stored in all
three pattern thirds, so the ramp is identical on the whole screen. The caption
is made of ordinary one-bit sprites because a fat-dot pattern cannot hold a
readable glyph.

### 201 — Graphics2Fat with sprites

The same overlay over the fat-dot ramp. Sprite composition does not change with
the playfield mode: Graphics2Fat only changes how a pattern row is decoded, so
the eight-sprite limit and the sprite palettes behave exactly as in lesson 101.

### 210 — Graphics2Fat with third banking

Lesson 200 wrote the same patterns into all three blocks. Here each block holds
a different colour mapping while the name bytes stay identical, so the three
bands differ only because of the bank. Every two seconds R4 alternates between
`03h` and `00h`; with `00h` the middle and bottom thirds fall back to the first
block and the screen becomes uniform again.

### 211 — Banked Graphics2Fat with sprites

Lesson 210 with the overlay running while R4 alternates between `03h` and
`00h`. The sprites are unaffected: sprite patterns are located by R6 and are
never banked, so only the playfield changes underneath them.

### 300 — Bitmap

The picture is stored line by line, 64 bytes per line, and built in a RAM
buffer before each transfer. The background is a two-pixel bar sequence 0, 1,
2, 3 repeated across every line; the Palette Map divides the screen into four
vertical zones, so identical pixel values produce four colour sets. A diagonal
drawn with value 3 crosses the zones and proves that the surface is a true
bitmap rather than repeated cells.

### 301 — Bitmap with sprites

Sprites over a bitmap behave as over a tile playfield. The palette of a sprite
comes from its own colour byte and is independent from the Palette Map of the
area it crosses, so the nine sprites keep their colours over all four zones.

### 310 — BitmapQ

The same experiment on the bitmap: with R4 = `03h` every third owns a
4096-byte block and the line inside a block is `Y AND 63`. The three blocks
hold one-, two- and four-pixel bars. The Palette Map is never touched, which
shows that banking moves the picture only, not the colour zones.

### 311 — BitmapQ with sprites

The overlay over the banked bitmap. As in lesson 211, changing R4 moves the
picture only.

### 400 — Text40X

The geometry is that of legacy Text. Each cell now holds a character and a
colour byte, ink in the upper half and paper in the lower half. Five sample
lines print their own colour byte; the last one uses `00h` and therefore
displays exactly like legacy Text, because a zero half falls back to R7.

### 401 — Text40X with the hardware cursor

Sprites 0 and 1 are the only sprites displayed in an extended text mode. At
forty columns the sprite X coordinate is in pixels, so column `c` of a
six-pixel cell is at `X = 6 * c`. The cursor blinks by alternating the entry
bits of its colour byte between 1 and 0, so no pattern is ever rewritten. The
cursor patterns are kept at `0800h`, clear of the character patterns.

### 410 and 510 — Third-banked extended text

`M3` banks the character generator of the extended text modes, so up to 768
characters are available at 40 and at 64 columns. Both lessons print the same
text in the three regions with a plain, a bold and an inverse font, and
alternate R4 between `03h` and `00h` every two seconds. Lesson 410 also writes
a colour byte per cell; lesson 510 uses the two colours of R7 and keeps the
last pixel column of the inverse font clear, because Text64 displays all eight
pattern bits.

### 411 and 511 — Cursor over a banked font

The same cursor over the third-banked text modes, while R4 alternates every two
seconds. The cursor patterns move to `1800h`, clear of the three font blocks,
and the sprite is unaffected by the banking. Lesson 511 shows the 64-column
rule: `X = 4 * column`, because the coordinate counts two-pixel steps.

### 500 — Text64

`T64` in R12 turns the extended text mode into 64 columns of 8x8 characters on
512-pixel lines, with one name byte per cell and the two colours of R7. A
64-character ruler shows that the first and last columns are visible. Sprite 0
is the hardware cursor: its X coordinate counts two-pixel steps, so column `c`
is at `X = 4 * c`, and it blinks by alternating the entry bits of its colour
byte between 1 and 0 without touching VRAM.

### 600 — Pairs, BANK and the channel limit

Three properties of the sprite colour byte on one screen. The first row shows a
pair at the same position, a pair offset by four pixels and the two patterns as
single sprites: where only the even sprite has a pixel the colour is entry 1,
where only the odd one has it entry 2, and where both have it entry 3, all from
the palette of the even sprite. The second row shows the same name byte with
BANK clear and set, which selects the second 2048-byte pattern table. The third
row holds four pairs, that is all eight channels, plus one single sprite that is
dropped; the status byte is published at `C100h`.

### 700 — Scrolling, MASK, HLOCK and VLOCK

A coordinate grid scrolled with R8 and R9 alone: not one VRAM byte is written
while the picture moves. The caption lives in the two locked rows, so HLOCK
keeps it still, and MASK blanks the partial column that pixel scrolling always
leaves at the left edge. The lesson cycles through four states of four seconds:
R8 alone, R9 alone, both together, and both with VLOCK added, which freezes
columns 24 to 31 and turns them into a side panel.

### 710 — Scanline interrupt

`R10 = 47` produces an interrupt at the end of lines 47, 95, 143 and 191. The
handler selects S1, reads it to find FL and clear it, then selects S0 and reads
that too, because the frame interrupt shares the INT pin. Each interrupt paints
the band that begins on the next line by writing R8 and one palette entry, so a
single playfield appears as four bands in four colours and at three scrolling
speeds. The number of scanline interrupts counted in the previous frame is
published at `C170h` and must be four.

## Mode coverage

Every documented and undocumented TMS9918A mode and every TMS9918B mode now has
a lesson, with and without sprites where the mode displays them.

| m | M3 M2 M1 | Locked device | Lesson | Extended device | Lesson |
|---:|---|---|---|---|---|
| 0 | 0 0 0 | Graphics I | 010, **011** | Graphics1X | 100, **101** |
| 1 | 0 0 1 | Text | 040 | Text40X, Text64 | 400, **401**, **500** |
| 2 | 0 1 0 | Graphics II | 020, **021** | Graphics2Fat | 200, **201**, 210, **211** |
| 3 | 0 1 1 | Text 1Q | 041 | Text40XQ, Text64Q | 410, **411**, 510, **511** |
| 4 | 1 0 0 | Multicolor | 030, **031** | Bitmap | 300, **301** |
| 5 | 1 0 1 | bars | 045 | bars, unchanged | 045 |
| 6 | 1 1 0 | Multicolor Q | **032** | BitmapQ | 310, **311** |
| 7 | 1 1 1 | bars | 045 | bars, unchanged | 045 |

Lessons in bold display sprites. Every mode that shows sprites has a pair of
lessons with the same screen, one without and one with them: modes 0, 2, 4 and
6 of a locked device, the extended graphics modes, and the extended text modes
through the hardware cursor. The bar modes display no sprites at all, so they
have a single lesson.

## Feature coverage

Besides the display modes, the suite now covers every extended function of the
Data Manual:

| Function | Lesson |
|---|---|
| Unlock, S1 identification, safe fallback | 000, and `tms9918b_require` in every extended lesson |
| Palette and luminance levels | 100 (one colour at three levels), 300 (four zones) |
| Two-bit patterns and attributes | 100, 110 |
| NAME8 and tile priority | 110 |
| Third banking | 210, 310, 410, 510, and 020, 032, 041 on a locked device |
| Eight sprites per line and the drop | 101, 201, 211, 301, 311, 600 |
| Sprite pairs and BANK | 600 |
| Hardware cursor | 401, 411, 500, 511 |
| Scrolling, MASK, HLOCK, VLOCK | 700 |
| Scanline interrupt and split screens | 710 |

## Planned lessons

- `022`: Graphics II with partial banking and the R3 colour-table leak.
- `900`: an integrated demonstration that changes mode without a reset.

All source comments and on-screen text are written in English so the suite can
be published unchanged with the device documentation.
