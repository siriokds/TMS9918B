; Shared body for lessons 010/011: locked-device Graphics I regression.
;
; Graphics I has a 32x24 one-byte Name Table, 256 8x8 patterns and one colour
; byte for each group of eight patterns. The lower half deliberately uses
; pattern numbers 64, 72, 80 and 88: identical geometry receives four colour
; pairs because those names belong to four different groups.

        include "include/tms9918b_hardware.inc"
        ROM_START start

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call vdp_clear

        ld de,PATTERN_TABLE
        call vdp_load_sega_font
        ld hl,test_tile
        ld de,PATTERN_TABLE + 64*8
        ld bc,8
        call vdp_copy
        ld hl,test_tile
        ld de,PATTERN_TABLE + 72*8
        ld bc,8
        call vdp_copy
        ld hl,test_tile
        ld de,PATTERN_TABLE + 80*8
        ld bc,8
        call vdp_copy
        ld hl,test_tile
        ld de,PATTERN_TABLE + 88*8
        ld bc,8
        call vdp_copy

        ld hl,COLOUR_TABLE
        ld bc,32
        ld e,0xF1
        call vdp_fill
        ld hl,group_colours
        ld de,COLOUR_TABLE + 8
        ld bc,4
        call vdp_copy

        ld hl,NAME_TABLE
        ld bc,32*24
        ld e,0
        call vdp_fill
        ld de,NAME_TABLE + 2*32 + 5
        ld hl,title
        call vdp_print_ascii
        ld de,NAME_TABLE + 5*32 + 4
        ld hl,line_one
        call vdp_print_ascii
        ld de,NAME_TABLE + 7*32 + 2
        ld hl,line_two
        call vdp_print_ascii
        IF LESSON_WITH_SPRITES
        ld de,NAME_TABLE + 9*32 + 2
        ld hl,line_three
        call vdp_print_ascii
        ENDIF

        ; Twelve rows of the same shape selected from four colour groups.
        ld hl,NAME_TABLE + 11*32
        call vdp_set_write
        ld d,12
.row:
        ld b,8
.four_groups:
        ld a,64
        out (VDP_DATA),a
        ld a,72
        out (VDP_DATA),a
        ld a,80
        out (VDP_DATA),a
        ld a,88
        out (VDP_DATA),a
        djnz .four_groups
        dec d
        jr nz,.row

        ld hl,SPRITE_TABLE
        ld bc,1
        ld e,0xD0
        call vdp_fill
        IF LESSON_WITH_SPRITES
        call legacy_sprite_overlay_load
        ENDIF
        ld hl,registers_visible
        call vdp_set_registers
        IF LESSON_WITH_SPRITES
.animation_loop:
        call wait_vblank
        call legacy_sprite_overlay_animate
        jr .animation_loop
        ELSE
        jp freeze
        ENDIF

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x80, 0x0E, 0x80, 0x00, 0x76, 0x03, 0x01
registers_visible: db 0x00, 0xC0, 0x0E, 0x80, 0x00, 0x76, 0x03, 0x01
group_colours:     db 0xF4,0xB1,0x71,0xD1
test_tile:         db 0x81,0x42,0x24,0x18,0x18,0x24,0x42,0x81
        IF LESSON_WITH_SPRITES
title:             db "011 GRAPHICS I + SPRITES",0
        ELSE
title:             db "010 LEGACY GRAPHICS I",0
        ENDIF
        IF LESSON_WITH_SPRITES
line_one:          db "MODE: GRAPHICS I - 32 X 24",0
line_two:          db "SPRITES: 8 X 8 NORMAL",0
line_three:        db "RIGHTMOST IS 5TH - MAY VANISH",0
        ELSE
line_one:          db "32 X 24 - 8 X 8 PATTERNS",0
line_two:          db "ONE COLOUR PAIR PER 8 PATTERNS",0
        ENDIF

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        IF LESSON_WITH_SPRITES
        include "include/legacy_sprite_overlay.inc"
        ENDIF
        ROM_END
