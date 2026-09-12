; Lesson 031: the same Multicolor blocks with the animated sprite overlay.
; Sprite composition over a Multicolor playfield behaves exactly as it does
; over Graphics I and Graphics II, including the four-sprite line limit.

LESSON_WITH_SPRITES: equ 1
LESSON_THIRDS:       equ 0
SPRITE_OVERLAY_LABEL: equ 0
SPRITE_OVERLAY_R1_BASE: equ 0xC8
        include "include/lesson_legacy_multicolor.inc"
