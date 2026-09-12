; Lesson 700: hardware scrolling, MASK, HLOCK and VLOCK.
;
; R8 moves the picture horizontally and R9 vertically, without a single VRAM
; write: the fetched column becomes (c + 1 + R8/8) AND 31 and the world line
; becomes (line + R9) MOD 192. Three bits of R12 shape the result:
;
;   MASK  (02h)  blanks the leftmost eight pixels. Horizontal scrolling by
;                pixels always shows a partial column there, so MASK is what
;                makes pixel scrolling usable.
;   HLOCK (08h)  character rows 0 and 1 ignore R8: a score bar stays still
;                while the playfield scrolls under it.
;   VLOCK (04h)  columns 24 to 31 ignore R9: a side panel stays still while
;                the playfield scrolls past it.
;
; The playfield is a coordinate grid, so the movement is visible everywhere.
; Every four seconds the program changes what it demonstrates and prints the
; current state in the locked area:
;
;   0  R8 alone          2  R8 and R9 together
;   1  R9 alone         3  both, with MASK, HLOCK and VLOCK all set

        include "include/tms9918b_hardware.inc"
        ROM_START start

G1X_PATTERNS:        equ 0x0000
G1X_NAMES:           equ 0x3800
EXT_SAT:             equ 0x3E00
EXT_SPRITE_PATTERNS: equ 0x2800

SCROLL_X:       equ 0xC160
SCROLL_Y:       equ 0xC161
STATE:          equ 0xC162
STATE_TIMER:    equ 0xC163

MARK:           equ 0x60        ; pattern of a grid intersection
BLOCK:          equ 0x61        ; solid pattern of the side panel

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
        ld hl,mark_pattern
        ld de,G1X_PATTERNS + MARK*16
        ld bc,32
        call vdp_copy
        ld hl,palette
        call vdp_load_palette

        ; The playfield: a grid of marks, with a digit every fourth column so
        ; that horizontal movement can be counted.
        ld hl,G1X_NAMES
        call vdp_set_write
        ld c,0                  ; row
.row:
        ld b,0                  ; column
.cell:
        ld a,b
        and 3
        jr nz,.mark
        ld a,c
        and 3
        jr nz,.mark
        ld a,b
        rrca
        rrca
        and 7
        add a,'0'-32            ; a digit every four rows and columns
        jr .write
.mark:
        ld a,MARK
.write:
        out (VDP_DATA),a
        push af
        pop af
        push af
        ld a,0x01               ; palette 1
        out (VDP_DATA),a
        pop af
        inc b
        ld a,b
        cp 32
        jr nz,.cell
        inc c
        ld a,c
        cp 24
        jr nz,.row

        ; The side panel: columns 24 to 31 of every row, in palette 2. VLOCK
        ; keeps these columns still while everything else scrolls vertically.
        ld c,0
.panel_row:
        ld a,c
        ld l,a
        ld h,0
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl               ; row * 32
        ld de,24
        add hl,de
        add hl,hl               ; two bytes per entry
        ld de,G1X_NAMES
        add hl,de
        call vdp_set_write
        ld b,8
.panel_cell:
        ld a,BLOCK
        out (VDP_DATA),a
        push af
        pop af
        push af
        ld a,0x02               ; palette 2
        out (VDP_DATA),a
        pop af
        push af
        pop af
        djnz .panel_cell
        inc c
        ld a,c
        cp 24
        jr nz,.panel_row

        call extended_sprite_overlay_load
        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers

        xor a
        ld (SCROLL_X),a
        ld (SCROLL_Y),a
        ld (STATE),a
        ld a,240
        ld (STATE_TIMER),a
        call apply_state

.frame:
        call wait_vblank
        call extended_sprite_overlay_animate
        call scroll_step

        ld hl,STATE_TIMER
        dec (hl)
        jr nz,.frame
        ld (hl),240
        ld a,(STATE)
        inc a
        and 3
        ld (STATE),a
        call apply_state
        jr .frame

; Advances R8 and R9 according to the current state.
scroll_step:
        ld a,(STATE)
        cp 1
        jr z,.vertical_only
        ld a,(SCROLL_X)
        inc a
        ld (SCROLL_X),a
        ld l,a
        ld h,8
        call vdp_set_register
        ld a,(STATE)
        or a
        ret z                   ; state 0 scrolls horizontally only
.vertical_only:
        ld a,(SCROLL_Y)
        inc a
        cp 192
        jr c,.store
        xor a
.store:
        ld (SCROLL_Y),a
        ld l,a
        ld h,9
        jp vdp_set_register

; Writes R12 for the current state and describes it in the locked rows.
apply_state:
        ld a,(STATE)
        ld e,a
        ld d,0
        ld hl,r13_values
        add hl,de
        ld l,(hl)
        ld h,12
        call vdp_set_register

        ld hl,G1X_NAMES         ; clear the two locked rows
        ld bc,2*32
        ld d,0
        ld e,0
        call vdp_fill_pairs
        ld a,(STATE)
        add a,a
        ld e,a
        ld d,0
        ld hl,state_texts
        add hl,de
        ld a,(hl)
        inc hl
        ld h,(hl)
        ld l,a
        CELL_AT 0, 1
        ld c,0
        jp vdp_print_ascii_pairs

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x80, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01
registers_visible: db 0x00, 0xC0, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01

extended_registers:
        db 11, R11_XE|R11_MX
        db 8, 0x00
        db 9, 0x00
        db 12, R12_MASK|R12_HLOCK
        db 0xFF

; R12 per state: the first three keep MASK and HLOCK so the caption stays
; readable; the fourth adds VLOCK.
r13_values:
        db R12_MASK|R12_HLOCK
        db R12_MASK|R12_HLOCK
        db R12_MASK|R12_HLOCK
        db R12_MASK|R12_HLOCK|R12_VLOCK

state_texts:
        dw text_0, text_1, text_2, text_3
text_0: db "R8: HORIZONTAL, MASK + HLOCK",0
text_1: db "R9: VERTICAL, MASK + HLOCK",0
text_2: db "R8 AND R9 TOGETHER",0
text_3: db "PLUS VLOCK: COLUMNS 24-31 STILL",0

palette:
        db 0x00, 0x0F, 0x0E, 0x04       ; 0: the locked caption
        db 0x00, 0x07, 0x06, 0x04       ; 1: the playfield grid
        db 0x00, 0x0A, 0x09, 0x06       ; 2: the side panel
        db 0x00, 0x03, 0x02, 0x0C       ; 3: sprites

; Pattern 60h: a grid intersection. Pattern 61h: a solid block.
mark_pattern:
        db 0x00, 0x00
        db 0x00, 0x00
        db 0x18, 0x00
        db 0x3C, 0x18
        db 0x3C, 0x18
        db 0x18, 0x00
        db 0x00, 0x00
        db 0x00, 0x00
        db 0xFF, 0x00
        db 0xFF, 0x00
        db 0xFF, 0x00
        db 0xFF, 0x00
        db 0xFF, 0x00
        db 0xFF, 0x00
        db 0xFF, 0x00
        db 0xFF, 0x00

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/sega_font_6x8.inc"
        include "include/extended_sprite_overlay.inc"
        ROM_END
