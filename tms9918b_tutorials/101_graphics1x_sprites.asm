; Lesson 101: the same Graphics1X frame with the extended sprite overlay.
; Nine sprites share the same scanlines: eight are displayed and the ninth is
; dropped, which is the visible difference from the four-sprite limit of a
; locked device. The colour bytes use the extended format, so the sprites take
; their colours from the four palettes loaded by the shared body.

LESSON_WITH_SPRITES: equ 1
        include "include/lesson_graphics1x.inc"
