; Shared hardware definitions for the executable TMS9918B tutorial.
; The legacy lessons intentionally use only the original TMS9918A interface.

VDP_DATA:       equ 0xBE
VDP_CTRL:       equ 0xBF
PSG_PORT:       equ 0x7F
STACK_TOP:      equ 0xC3F0

PATTERN_TABLE:  equ 0x0000
COLOUR_TABLE:   equ 0x2000
NAME_TABLE:     equ 0x3800
SPRITE_TABLE:   equ 0x3B00
SPRITE_PATTERN: equ 0x1800

; Lessons that use include/legacy_sprite_overlay.inc must define
; SPRITE_OVERLAY_LABEL: 1 where a character layer can print the sprite size,
; 0 in Multicolor and in the extended modes without a glyph layer.

    MACRO PAD_TO address
        ASSERT $ <= address
        IF $ < address
            BLOCK address-$, 0xFF
        ENDIF
    ENDM

    MACRO ROM_START entry
        org 0x0000
        di
        ld sp,STACK_TOP
        jp entry
        PAD_TO 0x0038
        reti
        PAD_TO 0x0066
        retn
        PAD_TO 0x0080
    ENDM

; As ROM_START, with an interrupt mode 1 handler at 0038h.
    MACRO ROM_START_IRQ entry, handler
        org 0x0000
        di
        ld sp,STACK_TOP
        jp entry
        PAD_TO 0x0038
        jp handler
        PAD_TO 0x0066
        retn
        PAD_TO 0x0080
    ENDM

    MACRO ROM_END
        ASSERT $ <= 0x8000
        BLOCK 0x8000-$,0xFF
    ENDM

