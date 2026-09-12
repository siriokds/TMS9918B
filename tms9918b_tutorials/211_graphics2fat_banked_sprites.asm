; Lesson 211: Graphics2Fat with third banking with the extended sprite overlay.
; The banking experiment continues underneath: the nine sprites are unaffected
; by R4, because sprite patterns are located by R6 and never banked.

LESSON_WITH_SPRITES: equ 1
        include "include/lesson_graphics2fat_banked.inc"
