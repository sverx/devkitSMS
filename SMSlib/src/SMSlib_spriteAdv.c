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

signed char SMS_reserveSprite (void) {
  if (SpriteNextFree<MAXSPRITES) {
    SpriteTableY[SpriteNextFree]=0xE0;            // so it's offscreen
    return(SpriteNextFree++);
  }
  return (-1);
}

void SMS_updateSpritePosition (signed char sprite, unsigned char x, unsigned char y) {
  if (y!=0xD1) {                                  // avoid placing sprites at this Y!
    SpriteTableY[(unsigned char)sprite]=(unsigned char)(y-1);
    SpriteTableXN[(unsigned char)sprite*2]=x;
  } else {
    SpriteTableY[(unsigned char)sprite]=0xE0;     // move it offscreen anyway
  }
}

void SMS_updateSpriteImage (signed char sprite, unsigned char image) {
  SpriteTableXN[(unsigned char)sprite*2+1]=image;
}

void SMS_hideSprite (signed char sprite) {
  SpriteTableY[(unsigned char)sprite]=0xE0;          // move it offscreen
}
