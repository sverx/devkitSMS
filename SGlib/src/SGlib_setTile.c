/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"

#pragma save
#pragma disable_warning 85
void SG_setNextTileatAddr (unsigned int addr) __naked __preserves_regs(a,b,d,e,l,iyh,iyl) {
  // addr in HL
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
  ret                ; this is naked
__endasm;
}

void SG_setTile (unsigned char tile) __naked __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) {
  // tile in A
__asm
  out (#_VDPDataPort),a
  ret                ; this is naked
__endasm;
}
#pragma restore
