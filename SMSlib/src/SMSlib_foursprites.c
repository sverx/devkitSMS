/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85

#ifdef NO_SPRITE_CHECKS
void SMS_addFourAdjoiningSprites_f (unsigned char y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1) {
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
    inc hl
    ld (hl),a                        ; write Y again for the fourth sprite (always as Y-1)

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
    ld b,a                           ; save it in B
#endif

    add a,d                          ; add to X
    ret c                            ; if new X is overflowing, do not place second sprite and leave

    inc hl
    ld (hl),a                        ; write X + spritesWidth

    ld a,(#_spritesTileOffset)       ; load current sprite tile offset
    ld c,a                           ; save it in C
    add a,e
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset

#ifdef NO_SPRITE_ZOOM
    ld a,#16
#else
    ld a,b                           ; load current sprite width from B
    add a,a                          ; double it
#endif

    add a,d                          ; add to X
    jr c,_thirdSpriteClipped         ; if new X is overflowing, do not place third sprite

    inc hl
    ld (hl),a                        ; write X + spritesWidth*2

    ld a,c                           ; load current sprite tile offset from C
    add a,a                          ; double it
    add a,e                          ; add to tile number
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset*2

#ifdef NO_SPRITE_ZOOM
    ld a,#24
#else
    ld a,b                           ; load current sprite width from B
    add a,a                          ; make it twice
    add a,b                          ; make it three times
#endif

    add a,d                          ; add to X
    jr c,_fourthSpriteClipped        ; if new X is overflowing, do not place fourth sprite

    inc hl
    ld (hl),a                        ; write X + spritesWidth*3

    ld a,c                           ; load current sprite tile offset from C
    add a,a                          ; double it
    add a,c                          ; make it three times
    add a,e                          ; add to tile number
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset*3

    ld a,(#_SpriteNextFree)
    add a,#3                         ; three more sprites added
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret

_thirdSpriteClipped:
    ld a,(#_SpriteNextFree)          ; third sprite has been clipped
    inc a                            ; so only one more sprite has been added
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret

_fourthSpriteClipped:
    ld a,(#_SpriteNextFree)          ; fourth sprite has been clipped
    add a,#2                         ; so only two more sprites have been added
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret
  __endasm;
}
#else
void SMS_addFourAdjoiningSprites_f (unsigned int y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1) {
  // Y passed in L
  // X passed in D
  // tile passed in E
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES-3
    ret nc                           ; we do not have 4 sprites left, leave!
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
    ld (hl),a                        ; write Y also for the third sprite (always as Y-1)
    inc hl
    ld (hl),a                        ; write Y once more for the fourth sprite (always as Y-1)

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
    ld b,a                           ; save it in B
#endif

    add a,d                          ; add to X
    jr c,_secondSpriteClipped        ; if new X is overflowing, do not place second sprite
    inc hl
    ld (hl),a                        ; write X + spritesWidth

    ld a,(#_spritesTileOffset)       ; load current sprite tile offset
    ld c,a                           ; save it in C
    add a,e
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset

#ifdef NO_SPRITE_ZOOM
    ld a,#16                         ; each sprite is 8 pixels wide
#else
    ld a,b                           ; load current sprite width
    add a,a                          ; double that
#endif

    add a,d                          ; add to X
    jr c,_thirdSpriteClipped         ; if new X is overflowing, do not place third sprite
    inc hl
    ld (hl),a                        ; write X + spritesWidth*2

    ld a,c                           ; load current sprite tile offset
    add a,a                          ; double that
    add a,e
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset*2

#ifdef NO_SPRITE_ZOOM
    ld a,#24                         ; each sprite is 8 pixels wide
#else
    ld a,b                           ; load current sprite width
    add a,a                          ; double that
    add a,b                          ; three times that
#endif

    add a,d                          ; add to X
    jr c,_fourthSpriteClipped        ; if new X is overflowing, do not place fourth sprite
    inc hl
    ld (hl),a                        ; write X + spritesWidth*3

    ld a,c                           ; load current sprite tile offset
    add a,a                          ; double that
    add a,c                          ; three times that
    add a,e
    inc hl
    ld (hl),a                        ; write tile number + spritesTileOffset*3

    ld a,(#_SpriteNextFree)          ; four sprites have been placed, then SpriteNextFree+=4
    add a,#4
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret

_secondSpriteClipped:
    ld a,(#_SpriteNextFree)          ; second sprite has been clipped, so just inc SpriteNextFree
    inc a
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret

_thirdSpriteClipped:
    ld a,(#_SpriteNextFree)          ; third sprite has been clipped, so just SpriteNextFree+=2
    add a,#2
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret

_fourthSpriteClipped:
    ld a,(#_SpriteNextFree)          ; fourth sprite has been clipped, so just SpriteNextFree+=3
    add a,#3
    ld (#_SpriteNextFree),a          ; save SpriteNextFree new value
    ret
 __endasm;
}
#endif
#pragma restore
