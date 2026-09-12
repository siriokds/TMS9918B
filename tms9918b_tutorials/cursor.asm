; Positioning a Text64 sprite at pixel resolution.
;
; The X coordinate is nine bits in a 512-pixel mode: bits 0-7 in SAT byte 1 and
; bit 8 in bit 1 of the colour byte, SAT byte 3. The two bytes are two entries
; apart, so one address setup covers both.
;
; Entry: HL = X (0..511), A = colour byte with bit 40h clear, DE = SAT address.

        include "include/tms9918b_hardware.inc"
        ROM_START start
start:

; ---------------------------------------------------------------------------
; Version A: X and colour written with two address setups.
; ---------------------------------------------------------------------------
sprite_x_two_writes:
        ld b,a                  ; 4   keep the colour byte
        ld a,h                  ; 4   bit 8 of X is bit 0 of H
        rrca                    ; 4   move it to bit 6
        rrca                    ; 4
        and 0x40                ; 7
        or b                    ; 4   merge with the colour
        ld c,a                  ; 4   C = final colour byte
        push de                 ; 11
        inc de                  ; 6   SAT + 1 is the X byte
        ex de,hl                ; 4
        call vdp_set_write      ; 17 + routine
        ex de,hl                ; 4
        ld a,l                  ; 4
        out (VDP_DATA),a        ; 11  X bits 0-7
        pop de                  ; 10
        push hl                 ; 11
        ld hl,3                 ; 10
        add hl,de               ; 11  SAT + 3 is the colour byte
        call vdp_set_write      ; 17 + routine
        pop hl                  ; 10
        ld a,c                  ; 4
        out (VDP_DATA),a        ; 11  colour byte with bit 8 of X
        ret                     ; 10

; ---------------------------------------------------------------------------
; Version B: one address setup, four bytes written in sequence. Y and the
; pattern name have to be supplied as well, which a moving object changes
; anyway. This is the version to use, because the four bytes of the entry
; reach the VDP back to back.
;
; Entry: HL = X (0..511), B = Y, C = pattern name, A = colour byte,
;        DE = SAT address of the sprite.
; ---------------------------------------------------------------------------
sprite_move:
        ld (colour_shadow),a    ; 13
        ex de,hl                ; 4    HL = SAT address, DE = X
        call vdp_set_write      ; 17 + routine
        ld a,b                  ; 4
        out (VDP_DATA),a        ; 11   Y
        ld a,e                  ; 4
        out (VDP_DATA),a        ; 11   X bits 0-7
        ld a,c                  ; 4
        out (VDP_DATA),a        ; 11   pattern name
        ld a,d                  ; 4    bit 8 of X is bit 0 of D
        rrca                    ; 4
        rrca                    ; 4
        and 0x40                ; 7
        ld hl,colour_shadow     ; 10
        or (hl)                 ; 7
        out (VDP_DATA),a        ; 11   colour byte with bit 8 of X
        ret                     ; 10

colour_shadow:  equ 0xC180

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
