/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85

// 2nd ASM version: 290 CPU cycles
void SMS_addTwoAdjoiningSprites (unsigned char x, unsigned char y, unsigned char tile) __naked __preserves_regs(iyh,iyl) {
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES-1
    ret nc                           ; we do not have 2 sprites left, leave!
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

    ld  a,c
    add a,#2
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret
    
_secondSpriteClipped:
    ld  hl,#_SpriteNextFree          ; second sprite has been clipped, so just inc SpriteNextFree
    inc (hl)
    ret
 __endasm;
}
#pragma restore
