; Lesson 401: the same Text40X screen with the hardware cursor.
; Sprites 0 and 1 are the only sprites displayed in an extended text mode. At
; forty columns the sprite X coordinate is in pixels, so column c of a
; six-pixel cell is at X = 6 * c. The cursor blinks by alternating the entry
; bits of its colour byte between 1 and 0, without touching VRAM.

LESSON_WITH_CURSOR: equ 1
        include "include/lesson_text40x.inc"
