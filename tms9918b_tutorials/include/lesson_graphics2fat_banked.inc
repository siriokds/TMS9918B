; Shared body for lessons 210: Graphics2Fat with third banking.
;
; Lesson 200 wrote the same four patterns into all three pattern blocks, so the
; screen looked uniform. Here each block holds a different colour mapping: the
; name bytes are identical on the whole screen, but the top third resolves them
; in the block at 0000h, the middle third in the block at 1000h and the bottom
; third in the block at 2000h, because R4 = 03h enables both banking bits.
;
; Every two seconds R4 alternates between 03h and 00h. With 00h the middle and
; bottom thirds fall back to the first block and the three bands become
; identical, which is the clearest way to see what the banking bits do.

        include "include/tms9918b_hardware.inc"
        ROM_START start

EXT_SAT:             equ 0x3E00
EXT_SPRITE_PATTERNS: equ 0x3000

FAT_PATTERNS:   equ 0x0000      ; three blocks: 0000h, 1000h, 2000h
FAT_NAMES:      equ 0x3800      ; R2 = 0Eh
FAT_SAT:        equ 0x3E00      ; R5 = 7Ch
FAT_SPRITES:    equ 0x3000      ; R6 = 06h

BANK_STATE:     equ 0xC100
BANK_TIMER:     equ 0xC101

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call tms9918b_require
        call vdp_clear

        ; Block t holds four patterns whose stratum 1 byte is taken from its
        ; own row of the table: the same four names give three colour sets.
        ld ix,mapping
        ld de,FAT_PATTERNS
        ld b,3
.block:
        push bc
        push de
        ld b,4                  ; four patterns per block
.pattern:
        push bc
        push de
        ex de,hl
        call vdp_set_write
        ld a,(ix+0)
        ld c,a                  ; stratum 1 of this pattern
        ld b,8                  ; eight rows
.row:
        ld a,0x1B               ; stratum 0: dots 0, 1, 2, 3
        out (VDP_DATA),a
        push af
        pop af
        ld a,c
        out (VDP_DATA),a
        push af
        pop af
        djnz .row
        inc ix
        pop de
        ld hl,16
        add hl,de
        ex de,hl
        pop bc
        djnz .pattern
        pop hl
        ld de,0x1000            ; next pattern block
        add hl,de
        ex de,hl
        pop bc
        djnz .block

        ; Identical names everywhere: eight groups of the four patterns.
        ld hl,FAT_NAMES
        call vdp_set_write
        ld c,24
.name_row:
        ld b,8
.group:
        xor a
.cell:
        out (VDP_DATA),a
        push af
        pop af
        push af
        xor a                   ; attribute: no NAME8, no PRIOR
        out (VDP_DATA),a
        pop af
        inc a
        cp 4
        jr nz,.cell
        djnz .group
        dec c
        jr nz,.name_row

        ld hl,sega_font_6x8
        ld de,FAT_SPRITES
        ld bc,sega_font_6x8_end-sega_font_6x8
        call vdp_copy
        IF LESSON_WITH_SPRITES
        call extended_sprite_overlay_load
        ELSE
        ld hl,caption
        ld de,FAT_SAT
        ld bc,caption_end-caption
        call vdp_copy
        ENDIF
        ld hl,palette
        call vdp_load_palette
        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers

        xor a
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
        xor 0x03                ; R4 = 03h <-> 00h
        ld (BANK_STATE),a
        ld l,a
        ld h,4
        call vdp_set_register
        jr .frame

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x02, 0x80, 0x0E, 0x00, 0x03, 0x7C, 0x06, 0x01
registers_visible: db 0x02, 0xC0, 0x0E, 0x00, 0x03, 0x7C, 0x06, 0x01

extended_registers:
        db 11, R11_XE|R11_MX
        db 12, 0x00
        db 0xFF

; Stratum 1 of the four patterns of each block. Stratum 0 is always 1Bh, so
; the dot values of a block are 4k + 0, 1, 2, 3 with k from this table.
mapping:
        db 0x00, 0x55, 0xAA, 0xFF       ; block 0: the complete ramp
        db 0x00, 0x00, 0x55, 0x55       ; block 1: only the first eight colours
        db 0xFF, 0xAA, 0x55, 0x00       ; block 2: the ramp reversed

palette:
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0F, 0x0E, 0x01

caption:
        db 7, 8,'2'-32,0x01, 7, 16,'1'-32,0x01, 7, 24,'0'-32,0x01
        db 7, 40,'B'-32,0x01, 7, 48,'A'-32,0x01, 7, 56,'N'-32,0x01
        db 7, 64,'K'-32,0x01
        db 0xD0
caption_end:

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/sega_font_6x8.inc"
        IF LESSON_WITH_SPRITES
        include "include/extended_sprite_overlay.inc"
        ENDIF
        ROM_END
