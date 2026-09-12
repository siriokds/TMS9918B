; Lesson 311: BitmapQ with the extended sprite overlay.
; The banking experiment continues underneath: the nine sprites are unaffected
; by R4, because sprite patterns are located by R6 and never banked.

LESSON_WITH_SPRITES: equ 1
        include "include/lesson_bitmapq.inc"
