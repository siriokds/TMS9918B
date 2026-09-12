; Lesson 050: locked-device sprite pipeline regression.
;
; One static frame demonstrates the original TMS9918 sprite rules:
;   - the visible top is raw Y + 1;
;   - four sprites may be selected on one scanline;
;   - the fifth is dropped and reported in S0 bits 4..0;
;   - colour bit 7 moves a sprite 32 pixels left (early clock);
;   - overlapping opaque sprite pixels set collision;
;   - raw Y D0h terminates the SAT.
;
; The accumulated status byte is stored at CPU address C100h. Once a complete
; frame has run it must be E4h: frame, fifth-sprite, collision, index 4.

        include "include/tms9918b_hardware.inc"

STATUS_LAST: equ 0xC100
STATUS_ACC:  equ 0xC101
FRAME_COUNT: equ 0xC102

        ROM_START start

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call vdp_clear

        ; A quiet Graphics I background documents what each sprite row tests.
        ld de,PATTERN_TABLE
        call vdp_load_sega_font
        ld hl,COLOUR_TABLE
        ld bc,32
        ld e,0xF1
        call vdp_fill
        ld hl,NAME_TABLE
        ld bc,32*24
        ld e,0
        call vdp_fill
        ld de,NAME_TABLE + 1*32 + 5
        ld hl,title
        call vdp_print_ascii
        ld de,NAME_TABLE + 6*32 + 2
        ld hl,limit_text
        call vdp_print_ascii
        ld de,NAME_TABLE + 12*32 + 2
        ld hl,clock_text
        call vdp_print_ascii
        ld de,NAME_TABLE + 18*32 + 2
        ld hl,collision_text
        call vdp_print_ascii

        ld hl,sprite_pattern
        ld de,SPRITE_PATTERN
        ld bc,sprite_pattern_end-sprite_pattern
        call vdp_copy
        ld hl,sprite_attributes
        ld de,SPRITE_TABLE
        ld bc,sprite_attributes_end-sprite_attributes
        call vdp_copy

        xor a
        ld (STATUS_LAST),a
        ld (STATUS_ACC),a
        ld (FRAME_COUNT),a
        in a,(VDP_CTRL)        ; discard setup status
        ld hl,registers_visible
        call vdp_set_registers

; Every status read clears the hardware flags. Accumulate all reads until F
; appears, then publish the complete frame result for the debugger.
main_loop:
        in a,(VDP_CTRL)
        ld hl,STATUS_ACC
        or (hl)
        ld (hl),a
        bit 7,a
        jr z,main_loop
        ld (STATUS_LAST),a
        xor a
        ld (STATUS_ACC),a
        ld hl,FRAME_COUNT
        inc (hl)
        jr main_loop

; Graphics I, 16 KiB, 16x16 sprites, no magnification.
;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x82, 0x0E, 0x80, 0x00, 0x76, 0x03, 0x01
registers_visible: db 0x00, 0xC2, 0x0E, 0x80, 0x00, 0x76, 0x03, 0x01

title:          db "050 LEGACY SPRITES",0
limit_text:     db "FOUR SHOWN - FIFTH DROPPED",0
clock_text:     db "EARLY CLOCK MOVES LEFT 32",0
collision_text: db "OVERLAP SETS COLLISION",0

; One 16x16 outline in the TMS four-pattern layout: top-left, bottom-left,
; top-right, bottom-right. In 16x16 mode all SAT names below resolve to zero.
sprite_pattern:
        db 0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0x81
        db 0x81,0x81,0x81,0x81,0x81,0x81,0x81,0xFF
        db 0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0x81
        db 0x81,0x81,0x81,0x81,0x81,0x81,0x81,0xFF
sprite_pattern_end:

sprite_attributes:
        ; raw Y, X, name, colour
        db 55,32,0,15
        db 55,72,0,6
        db 55,112,0,10
        db 55,152,0,4
        db 55,192,0,14       ; fifth on lines 56..71: not displayed
        db 103,72,0,0x8C     ; displayed at X=40 through early clock
        db 151,104,0,15
        db 151,108,0,6       ; opaque overlap with sprite 6
        db 0xD0              ; terminator: entries 9..31 do not exist
sprite_attributes_end:

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        ROM_END

