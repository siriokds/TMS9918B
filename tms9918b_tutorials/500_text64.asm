; Lesson 500: Text64, sixty-four columns.
;
; With T64 set in R12 the extended text mode displays 64 columns of 8x8
; characters on 512-pixel lines. The Name Table holds one byte per cell, all
; eight pattern bits are visible, and the whole screen uses the two colours of
; R7: a colour byte per character does not fit in the memory access sequence at
; this width. Characters are therefore narrower than they are tall, as on the
; terminals of the period, and an RGB or component monitor is required.
;
; Sprites 0 and 1 are displayed over the text as a hardware cursor. The sprite
; X coordinate counts in two-pixel steps, so column c is at X = 4 * c. The
; cursor blinks by alternating the entry bits of its colour byte between 1 and
; 0; entry 0 is invisible, so no VRAM pattern has to be rewritten.

        include "include/tms9918b_hardware.inc"
        ROM_START start

T64_PATTERNS:   equ 0x0000      ; R4 = 00h
T64_NAMES:      equ 0x3000      ; R2 = 0Ch, 64 x 24 bytes
CURSOR_PATTERNS: equ 0x0800     ; R6 = 01h
CURSOR_SAT:     equ 0x3B00      ; R5 = 76h

CURSOR_ROW:     equ 20
CURSOR_COLUMN:  equ 43

; Row and column to Name Table address.
    MACRO T64_AT row, column
        ld de,T64_NAMES + (row)*64 + (column)
    ENDM

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call tms9918b_require
        call vdp_clear

        ld de,T64_PATTERNS
        call vdp_load_sega_font
        ld hl,palette
        call vdp_load_palette

        ld hl,T64_NAMES
        ld bc,64*24
        ld e,0                  ; pattern 0 is the space
        call vdp_fill

        T64_AT 1, 24
        ld hl,title
        call vdp_print_ascii
        T64_AT 3, 8
        ld hl,line_one
        call vdp_print_ascii
        T64_AT 5, 8
        ld hl,line_two
        call vdp_print_ascii
        T64_AT 7, 8
        ld hl,line_three
        call vdp_print_ascii

        ; A full 64-character ruler: the first and last columns are visible.
        T64_AT 10, 0
        ld hl,ruler
        call vdp_print_ascii
        T64_AT 11, 0
        ld hl,alphabet
        call vdp_print_ascii

        T64_AT 14, 8
        ld hl,line_four
        call vdp_print_ascii
        T64_AT 16, 8
        ld hl,line_five
        call vdp_print_ascii

        T64_AT CURSOR_ROW, 30
        ld hl,prompt
        call vdp_print_ascii

        ld b,CURSOR_ROW*8-1     ; raw Y of the character row
        ld c,CURSOR_COLUMN*4    ; two-pixel steps: X = 4 * column
        call cursor_load

        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers

.frame:
        call wait_vblank
        call cursor_blink       ; entry 1 <-> entry 0 (invisible)
        jr .frame

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x90, 0x0C, 0x00, 0x00, 0x76, 0x01, 0xF4
registers_visible: db 0x00, 0xD0, 0x0C, 0x00, 0x00, 0x76, 0x01, 0xF4

extended_registers:
        db 11, R11_XE|R11_MX
        db 12, R12_T64          ; sixty-four columns
        db 0xFF

; Only the cursor uses the palette in this mode: entry 1 of palette 0.
palette:
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0F, 0x0E, 0x01

title:      db "500 TEXT64",0
line_one:   db "64 X 24 CHARACTERS OF 8 X 8 PIXELS - 512 PIXEL LINES",0
line_two:   db "ONE NAME BYTE PER CELL, ALL EIGHT PATTERN BITS VISIBLE",0
line_three: db "TWO COLOURS FOR THE WHOLE SCREEN, FROM R7",0
ruler:      db "0123456789012345678901234567890123456789012345678901234567890123",0
alphabet:   db "ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ",0
line_four:  db "SPRITES 0 AND 1 ARE THE HARDWARE CURSOR",0
line_five:  db "CURSOR X COUNTS TWO PIXEL STEPS: X = 4 * COLUMN",0
prompt:     db "READY",0

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/text_cursor.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
