/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"
#include <stdbool.h>

unsigned int  spritesHeight=8, spritesWidth=8;
unsigned char SpriteTable[MAXSPRITES*4];
unsigned char SpriteNextFree;

#ifdef AUTODETECT_SPRITE_OVERFLOW
unsigned char spriteOverflowFlipflop;
unsigned char spriteOverflowCounter;
volatile bool VDPSpriteCollision;
volatile bool VDPSpriteOverflow;
#endif

void SG_setSpriteMode (unsigned char mode) {
  if (mode & SG_SPRITEMODE_LARGE) {
    SG_VDPturnOnFeature(SG_VDPFEATURE_USELARGESPRITES);
    spritesHeight=16;
    spritesWidth=16;
  } else {
    SG_VDPturnOffFeature(SG_VDPFEATURE_USELARGESPRITES);
    spritesHeight=8;
    spritesWidth=8;
  }
  if (mode & SG_SPRITEMODE_ZOOMED) {
    SG_VDPturnOnFeature(SG_VDPFEATURE_ZOOMSPRITES);
    spritesWidth*=2;
    spritesHeight*=2;
  } else {
    SG_VDPturnOffFeature(SG_VDPFEATURE_ZOOMSPRITES);
  }
}

void SG_initSprites (void) {
  SpriteNextFree=0;
#ifdef AUTODETECT_SPRITE_OVERFLOW
  spriteOverflowCounter=0;
  spriteOverflowFlipflop=1-spriteOverflowFlipflop;
#endif
}

void SG_finalizeSprites (void) {
  // now deprecated, does nothing
}

#pragma save
#pragma disable_warning 85
_Bool SG_addSprite_f (unsigned int x_y, unsigned int tilenum_attrib) __naked __preserves_regs(d,e,iyh,iyl) {
  // X passed in H
  // Y passed in L
  // tilenum passed in D
  // attrib passed in E
  // return value will be in A
  __asm
    ld  a,(#_SpriteNextFree)
    cp  a,#MAXSPRITES
    jr  nc,_returnFalse              ; no sprites left, leave!
    ld  c,a                          ; save SpriteNextFree value in c

    ld  a,l
    dec a
    cp  a,#0xd0
    jr z,_skipped                    ; invalid Y, leave!
    ld b,a                           ; save Y in B

    ld a,c                           ; restore SpriteNextFree
    add a,a
    add a,a

    ld c,h                           ; save X in C

    ld  hl,#_SpriteTable
    add a,l
    ld l,a
    ld a,h
    adc #0
    ld h,a

    ld (hl),b                        ; write Y
    inc hl
    ld (hl),c                        ; write X
    inc hl
    ld (hl),d                        ; write tilenum
    inc hl
    ld (hl),e                        ; write attrib

    ld hl,#_SpriteNextFree
    inc (hl)                         ; increment SpriteNextFree value

_skipped:
    ld a,#1                          ; return true
    ret

_returnFalse:
    xor a                            ; return false
    ret
 __endasm;
}
#pragma restore

void SG_copySpritestoSAT (void) __naked __preserves_regs(d,e,iyh,iyl) {
__asm
    ld hl,#SATADDRESS+0x4000
    ld c,#_VDPControlPort
__endasm;

    VDP_SEMAPHORE_ON;

__asm
    out (c),l
    out (c),h
__endasm;

    VDP_SEMAPHORE_OFF;

__asm
    dec c                   ; same as ld c,#_VDPDataPort
    ld a,(#_SpriteNextFree)
    or a
    jr z,_no_sprites
    add a,a
    add a,a
    ld b,a                  ; now B is number of used sprites*4
    ld hl,#_SpriteTable
_inner_loop:
    outi                    ; 16 cycles
    jr nz,_inner_loop       ; 12 cycles = 28 (VRAM safe)
    cp #32                  ;  7 cycles
    ret z                   ;  5 cycles when ret not taken
_no_sprites:
    ld a,#0xD0              ;  7 cycles = 31 (VRAM safe)
    out (c),a
    ret                     ; because this function is naked
__endasm;
}
