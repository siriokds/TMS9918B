; Positioning a Text64 sprite at pixel resolution.
;
; In a 512-pixel mode SAT byte 1 is the coarse coordinate and XFINE is its
; one-pixel refinement: screen X = 2 * SAT X + XFINE. XFINE is bit 1 in TI's
; D0-is-MSB notation, physical mask 40h in SAT byte 3.
;
; Entry: HL = screen X (0..511), A = colour byte with bit 40h clear,
;        DE = SAT address.

        include "include/tms9918b_hardware.inc"
        ROM_START start
start:

; ---------------------------------------------------------------------------
; Version A: X and colour written with two address setups.
; ---------------------------------------------------------------------------
sprite_x_two_writes:
        ld (colour_shadow),a
        ld a,l
        and 0x01
        ld (xfine_shadow),a
        srl h
        rr l                    ; HL = screen X / 2 = coarse X
        ld a,l
        ld (x_shadow),a
        push de
        inc de                  ; SAT + 1 is the X byte
        ex de,hl
        call vdp_set_write
        ld a,(x_shadow)
        out (VDP_DATA),a
        pop de
        ld hl,3
        add hl,de               ; SAT + 3 is the colour byte
        call vdp_set_write
        ld a,(xfine_shadow)
        rrca
        rrca                    ; bit 0 becomes physical mask 40h
        ld hl,colour_shadow
        or (hl)
        out (VDP_DATA),a
        ret

; ---------------------------------------------------------------------------
; Version B: one address setup, four bytes written in sequence. Y and the
; pattern name have to be supplied as well, which a moving object changes
; anyway. This is the version to use, because the four bytes of the entry
; reach the VDP back to back.
;
; Entry: HL = screen X (0..511), B = Y, C = pattern name, A = colour byte,
;        DE = SAT address of the sprite.
; ---------------------------------------------------------------------------
sprite_move:
        ld (colour_shadow),a
        ld a,l
        and 0x01
        ld (xfine_shadow),a
        srl h
        rr l                    ; HL = screen X / 2 = coarse X
        ld a,l
        ld (x_shadow),a
        ex de,hl                ; HL = SAT address
        call vdp_set_write
        ld a,b
        out (VDP_DATA),a        ; Y
        ld a,(x_shadow)
        out (VDP_DATA),a        ; coarse X
        ld a,c
        out (VDP_DATA),a        ; pattern name
        ld a,(xfine_shadow)
        rrca
        rrca                    ; bit 0 becomes physical mask 40h
        ld hl,colour_shadow
        or (hl)
        out (VDP_DATA),a        ; colour byte with XFINE
        ret

colour_shadow: equ 0xC180
x_shadow:      equ 0xC181
xfine_shadow:  equ 0xC182

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
