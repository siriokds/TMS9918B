; Shared body for lessons 030/031/032: Multicolor and Multicolor Q.
;
;   LESSON_WITH_SPRITES  adds the animated sprite overlay (031, 032)
;   LESSON_THIRDS        selects Multicolor Q, the undocumented M3 variant
;                        whose pattern generator is banked per screen third
;                        exactly like Graphics II (032)
;
; Multicolor is a 64x48 logical display made from 4x4-pixel colour blocks.
; Each pattern byte supplies the left and right colours of one block pair.
; The Name Table mapping repeats every four character rows. The small title is
; made from ordinary sprites because Multicolor has no character glyph layer.

        include "include/tms9918b_hardware.inc"
        ROM_START start

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call vdp_clear

        ; Canonical Multicolor name mapping: column + (row AND 3) * 32.
        ld hl,NAME_TABLE
        call vdp_set_write
        ld d,0
.name_row:
        ld e,0
.name_column:
        ld a,d
        and 3
        rlca
        rlca
        rlca
        rlca
        rlca
        add a,e
        out (VDP_DATA),a
        inc e
        ld a,e
        cp 32
        jr nz,.name_column
        inc d
        ld a,d
        cp 24
        jr nz,.name_row

        ; 128 names x 8 bytes. Adjacent nibbles always differ, exposing both
        ; halves of every logical 8x4 Multicolor cell. Multicolor Q repeats the
        ; block three times with a different colour offset, so the same name
        ; produces three different bands.
        IF LESSON_THIRDS
        ld ix,0x0000            ; first pattern bank
        ld iyl,0                ; colour offset of the bank
.bank:
        push ix
        pop hl
        ELSE
        ld hl,0x0800
        ENDIF
        call vdp_set_write
        ld b,0
        ld c,0
.pattern:
        ld a,b
        add a,c
        IF LESSON_THIRDS
        add a,iyl
        ENDIF
        and 0x0F
        rlca
        rlca
        rlca
        rlca
        ld e,a
        ld a,b
        add a,c
        add a,8
        IF LESSON_THIRDS
        add a,iyl
        ENDIF
        and 0x0F
        or e
        out (VDP_DATA),a
        inc c
        ld a,c
        cp 8
        jr nz,.pattern
        ld c,0
        inc b
        ld a,b
        cp 128
        jr nz,.pattern
        IF LESSON_THIRDS
        ld de,0x0800
        add ix,de
        ld a,iyl
        add a,5
        ld iyl,a
        push ix
        pop de
        ld a,d
        cp 0x18                 ; three banks written
        jr nz,.bank
        ENDIF

        IF LESSON_WITH_SPRITES
        ; The animated overlay replaces the caption: Multicolor has no
        ; character layer, so the moving group and the fifth-sprite probe are
        ; the only readable objects.
        call legacy_sprite_overlay_load
        ld hl,registers_visible
        call vdp_set_registers
.animation_loop:
        call wait_vblank
        call legacy_sprite_overlay_animate
        jr .animation_loop
        ELSE
        ; Four letters per scanline remain below the legacy sprite limit.
        ld hl,sega_font_6x8
        ld de,SPRITE_PATTERN
        ld bc,sega_font_6x8_end-sega_font_6x8
        call vdp_copy
        ld hl,title_sprites
        ld de,SPRITE_TABLE
        ld bc,title_sprites_end-title_sprites
        call vdp_copy

        ld hl,registers_visible
        call vdp_set_registers
        jp freeze
        ENDIF

;                    R0    R1    R2    R3    R4    R5    R6    R7
        IF LESSON_THIRDS
; Multicolor Q: M3 = 1 and R4 = 03h, so the three 2 KiB pattern banks are at
; 0000h, 0800h and 1000h, as in Graphics II.
registers_blank:   db 0x02, 0x88, 0x0E, 0x00, 0x03, 0x76, 0x03, 0x01
registers_visible: db 0x02, 0xC8, 0x0E, 0x00, 0x03, 0x76, 0x03, 0x01
        ELSE
registers_blank:   db 0x00, 0x88, 0x0E, 0x00, 0x01, 0x76, 0x03, 0x01
registers_visible: db 0x00, 0xC8, 0x0E, 0x00, 0x01, 0x76, 0x03, 0x01
        ENDIF

; Raw Y is one less than the first visible line.
title_sprites:
        db 7,104,'0'-32,15, 7,112,'3'-32,15, 7,120,'0'-32,15
        db 23,100,'M'-32,15,23,108,'U'-32,15,23,116,'L'-32,15,23,124,'T'-32,15
        db 39,100,'I'-32,15,39,108,'C'-32,15,39,116,'O'-32,15,39,124,'L'-32,15
        db 55,104,'O'-32,15,55,112,'R'-32,15
        db 0xD0
title_sprites_end:

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        IF LESSON_WITH_SPRITES
        include "include/legacy_sprite_overlay.inc"
        ENDIF
        ROM_END

