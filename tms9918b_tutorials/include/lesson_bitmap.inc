; Shared body for lessons 300: the linear Bitmap mode.
;
; Bitmap has no pattern names. The picture is stored line by line: 64 bytes per
; line, two bytes for every eight pixels, stratum 0 first. Pixel values 1..3
; select entries of the palette named by the Palette Map, one byte per 8x8
; area, and value 0 shows the backdrop.
;
; The background is a two-pixel bar sequence 0, 1, 2, 3 repeated across every
; line, so the same three values appear everywhere. The Palette Map divides the
; screen into four vertical zones with four different palettes: the identical
; bit pattern therefore produces four different colour sets. A diagonal drawn
; with value 3 crosses the zones and shows that the surface is a true bitmap
; and not a grid of repeated cells.

        include "include/tms9918b_hardware.inc"
        ROM_START start

EXT_SAT:             equ 0x3B00
EXT_SPRITE_PATTERNS: equ 0x3000

BITMAP:         equ 0x0000      ; R4 = 00h, 192 lines x 64 bytes
BITMAP_SPRITES: equ 0x3000      ; R6 = 06h
PALETTE_MAP:    equ 0x3800      ; R2 = 0Eh, 768 bytes
BITMAP_SAT:     equ 0x3B00      ; R5 = 76h
LINE_BUFFER:    equ 0xC200      ; 64 bytes of work RAM

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call tms9918b_require
        call vdp_clear

        ld hl,palette
        call vdp_load_palette

        ; One Palette Map byte per 8x8 area: eight areas per palette.
        ld hl,PALETTE_MAP
        call vdp_set_write
        ld c,24                 ; area rows
.map_row:
        ld b,0                  ; area column
.map_area:
        ld a,b
        rrca
        rrca
        rrca
        and 0x03                ; palette = column / 8
        out (VDP_DATA),a
        push af
        pop af
        inc b
        ld a,b
        cp 32
        jr nz,.map_area
        dec c
        jr nz,.map_row

        ; The bitmap is built one line at a time in RAM, then transferred.
        ld hl,BITMAP
        call vdp_set_write
        ld c,0                  ; line number
.line:
        call build_line
        ld hl,LINE_BUFFER
        ld b,64
.send:
        ld a,(hl)
        out (VDP_DATA),a
        inc hl
        djnz .send
        inc c
        ld a,c
        cp 192
        jr nz,.line

        ld hl,sega_font_6x8
        ld de,BITMAP_SPRITES
        ld bc,sega_font_6x8_end-sega_font_6x8
        call vdp_copy
        IF LESSON_WITH_SPRITES
        call extended_sprite_overlay_load
        ELSE
        ld hl,caption
        ld de,BITMAP_SAT
        ld bc,caption_end-caption
        call vdp_copy
        ENDIF

        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers
        IF LESSON_WITH_SPRITES
.animation_loop:
        call wait_vblank
        call extended_sprite_overlay_animate
        jr .animation_loop
        ELSE
        jp freeze
        ENDIF

; C = line number. Fills LINE_BUFFER with the bar background and sets one
; diagonal pixel to value 3 at x = line + 32.
build_line:
        push bc
        ld hl,LINE_BUFFER
        ld b,32                 ; 32 groups of eight pixels
.bars:
        ld (hl),0x33            ; stratum 0: 0 0 1 1 0 0 1 1
        inc hl
        ld (hl),0x0F            ; stratum 1: 0 0 0 0 1 1 1 1
        inc hl
        djnz .bars

        ld a,c
        add a,32                ; x coordinate of the diagonal
        ld c,a
        and 0x07                ; pixel inside its group
        ld b,a
        inc b
        ld a,0x80
        jr .mask_entry
.mask:
        rrca
.mask_entry:
        djnz .mask              ; A = 80h >> pixel
        ld e,a

        ld a,c
        and 0xF8
        rrca
        rrca                    ; (x / 8) * 2 = offset of the stratum 0 byte
        ld l,a
        ld h,0
        ld bc,LINE_BUFFER
        add hl,bc
        ld a,(hl)
        or e
        ld (hl),a               ; value 3 needs a bit in both strata
        inc hl
        ld a,(hl)
        or e
        ld (hl),a
        pop bc
        ret

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x88, 0x0E, 0x00, 0x00, 0x76, 0x06, 0x01
registers_visible: db 0x00, 0xC8, 0x0E, 0x00, 0x00, 0x76, 0x06, 0x01

extended_registers:
        db 11, R11_XE|R11_MX
        db 12, 0x00
        db 0xFF

; The same three pixel values in four palettes; entry 0 is never displayed.
palette:
        db 0x00, 0x0F, 0x0E, 0x01                       ; 0: white, grey, black
        db 0x00, 0x0B, LUM_3_4|0x0B, LUM_1_2|0x0B       ; 1: one colour, three luminances
        db 0x00, 0x09, 0x08, 0x06                       ; 2: reds
        db 0x00, 0x03, 0x02, 0x0C                       ; 3: greens

caption:
        db 7, 8,'3'-32,0x01, 7, 16,'0'-32,0x01, 7, 24,'0'-32,0x01
        db 7, 40,'B'-32,0x01, 7, 48,'I'-32,0x01, 7, 56,'T'-32,0x01
        db 7, 64,'M'-32,0x01, 7, 72,'A'-32,0x01, 7, 80,'P'-32,0x01
        db 0xD0
caption_end:

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/sega_font_6x8.inc"
        IF LESSON_WITH_SPRITES
        include "include/extended_sprite_overlay.inc"
        ENDIF
        ROM_END
