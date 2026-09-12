; Shared body for lessons 310: BitmapQ, one bitmap block per screen third.
;
; With M3 set the bitmap is banked exactly like the Graphics II pattern
; generator. R4 = 03h gives every third its own 4096-byte block at 0000h,
; 1000h and 2000h, and the line inside a block is Y AND 63. R4 = 00h disables
; both banking bits, so all three thirds read the first block and show the
; same 64 lines three times.
;
; Each block holds a different bar width, so the three bands are immediately
; distinguishable. The program alternates R4 between 03h and 00h every two
; seconds; the Palette Map never changes, which shows that banking affects the
; picture only, not the colour zones.

        include "include/tms9918b_hardware.inc"
        ROM_START start

EXT_SAT:             equ 0x3B00
EXT_SPRITE_PATTERNS: equ 0x3000

BITMAP_BLOCK:   equ 0x1000      ; distance between blocks
BITMAP_SPRITES: equ 0x3000      ; R6 = 06h
PALETTE_MAP:    equ 0x3800      ; R2 = 0Eh
BITMAP_SAT:     equ 0x3B00      ; R5 = 76h

BANK_STATE:     equ 0xC100
BANK_TIMER:     equ 0xC101

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call tms9918b_require
        call vdp_clear
        ld hl,palette
        call vdp_load_palette

        ; Palette Map: four vertical zones, identical for every third.
        ld hl,PALETTE_MAP
        call vdp_set_write
        ld c,24
.map_row:
        ld b,0
.map_area:
        ld a,b
        rrca
        rrca
        rrca
        and 0x03
        out (VDP_DATA),a
        push af
        pop af
        inc b
        ld a,b
        cp 32
        jr nz,.map_area
        dec c
        jr nz,.map_row

        ; Three blocks of 64 lines. The bar pattern of a block comes from the
        ; table below: stratum 0 and stratum 1 of every eight pixels.
        ld ix,bars
        ld de,0x0000
        ld b,3
.block:
        push bc
        push de
        ex de,hl
        call vdp_set_write
        ld c,64                 ; lines in a block
.line:
        ld b,32                 ; groups of eight pixels
.group:
        ld a,(ix+0)
        out (VDP_DATA),a
        push af
        pop af
        ld a,(ix+1)
        out (VDP_DATA),a
        push af
        pop af
        djnz .group
        dec c
        jr nz,.line
        inc ix
        inc ix
        pop hl
        ld de,BITMAP_BLOCK
        add hl,de
        ex de,hl
        pop bc
        djnz .block

        ld hl,sega_font_6x8
        ld de,BITMAP_SPRITES
        ld bc,sega_font_6x8_end-sega_font_6x8
        call vdp_copy
        IF LESSON_WITH_SPRITES
        call extended_sprite_overlay_load
        ELSE
        ld hl,caption
        ld de,BITMAP_SAT
        ld bc,caption_end-caption
        call vdp_copy
        ENDIF

        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers

        ld a,0x03
        ld (BANK_STATE),a
        ld a,120
        ld (BANK_TIMER),a
.frame:
        call wait_vblank
        IF LESSON_WITH_SPRITES
        call extended_sprite_overlay_animate
        ENDIF
        ld hl,BANK_TIMER
        dec (hl)
        jr nz,.frame
        ld (hl),120
        ld a,(BANK_STATE)
        xor 0x03
        ld (BANK_STATE),a
        ld l,a
        ld h,4
        call vdp_set_register
        jr .frame

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x02, 0x88, 0x0E, 0x00, 0x03, 0x76, 0x06, 0x01
registers_visible: db 0x02, 0xC8, 0x0E, 0x00, 0x03, 0x76, 0x06, 0x01

extended_registers:
        db 11, R11_XE|R11_MX
        db 12, 0x00
        db 0xFF

; Stratum 0 and stratum 1 of one block: one, two and four pixel bars.
bars:
        db 0x55, 0x33           ; block 0: values 0 1 2 3 every pixel
        db 0x33, 0x0F           ; block 1: two pixels each
        db 0x0F, 0x00           ; block 2: four pixels, values 0 and 1 only

palette:
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0B, LUM_3_4|0x0B, LUM_1_2|0x0B
        db 0x00, 0x09, 0x08, 0x06
        db 0x00, 0x03, 0x02, 0x0C

caption:
        db 7, 8,'3'-32,0x01, 7, 16,'1'-32,0x01, 7, 24,'0'-32,0x01
        db 7, 40,'B'-32,0x01, 7, 48,'M'-32,0x01, 7, 56,'Q'-32,0x01
        db 0xD0
caption_end:

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/sega_font_6x8.inc"
        IF LESSON_WITH_SPRITES
        include "include/extended_sprite_overlay.inc"
        ENDIF
        ROM_END
