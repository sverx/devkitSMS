/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"

// VRAM unsafe functions. Fast, but dangerous!
/* UNSAFE_SMS_loadNTiles() and UNSAFE_SMS_loadTiles() are macros that call UNSAFE_SMS_VRAMmemcpy() */

#pragma save
#pragma disable_warning 85
void * UNSAFE_SMS_VRAMmemcpy (unsigned int dst, const void *src, unsigned int size) __naked __z88dk_callee __preserves_regs(iyh,iyl) __sdcccall(1) {
  // dst in HL
  // src in DE
  // size onto the stack
  // returns a void pointer to src+size in HL
__asm
  set 6, h           ; set VRAM address write flag
  rst #0x08

  ex de,hl           ; move src in hl

  pop bc             ; pop ret address
  pop de             ; pop size in DE
  push bc            ; push ret address back into stack

1$:
  ld a,e
  and #0x80          ; keep only msb
  or d
  jr z,2$            ; jump when there are less than 128 bytes to transfer left

  ld c,#_VDPDataPort
  call _OUTI128      ; quickly transfer 128 bytes

  ex de,hl           ; preserve HL,DE
  ld bc,#128
  xor a              ; reset carry
  sbc hl,bc
  ex de,hl           ; restore HL,DE
  jp 1$

2$:
  ld a,e             ; size left is <128
  add a,a            ; calculate jump address (and reset carry)
  ret z              ; end here when size equals zero

  ld c,a
  ld b,#0            ; size * 2 in BC
  ex de,hl           ; preserve HL
  ld hl,#_outi_block
  sbc hl,bc
  ex de,hl           ; restore HL, jump address in DE

  ld c,#_VDPDataPort
  push de            ; push jump address onto stack
  ret                ; jump to address
__endasm;
}
#pragma restore
