/*
 * tms9918b - TMS9918A and TMS9918B video display processor
 *
 * Copyright 2026 Saverio Russo
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sections, in order:
 *
 *   1. state and reset
 *   2. ports
 *   3. extended registers, unlock and palette
 *   4. table addresses
 *   5. TMS9918A display modes
 *   6. TMS9918B display modes
 *   7. sprites
 *   8. line counter and interrupts
 *   9. access timing
 */

#include "tms9918b.h"

#include <string.h>

#define VRAM_MASK       (TMS9918B_VRAM_SIZE - 1)

/* R1 */
#define R1_16K          0x80
#define R1_BLANK        0x40
#define R1_IE           0x20
#define R1_M1           0x10
#define R1_M2           0x08
#define R1_SIZE         0x02
#define R1_MAG          0x01

/* R0 */
#define R0_M3           0x02

/* S0 */
#define S0_INT          0x80
#define S0_5S           0x40
#define S0_COINCIDENCE  0x20

/* the unlock command: two consecutive writes of this value to this register */
#define UNLOCK_REGISTER 59
#define UNLOCK_VALUE    0x5a

/* R11, the mode register */
#define R11_IE1         0x80
#define R11_MX          0x02
#define R11_XE          0x01

/* R12, the screen register */
#define R12_HLOCK       0x08
#define R12_VLOCK       0x04
#define R12_MASK        0x02
#define R12_T64         0x01

/* the name table attribute byte of the extended tile modes */
#define ATTR_NAME8      0x08
#define ATTR_PRIOR      0x04
#define ATTR_PALETTE    0x03

/* the sprite colour byte */
#define SPR_EC          0x80
#define SPR_XFINE       0x40    /* low bit of X in a 512-pixel mode */
#define SPR_BANK        0x20
#define SPR_PAIR        0x10
#define SPR_PALETTE     0x0c
#define SPR_ENTRY       0x03

#define EXT(vdp, reg)   ((vdp)->ext[(reg) - 8])

/* ===========================================================================
 * 1. State and reset
 * ========================================================================= */

void tms9918b_init(tms9918b* vdp)
{
    if (vdp == NULL) return;

    memset(vdp, 0, sizeof(*vdp));
}

void tms9918b_reset(tms9918b* vdp)
{
    if (vdp == NULL) return;

    vdp->registers[0] = 0;
    vdp->registers[1] = 0;
    vdp->status = 0;
    vdp->address = 0;
    vdp->readAhead = 0;
    vdp->haveFirstByte = false;

    vdp->unlocked = false;
    vdp->keyArmed = false;
    vdp->paletteWrite = false;
    EXT(vdp, 11) = 0;
    EXT(vdp, 12) = 0;
    vdp->lineIrq = false;

    /* the palette and the other extended registers are undefined after a
       power-up; software loads them before it uses an extended mode */
}

bool tms9918b_display_enabled(const tms9918b* vdp)
{
    return (vdp->registers[1] & R1_BLANK) != 0;
}

/* is the extended interface enabled? (XE in R11) */
static bool extended_enabled(const tms9918b* vdp)
{
    return vdp->unlocked && (EXT(vdp, 11) & R11_XE) != 0;
}

/* are the extended display modes selected? (XE and MX) */
static bool extended_modes(const tms9918b* vdp)
{
    if (!vdp->unlocked) return false;

    return (EXT(vdp, 11) & (R11_XE | R11_MX)) == (R11_XE | R11_MX);
}

/* ===========================================================================
 * 2. Ports
 *
 * A control transfer is two bytes. The second one says what the pair meant: a
 * register write, a VRAM address for reading or writing, or - with the
 * extension enabled - a palette address.
 * ========================================================================= */

static bool write_register(tms9918b* vdp, uint8_t reg, uint8_t value);
static bool write_palette_address(tms9918b* vdp, uint8_t first, uint8_t second);
static bool write_palette_data(tms9918b* vdp, uint8_t value);

void tms9918b_write_control(tms9918b* vdp, uint8_t value)
{
    if (vdp == NULL) return;

    if (!vdp->haveFirstByte)
    {
        vdp->firstByte = value;
        vdp->haveFirstByte = true;

        /* the low byte of an address reaches the pointer at once, as it does
           on the real device */
        vdp->address = (uint16_t)((vdp->address & 0x3f00) | value);
        return;
    }

    vdp->haveFirstByte = false;

    if (write_palette_address(vdp, vdp->firstByte, value)) return;

    if (value & 0x80)
    {
        if (!write_register(vdp, (uint8_t)(value & 0x3f), vdp->firstByte))
        {
            /* while locked, a register number decodes three bits: register 59
               lands in R3 and register 11 lands in R3 as well */
            vdp->registers[value & 0x07] = vdp->firstByte;
        }
        return;
    }

    vdp->paletteWrite = false;
    vdp->address = (uint16_t)(vdp->firstByte | ((value & 0x3f) << 8));

    if ((value & 0x40) == 0)
    {
        /* a read address: the device fetches the first byte immediately */
        vdp->readAhead = vdp->vram[vdp->address & VRAM_MASK];
        vdp->address = (uint16_t)((vdp->address + 1) & VRAM_MASK);
    }
}

void tms9918b_write_data(tms9918b* vdp, uint8_t value)
{
    if (vdp == NULL) return;

    vdp->haveFirstByte = false;

    if (write_palette_data(vdp, value)) return;

    vdp->readAhead = value;
    vdp->vram[vdp->address & VRAM_MASK] = value;
    vdp->address = (uint16_t)((vdp->address + 1) & VRAM_MASK);
}

uint8_t tms9918b_read_data(tms9918b* vdp)
{
    uint8_t value;

    if (vdp == NULL) return 0;

    vdp->haveFirstByte = false;
    value = vdp->readAhead;
    vdp->readAhead = vdp->vram[vdp->address & VRAM_MASK];
    vdp->address = (uint16_t)((vdp->address + 1) & VRAM_MASK);

    return value;
}

uint8_t tms9918b_read_status(tms9918b* vdp)
{
    uint8_t value;

    if (vdp == NULL) return 0;

    vdp->haveFirstByte = false;

    /* R15 selects the register a read returns */
    if (extended_enabled(vdp) && (EXT(vdp, 15) & 0x0f) == 1)
    {
        value = (uint8_t)(0x18 | (vdp->lineIrq ? 0x01 : 0x00));
        vdp->lineIrq = false;
        return value;
    }

    value = vdp->status;
    vdp->status = 0;

    return value;
}

/* ===========================================================================
 * 3. Extended registers, unlock and palette
 * ========================================================================= */

static bool write_register(tms9918b* vdp, uint8_t reg, uint8_t value)
{
    if (!vdp->unlocked)
    {
        /* two consecutive writes of 5Ah to register 59, with no other register
           write in between. The write itself still lands in the aliased legacy
           register, so the caller is told to perform it. */
        const bool key = (reg == UNLOCK_REGISTER) && (value == UNLOCK_VALUE);

        vdp->unlocked = key && vdp->keyArmed;
        vdp->keyArmed = key;
        return false;
    }

    if (reg >= 8 && reg < 16)
    {
        vdp->ext[reg - 8] = value;

        /* writing the line count reloads the counter at once */
        if (reg == 10) vdp->lineCounter = value;

        return true;
    }

    if (reg >= 16) return true;     /* no register there: discarded */

    return false;                   /* 0 - 7: a legacy register */
}

static bool write_palette_address(tms9918b* vdp, uint8_t first, uint8_t second)
{
    if ((second & 0xc0) != 0xc0) return false;
    if (!extended_enabled(vdp)) return false;

    vdp->paletteIndex = (uint8_t)(first & 0x0f);
    vdp->paletteWrite = true;

    return true;
}

static bool write_palette_data(tms9918b* vdp, uint8_t value)
{
    if (!vdp->paletteWrite || !extended_enabled(vdp)) return false;

    vdp->palette[vdp->paletteIndex] = (uint8_t)(value & 0x3f);
    vdp->paletteIndex = (uint8_t)((vdp->paletteIndex + 1) & 0x0f);

    return true;
}

/* The fifteen colours of the TMS9918A, at eight bits per channel. Colour 0 is
 * transparent and is shown as black here, which is what a machine without an
 * external video source displays. */
static const uint8_t tms_colours[16][3] = {
    {   0,   0,   0 }, {   0,   0,   0 }, {  33, 200,  66 }, {  94, 220, 120 },
    {  84,  85, 237 }, { 125, 118, 252 }, { 212,  82,  77 }, {  66, 235, 245 },
    { 252,  85,  84 }, { 255, 121, 120 }, { 212, 193,  84 }, { 230, 206, 128 },
    {  33, 176,  59 }, { 201,  91, 186 }, { 204, 204, 204 }, { 255, 255, 255 }
};

void tms9918b_rgb(uint8_t pixel, uint8_t* r, uint8_t* g, uint8_t* b)
{
    /* the luminance bits scale the colour: 00 full, 01 three quarters,
       10 a half, 11 a quarter */
    static const uint16_t scale[4] = { 4, 3, 2, 1 };

    const uint8_t colour = (uint8_t)(pixel & 0x0f);
    const uint16_t factor = scale[(pixel >> 4) & 0x03];

    if (r) *r = (uint8_t)(tms_colours[colour][0] * factor / 4);
    if (g) *g = (uint8_t)(tms_colours[colour][1] * factor / 4);
    if (b) *b = (uint8_t)(tms_colours[colour][2] * factor / 4);
}

/* ===========================================================================
 * 4. Table addresses
 *
 * These follow the TMS9918A to the letter, undocumented modes included: with
 * M3 set the pattern generator is banked per screen third, and in Graphics II
 * the low bits of R3 mask the name as well.
 * ========================================================================= */

static uint16_t name_table(const tms9918b* vdp)
{
    return (uint16_t)((vdp->registers[2] & 0x0f) << 10);
}

static uint16_t sprite_attr_table(const tms9918b* vdp)
{
    return (uint16_t)((vdp->registers[5] & 0x7f) << 7);
}

static uint16_t sprite_pattern_table(const tms9918b* vdp)
{
    return (uint16_t)((vdp->registers[6] & 0x07) << 11);
}

/* the page offset of the screen third holding this tile row: 0, 800h or 1000h */
static uint16_t page_offset(const tms9918b* vdp, uint16_t line)
{
    const uint16_t third = (uint16_t)((line >> 6) & (vdp->registers[4] & 0x03));

    return (uint16_t)(third << 11);
}

/* ===========================================================================
 * 5. TMS9918A display modes
 * ========================================================================= */

/* M2 * 4 + M3 * 2 + M1, the mode number of the TI documents */
static uint8_t mode_number(const tms9918b* vdp)
{
    uint8_t mode = 0;

    if (vdp->registers[1] & R1_M1) mode |= 0x01;
    if (vdp->registers[0] & R0_M3) mode |= 0x02;
    if (vdp->registers[1] & R1_M2) mode |= 0x04;

    return mode;
}

tms9918b_mode tms9918b_display_mode(const tms9918b* vdp)
{
    const uint8_t mode = mode_number(vdp);

    if (!extended_modes(vdp))
    {
#if TMS9918B_UNDOCUMENTED
        return (tms9918b_mode)mode;
#else
        /* the reading most emulators use: M3 means Graphics II, and the two
           bar modes fall back to Graphics I */
        switch (mode)
        {
            case 1:  return TMS9918B_MODE_TEXT;
            case 2:
            case 3:
            case 6:
            case 7:  return TMS9918B_MODE_GRAPHICS_II;
            case 4:  return TMS9918B_MODE_MULTICOLOR;
            default: return TMS9918B_MODE_GRAPHICS_I;
        }
#endif
    }

    if ((mode == 1 || mode == 3) && (EXT(vdp, 12) & R12_T64))
    {
        return (mode == 1) ? TMS9918B_MODE_TEXT64 : TMS9918B_MODE_TEXT64Q;
    }

    return (tms9918b_mode)(TMS9918B_MODE_GRAPHICS_1X + mode);
}

uint16_t tms9918b_line_width(const tms9918b* vdp)
{
    const tms9918b_mode mode = tms9918b_display_mode(vdp);

    return (mode == TMS9918B_MODE_TEXT64 || mode == TMS9918B_MODE_TEXT64Q)
        ? TMS9918B_PIXELS_WIDE : TMS9918B_PIXELS;
}

static uint8_t backdrop_colour(const tms9918b* vdp)
{
    return (uint8_t)(vdp->registers[7] & 0x0f);
}

/* Graphics I: one colour byte per group of eight patterns.
 * Graphics II: a colour byte per pattern row, and both tables banked. */
static void render_graphics(tms9918b* vdp, uint16_t line, uint8_t* pixels, bool mode2)
{
    const uint16_t names = (uint16_t)(name_table(vdp) + (line >> 3) * 32);
    const uint16_t patternBase = (uint16_t)((vdp->registers[4] & (mode2 ? 0x04 : 0x07)) << 11);
    const uint16_t colourBase  = mode2
        ? (uint16_t)((vdp->registers[3] & 0x80) << 6)
        : (uint16_t)(vdp->registers[3] << 6);
    const uint16_t page = mode2 ? page_offset(vdp, line) : 0;
    const uint16_t nameMask = mode2
        ? (uint16_t)(((vdp->registers[3] & 0x1f) << 3) | 0x07) : 0xff;

    uint16_t column;

    for (column = 0; column < 32; ++column)
    {
        const uint8_t name = vdp->vram[(names + column) & VRAM_MASK];
        const uint16_t index = mode2 ? (uint16_t)(name & nameMask) : name;

        const uint8_t pattern = vdp->vram[(patternBase + page + 8 * index + (line & 7)) & VRAM_MASK];
        const uint8_t colour = mode2
            ? vdp->vram[(colourBase + page + 8 * index + (line & 7)) & VRAM_MASK]
            : vdp->vram[(colourBase + (name >> 3)) & VRAM_MASK];

        const uint8_t fg = (uint8_t)(colour >> 4);
        const uint8_t bg = (uint8_t)(colour & 0x0f);
        uint8_t bit;

        for (bit = 0; bit < 8; ++bit)
        {
            const uint8_t value = ((pattern << bit) & 0x80) ? fg : bg;

            pixels[column * 8 + bit] = value ? value : backdrop_colour(vdp);
        }
    }
}

/* Multicolor: four-pixel blocks, two per pattern byte, four rows per byte */
static void render_multicolor(tms9918b* vdp, uint16_t line, uint8_t* pixels, bool banked)
{
    const uint16_t names = (uint16_t)(name_table(vdp) + (line >> 3) * 32);
    const uint16_t patternBase = (uint16_t)((vdp->registers[4] & (banked ? 0x04 : 0x07)) << 11);
    const uint16_t page = banked ? page_offset(vdp, line) : 0;
    const uint16_t row = (uint16_t)((line >> 2) & 0x07);

    uint16_t column;

    for (column = 0; column < 32; ++column)
    {
        const uint8_t name = vdp->vram[(names + column) & VRAM_MASK];
        const uint8_t byte = vdp->vram[(patternBase + page + 8 * name + row) & VRAM_MASK];

        const uint8_t left = (uint8_t)(byte >> 4);
        const uint8_t right = (uint8_t)(byte & 0x0f);
        uint8_t bit;

        for (bit = 0; bit < 4; ++bit)
        {
            pixels[column * 8 + bit] = left ? left : backdrop_colour(vdp);
            pixels[column * 8 + 4 + bit] = right ? right : backdrop_colour(vdp);
        }
    }
}

/* Text: 40 columns of six visible pixels, eight pixels of border each side */
static void render_text(tms9918b* vdp, uint16_t line, uint8_t* pixels, bool banked)
{
    const uint16_t names = (uint16_t)(name_table(vdp) + (line >> 3) * 40);
    const uint16_t patternBase = (uint16_t)((vdp->registers[4] & (banked ? 0x04 : 0x07)) << 11);
    const uint16_t page = banked ? page_offset(vdp, line) : 0;
    const uint8_t fg = (uint8_t)(vdp->registers[7] >> 4);
    const uint8_t bg = backdrop_colour(vdp);

    uint16_t column;
    uint16_t x;

    for (x = 0; x < 8; ++x) pixels[x] = bg;
    for (x = 248; x < 256; ++x) pixels[x] = bg;

    for (column = 0; column < 40; ++column)
    {
        const uint8_t name = vdp->vram[(names + column) & VRAM_MASK];
        const uint8_t pattern = vdp->vram[(patternBase + page + 8 * name + (line & 7)) & VRAM_MASK];
        uint8_t bit;

        for (bit = 0; bit < 6; ++bit)
        {
            pixels[8 + column * 6 + bit] = ((pattern << bit) & 0x80) ? fg : bg;
        }
    }
}

/* ===========================================================================
 * 6. TMS9918B display modes
 *
 * Every extended mode reads its position through the scroll registers latched
 * at the start of the line, so a value written later takes effect on the next
 * one. Three bits of R12 shape the result: MASK blanks the leftmost eight
 * pixels, where a fine horizontal scroll always shows a partial column; HLOCK
 * keeps the first two character rows still; VLOCK keeps the last eight columns
 * still.
 * ========================================================================= */

/* pixel value 0 is the backdrop; 1 to 3 are entries of the given palette */
static uint8_t palette_pixel(const tms9918b* vdp, uint8_t palette, uint8_t value)
{
    if (value == 0) return backdrop_colour(vdp);

    return vdp->palette[((palette & 0x03) << 2) | (value & 0x03)];
}

static uint16_t world_line(const tms9918b* vdp, uint16_t line, bool locked)
{
    if (locked) return line;

    return (uint16_t)((line + vdp->latchedVScroll) % TMS9918B_LINES);
}

/* Graphics1X: two bytes per cell, sixteen bytes per pattern, a palette per
 * tile. Graphics2Fat: the same cell, but a pattern row holds four dots two
 * pixels wide, each carrying a TMS colour used directly.
 */
static void render_tiles(tms9918b* vdp, uint16_t line, uint8_t* pixels,
                         uint8_t* priority, bool fat)
{
    const uint8_t screen = EXT(vdp, 12);
    const bool hlocked = (screen & R12_HLOCK) && (line < 16);
    const uint8_t hscroll = hlocked ? 0 : vdp->latchedHScroll;
    const uint8_t fine = (uint8_t)(hscroll & 0x07);
    const uint8_t coarse = (uint8_t)(hscroll >> 3);

    const uint16_t names = name_table(vdp);
    const uint16_t patternBase = (uint16_t)((vdp->registers[4] & (fat ? 0x04 : 0x07)) << 11);
    const uint16_t fatMask = (uint16_t)(((vdp->registers[4] & 0x03) << 8) | 0xff);

    uint8_t cell;

    for (cell = 0; cell < 33; ++cell)
    {
        const int16_t screenX = (int16_t)(cell * 8 - fine);
        const bool vlocked = (screen & R12_VLOCK) && (screenX >= 192);
        const uint16_t wline = world_line(vdp, line, vlocked);
        const uint8_t column = (uint8_t)((cell + coarse) & 0x1f);

        const uint16_t entry = (uint16_t)(names + 2 * ((wline >> 3) * 32 + column));
        const uint8_t name = vdp->vram[entry & VRAM_MASK];
        const uint8_t attr = vdp->vram[(entry + 1) & VRAM_MASK];

        uint16_t pattern;
        uint8_t low;
        uint8_t high;
        uint8_t i;

        if (fat)
        {
            const uint16_t index = (uint16_t)((((wline >> 6) << 8) | name) & fatMask);

            pattern = (uint16_t)(patternBase + 16 * index + 2 * (wline & 7));
        }
        else
        {
            const uint16_t name9 = (uint16_t)(name | ((attr & ATTR_NAME8) ? 0x100 : 0));

            pattern = (uint16_t)(patternBase + 16 * name9 + 2 * (wline & 7));
        }

        low = vdp->vram[pattern & VRAM_MASK];
        high = vdp->vram[(pattern + 1) & VRAM_MASK];

        for (i = 0; i < 8; ++i)
        {
            const int16_t x = (int16_t)(screenX + i);
            uint8_t value;
            uint8_t colour;

            if (x < 0 || x >= TMS9918B_PIXELS) continue;

            if (fat)
            {
                const uint8_t shift = (uint8_t)(6 - 2 * (i >> 1));

                value = (uint8_t)(((low >> shift) & 0x03) | (((high >> shift) & 0x03) << 2));
                colour = value ? value : backdrop_colour(vdp);
            }
            else
            {
                const uint8_t shift = (uint8_t)(7 - i);

                value = (uint8_t)(((low >> shift) & 1) | (((high >> shift) & 1) << 1));
                colour = palette_pixel(vdp, (uint8_t)(attr & ATTR_PALETTE), value);
            }

            pixels[x] = colour;
            priority[x] = (uint8_t)(((attr & ATTR_PRIOR) && value) ? 1 : 0);
        }
    }

    if (screen & R12_MASK)
    {
        uint8_t x;

        for (x = 0; x < 8; ++x) pixels[x] = backdrop_colour(vdp);
    }
}

/* Bitmap: 64 bytes per line, two per group of eight pixels, and one Palette
 * Map byte per 8x8 area. The banked variant gives every screen third its own
 * 4096-byte block, with the line inside a block taken modulo 64.
 */
static void render_bitmap(tms9918b* vdp, uint16_t line, uint8_t* pixels, bool banked)
{
    const uint8_t screen = EXT(vdp, 12);
    const bool hlocked = (screen & R12_HLOCK) && (line < 16);
    const uint8_t hscroll = hlocked ? 0 : vdp->latchedHScroll;
    const uint8_t fine = (uint8_t)(hscroll & 0x07);
    const uint8_t coarse = (uint8_t)(hscroll >> 3);
    const uint16_t map = (uint16_t)((vdp->registers[2] & 0x0f) << 10);

    uint8_t cell;

    for (cell = 0; cell < 33; ++cell)
    {
        const int16_t screenX = (int16_t)(cell * 8 - fine);
        const bool vlocked = (screen & R12_VLOCK) && (screenX >= 192);
        const uint16_t wline = world_line(vdp, line, vlocked);
        const uint8_t column = (uint8_t)((cell + coarse) & 0x1f);

        uint16_t base;
        uint16_t row;
        uint16_t address;
        uint8_t low;
        uint8_t high;
        uint8_t palette;
        uint8_t i;

        if (banked)
        {
            const uint16_t third = (uint16_t)(wline >> 6);
            const uint16_t block = (third == 0) ? 0
                : (((vdp->registers[4] >> (third - 1)) & 1) ? third : 0);

            base = (uint16_t)(((vdp->registers[4] & 0x04) << 11) + 0x1000 * block);
            row = (uint16_t)(wline & 0x3f);
        }
        else
        {
            base = (uint16_t)((vdp->registers[4] & 0x07) << 11);
            row = wline;
        }

        address = (uint16_t)(base + 64 * row + 2 * column);
        low = vdp->vram[address & VRAM_MASK];
        high = vdp->vram[(address + 1) & VRAM_MASK];
        palette = (uint8_t)(vdp->vram[(map + 32 * (wline >> 3) + column) & VRAM_MASK] & 0x03);

        for (i = 0; i < 8; ++i)
        {
            const int16_t x = (int16_t)(screenX + i);
            const uint8_t shift = (uint8_t)(7 - i);
            const uint8_t value = (uint8_t)(((low >> shift) & 1) | (((high >> shift) & 1) << 1));

            if (x < 0 || x >= TMS9918B_PIXELS) continue;

            pixels[x] = palette_pixel(vdp, palette, value);
        }
    }

    if (screen & R12_MASK)
    {
        uint8_t x;

        for (x = 0; x < 8; ++x) pixels[x] = backdrop_colour(vdp);
    }
}

/* Text40X: the geometry of Text, with a colour byte per cell whose halves fall
 * back to R7 when they are zero. Text64: 64 columns of eight pixels on a
 * 512-pixel line, one name byte per cell, two colours from R7. Both bank the
 * character generator when M3 is set, and neither scrolls horizontally.
 */
static void render_extended_text(tms9918b* vdp, uint16_t line, uint8_t* pixels,
                                 bool wide, bool banked)
{
    const uint16_t columns = wide ? 64 : 40;
    const uint16_t cellWidth = wide ? 8 : 6;
    const uint16_t padding = wide ? 0 : 8;
    const uint16_t width = wide ? TMS9918B_PIXELS_WIDE : TMS9918B_PIXELS;

    const uint16_t wline = world_line(vdp, line, false);
    const uint16_t names = name_table(vdp);
    const uint16_t patternBase = (uint16_t)((vdp->registers[4] & (banked ? 0x04 : 0x07)) << 11);
    const uint16_t mask = banked ? (uint16_t)(((vdp->registers[4] & 0x03) << 8) | 0xff) : 0xff;

    const uint8_t mainFg = (uint8_t)(vdp->registers[7] >> 4);
    const uint8_t mainBg = backdrop_colour(vdp);

    uint16_t column;
    uint16_t x;

    for (x = 0; x < padding; ++x) pixels[x] = mainBg;
    for (x = (uint16_t)(width - padding); x < width; ++x) pixels[x] = mainBg;

    for (column = 0; column < columns; ++column)
    {
        uint8_t fg = mainFg;
        uint8_t bg = mainBg;
        uint16_t entry;
        uint8_t name;
        uint16_t index;
        uint8_t pattern;
        uint16_t bit;

        if (wide)
        {
            entry = (uint16_t)(names + (wline >> 3) * columns + column);
            name = vdp->vram[entry & VRAM_MASK];
        }
        else
        {
            uint8_t colour;

            entry = (uint16_t)(names + 2 * ((wline >> 3) * columns + column));
            name = vdp->vram[entry & VRAM_MASK];
            colour = vdp->vram[(entry + 1) & VRAM_MASK];

            if (colour >> 4) fg = (uint8_t)(colour >> 4);
            if (colour & 0x0f) bg = (uint8_t)(colour & 0x0f);
        }

        index = (uint16_t)(((((uint16_t)(wline >> 6)) << 8) | name) & mask);
        pattern = vdp->vram[(patternBase + 8 * index + (wline & 7)) & VRAM_MASK];

        for (bit = 0; bit < cellWidth; ++bit)
        {
            const uint16_t px = (uint16_t)(padding + column * cellWidth + bit);

            if (px >= width) break;

            pixels[px] = ((pattern << bit) & 0x80) ? fg : bg;
        }
    }
}

/* ===========================================================================
 * 7. Sprites
 *
 * The attribute table and the patterns are those of the TMS9918A. In an
 * extended mode the fourth byte changes meaning: PALETTE and ENTRY name a
 * palette entry, ENTRY 0 makes the sprite invisible, BANK selects a second
 * pattern table, PAIR turns an odd sprite into the second bit plane of the one
 * before it, and XFINE is the low bit of the horizontal position in a
 * 512-pixel mode.
 * ========================================================================= */

static void render_sprites(tms9918b* vdp, uint16_t line, uint8_t* pixels,
                           const uint8_t* priority, uint16_t width,
                           uint8_t maxSprites, uint8_t perLine, bool extended)
{
    const uint16_t sat = sprite_attr_table(vdp);
    const uint16_t patterns = sprite_pattern_table(vdp);
    const bool large = (vdp->registers[1] & R1_SIZE) != 0;
    const bool magnify = (vdp->registers[1] & R1_MAG) != 0;
    const uint8_t scale = (uint8_t)(magnify ? 2 : 1);
    const uint8_t size = (uint8_t)((large ? 16 : 8) * scale);
    const bool wide = width > TMS9918B_PIXELS;

    uint8_t coincidence[TMS9918B_PIXELS_WIDE / 8];
    uint8_t drawn = 0;
    uint8_t sprite;

    memset(coincidence, 0, sizeof(coincidence));

    for (sprite = 0; sprite < maxSprites; ++sprite)
    {
        const uint16_t attr = (uint16_t)(sat + 4 * sprite);
        const uint8_t rawY = vdp->vram[attr & VRAM_MASK];
        const uint8_t colour = vdp->vram[(attr + 3) & VRAM_MASK];

        int16_t top;
        int16_t x;
        uint8_t name;
        uint16_t pattern;
        uint8_t row;
        uint8_t entry;
        uint8_t palette;
        bool paired = false;
        uint16_t pattern2 = 0;
        int16_t x2 = 0;
        uint8_t row2 = 0;
        uint8_t px;

        if (rawY == 0xd0) break;                /* the table ends here */

        top = (int16_t)((int8_t)rawY + 1);
        if (line < (uint16_t)top || line >= (uint16_t)(top + size)) continue;

        /* an odd sprite with PAIR is not an object of its own */
        if (extended && (sprite & 1) && (colour & SPR_PAIR)) continue;

        if (drawn == perLine)
        {
            if ((vdp->status & S0_5S) == 0)
            {
                vdp->status = (uint8_t)((vdp->status & 0xe0) | S0_5S | (sprite & 0x1f));
            }
            break;
        }
        ++drawn;

        entry = extended ? (uint8_t)(colour & SPR_ENTRY) : (uint8_t)(colour & 0x0f);
        palette = extended ? (uint8_t)((colour & SPR_PALETTE) >> 2) : 0;

        x = (int16_t)vdp->vram[(attr + 1) & VRAM_MASK];
        if (colour & SPR_EC) x -= 32;
        if (wide) x = (int16_t)(2 * x + ((colour & SPR_XFINE) ? 1 : 0));

        name = (uint8_t)(vdp->vram[(attr + 2) & VRAM_MASK] & (large ? 0xfc : 0xff));
        pattern = (uint16_t)(patterns + 8 * name
            + ((extended && (colour & SPR_BANK)) ? 0x800 : 0));
        row = (uint8_t)((line - (uint16_t)top) / scale);

        /* the second plane, when the next sprite carries PAIR */
        if (extended && !(sprite & 1) && (sprite + 1) < maxSprites)
        {
            const uint16_t attr2 = (uint16_t)(sat + 4 * (sprite + 1));
            const uint8_t rawY2 = vdp->vram[attr2 & VRAM_MASK];
            const uint8_t colour2 = vdp->vram[(attr2 + 3) & VRAM_MASK];

            if (rawY2 != 0xd0 && (colour2 & SPR_PAIR))
            {
                const int16_t top2 = (int16_t)((int8_t)rawY2 + 1);

                if (line >= (uint16_t)top2 && line < (uint16_t)(top2 + size))
                {
                    const uint8_t name2 =
                        (uint8_t)(vdp->vram[(attr2 + 2) & VRAM_MASK] & (large ? 0xfc : 0xff));

                    paired = true;
                    pattern2 = (uint16_t)(patterns + 8 * name2
                        + ((colour2 & SPR_BANK) ? 0x800 : 0));
                    row2 = (uint8_t)((line - (uint16_t)top2) / scale);
                    x2 = (int16_t)vdp->vram[(attr2 + 1) & VRAM_MASK];
                    if (colour2 & SPR_EC) x2 -= 32;
                    if (wide) x2 = (int16_t)(2 * x2 + ((colour2 & SPR_XFINE) ? 1 : 0));
                }
            }
        }

        for (px = 0; px < size; ++px)
        {
            const uint8_t column = (uint8_t)(px / scale);
            const uint8_t half = (uint8_t)(column >> 3);
            const uint8_t bit = (uint8_t)(column & 0x07);
            const int16_t sx = (int16_t)(x + px);

            uint8_t value = (vdp->vram[(pattern + row + 16 * half) & VRAM_MASK] << bit) & 0x80
                ? 1 : 0;
            uint8_t index;

            if (paired)
            {
                const int16_t offset = (int16_t)(sx - x2);

                if (offset >= 0 && offset < (int16_t)size)
                {
                    const uint8_t column2 = (uint8_t)(offset / scale);
                    const uint8_t half2 = (uint8_t)(column2 >> 3);
                    const uint8_t bit2 = (uint8_t)(column2 & 0x07);

                    if ((vdp->vram[(pattern2 + row2 + 16 * half2) & VRAM_MASK] << bit2) & 0x80)
                    {
                        value |= 2;
                    }
                }
            }

            if (value == 0) continue;
            if (sx < 0 || sx >= (int16_t)width) continue;

            /* two sprites of a line writing the same pixel set the coincidence
               flag; the two planes of a pair are one object and never do */
            if (coincidence[sx >> 3] & (0x80 >> (sx & 7))) vdp->status |= S0_COINCIDENCE;
            coincidence[sx >> 3] |= (uint8_t)(0x80 >> (sx & 7));

            /* a tile with PRIOR covers every sprite where its pixels are not 0 */
            if (priority != NULL && priority[sx]) continue;

            index = paired ? value : entry;
            if (index == 0) continue;           /* transparent, or entry 0 */

            pixels[sx] = extended ? vdp->palette[palette * 4 + index] : index;
        }
    }
}

/* ===========================================================================
 * 8. The line, the line counter and interrupts
 * ========================================================================= */

void tms9918b_scanline(tms9918b* vdp, uint8_t line, uint8_t* pixels)
{
    const tms9918b_mode mode = tms9918b_display_mode(vdp);
    const uint16_t width = tms9918b_line_width(vdp);

    uint8_t priority[TMS9918B_PIXELS];

    if (vdp == NULL || pixels == NULL || line >= TMS9918B_LINES) return;

    /* the scroll registers are sampled once per line, so a value written later
       takes effect on the next one */
    if (extended_modes(vdp))
    {
        vdp->latchedHScroll = EXT(vdp, 8);
        vdp->latchedVScroll = EXT(vdp, 9);
    }
    else
    {
        vdp->latchedHScroll = 0;
        vdp->latchedVScroll = 0;
    }

    memset(priority, 0, sizeof(priority));
    memset(pixels, backdrop_colour(vdp), width);

    if (tms9918b_display_enabled(vdp))
    {
        switch (mode)
        {
            case TMS9918B_MODE_GRAPHICS_I:
                render_graphics(vdp, line, pixels, false);
                render_sprites(vdp, line, pixels, NULL, width, 32, 4, false);
                break;

            case TMS9918B_MODE_GRAPHICS_II:
                render_graphics(vdp, line, pixels, true);
                render_sprites(vdp, line, pixels, NULL, width, 32, 4, false);
                break;

            case TMS9918B_MODE_MULTICOLOR:
                render_multicolor(vdp, line, pixels, false);
                render_sprites(vdp, line, pixels, NULL, width, 32, 4, false);
                break;

            case TMS9918B_MODE_TEXT:
                render_text(vdp, line, pixels, false);
                break;

#if TMS9918B_UNDOCUMENTED
            case TMS9918B_MODE_TEXT_Q:
                render_text(vdp, line, pixels, true);
                break;

            case TMS9918B_MODE_MULTICOLOR_Q:
                render_multicolor(vdp, line, pixels, true);
                render_sprites(vdp, line, pixels, NULL, width, 32, 4, false);
                break;

            case TMS9918B_MODE_BARS:
            case TMS9918B_MODE_BARS_M3:
            case TMS9918B_MODE_BARS_X:
            case TMS9918B_MODE_BARS_XM3:
                /* these combinations read no table. What the device puts on
                   screen is not documented, so the backdrop is left alone. */
                break;
#endif

            case TMS9918B_MODE_GRAPHICS_1X:
            case TMS9918B_MODE_GRAPHICS_2FAT:
                render_tiles(vdp, line, pixels, priority,
                             mode == TMS9918B_MODE_GRAPHICS_2FAT);
                render_sprites(vdp, line, pixels, priority, width, 32, 8, true);
                break;

            case TMS9918B_MODE_BITMAP:
            case TMS9918B_MODE_BITMAP_Q:
                render_bitmap(vdp, line, pixels, mode == TMS9918B_MODE_BITMAP_Q);
                render_sprites(vdp, line, pixels, NULL, width, 32, 8, true);
                break;

            case TMS9918B_MODE_TEXT40X:
            case TMS9918B_MODE_TEXT40XQ:
            case TMS9918B_MODE_TEXT64:
            case TMS9918B_MODE_TEXT64Q:
                render_extended_text(vdp, line, pixels,
                    mode == TMS9918B_MODE_TEXT64 || mode == TMS9918B_MODE_TEXT64Q,
                    mode == TMS9918B_MODE_TEXT40XQ || mode == TMS9918B_MODE_TEXT64Q);
                /* sprites 0 and 1 are the only ones a text mode displays */
                render_sprites(vdp, line, pixels, NULL, width, 2, 2, true);
                break;

            default:
                break;
        }
    }

    /* the frame interrupt is raised at the end of the picture */
    if (line == TMS9918B_LINES - 1) vdp->status |= S0_INT;

    /* the line counter is loaded at the first line of the picture, and the end
       of that line is counted like any other: R10 = 47 therefore fires at the
       end of lines 47, 95, 143 and 191 */
    if (extended_enabled(vdp))
    {
        if (line == 0) vdp->lineCounter = EXT(vdp, 10);

        if (vdp->lineCounter == 0)
        {
            vdp->lineCounter = EXT(vdp, 10);
            vdp->lineIrq = true;
        }
        else
        {
            --vdp->lineCounter;
        }
    }
}

bool tms9918b_interrupt(const tms9918b* vdp)
{
    if (vdp == NULL) return false;

    if (extended_enabled(vdp) && vdp->lineIrq && (EXT(vdp, 11) & R11_IE1)) return true;

    return (vdp->registers[1] & R1_IE) && (vdp->status & S0_INT);
}

/* ===========================================================================
 * 9. Access timing
 *
 * Worst-case delay of a CPU access, in Z80 T-states at 3.58 MHz, from the
 * TMS9918B specification. A general-purpose emulator can ignore these; one
 * that models contention adds them to its cycle count.
 * ========================================================================= */

uint16_t tms9918b_access_tstates(const tms9918b* vdp)
{
    if (vdp == NULL) return 0;

    if (!tms9918b_display_enabled(vdp)) return 8;   /* blanked: no contention */

    switch (tms9918b_display_mode(vdp))
    {
        case TMS9918B_MODE_TEXT:
        case TMS9918B_MODE_TEXT_Q:
            return 12;

        case TMS9918B_MODE_MULTICOLOR:
        case TMS9918B_MODE_MULTICOLOR_Q:
            return 13;

        case TMS9918B_MODE_GRAPHICS_I:
        case TMS9918B_MODE_GRAPHICS_II:
            return 29;

        case TMS9918B_MODE_GRAPHICS_1X:
        case TMS9918B_MODE_GRAPHICS_2FAT:
            return 18;

        case TMS9918B_MODE_BITMAP:
        case TMS9918B_MODE_BITMAP_Q:
            return 15;

        case TMS9918B_MODE_TEXT40X:
        case TMS9918B_MODE_TEXT40XQ:
            return 16;

        case TMS9918B_MODE_TEXT64:
        case TMS9918B_MODE_TEXT64Q:
            return 14;

        default:
            return 8;                            /* the bar modes read nothing */
    }
}
