/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85

void SMS_addThreeAdjoiningSprites (unsigned char x, unsigned char y, unsigned char tile) __naked __preserves_regs(iyh,iyl) {
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES-2
    ret nc                           ; we do not have 3 sprites left, leave!
    ld  c,a                          ; save SpriteNextFree value in c
    ld  b,#0x00

    ld  hl,#2
    add hl,sp
    ld  e,(hl)                       ; read X
    inc hl
    ld  a,(hl)                       ; read Y
    cp  a,#0xd1
    ret z                            ; invalid Y, leave!
    inc hl
    ld  d,(hl)                       ; read tile number

    ld  hl,#_SpriteTableY
    add hl,bc
    dec a
    ld (hl),a                        ; write Y  (as Y-1)
    inc hl
    ld (hl),a                        ; write Y again for the second sprite (always as Y-1)
    inc hl
    ld (hl),a                        ; write Y once more for the third sprite (always as Y-1)

    ld hl,#_SpriteTableXN
    add hl,bc
    add hl,bc
    ld (hl),e                        ; write X
    inc hl
    ld (hl),d                        ; write tile number

    ld a,(#_spritesWidth)            ; load current sprite width
    add a,e
    jr c,_secondSpriteClipped        ; if new X is overflowing, do not place second sprite
    inc hl
    ld (hl),a                        ; write X + spritesWidth

    ld a,(#_spritesTileOffset)       ; load current sprite tile offset
    add a,d
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset

    ld a,(#_spritesWidth)            ; load current sprite width
    add a,a                          ; double that
    add a,e
    jr c,_thirdSpriteClipped         ; if new X is overflowing, do not place third sprite
    inc hl
    ld (hl),a                        ; write X + spritesWidth*2

    ld a,(#_spritesTileOffset)       ; load current sprite tile offset
    add a,a                          ; double that
    add a,d
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset*2

    ld  a,c                          ; three sprite has been placed, then SpriteNextFree+=3
    add a,#3
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret

_secondSpriteClipped:
    ld  a,c                          ; second sprite has been clipped, so just inc SpriteNextFree
    inc a
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret

_thirdSpriteClipped:
    ld  a,c                          ; third sprite has been clipped, so just SpriteNextFree+=2
    add a,#2
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret
 __endasm;
}
#pragma restore

