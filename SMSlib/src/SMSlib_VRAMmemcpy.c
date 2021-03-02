/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

/* low level functions */

#pragma save
#pragma disable_warning 85
void SMS_VRAMmemcpy (unsigned int dst, const void *src, unsigned int size) {
  //  optimized (faster looping) ASM code (SDCC generated then hand optimized)  
__asm
  push  ix
  ld  ix,#0
  add ix,sp

  ld  l, 4 (ix)
  ld  a, 5 (ix)
  set 6, a
  ld  h, a
  rst #0x08

  ld  l,6 (ix)
  ld  h,7 (ix)
  ld  a,8 (ix)       ; LO(size)
  or  a
  ld  b,a
  ld  a,9 (ix)       ; HI(size)
  jr  Z,noinc        ; if LO(size) is zero, do not inc HI(size)
  inc a              ; inc HI(size) because LO(size) is not zero
noinc:
  ld c,#_VDPDataPort
copyloop:
  outi
  jp  nz,copyloop    ; 10 = 26 (VRAM safe)
  dec a
  jp  nz,copyloop
  pop ix
__endasm;
}

void SMS_VRAMmemcpy_brief (unsigned int dst, const void *src, unsigned char size) {
  //  optimized (faster looping) ASM code (SDCC generated then hand optimized)
__asm
  push  ix
  ld  ix,#0
  add ix,sp
;SMSlib_VRAMmemcpy.c:12: SMS_setAddr(0x4000|dst);
  ld  l, 4 (ix)
  ld  a, 5 (ix)
  set 6, a
  ld  h, a
  rst #0x08
;SMSlib_VRAMmemcpy.c:13: SMS_byte_brief_array_to_VDP_data(src,size);
  ld  l,6 (ix)
  ld  h,7 (ix)
  ld  b,8 (ix)             ; size
copyloop_brief:
  outi                     ; 16
  jp  nz,copyloop_brief    ; 10 = 26 (VRAM safe)
  pop ix
__endasm;
}
#pragma restore
