; Lesson 011: the same Graphics I frame with the standard sprite overlay.
; Comparing 010 and 011 isolates sprite composition from every background
; table, colour and pattern used by the lesson.

LESSON_WITH_SPRITES: equ 1
SPRITE_OVERLAY_LABEL: equ 1     ; the overlay prints the sprite size on a text row
SPRITE_OVERLAY_R1_BASE: equ 0xC0
        include "include/lesson_legacy_graphics_i.inc"
