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
    add a,a                 ; SpriteNextFree*=2 (and reset carry)
    ld c,a
    ld b,#0
    ld hl,#_outi_block
    sbc hl,bc
    ex de,hl
    ld hl,#_SpriteTableY
    call _do_copy_Y
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
    dec a                   ; there is surely at least one sprite used
    add a,a
    add a,a                 ; a=(SpriteNextFree-1)*4 (and reset carry)
    ld c,a
    ld b,#0
    ld hl,#_outi_block-4
    sbc hl,bc
    push hl                 ; push jump address into stack
    ld hl,#_SpriteTableXN
    ld c,#_VDPDataPort
    ret                     ; get jump address from stack
_do_copy_Y:
    ld c,#_VDPDataPort
    push de
    ret                     ; get jump address from stack
_no_sprites:
    ld a,#0xD0
    out (#_VDPDataPort),a
  __endasm;
}

/*
// previous version
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
    push de                ; same as jp (de)
    ret                    ; which does not exists
_no_sprites:
    ld a,#0xD0
    out (#_VDPDataPort),a
  __endasm;
}
*/

void OUTI32(void *src) __z88dk_fastcall;
void OUTI64(void *src) __z88dk_fastcall;
void OUTI128(void *src) __z88dk_fastcall;

#define SETVDPDATAPORT  __asm ld c,#_VDPDataPort __endasm

void UNSAFE_SMS_VRAMmemcpy32 (unsigned int dst, void *src) {
  SMS_setAddr(0x4000|dst);
  SETVDPDATAPORT;
  OUTI32(src);
}

void UNSAFE_SMS_VRAMmemcpy64 (unsigned int dst, void *src) {
  SMS_setAddr(0x4000|dst);
  SETVDPDATAPORT;
  OUTI64(src);
}

void UNSAFE_SMS_VRAMmemcpy128 (unsigned int dst, void *src) {
  SMS_setAddr(0x4000|dst);
  SETVDPDATAPORT;
  OUTI128(src);
}
