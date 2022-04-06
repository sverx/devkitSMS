/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

unsigned char SpriteTableY[MAXSPRITES];
unsigned char SpriteTableXN[MAXSPRITES*2];
unsigned char SpriteNextFree;

void SMS_initSprites (void) {
  SpriteNextFree=0;
}

#pragma save
#pragma disable_warning 85

#ifdef NO_SPRITE_CHECKS
// This version doesn't check if there are available sprites,
// nor it checks if sprite's Y is the sprite terminator,
// so it's faster... but you should know what you're doing
// Also, it doesn't return any sprite handle
// 1st ASM version: 170 CPU cycles
/*
void SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) __naked __preserves_regs(iyh,iyl) __sdcccall(0) {
  __asm
    ld  hl,#2
    add hl,sp
    ld  e,(hl)                       ; read X (in E)
    inc hl
    ld  a,(hl)                       ; read Y (in A)
    inc hl
    ld  d,(hl)                       ; read tile number (in D)

    ld  hl,#_SpriteNextFree          ; load current SpriteNextFree value
    ld  c,(hl)                       ; (in C)
    inc (hl)                         ; increment SpriteNextFree
    ld  b,#0x00
    ld  hl,#_SpriteTableY
    add hl,bc                        ; hl+=SpriteNextFree
    dec a
    ld (hl),a                        ; write Y (as Y-1)

    ld hl,#_SpriteTableXN
    sla c
    add hl,bc                        ; hl+=(SpriteNextFree*2)
    ld (hl),e                        ; write X
    inc hl
    ld (hl),d                        ; write tile number
    ret
 __endasm;
}
*/

// 1st sdcccall(1) ASM version: 116 CPU cycles
void SMS_addSprite_f (unsigned char y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1) {
  // Y passed in A
  // X passed in D
  // tile passed in E
  __asm
    ld  hl,#_SpriteNextFree          ; load current SpriteNextFree value
    ld  c,(hl)                       ; (in C)
    inc (hl)                         ; increment SpriteNextFree
    ld  b,#0x00
    ld  hl,#_SpriteTableY
    add hl,bc                        ; hl+=SpriteNextFree
    dec a
    ld (hl),a                        ; write Y (as Y-1)

    ld hl,#_SpriteTableXN
    sla c
    add hl,bc                        ; hl+=(SpriteNextFree*2)
    ld (hl),d                        ; write X
    inc hl
    ld (hl),e                        ; write tile number
    ret
 __endasm;
}

#else
// 3rd ASM version: 212 CPU cycles
/*
signed char SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) __naked __preserves_regs(iyh,iyl) __sdcccall(0) {
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES
    jr  nc,_returnInvalidHandle1     ; no sprites left, leave!
    ld  c,a                          ; save SpriteNextFree value in c

    ld  hl,#2
    add hl,sp
    ld  e,(hl)                       ; read X
    inc hl
    ld  a,(hl)                       ; read Y
    cp  a,#0xd1
    jr  z,_returnInvalidHandle2      ; invalid Y, leave!
    inc hl
    ld  d,(hl)                       ; read tile number

    ld  hl,#_SpriteTableY
    ld  b,#0x00
    add hl,bc                        ; hl+=SpriteNextFree
    dec a
    ld (hl),a                        ; write Y (as Y-1)

    ld hl,#_SpriteTableXN
    ld a,c                           ; save sprite handle to A
    sla c
    add hl,bc                        ; hl+=(SpriteNextFree*2)
    ld (hl),e                        ; write X
    inc hl
    ld (hl),d                        ; write tile number

    ld l,a                           ; sprite handle to return
    inc a                            ; increment and
    ld (#_SpriteNextFree),a          ;    save SpriteNextFree value
    ret

_returnInvalidHandle1:
    ld l,#0xff
    ret

_returnInvalidHandle2:
    ld l,#0xfe
    ret
 __endasm;
}
*/

// 1st sdcccall(1) ASM version: 162 CPU cycles
signed char SMS_addSprite_f (unsigned int y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1) {
  // Y passed in L
  // X passed in D
  // tile passed in E
  // return value will be in A
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES
    jr  nc,_returnInvalidHandle1     ; no sprites left, leave!
    ld  c,a                          ; save SpriteNextFree value in c

    ld  a,l
    cp  a,#0xd1
    jr  z,_returnInvalidHandle2      ; invalid Y, leave!

    ld  hl,#_SpriteTableY
    ld  b,#0x00
    add hl,bc                        ; hl+=SpriteNextFree
    dec a
    ld (hl),a                        ; write Y (as Y-1)

    ld hl,#_SpriteTableXN
    ld a,c                           ; save sprite handle to A
    sla c
    add hl,bc                        ; hl+=(SpriteNextFree*2)
    ld (hl),d                        ; write X
    inc hl
    ld (hl),e                        ; write tile number

    inc a                            ; increment and
    ld (#_SpriteNextFree),a          ; save SpriteNextFree value
    dec a                            ; sprite handle to return
    ret

_returnInvalidHandle1:
    ld a,#0xff
    ret

_returnInvalidHandle2:
    ld a,#0xfe
    ret
 __endasm;
}
#endif
#pragma restore

void SMS_copySpritestoSAT (void) {
  SMS_setAddr(SMS_SATAddress);
  __asm
    ld a,(#_SpriteNextFree)
    or a
    jr z,_no_sprites
    ld b,a
    ld c,#_VDPDataPort
    ld hl,#_SpriteTableY
_next_spriteY:
    outi                    ; 16 cycles
    jr nz,_next_spriteY     ; 12 cycles = 28 (VRAM safe on GG too)
    cp #64                  ;  7 cycles
    jr z,_no_sprite_term    ;  7 cycles
    ld a,#0xD0              ;  7 cycles   =>  VRAM safe
    out (c),a
_no_sprite_term:
  __endasm;
  SMS_setAddr(SMS_SATAddress+128);
  __asm
    ld c,#_VDPDataPort
    ld a,(#_SpriteNextFree)
    add a,a
    ld b,a
    ld hl,#_SpriteTableXN
_next_spriteXN:
    outi                    ; 16 cycles
    jr nz,_next_spriteXN    ; 12 cycles = 28 (VRAM safe on GG too)
    ret

_no_sprites:
    ld a,#0xD0
    out (#_VDPDataPort),a
  __endasm;
}
