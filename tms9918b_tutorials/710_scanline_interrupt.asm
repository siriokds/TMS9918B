; Lesson 710: the scanline interrupt.
;
; R10 holds the number of displayed lines between two interrupts, minus one.
; The counter is loaded at the first line of the picture and whenever R10 is
; written, and FL is set when it underflows. IE1 in R11 enables the INT pin, and
; FL is cleared by reading S1, which R15 selects.
;
; The handler must tell the two interrupts apart: it selects S1, reads it,
; selects S0 and reads that too, because the frame interrupt shares the pin.
; Reading S0 also clears the frame flag, so the main program is free.
;
; With R10 = 47 the interrupt fires at the end of lines 47, 95, 143 and 191.
; The handler uses them to build a four-band screen out of one playfield:
;
;   band 0   R8 = 0, palette entry 1 white      (the counter starts here)
;   band 1   R8 moves, entry 1 light yellow
;   band 2   R8 moves twice as fast, entry 1 light red
;   band 3   R8 = 0 again, entry 1 light green
;
; The same tile rows therefore appear four times in four colours and at three
; scrolling speeds, which is what a scanline interrupt is for: parallax bands,
; a fixed status bar and mid-screen palette changes. The number of scanline
; interrupts counted in the last frame is published at C170h; it must be four.

        include "include/tms9918b_hardware.inc"
        ROM_START_IRQ start, interrupt

G1X_PATTERNS:   equ 0x0000
G1X_NAMES:      equ 0x3800
G1X_SAT:        equ 0x3E00

BAND:           equ 0xC168      ; band the handler is about to paint
SCROLL:         equ 0xC169
IRQ_COUNT:      equ 0xC16A
IRQ_LAST:       equ 0xC170      ; interrupts counted in the previous frame

MARK:           equ 0x60

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
        ld bc,16
        call vdp_copy
        ld hl,palette
        call vdp_load_palette

        ld hl,G1X_NAMES
        ld bc,32*24
        ld d,MARK
        ld e,0x01               ; palette 1 everywhere
        call vdp_fill_pairs

        CELL_AT 1, 2
        ld hl,title
        ld c,0x01
        call vdp_print_ascii_pairs
        CELL_AT 3, 2
        ld hl,line_one
        ld c,0x01
        call vdp_print_ascii_pairs
        CELL_AT 7, 2
        ld hl,band_one
        ld c,0x01
        call vdp_print_ascii_pairs
        CELL_AT 13, 2
        ld hl,band_two
        ld c,0x01
        call vdp_print_ascii_pairs
        CELL_AT 19, 2
        ld hl,band_three
        ld c,0x01
        call vdp_print_ascii_pairs

        ld hl,G1X_SAT
        ld bc,1
        ld e,0xD0
        call vdp_fill

        xor a
        ld (BAND),a
        ld (SCROLL),a
        ld (IRQ_COUNT),a
        ld (IRQ_LAST),a

        ld hl,extended_registers
        call vdp_set_register_list
        ld hl,registers_visible
        call vdp_set_registers

        im 1
        ei
.idle:
        jr .idle

; ---------------------------------------------------------------------------
; Interrupt handler, reached through the jump at 0038h placed by ROM_START_IRQ.
interrupt:
        push af
        push bc
        push de
        push hl

        ld hl,0x0F01            ; select S1
        call vdp_set_register
        in a,(VDP_CTRL)
        ld c,a                  ; S1: bit 0 is FL
        ld hl,0x0F00            ; back to S0 at once
        call vdp_set_register
        in a,(VDP_CTRL)
        ld b,a                  ; S0: bit 0 is the frame flag

        bit 0,c
        jr z,.frame_only

        ; Scanline interrupt: paint the band that starts on the next line.
        ld hl,IRQ_COUNT
        inc (hl)
        ld a,(BAND)
        inc a
        and 3
        ld (BAND),a
        call paint_band
        jr .done

.frame_only:
        bit 0,b
        jr z,.done
        ; Frame interrupt: the counter of the previous frame becomes the
        ; published value, the bands restart from the first one.
        ld a,(IRQ_COUNT)
        ld (IRQ_LAST),a
        xor a
        ld (IRQ_COUNT),a
        ld (BAND),a
        call paint_band
        ld a,(SCROLL)
        inc a
        ld (SCROLL),a

.done:
        pop hl
        pop de
        pop bc
        pop af
        ei
        reti

; Writes the scroll register and the palette entry of the current band.
paint_band:
        ld a,(BAND)
        ld e,a
        ld d,0
        ld hl,band_colours
        add hl,de
        ld c,(hl)               ; colour of entry 1 in this band

        ld l,0                  ; bands 0 and 3 do not scroll
        ld a,(BAND)
        or a
        jr z,.scroll_ready
        cp 3
        jr z,.scroll_ready
        ld a,(SCROLL)
        ld l,a
        ld a,(BAND)
        cp 2
        jr nz,.scroll_ready
        ld a,l
        add a,a                 ; band 2 moves twice as fast
        ld l,a
.scroll_ready:
        ld h,8
        push bc
        call vdp_set_register
        pop bc

        ; Palette entry 5 is entry 1 of palette 1, the colour of the grid.
        ld a,5
        out (VDP_CTRL),a
        rept 8
        nop
        endr
        ld a,PALETTE_WRITE
        out (VDP_CTRL),a
        push af
        pop af
        ld a,c
        out (VDP_DATA),a
        ret

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x80, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01
registers_visible: db 0x00, 0xE0, 0x0E, 0x00, 0x00, 0x7C, 0x05, 0x01

; R1 bit 2 (20h) enables the frame interrupt; R11 bit 0 (80h) the scanline one.
extended_registers:
        db 11, R11_XE|R11_MX|R11_IE1
        db 8, 0x00
        db 9, 0x00
        db 12, 47               ; one interrupt every 48 displayed lines
        db 13, R12_MASK
        db 0xFF

; Colour of palette entry 5, that is entry 1 of palette 1, in each band.
band_colours:   db 0x0F, 0x0B, 0x09, 0x03

palette:
        db 0x00, 0x0F, 0x0E, 0x04
        db 0x00, 0x0F, 0x0E, 0x04       ; entry 5 is rewritten by the handler
        db 0x00, 0x07, 0x06, 0x04
        db 0x00, 0x03, 0x02, 0x0C

mark_pattern:
        db 0x00, 0x00
        db 0x7E, 0x00
        db 0x42, 0x3C
        db 0x42, 0x24
        db 0x42, 0x24
        db 0x42, 0x3C
        db 0x7E, 0x00
        db 0x00, 0x00

title:      db "710 SCANLINE INTERRUPT",0
line_one:   db "R10 = 47: ONE IRQ EVERY 48 LINES",0
band_one:   db "BAND 1: SCROLLS, YELLOW",0
band_two:   db "BAND 2: TWICE AS FAST, RED",0
band_three: db "BAND 3: STILL, GREEN",0

        include "include/tms9918b_runtime.inc"
        include "include/tms9918b_extended.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
