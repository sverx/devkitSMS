/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85

#ifdef NO_SPRITE_CHECKS
void SMS_addThreeAdjoiningSprites_f (unsigned char y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1) {
  // Y passed in A
  // X passed in D
  // tile passed in E
  __asm
    ld  hl,#_SpriteNextFree          ; load current SpriteNextFree value
    ld  c,(hl)                       ; (in C)
    inc (hl)                         ; increment SpriteNextFree

    ld  b,#0x00
    ld  hl,#_SpriteTableY
    add hl,bc                        ; hl+=SpriteNextFree
    dec a
    ld (hl),a                        ; write Y (as Y-1)
    inc hl
    ld (hl),a                        ; write Y again for the second sprite (always as Y-1)
    inc hl
    ld (hl),a                        ; write Y again for the third sprite (always as Y-1)

    ld hl,#_SpriteTableXN
    sla c
    add hl,bc                        ; hl+=SpriteNextFree*2
    ld (hl),d                        ; write X
    inc hl
    ld (hl),e                        ; write tile number

#ifdef NO_SPRITE_ZOOM
    ld a,#8
#else
    ld a,(#_spritesWidth)            ; load current sprite width
#endif

    add a,d                          ; add to X
    ret c                            ; if new X is overflowing, do not place second sprite and leave

    inc hl
    ld (hl),a                        ; write X + spritesWidth

    ld a,(#_spritesTileOffset)       ; load current sprite tile offset
    add a,e                          ; add to tile number
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset

#ifdef NO_SPRITE_ZOOM
    ld a,#16
#else
    ld a,(#_spritesWidth)            ; load current sprite width
    add a,a                          ; double it
#endif

    add a,d                          ; add to X
    jr c,_thirdSpriteClipped         ; if new X is overflowing, do not place third sprite and leave

    inc hl
    ld (hl),a                        ; write X + spritesWidth

    ld a,(#_spritesTileOffset)       ; load current sprite tile offset
    add a,a                          ; double it
    add a,e                          ; add to tile number
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset*2

    ld a,(#_SpriteNextFree)
    add a,#2
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret

_thirdSpriteClipped:
    ld  hl,#_SpriteNextFree
    inc (hl)                         ; increment SpriteNextFree again
    ret
  __endasm;
}
#else
void SMS_addThreeAdjoiningSprites_f (unsigned int y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1) {
  // Y passed in L
  // X passed in D
  // tile passed in E
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES-2
    ret nc                           ; we do not have 3 sprites left, leave!
    ld  c,a                          ; save SpriteNextFree value in c

    ld  a,l
    cp  a,#0xd1
    ret z                            ; invalid Y, leave!

    ld  b,#0x00
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
    ld (hl),d                        ; write X
    inc hl
    ld (hl),e                        ; write tile number

#ifdef NO_SPRITE_ZOOM
    ld a,#8                          ; each sprite is 8 pixels wide
#else
    ld a,(#_spritesWidth)            ; load current sprite width
#endif

    add a,d                          ; add to X
    jr c,_secondSpriteClipped        ; if new X is overflowing, do not place second sprite
    inc hl
    ld (hl),a                        ; write X + spritesWidth

    ld a,(#_spritesTileOffset)       ; load current sprite tile offset
    add a,e
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset

#ifdef NO_SPRITE_ZOOM
    ld a,#16                         ; each sprite is 8 pixels wide
#else
    ld a,(#_spritesWidth)            ; load current sprite width
    add a,a                          ; double that
#endif

    add a,d                          ; add to X
    jr c,_thirdSpriteClipped         ; if new X is overflowing, do not place third sprite
    inc hl
    ld (hl),a                        ; write X + spritesWidth*2

    ld a,(#_spritesTileOffset)       ; load current sprite tile offset
    add a,a                          ; double that
    add a,e
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
#endif
#pragma restore
