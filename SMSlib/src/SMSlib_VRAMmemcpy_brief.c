/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85
void SMS_VRAMmemcpy_brief (unsigned int dst, const void *src, unsigned char size) __naked __z88dk_callee __preserves_regs(iyh,iyl) __sdcccall(1) {
  // dst in hl
  // src in de
  // size onto the stack
__asm
  set 6, h           ; Set VRAM address
  rst #0x08

  ex de,hl           ; move src in hl

  pop de             ; pop ret address
  dec sp
  pop bc             ; size (in b)
  push de            ; push ret address

  ld c,#_VDPDataPort

1$:
  outi               ; 16
  jr  nz,1$          ; 12 = 28 (VRAM safe on GG too)
  ret                ; because this function is naked
__endasm;
}
#pragma restore
