/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#if MAXSPRITES==64
unsigned char SpriteTableY[MAXSPRITES];
#else
unsigned char SpriteTableY[MAXSPRITES+1];
#endif
unsigned char SpriteTableXN[MAXSPRITES*2];
unsigned char SpriteNextFree;

void SMS_initSprites (void) {
  SpriteNextFree=0;
}

#pragma save
#pragma disable_warning 85

// 3rd ASM version: 212 CPU cycles
signed char SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) __naked __preserves_regs(iyh,iyl) {
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
    ld (hl),a                        ; write Y  (as Y-1)

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
#pragma restore

void SMS_copySpritestoSAT (void) {
  /*
  SMS_setAddr(SMS_SATAddress);
  if (SpriteNextFree)
    SMS_byte_brief_array_to_VDP_data(SpriteTableY,SpriteNextFree);
  if (SpriteNextFree<64)
    SMS_byte_to_VDP_data(0xD0);  // write sprite terminator byte if needed
  if (SpriteNextFree) {
    SMS_setAddr(SMS_SATAddress+128);
    SMS_byte_brief_array_to_VDP_data(SpriteTableXN,SpriteNextFree*2);
  }
  */
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
    jp nz,_next_spriteY     ; 10 cycles   =>  VRAM safe
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
    outi                    ; 16       cycles
    jp nz,_next_spriteXN    ; 10       cycles   =>  VRAM safe
    ret

_no_sprites:
    ld a,#0xD0
    out (#_VDPDataPort),a
  __endasm;
}

/*
// previous code: copies the whole SAT, even unused entries
//     profiling: 10916 cycles (48 screen lines)
void SMS_copySpritestoSAT (void) {
  // SMS_set_address_VRAM(SMS_SATAddress);
  SMS_setAddr(SMS_SATAddress);
#if MAXSPRITES==64
  SMS_byte_brief_array_to_VDP_data(SpriteTableY,MAXSPRITES);
#else
  SMS_byte_brief_array_to_VDP_data(SpriteTableY,MAXSPRITES+1);
#endif
  // SMS_set_address_VRAM(SMS_SATAddress+128);
  SMS_setAddr(SMS_SATAddress+128);
  SMS_byte_brief_array_to_VDP_data(SpriteTableXN,MAXSPRITES*2);
}
*/
