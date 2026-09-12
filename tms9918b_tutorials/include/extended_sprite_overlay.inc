; Animated sprite overlay for the extended graphics modes.
;
; A lesson that includes this file defines, before the include:
;
;   EXT_SAT              address of the sprite attribute table (R5)
;   EXT_SPRITE_PATTERNS  address of the sprite pattern table (R6)
;
; Nine sprites share the same scanlines. Eight are displayed, the ninth one
; disappears and its number is reported through 5S in S0: this is the visible
; difference from the four-sprite limit of a locked device, and the reason the
; extended lessons keep the same overlay in every mode.
;
; The colour bytes use the extended format: palette in bits 4..5 and entry in
; bits 6..7, so the nine sprites take their colours from the palette loaded by
; the lesson. Entry 0 would make a sprite invisible, so it is never used here.
;
; The group moves vertically between raw Y 79 and 143 while the rightmost
; sprite stays at raw Y 111: the ninth sprite therefore vanishes only while the
; group crosses its lines, and returns as the group leaves them.

EXT_SPRITE_Y:       equ 0xC150
EXT_SPRITE_DIR:     equ 0xC151
EXT_SPRITE_STATUS:  equ 0xC152      ; last S0 value, for a debugger

extended_sprite_overlay_load:
        ld hl,extended_sprite_patterns
        ld de,EXT_SPRITE_PATTERNS
        ld bc,extended_sprite_patterns_end-extended_sprite_patterns
        call vdp_copy
        ld hl,extended_sprite_sat
        ld de,EXT_SAT
        ld bc,extended_sprite_sat_end-extended_sprite_sat
        call vdp_copy
        ld a,79
        ld (EXT_SPRITE_Y),a
        ld a,1
        ld (EXT_SPRITE_DIR),a
        xor a
        ld (EXT_SPRITE_STATUS),a
        ret

; Called once per displayed frame, after wait_vblank.
extended_sprite_overlay_animate:
        ; S0 of the frame that just ended, as read by wait_vblank. Expect C8h
        ; while the group crosses the ninth sprite: F, 5S and sprite number 8.
        ld a,(VDP_STATUS)
        ld (EXT_SPRITE_STATUS),a

        ld a,(EXT_SPRITE_DIR)
        bit 7,a
        jr nz,.up
.down:
        ld a,(EXT_SPRITE_Y)
        inc a
        cp 144
        jr nz,.store
        ld a,143
        ld hl,EXT_SPRITE_DIR
        ld (hl),0xFF
        jr .store
.up:
        ld a,(EXT_SPRITE_Y)
        dec a
        cp 78
        jr nz,.store
        ld a,79
        ld hl,EXT_SPRITE_DIR
        ld (hl),1
.store:
        ld (EXT_SPRITE_Y),a
        ld e,a
        ld hl,EXT_SAT               ; the first eight sprites move together
        ld b,8
.next:
        push bc
        push hl
        call vdp_set_write
        ld a,e
        out (VDP_DATA),a
        pop hl
        ld bc,4
        add hl,bc
        pop bc
        djnz .next
        ret

; Four 8x8 patterns: a disc, a ring, a diamond and a square outline.
extended_sprite_patterns:
        db 0x3C,0x7E,0xFF,0xFF,0xFF,0xFF,0x7E,0x3C
        db 0x3C,0x42,0x81,0x81,0x81,0x81,0x42,0x3C
        db 0x18,0x3C,0x7E,0xFF,0xFF,0x7E,0x3C,0x18
        db 0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF
extended_sprite_patterns_end:

; Nine sprites on the same lines. The colour byte is palette * 4 + entry, so
; the overlay shows every palette and every visible entry at once.
extended_sprite_sat:
        db  79,  8,0,0x01           ; palette 0, entry 1
        db  79, 32,1,0x02           ; palette 0, entry 2
        db  79, 56,2,0x03           ; palette 0, entry 3
        db  79, 80,0,0x05           ; palette 1, entry 1
        db  79,104,1,0x06
        db  79,128,2,0x07
        db  79,152,0,0x09           ; palette 2, entry 1
        db  79,176,3,0x0A
        db 111,224,3,0x0F           ; palette 3, entry 3: the ninth sprite
        db 0xD0
extended_sprite_sat_end:
