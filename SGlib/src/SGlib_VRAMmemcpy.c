/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"

#pragma save
#pragma disable_warning 85
void SG_VRAMmemcpy (unsigned int dst, const void *src, unsigned int size) __naked __z88dk_callee __preserves_regs(iyh,iyl) {
  // dst in hl
  // src in de
  // size onto the stack
__asm
  set 6, h           ; set VRAM write flag
  ld c,#_VDPControlPort
__endasm;

  VDP_SEMAPHORE_ON;

__asm
  out (c),l
  out (c),h
__endasm;

  VDP_SEMAPHORE_OFF;

__asm
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
  jr  nz,1$          ; 12 = 28 (VRAM safe)
  dec a
  jp  nz,1$
  ret                ; because this function is naked
__endasm;
}
#pragma restore
