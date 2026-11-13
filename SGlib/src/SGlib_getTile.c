/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"

#pragma save
#pragma disable_warning 85
void SG_getNextTileatAddr (unsigned int addr) __naked __preserves_regs(a,b,d,e,l,iyh,iyl) {
  // addr in HL
__asm
  res 6, h           ; clear VRAM write flag (as we want to read)
  ld c,#_VDPControlPort
__endasm;

  VDP_SEMAPHORE_ON;

__asm
  out (c),l
  out (c),h
__endasm;

  VDP_SEMAPHORE_OFF;

__asm
  ret                ; this is naked
__endasm;
}

unsigned char SG_getTile (void) __naked __preserves_regs(b,c,d,e,h,l,iyh,iyl) {
  // returns value in A
__asm
  in a,(#_VDPDataPort)
  ret                ; this is naked
__endasm;
}
#pragma restore
