; Animated sprite overlay shared by the paired legacy-mode lessons.
; Four sprites move vertically as one group while a fifth remains fixed. When
; all five overlap the same scanlines, the legacy line limit hides sprite 4;
; it progressively reappears as the moving group leaves those lines. R1 also
; cycles through 8x8, 16x16, magnified 8x8 and magnified 16x16.

SPRITE_ANIMATION_Y:     equ 0xC110
SPRITE_ANIMATION_DIR:   equ 0xC111
SPRITE_ANIMATION_TIMER: equ 0xC112
SPRITE_ANIMATION_SIZE:  equ 0xC113

legacy_sprite_overlay_load:
        ld hl,legacy_sprite_patterns
        ld de,SPRITE_PATTERN
        ld bc,legacy_sprite_patterns_end-legacy_sprite_patterns
        call vdp_copy
        ld hl,legacy_sprite_sat
        ld de,SPRITE_TABLE
        ld bc,legacy_sprite_sat_end-legacy_sprite_sat
        call vdp_copy
        ld a,79
        ld (SPRITE_ANIMATION_Y),a
        ld a,1
        ld (SPRITE_ANIMATION_DIR),a
        ld a,120
        ld (SPRITE_ANIMATION_TIMER),a
        xor a
        ld (SPRITE_ANIMATION_SIZE),a
        ret

; Called once per displayed frame. The first four raw Y coordinates bounce
; between 79 and 143, below the explanatory text. The rightmost fifth sprite
; remains at raw Y=111.
legacy_sprite_overlay_animate:
        ld a,(SPRITE_ANIMATION_DIR)
        bit 7,a
        jr nz,.move_left
.move_right:
        ld a,(SPRITE_ANIMATION_Y)
        inc a
        cp 144
        jr nz,.store_x
        ld a,143
        ld hl,SPRITE_ANIMATION_DIR
        ld (hl),0xFF
        jr .store_x
.move_left:
        ld a,(SPRITE_ANIMATION_Y)
        dec a
        cp 78
        jr nz,.store_x
        ld a,79
        ld hl,SPRITE_ANIMATION_DIR
        ld (hl),1
.store_x:
        ld (SPRITE_ANIMATION_Y),a
        ld e,a
        ld hl,SPRITE_TABLE
        call .write_x
        ld a,(SPRITE_ANIMATION_Y)
        ld e,a
        ld hl,SPRITE_TABLE+4
        call .write_x
        ld a,(SPRITE_ANIMATION_Y)
        ld e,a
        ld hl,SPRITE_TABLE+8
        call .write_x
        ld a,(SPRITE_ANIMATION_Y)
        ld e,a
        ld hl,SPRITE_TABLE+12
        call .write_x

        ld hl,SPRITE_ANIMATION_TIMER
        dec (hl)
        ret nz
        ld (hl),120
        ld a,(SPRITE_ANIMATION_SIZE)
        inc a
        and 3
        ld (SPRITE_ANIMATION_SIZE),a
        IF SPRITE_OVERLAY_LABEL
        call .update_label
        ENDIF
        ld a,(SPRITE_ANIMATION_SIZE)
        push af
        and 1
        ld e,0
        jr nz,.pattern_selected
        ld e,4
.pattern_selected:
        call .write_patterns
        pop af
        ld e,a
        ld d,0
        ld hl,.r1_values
        add hl,de
        ld l,(hl)
        ld h,1
        jp vdp_set_register

.write_x:
        call vdp_set_write
        ld a,e
        out (VDP_DATA),a
        ret

.write_patterns:
        ld hl,SPRITE_TABLE+2
        call .write_x
        ld hl,SPRITE_TABLE+6
        call .write_x
        ld hl,SPRITE_TABLE+10
        call .write_x
        ld hl,SPRITE_TABLE+14
        call .write_x
        ld hl,SPRITE_TABLE+18
        jp .write_x

        IF SPRITE_OVERLAY_LABEL
.update_label:
        ld hl,NAME_TABLE + 7*32
        ld bc,32
        ld e,0
        call vdp_fill
        ld a,(SPRITE_ANIMATION_SIZE)
        or a
        ld hl,.label_8
        jr z,.print_label
        dec a
        ld hl,.label_16
        jr z,.print_label
        dec a
        ld hl,.label_8_double
        jr z,.print_label
        ld hl,.label_16_double
.print_label:
        ld de,NAME_TABLE + 7*32 + 2
        jp vdp_print_ascii

        ENDIF

; Display on, 16 KiB. Bits 1..0 select size and magnification.
.r1_values:
        db 0xC0,0xC2,0xC1,0xC3
.label_8:         db "SPRITES: 8 X 8 NORMAL",0
.label_16:        db "SPRITES: 16 X 16 NORMAL",0
.label_8_double:  db "SPRITES: 8 X 8 DOUBLED",0
.label_16_double: db "SPRITES: 16 X 16 DOUBLED",0

legacy_sprite_patterns:
        ; Patterns 0..3 form a 16x16 circle in TMS quadrant order:
        ; top-left, bottom-left, top-right, bottom-right.
        db 0x07,0x1F,0x3F,0x7F,0x7F,0xFF,0xFF,0xFF
        db 0xFF,0xFF,0xFF,0x7F,0x7F,0x3F,0x1F,0x07
        db 0xE0,0xF8,0xFC,0xFE,0xFE,0xFF,0xFF,0xFF
        db 0xFF,0xFF,0xFF,0xFE,0xFE,0xFC,0xF8,0xE0
        ; Pattern 4 is the independent circle selected in both 8x8 formats.
        db 0x3C,0x7E,0xFF,0xFF,0xFF,0xFF,0x7E,0x3C
legacy_sprite_patterns_end:

legacy_sprite_sat:
        ; The first four entries share an animated Y and therefore consume
        ; the complete legacy line budget. Entry 4 stays in the middle of the
        ; screen and is the visible fifth-sprite-limit probe.
        db  79, 16,4,15
        db  79, 64,4, 6
        db  79,112,4,10
        db  79,160,4, 4
        db 111,208,4, 7
        db 0xD0
legacy_sprite_sat_end:
