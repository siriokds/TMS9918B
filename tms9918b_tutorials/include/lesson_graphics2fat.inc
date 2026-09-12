; Shared body for lessons 200: Graphics2Fat, sixteen colours per dot.
;
; The cell and the sixteen-byte pattern are those of Graphics1X, but a pattern
; row describes four dots two pixels wide instead of eight pixels. Dot p takes
; bits (2p, 2p+1) of stratum 0 as the low half of its value and the same bits
; of stratum 1 as the high half. The four-bit result is a TMS colour code used
; directly, so the palette is not involved; code 0 shows the backdrop.
;
; The screen is a complete sixteen-colour ramp. Patterns 0..3 carry colours
; 0..3, 4..7, 8..11 and 12..15: their stratum 0 byte is always 1Bh (the dot
; sequence 0, 1, 2, 3) and only the stratum 1 byte changes. The caption is made
; of sprites because a fat-dot pattern cannot hold a readable glyph.

        include "include/tms9918b_hardware.inc"
        ROM_START start

EXT_SAT:             equ 0x3E00
EXT_SPRITE_PATTERNS: equ 0x3000

FAT_PATTERNS:   equ 0x0000      ; R4 = 03h, three 4096-byte thirds
FAT_NAMES:      equ 0x3800      ; R2 = 0Eh
FAT_SAT:        equ 0x3E00      ; R5 = 7Ch
FAT_SPRITES:    equ 0x3000      ; R6 = 06h

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call tms9918b_require
        call vdp_clear

        ; Patterns 0..3 in all three thirds, so any screen row can show them.
        ld b,3
        ld de,FAT_PATTERNS
.third:
        push bc
        push de
        ld hl,ramp_patterns
        ld bc,4*16
        call vdp_copy
        pop hl
        ld de,0x1000
        add hl,de
        ex de,hl
        pop bc
        djnz .third

        ; Eight repetitions of the four patterns fill a 32-cell row with the
        ; sixteen colours, each dot two pixels wide.
        ld hl,FAT_NAMES
        call vdp_set_write
        ld c,24                 ; rows
.row:
        ld b,8                  ; four patterns per group
.group:
        xor a
.cell:
        out (VDP_DATA),a        ; pattern name
        push af
        pop af
        push af
        xor a                   ; attribute: no NAME8, no PRIOR, palette unused
        out (VDP_DATA),a
        pop af
        inc a
        cp 4
        jr nz,.cell
        djnz .group
        dec c
        jr nz,.row

        ; The caption uses ordinary one-bit sprites: six per scanline stay
        ; below the eight-sprite limit of the extended graphics modes.
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
        IF LESSON_WITH_SPRITES
.animation_loop:
        call wait_vblank
        call extended_sprite_overlay_animate
        jr .animation_loop
        ELSE
        jp freeze
        ENDIF

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x02, 0x80, 0x0E, 0x00, 0x03, 0x7C, 0x06, 0x01
registers_visible: db 0x02, 0xC0, 0x0E, 0x00, 0x03, 0x7C, 0x06, 0x01

extended_registers:
        db 11, R11_XE|R11_MX
        db 12, 0x00
        db 0xFF

; Sprites still use the palette; entry 1 of palette 0 is white.
palette:
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0F, 0x0E, 0x01
        db 0x00, 0x0F, 0x0E, 0x01

; Pattern n: dots 4n, 4n+1, 4n+2, 4n+3. Stratum 0 is the low half of every
; dot value (0, 1, 2, 3 = 00 01 10 11 = 1Bh); stratum 1 is the high half.
ramp_patterns:
        rept 8
        db 0x1B, 0x00
        endr
        rept 8
        db 0x1B, 0x55
        endr
        rept 8
        db 0x1B, 0xAA
        endr
        rept 8
        db 0x1B, 0xFF
        endr

; Raw Y is one less than the first visible line; X is in pixels.
caption:
        db 15, 64,'2'-32,0x01, 15, 72,'0'-32,0x01, 15, 80,'0'-32,0x01
        db 15, 96,'F'-32,0x01, 15,104,'A'-32,0x01, 15,112,'T'-32,0x01
        db 31, 64,'1'-32,0x01, 31, 72,'6'-32,0x01
        db 31, 88,'C'-32,0x01, 31, 96,'O'-32,0x01, 31,104,'L'-32,0x01
        db 47, 64,'D'-32,0x01, 47, 72,'O'-32,0x01, 47, 80,'T'-32,0x01
        db 47, 96,'2'-32,0x01, 47,104,'P'-32,0x01, 47,112,'X'-32,0x01
        db 0xD0
caption_end:

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/sega_font_6x8.inc"
        IF LESSON_WITH_SPRITES
        include "include/extended_sprite_overlay.inc"
        ENDIF
        ROM_END
