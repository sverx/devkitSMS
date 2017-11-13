/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#if MAXSPRITES==64
extern unsigned char SpriteTableY[MAXSPRITES];
#else
extern unsigned char SpriteTableY[MAXSPRITES+1];
#endif
extern unsigned char SpriteTableXN[MAXSPRITES*2];
extern unsigned char SpriteNextFree;

// VRAM unsafe functions. Fast, but dangerous!
void UNSAFE_SMS_copySpritestoSAT (void) {
  SMS_setAddr(SMS_SATAddress);
  __asm
    ld a,(#_SpriteNextFree)
    or a
    jr z,_no_sprites
    ld b,a
    ld a,#64
    sub a,b
    add a,a
    ld c,a
    ld b,#0
    ld hl,#_OUTI64
    add hl,bc
    ex de,hl
    ld hl,#_SpriteTableY
    call _start_cpy
    ld a,(#_SpriteNextFree)
    cp #64
    jr z,_no_sprite_term
    ld a,#0xD0
    out (c),a
_no_sprite_term:
  __endasm;
 SMS_setAddr(SMS_SATAddress+128);
  __asm
    ld a,(#_SpriteNextFree)
    ld b,a
    ld a,#64
    sub a,b
    add a,a
    add a,a
    ld c,a
    ld b,#0
    ld hl,#_OUTI128
    add hl,bc    
    ex de,hl
    ld hl,#_SpriteTableXN
_start_cpy:
    ld c,#_VDPDataPort
    push de
    pop iy
    jp (iy)
_no_sprites:
    ld a,#0xD0
    out (#_VDPDataPort),a
  __endasm;
}

/*
// previous version - profiling: 3283 cycles (14 scanlines)
void UNSAFE_SMS_copySpritestoSAT (void) {
  // SMS_set_address_VRAM(SMS_SATAddress);
  SMS_setAddr(SMS_SATAddress);
  __asm
    ld c,#_VDPDataPort
    ld hl,#_SpriteTableY
#if MAXSPRITES==64
    call _outi_block-MAXSPRITES*2
#else
    call _outi_block-(MAXSPRITES+1)*2
#endif
  __endasm;
  // SMS_set_address_VRAM(SMS_SATAddress+128);
  SMS_setAddr(SMS_SATAddress+128);
  __asm
    ld c,#_VDPDataPort
    ld hl,#_SpriteTableXN
    call _outi_block-MAXSPRITES*4
  __endasm;
}
*/

void OUTI32(void *src) __z88dk_fastcall;
void OUTI64(void *src) __z88dk_fastcall;
void OUTI128(void *src) __z88dk_fastcall;

#define SETVDPDATAPORT  __asm ld c,#_VDPDataPort __endasm

void UNSAFE_SMS_VRAMmemcpy32 (unsigned int dst, void *src) {
  // SMS_set_address_VRAM(dst);
  SMS_setAddr(0x4000|dst);
  SETVDPDATAPORT;
  OUTI32(src);
}

void UNSAFE_SMS_VRAMmemcpy64 (unsigned int dst, void *src) {
  // SMS_set_address_VRAM(dst);
  SMS_setAddr(0x4000|dst);
  SETVDPDATAPORT;
  OUTI64(src);
}

void UNSAFE_SMS_VRAMmemcpy128 (unsigned int dst, void *src) {
  // SMS_set_address_VRAM(dst);
  SMS_setAddr(0x4000|dst);
  SETVDPDATAPORT;
  OUTI128(src);
}
