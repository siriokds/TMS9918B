# tms9918b

A TMS9918A and TMS9918B video display processor in C99, two files, no
dependencies. Copy `tms9918b.c` and `tms9918b.h` into an emulator, call the
port functions where the emulated machine writes to its VDP, and ask for one
scanline at a time.

The TMS9918B is a hypothetical 1983 revision of the TMS9918A: a palette,
extended display modes, eight sprites per line, hardware scrolling and a
scanline interrupt. Its extensions are inert until software unlocks them, so a
machine built on this library is a TMS9918A until it is asked not to be. The
specification, the reference model and a suite of test ROMs are at
[siriokds/TMS9918B](https://github.com/siriokds/TMS9918B).

## Using it

```c
static tms9918b vdp;
uint8_t pixels[TMS9918B_PIXELS_WIDE];

tms9918b_init(&vdp);

/* where the machine writes to its VDP ports */
tms9918b_write_data(&vdp, value);
tms9918b_write_control(&vdp, value);
value = tms9918b_read_data(&vdp);
value = tms9918b_read_status(&vdp);

/* once per displayed line, in order */
for (uint8_t line = 0; line < TMS9918B_LINES; ++line)
{
    tms9918b_scanline(&vdp, line, pixels);
    /* pixels[0 .. tms9918b_line_width(&vdp) - 1] */
}

if (tms9918b_interrupt(&vdp)) cpu_irq();
```

The whole state is one structure with no allocation, so a save state is a copy
of it.

**Line width.** Every mode is 256 pixels wide except the 64-column text modes,
which are 512. Ask `tms9918b_line_width` and size the buffer accordingly, or
just use `TMS9918B_PIXELS_WIDE` as above.

**Pixels** are palette entries. In the TMS9918A modes they are colour numbers
0 to 15, as usual. In the extended modes the low nibble is still the TMS colour
and bits 4-5 carry the luminance, so code that masks the low nibble keeps
working. `tms9918b_rgb` converts either into red, green and blue.

## Accuracy

This is a scanline renderer: it draws a line at a time and never makes the CPU
wait, which is what a general-purpose emulator wants. Effects that change a
register in the middle of a line are outside its reach - the TMS9918B has none,
because it samples its scroll registers once per line by design.

`tms9918b_access_tstates` reports what a VRAM access costs in the current mode,
for an emulator that models contention. Ignoring it changes nothing on screen.

## Options

`-DTMS9918B_UNDOCUMENTED=0` drops the undocumented TMS9918A modes and decodes
those four combinations the way most emulators do, as Graphics I and Graphics
II. The default keeps them: text and multicolor with a banked pattern
generator, and the two combinations that read no table at all.

## License

Apache-2.0. TMS9918A and TMS9918B are referenced for compatibility; this
library is not affiliated with Texas Instruments.
