; Lesson 301: the same bitmap with the extended sprite overlay.
; Sprites are composed over a bitmap exactly as over a tile playfield, and the
; palette of a sprite is independent from the Palette Map of the area it
; crosses: the nine sprites keep their colours over all four zones.

LESSON_WITH_SPRITES: equ 1
        include "include/lesson_bitmap.inc"
