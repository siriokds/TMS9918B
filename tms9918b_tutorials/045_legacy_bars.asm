; Lesson 045: the undocumented bar modes, mode 5 and mode 7.
;
; M1 together with M2 selects a mode that reads no table at all: the VDP
; produces a fixed pattern of vertical bars from its own colour logic, and the
; content of VRAM, the value of R2 to R6 and the sprite attribute table have no
; effect. Mode 7 adds M3, which changes nothing for the same reason. Sprites
; are not displayed, so this lesson cannot label its own screen.
;
; The program cycles through four states, two seconds each, and records the
; current one at CPU address C100h for a debugger:
;
;   0  mode 5, locked device        2  mode 5 with XE and MX set
;   1  mode 7, locked device        3  mode 7 with XE and MX set
;
; States 2 and 3 are the interesting ones: the extended device keeps the bar
; modes exactly as the TMS9918A produced them, which is why the mode table of
; the Data Manual lists them unchanged.

        include "include/tms9918b_hardware.inc"
        ROM_START start

BAR_STATE:      equ 0xC100
BAR_TIMER:      equ 0xC101
BAR_EXTENDED:   equ 0xC102      ; 0 until the probe has answered

start:
        call mute_psg
        ld hl,registers_blank
        call vdp_set_registers
        call vdp_clear

        ; The probe is the lesson 000 sequence without its failure screen: a
        ; locked device simply keeps the first two states.
        xor a
        ld (BAR_EXTENDED),a
        ld hl,0x3F5A
        call vdp_set_register
        call vdp_set_register
        ld hl,0x0701
        call vdp_set_register
        ld hl,0x0B01
        call vdp_set_register
        ld hl,0x0F01
        call vdp_set_register
        in a,(VDP_CTRL)
        in a,(VDP_CTRL)
        and 0x3E
        ld c,a
        ld hl,0x0F00
        call vdp_set_register
        ld hl,0x0300            ; restore R3, aliased by the R11 write
        call vdp_set_register
        ld hl,0x0701
        call vdp_set_register
        ld hl,0x0B00            ; leave the extension disabled for state 0
        call vdp_set_register
        ld a,c
        cp 0x18
        jr nz,.locked
        ld a,1
        ld (BAR_EXTENDED),a
.locked:
        xor a
        ld (BAR_STATE),a
        ld a,120
        ld (BAR_TIMER),a
        call apply_state

.frame:
        call wait_vblank
        ld hl,BAR_TIMER
        dec (hl)
        jr nz,.frame
        ld (hl),120
        ld a,(BAR_STATE)
        inc a
        and 3
        ld c,a
        ld a,(BAR_EXTENDED)
        or a
        ld a,c
        jr nz,.store
        and 1                   ; a locked device only has states 0 and 1
.store:
        ld (BAR_STATE),a
        call apply_state
        jr .frame

; Applies the current state: R11 first, then the mode bits in R0 and R1.
apply_state:
        ld a,(BAR_STATE)
        ld e,a
        ld d,0
        ld hl,r11_values
        add hl,de
        ld l,(hl)
        ld h,11
        call vdp_set_register
        ld a,(BAR_STATE)
        and 1
        ld l,0x00               ; mode 5: M1 and M2, no M3
        jr z,.mode_bits
        ld l,0x02               ; mode 7: M1, M2 and M3
.mode_bits:
        ld h,0
        call vdp_set_register
        ld hl,0x01D8            ; display on, 16K, M1 and M2
        jp vdp_set_register

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x98, 0x0E, 0x00, 0x00, 0x76, 0x03, 0x01

; R11 per state: locked, locked, extended, extended.
r11_values:      db 0x00, 0x00, 0x03, 0x03

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
