; Lesson 041: Text 1Q, the undocumented mode 3.
;
; Setting M3 together with M1 keeps the 40x24 text geometry - names are still
; read 40 per row and the pattern row is still line AND 7 - but the pattern
; generator becomes banked per screen third, exactly as in Graphics II. With
; R4 = 03h the three 2 KiB banks are at 0000h, 0800h and 1000h, and character
; rows 0..7, 8..15 and 16..23 read their own bank.
;
; The same string is printed in the three regions with the same name bytes. It
; appears in a plain font at the top, in a bold font in the middle and in
; an inverse font at the bottom, which is the only visible difference between
; mode 1 and mode 3. Colours still come from R7 alone: mode 3 has no colour
; table, so this is Text, not Graphics II.

        include "include/tms9918b_hardware.inc"
        ROM_START start

BANK_TOP:       equ 0x0000
BANK_MIDDLE:    equ 0x0800
BANK_BOTTOM:    equ 0x1000
TEXT_NAMES:     equ 0x3800      ; R2 = 0Eh, 960 bytes

    MACRO TEXT_AT row, column
        ld de,TEXT_NAMES + (row)*40 + (column)
    ENDM

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call vdp_clear

        ; The same character codes must resolve in every bank, so the font is
        ; written three times, twice through a transformation.
        ld de,BANK_TOP
        xor a
        call load_font_transformed
        ld de,BANK_MIDDLE
        ld a,1
        call load_font_transformed
        ld de,BANK_BOTTOM
        ld a,2
        call load_font_transformed

        ld hl,TEXT_NAMES
        ld bc,40*24
        ld e,0
        call vdp_fill

        TEXT_AT 1, 4
        ld hl,title
        call vdp_print_ascii
        TEXT_AT 3, 2
        ld hl,line_one
        call vdp_print_ascii
        TEXT_AT 5, 2
        ld hl,line_two
        call vdp_print_ascii

        TEXT_AT 9, 2
        ld hl,sample
        call vdp_print_ascii
        TEXT_AT 11, 2
        ld hl,middle_note
        call vdp_print_ascii

        TEXT_AT 17, 2
        ld hl,sample
        call vdp_print_ascii
        TEXT_AT 19, 2
        ld hl,bottom_note
        call vdp_print_ascii

        ld hl,registers_visible
        call vdp_set_registers
        jp freeze

; DE = bank address, A = transform: 0 plain, 1 bold, 2 inverse.
; The three banks are produced from the same ROM font while it is streamed to
; VRAM, so the character codes are identical in all of them.
load_font_transformed:
        ld (TRANSFORM),a
        ex de,hl
        call vdp_set_write
        ld hl,sega_font_6x8
        ld bc,sega_font_6x8_end-sega_font_6x8
.byte:
        ld a,(TRANSFORM)
        dec a
        jr z,.bold
        dec a
        jr z,.inverse
        ld a,(hl)
        jr .send
.bold:
        ld a,(hl)
        ld e,a
        rrca
        and 0x7F                ; one pixel to the right
        or e
        jr .send
.inverse:
        ld a,(hl)
        cpl
        and 0xFC                ; the two low bits are never displayed
.send:
        out (VDP_DATA),a
        inc hl
        dec bc
        ld a,b
        or c
        jr nz,.byte
        ret

TRANSFORM:      equ 0xC130

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x02, 0x90, 0x0E, 0x00, 0x03, 0x76, 0x03, 0xF4
registers_visible: db 0x02, 0xD0, 0x0E, 0x00, 0x03, 0x76, 0x03, 0xF4

title:       db "041 LEGACY TEXT 1Q - MODE 3",0
line_one:    db "TEXT GEOMETRY WITH GRAPHICS II BANKING",0
line_two:    db "SAME NAMES, ONE PATTERN BANK PER THIRD",0
sample:      db "ABCDEFGHIJKLM 0123456789",0
middle_note: db "SECOND BANK: BOLD FONT",0
bottom_note: db "THIRD BANK: INVERSE FONT",0

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
