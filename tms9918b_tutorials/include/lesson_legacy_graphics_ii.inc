; Shared body for lessons 020/021: locked-device Graphics II regression.
;
; The screen is divided into three 64-line regions. With R4=03h and R3=FFh,
; each region selects its own 2 KiB pattern and colour block. The same name 59
; therefore produces three independent shapes and colour-per-row sequences.
; Lesson 021 keeps every background byte and adds the standard sprite overlay.

        include "include/tms9918b_hardware.inc"
        ROM_START start

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call vdp_clear

        ; Replicate the font because Graphics II selects a pattern bank from
        ; the screen third, even when the character name is unchanged.
        ld de,0x0000
        call vdp_load_sega_font
        ld de,0x0800
        call vdp_load_sega_font
        ld de,0x1000
        call vdp_load_sega_font
        ld hl,tile_top
        ld de,0x0000 + 59*8
        ld bc,8
        call vdp_copy
        ld hl,tile_middle
        ld de,0x0800 + 59*8
        ld bc,8
        call vdp_copy
        ld hl,tile_bottom
        ld de,0x1000 + 59*8
        ld bc,8
        call vdp_copy

        ; White-on-black text everywhere, then a unique eight-row colour
        ; sequence for test pattern 59 in each bank.
        ld hl,COLOUR_TABLE
        ld bc,0x1800
        ld e,0xF1
        call vdp_fill
        ld hl,colours_top
        ld de,0x2000 + 59*8
        ld bc,8
        call vdp_copy
        ld hl,colours_middle
        ld de,0x2800 + 59*8
        ld bc,8
        call vdp_copy
        ld hl,colours_bottom
        ld de,0x3000 + 59*8
        ld bc,8
        call vdp_copy

        ld hl,NAME_TABLE
        ld bc,32*24
        ld e,0
        call vdp_fill
        ld de,NAME_TABLE + 1*32 + 5
        ld hl,title
        call vdp_print_ascii
        ld de,NAME_TABLE + 9*32 + 7
        ld hl,middle_text
        call vdp_print_ascii
        ld de,NAME_TABLE + 17*32 + 7
        ld hl,bottom_text
        call vdp_print_ascii

        ; Three full rows in each third use name 59. The bank, rather than the
        ; name byte, decides which pattern and colours become visible.
        ld hl,NAME_TABLE + 4*32
        ld bc,3*32
        ld e,59
        call vdp_fill
        ld hl,NAME_TABLE + 12*32
        ld bc,3*32
        ld e,59
        call vdp_fill
        ld hl,NAME_TABLE + 20*32
        ld bc,3*32
        ld e,59
        call vdp_fill

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
registers_blank:   db 0x02, 0x80, 0x0E, 0xFF, 0x03, 0x76, 0x03, 0x01
registers_visible: db 0x02, 0xC0, 0x0E, 0xFF, 0x03, 0x76, 0x03, 0x01
tile_top:          db 0x81,0x42,0x24,0x18,0x18,0x24,0x42,0x81
tile_middle:       db 0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55
tile_bottom:       db 0x18,0x3C,0x7E,0xFF,0xFF,0x7E,0x3C,0x18
colours_top:       db 0xF4,0xF4,0xB1,0xB1,0x71,0x71,0xD1,0xD1
colours_middle:    db 0x61,0x61,0xE1,0xE1,0x91,0x91,0xC1,0xC1
colours_bottom:    db 0x31,0x51,0x81,0xA1,0xA1,0x81,0x51,0x31
        IF LESSON_WITH_SPRITES
title:             db "021 GRAPHICS II + SPRITES",0
        ELSE
title:             db "020 LEGACY GRAPHICS II",0
        ENDIF
middle_text:       db "SECOND VRAM BANK",0
bottom_text:       db "THIRD VRAM BANK",0

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        IF LESSON_WITH_SPRITES
        include "include/legacy_sprite_overlay.inc"
        ENDIF
        ROM_END

