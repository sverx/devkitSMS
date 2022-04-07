/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

/* low level functions, just to be used for dirty tricks ;) */
/*
void SMS_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size) {
  // SMS_set_address_VRAM(dst);
  SMS_setAddr(0x4000|dst);
  while (size>0) {
    SMS_byte_to_VDP_data(value);
    size--;
  }
}
*/

#pragma save
#pragma disable_warning 85
void SMS_VRAMmemset_f (unsigned char value, unsigned int dst, unsigned int size) __naked __z88dk_callee __preserves_regs(a,h,l,iyh,iyl) __sdcccall(1) {
  // value in a
  // dst in de
  // size onto the stack
__asm
  set 6, d           ; Set VRAM address
  ld c, #_VDPControlPort
  di
  out (c),e
  out (c),d
  ei

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
  djnz 1$              ; 13 = 28 (VRAM safe on GG too)
  dec e
  jp  nz,1$
  ret                ; because this function is naked
__endasm;
}

void SMS_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size) __naked __z88dk_callee __preserves_regs(d,e,iyh,iyl) __sdcccall(1) {
/*
  SMS_setAddr(0x4000|dst);
  while (size>0) {
    SMS_byte_to_VDP_data(LO(value));
    WAIT_VRAM;
    SMS_byte_to_VDP_data(HI(value));
    size-=2;
  }
*/
  // dst in hl
  // value in de
  // size onto the stack
__asm
  set 6, h           ; Set VRAM address
  rst #0x08

  pop bc             ; pop ret address
  pop hl             ; pop size
  push bc            ; push ret address

  ld c,#_VDPDataPort

1$:
  out(c),e           ; 12
  dec hl             ;  6
  dec hl             ;  6
  ld a,h             ;  4 = 28 (VRAM safe on GG too)
  out(c),d           ; 12
  or a,l             ;  4
  jr nz,1$           ; 12 = 28 (VRAM safe on GG too)
  ret                ; because this function is naked
__endasm;
}
#pragma restore
