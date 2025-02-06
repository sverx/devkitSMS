/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

// VRAM unsafe functions. Fast, but dangerous!
/* UNSAFE_SMS_load*Tile(s) are macros to call these functions */

#pragma save
#pragma disable_warning 85
void * UNSAFE_SMS_VRAMmemcpy32 (unsigned int dst, const void *src) __naked __preserves_regs(a,iyh,iyl) __sdcccall(1) {
  // dst in hl
  // src in de
__asm
  set 6, h           ; set VRAM address write flag
  rst #0x08

  ex de,hl           ; move src in hl
  dec c              ; because c value is already #_VDPControlPort and this faster than using ld c,#_VDPDataPort
  jp _OUTI32         ; tail call optimization
__endasm;
}

void * UNSAFE_SMS_VRAMmemcpy64 (unsigned int dst, const void *src) __naked __preserves_regs(a,iyh,iyl) __sdcccall(1)  {
  // dst in hl
  // src in de
__asm
  set 6, h           ; set VRAM address write flag
  rst #0x08

  ex de,hl           ; move src in hl
  dec c              ; because c value is already #_VDPControlPort and this faster than using ld c,#_VDPDataPort
  jp _OUTI64         ; tail call optimization
__endasm;
}

void * UNSAFE_SMS_VRAMmemcpy96 (unsigned int dst, const void *src) __naked __preserves_regs(a,iyh,iyl) __sdcccall(1)  {
  // dst in hl
  // src in de
__asm
  set 6, h           ; set VRAM address write flag
  rst #0x08

  ex de,hl           ; move src in hl
  dec c              ; because c value is already #_VDPControlPort and this faster than using ld c,#_VDPDataPort
  jp _OUTI96         ; tail call optimization
__endasm;
}

void * UNSAFE_SMS_VRAMmemcpy128 (unsigned int dst, const void *src) __naked __preserves_regs(a,iyh,iyl) __sdcccall(1) {
  // dst in hl
  // src in de
__asm
  set 6, h           ; set VRAM address write flag
  rst #0x08

  ex de,hl           ; move src in hl
  dec c              ; because c value is already #_VDPControlPort and this faster than using ld c,#_VDPDataPort
  jp _OUTI128        ; tail call optimization
__endasm;
}
#pragma restore
