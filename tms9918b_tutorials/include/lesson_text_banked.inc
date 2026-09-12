; Shared body for lessons 410/510: the third-banked extended text modes.
;
;   LESSON_T64 = 0  Text40XQ: 40 columns, character and colour per cell
;   LESSON_T64 = 1  Text64Q:  64 columns, one name per cell, colours from R7
;
; M3 banks the character generator per screen third, as in Graphics II and in
; the undocumented Text 1Q. With R4 = 03h the three 2 KiB blocks are at 0000h,
; 0800h and 1000h, so up to 768 characters are available: rows 0..7 read the
; first block, rows 8..15 the second and rows 16..23 the third.
;
; The same text is printed in the three regions with the same name bytes. It
; appears plain, bold and inverse because only the block changes. Every two
; seconds R4 alternates between 03h and 00h: with 00h the middle and bottom
; thirds fall back to the first block and the three regions become identical.

        include "include/tms9918b_hardware.inc"
        ROM_START start

BANK_TOP:       equ 0x0000
BANK_MIDDLE:    equ 0x0800
BANK_BOTTOM:    equ 0x1000
TEXT_ENTRIES:   equ 0x3000      ; R2 = 0Ch
TEXT_SAT:       equ 0x3B00      ; R5 = 76h
CURSOR_SAT:     equ 0x3B00
CURSOR_PATTERNS: equ 0x1800      ; clear of the three font blocks
TRANSFORM:      equ 0xC130
BANK_STATE:     equ 0xC100
BANK_TIMER:     equ 0xC101

    IF LESSON_T64
COLUMNS:        equ 64
    ELSE
COLUMNS:        equ 40
    ENDIF

; Row and column to the address of the first byte of a cell.
    MACRO TEXT_AT row, column
    IF LESSON_T64
        ld de,TEXT_ENTRIES + (row)*COLUMNS + (column)
    ELSE
        ld de,TEXT_ENTRIES + 2*((row)*COLUMNS + (column))
    ENDIF
    ENDM

; HL = text, C = colour byte (Text40XQ only).
    MACRO TEXT_PRINT
    IF LESSON_T64
        call vdp_print_ascii
    ELSE
        call vdp_print_ascii_pairs
    ENDIF
    ENDM

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call tms9918b_require
        call vdp_clear

        ld de,BANK_TOP
        xor a
        call load_font_transformed
        ld de,BANK_MIDDLE
        ld a,1
        call load_font_transformed
        ld de,BANK_BOTTOM
        ld a,2
        call load_font_transformed

    IF LESSON_T64
        ld hl,TEXT_ENTRIES
        ld bc,COLUMNS*24
        ld e,0
        call vdp_fill
    ELSE
        ld hl,TEXT_ENTRIES
        ld bc,COLUMNS*24
        ld d,0
        ld e,0
        call vdp_fill_pairs
    ENDIF

        TEXT_AT 1, 2
        ld hl,title
        ld c,0xF1
        TEXT_PRINT
        TEXT_AT 3, 2
        ld hl,line_one
        ld c,0xB1
        TEXT_PRINT
        TEXT_AT 5, 2
        ld hl,sample
        ld c,0xF1
        TEXT_PRINT

        TEXT_AT 9, 2
        ld hl,middle_note
        ld c,0x91
        TEXT_PRINT
        TEXT_AT 11, 2
        ld hl,sample
        ld c,0xF1
        TEXT_PRINT

        TEXT_AT 17, 2
        ld hl,bottom_note
        ld c,0x31
        TEXT_PRINT
        TEXT_AT 19, 2
        ld hl,sample
        ld c,0xF1
        TEXT_PRINT

        IF LESSON_WITH_CURSOR
    IF LESSON_T64
        ld b,21*8-1
        ld c,4*40               ; two-pixel steps: column 40
    ELSE
        ld b,21*8-1
        ld c,6*26               ; pixels: column 26
    ENDIF
        call cursor_load
        ELSE
        ld hl,TEXT_SAT
        ld bc,1
        ld e,0xD0
        call vdp_fill
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
        IF LESSON_WITH_CURSOR
        call cursor_blink
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

; DE = block address, A = transform: 0 plain, 1 bold, 2 inverse.
load_font_transformed:
        ld (TRANSFORM),a
        ex de,hl
        call vdp_set_write
        ld hl,sega_font_6x8
        ld bc,sega_font_6x8_end-sega_font_6x8
.byte:
        ld a,(TRANSFORM)
        dec a
        jr z,.bold
        dec a
        jr z,.inverse
        ld a,(hl)
        jr .send
.bold:
        ld a,(hl)
        ld e,a
        rrca
        and 0x7F
        or e
        jr .send
.inverse:
        ld a,(hl)
        cpl
    IF LESSON_T64
        ; Text64 displays all eight bits, so the inverse font would touch the
        ; next character. The last column is kept clear as a separator.
        and 0xFE
    ELSE
        and 0xFC                ; bits 1..0 are never displayed at 40 columns
    ENDIF
.send:
        out (VDP_DATA),a
        inc hl
        dec bc
        ld a,b
        or c
        jr nz,.byte
        ret

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x02, 0x90, 0x0C, 0x00, 0x03, 0x76, 0x03, 0xF4
registers_visible: db 0x02, 0xD0, 0x0C, 0x00, 0x03, 0x76, 0x03, 0xF4

extended_registers:
        db 11, R11_XE|R11_MX
    IF LESSON_T64
        db 12, R12_T64
    ELSE
        db 12, 0x00
    ENDIF
        db 0xFF

    IF LESSON_T64
        IF LESSON_WITH_CURSOR
title:       db "511 TEXT64Q + CURSOR",0
        ELSE
title:       db "510 TEXT64Q - 64 COLUMNS WITH THIRD BANKED FONT",0
        ENDIF
line_one:    db "R4 = 03H: BLOCKS AT 0000H, 0800H AND 1000H",0
    ELSE
        IF LESSON_WITH_CURSOR
title:       db "411 TEXT40XQ + CURSOR",0
        ELSE
title:       db "410 TEXT40XQ - THIRD BANKED FONT",0
        ENDIF
line_one:    db "COLOUR PER CELL AND 768 CHARACTERS",0
    ENDIF
sample:      db "ABCDEFGHIJKLM 0123456789",0
middle_note: db "SECOND BLOCK: BOLD",0
bottom_note: db "THIRD BLOCK: INVERSE",0

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/text_cursor.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
