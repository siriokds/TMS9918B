# TMS9918B Compatibility Guidelines

The TMS9918B specification is open: anyone may implement it — in an emulator, an FPGA, an ASIC or a board — under
any license, commercial or not. These guidelines say when an implementation may **describe itself as compatible**,
so that software written for the TMS9918B runs the same everywhere. They work like the conformance rules of open
instruction-set architectures such as RISC-V: the specification defines the behaviour, the reference model and its
tests define how to check it.

The first part defines what the chip must do; the second and third parts give the conventions for integrating it in
hardware and for writing software around it. These are guidelines, not license conditions. The documents are licensed under CC BY 4.0 and the reference model
under Apache-2.0 whether or not an implementation follows them.

## Specification

| Document | Role |
|---|---|
| [TMS9918B Data Manual](tms9918b_data_manual/TMS9918B_Data_Manual.md) (TB-9918B-01) | normative: registers, modes, sprites, palette, scrolling, interrupts, VRAM interface, timing |
| [Programmer's Guide Supplement](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.md) (TB-9918B-02) | informative: programming practice and examples |
| [Reference model](tms9918b_model/README.md) | normative for memory access sequences, CPU access windows, table addresses and mode decode |

Where the Data Manual does not restate a TMS9918A behaviour, the TMS9918A/9928A/9929A Data Manual (MP010A) applies.

## Compatibility levels

| Level | Name | Requirements |
|---|---|---|
| **1** | TMS9918A mode | After reset the implementation behaves as a TMS9918A: registers R0-R7 with 3-bit register decode, status register, all eight M1-M2-M3 combinations (including Text 1Q, Multicolor Q and the bar modes) with the table addresses of `TMS99xxTableAddress.h`, four sprites per line, frame interrupt. |
| **2** | TMS9918B functional | Level 1, plus every function of Data Manual sections 2.1-2.7: unlock command, R8-R15, status register S1 with identification 18h, palette port and luminance levels, all extended modes including Text64 and third banking, sprite colour byte with BANK and PAIR, eight sprites per line with line-pair selection, tile priority, scrolling with MASK/HLOCK/VLOCK, scanline interrupt. |
| **3** | TMS9918B timing | Level 2, plus the memory access sequences of Data Manual Appendix C and `docs/generated/calendars.md`: CPU transfers are performed in the same CPU cycles, so the CPU access times of Appendix B and the write-loss behaviour of continuous loops are reproduced. |

A level 3 hardware implementation that uses dynamic VRAM must also meet Data Manual section 5 and Appendix A.

## Rules

1. **Claim a level explicitly**, for example "TMS9918B compatible, level 2", and name the Data Manual revision.
2. **Pass the reference tests** that apply to the level: `test_tms99xx_table_address` (levels 1-3), the mode-decode,
   address and alignment checks of `test_tms9918b` (levels 2-3), and the calendar and timing checks of
   `test_tms9918b` (level 3). Emulators should compare their own address generators and calendars with the model.
3. **Do not use reserved bits or registers.** R11, R14, reserved bits of R8, R13, R15, of the attribute bytes and of
   the sprite colour byte must behave as documented (written as 0, no effect).
4. **Keep extensions behind their own unlock.** Functions beyond the specification must be disabled after reset and
   after the TMS9918B unlock, must be enabled by a separate command, and must not change the value of S1 while
   disabled. Software written for the TMS9918B must never enable them by accident.
5. **Identify honestly.** An implementation that is not level 2 must not return 18h in S1.
6. **Do not suggest an official origin.** TMS9918A, TMS9928A and TMS9929A are Texas Instruments part numbers. Say
   "TMS9918B compatible" or "implements the TMS9918B specification", never that the product is made or endorsed by
   Texas Instruments.

## Hardware integration

These conventions apply to machines, boards and FPGA or emulator cores that host a TMS9918B.

1. **Same socket, same crystal.** The TMS9918B replaces a TMS9918A, TMS9928A or TMS9929A without changes to the
   board, the clock or the video circuit.
2. **Qualified VRAM only.** All eight VRAM chips must be 16K x 1 dynamic RAMs with 120 ns access time and page
   mode (for example Fujitsu MB8118-12, Motorola MCM4517-12); see
   [Appendix A](tms9918b_data_manual/TMS9918B_Data_Manual.md#appendix-a---choosing-vram-memory). A board with
   150 ns or slower parts needs all eight replaced; do not mix speeds.
3. **Keep the system's VDP ports.** Software finds the VDP where the original machine had it:

   | System | Data port | Control / status port | Interrupt input |
   |---|---|---|---|
   | Sega SC-3000, SG-1000 | BEh | BFh | Z80 INT |
   | MSX | 98h | 99h | Z80 INT |
   | ColecoVision | BEh | BFh | Z80 NMI |
   | TI-99/4A | 8800h read, 8C00h write | 8802h read, 8C02h write | TMS9901 |

   For other systems, use the addresses of the original VDP.
4. **INT carries two interrupts.** The frame and scanline interrupts share the INT pin. On systems where INT drives
   an edge-triggered input (ColecoVision NMI), the line stays active while either flag is set: the handler must
   clear both FL (read S1) and F (read S0), otherwise no further edge is generated.
5. **Monitors.** Text64 needs the RGB or component output of the TMS9928B/9929B, or a monitor of equivalent
   quality, to be legible. The other modes work on a composite television like the TMS9918A.
6. **FPGA and emulators.** An implementation may use static RAM or host memory instead of dynamic VRAM. For level 3
   it must reproduce the CPU access windows and memory access sequences, not the DRAM waveforms.
7. **Do not unlock at power-up.** Firmware and BIOS code must leave the TMS9918B locked unless the program that
   runs next uses the extensions (see Software conventions, rule 5).

## Software conventions

These conventions let programs, firmware and system extensions written by different authors coexist.

1. **Detect before use.** Run the detection sequence of the
   [Programmer's Guide](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.md#32-detecting-the-tms9918b)
   before touching R8-R15 or the palette. If the TMS9918B is not found, use TMS9918A modes only.
2. **Keep shadow copies.** The registers are write-only. Keep the last value written to every register (R0-R15),
   and restore R0 and R7 after the detection sequence.
3. **Leave R15 at 00h.** Select S1 only to read it, then write 00h to R15 at once, so that code which reads the
   status port expects S0 and finds S0.
4. **Handle interrupts in a fixed order.** Select S1, read it (clears FL), select S0, read it (clears F). Keep
   interrupts disabled in the main program during two-byte control transfers, because a handler that writes a
   register breaks a transfer in progress.
5. **Unlock only when needed, and clean up.** Once unlocked, register numbers 8-15 no longer alias R0-R7 until a
   hardware RESET. TMS9918A software that writes register numbers above 7 on purpose will behave differently.
   Before returning to BASIC, a menu, or any program that does not know the TMS9918B, write R8 = 00h, R13 = 00h,
   R9 = R10 = 00h and R15 = 00h; if the next program may rely on register aliasing, request a hardware reset.
6. **Load the palette before enabling extended modes.** The palette is undefined after power-up: set XE, load all
   16 entries, then set MX.
7. **Write reserved bits as 0** and never rely on the value of a reserved register.
8. **Place tables where the registers say.** Two- and four-byte entries are fetched together; do not offset a table
   by one byte. The suggested layouts in
   [Programmer's Guide section 5](tms9918b_programmers_guide/TMS9918B_Programmers_Guide_Supplement.md#5-initializing-the-extended-modes)
   fit every mode in 16K.
9. **Respect the transfer rates.** Use the loops of
   [Appendix B](tms9918b_data_manual/TMS9918B_Data_Manual.md#appendix-b---cpu-to-vdp-access-times) for writes
   during the active display, or write during vertical blanking. Palette writes can be made at any time.
10. **60 Hz and 50 Hz.** S1 does not tell a TMS9918B/9928B from a TMS9929B. Programs that need the frame rate
    measure it, as with the TMS9918A family.

## Deviations

An implementation that differs from the specification in any documented behaviour may still use the specification,
but should say "based on the TMS9918B specification" and list the differences.

## Revisions

Changes to the specification are published as new revisions of the Data Manual. A compatibility claim refers to
the revision it was tested against.
