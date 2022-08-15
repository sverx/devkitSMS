/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

/* SMS_loadTiles() and SMS_loadTileMap() are just calls to SMS_VRAMmemcpy() */

#pragma save
#pragma disable_warning 85
void SMS_VRAMmemcpy (unsigned int dst, const void *src, unsigned int size) __naked __z88dk_callee __preserves_regs(iyh,iyl) __sdcccall(1) {
  // dst in hl
  // src in de
  // size onto the stack
__asm
  set 6, h           ; Set VRAM address
  rst #0x08

  ex de,hl           ; move src in hl

  pop de             ; pop ret address
  pop bc             ; pop size
  push de            ; push ret address

  dec bc
  inc b
  inc c              ; increment B if C is not zero

  ld a,b             ; HI(size)
  ld b,c             ; LO(size)

  ld c,#_VDPDataPort

1$:
  outi               ; 16
  jr  nz,1$          ; 12 = 28 (VRAM safe on GG too)
  dec a
  jp  nz,1$
  ret                ; because this function is naked
__endasm;
}
#pragma restore
