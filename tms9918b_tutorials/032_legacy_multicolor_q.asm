; Lesson 032: Multicolor Q, the undocumented M3 variant (mode 6).
;
; Names and colour blocks behave as in Multicolor, but the pattern generator is
; banked per screen third exactly like Graphics II: with R4 = 03h the three
; 2 KiB banks are at 0000h, 0800h and 1000h. The same name therefore produces
; three different colour bands, which is what distinguishes mode 6 from mode 4.

LESSON_WITH_SPRITES: equ 1
LESSON_THIRDS:       equ 1
SPRITE_OVERLAY_LABEL: equ 0
SPRITE_OVERLAY_R1_BASE: equ 0xC8
        include "include/lesson_legacy_multicolor.inc"
