/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#if MAXSPRITES==64
unsigned char SpriteTableY[MAXSPRITES];
#else
unsigned char SpriteTableY[MAXSPRITES+1];
#endif
unsigned char SpriteTableXN[MAXSPRITES*2];
unsigned char SpriteNextFree;

void SMS_initSprites (void) {
  SpriteNextFree=0;
}

#pragma save
#pragma disable_warning 85

// 3rd ASM version: 212 CPU cycles
signed char SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) __naked __preserves_regs(iyh,iyl) {
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES
    jr  nc,_returnInvalidHandle1     ; no sprites left, leave!
    ld  c,a                          ; save SpriteNextFree value in c
    ld  b,#0x00

    ld  hl,#2
    add hl,sp
    ld  e,(hl)                       ; read X
    inc hl
    ld  a,(hl)                       ; read Y
    cp  a,#0xd1
    jr  z,_returnInvalidHandle2      ; invalid Y, leave!
    inc hl
    ld  d,(hl)                       ; read tile number

    ld  hl,#_SpriteTableY
    add hl,bc
    dec a
    ld (hl),a                        ; write Y  (as Y-1)

    ld hl,#_SpriteTableXN
    ld a,c                           ; preserve SpriteNextFree value in a
    sla c
    add hl,bc
    ld (hl),e                        ; write X
    inc hl
    ld (hl),d                        ; write tile number

    ld l,a                           ; sprite handle to return
    inc a                            ; increment and
    ld (#_SpriteNextFree),a          ;    save SpriteNextFree value
    ret

_returnInvalidHandle1:
    ld l,#0xff
    ret

_returnInvalidHandle2:
    ld l,#0xfe
    ret
 __endasm;
}
#pragma restore

void SMS_finalizeSprites (void) {
#if MAXSPRITES==64
  if (SpriteNextFree<MAXSPRITES)
#endif
    SpriteTableY[SpriteNextFree]=0xD0;
}

void SMS_copySpritestoSAT (void) {
  // SMS_set_address_VRAM(SMS_SATAddress);
  SMS_setAddr(SMS_SATAddress);
#if MAXSPRITES==64
  SMS_byte_brief_array_to_VDP_data(SpriteTableY,MAXSPRITES);
#else
  SMS_byte_brief_array_to_VDP_data(SpriteTableY,MAXSPRITES+1);
#endif
  // SMS_set_address_VRAM(SMS_SATAddress+128);
  SMS_setAddr(SMS_SATAddress+128);
  SMS_byte_brief_array_to_VDP_data(SpriteTableXN,MAXSPRITES*2);
}
