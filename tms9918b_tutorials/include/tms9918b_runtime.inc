; Shared, deliberately conservative TMS9918-compatible I/O routines.
; Transfers leave enough time between VDP accesses to work on real hardware.

; H = register number, L = value.
vdp_set_register:
        ld a,l
        out (VDP_CTRL),a
        ; Eight NOPs are 32 T-states (8.9 us at the SC-3000 clock). This is
        ; deliberately above the original TMS control-port recovery time and
        ; remains safe when the access begins at an unfavourable VRAM phase.
        rept 8
        nop
        endr
        ld a,h
        or 0x80
        out (VDP_CTRL),a
        push af
        pop af
        ret

; HL = table containing R0..R7.
vdp_set_registers:
        ld b,8
        ld c,0x80
.next:
        ld a,(hl)
        out (VDP_CTRL),a
        rept 8
        nop
        endr
        ld a,c
        out (VDP_CTRL),a
        push de
        pop de
        inc hl
        inc c
        djnz .next
        ret

; HL = VRAM write address.
vdp_set_write:
        ld a,l
        out (VDP_CTRL),a
        rept 8
        nop
        endr
        ld a,h
        and 0x3F
        or 0x40
        out (VDP_CTRL),a
        push af
        pop af
        ret

; HL = ROM source, DE = VRAM destination, BC = byte count.
vdp_copy:
        push hl
        ex de,hl
        call vdp_set_write
        pop hl
.byte:
        ld a,(hl)
        out (VDP_DATA),a
        inc hl
        dec bc
        ld a,b
        or c
        jr nz,.byte
        ret

; HL = VRAM destination, BC = byte count, E = value.
vdp_fill:
        call vdp_set_write
.byte:
        ld a,e
        out (VDP_DATA),a
        dec bc
        ld a,b
        or c
        jr nz,.byte
        ret

vdp_clear:
        ld hl,0
        ld bc,0x4000
        ld e,0
        jp vdp_fill

; DE = pattern-table destination. Loads ASCII 20h..5Ah as patterns 0..58.
vdp_load_sega_font:
        ld hl,sega_font_6x8
        ld bc,sega_font_6x8_end-sega_font_6x8
        jp vdp_copy

; HL = zero-terminated ASCII, DE = one-byte-per-cell name-table address.
vdp_print_ascii:
        push hl
        ex de,hl
        call vdp_set_write
        pop hl
.character:
        ld a,(hl)
        or a
        ret z
        sub 32
        out (VDP_DATA),a
        inc hl
        jr .character

; Silence the four SN76489 channels. Video lessons must not emit the chip's
; power-on tone when Audio/No Reset On Power Up is being tested separately.
mute_psg:
        ld hl,.commands
        ld b,4
.next:
        ld a,(hl)
        out (PSG_PORT),a
        inc hl
        djnz .next
        ret
.commands:
        db 0x9F,0xBF,0xDF,0xFF

; Reading the status register clears F, 5S and C, so the byte is kept: a
; lesson that wants the sprite flags of the frame must use VDP_STATUS instead
; of reading the port again.
VDP_STATUS:     equ 0xC10F

wait_vblank:
        in a,(VDP_CTRL)
        ld (VDP_STATUS),a
        and 0x80
        jr z,wait_vblank
        ret

freeze:
        call wait_vblank
        jr freeze
