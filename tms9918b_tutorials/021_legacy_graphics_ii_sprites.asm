; Lesson 021: the same Graphics II frame with the standard sprite overlay.
; The overlay label is drawn on character row 7, which belongs to the first
; pattern bank: the font is replicated in all three banks by the shared body.

LESSON_WITH_SPRITES: equ 1
SPRITE_OVERLAY_LABEL: equ 1     ; the overlay prints the sprite size on a text row
        include "include/lesson_legacy_graphics_ii.inc"
