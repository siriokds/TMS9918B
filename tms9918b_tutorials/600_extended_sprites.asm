; Lesson 600: sprite pairs, BANK and the eight-channel limit.
;
; Three properties of the extended sprite attribute byte are shown at once over
; a plain Graphics1X background:
;
;   PAIR (10h)  set on an odd sprite, it turns the pair 2k, 2k+1 into the two
;               bit planes of one object. Where only sprite 2k has a pixel the
;               colour is entry 1, where only sprite 2k+1 has one it is entry 2
;               and where both have one it is entry 3, all from the palette of
;               sprite 2k. The two sprites keep their own position and name, so
;               they can be placed exactly on top of each other, as here, or
;               offset. Their overlap never sets the coincidence flag.
;   BANK (20h)  the pattern comes from the 2048 bytes that follow the table
;               located by R6, which gives 512 patterns without a second name
;               bit. Two sprites with the same name byte and different BANK
;               therefore show different shapes.
;   the limit   a pair occupies two of the eight channels. The bottom row holds
;               four pairs, that is eight channels, plus one single sprite that
;               is dropped; its number appears in S0 and the program publishes
;               the status byte at C100h.

        include "include/tms9918b_hardware.inc"
        ROM_START start

G1X_PATTERNS:   equ 0x0000      ; R4 = 00h
G1X_NAMES:      equ 0x3800      ; R2 = 0Eh
SAT:            equ 0x3E00      ; R5 = 7Ch
SPRITE_BANK0:   equ 0x2800      ; R6 = 05h
SPRITE_BANK1:   equ 0x3000      ; the table selected by BANK
STATUS_BYTE:    equ 0xC100

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
        ld hl,palette
        call vdp_load_palette

        ld hl,bank0_patterns
        ld de,SPRITE_BANK0
        ld bc,bank0_patterns_end-bank0_patterns
        call vdp_copy
        ld hl,bank1_patterns
        ld de,SPRITE_BANK1
        ld bc,bank1_patterns_end-bank1_patterns
        call vdp_copy
        ld hl,sat
        ld de,SAT
        ld bc,sat_end-sat
        call vdp_copy

        ld hl,G1X_NAMES
        ld bc,32*24
        ld d,0
        ld e,0
        call vdp_fill_pairs

        CELL_AT 1, 5
        ld hl,title
        ld c,0
        call vdp_print_ascii_pairs
        CELL_AT 4, 1
        ld hl,pair_text
        ld c,0
        call vdp_print_ascii_pairs
        CELL_AT 5, 1
        ld hl,pair_text2
        ld c,0
        call vdp_print_ascii_pairs
        CELL_AT 10, 1
        ld hl,bank_text
        ld c,0
        call vdp_print_ascii_pairs
        CELL_AT 11, 1
        ld hl,bank_text2
        ld c,0
        call vdp_print_ascii_pairs
        CELL_AT 16, 1
        ld hl,limit_text
        ld c,0
        call vdp_print_ascii_pairs
        CELL_AT 17, 1
        ld hl,limit_text2
        ld c,0
        call vdp_print_ascii_pairs

        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers

.frame:
        call wait_vblank
        in a,(VDP_CTRL)         ; S0: 5S and the number of the ninth sprite
        ld (STATUS_BYTE),a
        jr .frame

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x80, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01
registers_visible: db 0x00, 0xC0, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01

extended_registers:
        db 11, R11_XE|R11_MX
        db 12, 0x00
        db 0xFF

palette:
        db 0x00, 0x0F, 0x0E, 0x04       ; 0: text
        db 0x00, 0x09, 0x0B, 0x0F       ; 1: the three colours of a pair
        db 0x00, 0x07, 0x06, 0x0C       ; 2: the BANK comparison
        db 0x00, 0x03, 0x02, 0x0C       ; 3: the channel limit row

; Sprite attribute table. Raw Y is one less than the first visible line.
sat:
        ; Row 1, three-colour pair: sprites 0 and 1 at the same position.
        ; Sprite 0 is the disc (plane 0), sprite 1 the ring (plane 1); their
        ; overlap takes entry 3 of palette 1.
        db  55, 40,0,0x05               ; palette 1, entry 1
        db  55, 40,1,SPR_PAIR           ; the palette and entry bits are ignored
        ; The same pair offset by four pixels: the two planes only partly
        ; overlap, so all three colours are visible at once.
        db  55, 88,0,0x05
        db  55, 92,1,SPR_PAIR
        ; Two single sprites with the same patterns for comparison.
        db  55,140,0,0x06               ; palette 1, entry 2
        db  55,170,1,0x07               ; palette 1, entry 3

        ; Row 2, BANK: the same name byte 00h twice, once from each table.
        db 103, 40,0,0x09               ; palette 2, entry 1, BANK clear
        db 103, 88,0,0x09|SPR_BANK      ; same name, second pattern table

        ; Row 3, the channel limit: four pairs and one single sprite.
        db 151, 16,2,0x0D               ; four pairs: eight channels
        db 151, 16,3,SPR_PAIR
        db 151, 56,2,0x0D
        db 151, 56,3,SPR_PAIR
        db 151, 96,2,0x0D
        db 151, 96,3,SPR_PAIR
        db 151,136,2,0x0D
        db 151,136,3,SPR_PAIR
        db 151,192,2,0x0F               ; the ninth channel: dropped
        db 0xD0
sat_end:

; Bank 0 patterns: 0 disc, 1 ring, 2 square, 3 diagonal.
bank0_patterns:
        db 0x3C,0x7E,0xFF,0xFF,0xFF,0xFF,0x7E,0x3C
        db 0x3C,0x42,0x81,0x81,0x81,0x81,0x42,0x3C
        db 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
        db 0x81,0x42,0x24,0x18,0x18,0x24,0x42,0x81
bank0_patterns_end:

; Bank 1: name 00h is a triangle instead of a disc.
bank1_patterns:
        db 0x10,0x38,0x38,0x7C,0x7C,0xFE,0xFE,0x00
        db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
bank1_patterns_end:

title:      db "600 PAIRS, BANK AND LIMITS",0
pair_text:  db "PAIR: SPRITE 2K+1 IS THE SECOND PLANE",0
pair_text2: db "SAME PLACE, OFFSET, THEN TWO SINGLES",0
bank_text:  db "BANK: SAME NAME BYTE, TWO TABLES",0
bank_text2: db "R6 SELECTS THE FIRST, BANK THE SECOND",0
limit_text: db "FOUR PAIRS USE ALL EIGHT CHANNELS",0
limit_text2: db "THE NINTH SPRITE IS DROPPED - SEE S0",0

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
