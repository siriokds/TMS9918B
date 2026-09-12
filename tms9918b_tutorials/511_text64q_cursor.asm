; Lesson 511: Text64Q with the hardware cursor. At 64 columns the sprite X
; coordinate counts two-pixel steps, so column c is at X = 4 * c.
; See include/lesson_text_banked.inc for the documented shared body.

LESSON_T64: equ 1
LESSON_WITH_CURSOR: equ 1
        include "include/lesson_text_banked.inc"
