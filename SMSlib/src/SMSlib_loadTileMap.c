/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

void SMS_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size) {
  //  original code:
  //  SMS_setAddr(SMS_PNTAddress+(y*32+x)*2);
  //  SMS_byte_array_to_VDP_data(src,size);
  //  optimized (faster looping) ASM code (SDCC generated then hand optimized)
__asm
	push	ix
	ld	ix,#0
	add	ix,sp
;  SMS_setAddr(SMS_PNTAddress+(y*32+x)*2);
	ld	l,5 (ix)
	ld	h,#0x00
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	ld	c,4 (ix)
	ld	b,#0x00
	add	hl,bc
	add	hl, hl
	ld	bc,#0x7800
	add	hl,bc
	rst	#0x08
	
; SMS_byte_array_to_VDP_data(src,size);
	ld	l,6 (ix)
	ld	h,7 (ix)
	ld	a,8 (ix)       ; LO(size)
	or	a
	ld	b,a
	ld	a,9 (ix)       ; HI(size)
	jr	Z,_noinc       ; if LO(size) is zero, do not inc HI(size)
        inc	a              ; inc HI(size) because LO(size) in not zero
_noinc:
	ld	c,#_VDPDataPort
_copyloop:
	outi                   ; 16
	jr	NZ,_copyloop   ; 12 = 28 (VRAM safe)
	dec	a
	jr	NZ,_copyloop
	pop	ix
__endasm;
}
