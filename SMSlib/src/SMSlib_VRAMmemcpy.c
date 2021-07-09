/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

/* SMS_loadTiles() and SMS_loadTileMap() are just calls to SMS_VRAMmemcpy() */

#pragma save
#pragma disable_warning 85
void SMS_VRAMmemcpy (unsigned int dst, const void *src, unsigned int size) __naked __z88dk_callee __preserves_regs(iyh,iyl) {
  //  handwritten asm code
__asm

  pop de             ; pop ret address
  pop hl             ; dst
  set 6, h
  rst #0x08

  pop hl             ; src
  pop bc             ; size
  push de            ; push ret address

  dec bc
  inc b
  inc c              ; increment B if C is not zero

  ld a,b             ; HI(size)
  ld b,c             ; LO(size)

  ld c,#_VDPDataPort

1$:
  outi
  jp  nz,1$          ; 10 = 26 (VRAM safe)
  dec a
  jp  nz,1$
  ret
__endasm;
}
#pragma restore
