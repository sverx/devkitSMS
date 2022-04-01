/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85
void SMS_VRAMmemcpy_brief (unsigned int dst, const void *src, unsigned char size) __naked __z88dk_callee __preserves_regs(iyh,iyl) {
  //  handwritten asm code
__asm

  pop de             ; pop ret address
  pop hl             ; dst
  set 6, h
  rst #0x08

  pop hl             ; src
  dec sp
  pop bc             ; size (in b)

  push de            ; push ret address

  ld c,#_VDPDataPort

1$:
  outi               ; 16
  jr  nz,1$          ; 12 = 28 (VRAM safe on GG too)
  ret
__endasm;
}
#pragma restore
