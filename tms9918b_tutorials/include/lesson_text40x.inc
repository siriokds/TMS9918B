; Shared body for lessons 400/401: Text40X, colours per character.
;
; The geometry is that of legacy Text: 40x24 cells of six visible pixels by
; eight, patterns of eight bytes with bits 1..0 ignored. What changes is the
; Name Table: every cell holds two bytes, the character and a colour byte whose
; upper half is the ink and lower half the paper. A zero half takes the
; corresponding half of R7, so a colour byte of 00h displays exactly like
; legacy Text mode - the last line below proves it.
;
; Sprites 0 and 1 are the hardware cursor in both extended text modes; this
; lesson leaves them terminated and lesson 500 uses them.

        include "include/tms9918b_hardware.inc"
        ROM_START start

CURSOR_SAT:      equ 0x3B00
CURSOR_PATTERNS: equ 0x0800

TEXT_PATTERNS:  equ 0x0000      ; R4 = 00h, eight bytes per character
TEXT_ENTRIES:   equ 0x3000      ; R2 = 0Ch, 960 entries x 2 bytes
TEXT_SAT:       equ 0x3B00      ; R5 = 76h

; Row and column to entry address.
    MACRO TEXT_AT row, column
        ld de,TEXT_ENTRIES + 2*((row)*40 + (column))
    ENDM

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call tms9918b_require
        call vdp_clear

        ld de,TEXT_PATTERNS
        call vdp_load_sega_font

        ; Space with colour byte 00h: the whole screen starts as legacy Text.
        ld hl,TEXT_ENTRIES
        ld bc,40*24
        ld d,0
        ld e,0
        call vdp_fill_pairs

        TEXT_AT 1, 12
        ld hl,title
        ld c,0xF1               ; white on black
        call vdp_print_ascii_pairs
        TEXT_AT 3, 5
        ld hl,line_one
        ld c,0x00
        call vdp_print_ascii_pairs
        TEXT_AT 5, 5
        ld hl,line_two
        ld c,0x00
        call vdp_print_ascii_pairs

        ; One line per colour byte, printed with its own value.
        ld hl,samples
        ld de,TEXT_ENTRIES + 2*(9*40 + 2)
        ld b,samples_count
.sample:
        push bc
        ld c,(hl)               ; colour byte of this sample
        inc hl                  ; HL -> text
        push de
        call vdp_print_ascii_pairs
        inc hl                  ; skip the terminator
        pop de
        ex de,hl
        ld de,2*80              ; two character rows
        add hl,de
        ex de,hl
        pop bc
        djnz .sample

        IF LESSON_WITH_CURSOR
        ld b,20*8-1             ; raw Y of character row 20
        ld c,7*6                ; X in pixels: column 7 of a six-pixel cell
        call cursor_load
        ELSE
        ld hl,TEXT_SAT
        ld bc,1
        ld e,0xD0               ; no cursor in this lesson
        call vdp_fill
        ENDIF

        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers
        IF LESSON_WITH_CURSOR
.frame:
        call wait_vblank
        call cursor_blink
        jr .frame
        ELSE
        jp freeze
        ENDIF

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x90, 0x0C, 0x00, 0x00, 0x76, 0x01, 0xF4
registers_visible: db 0x00, 0xD0, 0x0C, 0x00, 0x00, 0x76, 0x01, 0xF4

extended_registers:
        db 11, R11_XE|R11_MX
        db 12, 0x00             ; T64 = 0: forty columns
        db 0xFF

        IF LESSON_WITH_CURSOR
title:      db "401 TEXT40X + CURSOR",0
        ELSE
title:      db "400 TEXT40X",0
        ENDIF
line_one:   db "TWO BYTES PER CELL: CHARACTER, COLOUR",0
line_two:   db "INK IN BITS 0-3, PAPER IN BITS 4-7",0

; Colour byte followed by the text printed with it.
samples:
        db 0xF1, "F1 WHITE INK ON BLACK PAPER",0
        db 0xB4, "B4 LIGHT YELLOW ON DARK BLUE",0
        db 0x92, "92 LIGHT RED ON MEDIUM GREEN",0
        db 0x6F, "6F DARK RED ON WHITE",0
        db 0x00, "00 BOTH HALVES ZERO: R7 AS IN LEGACY TEXT",0
samples_count: equ 5

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/text_cursor.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
