/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"
#include <stdbool.h>

extern unsigned int  spritesHeight, spritesWidth;
#ifdef TARGET_GG
unsigned char clipWin_x0=48,clipWin_y0=24,clipWin_x1=207,clipWin_y1=167;
#else
unsigned char clipWin_x0=0,clipWin_y0=0,clipWin_x1=255,clipWin_y1=191;
#endif

#if MAXSPRITES==64
extern unsigned char SpriteTableY[MAXSPRITES];
#else
extern unsigned char SpriteTableY[MAXSPRITES+1];
#endif
extern unsigned char SpriteTableXN[MAXSPRITES*2];
extern unsigned char SpriteNextFree;

void SMS_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1) {
  clipWin_x0=x0;
  clipWin_y0=y0;
  clipWin_x1=x1;
  clipWin_y1=y1;
}

signed char SMS_addSpriteClipping (int x, int y, unsigned char tile) {
  unsigned char *stXN;
  if (SpriteNextFree<MAXSPRITES) {
    if ((x>clipWin_x1) || (x<((int)clipWin_x0-spritesWidth)))
      return (false);                               // sprite clipped
    if ((y>clipWin_y1) || (y<((int)clipWin_y0-spritesHeight)))
      return (false);                               // sprite clipped
    if ((y-1)!=0xD0) {                              // avoid placing sprites at this Y!
      SpriteTableY[SpriteNextFree]=y-1;
      stXN=&SpriteTableXN[SpriteNextFree*2];
      *stXN++=x;
      *stXN=tile;
      /* old code was:
      SpriteTableXN[SpriteNextFree*2]=x;
      SpriteTableXN[SpriteNextFree*2+1]=tile;
      */
      return(SpriteNextFree++);
    }
  }
  return (-1);
}
