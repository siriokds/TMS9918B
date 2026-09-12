/*
 * tms9918b - TMS9918A and TMS9918B video display processor
 *
 * A scanline renderer in C99 with no dependencies, meant to be dropped into an
 * emulator: copy tms9918b.c and tms9918b.h, call the port functions where the
 * emulated machine writes to its VDP, and ask for one line at a time.
 *
 * The TMS9918B is a hypothetical 1983 revision of the TMS9918A. Its extensions
 * are inert until software unlocks them, so a device built on this library is
 * a TMS9918A until it is asked not to be. The specification and its reference
 * model are at https://github.com/siriokds/TMS9918B
 *
 * Copyright 2026 Saverio Russo
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TMS9918B_H
#define TMS9918B_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The undocumented TMS9918A modes cost about two hundred lines. Compile with
 * -DTMS9918B_UNDOCUMENTED=0 to decode them the way most emulators do, as
 * Graphics I and Graphics II. */
#ifndef TMS9918B_UNDOCUMENTED
#define TMS9918B_UNDOCUMENTED 1
#endif

#define TMS9918B_VRAM_SIZE      0x4000
#define TMS9918B_LINES          192
#define TMS9918B_PIXELS         256     /* every mode but the 64-column ones */
#define TMS9918B_PIXELS_WIDE    512     /* Text64 and Text64Q */

/* ---------------------------------------------------------------------------
 * Display modes
 *
 * M1, M2 and M3 give eight combinations. Four are documented; the others are
 * the undocumented modes of the real device. With the extension unlocked and
 * MX set, the same eight bits select the TMS9918B modes instead.
 * ------------------------------------------------------------------------- */
typedef enum
{
    TMS9918B_MODE_GRAPHICS_I,
    TMS9918B_MODE_TEXT,
    TMS9918B_MODE_GRAPHICS_II,
    TMS9918B_MODE_TEXT_Q,           /* undocumented: text, banked patterns */
    TMS9918B_MODE_MULTICOLOR,
    TMS9918B_MODE_BARS,             /* undocumented: no table is read */
    TMS9918B_MODE_MULTICOLOR_Q,     /* undocumented: multicolor, banked */
    TMS9918B_MODE_BARS_M3,          /* undocumented: the same as BARS */

    TMS9918B_MODE_GRAPHICS_1X,      /* two-bit tiles, a palette per tile */
    TMS9918B_MODE_TEXT40X,          /* a colour byte per character */
    TMS9918B_MODE_GRAPHICS_2FAT,    /* four dots per row, sixteen colours */
    TMS9918B_MODE_TEXT40XQ,
    TMS9918B_MODE_BITMAP,           /* 64 bytes per line, palette per area */
    TMS9918B_MODE_BARS_X,
    TMS9918B_MODE_BITMAP_Q,
    TMS9918B_MODE_BARS_XM3,
    TMS9918B_MODE_TEXT64,           /* 64 columns on a 512-pixel line */
    TMS9918B_MODE_TEXT64Q
} tms9918b_mode;

/* ---------------------------------------------------------------------------
 * Device
 *
 * The whole state is in this structure: no allocation, no globals, so an
 * emulator can hold one per machine and copy it for a save state.
 * ------------------------------------------------------------------------- */
typedef struct
{
    uint8_t  vram[TMS9918B_VRAM_SIZE];
    uint8_t  registers[8];          /* R0 - R7, the TMS9918A file */
    uint8_t  status;                /* S0: F, 5S, C and the sprite number */

    uint16_t address;               /* VRAM pointer, auto-incrementing */
    uint8_t  readAhead;
    uint8_t  firstByte;             /* first byte of a control pair */
    bool     haveFirstByte;

    /* the extensions, all inert until unlocked */
    bool     unlocked;
    bool     keyArmed;
    bool     paletteWrite;
    uint8_t  paletteIndex;
    uint8_t  ext[8];                /* R8 - R15 */
    uint8_t  palette[16];
    uint8_t  lineCounter;
    bool     lineIrq;

    /* sampled once per line, so that a write takes effect on the next one */
    uint8_t  latchedHScroll;
    uint8_t  latchedVScroll;
} tms9918b;

/* ---------------------------------------------------------------------------
 * Lifetime
 * ------------------------------------------------------------------------- */

/* clears everything, as a power-up would */
void tms9918b_init(tms9918b* vdp);

/* the RESET pin: locks the extensions and clears R0, R1, R11 and R12 */
void tms9918b_reset(tms9918b* vdp);

/* ---------------------------------------------------------------------------
 * Ports
 *
 * Two ports, as on the device: data with MODE low, control with MODE high.
 * On an SC-3000 they are BEh and BFh, on an MSX 98h and 99h.
 * ------------------------------------------------------------------------- */

void    tms9918b_write_data(tms9918b* vdp, uint8_t value);
void    tms9918b_write_control(tms9918b* vdp, uint8_t value);
uint8_t tms9918b_read_data(tms9918b* vdp);
uint8_t tms9918b_read_status(tms9918b* vdp);

/* ---------------------------------------------------------------------------
 * Display
 *
 * Call tms9918b_scanline once per displayed line, from 0 to 191, in order: the
 * scroll registers are sampled and the line counter advances there.
 *
 * A pixel is a palette entry. In the TMS9918A modes it is a colour number 0 to
 * 15, as usual. In the extended modes the low nibble is still the TMS colour
 * and bits 4-5 carry the luminance - 00 full, 01 three quarters, 10 a half, 11
 * a quarter - so a caller that masks the low nibble keeps working and one that
 * wants the extra colours reads six bits. tms9918b_rgb converts either.
 * ------------------------------------------------------------------------- */

/* pixels the current mode puts on a line: 256, or 512 at 64 columns */
uint16_t tms9918b_line_width(const tms9918b* vdp);

/* renders one line into a buffer of tms9918b_line_width() bytes */
void tms9918b_scanline(tms9918b* vdp, uint8_t line, uint8_t* pixels);

/* the mode the registers select right now */
tms9918b_mode tms9918b_display_mode(const tms9918b* vdp);

/* is the display enabled? (BL in R1) */
bool tms9918b_display_enabled(const tms9918b* vdp);

/* one pixel as 8-bit red, green and blue, luminance included */
void tms9918b_rgb(uint8_t pixel, uint8_t* r, uint8_t* g, uint8_t* b);

/* ---------------------------------------------------------------------------
 * Interrupts
 *
 * The frame interrupt and the scanline interrupt share one pin, so a handler
 * reads S1 and S0 to find out which one occurred.
 * ------------------------------------------------------------------------- */

/* true while the device asserts its interrupt pin */
bool tms9918b_interrupt(const tms9918b* vdp);

/* ---------------------------------------------------------------------------
 * Access timing
 *
 * This library renders a line at a time and never makes the CPU wait, which is
 * what a general-purpose emulator wants. A machine that does model contention
 * can add the delay of an access to its own cycle count: the value is the one
 * the TMS9918B specification lists for the current mode, in Z80 T-states at
 * 3.58 MHz.
 * ------------------------------------------------------------------------- */

/* worst-case T-states a VRAM access can cost in the current mode */
uint16_t tms9918b_access_tstates(const tms9918b* vdp);

#ifdef __cplusplus
}
#endif

#endif /* TMS9918B_H */
