/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"
#include <stdbool.h>

unsigned int  spritesHeight=8, spritesWidth=8;
#if MAXSPRITES==32
unsigned char SpriteTable[MAXSPRITES*4];
#else
unsigned char SpriteTable[(MAXSPRITES+1)*4];
#endif
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

_Bool SG_addSprite (unsigned char x, unsigned char y, unsigned char tile, unsigned char attr) {
  unsigned char idx;
  if (SpriteNextFree < MAXSPRITES) {
    if (--y != 0xd0) {
#ifdef AUTODETECT_SPRITE_OVERFLOW
      spriteOverflowCounter ++;
      if (0 == VDPSpriteOverflow || (spriteOverflowCounter & 1) == spriteOverflowFlipflop)
#endif
      {
        idx = SpriteNextFree << 2;
        SpriteTable [idx ++] = y;
        SpriteTable [idx ++] = x;
        SpriteTable [idx ++] = tile;
        SpriteTable [idx] = attr;
        SpriteNextFree ++;
      }
    }
    return true;
  } else
    return false;
}

void SG_finalizeSprites (void) {
#if MAXSPRITES==32
  if (SpriteNextFree < MAXSPRITES)
#endif
    SpriteTable[SpriteNextFree<<2]=0xd0;
}

inline void SG_byte_brief_array_to_VDP_data (const unsigned char *data, unsigned char size) {
  /* INTERNAL FUNCTION */
  do {
    VDPDataPort = *(data ++);
  } while (-- size);
}

void SG_copySpritestoSAT (void) {
  SG_set_address_VRAM (SATADDRESS);
#if MAXSPRITES==32
  SG_byte_brief_array_to_VDP_data (SpriteTable, MAXSPRITES*4);
#else
  SG_byte_brief_array_to_VDP_data (SpriteTable, (MAXSPRITES+1)*4);
#endif
}
