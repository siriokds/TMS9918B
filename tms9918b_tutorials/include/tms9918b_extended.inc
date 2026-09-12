; Shared extension support for the TMS9918B lessons.
;
; Every routine here assumes the conservative timing of tms9918b_runtime.inc:
; consecutive VDP accesses stay well above the worst-case access window of the
; mode in use, so the same code is safe in locked and unlocked devices alike.
;
; Register bits use the Data Manual names. Bit 0 is the MSB, so the weight of
; a bit is 0x80 >> bit_number.

; --- R11 (mode) ---------------------------------------------------------------
R11_IE1:         equ 0x80        ; D0: scanline interrupt enable
R11_MX:          equ 0x02        ; D6: extended display modes (requires XE)
R11_XE:          equ 0x01        ; D7: extended enable

; --- R12 (screen) ------------------------------------------------------------
R12_HLOCK:      equ 0x08        ; D4: lines 0..15 are not scrolled horizontally
R12_VLOCK:      equ 0x04        ; D5: columns 24..31 are not scrolled vertically
R12_MASK:       equ 0x02        ; D6: blank the leftmost eight pixels
R12_T64:        equ 0x01        ; D7: 64-column text

; --- name-table attribute byte (Graphics1X, Graphics2Fat) --------------------
ATTR_NAME8:     equ 0x08        ; D4: ninth pattern-name bit
ATTR_PRIOR:     equ 0x04        ; D5: tile in front of every sprite
ATTR_PALETTE:   equ 0x03        ; D6..D7: palette 0..3

; --- sprite attribute byte 3 in extended modes -------------------------------
SPR_EC:         equ 0x80        ; D0: early clock (64 pixels in a 512-pixel mode)
SPR_XFINE:      equ 0x40        ; D1: low bit of X in 512-pixel modes; 0 elsewhere
SPR_BANK:       equ 0x20        ; D2: second sprite pattern table
SPR_PAIR:       equ 0x10        ; D3: odd sprite becomes the second bit plane
SPR_PALETTE:    equ 0x0C        ; D4..D5: palette 0..3
SPR_ENTRY:      equ 0x03        ; D6..D7: entry 1..3; 0 = invisible

; --- palette entry -----------------------------------------------------------
LUM_FULL:       equ 0x00
LUM_3_4:        equ 0x10
LUM_1_2:        equ 0x20
LUM_1_4:        equ 0x30
PALETTE_WRITE:  equ 0xC0        ; control code 11 with XE = 1

; Unlock the extended registers, enable XE and verify the S1 identification.
; While the device is locked the probe touches two legacy registers only:
; register 59 and register 11 both land in R3, register 15 lands in R7.
; Returns with XE set on a TMS9918B. On any other device every aliased legacy
; register is restored, a red screen is displayed and the program stops: a
; lesson that needs the extension never runs on a device that lacks it.
tms9918b_require:
        ld hl,0x3B5A            ; register 59, key value
        call vdp_set_register
        call vdp_set_register
        ld hl,0x0B00+R11_XE     ; register 11; aliases R3, like the key writes
        call vdp_set_register
        ld hl,0x0F01            ; select S1; aliases R7 on a locked device
        call vdp_set_register
        in a,(VDP_CTRL)         ; first read is the ordinary status read
        in a,(VDP_CTRL)         ; second read carries the identification
        and 0x3E
        ld c,a
        ld hl,0x0F00            ; back to S0 before anything else
        call vdp_set_register
        ld hl,0x0300            ; restore R3: the key writes and R11 alias it
        call vdp_set_register
        ld a,c
        cp 0x18
        ret z
        ld hl,0x0706            ; restore R7, aliased by R15, with a red backdrop
        call vdp_set_register
        ld hl,0x01C0            ; display on, Graphics I, empty tables
        call vdp_set_register
        jp freeze

; HL = list of (register, value) pairs terminated by 0xFF. Written for the
; extended registers, which are outside the eight-entry legacy table.
vdp_set_register_list:
.next:
        ld a,(hl)
        cp 0xFF
        ret z
        ld d,a
        inc hl
        ld e,(hl)
        inc hl
        push hl
        ex de,hl
        call vdp_set_register
        pop hl
        jr .next

; HL = 16 palette bytes, entry 0 first. The palette port does not wait for a
; memory access window, but the lessons keep the same conservative spacing.
vdp_load_palette:
        xor a
        out (VDP_CTRL),a        ; first control byte: entry number
        rept 8
        nop
        endr
        ld a,PALETTE_WRITE
        out (VDP_CTRL),a
        push af
        pop af
        ld b,16
.entry:
        ld a,(hl)
        out (VDP_DATA),a
        inc hl
        push af
        pop af
        djnz .entry
        ret

; DE = destination of a two-bit pattern table. Expands the shared 1-bit font:
; stratum 0 receives the glyph and stratum 1 stays clear, so ink pixels take
; value 1 and paper pixels value 0 (the backdrop).
vdp_load_font_2bpp:
        ex de,hl
        call vdp_set_write
        ld hl,sega_font_6x8
        ld bc,sega_font_6x8_end-sega_font_6x8
.byte:
        ld a,(hl)
        out (VDP_DATA),a        ; stratum 0
        push af
        pop af
        xor a
        out (VDP_DATA),a        ; stratum 1
        inc hl
        dec bc
        ld a,b
        or c
        jr nz,.byte
        ret

; HL = zero-terminated ASCII, DE = address of the first two-byte entry,
; C = second byte of every entry. Serves Graphics1X and Graphics2Fat
; (name + attribute) and Text40X (character + colour).
vdp_print_ascii_pairs:
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
        push af
        pop af
        ld a,c
        out (VDP_DATA),a
        inc hl
        push af
        pop af
        jr .character

; HL = VRAM address, BC = number of entries, D = first byte, E = second byte.
vdp_fill_pairs:
        call vdp_set_write
.entry:
        ld a,d
        out (VDP_DATA),a
        push af
        pop af
        ld a,e
        out (VDP_DATA),a
        dec bc
        ld a,b
        or c
        jr nz,.entry
        ret
