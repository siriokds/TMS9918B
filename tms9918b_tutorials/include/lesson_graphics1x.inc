; Shared body for lessons 100/101: Graphics1X, the first extended mode.
;
; Three things change with respect to Graphics I:
;   1. the Name Table holds two bytes per cell, the pattern name and an
;      attribute, stored next to each other because the VDP reads them in one
;      memory cycle;
;   2. a pattern is sixteen bytes, two per row: stratum 0 then stratum 1, so a
;      pixel value is stratum0_bit + 2 * stratum1_bit;
;   3. values 1..3 select entries of the palette named by the attribute, and
;      value 0 always shows the backdrop of R7.
;
; The screen prints its own explanation with palette 0 and repeats one ring
; pattern in four blocks, one per palette. The pattern bytes never change: only
; the two attribute bits do.

        include "include/tms9918b_hardware.inc"
        ROM_START start

EXT_SAT:             equ 0x3E00
EXT_SPRITE_PATTERNS: equ 0x2800

G1X_PATTERNS:   equ 0x0000      ; R4 = 00h, 512 patterns x 16 bytes
G1X_NAMES:      equ 0x3800      ; R2 = 0Eh, 768 entries x 2 bytes
G1X_SAT:        equ 0x3E00      ; R5 = 7Ch
RING:           equ 64          ; pattern name of the ring

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call tms9918b_require
        call vdp_clear

        ld de,G1X_PATTERNS
        call vdp_load_font_2bpp
        ld hl,ring_pattern
        ld de,G1X_PATTERNS + RING*16
        ld bc,16
        call vdp_copy

        ld hl,palette
        call vdp_load_palette

        ; Every cell starts as pattern 0 with palette 0.
        ld hl,G1X_NAMES
        ld bc,32*24
        ld d,0
        ld e,0
        call vdp_fill_pairs

        ld de,G1X_NAMES + 2*(1*32 + 4)
        ld hl,title
        ld c,0
        call vdp_print_ascii_pairs
        ld de,G1X_NAMES + 2*(3*32 + 1)
        ld hl,line_one
        ld c,0
        call vdp_print_ascii_pairs
        ld de,G1X_NAMES + 2*(5*32 + 1)
        ld hl,line_two
        ld c,0
        call vdp_print_ascii_pairs
        ld de,G1X_NAMES + 2*(7*32 + 1)
        ld hl,line_three
        ld c,0
        call vdp_print_ascii_pairs

        ; Four blocks of six columns, each with the same ring pattern and a
        ; different palette in the attribute byte.
        ld b,0                  ; palette number
.block:
        ld a,b
        add a,a
        add a,a
        add a,a                 ; first column of the block
        add a,1
        ld l,a
        ld h,0
        add hl,hl               ; two bytes per entry
        ld de,G1X_NAMES + 2*(11*32)
        add hl,de
        ex de,hl                ; DE = first entry of the block
        ld c,b                  ; attribute: palette number, no NAME8, no PRIOR
        push bc
        call .draw_block
        pop bc
        inc b
        ld a,b
        cp 4
        jr nz,.block

        IF LESSON_WITH_SPRITES
        call extended_sprite_overlay_load
        ELSE
        ; Sprites are enabled in every extended graphics mode: terminate the
        ; attribute table even when the lesson displays none.
        ld hl,G1X_SAT
        ld bc,1
        ld e,0xD0
        call vdp_fill
        ENDIF

        ; Caption each block with its palette number, printed in palette 0.
        ld de,G1X_NAMES + 2*(18*32 + 1)
        ld hl,captions
        ld c,0
        call vdp_print_ascii_pairs

        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers
        IF LESSON_WITH_SPRITES
.animation_loop:
        call wait_vblank
        call extended_sprite_overlay_animate
        jr .animation_loop
        ELSE
        jp freeze
        ENDIF

; DE = first entry, C = attribute. Draws six rings on six rows.
.draw_block:
        ld b,6                  ; rows
.row:
        push bc
        push de
        ex de,hl
        call vdp_set_write
        ld b,6                  ; columns
.cell:
        ld a,RING
        out (VDP_DATA),a
        push af
        pop af
        ld a,c
        out (VDP_DATA),a
        push af
        pop af
        djnz .cell
        pop de
        ex de,hl
        ld de,64                ; next tile row
        add hl,de
        ex de,hl
        pop bc
        djnz .row
        ret

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x80, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01
registers_visible: db 0x00, 0xC0, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01

extended_registers:
        db 11, R11_XE|R11_MX       ; extended interface and extended modes
        db 12, 0x00             ; no mask, no locks, 40-column text
        db 8, 0x00              ; no scrolling in this lesson
        db 9, 0x00
        db 0xFF

; Entry 0 of every palette is never displayed: value 0 is the backdrop.
palette:
        db 0x00, 0x0F, 0x0E, 0x04                       ; 0: white, grey, dark blue
        db 0x00, 0x0B, LUM_3_4|0x0B, LUM_1_2|0x0B       ; 1: one colour, three luminances
        db 0x00, 0x09, 0x08, 0x06                       ; 2: light, medium and dark red
        db 0x00, 0x03, 0x02, 0x0C                       ; 3: light, medium and dark green

; Values 1 (outer), 2 (middle) and 3 (inner) around a transparent centre.
;               s0    s1
ring_pattern:
        db 0xFF, 0x00
        db 0x81, 0x7E
        db 0xBD, 0x7E
        db 0xA5, 0x66
        db 0xA5, 0x66
        db 0xBD, 0x7E
        db 0x81, 0x7E
        db 0xFF, 0x00

        IF LESSON_WITH_SPRITES
title:      db "101 GRAPHICS1X + SPRITES",0
        ELSE
title:      db "100 GRAPHICS1X",0
        ENDIF
line_one:   db "NAME AND ATTRIBUTE PER CELL",0
line_two:   db "16 BYTE PATTERNS - 2 BITS PER PIXEL",0
        IF LESSON_WITH_SPRITES
line_three: db "NINE SPRITES: THE NINTH IS DROPPED",0
        ELSE
line_three: db "SAME PATTERN - FOUR PALETTES",0
        ENDIF
captions:   db "PAL0    PAL1    PAL2    PAL3",0

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/sega_font_6x8.inc"
        IF LESSON_WITH_SPRITES
        include "include/extended_sprite_overlay.inc"
        ENDIF
        ROM_END
