; Hardware cursor for the extended text modes.
;
; Sprites 0 and 1 are the only sprites displayed in Text40X, Text40XQ, Text64
; and Text64Q. A lesson defines CURSOR_SAT and CURSOR_PATTERNS before the
; include, calls cursor_load once and cursor_blink once per frame.
;
; The cursor position comes from the caller:
;   B = raw Y (one less than the first visible line)
;   C = X: pixels at forty columns, two-pixel steps at sixty-four
;
; At 64 columns the picture has 512 pixels, so X counts two-pixel steps and
; XFINE, bit 1 of the colour byte, supplies the odd pixel. A cursor sits on a
; character boundary and never needs it; a pointer does (Programmer's Guide
; section 8.5).

CURSOR_STATE:   equ 0xC120
CURSOR_TIMER:   equ 0xC121

cursor_load:
        push bc
        ld hl,cursor_pattern
        ld de,CURSOR_PATTERNS
        ld bc,8
        call vdp_copy
        pop bc
        ld hl,CURSOR_SAT
        call vdp_set_write
        ld a,b
        out (VDP_DATA),a            ; Y
        push af
        pop af
        ld a,c
        out (VDP_DATA),a            ; X
        push af
        pop af
        xor a
        out (VDP_DATA),a            ; pattern name 0
        push af
        pop af
        ld a,0x01                   ; palette 0, entry 1
        out (VDP_DATA),a
        push af
        pop af
        ld a,0xD0                   ; terminate after sprite 0
        out (VDP_DATA),a
        ld a,1
        ld (CURSOR_STATE),a
        ld a,30
        ld (CURSOR_TIMER),a
        ret

; Toggles the entry bits every thirty frames: entry 0 is invisible, so the
; cursor blinks without rewriting a pattern.
cursor_blink:
        ld hl,CURSOR_TIMER
        dec (hl)
        ret nz
        ld (hl),30
        ld a,(CURSOR_STATE)
        xor 0x01
        ld (CURSOR_STATE),a
        ld c,a
        ld hl,CURSOR_SAT + 3
        call vdp_set_write
        ld a,c
        out (VDP_DATA),a
        ret

cursor_pattern:
        db 0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0xFE
