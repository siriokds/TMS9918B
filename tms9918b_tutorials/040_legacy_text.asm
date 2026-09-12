; Lesson 040: locked-device Text regression.
;
; Text mode has 40x24 characters. Each cell displays pattern bits 7..2 in a
; 6x8 area; bits 1..0 are never visible. R7 supplies one foreground and one
; background colour for the entire screen, and sprites are disabled.

        include "include/tms9918b_hardware.inc"
        ROM_START start

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call vdp_clear
        ld de,PATTERN_TABLE
        call vdp_load_sega_font

        ld hl,NAME_TABLE
        ld bc,40*24
        ld e,0
        call vdp_fill
        ld de,NAME_TABLE + 2*40 + 8
        ld hl,title
        call vdp_print_ascii
        ld de,NAME_TABLE + 6*40 + 7
        ld hl,geometry
        call vdp_print_ascii
        ld de,NAME_TABLE + 10*40 + 5
        ld hl,bits
        call vdp_print_ascii
        ld de,NAME_TABLE + 14*40 + 4
        ld hl,alphabet
        call vdp_print_ascii
        ld de,NAME_TABLE + 18*40 + 9
        ld hl,colours
        call vdp_print_ascii

        ld hl,registers_visible
        call vdp_set_registers
        jp freeze

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x90, 0x0E, 0x00, 0x00, 0x76, 0x03, 0xF4
registers_visible: db 0x00, 0xD0, 0x0E, 0x00, 0x00, 0x76, 0x03, 0xF4
title:             db "040 LEGACY TEXT",0
geometry:          db "40 X 24 CHARACTERS - 6 X 8",0
bits:              db "PATTERN BITS 7 TO 2 ARE VISIBLE",0
alphabet:          db "ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789",0
colours:           db "GLOBAL COLOURS FROM R7",0

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        ROM_END

