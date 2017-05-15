/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

/* low level functions */

#pragma save
#pragma disable_warning 85
void SMS_VRAMmemcpy (unsigned int dst, void *src, unsigned int size) {
  //  original code:
  // SMS_setAddr(0x4000|dst);
  // SMS_byte_array_to_VDP_data(src,size);
  //  optimized (faster looping) ASM code (SDCC generated then hand optimized)
__asm
	push	ix
	ld	ix,#0
	add	ix,sp
;SMSlib_VRAMmemcpy.c:12: SMS_setAddr(0x4000|dst);
	ld	l, 4 (ix)
	ld	a, 5 (ix)
	set	6, a
	ld	h, a
	rst	#0x08
;SMSlib_VRAMmemcpy.c:13: SMS_byte_array_to_VDP_data(src,size);
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

void SMS_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size) {
  //  original code:
  // SMS_setAddr(0x4000|dst);
  // SMS_byte_brief_array_to_VDP_data(src,size);
  //  optimized (faster looping) ASM code (SDCC generated then hand optimized)
__asm
	push	ix
	ld	ix,#0
	add	ix,sp
;SMSlib_VRAMmemcpy.c:12: SMS_setAddr(0x4000|dst);
	ld	l, 4 (ix)
	ld	a, 5 (ix)
	set	6, a
	ld	h, a
	rst	#0x08
;SMSlib_VRAMmemcpy.c:13: SMS_byte_brief_array_to_VDP_data(src,size);
	ld	l,6 (ix)
	ld	h,7 (ix)
	ld	b,8 (ix)       ; size
	outi                   ; 16
	jr	NZ,_copyloop   ; 12 = 28 (VRAM safe)
	pop	ix
__endasm;
}
#pragma restore
