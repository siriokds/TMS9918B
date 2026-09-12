; Lesson 110: the other two attribute bits, NAME8 and PRIOR.
;
; The attribute byte of a Graphics1X cell carries four fields. Lesson 100 used
; only the two palette bits; this lesson uses the other two:
;
;   NAME8 (08h)  ninth bit of the pattern name. The name byte reaches 256
;                patterns, NAME8 reaches 512. The two halves are independent
;                sets: name 40h with NAME8 clear and name 40h with NAME8 set
;                are different patterns, sixteen bytes apart by 4096.
;   PRIOR (04h)  the tile is displayed in front of every sprite wherever its
;                pixel value is not 0. Value 0 is always the backdrop, so a
;                sprite still shows through the transparent parts of the tile.
;
; The upper half of the screen proves NAME8: two rows of the same name bytes,
; one with NAME8 clear and one with NAME8 set, resolve to two different shapes.
; The lower half proves PRIOR: a horizontal band of bars is drawn twice, once
; with PRIOR clear and once with PRIOR set, and the sprite overlay passes
; behind the second band and in front of the first one.

        include "include/tms9918b_hardware.inc"
        ROM_START start

G1X_PATTERNS:        equ 0x0000     ; R4 = 00h, 512 patterns x 16 bytes
G1X_NAMES:           equ 0x3800     ; R2 = 0Eh
EXT_SAT:             equ 0x3E00     ; R5 = 7Ch
EXT_SPRITE_PATTERNS: equ 0x2800     ; R6 = 05h

LOW_NAME:       equ 0x40        ; pattern 040h, NAME8 clear
HIGH_NAME:      equ 0x40        ; pattern 140h, NAME8 set
BAR_NAME:       equ 0x50

; Cell address from row and column.
    MACRO CELL_AT row, column
        ld de,G1X_NAMES + 2*((row)*32 + (column))
    ENDM

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call tms9918b_require
        call vdp_clear

        ld de,G1X_PATTERNS
        call vdp_load_font_2bpp

        ; Pattern 040h and pattern 140h: same name byte, different NAME8.
        ld hl,disc_pattern
        ld de,G1X_PATTERNS + LOW_NAME*16
        ld bc,16
        call vdp_copy
        ld hl,cross_pattern
        ld de,G1X_PATTERNS + 0x100*16 + HIGH_NAME*16
        ld bc,16
        call vdp_copy
        ld hl,bar_pattern
        ld de,G1X_PATTERNS + BAR_NAME*16
        ld bc,16
        call vdp_copy

        ld hl,palette
        call vdp_load_palette

        ld hl,G1X_NAMES
        ld bc,32*24
        ld d,0
        ld e,0
        call vdp_fill_pairs

        CELL_AT 1, 3
        ld hl,title
        ld c,0
        call vdp_print_ascii_pairs
        CELL_AT 3, 1
        ld hl,line_one
        ld c,0
        call vdp_print_ascii_pairs
        CELL_AT 5, 1
        ld hl,line_two
        ld c,0
        call vdp_print_ascii_pairs

        ; Row 7: name 40h with NAME8 clear. Row 9: the same name byte with
        ; NAME8 set, which selects pattern 140h instead.
        CELL_AT 7, 4
        ld a,LOW_NAME
        ld c,0x01               ; palette 1, no NAME8
        ld b,24
        call fill_row
        CELL_AT 9, 4
        ld a,HIGH_NAME
        ld c,0x01|ATTR_NAME8    ; palette 1, NAME8 set
        ld b,24
        call fill_row

        CELL_AT 12, 1
        ld hl,line_three
        ld c,0
        call vdp_print_ascii_pairs

        ; Two identical bands of bars. The first one lets the sprites pass in
        ; front of it, the second one is drawn in front of them.
        CELL_AT 15, 0
        ld a,BAR_NAME
        ld c,0x02
        ld b,32
        call fill_row
        CELL_AT 16, 0
        ld a,BAR_NAME
        ld c,0x02
        ld b,32
        call fill_row
        CELL_AT 19, 0
        ld a,BAR_NAME
        ld c,0x02|ATTR_PRIOR    ; palette 2, PRIOR set
        ld b,32
        call fill_row
        CELL_AT 20, 0
        ld a,BAR_NAME
        ld c,0x02|ATTR_PRIOR
        ld b,32
        call fill_row

        CELL_AT 17, 1
        ld hl,behind_text
        ld c,0
        call vdp_print_ascii_pairs
        CELL_AT 22, 1
        ld hl,front_text
        ld c,0
        call vdp_print_ascii_pairs

        call extended_sprite_overlay_load
        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers

.animation_loop:
        call wait_vblank
        call extended_sprite_overlay_animate
        jr .animation_loop

; DE = first cell, A = pattern name, C = attribute, B = cells.
fill_row:
        push af
        ex de,hl
        call vdp_set_write
        pop af
.cell:
        out (VDP_DATA),a
        push af
        pop af
        push af
        ld a,c
        out (VDP_DATA),a
        pop af
        push af
        pop af
        djnz .cell
        ret

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x80, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01
registers_visible: db 0x00, 0xC0, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01

extended_registers:
        db 11, R11_XE|R11_MX
        db 12, 0x00
        db 0xFF

palette:
        db 0x00, 0x0F, 0x0E, 0x04       ; 0: text
        db 0x00, 0x0B, 0x0A, 0x06       ; 1: the two NAME8 shapes
        db 0x00, 0x07, 0x05, 0x04       ; 2: the priority bands
        db 0x00, 0x03, 0x02, 0x0C       ; 3: sprites

; Pattern 040h: a filled disc, values 1 and 2.
disc_pattern:
        db 0x3C, 0x00
        db 0x7E, 0x3C
        db 0xFF, 0x7E
        db 0xFF, 0x7E
        db 0xFF, 0x7E
        db 0xFF, 0x7E
        db 0x7E, 0x3C
        db 0x3C, 0x00

; Pattern 140h: a cross, same name byte and NAME8 set.
cross_pattern:
        db 0x18, 0x18
        db 0x18, 0x18
        db 0xFF, 0x3C
        db 0xFF, 0x3C
        db 0xFF, 0x3C
        db 0xFF, 0x3C
        db 0x18, 0x18
        db 0x18, 0x18

; Vertical bars with a transparent column: sprites show through value 0 even
; when PRIOR is set.
bar_pattern:
        db 0xCC, 0x33
        db 0xCC, 0x33
        db 0xCC, 0x33
        db 0xCC, 0x33
        db 0xCC, 0x33
        db 0xCC, 0x33
        db 0xCC, 0x33
        db 0xCC, 0x33

title:       db "110 NAME8 AND PRIOR",0
line_one:    db "SAME NAME BYTE 40H TWICE:",0
line_two:    db "NAME8 CLEAR, THEN NAME8 SET",0
line_three:  db "THE SAME BAND, PRIOR CLEAR AND SET",0
behind_text: db "PRIOR CLEAR: SPRITES IN FRONT",0
front_text:  db "PRIOR SET: TILE IN FRONT",0

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/sega_font_6x8.inc"
        include "include/extended_sprite_overlay.inc"
        ROM_END
