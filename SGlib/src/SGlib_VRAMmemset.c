/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"

#pragma save
#pragma disable_warning 85
void SG_VRAMmemset_f (unsigned char value, unsigned int dst, unsigned int size) __naked __z88dk_callee __preserves_regs(a,h,l,iyh,iyl) {
  // value in a
  // dst in de
  // size onto the stack
__asm
  set 6, d           ; set VRAM write flag
  ld c, #_VDPControlPort
__endasm;

  VDP_SEMAPHORE_ON;

__asm
  out (c),e
  out (c),d
__endasm;

  VDP_SEMAPHORE_OFF;

__asm

  pop de             ; pop ret address
  pop bc             ; pop size
  push de            ; push ret address

  dec bc
  inc b
  inc c              ; increment B if C is not zero

  ld e,b             ; HI(size)
  ld b,c             ; LO(size)

1$:
  out(#_VDPDataPort),a ; 11
  nop                  ;  4
  djnz 1$              ; 13 = 28 (VRAM safe)
  dec e
  jp nz,1$
  ret                ; because this function is naked
__endasm;
}
#pragma restore

//~ inline void SG_byte_to_VDP_data (unsigned char data) {
  //~ /* INTERNAL FUNCTION */
  //~ VDPDataPort = data;
//~ }

//~ void SG_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size) {
  //~ SG_set_address_VRAM(dst);
  //~ while (size>0) {
    //~ SG_byte_to_VDP_data(value);
    //~ size--;
  //~ }
//~ }

//~ void SG_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size) {
  //~ SG_set_address_VRAM(dst);
  //~ while (size>0) {
    //~ SG_byte_to_VDP_data(LO(value));
    //~ WAIT_VRAM;                          /* ensure we're not pushing data too fast */
    //~ SG_byte_to_VDP_data(HI(value));
    //~ size-=2;
  //~ }
//~ }
