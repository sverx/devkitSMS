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

/*

signed char SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) {
  unsigned char *stXN;
  if (SpriteNextFree<MAXSPRITES) {
    if (y!=0xD1) {                     // avoid placing sprites at this Y!
      SpriteTableY[SpriteNextFree]=(unsigned char)(y-1);
      stXN=&SpriteTableXN[SpriteNextFree*2];
      *stXN++=x;
      *stXN=tile;
      // old code was:
      // SpriteTableXN[SpriteNextFree*2]=x;
      // SpriteTableXN[SpriteNextFree*2+1]=tile;
      return(SpriteNextFree++);
    }
  }
  return (-1);
}

//
//  SDCC generated quite a nice code, here it is,
//  but it can be improved by not using IY and some more asm shortcuts
//

;SMSlib.c:241: signed char SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) {
;	---------------------------------
; Function SMS_addSprite
; ---------------------------------
_SMS_addSprite::
;SMSlib.c:243: if (SpriteNextFree<MAXSPRITES) {
	ld	a,(#_SpriteNextFree + 0)
	sub	a, #0x40
	jr	NC,00104$
;SMSlib.c:244: if (y!=0xD1) {                     // avoid placing sprites at this Y!
	ld	iy, #3
	add	iy, sp
	ld	a, 0 (iy)
	sub	a, #0xd1
	jr	Z,00104$
;SMSlib.c:245: SpriteTableY[SpriteNextFree]=(unsigned char)(y-1);
	ld	a, #<(_SpriteTableY)
	ld	hl, #_SpriteNextFree
	add	a, (hl)
	ld	c, a
	ld	a, #>(_SpriteTableY)
	adc	a, #0x00
	ld	b, a
	ld	e, 0 (iy)
	dec	e
	ld	a, e
	ld	(bc), a
;SMSlib.c:246: stXN=&SpriteTableXN[SpriteNextFree*2];
	ld	a,(#_SpriteNextFree + 0)
	add	a, a
	ld	c, a
	ld	hl, #_SpriteTableXN
	ld	b, #0x00
	add	hl, bc
;SMSlib.c:247: *stXN++=x;
	ld	iy, #2
	add	iy, sp
	ld	a, 0 (iy)
	ld	(hl), a
	inc	hl
;SMSlib.c:248: *stXN=tile;
	ld	iy, #4
	add	iy, sp
	ld	a, 0 (iy)
	ld	(hl), a
;SMSlib.c:253: return(SpriteNextFree++);
	ld	iy, #_SpriteNextFree
	ld	c, 0 (iy)
	inc	0 (iy)
	ld	l, c
	ret
00104$:
;SMSlib.c:256: return (-1);
	ld	l, #0xff
	ret

*/

#pragma save
#pragma disable_warning 85

/*
// 1st ASM version: 243 CPU cycles
signed char SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) __naked {
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES
    jr  nc,_returnInvalidHandle      ; no sprites left, leave!
    ld  c,a                          ; save SpriteNextFree value in c
    ld  b,#0x00

    ld  hl,#3
    add hl,sp
    ld  a,(hl)
    cp  a,#0xd1
    jr  z,_returnInvalidHandle       ; invalid Y, leave!

    ld  hl,#_SpriteTableY
    add hl,bc
    dec a
    ld (hl),a                        ; write Y  (as Y-1)

    ld  hl,#2
    add hl,sp
    ld  a,(hl)                       ; read X
    inc hl
    inc hl
    ld  d,(hl)                       ; read tile number

    ld hl,#_SpriteTableXN
    add hl,bc
    add hl,bc
    ld (hl),a                        ; write X
    inc hl
    ld (hl),d                        ; write tile number

    ld hl,#_SpriteNextFree
    ld a,(hl)
    inc (hl)
    ld l,a                           ; return sprite handle
    ret

_returnInvalidHandle:
    ld l,#0xff
    ret
 __endasm;
}
*/

// 2nd ASM version: 215 CPU cycles
signed char SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) __naked {
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES
    jr  nc,_returnInvalidHandle      ; no sprites left, leave!
    ld  c,a                          ; save SpriteNextFree value in c
    ld  b,#0x00

    ld  hl,#2
    add hl,sp
    ld  e,(hl)                       ; read X
    inc hl
    ld  a,(hl)
    cp  a,#0xd1
    jr  z,_returnInvalidHandle       ; invalid Y, leave!
    inc hl
    ld  d,(hl)                       ; read tile number

    ld  hl,#_SpriteTableY
    add hl,bc
    dec a
    ld (hl),a                        ; write Y  (as Y-1)

    ld hl,#_SpriteTableXN
    add hl,bc
    add hl,bc
    ld (hl),e                        ; write X
    inc hl
    ld (hl),d                        ; write tile number

    ld a,c
    ld l,a                           ; sprite handle to return
    inc a
    ld (#_SpriteNextFree),a          ; save new SpriteNextFree value
    ret

_returnInvalidHandle:
    ld l,#0xff
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
