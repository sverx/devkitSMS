/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"
#include <stdbool.h>

#ifndef MAXSPRITES
#define MAXSPRITES      32
#endif

extern unsigned int  spritesHeight, spritesWidth;

#ifdef AUTODETECT_SPRITE_OVERFLOW
extern unsigned char spriteOverflowFlipflop=0;
extern unsigned char spriteOverflowCounter;
extern volatile bool VDPSpriteCollision=false;
extern volatile bool VDPSpriteOverflow=false;
#endif

#if MAXSPRITES==32
extern unsigned char SpriteTable[MAXSPRITES*4];
#else
extern unsigned char SpriteTable[(MAXSPRITES+1)*4];
#endif
extern unsigned char SpriteNextFree;

unsigned char clipWin_x0=0, clipWin_y0=0, clipWin_x1=255, clipWin_y1=191;    // this is SG_setClippingWindow (0, 0, 255, 191);

void SG_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1) {
  clipWin_x0=x0;
  clipWin_y0=y0;
  clipWin_x1=x1;
  clipWin_y1=y1;
}

_Bool SG_addSpriteClipping (int x, int y, unsigned char tile, unsigned char attr) {
  unsigned char idx;
  if (SpriteNextFree < MAXSPRITES) {
    if ((x > clipWin_x1) || (x < ((int) clipWin_x0 - spritesWidth)))
      return false;                             // sprite clipped
    if ((y > clipWin_y1) || (y < ((int) clipWin_y0 - spritesHeight)))
      return false;                             // sprite clipped
    if (y - 1 != 0xd0) {
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
