; TMS9918B executable tutorial — lesson 000: initialization and detection.
;
; This is the first program software should run before using any B feature.
; It starts through the original TMS9918A register interface, performs the
; documented unlock and S1 identification sequence, restores every aliased
; legacy register, and returns to Graphics I to report the result.
;
; Expected on TMS9918B: green screen with PASS and S1 ID 18H.
; Expected on TMS9918A: solid red screen. The failed probe must remain safe.

        include "include/tms9918b_hardware.inc"

        ROM_START start

start:
        call mute_psg

        ; Begin with the display blanked through the legacy register file.
        ld hl,registers_blank
        call vdp_set_registers
        call vdp_clear

        ; The two key writes alias R7 while locked. Restore R7 immediately,
        ; then enable the extension through R11. On a legacy chip R11 aliases R0.
        ld hl,0x3F5A
        call vdp_set_register
        call vdp_set_register
        ld hl,0x0701
        call vdp_set_register
        ld hl,0x0B01
        call vdp_set_register

        ; Select S1 through R15. The first status read is the normal delayed
        ; read; the second one is the normative identification value.
        ld hl,0x0F01
        call vdp_set_register
        in a,(VDP_CTRL)
        in a,(VDP_CTRL)
        and 0x3E
        ld c,a

        ; Restore the status selector and every legacy alias touched by the
        ; safe probe before deciding whether the device answered correctly.
        ld hl,0x0F00
        call vdp_set_register
        ld hl,0x0380            ; restore R3, aliased by the R11 write
        call vdp_set_register
        ld hl,0x0701
        call vdp_set_register

        ld a,c
        cp 0x18
        jr nz,detect_failed

        ; Disable XE again. The PASS screen therefore also proves that the B
        ; device can return cleanly to its TMS9918A-compatible renderer.
        ld hl,0x0B00
        call vdp_set_register
        call draw_pass_screen
        jp freeze

detect_failed:
        ; Register 11 aliases R3 on an original chip, so restore R3 and use
        ; only legacy registers to produce an unmistakable failure result.
        ld hl,0x0380
        call vdp_set_register
        ld hl,0x0706          ; red backdrop
        call vdp_set_register
        ld hl,0x01C0          ; display on, Graphics I
        call vdp_set_register
        jp freeze

draw_pass_screen:
        ld de,PATTERN_TABLE
        call vdp_load_sega_font

        ld hl,COLOUR_TABLE
        ld bc,32
        ld e,0xF1             ; white ink on black for every pattern group
        call vdp_fill

        ld hl,NAME_TABLE
        ld bc,32*24
        ld e,0
        call vdp_fill

        ld de,NAME_TABLE + 4*32 + 6
        ld hl,title
        call vdp_print_ascii
        ld de,NAME_TABLE + 8*32 + 10
        ld hl,id_text
        call vdp_print_ascii
        ld de,NAME_TABLE + 12*32 + 13
        ld hl,pass_text
        call vdp_print_ascii
        ld de,NAME_TABLE + 16*32 + 6
        ld hl,display_text
        call vdp_print_ascii

        ld hl,SPRITE_TABLE
        ld bc,1
        ld e,0xD0             ; terminate the legacy SAT
        call vdp_fill

        ld hl,registers_visible
        jp vdp_set_registers

;                    R0    R1    R2    R3    R4    R5    R6    R7
registers_blank:   db 0x00, 0x80, 0x0E, 0x80, 0x00, 0x76, 0x03, 0x01
registers_visible: db 0x00, 0xC0, 0x0E, 0x80, 0x00, 0x76, 0x03, 0xF1

title:      db "000 INIT AND DETECT",0
id_text:    db "S1 ID 18H",0
pass_text:  db "PASS",0
display_text: db "DISPLAY: GRAPHICS I",0

        include "include/tms9918b_runtime.inc"
        include "include/sega_font_6x8.inc"
        ROM_END
